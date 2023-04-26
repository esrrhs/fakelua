for key, value in pairs(data) do
    print(key, value)
end

for key in pairs(data) do
    data[key] = nil
end
