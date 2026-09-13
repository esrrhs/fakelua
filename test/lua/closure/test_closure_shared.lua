function make_bank(balance)
    local acc = balance
    local function deposit(amount)
        acc = acc + amount
        return acc
    end
    local function withdraw(amount)
        acc = acc - amount
        return acc
    end
    return deposit, withdraw
end

function test()
    local dep, wid = make_bank(500)
    dep(200)
    wid(150)
    dep(50)
    return wid(100)
end
