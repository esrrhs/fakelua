package "DbgYaml"
function test_invalid()
    local ok, msg = pcall(yaml.decode, "{{{invalid")
    print("ok=" .. tostring(ok))
    print("msg=" .. tostring(msg))
    if type(msg) == "string" then
        print("msg substr=" .. msg:sub(1, 50))
    end
    return 1
end
