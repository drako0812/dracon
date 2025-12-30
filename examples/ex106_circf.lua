function MAIN()
    for i = 0, 100 do
        local x = math.random(0, 191)
        local y = math.random(0, 191)
        local r = math.random(0, 192 // 2)
        local c = math.random(0, 23)

        circf(x, y, r, c)
    end
end
