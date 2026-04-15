#include "MoveComponent.h"
#include "GameObject.h"
#include "../imgui/imgui.h"

void MoveComponent::BeginPlay()  {
    // 初期化が必要ならここに書く
}

// Update は 1 つにまとめます
void MoveComponent::Update(float dt)  {
    extern GLFWwindow* window;

    // owner が存在するかチェックしてからアクセス（安全のため）
    if (!owner) return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) owner->position.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) owner->position.z += speed;

    // dt（デルタタイム）を使った移動
    owner->position.x += speed * dt;
}

// OnGui も 1 つにまとめます
void MoveComponent::OnGui()  {
    if (ImGui::CollapsingHeader("Move Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Speed", &speed, 0.001f, 0.0f, 1.0f);
    }
}