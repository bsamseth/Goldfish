#![allow(clippy::missing_errors_doc, clippy::missing_panics_doc, dead_code)]
use darling::ast::Fields;
use darling::{ast, util, FromDeriveInput, FromField, FromMeta};
use proc_macro::TokenStream;
use quote::{quote, ToTokens};
use syn::DeriveInput;
use syn::{Ident, Type};

#[derive(Debug, FromField)]
#[darling(attributes(uci))]
struct UciOptionAttrubute {
    ident: Option<Ident>,
    ty: Type,
    name: Option<syn::LitStr>,
    kind: UciOptionType,
    default: Option<syn::LitStr>,
    min: Option<syn::LitStr>,
    max: Option<syn::LitStr>,
}

#[derive(Debug, Clone, Copy, FromMeta)]
enum UciOptionType {
    String,
    Spin,
}

#[derive(Debug, FromDeriveInput)]
#[darling(attributes(uci), supports(struct_named))]
struct UciOptionsInput {
    ident: Ident,
    data: ast::Data<util::Ignored, UciOptionAttrubute>,
}

#[proc_macro_derive(UciOptions, attributes(uci))]
pub fn derive_uci_options(input: TokenStream) -> TokenStream {
    let input = syn::parse_macro_input!(input);
    let opts =
        UciOptionsInput::from_derive_input(&input).expect("failed to parse uci option attributes");
    let DeriveInput { ident, .. } = input;

    let ast::Data::Struct(fields) = opts.data else {
        panic!("UciOptions can only be derived for structs.")
    };

    let print_options = impl_print_options(&fields);
    let set_option = impl_set_option(&fields);
    let default_impl = impl_default(&fields);

    quote! {
        #[automatically_derived]
        impl uci::UciOptions for #ident {
            #print_options
            #set_option
        }
        #[automatically_derived]
        impl Default for #ident {
            #default_impl
        }
    }
    .into()
}

fn impl_default(fields: &Fields<UciOptionAttrubute>) -> proc_macro2::TokenStream {
    let field_defaults = fields.iter().map(|field| {
        let UciOptionAttrubute {
            ident, ty, default, ..
        } = field;
        let default = default.clone().map(|default| default.value());

        if type_is_option(ty) {
            assert!(
                default.is_none(),
                "option fields must not have a default value"
            );
            quote! {
                #ident: None
            }
        } else {
            assert!(
                default.is_some(),
                "non-option fields must have a default value"
            );
            quote! {
                #ident: #default.parse()
                    .expect(
                        concat!(
                            "default value for UCI option ",
                            stringify!(#ident),
                            " should be valid"
                        )
                    )
            }
        }
    });

    quote! {
        fn default() -> Self {
            Self {
                #(#field_defaults),*
            }
        }
    }
}

fn impl_print_options(fields: &Fields<UciOptionAttrubute>) -> proc_macro2::TokenStream {
    let fmts = fields.iter().map(|field| {
        let UciOptionAttrubute {
            ident,
            name,
            kind,
            default,
            min,
            max,
            ..
        } = field;
        let name = name.clone().map_or_else(
            || {
                ident
                    .clone()
                    .expect("only structs with named fields reach this")
                    .to_string()
            },
            |name| name.value(),
        );
        let kind = match kind {
            UciOptionType::String => "string",
            UciOptionType::Spin => "spin",
        };
        let min = min.clone().map(|min| min.value());
        let max = max.clone().map(|max| max.value());
        let default = default.clone().map(|default| default.value());

        let mut fmt = format!("option name {name} type {kind}");
        if let Some(min) = min {
            fmt = format!("{fmt} min {min}");
        }
        if let Some(max) = max {
            fmt = format!("{fmt} max {max}");
        }
        if let Some(default) = default {
            fmt = format!("{fmt} default {default}");
        }

        fmt
    });

    quote! {
        fn print_options(&self) {
            #(println!(#fmts);)*
        }
    }
}

fn impl_set_option(fields: &Fields<UciOptionAttrubute>) -> proc_macro2::TokenStream {
    let field_setter = fields.iter().map(|field| {
        let UciOptionAttrubute {
            ident, name, ty, ..
        } = field;
        let name = name
            .clone()
            .map_or_else(
                || {
                    ident
                        .clone()
                        .expect("only structs with named fields reach this")
                        .to_string()
                },
                |name| name.value(),
            )
            .to_lowercase();

        let parse = quote! { value.parse().context(concat!("parsing UCI option ", #name))? };

        let assign = if type_is_option(ty) {
            quote! { Some(#parse) }
        } else {
            parse
        };
        quote! {
            #name => {
                use ::anyhow::Context;
                self.#ident = #assign;
            }
        }
    });

    quote! {
        fn set_option(&mut self, name: &str, value: &str) -> ::anyhow::Result<()> {
            match name.to_lowercase().as_str() {
                #(#field_setter)*
                _ => return Err(anyhow::anyhow!("unknown UCI option: {name}")),
            }
            Ok(())
        }
    }
}

fn type_is_option(ty: &Type) -> bool {
    ty.into_token_stream().to_string().starts_with("Option <")
}
