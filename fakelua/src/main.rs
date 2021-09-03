use lua::lua::fakelua_newstate;

fn main() {
    fakelua_newstate();
    println!("Hello, world!");
}
