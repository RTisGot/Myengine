#pragma once
#include "Tagsystem.h"

/**
 * @class TagInteractionRules
 * @brief オブジェクト間のタグに基づく相互作用（インタラクション）ルールを定義・管理するクラス
 * @details 特定のタグの組み合わせ（例：Player × Movable）に対して、どのような振る舞いを発生させるかを定義します。
 */
class TagInteractionRules {
public:
    /**
     * @brief システムで使用するすべての相互作用ルールをレジストリに登録する
     * @details アプリケーションの初期化時に呼び出し、ラムダ式を用いて各ルールの具体的な挙動（Action）を定義します。
     */
    static void RegistarAllRules() {

        // --- ルール定義：プレイヤーによる移動対象の操作 ---
        InteractionRule moveRule;
        moveRule.tagA = "Player";  // 操作主体
        moveRule.tagB = "Movable"; // 操作対象

        /**
         * @brief タグが一致した際に実行されるアクション
         * @param a 操作主体のGameObject
         * @param b 操作対象のGameObject
         */
        moveRule.action = [](GameObject& a, GameObject& b) {
            float speed = 0.05f;
            extern GLFWwindow* window; // 外部定義されたメインウィンドウハンドルを参照

            // キーボード入力に基づいた座標更新処理
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) a.position.z -= speed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) a.position.z += speed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) a.position.x -= speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) a.position.x += speed;
            };

        // TODO: TagSystem::AddRule(moveRule); 等の登録処理をここに記述
    }
};