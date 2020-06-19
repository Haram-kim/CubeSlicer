#include "class.h"
#include <numeric>
#include <algorithm> 
#include <cmath>

#define PI 3.14159265358979323846
object* obj_init()
{
	object* newobj = new object();
	newobj->v_list = new std::vector<vertex*>();
	newobj->e_list = new std::vector<edge*>();
	newobj->f_list = new std::vector<face*>();
	newobj->vertices = new std::vector<GLfloat>();
	newobj->vertexIndices = new std::vector<unsigned int>();
	return newobj;
}

vertex* vtx_init()
{
	vertex* newvtx = new vertex();
	newvtx->e_list = new std::vector<edge*>();
	newvtx->f_list = new std::vector<face*>();
	newvtx->v_new = NULL;
	newvtx->idx = -1;
	return newvtx;
}

edge* edge_init()
{
	edge* newedge = new edge();
	newedge->f_list = new std::vector<face*>();
	newedge->v1 = NULL;
	newedge->v2 = NULL;
	newedge->edge_pt = NULL;
	return newedge;
}

face* face_init()
{
	face* newface = new face();
	newface->v_list = new std::vector<vertex*>();
	newface->e_list = new std::vector<edge*>();
	newface->face_pt = NULL;
	newface->isHit = false;
	return newface;
}

vertex* add_vertex(object* obj, const coord& coord)
{
	vertex* newvtx = vtx_init();
	newvtx->xyz.x = coord.x;
	newvtx->xyz.y = coord.y;
	newvtx->xyz.z = coord.z;
	newvtx->idx = obj->v_list->size();
	obj->v_list->push_back(newvtx);
	return newvtx;
}

edge* find_edge(object* obj, vertex* v1, vertex* v2)
{
	std::vector<edge*>* v1_edgeList = v1->e_list;
	for(int i = 0; i < v1_edgeList->size(); i++)
	{
		if((*v1_edgeList)[i]->v1 == v2 || (*v1_edgeList)[i]->v2 == v2)
		{
			return (*v1_edgeList)[i];
		}
	}
	return NULL;
}

edge* add_edge(object* obj, vertex* v1, vertex* v2)
{
	edge* newedge = edge_init();
	newedge->v1 = v1;
	newedge->v2 = v2;
	v1->e_list->push_back(newedge);
	v2->e_list->push_back(newedge);
	obj->e_list->push_back(newedge);

	coord temp;
	temp.add(newedge->v1->xyz);
	temp.add(newedge->v2->xyz);
	temp.div(2);

	newedge->avg = temp;
	return newedge;
}

face* add_face(object* obj, const std::vector<int>& vertexIndices)
{
	face* newface = face_init();
	int n = vertexIndices.size();
	for (int i = 0; i < n; i++)
	{
		vertex* v1 = (*(obj->v_list))[vertexIndices[i]];
		vertex* v2 = (*(obj->v_list))[vertexIndices[(i+1)%n]];
		v1->f_list->push_back(newface);
		
		edge* temp = find_edge(obj, v1, v2);
		if (!temp) temp = add_edge(obj, v1, v2);

		temp->f_list->push_back(newface);
		newface->e_list->push_back(temp);
		newface->v_list->push_back(v1);
	}
	obj->f_list->push_back(newface);
	return newface;
}

coord add(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.x + ord2.x;
	temp.y = ord1.y + ord2.y;
	temp.z = ord1.z + ord2.z;
	return temp;
}

coord sub(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.x - ord2.x;
	temp.y = ord1.y - ord2.y;
	temp.z = ord1.z - ord2.z;
	return temp;
}

coord mul(const coord& ord1, GLfloat m)
{
	coord temp;
	temp.x = ord1.x * m;
	temp.y = ord1.y * m;
	temp.z = ord1.z * m;
	return temp;
}

coord div(const coord& ord1, GLfloat d)
{
	coord temp;
	temp.x = ord1.x / d;
	temp.y = ord1.y / d;
	temp.z = ord1.z / d;
	return temp;
}

coord cross(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.y * ord2.z - ord1.z * ord2.y;
	temp.y = ord1.z * ord2.x - ord1.x * ord2.z;
	temp.z = ord1.x * ord2.y - ord1.y * ord2.x;
	return temp;
}

float dot(const coord& ord1, const coord& ord2) {
	return ord1.x * ord2.x + ord1.y * ord2.y + ord1.z * ord2.z;
}

float norm(const coord& ord1) {
	return sqrt(dot(ord1, ord1));
}

void setNorm(object* obj)
{
	for (int i = 0; i < obj->f_list->size(); i++)
	{
		face* temp = (*(obj->f_list))[i];
		coord v01 = sub((*(temp->v_list))[1]->xyz, (*(temp->v_list))[0]->xyz);
		coord v12 = sub((*(temp->v_list))[2]->xyz, (*(temp->v_list))[1]->xyz);
		coord crs = cross(v01, v12);
		crs.normalize();
		temp->norm = crs;
	}

	for (int i = 0; i < obj->v_list->size(); i++)
	{
		coord sum;
		std::vector<face*>* temp = (*(obj->v_list))[i]->f_list;
		int n = temp->size();
		for (int j = 0; j < n; j++)
		{
			sum.add((*temp)[j]->norm);
		}
		sum.div((GLfloat)n);
		sum.normalize();
		(*(obj->v_list))[i]->avg_norm = sum;
	}
}

void aggregate_vertices(object* obj)
{
	obj->vertices->clear();
	obj->vertexIndices->clear();

	for (int i = 0; i < obj->v_list->size(); i++)
	{
		coord temp_pos = (*(obj->v_list))[i]->xyz;
		coord temp_norm = (*(obj->v_list))[i]->avg_norm;
		obj->vertices->push_back(temp_pos.x);
		obj->vertices->push_back(temp_pos.y);
		obj->vertices->push_back(temp_pos.z);
		obj->vertices->push_back(temp_norm.x);
		obj->vertices->push_back(temp_norm.y);
		obj->vertices->push_back(temp_norm.z);
	}

	if (obj->vertices_per_face == 3)
	{
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			obj->vertexIndices->push_back((*temp)[0]->idx);
			obj->vertexIndices->push_back((*temp)[1]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
		}
	}

	else if (obj->vertices_per_face == 4)
	{
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			obj->vertexIndices->push_back((*temp)[0]->idx);
			obj->vertexIndices->push_back((*temp)[1]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
			obj->vertexIndices->push_back((*temp)[3]->idx);
			obj->vertexIndices->push_back((*temp)[0]->idx);
		}
	}
	else {
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			int v_size = temp->size();
			for (int j = 0; j < temp->size()-2; j++) {
				if (j % 2 == 0) {
					int j_temp = j / 2;
					obj->vertexIndices->push_back((*temp)[j_temp]->idx);
					obj->vertexIndices->push_back((*temp)[j_temp + 1]->idx);
					obj->vertexIndices->push_back((*temp)[v_size - j_temp -1]->idx);
				}
				else {
					int j_temp = (j + 1) / 2;
					obj->vertexIndices->push_back((*temp)[j_temp]->idx);
					obj->vertexIndices->push_back((*temp)[v_size - j_temp - 1]->idx);
					obj->vertexIndices->push_back((*temp)[v_size - j_temp]->idx);
				}
			}
		}
	}
}

object* cube()
{
	object* newobj = obj_init();
	newobj->vertices_per_face = 4;
	for (int x = -1; x <= 1; x += 2)
	{
		for (int y = -1; y <= 1; y += 2)
		{
			for (int z = -1; z <= 1; z += 2)
			{
				add_vertex(newobj, coord((GLfloat)x, (GLfloat)y, (GLfloat)z));
			}
		}
	}
	add_face(newobj, { 0,2,6,4 });
	add_face(newobj, { 0,4,5,1 });
	add_face(newobj, { 0,1,3,2 });
	add_face(newobj, { 2,3,7,6 });
	add_face(newobj, { 6,7,5,4 });
	add_face(newobj, { 1,5,7,3 });

	setNorm(newobj);

	aggregate_vertices(newobj);

	return newobj;
}

object* tetrahedron()
{
	object* newobj = obj_init();
	newobj->vertices_per_face = -1;

	float srqt_2 = 1.4142135;

	add_vertex(newobj, coord((GLfloat)1.0f, (GLfloat)-srqt_2 /4.0f, (GLfloat)-0.5f));
	add_vertex(newobj, coord((GLfloat)-1.0f, (GLfloat)-srqt_2 / 4.0f, (GLfloat)-0.5f));
	add_vertex(newobj, coord((GLfloat)0.0f, (GLfloat)-srqt_2 / 4.0f, (GLfloat)1.0f));
	add_vertex(newobj, coord((GLfloat)0.0f, (GLfloat)srqt_2 * 3.0f / 4.0f, (GLfloat)0.0f));

	add_face(newobj, { 0,1,2 });
	add_face(newobj, { 0,3,1 });
	add_face(newobj, { 0,2,3 });
	add_face(newobj, { 1,3,2 });

	setNorm(newobj);

	aggregate_vertices(newobj);

	return newobj;
}

object* octahedron()
{
	object* newobj = obj_init();
	newobj->vertices_per_face = -1;

	add_vertex(newobj, coord((GLfloat)1.0f, (GLfloat)0.0f, (GLfloat)0.0f));
	add_vertex(newobj, coord((GLfloat)-1.0f, (GLfloat)0.0f, (GLfloat)0.0f));
	add_vertex(newobj, coord((GLfloat)0.0f, (GLfloat)1.0f, (GLfloat)0.0f));
	add_vertex(newobj, coord((GLfloat)0.0f, (GLfloat)-1.0f, (GLfloat)0.0f));
	add_vertex(newobj, coord((GLfloat)0.0f, (GLfloat)0.0f, (GLfloat)1.0f));
	add_vertex(newobj, coord((GLfloat)0.0f, (GLfloat)0.0f, (GLfloat)-1.0f));


	add_face(newobj, { 0,2,5 });
	add_face(newobj, { 0,4,2 });
	add_face(newobj, { 1,5,2 });
	add_face(newobj, { 1,2,4 });

	add_face(newobj, { 0,3,4 });
	add_face(newobj, { 0,5,3 });
	add_face(newobj, { 1,4,3 });
	add_face(newobj, { 1,3,5 });

	setNorm(newobj);

	aggregate_vertices(newobj);

	return newobj;
}

bool is_holeEdge(edge* e)
{
	if (e->f_list->size() == 1) {
		return true;
	}
	else{
		return false;
	}
}

bool is_holeVertex(vertex* v)
{
	if (v->e_list->size() != v->f_list->size()) {
		return true;
	}
	else {
		return false;
	}
}

vertex* face_point(face* f)
{
	if (f->face_pt == NULL) {
		vertex* newvtx = vtx_init();
		coord temp;
		for (int i = 0; i < f->v_list->size(); i++) {
			temp.add(f->v_list->at(i)->xyz);
		}
		temp.div(f->v_list->size());
		newvtx->xyz = temp;
		f->face_pt = newvtx;
	}
	return f->face_pt;
}

vertex* edge_point(edge* e)
{
	if (e->edge_pt == NULL) {
		vertex* newvtx = vtx_init();
		coord temp;
		temp.add(e->v1->xyz);
		temp.add(e->v2->xyz);
		if (is_holeEdge(e)) {
			temp.div(2);
		}
		else {
			for (int i = 0; i < e->f_list->size(); i++) {
				temp.add(e->f_list->at(i)->face_pt->xyz);
			}
			temp.div(e->f_list->size() + 2);
		}
		newvtx->xyz = temp;
		e->edge_pt = newvtx;
	}

	return e->edge_pt;
}

vertex* vertex_point(vertex* v)
{
	if (v->v_new == NULL) {
		vertex* newvtx = vtx_init();
		coord temp;
		if (is_holeVertex(v)) {
			temp.add(v->xyz);
			for (int i = 0; i < v->e_list->size(); i++) {
				temp.add(v->e_list->at(i)->avg);
			}
			temp.div(v->e_list->size() + 1);
		}
		else {
			coord temp_1, temp_2, temp_3;
			for (int i = 0; i < v->f_list->size(); i++) {
				temp_1.add(v->f_list->at(i)->face_pt->xyz);
			}
			temp_1.div(v->f_list->size());

			for (int i = 0; i < v->e_list->size(); i++) {
				temp_2.add(v->e_list->at(i)->avg);
			}
			temp_2.div(v->e_list->size());
			temp_2.mul(2);

			int n = v->f_list->size();
			temp_3.add(v->xyz);
			temp_3.mul(n - 3);

			temp.add(temp_1);
			temp.add(temp_2);
			temp.add(temp_3);
			temp.div(n);
		}
		newvtx->xyz = temp;
		v->v_new = newvtx;
	}
	return v->v_new;
}

object* catmull_clark(object* obj)
{
	object * newobj = obj_init();
	newobj->vertices_per_face = 4;

	// add new points
	for (int f_iter = 0; f_iter < obj->f_list->size(); f_iter++) {
		face* curFace = obj->f_list->at(f_iter);
		// add face points
		auto curFacePts = face_point(curFace);
		curFacePts->idx = newobj->v_list->size();
		add_vertex(newobj, curFacePts->xyz);
	}
	for (int f_iter = 0; f_iter < obj->f_list->size(); f_iter++) {
		face* curFace = obj->f_list->at(f_iter);
		// add edge points
		for (int e_iter = 0; e_iter < curFace->e_list->size(); e_iter++) {
			auto curEdgePts = edge_point(curFace->e_list->at(e_iter));
			if (curEdgePts->idx == -1) {
				curEdgePts->idx = newobj->v_list->size();
				add_vertex(newobj, curEdgePts->xyz);
			}
		}
		// add vertex points
		for (int v_iter = 0; v_iter < curFace->v_list->size(); v_iter++) {
			auto curVertexPts = vertex_point(curFace->v_list->at(v_iter));
			if (curVertexPts->idx == -1) {
				curFace->v_list->at(v_iter)->v_new->idx = newobj->v_list->size();
				add_vertex(newobj, curVertexPts->xyz);
			}
				
		}
	}

	// add new faces
	for (int f_iter = 0; f_iter < obj->f_list->size(); f_iter++) {
		auto nVertex = obj->f_list->at(f_iter)->v_list->size();
		face* curFace = obj->f_list->at(f_iter);

		for (int v_iter = 0; v_iter < nVertex; v_iter++) {
			vertex* v0 = curFace->v_list->at((v_iter - 1 + nVertex) % nVertex);
			vertex* v1 = curFace->v_list->at(v_iter);
			vertex* v2 = curFace->v_list->at((v_iter + 1) % nVertex);

			edge* edge1 = find_edge(obj, v1, v2);
			edge* edge2 = find_edge(obj, v0, v1);
			add_face(newobj, { v1->v_new->idx, edge1->edge_pt->idx,
						curFace->face_pt->idx, edge2->edge_pt->idx });
		}
	}

	setNorm(newobj);
	aggregate_vertices(newobj);

	return newobj;

}

bool isCollision(object* obj, const coord& origin, const coord& direction) {
	if (obj->v_list->size() < 2) {
		return false;
	}

	bool flipCheck = dot(sub(obj->v_list->at(0)->xyz, origin), direction) > 0;
	
	for (int v_iter = 1; v_iter != obj->v_list->size(); v_iter++) {
		if (dot(sub(obj->v_list->at(v_iter)->xyz, origin), direction) > 0 != flipCheck) {
			return true;
		}
	}
	
	return false;
}

vertex* vertex_slice(vertex* v)
{
	if (v->v_slice == NULL) {
		vertex* newvtx = vtx_init();
		newvtx->xyz = v->xyz;
		v->v_slice = newvtx;
	}
	return v->v_slice;
}

void edge_slice(edge* e, const coord& origin, const coord& direction)
{
	if (e->e_slice1 == NULL || e->e_slice2 == NULL) {
		vertex* newvtx1 = vtx_init();
		vertex* newvtx2 = vtx_init();
		coord temp = add(mul(sub(e->v2->xyz, e->v1->xyz), dot(sub(origin, e->v1->xyz), direction) / dot(sub(e->v2->xyz, e->v1->xyz), direction)), e->v1->xyz);
		newvtx1->xyz = temp;
		newvtx2->xyz = temp;

		e->e_slice1 = newvtx1;
		e->e_slice2 = newvtx2;
	}
}

void sliceObj(object* srcObj, object* obj1, object* obj2, const coord& origin, const coord& direction) {

	obj1->vertices_per_face = -1;
	obj2->vertices_per_face = -1;
	// add existing points
	for (int v_iter = 0; v_iter != srcObj->v_list->size(); v_iter++) {
		vertex* curVertexPts = srcObj->v_list->at(v_iter);
		if (dot(sub(srcObj->v_list->at(v_iter)->xyz, origin), direction) > 0) {
			auto curPts = vertex_slice(curVertexPts);
			curPts->idx = obj1->v_list->size();
			add_vertex(obj1, curPts->xyz);
		}
		else {
			auto curPts = vertex_slice(curVertexPts);
			curPts->idx = obj2->v_list->size();
			add_vertex(obj2, curPts->xyz);
		}
	}
	// add new points
	std::vector<vertex *> edgePts1;
	std::vector<vertex *> edgePts2;
	vertex *temp_non_hit_vtx1;

	for (int f_iter = 0; f_iter != srcObj->f_list->size(); f_iter++) {
		face* curFace = srcObj->f_list->at(f_iter);
		for (int e_iter = 0; e_iter < curFace->e_list->size(); e_iter++) {
			// hit edge detection
			if (dot(sub(curFace->e_list->at(e_iter)->v1->xyz, origin), direction) * dot(sub(curFace->e_list->at(e_iter)->v2->xyz, origin), direction) < 0){
				curFace->isHit = true;
				// split edge, find split points
				if (curFace->e_list->at(e_iter)->e_slice1 == NULL && curFace->e_list->at(e_iter)->e_slice2 == NULL) {
					edge_slice(curFace->e_list->at(e_iter), origin, direction);
					auto curPts1 = curFace->e_list->at(e_iter)->e_slice1;
					curPts1->idx = obj1->v_list->size();
					add_vertex(obj1, curPts1->xyz);

					auto curPts2 = curFace->e_list->at(e_iter)->e_slice2;
					curPts2->idx = obj2->v_list->size();
					add_vertex(obj2, curPts2->xyz);

					edgePts1.push_back(curPts1);
					edgePts2.push_back(curPts2);
				}
			}
		}
	}

	// add new faces
	for (int f_iter = 0; f_iter < srcObj->f_list->size(); f_iter++) {
		auto nVertex = srcObj->f_list->at(f_iter)->v_list->size();
		face* curFace = srcObj->f_list->at(f_iter);

		// hit face
		if (curFace->isHit) {
			std::vector<int> temp1;
			std::vector<int> temp2;
			bool is_1;
			is_1 = dot(sub(curFace->v_list->at(0)->xyz, origin), direction) > 0;
			for (int idx_iter = 0; idx_iter < nVertex; idx_iter++) {
				if (is_1) {
					temp1.push_back(curFace->v_list->at(idx_iter)->v_slice->idx);
					temp_non_hit_vtx1 = curFace->v_list->at(idx_iter);
				}
				else {
					temp2.push_back(curFace->v_list->at(idx_iter)->v_slice->idx);
				}
				edge * curEdge = find_edge(srcObj, curFace->v_list->at(idx_iter), curFace->v_list->at((idx_iter + 1) % nVertex));
				if (curEdge->e_slice1 != NULL && curEdge->e_slice2 != NULL) {
					temp1.push_back(curEdge->e_slice1->idx);
					temp2.push_back(curEdge->e_slice2->idx);
					// target object switching
					if (is_1) {
						is_1 = false;
					}
					else {
						is_1 = true;
					}
				}
			}
			add_face(obj1, temp1);
			add_face(obj2, temp2);
		}
		// non-hit face
		else {
			std::vector<int> temp;
			for (int idx_iter = 0; idx_iter < nVertex; idx_iter++) {
				temp.push_back(curFace->v_list->at(idx_iter)->v_slice->idx);
			}
			if (dot(sub(curFace->v_list->at(0)->xyz, origin), direction) > 0) {
				add_face(obj1, temp);
			}
			else {
				add_face(obj2, temp);
			}
		}
	}

	// cutting face
	std::vector<int> tempEdge1;
	std::vector<int> tempEdge2;
	edge_pts_sort(tempEdge1, tempEdge2, edgePts1, edgePts2, temp_non_hit_vtx1);

	add_face(obj1, tempEdge1);
	add_face(obj2, tempEdge2);

	setNorm(obj1);
	setNorm(obj2);

	aggregate_vertices(obj1);
	aggregate_vertices(obj2);

	// delete obj;
}

void edge_pts_sort(std::vector<int> &cut1,
	std::vector<int> &cut2,
	const std::vector<vertex *> &edgePts1,
	const std::vector<vertex *> &edgePts2,
	vertex *v1) {

	std::vector<float> angle;
	coord normal;
	coord mean_pts;
	mean_pts.x = 0;
	mean_pts.y = 0;
	mean_pts.z = 0;
	int nEdgePts = edgePts1.size();
	for (int ev_iter = 0; ev_iter < nEdgePts; ev_iter++) {
		mean_pts = add(mean_pts, edgePts1.at(ev_iter)->xyz);
	}
	mean_pts = div(mean_pts, (float)nEdgePts);
	normal = cross(sub(edgePts1.at(0)->xyz, mean_pts), sub(edgePts1.at(1)->xyz, mean_pts));
	if (dot(normal, sub(v1->xyz, mean_pts)) > 0) {
		normal = mul(normal, -1.0);
	}
	coord baseline = sub(edgePts1.at(0)->xyz, mean_pts);
	angle.push_back(0.0);
	for (int ev_iter = 1; ev_iter < nEdgePts; ev_iter++) {
		coord temp = sub(edgePts1.at(ev_iter)->xyz, mean_pts);
		float temp_angle = std::acosf(dot(baseline, temp) / (norm(baseline)*norm(temp)));
		if (dot(cross(baseline, temp), normal) < 0) {
			temp_angle = 2*PI - temp_angle;
		}
		angle.push_back(temp_angle);
	}

	std::vector<int> e_idx(nEdgePts);
	std::iota(e_idx.begin(), e_idx.end(), 0);
	std::stable_sort(e_idx.begin(), e_idx.end(), [&angle](size_t i1, size_t i2) {return angle[i1] < angle[i2]; });

	for (int ev_iter = 0; ev_iter < nEdgePts; ev_iter++) {
		cut1.push_back(edgePts1.at(e_idx[ev_iter])->idx);
		cut2.push_back(edgePts2.at(e_idx[nEdgePts - ev_iter - 1])->idx);
	}
}

float computeVolume(object* obj) {
	float temp = 0;
	coord avg;
	for (int v_iter = 0; v_iter != obj->v_list->size(); v_iter++) {
		avg = add(avg, obj->v_list->at(v_iter)->xyz);
	}
	avg = div(avg, obj->v_list->size());
	for (int v_iter = 0; v_iter != obj->v_list->size(); v_iter++) {
		temp += norm(sub(obj->v_list->at(v_iter)->xyz, avg));
	}
	return temp;
}