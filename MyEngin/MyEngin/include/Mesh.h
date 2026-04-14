#pragma once

#define GLEW_STATIC
#define EDITOR_HELPERS
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>
#include "Shader.h"

//頂点構造体
struct Vertex
{
	//3D
	glm::vec3 position;//位置
	glm::vec3 Normal;  //ライト用法線
	glm::vec2 TexCoords;//テクスチャ座標
};

static constexpr Vertex vertex[] = {
	{{-0.5f, -0.5f, 0.0f}},
	{{ 0.5f, -0.5f, 0.0f}},
	{{ 0.0f,  0.5f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f} }
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	GLuint VAO;
	GLuint VBO, EBO;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
		this->vertices = vertices;
		this->indices = indices;
		setupMesh(); // ここで VAO/VBO/EBO を生成する
	}

	//
	void Draw(Shader& shader) {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	
	
private: // クラスの内部だけ
	void setupMesh(); 

};


inline void DrawGrid(Shader& shader, Mesh& cubeMesh) {
	glm::mat4 model;
	int size = 20; // グリッドの範囲
	float lineThickness = 0.005f; // 線の細さ

	for (int i = -size; i <= size; i++) {
		// X方向の線
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0, -0.001f, i));
		model = glm::scale(model, glm::vec3(size, lineThickness, lineThickness));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.3f, 0.3f, 0.3f); // 暗い灰色
		cubeMesh.Draw(shader);

		// Z方向の線
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(i, -0.001f, 0));
		model = glm::scale(model, glm::vec3(lineThickness, lineThickness, size));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		cubeMesh.Draw(shader);
	}
}