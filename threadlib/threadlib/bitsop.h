/**
 * @file bitsop.h
 * @author ahmed gublan
 * @brief   this file implement bits operations 
 * @version 0.1
 * @date 2022-01-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __BITS__
#define __BITS__ 

/* bit is 2^x, where x [0, 31] */

// #define IS_BIT_SET(n, bit)  (((n>>bit) & 0x1))
// #define TOGGLE_BIT(n, bit)  ((n ^= 1UL << bit))
// #define COMPLEMENT(n)       (n ^= 0xFFFFFFFF)
// #define UNSET_BIT(n, bit)   ((n &= ~(1UL << n)))
// #define SET_BIT(n, bit)     ((n |= (1UL << n)))
#define IS_BIT_SET(n, bit)      (n & bit)
#define TOGGLE_BIT(n, bit)      (n = (n ^ (bit)))
#define COMPLEMENT(n)           (n = (n ^ 0xFFFFFFFF))
#define UNSET_BIT(n, bit)       (n = (n & ((bit) ^ 0xFFFFFFFF)))
#define SET_BIT(n, bit)         (n = (n | (bit)))

#endif