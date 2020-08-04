// This program show how to obtain QM correlations using switch behavehour of a polarizer.
// note: result as graphic is available here http://pierrel5.free.fr/physique/sim_pol/sim_pol_gr_e.htm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// check C library produce compatible random values
#if (RAND_MAX & 0xfffff) == 0xfffff
  #define RBITS 20                               // limit to ensure no 32 bits overflow
#elif (RAND_MAX & 0x7fff) == 0x7fff
  #define RBITS 15
#else
  // should never happen with a standard stdlib.
  #error program require (2^n)-1 RAND_MAX value >= 0x7fff
#endif

#define R1 (1 << RBITS)                          // normalized 1 value for random max
#define RMAX (R1 - 1)                            // random max value
#define n_rand() (rand() & RMAX)                 // random function

#define DEG(d) ((int)(((d)*R1)/180) & RMAX)      // define angle using degree to local units

#define EP_DIFF 90                               // in degree (integer), source emission polarisation difference between particles

// -------------------------------------
// define photon with 2 hidden variables

struct pho_t
{
  int a;
  int b;
};

// -------------------------------------
// emit photon pair

// emit Alice and Bob photon pair
static void emit_photon_pair(struct pho_t *a, struct pho_t *b)
{
  int _r0 = n_rand();                            // define random numbers
  int _r1 = n_rand();
  int _r2 = n_rand();

  // emit Alice
  a->a = _r0;
  a->b = (_r1 + _r2)/2;                          // distribution to produce cos² correlations

  // emit Bob with EP_DIFF
  b->a = (a->a + DEG(EP_DIFF)) & RMAX;
  b->b = a->b;
}

// -------------------------------------
// polarizer switch

// macro return coincidences detected pair id
#define DETECT_ID(a, b) (((a ^ b) & 2) << 1) | ((a & 1) << 1) | (b & 1)

// pass photon through polarizer.
// a_pol is local polarizer angle.
// return polarization result in b.
static int polarize(struct pho_t *p, int a_pol)
{
  int a = ((p->a - a_pol) & RMAX) << 1;
  int b = ((a >> (RBITS-2)) & 0x2) ^ (0xbb74 >> ((((a + p->b) >> (RBITS-2)) & 0xc) | ((a >> (RBITS-1)) & 0x2)));
  return b;
}

// -------------------------------------
// EPR test

// detect pairs coincidence counters
union stat_ctr_t                                 // note: use union to access pairs using index
{
  int pair[8];                                   // pair access with index
  struct
  {
    //    AB                                     // Alice at left o=0 e=1
    int c_oo;                                    // same photon state correlated pairs
    int c_oe;
    int c_eo;
    int c_ee;
    int u_oo;                                    // different photon state uncorrelated pairs
    int u_oe;
    int u_eo;
    int u_ee;
  };
};

// EPR test
// a_pol : Alice polarizer angle
// b_pol : Bob polarizer angle
// pair_emit : count of pairs to emit
// stat : test stat results to return
static void epr_test_angles(int a_pol, int b_pol, int pair_emit, union stat_ctr_t *stat)
{
  int n;
  for (n=0; n<pair_emit; n ++)
  {
    struct pho_t pho_a, pho_b;                   // Alice and Bob photons
    int a, b;                                    // polarizer test result

    emit_photon_pair(&pho_a, &pho_b);            // emit Alice and Bob

    a = polarize(&pho_a, a_pol);                 // polarize Alice
    b = polarize(&pho_b, b_pol);                 // polarize Bob

    // update detection stats.
    stat->pair[DETECT_ID(a, b)]++;               // increment detected pair counter
  }
}

// -------------------------------------
// stats analysis

#define PI 3.14159265358979323846

static void print_stats(union stat_ctr_t *s, int pair_emit, int p_diff)
{
  // define theoricals values
  double a = ((p_diff + EP_DIFF)*PI)/180.0;      // angle in radians adjusted with EP_DIFF 
  double cos2 = pow(cos(a), 2);                  // cos²(a_diff), predict QM ++/-- correlations
  double sin2 = pow(sin(a), 2);                  // sin²(a_diff), predict QM +-/-+ correlations
  
  // get coincidence values using detected correlated pairs
  double co_count = s->c_oo + s->c_oe + s->c_eo + s->c_ee; // count of correlated pairs
  double co_same = (double)(s->c_oo + s->c_ee)/co_count;   // ratio detect ++/--
  double co_diff = (double)(s->c_oe + s->c_eo)/co_count;   // ratio detect +-/-+

  // print results
  printf("a_diff %d deg:\n"
      "\t++/--  cos2:%.3f  sim:%.3f  err:%.2f%%\n"
      "\t+-/-+  sin2:%.3f  sim:%.3f  err:%.2f%%\n",
      p_diff,                                     // polarizers angle diff
      cos2, co_same, fabs(cos2 - co_same)*100.0,  // compare error cos²
      sin2, co_diff, fabs(sin2 - co_diff)*100.0); // compare error sin²
}

// -------------------------------------
// main

#define PAIR_EMIT 1000000                        // count of pairs sended for test

void main(void)
{
  int p_diff;                                    // angle between polarizers
  printf ("QM detect coincidences test.\n\n");

  // test polarizer angles differences 0..90° in 5° steps
  for (p_diff = 0; p_diff <= 90; p_diff += 5)
  {
    union stat_ctr_t stat = { 0 };               // init empty stat counters
    int a_pol = n_rand();                        // define Alice polarizer random angle
    int b_pol = (a_pol + DEG(p_diff)) & RMAX;    // define Bob polarizer angle with p_diff offset
    
    // do test, get stats
    epr_test_angles(a_pol, b_pol, PAIR_EMIT, &stat);

    // print stats
    print_stats(&stat, PAIR_EMIT, p_diff);
  }
}
