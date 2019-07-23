#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdlib>

#define ASSERT(cond) \
    { \
        if (!(cond)) { \
            printf("ASSERT(%s) failed at %s L%i\n", #cond, __FILE__, __LINE__); \
            std::abort(); \
        } \
    }

#endif
