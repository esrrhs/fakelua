use std::fs;
use std::error::Error;

pub struct Lua {
    id: i32,
}

pub fn fakelua_newstate() -> Lua {
    let mut r = Lua { id: 1 };
    r
}

pub fn fakelua_dofile(l: &Lua, file: &str) -> Result<String, Box<dyn Error>> {
    let f = fs::read_to_string(file)?;
    Ok(f)
}
