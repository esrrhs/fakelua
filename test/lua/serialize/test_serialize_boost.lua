package "SerializeTest"

function test_text_roundtrip()
    local t = { id = 7, name = "ok", nested = { 1, 2 } }
    local s = serialize.text_encode(t)
    if type(s) ~= "string" or #s == 0 then return 0 end
    local d = serialize.text_decode(s)
    if d.id ~= 7 then return 0 end
    if d.name ~= "ok" then return 0 end
    if d.nested[1] ~= 1 or d.nested[2] ~= 2 then return 0 end
    if serialize.text_decode(serialize.text_encode(nil)) ~= nil then return 0 end
    if serialize.text_decode(serialize.text_encode(true)) ~= true then return 0 end
    if serialize.text_decode(serialize.text_encode(42)) ~= 42 then return 0 end
    local f = serialize.text_decode(serialize.text_encode(1.5))
    if f < 1.5 - 1e-9 or f > 1.5 + 1e-9 then return 0 end
    return 1
end

function test_xml_roundtrip()
    local t = { id = 9, name = "xml", child = { a = true } }
    local s = serialize.xml_encode(t)
    if type(s) ~= "string" or #s == 0 then return 0 end
    if not string.find(s, "<value", 1, true) then return 0 end
    local d = serialize.xml_decode(s)
    if d.id ~= 9 then return 0 end
    if d.name ~= "xml" then return 0 end
    if d.child.a ~= true then return 0 end
    return 1
end

function test_text_cycle()
    local t = { a = 1 }
    t.self = t
    serialize.text_encode(t)
end

function test_text_bad()
    serialize.text_decode("not an archive")
end
