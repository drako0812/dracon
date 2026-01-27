#include "editor/manager.hpp"
#include "cart.hpp"
#include "console.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "rlImGui.h"

namespace dracon::editor {

    auto CartManager(Console & console, bool & show) -> void {
        if (show) {
            auto cart = console.GetCart();

            auto cart_id = cart->ID.ToString64();
            ImGui::LabelText("ID", "%s", cart_id.c_str());
            ImGui::SameLine();
            if (ImGui::Button("New ID")) { cart->NewID(); }

            ImGui::InputText("Short Name", cart->ShortName, 16);
            ImGui::InputText("Long Name", cart->LongName, 64);
            ImGui::InputText("Author  #1", cart->Authors[0].Name, 32);
            ImGui::InputText("Author  #2", cart->Authors[1].Name, 32);
            ImGui::InputText("Author  #3", cart->Authors[2].Name, 32);
            ImGui::InputText("Author  #4", cart->Authors[3].Name, 32);
            ImGui::InputText("Author  #5", cart->Authors[4].Name, 32);
            ImGui::InputText("Author  #6", cart->Authors[5].Name, 32);
            ImGui::InputText("Author  #7", cart->Authors[6].Name, 32);
            ImGui::InputText("Author  #8", cart->Authors[7].Name, 32);
            ImGui::InputText("Author  #9", cart->Authors[8].Name, 32);
            ImGui::InputText("Author #10", cart->Authors[9].Name, 32);
            ImGui::InputText("License", cart->License, 32);
            ImGui::InputText("Version", cart->Version, 32);

            if (ImGui::Button("Reset Storage")) {
                for (std::size_t i = 0; i < 256; i++) { cart->Storage[i] = 0; }
            }

            if (ImGui::Button("Reset Code")) { cart->Code = DefaultCode; }

            ImGui::PushStyleColor(ImGuiCol_Text, 0xFF8080FF);
            if (ImGui::Button("Reset Cart")) {
                auto result = console.LoadNewCart();
                if (result.has_value()) { std::cerr << result.value() << "\n"; }
            }
            ImGui::PopStyleColor();
        }
    }

} // namespace dracon::editor
