classdef shm < handle
    properties (GetAccess = public, SetAccess = private)
        Id char % char - must begin with /
        Pointer
    end
    properties (Hidden, GetAccess = public, SetAccess = private)
        Address
        Descriptor
        Size
    end
    methods
        % creator, with arguments
        function obj=shm(id,size,oflag)
            %
            if ~libisloaded('dereferencing_helper')
                fpath = mfilename('fullpath');
                d = fileparts(fpath);
                libdir=fullfile(d,'../src');
                loadlibrary(fullfile(libdir,'dereferencing_helper.so'),...
                            fullfile(libdir,'dereferencing_helper.h'));
            end
            if ~exist('size','var')
                size=0;
            end
            if ~exist('oflag','var')
                oflag = bitor(OFLAGS.O_RDWR, OFLAGS.O_CREAT);
            end
            [obj.Descriptor,obj.Address] = obj.shm_mex('create',id,size,oflag);
            obj.Id=id;
            obj.Size=size;
            % now the uint64 pointer has to be converted to a libpointer
            obj.Pointer=calllib('dereferencing_helper','addressToPointer',...
                                obj.Address);
            obj.Pointer.reshape(1,size);
        end

        % destructor
        function delete(obj)
            if ~isempty(obj.Descriptor)
                % just detach
                obj.detach;
            end
        end
 
        function destroy(obj)
            if ~isempty(obj.Descriptor)
                % detach first
                obj.detach;
            end
            % call shm_unlink()
            if ~isempty(obj.Id)
                obj.shm_mex('destroy',obj.Id);
            end
        end
        
        function detach(obj)
            % call munmap()
            obj.shm_mex('detach',obj.Address,obj.Size);
            obj.Address=[];
            obj.Pointer=[];
            obj.Descriptor=[];
            obj.Size=[];
        end
    end
end