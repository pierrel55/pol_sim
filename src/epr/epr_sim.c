#include <math.h>
#include "epr.h"
#include "epr_sim.h"
#include "../win/debug.h"

struct epr_sim_conf_t epr_sim_conf = { 0 };

// convert state to e_ct_id counter index enum
const unsigned int st_to_ct_id[16] =
{
  e_id_uu,     // 00 00
  e_id_uo,     // 00 01
  e_id_ue,     // 00 10
  e_id_acc,    // 00 11  1 accidental
  e_id_ou,     // 01 00
  e_id_oo,     // 01 01
  e_id_oe,     // 01 10
  e_id_acc,    // 01 11  1 accidental
  e_id_eu,     // 10 00
  e_id_eo,     // 10 01
  e_id_ee,     // 10 10
  e_id_acc,    // 10 11  1 accidental
  e_id_acc,    // 11 00  1 accidental
  e_id_acc,    // 11 01  1 accidental
  e_id_acc,    // 11 10  1 accidental
  e_id_acc,    // 11 11  2 accidentals
};

// convert a triple detection state to detection counter index, define accidentals singles to emit
struct conv_st_t
{
  union
  {
    int acc;                   // not 0 if is accidental state
    struct
    {
      unsigned char acc_ae;    // single to emit alice e
      unsigned char acc_ao;    // single to emit alice o
      unsigned char acc_be;    // single to emit bob e
      unsigned char acc_bo;    // single to emit bob o
    };
  };
  int ct_id;                   // detection counter index to increment
};

static struct conv_st_t epr_conv_st[1 << 12];

// define pre-calculated converted states to increase execution speed
static void def_conv_states(void)
{
  struct conv_st_t *cs = epr_conv_st;
  int m;
  for (m = 0; m < (1 << 12); m++, cs++)
  {
    #define D0(out) ((m & out) != 0)             // test out of detection 0
    #define D1(out) ((m & (out << 4)) != 0)      // test out of detection 1 (dual pair emit)
    #define D2(out) ((m & (out << 8)) != 0)      // test out of detection 2 (background noise)

    #define DN(out) D0(out) + D1(out) + D2(out)  // count of detections on single detector

    // get count of detection for each detector
    int n_ao = DN(A_o);
    int n_ae = DN(A_e);
    int n_bo = DN(B_o);
    int n_be = DN(B_e);

    // detect if double detection occur in e and o on same polarizer
    int dbl_a = n_ao && n_ae;
    int dbl_b = n_bo && n_be;

    cs->acc = 0;                                 // clear all counters (union)

    // detect accidentals
    if (dbl_a || dbl_b || (n_ao > 1) || (n_ae > 1) || (n_bo > 1) || (n_be > 1))
    {
      cs->acc_ae = n_ae;
      cs->acc_ao = n_ao;
      cs->acc_be = n_be;
      cs->acc_bo = n_bo;
      cs->ct_id = e_id_uu;                       // state counted as uu or singles
    }
    else
    {
      int st = (m | (m >> 4) | (m >> 8)) & 0xf;  // mix 3 detections
      cs->ct_id = st_to_ct_id[st];               // convert to enum counter id
      W_ASSERT(cs->ct_id != e_id_acc);
    }
  }
}

void epr_count(int m, struct det_ctr_t *ctr)
{
  struct conv_st_t *cs;
  if (epr_sim_conf.p_noise)
  {
    if (RAND_1 < epr_sim_conf.p_noise) m |= (A_e << 8);
    if (RAND_1 < epr_sim_conf.p_noise) m |= (A_o << 8);
    if (RAND_1 < epr_sim_conf.p_noise) m |= (B_e << 8);
    if (RAND_1 < epr_sim_conf.p_noise) m |= (B_o << 8);
  }
  cs = &epr_conv_st[m];
  if (!cs->acc)
    ctr->id[cs->ct_id]++;
  else
  {
    ctr->acc_n++;
    ctr->acc_ao += cs->acc_ao;
    ctr->acc_ae += cs->acc_ae;
    ctr->acc_bo += cs->acc_bo;
    ctr->acc_be += cs->acc_be;
  }
  CHECK_THRD_CANCEL();
}

void epr_sim_init(void)
{
  def_conv_states();
}

// ----------------------------------------------

// init simulation constants
void epr_init_sim_const(bool win_fixed, bool qm_mode, bool ignore_acc,
                        float em_pdiff, 
                        float qm_r,
                        float l_src_beta_scale,
                        float l_s, bool l_polarize, float udet_dep_dpr, float udet_dep_q, float l_win_time,
                        float l_delay_o, float l_delay_e, float l_st1_delay,
                        float p_det_ao, float p_det_ae, float p_det_bo, float p_det_be,
                        float p_em_ent_pair, float p_em_n_ent_pair, float p_em_single, float p_em_ent_double,
                        float p_noise)
{
  // init qm/local commons parameters
  epr_sim_conf.win_fixed = win_fixed;
  epr_sim_conf.qm_mode = qm_mode;
  epr_sim_conf.ignore_acc = ignore_acc;

  epr_sim_conf.p_det_ao = p_det_ao;
  epr_sim_conf.p_det_ae = p_det_ae;
  epr_sim_conf.p_det_bo = p_det_bo;
  epr_sim_conf.p_det_be = p_det_be;

  // save emit prob
  epr_sim_conf.p_em_ent_pair   = p_em_ent_pair;
  epr_sim_conf.p_em_n_ent_pair = p_em_n_ent_pair;
  epr_sim_conf.p_em_single     = p_em_single;
  epr_sim_conf.p_em_ent_double = p_em_ent_double;

  epr_sim_conf.p_noise = p_noise;

  // mode specific inits
  if (qm_mode)
    qm_init_sim_const(em_pdiff, qm_r);
  else
    local_init_sim_const(em_pdiff, l_src_beta_scale, l_s, l_polarize, udet_dep_dpr, udet_dep_q,
                   l_win_time, l_delay_o, l_delay_e, l_st1_delay);
}

void epr_init_sim_N(int N)
{
  // define source count of emission for each types
  W_ASSERT( ( epr_sim_conf.p_em_ent_pair
            + epr_sim_conf.p_em_n_ent_pair
            + epr_sim_conf.p_em_single
            + epr_sim_conf.p_em_ent_double) <= 1.0f);  // check probability sum

  epr_sim_conf.N = N;
  epr_sim_conf.n_em_ent_pair   = (int)(N * epr_sim_conf.p_em_ent_pair);
  epr_sim_conf.n_em_n_ent_pair = (int)(N * epr_sim_conf.p_em_n_ent_pair);
  epr_sim_conf.n_em_single     = (int)(N * epr_sim_conf.p_em_single);
  epr_sim_conf.n_em_ent_double = (int)(N * epr_sim_conf.p_em_ent_double);
  // remaining is uu
}

void epr_simulate(float an_a, float an_b, struct det_ctr_t *ctr)
{
  if (epr_sim_conf.qm_mode)
    qm_simulate(an_a, an_b, ctr);
  else
  if (epr_sim_conf.win_fixed)
    local_simulate(an_a, an_b, ctr);
  else
    local_simulate_mobile(an_a, an_b, ctr);

  if (epr_sim_conf.ignore_acc)
    ctr->uu += ctr->acc_n;
  else
  {
    ctr->ou += ctr->acc_ao;
    ctr->eu += ctr->acc_ae;
    ctr->uo += ctr->acc_bo;
    ctr->ue += ctr->acc_be;
  }
}