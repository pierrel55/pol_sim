// dialog controlled application
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win/win.h"
#include "epr/epr.h"
#include "dlg_ctrl.h"

struct dlg_state_t epr_dlg = { 0 };

// called one time after window init (ready to print)
void dlg_ctrl_init(void)
{
  rng_cpu_detect();
  if (hw_rng_detected)
    co_printf("RNG: CPU hardware random generator detected.\n");
  epr_sim_init();
  init_config_1_qm_perfect();
  dlg_init_dialog();
  co_printf("Ready: please select an experiment configuration, or define experiment settings.\n");
}

// ----------------------------------------------------
// simulation

struct sim_res_t sim_result = { 0 };

static bool dlg_check_emit_prob(void)
{
  // check defined emission probabilities
  if (epr_dlg.source.em_no_emit_lbl_value < 0)
  {
    co_printf("ERROR: [Source][Emission prob]: probability sum > 1, No emission value is < 0\n");
    return false;
  }
  return true;
}

// detector detect probabiliy
static void dlg_set_detect_prob(void)
{
  if (epr_dlg.detectors.det_prob_bob_same_alice)
  {
    epr_dlg.detectors.det_prob_bob_det_o = epr_dlg.detectors.det_prob_alice_det_o;
    epr_dlg.detectors.det_prob_bob_det_e = epr_dlg.detectors.det_prob_alice_det_e;
  }
}

static void dlg_init_sim_conf_const(void)
{
#if 1
  // take care of rounding in gui edit box
  #define ROUND0(a) if (a < EPSILON) a = 0.0f
  ROUND0(epr_dlg.source.em_prob_ent_pair);
  ROUND0(epr_dlg.source.em_prob_nent_pair);
  ROUND0(epr_dlg.source.em_prob_single);
  ROUND0(epr_dlg.source.em_prob_double);
#endif
  epr_init_sim_const(epr_dlg.pairing.win_type_fixed,
                     epr_dlg.sim.model_qm,
                     epr_dlg.pairing.ignore_accidentals,
                     DEG_TO_RAD(epr_dlg.source.emit_pol_diff),
                     epr_dlg.source.ent_qm_r,
                     epr_dlg.source.emit_enable_jitter ? epr_dlg.source.emit_time_beta_scale : 0.0f,
                     epr_dlg.source.ent_local_s,
                     epr_dlg.source.local_polarize,
                     epr_dlg.detectors.udet_dep_local_dpr,
                     epr_dlg.detectors.udet_dep_local_q,
                     epr_dlg.pairing.win_width,
                     epr_dlg.polarizer.o_out_delay,
                     epr_dlg.polarizer.e_out_delay,
                     epr_dlg.polarizer.st1_delay,
                     epr_dlg.detectors.det_prob_alice_det_o,
                     epr_dlg.detectors.det_prob_alice_det_e,
                     epr_dlg.detectors.det_prob_bob_det_o,
                     epr_dlg.detectors.det_prob_bob_det_e,
                     epr_dlg.source.em_prob_ent_pair,
                     epr_dlg.source.em_prob_nent_pair,
                     epr_dlg.source.em_prob_single,
                     epr_dlg.source.em_prob_double,
                     epr_dlg.detectors.misc_noise_det_r);
}

// define graph datas to display source delay
static void gen_str_t_dist_graph(float x0, float x1, int data_count)
{
  if (epr_dlg.source.emit_enable_jitter)
  {
    int n = 1000*data_count;
    float n_max = 0;
    float bs = SC_JFUNC(epr_dlg.source.emit_time_beta_scale*PI);
    float kx = (data_count - 1.0f)/(x1 - x0);    // x to d offset
    float xf = SRC_JFUNC(bs, 1.0f);              // fixed min delay not displayed

    struct sim_data_t *d = sim_result.sim_data_res;
    struct sim_data_t *d_end = sim_result.sim_data_res + data_count;

    // clear counters
    for (; d < d_end; d++)
      d->src_t_dis = 0;

    // update counters
    d = sim_result.sim_data_res;
    while (n--)
    {
      float x = SRC_JFUNC(bs, rng.rand1()) - xf;
      float dx = x - x0;
      if (dx > 0)
      {
        int i = (int)(dx * kx);
        if (i < data_count)
        {
          float a = d[i].src_t_dis + 1.0f;
          if (a > n_max)
            n_max = a;                           // get max to normalize graph
          d[i].src_t_dis = a;
        }
      }
    }

    if (n_max)                                   // normalize graph
      for (d=sim_result.sim_data_res; d<d_end; d++)
        d->src_t_dis /= n_max;
  }
}

// graph average value, displayed on console
struct graph_avg_t
{
  double co_oo_ee;
  double co_oe_eo;
  double transmit_a;
  double det_pair_ratio;
  double det_sing_ratio;
  double det_uu_ratio;
  double det_acc_ratio;
};

static void def_sim_res(int N, struct det_ctr_t *ctr, struct sim_data_t *d, struct graph_avg_t *graph_avg);

void dlg_ctrl_update_graph(float x0, float x1, int data_count)
{
  struct sim_data_t *d = sim_result.sim_data_res;
  struct sim_data_t *d_end = sim_result.sim_data_res + data_count;
  int N = (int)epr_dlg.curves.graph_N;           // samples per curve pixel
  double x, dx = (x1 - x0)/(data_count-1);       // angle diff step
  int tr_rel_alice_o_dat[TR_GRAPH_N] = { 0 };
  struct graph_avg_t graph_avg = { 0 };
  W_ASSERT(data_count < CRV_MAX_Y_DATA);

  if (!dlg_check_emit_prob())
    return;
  dlg_set_detect_prob();

  if (epr_dlg.sim.model_qm)
    co_printf("QM sim: N:%d, r:%.2f, pdiff:%.2f ... ", N, epr_dlg.source.ent_qm_r, epr_dlg.source.emit_pol_diff);
  else
    co_printf("Locale sim: N:%d, s:%.2f, pdiff:%.2f, win size:%.3f... ", N, epr_dlg.source.ent_local_s, epr_dlg.source.emit_pol_diff, epr_dlg.pairing.win_width);

  // ----------------------------------
  // define simulation constants
  dlg_init_sim_conf_const();
  epr_init_sim_N(N);
  epr_sim_conf.local.arm_a.tr_dat = tr_rel_alice_o_dat;
  epr_sim_conf.local.arm_a.tr_N = 0;

  // ----------------------------------
  // define graph datas
  for (x=x0; d < d_end; d++, x+=dx)
  {
    struct det_ctr_t ctr = { 0 };
    float an_a = RAND_PI;                        // Alice angle
    float an_b = (float)(an_a + x);              // Bob angle (x: angle difference with Alice)
    float s = sinf(x);
    float c = cosf(x);

    epr_simulate(an_a, an_b, &ctr);              // simulation
    W_ASSERT(ctr.acc == 0);                      // check no un-counted accidentals
    def_sim_res(N, &ctr, d, &graph_avg);

    // reference curves
    d->sin2  = s*s;
    d->cos2  = c*c;
    d->triangle = (float)((x < PI/2) ? x*(2/PI) : (PI-x)*(2/PI));
    EPR_PROGRESS(((d - sim_result.sim_data_res)*100)/data_count);
  }

  gen_str_t_dist_graph(x0, x1, data_count);      // source delay distribution

  epr_sim_conf.local.arm_a.tr_dat = (int *)0;
  sim_result.x0 = x0;
  sim_result.x1 = x1;
  sim_result.sim_data_count = data_count;

  // transmittance relative curve
  if (epr_sim_conf.local.arm_a.tr_N)
  {
    for (d = sim_result.sim_data_res, x=x0; d < d_end; d++, x+=dx)
    {
      int i = (int)((x*TR_GRAPH_N)/PI) & (TR_GRAPH_N - 1);
      d->tr_rel_ao = (float)(tr_rel_alice_o_dat[i] * TR_GRAPH_N)/epr_sim_conf.local.arm_a.tr_N;
    }
  }

  // print averages
  co_printf("Done.\nAvg: "
    "Co.oo+ee:%.3f "
    "Co.oe+eo:%.3f "
    "Tr.Alice o:%.3f "
    "D.pair ratio:%.3f "
    "D.sing ratio:%.3f\n"
    "Avg: UU ratio:%.3f "
    "Acc ratio:%.3f (%s)\n",
  graph_avg.co_oo_ee       / data_count,
  graph_avg.co_oe_eo       / data_count,
  graph_avg.transmit_a     / data_count,
  graph_avg.det_pair_ratio / data_count,
  graph_avg.det_sing_ratio / data_count,
  graph_avg.det_uu_ratio   / data_count,
  graph_avg.det_acc_ratio  / data_count,
  epr_sim_conf.ignore_acc ? "counted as uu" : "counted as single");
}

static void def_sim_res(int N, struct det_ctr_t *ctr, struct sim_data_t *d, struct graph_avg_t *graph_avg)
{
  // def results
#if 0
  {
    // debug
    int n_sing = ctr->ou + ctr->eu + ctr->ue + ctr->uo;
    int n_acc = ctr->acc_ao + ctr->acc_ae + ctr->acc_bo + ctr->acc_be;
    int n_uu = ctr->uu;

    if (epr_dlg.pairing.ignore_accidentals)
      n_uu += ctr->acc_n;
    else
      n_sing += n_acc;

    d->co_oo_ee = (float)(ctr->oo + ctr->ee)/N;
    d->co_oe_eo = (float)(ctr->oe + ctr->eo)/N;
    d->det_pair_ratio = (float)(ctr->oo + ctr->ee + ctr->oe + ctr->eo)/N;
    d->det_sing_ratio = (float)(n_sing)/N;
    d->det_uu_ratio  = (float)(ctr->uu)/N;
    d->det_acc_ratio = (float)(n_acc)/N;

    d->transmit_a = (float)(ctr->oo + ctr->oe + ctr->ou + ctr->acc_ao)/N;
    d->tr_rel_ao  = (float)(ctr->ee + ctr->eo + ctr->eu + ctr->acc_ae)/N;
  }
#else
  int n_o, n_e;
  int n_pairs = (ctr->oo + ctr->ee + ctr->oe + ctr->eo);
  if (n_pairs > 100)
  {
    d->co_oo_ee = (float)(ctr->oo + ctr->ee) / n_pairs;
    d->co_oe_eo = (float)(ctr->oe + ctr->eo) / n_pairs;
  }
  else
  {
    d->co_oo_ee = 0;
    d->co_oe_eo = 0;
  }

  n_o = ctr->oo + ctr->oe + ctr->ou;
  n_e = ctr->ee + ctr->eo + ctr->eu;
  if (!epr_sim_conf.ignore_acc)               // must be included for local transmittance
  {
    n_o += ctr->acc_ao;
    n_e += ctr->acc_ae;
  }

  if ((n_o + n_e) > 100)
    d->transmit_a = (float)(n_o)/(n_o + n_e);
  else
    d->transmit_a = 0;

  d->det_pair_ratio = (float)(ctr->oo + ctr->ee + ctr->oe + ctr->eo)/N;
  d->det_sing_ratio = (float)(ctr->ou + ctr->eu + ctr->ue + ctr->uo)/N;

  d->det_uu_ratio   = (float)(ctr->uu)/N;
  d->det_acc_ratio  = (float)(ctr->acc_n)/N;

  graph_avg->co_oo_ee       += d->co_oo_ee;
  graph_avg->co_oe_eo       += d->co_oe_eo;
  graph_avg->transmit_a     += d->transmit_a;
  graph_avg->det_pair_ratio += d->det_pair_ratio;
  graph_avg->det_sing_ratio += d->det_sing_ratio;
  graph_avg->det_uu_ratio   += d->det_uu_ratio;
  graph_avg->det_acc_ratio  += d->det_acc_ratio;
#endif
}

struct ineq_search_res_t ineq_search_res = { 0 };

void dlg_ineq_tools_find_angles(void)
{
  if (!dlg_check_emit_prob())
    return;
  dlg_set_detect_prob();

  co_printf("QM find Eberhard..\n");

  // ----------------------------------
  // define simulation constants
  dlg_init_sim_conf_const();
  eber_search((int)epr_dlg.inequality.angles_max_search);
  dlg_update_ineq_res_combo();
}

void dlg_inequality_test(void)
{
  struct ineq_res_t r;
  int N = (int)epr_dlg.inequality.test_conf_N;

  if (!dlg_check_emit_prob())
    return;
  dlg_set_detect_prob();

  co_printf("Eval inequality, N = %d ..\n", N);

  // ----------------------------------
  // define simulation constants
  dlg_init_sim_conf_const();
  epr_init_sim_N(N);
  eber_eval_single(DEG_TO_RAD(epr_dlg.inequality.test_conf_a1),
                        DEG_TO_RAD(epr_dlg.inequality.test_conf_a2),
                        DEG_TO_RAD(epr_dlg.inequality.test_conf_b1),
                        DEG_TO_RAD(epr_dlg.inequality.test_conf_b2), &r);
}

void dlg_inequality_optimize(void)
{
  struct ineq_res_t r;
  int N = (int)epr_dlg.inequality.test_conf_N;

  if (!dlg_check_emit_prob())
    return;
  dlg_set_detect_prob();

  co_printf("Optimize inequality ..\n", N);

  dlg_init_sim_conf_const();
  eber_adjust_single(DEG_TO_RAD(epr_dlg.inequality.test_conf_a1),
                     DEG_TO_RAD(epr_dlg.inequality.test_conf_a2),
                     DEG_TO_RAD(epr_dlg.inequality.test_conf_b1),
                     DEG_TO_RAD(epr_dlg.inequality.test_conf_b2), &r);

  epr_dlg.inequality.test_conf_a1 = RAD_TO_DEG(r.a1_b1.an_a);
  epr_dlg.inequality.test_conf_a2 = RAD_TO_DEG(r.a2_b1.an_a);
  epr_dlg.inequality.test_conf_b1 = RAD_TO_DEG(r.a1_b1.an_b);
  epr_dlg.inequality.test_conf_b2 = RAD_TO_DEG(r.a1_b2.an_b);
}

#define SHOW_SPEED

void dlg_ineq_tools_eval_posi(void)
{
#ifdef SHOW_SPEED
  unsigned int t0 = get_ctr_ms();
#endif
  int N = (int)epr_dlg.inequality.test_conf_N;

  if (!dlg_check_emit_prob())
    return;
  dlg_set_detect_prob();

  // ----------------------------------
  // define simulation constants
  dlg_init_sim_conf_const();
  epr_init_sim_N(N);

  co_printf("Eval positivity. N = %d, an: %.2f %.2f %.2f %.2f ...\n", epr_sim_conf.N,
             epr_dlg.inequality.test_conf_a1,
             epr_dlg.inequality.test_conf_a2,
             epr_dlg.inequality.test_conf_b1,
             epr_dlg.inequality.test_conf_b2);

  eber_eval_positivity(DEG_TO_RAD(epr_dlg.inequality.test_conf_a1),
                       DEG_TO_RAD(epr_dlg.inequality.test_conf_a2),
                       DEG_TO_RAD(epr_dlg.inequality.test_conf_b1),
                       DEG_TO_RAD(epr_dlg.inequality.test_conf_b2),
                       (int)epr_dlg.inequality.eval_posi_count);
#ifdef SHOW_SPEED
  co_printf("Done in %.3f seconds.\n", (float)(get_ctr_ms() - t0)/1000.0f);
#endif
}

// ----------------------------------------------
// transmittance

static void print_test_inf(int max_aligned)
{
  co_printf(
    "--------------------------------------------------------------\n"
    "Test %d aligned polarizer. N: %d Max allowed error: %.2f%%.\n", max_aligned,
    (int)epr_dlg.transmit_test.sim_N, epr_dlg.transmit_test.max_error);
}

void dlg_transmit_random_test(void)
{
  int max_aligned = (int)epr_dlg.transmit_test.pol_count;
  float max_error = epr_dlg.transmit_test.max_error*0.01f;
  print_test_inf(max_aligned);
  check_malus_random(max_aligned, DEG_TO_RAD(epr_dlg.transmit_test.max_anv), (int)epr_dlg.transmit_test.sim_N, max_error);
}

// extract float list from string.
// return count, < 0 value if error
static int get_float_list(float *f_list, int list_max, const char *str)
{
  int n=0, l=0, dot=0;
  char a_str[16];
  while (1)
  {
    char c = *str++;
    if (    ((c >= '0') && (c <= '9'))
         || ((c == '-') && !l)
         || ((c == '.') && !dot && l && (a_str[l-1] != '-')))
    {
      if (l == sizeof(a_str)-1)
        return -1;                   // number too long
      if (c == '.')
        dot = 1;
      a_str[l++] = c;
    }
    else
    if (!c || (c == ','))
    {
      if (n == list_max)             // to many values
        return -3;
      if (!l)
      {
        if (c)
          return -2;                 // multi ','
        return n;                    // ended ','
      }
      if ((l == 1) && (a_str[0] == '-'))
        return -2;                   // incomplete number
      a_str[l] = 0;
      f_list[n++] = (float)atof(a_str);
      if (!c)
        return n;
      l=0;
      dot=0;
    }
    else
    if (c != ' ')                    // syntax
      return -2;
  }
}

// 1,12,-3,14,-5,16,-7,8,-9,-10,11,22
void dlg_transmit_list_test(void)
{
  float an_list[100];
  int n = get_float_list(an_list, 100, epr_dlg.transmit_test.pol_list_angles_str);
  if (n > 0)
  {
    float max_error = epr_dlg.transmit_test.max_error*0.01f;
    print_test_inf(n);
    check_malus_user_angles(an_list, n, (int)epr_dlg.transmit_test.sim_N, max_error);
  }
  else
  if (n == 0)
    co_printf("Test aligned polarizer: Please define angles list in degree, separated with ','\n"
              "Example: 0, 45, 90\n");
  else
    co_printf("ERROR found in angle list format.\n"
      "Expected format example: 10, 40.5, 80.25\n");
}
