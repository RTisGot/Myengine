#include "GameObject.h"
void GameObject::Update(float dt, GLFWwindow* window) {
    for (auto& comp : components) {
        comp->Update(dt, window); // ここで各コンポーネントのUpdateを実行
    }
}