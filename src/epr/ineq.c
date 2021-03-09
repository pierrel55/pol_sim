#include <string.h>
#include <math.h>                                // sqrt
#include "epr.h"
#include "../win/debug.h"

// define J and J/N
static void eber_def_res(int N, struct det_ctr_t *a1_b1,
                                struct det_ctr_t *a1_b2,
                                struct det_ctr_t *a2_b1,
                                struct det_ctr_t *a2_b2,
                                struct ineq_res_t *r)
{
  W_ASSERT(    (a1_b1->an_a == a1_b2->an_a)
            && (a2_b1->an_a == a2_b2->an_a)
            && (a1_b1->an_b == a2_b1->an_b)
            && (a1_b2->an_b == a2_b2->an_b));
  r->N = N;
  r->Eb.J = (a1_b2->oe + a1_b2->ou + a2_b1->eo + a2_b1->uo + a2_b2->oo) - a1_b1->oo;
  r->Eb.JoN = (double)(r->Eb.J)/N;
}

static void eber_eval_res(int N, struct ineq_res_t *r)
{
  eber_def_res(N, &r->a1_b1, &r->a1_b2, &r->a2_b1, &r->a2_b2, r);
}

static void epr_test(float an_a, float an_b, struct det_ctr_t *ctr)
{
  memset(ctr, 0, sizeof(*ctr));
  ctr->an_a = an_a;
  ctr->an_b = an_b;
  epr_simulate(an_a, an_b, ctr);
}

static void eber_test(struct ineq_res_t *r)
{
  #define EPR_EVAL(an) epr_test(r->an.an_a, r->an.an_b, &r->an);
  EPR_EVAL(a1_b1);
  EPR_EVAL(a1_b2);
  EPR_EVAL(a2_b1);
  EPR_EVAL(a2_b2);
  eber_eval_res(epr_sim_conf.N, r);
}

static void print_res(struct ineq_res_t *r)
{
  co_printf("J/N:%.6f\tJ:%d\tan:%.2f %.2f %.2f %.2f\n", r->Eb.JoN, r->Eb.J,
             RAD_TO_DEG(r->a1_b1.an_a),
             RAD_TO_DEG(r->a2_b1.an_a),
             RAD_TO_DEG(r->a1_b1.an_b),
             RAD_TO_DEG(r->a1_b2.an_b));
}

// ----------------------------------------------
// check a result stability
// ----------------------------------------------
#if 0
static bool check_pass(struct ineq_res_t *r)
{
  int i;
  co_printf("CHECK SOLUTION, N = %d ...\n", epr_sim_conf.N);
  for (i=0; i<10; i++)
  {
    eber_test(r);
    co_printf("\tCHK:[%d] J/N:%.5f\n", i, r->Eb.JoN);
    if (r->Eb.JoN > 0)
    {
      co_printf("Failed.\n");
      return false;
    }
  }
  co_printf("PASSED: ");
  print_res(r);
  return true;
}
#endif
// ----------------------------------------------
// optimize angles
// ----------------------------------------------

static void res_cmp(int N, struct det_ctr_t *a1_b1,
                           struct det_ctr_t *a1_b2,
                           struct det_ctr_t *a2_b1,
                           struct det_ctr_t *a2_b2,
                           struct ineq_res_t *r_best)
{
  struct ineq_res_t r;
  eber_def_res(N, a1_b1, a1_b2, a2_b1, a2_b2, &r);
  if (r.Eb.JoN < r_best->Eb.JoN)
  {
    W_ASSERT(r.Eb.JoN > -0.208);
    r_best->a1_b1 = *a1_b1;
    r_best->a1_b2 = *a1_b2;
    r_best->a2_b1 = *a2_b1;
    r_best->a2_b2 = *a2_b2;
    r_best->N = r.N;
    r_best->Eb = r.Eb;
  }
}

static bool eber_optim(struct ineq_res_t *r, float da)
{
  struct ineq_res_t r_best;

  struct det_ctr_t
  *a1_b1, *a1_b2,
  *a2_b1, *a2_b2,
  a1h_b1, a1h_b2,
  a1l_b1, a1l_b2,
  a2h_b1, a2h_b2,
  a2l_b1, a2l_b2,
  a1_b1h, a2_b1h,
  a1_b1l, a2_b1l,
  a1_b2h, a2_b2h,
  a1_b2l, a2_b2l;

  float a1 = r->a1_b1.an_a;
  float a2 = r->a2_b1.an_a;
  float b1 = r->a1_b1.an_b;
  float b2 = r->a1_b2.an_b;

  float a1h = a1 + da;
  float a1l = a1 - da;
  float a2h = a2 + da;
  float a2l = a2 - da;
  float b1h = b1 + da;
  float b1l = b1 - da;
  float b2h = b2 + da;
  float b2l = b2 - da;

  co_printf("ANGLES ADJUST +/-%.2f ... ", RAD_TO_DEG(da));

  a1_b1 = &r->a1_b1;
  a1_b2 = &r->a1_b2;
  a2_b1 = &r->a2_b1;
  a2_b2 = &r->a2_b2;

  // need to update counters for new N value
  epr_test(a1, b1, &r->a1_b1);
  epr_test(a1, b2, &r->a1_b2);
  epr_test(a2, b1, &r->a2_b1);
  epr_test(a2, b2, &r->a2_b2);
  eber_def_res(epr_sim_conf.N, &r->a1_b1, &r->a1_b2, &r->a2_b1, &r->a2_b2, &r_best);
  r_best = *r;

  #define EPR_2X(ct0, ct1, a1_0, b1_0, a1_1, b1_1)  epr_test(a1_0, b1_0, ct0); epr_test(a1_1, b1_1, ct1)

  EPR_2X(&a1h_b1, &a1h_b2, a1h,  b1, a1h,  b2);
  EPR_2X(&a1l_b1, &a1l_b2, a1l,  b1, a1l,  b2);
  EPR_2X(&a2h_b1, &a2h_b2, a2h,  b1, a2h,  b2);
  EPR_2X(&a2l_b1, &a2l_b2, a2l,  b1, a2l,  b2);
  EPR_2X(&a1_b1h, &a2_b1h,  a1, b1h,  a2, b1h);
  EPR_2X(&a1_b1l, &a2_b1l,  a1, b1l,  a2, b1l);
  EPR_2X(&a1_b2h, &a2_b2h,  a1, b2h,  a2, b2h);
  EPR_2X(&a1_b2l, &a2_b2l,  a1, b2l,  a2, b2l);

  res_cmp(epr_sim_conf.N, &a1h_b1, &a1h_b2,   a2_b1,   a2_b2, &r_best);
  res_cmp(epr_sim_conf.N, &a1l_b1, &a1l_b2,   a2_b1,   a2_b2, &r_best);
  res_cmp(epr_sim_conf.N,   a1_b1,   a1_b2, &a2h_b1, &a2h_b2, &r_best);
  res_cmp(epr_sim_conf.N,   a1_b1,   a1_b2, &a2l_b1, &a2l_b2, &r_best);

  res_cmp(epr_sim_conf.N, &a1_b1h,   a1_b2, &a2_b1h,   a2_b2, &r_best);
  res_cmp(epr_sim_conf.N, &a1_b1l,   a1_b2, &a2_b1l,   a2_b2, &r_best);
  res_cmp(epr_sim_conf.N,   a1_b1, &a1_b2h,   a2_b1, &a2_b2h, &r_best);
  res_cmp(epr_sim_conf.N,   a1_b1, &a1_b2l,   a2_b1, &a2_b2l, &r_best);

  res_cmp(epr_sim_conf.N, &a1h_b1, &a1h_b2, &a2h_b1, &a2h_b2, &r_best);
  res_cmp(epr_sim_conf.N, &a1l_b1, &a1l_b2, &a2h_b1, &a2h_b2, &r_best);
  res_cmp(epr_sim_conf.N, &a1h_b1, &a1h_b2, &a2l_b1, &a2l_b2, &r_best);
  res_cmp(epr_sim_conf.N, &a1l_b1, &a1l_b2, &a2l_b1, &a2l_b2, &r_best);

  res_cmp(epr_sim_conf.N, &a1_b1h, &a1_b2h, &a2_b1h, &a2_b2h, &r_best);
  res_cmp(epr_sim_conf.N, &a1_b1l, &a1_b2h, &a2_b1l, &a2_b2h, &r_best);
  res_cmp(epr_sim_conf.N, &a1_b1h, &a1_b2l, &a2_b1h, &a2_b2l, &r_best);
  res_cmp(epr_sim_conf.N, &a1_b1l, &a1_b2l, &a2_b1l, &a2_b2l, &r_best);

  if (r_best.Eb.JoN < r->Eb.JoN)
  {
    co_printf("Found J/N %.6f\n", r_best.Eb.JoN);
    *r = r_best;
    return true;
  }
  co_printf("\n");
  return false;
}

static void eber_adjust(struct ineq_res_t *r)
{
  epr_init_sim_N(250*1000);
  while (eber_optim(r, DEG_TO_RAD(5.0f)))
    EPR_PROGRESS_TO(60, 5);

  EPR_PROGRESS(50);
  epr_init_sim_N(500*1000);
  while (eber_optim(r, DEG_TO_RAD(1.0f)))
    EPR_PROGRESS_TO(85, 5);

  EPR_PROGRESS(75);
  epr_init_sim_N(1000*1000);
  while (eber_optim(r, DEG_TO_RAD(0.1f)))
    EPR_PROGRESS_TO(95, 5);

#if 0
  // 0.1f seem enough
  epr_init_sim_N(2000*1000);
  while (eber_optim(r, DEG_TO_RAD(0.05f)))
    ;
#endif

#if 0
  // strong check
  epr_init_sim_N(10*1000*1000);
  if (r->Eb.JoN < 0)
    check_pass(r);
#endif
}

// ----------------------------------------------
// find randomly angles
// ----------------------------------------------

static void eber_rand(struct ineq_res_t *r)
{
  double JoN_best = r->Eb.JoN;
  struct ineq_res_t r_best;
  int i;

  epr_init_sim_N(150*1000);
  for (i=0; i<100; i++)
  {
    float a1 = RAND_PI;
    float a2 = RAND_PI;
    float b1 = RAND_PI;
    float b2 = RAND_PI;

    epr_test(a1, b1, &r_best.a1_b1);
    epr_test(a1, b2, &r_best.a1_b2);
    epr_test(a2, b1, &r_best.a2_b1);
    epr_test(a2, b2, &r_best.a2_b2);
    eber_def_res(epr_sim_conf.N, &r_best.a1_b1, &r_best.a1_b2, &r_best.a2_b1, &r_best.a2_b2, &r_best);

    if (r_best.Eb.JoN < JoN_best)
    {
      JoN_best = r_best.Eb.JoN;
      *r = r_best;
    }
    EPR_PROGRESS(i/2);
  }
  co_printf("Found: "); print_res(r);
}

void eber_search(int count)
{
  int i, np = 0;
  for (i=0; i<count; i++)
  {
    struct ineq_res_t *r = &ineq_search_res.ineq_res[i];
    co_printf("-------------------------------------\n"
              "ANGLE SEARCH RANDOM [%d] ...\n", i+1);
    r->Eb.JoN = 1.0;
    eber_rand(r);
    eber_adjust(r);
    co_printf("ADD: ");
    print_res(r);
    np += r->Eb.JoN >= 0;
  }
  ineq_search_res.count_found = count;
  co_printf("FOUND %d result(s) >= 0 and %d < 0.\n", np, count - np);
}

void eber_eval_single(float a1, float a2, float b1, float b2, struct ineq_res_t *r)
{
  epr_test(a1, b1, &r->a1_b1); EPR_PROGRESS(25);
  epr_test(a1, b2, &r->a1_b2); EPR_PROGRESS(50);
  epr_test(a2, b1, &r->a2_b1); EPR_PROGRESS(75);
  epr_test(a2, b2, &r->a2_b2);
  eber_def_res(epr_sim_conf.N, &r->a1_b1, &r->a1_b2, &r->a2_b1, &r->a2_b2, r);
  co_printf("Result: "); print_res(r);
}

void eber_adjust_single(float a1, float a2, float b1, float b2, struct ineq_res_t *r)
{
  r->a1_b1.an_a = a1; r->a1_b1.an_b = b1;
  r->a1_b2.an_a = a1; r->a1_b2.an_b = b2;
  r->a2_b1.an_a = a2; r->a2_b1.an_b = b1;
  r->a2_b2.an_a = a2; r->a2_b2.an_b = b2;
  r->Eb.JoN = 1.0f;
  eber_adjust(r);
  co_printf("Result: "); print_res(r);
}

// save to define standard deviation (need global average before)
static float posi_JoN_list[MAX_INEQ_POSI_CNT];

static double get_std_dev(double avg, int n)
{
  double d2_sum = 0;
  int i;
  for (i=0; i<n; i++)
  {
    double d = posi_JoN_list[i] - avg;
    d2_sum += d*d;                     // sum of squared diff with average
  }
  return sqrt(d2_sum/n);
}

int eber_eval_positivity(float a1, float a2, float b1, float b2, int count)
{
  struct ineq_res_t r;
  int i, np = 0;
  double avg_JoN, dev_JoN, sum_JoN = 0;
  r.a1_b1.an_a = a1; r.a1_b1.an_b = b1;
  r.a1_b2.an_a = a1; r.a1_b2.an_b = b2;
  r.a2_b1.an_a = a2; r.a2_b1.an_b = b1;
  r.a2_b2.an_a = a2; r.a2_b2.an_b = b2;

  W_ASSERT(count <= MAX_INEQ_POSI_CNT);
  for (i=1; i<=count; i++)
  {
    eber_test(&r);
    np += r.Eb.JoN >= 0;
    sum_JoN += r.Eb.JoN;
    posi_JoN_list[i-1] = (float)r.Eb.JoN;
    co_printf("\tTest:[%d] J/N:%.6f \t (%d/%d = %2.f%%)\n", i, r.Eb.JoN, np, i, (np*100.0f)/i);
    EPR_PROGRESS((i*100)/count);
  }
  avg_JoN = sum_JoN/count;
  dev_JoN = get_std_dev(avg_JoN, count);

  co_printf("Result: %d/%d positive results. Avg J/N:%.6f rms:%.6f Positivity: %.2f%%\n", 
    np, count, 
    avg_JoN, 
    dev_JoN,
    (np*100.0f)/count);
  return np;
}
