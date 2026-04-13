#include "mex.h"
#include "matrix.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

/* Helper function to get C string from MATLAB string */
static char* get_string_from_marray(const mxArray *prhs) {
    if (!mxIsChar(prhs)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Input must be a string, got a %s instead",
                 mxGetClassName(prhs));
        mexErrMsgIdAndTxt("MATLAB:shm:invalidInput",error_msg);
    }

    int buflen = (mxGetM(prhs) * mxGetN(prhs)) + 1;
    char *buf = mxMalloc(buflen);
    mxGetString(prhs, buf, buflen);
    return buf;
}


/* SHM_OPEN: Create or open a shared memory segment, map it and then close it
   mandatory argument: name; optional: size, oflag  */
void shm_open_wrapper(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) {
        mexErrMsgIdAndTxt("MATLAB:shm:invalidInput", "shm_open requires at least segment name");
    }

    char *shm_name = get_string_from_marray(prhs[0]);
    int oflag = O_RDWR | O_CREAT;
    size_t bsize;
    long unsigned int *pointer=NULL, *output;
    mode_t mode = 0666;

    /* Parse optional arguments */
    if (nrhs >= 2 && mxIsDouble(prhs[1])) {
        bsize = mxGetScalar(prhs[1]);
    }

    if (nrhs >= 3 && mxIsDouble(prhs[2]) || mxIsUint16(prhs[2])) {
        oflag = (int)mxGetScalar(prhs[2]);
    }

    /* Open the memory segment */
    int shm_descriptor = shm_open(shm_name, oflag, mode);

    if (shm_descriptor == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "shm_open of %s failed: %s", shm_name, strerror(errno));
        mxFree(shm_name);
        mexErrMsgIdAndTxt("MATLAB:shm:openFailed", error_msg);
    }

   /* set segment size */
   int tret = ftruncate(shm_descriptor,bsize);

    if (tret == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "ftruncate to size %ld failed: %s", bsize, strerror(errno));
        mxFree(shm_name);
        mexErrMsgIdAndTxt("MATLAB:shm:truncateFailed", error_msg);
    }

    /* map the segment and return its pointer */
    pointer = mmap(NULL, bsize, PROT_READ | PROT_WRITE, MAP_SHARED,
                  shm_descriptor,0);

    /* We can close the descriptor now */
    int cret = close(shm_descriptor);

    if (cret == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "shm_close failed: %s", strerror(errno));
        mexErrMsgIdAndTxt("MATLAB:shm:closeFailed", error_msg);
    }

    if (*pointer == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "mmap failed: %s", strerror(errno));
        mxFree(shm_name);
        mexErrMsgIdAndTxt("MATLAB:shm:mmapFailed", error_msg);
    }

    /* Return segment descriptor as output */
    plhs[0] = mxCreateDoubleScalar((double)shm_descriptor);
    /* Return mapped pointer if requested (FIXME)*/
    if (nlhs >= 2) {
//        plhs[1] = mxCreateDoubleScalar(&pointer);
        plhs[1] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
        //mxGetUint64s(plhs[1], (long) &pointer);
        output = (long unsigned int*) mxGetPr(plhs[1]);
        //mxSetUint64s(plhs[1], output);
        *output = (long unsigned int) &pointer;
        //printf("%lx\n",(long) &pointer);
    }

    mxFree(shm_name);
}

/* SHM_DETACH: Unmap and close a shared memory segment
   mandatory arguments: descriptor, size  */
void shm_detach_wrapper(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 2) {
        mexErrMsgIdAndTxt("MATLAB:shm:invalidInput", "shm_detach requires pointer and size");
    }

    int descriptor;
    size_t bsize;
    long unsigned int *pointer;

    /* Parse input arguments */
    pointer = (long unsigned int *) mxGetPr(prhs[0]);
    printf("%lx\n", *pointer);
    bsize = mxGetScalar(prhs[1]);
 
    /* unmap the segment */
    int uret = munmap((void *) pointer, bsize);

    if (uret == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "unmmap failed: %s", strerror(errno));
        mexErrMsgIdAndTxt("MATLAB:shm:unmmapFailed", error_msg);
    }

}

/* SHM_DESTROY: Remove the shared memory segment (unlink) 
                mandatory argument: name  */
void shm_destroy_wrapper(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) {
        mexErrMsgIdAndTxt("MATLAB:shm:invalidInput", "shm_open requires the segment name");
    }

    char *shm_name = get_string_from_marray(prhs[0]);

    /* Unlink the memory segment */
    int uret = shm_unlink(shm_name);

    if (uret == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "shm_unlink failed: %s", strerror(errno));
        mxFree(shm_name);
        mexErrMsgIdAndTxt("MATLAB:shm:unlinkFailed", error_msg);
    }

    mxFree(shm_name);
}


/* Main MEX function dispatcher */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) {
        mexErrMsgIdAndTxt("MATLAB:shm:invalidInput", 
            "Usage: shm_mex(command, ...)\nCommands: 'create', 'detach', 'destroy'");
    }

    /* remember that calling as a class method, the first rhs is the class object itself */
    char *command = get_string_from_marray(prhs[1]);

    if (strcmp(command, "create") == 0) {
        shm_open_wrapper(nlhs, plhs, nrhs - 1, &prhs[2]);
    } else if (strcmp(command, "detach") == 0) {
        shm_detach_wrapper(nlhs, plhs, nrhs - 1, &prhs[2]);
    } else if (strcmp(command, "destroy") == 0) {
        shm_destroy_wrapper(nlhs, plhs, nrhs - 1, &prhs[2]);
    } else {
        mxFree(command);
        mexErrMsgIdAndTxt("MATLAB:mqueue:unknownCommand", "Unknown command");
    }

    mxFree(command);
}

