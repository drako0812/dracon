function MAIN()
    cls(0)
    if (btn(0)) then
        puts(0, 0, "A", 23)
    end
    if (btn(1)) then
        puts(0, 8, "B", 23)
    end
    if (btn(2)) then
        puts(0, 8*2, "X", 23)
    end
    if (btn(3)) then
        puts(0, 8*3, "Y", 23)
    end
    if (btn(4)) then
        puts(0, 8*4, "^", 23)
    end
    if (btn(5)) then
        puts(0, 8*5, "v", 23)
    end
    if (btn(6)) then
        puts(0, 8*6, "<", 23)
    end
    if (btn(7)) then
        puts(0, 8*7, ">", 23)
    end
    if (btn(8)) then
        puts(0, 8 * 8, "START", 23)
    end

    puts(0, 191-8, "Press Buttons", 23)
end
