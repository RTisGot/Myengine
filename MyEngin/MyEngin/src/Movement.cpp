#include "MoveComponent.h"
#include "GameObject.h"
#include "PlayerComponent.h"
#include "../imgui/imgui.h"

void MoveComponent::BeginPlay()  {
}


void MoveComponent::Update(float dt, GLFWwindow* window)  {
    // owner が存在するかチェック
    if (!owner) return;

    if (!window)return;
    if (!owner->HasComponent<PlayerComponent>()) return; // Playerコンポーネント付きのみ移動

    const float moveStep = speed * dt * 60.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) owner->position.z -= moveStep;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) owner->position.z += moveStep;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) owner->position.x -= moveStep;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) owner->position.x += moveStep;
}

void MoveComponent::OnGui()  {
    if (ImGui::CollapsingHeader("Move Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Speed", &speed, 0.001f, 0.0f, 1.0f);
    }
}