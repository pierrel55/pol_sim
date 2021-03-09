// random numbers generators

#define RNG_RAND_MAX 0x7fff                      // max value for rand_int

struct rng_t
{
  float (*rand1)(void);                          // return random [0..1[
  float (*rand_pi)(void);                        // return random [0..pi[
  int (*rand_int)(void);                         // return random [0..RNG_RAND_MAX]
  void (*srand)(int seed);                       // set seed
};

extern struct rng_t rng;

enum e_rng_type
{
  e_rng_c_lib = 0,
  e_rng_ms_c_lib,
  e_rng_xorshift,
  e_rng_cpu,
};

void rng_select_type(enum e_rng_type rnd_type);

// HW rng
extern bool hw_rng_detected;                     // hardware RNG detected
void rng_cpu_detect(void);                       // test if cpu contain hw generator

#define RAND_1 rng.rand1()
#define RAND_PI rng.rand_pi()

// integer
#define RAND_I rng.rand_int()
#define RAND_I_BITS 15