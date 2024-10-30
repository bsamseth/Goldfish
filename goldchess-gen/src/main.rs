use quote::{format_ident, quote};

use goldchess_gen::{
    attacks_from, find_magic, king_moves, knight_moves, number_of_relevant_occupancy_bits,
    occupancy_for_mask_and_index, occupancy_mask, pawn_attacks, pawn_quiets, Color, Square, BISHOP,
    ROOK,
};

macro_rules! gen_moves {
    ($name:ident, $func:ident) => {
        let moves = (0..64)
            .map(|sq| {
                let sq = unsafe { Square::new(sq) };
                let moves = $func(sq).0;

                quote! { #moves }
            })
            .collect::<Vec<_>>();

        let tokens = quote! {
            pub const $name: [u64; 64] = [
                #(#moves),*
            ];
        };
        pretty_print(&tokens);
    };
    ($name:ident, $func:ident, $color:expr) => {
        let moves = (0..64)
            .map(|sq| {
                let sq = unsafe { Square::new(sq) };
                let moves = $func(sq, $color).0;

                quote! { #moves }
            })
            .collect::<Vec<_>>();

        let tokens = quote! {
            pub const $name: [u64; 64] = [
                #(#moves),*
            ];
        };
        pretty_print(&tokens);
    };
}
macro_rules! gen_slider_moves {
    ($name:ident, $magics:expr) => {
        for sq in Square::ALL_SQUARES {
            let magic_number = find_magic::<$name>(sq);
            let relevant_bits = number_of_relevant_occupancy_bits::<$name>(sq);
            let shift = 64 - relevant_bits;

            let occupancies = 1 << relevant_bits;
            let mask = occupancy_mask::<$name>(sq);
            let mut moves = vec![0; occupancies];
            for index in 0..occupancies {
                let occ = occupancy_for_mask_and_index(mask, index);
                let mv = attacks_from::<$name>(sq, occ);

                #[allow(
                    clippy::cast_possible_truncation,
                    reason = "usize is 64 bit, nothing works if this is not true"
                )]
                let magic_index = (occ.0.wrapping_mul(magic_number) >> shift) as usize;
                assert!(moves[magic_index] == 0 || moves[magic_index] == mv.0);
                moves[magic_index] = mv.0;
            }

            let moves_name = format_ident!("{}_MOVES_{sq}", stringify!($name));
            let len = moves.len();

            pretty_print(&quote! {
                const #moves_name: [u64; #len] = [ #(#moves),* ];
            });

            let mask_raw = mask.0;

            $magics.push(quote! {
                Magic {
                    table: &#moves_name,
                    mask: #mask_raw,
                    magic: #magic_number,
                    shift: #shift,
                }
            });
        }
    };
}

fn pretty_print(tokens: &impl ToString) {
    let output = tokens.to_string();
    let syntax_tree = syn::parse_file(&output).unwrap();
    let pretty_output = prettyplease::unparse(&syntax_tree);

    println!("{pretty_output}");
}

fn main() {
    gen_moves!(KING_MOVES, king_moves);
    gen_moves!(KNIGHT_MOVES, knight_moves);
    gen_moves!(PAWN_ATTACKS_WHITE, pawn_attacks, Color::White);
    gen_moves!(PAWN_ATTACKS_BLACK, pawn_attacks, Color::Black);
    gen_moves!(PAWN_QUIETS_WHITE, pawn_quiets, Color::White);
    gen_moves!(PAWN_QUIETS_BLACK, pawn_quiets, Color::Black);
    pretty_print(&quote! {
        pub const PAWN_ATTACKS: [[u64; 64]; 2] = [PAWN_ATTACKS_WHITE, PAWN_ATTACKS_BLACK];
        pub const PAWN_QUIETS: [[u64; 64]; 2] = [PAWN_QUIETS_WHITE, PAWN_QUIETS_BLACK];
    });

    pretty_print(&quote! {
        #[derive(Debug)]
        pub struct Magic {
            pub table: &'static [u64],
            pub mask: u64,
            pub magic: u64,
            pub shift: u32,
        }

        impl Magic {
            pub fn get(&self, occupancy: u64) -> u64 {
                let index = ((occupancy & self.mask).wrapping_mul(self.magic) >> self.shift) as usize;
                // SAFETY: index is always in bounds, provided the magic generation is correct.
                unsafe { *self.table.get_unchecked(index) }
            }
        }
    });

    let mut bishop_magics = vec![];
    gen_slider_moves!(BISHOP, bishop_magics);
    let mut rook_magics = vec![];
    gen_slider_moves!(ROOK, rook_magics);

    pretty_print(&quote! { const BISHOP_MAGICS: [Magic; 64] = [ #(#bishop_magics),* ]; });
    pretty_print(&quote! { const ROOK_MAGICS: [Magic; 64] = [ #(#rook_magics),* ]; });
}
