#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define TIMESPEED 1.5
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "opengl_utils.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include "camera.h"

#include "physics.h"
#include "texture.h"
#include "texture_cube.h"
#include "math_utils.h"
#include "class.h"
#include <chrono>
#include <thread>
#include <irrKlang.h>
#include <windows.h>
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "irrKlang.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void soundPlayer(SoundTrigger &soundTrigger);

void generate_cubes();
void entityUpdate();
void catmull(std::vector<Entity * >::iterator e_iter);
void sliceEntity(std::vector<Entity * >::iterator e_iter);
void sliceCommand();

bool isWindowed = true;
bool isKeyboardDone[1024] = { 0 };

// setting
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float planeSize = 15.f;

// camera
Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool useFrag1 = false;

// object
std::vector< Entity* > entities;
std::vector< std::vector<Entity * >::iterator> deleteEntities;
std::vector< Entity* > newEntities;
Entity * dummyEntity = new Entity();

// slice surface
Slice slice;
// aim
MouseTraj mouseTraj;
int cubemap_idx = 0;
int cubemap_size = 0;
// key control
KeyWait keywait;
// Sound trigger
irrklang::ISoundEngine *SoundEngine = irrklang::createIrrKlangDevice();
irrklang::ISound *m_pBackground = SoundEngine->play2D("../resources/sound/BGM.mp3", true, false, true);
irrklang::ISound *ligh_saver = SoundEngine->play2D("../resources/sound/start.wav", false);
SoundTrigger soundTrigger;

int main()
{
	std::cout << "--------------------------Cube slicer--------------------------" << std::endl;
	std::cout << "Developer: Haram Kim " << std::endl;
	std::cout << "Lab for Autonomous Robotics Research / Seoul National University" << std::endl;
	std::cout << "Email: rlgkfka614@gmail.com" << std::endl;
	std::cout << "LARR: http://larr.snu.ac.kr/" << std::endl;
	std::cout << "Homepage: https://haram-kim.github.io/" << std::endl;
	std::cout << "BGM source: https://soundcloud.com/crig-1/the-imperial-march-darth" << std::endl;
	std::cout << "SFX source: https://freesound.org/" << std::endl;
	std::cout << "Skybox source: https://opengameart.org/content/space-skyboxes-0" << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------" << std::endl;
	std::cout << "---------------------------Control-----------------------------" << std::endl;
	std::cout << "W/A/S/D: Move" << std::endl;
	std::cout << "Mouse Press: Slice the space" << std::endl;
	std::cout << "Key 1: Change cube color" << std::endl;
	std::cout << "Key 2: Change background" << std::endl;
	std::cout << "Key 3: ON/OFF effect sound" << std::endl;
	std::cout << "Key 4: ON/OFF BGM" << std::endl;
	std::cout << "---------------------------------------------------------------" << std::endl;

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::cout << "Now loading";
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cube Slicer", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	std::cout << ".";
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// open opengl
	std::cout << ".";
	// build and compile our shader program
	// ------------------------------------
	Shader objShader("../shaders/shader_object.vs", "../shaders/shader_object.fs"); // you can name your shader files however you like
	Shader skyboxShader("../shaders/shader_skybox.vs", "../shaders/shader_skybox.fs");
	Shader aimShader("../shaders/aim.vs", "../shaders/aim.fs");
	Shader effectShader("../shaders/effect.vs", "../shaders/effect.fs");

	// skybox
	// image cut : https://panorama-to-skybox.herokuapp.com/
	std::vector<CubemapTexture> cubemapList;
	std::vector<std::string> skyboxList;
	std::string ext = ".png";

	skyboxList.push_back("../resources/red/bkg1_");
	skyboxList.push_back("../resources/blue/bkg1_");
	skyboxList.push_back("../resources/lightblue/");

	std::string skybox_dir;
	for (int sky_iter = 0; sky_iter < skyboxList.size(); sky_iter++) {
		skybox_dir = skyboxList[sky_iter];
		std::vector<std::string> faces
		{
			skybox_dir + "right" + ext,
			skybox_dir + "left" + ext,
			skybox_dir + "top" + ext,
			skybox_dir + "bottom" + ext,
			skybox_dir + "front" + ext,
			skybox_dir + "back" + ext
		};
		cubemapList.push_back(CubemapTexture(faces));
	}
	cubemap_size = cubemapList.size();
	// load skyboxes
	std::cout << ".";
	unsigned int VAOskybox, VBOskybox;
	getPositionVAO(skybox_positions, sizeof(skybox_positions), VAOskybox, VBOskybox);

	skyboxShader.use();
	skyboxShader.setInt("skyboxTexture1", 0);
	objShader.use();
	objShader.setInt("texture1", 0);

	// dummyEntity->diffuse = new Texture("../resources/colormap.png");
	dummyEntity->diffuse = new Texture("../resources/father.png");
	glm::mat4 projection;
	glm::mat4 view;
	float oldTime = 0;
	float time_trigger = -5.0f;
	float effect1_start_time = 0;
	float effect2_start_time = 0;
	float time_speed = TIMESPEED;

	int totalSlice = 0;
	int max_counter = 0;

	std::cout << "\nComplete" << std::endl;
	std::cout << "Let's slice the cube!!!" << std::endl;
	// generate_cubes();
	while (!glfwWindowShouldClose(window))// render loop
	{
		soundPlayer(soundTrigger);

		float currentTime = glfwGetTime();
		float dt = currentTime - oldTime;
		deltaTime = dt;
		oldTime = currentTime;
		time_trigger += time_speed * dt;
		if (time_trigger > 5) {
			int rand_n = rand();
			for (int n_iter = -1; n_iter < rand_n % 5; n_iter++) {
				generate_cubes();
			}
			time_trigger = 0;
			soundTrigger.generate = true;
		}

		// input
		processInput(window);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use skybox Shader
		glDepthFunc(GL_LEQUAL);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.use();
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setFloat("useEffect2", slice.Effect2);
		skyboxShader.setVec3("sliceDir", glm::normalize(slice.dir));
		skyboxShader.setVec3("sliceNor", glm::normalize(slice.nor));
		skyboxShader.setFloat("dt", currentTime - effect1_start_time);

		// render a skybox
		glBindVertexArray(VAOskybox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapList.at(cubemap_idx).textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		view = camera.GetViewMatrix();

		objShader.use();
		objShader.setMat4("view", view);
		objShader.setMat4("projection", projection);
		objShader.setMat4("world", glm::mat4(1.0));
		objShader.setFloat("useFrag1", useFrag1);
		objShader.setFloat("useEffect2", slice.Effect2);
		objShader.setVec3("sliceDir", glm::normalize(slice.dir));
		objShader.setVec3("sliceNor", glm::normalize(slice.nor));
		objShader.setFloat("dt", currentTime - effect1_start_time);
		objShader.use();
		for (auto e_iter = entities.begin(); e_iter != entities.end(); e_iter++) {
			Entity * curEntity = *e_iter;
			curEntity->bind();
			curEntity->dynamics(time_speed * dt);
			objShader.setMat4("world", curEntity->modelMatrix);
			objShader.setFloat("mass", curEntity->mass);

			glBindVertexArray(curEntity->VAO);
			glDrawElements(GL_TRIANGLES, curEntity->obj->vertexIndices->size(), GL_UNSIGNED_INT, 0);

			if (slice.sliceFlag) {
				if (curEntity->sliceCollision(slice)) {
					sliceEntity(e_iter);
					slice.counter++;
				}
			}
			if (curEntity->nBounce > 1) {
				deleteEntities.push_back(e_iter);
			}
		}
		if (slice.sliceFlag) {
			slice.sliceFlag = false;

			slice.Effect1 = true;
			soundTrigger.effect1 = true;
			effect1_start_time = currentTime;
			mouseTraj.bindEffect1(slice);

			if (slice.counter > 2) {
				slice.Effect2 = true;
				soundTrigger.effect2 = true;
				effect2_start_time = currentTime;
				slice.proj_dir = glm::vec3(projection * glm::mat4(glm::mat3(camera.GetViewMatrix())) *glm::vec4(slice.dir, 1.0f));
				slice.proj_nor = glm::vec3(projection * glm::mat4(glm::mat3(camera.GetViewMatrix())) *glm::vec4(slice.nor, 1.0f));

				time_speed = 0.02;
			}
			totalSlice += slice.counter;
			if (max_counter < slice.counter) {
				max_counter = slice.counter;
				std::cout << "Best slash: " << max_counter << std::endl;
			}
			std::cout << "Total slice : " << totalSlice << " \t \t Total number of object : " << entities.size() << std::endl;
			slice.counter = 0;
		}
		entityUpdate();

		// render aim
		aimShader.use();
		aimShader.setMat4("view", view);
		aimShader.setMat4("projection", projection);
		aimShader.setFloat("curTime", currentTime);

		mouseTraj.bindVertex(camera.Position + camera.Front * 10.0f, currentTime, (float)slice.estimating);
		glBindVertexArray(mouseTraj.VAO);
		glDrawArrays(GL_LINE_STRIP, 0, mouseTraj.vertices->size() / 5);

		// render effect
		if (slice.Effect1) {
			effectShader.use();
			effectShader.setMat4("view", view);
			effectShader.setMat4("projection", projection);
			effectShader.setFloat("dt", currentTime - effect1_start_time);
			glBindVertexArray(mouseTraj.VAO_eff1);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			if (currentTime - effect1_start_time > 2.0) {
				slice.Effect1 = false;

			}
		}

		if (slice.Effect2) {
			effectShader.use();
			effectShader.setMat4("view", glm::mat4(1.0));
			effectShader.setMat4("projection", glm::mat4(1.0));
			effectShader.setBool("effect2", true);
			mouseTraj.bindEffect2(slice, currentTime - effect1_start_time);
			glBindVertexArray(mouseTraj.VAO_eff2);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			if (currentTime - effect2_start_time > 1.25) {
				slice.Effect2 = false;
				time_speed = TIMESPEED;
				cubemap_idx++;
				if (cubemap_idx >= cubemap_size) {
					cubemap_idx = 0;
				}
			}
			effectShader.setBool("effect2", false);
		}
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	SoundEngine->drop();
	return 0;
}

void generate_cubes() {
	// Entity * newEntity = new Entity(cube());
	Entity * newEntity = new Entity();
	newEntity->diffuse = dummyEntity->diffuse;
	entities.push_back(newEntity);

	auto newCube = *(entities.end() - 1);
	newCube->bindVertex();
}
void entityUpdate() {
	for (auto d_iter = deleteEntities.rbegin(); d_iter != deleteEntities.rend(); d_iter++) {
		entities.erase(*d_iter);
	}
	deleteEntities.clear();

	for (auto n_iter = newEntities.begin(); n_iter != newEntities.end(); n_iter++) {
		entities.push_back(*n_iter);
	}
	newEntities.clear();
}


void catmull(std::vector<Entity * >::iterator e_iter) {
	Entity *curEntity = *e_iter;

	glDeleteVertexArrays(1, &curEntity->VAO);
	glDeleteBuffers(1, &curEntity->VBO);
	glDeleteBuffers(1, &curEntity->EBO);

	Entity * newEntity = new Entity(NULL, curEntity->position, curEntity->velocity, curEntity->rotAxis, curEntity->angularVel, curEntity->scale, curEntity->angle);
	newEntity->diffuse = curEntity->diffuse;
	// catmull_clark subdivision
	catmullObject(curEntity, newEntity);
	newEntity->bindVertex();

	deleteEntities.push_back(e_iter);
	newEntities.push_back(newEntity);
}

void sliceEntity(std::vector<Entity * >::iterator e_iter) {
	Entity *curEntity = *e_iter;

	glDeleteVertexArrays(1, &curEntity->VAO);
	glDeleteBuffers(1, &curEntity->VBO);
	glDeleteBuffers(1, &curEntity->EBO);

	Entity * entity1 = new Entity(NULL, curEntity->position, curEntity->velocity, curEntity->rotAxis, curEntity->angularVel, curEntity->scale, curEntity->angle);
	Entity * entity2 = new Entity(NULL, curEntity->position, curEntity->velocity, curEntity->rotAxis, curEntity->angularVel, curEntity->scale, curEntity->angle);
	entity1->diffuse = curEntity->diffuse;
	entity2->diffuse = curEntity->diffuse;
	// slice object
	sliceObject(curEntity, entity1, entity2, slice);
	entity1->bindVertex();
	entity2->bindVertex();

	entity1->position += 0.0002f * slice.direction;
	entity2->position -= 0.0002f * slice.direction;

	entity1->velocity += 3.0f * slice.direction * entity2->mass / curEntity->mass;
	entity2->velocity -= 3.0f * slice.direction * entity1->mass / curEntity->mass;

	entity1->angularVel += 5.0f * entity2->mass / curEntity->mass;
	entity2->angularVel -= 5.0f * entity1->mass / curEntity->mass;

	entity1->rotAxis += 0.02f * slice.direction * entity2->mass / curEntity->mass;
	entity2->rotAxis -= 0.02f * slice.direction * entity2->mass / curEntity->mass;

	entity1->nBounce = curEntity->nBounce;
	entity2->nBounce = curEntity->nBounce;

	//entity1->position += slice.direction;
	//entity2->position -= slice.direction;

	deleteEntities.push_back(e_iter);
	newEntities.push_back(entity1);
	newEntities.push_back(entity2);
}

void sliceCommand() {
	if (!slice.estimating) {
		slice.origin = camera.Position;
		slice.start = camera.Position + camera.Front;
		slice.front = camera.Front;
		slice.prevFront = camera.Front;
		slice.estimating = true;
	}
	if (slice.estimating) {
		slice.prevFront = slice.front;
		slice.front = camera.Front;
		slice.end = camera.Position + camera.Front;
		slice.dir = glm::normalize(slice.start - slice.end);
		slice.nor = glm::normalize(glm::cross(slice.dir, slice.end - slice.origin));
	}

}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
		camera.ProcessMouseMovement(0, 1000 * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		camera.ProcessMouseMovement(-1000 * deltaTime, 0);
	if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
		camera.ProcessMouseMovement(0, -1000 * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		camera.ProcessMouseMovement(1000 * deltaTime, 0);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		sliceCommand();
		if (glm::dot(slice.prevFront - slice.front, slice.prevFront - slice.front) > 0.01f && glm::dot(slice.dir, slice.prevFront - slice.front) < 0.0f) {
			slice.sliceFlag = true;
			slice.direction = glm::normalize(glm::cross(slice.end - slice.start, slice.end - slice.origin));
			slice.start = slice.end;
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && slice.estimating) {
		slice.sliceFlag = true;
		slice.direction = glm::normalize(glm::cross(slice.end - slice.start, slice.end - slice.origin));
		slice.estimating = false;
	}

	float t = 20.0f * deltaTime;


	// key 1 : toggle using normal map
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		keywait.key1 = true;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE && keywait.key1) {
		keywait.key1 = false;
		if (useFrag1) {
			useFrag1 = false;
		}
		else {
			useFrag1 = true;
		}
	}

	// key 2 : change skybox
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		keywait.key2 = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE && keywait.key2) {
		keywait.key2 = false;
		cubemap_idx++;
		if (cubemap_idx >= cubemap_size) {
			cubemap_idx = 0;
		}
	}

	// key 3 : sound on off
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		keywait.key3 = true;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE && keywait.key3) {
		keywait.key3 = false;
		if (soundTrigger.on) {
			soundTrigger.on = false;
		}
		else {
			soundTrigger.on = true;
		}
	}

	// key 3 : BGM on off
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		keywait.key4 = true;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE && keywait.key4) {
		keywait.key4 = false;
		if (soundTrigger.BGMon) {
			soundTrigger.BGMon = false;
		}
		else {
			soundTrigger.BGMon = true;
		}
		soundTrigger.BGMTrigger = true;
	}

}

void soundPlayer(SoundTrigger &soundTrigger) {
	if (soundTrigger.on) {
		if (soundTrigger.BGMon && soundTrigger.BGMTrigger) {
			m_pBackground = SoundEngine->play2D("../resources/sound/BGM.mp3", true, false, true);
			soundTrigger.BGMTrigger = false;
		}
		if (!soundTrigger.BGMon && soundTrigger.BGMTrigger) {
			m_pBackground->stop();
			m_pBackground->drop();
			soundTrigger.BGMTrigger = false;
		}
		if (soundTrigger.generate) {
			SoundEngine->play2D("../resources/sound/generate.wav", false);
			soundTrigger.generate = false;
		}
		if (soundTrigger.effect1 && !soundTrigger.effect2) {
			SoundEngine->play2D("../resources/sound/effect1.mp3", false);
			soundTrigger.effect1 = false;
		}
		if (soundTrigger.effect2) {
			SoundEngine->play2D("../resources/sound/effect2.wav", false);
			soundTrigger.effect1 = false;
			soundTrigger.effect2 = false;
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
