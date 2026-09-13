-- Test os error paths: bad argument types must throw
-- Covers: os.difftime / os.execute / os.exit / os.getenv / os.remove / os.rename / os.setlocale

function test_os_difftime_bad_arg()
    os.difftime(true, 1)
end

function test_os_execute_bad_arg()
    os.execute(true)
end

function test_os_exit_bad_arg()
    -- Lua 5.4 os.exit 接受 boolean/number/nil；String 才报错
    os.exit("bad")
end

function test_os_exit_nan()
    os.exit(0 / 0)
end

function test_os_getenv_bad_arg()
    os.getenv(true)
end

function test_os_remove_bad_arg()
    os.remove(true)
end

function test_os_rename_bad_arg()
    os.rename(true, "b")
end

function test_os_setlocale_bad_arg()
    os.setlocale(true)
end
