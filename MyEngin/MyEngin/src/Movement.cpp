#include "MoveComponent.h"
#include "GameObject.h"
#include "../imgui/imgui.h"

void MoveComponent::BeginPlay()  {
}


void MoveComponent::Update(float dt, GLFWwindow* window)  {
    // owner が存在するかチェック
    if (!owner) return;
    
    if (!window)return;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) owner->position.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) owner->position.z += speed;

    // dt（デルタタイム）を使った移動
    owner->position.x += speed * dt;
}

void MoveComponent::OnGui()  {
    if (ImGui::CollapsingHeader("Move Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Speed", &speed, 0.001f, 0.0f, 1.0f);
    }
}