-- Package boundary edge cases
-- Covers: assign/call form equivalence, circular cross-package calls,
--         name isolation, const folding, multi-return, vararg, package->global,
--         non-exported local function called internally.

-- Global helper (not in any package): used to test the package -> global direction.
function helper(n)
    return n * 3
end

function test_package_edge()
    -- Call-form package basic export
    if Pkg.add(2, 3) ~= 5 then return 1 end

    -- Assign-form package is equivalent to call-form
    if AssignPkg.scaled(7) ~= 14 then return 2 end

    -- Circular cross-package: Ping.pong <-> Pong.ping
    if Ping.pong(4) ~= 4 then return 3 end
    if Pong.ping(3) ~= 3 then return 4 end

    -- Same function name in different packages: must stay isolated
    if Ping.name() ~= "ping" then return 5 end
    if Pong.name() ~= "pong" then return 6 end

    -- Multiple return values propagate through a package call
    local a, b, c = Pkg.triple()
    if a ~= 1 or b ~= 2 or c ~= 3 then return 7 end

    -- Varargs propagate through a package call (including zero args)
    if Pkg.sum(1, 2, 3, 4, 5) ~= 15 then return 8 end
    if Pkg.sum() ~= 0 then return 9 end

    -- Chunk-local constant folded inside a package function (SECRET=7, 7*6=42)
    if Pkg.with_const(6) ~= 42 then return 10 end

    -- Package function calling a global function (helper(4) = 12)
    if Pkg.call_global(4) ~= 12 then return 11 end

    -- Local (non-exported) function called internally (5 + 100 = 105)
    if Pkg.use_hidden(5) ~= 105 then return 12 end

    return 5000
end
