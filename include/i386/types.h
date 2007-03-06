#ifndef __I386_TYPE
#define __I386_TYPE

/*****************************************
 *  NO 64bit support now
 *****************************************/

#ifndef int8_t
typedef signed char int8_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef int16_t
typedef signed short int int16_t;
#endif

#ifndef uint16_t
typedef unsigned short int uint16_t;
#endif

#ifndef int32_t
typedef signed int int32_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#endif /* I386_TYPES_H */
