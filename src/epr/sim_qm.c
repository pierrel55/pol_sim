#include <math.h>
#include "epr.h"
#include "epr_sim.h"
#include "../win/debug.h"

// ----------------------------------------------
// polarizer

// pair type probability
struct qm_polarize_pr_t
{
  float oo;
  float oe;
  float eo;
  float ee;
};

// define pair type emission probabilies
static void qm_init_polarize_pr(float an_a, float an_b, float qm_r, struct qm_polarize_pr_t *pr)
{
  float sa = sinf(an_a);
  float ca = cosf(an_a);
  float sb = sinf(an_b);
  float cb = cosf(an_b);

  float sin_a_sin_b = sa * sb;
  float sin_a_cos_b = sa * cb;
  float cos_a_sin_b = ca * sb;
  float cos_a_cos_b = ca * cb;

  float oo = (qm_r * cos_a_cos_b) + sin_a_sin_b;
  float oe = (qm_r * cos_a_sin_b) - sin_a_cos_b;
  float eo = (qm_r * sin_a_cos_b) - cos_a_sin_b;
  float ee = (qm_r * sin_a_sin_b) + cos_a_cos_b;

#if 1
  float k = (1.0f + qm_r*qm_r);
  pr->oo = (oo * oo) / k;
  pr->oe = (oe * oe) / k;
  pr->eo = (eo * eo) / k;
  pr->ee = (ee * ee) / k;
#else
  float k = 1.0f/(1.0f + qm_r*qm_r);
  pr->oo = k * (oo * oo);
  pr->oe = k * (oe * oe);
  pr->eo = k * (eo * eo);
  pr->ee = k * (ee * ee);
#endif
  W_ASSERT(fabs(1.0 - (pr->oo + pr->oe + pr->eo + pr->ee)) < EPSILON);  // check probability sum
}

// ----------------------------------------------
// detection

// qm specific inits
void qm_init_sim_const(float em_pdiff, float qm_r)
{
  int i;
  W_ASSERT(epr_sim_conf.win_fixed);
  epr_sim_conf.qm.em_pdiff = em_pdiff;
  epr_sim_conf.qm.r = qm_r;
  for (i=0; i<16; i++)               // define detect probabilities for each detection states
  {
    epr_sim_conf.qm.st_dpr_a[i] = (i & A_e) ? epr_sim_conf.p_det_ae : epr_sim_conf.p_det_ao;
    epr_sim_conf.qm.st_dpr_b[i] = (i & B_e) ? epr_sim_conf.p_det_be : epr_sim_conf.p_det_bo;
  }
}

#define ST_DETECT_OU ((RAND_1 < epr_sim_conf.p_det_ao) ? A_o : 0)
#define ST_DETECT_EU ((RAND_1 < epr_sim_conf.p_det_ae) ? A_e : 0)
#define ST_DETECT_UO ((RAND_1 < epr_sim_conf.p_det_bo) ? B_o : 0)
#define ST_DETECT_UE ((RAND_1 < epr_sim_conf.p_det_be) ? B_e : 0)

#define ST_DETECT_OO ST_DETECT_OU | ST_DETECT_UO
#define ST_DETECT_OE ST_DETECT_OU | ST_DETECT_UE
#define ST_DETECT_EO ST_DETECT_EU | ST_DETECT_UO
#define ST_DETECT_EE ST_DETECT_EU | ST_DETECT_UE

#define ST_DETECT(st) (st) & (   ((RAND_1 < epr_sim_conf.qm.st_dpr_a[st]) ? (A_e|A_o) : 0) \
                               | ((RAND_1 < epr_sim_conf.qm.st_dpr_b[st]) ? (B_e|B_o) : 0) )

void qm_simulate(float an_a, float an_b, struct det_ctr_t *ctr)
{
  int n1, n = 0;
  struct qm_polarize_pr_t pol_pr;
  W_ASSERT(epr_sim_conf.qm_mode);

  // define QM polarize probabilities
  qm_init_polarize_pr(an_a, an_b - epr_sim_conf.qm.em_pdiff, epr_sim_conf.qm.r, &pol_pr);

  // entangled pair 
  if (epr_sim_conf.n_em_ent_pair > 0)
  {
    n1 = n + (int)(pol_pr.oo * epr_sim_conf.n_em_ent_pair);
    for (; n < n1; n++)
      epr_count(ST_DETECT_OO, ctr);
  
    n1 = n + (int)(pol_pr.oe * epr_sim_conf.n_em_ent_pair);
    for (; n < n1; n++)
      epr_count(ST_DETECT_OE, ctr);

    n1 = n + (int)(pol_pr.eo * epr_sim_conf.n_em_ent_pair);
    for (; n < n1; n++)
      epr_count(ST_DETECT_EO, ctr);

    // remaining is ee
    for (; n<epr_sim_conf.n_em_ent_pair; n++)
      epr_count(ST_DETECT_EE, ctr);
  }

  // non entangled pair
  if (epr_sim_conf.n_em_n_ent_pair > 0)
  {
    n1 = n + epr_sim_conf.n_em_n_ent_pair;
    for (; n < n1; n+=4)
    {
      epr_count(ST_DETECT_OO, ctr);
      epr_count(ST_DETECT_OE, ctr);
      epr_count(ST_DETECT_EO, ctr);
      epr_count(ST_DETECT_EE, ctr);
    }
  }

  // single
  if (epr_sim_conf.n_em_single > 0)
  {
    n1 = n + epr_sim_conf.n_em_single;
    for (; n < n1; n+=4)
    {
      epr_count(ST_DETECT_OU, ctr);
      epr_count(ST_DETECT_EU, ctr);
      epr_count(ST_DETECT_UO, ctr);
      epr_count(ST_DETECT_UE, ctr);
    }
  }

  // double entangled pairs
  if (epr_sim_conf.n_em_ent_double > 0)
  {
    // QM polarizer cummulated probabilities used in QM_POL()
    float pr_oo       = pol_pr.oo;
    float pr_oo_oe    = pol_pr.oo + pol_pr.oe;
    float pr_oo_oe_eo = pol_pr.oo + pol_pr.oe + pol_pr.eo;

    // as pairs interract, emission type must be random
    #define QM_POL ((r = RAND_1) < pr_oo_oe) ? \
                   ((r < pr_oo)       ? (A_o|B_o) : (A_o|B_e)) \
                 : ((r < pr_oo_oe_eo) ? (A_e|B_o) : (A_e|B_e))

    n1 = n + epr_sim_conf.n_em_ent_double;
    for (; n < n1; n++)
    {
      float r;                                   // used in QM_POL()

      int p0 = QM_POL;                           // emit 1st pair
      int p1 = QM_POL;                           // emit 2nd pair
      
      p0 = ST_DETECT(p0);                        // detect 1st pair
      p1 = ST_DETECT(p1);                        // detect 2nd pair

      epr_count(p0 | (p1 << 4), ctr);            // note: store m1 in b4..b7 (2nd pair)
    }
  }

  // remain is no emission
  for (; n < epr_sim_conf.N; n++)
    epr_count(0, ctr);
}

