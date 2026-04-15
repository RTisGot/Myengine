#pragma once
#include "Core/Component.h"
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h" // ImGuiを使うために必要

// GameObject.h をインクルードするのではなく、
// 「GameObjectという名前の構造体が後で出てくるよ」とだけ教える（前方宣言）
struct GameObject;

class MoveComponent : public Component {
public:
    float speed = 0.05f;

    // BeginPlay は中身がなければ書かなくても親のものが呼ばれます
    void BeginPlay() override;

    // Update は 1 つにまとめます
    void Update(float dt) override;

    // OnGui も 1 つにまとめます
    void OnGui() override;
};