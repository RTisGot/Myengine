#define GLEW_STATIC
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Mesh.h"
#include "GameObject.h"
#include "Tagsystem.h"
#include "Core/EditorUI.h"
#include "PlayerComponent.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include<vector>
#include <iostream>

glm::mat4 modelMatrix = (1.f);         //行列の初期化

const GLint WIDTH = 800, HEIGHT = 600;//ウィンドウサイズ

std::vector<Vertex> vertices = {
	// 前面
	{{-0.5f, -0.5f,  0.5f}}, {{ 0.5f, -0.5f,  0.5f}}, {{ 0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f,  0.5f}},
	// 背面
	{{-0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f, -0.5f}}, {{ 0.5f,  0.5f, -0.5f}}, {{-0.5f,  0.5f, -0.5f}}
};

// インデックス
std::vector<unsigned int> indices = {
	0, 1, 2, 2, 3, 0, // ?O
	1, 5, 6, 6, 2, 1, // ?E
	7, 6, 5, 5, 4, 7, // ??
	4, 0, 3, 3, 7, 4, // ??
	3, 2, 6, 6, 7, 3, // ??
	4, 5, 1, 1, 0, 4  // ??
};

void processInput(GLFWwindow* window) {
	// ESCでゲームウィンドウを閉じる
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

// 
std::vector<GameObject> worldObjects;
std::vector<GameObject> editorBackup; // 

int main()
{
	//実行状態を管理するフラグ(Editorの実行フラグ)
	bool isPlaying = false;

	//------3D変換----
	static float position[3] = { 0.0f, 0.0f, 0.0f }; // X, Y, Z 位置
	static float rotation = 0.0f;                    // 回転角
	static float scale = 1.0f;                       // スケール
	//window初期化
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
	

	// ???j?^?[??????
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	// ?t???X?N???[???E?B???h?E???
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

	//glew???????
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW!" << "\n";
		
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	//?[?x?e?X?g??L???????
	glEnable(GL_DEPTH_TEST);

	// 1. ImGui ?R???e?L?X?g???
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// 2. ?o?b?N?G???h???????
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// 3. ?X?^?C???????????
	ImGui::StyleColorsDark(); 
	// ????Shader?N???X??? 
	Shader ourShader("shader.vert", "shader.frag");
	//????Mesh?N???X???
	Mesh myMesh(vertices, indices);
	

	static float yaw = -90.0f; // ???E?????
	static float pitch = 0.0f; // ???????
	float radius = 5.0f;
	static double lastX = 400, lastY = 300;
	static bool firstMouse = true;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);//?}?E?X???????????

  //------------------
	bool isDragging = false; //objectdrag??false?t?^
	bool lastLeftState = false;
	float dragDistance = 0.0f; // ?h???b?O???????????????
	glm::vec3 dragOffset; // ?N???b?N?????n?_???????S??Y??????
	static int selected = -1; //?I????e??index????????
	static Vector3 spawnPos;  //???j???[???J???????????W
	worldObjects.reserve(100);
	double lastFrameTime = glfwGetTime();

	
	//--------------------------------???C?????[?v---------------------//
	while (!glfwWindowShouldClose(window))
	{
		const double currentTime = glfwGetTime();
		const float dt = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;

		//?t???[???o?b?t?@?T?C?Y?????
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

		// UE5?????C?A?E?g: ??Outliner / ?EDetails / ??Toolbar ????????????3D?`???g??
		const int leftPanelWidth = (int)EditorUI::kLeftPanelWidth;
		const int rightPanelWidth = (int)EditorUI::kRightPanelWidth;
		const int topBarHeight = (int)EditorUI::kTopBarHeight;
		const int viewportWidth = display_w - leftPanelWidth - rightPanelWidth;
		const int viewportHeight = display_h - topBarHeight;
		glViewport(leftPanelWidth, 0, viewportWidth, viewportHeight);

		// ?A?X?y?N?g???????V?????T?C?Y???????v?Z??????
		float aspect = (float)viewportWidth / (float)viewportHeight;
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
		//Get and handle user inputs
		glfwPollEvents();

		// --- ImGui ??V?????t???[?????J?n ---
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//??????t
		processInput(window);
		

		//PIE???????Y???????
		if (isPlaying) {
			for (auto& obj : worldObjects) {
				obj.Update(dt, window);
			}
			Tagsystem::Update(worldObjects);
		}
		else {
		}
		//PIE UI?\??
		EditorUI::ShowMainEditor(isPlaying, worldObjects, editorBackup);

		//--- ?J???????u????????? ---
		static float camPos[3] = { 0.0f, 0.0f, 3.0f }; // ??????O??z?u

		//--------- ?s???v?Z (GLM???g?p) ---
	     Matrix4 model = Matrix4(1.0f); // ?P??s????????

		//-------?J???????u????	
		Vector3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Vector3 cameraFront = glm::normalize(front);

		Vector3 camVec = Vector3(camPos[0], camPos[1], camPos[2]);
		glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);

		// ??????x?N?g??
		glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);

		
		// ?J?????????
		ImGui::SetNextWindowPos(ImVec2((float)leftPanelWidth + 12.0f, EditorUI::kTopBarHeight + 12.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.75f);
		ImGui::Begin("Camera Editor");
		ImGui::SliderFloat3("Camera Position", camPos, -10.0f, 10.0f);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %.1f", io.Framerate);
		ImGui::End();
		
		// 2. ?J?????????W?b?N 
		if (!io.WantCaptureMouse) {
			// --- ?Y?[?? (?z?C?[??) ---
			if (io.MouseWheel != 0.0f) {
				radius -= io.MouseWheel * 1.0f;
				if (radius < 0.1f) radius = 0.1f;
			}

			// --- ??] (?????N???b?N) ---
			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
				float sensitivity = 0.2f;
				yaw += io.MouseDelta.x * sensitivity;
				pitch -= io.MouseDelta.y * sensitivity;
				if (pitch > 89.0f)  pitch = 89.0f;
				if (pitch < -89.0f) pitch = -89.0f;
			}
		}

		// 3. ???J???????W??View?s????m?????? (?????S????????)
		std::shared_ptr<PlayerComponent> activePlayerComp = nullptr;
		GameObject* activePlayerObject = nullptr;
		for (auto& obj : worldObjects) {
			auto playerComp = obj.GetComponent<PlayerComponent>();
			if (playerComp) {
				activePlayerComp = playerComp;
				activePlayerObject = &obj;
				break;
			}
		}

		if (isPlaying && activePlayerComp && activePlayerObject) {
			const glm::vec3 forward = glm::normalize(glm::vec3(front.x, front.y, front.z));
			if (activePlayerComp->viewMode == PlayerComponent::ViewMode::FirstPerson) {
				camVec = activePlayerObject->position + glm::vec3(0.0f, 1.6f, 0.0f);
				targetPos = camVec + forward;
			}
			else {
				targetPos = activePlayerObject->position;
				camVec.x = targetPos.x + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
				camVec.y = targetPos.y + radius * sin(glm::radians(pitch));
				camVec.z = targetPos.z + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
			}
		}
		else {
			camVec.x = targetPos.x + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
			camVec.y = targetPos.y + radius * sin(glm::radians(pitch));
			camVec.z = targetPos.z + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		}
		glm::mat4 view = glm::lookAt(camVec, targetPos, glm::vec3(0, 1, 0));

		// --- ?E?N???b?N????j???[???J?? ---
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !io.WantCaptureMouse) {
			ImGui::OpenPopup("ObjectMenu");//?I?u?W?F?N?g???j???[??E?B???h?E???J??

			//???j???[?\???v?Z
			double x, y;//???j???[????W(x,y)
			glfwGetCursorPos(window, &x, &y);//?J?[?\?????u?????
			Vector3 rayDir = calculateRayFromPixel(x, y,projection, view);//
			

			float dist = 5.0f;                            // ?J????????5m????u
			spawnPos = camVec + (rayDir * dist);          //??????z?u???W???v?Z
			float snap = 0.5f;                            //?X?i?b?v????
			spawnPos = glm::round(spawnPos / snap) * snap;//
		}
		// ImGui??E?N???b?N???j???[?i?|?b?v?A?b?v?j??`??
		if (ImGui::BeginPopup("ObjectMenu")) {
			// ?I????????????????????u???v??\??
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
			if (ImGui::MenuItem("Sphere")) { /* ???????? */ }

			ImGui::EndPopup();
		}

		ImGuiIO& io = ImGui::GetIO();
		bool currentLeftState = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

		//  ???{?^???????????????
		if (currentLeftState && !lastLeftState) {

			// UI??G??????????`?F?b?N
			if (io.WantCaptureMouse) {}
			else {
				// 2. UI??G?????????????A3D????????T????s??
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				// ??Viewport?????????????? xpos - sidebarWidth ???g??
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

				// 3. ???????K?p
				if (hitIndex != -1) {
					selected = hitIndex;
					isDragging = true;
					dragDistance = glm::distance(camVec, worldObjects[selected].position);
				}
				else {
					// ?w?i???N???b?N??????????????
					selected = -1;
					isDragging = false;
				}
			}
		}

		// 2. ?h???b?O???????
		if (isDragging && currentLeftState && selected != -1) {
			if (!io.WantCaptureMouse) {
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);

				// ?J?????????????????????????????
				float currentDist = glm::distance(camVec, worldObjects[selected].position);

				// ?????}?E?X??u????V???????C???v?Z
				Vector3 rayDir = calculateRayFromPixel(xpos, ypos, projection, view);

				// ??????u???X?V?F ?J??????u + (???? * ???????)
				worldObjects[selected].position = camVec + (rayDir * currentDist);

				// ?X?i?b?v?????i0.5?P???z???j
				float snapValue = 0.5f;
				worldObjects[selected].position = glm::round(worldObjects[selected].position / snapValue) * snapValue;//0.5????????
			}
		}
		

		lastLeftState = currentLeftState;
		if (selected >= 0 && selected < (int)worldObjects.size()) {
			float snapValue = 0.5f;
			worldObjects[selected].position.x = round(worldObjects[selected].position.x / snapValue) * snapValue;
			worldObjects[selected].position.y = round(worldObjects[selected].position.y / snapValue) * snapValue;
			worldObjects[selected].position.z = round(worldObjects[selected].position.z / snapValue) * snapValue;
		}
		//tag??X?V
		Tagsystem::Update(worldObjects);
		Tagsystem::ShowImGuiWindow();

		
		// --- ?`??????  ---
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // ?w?i?F?????
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		int colorLoc = glGetUniformLocation(ourShader.ID, "ourColor");
		// Location???? (?`???[?v????O??s??)
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
		int vertexColorLocation = glGetUniformLocation(ourShader.ID, "ourColor");

		// ?J?????E?v???W?F?N?V?????s?????]??
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		// ?O???b?h??`??
		glm::mat4 identity = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
		glUniform4f(vertexColorLocation, 0.4f, 0.4f, 0.4f, 1.0f);//?O???b?h??F?????
		DrawGrid(ourShader, myMesh);

		//?I?u?W?F?N?g??`???[?v
		for (int i = 0; i < worldObjects.size(); i++) {
			glm::mat4 objModel = worldObjects[i].getModelMatrix();

			// 1. ?I????????????A???u????g?v??`??
			if (i == selected) {
				// ?[?x?e?X?g?????I??I?t?????
				glDisable(GL_DEPTH_TEST);

				// 1.05?{??????g????????f???s??????
				glm::mat4 outlineModel = glm::scale(objModel, glm::vec3(1.02f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineModel));

				// ??F??`??
				glUniform4f(vertexColorLocation, 0.0f, 0.5f, 1.0f, 1.0f); 
				myMesh.Draw(ourShader);

				glEnable(GL_DEPTH_TEST); // ?[?x?e?X?g????
			}

			// 2. ?{???`??
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objModel));
			if (i == selected) {
				glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); // ?{?????F
			}
			else {
				glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.2f, 1.0f); // ???F
			}
			myMesh.Draw(ourShader);
		}
		

		// --- ImGui ????????f?????? ---
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

//?v???O?????I?u?W?F?N?g????????
//  vsrc:?o?[?e?b?N?V?F?[?_?[??\?[?X?v???O???????????
//  vsrc:?t???O?????g?V?F?[?_?[??\?[?X?v???O???????????
GLuint createProgram(const char* vsrc, const char* fsrc)
{
	//???I?u?W?F?N?g????????
	const GLuint program(glCreateProgram());

	if (vsrc != NULL)
	{
		//?o?[?e?b?N?X?V?F?[?_?[??V?F?C?_?[?I?u?W?F?N?g????
		const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
		glShaderSource(vobj, 1, &vsrc, NULL);
		glCompileShader(vobj);

		//?o?[?e?b?N?X?V?F?[?_?[?I?u?W?F?N?g???v???W?F?N?g?I?u?W?F?N?g???????
		glAttachShader(program, vobj);
		glDeleteShader(vobj);
	}

	if (fsrc != NULL)
	{
		//?t???O?????g?V?F?[?_?[??V?F?[?_?[?I?u?W?F?N?g????????
		const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(fobj, 1, &fsrc, NULL);
		glCompileShader(fobj);

		//?t???O?????g?V?F?[?_?[??V?F?[?_?[?I?u?W?F?N?g???v???O?????I?u?W?F?N?g??g?????
		glAttachShader(program, fobj);
		glDeleteShader(fobj);
	}

	//?v???O?????I?u?W?F?N?g???????N????
	glBindAttribLocation(program, 0, "position");
	glBindFragDataLocation(program, 0, "fragment");
	glLinkProgram(program);

	//???????v???O?????I?u?W?F?N?g????
	return program;
}

