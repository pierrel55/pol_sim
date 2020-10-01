// This program shows that Eberhard inequality can produce negative result with local realistic EPR simulation model without the need to invoke superdeterminism.
// With the model used here, the minimal J/N value can reach an average < -0.08 that represent around 38% of maximal theorical value. (-0.207)
// This code allows to conclude that a negative value of J/N > -0.08 does not allow to prove a non-local interaction.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// --------------------------------------------------------
// program configuration

#define GEN_SINGLE                               // produce single detection for particles in different state

#define EB_PAIRS 1000000                         // count of pairs used for Eberhard test

// ----------------------------------------------
// init constants

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

#define DEG(d) ((int)(((d)*R1)/180) & RMAX)      // define angle using degree to local integer units

#define EP_DIFF 90                               // in degree (integer), source emission polarisation difference between particles

// -------------------------------------
// define photon structure

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
  int _r0 = n_rand();
  int _r1 = n_rand();
  int _r2 = n_rand();

  int r0 = _r0;                                  // random polarisation
  int r1 = (_r1 + 2*_r2)/3;                      // produce exact Malus law

  // emit Alice
  a->a = r0;
  a->b = r1;

  // emit Bob with EP_DIFF
  b->a = (r0 + DEG(EP_DIFF)) & RMAX;
  b->b = r1;
}

// -------------------------------------
// polarizer switch

#if 1
// pass photon through polarizer.
static int polarize(struct pho_t *p, int a_pol)
{
  int o;                                         // output binary result
  int a = (R1 + p->a - a_pol) & RMAX;            // initial path offset modulus PI (here PI = R1)
  int b = 2*a + p->b;                            // adjusted path offset
  int c;                                         // destination distance

  if (a < (R1/2))
  {
    if   (b < R1)   { c = a;          o = 0; }
    else            { c = (R1/2) - a; o = 1; }
  }
  else
  {
    if   (b < 2*R1) { c = a - (R1/2); o = 1; }
    else            { c = R1 - a;     o = 0; }
  }

  if (c >= R1/4)  
    o |= 0x2;                                    // set bit state b1

#if 0                                            // update not required for this code
  // update photon states
  p->a = a_pol;
  p->b = (rand_pi() + 2*rand_pi())/3;
#endif
  return o;
}
#else
// speed optimized version
static int polarize(struct pho_t *p, int a_pol)
{
  int a = ((p->a - a_pol) & RMAX) << 1;          // photon/polarizer angle difference
  int b = ((a >> (RBITS-2)) & 0x2) ^ (0xbb74 >> ((((a + p->b) >> (RBITS-2)) & 0xc) | ((a >> (RBITS-1)) & 0x2)));
  return b;
}
#endif

// -------------------------------------
// EPR test

// detect pairs counter
union stat_ctr_t                                 // note: use union to access pairs using index
{
  int pair[8];                                   // pair access with index
  struct
  {
    //  AB                                       // Alice at left o=0 e=1
    int oo;                                      // pair detections
    int oe;
    int eo;
    int ee;

    int uo;                                      // single detections u=undetected
    int ue;
    int ou;
    int eu;
  };
};

// EPR test
static void epr_test_angles(int a_pol, int b_pol, int pair_emit, union stat_ctr_t *stat)
{
  int n;
  for (n=0; n<pair_emit; n ++)
  {
    struct pho_t pho_a, pho_b;                   // Alice and Bob photons
    int a, b, id;                                // polarizer status result

    emit_photon_pair(&pho_a, &pho_b);            // emit Alice and Bob

    a = polarize(&pho_a, a_pol);                 // polarize Alice
    b = polarize(&pho_b, b_pol);                 // polarize Bob
    id = ((a & 1) << 1) | (b & 1);               // get detect pair id [0..3]

#ifdef GEN_SINGLE
    // convert pairs with different photon states to single detection randomly on Alice or Bob
    if ((a ^ b) & 2)                             // detect pairs with different photon states
    {
      if (n_rand() < R1/2)                       // 50% of cases, undetect Alice or Bob
        id = 4 | (b & 1);                        // Alice not detected, single detection at Bob
      else
        id = 6 | (a & 1);                        // Bob not detected, single detection at Alice
    }
#endif

    stat->pair[id]++;                            // update stat counters
  }
}

// -----------------------------------------------------------
// Eberhard test

// Eberhard test results
struct eber_res_t
{
  int J;                                         // J
  double JoN;                                    // J/N
};

// Eberhard test
static void eber_test(int a1, int a2, int b1, int b2, struct eber_res_t *res)
{
  union stat_ctr_t a1_b1 = { 0 };
  union stat_ctr_t a1_b2 = { 0 };
  union stat_ctr_t a2_b1 = { 0 };
  union stat_ctr_t a2_b2 = { 0 };

  epr_test_angles(a1, b1, EB_PAIRS, &a1_b1);
  epr_test_angles(a1, b2, EB_PAIRS, &a1_b2);
  epr_test_angles(a2, b1, EB_PAIRS, &a2_b1);
  epr_test_angles(a2, b2, EB_PAIRS, &a2_b2);

  // define J and J/N
  res->J = (a1_b2.oe + a1_b2.ou + a2_b1.eo + a2_b1.uo + a2_b2.oo) - a1_b1.oo;
  res->JoN = (double)(res->J)/EB_PAIRS;
}

// -----------------------------------------------------------
// main test.

// Eberhard angles (in degree)
struct eb_angles_t
{
  double a1;
  double a2;
  double b1;
  double b2;
};

// array of test angles
static struct eb_angles_t eb_angles[] = 
{
  { 176.58, 10.61,  86.65, 64.08 },
  { 120.76, 96.92,  30.60, 33.14 },
  {  20.33, 22.02, 110.79, 78.96 },
  {  53.65, 48.87, 143.63, 17.26 },
  {  21.85, 30.57, 112.01, 85.85 },
  {  -1, 0, 0, 0 },                              // -1: define end of list
};

void main(void)
{
  struct eb_angles_t *an;
  srand(1234);                                   // to test with differents random seeds

  printf("Eberhard test by converting pairs of states 01 and 10 to single detection.\n\n");
  for (an = eb_angles; an->a1 > 0; an++)
  {
    struct eber_res_t res;                       // test result
    printf("Test for angles %.2f %.2f %.2f %.2f.. \t", an->a1, an->a2, an->b1, an->b2);

    eber_test(DEG(an->a1), DEG(an->a2), DEG(an->b1), DEG(an->b2), &res);
    printf("result: J/N = %.5f\n", res.JoN);
  }
}
