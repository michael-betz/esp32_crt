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
	obj->n_vertices = len;

	len = get_ptr_and_len(edge_data->edges_ends, edge_data->n_objects, asset_index, &start_index);
	if (len < 0)
		return;
	obj->edges = &edge_data->all_edges[start_index];
	obj->n_edges = len;

	printf("%d: n_verts: %d (%d, %d, %d), n_edges: %d (%d, %d)\n", asset_index, obj->n_vertices, obj->vertices[0], obj->vertices[1], obj->vertices[2], obj->n_edges, obj->edges[0], obj->edges[1]);
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

		// TODO apply coordinate transformation matrix

		if (is_goto)
			push_goto(x, y);
		else
			push_line(x, y, density);
	}
}

void wf_test(void)
{
	printf("wf_test\n");

	for (unsigned obj_index = 0; obj_index <= 10; obj_index++) {
		unsigned start_index = 0;

		int len = get_ptr_and_len(wf_numbers.vertices_ends, wf_numbers.n_objects, obj_index, &start_index);
		printf("%d:  start: %d, len: %d   (%d, %d, %d)\n", obj_index, start_index, len, wf_numbers.all_vertices[start_index], wf_numbers.all_vertices[start_index + 1], wf_numbers.all_vertices[start_index + 2]);
	}

	obj_3d_t o;
	obj_3d_set_edges(&o, &wf_numbers, 0);
	obj_3d_draw(&o, 100);
}
