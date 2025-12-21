my_on_frame_count = 0
MAIN_count = 0

function my_on_frame()
    puts(0, 0, string.format("my_on_frame: %d", my_on_frame_count), 23)
    my_on_frame_count = my_on_frame_count + 1
end

register_on_frame(my_on_frame)

function MAIN()
    cls(0)
    puts(0, 8, string.format("MAIN: %d", MAIN_count), 23)
    MAIN_count = MAIN_count + 1
end
