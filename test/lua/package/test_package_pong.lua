package "Pong"

-- Circular cross-package call: Pong.ping -> Ping.pong -> Pong.ping -> ...
function ping(n)
    if n <= 0 then
        return 0
    end
    return 1 + Ping.pong(n - 1)
end

function name() -- Exported to Pong.name (isolation: same name exists in Ping)
    return "pong"
end
