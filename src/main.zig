const std = @import("std");

const dracon = @import("dracon_zig");
const rl = @import("raylib");
const zlua = @import("zlua");
const ztracy = @import("ztracy");

const script_file = "examples/ex107_circ_border.lua";

pub fn main() anyerror!void {
    const tracy_zone = ztracy.ZoneS(@src(), 32);
    defer tracy_zone.End();

    var uids = [_]dracon.uid.UID{dracon.uid.UID.nil()} ** 10;
    for (0..uids.len) |i| {
        uids[i] = dracon.uid.UID.init();
    }
    var uid_buf: [30]u8 = undefined;
    var uid_buf32: [24]u8 = undefined;
    var uid_buf64: [20]u8 = undefined;
    for (uids, 0..) |cur_uid, i| {
        cur_uid.toString16(&uid_buf);
        cur_uid.toString32(&uid_buf32);
        cur_uid.toString64(&uid_buf64);
        std.debug.print("UID[{}] = {s}, {s}, {s}\n", .{ i, uid_buf, uid_buf32, uid_buf64 });
    }

    var gpa = std.heap.GeneralPurposeAllocator(.{}).init;
    const allocator = gpa.allocator();
    errdefer {
        _ = gpa.deinit();
    }

    rl.initWindow(dracon.constants.FRAMEBUFFER_PIX_WIDTH * 3, dracon.constants.FRAMEBUFFER_PIX_HEIGHT * 3, "raylib-zig");
    defer rl.closeWindow();

    var test_cart = try dracon.cart.Cartridge.alloc(allocator);
    defer {
        test_cart.deinit(allocator);
        allocator.destroy(test_cart);
    }
    try test_cart.saveToImage(allocator);

    var con = try allocator.create(dracon.console.Console);
    con.* = try dracon.console.Console.init(allocator);
    errdefer {
        con.deinit() catch |err| {
            std.debug.print("An Error occurred while deinitializing Console: {}", .{err});
        };
        allocator.destroy(con);
    }
    dracon.console.SetConsole(con);

    rl.setTargetFPS(60);

    try con.run(script_file);

    con.deinit() catch |err| {
        std.debug.print("An Error occurred while deinitializing Console: {}", .{err});
    };
    allocator.destroy(con);

    const result = gpa.deinit();
    if (result == .leak) {
        std.debug.print("LEAKS DETECTED!\n", .{});
    }
}
