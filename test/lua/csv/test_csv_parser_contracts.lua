function test_csv_parser_contracts()
    local text = string.char(239, 187, 191) .. " \t\r\n\n\r  left,right\r\n\"quoted\" \t,last\r\n\t  "
    local rows = csv.decode(text)
    assert(rows[1][1] == "  left" and rows[1][2] == "right")
    assert(rows[2][1] == "quoted" and rows[2][2] == "last")
    assert(rows[3][1] == "\t  ")
    assert(not pcall(function() return csv.decode('\"quoted\"invalid,next') end))
    assert(csv.encode({{true, false}}) == "true,false")
    assert(csv.encode({1, "text", false}) == "1\ntext\nfalse")
    return 1
end
