function MAIN()
    for i = 0, 100 do
        local x = math.random(0, 191)
        local y = math.random(0, 191)
        local w = math.random(0, 191)
        local h = math.random(0, 191)
        local c = math.random(0, 23)

        rect(x, y, w, h, c)
    end
end
