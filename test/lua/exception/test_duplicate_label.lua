-- 同一 block 内重复 label
function test_duplicate_label()
    ::my_label::
    ::my_label::
    return 1
end
