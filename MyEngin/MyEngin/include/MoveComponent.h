#pragma once
#include "Core/Component.h"
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"

// 前方宣言: ヘッダー間の循環参照を防止し、コンパイル時間を短縮
struct GameObject;

/**
 * @class MoveComponent
 * @brief オブジェクトに移動能力を付与するコンポーネント
 * @details キーボード入力や自動処理によって親GameObjectのTransformを操作します。
 */
class MoveComponent : public Component {
public:
    // 移動速度 (m/s 単位を想定)
    float speed = 0.05f;

    /**
     * @brief コンポーネント実行開始時に一度だけ呼ばれる初期化処理
     */
    void BeginPlay() override;

    /**
     * @brief 毎フレーム実行される更新処理
     * @param dt 前フレームからの経過時間 (DeltaTime)
     * @param window 入力検知用のGLFWウィンドウハンドル
     */
    void Update(float dt, GLFWwindow* window) override;

    /**
     * @brief エディタのInspectorウィンドウ等に表示するGUI描画処理
     * @details ImGuiを用いて、実行中にspeed等のパラメータをリアルタイム編集可能にします。
     */
    void OnGui() override;
};