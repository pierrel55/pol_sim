// local model simulation
#include <math.h>
#include "epr.h"
#include "epr_sim.h"
#include "../win/debug.h"

#define N1 (RNG_RAND_MAX+1)                      // fixed point integer 1.0 value for rand max

#ifdef USE_INT_LSIM

// Integer version code.
// Is more fast (about 2.3x) as do not require rng int to float conversions, but is less readable.
// No significant accuracy difference with float version.

#define NPI N1                                   // use pi normalized to 1 + fixed point
#define MSK1 RNG_RAND_MAX                        // 1 value and mask (is modulo PI)

// define photon
struct photon_t
{
  int p;                                         // polarization
  int q;                                         // hidden variable
  int r;                                         // hidden variable

  // simulation datas
  int p_time;                                    // elapsed time through polarizer
  int detect;                                    // detection result
};

// local 'entangled' source simulation
static void l_emit_ent_pair(struct photon_t *alice, struct photon_t *bob)
{
  int p = RAND_I;
  int q = RAND_I;
  int r = RAND_I;
  int p_bob;

  // emit Alice
  alice->p = p;
  alice->q = q;
  alice->r = r;

  // emit Bob
  p_bob = (p + epr_sim_conf.local.em_rdiff) & MSK1;    // Bob polarization + angle offset

  if (epr_sim_conf.local.s == N1)                // if 1, avoid to produce new randoms to optimize simulation speed
  {
    bob->p = p_bob;
    bob->q = q;
    bob->r = r;
  }
  else                                           // linear mix with independant randoms
  {
    int s0 = epr_sim_conf.local.s;
    int s1 = (N1 - s0);
    if (epr_sim_conf.local.local_polarize)       // force s=1 for p
      bob->p = p_bob;
    else
      bob->p = (s0*p_bob + s1*RAND_I) >> RAND_I_BITS;
    bob->q = (s0*q + s1*RAND_I) >> RAND_I_BITS;
    bob->r = (s0*r + s1*RAND_I) >> RAND_I_BITS;
  }
  
  // init sim datas
  alice->detect = DET_U;
  bob->detect = DET_U;
}

// local emit single
static void l_emit_single(struct photon_t *pho)
{
  pho->p = RAND_I;
  pho->q = RAND_I;
  pho->r = RAND_I;
  pho->detect = DET_U;
}

// local emit non entangled pair (same as entangled if s=0, and epr_sim_conf.local.local_polarize = false)
static void l_emit_ne_pair(struct photon_t *alice, struct photon_t *bob)
{
  alice->p = RAND_I;
  alice->q = RAND_I;
  alice->r = RAND_I;
  alice->detect = DET_U;

  bob->p = RAND_I;
  bob->q = RAND_I;
  bob->r = RAND_I;
  bob->detect = DET_U;
}

#if 1                                            // version use PI modulus (no negative angles)

// Local polarizer simulation + detection (merged to increase simulation speed).
static void l_polarize_detect(struct photon_t *pho, struct arm_t *arm)
{
  int e, rp;
  int d = pho->p - arm->an_pol;
  if (d < 0)
    d += NPI;                                    // get modulus PI positive value
  e = 6*d + 2*pho->q + pho->r;                   // switch potential (scaled x12, avoid divisions for speed and should be more accurate)

  // detect conditions
  #define D0(det) (RAND_I < det.dpr)
  #define D1(det, repol) ((rp = (repol)) <= det.max_rp)
  #define D2(det) ((pho->q*(NPI - pho->q)) <= det.max_q)
  // delay
  #define TM(delay) (rp < (NPI/4)) ? (rp + delay) : (rp + delay + arm->delay_st1)
  
#if 1                                            // allow fixed random numbers count
  #define PH_DETECT(out, det, delay, repol) \
  { if (D0(det) && D1(det, repol) && D2(det)) { pho->p_time = TM(delay); pho->detect = out; } }
#else
  #define PH_DETECT(out, det, delay, repol) \
  { if (D1(det, repol) && D2(det) && D0(det)) { pho->p_time = TM(delay); pho->detect = out; } }
#endif

  // define transmittance o counter for graph
  arm->tr_N++;
  #define DEF_TR_O_P() if (arm->tr_dat) arm->tr_dat[(d >> (RAND_I_BITS - TR_GRAPH_BITS)) & (TR_GRAPH_N-1)]++

  if (d < (NPI/2))
  {
    if  (e < 3*NPI)                              // compare with PI/4 scaled x12 limit
    {
      DEF_TR_O_P();
      PH_DETECT(OUT_O, arm->det_o, arm->delay_o, d)
    }
    else
      PH_DETECT(OUT_E, arm->det_e, arm->delay_e, NPI/2 - d)
  }
  else
  {
    if  (e < 6*NPI)                              // compare with PI/2 scaled x12 limit
      PH_DETECT(OUT_E, arm->det_e, arm->delay_e, d - NPI/2)
    else
    {
      DEF_TR_O_P();
      PH_DETECT(OUT_O, arm->det_o, arm->delay_o, NPI - d)
    }
  }


#if 0
  // is explained algorith in polarizer doc (produce same results)
  {
    int out1, rp1;
    if ((e >= 3*NPI) && (e < 6*NPI))
    {
      out1 = OUT_E;
      rp1 = d < (NPI/2) ? NPI/2 - d : d - NPI/2; 
    }
    else
    {
      out1 = OUT_O;
      rp1 = d < (NPI/2) ? d : NPI - d;
    }
    W_ASSERT(out1 == pho->detect);               // compare out results
    W_ASSERT(rp1 == pho->p_time);                // compare repolarization result
  }
#endif

}

#else
// do not use modulus
// Local polarizer simulation + detection (merged to increase simulation speed).
static void l_polarize_detect(struct photon_t *pho, struct arm_t *arm)
{
  int rp;
  int d = pho->p - arm->an_pol;                  // angle difference polarizer/photon polarization
  int e = 6*d + 2*pho->q + pho->r;               // potential to nearest output (scaled x6)

  // detect conditions
  #define D0(det) (RAND_I < det.dpr)
  #define D1(det, repol) ((rp = (repol)) <= det.max_rp)
  #define D2(det) ((pho->q*(NPI - pho->q)) <= det.max_q)
  // delay
  #define TM(delay) (rp < (NPI/4)) ? (rp + delay) : (rp + delay + arm->delay_st1)
  
#if 1                                            // allow fixed random numbers count
  #define PH_DETECT(out, det, delay, repol) \
  { if (D0(det) && D1(det, repol) && D2(det)) { pho->p_time = TM(delay); pho->detect = out; } }
#else
  #define PH_DETECT(out, det, delay, repol) \
  { if (D1(det, repol) && D2(det) && D0(det)) { pho->p_time = TM(delay); pho->detect = out; } }
#endif

  // define transmittance o counter for graph
  arm->tr_N++;
  #define DEF_TR_O_P() if (arm->tr_dat) arm->tr_dat[(d >> (RAND_I_BITS - TR_GRAPH_BITS)) & (TR_GRAPH_N-1)]++
  #define DEF_TR_O_N() if (arm->tr_dat) arm->tr_dat[((N1 + d) >> (RAND_I_BITS - TR_GRAPH_BITS)) & (TR_GRAPH_N-1)]++

  if (d >= 0)                                    // positive angle difference
  {
    if (d < (NPI/2))
    {
      if  (e < 3*NPI)
      {
        DEF_TR_O_P();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, d)
      }
      else
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, NPI/2 - d)
    }
    else
    {
      if  (e < 6*NPI)  
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, d - NPI/2)
      else
      {
        DEF_TR_O_P();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, NPI - d)
      }
    }
  }
  else                                           // negative angle difference
  {
    if (d < (-NPI/2))
    {
      if  (e < -3*NPI)
      {
        DEF_TR_O_N();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, NPI + d)
      }
      else
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, -NPI/2 - d)
    }
    else
    {
      if  (e < 0)
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, NPI/2 + d)
      else
      {
        DEF_TR_O_N();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, -d)
      }
    }
  }
}
#endif

#else

// Float version code.
// Used to check and compare with integer version

#define NPI 1.0f                                 // use pi normalized to 1 (is more accurate)

// define photon
struct photon_t
{
  float p;                                       // polarization
  float q;                                       // hidden variable
  float r;                                       // hidden variable

  // simulation datas
  float p_time;                                  // elapsed time through polarizer
  int detect;                                    // detection result
};

// local 'entangled' source simulation
static void l_emit_ent_pair(struct photon_t *alice, struct photon_t *bob)
{
  float p = RAND_1;
  float q = RAND_1;
  float r = RAND_1;
  float p_bob;

  // emit Alice
  alice->p = p;
  alice->q = q;
  alice->r = r;

  // emit Bob
  p_bob = p + epr_sim_conf.local.em_rdiff;             // Bob polarization + angle offset
  if (p_bob >= NPI)
    p_bob -= NPI;                                // value must be modulus NPI

  if (epr_sim_conf.local.s == 1.0f)              // if 1, avoid to produce new randoms to optimize simulation speed
  {
    bob->p = p_bob;
    bob->q = q;
    bob->r = r;
  }
  else                                           // linear mix with independant randoms
  {
    float s0 = epr_sim_conf.local.s;
    float s1 = (1.0f - s0);
    if (epr_sim_conf.local.local_polarize)       // force s=1 for p
      bob->p = p_bob;
    else
      bob->p = s0*p_bob + s1*RAND_1;
    bob->q = s0*q + s1*RAND_1;
    bob->r = s0*r + s1*RAND_1;
  }
  
  // init sim datas
  alice->detect = DET_U;
  bob->detect = DET_U;
}

// local emit single
static void l_emit_single(struct photon_t *pho)
{
  pho->p = RAND_1;
  pho->q = RAND_1;
  pho->r = RAND_1;
  pho->detect = DET_U;
}

// local emit non entangled pair (same as entangled if s=0, and epr_sim_conf.local.local_polarize = false)
static void l_emit_ne_pair(struct photon_t *alice, struct photon_t *bob)
{
  alice->p = RAND_1;
  alice->q = RAND_1;
  alice->r = RAND_1;
  alice->detect = DET_U;

  bob->p = RAND_1;
  bob->q = RAND_1;
  bob->r = RAND_1;
  bob->detect = DET_U;
}

// Local polarizer simulation + detection (merged to increase simulation speed).
static void l_polarize_detect(struct photon_t *pho, struct arm_t *arm)
{
  float rp;
  float d = pho->p - arm->an_pol;                // angle difference polarizer/photon polarization
  float e = 6*d + 2*pho->q + pho->r;             // potential to nearest output (scaled x6)

  // detect conditions
  #define D0(det) (RAND_1 < det.dpr)
  #define D1(det, repol) ((rp = (repol)) <= det.max_rp)
  #define D2(det) ((pho->q*(NPI - pho->q)) <= det.max_q)
  // delay
  #define TM(delay) (rp < (NPI/4)) ? (rp + delay) : (rp + delay + arm->delay_st1)
  
#if 1                                            // allow fixed random numbers count
  #define PH_DETECT(out, det, delay, repol) \
  { if (D0(det) && D1(det, repol) && D2(det)) { pho->p_time = TM(delay); pho->detect = out; } }
#else
  #define PH_DETECT(out, det, delay, repol) \
  { if (D1(det, repol) && D2(det) && D0(det)) { pho->p_time = TM(delay); pho->detect = out; } }
#endif

  // define transmittance o counter for graph
  arm->tr_N++;
  #define DEF_TR_O_P() if (arm->tr_dat) arm->tr_dat[(int)(d*TR_GRAPH_N) & (TR_GRAPH_N-1)]++
  #define DEF_TR_O_N() if (arm->tr_dat) arm->tr_dat[(TR_GRAPH_N + (int)(d*TR_GRAPH_N)) & (TR_GRAPH_N-1)]++

  if (d >= 0)                                    // positive angle difference
  {
    if (d < (NPI/2))
    {
      if  (e < 3*NPI)
      {
        DEF_TR_O_P();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, d)
      }
      else
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, NPI/2 - d)
    }
    else
    {
      if  (e < 6*NPI)  
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, d - NPI/2)
      else
      {
        DEF_TR_O_P();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, NPI - d)
      }
    }
  }
  else                                           // negative angle difference
  {
    if (d < (-NPI/2))
    {
      if  (e < -3*NPI)
      {
        DEF_TR_O_N();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, NPI + d)
      }
      else
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, -NPI/2 - d)
    }
    else
    {
      if  (e < 0)
        PH_DETECT(OUT_E, arm->det_e, arm->delay_e, NPI/2 + d)
      else
      {
        DEF_TR_O_N();
        PH_DETECT(OUT_O, arm->det_o, arm->delay_o, -d)
      }
    }
  }
}

#endif

// local simulation
void local_simulate(float an_a, float an_b, struct det_ctr_t *ctr)
{
  int n1, n = 0;
  struct arm_t *arm_a = &epr_sim_conf.local.arm_a;
  struct arm_t *arm_b = &epr_sim_conf.local.arm_b;
  struct photon_t a;                             // Alice
  struct photon_t b;                             // Bob

#ifdef USE_INT_LSIM
  int win_time = epr_sim_conf.local.win_time;
  int wt_max, j_beta, j_min;
  arm_a->an_pol = ((int)((an_a*NPI) / PI) + NPI) & MSK1;
  arm_b->an_pol = ((int)((an_b*NPI) / PI) + NPI) & MSK1;

#if 0
  W_ASSERT(0);  // overflow in 32 bits
  #define SRC_JFUNC_I(s, r)  ((s)/((N1/2) + 8*(r)))
  j_beta = (int)SC_JFUNC(epr_sim_conf.local.src_beta_scale)*N1;
  j_min  = (int)SRC_JFUNC_I(j_beta, N1);
  #define J_DELAY j_delay = (SRC_JFUNC_I(j_beta, RAND_I) - j_min)
#else
  #define SRC_JFUNC_I(s, r)  ((s)/((N1/16) + (r)))
  j_beta = (int)SC_JFUNC(epr_sim_conf.local.src_beta_scale)*(N1/8);
  j_min  = (int)SRC_JFUNC_I(j_beta, N1);
  #define J_DELAY (SRC_JFUNC_I(j_beta, RAND_I) - j_min)
#endif

#else
  // sim using floats
  float win_time = epr_sim_conf.local.win_time;
  float wt_max, j_beta, j_min;
  if (an_a >= PI) an_a -= PI;                    // polarizer require positives angles values
  if (an_b >= PI) an_b -= PI;
  arm_a->an_pol = an_a/PI;                       // normalize
  arm_b->an_pol = an_b/PI;
  W_ASSERT((arm_a->an_pol >= 0) && (arm_a->an_pol < 1.0f));
  W_ASSERT((arm_b->an_pol >= 0) && (arm_b->an_pol < 1.0f));

  j_beta = SC_JFUNC(epr_sim_conf.local.src_beta_scale);
  j_min  = SRC_JFUNC(j_beta, 1.0f);
  #define J_DELAY (SRC_JFUNC(j_beta, RAND_1) - j_min)
#endif
  W_ASSERT(!epr_sim_conf.qm_mode);

  #define A_P ((a.p_time < wt_max) ? (a.detect << 2) : DET_U)
  #define B_P ((b.p_time < wt_max) ? b.detect : DET_U)
  #define D_MSK (A_P | B_P)

  // entangled pair 
  if (epr_sim_conf.n_em_ent_pair > 0)
  {
    n1 = n + epr_sim_conf.n_em_ent_pair;
    for (; n < n1; n++)
    {
      wt_max = j_beta ? (win_time - J_DELAY) : win_time;
      l_emit_ent_pair(&a, &b);
      l_polarize_detect(&a, arm_a);
      l_polarize_detect(&b, arm_b);
      epr_count(D_MSK, ctr);
    }
  }

  // non entangled pair
  if (epr_sim_conf.n_em_n_ent_pair > 0)
  {
    n1 = n + epr_sim_conf.n_em_n_ent_pair;
    for (; n < n1; n++)
    {
      wt_max = j_beta ? (win_time - J_DELAY) : win_time;
      l_emit_ne_pair(&a, &b);
      l_polarize_detect(&a, arm_a);
      l_polarize_detect(&b, arm_b);
      epr_count(D_MSK, ctr);
    }
  }

  // single
  if (epr_sim_conf.n_em_single > 0)
  {
    n1 = n + epr_sim_conf.n_em_single;
    for (; n < n1; n++)
    {
      wt_max = j_beta ? (win_time - J_DELAY) : win_time;
      if (RAND_1 < 0.5)                          // emit 50%/50% on arm Alice/Bob
      {
        l_emit_single(&a);
        l_polarize_detect(&a, arm_a);
        epr_count(A_P, ctr);
      }
      else
      {
        l_emit_single(&b);
        l_polarize_detect(&b, arm_b);
        epr_count(B_P, ctr);
      }
    }
  }

  // double entangled pairs
  if (epr_sim_conf.n_em_ent_double > 0)
  {
    n1 = n + epr_sim_conf.n_em_ent_double;
    for (; n < n1; n++)
    {
      int m;
      wt_max = j_beta ? (win_time - J_DELAY) : win_time;

      l_emit_ent_pair(&a, &b);
      l_polarize_detect(&a, arm_a);
      l_polarize_detect(&b, arm_b);
      m = D_MSK;

      l_emit_ent_pair(&a, &b);
      l_polarize_detect(&a, arm_a);
      l_polarize_detect(&b, arm_b);
      epr_count(m | (D_MSK << 4), ctr);
    }
  }

  // remain is no emission
  for (; n < epr_sim_conf.N; n++)
    epr_count(0, ctr);
}

// resources defined in res.c
extern const float det_max_rp_to_dpr[1000];
extern const float det_max_q_to_dpr[1000];

static float get_max_rp(float dpr)
{
  int i;
  for (i=0; i<1000; i++)
    if (det_max_rp_to_dpr[i] > dpr)
      break;
  return (float)i/2000.0f;
}

static float get_max_q(float dpr)
{
  int i;
  for (i=0; i<1000; i++)
    if (det_max_q_to_dpr[i] > dpr)
      break;
  return (float)i/4000.0f;
}

#ifdef USE_INT_LSIM
  // scale and convert to integer
  #define SC1(a) (int)((a)*N1)
#else
  #define SC1(a) a
#endif

// Define max_rp and max_q to obtain det_ratio from udet_dep_dpr && udet_dep_q
// To keep simple, udet_dep_dpr && udet_dep_q assumed same for all detectors.
void local_conf_arm_detector(struct det_conf_t *det, float det_ratio, float udet_dep_dpr, float udet_dep_q)
{
  if (det_ratio == 1)
  {
    // easy case
#ifdef USE_INT_LSIM
    det->dpr    = N1;
    det->max_rp = NPI/2;
    det->max_q  = (NPI*NPI)/4;
#else
    det->dpr    = 1.0f;
    det->max_rp = NPI/2.0f;
    det->max_q  = (NPI*NPI)/4.0f;
#endif
  }
  else
  {
    float u_ratio = 1.0f - det_ratio;                    // undetect global ratio
    float u_rand  = 1.0f - (udet_dep_dpr + udet_dep_q);  // undetect randomness dependant (complement)
    float u_rp    = u_ratio * udet_dep_dpr;              // undetect rp dependant ratio
    float u_q     = u_ratio * udet_dep_q;                // undetect q dependant ratio

    det->dpr    = SC1(1.0f - (u_ratio * u_rand));
    det->max_rp = SC1(get_max_rp(1.0f - u_rp));
#ifdef USE_INT_LSIM
    det->max_q  = (int)(get_max_q(1.0f - u_q)*(NPI*NPI));
#else
    det->max_q  = get_max_q(1.0f - u_q)*(NPI*NPI);
#endif
  }
}

// local simulation specific constant inits
void local_init_sim_const(float em_pdiff, float l_src_beta_scale, float l_s, bool l_polarize, 
                          float udet_dep_dpr, float udet_dep_q,
                          float l_win_time, float l_delay_o, float l_delay_e, float l_st1_delay)
{
  struct arm_t *arm_a = &epr_sim_conf.local.arm_a;
  struct arm_t *arm_b = &epr_sim_conf.local.arm_b;

  epr_sim_conf.local.src_beta_scale = SC1(l_src_beta_scale);
  epr_sim_conf.local.em_rdiff       = SC1(em_pdiff / PI);     // PI normalized to 1 in local sim
  epr_sim_conf.local.s              = SC1(l_s);
  epr_sim_conf.local.win_time       = SC1(l_win_time);
  epr_sim_conf.local.local_polarize = l_polarize;

  arm_a->delay_o            = SC1(l_delay_o);
  arm_a->delay_e            = SC1(l_delay_e);
  arm_a->delay_st1          = SC1(l_st1_delay);
  arm_a->tr_dat = (int *)0;

  arm_b->delay_o            = SC1(l_delay_o);
  arm_b->delay_e            = SC1(l_delay_e);
  arm_b->delay_st1          = SC1(l_st1_delay);
  arm_b->tr_dat = (int *)0;

  local_conf_arm_detector(&arm_a->det_o, epr_sim_conf.p_det_ao, udet_dep_dpr, udet_dep_q);
  local_conf_arm_detector(&arm_a->det_e, epr_sim_conf.p_det_ae, udet_dep_dpr, udet_dep_q);
  local_conf_arm_detector(&arm_b->det_o, epr_sim_conf.p_det_bo, udet_dep_dpr, udet_dep_q);
  local_conf_arm_detector(&arm_b->det_e, epr_sim_conf.p_det_be, udet_dep_dpr, udet_dep_q);
}

#if 0
// ------------------------------------------------------
// produce curves resources for local_conf_arm_detector()

#include <stdio.h>
static void gen_data_file(const char *name, float *data, int size)
{
  FILE *f = fopen(name, "wt");
  if (f)
  {
    int i;
    for (i=0; i<size; i++)
    {
      if (!(i % 20))
        fprintf(f, "\n");
      fprintf(f, "%.3f, ", data[i]);
    }
    fclose(f);
  }
}

static float det_max_rp_to_dpr[1000];
static float det_max_q_to_dpr[1000];

// get detect ratio
static float gen_dpr_get_dr(int N, struct arm_t *arm)
{
  int i, nd = 0;
  for (i=0; i<N; i++)
  {
    struct photon_t pho;
    pho.p = RAND_1;
    pho.q = RAND_1;
    pho.r = RAND_1;
    pho.detect = DET_U;
    l_polarize_detect(&pho, arm);
    if (pho.detect != DET_U)
      nd++;
  }
  return ((float)nd)/N;
}

static void gen_data_file(const char *name, float *data, int size);

void gen_dpr_dat(void)
{
  int i;
  struct arm_t arm = { 0 };

  arm.det_o.dpr = 1.0f;
  arm.det_e.dpr = 1.0f;
#if 0
  arm.det_o.max_q = NPI;
  arm.det_e.max_q = NPI;
  for (i=0; i<1000; i++)
  {
    float p = (i*NPI)/2000.0f;             // max reached at PI/2
    arm.det_o.max_rp = p;
    arm.det_e.max_rp = p;
    dep_dpr_to_max_rp[i] = gen_dpr_get_dr(100000, &arm);
  }
  gen_data_file("det_max_rp_to_dpr.txt", dep_dpr_to_max_rp, 1000);
#endif
#if 0
  arm.det_o.max_rp = NPI;
  arm.det_e.max_rp = NPI;
  for (i=0; i<1000; i++)
  {
    float p = (i*NPI)/4000.0f;              // max reached at PI/4
    arm.det_o.max_q = p;
    arm.det_e.max_q = p;
    dep_q_to_max_q[i] = gen_dpr_get_dr(100000, &arm);
  }
  gen_data_file("det_max_q_to_dpr.txt", dep_q_to_max_q, 1000);
#endif
}
#endif


// ------------------------------------------------------------------
// Mobile windows

// note: detector noise not simulated in this mode

void local_simulate_mobile(float an_a, float an_b, struct det_ctr_t *ctr)
{
  struct arm_t *arm_a = &epr_sim_conf.local.arm_a;
  struct arm_t *arm_b = &epr_sim_conf.local.arm_b;
  struct photon_t a;                             // Alice
  struct photon_t b;                             // Bob
  int n;

#ifdef USE_INT_LSIM
  int win_time = epr_sim_conf.local.win_time;
  arm_a->an_pol = ((int)((an_a*NPI) / PI) + NPI) & MSK1;
  arm_b->an_pol = ((int)((an_b*NPI) / PI) + NPI) & MSK1;
#else
  float win_time = epr_sim_conf.local.win_time;
  if (an_a >= PI) an_a -= PI;                    // polarizer require positives angles values
  if (an_b >= PI) an_b -= PI;
  arm_a->an_pol = an_a/PI;                       // normalize
  arm_b->an_pol = an_b/PI;
  W_ASSERT((arm_a->an_pol >= 0) && (arm_a->an_pol < 1.0f));
  W_ASSERT((arm_b->an_pol >= 0) && (arm_b->an_pol < 1.0f));
#endif
  W_ASSERT(!epr_sim_conf.qm_mode);
  W_ASSERT(!epr_sim_conf.ignore_acc);

  #define GET_DT(a, b) ((a >= b) ? (a - b) : (b - a))
  #define SI_A(a) ctr->id[st_to_ct_id[a.detect << 2]]++
  #define SI_B(b) ctr->id[st_to_ct_id[b.detect]]++
  #define PA_AB(a, b) ctr->id[st_to_ct_id[(a.detect << 2) | b.detect]]++

  #define CNT(a, b) if (GET_DT(a.p_time, b.p_time) < win_time) PA_AB(a, b); else { SI_A(a); SI_B(b); }

  // entangled pair 
  for (n=0; n<epr_sim_conf.n_em_ent_pair; n++)
  {
    l_emit_ent_pair(&a, &b);
    l_polarize_detect(&a, arm_a);
    l_polarize_detect(&b, arm_b);
    CNT(a, b)
    CHECK_THRD_CANCEL();
  }

  // non entangled pair
  for (n=0; n<epr_sim_conf.n_em_n_ent_pair; n++)
  {
    l_emit_ne_pair(&a, &b);
    l_polarize_detect(&a, arm_a);
    l_polarize_detect(&b, arm_b);
    CNT(a, b)
    CHECK_THRD_CANCEL();
  }

  // single
  for (n=0; n<epr_sim_conf.n_em_single; n++)
  {
    if (RAND_I < (N1/2))                         // emit 50%/50% on arm Alice/Bob
    {
      l_emit_single(&a);
      l_polarize_detect(&a, arm_a);
      SI_A(a);
    }
    else
    {
      l_emit_single(&b);
      l_polarize_detect(&b, arm_b);
      SI_B(b);
    }
    CHECK_THRD_CANCEL();
  }

  // double entangled pairs.
  // Pair 2 first detected at Alice and Bob. (or produce 2 single if time diff > win_time)
  // Other detections (>2) are counted as accidentals. (become single if not ignored)

  for (n=0; n<epr_sim_conf.n_em_ent_double; n++)
  {
    struct photon_t c, d;                        // 2nd pair
    struct photon_t *a0, *a1, *b0, *b1;

    l_emit_ent_pair(&a, &b);
    l_polarize_detect(&a, arm_a);
    l_polarize_detect(&b, arm_b);

    l_emit_ent_pair(&c, &d);
    l_polarize_detect(&c, arm_a);
    l_polarize_detect(&d, arm_b);

    // order in time
    if (a.p_time < c.p_time) { a0 = &a; a1 = &c; }
    else                     { a0 = &c; a1 = &a; }

    if (b.p_time < d.p_time) { b0 = &b; b1 = &d; }
    else                     { b0 = &d; b1 = &b; }

    #define ACC_A(a) if (a->detect & OUT_O) ctr->acc_ao++; else if (a->detect & OUT_E) ctr->acc_ae++
    #define ACC_B(b) if (b->detect & OUT_O) ctr->acc_bo++; else if (b->detect & OUT_E) ctr->acc_be++

    if (a0->detect)
    {
      if (b0->detect)
      {
        CNT((*a0), (*b0))
        ACC_B(b1);
      }
      else
      if (b1->detect)
      {
        CNT((*a0), (*b1))
      }
      else
        SI_A((*a0));
      ACC_A(a1);
    }
    else
    if (a1->detect)
    {
      if (b0->detect)
      {
        CNT((*a1), (*b0))
        ACC_B(b1);
      }
      else
      if (b1->detect)
      {
        CNT((*a1), (*b1))
      }
      else
        SI_A((*a1));
    }
    else
    {
      if (b0->detect)
      {
        SI_B((*b0));
        ACC_B(b1);
      }
      else
      if (b1->detect)
        SI_B((*b1));
    }
    CHECK_THRD_CANCEL();
  }

  // noise:
  // background noise is not simulated, as not required to produce accidentals and inequality violation.
}
