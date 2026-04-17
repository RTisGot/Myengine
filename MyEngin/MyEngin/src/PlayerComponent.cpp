#include "PlayerComponent.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "Core/EditorUI.h"

void PlayerComponent::OnGui() {
    if (!ImGui::CollapsingHeader("Player Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    const char* viewItems[] = { "First Person", "Third Person" };
    int current = (viewMode == ViewMode::FirstPerson) ? 0 : 1;

    const bool hasMoveComponent = owner && owner->HasComponent<MoveComponent>();
    if (!hasMoveComponent) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "MoveComponent is required.");
        EditorUI::PushLog("PlayerComponent: MoveComponent is missing, cannot select view mode.", true);
    }

    ImGui::BeginDisabled(!hasMoveComponent);
    if (ImGui::Combo("View Mode", &current, viewItems, IM_ARRAYSIZE(viewItems))) {
        viewMode = (current == 0) ? ViewMode::FirstPerson : ViewMode::ThirdPerson;
    }
    ImGui::EndDisabled();
}
