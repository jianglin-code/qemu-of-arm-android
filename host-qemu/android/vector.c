#include "vector.h"
#include <limits.h>


// #define  AVECTOR_ENSURE(obj,name,newCount) \
//     do { \
//         unsigned  _newCount = (newCount); \
//         if (_newCount > (obj)->max_##name) \
//             _avector_ensure( (void**)&(obj)->name, sizeof((obj)->name[0]), \
//                              &(obj)->max_##name, _newCount ); \
//     } while (0);

// void  _avector_ensure( void**  items, size_t  itemSize,
//                               unsigned*  pMaxItems, unsigned  newCount );

//  void
// _avector_ensure( void**  items, size_t  itemSize, unsigned*  pMaxItems, unsigned  newCount )
// {
//     unsigned  oldMax = *pMaxItems;

//     if (newCount > oldMax) {
//         unsigned  newMax = oldMax;
//         unsigned  bigMax = UINT_MAX / itemSize;

//         //if (itemSize == 0) {
//         //    AASSERT_FAIL("trying to reallocate array of 0-size items (count=%d)\n", newCount);
//         //}

//         //if (newCount > bigMax) {
//         //    AASSERT_FAIL("trying to reallocate over-sized array of %d-bytes items (%d > %d)\n",
//         //                 itemSize, newCount, bigMax);
//         //}

//         while (newMax < newCount) {
//             unsigned newMax2 = newMax + (newMax >> 1) + 4;
//             if (newMax2 < newMax || newMax2 > bigMax)
//                 newMax2 = bigMax;
//             newMax = newMax2;
//         }

//         *items     = g_realloc(*items,itemSize*newMax);//_android_array_realloc( *items, itemSize, newMax );
//         *pMaxItems = newMax;
//     }
// }

