#include "editor/code_editor.hpp"
#include <string>
#include <utility>
#include <vector>
#include "TextEditor.h"
#include "cart.hpp"
#include "console.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "rlImGui.h"

namespace dracon::editor {

    static const std::vector<std::pair<std::string, std::string>> luaIdents = {
      {"print", "print(msg)"},
      {"exit", "exit(code)"},
      {"dofile", "dofile(path)"},
      {"loadfile", "loadfile(path)"},
      {"pix", "pix(x, y, col) | pix(x, y) -> integer"},
      {"cls", "cls(col)"},
      {"line", "line(x1, y1, x2, y2, col)"},
      {"rect", "rect(x, y, w, h, col)"},
      {"rectf", "rectf(x, y, w, h, col)"},
      {"circ", "circ(x, y, r, col)"},
      {"circf", "circf(x, y, r, col)"},
      {"putch", "putch(x, y, ch, col)"},
      {"puts", "puts(x, y, msg, col)"},
      {"register_on_frame", "register_on_frame(func)"},
      {"unregister_on_frame", "unregister_on_frame(func)"},
      {"tri", "tri(x1, y1, x2, y2, x3, y3, col)"},
      {"trif", "trif(x1, y1, x2, y2, x3, y3, col)"},
      {"spr", "spr(x, y, spr, flip, rot) | spr(x, y, spr, flip, rot, key)"},
      {"btn", "btn(button) -> bool"},
      {"btnp", "btnp(button) -> bool"},
      {"key", "key(k) -> bool"},
      {"keyp", "keyp(k) -> bool"},
      {"time", "time() -> double"},
      {"tstamp", "tstamp() -> integer"},
    };

    auto codeEditorInitialize(TextEditor & txt) -> void {
        auto lang = TextEditor::LanguageDefinition::Lua();

        for (auto const & p : luaIdents) {
            TextEditor::Identifier id;
            id.mDeclaration = p.second;
            lang.mIdentifiers.insert_or_assign(p.first, id);
        }

        txt.SetLanguageDefinition(lang);
        txt.SetColorizerEnable(true);

        txt.SetPalette(TextEditor::GetDarkPalette());
    }

    auto CodeEditor(Console & console, bool & show) -> void {
        static TextEditor editor{};
        static bool       initialized = false;

        if (!initialized) {
            codeEditorInitialize(editor);
            initialized = true;
        }

        if (show) {
            auto cart = console.GetCart();
            editor.SetText(cart->Code);

            editor.Render("##code-editor", ImVec2(-1, -1));
            // ImGui::InputTextMultiline("##code-editor", &(cart->Code), ImVec2(-1, -1), ImGuiInputTextFlags_AllowTabInput);

            cart->Code = editor.GetText();
        }
    }

} // namespace dracon::editor
