#[test]
fn test_debug() {
    let x = true;
    let y = !x;
    println!("y before the loop is {:?}", y);
    for i in 0..10 {
        let x = i;
        let y = x + 10;
        println!("y inside the loop is {:?}", y);
    }
    println!("{:?}", x);
    assert!(y != x);
}
