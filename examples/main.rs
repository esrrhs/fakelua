extern crate fakelua;

use fakelua::fakelua_newstate;

fn main() {
    fakelua_newstate();
    println!("Hello, world!");
}
