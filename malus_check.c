// Check classic Malus law
// Simulate a blocking polarizer.
// block e out and get o output transmission ratio.

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

#define PI_RAD 3.14159265358979323846

// -------------------------------------
// define photon with 2 hidden variables

struct pho_t
{
  int a;
  int b;
};

// -------------------------------------
// emit single photon in random state

static void emit_photon_single(struct pho_t *p)
{
  p->a = n_rand();
  p->b = (n_rand() + 2*n_rand())/3;
}

// -----------------------------
// polarize functions

// pass photon through polarizer.
// a_pol is local polarizer angle.
static int polarize(struct pho_t *p, int a_pol)
{
  int a = ((p->a - a_pol) & RMAX) << 1;
  int b = ((a >> (RBITS-2)) & 0x2) ^ (0xbb74 >> ((((a + p->b) >> (RBITS-2)) & 0xc) | ((a >> (RBITS-1)) & 0x2)));
  // update photon state
  p->a = a_pol;
  p->b = (n_rand() + 2*n_rand())/3;
  return b;
}

// -------------------------------------
// check Malus law

#define MAX_POL 100                              // max aligned polarizers

#define SIM_PHO 2000000                          // count of photons used for simulation

static double rand1(void)
{
  return (double)(rand())/RAND_MAX;
}

// simulation datas
struct sim_t
{
  double a_pol;                                  // polarizer angle used for test
  double t_res;                                  // theorical transmission results

  int a_pol_i;                                   // converted angle to local integer unit
  double s_res;                                  // simulated transmission results
};

static struct sim_t sim[MAX_POL];

// produce simulation results using sim datas.
// compare to theoricals results.
// return 1 if success
static int sim_malus_test(int n_pol)
{
  int i, err;
  int n_pho[MAX_POL] = { 0 };                    // count of photons passing o for each polarizer
  struct pho_t pho;
  struct sim_t *s = sim;                         // simulation datas

  // convert angles
  for (i=0; i<n_pol; i++)
    sim[i].a_pol_i = (int)((sim[i].a_pol*R1)/PI_RAD);

  // simulate
  for (i=0; i<SIM_PHO; i++)
  {
    int j;
    emit_photon_single(&pho);
    for (j=0; j<n_pol; j++)
    {
      int a = polarize(&pho, sim[j].a_pol_i);
      if (a & 1)                                 // pass e
        break;                                   // photon lost, send new one
      n_pho[j]++;                                // pass o
    }
  }

  // compute transmission ratio results
  for (i=0; i<n_pol; i++)
    sim[i].s_res = (double)(n_pho[i])/SIM_PHO;

  // compare with theoricals results.
  // allow 0.01 result difference (increase SIM_PHO to reduce value)
  err = 0;                                       // errors counter
  printf("p%d: ", n_pol);                        // print count of aligned polarizers
  for (i=0; i<n_pol; i++)
  {
    printf("%.2f(%.3f%%) ", (sim[i].a_pol*180.0)/PI_RAD, sim[i].s_res);  // print angle and transmission result
    err += (fabs(sim[i].t_res - sim[i].s_res) > 0.01);  // update error counter
  }

  if (err)
    printf("  FAIL(%d/%d)\n\n", err, n_pol);     // print count of errors
  else
    printf("  SUCCESS\n\n", n_pol);
  return !err;                                   // return result, 1 if ok
}

// init random angles until remaining more than 3% of initial photons
static int init_test_angles_random(void)
{
  int i;
  double a_pol = rand1()*PI_RAD;                 // random 0..PI
  double t = 0.5;                                // 1st polarizer pass 50%
  struct sim_t *s = sim;

  // 1st polarize pass 50 %
  s->a_pol = a_pol;
  s->t_res = t;
  s++;

  // define next using angle diff with previous
  for (i=1; i<MAX_POL; i++, s++)
  {
    double b_pol = fmod(a_pol + rand1()*(PI_RAD/4), PI_RAD);   // decal angle modulus PI
    double a_diff = fabs(b_pol - a_pol);
    double ma = pow(cos(a_diff), 2);
    t *= ma;
    s->a_pol = b_pol;
    s->t_res = t;                                // define theorical results
    if (t < 0.03)
      return i+1;
    a_pol = b_pol;
  }
  return i;                                      // return count of polarizer used
}

// make 'count' times test with random angles
static int check_malus_random(int count)
{
  int err_ctr = 0;
  int i;

  for (i=0; i<count; i++)
  {
    int n = init_test_angles_random();
    int r = sim_malus_test(n);
    if (!r)
      err_ctr++;
  }

  if (err_ctr)
  {
    printf("test failed %d/%d..\n", err_ctr, count);
    return 0;
  }

  printf("test passed..\n");
  return 1;
}

// init test with user defined angles
// nb: number of angles in list
static int test_user_angles(int nb, double *list_deg)
{
  int i;
  struct sim_t *s = sim;
  double t = 0.5;
  for (i=0; i<nb; i++, s++)
  {
    s->a_pol = (list_deg[i]*PI_RAD)/180.0;
    if (i)
    {
      double ar = ((list_deg[i] - list_deg[i-1])*PI_RAD)/180.0;
      t *= pow(cos(ar), 2);
    }
    s->t_res = t;
  }
  return sim_malus_test(nb);
}

// accuracy test using 91 polarizers step 1° [0..90]
// https://www.youtube.com/watch?v=S-ZEWGdGmak
// result is 0.486..
static void test_90p(void)
{
  int i;
  double an[91];
  printf("Test for 91 aligned polarizers, please wait..\n");  // take some time
  for (i=0; i<=90; i++)
    an[i] = i;
  test_user_angles(91, an);
}

// user defined angles
static double an_0[] = { 0.0, 45.0, 90.0 };

void main(void)
{
  check_malus_random(50);
  test_user_angles(3, an_0);
  test_90p();
}