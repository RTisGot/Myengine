#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
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

