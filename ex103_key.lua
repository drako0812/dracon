function MAIN()
    cls(0)

    local keys = {}

    for i = 0, 511 do
        keys[i] = key(i)
    end

    local buffer = ""

    for i, v in ipairs(keys) do
        if(v) then
            buffer = buffer .. string.format("%d, ", i)
        end
    end

    puts(0, 0, string.format("Keys: %s", buffer), 23)

    puts(0, 191-8, "Press Keys", 23)
end
