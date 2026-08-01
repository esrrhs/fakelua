-- 使用 local function 定义处理器，避免修改全局表
local function on_login(pid, pname)
    local player = new_native_obj(pid, "player", pid)
    player.hp = 100
    player.mp = 200
    player.name = pname
    return player.hp
end

local function on_talk(pid, words)
    local player = get_native_obj("player", pid)
    if player == nil then
        return "not_found"
    end

    player.hp = player.hp - 20
    player.last_talk = words

    return player.name .. " (" .. player.hp .. "hp): " .. player.last_talk
end

-- 处理器映射表（只读，通过 local function 引用）
local msg_handlers = {
    on_login = on_login,
    on_talk = on_talk,
}

-- C++ 调用的统一事件入口：on_msg(msg_name, ...)
function on_msg(msg_name, ...)
    local handler = msg_handlers[msg_name]
    if handler == nil then
        return "unknown_msg"
    end
    return handler(...)
end
