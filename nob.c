#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRACATED
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

const char * dracon_sources[] = {(SRC_FOLDER "main.cpp"),
                                 (SRC_FOLDER "config.cpp"),
                                 (SRC_FOLDER "uid.cpp"),
                                 (SRC_FOLDER "cart.cpp"),
                                 (SRC_FOLDER "console.cpp"),
                                 (SRC_FOLDER "console/font.cpp"),
                                 (SRC_FOLDER "console/pal.cpp"),
                                 (SRC_FOLDER "console/sprite.cpp"),
                                 (SRC_FOLDER "console/gfx.cpp"),
                                 (SRC_FOLDER "console/fb.cpp"),
                                 (SRC_FOLDER "console/gamepad.cpp"),
                                 (SRC_FOLDER "console/keyboard.cpp"),
                                 (SRC_FOLDER "editor/manager.cpp"),
                                 (SRC_FOLDER "editor/code_editor.cpp")};

const char * dracon_objs[] = {(SRC_FOLDER "main.o"),
                              (SRC_FOLDER "config.o"),
                              (SRC_FOLDER "uid.o"),
                              (SRC_FOLDER "cart.o"),
                              (SRC_FOLDER "console.o"),
                              (SRC_FOLDER "console/font.o"),
                              (SRC_FOLDER "console/pal.o"),
                              (SRC_FOLDER "console/sprite.o"),
                              (SRC_FOLDER "console/gfx.o"),
                              (SRC_FOLDER "console/fb.o"),
                              (SRC_FOLDER "console/gamepad.o"),
                              (SRC_FOLDER "console/keyboard.o"),
                              (SRC_FOLDER "editor/manager.o"),
                              (SRC_FOLDER "editor/code_editor.o")};

const char * imgui_sources[] = {
  "deps/imgui/imgui.cpp",
  "deps/imgui/imgui_demo.cpp",
  "deps/imgui/imgui_draw.cpp",
  "deps/imgui/imgui_tables.cpp",
  "deps/imgui/imgui_widgets.cpp",
  "deps/imgui/misc/cpp/imgui_stdlib.cpp",
};

const char * imgui_objs[] = {
  "deps/imgui/imgui.o",
  "deps/imgui/imgui_demo.o",
  "deps/imgui/imgui_draw.o",
  "deps/imgui/imgui_tables.o",
  "deps/imgui/imgui_widgets.o",
  "deps/imgui/misc/cpp/imgui_stdlib.o",
};

#define IMGUI_FLAGS "-I./deps/imgui", "-g"

const char * rlimgui_sources[] = {"deps/rlImGui/rlImGui.cpp"};

const char * rlimgui_objs[] = {"deps/rlImGui/rlImGui.o"};

#define RLIMGUI_FLAGS "-I./deps/imgui", "-I./deps/rlImGui", "-I./deps/raylib/build/raylib/include", "-g"

const char * imgui_color_text_edit_sources[] = {"deps/ImGuiColorTextEdit/TextEditor.cpp"};

const char * imgui_color_text_edit_objs[] = {"deps/ImGuiColorTextEdit/TextEditor.o"};

#define IMGUI_CTXTE_FLAGS "-I./deps/imgui", "-I./deps/ImGuiColorTextEdit", "-g"

#define DRACON_FLAGS                                                                                                        \
    "-L.", "-L./deps/lua/", "-L./deps/raylib/build/raylib", "-L./deps/PlatformFolders/build", "-L./deps/imgui",             \
      "-I./include", "-I./deps/lua", "-I./deps/sol2/include", "-I./deps/raylib/build/raylib/include",                       \
      "-I./deps/tomlplusplus/include", "-I./deps/PlatformFolders", "-I./deps/random/include", "-I./deps/imgui",             \
      "-I./deps/imgui/misc/cpp", "-I./deps/rlImGui", "-I./deps/rlImGui/extras", "-I./deps/ImGuiColorTextEdit", "-g"

#define DRACON_LIBS                                                                                                         \
    "-Wl,--start-group", "-llua", "-lraylib", "-lplatform_folders", "-lopengl32", "-luser32", "-lkernel32", "-lgdi32",      \
      "-lwinmm", "-lole32", "-luuid", "-lonecore", "-Wl,--end-group"

bool build_dracon_stage1(Nob_Cmd * cmd) {
    bool         result = true;
    const size_t len    = sizeof(dracon_sources) / sizeof(char *);
    for (size_t i = 0; i < len; i++) {
        if (nob_needs_rebuild1(dracon_objs[i], dracon_sources[i])) {
            nob_cc(cmd);
            nob_cc_flags(cmd);
            nob_cmd_append(cmd, DRACON_FLAGS, "-c");
            nob_cc_output(cmd, dracon_objs[i]);
            nob_cc_inputs(cmd, dracon_sources[i], DRACON_LIBS);
            if (!nob_cmd_run(cmd)) nob_return_defer(false);
        }
    }

    if (nob_needs_rebuild((BUILD_FOLDER "dracon"), dracon_objs, sizeof(dracon_objs) / sizeof(dracon_objs[0]))) {
        nob_cc(cmd);
        nob_cc_flags(cmd);
        nob_cmd_append(cmd, DRACON_FLAGS, "-g");
        nob_cc_output(cmd, BUILD_FOLDER "dracon");
        nob_cc_inputs(cmd,
                      (SRC_FOLDER "main.o"),
                      (SRC_FOLDER "config.o"),
                      (SRC_FOLDER "uid.o"),
                      (SRC_FOLDER "cart.o"),
                      (SRC_FOLDER "console.o"),
                      (SRC_FOLDER "console/font.o"),
                      (SRC_FOLDER "console/pal.o"),
                      (SRC_FOLDER "console/sprite.o"),
                      (SRC_FOLDER "console/gfx.o"),
                      (SRC_FOLDER "console/fb.o"),
                      (SRC_FOLDER "console/gamepad.o"),
                      (SRC_FOLDER "console/keyboard.o"),
                      (SRC_FOLDER "editor/manager.o"),
                      (SRC_FOLDER "editor/code_editor.o"),
                      "deps/imgui/imgui.o",
                      "deps/imgui/imgui_demo.o",
                      "deps/imgui/imgui_draw.o",
                      "deps/imgui/imgui_tables.o",
                      "deps/imgui/imgui_widgets.o",
                      "deps/imgui/misc/cpp/imgui_stdlib.o",
                      "deps/rlImGui/rlImGui.o",
                      "deps/ImGuiColorTextEdit/TextEditor.o",
                      DRACON_LIBS);
        if (!nob_cmd_run(cmd)) nob_return_defer(false);
    }

defer:
    return result;
}

bool build_imgui(Nob_Cmd * cmd) {
    bool         result = true;
    const size_t len    = sizeof(imgui_sources) / sizeof(char *);
    for (size_t i = 0; i < len; i++) {
        if (nob_needs_rebuild1(imgui_objs[i], imgui_sources[i])) {
            nob_cc(cmd);
            nob_cc_flags(cmd);
            nob_cmd_append(cmd, IMGUI_FLAGS, "-c");
            nob_cc_output(cmd, imgui_objs[i]);
            nob_cc_inputs(cmd, imgui_sources[i]);
            if (!nob_cmd_run(cmd)) nob_return_defer(false);
        }
    }

defer:
    return result;
}

bool build_rlimgui(Nob_Cmd * cmd) {
    bool         result = true;
    const size_t len    = sizeof(rlimgui_sources) / sizeof(char *);
    for (size_t i = 0; i < len; i++) {
        if (nob_needs_rebuild1(rlimgui_objs[i], rlimgui_sources[i])) {
            nob_cc(cmd);
            nob_cc_flags(cmd);
            nob_cmd_append(cmd, RLIMGUI_FLAGS, "-c");
            nob_cc_output(cmd, rlimgui_objs[i]);
            nob_cc_inputs(cmd, rlimgui_sources[i]);
            if (!nob_cmd_run(cmd)) nob_return_defer(false);
        }
    }

defer:
    return result;
}

bool build_imgui_ctxte(Nob_Cmd * cmd) {
    bool         result = true;
    const size_t len    = sizeof(imgui_color_text_edit_sources) / sizeof(char *);
    for (size_t i = 0; i < len; i++) {
        if (nob_needs_rebuild1(imgui_color_text_edit_objs[i], imgui_color_text_edit_sources[i])) {
            nob_cc(cmd);
            nob_cc_flags(cmd);
            nob_cmd_append(cmd, RLIMGUI_FLAGS, "-c");
            nob_cc_output(cmd, imgui_color_text_edit_objs[i]);
            nob_cc_inputs(cmd, imgui_color_text_edit_sources[i]);
            if (!nob_cmd_run(cmd)) nob_return_defer(false);
        }
    }

defer:
    return result;
}

bool build_lua(Nob_Cmd * cmd) {
    bool         result    = true;
    const char * sources[] = {"lapi.c",     "lcode.c",    "lctype.c",   "ldebug.c",  "ldo.c",      "ldump.c",   "lfunc.c",
                              "lgc.c",      "llex.c",     "lmem.c",     "lobject.c", "lopcodes.c", "lparser.c", "lstate.c",
                              "lstring.c",  "ltable.c",   "ltm.c",      "lundump.c", "lvm.c",      "lzio.c",    "lauxlib.c",
                              "lbaselib.c", "lcorolib.c", "ldblib.c",   "liolib.c",  "lmathlib.c", "loadlib.c", "loslib.c",
                              "lstrlib.c",  "ltablib.c",  "lutf8lib.c", "linit.c"};
    if (!nob_set_current_dir("deps/lua")) nob_return_defer(false);

    if (!nob_needs_rebuild("liblua.a", sources, sizeof(sources) / sizeof(sources[0]))) {
        nob_set_current_dir("../..");
        return true;
    }

    nob_cmd_append(cmd,
                   "gcc",
                   "-c",
                   "lapi.c",
                   "lcode.c",
                   "lctype.c",
                   "ldebug.c",
                   "ldo.c",
                   "ldump.c",
                   "lfunc.c",
                   "lgc.c",
                   "llex.c",
                   "lmem.c",
                   "lobject.c",
                   "lopcodes.c",
                   "lparser.c",
                   "lstate.c",
                   "lstring.c",
                   "ltable.c",
                   "ltm.c",
                   "lundump.c",
                   "lvm.c",
                   "lzio.c",
                   "lauxlib.c",
                   "lbaselib.c",
                   "lcorolib.c",
                   "ldblib.c",
                   "liolib.c",
                   "lmathlib.c",
                   "loadlib.c",
                   "loslib.c",
                   "lstrlib.c",
                   "ltablib.c",
                   "lutf8lib.c",
                   "linit.c");

    if (!nob_cmd_run(cmd)) nob_return_defer(false);

    nob_cmd_append(cmd,
                   "ar",
                   "rcs",
                   "liblua.a",
                   "lapi.o",
                   "lcode.o",
                   "lctype.o",
                   "ldebug.o",
                   "ldo.o",
                   "ldump.o",
                   "lfunc.o",
                   "lgc.o",
                   "llex.o",
                   "lmem.o",
                   "lobject.o",
                   "lopcodes.o",
                   "lparser.o",
                   "lstate.o",
                   "lstring.o",
                   "ltable.o",
                   "ltm.o",
                   "lundump.o",
                   "lvm.o",
                   "lzio.o",
                   "lauxlib.o",
                   "lbaselib.o",
                   "lcorolib.o",
                   "ldblib.o",
                   "liolib.o",
                   "lmathlib.o",
                   "loadlib.o",
                   "loslib.o",
                   "lstrlib.o",
                   "ltablib.o",
                   "lutf8lib.o",
                   "linit.o");

    if (!nob_cmd_run(cmd)) nob_return_defer(false);

defer:
    nob_set_current_dir("../..");
    return result;
}

bool build_raylib(Nob_Cmd * cmd) {
    bool result = true;
    if (!nob_set_current_dir("deps/raylib")) nob_return_defer(false);

    if (!nob_mkdir_if_not_exists("build")) nob_return_defer(false);

    nob_cmd_append(cmd,
                   "cmake",
                   "-B",
                   "build",
                   "-DCUSTOMIZE_BUILD=ON",
                   "-DPLATFORM=Desktop",
                   "-DOPENGL_VERSION=3.3",
                   "-DINCLUDE_EVERYTHING=ON",
                   "-DSUPPORT_SAVE_BMP=ON");

    if (!nob_cmd_run(cmd)) nob_return_defer(false);

    nob_cmd_append(cmd, "cmake", "--build", "build");

    if (!nob_cmd_run(cmd)) nob_return_defer(false);

defer:
    nob_set_current_dir("../..");
    return result;
}

bool build_PlatformFolders(Nob_Cmd * cmd) {
    bool result = true;
    if (!nob_set_current_dir("deps/PlatformFolders")) nob_return_defer(false);

    if (!nob_mkdir_if_not_exists("build")) nob_return_defer(false);

    nob_cmd_append(cmd, "cmake", "-B", "build", "-DBUILD_TESTING=OFF");

    if (!nob_cmd_run(cmd)) nob_return_defer(false);

    nob_cmd_append(cmd, "cmake", "--build", "build");

    if (!nob_cmd_run(cmd)) nob_return_defer(false);

defer:
    nob_set_current_dir("../..");
    return result;
}

int main(int argc, char ** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};

    if (!build_lua(&cmd)) return 1;
    if (!build_raylib(&cmd)) return 1;
    if (!build_PlatformFolders(&cmd)) return 1;
    if (!build_imgui(&cmd)) return 1;
    if (!build_rlimgui(&cmd)) return 1;
    if (!build_imgui_ctxte(&cmd)) return 1;
    if (!build_dracon_stage1(&cmd)) return 1;

    return 0;
}
