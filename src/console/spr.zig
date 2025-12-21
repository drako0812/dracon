const std = @import("std");
const constants = @import("./constants.zig");

pub const Sprite = struct {
    pixels: [constants.SPRITE_PIXELS]u5,

    pub fn init() Sprite {
        return Sprite{ .pixels = std.mem.zeroes([constants.SPRITE_PIXELS]u5) };
    }
};

pub const SpriteData = struct {
    sprites: [constants.SPRITE_MEMORY_SIZE]Sprite,

    pub fn init() SpriteData {
        return SpriteData{ .sprites = .{Sprite.init()} ** constants.SPRITE_MEMORY_SIZE };
    }
};
