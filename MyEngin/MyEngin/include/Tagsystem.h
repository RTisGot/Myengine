#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <functional> // std::function のために必要
#include "GameObject.h"

/**
 * @struct InteractionRule
 * @brief 特定のタグを持つオブジェクト同士が接触した際の振る舞いを定義する構造体
 */
struct InteractionRule {
    std::string tagA; // 対象タグA
    std::string tagB; // 対象タグB
    std::function<void(GameObject&, GameObject&)> action; // 条件合致時に実行されるコールバック関数
};

/**
 * @class Tagsystem
 * @brief タグベースのインタラクション（相互作用）を統合管理するシステム
 * @details オブジェクト間の距離を監視し、登録されたタグの組み合わせに応じた処理を自動的に実行します。
 */
class Tagsystem {
public:
    // ゲーム制作者がエディタ等で定義した、有効なインタラクションルールの一覧
    static std::vector<InteractionRule> rules;

    /**
     * @brief 全オブジェクト間の近接判定とルール適用を行う更新処理
     * @param objects シーン内に存在する全オブジェクトのリスト
     * @details 全対全の距離チェックを行い、一定距離（1.0ユニット）以内のペアに対してルールを照合します。
     */
    static void Update(std::vector<GameObject>& objects) {
        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                // 近接判定：中心点間の距離が 1.0f 未満なら接触とみなす
                if (glm::distance(objects[i].position, objects[j].position) < 1.0f) {
                    ApplyRules(objects[i], objects[j]);
                }
            }
        }
    }

    /**
     * @brief タグインタラクションを編集するための専用GUIウィンドウ（ImGui）
     * @details 実行中に新しいルールの追加や、既存ルールの削除を動的に行えるツールを提供します。
     */
    static void ShowImGuiWindow() {
        ImGui::Begin("Tag Interaction Editor");

        static char tagABuf[64] = "";
        static char tagBBuf[64] = "";
        static int selectedAction = 0;
        const char* actionNames[] = { "Destroy", "Apply Fire Status" }; // プリセットアクション

        // --- ルール作成インターフェース ---
        ImGui::InputText("Source Tag (A)", tagABuf, 64);
        ImGui::InputText("Target Tag (B)", tagBBuf, 64);
        ImGui::Combo("Action to Execute", &selectedAction, actionNames, IM_ARRAYSIZE(actionNames));

        if (ImGui::Button("Add Interaction Rule")) {
            InteractionRule newRule;
            newRule.tagA = tagABuf;
            newRule.tagB = tagBBuf;

            // 選択されたタイプに応じて挙動（ラムダ式）をバインド
            if (selectedAction == 0) {
                newRule.action = [](GameObject& a, GameObject& b) {
                    std::cout << "[Tagsystem] オブジェクトを破棄対象としてマークしました。" << std::endl;
                    };
            }
            else if (selectedAction == 1) {
                newRule.action = [](GameObject& a, GameObject& b) {
                    a.addTag("OnFire");
                    b.addTag("OnFire");
                    std::cout << "[Tagsystem] 状態異常 'OnFire' を付与しました。" << std::endl;
                    };
            }
            rules.push_back(newRule);
        }

        ImGui::Separator();

        // --- ルール管理リスト ---
        ImGui::Text("Active Rules List:");
        for (int i = 0; i < (int)rules.size(); i++) {
            ImGui::BulletText("%s + %s -> [Defined Action]", rules[i].tagA.c_str(), rules[i].tagB.c_str());
            ImGui::SameLine();
            if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
                rules.erase(rules.begin() + i);
            }
        }

        ImGui::End();
    }

private:
    /**
     * @brief 接触した2つのオブジェクトに対し、合致するタグのルールがあるか照合する
     */
    static void ApplyRules(GameObject& a, GameObject& b) {
        for (auto& rule : rules) {
            // AとBのタグが、ルール(tagA, tagB)の組み合わせを満たすかチェック（順不同）
            if ((a.hasTag(rule.tagA) && b.hasTag(rule.tagB)) ||
                (a.hasTag(rule.tagB) && b.hasTag(rule.tagA))) {
                ExecuteAction(rule, a, b);
            }
        }
    }

    /**
     * @brief 確定したルールに基づき、アクションを実行する内部関数
     */
    static void ExecuteAction(const InteractionRule& rule, GameObject& a, GameObject& b) {
        if (rule.action) {
            rule.action(a, b);
        }
    }
};

// 静的メンバ変数の実体定義
std::vector<InteractionRule> Tagsystem::rules;