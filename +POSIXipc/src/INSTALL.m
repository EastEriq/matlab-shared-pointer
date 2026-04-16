function INSTALL()
p = mfilename('fullpath');
d = fileparts(p);
mex(fullfile(d,'shm_mex.c'), '-R2018a', '-lrt', '-outdir', ...
    fullfile(d,'..','@shm/private'))

curdir=pwd;
cd(d)
if ~system(['gcc -c -fPIC dereferencing_helper.c -o dereferencing_helper.o;', ...
            'gcc -shared -o dereferencing_helper.so dereferencing_helper.o'])
    fprintf('dereferencing_helper.so compiled successfully\n')
end
cd(curdir)
