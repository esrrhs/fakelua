package "LogIso"

function set_level(n)
    log.set_level(n)
    return 1
end

function set_file(path)
    log.set_file(path)
    return 1
end

function debug_msg(msg)
    log.debug(msg)
    return 1
end

function error_msg(msg)
    log.error(msg)
    return 1
end

function info_msg(msg)
    log.info(msg)
    return 1
end
