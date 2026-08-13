local t = {a = 1, b = 2}

function hash_ops()
    t.c = 3
    return t.a + t.b + t.c
end
