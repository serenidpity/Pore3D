// From C library:
#include <stdio.h>
#include <stdlib.h>

// Locals:
#include "_p3d_idlCommon.h"
#include "p3d_idlAdapter.h"

#include "p3dFilt.h"

IDL_VPTR p3d_idlCreateBinaryCircle(int argc, IDL_VPTR argv[], char* argk) {

    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD; // Must be first entry in structure
        IDL_LONG cen_data[2];
        int cen_there;
        IDL_MEMINT cen_n;
        IDL_LONG radius;
        int rad_there;
    } KW_RESULT;

    static IDL_KW_ARR_DESC_R center = {(char*) IDL_KW_OFFSETOF(cen_data), 2, 2, (IDL_LONG*) IDL_KW_OFFSETOF(cen_n)};


    // Alphabetical order is crucial:
    static IDL_KW_PAR kw_pars[] = {
        IDL_KW_FAST_SCAN,
        { "CENTER", IDL_TYP_LONG, 1, IDL_KW_ARRAY, (int*) IDL_KW_OFFSETOF(cen_there), (char*) (&center)},
        { "RADIUS", IDL_TYP_LONG, 1, 0, (int*) IDL_KW_OFFSETOF(rad_there), (char*) IDL_KW_OFFSETOF(radius)},
        { NULL}
    };

    KW_RESULT kw;

    IDL_VPTR idl_out_rev, idl_dims;
    IDL_INT* dims;
    IDL_MEMINT tmp_dims[2];
    unsigned char *out_rev8;
    int err_code;
    int radius;
    

    // Process keywords:
    IDL_KWProcessByOffset(argc, argv, argk, kw_pars, NULL, 1, &kw);

    // Get DIMX and DIMY input arguments:		
    idl_dims = argv[0];

    IDL_ENSURE_SIMPLE(idl_dims);
    IDL_ENSURE_ARRAY(idl_dims);


    if ((idl_dims->type != IDL_TYP_BYTE) && (idl_dims->type != IDL_TYP_INT) &&
            (idl_dims->type != IDL_TYP_UINT) && (idl_dims->type != IDL_TYP_LONG) &&
            (idl_dims->type != IDL_TYP_ULONG) && (idl_dims->type != IDL_TYP_LONG64) &&
            (idl_dims->type != IDL_TYP_ULONG64))
        _p3d_idlPrintNamedError("Input argument DIMS must be an array of integer type.");

        
    // User wants to read 2D RAW data:
    if (idl_dims->value.arr->n_elts == 2) {


        if (idl_dims->type == IDL_TYP_BYTE)
            dims = (UCHAR*) idl_dims->value.arr->data;
        else if (idl_dims->type == IDL_TYP_INT)
            dims = (IDL_INT*) idl_dims->value.arr->data;
        else if (idl_dims->type == IDL_TYP_UINT)
            dims = (IDL_UINT*) idl_dims->value.arr->data;
        else if (idl_dims->type == IDL_TYP_LONG)
            dims = (IDL_LONG*) idl_dims->value.arr->data;
        else if (idl_dims->type == IDL_TYP_ULONG)
            dims = (IDL_ULONG*) idl_dims->value.arr->data;
        else if (idl_dims->type == IDL_TYP_LONG64)
            dims = (IDL_LONG64*) idl_dims->value.arr->data;
        else if (idl_dims->type == IDL_TYP_ULONG64)
            dims = (IDL_ULONG64*) idl_dims->value.arr->data;

        tmp_dims[0] = dims[0];
        tmp_dims[1] = dims[1];
    } else {
        _p3d_idlPrintNamedError("Input argument DIMS must contain two [ X, Y ] elements.");
    }
        
        

   
   

    // Allocate memory for output:
    out_rev8 = (unsigned char *) IDL_MakeTempArray(
            IDL_TYP_BYTE,
            2,
            tmp_dims,
            IDL_ARR_INI_NOP,
            &idl_out_rev
            );

       
    // Get the RADIUS input argument:
    if (kw.rad_there) {
        // Get values:
        radius = (int) kw.radius;
    }
    else
    {
        radius = MIN(tmp_dims[0],tmp_dims[1]) / 2;
    }
    /*if (argv[1]->type == IDL_TYP_INT)
        radius = (unsigned int) argv[1]->value.i;
    else
        _p3d_idlPrintNamedError("Input argument RADIUS must be of type INT.");*/
    
    // Get CENTER keyword:
    if (kw.cen_there) {
        // Check values:
        if (kw.cen_n != 2)
            _p3d_idlPrintNamedError("Input argument CENTER must contain two [ X, Y ] elements.");

        if ((kw.cen_data[0] < 0) || (kw.cen_data[0] > tmp_dims[0]))
            _p3d_idlPrintNamedError("X value of input argument CENTER must be within specified DIM.");

        if ((kw.cen_data[1] < 0) || (kw.cen_data[1] > tmp_dims[1]))
            _p3d_idlPrintNamedError("Y value of input argument CENTER must be within specified DIM.");

        // Call Pore3D:
        err_code = p3dCreateBinaryCircle(
                out_rev8,
                (int) tmp_dims[0],
                (int) tmp_dims[1],
                (int) kw.cen_data[0],
                (int) kw.cen_data[1],
                radius,
                _p3d_idlPrintInfo
                );

        // On exception print error:
        if (err_code == P3D_MEM_ERROR)
            _p3d_idlPrintNamedError("Error on code execution.");
    } else {
        // Call Pore3D:
        err_code = p3dCreateBinaryCircle(
                out_rev8,
                (int) tmp_dims[0],
                (int) tmp_dims[1],
                (int) tmp_dims[0] / 2,
                (int) tmp_dims[1] / 2,
                radius,
                _p3d_idlPrintInfo
                );

        // On exception print error:
        if (err_code == P3D_MEM_ERROR)
            _p3d_idlPrintNamedError("Error on code execution.");
    }

    // Free resources:
    IDL_KW_FREE;

    // Return output in IDL Format
    return (idl_out_rev);
}
