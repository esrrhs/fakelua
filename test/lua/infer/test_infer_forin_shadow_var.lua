local k = 123

function test()
    for k, v in pairs({["x"] = 1}) do
        return k + 1
    end
    return 0
end
