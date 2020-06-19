#pragma once
#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>
#include <cmath>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "class.h"
// #include "math_utils.h"

#include <iostream>

#define GRAVITY_COEFF -10
struct SoundTrigger {
	bool generate = false;
	bool effect1 = false;
	bool effect2 = false;
	bool on = true;
	bool BGMon = true;
	bool BGMTrigger = false;
};
struct Slice {
	glm::vec3 origin;
	glm::vec3 direction;

	glm::vec3 dir;
	glm::vec3 nor;
	glm::vec3 proj_dir;
	glm::vec3 proj_nor;

	bool sliceFlag = false;
	bool estimating = false;
	
	bool Effect1 = false;
	bool Effect2 = false;

	glm::vec3 front;
	glm::vec3 prevFront;

	glm::vec3 start;
	glm::vec3 end;

	int counter = 0;
};

struct KeyWait {
	bool key1 = false;
	bool key2 = false;
	bool key3 = false;
	bool key4 = false;
};

class MouseTraj
{
public:
	unsigned int VAO, VBO, VAO_eff1, VBO_eff1, VAO_eff2, VBO_eff2;
	glm::vec3 originColor;
	glm::vec3 startColor;
	glm::vec3 endColor;
	std::vector<GLfloat>* vertices;
	MouseTraj();
	void bindVertex(glm::vec3 viewCenter, float time, float sliceOn);
	void bindEffect1(Slice slice);
	void bindEffect2(Slice slice, float dt);
};

class Texture;

class Entity
{
public:
	unsigned int VAO, VBO, EBO;
	object* obj;
	Texture* diffuse;
	int nBounce;
	// mass
	float mass;
	// position
	float scale;
	glm::vec3 position;
	glm::vec3 velocity;
	// angle
	glm::vec3 rotAxis;
	float angle;
	float angularVel;
	// gravity
	glm::vec3 gravityAccel;
	// transfrom
	glm::mat4 modelMatrix;

	Entity();
	Entity(object* obj, glm::vec3 position = glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rotAxis = glm::vec3(0.0f, 1.0f, 0.0f),
		float angularVel = 0.0f, float scale = 0.5f, float angle = 0.0f);
	~Entity() {
		delete obj;
	}

	void dynamics(float dt);
	void gravity(float dt);
	void computeModel();
	void volume();
	void bind();
	void bindVertex();
	float getRandom() {
		float r = (float)rand() / (float)RAND_MAX;
		return 2 * r - 1;

		//return getRandomValueBetween(-1.0f, 1.0f);
	}
	bool sliceCollision(Slice slice);

};

void sliceObject(Entity* srcEntity, Entity* entity1, Entity* entity2, Slice slice);
void catmullObject(Entity* srcEntity, Entity* entity);

#endif