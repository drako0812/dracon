function MAIN()
    cls(0)

    puts(0, 0, "Seconds elapsed:", 23)
    puts(0, 8, string.format("%.4f", time() / 1000), 23)
    puts(0, 16, "Time since epoch:", 23)
    puts(0, 24, string.format("%d", tstamp()), 23)

    if (time() % 500 > 250) then
        puts(0, 32, 'Warning!', 12)
    end

    if(time()>2000) then
        puts(0, 64, 'Fugit inreparabile tempus', 5)
    end
end
