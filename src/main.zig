const dracon = @import("dracon_zig");
const rl = @import("raylib");

pub fn main() anyerror!void {
    var fb = dracon.console.FrameBuffer.init();
    fb.line(1, 1, 10, 11, 23);

    const screenWidth = 800;
    const screenHeight = 450;

    rl.initWindow(screenWidth, screenHeight, "raylib-zig");
    defer rl.closeWindow();

    rl.setTargetFPS(60);

    while (!rl.windowShouldClose()) {
        rl.beginDrawing();
        defer rl.endDrawing();

        rl.clearBackground(.black);

        rl.drawText("Congrats! You created your first window!", 190, 200, 20, .ray_white);
    }
}
