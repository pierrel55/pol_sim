// transmittance test.
// compare with Malus's law

// local model simulation
#include <math.h>
#include "epr.h"
#include "../win/debug.h"

// photon model
struct pho_t
{
  float p;         // polarization angle
  float q;         // hidden 1
  float r;         // hidden 2
};

// photon source single
static void emit_photon_single(struct pho_t *p)
{
  p->p = RAND_PI;
  p->q = RAND_PI;
  p->r = RAND_PI;
}

// simulate a blocking polarizer, (use only o out of a PBS)
static int polarize_transmit(struct pho_t *pho, float a_pol)
{
  int t;                                         // transmitted (o out)
  float d = pho->p - a_pol;                      // angle difference polarizer/photon polarization

#if 0
  float e = d + pho->q*(1.0f/3.0f) + pho->r*(1.0f/6.0f);
  if (d >= 0)                                    // positive angle difference
    t = (d <  PI/2) ? (e <  PI/2) : (e >= PI);
  else
    t = (d < -PI/2) ? (e < -PI/2) : (e >= 0);
#else
  // scaled e coefficient (x6). todo: test if is more accurate with float rounding.
  float e = 6*d + 2*pho->q + pho->r;
  if (d >= 0)                                    // positive angle difference
    t = (d <  PI/2) ? (e <  3*PI) : (e >= 6*PI);
  else
    t = (d < -PI/2) ? (e < -3*PI) : (e >= 0);
#endif

  // required for successive polarizations, update p/q/r
  if (t)                                         // o out
  {
    pho->p = a_pol;                              // align polarization on o
    // todo: find deterministic law to alter q and r after passing polarizer
    pho->q = RAND_PI;
    pho->r = RAND_PI;
    return true;
  }
  return false;                                  // photon blocked
}

// ----------------------------------------
// check transmittance conform to Malus law

#define MAX_POL 100                              // max aligned polarizers

// simulation datas
struct level_sim_t
{
  float a_pol;                                   // polarizer angle used for test
  double ref_res;                                // theoretical transmit ratio from 1st
};

static struct level_sim_t level_sim[MAX_POL];

// return positive angle modulus PI.
static float fmod_pi(float an)
{
  an = an - (int)(an / PI)*PI;
  return (an < 0) ? an += PI : an;
}

// produce simulation results using level_sim datas.
// compare to theoretical results.
static void sim_malus_test(int max_aligned, int N_sim, float max_error)
{
  int n_pho[MAX_POL] = { 0 };                    // count of transmitted photons for each polarizer
  int i, err_ctr;
  struct pho_t pho;

  // pass each photon through polarizer chain, stop if blocked
  for (i=0; i<N_sim; i++)
  {
    int j;
    emit_photon_single(&pho);                    // initials photons
    for (j=0; j<max_aligned; j++)
    {
      int t = polarize_transmit(&pho, level_sim[j].a_pol);
      if (!t)
        break;                                   // photon blocked
      n_pho[j]++;                                // photon passed
    }
    CHECK_THRD_CANCEL();
    EPR_PROGRESS((i*100)/N_sim);
  }

  // compare with theoretical results.
  // print angles and transmission ratio
  err_ctr = 0;                                   // errors counter
  for (i=0; i<max_aligned; i++)
  {
    int n_curr = n_pho[i];
    double new_tr = (double)(n_curr)/N_sim;
    double err_tr = fabs(level_sim[i].ref_res - new_tr);
    const char *err_msg = (err_tr > max_error) ? err_ctr++, "fail." : "ok.";

    co_printf("pol[%d]\t an: %.2f\t theoretical.: %.4f%%  sim.: %.4f%%  err: %.5f%%  %s\n",
      i+1,
      RAD_TO_DEG(level_sim[i].a_pol),
      level_sim[i].ref_res*100.0,
      new_tr*100.0,
      err_tr*100.0,
      err_msg);

    if (i == (max_aligned-1))                    // last, display cumulated error through all
      co_printf("Cumulated error from first polarizer: %.5f%%\n", err_tr*100.0);
  }

  if (err_ctr)
    co_printf("TEST FAIL(%d/%d)\n\n", err_ctr, max_aligned);
  else
    co_printf("TEST PASS\n\n");
}

// Init random angles until remaining more than 5% of initial photons
// Define theoretical check values using Malus law.
static int init_test_angles_random(int max_aligned, float an_var)
{
  int i;
  float a_pol = RAND_1*PI;                       // random 0..PI
  double t = 0.5;                                // 1st polarizer pass 50%
  struct level_sim_t *s = level_sim;

  // check argument
  if (max_aligned > MAX_POL)
    max_aligned = MAX_POL;

  // 1st polarizer pass 50 %
  s->a_pol = a_pol;
  s->ref_res = (float)t;
  s++;

  // define next using angle diff with previous
  for (i=1; i<max_aligned; i++, s++)
  {
    float da = (RAND_1 - 0.5f)*an_var;         // random angle variation +/- (PI/2)
    float b_pol = fmod_pi(a_pol + da);           // decal angle modulus PI
    double c = cos(b_pol - a_pol);               // cos angle diff
    double ma = c*c;                             // cos² Malus

    t *= ma;
    s->a_pol = b_pol;
    s->ref_res = t;                              // define theorical results
    if (t < 0.05)                                // remain less than 5% of initial photons (cannot evaluate ratio with accuracy)
      return i;
    a_pol = b_pol;
  }
  return i;                                      // return count of polarizer used
}

// make 'count' times test with random angles
void check_malus_random(int max_aligned, float an_var, int N_sim, float max_error)
{
  int N_pol = init_test_angles_random(max_aligned, an_var);
  sim_malus_test(N_pol, N_sim, max_error);
}

// test with user defined angles
void check_malus_user_angles(float *an_list_deg, int list_size, int N_sim, float max_error)
{
  int i;
  float t = 0.5;
  struct level_sim_t *s = level_sim;

  for (i=0; i<list_size; i++, s++)
  {
    float an = DEG_TO_RAD(an_list_deg[i]);
    s->a_pol = fmod_pi(an);                       // angles must be >= 0
    if (i)
    {
      float a_diff = s->a_pol - (s - 1)->a_pol;
      float c = cosf(a_diff);
      t *= c*c;
    }
    s->ref_res = t;
  }
  sim_malus_test(list_size, N_sim, max_error);
}
