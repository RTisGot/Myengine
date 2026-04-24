#include "MoveComponent.h"
#include "GameObject.h"
#include "PlayerComponent.h"
#include "../imgui/imgui.h"

/**
 * @brief コンポーネント開始時の初期化
 */
void MoveComponent::BeginPlay() {
    // 必要に応じて初期化ロジックをここに記述
}

/**
 * @brief 毎フレームの移動ロジック更新
 * @param dt 前フレームからの経過時間 (DeltaTime)
 * @param window 入力を取得するためのGLFWウィンドウポインタ
 */
void MoveComponent::Update(float dt, GLFWwindow* window) {
    // 1. 所有者（GameObject）の有効性チェック
    if (!owner) return;

    // 2. ウィンドウハンドルの有効性チェック
    if (!window) return;

    // 3. 依存関係のチェック: PlayerComponentを持つオブジェクトのみを操作対象とする
    // ※これにより、NPC等にこのコンポーネントがついていても勝手に動かないように制御
    if (!owner->HasComponent<PlayerComponent>()) return;

    // 4. フレームレートに依存しない移動量の計算
    // dt * 60.0f により、60FPSを基準とした移動速度を維持する
    const float moveStep = speed * dt * 60.0f;

    // 5. キーボード入力に基づいた座標更新（XZ平面の移動）
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) owner->position.z -= moveStep;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) owner->position.z += moveStep;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) owner->position.x -= moveStep;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) owner->position.x += moveStep;
}

/**
 * @brief インスペクター上でのパラメータ編集UI
 */
void MoveComponent::OnGui() {
    // CollapsingHeaderを使用して、コンポーネントごとにUIをグループ化
    if (ImGui::CollapsingHeader("Move Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        // ドラッグ操作で移動速度を直感的に調整可能にする
        ImGui::DragFloat("Movement Speed", &speed, 0.001f, 0.0f, 1.0f);
    }
}