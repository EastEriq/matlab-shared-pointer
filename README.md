# matlab-shared-pointer
Minimalist toolbox for sharing large byte arrays between matlab sessions via POSIX shared memory segments

## Functionality:

Implements a class `shm` representing a shared memory object. Its data is accessible for writing and reading as
`shm.Pointer.Value`. Other properties are readonly and set at the time of creation.

## `shm.create(id,[size],[oflag])`

Interface to `shm_open()` (creates the file descriptor under `/dev/shm`), `ftruncate()`
(sizes it), `mmap()` (maps the memory in matlab's space) and `close()`.

This method populates the property `.Pointer` with a `libpointer` object,
whose data is of type `uint8` (simplistic choice to implement a raw buffer).

Standard `oflags` like `O_RDRW`, `O_CREAT` can be passed. Some of them are
defined in the enum class `OFLAGS`. Use `bitor()` to combine them.

Congobating the four API functions in a single matlab call assumes that most of the times
the segments will not be resized, and mapped as soon as opened.

If `size` is not passed and `id`  points to an existing `/dev/shm` file, the existing
segment size is used. This allows to reconnect to previously created segments.


## `shm.detach`

Interface to `munmap()`, when the segment is no more needed in matlab. (this is also called by the class destructor
`.delete()`)

## `shm.destroy()`

Calls `.detach` if needed, and then interfaces to `shm_unlink()`,
removing the segment identified by `shm.Id` completely
from system memory and `/dev/shm/`, in fact equivalent to deleting the virtual file from OS.

## Errors

Typical errors like `EACCES`, `EEXIST` etc. are reported.

## Target

Has to work at least for Matlab R2020b on Ubuntu 20.04.