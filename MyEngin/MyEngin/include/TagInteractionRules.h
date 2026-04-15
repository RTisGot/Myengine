#pragma once
#include "Tagsystem.h"

// タグの組み合わせとルールを定義する構造体

class TagInteractionRules {
public:
	static void RegistarAllRules() {
		//---移動-----
		InteractionRule moveRule;
		moveRule.tagA = "Player";
		moveRule.tagB = "Movable";
		moveRule.action = [](GameObject& a, GameObject& b) {
			float speed = 0.05f;
			extern GLFWwindow* window;
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) a.position.z -= speed;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) a.position.z += speed;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) a.position.x -= speed;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) a.position.x += speed;
			};
	}
};
