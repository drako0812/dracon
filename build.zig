const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const mod = b.addModule("dracon_zig", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
    });

    const exe = b.addExecutable(.{
        .name = "dracon_zig",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "dracon_zig", .module = mod },
            },
            .link_libc = true,
        }),
    });

    const raylib_dep = b.dependency("raylib_zig", .{
        .target = target,
        .optimize = optimize,
    });

    const lua_dep = b.dependency("zlua", .{
        .target = target,
        .optimize = optimize,
        .lang = .lua54,
    });

    //const toml = b.dependency("toml", .{
    //    .target = target,
    //    .optimize = optimize,
    //});

    //const yaml = b.dependency("yaml", .{
    //    .target = target,
    //    .optimize = optimize,
    //});

    const raylib = raylib_dep.module("raylib");
    const raygui = raylib_dep.module("raygui");
    const raylib_artifact = raylib_dep.artifact("raylib");

    raylib_artifact.root_module.addCMacro("SUPPORT_FILEFORMAT_PNG", "");
    raylib_artifact.root_module.addCMacro("SUPPORT_FILEFORMAT_BMP", "");
    raylib_artifact.root_module.addCMacro("SUPPORT_FILEFORMAT_TGA", "");
    raylib_artifact.root_module.addCMacro("SUPPORT_FILEFORMAT_JPG", "");
    raylib_artifact.root_module.addCMacro("SUPPORT_FILEFORMAT_GIF", "");

    mod.linkLibrary(raylib_artifact);
    //exe.linkLibrary(raylib_artifact);
    exe.root_module.linkLibrary(raylib_artifact);

    mod.addImport("raylib", raylib);
    mod.addImport("raygui", raygui);
    mod.addImport("zlua", lua_dep.module("zlua"));
    //mod.addImport("toml", toml.module("toml"));
    //mod.addImport("yaml", yaml.module("yaml"));
    //mod.addImport("tracy", tracy.module("tracy"));
    //if (tracy_enable) {
    //    mod.linkLibrary(tracy.artifact("tracy"));
    //    mod.link_libcpp = true;
    //}
    exe.root_module.addImport("raylib", raylib);
    exe.root_module.addImport("raygui", raygui);
    exe.root_module.addImport("zlua", lua_dep.module("zlua"));
    //exe.root_module.addImport("toml", toml.module("toml"));
    //exe.root_module.addImport("yaml", yaml.module("yaml"));

    const install_data = b.addInstallDirectory(.{
        .source_dir = .{ .src_path = .{ .owner = b, .sub_path = "examples" } },
        .install_dir = .{ .prefix = {} },
        .install_subdir = "examples",
    });

    const install_data2 = b.addInstallDirectory(.{
        .source_dir = .{ .src_path = .{ .owner = b, .sub_path = "default_filesystem" } },
        .install_dir = .{ .prefix = {} },
        .install_subdir = "default_filesystem",
    });

    b.getInstallStep().dependOn(&install_data.step);
    b.getInstallStep().dependOn(&install_data2.step);

    b.installArtifact(exe);
    b.getInstallStep().dependOn(&exe.step);

    const run_step = b.step("run", "Run the app");

    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const mod_tests = b.addTest(.{
        .root_module = mod,
    });

    const run_mod_tests = b.addRunArtifact(mod_tests);

    const exe_tests = b.addTest(.{
        .root_module = exe.root_module,
    });

    const run_exe_tests = b.addRunArtifact(exe_tests);

    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&run_mod_tests.step);
    test_step.dependOn(&run_exe_tests.step);
}
