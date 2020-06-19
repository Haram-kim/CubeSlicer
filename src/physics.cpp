#include "physics.h"
#include <iomanip>
Entity::Entity() : position(glm::vec3(0.0f, 0.0f, 0.0f)), velocity(glm::vec3(0.0f, 0.0f, 0.0f)), rotAxis(glm::vec3(0.0f, 0.0f, 0.0f)), angularVel(0.0f) {
	float randomPos = 10 * getRandom();

	position = glm::vec3(randomPos, -20.0f, -20.0f);
	velocity = glm::vec3(-0.5 * randomPos, 2* getRandom() + 25.0f, 0.0f);
	rotAxis = glm::normalize(glm::vec3(getRandom(), getRandom(), getRandom()));
	angularVel = 90.0f * getRandom();
	angle = 90.0f * getRandom();
	scale = (0.5*getRandom() + 5);

	int obj_shape_decision = floor((getRandom() + 1.0f) * 10.0f);

	if (obj_shape_decision == 0 || obj_shape_decision == 1) {
		obj = tetrahedron();
	}
	else if (obj_shape_decision == 2)  {
		obj = octahedron();
	}
	else {
		obj = cube();
	}
	if (getRandom() > 0.875f) {
		obj = catmull_clark(obj);
		if (getRandom() > 0.9375f) {
			obj = catmull_clark(obj);
		}
	}

	this->gravityAccel = glm::vec3(0.0f, GRAVITY_COEFF, 0.0f);
	modelMatrix = glm::mat4(1.0f);
	nBounce = 0;
}

Entity::Entity(object* obj, glm::vec3 position, glm::vec3 velocity, glm::vec3 rotAxis, float angularVel, float scale, float angle){
	
	this->position = position;
	this->velocity = velocity;
	this->rotAxis = rotAxis;
	this->angularVel = angularVel;
	this->angle = angle;
	this->scale = scale;
	
	this->obj = cube();

	// this->obj = octahedron();
	// this->obj = catmull_clark(this->obj);

	this->gravityAccel = glm::vec3(0.0f, GRAVITY_COEFF, 0.0f);
	modelMatrix = glm::mat4(1.0f);
	nBounce = 0;
}

void Entity::dynamics(float dt) {
	gravity(dt);
	this->angle += this->angularVel * dt;
	this->position += this->velocity * dt;
	if (this->position[1] < -40) {
		this->velocity[1] *= -0.90;
		this->position[1] = -40;
		nBounce++;
	}
	computeModel();
}

void Entity::gravity(float dt) {
	this->velocity += this->gravityAccel * dt;
}

void Entity::computeModel() {
	glm::mat4 transform = glm::mat4(1.0f);
	modelMatrix = glm::translate(transform, this->position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(this->angle), this->rotAxis);
	modelMatrix[3][3] /= scale;
};

void Entity::volume() {
	this->mass = scale * computeVolume(obj);
}

void Entity::bind() {
	// glBindVertexArray(this->VAO);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, diffuse->ID);
}

void Entity::bindVertex() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *  obj->vertices->size(), obj->vertices->data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * obj->vertexIndices->size(), obj->vertexIndices->data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	volume();
}

bool Entity::sliceCollision(Slice slice) {
	glm::vec4 slOriginTemp = glm::inverse(modelMatrix) * glm::vec4(slice.origin, 1);
	glm::vec3 slOrigin = glm::vec3(slOriginTemp / slOriginTemp[3]);
	glm::vec3 slDirection = glm::inverse(glm::mat3(modelMatrix)) * slice.direction;

	coord origin;
	coord direction;

	origin.x = slOrigin[0];
	origin.y = slOrigin[1];
	origin.z = slOrigin[2];

	direction.x = slDirection[0];
	direction.y = slDirection[1];
	direction.z = slDirection[2];

	// std::cout << "entity direction " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
	// std::cout << "entity origin " << origin.x << ", " << origin.y << ", " << origin.z << std::endl;
	if (sqrt(abs(glm::dot(position - slOrigin, slDirection))) > 4.0f * scale) {
		return false;
	}
	else {
		return isCollision(this->obj, origin, direction);
	}
}

void sliceObject(Entity* srcEntity, Entity* entity1, Entity* entity2, Slice slice) {
	glm::vec4 slOriginTemp = glm::inverse(srcEntity->modelMatrix) * glm::vec4(slice.origin, 1);
	glm::vec3 slOrigin = glm::vec3(slOriginTemp / slOriginTemp[3]);
	glm::vec3 slDirection = glm::mat3(glm::inverse(srcEntity->modelMatrix)) * slice.direction;

	coord origin;
	coord direction;

	origin.x = slOrigin[0];
	origin.y = slOrigin[1];
	origin.z = slOrigin[2];

	direction.x = slDirection[0];
	direction.y = slDirection[1];
	direction.z = slDirection[2];

	entity1->obj = obj_init();
	entity2->obj = obj_init();
	sliceObj(srcEntity->obj, entity1->obj, entity2->obj, origin, direction);
}

void catmullObject(Entity* srcEntity, Entity* entity) {
	entity->obj = catmull_clark(srcEntity->obj);
}

MouseTraj::MouseTraj(){
	vertices = new std::vector<GLfloat>();
	originColor = glm::vec3(0.1, 0.1, 0.1);
	startColor = glm::vec3(0.7, 0.7, 0.0);
	endColor = glm::vec3(1.0, 0.0, 0.0);
}

void MouseTraj::bindVertex(glm::vec3 viewCenter, float time, float sliceOn) {
	vertices->push_back(viewCenter[0]);
	vertices->push_back(viewCenter[1]);
	vertices->push_back(viewCenter[2]);
	vertices->push_back(time);
	vertices->push_back(sliceOn);
	if (vertices->size() > 45) {
		vertices->erase(vertices->begin());
		vertices->erase(vertices->begin());
		vertices->erase(vertices->begin());
		vertices->erase(vertices->begin());
		vertices->erase(vertices->begin());
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *  vertices->size(), vertices->data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
}

void MouseTraj::bindEffect1(Slice slice) {
	std::vector<GLfloat>* effect_vtx = new std::vector<GLfloat>();
	glm::vec3 slice_origin = slice.origin + glm::vec3(0.0, -0.5, 0.0);
	glm::vec3 slice_start = slice.start + (slice.start - slice.end) * 1e3f + (slice.start - slice.origin) * 1e2f;
	glm::vec3 slice_end = slice.end + (slice.end - slice.start) * 1e3f + (slice.start - slice.origin) * 1e2f;
	
	effect_vtx->push_back(slice_origin[0]);
	effect_vtx->push_back(slice_origin[1]);
	effect_vtx->push_back(slice_origin[2]);

	effect_vtx->push_back(originColor[0]);
	effect_vtx->push_back(originColor[1]);
	effect_vtx->push_back(originColor[2]);

	effect_vtx->push_back(slice_start[0]);
	effect_vtx->push_back(slice_start[1]);
	effect_vtx->push_back(slice_start[2]);

	effect_vtx->push_back(startColor[0]);
	effect_vtx->push_back(startColor[1]);
	effect_vtx->push_back(startColor[2]);

	effect_vtx->push_back(slice_end[0]);
	effect_vtx->push_back(slice_end[1]);
	effect_vtx->push_back(slice_end[2]);

	effect_vtx->push_back(endColor[0]);
	effect_vtx->push_back(endColor[1]);
	effect_vtx->push_back(endColor[2]);

	glGenVertexArrays(1, &VAO_eff1);
	glGenBuffers(1, &VBO_eff1);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO_eff1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_eff1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *  effect_vtx->size(), effect_vtx->data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

}

void MouseTraj::bindEffect2(Slice slice, float dt) {
	std::vector<GLfloat>* effect_vtx = new std::vector<GLfloat>();
	glm::vec3 tr = slice.proj_dir * 5.0f + slice.proj_nor * (float)exp(-5 * dt);
	glm::vec3 tl = slice.proj_dir * 5.0f - slice.proj_nor * (float)exp(-5 * dt);;
	glm::vec3 br = - slice.proj_dir * 5.0f + slice.proj_nor * (float)exp(-5 * dt);
	glm::vec3 bl = - slice.proj_dir * 5.0f - slice.proj_nor * (float)exp(-5 * dt);
		
	effect_vtx->push_back(tr[0]);
	effect_vtx->push_back(tr[1]);
	effect_vtx->push_back(tr[2]);

	effect_vtx->push_back(tl[0]);
	effect_vtx->push_back(tl[1]);
	effect_vtx->push_back(tl[2]);

	effect_vtx->push_back(bl[0]);
	effect_vtx->push_back(bl[1]);
	effect_vtx->push_back(bl[2]);

	effect_vtx->push_back(bl[0]);
	effect_vtx->push_back(bl[1]);
	effect_vtx->push_back(bl[2]);

	effect_vtx->push_back(br[0]);
	effect_vtx->push_back(br[1]);
	effect_vtx->push_back(br[2]);

	effect_vtx->push_back(tr[0]);
	effect_vtx->push_back(tr[1]);
	effect_vtx->push_back(tr[2]);

	glGenVertexArrays(1, &VAO_eff2);
	glGenBuffers(1, &VBO_eff2);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO_eff2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_eff2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *  effect_vtx->size(), effect_vtx->data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}


