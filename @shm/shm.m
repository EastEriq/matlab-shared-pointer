classdef shm < handle
    properties
        Id char % char - must begin with /
        Pointer
        Descriptor
        Size
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
            obj.Descriptor = obj.shm_mex('create',id,size,oflag);
            obj.Id=id;
            obj.Size=size;
            % now the uint64 pointer has to be converted to a libpointer
        end

        % destructor
        function delete(obj)
            if ~isempty(obj.Descriptor)
                % detach first
                obj.detach;
            end
            % call shm_unlink()
            obj.shm_mex('destroy',obj.Id);
        end
 
        function detach(obj)
            % call munmap() and close()
            obj.shm_mex('detach',obj.Pointer,obj.Size,obj.Descriptor);
            obj.Pointer=[];
            obj.Descriptor=[];
        end
    end
end