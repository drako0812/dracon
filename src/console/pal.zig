const std = @import("std");
const constants = @import("./constants.zig");
const Console = @import("../console.zig").Console;

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
    pal: [constants.PALETTE_SIZE]PaletteEntry,

    pub fn init() PaletteData {
        return PaletteData{ .pal = std.mem.zeroes([constants.PALETTE_SIZE]PaletteEntry) };
    }

    pub fn setEntry(self: *PaletteData, console: *Console, index: u8, red: u8, green: u8, blue: u8) void {
        self.pal[index] = .{ .R = red, .G = green, .B = blue };
        console.pal_dirty = true;
    }

    pub fn getEntry(self: *PaletteData, index: u8) PaletteEntry {
        return self.pal[index];
    }

    pub fn loadDefaultPriv(self: *PaletteData, console: *Console) void {
        self.loadDefault();
        console.pal_dirty = true;
    }

    pub fn loadDefault(self: *PaletteData) void {
        self.pal = [constants.PALETTE_SIZE]PaletteEntry{
            PAL(0x000000), PAL(0x000080), PAL(0x008000), PAL(0x008080), PAL(0x800000), PAL(0x800080), PAL(0x808000), PAL(0x909090),
            PAL(0x303030), PAL(0x0000FF), PAL(0x00FF00), PAL(0x00FFFF), PAL(0xFF0000), PAL(0xFF00FF), PAL(0xFFFF00), PAL(0xC0C0C0),
            PAL(0x606060), PAL(0x6000C0), PAL(0x00C060), PAL(0x0060C0), PAL(0xC06000), PAL(0xC00060), PAL(0x60C000), PAL(0xFFFFFF),
        };
    }
};
