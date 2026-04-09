#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

struct GameObject {
    std::string name;
    glm::vec3 position;
    glm::vec3 rotation; // X, Y, Z軸それぞれの回転を管理できるようにvec3に
    glm::vec3 scale;    // 縦横高さバラバラに変えられるようにvec3に
    float color[3];

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
inline glm::vec3 calculateRayFromPixel(double xpos, double ypos, GLFWwindow* window, const glm::mat4& projection, const glm::mat4& view) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // 1. NDC座標 (-1.0 ～ 1.0) に変換
    float x = (2.0f * (float)xpos) / width - 1.0f;
    float y = 1.0f - (2.0f * (float)ypos) / height;

    // 2. 逆行列を使ってワールド空間の方向に変換
    glm::mat4 invVP = glm::inverse(projection * view);
    glm::vec4 screenPos = glm::vec4(x, y, 1.0f, 1.0f);
    glm::vec4 worldPos = invVP * screenPos;

    // 同次座標の w で割って正規化
    return glm::normalize(glm::vec3(worldPos) / worldPos.w - glm::vec3(glm::inverse(view)[3]));
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