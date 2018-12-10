// system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// acados
#include "acados/sim/sim_common.h"
#include "acados_c/sim_interface.h"
// mex
#include "mex.h"



void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
	{

//	mexPrintf("\nin sim_destroy\n");

	long long *ptr;

//	void *config = mxGetPr( mxGetField( prhs[0], 0, "config" ) );
//	long long *config_mat = (long long *) mxGetData( mxGetField( prhs[0], 0, "config" ) );
//	long long config = (long long) mxGetScalar( mxGetField( prhs[0], 0, "config" ) );



	/* RHS */

	// config
	ptr = (long long *) mxGetData( mxGetField( prhs[0], 0, "config" ) );
	sim_solver_config *config = (sim_solver_config *) ptr[0];
//	mexPrintf("\n%lld %p\n", config_mat[0], config_mat[0]);
	// dims
	ptr = (long long *) mxGetData( mxGetField( prhs[0], 0, "dims" ) );
	void *dims = (void *) ptr[0];
	// opts
	ptr = (long long *) mxGetData( mxGetField( prhs[0], 0, "opts" ) );
	sim_rk_opts *opts = (sim_rk_opts *) ptr[0];
	// in
	ptr = (long long *) mxGetData( mxGetField( prhs[0], 0, "in" ) );
	sim_in *in = (sim_in *) ptr[0];
	// out
	ptr = (long long *) mxGetData( mxGetField( prhs[0], 0, "out" ) );
	sim_out *out = (sim_out *) ptr[0];



	/* free memory */

	sim_config_free(config);
	sim_dims_free(dims);
	sim_opts_free(opts);
	sim_in_free(in);
	sim_out_free(out);



	/* return */

	return;

	}