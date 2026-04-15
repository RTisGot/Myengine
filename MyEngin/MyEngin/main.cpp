#define GLEW_STATIC
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Mesh.h"
#include "GameObject.h"
#include "Tagsystem.h"
#include "Core/EditorUI.h"
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

void processInput(GLFWwindow* window) {
	// ESCキーが押されたらウィンドウを閉じる
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// 例：W/A/S/D キーで何かを動かすロジックをここに書くこともできます
	// if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { ... }
}

// ゲームオブジェクトのリスト
std::vector<GameObject> worldObjects;
std::vector<GameObject> editorBackup; // 停止時に戻すためのバックアップ

int main()
{
	//モード管理
	bool isPlaying = false;

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
	float radius = 5.0f;
	static double lastX = 400, lastY = 300;
	static bool firstMouse = true;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);//マウスの移動量だけで

  //------------------
	bool isDragging = false; //objectdragのfalse付与
	bool lastLeftState = false;
	float dragDistance = 0.0f; // ドラッグ中の距離を保持する
	glm::vec3 dragOffset; // クリックした地点と物体中心のズレを保持
	static int selected = -1; //選択内容のindexを初期化
	static Vector3 spawnPos;  //メニューを開いたときの座標
	worldObjects.reserve(100);


	//--------------------------------メインループ---------------------//
	while (!glfwWindowShouldClose(window))
	{
		//フレームバッファサイズを取得
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

		// フレームバッファサイズ取得の直後
		int sidebarWidth = 300; // 左側のメニュー幅
		glViewport(sidebarWidth, 0, display_w - sidebarWidth, display_h);

		// アスペクト比もこの新しいサイズに合わせて計算し直す
		float aspect = (float)(display_w - sidebarWidth) / (float)display_h;
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
		//Get and handle user inputs
		glfwPollEvents();

		// --- ImGui の新しいフレームを開始 ---
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//入力の受付
		processInput(window);
		

		//PIEの状態による処理の分岐
		if (isPlaying) {
			Tagsystem::Update(worldObjects);
		}
		else {

		}
		//PIE UI表示
		EditorUI::ShowMainEditor(isPlaying, worldObjects, editorBackup);

		
		// 右側トランスフォーム詳細
		// ※今回は左側の下半分に配置する例
		ImGui::SetNextWindowPos(ImVec2(0, display_h * 0.5f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(300, display_h * 0.5f), ImGuiCond_Always);
		ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		{
			if (selected != -1) {
				GameObject& obj = worldObjects[selected];
				ShowDetails(obj);
				ImGui::Text("Name: %s", obj.name.c_str());
				ImGui::DragFloat3("Location", &obj.position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &obj.rotation.x, 1.0f);
				ImGui::DragFloat3("Scale", &obj.scale.x, 0.1f);
			}
			else {
				ImGui::Text("Select an object to edit.");
			}
		}
		ImGui::End();

		//  真ん中：エディタ情報
		ImGui::SetNextWindowPos(ImVec2(310, 10), ImGuiCond_Always);
		ImGui::Begin("Viewport Info", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %.1f", io.Framerate);
			if (selected != -1) ImGui::Text("Editing: %s", worldObjects[selected].name.c_str());
		}
		ImGui::End();


		//--- カメラの位置と向きの管理 ---
		static float camPos[3] = { 0.0f, 0.0f, 3.0f }; // 少し手前に配置

		//--------- 行列の計算 (GLMを使用) ---
	     Matrix4 model = Matrix4(1.0f); // 単位行列で初期化

		//-------カメラの位置を設定	
		Vector3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Vector3 cameraFront = glm::normalize(front);

		Vector3 camVec = Vector3(camPos[0], camPos[1], camPos[2]);
		glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);

		// 上方向ベクトル
		glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);

		
		//カメラの操作
		ImGui::Begin("Camera Editor");
		ImGui::SliderFloat3("Camera Position", camPos, -10.0f, 10.0f);
		ImGui::End();
		
		// 2. カメラ操作ロジック 
		if (!io.WantCaptureMouse) {
			// --- ズーム (ホイール) ---
			if (io.MouseWheel != 0.0f) {
				radius -= io.MouseWheel * 1.0f;
				if (radius < 0.1f) radius = 0.1f;
			}

			// --- 回転 (中央クリック) ---
			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
				float sensitivity = 0.2f;
				yaw += io.MouseDelta.x * sensitivity;
				pitch -= io.MouseDelta.y * sensitivity;
				if (pitch > 89.0f)  pitch = 89.0f;
				if (pitch < -89.0f) pitch = -89.0f;
			}
		}

		// 3. ★カメラ座標とView行列を確定させる (これが全ての基準になる)
		camVec.x = targetPos.x + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		camVec.y = targetPos.y + radius * sin(glm::radians(pitch));
		camVec.z = targetPos.z + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		glm::mat4 view = glm::lookAt(camVec, targetPos, glm::vec3(0, 1, 0));

		// --- 右クリックでメニューを開く ---
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !io.WantCaptureMouse) {
			ImGui::OpenPopup("ObjectMenu");//オブジェクトメニューのウィンドウを開く

			//メニュー表示計算
			double x, y;//メニューの座標(x,y)
			glfwGetCursorPos(window, &x, &y);//カーソルの位置を取得
			Vector3 rayDir = calculateRayFromPixel(x, y,projection, view);//
			

			float dist = 5.0f;                            // カメラから5m先に設置
			spawnPos = camVec + (rayDir * dist);          //空間内の配置座標を計算
			float snap = 0.5f;                            //スナップ処理
			spawnPos = glm::round(spawnPos / snap) * snap;//
		}
		// ImGuiで右クリックメニュー（ポップアップ）を描画
		if (ImGui::BeginPopup("ObjectMenu")) {
			// 選択されている物がある場合のみ「削除」を表示
			if (selected != -1) {
				ImGui::Text("Selected: %s", worldObjects[selected].name.c_str());
				if (ImGui::MenuItem("Delete", "Delete Key")) {
					worldObjects.erase(worldObjects.begin() + selected);
					selected = -1;
					isDragging = false;
				}
				ImGui::Separator();
			}

			ImGui::Text("--- Add Object ---");
			if (ImGui::MenuItem("Cube")) { worldObjects.push_back(GameObject("Cube", Vector3(0, 0, 0))); }
			if (ImGui::MenuItem("Sphere")) { /* 生成処理 */ }

			ImGui::EndPopup();
		}

		ImGuiIO& io = ImGui::GetIO();
		bool currentLeftState = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

		//  左ボタンが押されている間
		if (currentLeftState && !lastLeftState) {

			// UIを触っているかをチェック
			if (io.WantCaptureMouse) {}
			else {
				// 2. UIを触っていない場合のみ、3D空間の物体を探しに行く
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				// ※Viewportをずらしている場合は xpos - sidebarWidth を使う
				glm::vec3 rayDir = calculateRayFromPixel(xpos, ypos, projection, view);
				

				int hitIndex = -1;
				float closestDist = 100000.0f;

				for (int i = 0; i < (int)worldObjects.size(); i++) {
					float distToRay = glm::length(glm::cross(rayDir, worldObjects[i].position - camVec));
					if (distToRay < 0.8f) {
						float distToCam = glm::distance(camVec, worldObjects[i].position);
						if (distToCam < closestDist) {
							closestDist = distToCam;
							hitIndex = i;
						}
					}
				}

				// 3. 判定結果の適用
				if (hitIndex != -1) {
					selected = hitIndex;
					isDragging = true;
					dragDistance = glm::distance(camVec, worldObjects[selected].position);
				}
				else {
					// 背景をクリックした時だけ解除
					selected = -1;
					isDragging = false;
				}
			}
		}

		// 2. ドラッグ中の処理
		if (isDragging && currentLeftState && selected != -1) {
			if (!io.WantCaptureMouse) {
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);

				// カメラからの距離を保ったまま移動させる
				float currentDist = glm::distance(camVec, worldObjects[selected].position);

				// 現在のマウス位置から新しいレイを計算
				Vector3 rayDir = calculateRayFromPixel(xpos, ypos, projection, view);

				// 物体の位置を更新： カメラ位置 + (向き * 元の距離)
				worldObjects[selected].position = camVec + (rayDir * currentDist);

				// スナップ処理（0.5単位に吸着）
				float snapValue = 0.5f;
				worldObjects[selected].position = glm::round(worldObjects[selected].position / snapValue) * snapValue;//0.5刻みにして
			}
		}
		

		lastLeftState = currentLeftState;
		if (selected >= 0 && selected < (int)worldObjects.size()) {
			float snapValue = 0.5f;
			worldObjects[selected].position.x = round(worldObjects[selected].position.x / snapValue) * snapValue;
			worldObjects[selected].position.y = round(worldObjects[selected].position.y / snapValue) * snapValue;
			worldObjects[selected].position.z = round(worldObjects[selected].position.z / snapValue) * snapValue;
		}
		//tagの更新
		Tagsystem::Update(worldObjects);
		Tagsystem::ShowImGuiWindow();

		/*if (selected != -1 && selected < (int)worldObjects.size()) {
			ShowDetails(worldObjects[0]);
		}*/
		// --- 描画の準備  ---
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // 背景色を固定
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		int colorLoc = glGetUniformLocation(ourShader.ID, "ourColor");
		// Locationの取得 (描画ループの直前で行う)
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
		int vertexColorLocation = glGetUniformLocation(ourShader.ID, "ourColor");

		// カメラ・プロジェクション行列を先に転送
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		// グリッドの描画
		glm::mat4 identity = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
		glUniform4f(vertexColorLocation, 0.4f, 0.4f, 0.4f, 1.0f);//グリッドの色を固定
		DrawGrid(ourShader, myMesh);

		//オブジェクトの描画ループ
		for (int i = 0; i < worldObjects.size(); i++) {
			glm::mat4 objModel = worldObjects[i].getModelMatrix();

			// 1. 選択されている場合、先に「青い枠」を描画
			if (i == selected) {
				// 深度テストを一時的にオフにする
				glDisable(GL_DEPTH_TEST);

				// 1.05倍くらいに拡大したモデル行列を作る
				glm::mat4 outlineModel = glm::scale(objModel, glm::vec3(1.02f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineModel));

				// 青色で描画
				glUniform4f(vertexColorLocation, 0.0f, 0.5f, 1.0f, 1.0f); 
				myMesh.Draw(ourShader);

				glEnable(GL_DEPTH_TEST); // 深度テストを戻す
			}

			// 2. 本体の描画
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objModel));
			if (i == selected) {
				glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); // 本体は黄色
			}
			else {
				glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.2f, 1.0f); // 通常色
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

