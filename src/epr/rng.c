#include <stdlib.h>
#include <stdbool.h>
#include "rng.h"                                 // need common PI value
#include "../win/debug.h"

#define PI 3.14159265358979323846f

#define POW_32 4294967296                        // = 2^32
#define POW_24 16777216                          // = 2^24

// ----------------------------------------------------------------------
// c lib
// ----------------------------------------------------------------------

#if RAND_MAX >= 0xffffff
static float rng_c_lib_rand1(void)
{
  return (float)(rand() & 0xffffff) * (1.0f/POW_24);
}

static float rng_c_lib_rand_pi(void)
{
  return (float)(rand() & 0xffffff) * (PI/POW_24);
}
#else
static float rng_c_lib_rand1(void)
{
  return (float)(rand()) * (1.0f/(RAND_MAX+1));
}

static float rng_c_lib_rand_pi(void)
{
  return (float)(rand()) * (PI/(RAND_MAX+1));
}
#endif


static int rng_c_lib_rand_int(void)
{
#if RAND_MAX == RNG_RAND_MAX
  return rand();
#else
  return rand() & RNG_RAND_MAX;
#endif
}

static void rng_c_lib_srand(int seed)
{
  srand(seed);
}

// ----------------------------------------------------------------------
// ms c random
// ----------------------------------------------------------------------

static unsigned int ms_seed = 0;

#define MS_RAND_16 ((ms_seed = ms_seed * 214013L + 2531011L) >> 16)
#define DIV_16 65536                             // 2^16

float rng_ms_c_lib_rand1(void)
{
  return (float)MS_RAND_16 * (1.0f/DIV_16);
}

float rng_ms_c_lib_rand_pi(void)
{
  return (float)MS_RAND_16 * (PI/DIV_16);
}

int rng_ms_c_lib_rand_int(void)
{
  return MS_RAND_16 & RNG_RAND_MAX;
}

void rng_ms_c_lib_srand(int seed)
{
  ms_seed = seed;
}

// ----------------------------------------------------------------------
// xoshiro256+ (https://en.wikipedia.org/wiki/Xorshift)
// ----------------------------------------------------------------------

#ifdef __GNUC__
typedef unsigned int long uint64_t;
#else
typedef unsigned __int64 uint64_t;               // used type in xoshiro256 code
#endif

#define XO_GEN_COUNT 256                         // must be 2^n value
static int xo_id = 0;

struct xoshiro256p_state
{
  uint64_t s[4];
};

static unsigned int xoshiro256p(struct xoshiro256p_state *state)
{
  uint64_t *s = state->s;
  uint64_t t = s[1] << 17;
  unsigned int result = (unsigned int)(s[0] + s[3]);

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = (s[3] << 45) | (s[3] >> (64 - 45));

  return result;
}

static uint64_t splitmix64(uint64_t *seed64)
{
  uint64_t result = *seed64;
  *seed64 += 0x9E3779B97f4A7C15;
  result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
  result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
  return result ^ (result >> 31);
}

static void init_xoshiro(struct xoshiro256p_state *rs, uint64_t seed64)
{
  rs->s[0] = splitmix64(&seed64);
  rs->s[1] = splitmix64(&seed64);
  rs->s[2] = splitmix64(&seed64);
  rs->s[3] = splitmix64(&seed64);
}

// generators states
static struct xoshiro256p_state rs[XO_GEN_COUNT] = { 0 };

static uint64_t get_seed_64(void)
{
  uint64_t s64;
  s64 = MS_RAND_16;
  s64 = (s64 << 16) | MS_RAND_16;
  s64 = (s64 << 16) | MS_RAND_16;
  s64 = (s64 << 16) | MS_RAND_16;
  return s64;
}

// use a different generator at each call, this ensure no dependencies between two calls
#define XO_ID (xo_id = ((xo_id + 1) & (XO_GEN_COUNT - 1)))

static float xo_rand1(void)
{
  unsigned int r = xoshiro256p(&rs[XO_ID]);
  return (float)(r)*(1.0f/POW_32);
}

static float xo_rand_pi(void)
{
  unsigned int r = xoshiro256p(&rs[XO_ID]);
  return (float)(r)*(PI/POW_32);
}

static int xo_rand_int(void)
{
  unsigned int r = xoshiro256p(&rs[XO_ID]);
  return r & RNG_RAND_MAX;
}

static void xo_srand(int seed)
{
  int i;
  ms_seed = seed;
  xo_id = 0;
  for (i=0; i<XO_GEN_COUNT; i++)
    init_xoshiro(&rs[i], get_seed_64());
}

// ----------------------------------------------------------------------
// cpu build-in
// ----------------------------------------------------------------------

bool hw_rng_detected = false;                    // hardware RNG detected

#ifdef _MSVC                                     // visual studio

#include "intrin.h"                              // for __cpuid

#define CPU_RAND_MAX 0xffffffff                  // is POW_32 - 1

// return 0 if not detected else return 1
void rng_cpu_detect(void)
{
  int a[4];
  __cpuid(a, 0x1);
  hw_rng_detected = (a[2] & (1 << 30)) != 0;
}

static unsigned int cpu_rand(void)
{
  __asm
  {
    xor eax, eax

    ;RDRAND instruction = Set random value into EAX.
    ;Will set overflow [C] flag if success
    _emit 0x0F
    _emit 0xC7
    _emit 0xF0
  }
}
#else
// cpuid: RDRAND instruction = true
// add -mrdrnd option to compile

#include <cpuid.h>
#include <immintrin.h>

void rng_cpu_detect(void)
{
  hw_rng_detected = false;
  if (__get_cpuid_max(0, NULL))                  // test if __get_cpuid supported
  {
    unsigned int eax, ebx, ecx = 0, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
    {
      if (ecx & (1 << 30))
       hw_rng_detected = true;
    }
  }
}

static unsigned int cpu_rand(void)
{
  unsigned int res;// = 0UL;
  _rdrand32_step(&res);
  return res;
}
#endif

static float rng_cpu_rand1(void)
{
  return (float)cpu_rand() * (1.0f/POW_32);
}

static float rng_cpu_rand_pi(void)
{
  return (float)cpu_rand() * (PI/POW_32);
}

static int rng_cpu_rand_int(void)
{
  return cpu_rand() & RNG_RAND_MAX;
}

static void rng_cpu_srand(int seed)
{
  W_ASSERT(0);
}

// ----------------------------------------------------------------------
// inits/common
// ----------------------------------------------------------------------

struct rng_t rng;

void rng_select_type(enum e_rng_type rnd_type)
{
  switch (rnd_type)
  {
    case e_rng_c_lib:
      rng.rand1    = rng_c_lib_rand1;
      rng.rand_pi  = rng_c_lib_rand_pi;
      rng.rand_int = rng_c_lib_rand_int;
      rng.srand    = rng_c_lib_srand;
    break;
    case e_rng_ms_c_lib:
      rng.rand1    = rng_ms_c_lib_rand1;
      rng.rand_pi  = rng_ms_c_lib_rand_pi;
      rng.rand_int = rng_ms_c_lib_rand_int;
      rng.srand    = rng_ms_c_lib_srand;
    break;
    case e_rng_xorshift:
      rng.rand1    = xo_rand1;
      rng.rand_pi  = xo_rand_pi;
      rng.rand_int = xo_rand_int;
      rng.srand    = xo_srand;
    break;
    case e_rng_cpu:
      W_ASSERT(hw_rng_detected);
      rng.rand1    = rng_cpu_rand1;
      rng.rand_pi  = rng_cpu_rand_pi;
      rng.rand_int = rng_cpu_rand_int;
      rng.srand    = rng_cpu_srand;
    break;
    default:
      W_ASSERT(0);
  }
}

#if 0
// test code, eval distribution and speed
#include <time.h>
int ctr[RNG_RAND_MAX+128] = { 0 };

static float rng_dt = 0;

void rng_test(void)
{
  unsigned int t0, t1;
  //rng_select_type(e_rng_c_lib);            // 0.888s
  rng_select_type(e_rng_ms_c_lib);         // 0.143s
  //rng_select_type(e_rng_xorshift);           // 0.583s
  //rng_select_type(e_rng_cpu);
  rng.srand(-2564);

  t0 = clock();
#if 0
  while (ctr[0] < 1000)
    ctr[rng.rand_int() & 3]++;
#endif
#if 1
  while (ctr[0] < 1000)
  {
    int r = rng.rand_int();
    int i = 0;
    while (r)
    {
      ctr[i++] += r & 1;
      r >>= 1;
    }
  }
#endif
#if 0
  while (ctr[0] < 1000)
    ctr[rng.rand_int()]++;
#endif
#if 0
  while (ctr[0] < 1000)
    ctr[(int)((rng.rand1()*(RNG_RAND_MAX+1)) + 0.01f)]++;
#endif
#if 0
  while (ctr[0] < 1000)
    ctr[(int)((rng.rand_pi()*(RNG_RAND_MAX+1.0f))/PI + 0.01f)]++;
#endif
  t1 = clock();

  rng_dt = (float)(t1 - t0)/CLOCKS_PER_SEC;
  // here in debug, check uniform distribution in ctr
}
#endif

