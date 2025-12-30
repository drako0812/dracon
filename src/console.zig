const std = @import("std");

const rl = @import("raylib");
const zlua = @import("zlua");
const ztracy = @import("ztracy");

const cart = @import("cart.zig");
const config = @import("config.zig");
pub const FrameBuffer = @import("console/fb.zig").FrameBuffer;
pub const PaletteData = @import("console/pal.zig").PaletteData;
pub const FontData = @import("console/font.zig").FontData;
pub const SpriteData = @import("console/spr.zig").SpriteData;
pub const Gfx = @import("console/gfx.zig").Gfx;
const constants = @import("console/constants.zig");
const api = @import("console/lua.zig");

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
        const tmp_frmbuf = rl.genImageColor(constants.FRAMEBUFFER_PIX_WIDTH, constants.FRAMEBUFFER_PIX_HEIGHT, .black);
        const tmp_pal = rl.genImageColor(constants.PALETTE_SIZE, 1, .black);
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
            .rtex = try rl.loadRenderTexture(constants.FRAMEBUFFER_PIX_WIDTH, constants.FRAMEBUFFER_PIX_HEIGHT),
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
        const zone = ztracy.ZoneS(@src(), 32);
        defer zone.End();

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
            const zone_loop = ztracy.ZoneS(@src(), 32);
            defer zone_loop.End();
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
        const zone = ztracy.ZoneS(@src(), 32);
        defer zone.End();

        if (self.pal_dirty) {
            self.rebuildPal();
        }

        if (self.frmbuf_dirty) {
            self.rebuildFramebuffer();
        }

        rl.beginTextureMode(self.rtex);
        rl.beginShaderMode(self.pal_shdr);
        rl.setShaderValueTexture(self.pal_shdr, self.pal_shdr_palette_loc, self.pal_copy);
        rl.drawTextureRec(self.frmbuf_copy, rl.Rectangle{ .x = 0, .y = 0, .width = constants.FRAMEBUFFER_PIX_WIDTH, .height = -constants.FRAMEBUFFER_PIX_HEIGHT }, rl.Vector2{ .x = 0, .y = 0 }, .white);

        rl.endShaderMode();
        rl.endTextureMode();

        rl.beginDrawing();
        rl.clearBackground(.black);
        rl.drawTexturePro(self.rtex.texture, rl.Rectangle{ .x = 0, .y = 0, .width = @as(f32, constants.FRAMEBUFFER_PIX_WIDTH), .height = @as(f32, constants.FRAMEBUFFER_PIX_HEIGHT) }, rl.Rectangle{ .x = 0, .y = 0, .width = @floatFromInt(rl.getRenderWidth()), .height = @floatFromInt(rl.getRenderHeight()) }, rl.Vector2{ .x = 0, .y = 0 }, 0, .white);
        if (self.cfg.show_fps) {
            rl.drawFPS(0, 0);
        }
        rl.endDrawing();

        ztracy.FrameMark();
    }

    pub fn isRunning(self: *Console) bool {
        return self.running;
    }

    fn rebuildPal(self: *Console) void {
        const zone = ztracy.ZoneS(@src(), 32);
        defer zone.End();

        for (0..constants.PALETTE_SIZE) |i| {
            const pal_col = self.gfx.pal_data.getEntry(@intCast(i));
            rl.imageDrawPixel(&(self.pal_copy_img), @intCast(i), 0, rl.Color{ .r = pal_col.R, .g = pal_col.G, .b = pal_col.B, .a = 255 });
        }
        rl.updateTexture(self.pal_copy, self.pal_copy_img.data);

        self.pal_dirty = false;
    }

    fn rebuildFramebuffer(self: *Console) void {
        const zone = ztracy.ZoneS(@src(), 32);
        defer zone.End();

        for (0..constants.FRAMEBUFFER_PIX_HEIGHT) |y| {
            for (0..constants.FRAMEBUFFER_PIX_WIDTH) |x| {
                const pal_idx = self.gfx.framebuf.getPixelFast(@intCast(x), @intCast(y));
                rl.imageDrawPixel(&(self.frmbuf_copy_img), @intCast(x), @intCast(y), rl.Color{ .r = pal_idx, .g = pal_idx, .b = pal_idx, .a = 255 });
            }
        }
        rl.updateTexture(self.frmbuf_copy, self.frmbuf_copy_img.data);

        self.frmbuf_dirty = false;
    }

    fn loadLuaAPI(self: *Console) void {
        _ = self.lua.atPanic(api.lua_on_panic);

        self.lua.pushFunction(zlua.wrap(api.lua_api_print));
        self.lua.setGlobal("print");

        self.lua.pushFunction(zlua.wrap(api.lua_api_exit));
        self.lua.setGlobal("exit");

        self.lua.pushFunction(zlua.wrap(api.lua_api_dofile));
        self.lua.setGlobal("dofile");

        self.lua.pushFunction(zlua.wrap(api.lua_api_loadfile));
        self.lua.setGlobal("loadfile");

        self.lua.pushFunction(zlua.wrap(api.lua_api_cls));
        self.lua.setGlobal("cls");

        self.lua.pushFunction(zlua.wrap(api.lua_api_pix));
        self.lua.setGlobal("pix");

        self.lua.pushFunction(zlua.wrap(api.lua_api_line));
        self.lua.setGlobal("line");

        self.lua.pushFunction(zlua.wrap(api.lua_api_rect));
        self.lua.setGlobal("rect");

        self.lua.pushFunction(zlua.wrap(api.lua_api_rectf));
        self.lua.setGlobal("rectf");

        self.lua.pushFunction(zlua.wrap(api.lua_api_circ));
        self.lua.setGlobal("circ");

        self.lua.pushFunction(zlua.wrap(api.lua_api_circf));
        self.lua.setGlobal("circf");

        self.lua.pushFunction(zlua.wrap(api.lua_api_putch));
        self.lua.setGlobal("putch");

        self.lua.pushFunction(zlua.wrap(api.lua_api_puts));
        self.lua.setGlobal("puts");

        self.lua.pushFunction(zlua.wrap(api.lua_api_puts_rainbow));
        self.lua.setGlobal("puts_rainbow");

        self.lua.pushFunction(zlua.wrap(api.lua_api_register_on_frame));
        self.lua.setGlobal("register_on_frame");

        self.lua.pushFunction(zlua.wrap(api.lua_api_unregister_on_frame));
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
