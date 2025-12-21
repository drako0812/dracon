const std = @import("std");

const rl = @import("raylib");
const zlua = @import("zlua");

const cart = @import("cart.zig");
const config = @import("config.zig");

pub const FRAMEBUFFER_PIX_WIDTH = 192;
pub const FRAMEBUFFER_PIX_HEIGHT = 192;
pub const FRAMEBUFFER_BPP = 5;
pub const FRAMEBUFFER_PIXELS = FRAMEBUFFER_PIX_WIDTH * FRAMEBUFFER_PIX_HEIGHT;
pub const FRAMEBUFFER_BITS = FRAMEBUFFER_PIXELS * FRAMEBUFFER_BPP;
pub const FRAMEBUFFER_SIZE = FRAMEBUFFER_BITS / 8;
pub const PALETTE_SIZE = 24;
pub const FONT_GLYPH_COUNT = 256;
pub const SPRITE_PIX_WIDTH = 8;
pub const SPRITE_PIX_HEIGHT = 8;
pub const SPRITE_PIXELS = 64;
pub const SPRITE_BITS = 320;
pub const SPRITE_SIZE = 40;
pub const SPRITE_MEMORY_SIZE = 2048;

const DefaultFontData = @embedFile("font.dat");

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
        return self.buffer[@as(usize, @intCast(pix_idx))];
    }

    pub fn getPixel(self: *FrameBuffer, x: i32, y: i32) ?u5 {
        if ((x < 0) or (x >= FRAMEBUFFER_PIX_WIDTH) or (y < 0) or (y >= FRAMEBUFFER_PIX_HEIGHT)) {
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
            self.putch(con, x + @as(i32, @intCast(idx * SPRITE_PIX_WIDTH)), y, ch, color);
        }
    }

    pub fn puts_rainbow(self: *FrameBuffer, con: *Console, x: i32, y: i32, str: []const u8, color: u5) void {
        var col = color;
        for (str, 0..) |ch, idx| {
            self.putch(con, x + @as(i32, @intCast(idx * SPRITE_PIX_WIDTH)), y, ch, col);
            col +%= 1;
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

    fn loadDefaultPriv(self: *PaletteData, console: *Console) void {
        self.loadDefault();
        console.pal_dirty = true;
    }

    pub fn loadDefault(self: *PaletteData) void {
        self.pal = [PALETTE_SIZE]PaletteEntry{
            PAL(0x000000), PAL(0x000080), PAL(0x008000), PAL(0x008080), PAL(0x800000), PAL(0x800080), PAL(0x808000), PAL(0x909090),
            PAL(0x303030), PAL(0x0000FF), PAL(0x00FF00), PAL(0x00FFFF), PAL(0xFF0000), PAL(0xFF00FF), PAL(0xFFFF00), PAL(0xC0C0C0),
            PAL(0x606060), PAL(0x6000C0), PAL(0x00C060), PAL(0x0060C0), PAL(0xC06000), PAL(0xC00060), PAL(0x60C000), PAL(0xFFFFFF),
        };
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

    pub fn init(gpa: std.mem.Allocator) !FontData {
        var ret = FontData{ .glyphs = .{FontGlyph.init()} ** FONT_GLYPH_COUNT };

        //const fdat = std.mem.bytesAsSlice(u1, DefaultFontData);
        //const fdat: *const [2048 * 8]u1 = @ptrCast(DefaultFontData);
        var fdat = try std.array_list.Aligned(u1, null).initCapacity(gpa, 2048 * 8);
        defer fdat.deinit(gpa);

        // Copy data to fdat
        for (0..DefaultFontData.len) |sidx| {
            var val = DefaultFontData[sidx];
            for (0..8) |_| {
                const b = (val & 0b10000000) >> 7;
                try fdat.append(gpa, @intCast(b));
                val <<= 1;
            }
        }

        //std.debug.print("Length of DefaultFontData: {d}\n", .{DefaultFontData.len});
        //std.debug.print("Length of fdat: {d}\n", .{fdat.items.len});

        //std.debug.print("Dumping font:\n", .{});
        //
        //for (0..FONT_GLYPH_COUNT) |ch| {
        //    const index = ch * 64;
        //    std.debug.print("{x:>02}: ", .{ch});
        //    var count: i32 = 0;
        //    for (0..64) |bit| {
        //        if (count >= 8) {
        //            std.debug.print("\n    ", .{});
        //            count = 0;
        //        }
        //        std.debug.print("{d}", .{fdat.items[index + bit]});
        //        count += 1;
        //    }
        //    std.debug.print("\n", .{});
        //}
        //std.debug.print("\n", .{});

        for (0..FONT_GLYPH_COUNT) |ch| {
            const index = ch * 64;
            for (index..index + 64, 0..64) |bitidx, gbitidx| {
                //std.debug.print("{d:>4}:{d:>6}:{d:>6}:{d:>6}, ", .{ ch, index, gbitidx, bitidx });
                ret.glyphs[ch].data[gbitidx] = fdat.items[bitidx]; // TODO: Investigate index out of bounds
            }
            //std.debug.print("\n", .{});
        }
        //std.debug.print("\n", .{});

        return ret;
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

    pub fn init(gpa: std.mem.Allocator) !Gfx {
        return Gfx{ .framebuf = FrameBuffer.init(), .pal_data = PaletteData.init(), .font_data = try FontData.init(gpa), .spr_data = SpriteData.init() };
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
    \\    finalColor = texture(palette, vec2((texelColor.r * 11.0), 0.5));
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
    allocator: std.mem.Allocator,
    lua: *zlua.Lua,
    rand: std.Random,
    on_frame_callbacks: std.ArrayList(i32),

    cfg: config.Config,
    cart_slot: ?*cart.Cartridge,

    pub fn init(allocator: std.mem.Allocator) !Console {
        const tmp_frmbuf = rl.genImageColor(FRAMEBUFFER_PIX_WIDTH, FRAMEBUFFER_PIX_HEIGHT, .black);
        const tmp_pal = rl.genImageColor(PALETTE_SIZE, 1, .black);
        const tmp_shdr = try rl.loadShaderFromMemory(null, frag_shader_src);
        var seed: u64 = undefined;
        std.crypto.random.bytes(std.mem.asBytes(&seed));
        var prng = std.Random.DefaultPrng.init(seed);
        return Console{
            .gfx = try Gfx.init(allocator),
            .running = false,
            .frmbuf_dirty = true,
            .pal_dirty = true,
            .frmbuf_copy_img = tmp_frmbuf,
            .pal_copy_img = tmp_pal,
            .frmbuf_copy = try rl.loadTextureFromImage(tmp_frmbuf),
            .pal_copy = try rl.loadTextureFromImage(tmp_pal),
            .rtex = try rl.loadRenderTexture(FRAMEBUFFER_PIX_WIDTH, FRAMEBUFFER_PIX_HEIGHT),
            .pal_shdr = tmp_shdr,
            .pal_shdr_palettedTexture_loc = 0,
            .pal_shdr_palette_loc = rl.getShaderLocation(tmp_shdr, "palette"),
            .exit_value = 0,
            .allocator = allocator,
            .lua = try zlua.Lua.init(allocator),
            .rand = prng.random(),
            .on_frame_callbacks = try std.ArrayList(i32).initCapacity(allocator, 1),
            .cfg = config.Config.load(allocator) catch config.Config.default(),
            .cart_slot = null,
        };
    }

    pub fn load_new_cart(self: *Console) !void {
        if (self.cart_slot != null) {
            self.cart_slot.?.deinit(self.allocator);
            self.allocator.free(self.cart_slot.?);
            self.cart_slot = null;
        }

        self.cart_slot = try cart.Cartridge.alloc(self.allocator);
    }

    pub fn deinit(self: *Console) !void {
        // Save Configuration
        self.cfg.save(self.allocator) catch |err| {
            std.debug.print("Unable to save Configuration! {}\n", .{err});
        };

        if (self.cart_slot != null) {
            self.cart_slot.?.deinit(self.allocator);
            self.allocator.destroy(self.cart_slot.?);
            self.cart_slot = null;
        }
        self.cfg.deinit(self.allocator);
        self.on_frame_callbacks.deinit(self.allocator);
        self.lua.deinit();
        rl.unloadShader(self.pal_shdr);
        rl.unloadRenderTexture(self.rtex);
        rl.unloadTexture(self.pal_copy);
        rl.unloadTexture(self.frmbuf_copy);
        rl.unloadImage(self.pal_copy_img);
        rl.unloadImage(self.frmbuf_copy_img);
    }

    pub fn run(self: *Console, script: ?[:0]const u8) !void {
        // Load Lua default state.
        self.lua.openBase();
        self.lua.openPackage();
        self.lua.openTable();
        self.lua.openString();
        self.lua.openMath();
        self.lua.openDebug();

        self.loadLuaAPI();

        // Load the default palette.
        self.gfx.pal_data.loadDefaultPriv(self);

        self.running = true;
        self.exit_value = 0;
        self.pal_dirty = true;
        self.frmbuf_dirty = true;

        if (script != null) {
            var file = try std.fs.cwd().openFile(script.?, .{});
            defer file.close();

            const stat = try file.stat();
            const file_size = stat.size;

            var bbuf: [1024]u8 = undefined;
            var reader = file.reader(&bbuf);
            const ioreader = &reader.interface;
            const buffer = try ioreader.readAlloc(self.allocator, file_size);
            defer self.allocator.free(buffer);
            const bufferz = try self.allocator.dupeZ(u8, buffer);
            defer self.allocator.free(bufferz);

            const result = self.lua.doString(bufferz);

            if (@typeInfo(@TypeOf(result)).error_union.error_set == anyerror) {
                switch (result) {
                    zlua.Error => |lua_err| {
                        const emsg = self.lua.toStringEx(1);
                        std.debug.print("Lua Error: {}", .{emsg});
                        self.lua.traceback(self.lua, null, 1);
                        const tmsg = self.lua.toStringEx(1);
                        std.debug.print("Stack Track: {}", .{tmsg});
                        self.lua.pop(2);
                        return lua_err;
                    },
                    else => |other_err| return other_err,
                }
            }
        }

        var main_fn_t: zlua.LuaType = undefined;
        if (self.lua.getGlobal("MAIN")) |lua_type| {
            main_fn_t = lua_type;
        } else |err| {
            const emsg = self.lua.toStringEx(1);
            std.debug.print("Lua Error: {s}", .{emsg});
            self.lua.pop(1);
            return err;
        }
        if (main_fn_t == zlua.LuaType.function) {
            std.debug.print("MAIN function found!\n", .{});
        } else {
            std.debug.print("MAIN found but is not a function!\n", .{});
            const DraconRunError = error{DraconMainNotAFunction};
            return DraconRunError.DraconMainNotAFunction;
        }
        self.lua.pop(1);

        while ((!rl.windowShouldClose()) and (self.isRunning())) {
            //_ = try self.lua.autoCall(void, "MAIN", .{});
            _ = try self.lua.getGlobal("MAIN");
            self.lua.call(.{ .args = 0, .results = 0 });

            // Do On Frame Callbacks
            for (self.on_frame_callbacks.items) |id| {
                _ = self.lua.rawGetIndex(zlua.registry_index, id);
                self.lua.call(.{ .args = 0, .results = 0 });
            }
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
        rl.drawTexturePro(self.rtex.texture, rl.Rectangle{ .x = 0, .y = 0, .width = @as(f32, FRAMEBUFFER_PIX_WIDTH), .height = @as(f32, FRAMEBUFFER_PIX_HEIGHT) }, rl.Rectangle{ .x = 0, .y = 0, .width = @floatFromInt(rl.getRenderWidth()), .height = @floatFromInt(rl.getRenderHeight()) }, rl.Vector2{ .x = 0, .y = 0 }, 0, .white);
        if (self.cfg.show_fps) {
            rl.drawFPS(0, 0);
        }
        rl.endDrawing();
    }

    pub fn isRunning(self: *Console) bool {
        return self.running;
    }

    fn rebuildPal(self: *Console) void {
        for (0..PALETTE_SIZE) |i| {
            const pal_col = self.gfx.pal_data.getEntry(@intCast(i));
            rl.imageDrawPixel(&(self.pal_copy_img), @intCast(i), 0, rl.Color{ .r = pal_col.R, .g = pal_col.G, .b = pal_col.B, .a = 255 });
        }
        rl.updateTexture(self.pal_copy, self.pal_copy_img.data);

        self.pal_dirty = false;
    }

    fn rebuildFramebuffer(self: *Console) void {
        for (0..FRAMEBUFFER_PIX_HEIGHT) |y| {
            for (0..FRAMEBUFFER_PIX_WIDTH) |x| {
                const pal_idx = self.gfx.framebuf.getPixelFast(@intCast(x), @intCast(y));
                rl.imageDrawPixel(&(self.frmbuf_copy_img), @intCast(x), @intCast(y), rl.Color{ .r = pal_idx, .g = pal_idx, .b = pal_idx, .a = 255 });
            }
        }
        rl.updateTexture(self.frmbuf_copy, self.frmbuf_copy_img.data);

        self.frmbuf_dirty = false;
    }

    fn loadLuaAPI(self: *Console) void {
        _ = self.lua.atPanic(lua_on_panic);

        self.lua.pushFunction(zlua.wrap(lua_api_print));
        self.lua.setGlobal("print");

        self.lua.pushFunction(zlua.wrap(lua_api_exit));
        self.lua.setGlobal("exit");

        self.lua.pushFunction(zlua.wrap(lua_api_dofile));
        self.lua.setGlobal("dofile");

        self.lua.pushFunction(zlua.wrap(lua_api_loadfile));
        self.lua.setGlobal("loadfile");

        self.lua.pushFunction(zlua.wrap(lua_api_cls));
        self.lua.setGlobal("cls");

        self.lua.pushFunction(zlua.wrap(lua_api_pix));
        self.lua.setGlobal("pix");

        self.lua.pushFunction(zlua.wrap(lua_api_putch));
        self.lua.setGlobal("putch");

        self.lua.pushFunction(zlua.wrap(lua_api_puts));
        self.lua.setGlobal("puts");

        self.lua.pushFunction(zlua.wrap(lua_api_puts_rainbow));
        self.lua.setGlobal("puts_rainbow");

        self.lua.pushFunction(zlua.wrap(lua_api_register_on_frame));
        self.lua.setGlobal("register_on_frame");

        self.lua.pushFunction(zlua.wrap(lua_api_unregister_on_frame));
        self.lua.setGlobal("unregister_on_frame");
    }

    //pub fn registerOnFrameCallback(self: *Console, func: []const u8) !void {
    //    try self.on_frame_callbacks.append(self.allocator, func);
    //}

    pub fn registerOnFrameCallback(self: *Console) !void {
        for (self.on_frame_callbacks.items) |id| {
            _ = self.lua.rawGetIndex(zlua.registry_index, id);
            defer self.lua.pop(1);
            const same = self.lua.compare(1, 2, .eq);
            if (same) {
                return;
            }
        }
        self.lua.pushValue(1);
        const ref = try self.lua.ref(zlua.registry_index);
        try self.on_frame_callbacks.append(self.allocator, ref);
    }

    //pub fn unregisterOnFrameCallback(self: *Console, func: []const u8) !void {
    //    const loc = std.mem.find([]const u8, self.on_frame_callbacks, func);
    //    if (loc != null) {
    //        _ = self.on_frame_callbacks.orderedRemove(loc);
    //    }
    //}

    pub fn unregisterOnFrameCallback(self: *Console) !void {
        var id_to_remove: i32 = undefined;
        var id_found = false;
        for (self.on_frame_callbacks.items) |id| {
            _ = self.lua.rawGetIndex(zlua.registry_index, id);
            defer self.lua.pop(1);
            const same = self.lua.compare(1, 2, .eq);
            if (same) {
                self.lua.unref(zlua.registry_index, id);
                id_to_remove = id;
                id_found = true;
                break;
            }
        }
        if (!id_found) {
            return;
        }
        const loc = std.mem.indexOfScalar(i32, self.on_frame_callbacks.items, id_to_remove);
        if (loc != null) {
            _ = self.on_frame_callbacks.orderedRemove(loc.?);
        }
    }
};

var GlobalConsole: ?*Console = null;
pub fn SetConsole(con: ?*Console) void {
    GlobalConsole = con;
}
pub fn GetConsole() ?*Console {
    return GlobalConsole;
}

fn lua_on_panic(_: ?*zlua.LuaState) callconv(.c) c_int {
    const emsg = GetConsole().?.lua.toStringEx(1);
    std.debug.print("Lua Error: {s}", .{emsg});
    GetConsole().?.lua.pop(1);
    return 0;
}

fn lua_api_print(lua: *zlua.Lua) !i32 {
    const txt = lua.toString(1) catch "";
    std.debug.print("[DRACON PRINT]{s}", .{txt});
    return 0;
}

fn lua_api_exit(lua: *zlua.Lua) !i32 {
    const con = GetConsole().?;
    const a = try lua.toInteger(1);
    con.exit_value = @intCast(a);
    con.running = false;
    return 0;
}

fn lua_api_dofile(lua: *zlua.Lua) !i32 {
    _ = try lua.toString(1);
    lua.pushInteger(0);
    return 1;
}

fn lua_api_loadfile(lua: *zlua.Lua) !i32 {
    _ = try lua.toString(1);
    lua.pushInteger(0);
    return 1;
}

fn lua_api_pix(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc == 3) {
        // This is the output version.
        const x = try lua.toInteger(1);
        const y = try lua.toInteger(2);
        const col = try lua.toInteger(3);
        GetConsole().?.gfx.framebuf.setPixel(GetConsole().?, @intCast(x), @intCast(y), @intCast(col));
        return 0;
    } else if (argc == 2) {
        // This is the input version.
        const x = try lua.toInteger(1);
        const y = try lua.toInteger(2);
        const ret = GetConsole().?.gfx.framebuf.getPixel(@intCast(x), @intCast(y)) orelse 0;
        lua.pushInteger(ret);
        return 1;
    } else {
        // This is an invalid number of arguments.
        lua.raiseErrorStr("`pix` expects 2 or 3 arguments, got %d", .{argc});
    }
}

fn lua_api_cls(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 1) {
        lua.raiseErrorStr("`cls` expects 1 argument, got %d", .{argc});
    }
    const col = try lua.toInteger(1);
    GetConsole().?.gfx.framebuf.cls(GetConsole().?, @intCast(col));
    return 0;
}

fn lua_api_putch(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`putch` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const ty = lua.typeOf(3);
    var ch: u8 = undefined;
    if (ty == zlua.LuaType.string) {
        const chs = try lua.toString(3);
        ch = chs[0];
    } else if (ty == zlua.LuaType.number) {
        const tch = try lua.toInteger(3);
        ch = @intCast(tch);
    } else {
        lua.raiseErrorStr("`putch` expects a string or integer, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    const col = try lua.toInteger(4);
    GetConsole().?.gfx.framebuf.putch(GetConsole().?, @intCast(x), @intCast(y), @intCast(ch), @intCast(col));
    return 0;
}

fn lua_api_puts(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`puts` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const ty = lua.typeOf(3);
    var str: []const u8 = undefined;
    if (ty == zlua.LuaType.string) {
        str = try lua.toString(3);
    } else {
        lua.raiseErrorStr("`puts` expects a string, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    const col = try lua.toInteger(4);
    GetConsole().?.gfx.framebuf.puts(GetConsole().?, @intCast(x), @intCast(y), str, @intCast(col));
    return 0;
}

fn lua_api_puts_rainbow(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`puts_rainbow` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const ty = lua.typeOf(3);
    var str: []const u8 = undefined;
    if (ty == zlua.LuaType.string) {
        str = try lua.toString(3);
    } else {
        lua.raiseErrorStr("`puts_rainbow` expects a string, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    const col = try lua.toInteger(4);
    GetConsole().?.gfx.framebuf.puts_rainbow(GetConsole().?, @intCast(x), @intCast(y), str, @intCast(col));
    return 0;
}

fn lua_api_register_on_frame(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 1) {
        lua.raiseErrorStr("`register_on_frame` expects 1 argument, got %d", .{argc});
    }
    const ty = lua.typeOf(1);
    if (ty != zlua.LuaType.function) {
        lua.raiseErrorStr("`register_on_frame` expects a function, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    try GetConsole().?.registerOnFrameCallback();
    return 0;
}

fn lua_api_unregister_on_frame(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 1) {
        lua.raiseErrorStr("`unregister_on_frame` expects 1 argument, got %d", .{argc});
    }
    const ty = lua.typeOf(1);
    if (ty != zlua.LuaType.function) {
        lua.raiseErrorStr("`unregister_on_frame` expects a function, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    try GetConsole().?.unregisterOnFrameCallback();
    return 0;
}
