#ifndef _SHA1_H
#define _SHA1_H

#include <stdlib.h>
#include "brg_types.h"

#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20

#if defined(__cplusplus)
extern "C"
{
#endif

/** BEGIN: UTS RNG Harness **/

#define POS_MASK    0x7fffffff
#define HIGH_BITS   0x80000000

#define sha1_context sha1_ctx_s
//#define RNG_state uint8
typedef u_int8_t RNG_state;
typedef u_int8_t uint8;
typedef u_int32_t  uint32;
//typedef char *   caddr_t;

/**********************************/
/* random number generator state  */
/**********************************/
struct state_t {
  uint8 state[20];
};


/***************************************/
/* random number generator operations  */
/***************************************/
void   rng_init(RNG_state *state, int seed);
void   rng_spawn(RNG_state *mystate, RNG_state *newstate, int spawnNumber);
int    rng_rand(RNG_state *mystate);
int    rng_nextrand(RNG_state *mystate);
char * rng_showstate(RNG_state *state, char *s);
void   rng_showtype( void );

/** END: UTS RNG Harness **/
/* type to hold the SHA256 context  */

struct sha1_ctx_s
{   uint_32t count[2];
    uint_32t hash[5];
    uint_32t wbuf[16];
};

typedef struct sha1_ctx_s sha1_ctx;

/* Note that these prototypes are the same for both bit and */
/* byte oriented implementations. However the length fields */
/* are in bytes or bits as appropriate for the version used */
/* and bit sequences are input as arrays of bytes in which  */
/* bit sequences run from the most to the least significant */
/* end of each byte                                         */

VOID_RETURN sha1_compile(sha1_ctx ctx[1]);

VOID_RETURN sha1_begin(sha1_ctx ctx[1]);
VOID_RETURN sha1_hash(const unsigned char data[], unsigned long len, sha1_ctx ctx[1]);
VOID_RETURN sha1_end(unsigned char hval[], sha1_ctx ctx[1]);
VOID_RETURN sha1(unsigned char hval[], const unsigned char data[], unsigned long len);

#if defined(__cplusplus)
}
#endif

#endif


