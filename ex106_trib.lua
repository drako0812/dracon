function MAIN()
    local mode = math.random(0, 11)
    local x1 = math.random(0, 191)
    local y1 = math.random(0, 191)
    local x2 = math.random(0, 191)
    local y2 = math.random(0, 191)
    local x3 = math.random(0, 191)
    local y3 = math.random(0, 191)
    local bc = math.random(0, 8)
    local fc = math.random(9, 23) | (mode << 5)

    trif(x1, y1, x2, y2, x3, y3, fc)
    tri(x1, y1, x2, y2, x3, y3, bc)
end
