const zlua = @import("zlua");
const console = @import("../console.zig");
const std = @import("std");

pub fn lua_on_panic(_: ?*zlua.LuaState) callconv(.c) c_int {
    const emsg = console.GetConsole().?.lua.toStringEx(1);
    std.debug.print("Lua Error: {s}", .{emsg});
    console.GetConsole().?.lua.pop(1);
    return 0;
}

pub fn lua_api_print(lua: *zlua.Lua) !i32 {
    const txt = lua.toString(1) catch "";
    std.debug.print("[DRACON PRINT]{s}", .{txt});
    return 0;
}

pub fn lua_api_exit(lua: *zlua.Lua) !i32 {
    const con = console.GetConsole().?;
    const a = try lua.toInteger(1);
    con.exit_value = @intCast(a);
    con.running = false;
    return 0;
}

pub fn lua_api_dofile(lua: *zlua.Lua) !i32 {
    _ = try lua.toString(1);
    lua.pushInteger(0);
    return 1;
}

pub fn lua_api_loadfile(lua: *zlua.Lua) !i32 {
    _ = try lua.toString(1);
    lua.pushInteger(0);
    return 1;
}

pub fn lua_api_pix(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc == 3) {
        // This is the output version.
        const x = try lua.toInteger(1);
        const y = try lua.toInteger(2);
        const col = try lua.toInteger(3);
        console.GetConsole().?.gfx.framebuf.setPixel(console.GetConsole().?, @intCast(x), @intCast(y), @intCast(col));
        return 0;
    } else if (argc == 2) {
        // This is the input version.
        const x = try lua.toInteger(1);
        const y = try lua.toInteger(2);
        const ret = console.GetConsole().?.gfx.framebuf.getPixel(@intCast(x), @intCast(y)) orelse 0;
        lua.pushInteger(ret);
        return 1;
    } else {
        // This is an invalid number of arguments.
        lua.raiseErrorStr("`pix` expects 2 or 3 arguments, got %d", .{argc});
    }
}

pub fn lua_api_cls(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 1) {
        lua.raiseErrorStr("`cls` expects 1 argument, got %d", .{argc});
    }
    const col = try lua.toInteger(1);
    console.GetConsole().?.gfx.framebuf.cls(console.GetConsole().?, @intCast(col));
    return 0;
}

pub fn lua_api_line(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 5) {
        lua.raiseErrorStr("`line` expects 5 arguments, got %d", .{argc});
    }
    const x1 = try lua.toInteger(1);
    const y1 = try lua.toInteger(2);
    const x2 = try lua.toInteger(3);
    const y2 = try lua.toInteger(4);
    const col = try lua.toInteger(5);
    console.GetConsole().?.gfx.framebuf.line(console.GetConsole().?, @intCast(x1), @intCast(y1), @intCast(x2), @intCast(y2), @intCast(col));
    return 0;
}

pub fn lua_api_rect(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 5) {
        lua.raiseErrorStr("`rect` expects 5 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const w = try lua.toInteger(3);
    const h = try lua.toInteger(4);
    const col = try lua.toInteger(5);
    console.GetConsole().?.gfx.framebuf.rect(console.GetConsole().?, @intCast(x), @intCast(y), @intCast(w), @intCast(h), @intCast(col));
    return 0;
}

pub fn lua_api_rectf(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 5) {
        lua.raiseErrorStr("`rectf` expects 5 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const w = try lua.toInteger(3);
    const h = try lua.toInteger(4);
    const col = try lua.toInteger(5);
    console.GetConsole().?.gfx.framebuf.rectF(console.GetConsole().?, @intCast(x), @intCast(y), @intCast(w), @intCast(h), @intCast(col));
    return 0;
}

pub fn lua_api_circ(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`circ` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const r = try lua.toInteger(3);
    const col = try lua.toInteger(4);
    console.GetConsole().?.gfx.framebuf.circ(console.GetConsole().?, @intCast(x), @intCast(y), @intCast(r), @intCast(col));
    return 0;
}

pub fn lua_api_circf(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`circf` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const r = try lua.toInteger(3);
    const col = try lua.toInteger(4);
    console.GetConsole().?.gfx.framebuf.circF(console.GetConsole().?, @intCast(x), @intCast(y), @intCast(r), @intCast(col));
    return 0;
}

pub fn lua_api_putch(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`putch` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const ty = lua.typeOf(3);
    var ch: u8 = undefined;
    if (ty == zlua.LuaType.string) {
        const chs = try lua.toString(3);
        ch = chs[0];
    } else if (ty == zlua.LuaType.number) {
        const tch = try lua.toInteger(3);
        ch = @intCast(tch);
    } else {
        lua.raiseErrorStr("`putch` expects a string or integer, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    const col = try lua.toInteger(4);
    console.GetConsole().?.gfx.framebuf.putch(console.GetConsole().?, @intCast(x), @intCast(y), @intCast(ch), @intCast(col));
    return 0;
}

pub fn lua_api_puts(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`puts` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const ty = lua.typeOf(3);
    var str: []const u8 = undefined;
    if (ty == zlua.LuaType.string) {
        str = try lua.toString(3);
    } else {
        lua.raiseErrorStr("`puts` expects a string, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    const col = try lua.toInteger(4);
    console.GetConsole().?.gfx.framebuf.puts(console.GetConsole().?, @intCast(x), @intCast(y), str, @intCast(col));
    return 0;
}

pub fn lua_api_puts_rainbow(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 4) {
        lua.raiseErrorStr("`puts_rainbow` expects 4 arguments, got %d", .{argc});
    }
    const x = try lua.toInteger(1);
    const y = try lua.toInteger(2);
    const ty = lua.typeOf(3);
    var str: []const u8 = undefined;
    if (ty == zlua.LuaType.string) {
        str = try lua.toString(3);
    } else {
        lua.raiseErrorStr("`puts_rainbow` expects a string, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    const col = try lua.toInteger(4);
    console.GetConsole().?.gfx.framebuf.puts_rainbow(console.GetConsole().?, @intCast(x), @intCast(y), str, @intCast(col));
    return 0;
}

pub fn lua_api_register_on_frame(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 1) {
        lua.raiseErrorStr("`register_on_frame` expects 1 argument, got %d", .{argc});
    }
    const ty = lua.typeOf(1);
    if (ty != zlua.LuaType.function) {
        lua.raiseErrorStr("`register_on_frame` expects a function, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    try console.GetConsole().?.registerOnFrameCallback();
    return 0;
}

pub fn lua_api_unregister_on_frame(lua: *zlua.Lua) !i32 {
    const argc = lua.getTop();
    if (argc != 1) {
        lua.raiseErrorStr("`unregister_on_frame` expects 1 argument, got %d", .{argc});
    }
    const ty = lua.typeOf(1);
    if (ty != zlua.LuaType.function) {
        lua.raiseErrorStr("`unregister_on_frame` expects a function, got %d", .{@as(i32, @intFromEnum(ty))});
    }
    try console.GetConsole().?.unregisterOnFrameCallback();
    return 0;
}
