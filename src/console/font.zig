const std = @import("std");
const constants = @import("./constants.zig");

const DefaultFontData = @embedFile("font.dat");

pub const FontGlyph = struct {
    data: [constants.SPRITE_PIXELS]u1,

    pub fn init() FontGlyph {
        return FontGlyph{ .data = std.mem.zeroes([constants.SPRITE_PIXELS]u1) };
    }
};

pub const FontData = struct {
    glyphs: [constants.FONT_GLYPH_COUNT]FontGlyph,

    pub fn init(gpa: std.mem.Allocator) !FontData {
        var ret = FontData{ .glyphs = .{FontGlyph.init()} ** constants.FONT_GLYPH_COUNT };

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

        for (0..constants.FONT_GLYPH_COUNT) |ch| {
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
