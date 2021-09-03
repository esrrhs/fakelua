pub struct Lua {
    id: i32,
}

pub fn fakelua_newstate() -> Lua {
    let mut r = Lua { id: 1 };
    r
}
