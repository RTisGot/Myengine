#include <GL/glew.h>
#include <vector> 
#include "Mesh.h"
#include <string>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shader.h"


void Mesh::setupMesh() {
    // 1. バッファの生成
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 2. VBOに頂点データをコピー
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 3. EBOにインデックスデータをコピー
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 4. 頂点属性の設定（Shader側でのlocation=0に対応）
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glBindVertexArray(0); // 終わったらバインドを解除
}