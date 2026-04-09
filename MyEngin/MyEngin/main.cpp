#define GLEW_STATIC
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Mesh.h"
#include "GameObject.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include<vector>
#include <iostream>

glm::mat4 modelMatrix = (1.f);         //単位行列で初期化

const GLint WIDTH = 800, HEIGHT = 600;//ウィンドウの幅と高さ

std::vector<Vertex> vertices = {
	// 前面
	{{-0.5f, -0.5f,  0.5f}}, {{ 0.5f, -0.5f,  0.5f}}, {{ 0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f,  0.5f}},
	// 背面
	{{-0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f, -0.5f}}, {{ 0.5f,  0.5f, -0.5f}}, {{-0.5f,  0.5f, -0.5f}}
};

// インデックス
std::vector<unsigned int> indices = {
	0, 1, 2, 2, 3, 0, // 前
	1, 5, 6, 6, 2, 1, // 右
	7, 6, 5, 5, 4, 7, // 後
	4, 0, 3, 3, 7, 4, // 左
	3, 2, 6, 6, 7, 3, // 上
	4, 5, 1, 1, 0, 4  // 下
};

std::vector<GameObject> worldObjects;

int main()
{
	//------3Dの位置管理----
	static float position[3] = { 0.0f, 0.0f, 0.0f }; // X, Y, Z の位置
	static float rotation = 0.0f;                    // 回転角度
	static float scale = 1.0f;                       // 大きさ
	//windowの初期化
	if (!glfwInit())
	{
		std::cout << "GLFW is faild Initialize!" << "/n";
		glfwTerminate();
		return 1;
	}

	//Setup Window Properties 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	

	// モニター情報の取得
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	// フルスクリーンウィンドウの作成
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "My Engine", monitor, NULL);

	if (!window)
	{
		std::cout << "Failed to create window!" << "/n";
		glfwTerminate();
		return 1;
	}

	//Get frame buffer size
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);

	//Set context
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	//glewの初期化
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW!" << "\n";
		
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	//深度テストを有効にする
	glEnable(GL_DEPTH_TEST);

	// 1. ImGui コンテキストの作成
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// 2. バックエンドの初期化
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// 3. スタイル見た目の設定
	ImGui::StyleColorsDark(); 
	// 自作Shaderクラスの作成 
	Shader ourShader("shader.vert", "shader.frag");
	//自作Meshクラスの作成
	Mesh myMesh(vertices, indices);
	


	static float yaw = -90.0f; // 左右の向き
	static float pitch = 0.0f; // 上下の向き
	static double lastX = 400, lastY = 300;
	static bool firstMouse = true;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//マウスの移動量だけで

	
	bool isDragging = false;
	glm::vec3 dragOffset; // クリックした地点と物体中心のズレを保持
	static int selected = -1;
	//Main loop
	while (!glfwWindowShouldClose(window))
	{
		// 1. 物理的なピクセルサイズ（フレームバッファサイズ）を取得
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

		// 2. ビューポートをそのサイズに合わせる
		glViewport(0, 0, display_w, display_h);

		// 3. アスペクト比を計算して Projection を作り直す
		float aspect = (float)display_w / (float)display_h;
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
		//Get and handle user inputs
		glfwPollEvents();

		// --- ImGui の新しいフレームを開始 ---
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Transform Editor");
		if (selected >= 0 && selected < (int)worldObjects.size()) {
			// 選択されている物体のデータを直接参照する
			GameObject& obj = worldObjects[selected];

			ImGui::Text("Selected: %s", obj.name.c_str());
			ImGui::Separator();

			// Position: glm::vec3 のアドレスを渡す
			ImGui::DragFloat3("Position", &obj.position.x, 0.1f);

			// Rotation: 角度を直接いじる
			ImGui::DragFloat3("Rotation", &obj.rotation.x, 1.0f);

			// Scale: 大きさをいじる
			ImGui::DragFloat3("Scale", &obj.scale.x, 0.1f);
		}
		else {
			ImGui::Text("No object selected.");
			ImGui::Text("Press 'A' to spawn and click to select.");
		}

		ImGui::End();
		
		//--- カメラの位置と向きの管理 ---
		static float camPos[3] = { 0.0f, 0.0f, 3.0f }; // 少し手前に配置

		static int selected = -1; // リスト番号
		// --- マウス操作でカメラの向きを変更 ---
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			// 右クリック中だけマウスカーソルを非表示にして操作
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			if (firstMouse) {
				lastX = xpos; lastY = ypos;
				firstMouse = false;
			}

			float xoffset = (float)(xpos - lastX);
			float yoffset = (float)(lastY - ypos); // Y軸は逆
			lastX = xpos; lastY = ypos;

			float sensitivity = 0.1f;
			yaw += xoffset * sensitivity;
			pitch += yoffset * sensitivity;

			// 真上・真下を向いた時に反転しないよう制限
			if (pitch > 89.0f) pitch = 89.0f;
			if (pitch < -89.0f) pitch = -89.0f;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstMouse = true;
		}
		

	//--------- 行列の計算 (GLMを使用) ---
		glm::mat4 model = glm::mat4(1.0f); // 単位行列で初期化

		//-------カメラの位置を設定	
		
		
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glm::vec3 cameraFront = glm::normalize(front);

		glm::mat4 view = glm::lookAt(
			glm::vec3(camPos[0], camPos[1], camPos[2]),
			glm::vec3(camPos[0], camPos[1], camPos[2]) + cameraFront,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		//カメラの操作
		ImGui::Begin("Camera Editor");
		ImGui::SliderFloat3("Camera Position", camPos, -10.0f, 10.0f);
		ImGui::End();

		// 1. 左ボタンが押された瞬間（選択）
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if (!isDragging && selected != -1) {
				isDragging = true;
			}
		}
		else {
			isDragging = false; // 離したらドラッグ終了
		}

		// 2. ドラッグ中の処理
		if (isDragging && selected != -1) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			// カメラからの距離を保ったまま移動させる（簡易UE5スタイル）
			glm::vec3 camVec = glm::vec3(camPos[0], camPos[1], camPos[2]);
			float dist = glm::distance(camVec, worldObjects[selected].position);

			// 現在のマウス位置から新しいレイを計算
			glm::vec3 rayDir = calculateRayFromPixel(xpos, ypos, window, projection, view);

			// 物体の位置を更新： カメラ位置 + (向き * 元の距離)
			worldObjects[selected].position = glm::vec3(camPos[0], camPos[1], camPos[2]) + (rayDir * dist);
		}
		if (selected >= 0 && selected < (int)worldObjects.size()) {
			float snapValue = 0.5f;
			worldObjects[selected].position.x = round(worldObjects[selected].position.x / snapValue) * snapValue;
			worldObjects[selected].position.y = round(worldObjects[selected].position.y / snapValue) * snapValue;
			worldObjects[selected].position.z = round(worldObjects[selected].position.z / snapValue) * snapValue;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			// カメラの前に新しいオブジェクトを追加
			worldObjects.push_back(GameObject("New Cube", cameraFront * 2.0f + glm::vec3(camPos[0], camPos[1], camPos[2])));
		}
		// 1. 移動
		model = glm::translate(model, glm::vec3(position[0], position[1], position[2]));
		// 2. 回転
		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		// 3. 拡大縮小
		model = glm::scale(model, glm::vec3(scale, scale, scale));

		//Viewport
		glViewport(0, 0, bufferWidth, bufferHeight);

		// --- 描画処理 ---
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 画面と深度バッファをクリア
		ourShader.use();



		// 行列をシェーダーへ転送-----
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// View
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// Projection
		unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	// --- UIの内容を定義する（ここがエディタの中身！） ---
		ImGui::Begin("Engine Editor"); // ウィンドウ開始
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		static float color[3] = { 1.0f, 0.5f, 0.2f };
		if (ImGui::ColorEdit3("Clear Color", color)) {
			// ここで色を変更する処理などを書ける
		}
		ImGui::End(); // ウィンドウ終了

		// 背景色（少し落ち着いた色にしてみましょう）
		glClearColor(color[0], color[1], color[2], 1.0f);// UIで決めた色を背景色にする
		glClear(GL_COLOR_BUFFER_BIT);

		// 1. シェーダーの使用
		ourShader.use();

		// UIで決めた色をシェーダーに送る
		int vertexColorLocation = glGetUniformLocation(ourShader.ID, "ourColor");
		glUseProgram(ourShader.ID);
		glUniform4f(vertexColorLocation, color[0], color[1], color[2], 1.0f);

		for (int i = 0; i < worldObjects.size(); i++) {
			glm::mat4 objModel = worldObjects[i].getModelMatrix();
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objModel));

			// 選択状態の判定をインデックスで行う
			if (i == selected) {
				glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); // 選択中は黄色
			}
			else {
				glUniform4f(vertexColorLocation, color[0], color[1], color[2], 1.0f);
			}
			myMesh.Draw(ourShader);
		}
		
	

		// --- ImGui を画面に反映させる ---
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);

		
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;

	return 0;
}

//プログラムオブジェクトを作成する
//  vsrc:バーテックシェーダーのソースプログラムの文字列
//  vsrc:フラグメントシェーダーのソースプログラムの文字列
GLuint createProgram(const char* vsrc, const char* fsrc)
{
	//空のオブジェクトを作成する
	const GLuint program(glCreateProgram());

	if (vsrc != NULL)
	{
		//バーテックスシェーダーのシェイダーオブジェクトを作成
		const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
		glShaderSource(vobj, 1, &vsrc, NULL);
		glCompileShader(vobj);

		//バーテックスシェーダーオブジェクトをプロジェクトオブジェクトに読み込む
		glAttachShader(program, vobj);
		glDeleteShader(vobj);
	}

	if (fsrc != NULL)
	{
		//フラグメントシェーダーのシェーダーオブジェクトを作成する
		const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(fobj, 1, &fsrc, NULL);
		glCompileShader(fobj);

		//フラグメントシェーダーのシェーダーオブジェクトをプログラムオブジェクトに組み込む
		glAttachShader(program, fobj);
		glDeleteShader(fobj);
	}

	//プログラムオブジェクトをリンクする
	glBindAttribLocation(program, 0, "position");
	glBindFragDataLocation(program, 0, "fragment");
	glLinkProgram(program);

	//作成したプログラムオブジェクトを返す
	return program;
}

