const std = @import("std");

const uid = @import("uid.zig");
const console = @import("console.zig");

pub const Cartridge = struct {
    const Self = @This();

    id: uid.UID,
    short_name: [16:0]u8,
    long_name: [64:0]u8,
    authors: [10][32:0]u8,
    license: [32:0]u8,
    version: [32:0]u8,

    // TODO: font

    palette: console.PaletteData,
    sprites: console.SpriteData,

    // TODO: maps
    // TODO: sounds
    // TODO: songs

    storage: [256]i32,
    code: [:0]u8,

    pub fn alloc(allocator: std.mem.Allocator) !*Self {
        var ret = try allocator.create(Self);
        ret.id = uid.UID.init();
        @memmove(ret.short_name, "NEW-CART-NAME");
        @memmove(ret.long_name, "NEW CART LONG NAME");
        @memmove(ret.authors[0], "ENTER AUTHOR NAME");
        for (1..ret.authors.len) |i| {
            @memset(ret.authors[i], 0);
        }
        @memmove(ret.license, "ENTER LICENSE NAME");
        @memmove(ret.version, "0.1.0");
        ret.palette = console.PaletteData.init();
        ret.palette.loadDefault();
        ret.sprites = console.SpriteData.init();
        @memset(ret.storage, 0);
        const hello_script =
            \\-- This is a demo hello world script
            \\function MAIN()
            \\  puts(0, 0, "Hello from Dracon!", 23)
            \\end
        ;
        ret.code = try allocator.allocSentinel(u8, hello_script.len, 0);
        @memmove(ret.code, hello_script);

        return ret;
    }

    pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
        allocator.free(self.code);
    }

    pub fn loadFromInto(self: *Self, allocator: std.mem.Allocator, name: []u8) !void {
        const loaded = try Self.loadFrom(name);
        self.id = loaded.id;
        @memset(self.short_name, 0);
        @memmove(self.short_name, loaded.short_name);
        @memset(self.long_name, 0);
        @memmove(self.long_name, loaded.long_name);
        @memset(self.authors, '\x00' ** 32);
        for (0..self.authors.len) |i| {
            @memmove(self.authors[i], loaded.authors[i]);
        }
        @memset(self.license, 0);
        @memmove(self.license, loaded.license);
        @memset(self.version, 0);
        @memmove(self.version, loaded.version);
        self.palette = loaded.palette;
        self.sprites = loaded.sprites;
        @memset(self.storage, 0);
        @memmove(self.storage, loaded.storage);
        allocator.free(self.code);
        self.code = try allocator.dupeZ(u8, loaded.code);
    }

    pub fn new_id(self: *Self) void {
        self.id = uid.UID.init();
    }

    pub fn save(self: *const Self) !void {
        try self.saveTo(self.short_name);
    }

    pub fn saveTo(self: *const Self, name: []u8) !void {
        // Build filename
        var name_buf: [128]u8 = undefined;
        const fname = try std.fmt.bufPrint(&name_buf, "{s}.dccart", .{name});
        var file = try std.fs.cwd().openFileZ(fname, .{ .mode = .write_only });
        defer file.close();

        var wbuf: [1024]u8 = undefined;
        var writer = file.writer(&wbuf);
        const iowriter = &writer.interface;
        try iowriter.writeStruct(*self, .little);
    }

    pub fn loadFrom(name: []u8) !Self {
        // Build filename
        var name_buf: [128]u8 = undefined;
        const fname = try std.fmt.bufPrint(&name_buf, "{s}.dccart", .{name});
        var file = try std.fs.cwd().openFileZ(fname, .{});
        defer file.close();

        var rbuf: [1024]u8 = undefined;
        var reader = file.reader(&rbuf);
        const ioreader = &reader.interface;
        return try ioreader.takeStruct(Self, .little);
    }
};

test "Size of Cartridge [not a test]" {
    std.debug.print("Cartridge Size: {} B, {:.2} KiB, {:.2} MiB\n", .{
        @sizeOf(Cartridge),
        @as(f64, @sizeOf(Cartridge)) / 1024,
        (@as(f64, @sizeOf(Cartridge)) / 1024) / 1024,
    });
}
