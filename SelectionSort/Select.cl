/*Each kernel will find the kth smallest number, 
and insert it into the kth place in output buffer.
NOTE: If duplicates exist, will have strange behaviour.*/
__kernel void select_sort(__global int *in, __global int *out, uint count) { 
	// find the index of number to find
	size_t k = get_global_id(0); 

	// determine the index to insert number into
	uint index = 0;
	for (uint i = 0; i < count; ++i) 
		index += in[k] > in[i];

	out[index] = in[k]; // insert number into correct index
}