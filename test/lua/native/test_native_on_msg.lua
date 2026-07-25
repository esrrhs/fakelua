local msg_handlers = {}

-- 1. 登录消息处理器：使用内置 new_native_obj(type, id) 创建 C++ 持久对象
msg_handlers["on_login"] = function(pid, pname)
    local player = new_native_obj("player", pid)
    player.hp = 100
    player.mp = 200
    player.name = pname
    return player.hp
end

-- 2. 对话消息处理器：使用内置 get_native_obj(type, id) 获取已有 C++ 原生对象
msg_handlers["on_talk"] = function(pid, words)
    local player = get_native_obj("player", pid)
    if player == nil then
        return "not_found"
    end

    player.hp = player.hp - 20
    player.last_talk = words

    return player.name .. " (" .. player.hp .. "hp): " .. player.last_talk
end

-- C++ 调用的统一事件入口：on_msg(msg_name, ...)
function on_msg(msg_name, ...)
    local handler = msg_handlers[msg_name]
    if handler == nil then
        return "unknown_msg"
    end
    return handler(...)
end
