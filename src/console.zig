const std = @import("std");

const rl = @import("raylib");

pub const FRAMEBUFFER_PIX_WIDTH = 192;
pub const FRAMEBUFFER_PIX_HEIGHT = 192;
pub const FRAMEBUFFER_BPP = 5;
pub const FRAMEBUFFER_PIXELS = FRAMEBUFFER_PIX_WIDTH * FRAMEBUFFER_PIX_HEIGHT;
pub const FRAMEBUFFER_BITS = FRAMEBUFFER_PIXELS * FRAMEBUFFER_BPP;
pub const FRAMEBUFFER_SIZE = FRAMEBUFFER_BITS / 8;
pub const PALETTE_SIZE = 24;
pub const FONT_GLYPH_COUNT = 512;
pub const SPRITE_PIX_WIDTH = 8;
pub const SPRITE_PIX_HEIGHT = 8;
pub const SPRITE_PIXELS = SPRITE_PIX_WIDTH * SPRITE_PIX_HEIGHT;
pub const SPRITE_BITS = SPRITE_PIXELS * FRAMEBUFFER_BPP;
pub const SPRITE_SIZE = SPRITE_BITS / 8;
pub const SPRITE_MEMORY_SIZE = 256 * 8;

pub const FrameBuffer = struct {
    buffer: [FRAMEBUFFER_PIXELS]u5,

    pub fn init() FrameBuffer {
        return FrameBuffer{ .buffer = std.mem.zeroes([FRAMEBUFFER_PIXELS]u5) };
    }

    /// Sets a pixel in the `FrameBuffer` to the `color`.
    /// - `x`: X-Coordinate
    /// - `y`: Y-Coordinate
    /// - `color`: 5-bit color palette value.
    ///
    /// **NOTE**: Does not do bounds checking.
    pub fn setPixelFast(self: *FrameBuffer, console: *Console, x: i32, y: i32, color: u5) void {
        const pix_idx = (y * FRAMEBUFFER_PIX_WIDTH) + x;
        self.buffer[@intCast(pix_idx)] = color;
        console.frmbuf_dirty = true;
    }

    /// Sets a pixel in the `FrameBuffer` to the `color`.
    /// - `x`: X-Coordinate
    /// - `y`: Y-Coordinate
    /// - `color`: 5-bit color palette value.
    pub fn setPixel(self: *FrameBuffer, console: *Console, x: i32, y: i32, color: u5) void {
        if ((x < 0) or (x >= FRAMEBUFFER_PIX_WIDTH) or (y < 0) or (y >= FRAMEBUFFER_PIX_HEIGHT)) {
            return;
        }

        self.setPixelFast(console, x, y, color);
    }

    pub fn getPixelFast(self: *FrameBuffer, x: i32, y: i32) u5 {
        const pix_idx = (y * FRAMEBUFFER_PIX_WIDTH) + x;
        return self.buffer[pix_idx];
    }

    pub fn getPixel(self: *FrameBuffer, x: i32, y: i32) ?u5 {
        if ((x < 0) or (x >= FRAMEBUFFER_PIX_WIDTH) or (y < 0) or (y >= FRAMEBUFFER_PIX_HEIGHT)) {
            return null;
        }

        return self.getPixelFast(x, y);
    }

    pub fn cls(self: *FrameBuffer, console: *Console, color: u5) void {
        @memset(self.buffer, color);
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
};

pub const PaletteEntry = struct {
    R: u8,
    G: u8,
    B: u8,
};

inline fn PAL(value: u24) PaletteEntry {
    return PaletteEntry{
        .R = (value >> 16) & 0xFF,
        .G = (value >> 8) & 0xFF,
        .B = value & 0xFF,
    };
}

pub const PaletteData = struct {
    pal: [PALETTE_SIZE]PaletteEntry,

    pub fn init() PaletteData {
        return PaletteData{ .pal = std.mem.zeroes([PALETTE_SIZE]PaletteEntry) };
    }

    pub fn setEntry(self: *PaletteData, console: *Console, index: u8, red: u8, green: u8, blue: u8) void {
        self.pal[index] = .{ .R = red, .G = green, .B = blue };
        console.pal_dirty = true;
    }

    pub fn getEntry(self: *PaletteData, index: u8) PaletteEntry {
        return self.pal[index];
    }

    fn loadDefault(self: *PaletteData, console: *Console) void {
        self.pal = [PALETTE_SIZE]PaletteEntry{
            PAL(0x000000), PAL(0x000080), PAL(0x008000), PAL(0x008080), PAL(0x800000), PAL(0x800080), PAL(0x808000), PAL(0x909090),
            PAL(0x303030), PAL(0x0000FF), PAL(0x00FF00), PAL(0x00FFFF), PAL(0xFF0000), PAL(0xFF00FF), PAL(0xFFFF00), PAL(0xC0C0C0),
            PAL(0x606060), PAL(0x6000C0), PAL(0x00C060), PAL(0x0060C0), PAL(0xC06000), PAL(0xC00060), PAL(0x60C000), PAL(0xFFFFFF),
        };
        console.pal_dirty = true;
    }
};

pub const FontGlyph = struct {
    data: [SPRITE_PIXELS]u1,

    pub fn init() FontGlyph {
        return FontGlyph{ .data = std.mem.zeroes([SPRITE_PIXELS]u1) };
    }
};

pub const FontData = struct {
    glyphs: [FONT_GLYPH_COUNT]FontGlyph,

    pub fn init() FontData {
        return FontData{ .glyphs = .{FontGlyph.init()} ** FONT_GLYPH_COUNT };
    }
};

pub const Sprite = struct {
    pixels: [SPRITE_PIXELS]u5,

    pub fn init() Sprite {
        return Sprite{ .pixels = std.mem.zeroes([SPRITE_PIXELS]u5) };
    }
};

pub const SpriteData = struct {
    sprites: [SPRITE_MEMORY_SIZE]Sprite,

    pub fn init() SpriteData {
        return SpriteData{ .sprites = .{Sprite.init()} ** SPRITE_MEMORY_SIZE };
    }
};

pub const Gfx = struct {
    framebuf: FrameBuffer,
    pal_data: PaletteData,
    font_data: FontData,
    spr_data: SpriteData,

    pub fn init() Gfx {
        return Gfx{ .framebuf = FrameBuffer.init(), .pal_data = PaletteData.init(), .font_data = FontData.init(), .spr_data = SpriteData.init() };
    }
};

const frag_shader_src =
    \\#version 330
    \\
    \\const int PALETTE_SIZE = 24;
    \\
    \\in vec2 fragTexCoord;
    \\in vec4 fragColor;
    \\
    \\uniform sampler2D texture0;
    \\uniform sampler2D palette;
    \\
    \\out vec4 finalColor;
    \\
    \\void main() {
    \\    vec4 texelColor = texture(texture0, fragTexCoord);
    \\
    \\    finalColor = texture(palette, vec2((texelColor.r * 24.0), 0));
    \\    //finalColor = texture(palette, fragTexCoord.xy);
    \\}
;

pub const Console = struct {
    gfx: Gfx,
    running: bool,
    frmbuf_dirty: bool,
    pal_dirty: bool,

    frmbuf_copy_img: rl.Image,
    pal_copy_img: rl.Image,
    frmbuf_copy: rl.Texture,
    pal_copy: rl.Texture,
    rtex: rl.RenderTexture,
    pal_shdr: rl.Shader,
    pal_shdr_palettedTexture_loc: i32,
    pal_shdr_palette_loc: i32,
    exit_value: i32,
    // lua: lua,
    rand: std.Random,

    pub fn init() Console {
        const tmp_frmbuf = rl.genImageColor(FRAMEBUFFER_PIX_WIDTH, FRAMEBUFFER_PIX_HEIGHT, .black);
        const tmp_pal = rl.genImageColor(PALETTE_SIZE, 1, .black);
        const tmp_shdr = rl.loadShaderFromMemory(null, frag_shader_src);
        var seed: u64 = undefined;
        try std.crypto.random.bytes(std.mem.asBytes(&seed));
        var prng = std.DefaultPrng.init(seed);
        return Console{
            .gfx = Gfx.init(),
            .running = false,
            .frmbuf_dirty = true,
            .pal_dirty = true,
            .frmbuf_copy_img = tmp_frmbuf,
            .pal_copy_img = tmp_pal,
            .frmbuf_copy = rl.loadTextureFromImage(tmp_frmbuf),
            .pal_copy = rl.loadTextureFromImage(tmp_pal),
            .rtex = rl.loadRenderTexture(FRAMEBUFFER_PIX_WIDTH, FRAMEBUFFER_PIX_HEIGHT),
            .pal_shdr = tmp_shdr,
            .pal_shdr_palettedTexture_loc = 0,
            .pal_shdr_palette_loc = rl.getShaderLocation(tmp_shdr, "palette"),
            .exit_value = 0,
            .rand = prng.random(),
        };
    }

    pub fn deinit(self: *Console) void {
        rl.unloadShader(self.pal_shdr);
        rl.unloadRenderTexture(self.rtex);
        rl.unloadTexture(self.pal_copy);
        rl.unloadTexture(self.frmbuf_copy);
        rl.unloadImage(self.pal_copy_img);
        rl.unloadImage(self.frmbuf_copy_img);
    }

    pub fn Run(self: *Console) void {
        // Load the default palette.
        self.gfx.pal_data.loadDefault(self);

        // Load test framebuffer data.
        // TODO: Remove this.
        var pidx: i32 = 0;
        for (0..FRAMEBUFFER_PIX_HEIGHT) |y| {
            for (0..FRAMEBUFFER_PIX_WIDTH) |x| {
                self.gfx.framebuf.setPixelFast(self, x, y, pidx);
                pidx = (pidx + 1) % 24;
            }
        }

        self.running = true;
        self.exit_value = 0;
        self.pal_dirty = true;
        self.frmbuf_dirty = true;

        while ((!rl.windowShouldClose()) and (self.isRunning())) {
            self.render();
        }
    }

    pub fn render(self: *Console) void {
        if (self.pal_dirty) {
            self.rebuildPal();
        }

        if (self.frmbuf_dirty) {
            self.rebuildFramebuffer();
        }

        rl.beginTextureMode(self.rtex);
        rl.beginShaderMode(self.pal_shdr);
        rl.setShaderValueTexture(self.pal_shdr, self.pal_shdr_palette_loc, self.pal_copy);
        rl.drawTextureRec(self.frmbuf_copy, rl.Rectangle{ .x = 0, .y = 0, .width = FRAMEBUFFER_PIX_WIDTH, .height = -FRAMEBUFFER_PIX_HEIGHT }, rl.Vector2{ .x = 0, .y = 0 }, .white);

        rl.endShaderMode();
        rl.endTextureMode();

        rl.beginDrawing();
        rl.clearBackground(.black);
        rl.drawTexturePro(self.rtex.texture, rl.Rectangle{ .x = 0, .y = 0, .width = FRAMEBUFFER_PIX_WIDTH, .height = FRAMEBUFFER_PIX_HEIGHT }, rl.Rectangle{ .x = 0, .y = 0, .width = rl.getRenderWidth(), .height = rl.getRenderHeight() }, rl.Vector2{ .x = 0, .y = 0 }, 0, .white);
        rl.endDrawing();

        // Randomize Framebuffer
        // TODO: Remove this.
        for (0..FRAMEBUFFER_PIX_HEIGHT) |y| {
            for (0..FRAMEBUFFER_PIX_WIDTH) |x| {
                const pidx = self.rand.uintAtMost(u5, PALETTE_SIZE - 1);
                self.gfx.framebuf.setPixelFast(self, x, y, pidx);
            }
        }
    }

    pub fn isRunning(self: *Console) bool {
        return self.running;
    }
};
