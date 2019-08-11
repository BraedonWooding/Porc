#ifndef PORC_COMMON_H
#define PORC_COMMON_H

#ifndef __GNUC__
#define __PORC_ATTRIBUTE__(x) /* nothing */
#else
#define __PORC_ATTRIBUTE__(x) __attribute__((x))
#endif

#define __PORC_PACKED__ __PORC_ATTRIBUTE__(__packed__)

#include <inttypes.h>
#include <stddef.h>

typedef unsigned char byte;

#endif