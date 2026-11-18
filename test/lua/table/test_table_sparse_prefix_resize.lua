function test_sparse_prefix_resize()
    for _, bucketed in ipairs({false, true}) do
        local values = {}
        if bucketed then
            for i = 1, 8 do values["field" .. tostring(i)] = i * 10 end
        end
        for i = 2, 9 do values[i] = i * 3 end
        values[100000] = "far"
        assert(values[1] == nil and values[100000] == "far")
        for i = 2, 9 do assert(values[i] == i * 3) end
        if bucketed then
            for i = 1, 8 do assert(values["field" .. tostring(i)] == i * 10) end
        end
        values[1] = 3
        assert(#values == 9)
        values[10] = true
        values[11] = false
        values[12] = 2.5
        for i = 13, 14 do values[i] = 3.5 end
        assert(values[10] == true and values[11] == false and values[12] == 2.5 and values[13] == 3.5 and values[14] == 3.5)
    end
    return 1
end
