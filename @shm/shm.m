classdef shm < handle
    properties
        Id char % char - must begin with /
        Pointer libpointer
    end
    methods
        % creator, with arguments
        function obj=shm(id,size,oflag)
            if ~exist('size','var')
                size=0;
            end
            if ~exist('oflag','var')
                oflag = bitor(OFLAGS.O_RDWR, OFLAGS.O_CREAT);
            end
            pointer=shm_mex('create',id,size,oflag);
            % now the uint64 pointer has to be converted to a libpointer
            obj.Id=id;
        end

        % destructor
        function delete(obj)
            % call shm_unlink()
        end
 
        function detach(obj)
            % call munmap() and close()
            obj.Pointer=[];
        end
    end
end