// define experiments configurations and help
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "epr.h"
#include "../dlg_ctrl.h"

int experiment_str_list_size = 11;
const char *experiment_str_list[MAX_EXP_COUNT] =
{
  "1/ QM, perfect source and detection",
  "2/ QM, Detection rate 0.830, r=1",
  "3/ QM, Detection rate 0.827, r=1",
  "4/ QM, Detection rate 0.750, r=0.26",
  "5/ Local model, perfect source and detection",
  "6/ Local model, sin� correlations detection based",
  "7/ Local model, sin� correlations pairing based 1",
  "8/ Local model, sin� correlations pairing based 2",
  "9/ Local model, sin� correlations pairing based 3",
  "10/ Local model, Mobile windows pairing",
  "11/ Local model, Fixed windows pairing",
  NULL,
};

void init_config_1_qm_perfect(void)
{
  epr_dlg.curves.graph_N                    = 5000;
  epr_dlg.curves.corr_oo_ee                 = true;

  epr_dlg.sim.model_qm                      = true;
  epr_dlg.sim.rng_type                      = e_rng_ms_c_lib;
  epr_dlg.sim.rng_seed                      = 123;

  epr_dlg.source.emit_pol_diff              = 90.0f;
  epr_dlg.source.ent_qm_r                   = 1.0f;
  epr_dlg.source.em_prob_ent_pair           = 1.0f;

  epr_dlg.detectors.det_prob_alice_det_o    = 1.0f;
  epr_dlg.detectors.det_prob_alice_det_e    = 1.0f;

  epr_dlg.detectors.det_prob_bob_same_alice = true;
  epr_dlg.detectors.det_prob_bob_det_o      = 1.0f;
  epr_dlg.detectors.det_prob_bob_det_e      = 1.0f;

  epr_dlg.pairing.win_type_fixed            = true;
  epr_dlg.inequality.test_conf_N            = 500000;
  epr_dlg.inequality.eval_posi_count        = 20;
  epr_dlg.inequality.auto_select_res        = true;

  // local
  epr_dlg.source.ent_local_s                = 1.0f;
  epr_dlg.source.emit_time_beta_scale       = 1.0f;
  epr_dlg.pairing.win_width                 = 0.5f;
}

void dlg_select_experiment(int conf_id)
{
  const char *desc_txt = "";
  memset(&epr_dlg, 0, sizeof(epr_dlg));
  conf_id++;                           // combo is 0 based
  if (conf_id == 1)
  {
    init_config_1_qm_perfect();
    epr_dlg.inequality.test_conf_a1 = 11.86f;
    epr_dlg.inequality.test_conf_a2 = 56.86f;
    epr_dlg.inequality.test_conf_b1 = 124.31f;
    epr_dlg.inequality.test_conf_b2 = 79.41f;
    desc_txt =
    "1/ QM perfect source and detection.\n"
    "The source and the detectors are perfect.\n"
    "This configuration makes it possible to display the theoretical curves and\n"
    "to test the Eberhard inequality.\n"
    "With a maximum entanglement (r = 1) we must obtain a value of J/N close to -0.207\n"
    "(Use the 'Inequality / Eval positivity / Eval' button to check).\n\n";
  }
  else
  if (conf_id == 2)
  {
    init_config_1_qm_perfect();
    epr_dlg.detectors.det_prob_alice_det_o    = 0.83f;
    epr_dlg.detectors.det_prob_alice_det_e    = 0.83f;

    epr_dlg.inequality.test_conf_a1 = 11.86f;
    epr_dlg.inequality.test_conf_a2 = 56.86f;
    epr_dlg.inequality.test_conf_b1 = 124.31f;
    epr_dlg.inequality.test_conf_b2 = 79.41f;
    desc_txt =
    "2/ QM, Detection rate 0.83, r=1\n"
    "The source is perfect, the detection rate is 83% with r = 1.0\n"
    "The violation of Eberhard's inequality still occurs with an average value of J/N around -0.0015\n"
    "(Use the 'Inequality / Eval positivity / Eval' button to check).\n\n";
  }
  else
  if (conf_id == 3)
  {
    init_config_1_qm_perfect();
    epr_dlg.detectors.det_prob_alice_det_o    = 0.827f;
    epr_dlg.detectors.det_prob_alice_det_e    = 0.827f;

    epr_dlg.inequality.test_conf_a1 = 11.86f;
    epr_dlg.inequality.test_conf_a2 = 56.86f;
    epr_dlg.inequality.test_conf_b1 = 124.31f;
    epr_dlg.inequality.test_conf_b2 = 79.41f;
    desc_txt =
    "3/ QM, Detection rate 0.827, r=1\n"
    "The source is perfect, the detection rate is 82.7% with r = 1.0\n"
    "Eberhard's inequality violation no longer occurs, with an average J/N value around +0.0015\n"
    "(Use the 'Inequality / Eval positivity / Eval' button to check).\n\n";
  }
  else
  if (conf_id == 4)
  {
    init_config_1_qm_perfect();
    epr_dlg.source.ent_qm_r                   = 0.26f;
    epr_dlg.detectors.det_prob_alice_det_o    = 0.75f;
    epr_dlg.detectors.det_prob_alice_det_e    = 0.75f;

    epr_dlg.inequality.test_conf_a1 = 174.89f;
    epr_dlg.inequality.test_conf_a2 = 20.87f;
    epr_dlg.inequality.test_conf_b1 = 92.6f;
    epr_dlg.inequality.test_conf_b2 = 59.3f;
    desc_txt =
    "4/ QM, Detection rate 0.750, r=0.26\n"
    "With r = 0.26 we get J/N varying around -0.0054\n"
    "We can see that reducing the entanglement level makes it possible to obtain the violation\n"
    "of the inequality with a detection rate of 0.75\n"
    "(Use the 'Inequality / Eval positivity / Eval' button to check).\n\n";
  }
  else
  if (conf_id == 5)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.curves.ref_sin2 = true;

    epr_dlg.inequality.test_conf_a1 = 35.24f;
    epr_dlg.inequality.test_conf_a2 = 176.92f;
    epr_dlg.inequality.test_conf_b1 = 103.28f;
    epr_dlg.inequality.test_conf_b2 = 169.15f;
    desc_txt =
    "5/ Local, perfect source and detection\n"
    "The plot of the curve shows a classic correlation in the form of a triangle.\n"
    "The inequality and positivity test shows that this configuration generates J/N oscillating around 0\n"
    "\n"
    "A positivity test with 50 evaluations shows a positivity approaching 0.5, with a standard deviation of\n"
    "J/N depending on the number of samples used.\n"
    "\n"
    "Positivity test example:\n"
    "50x with N = 1500000 => positivity 52% rms: 0.000754\n"
    "50x with N = 2000000 => positivity 56% rms: 0.000536\n"
    "50x with N = 2500000 => positivity 54% rms: 0.000456\n"
    "50x with N = 3000000 => positivity 60% rms: 0.000519\n"
    "100x with N = 4000000 => positivity 46% average J/N = -0.000007 rms: 0.000413\n"
    "=> The positivity varies around 50%.\n"
    "=> J/N and the standard deviation from 0 tends towards 0 when N increase.\n\n";
  }
  else
  if (conf_id == 6)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.detectors.det_prob_alice_det_o = 0.83f;
    epr_dlg.detectors.det_prob_alice_det_e = 0.83f;
    epr_dlg.detectors.udet_dep_local_dpr = 1.0;

    desc_txt =
    "6/ Local, sin^2 correlations detection based\n"
    "This configuration generates detection correlations in sin^2 or cos^2 by reducing the detection\n"
    "rate of the sensors.\n"
    "The detection rate is defined to 83% with origin of non-detection depending on the dpr value.\n"
    "Value dpr is the photon polarisation adjustment occured when passing through the polarizer.\n"
    "With this configuration, the photons that undergo the most adjustment of their initial\n"
    "polarization are less likely to be detected.\n"
    "We then obtain a correlation curve in sin^2 with an average of 25.3% of single detection\n"
    "and 4.2% of double non-detection. (uu)\n"
    "By activating the Ref.sin^2 curve on graph, difference with theoretical result can be evaluated.\n\n";
  }
  else
  if (conf_id == 7)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.pairing.win_width = 0.256f;

    desc_txt =
    "7/ Local, sin^2 correlations pairing based 1.\n"
    "This configuration generates detection correlation in sin^2 or cos^2 without modifying the\n"
    "detection rate but by acting on the pairing.\n"
    "Some photons will be detected as simple measurements because they are detected outside\n"
    "the maximum time of the pairing window (fixed window).\n"
    "There are several ways to cause these rejections.\n"
    "This first test acts only on the size of the window and reduces its size in order to reject\n"
    "the later detections.\n"
    "The optimum size for detecting all particles is 0.5 (* time constant * PI).\n"
    "By setting a window size to 0.256 (about 50% too narrow), this produces accurately sin^2\n"
    "correlations.\n\n";
  }
  else
  if (conf_id == 8)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.pairing.win_width = 0.32f;
    epr_dlg.source.emit_enable_jitter = true;
    epr_dlg.source.emit_time_beta_scale = 2.0f;

    desc_txt =
    "8/ Local, sin^2 correlations pairing based 2.\n"
    "This example is based on the previous example (7), but additionally simulates a jitter effect\n"
    "at the source.\n"
    "This jitter is a random delay added when sending a pair.\n"
    "It increases the probability that a particle will be detected outside the detection window.\n"
    "(fixed window, pulsed source).\n"
    "This jitter is defined with a beta distribution whose scale can be set from 1 to 5 (in unit\n"
    "of time proportional to PI)\n"
    "This distribution can be displayed on the graph by activating the 'Src Jitter' curve.\n"
    "There are several 'Window width' + 'Beta scale' configurations to obtain a sin^2 correlations\n"
    "The example uses Window with = 0.32 and Beta scale = 2.0\n\n";
  }
  else
  if (conf_id == 9)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.polarizer.st1_delay = 0.245f;

    desc_txt =
    "9/ Local, sin^2 correlations pairing based 3.\n"
    "This last example based on pairing, generates sin^2 correlations without requiring a reduction\n"
    "in the window size.\n"
    "It assumes a nonlinear effect on the propagation delay of the photon through the polarizer.\n"
    "This delay follows a form of sigmoid function with an inflection point in PI / 4.\n"
    "The amplitude of the delay can be adjusted in the polarizer frame with the parameter 'st1 delay'.\n"
    "By setting st1 to 0.245 we get correlations in sin^2 with a window size of 0.5\n"
    "The same effect can be obtained with for example st1 at 0.18 + beta jitter at 3.5.\n\n";
  }
  else
  if (conf_id == 10)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.polarizer.st1_delay = 0.2f;
    epr_dlg.pairing.win_type_fixed = false;
    epr_dlg.pairing.win_type_mobile = true;
    epr_dlg.pairing.win_width = 0.35f;
    epr_dlg.detectors.det_prob_alice_det_o = 0.75f;
    epr_dlg.detectors.det_prob_alice_det_e = 0.75f;
    epr_dlg.detectors.udet_dep_local_dpr = 0.65f;
    epr_dlg.detectors.udet_dep_local_q = 0.35f;

    epr_dlg.inequality.test_conf_a1 = 64.73f;
    epr_dlg.inequality.test_conf_a2 = 33.94f;
    epr_dlg.inequality.test_conf_b1 = 147.92f;
    epr_dlg.inequality.test_conf_b2 = 161.56f;
    epr_dlg.inequality.eval_posi_count = 50;
    epr_dlg.inequality.test_conf_N = 1000000;

    desc_txt =
    "10/ Local, Mobile windows pairing.\n"
    "This configuration is the only one which uses a pairing by movable window.\n"
    "Although this method of pairing is no longer valid for modern experiments, the goal is to show the\n"
    "effect that this method can produce with a local model.\n"
    "To get closer to real experiments, the configuration defines a detection efficiency of 0.75\n"
    "The window size is set to 0.35 with a st1 delay of 0.2\n"
    "This produces approximately 26.2% of single measurements.\n"
    "This configuration produce sin^2 correlation curve and produces a stable violation of Eberhard\n"
    "inequality with J/N averaging -0.008 (use the 'eval positivity' tool to check)\n"
    "It validates the fact that the fair sampling assumption cannot be used.\n\n";
  }
  else
  if (conf_id == 11)
  {
    init_config_1_qm_perfect();
    epr_dlg.sim.model_qm = false;
    epr_dlg.sim.model_local = true;
    epr_dlg.curves.graph_N = 10000;
    epr_dlg.source.em_prob_ent_pair = 0.97f;
    epr_dlg.source.em_prob_double = 0.03f;
    epr_dlg.source.emit_enable_jitter = true;
    epr_dlg.pairing.win_type_fixed = true;
    epr_dlg.pairing.win_type_mobile = false;
    epr_dlg.pairing.ignore_accidentals = true;
    epr_dlg.detectors.det_prob_alice_det_o = 0.75f;
    epr_dlg.detectors.det_prob_alice_det_e = 0.75f;
    epr_dlg.detectors.udet_dep_local_dpr = 0.35f;
    epr_dlg.detectors.udet_dep_local_q = 0.65f;

    epr_dlg.inequality.test_conf_a1 = 166.69f;
    epr_dlg.inequality.test_conf_a2 = 164.14f;
    epr_dlg.inequality.test_conf_b1 = 75.09f;
    epr_dlg.inequality.test_conf_b2 = 98.97f;
    epr_dlg.inequality.eval_posi_count = 50;
    epr_dlg.inequality.test_conf_N = 4000000;

    desc_txt =
    "11/ Local, Fixed windows pairing\n"
    "This configuration produces a stable violation using a fixed window pairing.\n"
    "Some counts are difficult to take into account.\n"
    "For example, how to take into account a single detection on the bob side with simultaneous\n"
    "detection on e and o on the Alice side.\n"
    "An option chosen in some experiments is to restart the measurement when that does not allow\n"
    "a normal count to be made.\n"
    "This is taken into account in the number of measurements carried out, but not its result\n"
    "which is ignored and counted as uu.\n"
    "This is supposed to have only an effect on the amplitude of J/N, but not on its sign.\n"
    "This is indeed true if these parasitic detections occur at random, due to local fluctuations\n"
    "at the level of the detectors. They are called 'accidentals'.\n"
    "But then we have to make an assumption of fair accidentals, which is similar to the assumption\n"
    "of fair sampling required with movable windows.\n\n"
    "This Configuration defines accidentals which are not produced by randomness at the detectors, but\n"
    "assuming that the source can emit a certain proportion of double pairs.\n"
    "This then produces double detections in the pairing windows which depend on the orientation\n"
    "of the polarizers.\n"
    "To get closer to real experiments, the configuration defines a detection efficiency of 0.75, and\n"
    "a time jitter at emission.\n"
    "The probability of emitting more than one pair is set to 0.03\n"
    "This produces 1.8% accidentals readings, and if these are counted as uu, generates stable\n"
    "violation of Eberhard inequality with J/N value around -0.001\n"
    "(use the 'eval positivity' tool to check)\n\n";
  }


  // init dialog
  dlg_init_dialog();
  // display config quick description
  co_printf("-----------------------------------------------------------------------\n"
            "Experiment selected : %s", desc_txt);
  dlg_run_thread_task(e_task_type_upd_graph, false, "Update curves");
}
