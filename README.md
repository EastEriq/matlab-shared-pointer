# matlab-shared-pointer
Minimalist toolbox for sharing large byte arrays between matlab sessions via POSIX shared memory segments

## Functionality to support:

Matlab calls: the neatest would be to have a class `shm`, with properties like `.Id`
`.Pointer` and `Buffer` (equivalent to `.Pointer.data`), and the following methods.

## `shm.create(id,[size],[oflag])`

Interface to `shm_open()` (creates the file descriptor under `/dev/shm`), `ftruncate()`
(sizes it), `mmap()` (maps the memory in matlab's space) and `close()`.

This method populates the property `.Pointer` with a `libpointer` object,
whose data is of type `uint8` (simplistic choice to implement a raw buffer).

If `size` is not passed and `id`  points to an existing `/dev/shm` file, the existing
segment size is used (maybe via a call to `fstat()`). This allows to reconnect to previously created segments.

Standard `oflags` like `O_RDRW`, `O_CREAT` could be passed, but maybe they can assumed.

Congobating the three API functions in a single matlab call assumes that most of the times
the segments will not be resized, and mapped as soon as opened.


## `shm.detach`

Interface to `munmap()`, when the segment is no more needed in matlab.
It would be nice if it was possible to figure out the segment id from the pointer or viceversa.

## `shm.destroy()`

Interface to `shm_unlink()`, removes the segment identified by `shm.Id` completely
from system memory and `/dev/shm/`.

## Errors

Typical errors like `EACCES`, `EEXIST` etc. are reported.

## Target

Has to work at least for Matlab R2020b on Ubuntu 20.04.