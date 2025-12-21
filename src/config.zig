//! This module handles the saving and loading of configuration data for the Dracon console.

const std = @import("std");
//const toml = @import("toml");
//const Yaml = @import("yaml").Yaml;

pub const ConfigError = error{
    ConfigLoadShowFPSFailure,
};

pub const Config = struct {
    const Self = @This();

    show_fps: bool = false,

    pub fn default() Self {
        return .{};
    }

    //pub fn load(allocator: std.mem.Allocato) Self {
    //    var parser = toml.Parser(Self).init(allocator);
    //    defer parser.deinit();
    //
    //    var result = parser.parseFile("./config.toml") catch {
    //        std.debug.print("Unable to load Configuration, creating default!\n", .{});
    //        return Self.default();
    //    };
    //    defer result.deinit();
    //
    //    return result.value;
    //}

    pub fn load(allocator: std.mem.Allocator) !Self {
        _ = allocator;
        errdefer |err| {
            std.debug.print("Unable to open config.txt, creating default: {}\n", .{err});
        }

        const file = try std.fs.cwd().openFile("config.txt", .{});
        defer file.close();

        var rbuf: [1024]u8 = undefined;
        var reader = file.reader(&rbuf);
        const ioreader = &reader.interface;
        var ret = Self{};
        const tmp_show_fps = try ioreader.takeDelimiter('\n');
        if (tmp_show_fps == null) {
            return @as(anyerror!Self, ConfigError.ConfigLoadShowFPSFailure);
        }
        if (std.mem.eql(u8, "show_fps on", tmp_show_fps.?)) {
            ret.show_fps = true;
        } else if (std.mem.eql(u8, "show_fps off", tmp_show_fps.?)) {
            ret.show_fps = false;
        } else {
            return @as(anyerror!Self, ConfigError.ConfigLoadShowFPSFailure);
        }

        return ret;
    }

    pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
        _ = self;
        _ = allocator;
        // TODO: This doesn't do anything yet.
    }

    //pub fn save(self: *Self, allocator: std.mem.Allocator) !void {
    //    var file = std.fs.cwd().createFile("config.toml", .{}) catch |err| {
    //        std.debug.print("Error while trying to open config.toml for saving: {}\n", .{err});
    //        return err;
    //    };
    //    defer file.close();
    //
    //    var wbuf: [1024]u8 = undefined;
    //    var writer = file.writer(&wbuf);
    //    const iowriter = &writer.interface;
    //    toml.serialize(allocator, self, iowriter) catch |err| {
    //        std.debug.print("Error while serializing: {}\n", .{err});
    //        return err;
    //    };
    //}

    pub fn save(self: *Self, allocator: std.mem.Allocator) !void {
        _ = allocator;
        errdefer |err| {
            std.debug.print("Error while trying to open config.txt for saving: {}\n", .{err});
        }

        var file = try std.fs.cwd().createFile("config.txt", .{});
        defer file.close();

        var wbuf: [1024]u8 = undefined;
        var writer = file.writer(&wbuf);
        const iowriter = &writer.interface;
        if (self.show_fps) {
            try iowriter.print("show_fps on\n", .{});
            try iowriter.flush();
        } else {
            try iowriter.print("show_fps off\n", .{});
            try iowriter.flush();
        }
    }
};
