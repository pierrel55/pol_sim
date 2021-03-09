
// experiment list
#define MAX_EXP_COUNT 25

extern int experiment_str_list_size;
extern const char *experiment_str_list[MAX_EXP_COUNT];

void dlg_ctrl_update_graph(float x0, float x1, int data_count);

// all dialog states
struct dlg_state_t
{
  struct
  {
    // checkbox
    bool corr_oo_ee;
    bool corr_oe_eo;
    bool det_pair_ratio;
    bool det_sing_ratio;
    bool det_uu_ratio;
    bool det_acc_ratio;
    bool transm_alice_o;
    bool tr_rel_alice_o;
    bool ref_sin2;
    bool ref_cos2;
    bool ref_triangle;
    bool src_emit_t_dist;

    // drawing
    float graph_N;
  } curves;
  struct
  {
    bool model_qm;
    bool model_local;
    enum e_rng_type rng_type;
    float rng_seed;
  } sim;
  struct
  {
    float emit_pol_diff;
    float ent_qm_r;
    float ent_local_s;
    bool local_polarize;
    float em_prob_ent_pair;
    float em_prob_nent_pair;
    float em_prob_single;
    float em_prob_double;
    float em_no_emit_lbl_value;
    bool emit_enable_jitter;
    float emit_time_beta_scale;
  } source;
  struct
  {
    float det_prob_alice_det_o;
    float det_prob_alice_det_e;
    bool det_prob_bob_same_alice;
    float det_prob_bob_det_o;
    float det_prob_bob_det_e;
    float udet_dep_random;
    float udet_dep_local_dpr;
    float udet_dep_local_q;
    float app_det_ratio_lbl;
    float misc_noise_det_r;
    float misc_retrig_delay;
  } detectors;
  struct
  {
    float e_out_delay;
    float o_out_delay;
    float st1_delay;
  } polarizer;
  struct
  {
    bool win_type_fixed;
    bool win_type_mobile;
    float win_width;
    bool ignore_accidentals;
  } pairing;
  struct
  {
    float test_conf_a1;
    float test_conf_a2;
    float test_conf_b1;
    float test_conf_b2;
    float test_conf_N;
    bool auto_seed;
    float eval_posi_count;
    float angles_max_search;
    bool auto_select_res;
  } inequality;
  struct
  {
    float pol_count;
    float max_error;
    float max_anv;
    float sim_N;
    char pol_list_angles_str[256];
  } transmit_test;
};

extern struct dlg_state_t epr_dlg;

void dlg_ctrl_init(void);
void dlg_init_dialog(void);

void dlg_inequality_test(void);
void dlg_inequality_optimize(void);
void dlg_ineq_tools_find_angles(void);
void dlg_ineq_tools_eval_posi(void);
void dlg_transmit_random_test(void);
void dlg_transmit_list_test(void);

void init_config_1_qm_perfect(void);             // default config
void dlg_update_ineq_res_combo(void);            // is in dlg_init.c

void dlg_select_experiment(int sel_id);          // defined in epr_exp.c
