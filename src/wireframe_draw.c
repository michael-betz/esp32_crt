#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "wireframe_draw.h"
#include "draw.h"
#include "fast_sin.h"

// 1. Initialize 3D objects (which edges to show, their positions and orientations)
// 2. Set camera position
// 3. call calc_transform_matrix(entity, camera) for each entity, it calculates the transform matrix
// 4. the draw thread has a list of entities and uses the transform_matrix to get transformed vertices
//    it then uses the indexes from edges to draw lines between some of the transformed vertices

void m_print(t_m4 m)
{
	for(int y = 0; y < 3; y++) {
		for(int x = 0; x < 4; x++) {
			printf("%9d ", m[y][x]);
		}
		printf("\n");
	}
}

void v_m_multiply(const int16_t *in, t_m4 m, t_v3 out)
{
    // out = in * m;
    out[0] = m[0][0] * in[0] / WF_ONE + m[0][1] * in[1] / WF_ONE + m[0][2] * in[2] / WF_ONE + m[0][3];
    out[1] = m[1][0] * in[0] / WF_ONE + m[1][1] * in[1] / WF_ONE + m[1][2] * in[2] / WF_ONE + m[1][3];
    out[2] = m[2][0] * in[0] / WF_ONE + m[2][1] * in[1] / WF_ONE + m[2][2] * in[2] / WF_ONE + m[2][3];
}

// add rotation to the transformation matrix `out`
// rotate around the X, Y or Z axis (axis = 0, 1, 2)
void m_rotation(int a, int axis, t_m4 out)
{
	// zero angle = no rotation needed
	if ((a & (MAX_ANGLE - 1)) == 0)
		return;

	int sina = get_sin(a) / WF_INT_DIV;
	int cosa = get_cos(a) / WF_INT_DIV;

	t_m4 tmp;
	memcpy(tmp, out, sizeof(tmp));

	for (int i = 0; i < 4; i++) {
		if (axis == 0) {
			out[1][i] = cosa * tmp[1][i] / WF_ONE - sina * tmp[2][i] / WF_ONE;
			out[2][i] = sina * tmp[1][i] / WF_ONE + cosa * tmp[2][i] / WF_ONE;
		} else if (axis == 1) {
			out[0][i] = cosa * tmp[0][i] / WF_ONE + sina * tmp[2][i] / WF_ONE;
			out[2][i] = -sina * tmp[0][i] / WF_ONE + cosa * tmp[2][i] / WF_ONE;
		} else if (axis == 2) {
			out[0][i] = cosa * tmp[0][i] / WF_ONE - sina * tmp[1][i] / WF_ONE;
			out[1][i] = sina * tmp[0][i] / WF_ONE + cosa * tmp[1][i] / WF_ONE;
		}
	}
}

// Find the beginning and length of a variable width array.
// sets start_index_out to the index of the first element and returns the length
static int get_ptr_and_len(const uint16_t *end_index_array, unsigned end_index_array_len, unsigned asset_index, unsigned *start_index_out)
{
	if (asset_index >= end_index_array_len)
		return -1;

	unsigned end_index = end_index_array[asset_index];
	unsigned start_index = 0;

	if (asset_index > 0)
		start_index = end_index_array[asset_index - 1];

	if (start_index_out != NULL)
		*start_index_out = start_index;

	return end_index - start_index;
}

// apply scaling, rotation around xYZ and translation to an object
// from the parameters given in obj_3d_t
void obj_3d_update_transform_matrix(obj_3d_t *obj)
{
	t_m4 tmp = {
		{obj->scale, 0, 0, 0},
		{0, obj->scale, 0, 0},
		{0, 0, obj->scale, 0}
	};

	// Rotate about the objects center
	m_rotation(obj->u, 0, tmp);
	m_rotation(obj->v, 1, tmp);
	m_rotation(obj->w, 2, tmp);

	// Translate the object
	tmp[0][3] = obj->x;
	tmp[1][3] = obj->y;
	tmp[2][3] = obj->z;

	memcpy(obj->transform_matrix, tmp, sizeof(tmp));
}

// load 3D data into an obj_3d_t
void obj_3d_set_edges(obj_3d_t *obj, const edges_3d_t *edge_data, unsigned asset_index)
{
	if (obj == NULL || edge_data == NULL)
		return;

	// memset(obj, 0, sizeof(obj_3d_t));

	unsigned start_index = 0;
	int len = get_ptr_and_len(edge_data->vertices_ends, edge_data->n_objects, asset_index, &start_index);
	if (len < 0)
		return;
	obj->vertices = &edge_data->all_vertices[start_index];
	obj->n_vertices = len / 3;

	len = get_ptr_and_len(edge_data->edges_ends, edge_data->n_objects, asset_index, &start_index);
	if (len < 0)
		return;
	obj->edges = &edge_data->all_edges[start_index];
	obj->n_edges = len;

	printf("%d: n_verts: %d (%d, %d, %d), n_edges: %d\n", asset_index, obj->n_vertices, obj->vertices[0], obj->vertices[1], obj->vertices[2], obj->n_edges);
}

static obj_3d_t o_camera;

// draw an obj_3d_t
void obj_3d_draw(obj_3d_t *obj, unsigned density)
{
	unsigned n_edges = obj->n_edges;
	const uint16_t *p = obj->edges;

	if (obj == NULL)
		return;

	// apply global coordinate transformation (simulates camera position)
	t_m4 m_cam;
	memcpy(m_cam, obj->transform_matrix, sizeof(m_cam));
	m_rotation(o_camera.u, 0, m_cam);
	m_rotation(o_camera.v, 1, m_cam);
	m_rotation(o_camera.w, 2, m_cam);
	m_cam[0][3] -= o_camera.x;
	m_cam[1][3] -= o_camera.y;
	m_cam[2][3] -= o_camera.z;

	while (n_edges-- > 0) {
		unsigned index = *p++;
		bool is_goto = (index & 0x8000) > 0;
		index = (index & 0x7FFF) * 3;

		// Apply the coordinate transformation matrix for translation / rotation / scaling
		t_v3 vert;
		v_m_multiply(&obj->vertices[index], m_cam, vert);

		int x = vert[0];
		int y = vert[1];
		int z = vert[2];

		// Simple perspective with wide FOV
		x = x * (WF_ONE / 2) / z;
		y = y * (WF_ONE / 2) / z;

		if (is_goto)
			push_goto(x, y);
		else
			push_line(x, y, density);
	}
}

void wf_test(void)
{
	static int frame = 0;
	static obj_3d_t objs[5];


	if (frame == 0) {
		o_camera.z = -0x1000;

		// FIXME: this mirrors things :(
		// objs[4].u = 0x1;

		for (unsigned i=0; i<5; i++) {
			objs[i].scale = WF_ONE / 16;
			objs[i].x = -0xC00 + i * 0xC00 / 2;
			obj_3d_update_transform_matrix(&objs[i]);
		}

		obj_3d_set_edges(&objs[2], &wf_numbers, 10);
	}

	// o_camera.v += 1;
	// o_camera.w = 0x100;

	if ((frame % 50) == 0) {
		int frm = frame / 50;
		printf("obj_3d_set_edges(%d)\n", frm);

		int secs = frm % 60;
		int min = (frm / 60) % 60;

		obj_3d_set_edges(&objs[0], &wf_numbers, min / 10);
		obj_3d_set_edges(&objs[1], &wf_numbers, min % 10);

		obj_3d_set_edges(&objs[3], &wf_numbers, secs / 10);
		obj_3d_set_edges(&objs[4], &wf_numbers, secs % 10);
	}

	for (unsigned i=0; i<5; i++)
		obj_3d_draw(&objs[i], 50);

	frame++;
}
