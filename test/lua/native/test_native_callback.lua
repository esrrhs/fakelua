function run_test()
    local user = { name = "Bob", score = 50 }
    return cpp_process_user(user)
end

function calc()
    return cpp_add_hp(120, 30)
end
