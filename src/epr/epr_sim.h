// ----------------------------------------------------
// 4 bits code polarizer out & coincidences & detection
// Max 3 detections can be done (2 pair emit + background noise)
// This can produce a 12 bits detection state.

//   Alice          Bob
// b3     b2     b2     b0
//  e      o      e      o
// This produce a 0..15 value to code state

#define DET_U 0              // no detection
#define OUT_O 1              // o out (b0 = 1)
#define OUT_E 2              // e out (b1 = 1)

#define B_o OUT_O
#define B_e OUT_E
#define A_o (OUT_O << 2)
#define A_e (OUT_E << 2)

extern const unsigned int st_to_ct_id[16];

// update counters
void epr_count(int m, struct det_ctr_t *ctr);

// qm
void qm_init_sim_const(float em_pdiff, float qm_r);
void qm_simulate(float an_a, float an_b, struct det_ctr_t *ctr);

// local
void local_init_sim_const(float em_pdiff, float l_src_beta_scale, float l_s, bool l_polarize, 
                          float udet_dep_dpr, float udet_dep_q,
                          float l_win_time, float l_delay_o, float l_delay_e, float l_st1_delay);
void local_simulate(float an_a, float an_b, struct det_ctr_t *ctr);
void local_simulate_mobile(float an_a, float an_b, struct det_ctr_t *ctr);

