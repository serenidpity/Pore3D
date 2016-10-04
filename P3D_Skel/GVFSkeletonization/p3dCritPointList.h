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

/********************************************************************************
 * File:			p3dCritPointList.h
 *
 * Description:		Header file for the implementation of a dynamic structure
 *					with LIFO (Last-In-First-Out) policy (list or stack) 
 *					containing CritPointT elements (see p3dCritPointT.h)
 *					Duplicate elements allowed.
 *
 * Interfaces:		crit_point_list_init
 *					crit_point_list_push
 *					crit_point_list_pop
 *					crit_point_list_isempty
 *					crit_point_list_toarray
 *
 * Author:			FB
 *
 ********************************************************************************/
#include "p3dCritPointT.h"

/*
	Type definitions:
*/

#ifndef CRIT_POINT_LIST_T_DEFINED
	#define CRIT_POINT_LIST_T_DEFINED

	struct crit_point_lelem_t {
		crit_point_t elem;
		struct crit_point_lelem_t	*next;
	};

	typedef struct crit_point_lelem_t crit_point_list_elem_t;
		
	typedef crit_point_list_elem_t* crit_point_list_t;

#endif




/*
	Interfaces:
*/

void crit_point_list_init (crit_point_list_t *list);

/********************************************************************************
 * Function:		crit_point_list_push
 * 
 * Description:		Push the specified element into the list.
 *
 * Input(s):		coords_list_t*		- The list to extend
 *					coords_t			- The element to push into the list
 *	
 * Output:			P3D_SUCCESS if element is successfully pushed into the list
 *					P3D_MEM_ERROR if there is not enough memory for the additional 
 *                  element
 ********************************************************************************/
int crit_point_list_push (crit_point_list_t *list, crit_point_t item);

crit_point_t crit_point_list_pop (crit_point_list_t *list);

int crit_point_list_isempty(crit_point_list_t list);



