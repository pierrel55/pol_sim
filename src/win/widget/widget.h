// base window objects
#include "../win.h"

// clear alloc
#define C_NEW(type) (type *)W_CALLOC(1, sizeof(type))
#define GET_WD win_data_t *wd = win_get_data()   // declare local on window datas

// widget frame
typedef struct
{
  int width;
  const pix_t *tl;                               // top left color list
  const pix_t *br;                               // botom right color list
} frm_t;

typedef struct                                   // client frame, have 2 states, pressed/unpressed colors
{
  frm_t frm_u;
  frm_t frm_p;
} c_frm_t;

typedef struct
{
  pix_t t_color;
  pix_t aa_color;
  pix_t bk_color;
} text_color_t;

typedef struct
{
  enum e_font_id text_font_id;
  text_color_t u_col;                            // unselect color
  text_color_t s_col;                            // select color
  pix_t h_line[2];                               // horizontal line color
} menu_style_t;

extern text_color_t tu_color;                    // no selection text color
extern text_color_t ts_color;                    // selection text color

// widget style configuration
#define COL_ND COL_RGB(1, 0, 1)                  // not drawn color

struct wg_style_t
{
  pix_t clear_bk_color;                          // clear background color
  pix_t txt_en_bk_color;                         // enabled text back color
  pix_t text_aa_color;                           // anti aliasing color
  pix_t txt_dis_color;                           // text disable color
  pix_t txt_dis_color_s;                         // text shifted disable color
  pix_t bt_ctrl_color;                           // contol buttons char color

  struct                                         // selected text
  {
    pix_t t_color;
    pix_t aa_color;
    pix_t bk_color;
  } text_select;

  struct
  {
    enum e_font_id text_font_id;
    text_color_t msg_color;
  } msg_box;

  menu_style_t menu_combo;
  menu_style_t menu_ctx;                         // right click menu

  // check box frame
  frm_t frm_check_box;

  // ------------------------
  // slider (scroll bar)
  struct
  {
    pix_t slider_bk_color;                       // back color of sliders (scroll bar)
    frm_t bt_frm;                                // button frame
    int bt_width;                                // default bouton width
  } slider;
  // ------------------------
  // combo box menu
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
    frm_t e_frm;
  } menu_cb;
  // ------------------------
  // wg_text
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
  } text;
  // ------------------------
  // wg_frame
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
    frm_t e_frm;
  } frame;
  // ------------------------
  // wg_text_button
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
    c_frm_t c_frm;
  } text_button;
  // ------------------------
  // wg_combo_box
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
    pix_t text_bk_color;
    frm_t e_frm;
    c_frm_t c_frm_bt_down;
  } combo_box;
  // ------------------------
  // wg_edit_box
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
    pix_t text_bk_color;
    pix_t cursor_color;
    pix_t text_select_bk_color;
    frm_t e_frm;
  } edit_box;
  // ------------------------
  // wg_progress_bar
  struct
  {
    pix_t fill_color;
    frm_t e_frm;
  } progress_bar;
  // ------------------------
  // wg_console_out
  struct
  {
    enum e_font_id text_font_id;
    pix_t text_color;
    pix_t text_aa_color;
    pix_t text_bk_color;
    frm_t e_frm;
  } co_out;
};

extern struct wg_style_t wgs;

// text util
typedef struct
{
  const char *str;
  const font_t *font;
  pix_t color;
  pix_t aa_color;                                // anti-alias color, use 0 if none
  vec2i size;
  vec2i pos;
} text_t;

// some base draw object type
enum e_dr_obj_typ
{
  e_obj_none = 0,
  e_obj_text,
  e_obj_char,
  e_obj_bitmap,
};

typedef struct dr_obj_t
{
  enum e_dr_obj_typ type;                        // type of object to draw
  pix_t bk_color;                                // clear back color
  union
  {
    const void *obj;
    text_t *text;
    const uint32_t *ch;
    const bitmap_t *bm;
  };
} dr_obj_t;

typedef void (* wg_ev_proc_t)(hwin_t *hw, struct widget_t *wg);
typedef void (* wg_sz_proc_t)(struct widget_t *wg);

typedef struct widget_t
{
  wg_ev_proc_t ev_proc;                          // event proc, define widget type
  wg_sz_proc_t sz_proc;                          // size proc, optional
  int flags;                                     // WG_xxx flags
  vec2i e_pos;                                   // external position in parent
  vec2i e_size;                                  // external size including frame
  frm_t *e_frm;                                  // external frame NULL if none
  vec2i c_pos;                                   // client position in parent
  vec2i c_size;                                  // client size
  c_frm_t *c_frm;                                // client frame (buttons) NULL if none
  dr_obj_t dr_obj;                               // object to draw in client area
  const char *help_text;                         // bubble help text
  struct widget_t *i_wg;                         // widget to insert into client area (chained list 1st)
  struct widget_t *next;                         // same tree level chained list
  struct widget_t *wg_parent;                    // parent or linked widget is used
} widget_t;

// ----------------------------------------------
// place/size utils (code in wg_wl.c)

// size set
void wg_set_size(widget_t *wg, int size_x, int size_y);

// position set
void wg_set_pos(widget_t *wg, int x, int y);
void wg_set_pos_x(widget_t *wg, int x);
void wg_set_pos_y(widget_t *wg, int x);

// x position utils
void wg_align_left(widget_t *wg, int l_margin);
void wg_align_right(widget_t *wg, int r_margin, int x_width);
void wg_center(widget_t *wg, int x_width);
void wg_center_between(widget_t *wg, int l_margin, int r_margin, int x_width);

// set size and position in window
void wg_set_pos_size(widget_t *wg, int pos_x, int pos_y, int size_x, int size_y);

// relative placement using stack
void wl_init(vec2i *pos_ini, int max_wx, bool get_size);
vec2i *wl_pos(void);
int *wl_wx(void);
void wl_get_size(vec2i *size);
int wl_size_x(void);

void wl_push(int dx, int dy);
void wl_pop(int y_margin);

void wl_set_push_frm(int push_frm_dx, int push_frm_dy, int pop_frm_dy);
void wl_push_frm(widget_t *wg);
void wl_pop_frm(void);

void wl_place_set_y_margin(int y_margin);
void wl_place(widget_t *wg, int wy);
void wl_place_center(widget_t *wg, int wy);
void wl_place_2h(widget_t *wg0, int dx0, widget_t *wg1, int wy);
void wl_place_2h_r(widget_t *wg0, widget_t *wg1, int w1, int wy);
void wl_place_2h_r_c0(widget_t *wg0, widget_t *wg1, int w1, int wy);
void wl_place_2h_spc_r(widget_t *wg0, int spc, widget_t *wg1, int w1, int wy);
void wl_place_2h_center(widget_t *wg0, int w0, widget_t *wg1, int w1, int wy);
void wl_place_3h_center(widget_t *wg0, widget_t *wg1, widget_t *wg2, int w_ma, int wy);
void wl_place_4h_center(widget_t *wg0, widget_t *wg1, widget_t *wg2, widget_t *wg3, int w_ma, int wg3_w_inc, int wy);

// horizontal placements
void wl_place_h_first(widget_t *wg, int wx, int wy);
void wl_place_h_next(widget_t *wg, int x_margin, int wx, int wy, bool is_last);

// ----------------------------------------------
// widget generic

// dispatch widget events
void widget_dispatch_events(hwin_t *hw);

// show/hide widget
void wg_show(hwin_t *hw, widget_t *wg, bool show);

// get pressed/checked state (buttons/check box/radio buttons)
bool wg_get_state(widget_t *wg);

// set pressed/checked state (buttons/check box/radio buttons)
void wg_set_state(hwin_t *hw, widget_t *wg, bool state);

enum e_text_align
{
  wg_ta_none = 0,                // left align, no margin
  wg_ta_left_ma,                 // left align + margin
  wg_ta_center,                  // centered
  wg_ta_right,                   // right align
  wg_ta_right_ma,                // right align + margin
};

// set alignment of text in client area
void wg_set_text_align(widget_t *wg, enum e_text_align text_align);

// ----------------------------------------------
// enable

// init in disable state (default is enable)
void wg_init_disable(widget_t *wg);

// enable/disable
void wg_enable(hwin_t *hw, widget_t *wg, bool enable);

// enable all windget in list (NULL terminated) and disable others.
void wg_enable_list_only(hwin_t *hw, widget_t **wg_list);

// enable/disable all windget in list (NULL terminated).
void wg_enable_list(hwin_t *hw, widget_t **wg_list, bool enable);

// restore list initial states
void wg_enable_restore_backup(hwin_t *hw);

// ----------------------------------------------
// focus enable

// allow focus only for list (thread in progress)
void wg_focus_list_only(hwin_t *hw, widget_t **wg_list);

// restore focus state before call to wg_focus_list_only
void wg_focus_list_restore(hwin_t *hw);

// ----------------------------------------------
// graph, special widget to draw curves

struct wg_graph_axis_list
{
  int size;
  float pos[];
};

typedef void (*wg_graph_pos_draw_cb_t)(hwin_t *hw, widget_t *wg, bitmap_t *bm);

struct wg_graph_curve
{
  int user_id;                 // user free usage id
  const char *name;            // curve name, can be NULL if unused
  bool show;                   // show/hide
  pix_t color[2];              // draw color
  pix_t aa_color[2];           // aa color. 0:none -1:auto else value
  float x0, x1;                // x range in graph units
  int data_count;              // count of y values in y_data
  int data_stride_bytes;       // stride in bytes between y_data
  int draw_stride;             // (1..) stride in datas used to draw curve
  float *y_data;               // y drawing datas
};

widget_t *wg_init_graph(hwin_t *hw, wg_graph_pos_draw_cb_t pos_draw_cb);

// note: x_axis_list & y_axis_list can be NULL
void wg_config_graph(widget_t *wg,
                     float x0, float x1, float y0, float y1,
                     float x_units, float y_units, 
                     const struct wg_graph_axis_list *x_axis_list,
                     const struct wg_graph_axis_list *y_axis_list,
                     pix_t bk_color, pix_t grid_color);

void wg_graph_def_curve_list(widget_t *wg, const struct wg_graph_curve **curve_list);
void wg_graph_refresh(hwin_t *hw, widget_t *wg);
void wg_graph_select_color_id(hwin_t *hw, widget_t *wg, int crv_col_id, pix_t bk_color, pix_t grid_color, bool refresh);
void wg_graph_enable_aa(widget_t *wg, bool enable);

// ----------------------------------------------
// message box

// message box type
enum e_msg_box_type
{
  msg_box_ok = 0,                                // ok button only
  msg_box_ok_cancel,                             // ok + cancel buttons
  msg_box_yes_no,                                // yes + no buttons
  msg_box_yes_no_cancel,                         // yes + no + cancel buttons
};

// message box result
enum e_msg_box_res
{
  msg_res_cancel = 0,
  msg_res_ok,
  msg_res_yes,
  msg_res_no
};

// numeric value descriptor (for edit_box_num widget)
typedef struct
{
  float init_value;
  float min_value;
  float max_value;
  float inc_step;                                // button press icrement/decrement value
  int max_deci;                                  // edit max decimal digits
  int spc_digi;                                  // spaces in big numbers (integers) ex: 1000000 => 1 000 000 (for spc_digi = 3)
  const char *help_text;                         // description text
} num_desc_t;

void num_desc_init(num_desc_t *nd, float init_value, float min_value, float max_value, float inc_step, int max_deci, int spc_digi, const char *help_text);

// ----------------------------------------------
// widget callback definition
typedef void (*msg_box_resume_t)(hwin_t *hw, enum e_msg_box_res res);
typedef void (*win_menu_select_cb_t)(hwin_t *hw, void *user_ptr, int sel_id);
typedef void (*button_pressed_cb_t)(hwin_t *hw, widget_t *wg);
typedef void (*combo_box_select_cb_t)(hwin_t *hw, int sel_id);
typedef void (*edit_box_return_pressed_cb_t)(hwin_t *hw, widget_t *wg);
typedef void (*edit_box_num_value_changed_cb_t)(widget_t *wg, float value);
typedef void (*slider_moved_cb_t)(hwin_t *hw, widget_t *wg, int move_ofs);

// ----------------------------------------------
// widget specific functions

// message box
hwin_t *wg_message_box(const char *message, const char *win_title, enum e_msg_box_type box_type, hwin_t *parent, msg_box_resume_t resume_proc);

// menu
bool wg_menu_init_ex(int x, int y, int dx_min, int dx_max, int w_expand,
                     enum e_win_border_type border_type, const menu_style_t *menu_style,
                     const char **str_list, int str_count, int l_margin, int dy_line, 
                     win_menu_select_cb_t select_cb, hwin_t *parent, void *user_ptr);
bool wg_menu_ctx_init(int x, int y, int dx_max, const char **str_list, int str_count, 
                     win_menu_select_cb_t select_cb, hwin_t *parent, void *user_ptr);

// image
widget_t *wg_init_image(hwin_t *hw, const bitmap_t *img_bm); // image to display

// text
widget_t *wg_init_text(hwin_t *hw, const char *str);
widget_t *wg_init_text_ex(hwin_t *hw, const char *str, const font_t *font, pix_t color, pix_t aa_color);
void wg_text_update(hwin_t *hw, widget_t *wg, const char *str);
void wg_text_update_ex(hwin_t *hw, widget_t *wg, font_t *font, pix_t color, pix_t aa_color, const char *str);

// window frame: frame with title
widget_t *wg_init_frame_ex(hwin_t *hw, const char *title, const font_t *font, pix_t color, pix_t aa_color);
widget_t *wg_init_frame(hwin_t *hw, const char *title);

// text button
widget_t *wg_init_text_button(hwin_t *hw, const char *name, button_pressed_cb_t pressed_cb);
void wg_text_button_uptate(hwin_t *hw, widget_t *tb, const char *name);

// check box
widget_t *wg_init_check_box(hwin_t *hw, const char *name, button_pressed_cb_t check_cb);

// radio button
widget_t *wg_init_radio_button(hwin_t *hw, const char *name, button_pressed_cb_t check_cb);

// combo box
widget_t *wg_init_combo_box(hwin_t *hw, const char **str_list, int str_count, int sel_id, int l_margin,
                              int dy_line, int w_expand, combo_box_select_cb_t combo_select_cb);
int wg_combo_get_select_id(widget_t *wg);
void wg_combo_set_select_id(hwin_t *hw, widget_t *wg, int sel_id);
void wg_combo_update_list_size(hwin_t *hw, widget_t *wg, int str_count);

// edit box
widget_t *wg_init_edit_box(hwin_t *hw, char *str_edit, int str_edit_buff_size, edit_box_return_pressed_cb_t return_cb);
void wg_edit_box_set_text(hwin_t *hw, widget_t *wg, char *str_edit, int str_edit_buff_size);

// spin control (up/down buttons)
widget_t *wg_init_spin_button(hwin_t *hw, button_pressed_cb_t pressed_cb_up, button_pressed_cb_t pressed_cb_dn);

// edit box + integrated spin control
widget_t *wg_init_edit_spin_box(hwin_t *hw, char *str_edit, int str_edit_buff_size, edit_box_return_pressed_cb_t return_cb, 
                                  button_pressed_cb_t pressed_cb_up, button_pressed_cb_t pressed_cb_dn);
widget_t *wg_init_edit_box_num(hwin_t *hw, num_desc_t *num_desc, bool add_spin, edit_box_num_value_changed_cb_t value_changed_cb);
void wg_edit_box_num_set_value(hwin_t *hw, widget_t *wg, float n);
float wg_edit_box_num_get_value(widget_t *wg);
void wg_edit_box_num_update_min_max(hwin_t *hw, widget_t *wg, float min, float max);

// progress bar
widget_t *wg_init_progress_bar(hwin_t *hw, int progress_100, pix_t fill_color);
void wg_progress_bar_set_color(widget_t *wg, pix_t fill_color);
void wg_progress_bar_set_ratio(hwin_t *hw, widget_t *wg, int progress_100);

// console output
widget_t *wg_init_console_out(hwin_t *hw, int max_lines);
void wg_console_clear(hwin_t *hw, widget_t *wg);
void wg_console_copy_to_clipboard(hwin_t *hw, widget_t *wg);
void wg_console_print(hwin_t *hw, widget_t *wg, const char *str);
void wg_console_printf(hwin_t *hw, widget_t *wg, const char *fmt, ...);

// slider y
widget_t *wg_init_vslider(hwin_t *hw, int move_range, int move_ofs, int move_step, slider_moved_cb_t moved_cb);
void wg_vslider_set_range(hwin_t *hw, widget_t *wg, int move_range);
int wg_vslider_get_ofs(widget_t *wg);
void wg_vslider_set_ofs(hwin_t *hw, widget_t *wg, int move_ofs, bool call_cb);
void wg_vslider_set_range_ofs(hwin_t *hw, widget_t *wg, int move_range, int move_ofs, bool call_cb, bool blit);
bool wg_slider_control(widget_t *wg);

// vertical scroll bar
widget_t *wg_get_scroll_bar_v(int move_range, int move_ofs, int move_step, slider_moved_cb_t moved_cb);
void wg_insert_scroll_bar_v(widget_t *wg, int move_range, int move_ofs, int move_step, slider_moved_cb_t moved_cb);
widget_t *wg_init_scroll_bar_v(hwin_t *hw, int move_range, int move_ofs, int move_step, slider_moved_cb_t moved_cb);
