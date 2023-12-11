if (self.methods.init) then
    if (tostring(arg[1]) ~= "___CREATE_ONLY___") then
        obj:init(unpack(arg))
    else
        obj.___CREATE_ONLY___ = true
    end
elseif (tostring(arg[1]) ~= "___CREATE_ONLY___") then
    error("No init method found for class " .. self.name)
end
