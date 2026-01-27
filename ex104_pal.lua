function MAIN()
    cls(0)
    local x = 0
    local y = 0
    for c = 0, 24 do
        puts(x, y, string.format("%d", c), c)
        x = x + 24
        if x > (192 - 24) then
            x = 0
            y = y + 8
        end
    end
end
