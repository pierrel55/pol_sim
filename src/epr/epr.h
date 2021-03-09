#include <stdbool.h>
#include "rng.h"

#define PI 3.14159265358979323846f               // code common PI value

#define CRV_MAX_Y_DATA 1200                      // max y datas for curves

#define MAX_INTF 16777216                        // max integer value (+/- 0x1000000) that can be stored without loss in 32 bits float (IEEE 754)

#define RAD_TO_DEG(r) (((r)*180.0f)/PI)
#define DEG_TO_RAD(d) (((d)*PI)/180.0f)

#define EPSILON 0.000001f

// out coincidence counter index
enum e_ct_id
{
  e_id_oo = 0,
  e_id_oe,
  e_id_eo,
  e_id_ee,

  e_id_uo,
  e_id_ue,
  e_id_ou,
  e_id_eu,

  e_id_uu,
  e_id_acc,
};

// coincidence/detections counters
struct det_ctr_t
{
  float an_a;                                    // Alice angle used for test
  float an_b;                                    // Bob angle used for test
  // note: Alice at left, Bob at right
  union
  {
    int id[10];
    struct
    {
      // pair detection
      int oo;
      int oe;
      int eo;
      int ee;

      // single detection (u: undetected)
      int uo;
      int ue;
      int ou;
      int eu;

      // double non-detection
      int uu;
      // undetected accidentals (must be 0)
      int acc;
    };
  };
  // accidentals per detector
  int acc_n;
  int acc_ao;
  int acc_ae;
  int acc_bo;
  int acc_be;
};

// simulation result data
struct sim_data_t
{
  float co_oo_ee;
  float co_oe_eo;

  float det_pair_ratio;
  float det_sing_ratio;
  float det_uu_ratio;
  float det_acc_ratio;
  float transmit_a;
  float tr_rel_ao;
  float src_t_dis;

  // references curves
  float sin2;                                    // sin² reference
  float cos2;                                    // cos² reference
  float triangle;                                // realistic triangle
};

// simulation results
struct sim_res_t
{
  float x0;
  float x1;
  int sim_data_count;
  struct sim_data_t sim_data_res[CRV_MAX_Y_DATA];
};

extern struct sim_res_t sim_result;

// print on console
void co_printf(const char *fmt, ...);

// ----------------------------------------------
// experiment configuration

#define USE_INT_LSIM                             // use integers instead of floats for locale simulation.

// detectors detection parameters
struct det_conf_t
{
#ifdef USE_INT_LSIM
  int dpr;
  int max_rp;
  int max_q;
#else
  float dpr;                                     // detection dependance to randomness
  float max_rp;                                  // detection dependance to rp
  float max_q;                                   // detection dependance to q
#endif
};

#define TR_GRAPH_BITS 10                         // define TR_GRAPH_N value (must be 2^n value)
#define TR_GRAPH_N (1 << TR_GRAPH_BITS)          // count of counters for transmittance graph.

// arm parameters (local simulation)
struct arm_t
{
  struct det_conf_t det_o;
  struct det_conf_t det_e;
#ifdef USE_INT_LSIM
  int an_pol;                                    // polarizer angle
  int delay_e;
  int delay_o;
  int delay_st1;
#else
  float an_pol;                                  // polarizer angle
  float delay_e;
  float delay_o;
  float delay_st1;
#endif
  // transmittance graph
  int tr_N;                                      // count of polarizations
  int *tr_dat;
};

// ------------------------------------------
// source beta jitter function parameters
#define TJIT_A 0.5f
#define TJIT_B 8.0f

// scale correction: s = dmax*(ab + a²)/b
#define SC_JFUNC(m) (((m)*(TJIT_A*TJIT_B + TJIT_A*TJIT_A))/TJIT_B)
#define SRC_JFUNC(s, r)  ((s)/(TJIT_A + TJIT_B*r))

struct epr_sim_conf_t
{
  bool win_fixed;
  bool qm_mode;
  bool ignore_acc;
  // qm model specific
  struct 
  {
    float em_pdiff;
    float r;
    float st_dpr_a[16];
    float st_dpr_b[16];
  } qm;
  // local model specific
  struct
  {
#ifdef USE_INT_LSIM
    int src_beta_scale;
    int em_rdiff;
    int s;
    int win_time;
#else
    float src_beta_scale;
    float em_rdiff;
    float s;
    float win_time;
#endif
    bool local_polarize;
    struct arm_t arm_a, arm_b;
  } local;
  // detectors probabilities
  float p_det_ao;
  float p_det_ae;
  float p_det_bo;
  float p_det_be;
  // source emit probabilities
  float p_em_ent_pair;
  float p_em_n_ent_pair;
  float p_em_single;
  float p_em_ent_double;  
  // source emit count
  int N;
  int n_em_ent_pair;
  int n_em_n_ent_pair;
  int n_em_single;
  int n_em_ent_double;
  // noise
  float p_noise;
};

extern struct epr_sim_conf_t epr_sim_conf;

// ----------------------------------------------
// Inequality test result
// ----------------------------------------------

// inequality test result
struct ineq_res_t
{
  struct det_ctr_t a1_b1;                        // counters
  struct det_ctr_t a1_b2;
  struct det_ctr_t a2_b1;
  struct det_ctr_t a2_b2;
  int N;                                         // used N
  struct
  {
    int J;                                       // J
    double JoN;                                  // J/N
  } Eb;
};

#define MAX_INEQ_TEST_RES 20
#define MAX_INEQ_POSI_CNT 1000

struct ineq_search_res_t
{
  struct ineq_res_t ineq_res[MAX_INEQ_TEST_RES];
  int count_found;
};

extern struct ineq_search_res_t ineq_search_res;

void eber_search(int count);
void eber_eval_single(float a1, float a2, float b1, float b2, struct ineq_res_t *r);
void eber_adjust_single(float a1, float a2, float b1, float b2, struct ineq_res_t *r);
int eber_eval_positivity(float a1, float a2, float b1, float b2, int count);

// ----------------------------------------------
// transmittance

void check_malus_random(int max_aligned, float an_var, int N_sim, float max_error);
void check_malus_user_angles(float *an_list, int list_size, int N_sim, float max_error);

// ----------------------------------------------
// QM

void epr_sim_init(void);

// init simulation constants
void epr_init_sim_const(bool win_fixed, bool qm_mode, bool ignore_acc,
                        float em_pdiff, 
                        float qm_r,
                        float l_src_beta_scale,
                        float l_s, bool l_polarize, float udet_dep_dpr, float udet_dep_q, float l_win_time, 
                        float l_delay_o, float l_delay_e, float l_st1_delay,
                        float p_det_ao, float p_det_ae, float p_det_bo, float p_det_be,
                        float p_em_ent_pair, float p_em_n_ent_pair, float p_em_single, float p_em_ent_double,
                        float p_noise);

void epr_init_sim_N(int N);

void epr_simulate(float an_a, float an_b, struct det_ctr_t *ctr);

// ----------------------------------------------

enum epr_task_type
{
  e_task_type_undef = 0,
  e_task_type_test_ineq,
  e_task_type_optimize,
  e_task_type_eval_posi,
  e_task_type_find_angles,
  e_task_type_test_tr_random,
  e_task_type_test_tr_list,
  e_task_type_upd_graph,
};

void dlg_run_thread_task(enum epr_task_type task_type, bool reset_seed, const char *run_status);

struct epr_task_t
{
  enum epr_task_type task_type;
  int progress100;
  volatile bool cancel;                // set by main thread
};

extern struct epr_task_t epr_task;

void epr_task_progress(int progress);
void epr_task_progress_to(int progress, int step);
void epr_task_cancel(void);

#define CHECK_THRD_CANCEL() if (epr_task.cancel) epr_task_cancel()
#define EPR_PROGRESS(p) epr_task_progress(p)
#define EPR_PROGRESS_TO(p, s) epr_task_progress_to(p, s)


