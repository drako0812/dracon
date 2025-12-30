const FrameBuffer = @import("./fb.zig").FrameBuffer;
const PaletteData = @import("./pal.zig").PaletteData;
const FontData = @import("./font.zig").FontData;
const SpriteData = @import("./spr.zig").SpriteData;
const std = @import("std");

pub const Gfx = struct {
    framebuf: FrameBuffer,
    pal_data: PaletteData,
    font_data: FontData,
    spr_data: SpriteData,

    pub fn init(gpa: std.mem.Allocator) !Gfx {
        return Gfx{ .framebuf = FrameBuffer.init(), .pal_data = PaletteData.init(), .font_data = try FontData.init(gpa), .spr_data = SpriteData.init() };
    }
};
