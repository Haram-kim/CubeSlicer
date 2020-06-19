#pragma once

#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class coord;
class vertex;
class edge;
class face;
class object;

class coord
{
public:
	GLfloat x, y, z;
	coord():x(0.f),y(0.f),z(0.f){}
	coord(GLfloat _x, GLfloat _y, GLfloat _z):x(_x), y(_y), z(_z){}
	void add(const coord& ord)
	{
		this->x += ord.x;
		this->y += ord.y;
		this->z += ord.z;
	}
	void sub(const coord& ord)
	{
		this->x -= ord.x;
		this->y -= ord.y;
		this->z -= ord.z;
	}
	void mul(GLfloat m)
	{
		this->x *= m;
		this->y *= m;
		this->z *= m;
	}
	void div(GLfloat d)
	{
		this->x /= d;
		this->y /= d;
		this->z /= d;
	}
	void normalize()
	{
		GLfloat size = (GLfloat)sqrt(((double)this->x * this->x + (double)this->y * this->y + (double)this->z * this->z));
		this->x /= size;
		this->y /= size;
		this->z /= size;
	}
};

class vertex
{
public:
	coord xyz, avg_norm;
	std::vector<edge*>* e_list;
	std::vector<face*>* f_list;
	int idx;
	vertex* v_new;
	vertex* v_slice;
	~vertex()
	{
		delete e_list;
		delete f_list;
		delete v_new;
	}
};

class edge
{
public:
	std::vector<face*>* f_list;
	vertex* v1, *v2;
	vertex* edge_pt;
	vertex* e_slice1;
	vertex* e_slice2;
	coord avg;
	~edge()
	{
		delete f_list;
		delete v1;
		delete v2;
		delete edge_pt;
	}
};

class face
{
public:
	std::vector<vertex*>* v_list;
	std::vector<edge*>* e_list;
	coord norm;
	vertex* face_pt;
	bool isHit;
	~face()
	{
		delete v_list;
		delete e_list;
		delete face_pt;
	}
};

class object
{
public:
	int vertices_per_face;
	std::vector<vertex*>* v_list;
	std::vector<edge*>* e_list;
	std::vector<face*>* f_list;
	std::vector<GLfloat>* vertices;
	std::vector<unsigned int>* vertexIndices;
	~object()
	{
		delete v_list;
		delete e_list;
		delete f_list;
		delete vertices;
		delete vertexIndices;
	}
};
object* obj_init();
object* cube();
object* tetrahedron();
object* octahedron();

object* catmull_clark(object* obj);
bool isCollision(object * obj, const coord& origin, const coord& direction);
void sliceObj(object* srcObj, object* obj1, object* obj2, const coord& origin, const coord& direction);
void edge_pts_sort(std::vector<int> &cut1,
	std::vector<int> &cut2,
	const std::vector<vertex *> &edgePts1, 
	const std::vector<vertex *> &edgePts2,
	vertex *v1);
float computeVolume(object* obj);

