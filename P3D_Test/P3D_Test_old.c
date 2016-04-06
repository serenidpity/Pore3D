#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <args.h>
#include <stdarg.h>
#include <pthread.h>
#include <limits.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include <math.h>
#include <limits.h>

#define clamp(a, b1, b2) min(max(a, b1), b2);
#define absd(a) ((a)>(-a)?(a):(-a))
#define pow2(a) (a*a)
#define pow4(a) (pow2(a)*pow2(a))
#define n 3
#define inv3 0.3333333333333333
#define root3 1.7320508075688772
#ifndef min
#define min(a,b)        ((a) < (b) ? (a): (b))
#endif
#ifndef max
#define max(a,b)        ((a) > (b) ? (a): (b))
#endif

#define P3D_FALSE				-1 
#define P3D_TRUE				1 

#define P3D_AUTH_ERROR                          -1
#define P3D_MEM_ERROR                           NULL	/* Leave it NULL for simplify tests */
#define P3D_IO_ERROR                            1
#define P3D_SUCCESS				2	/* Any number */

#define BACKGROUND				0
#define OBJECT					UCHAR_MAX	

// Constants for 3D connectivity:
#define CONN6   711
#define CONN18  712
#define CONN26  713

#define I(i,j,k,N,M)    ( (j)*(N) + (i) + (k)*(N)*(M) )
#define I2(i,j,N) ( (j)*(N) + (i) )
#define MIN(x,y)        (((x) < (y))?(x):(y))
#define MAX(x,y)        (((x) > (y))?(x):(y))

/* A sort of TRY-CATCH constructor: */
#define P3D_TRY( function ) if ( (function) == P3D_MEM_ERROR) { goto MEM_ERROR; }
#define ELEM_SWAP(a,b) { register double t=(a);(a)=(b);(b)=t; }

int p3dZeroPadding2D_8(
	unsigned char* in_im,
	unsigned char* out_im,
	const int dimx, // ncols
	const int dimy, // nrows
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy;
		int i, j;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;

		// Set to zero all values:
		memset(out_im, 0, a_dimx * a_dimy * sizeof (unsigned char));

		// Copy original (internal) values:
		for (j = 0; j < dimy; j++)
			for (i = 0; i < dimx; i++)
				out_im[ I2(i + size, j + size, a_dimx) ] = in_im[ I2(i, j, dimx) ];


		return P3D_SUCCESS;
}

int p3dZeroPadding2D_16(
	unsigned short* in_im,
	unsigned short* out_im,
	const int dimx, // ncols
	const int dimy, // nrows
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy;
		int i, j;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;

		// Set to zero all values:
		memset(out_im, 0, a_dimx * a_dimy * sizeof (unsigned short));

		// Copy original (internal) values:
		for (j = 0; j < dimy; j++)
			for (i = 0; i < dimx; i++)
				out_im[ I2(i + size, j + size, a_dimx) ] = in_im[ I2(i, j, dimx) ];


		return P3D_SUCCESS;
}



int p3dZeroPadding3D_16(
	unsigned short* in_rev,
	unsigned short* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k;

		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;

		// Set to zero all values:
		memset(out_rev, 0, a_dimx * a_dimy * a_dimz * sizeof (unsigned short));


		// Copy original (internal) values:
		for (k = 0; k < dimz; k++)
			for (j = 0; j < dimy; j++)
				for (i = 0; i < dimx; i++)
					out_rev[ I(i + size, j + size, k + size, a_dimx, a_dimy) ] =
					in_rev[ I(i, j, k, dimx, dimy) ];

		// Return OK:
		return P3D_SUCCESS;
}

int p3dReplicatePadding2D_8(
	unsigned char* in_im,
	unsigned char* out_im,
	const int dimx, // ncols
	const int dimy, // nrows
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy;
		int i, j, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;


		// Perform first zero padding:
		p3dZeroPadding2D_8(in_im, out_im, dimx, dimy, size, NULL, NULL);


		// Replicate border values:
		for (ct = size; ct > 0; ct--) {
			// Edges:

			for (i = ct; i < (a_dimx - ct); i++) {
				out_im[ I2(i, ct - 1, a_dimx) ] =
					out_im[ I2(i, ct, a_dimx) ];
				out_im[ I2(i, a_dimy - ct, a_dimx) ] =
					out_im[ I2(i, a_dimy - 1 - ct, a_dimx) ];
			}

			for (j = ct; j < (a_dimy - ct); j++) {
				out_im[ I2(ct - 1, j, a_dimx) ] =
					out_im[ I2(ct, j, a_dimx) ];
				out_im[ I2(a_dimx - ct, j, a_dimx) ] =
					out_im[ I2(a_dimx - 1 - ct, j, a_dimx) ];
			}

			// Corners:

			out_im[ I2(ct - 1, ct - 1, a_dimx)] =
				out_im[ I2(ct, ct, a_dimx) ];

			out_im[ I2(a_dimx - ct, ct - 1, a_dimx)] =
				out_im[ I2(a_dimx - 1 - ct, ct, a_dimx) ];

			out_im[ I2(ct - 1, a_dimy - ct, a_dimx)] =
				out_im[ I2(ct, a_dimy - 1 - ct, a_dimx) ];

			out_im[ I2(a_dimx - ct, a_dimy - ct, a_dimx)] =
				out_im[ I2(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimx) ];

		}

		return P3D_SUCCESS;
}

int p3dReplicatePadding2D_16(
	unsigned short* in_im,
	unsigned short* out_im,
	const int dimx, // ncols
	const int dimy, // nrows
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy;
		int i, j, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;


		// Perform first zero padding:
		p3dZeroPadding2D_16(in_im, out_im, dimx, dimy, size, NULL, NULL);


		// Replicate border values:
		for (ct = size; ct > 0; ct--) {
			// Edges:

			for (i = ct; i < (a_dimx - ct); i++) {
				out_im[ I2(i, ct - 1, a_dimx) ] =
					out_im[ I2(i, ct, a_dimx) ];
				out_im[ I2(i, a_dimy - ct, a_dimx) ] =
					out_im[ I2(i, a_dimy - 1 - ct, a_dimx) ];
			}

			for (j = ct; j < (a_dimy - ct); j++) {
				out_im[ I2(ct - 1, j, a_dimx) ] =
					out_im[ I2(ct, j, a_dimx) ];
				out_im[ I2(a_dimx - ct, j, a_dimx) ] =
					out_im[ I2(a_dimx - 1 - ct, j, a_dimx) ];
			}

			// Corners:

			out_im[ I2(ct - 1, ct - 1, a_dimx)] =
				out_im[ I2(ct, ct, a_dimx) ];

			out_im[ I2(a_dimx - ct, ct - 1, a_dimx)] =
				out_im[ I2(a_dimx - 1 - ct, ct, a_dimx) ];

			out_im[ I2(ct - 1, a_dimy - ct, a_dimx)] =
				out_im[ I2(ct, a_dimy - 1 - ct, a_dimx) ];

			out_im[ I2(a_dimx - ct, a_dimy - ct, a_dimx)] =
				out_im[ I2(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimx) ];

		}

		return P3D_SUCCESS;

}



int p3dReplicatePadding3D_16(
	unsigned short* in_rev,
	unsigned short* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;


		// Perform first zero padding:
		p3dZeroPadding3D_16(in_rev, out_rev, dimx, dimy, dimz, size, NULL, NULL);


		// Replicate border values:
		for (ct = size; ct > 0; ct--) {
			// Faces:

			for (i = ct; i < (a_dimx - ct); i++)
				for (j = ct; j < (a_dimy - ct); j++) {
					out_rev[ I(i, j, ct - 1, a_dimx, a_dimy) ] =
						out_rev[ I(i, j, ct, a_dimx, a_dimy) ];
					out_rev[ I(i, j, a_dimz - ct, a_dimx, a_dimy)] =
						out_rev[ I(i, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
				}

				for (j = ct; j < (a_dimy - ct); j++)
					for (k = ct; k < (a_dimz - ct); k++) {
						out_rev[ I(ct - 1, j, k, a_dimx, a_dimy) ] =
							out_rev[ I(ct, j, k, a_dimx, a_dimy) ];
						out_rev[ I(a_dimx - ct, j, k, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, j, k, a_dimx, a_dimy) ];
					}

					for (i = ct; i < (a_dimx - ct); i++)
						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(i, ct - 1, k, a_dimx, a_dimy) ] =
								out_rev[ I(i, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, k, a_dimx, a_dimy)] =
								out_rev[ I(i, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Edges:

						for (i = ct; i < (a_dimx - ct); i++) {
							out_rev[ I(i, ct - 1, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(i, ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(i, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(i, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(i, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (j = ct; j < (a_dimy - ct); j++) {
							out_rev[ I(ct - 1, j, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, j, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(a_dimx - 1 - ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(ct - 1, ct - 1, k, a_dimx, a_dimy) ] =
								out_rev[ I(ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, ct - 1, k, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, a_dimy - ct, k, a_dimx, a_dimy) ] =
								out_rev[ I(ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, a_dimy - ct, k, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Corners:

						out_rev[ I(ct - 1, ct - 1, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


						out_rev[ I(a_dimx - ct, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


		}

		// Return OK:
		return P3D_SUCCESS;
}


int _p3dZeroPadding3D_float(
	float* in_rev,
	float* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k;

		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;

		// Set to zero all values:
		memset(out_rev, 0, a_dimx * a_dimy * a_dimz * sizeof (float));


		// Copy original (internal) values:
#pragma omp parallel for private(i, j)
		for (k = 0; k < dimz; k++)
			for (j = 0; j < dimy; j++)
				for (i = 0; i < dimx; i++)
					out_rev[ I(i + size, j + size, k + size, a_dimx, a_dimy) ] =
					(float) (in_rev[ I(i, j, k, dimx, dimy) ]);

		// Return OK:
		return P3D_SUCCESS;
}

int _p3dReplicatePadding3D_float(
	float* in_rev,
	float* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;


		// Perform first zero padding:
		_p3dZeroPadding3D_float(in_rev, out_rev, dimx, dimy, dimz, size);


		// Replicate border values:
#pragma omp parallel for private(i, j, k)
		for (ct = size; ct > 0; ct--) {
			// Faces:

			for (i = ct; i < (a_dimx - ct); i++)
				for (j = ct; j < (a_dimy - ct); j++) {
					out_rev[ I(i, j, ct - 1, a_dimx, a_dimy) ] =
						out_rev[ I(i, j, ct, a_dimx, a_dimy) ];
					out_rev[ I(i, j, a_dimz - ct, a_dimx, a_dimy)] =
						out_rev[ I(i, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
				}

				for (j = ct; j < (a_dimy - ct); j++)
					for (k = ct; k < (a_dimz - ct); k++) {
						out_rev[ I(ct - 1, j, k, a_dimx, a_dimy) ] =
							out_rev[ I(ct, j, k, a_dimx, a_dimy) ];
						out_rev[ I(a_dimx - ct, j, k, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, j, k, a_dimx, a_dimy) ];
					}

					for (i = ct; i < (a_dimx - ct); i++)
						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(i, ct - 1, k, a_dimx, a_dimy) ] =
								out_rev[ I(i, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, k, a_dimx, a_dimy)] =
								out_rev[ I(i, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Edges:

						for (i = ct; i < (a_dimx - ct); i++) {
							out_rev[ I(i, ct - 1, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(i, ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(i, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(i, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(i, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (j = ct; j < (a_dimy - ct); j++) {
							out_rev[ I(ct - 1, j, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, j, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(a_dimx - 1 - ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(ct - 1, ct - 1, k, a_dimx, a_dimy) ] =
								out_rev[ I(ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, ct - 1, k, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, a_dimy - ct, k, a_dimx, a_dimy) ] =
								out_rev[ I(ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, a_dimy - ct, k, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Corners:

						out_rev[ I(ct - 1, ct - 1, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


						out_rev[ I(a_dimx - ct, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


		}

		// Return OK:
		return P3D_SUCCESS;
}

int _p3dZeroPadding3D_uchar2float(
	unsigned char* in_rev,
	float* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k;

		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;

		// Set to zero all values:
		memset(out_rev, 0, a_dimx * a_dimy * a_dimz * sizeof (float));


		// Copy original (internal) values:
#pragma omp parallel for private(i, j)
		for (k = 0; k < dimz; k++)
			for (j = 0; j < dimy; j++)
				for (i = 0; i < dimx; i++)
					out_rev[ I(i + size, j + size, k + size, a_dimx, a_dimy) ] =
					(float) (in_rev[ I(i, j, k, dimx, dimy) ]);

		// Return OK:
		return P3D_SUCCESS;
}

int _p3dReplicatePadding3D_uchar2float(
	unsigned char* in_rev,
	float* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;


		// Perform first zero padding:
		_p3dZeroPadding3D_uchar2float(in_rev, out_rev, dimx, dimy, dimz, size);


		// Replicate border values:
#pragma omp parallel for private(i, j, k)
		for (ct = size; ct > 0; ct--) {
			// Faces:

			for (i = ct; i < (a_dimx - ct); i++)
				for (j = ct; j < (a_dimy - ct); j++) {
					out_rev[ I(i, j, ct - 1, a_dimx, a_dimy) ] =
						(float) out_rev[ I(i, j, ct, a_dimx, a_dimy) ];
					out_rev[ I(i, j, a_dimz - ct, a_dimx, a_dimy)] =
						(float) out_rev[ I(i, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
				}

				for (j = ct; j < (a_dimy - ct); j++)
					for (k = ct; k < (a_dimz - ct); k++) {
						out_rev[ I(ct - 1, j, k, a_dimx, a_dimy) ] =
							(float) out_rev[ I(ct, j, k, a_dimx, a_dimy) ];
						out_rev[ I(a_dimx - ct, j, k, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, j, k, a_dimx, a_dimy) ];
					}

					for (i = ct; i < (a_dimx - ct); i++)
						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(i, ct - 1, k, a_dimx, a_dimy) ] =
								(float) out_rev[ I(i, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, k, a_dimx, a_dimy)] =
								(float) out_rev[ I(i, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Edges:

						for (i = ct; i < (a_dimx - ct); i++) {
							out_rev[ I(i, ct - 1, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(i, ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(i, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(i, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(i, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (j = ct; j < (a_dimy - ct); j++) {
							out_rev[ I(ct - 1, j, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, j, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(a_dimx - 1 - ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(a_dimx - 1 - ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(ct - 1, ct - 1, k, a_dimx, a_dimy) ] =
								(float) out_rev[ I(ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, ct - 1, k, a_dimx, a_dimy)] =
								(float) out_rev[ I(a_dimx - 1 - ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, a_dimy - ct, k, a_dimx, a_dimy) ] =
								(float) out_rev[ I(ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, a_dimy - ct, k, a_dimx, a_dimy)] =
								(float) out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Corners:

						out_rev[ I(ct - 1, ct - 1, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


						out_rev[ I(a_dimx - ct, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


		}

		// Return OK:
		return P3D_SUCCESS;
}

int _p3dZeroPadding3D_ushort2float(
	unsigned short* in_rev,
	float* out_rev,
	const int dimx,
	const int dimy,
	const int dimz,
	const int size
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k;

		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;

		// Set to zero all values:
		memset(out_rev, 0, a_dimx * a_dimy * a_dimz * sizeof (float));


		// Copy original (internal) values:
#pragma omp parallel for private(i, j)
		for (k = 0; k < dimz; k++)
			for (j = 0; j < dimy; j++)
				for (i = 0; i < dimx; i++)
					out_rev[ I(i + size, j + size, k + size, a_dimx, a_dimy) ] =
					(float) (in_rev[ I(i, j, k, dimx, dimy) ]);

		// Return OK:
		return P3D_SUCCESS;
}

int _p3dReplicatePadding3D_ushort2float(
	unsigned short* in_rev,
	float* out_rev,
	const int dimx,
	const int dimy,
	const int dimz,
	const int size
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;


		// Perform first zero padding:
		_p3dZeroPadding3D_ushort2float(in_rev, out_rev, dimx, dimy, dimz, size);


		// Replicate border values:
#pragma omp parallel for private(i, j, k)
		for (ct = size; ct > 0; ct--) {
			// Faces:

			for (i = ct; i < (a_dimx - ct); i++)
				for (j = ct; j < (a_dimy - ct); j++) {
					out_rev[ I(i, j, ct - 1, a_dimx, a_dimy) ] =
						(float) out_rev[ I(i, j, ct, a_dimx, a_dimy) ];
					out_rev[ I(i, j, a_dimz - ct, a_dimx, a_dimy)] =
						(float) out_rev[ I(i, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
				}

				for (j = ct; j < (a_dimy - ct); j++)
					for (k = ct; k < (a_dimz - ct); k++) {
						out_rev[ I(ct - 1, j, k, a_dimx, a_dimy) ] =
							(float) out_rev[ I(ct, j, k, a_dimx, a_dimy) ];
						out_rev[ I(a_dimx - ct, j, k, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, j, k, a_dimx, a_dimy) ];
					}

					for (i = ct; i < (a_dimx - ct); i++)
						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(i, ct - 1, k, a_dimx, a_dimy) ] =
								(float) out_rev[ I(i, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, k, a_dimx, a_dimy)] =
								(float) out_rev[ I(i, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Edges:

						for (i = ct; i < (a_dimx - ct); i++) {
							out_rev[ I(i, ct - 1, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(i, ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(i, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(i, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(i, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (j = ct; j < (a_dimy - ct); j++) {
							out_rev[ I(ct - 1, j, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, j, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, ct - 1, a_dimx, a_dimy) ] =
								(float) out_rev[ I(a_dimx - 1 - ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, a_dimz - ct, a_dimx, a_dimy)] =
								(float) out_rev[ I(a_dimx - 1 - ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(ct - 1, ct - 1, k, a_dimx, a_dimy) ] =
								(float) out_rev[ I(ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, ct - 1, k, a_dimx, a_dimy)] =
								(float) out_rev[ I(a_dimx - 1 - ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, a_dimy - ct, k, a_dimx, a_dimy) ] =
								(float) out_rev[ I(ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, a_dimy - ct, k, a_dimx, a_dimy)] =
								(float) out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Corners:

						out_rev[ I(ct - 1, ct - 1, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


						out_rev[ I(a_dimx - ct, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							(float) out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


		}

		// Return OK:
		return P3D_SUCCESS;
}



struct options {
	double T;
	double dt;
	double sigma;
	double rho;
	double C;
	double m;
	double alpha;
	int eigenmode;
	double lambda_e;
	double lambda_h;
	double lambda_c;
};

__inline int mindex3(int x, int y, int z, int sizx, int sizy) {
	return z * sizx * sizy + y * sizx + x;
}

float * mallocf(int a) {
	float *ptr = (float*) calloc(a, sizeof (float));
	//if (ptr == NULL) { mexErrMsgTxt("Out of Memory"); }
	return ptr;
}

void GaussianFiltering3D_float(
	float* in_im,
	float* out_im,
	int* dimsI,
	double sigma,
	double size
	) {
		// Padded input and related dims:
		float* tmp_im = NULL;
		float* kernel = NULL;

		int dimx = dimsI[0];
		int dimy = dimsI[1];
		int dimz = dimsI[2];
		int a_dimx, a_dimy, a_dimz;
		int i, j, k;
		int x, y, z;
		int ct, a_rad, a_size;
		double d;
		double f, sum, sum_w;

		// Set kernel size and variance:
		if (size < 1.0)
			a_rad = 1;
		else
			a_rad = ceil(size / 2);
		a_size = 2 * a_rad + 1;

		d = (2.0 * sigma * sigma);


		// Compute dimensions of padded volume:
		a_dimx = dimx + a_rad * 2;
		a_dimy = dimy + a_rad * 2;
		a_dimz = dimz + a_rad * 2;


		// Initialize input:
		tmp_im = (float*) malloc(a_dimx*a_dimy*a_dimz*sizeof (float));
		kernel = (float*) malloc(a_size*sizeof(float));

		_p3dReplicatePadding3D_float(in_im, tmp_im, dimx, dimy, dimz, a_rad);

		ct = 0;
		sum_w = 0.0;

		// Create gausssian kernel:
		for (x = (-a_rad); x <= a_rad; x++) {

			f = (x)*(x) / d;
			kernel[ct] = exp(-f);
			sum_w = sum_w + kernel[ct++];
		}
		ct = 0;
		// Normalize kernel
		for (x = (-a_rad); x <= a_rad; x++) 
			kernel[ct] = kernel[ct++]/sum_w;

		// X-direction scanning
#pragma omp parallel for private(i, j, x, sum, ct)
		for (k = a_rad; k < (a_dimz - a_rad); k++)
			for (j = a_rad; j < (a_dimy - a_rad); j++)
				for (i = a_rad; i < (a_dimx - a_rad); i++) {
					sum = 0.0; 
					ct = 0;

					// Process kernel:
					for (x = (i - a_rad); x <= (i + a_rad); x++) {
						// Gausssian kernel:                       
						sum = sum + kernel[ct++] * tmp_im[ I(x, j, k, a_dimx, a_dimy) ];
					}

					// Set out voxel with the mean of the sorted temporary array:                
					tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = sum;
				}

				// Y-direction scanning
#pragma omp parallel for private(i, j, y, sum, ct)
				for (k = a_rad; k < (a_dimz - a_rad); k++)
					for (j = a_rad; j < (a_dimy - a_rad); j++)
						for (i = a_rad; i < (a_dimx - a_rad); i++) {
							sum = 0.0;
							ct = 0;

							// Process kernel:
							for (y = (j - a_rad); y <= (j + a_rad); y++) {
								// Gausssian kernel:
								sum = sum + kernel[ct++] * tmp_im[ I(i, y, k, a_dimx, a_dimy) ];
							}

							// Set out voxel with the mean of the sorted temporary array:             
							tmp_im[ I(i, j, k, a_dimx, a_dimy) ] = sum; 
						}

						// Z-direction scanning
#pragma omp parallel for private(i, j, z, sum, ct)
						for (k = a_rad; k < (a_dimz - a_rad); k++)
							for (j = a_rad; j < (a_dimy - a_rad); j++)
								for (i = a_rad; i < (a_dimx - a_rad); i++) {
									sum = 0.0;
									ct = 0;

									// Process kernel:
									for (z = (k - a_rad); z <= (k + a_rad); z++) {
										// Gausssian kernel:
										sum = sum + kernel[ct++] * tmp_im[ I(i, j, z, a_dimx, a_dimy) ];
									}

									// Set out voxel with the mean of the sorted temporary array:           
									out_im[ I(i-a_rad, j-a_rad, k-a_rad, dimx, dimy) ] = sum; 
								}



								// Release resources:
								if (tmp_im != NULL) free(tmp_im);
								if (kernel != NULL) free(kernel);
}

void gradient3Dx_float(float *I, int *sizeI, float *Ix) {
	int x, y, z;
	int xp, xn, yp, yn;
	int i;
	int indexn, indexc, indexp;
	float *Irow, *Islices;
	int nSlice;
	int offsetz, offset_slice;
	int slice_select = 0;
	int slice_select_p1 = 0;
	int slice_select_p2 = 0;

	const float smoothfilter[3] = {0.187500f, 0.625000f, 0.187500f};
	const float derivafilter[3] = {-0.5f, 0.0f, 0.5f};

	nSlice = sizeI[0] * sizeI[1];
	Islices = mallocf(4 * nSlice);
	Irow = mallocf(sizeI[0]);

	//#pragma omp parallel for private (offsetz, offset_slice, y, yn, yp, indexn, indexc, indexp, x, xn, xp, i, slice_selec slice_select_p1, slice_select_p2)
	for (z = 0; z < sizeI[2]; z++) {

		offsetz = nSlice*z;
		offset_slice = nSlice*slice_select;

		for (y = 0; y < sizeI[1]; y++) {
			/* Smooth y - direction  */
			yn = max(y - 1, 0);
			yp = min(y + 1, sizeI[1] - 1);

			indexn = yn * sizeI[0] + offsetz;
			indexc = y * sizeI[0] + offsetz;
			indexp = yp * sizeI[0] + offsetz;

			for (x = 0; x < sizeI[0]; x++) {
				Irow[x] = smoothfilter[0] * I[indexn + x];
				Irow[x] += smoothfilter[1] * I[indexc + x];
				Irow[x] += smoothfilter[2] * I[indexp + x];
			}

			indexc = y * sizeI[0] + offset_slice;
			/*  Gradient in x - direction  */
			for (x = 0; x < sizeI[0]; x++) {
				xn = max(x - 1, 0);
				xp = min(x + 1, sizeI[0] - 1);
				Islices[indexc + x] = derivafilter[0] * Irow[xn] + derivafilter[1] * Irow[x] + derivafilter[2] * Irow[xp];
			}
		}

		/* Smooth in z - direction  */
		if (z == 1) /* Forward          */ {
			indexn = slice_select_p1*nSlice;
			indexc = slice_select_p1*nSlice;
			indexp = slice_select*nSlice;
			for (i = 0; i < nSlice; i++) {
				Ix[i] = smoothfilter[0] * Islices[i + indexn] + smoothfilter[1] * Islices[i + indexc] + smoothfilter[2] * Islices[i + indexp];
			}
		} else if (z > 1) /* Central  */ {
			indexn = slice_select_p2*nSlice;
			indexc = slice_select_p1*nSlice;
			indexp = slice_select*nSlice;
			offsetz = nSlice * (z - 1);
			for (i = 0; i < nSlice; i++) {
				Ix[offsetz + i] = smoothfilter[0] * Islices[i + indexn] + smoothfilter[1] * Islices[i + indexc] + smoothfilter[2] * Islices[i + indexp];
			}
		}

		if (z == (sizeI[2] - 1)) /* Backward  */ {
			indexn = slice_select_p1*nSlice;
			indexc = slice_select*nSlice;
			indexp = slice_select*nSlice;
			offsetz = nSlice*z;
			for (i = 0; i < nSlice; i++) {
				Ix[offsetz + i] = smoothfilter[0] * Islices[i + indexn] + smoothfilter[1] * Islices[i + indexc] + smoothfilter[2] * Islices[i + indexp];
			}
		}

		slice_select_p2 = slice_select_p1;
		slice_select_p1 = slice_select;
		slice_select++;
		if (slice_select > 3) {
			slice_select = 0;
		}

	}
	free(Irow);
	free(Islices);
	//}
}

void gradient3Dy_float(float *I, int *sizeI, float *Iy) {
	int x, y, z;
	int xp, xn, yp, yn;
	int i;
	int indexn, indexc, indexp;
	float *Irow, *Islices;
	int nSlice;
	int offsetz, offset_slice;
	int slice_select = 0, slice_select_p1 = 0, slice_select_p2 = 0;

	const float smoothfilter[3] = {0.187500f, 0.625000f, 0.187500f};
	const float derivafilter[3] = {-0.5f, 0.0f, 0.5f};

	nSlice = sizeI[0] * sizeI[1];
	Islices = mallocf(4 * nSlice);
	Irow = mallocf(sizeI[0]);

	for (z = 0; z < sizeI[2]; z++) {
		offsetz = nSlice*z;
		offset_slice = nSlice*slice_select;

		for (y = 0; y < sizeI[1]; y++) {
			/* Smooth y - direction  */
			yn = max(y - 1, 0);
			yp = min(y + 1, sizeI[1] - 1);

			indexn = yn * sizeI[0] + offsetz;
			indexc = y * sizeI[0] + offsetz;
			indexp = yp * sizeI[0] + offsetz;

			for (x = 0; x < sizeI[0]; x++) {
				Irow[x] = derivafilter[0] * I[indexn + x];
				Irow[x] += derivafilter[1] * I[indexc + x];
				Irow[x] += derivafilter[2] * I[indexp + x];
			}

			indexc = y * sizeI[0] + offset_slice;
			/*  Gradient in x - direction  */
			for (x = 0; x < sizeI[0]; x++) {
				xn = max(x - 1, 0);
				xp = min(x + 1, sizeI[0] - 1);
				Islices[indexc + x] = smoothfilter[0] * Irow[xn] + smoothfilter[1] * Irow[x] + smoothfilter[2] * Irow[xp];
			}
		}

		/* Smooth in z - direction  */
		if (z == 1) /* Forward          */ {
			indexn = slice_select_p1*nSlice;
			indexc = slice_select_p1*nSlice;
			indexp = slice_select*nSlice;
			for (i = 0; i < nSlice; i++) {
				Iy[i] = smoothfilter[0] * Islices[i + indexn] + smoothfilter[1] * Islices[i + indexc] + smoothfilter[2] * Islices[i + indexp];
			}
		} else if (z > 1) /* Central  */ {
			indexn = slice_select_p2*nSlice;
			indexc = slice_select_p1*nSlice;
			indexp = slice_select*nSlice;
			offsetz = nSlice * (z - 1);
			for (i = 0; i < nSlice; i++) {
				Iy[offsetz + i] = smoothfilter[0] * Islices[i + indexn] + smoothfilter[1] * Islices[i + indexc] + smoothfilter[2] * Islices[i + indexp];
			}
		}

		if (z == (sizeI[2] - 1)) /* Backward  */ {
			indexn = slice_select_p1*nSlice;
			indexc = slice_select*nSlice;
			indexp = slice_select*nSlice;
			offsetz = nSlice*z;
			for (i = 0; i < nSlice; i++) {
				Iy[offsetz + i] = smoothfilter[0] * Islices[i + indexn] + smoothfilter[1] * Islices[i + indexc] + smoothfilter[2] * Islices[i + indexp];
			}
		}

		slice_select_p2 = slice_select_p1;
		slice_select_p1 = slice_select;
		slice_select++;
		if (slice_select > 3) {
			slice_select = 0;
		}
	}
	free(Irow);
	free(Islices);
}

void gradient3Dz_float(float *I, int *sizeI, float *Iz) {
	int x, y, z;
	int xp, xn, yp, yn;
	int i;
	int indexn, indexc, indexp;
	float *Irow, *Islices;
	int nSlice;
	int offsetz, offset_slice;
	int slice_select = 0, slice_select_p1 = 0, slice_select_p2 = 0;

	const float smoothfilter[3] = {0.187500f, 0.625000f, 0.187500f};
	const float derivafilter[3] = {-0.5f, 0.0f, 0.5f};

	nSlice = sizeI[0] * sizeI[1];
	Islices = mallocf(4 * nSlice);
	Irow = mallocf(sizeI[0]);

	for (z = 0; z < sizeI[2]; z++) {
		offsetz = nSlice*z;
		offset_slice = nSlice*slice_select;

		for (y = 0; y < sizeI[1]; y++) {
			/* Smooth y - direction  */
			yn = max(y - 1, 0);
			yp = min(y + 1, sizeI[1] - 1);

			indexn = yn * sizeI[0] + offsetz;
			indexc = y * sizeI[0] + offsetz;
			indexp = yp * sizeI[0] + offsetz;

			for (x = 0; x < sizeI[0]; x++) {
				Irow[x] = smoothfilter[0] * I[indexn + x];
				Irow[x] += smoothfilter[1] * I[indexc + x];
				Irow[x] += smoothfilter[2] * I[indexp + x];
			}

			indexc = y * sizeI[0] + offset_slice;
			/*  Gradient in x - direction  */
			for (x = 0; x < sizeI[0]; x++) {
				xn = max(x - 1, 0);
				xp = min(x + 1, sizeI[0] - 1);
				Islices[indexc + x] = smoothfilter[0] * Irow[xn] + smoothfilter[1] * Irow[x] + smoothfilter[2] * Irow[xp];
			}
		}

		/* Smooth in z - direction  */
		if (z == 1) /* Forward          */ {
			indexn = slice_select_p1*nSlice;
			indexc = slice_select_p1*nSlice;
			indexp = slice_select*nSlice;
			for (i = 0; i < nSlice; i++) {
				Iz[i] = derivafilter[0] * Islices[i + indexn] + derivafilter[1] * Islices[i + indexc] + derivafilter[2] * Islices[i + indexp];
			}
		} else if (z > 1) /* Central  */ {
			indexn = slice_select_p2*nSlice;
			indexc = slice_select_p1*nSlice;
			indexp = slice_select*nSlice;
			offsetz = nSlice * (z - 1);
			for (i = 0; i < nSlice; i++) {
				Iz[offsetz + i] = derivafilter[0] * Islices[i + indexn] + derivafilter[1] * Islices[i + indexc] + derivafilter[2] * Islices[i + indexp];
			}
		}

		if (z == (sizeI[2] - 1)) /* Backward  */ {
			indexn = slice_select_p1*nSlice;
			indexc = slice_select*nSlice;
			indexp = slice_select*nSlice;
			offsetz = nSlice*z;
			for (i = 0; i < nSlice; i++) {
				Iz[offsetz + i] = derivafilter[0] * Islices[i + indexn] + derivafilter[1] * Islices[i + indexc] + derivafilter[2] * Islices[i + indexp];
			}
		}

		slice_select_p2 = slice_select_p1;
		slice_select_p1 = slice_select;
		slice_select++;
		if (slice_select > 3) {
			slice_select = 0;
		}

	}
	free(Irow);
	free(Islices);
}

void StructureTensor3D(float *ux, float *uy, float *uz, float **J, int *dimsu, double rho) {
	int npixelsu;
	int i;
	float *Jxx, *Jxy, *Jxz, *Jyy, *Jyz, *Jzz; /* Structure tensor */

	npixelsu = dimsu[0] * dimsu[1] * dimsu[2];

	Jxx = mallocf(npixelsu);
	Jyy = mallocf(npixelsu);
	Jzz = mallocf(npixelsu);
	Jxy = mallocf(npixelsu);
	Jxz = mallocf(npixelsu);
	Jyz = mallocf(npixelsu);

	/* J(grad u_sigma) */
#pragma omp parallel for
	for (i = 0; i < npixelsu; i++) {
		Jxx[i] = ux[i] * ux[i];
		Jxy[i] = ux[i] * uy[i];
		Jxz[i] = ux[i] * uz[i];
		Jyy[i] = uy[i] * uy[i];
		Jyz[i] = uy[i] * uz[i];
		Jzz[i] = uz[i] * uz[i];
	}

	/* Do the gaussian smoothing */
	J[0] = mallocf(npixelsu);
	GaussianFiltering3D_float(Jxx, J[0], dimsu, rho, 4 * rho);
	free(Jxx);
	J[1] = mallocf(npixelsu);
	GaussianFiltering3D_float(Jyy, J[1], dimsu, rho, 4 * rho);
	free(Jyy);
	J[2] = mallocf(npixelsu);
	GaussianFiltering3D_float(Jzz, J[2], dimsu, rho, 4 * rho);
	free(Jzz);
	J[3] = mallocf(npixelsu);
	GaussianFiltering3D_float(Jxy, J[3], dimsu, rho, 4 * rho);
	free(Jxy);
	J[4] = mallocf(npixelsu);
	GaussianFiltering3D_float(Jxz, J[4], dimsu, rho, 4 * rho);
	free(Jxz);
	J[5] = mallocf(npixelsu);
	GaussianFiltering3D_float(Jyz, J[5], dimsu, rho, 4 * rho);
	free(Jyz);
}

/* domain Java Matrix library JAMA. */
static double hypot2(double x, double y) {
	return sqrt(x * x + y * y);
}

/* Symmetric Householder reduction to tridiagonal form. */
static void tred2(double V[n][n], double d[n], double e[n]) {

	/*  This is derived from the Algol procedures tred2 by */
	/*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
	/*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
	/*  Fortran subroutine in EISPACK. */
	int i, j, k;
	double scale;
	double f, g, h;
	double hh;
	for (j = 0; j < n; j++) {
		d[j] = V[n - 1][j];
	}

	/* Householder reduction to tridiagonal form. */

	for (i = n - 1; i > 0; i--) {
		/* Scale to avoid under/overflow. */
		scale = 0.0;
		h = 0.0;
		for (k = 0; k < i; k++) {
			scale = scale + fabs(d[k]);
		}
		if (scale == 0.0) {
			e[i] = d[i - 1];
			for (j = 0; j < i; j++) {
				d[j] = V[i - 1][j];
				V[i][j] = 0.0;
				V[j][i] = 0.0;
			}
		} else {

			/* Generate Householder vector. */

			for (k = 0; k < i; k++) {
				d[k] /= scale;
				h += d[k] * d[k];
			}
			f = d[i - 1];
			g = sqrt(h);
			if (f > 0) {
				g = -g;
			}
			e[i] = scale * g;
			h = h - f * g;
			d[i - 1] = f - g;
			for (j = 0; j < i; j++) {
				e[j] = 0.0;
			}

			/* Apply similarity transformation to remaining columns. */

			for (j = 0; j < i; j++) {
				f = d[j];
				V[j][i] = f;
				g = e[j] + V[j][j] * f;
				for (k = j + 1; k <= i - 1; k++) {
					g += V[k][j] * d[k];
					e[k] += V[k][j] * f;
				}
				e[j] = g;
			}
			f = 0.0;
			for (j = 0; j < i; j++) {
				e[j] /= h;
				f += e[j] * d[j];
			}
			hh = f / (h + h);
			for (j = 0; j < i; j++) {
				e[j] -= hh * d[j];
			}
			for (j = 0; j < i; j++) {
				f = d[j];
				g = e[j];
				for (k = j; k <= i - 1; k++) {
					V[k][j] -= (f * e[k] + g * d[k]);
				}
				d[j] = V[i - 1][j];
				V[i][j] = 0.0;
			}
		}
		d[i] = h;
	}

	/* Accumulate transformations. */

	for (i = 0; i < n - 1; i++) {
		V[n - 1][i] = V[i][i];
		V[i][i] = 1.0;
		h = d[i + 1];
		if (h != 0.0) {
			for (k = 0; k <= i; k++) {
				d[k] = V[k][i + 1] / h;
			}
			for (j = 0; j <= i; j++) {
				g = 0.0;
				for (k = 0; k <= i; k++) {
					g += V[k][i + 1] * V[k][j];
				}
				for (k = 0; k <= i; k++) {
					V[k][j] -= g * d[k];
				}
			}
		}
		for (k = 0; k <= i; k++) {
			V[k][i + 1] = 0.0;
		}
	}
	for (j = 0; j < n; j++) {
		d[j] = V[n - 1][j];
		V[n - 1][j] = 0.0;
	}
	V[n - 1][n - 1] = 1.0;
	e[0] = 0.0;
}

/* Symmetric tridiagonal QL algorithm. */
static void tql2(double V[n][n], double d[n], double e[n]) {

	/*  This is derived from the Algol procedures tql2, by */
	/*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
	/*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
	/*  Fortran subroutine in EISPACK. */

	int i, j, k, l, m;
	double f;
	double tst1;
	double eps;
	int iter;
	double g, p, r;
	double dl1, h, c, c2, c3, el1, s, s2;

	for (i = 1; i < n; i++) {
		e[i - 1] = e[i];
	}
	e[n - 1] = 0.0;

	f = 0.0;
	tst1 = 0.0;
	eps = pow(2.0, -52.0);
	for (l = 0; l < n; l++) {

		/* Find small subdiagonal element */

		tst1 = max(tst1, fabs(d[l]) + fabs(e[l]));
		m = l;
		while (m < n) {
			if (fabs(e[m]) <= eps * tst1) {
				break;
			}
			m++;
		}

		/* If m == l, d[l] is an eigenvalue, */
		/* otherwise, iterate. */

		if (m > l) {
			iter = 0;
			do {
				iter = iter + 1; /* (Could check iteration count here.) */
				/* Compute implicit shift */
				g = d[l];
				p = (d[l + 1] - g) / (2.0 * e[l]);
				r = hypot2(p, 1.0);
				if (p < 0) {
					r = -r;
				}
				d[l] = e[l] / (p + r);
				d[l + 1] = e[l] * (p + r);
				dl1 = d[l + 1];
				h = g - d[l];
				for (i = l + 2; i < n; i++) {
					d[i] -= h;
				}
				f = f + h;
				/* Implicit QL transformation. */
				p = d[m];
				c = 1.0;
				c2 = c;
				c3 = c;
				el1 = e[l + 1];
				s = 0.0;
				s2 = 0.0;
				for (i = m - 1; i >= l; i--) {
					c3 = c2;
					c2 = c;
					s2 = s;
					g = c * e[i];
					h = c * p;
					r = hypot2(p, e[i]);
					e[i + 1] = s * r;
					s = e[i] / r;
					c = p / r;
					p = c * d[i] - s * g;
					d[i + 1] = h + s * (c * g + s * d[i]);
					/* Accumulate transformation. */
					for (k = 0; k < n; k++) {
						h = V[k][i + 1];
						V[k][i + 1] = s * V[k][i] + c * h;
						V[k][i] = c * V[k][i] - s * h;
					}
				}
				p = -s * s2 * c3 * el1 * e[l] / dl1;
				e[l] = s * p;
				d[l] = c * p;

				/* Check for convergence. */
			} while (fabs(e[l]) > eps * tst1);
		}
		d[l] = d[l] + f;
		e[l] = 0.0;
	}

	/* Sort eigenvalues and corresponding vectors. */
	for (i = 0; i < n - 1; i++) {
		k = i;
		p = d[i];
		for (j = i + 1; j < n; j++) {
			if (d[j] < p) {
				k = j;
				p = d[j];
			}
		}
		if (k != i) {
			d[k] = d[i];
			d[i] = p;
			for (j = 0; j < n; j++) {
				p = V[j][i];
				V[j][i] = V[j][k];
				V[j][k] = p;
			}
		}
	}
}

void roots3(double d[3], double c0, double c1, double c2) {
	double c2Div3, aDiv3, mbDiv2, q, magnitude, angle, cs, sn;

	/* Solve the roots of  y^3 + c2 * y^2 + c1 *y + c0  */
	c2Div3 = -c2*inv3;
	aDiv3 = (c1 + c2 * c2Div3) * inv3;
	if (aDiv3 > 0.0) {
		aDiv3 = 0.0;
	}
	mbDiv2 = 0.5 * (-c0 + c2Div3 * (2.0 * c2Div3 * c2Div3 - c1));
	q = mbDiv2 * mbDiv2 + aDiv3 * aDiv3*aDiv3;
	if (q > 0.0) {
		q = 0.0;
	}
	magnitude = sqrt(-aDiv3);
	angle = atan2(sqrt(-q), mbDiv2) * inv3;
	cs = cos(angle);
	sn = sin(angle);
	d[0] = c2Div3 + 2.0 * magnitude*cs;
	d[1] = c2Div3 - magnitude * (cs + root3 * sn);
	d[2] = c2Div3 - magnitude * (cs - root3 * sn);
}

int  fast_eigen3x3(double A[3][3], double V[3][3], double d[3]) {
	const double smallv = 1e-12;
	double c0, c1, c2;
	int check;
	double l1, l2, l3;
	double t;
	double a1, a2, a3, b1, b2;
	double da[3];
	check = (absd(A[0][1]) < smallv)+(absd(A[0][2]) < smallv)+(absd(A[1][2]) < smallv);
	if (check > 1) {
		return 0;
	}

	/* 0 = - det (A - yI) = y^3 + c2 * y^2 + c1 *y + c0 */
	c0 = -(A[0][0] * A[1][1] * A[2][2] + 2 * A[0][1] * A[0][2] * A[1][2] - A[0][0] * pow2(A[1][2]) - A[1][1] * pow2(A[0][2]) - A[2][2] * pow2(A[0][1]));
	c1 = A[0][0] * A[1][1] - pow2(A[0][1]) + A[0][0] * A[2][2] - pow2(A[0][2]) + A[1][1] * A[2][2] - pow2(A[1][2]);
	c2 = -(A[0][0] + A[1][1] + A[2][2]);

	/* Solve the roots of  y^3 + c2 * y^2 + c1 *y + c0  */
	roots3(d, c0, c1, c2);

	da[0] = absd(d[0]);
	da[1] = absd(d[1]);
	da[2] = absd(d[2]);
	/* Sort eigenvalues */
	if (da[0] >= da[1]) {
		if (da[0] > da[2]) {
			t = d[0];
			d[0] = d[2];
			d[2] = t;
			t = da[0];
			da[0] = da[2];
			da[2] = t;
		}
	} else if (da[1] > da[2]) {
		t = d[1];
		d[1] = d[2];
		d[2] = t;
		t = da[1];
		da[1] = da[2];
		da[2] = t;

	}

	if (da[0] >= da[1]) {
		t = d[0];
		d[0] = d[1];
		d[1] = t;
		t = da[0];
		da[0] = da[1];
		da[1] = t;
	}

	if ((da[1] - da[0]) < smallv) {
		return 0;
	}
	if ((da[2] - da[1]) < smallv) {
		return 0;
	}

	/* Calculate eigen vectors */
	a1 = A[0][1] * A[1][2];
	a2 = A[0][1] * A[0][2];
	a3 = pow2(A[0][1]);

	b1 = A[0][0] - d[0];
	b2 = A[1][1] - d[0];
	V[0][0] = a1 - A[0][2] * b2;
	V[1][0] = a2 - A[1][2] * b1;
	V[2][0] = b1 * b2 - a3;

	b1 = A[0][0] - d[1];
	b2 = A[1][1] - d[1];
	V[0][1] = a1 - A[0][2] * b2;
	V[1][1] = a2 - A[1][2] * b1;
	V[2][1] = b1 * b2 - a3;

	b1 = A[0][0] - d[2];
	b2 = A[1][1] - d[2];
	V[0][2] = a1 - A[0][2] * b2;
	V[1][2] = a2 - A[1][2] * b1;
	V[2][2] = b1 * b2 - a3;


	/* Eigen vector normalization */
	l1 = sqrt(pow2(V[0][0]) + pow2(V[1][0]) + pow2(V[2][0]));
	l2 = sqrt(pow2(V[0][1]) + pow2(V[1][1]) + pow2(V[2][1]));
	l3 = sqrt(pow2(V[0][2]) + pow2(V[1][2]) + pow2(V[2][2]));

	/* Detect fail : eigenvectors with only zeros */
	if (l1 < smallv) {
		return 0;
	}
	if (l2 < smallv) {
		return 0;
	}
	if (l3 < smallv) {
		return 0;
	}

	V[0][0] /= l1;
	V[0][1] /= l2;
	V[0][2] /= l3;
	V[1][0] /= l1;
	V[1][1] /= l2;
	V[1][2] /= l3;
	V[2][0] /= l1;
	V[2][1] /= l2;
	V[2][2] /= l3;

	/* Succes    */
	return 1;
}

void eigen_decomposition(double A[n][n], double V[n][n], double d[n]) {
	double e[n];
	double da[3];
	double dt, dat;
	double vet[3];
	int i, j;

	if (fast_eigen3x3(A, V, d)) {
		return;
	}
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			V[i][j] = A[i][j];
		}
	}
	tred2(V, d, e);
	tql2(V, d, e);

	/* Sort the eigen values and vectors by abs eigen value */
	da[0] = absd(d[0]);
	da[1] = absd(d[1]);
	da[2] = absd(d[2]);
	if ((da[0] >= da[1]) && (da[0] > da[2])) {
		dt = d[2];
		dat = da[2];
		vet[0] = V[0][2];
		vet[1] = V[1][2];
		vet[2] = V[2][2];
		d[2] = d[0];
		da[2] = da[0];
		V[0][2] = V[0][0];
		V[1][2] = V[1][0];
		V[2][2] = V[2][0];
		d[0] = dt;
		da[0] = dat;
		V[0][0] = vet[0];
		V[1][0] = vet[1];
		V[2][0] = vet[2];
	} else if ((da[1] >= da[0]) && (da[1] > da[2])) {
		dt = d[2];
		dat = da[2];
		vet[0] = V[0][2];
		vet[1] = V[1][2];
		vet[2] = V[2][2];
		d[2] = d[1];
		da[2] = da[1];
		V[0][2] = V[0][1];
		V[1][2] = V[1][1];
		V[2][2] = V[2][1];
		d[1] = dt;
		da[1] = dat;
		V[0][1] = vet[0];
		V[1][1] = vet[1];
		V[2][1] = vet[2];
	}
	if (da[0] > da[1]) {
		dt = d[1];
		dat = da[1];
		vet[0] = V[0][1];
		vet[1] = V[1][1];
		vet[2] = V[2][1];
		d[1] = d[0];
		da[1] = da[0];
		V[0][1] = V[0][0];
		V[1][1] = V[1][0];
		V[2][1] = V[2][0];
		d[0] = dt;
		da[0] = dat;
		V[0][0] = vet[0];
		V[1][0] = vet[1];
		V[2][0] = vet[2];
	}
}

void diffusion_scheme_3D_rotation_invariance(float *u, float *u_new, int *dimsu, float *Dxx, float *Dxy, float *Dxz, float *Dyy, float *Dyz, float *Dzz, double dt_d) {
	float *j1, *j2, *j3, *ud, *du;
	int npixels;
	float dt = 0.5f;
	int i;
	int x, y, z, index;

	dt = (float) dt_d;
	npixels = dimsu[0] * dimsu[1] * dimsu[2];

	j1 = mallocf(npixels);
	j2 = mallocf(npixels);
	j3 = mallocf(npixels);
	ud = mallocf(npixels);

	/* 3 : Calculate the flux components */
	/* j1 = Dxx .* ux + Dxy .*uy + Dxz .*uz; */
	/* j2 = Dxy .* ux + Dyy .*uy + Dyz .*uz; */
	/* j3 = Dxz .* ux + Dyz .*uy + Dzz .*uz; */

	gradient3Dx_float(u, dimsu, ud);
#pragma omp parallel for
	for (i = 0; i < npixels; i++) {
		j1[i] = Dxx[i] * ud[i];
		j2[i] = Dxy[i] * ud[i];
		j3[i] = Dxz[i] * ud[i];
	}
	gradient3Dy_float(u, dimsu, ud);
#pragma omp parallel for
	for (i = 0; i < npixels; i++) {
		j1[i] += Dxy[i] * ud[i];
		j2[i] += Dyy[i] * ud[i];
		j3[i] += Dyz[i] * ud[i];
	}
	gradient3Dz_float(u, dimsu, ud);
#pragma omp parallel for
	for (i = 0; i < npixels; i++) {
		j1[i] += Dxz[i] * ud[i];
		j2[i] += Dyz[i] * ud[i];
		j3[i] += Dzz[i] * ud[i];
	}

	/*j1(:,:,1)=0; j1(:,:,end)=0; j1(:,1,:)=0; j1(:,end,:)=0; j1(1,:,:)=0; j1(end,:,:)=0; */
	/*j2(:,:,1)=0; j2(:,:,end)=0; j2(:,1,:)=0; j2(:,end,:)=0; j2(1,:,:)=0; j2(end,:,:)=0; */
	/*j3(:,:,1)=0; j3(:,:,end)=0; j3(:,1,:)=0; j3(:,end,:)=0; j3(1,:,:)=0; j3(end,:,:)=0; */

#pragma omp parallel for private (x, index)
	for (y = 0; y < dimsu[1]; y++) {
		for (x = 0; x < dimsu[0]; x++) {
			index = mindex3(x, y, 0, dimsu[0], dimsu[1]);
			j1[index] = 0;
			j2[index] = 0;
			j3[index] = 0;
			index = mindex3(x, y, dimsu[2] - 1, dimsu[0], dimsu[1]);
			j1[index] = 0;
			j2[index] = 0;
			j3[index] = 0;
		}
	}

#pragma omp parallel for private(x,index)
	for (z = 0; z < dimsu[2]; z++) {
		for (x = 0; x < dimsu[0]; x++) {
			index = mindex3(x, 0, z, dimsu[0], dimsu[1]);
			j1[index] = 0;
			j2[index] = 0;
			j3[index] = 0;
			index = mindex3(x, dimsu[1] - 1, z, dimsu[0], dimsu[1]);
			j1[index] = 0;
			j2[index] = 0;
			j3[index] = 0;
		}
	}

#pragma omp parallel for private(y,index)
	for (z = 0; z < dimsu[2]; z++) {
		for (y = 0; y < dimsu[1]; y++) {
			index = mindex3(0, y, z, dimsu[0], dimsu[1]);
			j1[index] = 0;
			j2[index] = 0;
			j3[index] = 0;
			index = mindex3(dimsu[0] - 1, y, z, dimsu[0], dimsu[1]);
			j1[index] = 0;
			j2[index] = 0;
			j3[index] = 0;
		}
	}

	/* 4 : Calculate ... by means of the optimized derivative filters */
	/* du = derivatives(j1,'x')+derivatives(j2,'y')+derivatives(j3,'z'); */
	du = mallocf(npixels);
	gradient3Dx_float(j1, dimsu, du);
	gradient3Dy_float(j2, dimsu, ud);
#pragma omp parallel for
	for (i = 0; i < npixels; i++) {
		du[i] += ud[i];
	}
	gradient3Dz_float(j3, dimsu, ud);
#pragma omp parallel for
	for (i = 0; i < npixels; i++) {
		du[i] += ud[i];
	}

	/* Free memory */
	free(j1);
	free(j2);
	free(j3);
	free(ud);

	/* 5 : Update in an explicit way. */
	/* u=u+du*dt; */

#pragma omp parallel for
	for (i = 0; i < npixels; i++) {
		u_new[i] += u[i] + du[i] * dt;
	}

	/* Free memory */
	free(du);
}

void StructureTensor2DiffusionTensorThread(float **Args) {

	/* Matrices of Eigenvector calculation */
	double Ma[3][3];
	double Davec[3][3];
	double Daeig[3];

	/* Eigenvector and eigenvalues as scalars */
	double mu1, mu2, mu3, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z;

	/* Magnitude of gradient */
	float *gradA;

	/* Amplitudes of diffustion tensor */
	double lambda1, lambda2, lambda3;
	double lambdac1, lambdac2, lambdac3;
	double lambdae1, lambdae2, lambdae3;

	/* Eps for finite values */
	const float eps = (float) 1e-20;

	/* Loop variable */
	int i;

	/* Number of pixels */
	int npixels = 1;

	/* The diffusion tensors and structure tensors */
	float *Jxx, *Jxy, *Jxz, *Jyy, *Jyz, *Jzz;
	float *Dxx, *Dxy, *Dxz, *Dyy, *Dyz, *Dzz;

	int dimsu[3];
	float *dimsu_f, *constants_f, *Nthreads_f, *ThreadID_f;

	/* Number of threads */
	int ThreadOffset, Nthreads;

	/* Constants */
	double C, m, alpha, lambda_h, lambda_e, lambda_c;

	/* Choice of eigenvalue equation */
	int eigenmode;

	/* Temporary variables */
	double di, epsilon, xi;

	//float **Args = (float**) Args_void;

	Jxx = Args[0];
	Jxy = Args[1];
	Jxz = Args[2];
	Jyy = Args[3];
	Jyz = Args[4];
	Jzz = Args[5];
	Dxx = Args[6];
	Dxy = Args[7];
	Dxz = Args[8];
	Dyy = Args[9];
	Dyz = Args[10];
	Dzz = Args[11];
	gradA = Args[12];
	dimsu_f = Args[13];
	constants_f = Args[14];
	ThreadID_f = Args[15];
	Nthreads_f = Args[16];

	for (i = 0; i < 3; i++) {
		dimsu[i] = (int) dimsu_f[i];
	}
	eigenmode = (int) constants_f[0];
	C = (double) constants_f[1];
	m = (double) constants_f[2];
	alpha = (double) constants_f[3];
	lambda_e = (double) constants_f[4];
	lambda_h = (double) constants_f[5];
	lambda_c = (double) constants_f[6];


	ThreadOffset = (int) ThreadID_f[0];
	Nthreads = (int) Nthreads_f[0];

	npixels = dimsu[0] * dimsu[1] * dimsu[2];

	for (i = ThreadOffset; i < npixels; i = i + Nthreads) {
		/* Calculate eigenvectors and values of local Hessian */
		Ma[0][0] = (double) Jxx[i] + eps;
		Ma[0][1] = (double) Jxy[i];
		Ma[0][2] = (double) Jxz[i];
		Ma[1][0] = (double) Jxy[i];
		Ma[1][1] = (double) Jyy[i] + eps;
		Ma[1][2] = (double) Jyz[i];
		Ma[2][0] = (double) Jxz[i];
		Ma[2][1] = (double) Jyz[i];
		Ma[2][2] = (double) Jzz[i] + eps;
		eigen_decomposition(Ma, Davec, Daeig);

		/* Convert eigenvector and eigenvalue matrices back to scalar variables */
		mu1 = Daeig[2];
		mu2 = Daeig[1];
		mu3 = Daeig[0];
		v1x = Davec[0][0];
		v1y = Davec[1][0];
		v1z = Davec[2][0];
		v2x = Davec[0][1];
		v2y = Davec[1][1];
		v2z = Davec[2][1];
		v3x = Davec[0][2];
		v3y = Davec[1][2];
		v3z = Davec[2][2];

		/* Scaling of diffusion tensor */
		if (eigenmode == 0) /* Weickert line shaped */ {
			di = (mu1 - mu3);
			if ((di < eps) && (di>-eps)) {
				lambda1 = alpha;
			} else {
				lambda1 = alpha + (1.0 - alpha) * exp(-C / pow(di, (2.0 * m)));
			}
			lambda2 = alpha;
			lambda3 = alpha;
		} else if (eigenmode == 1) /* Weickert plane shaped */ {
			di = (mu1 - mu3);
			if ((di < eps) && (di>-eps)) {
				lambda1 = alpha;
			} else {
				lambda1 = alpha + (1.0 - alpha) * exp(-C / pow(di, (2.0 * m)));
			}
			di = (mu2 - mu3);
			if ((di < eps) && (di>-eps)) {
				lambda2 = alpha;
			} else {
				lambda2 = alpha + (1.0 - alpha) * exp(-C / pow(di, (2.0 * m)));
			}
			lambda3 = alpha;
		} else if (eigenmode == 2) /* EED */ {
			if (gradA[i] < eps) {
				lambda3 = 1;
			} else {
				lambda3 = 1 - exp(-3.31488 / pow4(gradA[i] / pow2(lambda_e)));
			}
			lambda2 = 1;
			lambda1 = 1;
		} else if (eigenmode == 3) /* CED */ {
			lambda3 = alpha;
			lambda2 = alpha;
			if ((mu2 < eps) && (mu2>-eps)) {
				lambda1 = 1;
			} else if ((mu3 < eps) && (mu3>-eps)) {
				lambda1 = 1;
			} else {
				lambda1 = alpha + (1.0 - alpha) * exp(-0.6931 * pow2(lambda_c) / pow4(mu2 / (alpha + mu3)));
			}
		} else if (eigenmode == 4) /* Hybrid Diffusion with Continous Switch */ {
			if (gradA[i] < eps) {
				lambdae3 = 1;
			} else {
				lambdae3 = 1 - exp(-3.31488 / pow4(gradA[i] / pow2(lambda_e)));
			}
			lambdae2 = 1;
			lambdae1 = 1;

			lambdac3 = alpha;
			lambdac2 = alpha;
			if ((mu2 < eps) && (mu2>-eps)) {
				lambdac1 = 1;
			} else if ((mu3 < eps) && (mu3>-eps)) {
				lambdac1 = 1;
			} else {
				lambdac1 = alpha + (1.0 - alpha) * exp(-0.6931 * pow2(lambda_c) / pow4(mu2 / (alpha + mu3)));
			}

			xi = ((mu1 / (alpha + mu2)) - (mu2 / (alpha + mu3)));
			di = 2.0 * pow4(lambda_h);
			epsilon = exp(mu2 * (pow2(lambda_h)*(xi - absd(xi)) - 2.0 * mu3) / di);


			lambda1 = (1 - epsilon) * lambdac1 + epsilon * lambdae1;
			lambda2 = (1 - epsilon) * lambdac2 + epsilon * lambdae2;
			lambda3 = (1 - epsilon) * lambdac3 + epsilon * lambdae3;
		}



		/* Construct the diffusion tensor */
		Dxx[i] = (float) (lambda1 * v1x * v1x + lambda2 * v2x * v2x + lambda3 * v3x * v3x);
		Dyy[i] = (float) (lambda1 * v1y * v1y + lambda2 * v2y * v2y + lambda3 * v3y * v3y);
		Dzz[i] = (float) (lambda1 * v1z * v1z + lambda2 * v2z * v2z + lambda3 * v3z * v3z);
		Dxy[i] = (float) (lambda1 * v1x * v1y + lambda2 * v2x * v2y + lambda3 * v3x * v3y);
		Dxz[i] = (float) (lambda1 * v1x * v1z + lambda2 * v2x * v2z + lambda3 * v3x * v3z);
		Dyz[i] = (float) (lambda1 * v1y * v1z + lambda2 * v2y * v2z + lambda3 * v3y * v3z);
	}

	/*  explicit end thread, helps to ensure proper recovery of resources allocated for the thread */

	pthread_exit(NULL);

}

void StructureTensor2DiffusionTensor(float *Jxx, float *Jxy, float *Jxz, float *Jyy, float *Jyz, float *Jzz, float *Dxx, float *Dxy, float *Dxz, float *Dyy, float *Dyz, float *Dzz, float *gradA, int *dimsu, int eigenmode, double C, double m, double alpha, double lambda_e, double lambda_h, double lambda_c) {


	/* ID of Threads */
	float **ThreadID;
	float *ThreadID1;
	float ***ThreadArgs;
	float **ThreadArgs1;
	float Nthreads_f[1] = {0};
	float dimsu_f[3];
	float constants_f[7];
	int Nthreads;
	int i;

	pthread_t *ThreadList;

	Nthreads = omp_get_num_procs( );
	Nthreads_f[0] = (float) Nthreads;
	for (i = 0; i < 3; i++) {
		dimsu_f[i] = (float) dimsu[i];
	}
	constants_f[0] = (float) eigenmode;
	constants_f[1] = (float) C;
	constants_f[2] = (float) m;
	constants_f[3] = (float) alpha;
	constants_f[4] = (float) lambda_e;
	constants_f[5] = (float) lambda_h;
	constants_f[6] = (float) lambda_c;

	/* Reserve room for handles of threads in ThreadList  */
	ThreadList = (pthread_t*) malloc(Nthreads * sizeof ( pthread_t));
	ThreadID = (float **) malloc(Nthreads * sizeof (float *));
	ThreadArgs = (float ***) malloc(Nthreads * sizeof (float **));

	for (i = 0; i < Nthreads; i++) {
		/*  Make Thread ID  */
		ThreadID1 = (float *) malloc(1 * sizeof (float));
		ThreadID1[0] = (float) i;
		ThreadID[i] = ThreadID1;

		/*  Make Thread Structure  */
		ThreadArgs1 = (float **) malloc(17 * sizeof ( float *));
		ThreadArgs1[0] = Jxx;
		ThreadArgs1[1] = Jxy;
		ThreadArgs1[2] = Jxz;
		ThreadArgs1[3] = Jyy;
		ThreadArgs1[4] = Jyz;
		ThreadArgs1[5] = Jzz;
		ThreadArgs1[6] = Dxx;
		ThreadArgs1[7] = Dxy;
		ThreadArgs1[8] = Dxz;
		ThreadArgs1[9] = Dyy;
		ThreadArgs1[10] = Dyz;
		ThreadArgs1[11] = Dzz;
		ThreadArgs1[12] = gradA;
		ThreadArgs1[13] = dimsu_f;
		ThreadArgs1[14] = constants_f;
		ThreadArgs1[15] = ThreadID[i];
		ThreadArgs1[16] = Nthreads_f;

		/* Start a Thread  */
		ThreadArgs[i] = ThreadArgs1;
		pthread_create((pthread_t*) & ThreadList[i], NULL, (void*) &StructureTensor2DiffusionTensorThread, ThreadArgs[i]);

	}

	for (i = 0; i < Nthreads; i++) {
		pthread_join(ThreadList[i], NULL);
	}

	for (i = 0; i < Nthreads; i++) {
		free(ThreadArgs[i]);
		free(ThreadID[i]);
	}

	free(ThreadArgs);
	free(ThreadID);
	free(ThreadList);

}

int _p3dAnisotropicDiffusionFilter3D_float(
	float* u,
	float* u_new,
	const int dimx,
	const int dimy,
	const int dimz,
	const double lambda,
	const int m,
	const double sigma,
	const int iter,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		/* Options structure variables */
		struct options Options;

		/* Size input image volume */
		int ndimsu;
		int dimsu[3];
		int npixelsu;
		int i, ct;

		float *usigma; /* Gaussian filtered image volume */
		float *ux, *uy, *uz, *gradA; /* Gradients of smoothed image */
		float *Jxx, *Jxy, *Jxz, *Jyy, *Jyz, *Jzz, *J[6]; /* Structure tensor */
		float *Dxx, *Dxy, *Dxz, *Dyy, *Dyz, *Dzz; /* Diffusion tensor */

		ndimsu = 3;
		dimsu[0] = dimx;
		dimsu[1] = dimy;
		dimsu[2] = dimz;
		npixelsu = dimsu[0] * dimsu[1] * dimsu[2];

		Options.T = iter; // ok... call it iteration
		Options.dt = 0.24; // below 0.25 for stability //fix
		Options.sigma = sigma; // ok... call it sigma
		Options.rho = 1.0; // fix
		Options.C = 1e-10; // fix
		Options.m = m; // ok... call it mu
		Options.alpha = 0.01; // fix
		Options.eigenmode = 2; // only 4 and 2 with standard parameters below!!!!!!!!! //fix on 2
		Options.lambda_e = lambda; // ok... call it lambda
		Options.lambda_h = 0.01; //delete
		Options.lambda_c = 0.5; //delete

		for (ct = 0; ct < Options.T; ct++) {

			memset(u_new, 0, dimx * dimy * dimz * sizeof (float));

			/* Gaussian Filtering of input image volume*/
			usigma = mallocf(npixelsu);
			//wr_log("Eseguo il gaussian filtering 3D...");
			GaussianFiltering3D_float(u, usigma, dimsu, Options.sigma, 4 * Options.sigma);
			//wr_log("Done!\n");

			/* Calculate the image gradients of the smoothed image volume */
			ux = mallocf(npixelsu);
			uy = mallocf(npixelsu);
			uz = mallocf(npixelsu);

			//wr_log("Eseguo il calcolo dei gradienti...");
			gradient3Dx_float(usigma, dimsu, ux);
			//wr_log("Done x!\n");
			gradient3Dy_float(usigma, dimsu, uy);
			//wr_log("Done y!\n");
			gradient3Dz_float(usigma, dimsu, uz);
			//wr_log("Done z!\n");

			/* remove usigma from memory */
			free(usigma);

			/* Compute the 3D structure tensors J of the image */
			// wr_log("Calcolato StructureTensor3D.\n");
			StructureTensor3D(ux, uy, uz, J, dimsu, Options.rho);
			//wr_log("Done!\n");

			Jxx = J[0];
			Jyy = J[1];
			Jzz = J[2];
			Jxy = J[3];
			Jxz = J[4];
			Jyz = J[5];

			/* calculate gradient magnitude */
			gradA = mallocf(npixelsu);

#pragma omp parallel for
			for (i = 0; i < npixelsu; i++) {
				gradA[i] = ux[i] * ux[i] + uy[i] * uy[i] + uz[i] * uz[i];
			}

			/* remove gradients from memory */
			free(ux);
			free(uy);
			free(uz);

			/* Structure to Diffusion Tensor Weickert */
			Dxx = mallocf(npixelsu);
			Dyy = mallocf(npixelsu);
			Dzz = mallocf(npixelsu);
			Dxy = mallocf(npixelsu);
			Dxz = mallocf(npixelsu);
			Dyz = mallocf(npixelsu);

			StructureTensor2DiffusionTensor(Jxx, Jxy, Jxz, Jyy, Jyz, Jzz, Dxx, Dxy, Dxz, Dyy, Dyz, Dzz, gradA, dimsu, Options.eigenmode, Options.C, Options.m, Options.alpha, Options.lambda_e, Options.lambda_h, Options.lambda_c);
			//wr_log("Calcolato StructureTensor2DiffusionTensor.\n");
			/* Gradient Magnitude no longer needed */
			free(gradA);
			/* remove structure tensor from memory */
			free(Jxx);
			free(Jyy);
			free(Jzz);
			free(Jxy);
			free(Jxz);
			free(Jyz);

			/* Perform the image diffusion */
			diffusion_scheme_3D_rotation_invariance(u, u_new, dimsu, Dxx, Dxy, Dxz, Dyy, Dyz, Dzz, Options.dt);
			//wr_log("Calcolato diffusion_scheme_3D_rotation_invariance.\n");

			/* remove diffusion tensor from memory */
			free(Dxx);
			free(Dyy);
			free(Dzz);
			free(Dxy);
			free(Dxz);
			free(Dyz);

			// Prepare for next step:
			memcpy(u, u_new, dimx * dimy * dimz * sizeof (float));
		}
}

int p3dAnisotropicDiffusionFilter3D_8(
	unsigned char* in_im,
	unsigned char* out_im,
	const int dimx,
	const int dimy,
	const int dimz,
	const int m,
	const double lambda,
	const double sigma,
	const int iter,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {

		float* u = NULL;
		float* u_new = NULL;

		//float u_min = UCHAR_MAX;
		//float u_max = 0;

		//float u_new_min = UCHAR_MAX;
		//float u_new_max = 0;

		int ct, i, j, k;
		const int a_rad = 1;
		int a_dimx, a_dimy, a_dimz;
		char* auth_code;

		//
		// Authenticate:
		//
		/*auth_code = authenticate("p3dAnisotropicDiffusionFilter3D_8");
		if (strcmp(auth_code, "1") != 0) goto AUTH_ERROR;*/

		// Start tracking computational time:
		if (wr_log != NULL) {
			//p3dResetStartTime();
			wr_log("Pore3D - Applying anisotropic diffusion filter...");
			wr_log("\tLambda: %0.3f", lambda);
			wr_log("\tMu: %d.", m);
			wr_log("\tSigma: %0.3f.", sigma);
			wr_log("\tNumber of iterations: %d.", iter);
		}

		a_dimx = dimx + a_rad * 2;
		a_dimy = dimy + a_rad * 2;
		a_dimz = dimz + a_rad * 2;

		// Allocate memory:
		u = (float*) malloc(a_dimx * a_dimy * a_dimz * sizeof (float));
		u_new = (float*) malloc(a_dimx * a_dimy * a_dimz * sizeof (float));

		_p3dReplicatePadding3D_uchar2float(in_im, u, dimx, dimy, dimz, a_rad);

		// Convert the input:
#pragma omp parallel for
		for (ct = 0; ct < (a_dimx * a_dimy * a_dimz); ct++) {
			u[ct] = u[ct] / UCHAR_MAX;
			//u_min = MIN(u_min, u[ct]);
			//u_max = MAX(u_max, u[ct]);
		}

		// Call the 32 bit version:
		_p3dAnisotropicDiffusionFilter3D_float(u,
			u_new,
			a_dimx,
			a_dimy,
			a_dimz,
			lambda,
			m,
			sigma,
			iter,
			wr_log,
			wr_progress
			);

		// Convert the input:
		/*for (ct = 0; ct < (a_dimx * a_dimy * a_dimz); ct++) {
		u_new_min = MIN(u_new_min, u_new[ct]);
		u_new_max = MAX(u_new_max, u_new[ct]);
		}*/


		// Rescale output:
#pragma omp parallel for private(i,j)
		for (k = a_rad; k < (a_dimz - a_rad); k++)
			for (j = a_rad; j < (a_dimy - a_rad); j++)
				for (i = a_rad; i < (a_dimx - a_rad); i++) {
					// out_im[I(i - a_rad, j - a_rad, k - a_rad,  dimx, dimy)] = (unsigned char)
					//((u_new[I(i, j, k, a_dimx, a_dimy)] - u_new_min) / (
					//(u_new_max - u_new_min))*(u_max - u_min)) + u_min;
					if (u_new[ I(i, j, k, a_dimx, a_dimy)] >= 1.0)
						out_im[ I(i - a_rad, j - a_rad, k - a_rad, dimx, dimy) ] = UCHAR_MAX;
					else if (u_new[ I(i, j, k, a_dimx, a_dimy)] <= 0.0)
						out_im[ I(i - a_rad, j - a_rad, k - a_rad, dimx, dimy) ] = 0;
					else
						out_im[ I(i - a_rad, j - a_rad, k - a_rad, dimx, dimy) ] =
						(unsigned char) (u_new[ I(i, j, k, a_dimx, a_dimy)] * UCHAR_MAX);
				}

				// Print elapsed time (if required):
				if (wr_log != NULL) {
					// wr_log("Pore3D - Anisotropic diffusion filter applied successfully in %0.3f sec.", p3dGetElapsedTime());
				}

				// Release resources:
				if (u != NULL) free(u);
				if (u_new != NULL) free(u_new);

				// Return success:
				return P3D_SUCCESS;


MEM_ERROR:

				if (wr_log != NULL) {
					wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
				}

				// Release resources:
				if (u != NULL) free(u);
				if (u_new != NULL) free(u_new);

				// Return error:
				return P3D_MEM_ERROR;

AUTH_ERROR:

				if (wr_log != NULL) {
					wr_log("Pore3D - Authentication error. Program will exit.");
				}

				return P3D_AUTH_ERROR;
}


#define NO_AUTH 1

/*ypedef struct {
int x;
int y;
int z;
} coords_t;

typedef struct {
double x;
double y;
double z;
} fcoords_t;

struct coords_qelem_t {
coords_t item;
struct coords_qelem_t *next;
};

typedef struct coords_qelem_t coords_queue_elem_t;

typedef struct {
coords_queue_elem_t *tail;
coords_queue_elem_t *head;
} coords_queue_t;*/

int __p3dReadRaw8(
	char* filename,
	unsigned char* out_im,
	const int dimx,
	const int dimy,
	const int dimz
	) {
		FILE* fvol;

		/* Get a handler for the input file */
		if ((fvol = fopen(filename, "rb")) == NULL) {
			printf("Cannot open input file %s.", filename);
			out_im = NULL;

			return -1;
		}

		/* Read raw data from file: */
		fread(out_im, sizeof (unsigned char), dimx * dimy*dimz, fvol);

		/* Close file handler: */
		fclose(fvol);

		return 0;
}

int __p3dReadRaw32(
	char* filename,
	float* out_im,
	const int dimx,
	const int dimy,
	const int dimz
	) {
		FILE* fvol;

		/* Get a handler for the input file */
		if ((fvol = fopen(filename, "rb")) == NULL) {
			printf("Cannot open input file %s.", filename);
			out_im = NULL;

			return -1;
		}

		/* Read raw data from file: */
		fread(out_im, sizeof (float), dimx * dimy*dimz, fvol);

		/* Close file handler: */
		fclose(fvol);

		return 0;
}

int __p3dWriteRaw8(
	unsigned char* in_im,
	char* filename,
	const int dimx,
	const int dimy,
	const int dimz
	) {
		FILE* fvol;

		/* Get a handler for the input file */
		if ((fvol = fopen(filename, "wb")) == NULL) {
			printf("Cannot open output file %s.", filename);
			in_im = NULL;

			return -1;
		}

		/* Write raw data to file: */
		fwrite(in_im, sizeof (unsigned char), dimx * dimy*dimz, fvol);

		/* Close file handler: */
		fclose(fvol);

		return 0;
}

int __p3dWriteRaw32(
	float* in_im,
	char* filename,
	const int dimx,
	const int dimy,
	const int dimz
	) {
		FILE* fvol;

		/* Get a handler for the input file */
		if ((fvol = fopen(filename, "wb")) == NULL) {
			printf("Cannot open output file %s.", filename);
			in_im = NULL;

			return -1;
		}

		/* Write raw data to file: */
		fwrite(in_im, sizeof (float), dimx * dimy*dimz, fvol);

		/* Close file handler: */
		fclose(fvol);

		return 0;
}

int customPrint_nolf(const char *msg, ...) {
	va_list fmtargs;
	char buffer[1024];
	//FILE *foutVF;

	va_start(fmtargs, msg);
	vsnprintf(buffer, sizeof (buffer) - 1, msg, fmtargs);
	va_end(fmtargs);

	return printf("%s", buffer);

	/*
	//printf( "%s\n", buffer );
	// open the file
	if ((foutVF = fopen("C:\\loglog.txt","a")) == NULL)
	{
	printf("Cannot open output file loglog.txt for appending");
	return -1;
	}

	fprintf(foutVF, "%s\n", buffer );

	// close the file
	fclose(foutVF);
	return 0;*/
}

int customPrint(const char *msg, ...) {
	va_list fmtargs;
	char buffer[1024];
	//FILE *foutVF;

	va_start(fmtargs, msg);
	vsnprintf(buffer, sizeof (buffer) - 1, msg, fmtargs);
	va_end(fmtargs);

	return printf("%s\n", buffer);

	/*
	//printf( "%s\n", buffer );
	// open the file
	if ((foutVF = fopen("C:\\loglog.txt","a")) == NULL)
	{
	printf("Cannot open output file loglog.txt for appending");
	return -1;
	}

	fprintf(foutVF, "%s\n", buffer );

	// close the file
	fclose(foutVF);
	return 0;*/
}

int customProgress(const int msg, ...) {
	return customPrint_nolf("\tProgress: %d.\r", msg);

}

/*int _p3dSkeletonAnalysis_EndPoints(
unsigned char* vol_im, // IN: Input segmented (binary) volume
unsigned short* dt_im, // IN: Input squared euclidean distance transform of vol_im
unsigned char* lbl_skl_im, // IN: Input labeled skeleton of the segmented volume
unsigned char* ends_im, // OUT: Image with maximal balls filled on endpoints
struct SkeletonStats* out_stats, // OUT: Skeleton statistics
const int dimx,
const int dimy,
const int dimz,
const double voxelsize // IN: voxel resolution
) {
unsigned short* tmp_im = NULL;

double max_width;

int cc_array_numel;
unsigned int* cc_array = NULL;

bb_t* bbs = NULL;
bb_t curr_bb;
int ct, i, j, k, a, b, c;
int rad;
double delta;

// Allocate memory:
P3D_TRY(tmp_im = (unsigned short*) calloc(dimx * dimy*dimz, sizeof (unsigned short)));

// Fill the balls:
#pragma omp parallel for private(i, j, rad, a, b, c, delta)
for (k = 0; k < dimz; k++)
for (j = 0; j < dimy; j++)
for (i = 0; i < dimx; i++) {
if (lbl_skl_im[ I(i, j, k, dimx, dimy) ] == END_LABEL) {
// Get the radius:
rad = (int) (sqrt((double) dt_im [ I(i, j, k, dimx, dimy) ]));

// Fill the ball:
for (c = k - rad; c <= k + rad; c++)
for (b = j - rad; b <= j + rad; b++)
for (a = i - rad; a <= i + rad; a++) {
// We are scanning the bounding box, so we need to be sure
// if current position (a,b,c) is inside the ball:
delta = sqrt((double) ((a - i)*(a - i) + (b - j)*(b - j)
+ (c - k)*(c - k)));

if ((a >= 0) && (b >= 0) && (c >= 0) &&
(a < dimx) && (b < dimy) && (c < dimz) &&
(delta < rad)) {
ends_im [ I(a, b, c, dimx, dimy) ] = OBJECT;
}
}
}
}


// Perform connected components labeling of END points:
P3D_TRY(p3dConnectedComponentsLabeling(ends_im, tmp_im, &cc_array_numel, &cc_array, &bbs,
dimx, dimy, dimz, CONN6, P3D_TRUE));

// If there are endpoints:
if (cc_array_numel != 0) {
// Allocate memory for the distribution of widths on endpoints:
P3D_TRY(out_stats->End_Width = (double*) malloc(cc_array_numel * sizeof (double)));
out_stats->End_Counter = cc_array_numel;

// Compute max width on endpoints:
for (ct = 0; ct < cc_array_numel; ct++) {
curr_bb = bbs[ct];

max_width = 0.0;

// Scan the bounding box for maximum value:			
for (k = curr_bb.min_z; k <= curr_bb.max_z; k++)
for (j = curr_bb.min_y; j <= curr_bb.max_y; j++)
for (i = curr_bb.min_x; i <= curr_bb.max_x; i++) {
if (ends_im[ I(i, j, k, dimx, dimy) ] == OBJECT) {
max_width = MAX(max_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
}
}


// Copy statistics to output structure:
out_stats->End_Width[ct] = 2 * sqrt(max_width) * voxelsize;
}
} else {
// Copy empty statistics to output structure:
out_stats->End_Counter = 0;
out_stats->End_Width = NULL;
}



// Release resources:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);

return P3D_SUCCESS;

MEM_ERROR:

// Release resources:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);

return P3D_MEM_ERROR;
}

int _p3dSkeletonAnalysis_NodePoints(
unsigned char* vol_im, // IN: Input segmented (binary) volume
unsigned short* dt_im, // IN: Input squared euclidean distance transform of vol_im
unsigned char* lbl_skl_im, // IN: Input labeled skeleton of the segmented volume
unsigned char* nodes_im, // OUT: Image with maximal balls filled on nodepoints
struct SkeletonStats* out_stats, // OUT: Skeleton statistics
const int dimx,
const int dimy,
const int dimz,
const double voxelsize // IN: voxel resolution
) {
unsigned short* tmp_im = NULL;
unsigned short* tmp_roi = NULL;

double max_width;

int cc_array_numel;
unsigned int* cc_array = NULL;

bb_t* bbs = NULL;
bb_t curr_bb;
int i, j, k, a, b, c;
int ct, ct2, rad, delta;
coords_t coords;

unsigned short curr_lbl;
int offset = 1; // Extra-marging for bounding box


// Allocate memory:	
P3D_TRY(tmp_im = (unsigned short*) calloc(dimx * dimy*dimz, sizeof (unsigned short)));
P3D_TRY(tmp_roi = (unsigned short*) malloc(dimx * dimy * dimz * sizeof (unsigned short)));

// Fill the balls:
#pragma omp parallel for private(i, j, rad, a, b, c, delta)
for (k = 0; k < dimz; k++)
for (j = 0; j < dimy; j++)
for (i = 0; i < dimx; i++) {
if (lbl_skl_im[ I(i, j, k, dimx, dimy) ] == NODE_LABEL) {
// Get the radius:
rad = (int) (sqrt((double) dt_im [ I(i, j, k, dimx, dimy) ]));

// Fill the ball:
for (c = k - rad; c <= k + rad; c++)
for (b = j - rad; b <= j + rad; b++)
for (a = i - rad; a <= i + rad; a++) {
// We are scanning the bounding box, so we need to be sure
// if current position (a,b,c) is inside the ball:
delta = (a - i)*(a - i) + (b - j)*(b - j) + (c - k)*(c - k);

if ((a >= 0) && (b >= 0) && (c >= 0) &&
(a < dimx) && (b < dimy) && (c < dimz) &&
(delta < dt_im [ I(i, j, k, dimx, dimy) ])) {
nodes_im [ I(a, b, c, dimx, dimy) ] = OBJECT;
}
}
}
}

// Perform connected components labeling of NODE points:
P3D_TRY(p3dConnectedComponentsLabeling(nodes_im, tmp_im, &cc_array_numel, &cc_array, &bbs,
dimx, dimy, dimz, CONN6, P3D_TRUE));

if (cc_array_numel != 0) {
// Start stuffs for connectivity density:
out_stats->ConnectivityDensity = cc_array_numel;

// Allocate memory for the distribution of widths on NODE points:
P3D_TRY(out_stats->Node_Width = (double*) malloc(cc_array_numel * sizeof (double)));
out_stats->Node_Counter = cc_array_numel;

// Allocate memory for the coordination number distribution:
P3D_TRY(out_stats->CoordinationNumber = (int*) calloc(cc_array_numel, sizeof (int)));

// Compute maximum width on NODE points:
for (ct = 0; ct < cc_array_numel; ct++) {
curr_bb = bbs[ct];

max_width = 0.0;

// Reset the copy of the ROI of the bounding box:
memset(tmp_roi, 0, dimx * dimy * dimz * sizeof (unsigned short));

// Scan the bounding box:			
for (k = (curr_bb.min_z - offset); k <= (curr_bb.max_z + offset); k++)
for (j = (curr_bb.min_y - offset); j <= (curr_bb.max_y + offset); j++)
for (i = (curr_bb.min_x - offset); i <= (curr_bb.max_x + offset); i++) {
if ((i >= 0) && (j >= 0) && (k >= 0) &&
(i < dimx) && (j < dimy) && (k < dimz)) {
// Compute the maximum value:
//if ( nodes_im[ I( i,j,k,dimx,dimy ) ] == OBJECT )
if (tmp_im[ I(i, j, k, dimx, dimy) ] == (unsigned short) (ct + 3)) {
max_width = MAX(max_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
}

// Create a temporary copy of the ROI (initialized with the same dimension of the 
// whole image for semplicity even it's a waste of memory) of the bounding box.
// At this point, tmp_im is unsigned short with labeled nodes and lbl_skl_im is 
// unsigned short with classification of branches. So the ROI is created with the 
// labeled current node (direct copy from tmp_im) and NODE-TO-NODE and NODE-TO-END 
// assigned to USHORT_MAX.

if ((lbl_skl_im[ I(i, j, k, dimx, dimy) ] == NODETONODE_LABEL) ||
(lbl_skl_im[ I(i, j, k, dimx, dimy) ] == NODETOEND_LABEL)) {
tmp_roi[ I(i, j, k, dimx, dimy) ] = USHRT_MAX;
}


if (tmp_im[ I(i, j, k, dimx, dimy) ] != BACKGROUND)
tmp_roi[ I(i, j, k, dimx, dimy) ] = tmp_im[ I(i, j, k, dimx, dimy) ];

}
}

// Compute the coordination number re-scanning bounding box:
for (k = (curr_bb.min_z - offset); k <= (curr_bb.max_z + offset); k++)
for (j = (curr_bb.min_y - offset); j <= (curr_bb.max_y + offset); j++)
for (i = (curr_bb.min_x - offset); i <= (curr_bb.max_x + offset); i++) {
if ((i >= 0) && (j >= 0) && (k >= 0) &&
(i < dimx) && (j < dimy) && (k < dimz)) {
// If a branch is found (USHRT_MAX voxel):
if (tmp_roi[ I(i, j, k, dimx, dimy) ] == USHRT_MAX) {
// Increment coordination number if the current node label is found in
// the neighborhood:
if (_countNeighbors(tmp_roi, dimx, dimy, dimz, i, j, k,
(unsigned short) (ct + 3)) >= 1) {
out_stats->CoordinationNumber[ct]++;

// Remove current branch in order to avoid counting more than once
// non-perfect interconnections with the maximal ball (simple points 
// occur):							
tmp_roi[ I(i, j, k, dimx, dimy) ] = BACKGROUND;

while (_findNeighbor(tmp_roi, dimx, dimy, dimz, i, j, k, &coords) != 0) {
a = i;
b = j;
c = k;
while (_findNeighbor(tmp_roi, dimx, dimy, dimz, a, b, c, &coords) != 0) {
a = coords.x;
b = coords.y;
c = coords.z;
tmp_roi[ I(a, b, c, dimx, dimy) ] = BACKGROUND;
}
}
}
}
}
}

// Copy statistics to output structure:
out_stats->Node_Width[ct] = 2 * sqrt(max_width) * voxelsize;
}
} else {
// Copy empty statistics to output structure:
out_stats->Node_Counter = 0;
out_stats->Node_Width = NULL;
out_stats->ConnectivityDensity = 0;
}



// Release resources:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_roi != NULL) free(tmp_roi);

return P3D_SUCCESS;

MEM_ERROR:

// Release resources:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_roi != NULL) free(tmp_roi);


return P3D_MEM_ERROR;
}

int _p3dSkeletonAnalysis_NodeToNodeBranches(
unsigned char* vol_im, // IN: Input segmented (binary) volume
unsigned short* dt_im, // IN: Input squared euclidean distance transform of vol_im
unsigned char* lbl_skl_im, // IN: Input labeled skeleton of the segmented volume
unsigned char* nodes_im, // IN: Input image of identified nodes with merging criteria
struct SkeletonStats* out_stats, // IN/OUT: Skeleton statistics
unsigned char* throats_im, // OUT: Input image of identified throats with merging criteria
const int dimx,
const int dimy,
const int dimz,
const double voxelsize // IN: voxel resolution
) {
unsigned char* tmp_im = NULL;
unsigned short* tmp_im2 = NULL;

double min_width, mean_width, max_width, delta;

unsigned int* cc_array = NULL;
int cc_array_numel;

bb_t* bbs = NULL;
bb_t curr_bb;

int ct, i, j, k, a, b, c, skip_throat, rad;
int min_coord_x, min_coord_y, min_coord_z;


//
// Isolate NODE-TO-NODE branches:
//

// Allocate memory for temp image skeleton:
P3D_TRY(tmp_im = (unsigned char*) calloc(dimx * dimy*dimz, sizeof (unsigned char)));
P3D_TRY(tmp_im2 = (unsigned short*) calloc(dimx * dimy*dimz, sizeof (unsigned short)));



// Create temporary matrix removing the filled nodes. Doing so, only the part of a 
// branch outside nodes_im, i.e. the image with filled balls on skeleton nodes, is 
// taken into acccount.
#pragma omp parallel for
for (ct = 0; ct < (dimx * dimy * dimz); ct++) {
if (lbl_skl_im[ ct ] == NODETONODE_LABEL) {
tmp_im [ ct ] = OBJECT;
}
if (nodes_im[ ct ] == OBJECT) {
tmp_im [ ct ] = BACKGROUND;
}
}


// Perform connected components labeling of NODE-TO-NODE branches:
P3D_TRY(p3dConnectedComponentsLabeling(tmp_im, tmp_im2, &cc_array_numel, &cc_array, &bbs,
dimx, dimy, dimz, CONN26, P3D_TRUE));

// Tmp_im2 is no needed anymore:
if (tmp_im2 != NULL) free(tmp_im2);
tmp_im2 = NULL;

if (cc_array_numel != 0) {
// Allocate memory for the distribution of widths on endpoints:
P3D_TRY(out_stats->NodeToNode_Length = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->NodeToNode_MinWidth = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->NodeToNode_MeanWidth = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->NodeToNode_MaxWidth = (double*) malloc(cc_array_numel * sizeof (double)));

// Copy NODE-TO-END branches statistics to output structure:
out_stats->NodeToNode_Counter = cc_array_numel;

// At this point connectivity density contains the number of node points, so
// it suffices to subtract the number of NODE-TO-NODE branches:
out_stats->ConnectivityDensity = out_stats->ConnectivityDensity - cc_array_numel;

// Compute mean length on NODE-TO-NODE branches:
for (ct = 0; ct < cc_array_numel; ct++) {
curr_bb = bbs[ct];

min_width = LONG_MAX;
mean_width = 0.0;
max_width = LONG_MIN;

// Scan the bounding box for minimum, mean and maximum value:			
for (k = curr_bb.min_z; k <= curr_bb.max_z; k++)
for (j = curr_bb.min_y; j <= curr_bb.max_y; j++)
for (i = curr_bb.min_x; i <= curr_bb.max_x; i++) {
if (tmp_im[ I(i, j, k, dimx, dimy) ] == OBJECT) {
max_width = MAX(max_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
mean_width += (double) dt_im [ I(i, j, k, dimx, dimy) ];

if ((double) (dt_im [ I(i, j, k, dimx, dimy) ]) < min_width) {
min_width = (double) (dt_im [ I(i, j, k, dimx, dimy) ]);
min_coord_x = i;
min_coord_y = j;
min_coord_z = k;
}
}
}

// Get the radius:
rad = (int) (sqrt(min_width));

// Fill the ball on throat:		
for (c = min_coord_z - rad; c <= min_coord_z + rad; c++)
for (b = min_coord_y - rad; b <= min_coord_y + rad; b++)
for (a = min_coord_x - rad; a <= min_coord_x + rad; a++) {
//skip_throat = FALSE;

// We are scanning the bounding box, so we need to be sure
// if current position (a,b,c) is inside the ball:
delta = (double) ((a - min_coord_x)*(a - min_coord_x) +
(b - min_coord_y)*(b - min_coord_y) + (c - min_coord_z)*(c - min_coord_z));

if ((a >= 0) && (b >= 0) && (c >= 0) &&
(a < dimx) && (b < dimy) && (c < dimz) &&
(delta < min_width)) {
throats_im [ I(a, b, c, dimx, dimy) ] = OBJECT;
/*if ( ( a == 0 ) || ( b == 0 ) || ( c == 0 ) ||
( a == (dimx-1) ) || ( b == (dimy-1) ) || ( c == (dimz-1) ) )
{
skip_throat = TRUE;
}*/
/* }
}

mean_width /= cc_array[ ct ];

// Copy NODE-TO-NODE branches statistics to output structure:
out_stats->NodeToNode_Length[ct] = cc_array[ ct ] * voxelsize;
out_stats->NodeToNode_MinWidth[ct] = 2 * sqrt(min_width) * voxelsize;
out_stats->NodeToNode_MeanWidth[ct] = 2 * sqrt(mean_width) * voxelsize;
out_stats->NodeToNode_MaxWidth[ct] = 2 * sqrt(max_width) * voxelsize;
}
} else {
// Copy NODE-TO-NODE branches statistics to output structure:
out_stats->NodeToNode_Counter = 0;
out_stats->NodeToNode_Length = NULL;
out_stats->NodeToNode_MinWidth = NULL;
out_stats->NodeToNode_MeanWidth = NULL;
out_stats->NodeToNode_MaxWidth = NULL;
}

// TODO: Reshape arrays due to the skip_throat flag

// Release resources after each call to _p3dConnectedComponentsLabeling:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);

return P3D_SUCCESS;

MEM_ERROR:

// Release resources after each call to _p3dConnectedComponentsLabeling:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_im2 != NULL) free(tmp_im2);

return P3D_MEM_ERROR;
}

int _p3dSkeletonAnalysis_NodeToEndBranches(
unsigned char* vol_im, // IN: Input segmented (binary) volume
unsigned short* dt_im, // IN: Input squared euclidean distance transform of vol_im
unsigned char* lbl_skl_im, // IN: Input labeled skeleton of the segmented volume
unsigned char* nodes_im, // IN: Input image of identified nodes with merging criteria
unsigned char* ends_im, // IN: Input image of identified nodes with merging criteria
struct SkeletonStats* out_stats, // OUT: Skeleton statistics
const int dimx,
const int dimy,
const int dimz,
const double voxelsize // IN: voxel resolution
) {
unsigned char* tmp_im = NULL;
unsigned short* tmp_im2 = NULL;

double min_width, mean_width, max_width;

unsigned int* cc_array = NULL;
int cc_array_numel;

bb_t* bbs = NULL;
bb_t curr_bb;

int ct, i, j, k;


//
// Isolate NODE-TO-NODE branches:
//

// Allocate memory for temp image skeleton:
P3D_TRY(tmp_im = (unsigned char*) calloc(dimx * dimy*dimz, sizeof (unsigned char)));
P3D_TRY(tmp_im2 = (unsigned short*) calloc(dimx * dimy*dimz, sizeof (unsigned short)));


// Create temporary matrix removing the filled balls. Doing so, only the part of a 
// branch outside nodes_im, i.e. the image with filled balls on skeleton nodes, and 
// outside ends_im, i.e. the image with filled balls on skeleton ends, is taken into 
// acccount.
#pragma omp parallel for
for (ct = 0; ct < (dimx * dimy * dimz); ct++) {
if (lbl_skl_im[ ct ] == NODETOEND_LABEL) {
tmp_im [ ct ] = OBJECT;
}
if (nodes_im[ ct ] == OBJECT) {
tmp_im [ ct ] = BACKGROUND;
}
if (ends_im[ ct ] == OBJECT) {
tmp_im [ ct ] = BACKGROUND;
}
}


// Perform connected components labeling of NODE-TO-END branches:
P3D_TRY(p3dConnectedComponentsLabeling(tmp_im, tmp_im2, &cc_array_numel, &cc_array, &bbs,
dimx, dimy, dimz, CONN26, P3D_TRUE));


if (cc_array_numel != 0) {
// Allocate memory for the distribution of widths on endpoints:
P3D_TRY(out_stats->NodeToEnd_Length = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->NodeToEnd_MinWidth = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->NodeToEnd_MeanWidth = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->NodeToEnd_MaxWidth = (double*) malloc(cc_array_numel * sizeof (double)));

// Copy NODE-TO-END branches statistics to output structure:
out_stats->NodeToEnd_Counter = cc_array_numel;

// Uncomment this line for taking into account NODE-TO-END branches
// in the computation of the connectivity density. However, taking into 
// account only NODE-TO-NODE branches the connectivity density can be 
// correctly interpreted as the number of redundant connections per 
// volume element.
//out_stats->ConnectivityDensity = out_stats->ConnectivityDensity - cc_array_numel;


// Compute mean length on NODE-TO-NODE branches:
for (ct = 0; ct < cc_array_numel; ct++) {
curr_bb = bbs[ct];

min_width = LONG_MAX;
mean_width = 0.0;
max_width = LONG_MIN;

// Scan the bounding box for minimum, mean and maximum value:			
for (k = curr_bb.min_z; k <= curr_bb.max_z; k++)
for (j = curr_bb.min_y; j <= curr_bb.max_y; j++)
for (i = curr_bb.min_x; i <= curr_bb.max_x; i++) {
if (tmp_im[ I(i, j, k, dimx, dimy) ] == OBJECT) {
min_width = MIN(min_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
max_width = MAX(max_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
mean_width += (double) dt_im [ I(i, j, k, dimx, dimy) ];
}
}

mean_width /= cc_array[ ct ];

// Copy NODE-TO-NODE branches statistics to output structure:
out_stats->NodeToEnd_Length[ct] = cc_array[ ct ] * voxelsize;
out_stats->NodeToEnd_MinWidth[ct] = 2 * sqrt(min_width) * voxelsize;
out_stats->NodeToEnd_MeanWidth[ct] = 2 * sqrt(mean_width) * voxelsize;
out_stats->NodeToEnd_MaxWidth[ct] = 2 * sqrt(max_width) * voxelsize;
}
} else {
// Copy NODE-TO-NODE branches statistics to output structure:
out_stats->NodeToEnd_Counter = 0;
out_stats->NodeToEnd_Length = NULL;
out_stats->NodeToEnd_MinWidth = NULL;
out_stats->NodeToEnd_MeanWidth = NULL;
out_stats->NodeToEnd_MaxWidth = NULL;
}

// Release resources after each call to _p3dConnectedComponentsLabeling:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_im2 != NULL) free(tmp_im2);


return P3D_SUCCESS;

MEM_ERROR:

// Release resources after each call to _p3dConnectedComponentsLabeling:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_im2 != NULL) free(tmp_im2);

return P3D_MEM_ERROR;
}

int _p3dSkeletonAnalysis_EndToEndBranches(
unsigned char* vol_im, // IN: Input segmented (binary) volume
unsigned short* dt_im, // IN: Input squared euclidean distance transform of vol_im
unsigned char* lbl_skl_im, // IN: Input labeled skeleton of the segmented volume
unsigned char* ends_im, // IN: Input image of identified nodes with merging criteria
struct SkeletonStats* out_stats, // OUT: Skeleton statistics
const int dimx,
const int dimy,
const int dimz,
const double voxelsize // IN: voxel resolution
) {
unsigned char* tmp_im = NULL;
unsigned short* tmp_im2 = NULL;

double min_width, mean_width, max_width;

unsigned int* cc_array = NULL;
int cc_array_numel;

bb_t* bbs = NULL;
bb_t curr_bb;

int ct, i, j, k;




//
// Isolate END-TO-END branches:
//

// Allocate memory for labeled skeleton:
P3D_TRY(tmp_im = (unsigned char*) malloc(dimx * dimy * dimz * sizeof (unsigned char)));
P3D_TRY(tmp_im2 = (unsigned short*) malloc(dimx * dimy * dimz * sizeof (unsigned short)));


// Set memory of tmp_im:
memset(tmp_im, BACKGROUND, dimx * dimy * dimz * sizeof (unsigned char));


// Create temporary matrix removing the filled balls. Doing so, only the part of a 
// branch outside ends_im, i.e. the image with filled balls on skeleton ends, is 
// taken into acccount.
#pragma omp parallel for
for (ct = 0; ct < (dimx * dimy * dimz); ct++) {
if (lbl_skl_im[ ct ] == ENDTOEND_LABEL) {
// Assign temporary image:
tmp_im [ ct ] = OBJECT;
}
if (ends_im[ ct ] == OBJECT) {
// Assign temporary image:
tmp_im [ ct ] = BACKGROUND;
}
}


// Perform connected components labeling of NODE-TO-NODE branches:
P3D_TRY(p3dConnectedComponentsLabeling(tmp_im, tmp_im2, &cc_array_numel, &cc_array, &bbs,
dimx, dimy, dimz, CONN26, P3D_TRUE));


if (cc_array_numel != 0) {
// Allocate memory for the distribution of widths on endpoints:
P3D_TRY(out_stats->EndToEnd_Length = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->EndToEnd_MinWidth = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->EndToEnd_MeanWidth = (double*) malloc(cc_array_numel * sizeof (double)));
P3D_TRY(out_stats->EndToEnd_MaxWidth = (double*) malloc(cc_array_numel * sizeof (double)));

// Copy NODE-TO-END branches statistics to output structure:
out_stats->EndToEnd_Counter = cc_array_numel;

// Compute mean length on NODE-TO-NODE branches:
for (ct = 0; ct < cc_array_numel; ct++) {
curr_bb = bbs[ct];

min_width = LONG_MAX;
mean_width = 0.0;
max_width = LONG_MIN;

// Scan the bounding box for minimum, mean and maximum value:			
for (k = curr_bb.min_z; k <= curr_bb.max_z; k++)
for (j = curr_bb.min_y; j <= curr_bb.max_y; j++)
for (i = curr_bb.min_x; i <= curr_bb.max_x; i++) {
if (tmp_im[ I(i, j, k, dimx, dimy) ] == OBJECT) {
min_width = MIN(min_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
max_width = MAX(max_width, (double) dt_im [ I(i, j, k, dimx, dimy) ]);
mean_width += (double) dt_im [ I(i, j, k, dimx, dimy) ];
}
}

mean_width /= cc_array[ ct ];

// Copy NODE-TO-NODE branches statistics to output structure:
out_stats->EndToEnd_Length[ct] = cc_array[ ct ] * voxelsize;
out_stats->EndToEnd_MinWidth[ct] = 2 * sqrt(min_width) * voxelsize;
out_stats->EndToEnd_MeanWidth[ct] = 2 * sqrt(mean_width) * voxelsize;
out_stats->EndToEnd_MaxWidth[ct] = 2 * sqrt(max_width) * voxelsize;
}
} else {
// Copy NODE-TO-NODE branches statistics to output structure:
out_stats->EndToEnd_Counter = 0;
out_stats->EndToEnd_Length = NULL;
out_stats->EndToEnd_MinWidth = NULL;
out_stats->EndToEnd_MeanWidth = NULL;
out_stats->EndToEnd_MaxWidth = NULL;
}

// Release resources after each call to _p3dConnectedComponentsLabeling:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_im2 != NULL) free(tmp_im2);


return P3D_SUCCESS;

MEM_ERROR:

// Release resources after each call to _p3dConnectedComponentsLabeling:
if (cc_array != NULL) free(cc_array);
if (bbs != NULL) free(bbs);
if (tmp_im != NULL) free(tmp_im);
if (tmp_im2 != NULL) free(tmp_im2);

return P3D_MEM_ERROR;
}

int p3dSkeletonAnalysis2(
unsigned char* vol_im, // IN: Input segmented (binary) volume
unsigned char* skl_im, // IN: Input (binary) skeleton of the segmented volume
struct SkeletonStats* out_stats, // OUT: Skeleton statistics
unsigned char* nodes_im,
unsigned char* ends_im,
unsigned char* throats_im,
const int dimx,
const int dimy,
const int dimz,
const double voxelsize, // IN: voxel resolution
int (*wr_log)(const char*, ...)
) {

// Temporary matrices:
unsigned char* lbl_skl_im;
unsigned short* dt_im;

double mean = 0.0;
int i = 0;
int ct;
int flag_nodes_null = P3D_FALSE;
int flag_ends_null = P3D_FALSE;
int flag_throats_null = P3D_FALSE;

char* auth_code;

//
// Authenticate:
//
#ifndef NO_AUTH  
auth_code = authenticate("p3dIterativeSkeletonPruning");
if (strcmp(auth_code, "1") != 0) goto MEM_ERROR;
#endif

// Start tracking computational time:
if (wr_log != NULL) {
p3dResetStartTime();
wr_log("Pore3D - Performing skeleton analysis...");
wr_log("\tAdopted voxelsize: %0.6f mm.", voxelsize);
}


// Allocate memory:
if (nodes_im == NULL) {
flag_nodes_null = P3D_TRUE;
P3D_TRY(nodes_im = (unsigned char*) calloc(dimx * dimy*dimz, sizeof (unsigned char)));
}
if (ends_im == NULL) {
flag_ends_null = P3D_TRUE;
P3D_TRY(ends_im = (unsigned char*) calloc(dimx * dimy*dimz, sizeof (unsigned char)));
}
if (throats_im == NULL) {
flag_throats_null = P3D_TRUE;
P3D_TRY(throats_im = (unsigned char*) calloc(dimx * dimy*dimz, sizeof (unsigned char)));
}

P3D_TRY(dt_im = (unsigned short*) malloc(dimx * dimy * dimz * sizeof (unsigned short)));
P3D_TRY(lbl_skl_im = (unsigned char*) malloc(dimx * dimy * dimz * sizeof (unsigned char)));

// Compute distance transform for further use:
P3D_TRY(p3dSquaredEuclideanDT(vol_im, dt_im, dimx, dimy, dimz));

// Perform skeleton labeling:
P3D_TRY(p3dSkeletonLabeling(skl_im, lbl_skl_im, dimx, dimy, dimz, NULL));


// ANALYSIS STEP. Computes number and lengths of each identified component.
// This implementation uses connected component labeling for segments but 
// also for nodes to mitigate thinning algorithm defects:

P3D_TRY(_p3dSkeletonAnalysis_EndPoints(vol_im, dt_im, lbl_skl_im, ends_im, out_stats,
dimx, dimy, dimz, voxelsize));
P3D_TRY(_p3dSkeletonAnalysis_NodePoints(vol_im, dt_im, lbl_skl_im, nodes_im, out_stats,
dimx, dimy, dimz, voxelsize));

P3D_TRY(_p3dSkeletonAnalysis_NodeToNodeBranches(vol_im, dt_im, lbl_skl_im, nodes_im,
out_stats, throats_im, dimx, dimy, dimz, voxelsize));
P3D_TRY(_p3dSkeletonAnalysis_NodeToEndBranches(vol_im, dt_im, lbl_skl_im, nodes_im, ends_im,
out_stats, dimx, dimy, dimz, voxelsize));
P3D_TRY(_p3dSkeletonAnalysis_EndToEndBranches(vol_im, dt_im, lbl_skl_im, ends_im,
out_stats, dimx, dimy, dimz, voxelsize));

// Set connectivity index. At this point, out_stats->ConnectivityDensity contains the 
// Euler number, i.e. nodes - branches (only NODE-TO-NODEs are considered). We need to 
// compute the final connectivity index as (1 - Euler#)/Volume:
out_stats->ConnectivityDensity = (1 - out_stats->ConnectivityDensity) /
(dimx * voxelsize * dimy * voxelsize * dimz * voxelsize);


// Print out number of connected components and mean values of parameters:
if (wr_log != NULL) {
wr_log("\t----");
wr_log("\tNumber of NODES: %d. ", out_stats->Node_Counter);
wr_log("\tNumber of ENDS: %d.", out_stats->End_Counter);
wr_log("\tNumber of NODE-TO-NODE branches: %d.", out_stats->NodeToNode_Counter);
wr_log("\tNumber of NODE-TO-END branches: %d.", out_stats->NodeToEnd_Counter);
wr_log("\tNumber of END-TO-END branches: %d.", out_stats->EndToEnd_Counter);
wr_log("\t----");


if (out_stats->Node_Counter > 0) {
// Compute mean values of volume:
mean = 0.0;
for (ct = 0; ct < (out_stats->Node_Counter); ct++) {
mean = mean + (double) (out_stats->Node_Width[ct]);
}
mean = mean / ((double) (out_stats->Node_Counter));

wr_log("\tMean values of width on NODES: %0.3f [mm].", mean);
}

if (out_stats->NodeToNode_Counter > 0) {
// Compute mean values of volume:
mean = 0.0;
for (ct = 0; ct < out_stats->NodeToNode_Counter; ct++)
mean = mean + (double) (out_stats->NodeToNode_MinWidth[ct]);
mean = mean / ((double) (out_stats->NodeToNode_Counter));

wr_log("\tMean values of throat size: %0.3f [mm].", mean);
}

if (out_stats->Node_Counter > 0) {
// Compute mean values of volume:
mean = 0.0;
for (ct = 0; ct < (out_stats->Node_Counter); ct++) {
mean = mean + (double) (out_stats->CoordinationNumber[ct]);
//wr_log ("\t\tCoordination number: %d ", out_stats->CoordinationNumber[ct] );
}
mean = mean / ((double) (out_stats->Node_Counter));

wr_log("\tMean coordination number: %0.3f [-].", mean);
}

if (out_stats->NodeToNode_Counter > 0) {
// Print out connectivity index:
wr_log("\tConnectivity density: %0.3f [mm^-3].", out_stats->ConnectivityDensity);
}
}


// Print elapsed time (if required):
if (wr_log != NULL) {
wr_log("Pore3D - Skeleton analysis performed successfully in %0.3f sec.", p3dGetElapsedTime());
}


// Release resources:
if (dt_im != NULL) free(dt_im);
if (lbl_skl_im != NULL) free(lbl_skl_im);
if (flag_nodes_null == P3D_TRUE) {
if (nodes_im != NULL) free(nodes_im);
nodes_im = NULL;
}
if (flag_ends_null == P3D_TRUE) {
if (ends_im != NULL) free(ends_im);
ends_im = NULL;
}
if (flag_throats_null == P3D_TRUE) {
if (throats_im != NULL) free(throats_im);
throats_im = NULL;
}

// Return OK:
return P3D_SUCCESS;


MEM_ERROR:

// Log a ERROR message:
if (wr_log != NULL) {
wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
}

// Free memory if previous malloc were successfully:
if (dt_im != NULL) free(dt_im);
if (lbl_skl_im != NULL) free(lbl_skl_im);
if (flag_nodes_null == P3D_TRUE) {
if (nodes_im != NULL) free(nodes_im);
nodes_im = NULL;
}
if (flag_ends_null == P3D_TRUE) {
if (ends_im != NULL) free(ends_im);
ends_im = NULL;
}
if (flag_throats_null == P3D_TRUE) {
if (throats_im != NULL) free(throats_im);
throats_im = NULL;
}


// Return error code and exit:
return P3D_MEM_ERROR;
}



*/
int p3dZeroPadding3D_8(
	unsigned char* in_rev,
	float* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k;

		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;

		// Set to zero all values:
		memset(out_rev, 0, a_dimx * a_dimy * a_dimz * sizeof (float));


		// Copy original (internal) values:
		for (k = 0; k < dimz; k++)
			for (j = 0; j < dimy; j++)
				for (i = 0; i < dimx; i++)
					out_rev[ I(i + size, j + size, k + size, a_dimx, a_dimy) ] = (float)
						in_rev[ I(i, j, k, dimx, dimy) ];

		// Return OK:
		return P3D_SUCCESS;
}

int p3dReplicatePadding3D_8(
	unsigned char* in_rev,
	float* out_rev,
	const int dimx, // ncols
	const int dimy, // nrows
	const int dimz, // nplanes
	const int size,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) {
		int a_dimx, a_dimy, a_dimz;
		int i, j, k, ct;


		// Compute dimensions of padded REV:
		a_dimx = dimx + size * 2;
		a_dimy = dimy + size * 2;
		a_dimz = dimz + size * 2;


		// Perform first zero padding:
		p3dZeroPadding3D_8(in_rev, out_rev, dimx, dimy, dimz, size, NULL, NULL);


		// Replicate border values:
		for (ct = size; ct > 0; ct--) {
			// Faces:

			for (i = ct; i < (a_dimx - ct); i++)
				for (j = ct; j < (a_dimy - ct); j++) {
					out_rev[ I(i, j, ct - 1, a_dimx, a_dimy) ] =
						out_rev[ I(i, j, ct, a_dimx, a_dimy) ];
					out_rev[ I(i, j, a_dimz - ct, a_dimx, a_dimy)] =
						out_rev[ I(i, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
				}

				for (j = ct; j < (a_dimy - ct); j++)
					for (k = ct; k < (a_dimz - ct); k++) {
						out_rev[ I(ct - 1, j, k, a_dimx, a_dimy) ] =
							out_rev[ I(ct, j, k, a_dimx, a_dimy) ];
						out_rev[ I(a_dimx - ct, j, k, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, j, k, a_dimx, a_dimy) ];
					}

					for (i = ct; i < (a_dimx - ct); i++)
						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(i, ct - 1, k, a_dimx, a_dimy) ] =
								out_rev[ I(i, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, k, a_dimx, a_dimy)] =
								out_rev[ I(i, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Edges:

						for (i = ct; i < (a_dimx - ct); i++) {
							out_rev[ I(i, ct - 1, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(i, ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(i, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(i, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];
							out_rev[ I(i, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(i, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (j = ct; j < (a_dimy - ct); j++) {
							out_rev[ I(ct - 1, j, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, j, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, ct - 1, a_dimx, a_dimy) ] =
								out_rev[ I(a_dimx - 1 - ct, j, ct, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, j, a_dimz - ct, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, j, a_dimz - 1 - ct, a_dimx, a_dimy) ];
						}

						for (k = ct; k < (a_dimz - ct); k++) {
							out_rev[ I(ct - 1, ct - 1, k, a_dimx, a_dimy) ] =
								out_rev[ I(ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, ct - 1, k, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, ct, k, a_dimx, a_dimy) ];
							out_rev[ I(ct - 1, a_dimy - ct, k, a_dimx, a_dimy) ] =
								out_rev[ I(ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
							out_rev[ I(a_dimx - ct, a_dimy - ct, k, a_dimx, a_dimy)] =
								out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, k, a_dimx, a_dimy) ];
						}

						// Corners:

						out_rev[ I(ct - 1, ct - 1, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


						out_rev[ I(a_dimx - ct, a_dimy - ct, ct - 1, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, ct, a_dimx, a_dimy) ];

						out_rev[ I(ct - 1, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, ct - 1, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];

						out_rev[ I(a_dimx - ct, a_dimy - ct, a_dimz - ct, a_dimx, a_dimy)] =
							out_rev[ I(a_dimx - 1 - ct, a_dimy - 1 - ct, a_dimz - 1 - ct, a_dimx, a_dimy) ];


		}

		// Return OK:
		return P3D_SUCCESS;
}

int p3dCrop3D_8(
        float* in_rev,
        unsigned char* out_rev,
        const int dimx, // ncols
        const int dimy, // nrows
        const int dimz, // nplanes
        const int size,
        int (*wr_log)(const char*, ...),
        int (*wr_progress)(const int, ...)
        ) {
    int a_dimx, a_dimy, a_dimz;
    int i, j, k;


    // Compute dimensions of padded REV:
    a_dimx = dimx - size * 2;
    a_dimy = dimy - size * 2;
    a_dimz = dimz - size * 2;

    // Copy original (internal) values:
    for (k = 0; k < a_dimz; k++)
        for (j = 0; j < a_dimy; j++)
            for (i = 0; i < a_dimx; i++)
			{
				// Set out voxel:                    
                if ( in_rev[ I(i + size, j + size, k + size, dimx, dimy) ] > UCHAR_MAX)
					out_rev[ I(i, j, k, a_dimx, a_dimy) ] = (unsigned char) UCHAR_MAX;
				else if ( in_rev[ I(i + size, j + size, k + size, dimx, dimy) ] < 0)
					out_rev[ I(i, j, k, a_dimx, a_dimy) ] = (unsigned char) 0;
                else
                    out_rev[ I(i, j, k, a_dimx, a_dimy) ] = (unsigned char) in_rev[ I(i + size, j + size, k + size, dimx, dimy) ];
			}

    // Return OK:
    return P3D_SUCCESS;
}

int p3dBilateralFilter3D_8(
        unsigned char* in_im,
        unsigned char* out_im,
        const int dimx,
        const int dimy,
        const int dimz,
        const int size,
        const double sigma_d,
        const double sigma_r,
        const int iter,
        int (*wr_log)(const char*, ...),
        int (*wr_progress)(const int, ...)
        ) {
    // Temporary padded input-volume:
    float* tmp_im;
    float* tmp_im2;

    int a_dimx, a_dimy, a_dimz;
    int i, j, k;
    int x, y, z;
    int ct;

    // Variables for computing gaussian kernel:
    int a_rad;
    double tmp;

    // Variables for filter management:
    double w, sum_f, sum_fi;

    char auth_code;

    //
    // Authenticate:
    //
    /*auth_code = authenticate("p3dBilateralFilter3D_8");
    if (auth_code == '0') goto AUTH_ERROR;*/

    // Start tracking computational time:
    if (wr_log != NULL) {
        //p3dResetStartTime();
        wr_log("Pore3D - Applying bilateral filter...");
        wr_log("\tKernel size: %d.", size);
        wr_log("\tDomain sigma: %0.3f.", sigma_d);
        wr_log("\tRange sigma: %0.3f.", sigma_r);
        wr_log("\tNumber of iterations: %d.", iter);
    }


    // Init variables:
    a_rad = size / 2; // integer division

    // Compute dimensions of padded REV:
    a_dimx = dimx + a_rad * 2;
    a_dimy = dimy + a_rad * 2;
    a_dimz = dimz + a_rad * 2;

    // Try to allocate memory:
    P3D_TRY(tmp_im = (float*) malloc(a_dimx * a_dimy * a_dimz * sizeof (float)));
    P3D_TRY(tmp_im2 = (float*) malloc(a_dimx * a_dimy * a_dimz * sizeof (float)));
    P3D_TRY(p3dReplicatePadding3D_8(in_im, tmp_im, dimx, dimy, dimz, a_rad, NULL, NULL));

    for (ct = 0; ct < iter; ct++) {
        // Volume scanning:
#pragma omp parallel for private(i, j, x, y, z, sum_f, sum_fi, w)
        for (k = a_rad; k < (a_dimz - a_rad); k++)
            for (j = a_rad; j < (a_dimy - a_rad); j++)
                for (i = a_rad; i < (a_dimx - a_rad); i++) {
                    // Init variables:
                    sum_f = 0.0;
                    sum_fi = 0.0;

                    // Convolve (i,j,k) voxel:
                    for (z = (k - a_rad); z <= (k + a_rad); z++)
                        for (y = (j - a_rad); y <= (j + a_rad); y++)
                            for (x = (i - a_rad); x <= (i + a_rad); x++) {
                                // Gaussian intensity weights:
                                w = exp(-(
                                        (tmp_im[ I(x, y, z, a_dimx, a_dimy) ] - tmp_im [ I(i, j, k, a_dimx, a_dimy) ])*
                                        (tmp_im[ I(x, y, z, a_dimx, a_dimy) ] - tmp_im [ I(i, j, k, a_dimx, a_dimy) ]) /
                                        (2.0 * sigma_r * sigma_r))
                                        - ((x - i)*(x - i) + (y - j)*(y - j) + (z - k)*(z - k)) / (2.0 * sigma_d * sigma_d)
                                        );

                                // Bilateral filter response:
                                sum_f  += w;
                                sum_fi += w * tmp_im[ I(x, y, z, a_dimx, a_dimy) ];      
                            }

                    // Set out voxel:                    
                    tmp_im2[ I(i, j, k, a_dimx, a_dimy) ] = (sum_fi / sum_f);
                }

        // Prepare for next iteration:
        memcpy(tmp_im, tmp_im2, a_dimx * a_dimy * a_dimz * sizeof (float));
		if (wr_log != NULL) {
			//p3dResetStartTime();
			wr_log("\Step %d performed", ct);
		}
    }

    // Crop image:
    P3D_TRY(p3dCrop3D_8(tmp_im2, out_im, a_dimx, a_dimy, a_dimz, a_rad, NULL, NULL));


    // Print elapsed time (if required):
    if (wr_log != NULL) {
        //wr_log("Pore3D - Bilateral filter applied successfully in %dm%0.3fs.", p3dGetElapsedTime_min(), p3dGetElapsedTime_sec());
    }

    // Release resources:
    if (tmp_im != NULL) free(tmp_im);
    if (tmp_im2 != NULL) free(tmp_im2);

    // Return success:
    return P3D_SUCCESS;


MEM_ERROR:

    if (wr_log != NULL) {
        wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
    }

    // Release resources:	
    if (tmp_im != NULL) free(tmp_im);
    if (tmp_im2 != NULL) free(tmp_im2);

    // Return error:
    return P3D_MEM_ERROR;

AUTH_ERROR:

    if (wr_log != NULL) {
        wr_log("Pore3D - Authentication error: %s. Program will exit.", auth_code);
    }

    return P3D_AUTH_ERROR;
}


int main(int argc, char* argv[]) {
	const int dimx = atoi(argv[3]);
	const int dimy = atoi(argv[4]);
	const int dimz = atoi(argv[5]);
	int ct;
	char str[255];


	/*unsigned char* nodes_im = NULL;
	unsigned char* ends_im = NULL;
	unsigned char* throats_im = NULL;*/

	//struct SkeletonStats skl_stats;
	//SkeletonStats skl_stats;

	// Allocate memory for input and output images:
	unsigned char* in_im = (unsigned char*) malloc(dimx * dimy * dimz * sizeof (unsigned char));
	unsigned char* out_im = (unsigned char*) malloc(dimx * dimy * dimz * sizeof (unsigned char));

	//skl_stats = (SkeletonStats*) malloc(sizeof(SkeletonStats));

	//omp_set_num_threads(8);

	// Read the slice:
	//__p3dReadRaw8 ( "/home/francesco.brun/Pore3D/r3_slice0060.raw", in_im, 1984, 1984, 1 );

	__p3dReadRaw8(argv[1], in_im, dimx, dimy, dimz); //, customPrint, NULL);


	//p3dSingleRegionGrowing3D_8(in_im, skl_im, dimx, dimy, dimz, 75, 75, 10, 0.2, CONN6, customPrint, NULL);


	//p3dMunchEtAlRingRemover2D_8(in_im, out_im, dimx, dimy, 991.5, 991.5, 8, 2.5, 1, 1.5, customPrint , NULL);
	//p3dAnisotropicDiffusionFilter3D_8(in_im, out_im, dimx, dimy, dimz, 1, 0.01, 0.01, 50, customPrint, customProgress);
	p3dBilateralFilter3D_8(in_im, out_im, dimx, dimy, dimz, 3, 1.0, 10.0, 50, customPrint, customProgress );
	//p3dGaussianFilter3D_8(in_im, out_im, dimx, dimy, dimz, 3, 30.0, customPrint, customProgress );
	//p3dSkeletonAnalysis2(in_im, skl_im, &skl_stats, nodes_im, ends_im, throats_im, dimx, dimy, dimz, 0.00298, customPrint);
	//p3dSkeletonAnalysis2(in_im, skl_im, &skl_stats, NULL, NULL, NULL, dimx, dimy, dimz, 0.00298, customPrint);


	__p3dWriteRaw8(out_im, argv[2], dimx, dimy, dimz);
	//__p3dWriteRaw8(out_im, "/home/francesco.brun/Pore3D/co010b_17_400x400x300.raw_flt", dimx, dimy, dimz);


	// Release resources:	
	if (in_im != NULL) free(in_im);
	if (out_im != NULL) free(out_im);
	/*if (nodes_im != NULL) free(nodes_im);
	if (ends_im != NULL) free(ends_im);
	if (throats_im != NULL) free(throats_im);*/

	// Free C memory:
	/* if (skl_stats.Node_Width != NULL) free ( skl_stats.Node_Width );
	if (skl_stats.End_Width != NULL) free ( skl_stats.End_Width );

	if (skl_stats.EndToEnd_Length != NULL) free ( skl_stats.EndToEnd_Length );
	if (skl_stats.EndToEnd_MinWidth != NULL) free ( skl_stats.EndToEnd_MinWidth );
	if (skl_stats.EndToEnd_MeanWidth != NULL) free ( skl_stats.EndToEnd_MeanWidth );
	if (skl_stats.EndToEnd_MaxWidth != NULL) free ( skl_stats.EndToEnd_MaxWidth );

	if (skl_stats.NodeToEnd_Length != NULL) free ( skl_stats.NodeToEnd_Length );
	if (skl_stats.NodeToEnd_MinWidth != NULL) free ( skl_stats.NodeToEnd_MinWidth );
	if (skl_stats.NodeToEnd_MeanWidth != NULL) free ( skl_stats.NodeToEnd_MeanWidth );
	if (skl_stats.NodeToEnd_MaxWidth != NULL) free ( skl_stats.NodeToEnd_MaxWidth );

	if (skl_stats.NodeToNode_Length != NULL) free ( skl_stats.NodeToNode_Length );
	if (skl_stats.NodeToNode_MinWidth != NULL) free ( skl_stats.NodeToNode_MinWidth );
	if (skl_stats.NodeToNode_MeanWidth != NULL) free ( skl_stats.NodeToNode_MeanWidth );
	if (skl_stats.NodeToNode_MaxWidth != NULL) free ( skl_stats.NodeToNode_MaxWidth );

	if (skl_stats.CoordinationNumber != NULL ) free ( skl_stats.CoordinationNumber ); */

	//if (skl_stats != NULL ) free ( skl_stats );


	//return 0;
}