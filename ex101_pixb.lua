function MAIN()
    for i = 0, 100 do
        local x = math.random(0, 191)
        local y = math.random(0, 191)
        local c = math.random(0, 23)
        local blend = math.random(0, 11)
        c = (blend << 4) | c

        pix(x, y, c)
    end
end
