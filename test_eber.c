// This program shows that the violation of Eberhard's inequality occurs if the probability of detecting
// a pair containing particles in a different state is less than the probability of detecting a pair of
// particles in an identical state.
// The level of the violation is then proportional to the difference in probability.
// The test performed here sets a 10% lower probability of detection, and shows by repeating the same
// test 100 times for a series of angles that the inequality is violated each time.
// The results show that this difference only generates very few simple measurements, around 1% of the
// total particles emitted.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ----------------------------------------------
// program configuration

#define DIST_1                                   // select distribution for b (DIST_1..DIST_4)

#define P_DETECT_U 90                            // probability to detect pair containing particles in different state % [0..100]

#define EB_PAIRS 500000                          // count of pairs used for single Eberhard test

#define MAX_JON -0.003                           // maximum J/N value to accept Eberhard test passed (significant enough inequality violation)

// ----------------------------------------------

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

#ifdef DIST_1
  int r1 = (_r1 + 2*_r2)/3;                      // produce exact Malus law
#elif defined(DIST_2)
  int r1 = (_r1 + _r2)/2;                        // produce exact cos² QM correlations
#elif defined(DIST_3)
  int r1 = (_r1 + _r2 + n_rand())/3;             // gaussian 1
#elif defined(DIST_4)
  int r1 = (_r1 + 2*_r2 + 3*n_rand())/6;         // gaussian 2
#else
  #error please select a distribution..
#endif

  // emit Alice
  a->a = r0;
  a->b = r1;

  // emit Bob with EP_DIFF
  b->a = (r0 + DEG(EP_DIFF)) & RMAX;
  b->b = r1;
}

// -------------------------------------
// polarizer switch

// pass photon through polarizer.
// a_pol is local polarizer angle.
static int polarize(struct pho_t *p, int a_pol)
{
  int a = ((p->a - a_pol) & RMAX) << 1;        // photon/polarizer angle difference
  int b = ((a >> (RBITS-2)) & 0x2) ^ (0xbb74 >> ((((a + p->b) >> (RBITS-2)) & 0xc) | ((a >> (RBITS-1)) & 0x2)));
  return b;
}

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
    int d;                                       // uncorrelated pair detect

    emit_photon_pair(&pho_a, &pho_b);            // emit Alice and Bob

    a = polarize(&pho_a, a_pol);                 // polarize Alice
    b = polarize(&pho_b, b_pol);                 // polarize Bob
    id = ((a & 1) << 1) | (b & 1);               // get detect pair id [0..3]

    // detect pairs with different photon states
    d = (a ^ b) & 2;                             // detect different particle states in pair

    // simulate single mesure for these pairs
    if (d && (n_rand() > ((P_DETECT_U*R1)/100))) // simulate single detection using detect probability
    {
      // produce randomly single mesure on Alice or Bob
      if (n_rand() < R1/2)                       // 50% of cases, undetect Alice or Bob
        id = 4 | (b & 1);                        // Alice not detected, single detection at Bob
      else
        id = 6 | (a & 1);                        // Bob not detected, single detection at Alice
    }
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
  double dr;                                     // particle detection ratio (for stats)
};

// return count of detected particules (pairs + single detections)
static int get_part_detect(union stat_ctr_t *s)
{
  return (s->oo + s->oe + s->eo + s->ee)*2 + s->uo + s->ue + s->ou + s->eu;
}

// acquisition for Eberhard test
static void eber_single_test(int a1, int a2, int b1, int b2, struct eber_res_t *res)
{
  double n_detect;
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
  
  // get count of particles detected to define a detection ratio (for stats)
  n_detect  = get_part_detect(&a1_b1);
  n_detect += get_part_detect(&a1_b2);
  n_detect += get_part_detect(&a2_b1);
  n_detect += get_part_detect(&a2_b2);
  res->dr = n_detect/(4*EB_PAIRS*2.0);
}

// -----------------------------------------------------------
// main test.

// Test Eberhard for a list of angle.
// Angles depend of b distribution.

struct eb_angles_t
{
  double a1;
  double a2;
  double b1;
  double b2;
};

// set of test angles for various b distributions
static struct eb_angles_t eb_angles[] = 
{
#ifdef DIST_1
  { 176.58, 10.61,  86.65, 64.08 },
  { 120.76, 96.92,  30.60, 33.14 },
  {  20.33, 22.02, 110.79, 78.96 },
  {  53.65, 48.87, 143.63, 17.26 },
  {  21.85, 30.57, 112.01, 85.85 },
#elif defined(DIST_2)
  {  92.72,  61.83,   2.31,  20.43 },
  { 162.45,  10.35,  72.47,  40.91 },
  {  83.05,  93.02, 173.52, 134.33 },
  { 152.42, 127.03,  62.03,  84.38 },
  { 130.62, 115.55,  40.59,  77.86 },
#elif defined(DIST_3)
  {  14.64,  36.56, 104.85,  67.66 },
  {  70.98,  70.14, 160.57,  45.54 },
  { 167.75, 179.44,  77.82,  50.64 },
  {   0.87,  13.78,  90.77,  61.67 },
  {  38.73, 161.21, 128.63, 141.80 },
#elif defined(DIST_4)
  { 176.51,  15.35,  86.57,  37.39 },
  { 109.28,  72.43,  19.00,  32.23 },
  {  20.43, 167.03, 110.34, 141.75 },
  {   2.02, 146.93,  91.96, 103.76 },
  {  12.40,  55.48, 102.46,  90.24 },
#endif
  {  -1, 0, 0, 0 },                              // -1: define end of list
};

// Pass 100 times same test.
// Count time J/N < MAX_JON
// Display count test passed and average J/N.
static void eber_test_x100(int a1, int a2, int b1, int b2)
{
  int i;
  int n_fail = 0;                                // J/N >= MAX_JON
  double JoN_avg = 0;                            // get average J/N value
  double dr_avg = 0;                             // get average detection ratio

  for (i=0; i<100; i++)
  {
    struct eber_res_t res;                       // test result
    eber_single_test(a1, a2, b1, b2, &res);      // single Eberhard test
    JoN_avg += res.JoN;                          // update J/N sum
    dr_avg += res.dr;                            // update detect ratio sum
    if (res.JoN >= MAX_JON)                      // update test fail counter
      n_fail++;
  }
  printf("Check result: %d fail/100. J/N average: %.5f detect ratio: %.2f %%\n", n_fail, JoN_avg/100.0, dr_avg);
}

void main(void)
{
  struct eb_angles_t *an;
  srand(1234);                                   // to test with differents random seeds
  for (an = eb_angles; an->a1 > 0; an++)
  {
    printf("Test 100x Eberhard for angles %.2f %.2f %.2f %.2f..\n", an->a1, an->a2, an->b1, an->b2);
    eber_test_x100(DEG(an->a1), DEG(an->a2), DEG(an->b1), DEG(an->b2));
  }
}


