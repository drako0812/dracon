const std = @import("std");
const constants = @import("./constants.zig");
const Console = @import("../console.zig").Console;

pub const FrameBuffer = struct {
    buffer: [constants.FRAMEBUFFER_PIXELS]u5,

    pub fn init() FrameBuffer {
        return FrameBuffer{ .buffer = std.mem.zeroes([constants.FRAMEBUFFER_PIXELS]u5) };
    }

    /// Sets a pixel in the `FrameBuffer` to the `color`.
    /// - `x`: X-Coordinate
    /// - `y`: Y-Coordinate
    /// - `color`: 5-bit color palette value.
    ///
    /// **NOTE**: Does not do bounds checking.
    pub fn setPixelFast(self: *FrameBuffer, console: *Console, x: i32, y: i32, color: u5) void {
        const pix_idx = (y * constants.FRAMEBUFFER_PIX_WIDTH) + x;
        self.buffer[@intCast(pix_idx)] = color;
        console.frmbuf_dirty = true;
    }

    /// Sets a pixel in the `FrameBuffer` to the `color`.
    /// - `x`: X-Coordinate
    /// - `y`: Y-Coordinate
    /// - `color`: 5-bit color palette value.
    pub fn setPixel(self: *FrameBuffer, console: *Console, x: i32, y: i32, color: u5) void {
        if ((x < 0) or (x >= constants.FRAMEBUFFER_PIX_WIDTH) or (y < 0) or (y >= constants.FRAMEBUFFER_PIX_HEIGHT)) {
            return;
        }

        self.setPixelFast(console, x, y, color);
    }

    pub fn getPixelFast(self: *FrameBuffer, x: i32, y: i32) u5 {
        const pix_idx = (y * constants.FRAMEBUFFER_PIX_WIDTH) + x;
        return self.buffer[@as(usize, @intCast(pix_idx))];
    }

    pub fn getPixel(self: *FrameBuffer, x: i32, y: i32) ?u5 {
        if ((x < 0) or (x >= constants.FRAMEBUFFER_PIX_WIDTH) or (y < 0) or (y >= constants.FRAMEBUFFER_PIX_HEIGHT)) {
            return null;
        }

        return self.getPixelFast(x, y);
    }

    pub fn cls(self: *FrameBuffer, console: *Console, color: u5) void {
        @memset(&self.buffer, color);
        console.frmbuf_dirty = true;
    }

    pub fn hLine(self: *FrameBuffer, x: i32, y: i32, l: i32, color: u5) void {
        var xx = x;
        while (xx < x + l) {
            self.setPixel(xx, y, color);
            xx += 1;
        }
    }

    pub fn vLine(self: *FrameBuffer, x: i32, y: i32, l: i32, color: u5) void {
        var yy = y;
        while (yy < y + l) {
            self.setPixel(x, yy, color);
            yy += 1;
        }
    }

    pub fn line(self: *FrameBuffer, x1: i32, y1: i32, x2: i32, y2: i32, color: u5) void {
        var x1c = x1;
        var y1c = y1;
        const dx: i32 = @intCast(@abs(x2 - x1c));
        const sx: i32 = if (x1c < x2) 1 else -1;
        const dy: i32 = -@as(i32, @intCast(@abs(y2 - y1c)));
        const sy: i32 = if (y1c < y2) 1 else -1;
        var e: i32 = dx + dy;

        if ((dx == 0) and (dy == 0)) {
            self.setPixel(x1c, y1c, color);
            return;
        }

        if ((dx == 0) and (dy != 0)) {
            self.vLine(sx, sy, dy, color);
            return;
        }

        if ((dy == 0) and (dx != 0)) {
            self.hLine(sx, sy, dx, color);
        }

        while (true) {
            self.setPixel(x1c, y1c, color);
            const e2 = 2 * e;
            if (e2 >= dy) {
                if (x1c == x2) {
                    break;
                }
                e += dy;
                x1c += sx;
            }
            if (e2 <= dx) {
                if (y1c == y2) {
                    break;
                }
                e += dx;
                y1c += sy;
            }
        }
    }

    pub fn rect(self: *FrameBuffer, x: i32, y: i32, w: i32, h: i32, color: u5) void {
        self.line(x, y, x + w, y, color);
        self.line(x, y + h, x + w, y + h, color);
        self.line(x, y, x, y + h, color);
        self.line(x + w, y, x + w, y + h, color);
    }

    pub fn rectF(self: *FrameBuffer, x: i32, y: i32, w: i32, h: i32, color: u5) void {
        var xx = x;
        while (xx < x + w) {
            self.line(xx, y, xx, y + h, color);
            xx += 1;
        }
    }

    pub fn circ(self: *FrameBuffer, x: i32, y: i32, r: i32, color: u5) void {
        var f: i32 = 1 - r;
        var ddF_x: i32 = 0;
        var ddF_y: i32 = -2 * r;
        var xx: i32 = 0;
        var yy: i32 = r;

        self.setPixel(x, y + r, color);
        self.setPixel(x, y - r, color);
        self.setPixel(x + r, y, color);
        self.setPixel(x - r, y, color);

        while (xx < yy) {
            if (f >= 0) {
                yy -= 1;
                ddF_y += 2;
                f += ddF_y;
            }
            xx += 1;
            ddF_x += 2;
            f += ddF_x + 1;
            self.setPixel(x + xx, y + yy, color);
            self.setPixel(x - xx, y + yy, color);
            self.setPixel(x + xx, y - yy, color);
            self.setPixel(x - xx, y - yy, color);
            self.setPixel(x + yy, y + xx, color);
            self.setPixel(x - yy, y + xx, color);
            self.setPixel(x + yy, y - xx, color);
            self.setPixel(x - yy, y - xx, color);
        }
    }

    pub fn circF(self: *FrameBuffer, x: i32, y: i32, r: i32, color: u5) void {
        var f: i32 = 1 - r;
        var ddF_x: i32 = 0;
        var ddF_y: i32 = -2 * r;
        var xx: i32 = 0;
        var yy: i32 = r;

        self.setPixel(x, y + r, color);
        self.setPixel(x, y - r, color);
        self.setPixel(x + r, y, color);
        self.setPixel(x - r, y, color);

        while (xx < yy) {
            if (f >= 0) {
                yy -= 1;
                ddF_y += 2;
                f += ddF_y;
            }
            xx += 1;
            ddF_x += 2;
            f += ddF_x + 1;
            self.line(x - xx, y + yy, x + xx, y + yy, color);
            self.line(x - xx, y - yy, x + xx, y - yy, color);
            self.line(x + yy, y + yy, x - yy, y + xx, color);
            self.line(x + yy, y - xx, x - yy, y - xx, color);
        }
    }

    pub fn putch(self: *FrameBuffer, con: *Console, x: i32, y: i32, ch: u8, color: u5) void {
        // Get Glyph
        const glyph = con.gfx.font_data.glyphs[ch];

        // Draw glyph
        for (0..8) |yy| {
            for (0..8) |xx| {
                const idx = (yy * 8) + xx;
                if (glyph.data[idx] == 1) {
                    self.setPixel(con, x + @as(i32, @intCast(xx)), y + @as(i32, @intCast(yy)), color);
                }
            }
        }
    }

    pub fn puts(self: *FrameBuffer, con: *Console, x: i32, y: i32, str: []const u8, color: u5) void {
        for (str, 0..) |ch, idx| {
            self.putch(con, x + @as(i32, @intCast(idx * constants.SPRITE_PIX_WIDTH)), y, ch, color);
        }
    }

    pub fn puts_rainbow(self: *FrameBuffer, con: *Console, x: i32, y: i32, str: []const u8, color: u5) void {
        var col = color;
        for (str, 0..) |ch, idx| {
            self.putch(con, x + @as(i32, @intCast(idx * constants.SPRITE_PIX_WIDTH)), y, ch, col);
            col +%= 1;
        }
    }
};
