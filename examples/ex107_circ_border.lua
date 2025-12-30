function MAIN()
    for i = 0, 100 do
        local x = math.random(0, 191)
        local y = math.random(0, 191)
        local r = math.random(0, 192 // 2)
        local c1 = math.random(0, 23)
        local c2 = math.random(0, 23)

        circf(x, y, r, c1)
        circ(x, y, r, c2)
    end
end
