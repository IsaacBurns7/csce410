Below is the psuedocode for a linked list implementation for cont_frame_pool.H
/*
 * we need base frames, info frames, protected frames
 * possible implementations: bitmap, linked list, map
 * linked list would look like...
 * 	get_frames(n): //allocate anywhere
 *		if n > free_frames: return 0
 * 		walk through naively-implemented free list 
 * 		4 cases:
 * 		<allocating [base, n] against free list node [base2, n2]> 
 * 			base = base2
 * 				replace w/ [n, n2-n] 
 * 			base = base2 and n = n2 
 * 				remove [base2, n2] 
 * 			base < base2 
 * 				replace w/ [base2, base] and [base2+n, n2] 
 *			base < base2 and n = n2 - (base2 - base) 
 *				replace w/ [base2, base]
 *		NOTE: only case 1 and 2 exist since we can just enforce base = base2, since only n is specified, not base_frame_no 
 *		if couldnt find viable free list node: return 0 
 *		free_frames -= n
 * 	mark_inaccessible(base_frame_no, n): 
 * 		//OPEN QUESTIONS
 * 			//what if this frame pool doesn't own it...?
 *			//what if this block is not in the freelist (somebody thinks this block is allocated to them...) 
 * 		walk through naively-implemented free list 
 * 		4 valid cases, 3 autoreject:`
 * 		<marking inacessible [base, n] against free list node [base2, n2]> 
 * 			autoreject if base > base2
 * 			autoreject if base = n and n > n2
 * 			autoreject if base < base2 and n > n2 - (base2 - base)
 * 				at this point can we just return ? 
 * 			base = base2 and n < n2 
 * 				replace w/ [n, n2-n] 
 * 			base = base2 and n = n2 
 * 				remove [base2, n2] 
 * 			base < base2 and n < n2 - (base2 - base)
 * 				replace w/ [base2, base] and [base2+n, n2] 
 *			base < base2 and n = n2 - (base2 - base) 
 *				replace w/ [base2, base]
 *		add [base, n] to inaccessible list
 *	static release_frames(first_frame_no, n):
 *		ContFramePool* owning_pool;
 *		<set owning_pool to correct owning_pool> 
 *		owning_pool->release_frames(first_frame_no, n);
 *	release_frames(first_frame_no, n):
 *		walk through naively-implemented free list
 *		<freeing [base, n] against free list node [base2, n2]> 
 * 			autoreject if base > base2
 * 			autoreject if base = n and n > n2
 * 			autoreject if base < base2 and n > n2 - (base2 - base)
 * 				at this point can we just return ?
 * 			base = base2 and n < n2 
 * 				replace w/ [n, n2-n] 
 * 			base = base2 and n = n2 
 * 				remove [base2, n2] 
 * 			base < base2 and n < n2 - (base2 - base)
 * 				replace w/ [base2, base] and [base2+n, n2] 
 *			base < base2 and n = n2 - (base2 - base) 
 *				replace w/ [base2, base]
 */
 
