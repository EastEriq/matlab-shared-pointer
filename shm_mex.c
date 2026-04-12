#include "mex.h"
#include "matrix.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/* Helper function to get C string from MATLAB string */
static char* get_string_from_marray(const mxArray *prhs) {
    if (!mxIsChar(prhs)) {
        mexErrMsgIdAndTxt("MATLAB:mqueue:invalidInput", "Input must be a string");
    }
    
    int buflen = (mxGetM(prhs) * mxGetN(prhs)) + 1;
    char *buf = mxMalloc(buflen);
    mxGetString(prhs, buf, buflen);
    return buf;
}


/* SHM_OPEN: Create or open a shared memory segment and map it 
   mandatory argument: name; optional: size, oflag  */
void shm_open_wrapper(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) {
        mexErrMsgIdAndTxt("MATLAB:mqueue:invalidInput", "shm_open requires at least queue name");
    }
    
    char *shm_name = get_string_from_marray(prhs[0]);
    int oflag = O_RDWR, bsize;
    mode_t mode = 0644;
    
    /* Parse optional arguments */
    if (nrhs >= 2 && mxIsDouble(prhs[1])) {
        bsize = mxGetScalar(prhs[1]);
    }
    
    if (nrhs >= 3 && mxIsDouble(prhs[2])) {
        oflag = (int)mxGetScalar(prhs[2]);
    }
    
    /* Open the memory segment */
    mqd = shm_open(shm_name, oflag, mode);
    
    if (mqd == -1) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "shm_open failed: %s", strerror(errno));
        mxFree(shm_name);
        mexErrMsgIdAndTxt("MATLAB:shm:openFailed", error_msg);
    }
    
    /* Return queue descriptor as output */
    plhs[0] = mxCreateDoubleScalar((double)mqd);
    
    mxFree(shm_name);
}


/* Main MEX function dispatcher */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) {
        mexErrMsgIdAndTxt("MATLAB:shm:invalidInput", 
            "Usage: shm(command, ...)\nCommands: 'create', 'detach', 'destroy'");
    }
    
    char *command = get_string_from_marray(prhs[0]);
    
    if (strcmp(command, "create") == 0) {
        shm_open_wrapper(nlhs, plhs, nrhs - 1, &prhs[1]);
    } else if (strcmp(command, "detach") == 0) {
        shm_detach_wrapper(nlhs, plhs, nrhs - 1, &prhs[1]);
    } else if (strcmp(command, "destroy") == 0) {
        shm_destroy_wrapper(nlhs, plhs, nrhs - 1, &prhs[1]);
    } else {
        mxFree(command);
        mexErrMsgIdAndTxt("MATLAB:mqueue:unknownCommand", "Unknown command");
    }
    
    mxFree(command);
}

