/***************************************************************************/
/* (C) 2016 Elettra - Sincrotrone Trieste S.C.p.A.. All rights reserved.   */
/*                                                                         */
/*                                                                         */
/* This file is part of Pore3D, a software library for quantitative        */
/* analysis of 3D (volume) images.                                         */
/*                                                                         */
/* Pore3D is free software: you can redistribute it and/or modify it       */
/* under the terms of the GNU General Public License as published by the   */
/* Free Software Foundation, either version 3 of the License, or (at your  */
/* option) any later version.                                              */
/*                                                                         */
/* Pore3D is distributed in the hope that it will be useful, but WITHOUT   */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   */
/* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License    */
/* for more details.                                                       */
/*                                                                         */
/* You should have received a copy of the GNU General Public License       */
/* along with Pore3D. If not, see <http://www.gnu.org/licenses/>.          */
/*                                                                         */
/***************************************************************************/

//
// Author: Francesco Brun
// Last modified: Sept, 28th 2016
//

#include <stdlib.h>
#include <omp.h>

#include "p3dSkel.h"
#include "p3dTime.h"

#include "Common/p3dUtils.h"

#define VISITED 128

int p3dIterativeSkeletonPruning(   
	unsigned char* in_im,		// IN: Input (binary) skeleton
	unsigned char* out_im,		// OUT: Labeled skeleton
	const int dimx,
	const int dimy, 
	const int dimz,
	const int thresh,
	int (*wr_log)(const char*, ...)
	)
{
	coords_list_t list; 		// FIFO data structure for coords storage
	coords_t coords;			// Current coords

	int i,j,k;
	int a,b,c;
	int ct;
	int pruned;

	// Dimensions for padded/cropped volumes:
	int a_dimx, a_dimy, a_dimz;
	int a_rad;

	// Padded and cropped temporary input and output:
	unsigned char* tmp_im;
	unsigned char* tmp_im2;
	
	// Counters:
	int neigh, length, noMoreChanges, curr_th;

    /*char auth_code;

    //
    // Authenticate:
    //
    auth_code = authenticate("p3dIterativeSkeletonPruning");
    if (auth_code == '0') goto AUTH_ERROR;*/

	// Start tracking computational time:
	if (wr_log != NULL)
	{
		p3dResetStartTime(); 
		wr_log ("Pore3D - Performing iterative skeleton pruning..." );
		wr_log ("\tPruning threshold: %d voxels.", thresh );
	}

	// Create temporary input replicate-padded:
	a_rad = 1;

	// Compute dimensions of padded REV:
	a_dimx = dimx + a_rad*2;
	a_dimy = dimy + a_rad*2;
	a_dimz = dimz + a_rad*2;

	// Initialize input:
	P3D_TRY( tmp_im = (unsigned char*) malloc( a_dimx*a_dimy*a_dimz*sizeof(unsigned char) ) );
	P3D_TRY( p3dZeroPadding3D_uchar2uchar ( in_im, tmp_im, dimx, dimy, dimz, a_rad ) );
	
	P3D_TRY( tmp_im2 = (unsigned char*) malloc( a_dimx*a_dimy*a_dimz*sizeof(unsigned char) ) );
	memcpy ( tmp_im2, tmp_im, a_dimx*a_dimy*a_dimz*sizeof(unsigned char) );


	// Initialize number of pruned objects:
	pruned = 0;

	// Loop until user-specified thresh:
	for ( curr_th = 1; curr_th <= thresh; curr_th++ )
	{
		// Set the flag for iterative version of pruning:
		noMoreChanges = 1;

		// Loop for iterations:
		while ( noMoreChanges > 0)
		{	
			// Clear the flag for iterations (it will be re-set if pruning occurs):
			noMoreChanges = 0;

			// Volume scanning:
			#pragma omp parallel for private(a, b, i, j, k, ct, coords, list, neigh, length) reduction ( + : noMoreChanges )
			for( c = a_rad; c < (a_dimz - a_rad); c++ )  
				for( b = a_rad; b < (a_dimy - a_rad); b++ )
					for( a = a_rad; a < (a_dimx - a_rad); a++ )
					{
						// If we're on a skeleton voxel:
						if ( tmp_im[ I( a, b, c, a_dimx, a_dimy ) ] == OBJECT ) 					
						{
							// Check Neighborhood:
							ct = countNeighbors ( tmp_im, a_dimx, a_dimy, a_dimz, a, b, c );

							// Is an end point?
							if (ct == 1)
							{
								// Initialize variables:
								coords_list_init(&list);

								// End point found:
								coords.x = a;
								coords.y = b;
								coords.z = c;

								// Start tracking length:
								length = 0;

								do
								{
									// Push the voxel into list:
									coords_list_push(&list, coords);

									// Get coords:
									k = coords.z;
									j = coords.y;
									i = coords.x;	

									// Temporary delete voxel
									tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = BACKGROUND;

									// Determine number of neighbors:
									neigh = findNeighbor(tmp_im, a_dimx, a_dimy, a_dimz, i, j, k, &coords); 

									// Increment counter of branch length:
									length++;

								} while ( neigh == 1 );
								
								// At this point, we're on last voxel of node-to-end branch (ct > 1) or we 
								// completely scanned a end-to-end branch (ct == 0). In the first case we 
								// need to take care whether last voxel is a simple point or not:
								if ( neigh > 1 )
								{	
									if ( isSimplePoint(tmp_im, a_dimx, a_dimy, a_dimz, i, j, k ) == P3D_FALSE )
									{
										// Reset voxel:
										tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = OBJECT;

										// Remove coordinates from the list:
										coords = coords_list_pop(&list);	

										// Increment counter of branch length:
										length--;
									}
								}		


								// Check if scanned branch needs to be pruned (negative condition):
								if ( length > curr_th )
								{
									// Restore branch:
									while( coords_list_isempty(list) == P3D_FALSE )
									{
										// Get coordinates from the list:
										coords = coords_list_pop(&list);							
										
										k = coords.z;
										j = coords.y;
										i = coords.x;	

										// Re-assign voxel:
										tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = OBJECT; // NODE-TO-END Label
									}

									// Set the endpoint as VISITED:
									tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = VISITED; // END-POINT Label
								}
								else
								{
									// Changes performed:
									noMoreChanges++;

									// Count the branches pruned:
									pruned++;

									// Delete the branch from tmp2 and restore it from tmp1:
									while( coords_list_isempty(list) == P3D_FALSE )
									{
										// Get coordinates from the list:
										coords = coords_list_pop(&list);	

										k = coords.z;
										j = coords.y;
										i = coords.x;	

										// Delete the voxel:
										tmp_im2[ I(i, j, k, a_dimx, a_dimy) ] = BACKGROUND; 

										// Re-assign voxel:
										tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = OBJECT; 
									}
								}
							} // end of cycle on each endpoint
						
							// Is an isolated voxel?
							else if (ct == 0)
							{
								// Isolated voxel removal:
								tmp_im2[ I(a, b, c, a_dimx, a_dimy) ] = BACKGROUND;
							}
						}
					}

			// Copy tmp_im2 into tmp_im:
			memcpy ( tmp_im, tmp_im2, a_dimx*a_dimy*a_dimz*sizeof(unsigned char) );
		}
	}


	// Crop output:
	P3D_TRY( p3dCrop3D_uchar2uchar ( tmp_im2, out_im, a_dimx, a_dimy, a_dimz, a_rad ) );	
	
	// Print elapsed time (if required):
	if (wr_log != NULL)
	{	
		wr_log ("\tNumber of pruned branches: %d.", pruned );
		wr_log ("Pore3D - Iterative skeleton pruning performed successfully in %dm%0.3fs.", p3dGetElapsedTime_min(), p3dGetElapsedTime_sec());
	}	
	
    // Release resources:		
	if ( tmp_im != NULL ) free(tmp_im);	
	if ( tmp_im2 != NULL ) free(tmp_im2);	

	// Return OK:
	return P3D_SUCCESS;
	

MEM_ERROR:

	if (wr_log != NULL)
	{
		wr_log ("Pore3D - Not enough (contiguous) memory. Program will exit.");
	}

    // Release resources:	
	if (tmp_im != NULL) free (tmp_im);
	if (tmp_im2 != NULL) free (tmp_im2);

	
	return P3D_MEM_ERROR;
        
    /*    AUTH_ERROR:

    if (wr_log != NULL) {
        wr_log("Pore3D - Authentication error: %s. Program will exit.", auth_code);
    }

    return P3D_AUTH_ERROR;*/
}



