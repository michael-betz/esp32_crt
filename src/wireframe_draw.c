#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "wireframe_draw.h"
#include "draw.h"

// 1. Initialize 3D objects (which edges to show, their positions and orientations)
// 2. Set camera position
// 3. call calc_transform_matrix(entity, camera) for each entity, it calculates the transform matrix
// 4. the draw thread has a list of entities and uses the transform_matrix to get transformed vertices
//    it then uses the indexes from edges to draw lines between some of the transformed vertices

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

void obj_3d_draw(obj_3d_t *obj, unsigned density)
{
	if (obj == NULL)
		return;

	unsigned n_edges = obj->n_edges;
	const uint16_t *p = obj->edges;

	while (n_edges-- > 0) {
		unsigned index = *p++;
		bool is_goto = (index & 0x8000) > 0;
		index = (index & 0x7FFF) * 3;

		int x = obj->vertices[index];
		int y = obj->vertices[index + 1];
		int z = obj->vertices[index + 2];

		// TODO apply a general coordinate transformation matrix

		x /= 128;
		y /= 128;
		z /= 128;

		// Simple orthographic projection
		x += z / 4;
		y += z / 4;

		if (is_goto)
			push_goto(x, y);
		else
			push_line(x, y, density);
	}
}

void wf_test(void)
{
	static int frame = 0;
	static obj_3d_t o;

	if ((frame % 100) == 0) {
		int ind = frame / 100;
		ind %= 11;
		printf("obj_3d_set_edges(%d)\n", ind);
		obj_3d_set_edges(&o, &wf_numbers, ind);
	}

	obj_3d_draw(&o, 50);

	frame++;
}
