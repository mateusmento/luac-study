local grades = Buffer.number(7.2, 5.6, 8.9, 10, 3.6)
local names = array.string("Jessy", "Matt", "Rick", "Steven")
-- local points = array.table({x=1, y=2}, {x=5, y=9}, {x=4, y=8})


grades[3] = 9
print(#grades, grades[4])

grades.assign(4.5, 5)   -- assigning the value 4.5 into the index 5
grades.add(6.7)         -- adding a element of value 6.7
grades.remove(4)        -- removing the 4th element
grades.insert(5.4, 2);  -- inserting a element of value 5.4 into the 2th position

grades.foreach(function(value, index)
    print(value)
end)
