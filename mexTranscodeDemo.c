/*
* GPL 3 License */

// MATLAB RELATED :

#include "mex.h"

/* Example Matlab Input Arguments */
#define	T_IN	prhs[0]
#define	Y_IN	prhs[1]

/* Example Matlab Output Arguments */
#define	YP_OUT	plhs[0]

#if !defined(MAX)
#define	MAX(A, B)	((A) > (B) ? (A) : (B))
#endif

#if !defined(MIN)
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#endif

static	double	mu = 1/82.45;
static	double	mus = 1 - 1/82.45;

/*  END/STOP of MATLAB Related */


/* Example global vars non matlab related */
/*
typedef struct FilteringContext {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;
static FilteringContext *filter_ctx;
*/

// int main(int argc, char **argv) //if not matlab compilation:
void mexTranscode( int nlhs, mxArray *plhs[], int nrhs, const mxArray*prhs[] ) //for Matlab
/*
{
    double *yp;
    double *t,*y;
    size_t m,n;

    // Check for proper number of arguments

    if (nrhs != 2) {
	    mexErrMsgIdAndTxt( "MATLAB:yprime:invalidNumInputs",
                "Two input arguments required.");
    } else if (nlhs > 1) {
	    mexErrMsgIdAndTxt( "MATLAB:yprime:maxlhs",
                "Too many output arguments.");
    }

    // Check the dimensions of Y.  Y can be 4 X 1 or 1 X 4.

    m = mxGetM(Y_IN);
    n = mxGetN(Y_IN);
    if (!mxIsDouble(Y_IN) || mxIsComplex(Y_IN) ||
	(MAX(m,n) != 4) || (MIN(m,n) != 1)) {
	    mexErrMsgIdAndTxt( "MATLAB:yprime:invalidY",
                "YPRIME requires that Y be a 4 x 1 vector.");
    }

    // Create a matrix for the return argument
    YP_OUT = mxCreateDoubleMatrix( (mwSize)m, (mwSize)n, mxREAL);

    // Assign pointers to the various parameters
    yp = mxGetPr(YP_OUT);

    t = mxGetPr(T_IN);
    y = mxGetPr(Y_IN);

    // Do the actual computations in a subroutine
    //yprime(yp,t,y);
    transcode(1,NULL);
    return;
}
*/
