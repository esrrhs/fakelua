package "MultilineErrorLocation"
local label = [[first
second
third]]

function fail()
    local value = nil
    return value.field
end

function text()
    return label
end
