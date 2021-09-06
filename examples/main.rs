extern crate fakelua;

use fakelua::*;

fn main() {
    let mut l = fakelua_newstate();
    let str = fakelua_dofile(&l, "test.lua").expect("Failed to dofile");
    println!("{}\n", str);
}
