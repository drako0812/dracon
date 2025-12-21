local cli = {}

cli.buffer = {}
cli.csrx = 0      -- Cursor X
cli.csry = 0      -- Cursor Y
cli.csre = true   -- Cursor Enabled
cli.scroll = true -- Auto Scroll buffer

function cli.init()
    cli.buffer = {}
    local idx = 1
    for y = 0, 24 do
        for x = 0, 24 do
            cli.buffer[idx] = { ch = ' ', fg = 23, bg = 0 }
            idx = idx + 1
        end
    end
    cli.csrx = 0
    cli.csry = 0
    cli.csre = true
    cli.scroll = true

    register_on_frame(cli.on_frame)
end

function cli.deinit()
    unregister_on_frame(cli.on_frame)
end

local function coord2idx(x, y)
    if x < 0 then return nil end
    if x >= 24 then return nil end
    if y < 0 then return nil end
    if y >= 24 then return nil end
    return x + (y * 24)
end

function cli.putc(x, y, ch, fg, bg)
    local idx = coord2idx(x, y)
    if idx == nil then return end
    cli.buffer[idx + 1] = { ch = ch, fg = fg, bg = bg }
end

function cli.on_frame(dt)
    -- Render CLI
end
