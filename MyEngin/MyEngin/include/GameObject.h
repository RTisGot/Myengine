#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <set>
#include <string>
#include <vector>

struct GameObject {
    std::string name;
    glm::vec3 position;
    glm::vec3 rotation; // X, Y, Z軸それぞれの回転を管理できるようにvec3に
    glm::vec3 scale;    // 縦横高さバラバラに変えられるようにvec3に
    std::set<std::string> tags; // タグを文字列で保持(std::set 二重を防ぐ)
    float color[3];

    //------tagに対しての関数---
    void addTag(const std::string& t) { tags.insert(t); }                  //Tad追加
	void removeTag(const std::string& t) { tags.erase(t); }                //Tag削除
	bool hasTag(const std::string& t) const { return tags.count(t) > 0; }  //Tagの有無確認

    GameObject(std::string n, glm::vec3 p)
        : name(n), position(p), rotation(0.0f), scale(1.0f) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    }

    // 行列を計算して返す
    glm::mat4 getModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale);
        return model;//ローカル変数を返す
    }
};
inline glm::vec3 calculateRayFromPixel(double xpos, double ypos, const glm::mat4& projection, const glm::mat4& view) {
    // 1. 現在の Viewport 設定 (x, y, width, height) を取得
    // これにより、glViewport(350, 0, width-350, height) の値が自動的に取れる
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    float vpX = (float)viewport[0];      // 350
    float vpY = (float)viewport[1];      // 0
    float vpWidth = (float)viewport[2];  // width - 350
    float vpHeight = (float)viewport[3]; // height

    // 2. マウス座標をビューポート相対に変換し、NDC (-1.0 ～ 1.0) にする
    // xpos(マウス全体の座標) から vpX(350) を引いて計算
    float x = (2.0f * ((float)xpos - vpX)) / vpWidth - 1.0f;
    float y = 1.0f - (2.0f * ((float)ypos - vpY)) / vpHeight;

    // 3. 逆行列を使ってワールド空間の方向に変換
    glm::mat4 invVP = glm::inverse(projection * view);
    glm::vec4 screenPos = glm::vec4(x, y, 1.0f, 1.0f);
    glm::vec4 worldPos = invVP * screenPos;

    glm::vec3 cameraPos = glm::vec3(glm::inverse(view)[3]);

    return glm::normalize(glm::vec3(worldPos) / worldPos.w - cameraPos);
}

bool rayIntersectsSphere(glm::vec3 origin, glm::vec3 dir, glm::vec3 center, float radius, float& distance) {
    glm::vec3 L = center - origin;
    float tca = glm::dot(L, dir);
    if (tca < 0) return false;
    float d2 = glm::dot(L, L) - tca * tca;
    if (d2 > radius * radius) return false;
    float thc = sqrt(radius * radius - d2);
    distance = tca - thc;
    return true;
}