#include <stddef.h>
#include <stdint.h>
#include "wireframe_draw.h"

// 1. Initialize 3D objects (which edges to show, their positions and orientations)
// 2. Set camera position
// 3. call calc_transform_matrix(entity, camera) for each entity, it calculates the transform matrix
// 4. the draw thread has a list of entities and uses the transform_matrix to get transformed vertices
//    it then uses the indexes from edges to draw lines between some of the transformed vertices

// Find the beginning and length of a variable width array.
// sets start_index_out to the index of the first element and returns the length
int get_ptr_and_len(const uint16_t *end_index_array, unsigned end_index_array_len, unsigned obj_index, unsigned *start_index_out)
{
	if (obj_index >= end_index_array_len)
		return -1;

	unsigned end_index = end_index_array[obj_index];
	unsigned start_index = 0;

	if (obj_index > 0)
		start_index = end_index_array[obj_index - 1];

	if (start_index_out != NULL)
		*start_index_out = start_index;

	return end_index - start_index;
}


void wf_test(void)
{
	printf("wf_test\n");

	for (unsigned obj_index = 0; obj_index <= 10; obj_index++) {
		unsigned start_index = 0;

		int len = get_ptr_and_len(wf_numbers.vertices_ends, wf_numbers.n_objects, obj_index, &start_index);
		printf("%d:  start: %d, len: %d   (%d, %d, %d)\n", obj_index, start_index, len, wf_numbers.all_vertices[start_index], wf_numbers.all_vertices[start_index + 1], wf_numbers.all_vertices[start_index + 2]);
	}
}
