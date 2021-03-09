// init dialog window and widgets
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "win/widget/widget.h"
#include "epr/epr.h"
#include "dlg_ctrl.h"

extern const res_img8c_t epr_img_res;  // epr image reource

// inequality test results list combo strings ptrs
static const char *ineq_test_res_list[MAX_INEQ_TEST_RES];
static int ineq_test_res_list_size = 0;

static hwin_t *hw = NULL;              // the main dialog window handle

// edit numerical value + num descriptor
typedef struct
{
  num_desc_t nd;                       // numeric descriptor
  widget_t *lbl;                       // left label
  widget_t *ed;                        // edit + spin widget
} eds_t;

// define widgets list and static datas
static struct
{
  // ----------------------------------
  // frame simulation
  struct
  {
    widget_t *frm;
    struct
    {
      widget_t *frm;
      widget_t *rad_qm;
      widget_t *rad_local;
    } model;

    struct
    {
      widget_t *frm;
      widget_t *rad_c_lib;
      widget_t *rad_ms_c_lib;
      widget_t *rad_xorshift;
      widget_t *rad_cpu_buildin;
      widget_t *bt_test_col;
      widget_t *bt_test_grey;
      widget_t *bt_test_bw;
      widget_t *label_select_seed;
      num_desc_t nd_seed;
      widget_t *edn_seed;
      widget_t *bt_apply_seed;
    } rng;
  } simulation;

  // ----------------------------------
  // frame source
  struct
  {
    widget_t *frm;
    eds_t eds_emit_pol_diff;
    struct
    {
      widget_t *frm;
      eds_t eds_qm_r;
      eds_t eds_local_s;
      widget_t *chk_local_polarize;
    } entang;
    struct
    {
      widget_t *frm;
      eds_t eds_ent_pair;
      eds_t eds_nent_pair;
      eds_t eds_single;
      eds_t eds_2nd;
      char lbl_no_emit_str[32];
      widget_t *lbl_no_emit;
    } emit_prob;
    struct
    {
      widget_t *frm;
      widget_t *chk_enable_jitter;
      eds_t eds_beta_scale;
    } emit_delay;
  } source;

  // ----------------------------------
  // frame polarizer
  struct
  {
    widget_t *frm;
    eds_t eds_e_out_delay;
    eds_t eds_o_out_delay;
    eds_t eds_st1_delay;
  } polarizer;

  // ----------------------------------
  // frame detectors
  struct
  {
    widget_t *frm;
    struct
    {
      widget_t *frm;
      struct
      {
        widget_t *frm;
        eds_t eds_det_o;
        eds_t eds_det_e;
      } alice;

      struct
      {
        widget_t *frm;
        widget_t *chk_same_alice;
        eds_t eds_det_o;
        eds_t eds_det_e;
      } bob;

      struct
      {
        widget_t *frm;
        eds_t eds_loc_dpr;
        eds_t eds_loc_qr;
        char lbl_udet_rand_str[32];
        widget_t *lbl_udet_rand;
      } u_detect_dep;
    } detect_prob;

    struct
    {
      widget_t *frm;
      eds_t eds_dnoise_ratio;
    } misc;
  } detectors;

  // ----------------------------------
  // frame pairing
  struct
  {
    widget_t *frm;
    struct
    {
      widget_t *frm;
      widget_t *rad_fixed;
      widget_t *rad_mobile;
    } win_type;

    eds_t eds_win_width;
    widget_t *chk_ignore_dual;
  } pairing;

  // ----------------------------------
  // frame inequality

  struct
  {
    widget_t *frm;
    struct
    {
      widget_t *frm;

      struct
      {
        widget_t *frm;
        eds_t eds_a1;
        eds_t eds_a2;
        eds_t eds_b1;
        eds_t eds_b2;
      } angles;

      eds_t eds_N;

      widget_t *but_test;
      widget_t *but_optimize;
      widget_t *chk_auto_seed;

      struct
      {
        widget_t *frm;
        eds_t eds_count;
        widget_t *but_eval;
      } eval_posi;
    } test;

    struct
    {
      widget_t *frm;
      eds_t eds_count;
      widget_t *but_search;
      widget_t *lbl_results;
      widget_t *cbo_results;
      widget_t *chk_auto_select;
    } find_angles;
  } inequality;

  // ----------------------------------
  // frame transmit
  struct
  {
    widget_t *frm;
    // random
    widget_t *but_random;

    widget_t *lbl_nb_pol;
    num_desc_t edn_pol_cnt_nd;
    widget_t *edn_pol_cnt;

    widget_t *lbl_max_anv;
    num_desc_t edn_max_anv_nd;
    widget_t *edn_max_anv;

    widget_t *lbl_sim_N;
    num_desc_t edn_sim_N_nd;
    widget_t *edn_sim_N;

    widget_t *lbl_max_err;
    num_desc_t edn_max_err_nd;
    widget_t *edn_max_err;

    // list
    widget_t *but_list;
    widget_t *ed_list;
  } transmit;

  // ----------------------------------
  // frame experiment
  struct
  {
    bitmap_t exp_logo;
    widget_t *frm;
    widget_t *wg_logo;
    widget_t *label_select_exp;
    widget_t *combo_select_exp;
    const char *lbl_status_idle_str;
    widget_t *lbl_status;
    widget_t *run_progress;
    widget_t *but_break;
  } experiment;

  // -----------------------------------
  // frame curves
  struct
  {
    widget_t *frm;
    widget_t *co_oo_ee;
    widget_t *co_oe_eo;
    widget_t *det_pair_ratio;
    widget_t *det_sing_ratio;
    widget_t *det_uu_ratio;
    widget_t *det_acc_ratio;
    widget_t *transm_alice_o;
    widget_t *tr_rel_alice_o;
    widget_t *ref_sin2;
    widget_t *ref_cos2;
    widget_t *ref_triangle;
    widget_t *src_emit_t_dist;

    // graph control
    widget_t *frm_drawing;
    eds_t eds_graph_N;
    int graph_col_id;
    widget_t *chk_dark_col;
    widget_t *chk_no_pix_aa;
    widget_t *bt_update;
  } curves;

  // -----------------------------------
  // frame graph
  struct
  {
    widget_t *frm;
    widget_t *wg_graph;
  } graph;

  // -----------------------------------
  // frame messages console
  struct
  {
    widget_t *frm;
    widget_t *console_out;
    widget_t *bt_clear;
    widget_t *bt_copy;
    widget_t *bt_help;
    widget_t *bt_exit;
  } console;

} dlg = { 0 };

#define ED_R1(ini, step) ini, 0.0f, 1.0f, step, 3, 0       // 0..1 range edit
#define MFRAME_BOLD                              // if defined, use bold text on group frames

#ifdef MFRAME_BOLD                               // use bold drawing (convert tahoma9)
  #define FRM_EX win_font_list[fnt_vthm9], wgs.frame.text_color, bold_aa_color
#else
  #define FRM_EX win_font_list[wgs.frame.text_font_id], wgs.frame.text_color, wgs.frame.text_aa_color
#endif

// widget call backs
static void radio_button_cb(hwin_t *hwin, widget_t *wg);
static void check_box_cb(hwin_t *hwin, widget_t *wg);
static void check_box_cb_crv(hwin_t *hwin, widget_t *wg);
static void text_bt_cb(hwin_t *hwin, widget_t *wg);
static void select_experiment_cb(hwin_t *hwin, int sel_id);
static void select_ineq_test_result_cb(hwin_t *hwin, int sel_id);
static void eds_num_value_changed_cb(widget_t *wg, float value);
static void graph_post_draw_cb(hwin_t *hwin, widget_t *wg, bitmap_t *bm);

// init composed widget, label + numeric edit box + spin
static void eds_init(eds_t *eds, const char *label, float init_value, float min_value, float max_value,
                     float inc_step, int max_deci, int spc_digi, const char *help_text)
{
  num_desc_init(&eds->nd, init_value, min_value, max_value, inc_step, max_deci, spc_digi, help_text);
  eds->lbl = wg_init_text(hw, label);
  eds->lbl->help_text = help_text;
  eds->ed = wg_init_edit_box_num(hw, &eds->nd, true, eds_num_value_changed_cb);
}

// init widget list, define help
static void init_widget_list(void)
{
#ifdef MFRAME_BOLD                        // init bold font and add 1 pixel space between chars.
  pix_t bold_aa_color = font_gen_bold(win_font_list[fnt_vthm9], wgs.frame.text_aa_color, win_font_list[fnt_vthm9], 1);
#endif

  // --------------------------------------------
  // Simulation

  // frame simulation
  dlg.simulation.frm = wg_init_frame_ex(hw, "Simulation", FRM_EX);

  // frame model
  dlg.simulation.model.frm             = wg_init_frame(hw, "Model");
  dlg.simulation.model.rad_qm            = wg_init_radio_button(hw, "QM", radio_button_cb);
  dlg.simulation.model.rad_qm->help_text =
    "Simulate QM theorical results.";
  dlg.simulation.model.rad_local         = wg_init_radio_button(hw, "Local", radio_button_cb);
  dlg.simulation.model.rad_local->help_text =
    "Simulate a local polarizer model.";

  // frame RNG
  dlg.simulation.rng.frm               = wg_init_frame(hw, "RNG");
  dlg.simulation.rng.frm->help_text =
    "This frame allow to select random generators used for simulation.";
  dlg.simulation.rng.rad_c_lib           = wg_init_radio_button(hw, "C library", radio_button_cb);
  dlg.simulation.rng.rad_c_lib->help_text =
    "Use the compiler's C library RNG used to build the program.";
  dlg.simulation.rng.rad_ms_c_lib        = wg_init_radio_button(hw, "ms C library", radio_button_cb);
  dlg.simulation.rng.rad_ms_c_lib->help_text =
    "Use the microsoft C library RNG algorithm. Is fast.";
  dlg.simulation.rng.rad_xorshift        = wg_init_radio_button(hw, "xorshift", radio_button_cb);
  dlg.simulation.rng.rad_xorshift->help_text =
    "Use xoshiro256+ RNG algorithm.";
  dlg.simulation.rng.rad_cpu_buildin     = wg_init_radio_button(hw, "CPU build-in", radio_button_cb);
  dlg.simulation.rng.rad_cpu_buildin->help_text =
    "Use CPU integrated hardware random generator (if detected).\n"
    "Very good quality but rather slow RNG";
  dlg.simulation.rng.bt_test_col         = wg_init_text_button(hw, "Test c", text_bt_cb);
  dlg.simulation.rng.bt_test_col->help_text =
    "Fill graph with RGB pixels colors produced by selected RNG.\n"
    "Dsplay result must be uniform";
  dlg.simulation.rng.bt_test_grey        = wg_init_text_button(hw, "Test g", text_bt_cb);
  dlg.simulation.rng.bt_test_grey->help_text =
    "Fill graph with pixels greyscales produced by selected RNG.";
  dlg.simulation.rng.bt_test_bw         = wg_init_text_button(hw, "Test b", text_bt_cb);
  dlg.simulation.rng.bt_test_bw->help_text =
    "Dill graph with pixels black or white color produced by selected RNG.";

  dlg.simulation.rng.label_select_seed   = wg_init_text(hw, "RNG seed");
  dlg.simulation.rng.label_select_seed->help_text =
    "Seed value for random generators (ignored if CPU build-in selected)";

  num_desc_init(&dlg.simulation.rng.nd_seed, 123, -MAX_INTF, MAX_INTF, 0, 0, 0, dlg.simulation.rng.label_select_seed->help_text);
  dlg.simulation.rng.edn_seed            = wg_init_edit_box_num(hw, &dlg.simulation.rng.nd_seed, false, eds_num_value_changed_cb);

  dlg.simulation.rng.bt_apply_seed       = wg_init_text_button(hw, "Apply seed", text_bt_cb);
  dlg.simulation.rng.bt_apply_seed->help_text =
    "Reset current selected RNG with seed.\n"
    "note: This is also done when seed value or RNG type is changed.";

  // --------------------------------------------
  // Source

  // frame source
  dlg.source.frm                       = wg_init_frame_ex(hw, "Source", FRM_EX);
  dlg.source.frm->help_text =
    "Define simulation parameters of a pulsed source.";

  // --------------------
  // pol emit adiff
  eds_init(&dlg.source.eds_emit_pol_diff, "Emit pol diff.", 90.0f, 0.0f, 90.0f, 90.0, 2, 0,
    "Value range [0..90] (degree)\n"
    "Emission polarisation difference between Alice and Bob.");

  // --------------------
  // frame entang
  dlg.source.entang.frm                = wg_init_frame(hw, "Entanglement");
  eds_init(&dlg.source.entang.eds_qm_r, "QM: r", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "QM entanglement level.");
  eds_init(&dlg.source.entang.eds_local_s, "Local: s", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "There is no entangled state with a local model, but the parameter s defines a level\n"
    "of coupling between hidden variables values when emitted by the source.\n\n"
    "A value of s = 0 defines independent values.\n"
    "A value of s = 1 defines identical values.\n\n"
    "The detection correlations are maximum for s = 1.");
  dlg.source.entang.chk_local_polarize = wg_init_check_box(hw, "Local polarize.", check_box_cb);
  dlg.source.entang.chk_local_polarize->help_text =
    "For local model only.\n"
    "If defined, force s = 1 only for polarization.\n"
    "This allow to simulate independant photons emitted with same polarization and\n"
    "check if the correlations and conform to Maxwell theory.\n\n"
    "In not defined, assume emission polarization coupling between Alice and Bod, is,\n"
    "as other hidden variables, dependant of s.\n"
    "Then, if s = 0, produce only random correlations.";

  // --------------------
  // emit_prob
  dlg.source.emit_prob.frm             = wg_init_frame(hw, "Emission prob.");
  dlg.source.emit_prob.frm->help_text =
    "Define source behavehour probability when emission request is done.";
  eds_init(&dlg.source.emit_prob.eds_ent_pair, "Ent. pair", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability of source to emit entangled pair.");
  eds_init(&dlg.source.emit_prob.eds_nent_pair, "n-Ent. pair", ED_R1(0.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability of source to emit non-entangled pair.");
  eds_init(&dlg.source.emit_prob.eds_single, "Single", ED_R1(0.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability of source to emit single photon.");
  eds_init(&dlg.source.emit_prob.eds_2nd, "Ent. Double", ED_R1(0.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability of source to emit more than one entangled pair. (value of 2 is used)\n"
    "This have effect to produce dual detections in pairing window.");

  dlg.source.emit_prob.lbl_no_emit     = wg_init_text(hw, "No emission: 0.000");
  dlg.source.emit_prob.lbl_no_emit->help_text =
    "Display the probability that source produce no emission.\n"
    "Result depend of emission defined probabilities.";

  // --------------------
  // emit times
  dlg.source.emit_delay.frm            = wg_init_frame(hw, "Emission times");
  dlg.source.emit_delay.frm->help_text =
    "Allow to define time jitter on emission request.";

  dlg.source.emit_delay.chk_enable_jitter = wg_init_check_box(hw, "Simulate jitter", check_box_cb);
  dlg.source.emit_delay.chk_enable_jitter->help_text =
    "If enabled, adds random time jitter when photons are emitted by the source.\n"
    "The added delay follow a beta random distribution. (The 'Src jitter' curve show the distribution).\n"
    "This allow to test, when fixed window method is used in pairing, the influence on inequality results.\n"
    "notes:\n"
    "  - This option is ignored if pairing method use mobile windows. (Jitter delais are removed when\n"
    "    time difference is used)\n"
    "  - Used with fixed windows, option may require to increase window size.";
  eds_init(&dlg.source.emit_delay.eds_beta_scale, "Beta scale", 1.0f, 0.0f, 5.0f, 0.25f, 2, 0,
    "Value range [0..5] (time scale).\n"
    "Time scale of a random beta distributed delay added after fixed source minimal emission delay.\n"
    "A value of 1 can add max 1x PI delay, 5 can add max 5x PI delay.\n"
    "PI delay is an arbitrary constant time proportional to PI value.\n\n"
    "note: The added delay distribution can be displayed using graph curve 'Src jitter'.\n"
    "      (The graph x axis then represent a time value).");

  // --------------------------------------------
  // frame polarizer
  dlg.polarizer.frm  = wg_init_frame_ex(hw, "Polarizer", FRM_EX);
  dlg.polarizer.frm->help_text =
    "Define some polarizer properties for local model";

  eds_init(&dlg.polarizer.eds_e_out_delay, "e out delay", ED_R1(0.0f, 0.05f),
    "Value range [0..1] (PI relative time delay)\n"
    "Delay added if polarizer output is e.");
  eds_init(&dlg.polarizer.eds_o_out_delay, "o out delay", ED_R1(0.0f, 0.05f),
    "Value range [0..1] (PI relative time delay)\n"
    "Delay added if polarizer output is o.");
  eds_init(&dlg.polarizer.eds_st1_delay, "st1 delay", ED_R1(0.0f, 0.01f),
    "Value range [0..1] (PI relative time delay)\n"
    "Delay added if repolarisation adjustement is > PI/4 (state 1)");

  // --------------------------------------------
  // detectors

  dlg.detectors.frm                    = wg_init_frame_ex(hw, "Detectors", FRM_EX);
  dlg.detectors.frm->help_text =
    "Define some photon detectors parameters";

  // --------------------
  // frame detect probability
  dlg.detectors.detect_prob.frm        = wg_init_frame(hw, "Detect probabilities");

  dlg.detectors.detect_prob.alice.frm  = wg_init_frame(hw, "Alice");
  eds_init(&dlg.detectors.detect_prob.alice.eds_det_o, "Det. out o", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability to detect Alice on o output.");
  eds_init(&dlg.detectors.detect_prob.alice.eds_det_e, "Det. out e", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability to detect Alice on e output.");

  dlg.detectors.detect_prob.bob.frm    = wg_init_frame(hw, "Bob");
  dlg.detectors.detect_prob.bob.chk_same_alice = wg_init_check_box(hw, "Same as Alice", check_box_cb);
  dlg.detectors.detect_prob.bob.chk_same_alice->help_text =
    "Use same probabilities values defined for Alice.";
  eds_init(&dlg.detectors.detect_prob.bob.eds_det_o, "Det. out o", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability to detect Bob on o output.");
  eds_init(&dlg.detectors.detect_prob.bob.eds_det_e, "Det. out e", ED_R1(1.0f, 0.01f),
    "Value range [0..1]\n"
    "Probability to detect Bob on e output.");

  dlg.detectors.detect_prob.u_detect_dep.frm = wg_init_frame(hw, "Undetect dep.");
  dlg.detectors.detect_prob.u_detect_dep.frm->help_text =
    "Theses parameters define the causes of undetections.\n"
    "They can be produced by randomness (QM) or/and for local model, depend of hidden variables values.";
  eds_init(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr, "Local dpr", ED_R1(0.0f, 0.05f),
    "Value range [0..1]\n"
    "For local model.\n"
    "Undetection dependance with amplitude of re-polarisation occured through polarizer.");
  eds_init(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr, "Local q", ED_R1(0.0f, 0.05f),
    "Value range [0..1]\n"
    "For local model.\n"
    "Undetection dependance with amplitude of q hidden variable.");
  dlg.detectors.detect_prob.u_detect_dep.lbl_udet_rand = wg_init_text(hw, "Randomness: 1.000");
  dlg.detectors.detect_prob.u_detect_dep.lbl_udet_rand->help_text =
    "Ratio of un-detections having random origin.\n"
    "For QM, as there is no hidden variables, it can be randomness only.";

  dlg.detectors.misc.frm               = wg_init_frame(hw, "Misc.");
  eds_init(&dlg.detectors.misc.eds_dnoise_ratio, "Noise det. r.", 0.0f, 0.0f, 1.0f, 0.001f, 3, 0,
    "Value range [0..1]\n"
    "Define for a single detector, probability to produce a spurious random detection.\n"
    "note: This is not simulated if pairing use mobile windows.");

  // --------------------
  // frame pairing
  dlg.pairing.frm              = wg_init_frame_ex(hw, "Pairing", FRM_EX);

    dlg.pairing.win_type.frm   = wg_init_frame(hw, "Window type");
      dlg.pairing.win_type.rad_fixed  = wg_init_radio_button(hw, "Fixed", radio_button_cb);
      dlg.pairing.win_type.rad_fixed->help_text =
        "Window time origin is synchronized with source emission command.\n"
        "Require a pulsed source.";
      dlg.pairing.win_type.rad_mobile = wg_init_radio_button(hw, "Mobile", radio_button_cb);
      dlg.pairing.win_type.rad_mobile->help_text =
        "Window time origin is synchronized on first Alice or Bob detection.\n"
        "Is sensitive to time coincidence loophole, and require fair sampling assumption.\n"
        "Can be used with pulsed or continuous source.";

    eds_init(&dlg.pairing.eds_win_width, "Window width", 0.5f, 0.0f, 5.0f, 0.01f, 3, 0,
      "Value range [0..5] (time scale)\n"
      "Define pairing window width."
      "A value of 0.5 (PI/2*k proportional) allow to detect all photons if no additional\n"
      "delay is added (jitter, e/o out delay, st1 delay)");
    dlg.pairing.chk_ignore_dual = wg_init_check_box(hw, "Ignore dual det.", check_box_cb);
    dlg.pairing.chk_ignore_dual->help_text =
      "If selected, mesures producing more than one detection in same detection window or arm\n"
      "are counted as uu (no detection).\n"
      "This can be produced by dual source emission or background noise detection.\n"
      "If not selected, all detections of invalid mesures are counted as single.\n"
      "note: This apply only to fixed windows pairing method. With mobile windows, all un-paired\n"
      "detections are always counted as single.";

  // --------------------------------------------
  // inequality

  // frame inequality
  dlg.inequality.frm                    = wg_init_frame_ex(hw, "Inequality", FRM_EX);

  // --------------------
  // frame test
  dlg.inequality.test.frm                 = wg_init_frame(hw, "Eberhard conf.");

    dlg.inequality.test.angles.frm        = wg_init_frame(hw, "Angles");
    dlg.inequality.test.angles.frm->help_text =
      "Value range [-360..360] (degree)\n"
      "Angle set to test Eberhard inequality";
      eds_init(&dlg.inequality.test.angles.eds_a1, "a1", 0, -360, 360, 1, 2, 0, "Value range [-360..360] (degree)\nInequality test Alice angle 1");
      eds_init(&dlg.inequality.test.angles.eds_a2, "a2", 0, -360, 360, 1, 2, 0, "Value range [-360..360] (degree)\nInequality test Alice angle 2");
      eds_init(&dlg.inequality.test.angles.eds_b1, "b1", 0, -360, 360, 1, 2, 0, "Value range [-360..360] (degree)\nInequality test Bob angle 1");
      eds_init(&dlg.inequality.test.angles.eds_b2, "b2", 0, -360, 360, 1, 2, 0, "Value range [-360..360] (degree)\nInequality test Bob angle 2");

    eds_init(&dlg.inequality.test.eds_N, "N Ineq.", 500000, 500000, 10000000, 500000, 0, 3,
      "Number of pairs emitted to test inequality for each a/b combination of angles");

    dlg.inequality.test.but_test = wg_init_text_button(hw, "Test", text_bt_cb);
    dlg.inequality.test.but_test->help_text =
      "Test selected inequality";
    dlg.inequality.test.but_optimize = wg_init_text_button(hw, "Opt.", text_bt_cb);
    dlg.inequality.test.but_optimize->help_text =
      "Adjust current angles to try to produce inequality violation, if possible";

    dlg.inequality.test.chk_auto_seed = wg_init_check_box(hw, "Auto seed", check_box_cb);
    dlg.inequality.test.chk_auto_seed->help_text =
      "Automatically reset RNG seed when button Test or Opt. is used.\n"
      "This allow to adjust angles and test using the same RNG seed.";

    // eval positivity
    dlg.inequality.test.eval_posi.frm     = wg_init_frame(hw, "Eval positivity");
    dlg.inequality.test.eval_posi.frm->help_text =
      "This tool evaluate ratio of positive/negative results during inequality evaluation.\n"
      "This makes it possible to check the stability of the sign of J/N result by repeating\n"
      "a simulation several times.\n"
      "This also makes it possible to evaluate the mean amplitude of J/N.";

      eds_init(&dlg.inequality.test.eval_posi.eds_count, "Eval count", 20, 10, MAX_INEQ_POSI_CNT, 10, 0, 0,
        "Count of run used for positivity evaluation.");
      dlg.inequality.test.eval_posi.but_eval = wg_init_text_button(hw, "Eval", text_bt_cb);
      dlg.inequality.test.eval_posi.but_eval->help_text =
        "Start eval positivity tool.";

    // ----------------------------------
    // find angles
    dlg.inequality.find_angles.frm   = wg_init_frame(hw, "Search angles");
    dlg.inequality.find_angles.frm->help_text =
      "This tool try to find angles that produce inequality violation.";
      eds_init(&dlg.inequality.find_angles.eds_count, "Search count", 1, 1, MAX_INEQ_TEST_RES, 1, 0, 0,
        "Count of solutions to search [1..20]\n");
      dlg.inequality.find_angles.but_search = wg_init_text_button(hw, "Search", text_bt_cb);
      dlg.inequality.find_angles.but_search->help_text =
        "Start search angles tool.";
      dlg.inequality.find_angles.lbl_results = wg_init_text(hw, "Found results");
      dlg.inequality.find_angles.cbo_results = wg_init_combo_box(hw, ineq_test_res_list, ineq_test_res_list_size, 0, 4, 2, 130, select_ineq_test_result_cb);
      dlg.inequality.find_angles.chk_auto_select = wg_init_check_box(hw, "Auto select result", check_box_cb);
      dlg.inequality.find_angles.chk_auto_select->help_text =
        "Automatically select best found result in list when search done,\n"
        "and copy a1 a2 b1 b2 angles values in edition fields.";

  // ----------------------------------
  // frame transmit
  dlg.transmit.frm = wg_init_frame_ex(hw, "Transmittance test", FRM_EX);

    dlg.transmit.but_random = wg_init_text_button(hw, "Random", text_bt_cb);
    dlg.transmit.but_random->help_text = "Test a alignment of 'Polarizer count' polarizers, oriented with random angles, and compare\n"
      "with Malus'law theorical results.";

    dlg.transmit.lbl_nb_pol = wg_init_text(hw, "N Pol.");
    dlg.transmit.lbl_nb_pol->help_text =
      "Define max count of aligned polarizer in test.\n"
      "note: Test may reduce this value if remaining photons become too low before maximum is reached.";
    num_desc_init(&dlg.transmit.edn_pol_cnt_nd, 5, 1, 30, 1, 0, 0, dlg.transmit.lbl_nb_pol->help_text);
    dlg.transmit.edn_pol_cnt = wg_init_edit_box_num(hw, &dlg.transmit.edn_pol_cnt_nd, true, NULL);

    dlg.transmit.lbl_max_anv = wg_init_text(hw, "Av.");
    dlg.transmit.lbl_max_anv->help_text =
      "Range [0..360] Angle degree.\n"
      "Define max random angle variation between 2 polarizers. (absolute value)";
    num_desc_init(&dlg.transmit.edn_max_anv_nd, 45, 0, 360, 15, 1, 0, dlg.transmit.lbl_max_anv->help_text);
    dlg.transmit.edn_max_anv = wg_init_edit_box_num(hw, &dlg.transmit.edn_max_anv_nd, true, NULL);

    dlg.transmit.lbl_sim_N = wg_init_text(hw, "N");
    dlg.transmit.lbl_sim_N->help_text =
      "Range [1000 000..10 000 000] Count of initial photons used for test.";
    num_desc_init(&dlg.transmit.edn_sim_N_nd, 2000000, 1000000, 10000000, 1000000, 0, 3, dlg.transmit.lbl_sim_N->help_text);
    dlg.transmit.edn_sim_N = wg_init_edit_box_num(hw, &dlg.transmit.edn_sim_N_nd, true, NULL);

    dlg.transmit.lbl_max_err = wg_init_text(hw, "Err %");
    dlg.transmit.lbl_max_err->help_text =
      "Value percent 0..2%\n"
      "Define max allowed cummulated transmittance error ratio with theorical Malus'law.";
    num_desc_init(&dlg.transmit.edn_max_err_nd, 1.0f, 0.01f, 2.5f, 0.1f, 2, 0, dlg.transmit.lbl_max_err->help_text);
    dlg.transmit.edn_max_err = wg_init_edit_box_num(hw, &dlg.transmit.edn_max_err_nd, true, NULL);

    dlg.transmit.but_list = wg_init_text_button(hw, "List", text_bt_cb);
    dlg.transmit.but_list->help_text = "Test alignment of polarizers using angles defined in list.";
    dlg.transmit.ed_list = wg_init_edit_box(hw, epr_dlg.transmit_test.pol_list_angles_str,
       sizeof(epr_dlg.transmit_test.pol_list_angles_str), NULL);
    dlg.transmit.ed_list->help_text = "Enter list of angles in degrees, separated by commas. ex: 0,45,90";

  // ----------------------------------
  // frame experiment
  bm_init_from_res(&dlg.experiment.exp_logo, &epr_img_res);
  dlg.experiment.frm              = wg_init_frame_ex(hw, "Experiment", FRM_EX);

    dlg.experiment.wg_logo          = wg_init_image(hw, &dlg.experiment.exp_logo);
    dlg.experiment.label_select_exp = wg_init_text(hw, "Select experiment configuration:");
    dlg.experiment.combo_select_exp = wg_init_combo_box(hw, experiment_str_list, experiment_str_list_size, 0, 4, 2, 80, select_experiment_cb);

    dlg.experiment.lbl_status_idle_str = "Run status : idle";
    dlg.experiment.lbl_status       = wg_init_text(hw, dlg.experiment.lbl_status_idle_str);
    dlg.experiment.run_progress     = wg_init_progress_bar(hw, 0, -1);
    wg_progress_bar_set_color(dlg.experiment.run_progress, COL_RGB(125, 160, 220));
    dlg.experiment.but_break        = wg_init_text_button(hw, "Break", text_bt_cb);
    dlg.experiment.but_break->help_text =
      "This button allow to cancel execution of current running tool.";
    wg_init_disable(dlg.experiment.but_break);

  // -----------------------------------
  // frame curves
  dlg.curves.frm              = wg_init_frame_ex(hw, "Curves", FRM_EX);

  dlg.curves.co_oo_ee           = wg_init_check_box(hw, "Co. oo+ee"   , check_box_cb_crv);
  dlg.curves.co_oo_ee->help_text =
    "oo+ee detection correlations. (ratio (oo+ee) / (oo+ee+eo+oe))";
  dlg.curves.co_oe_eo           = wg_init_check_box(hw, "Co. oe+eo"   , check_box_cb_crv);
  dlg.curves.co_oe_eo->help_text =
    "oe+eo detection correlations. (ratio (oe+eo) / (oo+ee+eo+oe))";
  dlg.curves.det_pair_ratio     = wg_init_check_box(hw, "D.pair ratio", check_box_cb_crv);
  dlg.curves.det_pair_ratio->help_text =
    "Ratio of detected pairs on emit request (oo + ee + oe + eo)/N";
  dlg.curves.det_sing_ratio     = wg_init_check_box(hw, "D.sing.ratio", check_box_cb_crv);
  dlg.curves.det_sing_ratio->help_text =
    "Ratio of detected single on emit request (ou + eu + uo + ue)/N";
  dlg.curves.det_uu_ratio       = wg_init_check_box(hw, "UU ratio"    , check_box_cb_crv);
  dlg.curves.det_uu_ratio->help_text =
    "Ratio of mesures that produce no detection on both Alice and Bob (uu/N)";
  dlg.curves.det_acc_ratio      = wg_init_check_box(hw, "Acc ratio"   , check_box_cb_crv);
  dlg.curves.det_acc_ratio->help_text =
    "Ratio of mesures containing more than one detection in pairing window.\n"
    "This can be produced by background noise detection, or source multi emissions.";
  dlg.curves.transm_alice_o     = wg_init_check_box(hw, "Tr. Alice o" , check_box_cb_crv);
  dlg.curves.transm_alice_o->help_text =
    "Ratio of detected photon in o ouput.  (N(o) / N(o+e))\n"
    "With identical detection sensitivity on e and o, the ratio must be 50%.\n"
    "Ratio of detected photons on e ouput is the complement. (1 - r)\n"
    "Ratios for Bob are identicals (not displayed).";
  dlg.curves.tr_rel_alice_o     = wg_init_check_box(hw, "Tr.rel Alice o", check_box_cb_crv);
  dlg.curves.tr_rel_alice_o->help_text =
    "Ratio of photon emitted to Alice passing to polarizer o ouput, displayed relatively to\n"
    "Alice photon emitted polarisation and Alice polarizer angle. (Then x axis on curve is\n"
    "angle difference between photon polarisation and polarizer angle).\n"
    "In this case the Malus'law must be observed.\n"
    "The ratio on e is the complement. (1 - r)\n"
    "This curve cannot be displayed in QM mode because no emission polarization is defined.";
  dlg.curves.ref_sin2           = wg_init_check_box(hw, "Ref. sin²"   , check_box_cb_crv);
  dlg.curves.ref_sin2->help_text =
    "Draw a reference perfect sin² curve. Used to visually compare with simulated curves.";
  dlg.curves.ref_cos2           = wg_init_check_box(hw, "Ref. cos²"   , check_box_cb_crv);
  dlg.curves.ref_cos2->help_text =
    "Draw a reference perfect cos² curve. Used to visually compare with simulated curves.";
  dlg.curves.ref_triangle       = wg_init_check_box(hw, "Ref. tri."   , check_box_cb_crv);
  dlg.curves.ref_triangle->help_text =
    "Draw a realistic correlations triangle.";
  dlg.curves.src_emit_t_dist    = wg_init_check_box(hw, "Src jitter"  , check_box_cb_crv);
  dlg.curves.src_emit_t_dist->help_text =
    "Draw source time jitter delay distribution. (Then x axis represents a time value).";
  dlg.curves.frm_drawing        = wg_init_frame(hw, "Drawing");

  eds_init(&dlg.curves.eds_graph_N, "N graph", 5000, 5000, 50000, 5000, 0, 3,
    "Define count of emissions used to define each pixels of curves.");
  wg_set_text_align(dlg.curves.eds_graph_N.lbl, wg_ta_right);

  dlg.curves.chk_dark_col       = wg_init_check_box(hw, "Dark color", check_box_cb_crv);
  dlg.curves.chk_dark_col->help_text =
    "Use dark background color for the graphics.";
  dlg.curves.chk_no_pix_aa       = wg_init_check_box(hw, "Disable aa.", check_box_cb_crv);
  dlg.curves.chk_no_pix_aa->help_text =
    "Disable pixel anti-aliasing. This reduce contrast.";

  dlg.curves.bt_update          = wg_init_text_button(hw, "Update", text_bt_cb);
  dlg.curves.bt_update->help_text =
    "Update graph datas with experiment settings and refresh curves drawing.";

  // -----------------------------------
  // frame graph
  dlg.graph.frm               = wg_init_frame_ex(hw, "Graph", FRM_EX);
  dlg.graph.wg_graph          = wg_init_graph(hw, graph_post_draw_cb);

  // -----------------------------------
  // frame messages console
  dlg.console.frm             = wg_init_frame_ex(hw, "Messages", FRM_EX);

  // change console default style
  wgs.co_out.text_bk_color = COL_RGB(219, 217, 207);
  wgs.co_out.text_color    = COL_RGB(0, 0, 0);
  // wgs.co_out.text_aa_color = COL_RGB(0, 0, 0);
  wgs.co_out.text_aa_color = font_get_aa_color(wgs.co_out.text_color, wgs.co_out.text_bk_color, 12);

  dlg.console.console_out       = wg_init_console_out(hw, 32000);
  dlg.console.bt_clear          = wg_init_text_button(hw, "Clear", text_bt_cb);
  dlg.console.bt_clear->help_text =
    "Clear console content.";
  dlg.console.bt_copy           = wg_init_text_button(hw, "Copy", text_bt_cb);
  dlg.console.bt_copy->help_text =
    "Copy console content to clipboard.";
  dlg.console.bt_help           = wg_init_text_button(hw, "Help", text_bt_cb);
  dlg.console.bt_help->help_text =
    "Display help informations.";
  dlg.console.bt_exit           = wg_init_text_button(hw, "Exit", text_bt_cb);
  dlg.console.bt_exit->help_text =
    "Exit program.";
}

// -----------------------------------------------------------
// define position of widgets

#define DX_EDS_R1 65         // x size for edit num 1.0 range + 2 decimals

// contain all adjustable sizes for widget/frame placements
struct sz_adj_t
{
  // y size of some widgets
  struct
  {
    int edl;                 // edit box y size
    int eds;                 // edit box + spin
    int but;                 // text button
    int cbo;                 // combo box
  } wg_dy;

  // frame margin for placements
  struct
  {
    int dx_spc;              // x space between frames
    int dx_push;             // dx after wl_push_frm
    int dy_push;             // dy after wl_push_frm
    int dy_pop_frm;          // dy added before and after wl_pop_frm
    int dy_place;            // dy added after wl_place
    int dy_transm;           // transmittance bottom block
  } frm_margin;

  // window margin
  struct
  {
    int y_up;                // up margin
    int y_bottom;            // bottom margin
    int x_lr;                // left/right margin
  } win_margin;

  // frame blocks for min y size
  struct
  {
    int ineq;
    int det;
    int sim;
  } col_wx;
};

static struct sz_adj_t sz = { 0 };

// place edit spin box + label
static void eds_place(eds_t *eds, int w1)
{
  wl_place_2h_r(eds->lbl, eds->ed, w1, sz.wg_dy.eds);
}

// place edit spin box + label, center first
static void eds_place_c0(eds_t *eds, int w1)
{
  wl_place_2h_r_c0(eds->lbl, eds->ed, w1, sz.wg_dy.eds);
}

static void place_widget_block_simulation(void)
{
  wl_push_frm(dlg.simulation.frm);
    // frame model
    wl_push_frm(dlg.simulation.model.frm);
      wl_place(dlg.simulation.model.rad_qm, 0);
      wl_place(dlg.simulation.model.rad_local, 0);
    wl_pop_frm();
    // frame RNG
    wl_push_frm(dlg.simulation.rng.frm);
      wl_place(dlg.simulation.rng.rad_c_lib, 0);
      wl_place(dlg.simulation.rng.rad_ms_c_lib, 0);
      wl_place(dlg.simulation.rng.rad_xorshift, 0);
      wl_place(dlg.simulation.rng.rad_cpu_buildin, 0);
      wl_place_3h_center(dlg.simulation.rng.bt_test_col,
                         dlg.simulation.rng.bt_test_grey,
                         dlg.simulation.rng.bt_test_bw, sz.frm_margin.dx_push, sz.wg_dy.but);
      wl_place_center(dlg.simulation.rng.label_select_seed, 0);
      wl_place(dlg.simulation.rng.edn_seed, sz.wg_dy.edl);
      wl_place(dlg.simulation.rng.bt_apply_seed, sz.wg_dy.but);
    wl_pop_frm();
  wl_pop_frm();
}

static void place_widget_block_source(void)
{
  wl_push_frm(dlg.source.frm);
    eds_place(&dlg.source.eds_emit_pol_diff, 64);
    // frame entang
    wl_push_frm(dlg.source.entang.frm);
      eds_place(&dlg.source.entang.eds_qm_r, DX_EDS_R1);
      eds_place(&dlg.source.entang.eds_local_s, DX_EDS_R1);
      wl_place_center(dlg.source.entang.chk_local_polarize, 0);
    wl_pop_frm();
    // emit_prob
    wl_push_frm(dlg.source.emit_prob.frm);
      eds_place(&dlg.source.emit_prob.eds_ent_pair, DX_EDS_R1);
      eds_place(&dlg.source.emit_prob.eds_nent_pair, DX_EDS_R1);
      eds_place(&dlg.source.emit_prob.eds_single, DX_EDS_R1);
      eds_place(&dlg.source.emit_prob.eds_2nd, DX_EDS_R1);
      wl_place_center(dlg.source.emit_prob.lbl_no_emit, 0);
    wl_pop_frm();
    // emit_prob
    wl_push_frm(dlg.source.emit_delay.frm);
      wl_place_center(dlg.source.emit_delay.chk_enable_jitter, 0);
      eds_place(&dlg.source.emit_delay.eds_beta_scale, DX_EDS_R1);
    wl_pop_frm();
  wl_pop_frm();
}

static void place_widget_block_polarizer(void)
{
  wl_push_frm(dlg.polarizer.frm);
    eds_place(&dlg.polarizer.eds_e_out_delay, DX_EDS_R1);
    eds_place(&dlg.polarizer.eds_o_out_delay, DX_EDS_R1);
    eds_place(&dlg.polarizer.eds_st1_delay, DX_EDS_R1);
  wl_pop_frm();
}

static void place_widget_block_detectors(void)
{
  wl_push_frm(dlg.detectors.frm);
    // frame detect probabilities
    wl_push_frm(dlg.detectors.detect_prob.frm);
      // frame alice
      wl_push_frm(dlg.detectors.detect_prob.alice.frm);
        eds_place(&dlg.detectors.detect_prob.alice.eds_det_o, DX_EDS_R1);
        eds_place(&dlg.detectors.detect_prob.alice.eds_det_e, DX_EDS_R1);
      wl_pop_frm();
      // frame bob
      wl_push_frm(dlg.detectors.detect_prob.bob.frm);
        wl_place_center(dlg.detectors.detect_prob.bob.chk_same_alice, 0);
        eds_place(&dlg.detectors.detect_prob.bob.eds_det_o, DX_EDS_R1);
        eds_place(&dlg.detectors.detect_prob.bob.eds_det_e, DX_EDS_R1);
      wl_pop_frm();
      // frame undetect dep.
      wl_push_frm(dlg.detectors.detect_prob.u_detect_dep.frm);
        eds_place(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr, DX_EDS_R1);
        eds_place(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr, DX_EDS_R1);
        wl_place_center(dlg.detectors.detect_prob.u_detect_dep.lbl_udet_rand, 0);
      wl_pop_frm();
    wl_pop_frm();
    // frame misc
    wl_push_frm(dlg.detectors.misc.frm);
      eds_place(&dlg.detectors.misc.eds_dnoise_ratio, DX_EDS_R1);
    wl_pop_frm();
  wl_pop_frm();
}

static void place_widget_block_pairing(void)
{
  // frame pairing
  wl_push_frm(dlg.pairing.frm);
    wl_push_frm(dlg.pairing.win_type.frm);
      wl_place(dlg.pairing.win_type.rad_fixed, 0);
      wl_place(dlg.pairing.win_type.rad_mobile, 0);
    wl_pop_frm();
    eds_place_c0(&dlg.pairing.eds_win_width, DX_EDS_R1);
    wl_place(dlg.pairing.chk_ignore_dual, 0);
  wl_pop_frm();
}

static void place_widget_block_inequality(void)
{
  wl_push_frm(dlg.inequality.frm);
    // frame test
    wl_push_frm(dlg.inequality.test.frm);
      wl_push_frm(dlg.inequality.test.angles.frm);
        eds_place_c0(&dlg.inequality.test.angles.eds_a1, 75);
        eds_place_c0(&dlg.inequality.test.angles.eds_a2, 75);
        eds_place_c0(&dlg.inequality.test.angles.eds_b1, 75);
        eds_place_c0(&dlg.inequality.test.angles.eds_b2, 75);
      wl_pop_frm();
      eds_place_c0(&dlg.inequality.test.eds_N, 95);
      wl_place_2h_spc_r(dlg.inequality.test.but_test, 6, dlg.inequality.test.but_optimize, 35, sz.wg_dy.but);
      wl_place_center(dlg.inequality.test.chk_auto_seed, 0);

      // eval positivity
      wl_push_frm(dlg.inequality.test.eval_posi.frm);
        eds_place_c0(&dlg.inequality.test.eval_posi.eds_count, 60);
        wl_place(dlg.inequality.test.eval_posi.but_eval, sz.wg_dy.but);
      wl_pop_frm();
    wl_pop_frm();

    // find angles
    wl_push_frm(dlg.inequality.find_angles.frm);
      eds_place_c0(&dlg.inequality.find_angles.eds_count, 55);
      wl_place(dlg.inequality.find_angles.but_search, sz.wg_dy.but);
      wl_place_center(dlg.inequality.find_angles.lbl_results, 0);
      wl_place(dlg.inequality.find_angles.cbo_results, sz.wg_dy.cbo);
      wl_place_center(dlg.inequality.find_angles.chk_auto_select, 0);
    wl_pop_frm();
  wl_pop_frm();
}

static void place_widget_block_transmit(void)
{
  wl_push_frm(dlg.transmit.frm);
  {
    int dx_spc = sz.frm_margin.dx_spc;
    wl_place_h_first(dlg.transmit.but_random, 60, sz.wg_dy.eds);
    wl_place_h_next(dlg.transmit.lbl_nb_pol, dx_spc, 0, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.edn_pol_cnt, 5, 50, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.lbl_max_anv, dx_spc, 0, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.edn_max_anv, 5, 60, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.lbl_sim_N, dx_spc, 0, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.edn_sim_N, 5, 88, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.lbl_max_err, dx_spc, 0, sz.wg_dy.eds, false);
    wl_place_h_next(dlg.transmit.edn_max_err, 5, 55, sz.wg_dy.eds, true);

    wl_place_h_first(dlg.transmit.but_list, 60, sz.wg_dy.edl);
    wl_place_h_next(dlg.transmit.ed_list, 8, 0, sz.wg_dy.edl, true);
  }
  wl_pop_frm();
}

static void place_widget_block_experiment(void)
{
  wl_push_frm(dlg.experiment.frm);
    wl_place(dlg.experiment.wg_logo, 0);
    wl_place_center(dlg.experiment.label_select_exp, 0);
    wl_place(dlg.experiment.combo_select_exp, sz.wg_dy.cbo);
    wl_place_center(dlg.experiment.lbl_status, 0);
    wl_place_2h_spc_r(dlg.experiment.run_progress, 10, dlg.experiment.but_break, 60, sz.wg_dy.but);
  wl_pop_frm();
}

#define CRV_COL_RECT 8                           // size of colored rectangle showing graph color near button
#define CRV_COL_W 12

static void place_widget_block_curves(void)
{
  wl_push_frm(dlg.curves.frm);
    wl_pos()->x += CRV_COL_W;
    *wl_wx() -= CRV_COL_W;
    wl_place_3h_center(dlg.curves.co_oo_ee, dlg.curves.det_pair_ratio, dlg.curves.det_uu_ratio, 10, 0);
    wl_place_3h_center(dlg.curves.co_oe_eo, dlg.curves.det_sing_ratio, dlg.curves.det_acc_ratio, 10, 0);
    wl_place_3h_center(dlg.curves.transm_alice_o, dlg.curves.tr_rel_alice_o, dlg.curves.src_emit_t_dist, 10, 0);
    wl_place_3h_center(dlg.curves.ref_sin2, dlg.curves.ref_cos2, dlg.curves.ref_triangle, 10, 0);
    wl_pos()->x -= CRV_COL_W;
    *wl_wx() += CRV_COL_W;

    wl_push_frm(dlg.curves.frm_drawing);
      wl_place_2h(dlg.curves.chk_dark_col, 80, dlg.curves.chk_no_pix_aa, 0);
      wl_pos()->y += 2;
      wl_place_h_first(dlg.curves.eds_graph_N.lbl, 0, sz.wg_dy.eds);
      wl_place_h_next(dlg.curves.eds_graph_N.ed, 8, 80, sz.wg_dy.eds, false);
      wl_place_h_next(dlg.curves.bt_update, 25, 120, sz.wg_dy.eds, true);
    wl_pop_frm();

  wl_pop_frm();
}

// ----------------------------------------------

// frame block list, used for block placement into window
struct dlg_blk_size_t
{
  vec2i simulation;
  vec2i source;
  vec2i polarizer;
  vec2i detectors;
  vec2i pairing;
  vec2i inequality;
  vec2i transmit;
  vec2i experiment;
  vec2i curves;
};

// get size of blocks
static void get_block_sizes(struct dlg_blk_size_t *blk_size)
{
  vec2i pos = { 0, 0 };
  int dxl = dlg.experiment.exp_logo.size.x + 2*sz.frm_margin.dx_push;  // logo size + margin

  wl_init(&pos, sz.col_wx.sim, true);  place_widget_block_simulation(); wl_get_size(&blk_size->simulation);
  wl_init(&pos, sz.col_wx.sim, true);  place_widget_block_source();     wl_get_size(&blk_size->source);

  wl_init(&pos, sz.col_wx.det, true);  place_widget_block_polarizer();  wl_get_size(&blk_size->polarizer);
  wl_init(&pos, sz.col_wx.det, true);  place_widget_block_detectors();  wl_get_size(&blk_size->detectors);

  wl_init(&pos, sz.col_wx.ineq, true);  place_widget_block_pairing(); wl_get_size(&blk_size->pairing);
  wl_init(&pos, sz.col_wx.ineq, true);  place_widget_block_inequality(); wl_get_size(&blk_size->inequality);

  wl_init(&pos, dxl, true);  place_widget_block_transmit();   wl_get_size(&blk_size->transmit);
  wl_init(&pos, dxl, true);  place_widget_block_experiment(); wl_get_size(&blk_size->experiment);
  wl_init(&pos, dxl, true);  place_widget_block_curves();     wl_get_size(&blk_size->curves);
}

static void place_widget_block_graph(vec2i *pos, vec2i *size)
{
  wg_set_pos_size(dlg.graph.frm, pos->x, pos->y, size->x, size->y);
  pos->x += sz.frm_margin.dx_push;
  pos->y += sz.frm_margin.dy_push;
  size->x -= 2*sz.frm_margin.dx_push;
  size->y -= sz.frm_margin.dy_push + 2*sz.frm_margin.dy_pop_frm;
  wg_set_pos_size(dlg.graph.wg_graph, pos->x, pos->y, size->x, size->y);
}

static void place_widget_block_console(vec2i *pos, vec2i *size)
{
  #define CONS_BT_SIZE 40
  #define CONS_BT_MARGIN 6
  wg_set_pos_size(dlg.console.frm, pos->x, pos->y, size->x, size->y);

  pos->x += sz.frm_margin.dx_push;
  pos->y += sz.frm_margin.dy_push;
  size->x -= sz.frm_margin.dx_push + CONS_BT_SIZE + CONS_BT_MARGIN*2 + 2;
  size->y -= sz.frm_margin.dy_push + sz.frm_margin.dy_pop_frm + 2;
  wg_set_pos_size(dlg.console.console_out, pos->x, pos->y, size->x, size->y);

  pos->x += size->x + CONS_BT_MARGIN;
  wg_set_pos_size(dlg.console.bt_clear, pos->x, pos->y, CONS_BT_SIZE, sz.wg_dy.but);
  pos->y += sz.wg_dy.but + 6;
  wg_set_pos_size(dlg.console.bt_copy,  pos->x, pos->y, CONS_BT_SIZE, sz.wg_dy.but);
  pos->y += sz.wg_dy.but + 20;
  wg_set_pos_size(dlg.console.bt_help,  pos->x, pos->y, CONS_BT_SIZE, sz.wg_dy.but);
  pos->y += sz.wg_dy.but + 6;
  wg_set_pos_size(dlg.console.bt_exit,  pos->x, pos->y, CONS_BT_SIZE, sz.wg_dy.but);
}

// place widget blocks into window bitmap
static void win_place_blocks(struct dlg_blk_size_t *blk_size, bitmap_t *bm)
{
  vec2i pos, gr_pos, gr_size, cons_pos, cons_size;
  int y_tr;

  // ------------
  pos.x = bm->size.x - (blk_size->pairing.x + sz.win_margin.x_lr);
  pos.y = sz.win_margin.y_up;
  wl_init(&pos, blk_size->pairing.x, false);  place_widget_block_pairing();

  y_tr = bm->size.y - sz.win_margin.y_bottom - (blk_size->transmit.y - sz.frm_margin.dy_pop_frm);

  pos.y = y_tr - sz.frm_margin.dy_transm - blk_size->inequality.y;
  wl_init(&pos, blk_size->inequality.x, false);  place_widget_block_inequality();

  // ------------
  pos.x -= (blk_size->detectors.x + sz.frm_margin.dx_spc);
  pos.y = sz.win_margin.y_up;

  wl_init(&pos, blk_size->polarizer.x, false);  place_widget_block_polarizer();
  pos.y = y_tr - sz.frm_margin.dy_transm - blk_size->detectors.y;
  wl_init(&pos, blk_size->detectors.x, false);  place_widget_block_detectors();

  // ------------
  pos.x -= (blk_size->simulation.x + sz.frm_margin.dx_spc);
  pos.y = sz.win_margin.y_up;
  wl_init(&pos, blk_size->simulation.x, false);  place_widget_block_simulation();

  pos.y = y_tr - sz.frm_margin.dy_transm - blk_size->source.y;
  wl_init(&pos, blk_size->source.x, false);  place_widget_block_source();

  cons_size.x = pos.x;                           // set console x max
  pos.y = y_tr;
  wl_init(&pos, bm->size.x - pos.x - sz.win_margin.x_lr , false);  place_widget_block_transmit();

  // ------------
  pos.x -= (blk_size->experiment.x + sz.frm_margin.dx_spc);
  pos.y = sz.win_margin.y_up;
  wl_init(&pos, blk_size->experiment.x, false);  place_widget_block_experiment();

  // ------------
  pos.y += blk_size->experiment.y;
  wl_init(&pos, blk_size->curves.x, false);  place_widget_block_curves();

  pos.y += (blk_size->curves.y - sz.frm_margin.dy_pop_frm);

  // place graph with remaining size
  gr_pos.x = sz.win_margin.x_lr;
  gr_pos.y = sz.win_margin.y_up;
  gr_size.x = pos.x - sz.win_margin.x_lr - sz.frm_margin.dx_spc;
  gr_size.y = pos.y - sz.win_margin.y_up;

  // place console with remaining size
  cons_pos.x = sz.win_margin.x_lr;
  cons_pos.y = pos.y + sz.frm_margin.dy_pop_frm;
  cons_size.x -= (sz.win_margin.x_lr + sz.frm_margin.dx_spc);
  cons_size.y = bm->size.y - cons_pos.y - sz.win_margin.y_bottom;

  place_widget_block_graph(&gr_pos, &gr_size);
  place_widget_block_console(&cons_pos, &cons_size);
}

// ref
// static const struct sz_adj_t sz_adj_ref = { { 22, 22, 22, 22 }, { 8, 8, 16, 4, 4, 20 }, { 10, 8, 8 }, { 170, 164, 160 } };
// y cli 640
static const struct sz_adj_t sz_adj_min = { { 19, 19, 18, 18 }, { 4, 4, 13, 3, 3,  0 }, { 8, 6, 5 }, { 150, 140, 142 } };
// y cli 865
static const struct sz_adj_t sz_adj_max = { { 23, 23, 23, 23 }, { 10, 8, 18, 6, 6, 20 }, { 12, 8, 8 }, { 170, 164, 160 } };

static int id_dec_margin_y(int id)
{
  switch (id)
  {
    #define SW_DEC(n,s,mbr) case n: if (sz.s.mbr > sz_adj_min.s.mbr) { sz.s.mbr--; return n; }

    SW_DEC(0, frm_margin, dy_transm)
    SW_DEC(1, frm_margin, dy_pop_frm)
    SW_DEC(2, frm_margin, dy_place)
    SW_DEC(3, frm_margin, dy_push)

    SW_DEC(4, wg_dy, but)
    SW_DEC(5, wg_dy, cbo)
    SW_DEC(6, wg_dy, edl)
    SW_DEC(7, wg_dy, eds)

    SW_DEC(8, win_margin, y_up)
    SW_DEC(9, win_margin, y_bottom)
  };
  return -1;
}

// min/max dialog client window size for correct display
static const struct
{
  vec2i min;
  vec2i max;
} dlg_size = { { 1160, 640 }, { 1450, 870 } };

// adjust x sizes of frames, margin
static void adjust_sizes_x(int cli_x)
{
  int dx = cli_x - dlg_size.min.x;
  int r = dlg_size.max.x - dlg_size.min.x;
  #define LI(s) sz.s = sz_adj_min.s + (dx*(sz_adj_max.s - sz_adj_min.s))/r

  LI(col_wx.ineq);
  LI(col_wx.det);
  LI(col_wx.sim);
  LI(frm_margin.dx_push);
  LI(frm_margin.dx_spc);
  LI(win_margin.x_lr);
}

// adjust y sizes of frames, margin
static void adjust_sizes_y(int cli_y)
{
  int id = 0;
  while (id >= 0)
  {
    int wy;
    vec2i pos = { 0, 0 };
    vec2i sz0, sz1, sz_transmit;

    // define move offsets for wl_push_frm and wl_place
    wl_set_push_frm(sz.frm_margin.dx_push, sz.frm_margin.dy_push, sz.frm_margin.dy_pop_frm);
    wl_place_set_y_margin(sz.frm_margin.dy_place);

    // uses the blocks column requiring highest y size
#if 0
    wl_init(&pos, 300, true);  place_widget_block_pairing(); wl_get_size(&sz0);
    wl_init(&pos, 300, true);  place_widget_block_inequality(); wl_get_size(&sz1);
#else
    wl_init(&pos, 300, true);  place_widget_block_simulation(); wl_get_size(&sz0);
    wl_init(&pos, 300, true);  place_widget_block_source(); wl_get_size(&sz1);
#endif
    wl_init(&pos, 300, true);  place_widget_block_transmit();   wl_get_size(&sz_transmit);

    // get wy
    wy = sz0.y + sz1.y + sz_transmit.y + sz.frm_margin.dy_transm + sz.win_margin.y_up + sz.win_margin.y_bottom;
    if (wy <= cli_y)
      break;
    id = id_dec_margin_y(id);
    if (id < 0)
      id = id_dec_margin_y(0);
  }
}

// adjust sizes to client bitmap size, and place all objects
static void resize_dialog(bitmap_t *cli_bm)
{
  struct dlg_blk_size_t blk_size;
  sz = sz_adj_max;                               // start to max for y adjust
  adjust_sizes_x(cli_bm->size.x);                // note: must be done before adjust_sizes_y
  adjust_sizes_y(cli_bm->size.y);
  get_block_sizes(&blk_size);
  win_place_blocks(&blk_size, cli_bm);
}

void dlg_configure_graph_base(void);
static void curves_ckbox_draw_color(bitmap_t *bm, bool blit);

// window message proc
static void win_msg_proc(hwin_t *hwin)
{
  switch (hwin->ev.type)
  {
    // mouse messages
    case EV_RBUTTONDOWN:
      // open_ctx_menu(hw->ev.mouse_pos.x, hw->ev.mouse_pos.y);
    break;
    case EV_CREATE:                              // window creation
      hw = hwin;                                 // set global ptr
      init_widget_list();                        // init list of widgets
      dlg_configure_graph_base();
    break;
    case EV_CLOSE:
      epr_task.cancel = true;                    // kill thread if running
      win_wait_thread_end();
    break;
    case EV_DESTROY:                             // window destroyed
      W_FREE(dlg.experiment.exp_logo.pix_ptr);   // free image
    break;
    case EV_SIZE:                                // window sized/resized
      if (hw->show_mode != show_minimized)       // then client y size if 0
        resize_dialog(&hw->cli_bm);
    break;
    case EV_PAINT:                               // paint required. bitmap is valid in this message
      bm_paint(&hw->cli_bm, wgs.clear_bk_color); // clear widget area background
      curves_ckbox_draw_color(&hw->cli_bm, false); // checkbox curves colors
    break;
    default:;
  }
  widget_dispatch_events(hw);                    // send events to widgets
}

// ----------------------------------------------
// call backs

// --------------------
// radio buttons

static void update_widget_enable(void);
static void update_radio_buttons(void)
{
  wg_set_state(hw, dlg.simulation.model.rad_qm        , epr_dlg.sim.model_qm);
  wg_set_state(hw, dlg.simulation.model.rad_local     , epr_dlg.sim.model_local);

  wg_set_state(hw, dlg.simulation.rng.rad_c_lib       , epr_dlg.sim.rng_type == e_rng_c_lib);
  wg_set_state(hw, dlg.simulation.rng.rad_ms_c_lib    , epr_dlg.sim.rng_type == e_rng_ms_c_lib);
  wg_set_state(hw, dlg.simulation.rng.rad_xorshift    , epr_dlg.sim.rng_type == e_rng_xorshift);
  wg_set_state(hw, dlg.simulation.rng.rad_cpu_buildin , epr_dlg.sim.rng_type == e_rng_cpu);

  wg_set_state(hw, dlg.pairing.win_type.rad_fixed     , epr_dlg.pairing.win_type_fixed);
  wg_set_state(hw, dlg.pairing.win_type.rad_mobile    , epr_dlg.pairing.win_type_mobile);

  update_widget_enable();
}

static void rng_srand(void)
{
  epr_dlg.sim.rng_seed = wg_edit_box_num_get_value(dlg.simulation.rng.edn_seed);
  rng.srand((int)epr_dlg.sim.rng_seed);
  co_printf("RNG: set seed = %.0f\n", epr_dlg.sim.rng_seed);
}

static void select_rng_type(enum e_rng_type rng_type)
{
  const char *rng_names[4] = { "C lib", "ms C lib", "xorshift", "CPU build-in" };
  co_printf("RNG: %s selected\n", rng_names[rng_type]);
  epr_dlg.sim.rng_type = rng_type;
  rng_select_type(rng_type);
  rng_srand();
}

static void update_check_boxes(void);
static void set_undetect_random_origin_ratio(float dep_rand);
static void update_undetect_random_origin_ratio(widget_t *wg);

static void radio_button_cb(hwin_t *hwin, widget_t *wg)
{
  if (wg == dlg.simulation.model.rad_qm)
  {
    epr_dlg.sim.model_qm     = true;
    epr_dlg.sim.model_local  = false;
    epr_dlg.pairing.win_type_fixed = true;
    epr_dlg.pairing.win_type_mobile = false;
    set_undetect_random_origin_ratio(1.0f);
    epr_dlg.curves.tr_rel_alice_o = false;
    epr_dlg.source.emit_enable_jitter = false;
    epr_dlg.curves.src_emit_t_dist = false;
  }
  else
  if (wg == dlg.simulation.model.rad_local)
  {
    epr_dlg.sim.model_qm     = false;
    epr_dlg.sim.model_local  = true;
    update_undetect_random_origin_ratio(dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr.ed);
  }
  else
  if (wg == dlg.simulation.rng.rad_c_lib)
    select_rng_type(e_rng_c_lib);
  else
  if (wg == dlg.simulation.rng.rad_ms_c_lib)
    select_rng_type(e_rng_ms_c_lib);
  else
  if (wg == dlg.simulation.rng.rad_xorshift)
    select_rng_type(e_rng_xorshift);
  else
  if (wg == dlg.simulation.rng.rad_cpu_buildin)
    select_rng_type(e_rng_cpu);
  else
  if (wg == dlg.pairing.win_type.rad_fixed)
  {
    epr_dlg.pairing.win_type_fixed  = true;
    epr_dlg.pairing.win_type_mobile = false;
  }
  else
  if (wg == dlg.pairing.win_type.rad_mobile)
  {
    epr_dlg.pairing.win_type_fixed  = false;
    epr_dlg.pairing.win_type_mobile = true;
    epr_dlg.pairing.ignore_accidentals = false;
  }
  else
    W_ASSERT(0);
  update_radio_buttons();
  update_check_boxes();
}

// --------------------
// check box

static void update_check_boxes(void)
{
  wg_set_state(hw, dlg.source.entang.chk_local_polarize , epr_dlg.source.local_polarize);
  wg_set_state(hw, dlg.source.emit_delay.chk_enable_jitter, epr_dlg.source.emit_enable_jitter);
  wg_set_state(hw, dlg.detectors.detect_prob.bob.chk_same_alice,  epr_dlg.detectors.det_prob_bob_same_alice);
  wg_set_state(hw, dlg.pairing.chk_ignore_dual    ,  epr_dlg.pairing.ignore_accidentals);
  wg_set_state(hw, dlg.inequality.find_angles.chk_auto_select, epr_dlg.inequality.auto_select_res);
  wg_set_state(hw, dlg.curves.co_oo_ee            ,  epr_dlg.curves.corr_oo_ee);
  wg_set_state(hw, dlg.curves.co_oe_eo            ,  epr_dlg.curves.corr_oe_eo);
  wg_set_state(hw, dlg.curves.det_pair_ratio      ,  epr_dlg.curves.det_pair_ratio);
  wg_set_state(hw, dlg.curves.det_sing_ratio      ,  epr_dlg.curves.det_sing_ratio);
  wg_set_state(hw, dlg.curves.det_uu_ratio        ,  epr_dlg.curves.det_uu_ratio);
  wg_set_state(hw, dlg.curves.det_acc_ratio       ,  epr_dlg.curves.det_acc_ratio);
  wg_set_state(hw, dlg.curves.transm_alice_o      ,  epr_dlg.curves.transm_alice_o);
  wg_set_state(hw, dlg.curves.tr_rel_alice_o      ,  epr_dlg.curves.tr_rel_alice_o);
  wg_set_state(hw, dlg.curves.ref_sin2            ,  epr_dlg.curves.ref_sin2);
  wg_set_state(hw, dlg.curves.ref_cos2            ,  epr_dlg.curves.ref_cos2);
  wg_set_state(hw, dlg.curves.ref_triangle        ,  epr_dlg.curves.ref_triangle);
  wg_set_state(hw, dlg.curves.src_emit_t_dist     ,  epr_dlg.curves.src_emit_t_dist);
}

void check_box_cb(hwin_t *hwin, widget_t *wg)
{
  epr_dlg.source.local_polarize             = wg_get_state(dlg.source.entang.chk_local_polarize);
  epr_dlg.source.emit_enable_jitter         = wg_get_state(dlg.source.emit_delay.chk_enable_jitter);
  epr_dlg.detectors.det_prob_bob_same_alice = wg_get_state(dlg.detectors.detect_prob.bob.chk_same_alice);
  epr_dlg.pairing.ignore_accidentals        = wg_get_state(dlg.pairing.chk_ignore_dual);

  if (wg == dlg.source.emit_delay.chk_enable_jitter)
  {
    epr_dlg.curves.src_emit_t_dist &= epr_dlg.source.emit_enable_jitter;
    wg_set_state(hw, dlg.curves.src_emit_t_dist, epr_dlg.curves.src_emit_t_dist);
  }
  update_widget_enable();
}

// --------------------
// numeric edit box

static void eds_set_value(eds_t *eds, float value)
{
  wg_edit_box_num_set_value(hw, eds->ed, value);
}

static void update_edn_values(void)
{
  eds_set_value(&dlg.curves.eds_graph_N                            , epr_dlg.curves.graph_N);
  eds_set_value(&dlg.source.eds_emit_pol_diff                      , epr_dlg.source.emit_pol_diff);
  eds_set_value(&dlg.source.entang.eds_qm_r                        , epr_dlg.source.ent_qm_r);
  eds_set_value(&dlg.source.entang.eds_local_s                     , epr_dlg.source.ent_local_s);
  eds_set_value(&dlg.source.emit_prob.eds_ent_pair                 , epr_dlg.source.em_prob_ent_pair);
  eds_set_value(&dlg.source.emit_prob.eds_nent_pair                , epr_dlg.source.em_prob_nent_pair);
  eds_set_value(&dlg.source.emit_prob.eds_single                   , epr_dlg.source.em_prob_single);
  eds_set_value(&dlg.source.emit_prob.eds_2nd                      , epr_dlg.source.em_prob_double);
  eds_set_value(&dlg.source.emit_delay.eds_beta_scale              , epr_dlg.source.emit_time_beta_scale);
  eds_set_value(&dlg.polarizer.eds_e_out_delay                     , epr_dlg.polarizer.e_out_delay);
  eds_set_value(&dlg.polarizer.eds_o_out_delay                     , epr_dlg.polarizer.o_out_delay);
  eds_set_value(&dlg.polarizer.eds_st1_delay                       , epr_dlg.polarizer.st1_delay);
  eds_set_value(&dlg.detectors.detect_prob.alice.eds_det_o         , epr_dlg.detectors.det_prob_alice_det_o);
  eds_set_value(&dlg.detectors.detect_prob.alice.eds_det_e         , epr_dlg.detectors.det_prob_alice_det_e);
  eds_set_value(&dlg.detectors.detect_prob.bob.eds_det_o           , epr_dlg.detectors.det_prob_bob_det_o);
  eds_set_value(&dlg.detectors.detect_prob.bob.eds_det_e           , epr_dlg.detectors.det_prob_bob_det_e);
  eds_set_value(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr, epr_dlg.detectors.udet_dep_local_dpr);
  eds_set_value(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr , epr_dlg.detectors.udet_dep_local_q);
  eds_set_value(&dlg.detectors.misc.eds_dnoise_ratio               , epr_dlg.detectors.misc_noise_det_r);
  eds_set_value(&dlg.pairing.eds_win_width                         , epr_dlg.pairing.win_width);
  eds_set_value(&dlg.inequality.test.angles.eds_a1                 , epr_dlg.inequality.test_conf_a1);
  eds_set_value(&dlg.inequality.test.angles.eds_a2                 , epr_dlg.inequality.test_conf_a2);
  eds_set_value(&dlg.inequality.test.angles.eds_b1                 , epr_dlg.inequality.test_conf_b1);
  eds_set_value(&dlg.inequality.test.angles.eds_b2                 , epr_dlg.inequality.test_conf_b2);
  eds_set_value(&dlg.inequality.test.eds_N                         , epr_dlg.inequality.test_conf_N);
  eds_set_value(&dlg.inequality.find_angles.eds_count              , epr_dlg.inequality.angles_max_search);
  eds_set_value(&dlg.inequality.test.eval_posi.eds_count           , epr_dlg.inequality.eval_posi_count);

  // rng seed
  wg_edit_box_num_set_value(hw, dlg.simulation.rng.edn_seed, epr_dlg.sim.rng_seed);

  // transmittance polarizer angle list
  wg_edit_box_set_text(hw, dlg.transmit.ed_list, epr_dlg.transmit_test.pol_list_angles_str,
       sizeof(epr_dlg.transmit_test.pol_list_angles_str));
};

static float eds_get_value(eds_t *eds)
{
  return wg_edit_box_num_get_value(eds->ed);
}

static void read_eds_values(void)
{
  epr_dlg.curves.graph_N                 = eds_get_value(&dlg.curves.eds_graph_N);
  epr_dlg.sim.rng_seed                   = wg_edit_box_num_get_value(dlg.simulation.rng.edn_seed);
  epr_dlg.source.emit_pol_diff           = eds_get_value(&dlg.source.eds_emit_pol_diff);
  epr_dlg.source.ent_qm_r                = eds_get_value(&dlg.source.entang.eds_qm_r);
  epr_dlg.source.ent_local_s             = eds_get_value(&dlg.source.entang.eds_local_s);
  epr_dlg.source.em_prob_ent_pair        = eds_get_value(&dlg.source.emit_prob.eds_ent_pair);
  epr_dlg.source.em_prob_nent_pair       = eds_get_value(&dlg.source.emit_prob.eds_nent_pair);
  epr_dlg.source.em_prob_single          = eds_get_value(&dlg.source.emit_prob.eds_single);
  epr_dlg.source.em_prob_double          = eds_get_value(&dlg.source.emit_prob.eds_2nd);
  epr_dlg.source.emit_time_beta_scale    = eds_get_value(&dlg.source.emit_delay.eds_beta_scale);
  epr_dlg.polarizer.e_out_delay          = eds_get_value(&dlg.polarizer.eds_e_out_delay);
  epr_dlg.polarizer.o_out_delay          = eds_get_value(&dlg.polarizer.eds_o_out_delay);
  epr_dlg.polarizer.st1_delay            = eds_get_value(&dlg.polarizer.eds_st1_delay);
  epr_dlg.detectors.det_prob_alice_det_o = eds_get_value(&dlg.detectors.detect_prob.alice.eds_det_o);
  epr_dlg.detectors.det_prob_alice_det_e = eds_get_value(&dlg.detectors.detect_prob.alice.eds_det_e);
  epr_dlg.detectors.det_prob_bob_det_o   = eds_get_value(&dlg.detectors.detect_prob.bob.eds_det_o);
  epr_dlg.detectors.det_prob_bob_det_e   = eds_get_value(&dlg.detectors.detect_prob.bob.eds_det_e);
  epr_dlg.detectors.udet_dep_local_dpr   = eds_get_value(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr);
  epr_dlg.detectors.udet_dep_local_q     = eds_get_value(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr);
  epr_dlg.detectors.misc_noise_det_r     = eds_get_value(&dlg.detectors.misc.eds_dnoise_ratio);
  epr_dlg.pairing.win_width              = eds_get_value(&dlg.pairing.eds_win_width);
  epr_dlg.inequality.test_conf_a1        = eds_get_value(&dlg.inequality.test.angles.eds_a1);
  epr_dlg.inequality.test_conf_a2        = eds_get_value(&dlg.inequality.test.angles.eds_a2);
  epr_dlg.inequality.test_conf_b1        = eds_get_value(&dlg.inequality.test.angles.eds_b1);
  epr_dlg.inequality.test_conf_b2        = eds_get_value(&dlg.inequality.test.angles.eds_b2);
  epr_dlg.inequality.test_conf_N         = eds_get_value(&dlg.inequality.test.eds_N);
  epr_dlg.inequality.angles_max_search   = eds_get_value(&dlg.inequality.find_angles.eds_count);
  epr_dlg.inequality.eval_posi_count     = eds_get_value(&dlg.inequality.test.eval_posi.eds_count);
  epr_dlg.transmit_test.pol_count        = wg_edit_box_num_get_value(dlg.transmit.edn_pol_cnt);
  epr_dlg.transmit_test.max_anv          = wg_edit_box_num_get_value(dlg.transmit.edn_max_anv);
  epr_dlg.transmit_test.sim_N            = wg_edit_box_num_get_value(dlg.transmit.edn_sim_N);
  epr_dlg.transmit_test.max_error        = wg_edit_box_num_get_value(dlg.transmit.edn_max_err);
};

// --------------------
// text buttons

// test rng
static void rng_graph_test(int mode)
{
  int y;
  bitmap_t bm;
  widget_t *wg = dlg.graph.wg_graph;
  bm_init_child(&bm, &hw->cli_bm, wg->c_pos.x, wg->c_pos.y, wg->c_size.x, wg->c_size.y);

  for (y=0; y<bm.size.y; y++)
  {
    pix_t *pix = BM_PIX_ADDR((&bm), 0, y);
    pix_t *pix_end = pix + bm.size.x;

    // #define RGEN_ff rng.rand_int() & 0xff
    // #define RGEN_ff (int)(RAND_1*256.0f)
    #define RGEN_ff (int)(RAND_PI*(256.0f/PI))

    if (mode == 0)   // rgb mode
    {
      while (pix < pix_end)
      {
        int r = RGEN_ff;
        int g = RGEN_ff;
        int b = RGEN_ff;
        W_ASSERT(!((r | g | b) & 0xffffff00));
        *pix++ = COL_RGB(r, g, b);
      }
    }
    else
    if (mode == 1)   // greyscale mode
    {
      while (pix < pix_end)
      {
        int i = RGEN_ff;
        W_ASSERT(!(i & 0xffffff00));
        *pix++ = COL_RGB(i, i, i);
      }
    }
    else             // black and white mode
    {
      while (pix < pix_end)
        *pix++ = rng.rand1() > 0.5 ? COL_RGB(255, 255, 255) : COL_RGB(0, 0, 0);
    }
  }
  win_cli_blit_rect(hw, wg->c_pos.x, wg->c_pos.y, wg->c_size.x, wg->c_size.y);
}

static void curves_update_datas(void);

// ----------------------------------------------
// background epr task
// - disable all buttons except cancel and exit
// - start thread
// - wait until ended or canceled

struct epr_task_t epr_task = { 0 };

static void progress_bar_end_thread(void)
{
  wg_text_update(hw, dlg.experiment.lbl_status, dlg.experiment.lbl_status_idle_str);
  wg_enable(hw, dlg.experiment.but_break, false);
  wg_focus_list_restore(hw);
  switch (epr_task.task_type)
  {
    case e_task_type_undef:
      W_ASSERT(0);
    break;
    case e_task_type_optimize:
      if (!epr_task.cancel)
      {
        eds_set_value(&dlg.inequality.test.angles.eds_a1, epr_dlg.inequality.test_conf_a1);
        eds_set_value(&dlg.inequality.test.angles.eds_a2, epr_dlg.inequality.test_conf_a2);
        eds_set_value(&dlg.inequality.test.angles.eds_b1, epr_dlg.inequality.test_conf_b1);
        eds_set_value(&dlg.inequality.test.angles.eds_b2, epr_dlg.inequality.test_conf_b2);
      }
    break;
    default:;
  }
}

// update progress bar
void epr_task_progress(int progress)
{
  epr_task.progress100 = (progress/5)*5;               // 5% step is good
  wg_progress_bar_set_ratio(hw, dlg.experiment.run_progress, epr_task.progress100);
}

// progress to limit, make changes at end to show activity
void epr_task_progress_to(int progress, int step)
{
  epr_task.progress100 += (epr_task.progress100 < progress) ? step : -step;
  wg_progress_bar_set_ratio(hw, dlg.experiment.run_progress, epr_task.progress100);
}

// must be called by thread to terminate
void epr_task_cancel(void)
{
  co_printf("Tool canceled.\n");
  EPR_PROGRESS(0);
  progress_bar_end_thread();
  win_exit_thread();
}

static void epr_start_task(void)
{
  switch (epr_task.task_type)
  {
    case e_task_type_test_ineq:
      dlg_inequality_test();
    break;
    case e_task_type_optimize:
      dlg_inequality_optimize();
    break;
    case e_task_type_eval_posi:
      dlg_ineq_tools_eval_posi();
    break;
    case e_task_type_find_angles:
      dlg_ineq_tools_find_angles();
    break;
    case e_task_type_test_tr_random:
      dlg_transmit_random_test();
    break;
    case e_task_type_test_tr_list:
      dlg_transmit_list_test();
    break;
    case e_task_type_upd_graph:
      curves_update_datas();
    break;
    default: W_ASSERT(0);
  }
  EPR_PROGRESS(0);
  progress_bar_end_thread();
}

void dlg_run_thread_task(enum epr_task_type task_type, bool reset_seed, const char *run_status)
{
  widget_t *wg_focus_enable[] =
  {
    dlg.experiment.but_break,
    dlg.console.bt_exit,
    NULL,
  };
  if (reset_seed && wg_get_state(dlg.inequality.test.chk_auto_seed))
    rng_srand();

  wg_text_update(hw, dlg.experiment.lbl_status, run_status);
  wg_enable(hw, dlg.experiment.but_break, true);
  wg_focus_list_only(hw, wg_focus_enable);           // disable all widgets, except break and exit button
  epr_task.cancel = 0;
  epr_task.task_type = task_type;
  win_create_thread(epr_start_task);
}

static void show_help_win(void);

static void text_bt_cb(hwin_t *hwin, widget_t *wg)
{
  read_eds_values();
  if      (wg == dlg.simulation.rng.bt_test_col)        rng_graph_test(0);
  else if (wg == dlg.simulation.rng.bt_test_grey)       rng_graph_test(1);
  else if (wg == dlg.simulation.rng.bt_test_bw)         rng_graph_test(2);
  else if (wg == dlg.simulation.rng.bt_apply_seed)      rng_srand();
  else
  if (wg == dlg.inequality.test.but_test)
    dlg_run_thread_task(e_task_type_test_ineq,       true, "Test inequality");
  else
  if (wg == dlg.inequality.test.but_optimize)
    dlg_run_thread_task(e_task_type_optimize,        true, "Optimize inequality");
  else
  if (wg == dlg.inequality.find_angles.but_search)
    dlg_run_thread_task(e_task_type_find_angles,    false, "Search ineq. angles");
  else
  if (wg == dlg.inequality.test.eval_posi.but_eval)
    dlg_run_thread_task(e_task_type_eval_posi,      false, "Eval positivity");
  else
  if (wg == dlg.transmit.but_random)
    dlg_run_thread_task(e_task_type_test_tr_random, false, "Eval transmittance");
  else
  if (wg == dlg.transmit.but_list)
    dlg_run_thread_task(e_task_type_test_tr_list,   false, "Eval transmittance");
  else
  if (wg == dlg.curves.bt_update)
    dlg_run_thread_task(e_task_type_upd_graph,      false, "Update curves");
  else
  if (wg == dlg.experiment.but_break)
    epr_task.cancel = true;
  else
  if      (wg == dlg.console.bt_clear)  wg_console_clear(hw, dlg.console.console_out);
  else if (wg == dlg.console.bt_copy)   wg_console_copy_to_clipboard(hw, dlg.console.console_out);
  else if (wg == dlg.console.bt_help)   show_help_win();
  else if (wg == dlg.console.bt_exit)
    win_close(hw);
}

// ----------------------------------------------
// combo box

static void select_experiment_cb(hwin_t *hwin, int sel_id)
{
  dlg_select_experiment(sel_id);
}

static void select_ineq_test_result_cb(hwin_t *hwin, int sel_id)
{
  struct ineq_res_t *r = &ineq_search_res.ineq_res[sel_id];
  // copy angles
  wg_edit_box_num_set_value(hw, dlg.inequality.test.angles.eds_a1.ed, RAD_TO_DEG(r->a1_b1.an_a));
  wg_edit_box_num_set_value(hw, dlg.inequality.test.angles.eds_a2.ed, RAD_TO_DEG(r->a2_b1.an_a));
  wg_edit_box_num_set_value(hw, dlg.inequality.test.angles.eds_b1.ed, RAD_TO_DEG(r->a1_b1.an_b));
  wg_edit_box_num_set_value(hw, dlg.inequality.test.angles.eds_b2.ed, RAD_TO_DEG(r->a1_b2.an_b));
}

// ----------------------------------------------
// widgets enable

static void eds_enable(eds_t *eds, bool enable)
{
  wg_enable(hw, eds->lbl, enable);
  wg_enable(hw, eds->ed, enable);
}

static void update_widget_enable(void)
{
  // -----------------------------------
  // frame RNG
  wg_enable(hw, dlg.simulation.rng.rad_cpu_buildin   , hw_rng_detected);
  wg_enable(hw, dlg.simulation.rng.label_select_seed , epr_dlg.sim.rng_type != e_rng_cpu);
  wg_enable(hw, dlg.simulation.rng.edn_seed          , epr_dlg.sim.rng_type != e_rng_cpu);
  wg_enable(hw, dlg.simulation.rng.bt_apply_seed     , epr_dlg.sim.rng_type != e_rng_cpu);

  // -----------------------------------
  // frame entang
  eds_enable(&dlg.source.entang.eds_qm_r   , epr_dlg.sim.model_qm);
  eds_enable(&dlg.source.entang.eds_local_s, epr_dlg.sim.model_local);
  wg_enable(hw, dlg.source.entang.chk_local_polarize, epr_dlg.sim.model_local);

  // -----------------------------------
  // emit times
  wg_enable(hw, dlg.source.emit_delay.frm               , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.source.emit_delay.chk_enable_jitter , epr_dlg.sim.model_local);
  eds_enable(&dlg.source.emit_delay.eds_beta_scale  , epr_dlg.sim.model_local);

  // -----------------------------------
  // frame polarizer
  wg_enable(hw, dlg.polarizer.frm               , epr_dlg.sim.model_local);
  eds_enable(&dlg.polarizer.eds_e_out_delay , epr_dlg.sim.model_local);
  eds_enable(&dlg.polarizer.eds_o_out_delay , epr_dlg.sim.model_local);
  eds_enable(&dlg.polarizer.eds_st1_delay   , epr_dlg.sim.model_local);

  // -----------------------------------
  // frame detect probability
  eds_enable(&dlg.detectors.detect_prob.bob.eds_det_o, !epr_dlg.detectors.det_prob_bob_same_alice);
  eds_enable(&dlg.detectors.detect_prob.bob.eds_det_e, !epr_dlg.detectors.det_prob_bob_same_alice);
  eds_enable(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr, epr_dlg.sim.model_local);
  eds_enable(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr , epr_dlg.sim.model_local);
  eds_enable(&dlg.detectors.misc.eds_dnoise_ratio, !epr_dlg.pairing.win_type_mobile);

  // -----------------------------------
  // frame pairing
  wg_enable(hw, dlg.pairing.win_type.frm, epr_dlg.sim.model_local);
  wg_enable(hw, dlg.pairing.win_type.rad_fixed, epr_dlg.sim.model_local);
  wg_enable(hw, dlg.pairing.win_type.rad_mobile, epr_dlg.sim.model_local);
  wg_enable(hw, dlg.pairing.chk_ignore_dual, epr_dlg.pairing.win_type_fixed);
  eds_enable(&dlg.pairing.eds_win_width , epr_dlg.sim.model_local);

  // ----------------------------------
  // frame transmit
  wg_enable(hw, dlg.transmit.frm         , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.but_random  , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.lbl_nb_pol  , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.edn_pol_cnt , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.lbl_max_anv , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.edn_max_anv , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.lbl_sim_N   , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.edn_sim_N   , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.lbl_max_err , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.edn_max_err , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.but_list    , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.transmit.ed_list     , epr_dlg.sim.model_local);

  // -----------------------------------
  // frame curves
  wg_enable(hw, dlg.curves.tr_rel_alice_o  , epr_dlg.sim.model_local);
  wg_enable(hw, dlg.curves.src_emit_t_dist, epr_dlg.sim.model_local && epr_dlg.pairing.win_type_fixed && epr_dlg.source.emit_enable_jitter);
}

#if 0
// init application main window
bool win_app_main(char *cmd_line)
{
  const vec2i *scr_size = win_get_screen_size(); // screen size
  vec2i cli_size;                                // window client size

  // define minimal x borders
  cli_size.x = scr_size->x - 50;

  // define aspect ratio
  cli_size.y = (cli_size.x*dlg_size.max.y)/dlg_size.max.x;

  // clip to min/max dialog sizes
  vec_clip_min_max(&cli_size, &dlg_size.min, &dlg_size.max);

  font_init_aa();                                // init font anti aliasing
  if (win_create("EPR experiment simulation", NULL, &cli_size, &dlg_size.min, &dlg_size.max, win_msg_proc, NULL, NULL))
  {
    dlg_ctrl_init();
    return true;
  }
  return false;
}
#else
void rng_test(void);

bool win_app_main(char *cmd_line)
{
  const vec2i *scr_size = win_get_screen_size(); // screen size
  vec2i cli_size;                                // window client size
  vec2i max_open;                                // max opening size

#if 0
  rng_select_type(e_rng_ms_c_lib);
  rng.srand(-123357);
  gen_dpr_dat();
#endif
#if 0
  rng_test();
  return 0;
#endif

  max_open.x = dlg_size.max.x - 20;
  max_open.y = dlg_size.max.y - 15;

  // define minimal x borders
  cli_size.x = scr_size->x - 20;

  // define aspect ratio
  cli_size.y = (cli_size.x*max_open.y)/max_open.x;

  // clip to min/max dialog sizes
  vec_clip_min_max(&cli_size, &dlg_size.min, &max_open);

  font_init_aa();                                // init font anti aliasing
  if (win_create("EPR experiment simulation", NULL, &cli_size, &dlg_size.min, &dlg_size.max, win_msg_proc, NULL, NULL))
  {
    dlg_ctrl_init();
    return true;
  }
  return false;
}
#endif

// ----------------------------------------------
// curves drawing
// ----------------------------------------------

// curves
static struct wg_graph_curve crv_co_oo_ee        = { 0, "correllation oo+ee"   , false, { COL_RGB(200,   0,   0), COL_RGB(250,   0,   0) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].co_oo_ee };
static struct wg_graph_curve crv_co_oe_eo        = { 0, "correllation oe+eo"   , false, { COL_RGB( 50, 150, 200), COL_RGB( 70, 190, 255) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].co_oe_eo };
static struct wg_graph_curve crv_det_pair_ratio  = { 0, "detect pair ratio"    , false, { COL_RGB( 70,  70, 220), COL_RGB(100, 100, 250) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].det_pair_ratio };
static struct wg_graph_curve crv_det_sing_ratio  = { 0, "detect single ratio"  , false, { COL_RGB(200, 200,   0), COL_RGB(250, 250,   0) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].det_sing_ratio };
static struct wg_graph_curve crv_det_uu_ratio    = { 0, "undetect uu ratio"    , false, { COL_RGB(200,   0, 200), COL_RGB(250,   0, 250) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].det_uu_ratio };
static struct wg_graph_curve crv_det_acc_ratio   = { 0, "accidentals ratio"    , false, { COL_RGB(  0, 200, 200), COL_RGB(  0, 200, 200) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].det_acc_ratio };
static struct wg_graph_curve crv_transm_alice_o  = { 0, "transmittance Alice"  , false, { COL_RGB(128, 128, 128), COL_RGB(180, 180, 180) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].transmit_a };
static struct wg_graph_curve crv_tr_rel_alice_o  = { 0, "transm. rel Alice"    , false, { COL_RGB(  0, 200,   0), COL_RGB(  0, 250,   0) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].tr_rel_ao };
static struct wg_graph_curve crv_ref_sin2        = { 0, "reference sin²"       , false, { COL_RGB(128, 128, 128), COL_RGB(180, 180, 180) }, { 0,  0 }, 0, PI, 0, sizeof(struct sim_data_t), 3, &sim_result.sim_data_res[0].sin2 };
static struct wg_graph_curve crv_ref_cos2        = { 0, "reference cos²"       , false, { COL_RGB(128, 128, 128), COL_RGB(180, 180, 180) }, { 0,  0 }, 0, PI, 0, sizeof(struct sim_data_t), 3, &sim_result.sim_data_res[0].cos2 };
static struct wg_graph_curve crv_ref_triangle    = { 0, "reference triangle"   , false, { COL_RGB(128, 128, 128), COL_RGB(180, 180, 180) }, { 0,  0 }, 0, PI, 0, sizeof(struct sim_data_t), 3, &sim_result.sim_data_res[0].triangle };
static struct wg_graph_curve crv_src_emit_t_dist = { 0, "source emit t dist."  , false, { COL_RGB(128, 128, 128), COL_RGB(180, 180, 180) }, {-1, -2 }, 0, PI, 0, sizeof(struct sim_data_t), 1, &sim_result.sim_data_res[0].src_t_dis };

static struct gr_color_conf_t
{
  pix_t gr_bk_color;
  pix_t gr_axis_color;
  pix_t txt_legend_color;
  pix_t txt_legend_aa;
} gr_color_conf[2] =
{
  { // white background
    COL_RGB(255, 255, 255),   // gr_bk_color
    COL_RGB(  0,   0,   0),   // gr_axis_color
    COL_RGB(  0,   0,   0),   // txt_legend_color
    COL_RGB(225, 225, 225),   // txt_legend_aa
  },
  { // dark backgrount
    COL_RGB( 32,  32,  32),   // gr_bk_color;
    COL_RGB(140, 140, 140),   // gr_axis_color;
    COL_RGB(225, 225, 225),   // txt_legend_color;
    COL_RGB(  0,   0,   0)    // txt_legend_aa;
  }
};

// note: declare in draw order
static struct wg_graph_curve *curve_list[] =
{
  &crv_ref_sin2,
  &crv_ref_cos2,
  &crv_ref_triangle,
  &crv_det_pair_ratio,
  &crv_det_sing_ratio,
  &crv_det_uu_ratio,
  &crv_det_acc_ratio,
  &crv_transm_alice_o,
  &crv_tr_rel_alice_o,
  &crv_src_emit_t_dist,
  &crv_co_oo_ee,
  &crv_co_oe_eo,
  NULL                           // define list end
};

#define GR_PI_RANGE_XMIN -0.05f
#define GR_PI_RANGE_XMAX (PI+0.05f)

// configure graph drawing
void dlg_configure_graph_base(void)
{
  static const struct wg_graph_axis_list axis_x = { 3, { 0.0f, PI/2.0f, PI } };
  static const struct wg_graph_axis_list axis_y = { 2, { 0.0f, 1.0f } };
  struct gr_color_conf_t *gr_cc = &gr_color_conf[dlg.curves.graph_col_id];
  wg_config_graph(dlg.graph.wg_graph, GR_PI_RANGE_XMIN, GR_PI_RANGE_XMAX, -0.07f, 1.04f, PI/18.0f, 0.1f,
                  &axis_x, &axis_y, gr_cc->gr_bk_color, gr_cc->gr_axis_color);
  wg_graph_def_curve_list(dlg.graph.wg_graph, (const struct wg_graph_curve **)curve_list);  // note: GCC complain if not const ?
}

// graph post draw call back, display axis
static void graph_post_draw_cb(hwin_t *hwin, widget_t *wg, bitmap_t *bm)
{
  font_t *f = win_font_list[fnt_vthm8];
  struct gr_color_conf_t *gr_cc = &gr_color_conf[dlg.curves.graph_col_id];
  bm_draw_string(bm, 14, wg->c_size.y - f->dy - 4, "x 0..PI   y 0..1", gr_cc->txt_legend_color, gr_cc->txt_legend_aa, f, -1);
}

// refresh curve list
static void curves_refresh_list(void)
{
  crv_co_oo_ee.show        = epr_dlg.curves.corr_oo_ee;
  crv_co_oe_eo.show        = epr_dlg.curves.corr_oe_eo;
  crv_det_pair_ratio.show  = epr_dlg.curves.det_pair_ratio;
  crv_det_sing_ratio.show  = epr_dlg.curves.det_sing_ratio;
  crv_det_uu_ratio.show    = epr_dlg.curves.det_uu_ratio;
  crv_det_acc_ratio.show   = epr_dlg.curves.det_acc_ratio;
  crv_transm_alice_o.show  = epr_dlg.curves.transm_alice_o;
  crv_tr_rel_alice_o.show  = epr_dlg.curves.tr_rel_alice_o;
  crv_ref_sin2.show        = epr_dlg.curves.ref_sin2;
  crv_ref_cos2.show        = epr_dlg.curves.ref_cos2;
  crv_ref_triangle.show    = epr_dlg.curves.ref_triangle;
  crv_src_emit_t_dist.show = epr_dlg.curves.src_emit_t_dist;
  wg_graph_refresh(hw, dlg.graph.wg_graph);
}

// draw curve colors near checkbox
static void crv_dr_col(bitmap_t *bm, widget_t *wg, pix_t color, bool blit)
{
  int x = wg->e_pos.x - CRV_COL_RECT-2;
  int y = wg->e_pos.y + 2;
  bm_paint_rect(bm, x, y, CRV_COL_RECT, CRV_COL_RECT, color);
  if (blit)
    win_cli_blit_rect(hw, x, y, CRV_COL_RECT, CRV_COL_RECT);
}

static void curves_ckbox_draw_color(bitmap_t *bm, bool blit)
{
  int col_id = dlg.curves.graph_col_id;
  crv_dr_col(bm, dlg.curves.co_oo_ee       , crv_co_oo_ee.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.det_pair_ratio , crv_det_pair_ratio.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.det_uu_ratio   , crv_det_uu_ratio.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.co_oe_eo       , crv_co_oe_eo.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.det_sing_ratio , crv_det_sing_ratio.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.det_acc_ratio  , crv_det_acc_ratio.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.transm_alice_o , crv_transm_alice_o.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.tr_rel_alice_o , crv_tr_rel_alice_o.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.src_emit_t_dist, crv_src_emit_t_dist.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.ref_sin2       , crv_ref_sin2.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.ref_cos2       , crv_ref_cos2.color[col_id], blit);
  crv_dr_col(bm, dlg.curves.ref_triangle   , crv_ref_triangle.color[col_id], blit);
}

// update datas for selected curves and refresh graph
static void curves_update_datas(void)
{
  struct wg_graph_curve **crv;
  int data_count = (int)((dlg.graph.wg_graph->c_size.x * PI)/(GR_PI_RANGE_XMAX-GR_PI_RANGE_XMIN))*2;
  W_ASSERT(data_count < CRV_MAX_Y_DATA);
  if (data_count > CRV_MAX_Y_DATA)
    data_count = CRV_MAX_Y_DATA;

  read_eds_values();
  dlg_ctrl_update_graph(0.0f, PI, data_count);
  for (crv = curve_list; *crv; crv++)
    (*crv)->data_count = data_count;
  curves_refresh_list();
}

// change graph colors
static void graph_toggle_colors(void)
{
  struct gr_color_conf_t *gr_cc = &gr_color_conf[dlg.curves.graph_col_id];
  wg_graph_select_color_id(hw, dlg.graph.wg_graph, dlg.curves.graph_col_id, gr_cc->gr_bk_color, gr_cc->gr_axis_color, true);
  curves_ckbox_draw_color(&hw->cli_bm, true);
}

static void check_box_cb_crv(hwin_t *hwin, widget_t *wg)
{
  epr_dlg.curves.corr_oo_ee      = wg_get_state(dlg.curves.co_oo_ee);
  epr_dlg.curves.corr_oe_eo      = wg_get_state(dlg.curves.co_oe_eo);
  epr_dlg.curves.det_pair_ratio  = wg_get_state(dlg.curves.det_pair_ratio);
  epr_dlg.curves.det_sing_ratio  = wg_get_state(dlg.curves.det_sing_ratio);
  epr_dlg.curves.det_uu_ratio    = wg_get_state(dlg.curves.det_uu_ratio);
  epr_dlg.curves.det_acc_ratio   = wg_get_state(dlg.curves.det_acc_ratio);
  epr_dlg.curves.transm_alice_o  = wg_get_state(dlg.curves.transm_alice_o);
  epr_dlg.curves.tr_rel_alice_o  = wg_get_state(dlg.curves.tr_rel_alice_o);
  epr_dlg.curves.ref_sin2        = wg_get_state(dlg.curves.ref_sin2);
  epr_dlg.curves.ref_cos2        = wg_get_state(dlg.curves.ref_cos2);
  epr_dlg.curves.ref_triangle    = wg_get_state(dlg.curves.ref_triangle);
  epr_dlg.curves.src_emit_t_dist = wg_get_state(dlg.curves.src_emit_t_dist);

  if (wg == dlg.curves.chk_dark_col)
  {
    dlg.curves.graph_col_id = wg_get_state(dlg.curves.chk_dark_col);
    graph_toggle_colors();
  }
  else
  if (wg == dlg.curves.chk_no_pix_aa)
  {
    wg_graph_enable_aa(dlg.graph.wg_graph, !wg_get_state(dlg.curves.chk_no_pix_aa));
    curves_refresh_list();
  }
  else
    curves_refresh_list();
}

static void update_source_emit_prob(widget_t *wg, float value)
{
  if (    (wg == dlg.source.emit_prob.eds_ent_pair.ed)
       || (wg == dlg.source.emit_prob.eds_nent_pair.ed)
       || (wg == dlg.source.emit_prob.eds_single.ed)
       || (wg == dlg.source.emit_prob.eds_2nd.ed))
  {
    // keep probability of 1, adjust non emission result
    pix_t col_txt, col_aa;
    float nem = 1.0f;                                          // no emit remaining
    nem -= eds_get_value(&dlg.source.emit_prob.eds_ent_pair);  // entangled pairs
    nem -= eds_get_value(&dlg.source.emit_prob.eds_nent_pair); // non-entangled pairs
    nem -= eds_get_value(&dlg.source.emit_prob.eds_single);    // single
    nem -= eds_get_value(&dlg.source.emit_prob.eds_2nd);       // >= double entangled pairs

    epr_dlg.source.em_no_emit_lbl_value = nem;
    sprintf(dlg.source.emit_prob.lbl_no_emit_str, "No emission: %.3f", nem);
    if (nem >= 0)
    {
      col_txt = wgs.text.text_color;
      col_aa = wgs.text.text_aa_color;
    }
    else
    {
      col_txt = COL_RGB(220, 0, 0);
      col_aa = font_get_aa_color(col_txt, wgs.clear_bk_color, 20);
    }
    wg_text_update_ex(hw, dlg.source.emit_prob.lbl_no_emit, NULL, col_txt, col_aa, dlg.source.emit_prob.lbl_no_emit_str);
  }
}

static void set_undetect_random_origin_ratio(float dep_rand)
{
  pix_t col_txt, col_aa;
  epr_dlg.detectors.udet_dep_random = dep_rand;
  sprintf(dlg.detectors.detect_prob.u_detect_dep.lbl_udet_rand_str, "Randomness: %.3f", dep_rand);
  if (dep_rand >= 0)
  {
    col_txt = wgs.text.text_color;
    col_aa = wgs.text.text_aa_color;
  }
  else
  {
    col_txt = COL_RGB(220, 0, 0);
    col_aa = font_get_aa_color(col_txt, wgs.clear_bk_color, 20);
  }
  wg_text_update_ex(hw, dlg.detectors.detect_prob.u_detect_dep.lbl_udet_rand, NULL, col_txt, col_aa, dlg.detectors.detect_prob.u_detect_dep.lbl_udet_rand_str);
}

static void update_undetect_random_origin_ratio(widget_t *wg)
{
  if (    (wg == dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr.ed)
       || (wg == dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr.ed))
  {
    // keep probability of 1, adjust non emission result
    float dep_rand = 1.0f;                                       // random origin
    dep_rand -= eds_get_value(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_dpr);  // dpr
    dep_rand -= eds_get_value(&dlg.detectors.detect_prob.u_detect_dep.eds_loc_qr);   // q
    set_undetect_random_origin_ratio(dep_rand);
  }
}

// a numerical value have been changed
void eds_num_value_changed_cb(widget_t *wg, float value)
{
  if (wg == dlg.simulation.rng.edn_seed)
  {
    rng_srand();
    return;
  }

  // check if is emission prob
  update_source_emit_prob(wg, value);

  // check if is no detection origin
  update_undetect_random_origin_ratio(wg);
}

// dialog update
void dlg_init_dialog(void)
{
  update_edn_values();
  update_radio_buttons();
  update_check_boxes();
  read_eds_values();          // read back
  select_rng_type(epr_dlg.sim.rng_type);
}

#include <stdarg.h>

// print on console using console widget
void co_printf(const char *fmt, ...)
{
  char str[2048];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  wg_console_print(hw, dlg.console.console_out, str);
  va_end(args);
}

// ----------------------------------------------
// inequality test result list

#define CBO_TEST_RES_STR_LENMAX 80
static char cbo_test_res_str_list[MAX_INEQ_TEST_RES][CBO_TEST_RES_STR_LENMAX];

// update inequality list combo box
void dlg_update_ineq_res_combo(void)
{
  int i;
  struct ineq_res_t *r_best = NULL;
  double Jmin = 2.0;

  for (i=0; i<ineq_search_res.count_found; i++)
  {
    struct ineq_res_t *r = &ineq_search_res.ineq_res[i];
    sprintf(cbo_test_res_str_list[i], "%d/ J/N %.6f an %.2f %.2f %.2f %.2f", i+1, r->Eb.JoN,
           RAD_TO_DEG(r->a1_b1.an_a),
           RAD_TO_DEG(r->a2_b1.an_a),
           RAD_TO_DEG(r->a2_b1.an_b),
           RAD_TO_DEG(r->a2_b2.an_b));
    ineq_test_res_list[i] = cbo_test_res_str_list[i];
    if (r->Eb.JoN < Jmin)
    {
      Jmin = r->Eb.JoN;
      r_best = r;
    }
  }
  ineq_test_res_list_size = ineq_search_res.count_found;
  wg_combo_update_list_size(hw, dlg.inequality.find_angles.cbo_results, ineq_search_res.count_found);

  if (wg_get_state(dlg.inequality.find_angles.chk_auto_select) && r_best)
  {
    eds_set_value(&dlg.inequality.test.angles.eds_a1, RAD_TO_DEG(r_best->a1_b1.an_a));
    eds_set_value(&dlg.inequality.test.angles.eds_a2, RAD_TO_DEG(r_best->a2_b1.an_a));
    eds_set_value(&dlg.inequality.test.angles.eds_b1, RAD_TO_DEG(r_best->a2_b1.an_b));
    eds_set_value(&dlg.inequality.test.angles.eds_b2, RAD_TO_DEG(r_best->a2_b2.an_b));
  }
}

// ----------------------------------------------
// help message
static void show_help_win(void)
{
  const char *help_msg =
  "EPR sim v1.0\n\n"
  "A tooltip is displayed for most of the parameters by immobilizing\n"
  "the mouse cursor on an field or label.\n\n"
  "An experiment configuration can be selected in Experiment frame.\n"
  "More detailed help and a tutorial are available on the github site\n"
  "https://github.com/pierrel55/pol_sim\n\n"
  "For a bug report please contact me by email.\n";
  wg_message_box(help_msg, "Help", msg_box_ok, hw, NULL);
}
