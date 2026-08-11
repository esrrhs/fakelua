package "Ping"

-- Circular cross-package call: Ping.pong -> Pong.ping -> Ping.pong -> ...
function pong(n)
    if n <= 0 then
        return 0
    end
    return 1 + Pong.ping(n - 1)
end

function name() -- Exported to Ping.name (isolation: same name exists in Pong)
    return "ping"
end
