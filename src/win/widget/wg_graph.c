// graph widget
#include <string.h>
#include <stdlib.h>
#include "widget.h"
#include "wg_priv.h"

typedef struct
{
  widget_t wg;
  float x0;
  float x1;
  float y0;
  float y1;
  float x_units;
  float y_units;

  // axis draw config
  const struct wg_graph_axis_list *x_axis_list;
  const struct wg_graph_axis_list *y_axis_list;

  // curves list
  const struct wg_graph_curve **curve_list;

  // draw scale
  float dr_scale_x;
  float dr_scale_y;

  // colors
  int col_id;
  pix_t bk_color;
  pix_t grid_color;
  bool enable_aa;

  // post draw call back (user add text on graph)
  wg_graph_pos_draw_cb_t pos_draw_cb;
} wg_graph_t;

static void draw_graph(hwin_t *hw, wg_graph_t *gr);

static void wg_graph_ev_proc(hwin_t *hw, widget_t *wg)
{
  switch (hw->ev.type)
  {
    case EV_PAINT:
    {
      wg_graph_t *gr = (wg_graph_t *)wg;
      wg_draw_e_frame(hw, wg);
      draw_graph(hw, gr);
    }
    break;
    default:;
  }
}

static void wg_graph_size_proc(widget_t *wg)
{
  wg_graph_t *gr = (wg_graph_t *)wg;
  gr->dr_scale_x = (float)(wg->c_size.x)/(gr->x1 - gr->x0);
  gr->dr_scale_y = (float)(wg->c_size.y)/(gr->y1 - gr->y0);
}

widget_t *wg_init_graph(hwin_t *hw, wg_graph_pos_draw_cb_t pos_draw_cb)
{
  wg_graph_t *gr = C_NEW(wg_graph_t);
  if (gr)
  {
    init_dr_obj(&gr->wg, e_obj_none, NULL, COL_ND, NULL);
    wg_init(&gr->wg, wg_graph_ev_proc, WG_SHOW, &wgs.co_out.e_frm);
    gr->wg.sz_proc = wg_graph_size_proc;
    gr->pos_draw_cb = pos_draw_cb;
    return win_add_widget(hw, &gr->wg);
  }
  return &wg_void;
}

void wg_config_graph(widget_t *wg,
                     float x0, float x1, float y0, float y1,
                     float x_units, float y_units,
                     const struct wg_graph_axis_list *x_axis_list,
                     const struct wg_graph_axis_list *y_axis_list,
                     pix_t bk_color, pix_t grid_color)
{
  GET_WG(gr, graph);
  gr->x0 = x0;
  gr->x1 = x1;
  gr->y0 = y0;
  gr->y1 = y1;
  gr->x_units = x_units;
  gr->y_units = y_units;

  gr->x_axis_list = x_axis_list;
  gr->y_axis_list = y_axis_list;

  gr->bk_color = bk_color;
  gr->grid_color = grid_color;
  gr->enable_aa = true;
}

void wg_graph_def_curve_list(widget_t *wg, const struct wg_graph_curve **curve_list)
{
  GET_WG(gr, graph);
  gr->curve_list = curve_list;
}

void wg_graph_refresh(hwin_t *hw, widget_t *wg)
{
  GET_WG(gr, graph);
  draw_graph(hw, gr);
  win_cli_blit_rect(hw, wg->c_pos.x, wg->c_pos.y, wg->c_size.x, wg->c_size.y);
}

void wg_graph_select_color_id(hwin_t *hw, widget_t *wg, int crv_col_id, pix_t bk_color, pix_t grid_color, bool refresh)
{
  GET_WG(gr, graph);
  gr->col_id = crv_col_id;
  gr->bk_color = bk_color;
  gr->grid_color = grid_color;
  if (refresh)
    wg_graph_refresh(hw, wg);
}

void wg_graph_enable_aa(widget_t *wg, bool enable)
{
  GET_WG(gr, graph);
  gr->enable_aa = enable;
}

// ----------------------------------------------
// drawing

#define X_BM(x_gr) (int)((x_gr - gr->x0)*gr->dr_scale_x)
#define Y_BM(y_gr) (bm->size.y - (int)((y_gr - gr->y0)*gr->dr_scale_y))

static void graph_draw_grid(wg_graph_t *gr, bitmap_t *bm)
{
  double x_gr, y_gr;
  double dx = gr->x_units / 4.0;
  double dy = gr->y_units / 4.0;
  double x0_s = (int)(gr->x0 / dx)*dx;
  double y0_s = (int)(gr->y0 / dy)*dy;
  double x0_l = (int)(gr->x0 / gr->x_units)*gr->x_units;
  double y0_l = (int)(gr->y0 / gr->y_units)*gr->y_units;

  for (y_gr = y0_l; y_gr < gr->y1; y_gr += gr->y_units)
  {
    int y = Y_BM(y_gr);
    for (x_gr = x0_s; x_gr < gr->x1; x_gr += dx)
      bm_put_pixel(bm, X_BM(x_gr), y, gr->grid_color);
  }

  for (x_gr = x0_l; x_gr < gr->x1; x_gr += gr->x_units)
  {
    int x = X_BM(x_gr);
    for (y_gr = y0_s; y_gr < gr->y1; y_gr += dy)
      bm_put_pixel(bm, x, Y_BM(y_gr), gr->grid_color);
  }

  if (gr->x_axis_list)
  {
    double dy_a = gr->y_units / 8.0;
    double y0_a = (int)(gr->y0 / dy_a)*dy_a;
    int i;
    for (i=0; i<gr->x_axis_list->size; i++)
    {
      double y;
      int x = X_BM(gr->x_axis_list->pos[i]);
      for (y = y0_a; y < gr->y1; y += dy_a)
        bm_put_pixel(bm, x, Y_BM(y), gr->grid_color);
    }
  }

  if (gr->y_axis_list)
  {
    double dx_a = gr->x_units / 8.0;
    double x0_a = (int)(gr->x0 / dx_a)*dx_a;
    int i;
    for (i=0; i<gr->y_axis_list->size; i++)
    {
      double x;
      int y = Y_BM(gr->y_axis_list->pos[i]);
      for (x = x0_a; x < gr->x1; x += dx_a)
        bm_put_pixel(bm, X_BM(x), y, gr->grid_color);
    }
  }
}

// draw single curve
static void draw_curve(wg_graph_t *gr, bitmap_t *bm, const struct wg_graph_curve *crv)
{
  float x = crv->x0;
  float dx = ((crv->x1 - crv->x0)/(crv->data_count - 1))*crv->draw_stride;
  int float_stride = crv->data_stride_bytes / sizeof(float);
  int dy = float_stride*crv->draw_stride;
  const float *y = crv->y_data;
  const float *y_end = crv->y_data + crv->data_count*float_stride;
  pix_t crv_col = crv->color[gr->col_id];
  pix_t aa_col = crv->aa_color[gr->col_id];
  W_ASSERT(crv->data_stride_bytes == float_stride*sizeof(float));     // ensure stride is aligned on float

  if (!aa_col || !gr->enable_aa)
  {
    for (; y<y_end; y+=dy, x += dx)
    {
      int x_bm = X_BM(x);
      int y_bm = Y_BM(*y);
      if (     ((unsigned int)y_bm < (unsigned int)bm->size.y)
            && ((unsigned int)x_bm < (unsigned int)bm->size.x))
        *BM_PIX_ADDR(bm, x_bm, y_bm) = crv_col;
    }
  }
  else
  {
    int x_max = bm->size.x-1;
    int y_max = bm->size.y-1;
    pix_t bk_color = gr->bk_color;
    if (aa_col == -1)                   // light background
    {
#if 0
      int r = ((crv_col &     0xff) + (bk_color &     0xff)*7)/8;
      int g = ((crv_col &   0xff00) + (bk_color &   0xff00)*7)/8;
      int b = ((crv_col & 0xff0000) + (bk_color & 0xff0000)*7)/8;
      aa_col = (r & 0xff) | (g & 0xff00) | (b & 0xff0000);
#else
      // aa_col = ((crv_col & 0xf8f8f8) + 7*(bk_color & 0xf8f8f8)) >> 3;  // auto defined
      aa_col = ((crv_col & 0xf0f0f0) + 15*(bk_color & 0xf0f0f0)) >> 4;  // auto defined (light background)
#endif
    }
    else
    if (aa_col == -2)                  // dark background
      aa_col = ((crv_col & 0xf0f0f0)*6 + 10*(bk_color & 0xf0f0f0)) >> 4;  // auto defined (dark background)

    for (; y<y_end; y+=dy, x += dx)
    {
      int x_bm = X_BM(x);
      int y_bm = Y_BM(*y);
      if ((y_bm > 0) && (y_bm < y_max) && (x_bm > 0) && (x_bm < x_max))
      {
        pix_t *p = BM_PIX_ADDR(bm, x_bm, y_bm);
        *p = crv_col;
        if (p[-1] == bk_color) p[-1] = aa_col;
        if (p[ 1] == bk_color) p[ 1] = aa_col;
        if (p[-bm->l_size] == bk_color) p[-bm->l_size] = aa_col;
        if (p[ bm->l_size] == bk_color) p[ bm->l_size] = aa_col;
#if 0
        if (p[-1-bm->l_size] == bk_color) p[-1-bm->l_size] = aa_col;
        if (p[ 1-bm->l_size] == bk_color) p[ 1-bm->l_size] = aa_col;
        if (p[-1+bm->l_size] == bk_color) p[-1+bm->l_size] = aa_col;
        if (p[ 1+bm->l_size] == bk_color) p[ 1+bm->l_size] = aa_col;
#endif
      }
    }
  }
}

// draw full graph
static void draw_graph(hwin_t *hw, wg_graph_t *gr)
{
  bitmap_t bm;
  widget_t *wg = &gr->wg;
  bm_init_child(&bm, &hw->cli_bm, wg->c_pos.x, wg->c_pos.y, wg->c_size.x, wg->c_size.y);
  bm_paint(&bm, gr->bk_color);
  graph_draw_grid(gr, &bm);

  if (gr->curve_list)
  {
    const struct wg_graph_curve **crv;
    for (crv = gr->curve_list; *crv; crv++)
      if ((*crv)->show && (*crv)->data_count)
        draw_curve(gr, &bm, *crv);
  }

  if (gr->pos_draw_cb)
    gr->pos_draw_cb(hw, wg, &bm);
}

