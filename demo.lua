firstname = "Mateus"
secondname = "Sarmento"

me = {
    name = "Mateus Sarmento",
    age = 23
}

function printValues()
    print('num = ' .. num)

    if Math and Math.PI and Math.GOLDEN_RATIO then
        print(Math.PI, Math.GOLDEN_RATIO)
    end
end

function write(x)
    print(x or 'undefined')
    return x or 'undefined'
end

function callc(a, b)
    print(add(a, b))
end

function useVector()
    local vec1 = Vector(12, 14)
    local vec2 = Vector(8, 11)

    -- print(vec1.x, vec1.y)
    -- print(vec2.x, vec2.y)
    -- print(vec1:distance(vec2))
end

function useArray()
    local arr = array(43, 12, 553)
    arr[1] = 123
    print(#arr)

    -- arr = Array.new(4)

    for i = 1, #arr do
        print(arr[i])
    end
end
