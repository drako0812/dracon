const std = @import("std");
const crypto = @import("std").crypto;
const mem = @import("std").mem;

pub const UID = struct {
    data: [120]u1,

    const Self = @This();

    pub inline fn eq(self: *const Self, other: *const Self) bool {
        return mem.eql(u1, &self.data, &other.data);
    }

    pub fn nil() Self {
        return .{ .data = [_]u1{0} ** 120 };
    }

    pub fn init() Self {
        var bits: [120]u1 = undefined;
        for (0..bits.len) |i| {
            bits[i] = if (crypto.random.boolean()) 1 else 0;
        }
        return .{ .data = bits };
    }

    inline fn bitsToByte(bits: *const [8]u1) u8 {
        return (@as(u8, @intCast(bits[0])) << 7) |
            (@as(u8, @intCast(bits[1])) << 6) |
            (@as(u8, @intCast(bits[2])) << 5) |
            (@as(u8, @intCast(bits[3])) << 4) |
            (@as(u8, @intCast(bits[4])) << 3) |
            (@as(u8, @intCast(bits[5])) << 2) |
            (@as(u8, @intCast(bits[6])) << 1) |
            (@as(u8, @intCast(bits[7])));
    }

    pub fn toBytes(self: *const Self) [120 / 8]u8 {
        var ret: [120 / 8]u8 = undefined;

        for (0..(120 / 8)) |i| {
            ret[i] = Self.bitsToByte(&[8]u1{
                self.data[i * 8],
                self.data[(i * 8) + 1],
                self.data[(i * 8) + 2],
                self.data[(i * 8) + 3],
                self.data[(i * 8) + 4],
                self.data[(i * 8) + 5],
                self.data[(i * 8) + 6],
                self.data[(i * 8) + 7],
            });
        }

        return ret;
    }

    inline fn bitsToNibble(bits: *const [4]u1) u4 {
        return (@as(u4, @intCast(bits[0])) << 3) | (@as(u4, @intCast(bits[1])) << 2) | (@as(u4, @intCast(bits[2])) << 1) | (@as(u4, @intCast(bits[3])));
    }

    fn bitsToNibbles(source: *const [120]u1, dest: *[30]u4) void {
        for (0..dest.*.len) |i| {
            const bits = [4]u1{ source[i * 4], source[i * 4 + 1], source[i * 4 + 2], source[i * 4 + 3] };
            dest[i] = Self.bitsToNibble(&bits);
        }
    }

    inline fn bitsTo5Bit(bits: *const [5]u1) u5 {
        return (@as(u5, @intCast(bits[0])) << 4) | (@as(u5, @intCast(bits[1])) << 3) | (@as(u5, @intCast(bits[2])) << 2) | (@as(u5, @intCast(bits[3])) << 1) | (@as(u5, @intCast(bits[4])));
    }

    fn bitsTo5Bits(source: *const [120]u1, dest: *[24]u5) void {
        for (0..dest.*.len) |i| {
            const bits = [5]u1{ source[i * 5], source[i * 5 + 1], source[i * 5 + 2], source[i * 5 + 3], source[i * 5 + 4] };
            dest[i] = Self.bitsTo5Bit(&bits);
        }
    }

    inline fn bitsTo6Bit(bits: *const [6]u1) u6 {
        return (@as(u6, @intCast(bits[0])) << 5) | (@as(u6, @intCast(bits[1])) << 4) | (@as(u6, @intCast(bits[2])) << 3) | (@as(u6, @intCast(bits[3])) << 2) | (@as(u6, @intCast(bits[4])) << 1) | (@as(u6, @intCast(bits[5])));
    }

    fn bitsTo6Bits(source: *const [120]u1, dest: *[20]u6) void {
        for (0..dest.*.len) |i| {
            const bits = [6]u1{ source[i * 6], source[i * 6 + 1], source[i * 6 + 2], source[i * 6 + 3], source[i * 6 + 4], source[i * 6 + 5] };
            dest[i] = Self.bitsTo6Bit(&bits);
        }
    }

    const CHARS16: []const u8 = "0123456789ABCDEF";
    const CHARS32: []const u8 = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
    const CHARS64: []const u8 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";

    pub fn toString16(self: *const Self, buffer: *[30]u8) void {
        var out: [30]u4 = undefined;
        Self.bitsToNibbles(&self.data, &out);
        for (0..30) |i| {
            buffer.*[i] = CHARS16[out[i]];
        }
    }

    pub fn toString32(self: *const Self, buffer: *[24]u8) void {
        var out: [24]u5 = undefined;
        Self.bitsTo5Bits(&self.data, &out);
        for (0..24) |i| {
            buffer.*[i] = CHARS32[out[i]];
        }
    }

    pub fn toString64(self: *const Self, buffer: *[20]u8) void {
        var out: [20]u6 = undefined;
        Self.bitsTo6Bits(&self.data, &out);
        for (0..20) |i| {
            buffer.*[i] = CHARS64[out[i]];
        }
    }

    fn in(ch: u8, options: []const u8) struct { found: bool, index: usize } {
        for (options, 0..) |option, i| {
            if (ch == option) {
                return .{ .found = true, .index = i };
            }
        }
        return .{ .found = false, .index = 0 };
    }

    inline fn nibbleToBits(nibble: u4) [4]u1 {
        return .{
            @as(u1, @intCast((nibble >> 3) & 1)),
            @as(u1, @intCast((nibble >> 2) & 1)),
            @as(u1, @intCast((nibble >> 1) & 1)),
            @as(u1, @intCast(nibble & 1)),
        };
    }

    fn nibblesToBits(source: *const [30]u4, dest: *[120]u1) void {
        for (0..source.*.len) |i| {
            @memmove(dest.*[i * 4 .. (i * 4) + 4], &Self.nibbleToBits(source.*[i]));
        }
    }

    inline fn bit5ToBits(bits: u5) [5]u1 {
        return .{
            @as(u1, @intCast((bits >> 4) & 1)),
            @as(u1, @intCast((bits >> 3) & 1)),
            @as(u1, @intCast((bits >> 2) & 1)),
            @as(u1, @intCast((bits >> 1) & 1)),
            @as(u1, @intCast(bits & 1)),
        };
    }

    fn bits5ToBits(source: *const [24]u5, dest: *[120]u1) void {
        for (0..source.*.len) |i| {
            @memmove(dest.*[i * 5 .. (i * 5) + 5], &Self.bit5ToBits(source.*[i]));
        }
    }

    inline fn bit6ToBits(bits: u6) [6]u1 {
        return .{
            @as(u1, @intCast((bits >> 5) & 1)),
            @as(u1, @intCast((bits >> 4) & 1)),
            @as(u1, @intCast((bits >> 3) & 1)),
            @as(u1, @intCast((bits >> 2) & 1)),
            @as(u1, @intCast((bits >> 1) & 1)),
            @as(u1, @intCast(bits & 1)),
        };
    }

    fn bits6ToBits(source: *const [20]u6, dest: *[120]u1) void {
        for (0..source.*.len) |i| {
            @memmove(dest.*[i * 6 .. (i * 6) + 6], &Self.bit6ToBits(source.*[i]));
        }
    }

    pub const UIDError = error{
        UIDParseError,
    };

    pub fn fromString16(buffer: *const [30]u8) !Self {
        var data_parts: [30]u4 = undefined;
        for (0..30) |i| {
            const result = Self.in(buffer[i], CHARS16);
            if (result.found) {
                data_parts[i] = @intCast(result.index);
            } else {
                return UIDError.UIDParseError;
            }
        }
        var data: [120]u1 = undefined;
        Self.nibblesToBits(&data_parts, &data);
        return Self{ .data = data };
    }

    pub fn fromString32(buffer: *const [24]u8) !Self {
        var data_parts: [24]u5 = undefined;
        for (0..24) |i| {
            const result = Self.in(buffer[i], CHARS32);
            if (result.found) {
                data_parts[i] = @intCast(result.index);
            } else {
                return UIDError.UIDParseError;
            }
        }
        var data: [120]u1 = undefined;
        Self.bits5ToBits(&data_parts, &data);
        return Self{ .data = data };
    }

    pub fn fromString64(buffer: *const [20]u8) !Self {
        var data_parts: [20]u6 = undefined;
        for (0..20) |i| {
            const result = Self.in(buffer[i], CHARS64);
            if (result.found) {
                data_parts[i] = @intCast(result.index);
            } else {
                return UIDError.UIDParseError;
            }
        }
        var data: [120]u1 = undefined;
        Self.bits6ToBits(&data_parts, &data);
        return Self{ .data = data };
    }
};

test "UID Demo [not an actual test]" {
    const uid1 = UID.init();
    var buf16: [30]u8 = undefined;
    var buf32: [24]u8 = undefined;
    var buf64: [20]u8 = undefined;
    uid1.toString16(&buf16);
    uid1.toString32(&buf32);
    uid1.toString64(&buf64);
    std.debug.print("UID = {s}, {s}, {s}\n", .{ buf16, buf32, buf64 });
    const uid16 = try UID.fromString16(&buf16);
    const uid32 = try UID.fromString32(&buf32);
    const uid64 = try UID.fromString64(&buf64);
    try std.testing.expect(uid1.eq(&uid16));
    try std.testing.expect(uid1.eq(&uid32));
    try std.testing.expect(uid1.eq(&uid64));
}
