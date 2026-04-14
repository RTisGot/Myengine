#pragma once


// タグシステムを管理するクラス
#include <vector>
#include <string>
#include <iostream>
#include "GameObject.h"

// タグの組み合わせとルールを定義する構造体
struct InteractionRule {
    std::string tagA;
    std::string tagB;
    std::function<void(GameObject&, GameObject&)> action; // ルール適用時の処理
};


class Tagsystem {
public:

    // ゲーム制作者がエディタで追加したルールを保持するリスト
    static std::vector<InteractionRule> rules;

	static void Update(std::vector<GameObject>& objects) {
        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                // 距離判定（1.0ユニット以内なら接触）
                if (glm::distance(objects[i].position, objects[j].position) < 1.0f) {
                    ApplyRules(objects[i], objects[j]);
                }
            }
        }
    }
private:
	// ルールを適用する関数
    static void ApplyRules(GameObject& a, GameObject& b) {
        for (auto& rule : rules) {
            // AとBが、ルールに設定されたタグをそれぞれ持っているかチェック
            if ((a.hasTag(rule.tagA) && b.hasTag(rule.tagB)) ||
                (a.hasTag(rule.tagB) && b.hasTag(rule.tagA))) {

                ExecuteAction(rule, a, b);
            }
        }
    }
    static void ExecuteAction(const InteractionRule& rule, GameObject& a, GameObject& b) {
        if (rule.action) {
            rule.action(a, b);
        }
    }
};

std::vector<InteractionRule> Tagsystem::rules;
