#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum LightType {
	DIRECTIONAL,
	POINT
};
class DirectionalLight {
public:
	float azimuth;
	float elevation;
	glm::vec3 lightDir; // direction of light. If elevation is 90, it would be (0,-1,0)
	glm::vec3 lightColor; // this is I_d (I_s = I_d, I_a = 0.3 * I_d)

	DirectionalLight(float azimuth, float elevation, glm::vec3 lightColor) {
		this->azimuth = azimuth;
		this->elevation = elevation;
		updateLightDir();
		this->lightColor = lightColor;
	}

	DirectionalLight(glm::vec3 lightDir, glm::vec3 lightColor) {
		this->lightDir = lightDir;
		this->lightColor = lightColor;
	}

	glm::mat4 getViewMatrix(glm::vec3 cameraPosition) {
		// directional light has no light position. Assume fake light position depending on camera position.
		float lightDistance = 15.0f;
		glm::vec3 lightPos = cameraPosition + this->lightDir * lightDistance;
		return glm::lookAt(lightPos, cameraPosition, glm::vec3(0, 1, 0));
	}

	glm::mat4 getProjectionMatrix() {
		// For simplicity, just use static projection matrix. (Actually we have to be more accurate with considering camera's frustum)
		return glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 50.0f);
	}

	void updateLightDir() {
		glm::vec3 lightdir;
		lightdir.x = cos(glm::radians(azimuth)) * cos(glm::radians(elevation));
		lightdir.y = sin(glm::radians(elevation));
		lightdir.z = sin(glm::radians(azimuth)) * cos(glm::radians(elevation));
		this->lightDir = glm::normalize(lightdir);
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x(azimuth) and y(elevation) direction.
	void processKeyboard(float xoffset, float yoffset)
	{
		azimuth += xoffset;
		elevation += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (elevation > 80.0f)
			elevation = 80.0f;
		if (elevation < 15.0f)
			elevation = 15.0f;
		// Update Front, Right and Up Vectors using the updated Euler angles
		updateLightDir();
	}
};

#endif