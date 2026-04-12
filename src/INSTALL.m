function INSTALL()
p = mfilename('fullpath');
d = fileparts(p);
mex(fullfile(d,'shm_mex.c'), '-g', '-R2018a', '-lrt', '-outdir', fullfile(d,'..','@shm'))
