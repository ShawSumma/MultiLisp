function sub(a)
    return function (b) return a - b end
end

function fib(n)
    if n < 2 then
        return n
    else
        return fib(sub(n)(2)) + fib(sub(n)(1))
    end
end

print(fib(32))