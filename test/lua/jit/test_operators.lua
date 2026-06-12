-- Lua 5.4 operator precedence and associativity tests

function test_precedence()
    if not (2 + 3 * 4 == 14) then return 0 end
    if not (2 * 3 + 4 == 10) then return 0 end
    if not (2^3^2 == 512) then return 0 end
    return 1
end

function test_logical()
    if not (not nil and 2 and not(2 > 3 or 3 < 2)) then return 0 end
    if not ((((1 or false) and true) or false) == true) then return 0 end
    if not ((((nil and true) or false) and true) == false) then return 0 end
    return 1
end

function test_string_concat()
    local s = "hello" .. " " .. "world"
    if s ~= "hello world" then return 0 end
    return 1
end

function test_complex_expr()
    if not (-3 + 4*5//2^3^2//9 + 4%10/3 ==
               (-3) + (((4*5)//(2^(3^2)))//9) + ((4%10)/3)) then return 0 end
    return 1
end

function test_numeric_edge()
    if not (0 == -0) then return 0 end
    if not (7 // 2 == 3) then return 0 end
    if not (-7 // 2 == -4) then return 0 end
    if not (7 // -2 == -4) then return 0 end
    if not (-7 // -2 == 3) then return 0 end
    if not (7 % 2 == 1) then return 0 end
    if not (-7 % 2 == 1) then return 0 end
    if not (7 % -2 == -1) then return 0 end
    if not (-7 % -2 == -1) then return 0 end
    return 1
end

function test_bitwise()
    if not (1 << 8 == 256) then return 0 end
    if not (256 >> 8 == 1) then return 0 end
    return 1
end
