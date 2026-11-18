function test_xml_structure_contracts()
    local decoded = xml.decode("<root>alpha<![CDATA[beta]]><item>a</item><item>b</item>omega</root>")
    assert(decoded._text == "alphabetaomega")
    assert(#decoded.item == 2 and decoded.item[1]._text == "a" and decoded.item[2]._text == "b")
    assert(xml.decode(xml.encode(true))._text == "true")
    assert(xml.decode(xml.encode(1.25))._text == "1.25")
    assert(xml.decode(xml.encode("plain text"))._text == "plain text")
    decoded = xml.decode(xml.encode({alpha = "a", beta = "b"}))
    assert(decoded.alpha._text == "a" and decoded.beta._text == "b")
    decoded = xml.decode(xml.encode({root = {list = {{_text = "a"}, {_text = "b"}}}}))
    assert(#decoded.list.item == 2 and decoded.list.item[2]._text == "b")
    local cycle = {other = 1}
    cycle.nested = cycle
    local ok, err = pcall(function() return xml.encode({root = cycle}) end)
    assert(not ok and string.find(err, "cyclic", 1, true) ~= nil)
    local deep = {_text = "leaf"}
    for i = 1, 66 do deep = {child = deep} end
    ok, err = pcall(function() return xml.encode({root = deep}) end)
    assert(not ok and string.find(err, "nesting too deep", 1, true) ~= nil)
    return 1
end
