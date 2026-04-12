classdef OFLAGS < uint16
    % dynamically, these values should be obtained on the specific
    %  system with #include <fcntl.h> (whic refers to further system
    %  includes)
    enumeration
        O_RDONLY   (0)    % (Open for reading only)
        O_WRONLY   (1)    % (Open for writing only)
        O_RDWR     (2)    % (Open for reading and writing)
        O_CREAT    (64)   % (Create if doesn't exist)
        O_EXCL     (128)  % (Fail if already exists)
        O_TRUNC    (512)  % (Truncate to zero bytes if already exists)
        O_NONBLOCK (2048) % (nonblocking receive)
    end
end
    