const std = @import("std");
const rl = @import("raylib");
const uid = @import("uid.zig");
const console = @import("console.zig");
const constants = @import("console/constants.zig");

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
        @memset(&ret.short_name, 0);
        @memcpy(ret.short_name[0.."NEW-CART-NAME".len], "NEW-CART-NAME");
        ret.short_name["NEW-CART-NAME".len] = 0;
        @memset(&ret.long_name, 0);
        @memcpy(ret.long_name[0.."NEW CART LONG NAME".len], "NEW CART LONG NAME");
        ret.long_name["NEW CART LONG NAME".len] = 0;
        @memset(&ret.authors[0], 0);
        @memcpy(ret.authors[0][0.."ENTER AUTHOR NAME".len], "ENTER AUTHOR NAME");
        ret.authors[0]["ENTER AUTHOR NAME".len] = 0;
        for (1..ret.authors.len) |i| {
            @memset(&ret.authors[i], 0);
        }
        @memset(&ret.license, 0);
        @memcpy(ret.license[0.."ENTER LICENSE NAME".len], "ENTER LICENSE NAME");
        ret.license["ENTER LICENSE NAME".len] = 0;
        @memset(&ret.version, 0);
        @memcpy(ret.version[0.."0.1.0".len], "0.1.0");
        ret.version["0.1.0".len] = 0;
        ret.palette = console.PaletteData.init();
        ret.palette.loadDefault();
        ret.sprites = console.SpriteData.init();
        @memset(&ret.storage, 0);
        const hello_script =
            \\-- This is a demo hello world script
            \\function MAIN()
            \\  puts(0, 0, "Hello from Dracon!", 23)
            \\end
        ;
        ret.code = try allocator.allocSentinel(u8, hello_script.len, 0);
        @memcpy(ret.code, hello_script);
        //ret.code[hello_script.len] = 0;

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

    pub fn saveToImage(self: *const Self, allocator: std.mem.Allocator) !void {
        var writer = std.Io.Writer.Allocating.init(allocator);
        try writer.writer.writeSliceEndian(u8, &self.id.toBytes(), .little);
        try writer.writer.writeAll(&self.short_name);
        try writer.writer.writeAll(&self.long_name);
        for (self.authors) |author| {
            try writer.writer.writeAll(&author);
        }
        try writer.writer.writeAll(&self.license);
        try writer.writer.writeAll(&self.version);
        //try writer.writer.writeStruct(self.palette, .little);
        //try writer.writer.writeStruct(self.sprites, .little);
        try writer.writer.writeSliceEndian(i32, &self.storage, .little);
        try writer.writer.writeAll(self.code);
        var data = writer.toArrayList();
        defer data.deinit(allocator);

        if (data.items.len >= 288 * 1024) { // Maximum size is 288 KiB
            return error.CartridgeTooLargeError;
        }

        // TODO: Dumping data
        for (data.items) |b| {
            std.debug.print("{x:02.}", .{b});
        }
        std.debug.print("\n", .{});

        var split_data = try allocator.alloc(u4, data.items.len * 2);
        defer allocator.free(split_data);

        for (data.items, 0..) |item, i| {
            split_data[i * 2] = @intCast((item >> 4) & 0xF);
            split_data[(i * 2) + 1] = @intCast(item & 0xF);
        }

        // TODO: Replace with using cartridge's screenshot.
        var img = rl.genImageColor(constants.FRAMEBUFFER_PIX_WIDTH * 2, constants.FRAMEBUFFER_PIX_HEIGHT * 2, .black);

        var x: i32 = 0;
        var y: i32 = 0;
        var i: i32 = 0;
        for (split_data) |nibble| {
            var c = img.getColor(x, y);
            switch (i) {
                0 => {
                    c.r = (c.r & 0xF0) | @as(u8, @intCast(nibble));
                },
                1 => {
                    c.g = (c.g & 0xF0) | @as(u8, @intCast(nibble));
                },
                2 => {
                    c.b = (c.b & 0xF0) | @as(u8, @intCast(nibble));
                },
                3 => {
                    c.a = (c.a & 0xF0) | @as(u8, @intCast(nibble));
                },
                else => {
                    unreachable;
                },
            }
            img.drawPixel(x, y, c);

            i += 1;

            if (i > 3) {
                i = 0;
                x += 1;
            }

            if (x >= constants.FRAMEBUFFER_PIX_WIDTH * 2) {
                x = 0;
                y += 1;
            }
        }

        // Build filename
        //var name_buf: [128]u8 = undefined;
        //const fname = try std.fmt.bufPrintZ(&name_buf, "{s}.png", .{self.short_name});
        //std.debug.print("Saving cartridge to `{s}`\n", .{fname});
        //if (!img.exportToFile(fname)) {
        if (!img.exportToFile("cart.png")) {
            return error.UnableToExportCartImage;
        }
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

test "Test Saving of Cartridge Image" {
    errdefer |err| {
        std.debug.print("Test Saving of Cartridge Image failed: {}\n", .{err});
    }
    var gpa = std.heap.GeneralPurposeAllocator(.{}).init;
    const allocator = gpa.allocator();
    defer {
        _ = gpa.deinit();
    }
    var cart = try Cartridge.alloc(allocator);
    defer cart.deinit(allocator);
    try cart.saveToImage(allocator);
    std.debug.print("Test Saving of Cartridge Image DONE.\n", .{});
}
