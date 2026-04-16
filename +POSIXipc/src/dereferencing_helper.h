#include <stdint.h>

#define EXPORTC
#define STDCALL

EXPORTC uint64_t STDCALL pointerToAddress(uint8_t *datapointer);

EXPORTC uint8_t* STDCALL addressToPointer(uint64_t address);


