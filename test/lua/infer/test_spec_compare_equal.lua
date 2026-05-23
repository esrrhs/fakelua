-- n is specialised via arithmetic (n+1, n-1); m stays CVar since == alone cannot
-- drive numeric specialisation (== works on any Lua type, not just numbers).
function test(n, m)
    if n == m then
        return n + 1
    end
    return n - 1
end
