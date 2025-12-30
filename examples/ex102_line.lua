function MAIN()
    for i = 0, 100 do
        local x1 = math.random(0, 191)
        local y1 = math.random(0, 191)
        local x2 = math.random(0, 191)
        local y2 = math.random(0, 191)
        local c = math.random(0, 23)

        line(x1, y1, x2, y2, c)
    end
end
