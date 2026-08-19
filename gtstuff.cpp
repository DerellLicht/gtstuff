//****************************************************************************
//  Copyright (c) 1985-2026  Derell Licht
//  Demo program for resizeable-dialog applications
//
//  Written by:  Dan Miller
//****************************************************************************

#define  USE_WIDTH_RESIZE

static const char *Version = "GTstuff program, Version 1.01" ;

#include <windows.h>
#include <stdio.h>   //  vsprintf, sprintf, which supports %f

#include "resource.h"
#include "common.h"
#include "commonw.h"
#include "gtstuff.h"
#include "gfuncs.h"
#include "alg_selector.h"
#include "config.h"
#include "statbar.h"
#include "winmsgs.h"

//***********************************************************************
static HINSTANCE g_hinst = 0;

static HWND hwndMain ;
static HWND hwndGFrame = nullptr ;

//  Claude 08/17/26 - see the comment on this prototype in gtstuff.h
HWND get_hwndGFrame(void)
{
   return hwndGFrame ;
}
static HWND hwndGInfo = nullptr ;

//  both of these fields are used by config.cpp to update ini file
uint cxClient = 0 ;
uint cyClient = 0 ;

// static CStatusBar *MainStatusBar = NULL;
static std::unique_ptr<CStatusBar> MainStatusBar {};

//*******************************************************************
//  *** BEGIN Claude resize data block
//*******************************************************************

//  frame dimensions for graphics frame
uint cxGFrame = 0 ;
uint cyGFrame = 0 ;

// Claude 08/14/26 - smallest listview height (pixels) we'll allow the
// live-resize floor to shrink down to, so a few rows stay visible/usable
// no matter how far the user drags the bottom edge up.
#define  MIN_LISTVIEW_VISIBLE_DY   80

// Claude 08/14/26
// MINMAXINFO's ptMinTrackSize/ptMaxTrackSize are WINDOW (outer) dimensions,
// not client-area dimensions -- but cxClient/cyClient come from GetClientRect(),
// which excludes the caption/border. Pinning ptMinTrackSize.x==ptMaxTrackSize.x
// directly to cxClient tells Windows "the whole window, borders included, is
// only as wide as the client area" -- i.e. a few pixels *too narrow* by exactly
// the border width. That's the "shrinks by a few pixels on first width-drag"
// symptom. Fix: measure the real window-minus-client delta once at init, and
// add it back in whenever a track size is derived from a client dimension.
static int dx_frame = 0;   //  window width  - client width
static int dy_frame = 0;   //  window height - client height

// Claude 08/14/26 - term_window_height tracks the LISTVIEW's current height
// and gets recalculated on every resize (see resize_dialog_and_workspace). It is NOT
// a safe floor for WM_GETMINMAXINFO, because by the time a live drag is
// underway its value has already moved. min_application_window_height is
// the true floor: computed once in do_init_dialog from the fixed pieces
// (top controls + a minimum usable listview height + status bar + frame)
// and never modified afterward.
static uint min_application_window_height = 0;

#ifdef USE_WIDTH_RESIZE
// Claude 08/16/26 - width-resize floor, same shape as min_application_window_height
// above. MIN_TERMINAL_VISIBLE_DX is a placeholder client-width floor (pixels) --
// this app has no existing "minimum usable width" concept the way it has
// MIN_LISTVIEW_VISIBLE_DY, so pick a real value based on what your top control
// row actually needs (e.g. right edge of your last button + margin) rather than
// trusting this default.
#define  MIN_TERMINAL_VISIBLE_DX   200
static uint min_application_window_width = 0;
#endif

// Claude 08/17/26 - resize anchors for hwndGFrame (the SS_BLACKFRAME graphics
// placeholder). gframe_left and gframe_right_margin are captured once, in
// do_init_dialog, from the control's as-designed position in the .rc file --
// gframe_left is the fixed left inset, gframe_right_margin is however much
// space (if any) the .rc left between the control's right edge and the
// dialog's client edge. Both stay constant afterward; resize_gframe() re-solves
// the frame's width from them and the current cxClient on every resize, the
// same way MainStatusBar spans cxClient. Top/bottom don't need captured
// anchors -- they're re-derived each time from get_terminal_top() and
// MainStatusBar->height(), which are already the authoritative values for
// "bottom of button row" and "top of status bar".
static int gframe_left = 0;
static int gframe_right_margin = 0;

// Claude 08/17/26 - CreateDialog() sends WM_INITDIALOG *before* the dialog
// window is actually shown (even with WS_VISIBLE in the .rc -- it's shown
// afterward, once WM_INITDIALOG returns). GetDC() drawing against a window
// that isn't shown yet has no visible clip region, so any draw attempted
// during do_init_dialog() has nothing to land on and is silently lost. This
// flag lets DialogProc's WM_PAINT case do the real first draw once, on the
// dialog's genuine first paint, when it's actually on screen.
static bool gframe_drawn_once = false;

//*******************************************************************
//  *** END Claude resize data block
//*******************************************************************

//*******************************************************************
//lint -esym(714, status_message)
//lint -esym(759, status_message)
//lint -esym(765, status_message)
void status_message(char *msgstr)
{
   MainStatusBar->show_message(msgstr);
}

void status_message(uint idx, char *msgstr)
{
   MainStatusBar->show_message(idx, msgstr);
}

void show_graph_desc(char const *desc)
{
   SetWindowText(hwndGInfo, desc) ;
}

//****************************************************************************
//  small font-dependent layout fudge factor; shared by do_init_dialog's
//  min-height calculation and resize_dialog_and_workspace's live layout so the two
//  stay consistent with each other.
//****************************************************************************
static int get_dy_offset(void)
{
   return 0 ;
}

//****************************************************************************
static uint get_terminal_top(void)
{
   static uint local_ctrl_top = 0 ;
   if (local_ctrl_top == 0) {
      local_ctrl_top = get_bottom_line(hwndMain, IDB_CLOSE) ;
      local_ctrl_top += 3 ;
      // syslog("CommPort: ctrl_top = %u, or %u\n", local_ctrl_top, win_ctrl_top+3) ;
   }
   return local_ctrl_top ;
}  //lint !e715

//***********************************************************************
//  Claude 08/17/26 - the menu is now attached via IDD_MAIN_DIALOG's own
//  MENU statement in gtstuff.rc, so it exists from CreateDialog() time --
//  no setup_main_menu()/SetMenu() call needed here at all. That's also why
//  this doesn't disturb the status bar the way a runtime SetMenu() call
//  previously did: cxClient/cyClient (below, and in do_init_dialog) are
//  measured *after* the menu already exists, so they're never stale.
//***********************************************************************

//****************************************************************************
//  Claude 08/16/26 - status-bar part boundaries are computed as a proportion
//  of cxClient. Under fixed width this only ever needs to run once (at init),
//  but under USE_WIDTH_RESIZE it needs to be redone whenever width changes --
//  split out of do_init_dialog so resize_dialog_and_workspace() can call it too.
//****************************************************************************
static void update_statusbar_parts(void)
{
   int sbparts[3];
   sbparts[0] = (int) (6 * cxClient / 10) ;
   sbparts[1] = (int) (8 * cxClient / 10) ;
   sbparts[2] = -1;
   MainStatusBar->SetParts(3, &sbparts[0]);
}

//****************************************************************
//  Claude 08/15/26 - restore previously-saved window size/position
//  from the .ini file. client_height/window_left/window_top were
//  populated by init_config() above (which creates a default config
//  file if one doesn't exist yet, so these are always valid here --
//  no first-run guard needed).
//  Claude 08/16/26 - width is only ever saved/restored under
//  USE_WIDTH_RESIZE; otherwise it stays locked to the dialog's fixed
//  layout, same as before.
//****************************************************************
static void restore_dialog_settings(HWND hwnd)
{
#ifdef USE_WIDTH_RESIZE
   uint restored_win_width  = client_width + (uint) dx_frame ;
#else
   uint restored_win_width  = cxClient + (uint) dx_frame ;   //  width never changes
#endif
   uint restored_win_height = client_height + (uint) dy_frame ;

#ifdef USE_WIDTH_RESIZE
   //  clamp width to the same bounds WM_GETMINMAXINFO enforces --
   //  screen resolution may have changed since this was last saved
   if (restored_win_width < min_application_window_width) {
      restored_win_width = min_application_window_width ;
   }
   uint max_win_width = (uint) get_screen_width() ;
   if (restored_win_width > max_win_width) {
      restored_win_width = max_win_width ;
   }
#endif

   //  clamp height to the same bounds WM_GETMINMAXINFO enforces --
   //  screen resolution may have changed since this was last saved
   if (restored_win_height < min_application_window_height) {
      restored_win_height = min_application_window_height ;
   }
   uint max_win_height = (uint) get_screen_height() ;
   if (restored_win_height > max_win_height) {
      restored_win_height = max_win_height ;
   }

   //  clamp position to the current monitor (get_screen_width/height
   //  reflect get_monitor_dimens(hwnd), already called above) so a saved
   //  position from a monitor that's since been unplugged, or a screen
   //  res that's since shrunk, doesn't put us off-screen
   uint restored_left = window_left ;
   uint restored_top  = window_top ;
   uint scr_cx = (uint) get_screen_width() ;
   uint scr_cy = (uint) get_screen_height() ;
   if (restored_left + restored_win_width > scr_cx) {
      restored_left = (restored_win_width < scr_cx) ? (scr_cx - restored_win_width) : 0 ;
   }
   if (restored_top + restored_win_height > scr_cy) {
      restored_top = (restored_win_height < scr_cy) ? (scr_cy - restored_win_height) : 0 ;
   }

   //  applying this here (after all child controls exist) triggers
   //  WM_SIZE synchronously, which runs resize_dialog_and_workspace() and lays
   //  out the status bar/listview/etc. for the restored height --
   //  no separate relayout call needed
   SetWindowPos(hwnd, NULL, (int) restored_left, (int) restored_top,
      (int) restored_win_width, (int) restored_win_height, SWP_NOZORDER) ;
}

//****************************************************************************
//  Claude 08/17/26 - capture hwndGFrame's as-designed left inset and right
//  margin, once, from its live position right after it's fetched from the
//  dialog template. Must run while cxClient still holds the dialog's initial
//  client width (i.e. before restore_dialog_settings can change it).
//****************************************************************************
static void capture_gframe_layout(void)
{
   RECT r ;
   GetWindowRect(hwndGFrame, &r) ;
   MapWindowPoints(HWND_DESKTOP, hwndMain, (LPPOINT) &r, 2) ;
   gframe_left         = r.left ;
   gframe_right_margin = (int) cxClient - r.right ;
}

//****************************************************************************
//  Claude 08/17/26 - fills and borders hwndGFrame's client area. This is
//  frame housekeeping only -- actual demo content (including the intro
//  placeholder X, now menu_items[0]'s draw_func in alg_selector.cpp) is
//  drawn separately, dispatched by display_current_operation() from
//  WinMain's idle loop. Called from GFrameSubclassProc's WM_PAINT below --
//  see that function's comment for why hwndGFrame needs to be subclassed at
//  all, rather than just calling this after every resize.
//****************************************************************************
//  Derell Licht note:
//  I'm not really sure this function belongs here; 
//  gtstuff.cpp handles just the Windows interface functions;
//  graphics operations are done in the gobjects or in alg_selector.cpp .
//  Well, for now, leave it here...
//****************************************************************************
static void draw_gframe_contents(void)
{
   RECT rect ;
   GetClientRect(hwndGFrame, &rect) ;
   if (rect.right <= rect.left  ||  rect.bottom <= rect.top) {
      return ;   //  nothing to draw yet (frame not sized)
   }

   //  Claude 08/17/26 - GetClientRect() always returns left=0, top=0 by
   //  definition (client coordinates are relative to the client area's own
   //  origin), so right/bottom *are* the frame's width/height. This is the
   //  one place that needs to know that -- every draw call below already
   //  works in hwndGFrame's own coordinate space, so gfuncs.cpp routines
   //  never need to see it. Set here (and only here) so cxGFrame/cyGFrame
   //  are guaranteed current on every repaint, without a separate sync step.
   cxGFrame = (uint) rect.right ;
   cyGFrame = (uint) rect.bottom ;

   HDC hdc = GetDC(hwndGFrame) ;

   //  clear the interior first -- neither the border below nor a plain
   //  static's default painting erase anything meaningful for us, so
   //  without this, old content piles up instead of being replaced.
   HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE)) ;
   FillRect(hdc, &rect, hBrush) ;
   DeleteObject(hBrush) ;

   //  the frame border, drawn by us (see .rc note next to IDC_GFRAME for
   //  why this isn't SS_BLACKFRAME).
   Box(hdc, rect.left, rect.top, rect.right - 1, rect.bottom - 1, (COLORREF) WIN_BLACK) ;

   ReleaseDC(hwndGFrame, hdc) ;
   GdiFlush() ;   //  commit immediately rather than risk a deferred/batched paint

   //  Claude 08/17/26 - the canvas above just got wiped, regardless of why
   //  this ran (an actual resize via resize_gframe(), or any other
   //  incidental repaint -- window uncovered, restored from minimized,
   //  etc.). Flag that whatever's currently selected needs to redraw, same
   //  as change_graph_state() does on a menu switch.
   //
   //  Claude 08/17/26 - deliberately NOT also calling
   //  display_current_operation() directly here: tried that (see
   //  conversation history), but for a continuously-running demo it means
   //  every single WM_SIZE during a live drag does wipe-then-one-frame,
   //  which looks worse than the blank-during-drag this reverts to. A
   //  blank frame during an active resize is an accepted tradeoff --
   //  resizing isn't a continuous-use mode, and content resumes the instant
   //  the drag ends and the idle loop gets a turn again.
   we_should_redraw = 1 ;
}

// Claude 08/17/26 - hwndGFrame's original (Static class) window procedure,
// saved by SetWindowLongA(GWL_WNDPROC) in do_init_dialog so GFrameSubclassProc
// below can chain to it for every message it doesn't handle itself.
static WNDPROC gframe_orig_proc = nullptr ;

//****************************************************************************
//  Claude 08/17/26 - subclass proc for hwndGFrame.
//
//  Why this exists at all: a plain (un-subclassed) child control repaints
//  itself on Windows' own schedule, completely independent of anything we
//  do -- window activation, another window dragging over it and away, the
//  live-resize "settle" pass when the mouse button comes up, etc. all send
//  it a fresh WM_PAINT. Since hwndGFrame is a bare SS_LEFT static, its
//  *default* WM_PAINT just erases to background and draws nothing -- so any
//  of those incidental repaints silently wipes out whatever 
//  draw_gframe_contents() last drew, with no relationship to our own 
//  resize_gframe() calls at all. That's what was causing content to vanish 
//  (sometimes entirely, sometimes just an edge) even well after a resize 
//  had finished and even on first show.
//
//  Fix: make our own drawing the control's *actual* WM_PAINT handler, so it
//  reruns on every repaint Windows asks for, not just the ones we trigger.
//****************************************************************************
static LRESULT CALLBACK GFrameSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
   case WM_PAINT:
      {
      PAINTSTRUCT ps ;
      BeginPaint(hwnd, &ps) ;
      draw_gframe_contents() ;
      EndPaint(hwnd, &ps) ;
      }
      return 0 ;

   case WM_ERASEBKGND:
      //  draw_gframe_contents() already fills the interior every time it
      //  runs (from WM_PAINT above) -- skip the redundant separate erase.
      return 1 ;
   }
   return CallWindowProcA(gframe_orig_proc, hwnd, message, wParam, lParam) ;
}

//****************************************************************************
//  Claude 08/17/26 - (re)positions hwndGFrame to fill the space between the
//  button row and the status bar, spanning cxClient the same way MainStatusBar
//  does, then forces an immediate repaint through GFrameSubclassProc rather
//  than waiting for Windows to eventually get around to it on its own.
//  Called once from do_init_dialog and again from resize_dialog_and_workspace()
//  on every actual resize. Requires MainStatusBar to already exist.
//****************************************************************************
static void resize_gframe(void)
{
   if (hwndGFrame == nullptr) {
      return ;
   }

   int top    = (int) get_terminal_top() ;
   int bottom = (int) cyClient - (int) MainStatusBar->height() - 3 ;
   int width  = (int) cxClient - gframe_right_margin - gframe_left ;
   int height = bottom - top ;
   if (width  < 1) { width  = 1 ; }
   if (height < 1) { height = 1 ; }

   MoveWindow(hwndGFrame, gframe_left, top, width, height, TRUE) ;
   InvalidateRect(hwndGFrame, NULL, TRUE) ;   //  whole client area, not just the resize delta
   UpdateWindow(hwndGFrame) ;                 //  process it now -- runs GFrameSubclassProc's WM_PAINT
}


//***********************************************************************
static void do_init_dialog(HWND hwnd)
{
   char msgstr[81] ;
   // hwndTopLevel = hwnd ;   //  do I need this?
   wsprintfA(msgstr, "%s", Version) ;
   SetWindowTextA(hwnd, msgstr) ;

   SetClassLongA(hwnd, GCL_HICON,   (LONG) LoadIcon(g_hinst, (LPCTSTR)GTSTUFF_ICO));
   SetClassLongA(hwnd, GCL_HICONSM, (LONG) LoadIcon(g_hinst, (LPCTSTR)GTSTUFF_ICO));

   hwndMain = hwnd ;
   get_monitor_dimens(hwnd);

   RECT myRect ;
   // GetWindowRect(hwnd, &myRect) ;
   GetClientRect(hwnd, &myRect) ;
   cxClient = (myRect.right - myRect.left) ;
   cyClient = (myRect.bottom - myRect.top) ;
   
   // Claude 08/14/26 - measure actual border/caption size once, from live
   // window+client rects, rather than guessing at SM_CXFRAME/SM_CYCAPTION
   // (which can be wrong under theming/DPI). Used to convert client-size
   // values into the window-size values WM_GETMINMAXINFO actually wants.
   {
   RECT winRect ;
   GetWindowRect(hwnd, &winRect) ;
   dx_frame = (winRect.right - winRect.left) - (int) cxClient ;
   dy_frame = (winRect.bottom - winRect.top) - (int) cyClient ;
   // syslog("frame delta: dx_frame=%d, dy_frame=%d\n", dx_frame, dy_frame) ;
   }

   init_config();
   
   //  get global handles for graphics components
   hwndGFrame = GetDlgItem(hwnd, IDC_GFRAME) ;
   hwndGInfo  = GetDlgItem(hwnd, IDC_GINFO) ;
   
   capture_gframe_layout() ;   //  Claude 08/17/26 - must run before cxClient can change
   //  Claude 08/17/26 - see GFrameSubclassProc's comment for why this is
   //  necessary rather than just calling draw_gframe_contents() after resizes.
   gframe_orig_proc = (WNDPROC) SetWindowLongA(hwndGFrame, GWL_WNDPROC, (LONG) GFrameSubclassProc) ;

   center_dialog_on_screen(hwnd);
   
   //****************************************************************
   //  create/configure status bar
   //****************************************************************
   // MainStatusBar = new CStatusBar(hwnd) ;
   MainStatusBar = std::make_unique<CStatusBar>(hwnd);
   MainStatusBar->MoveToBottom(cxClient, cyClient) ;
   //  re-position status-bar parts
   update_statusbar_parts() ;

   //  Claude 08/17/26 - select the intro page (menu_items[0]) at startup,
   //  same as clicking a menu item would -- establishes miptr/demo_state so
   //  display_current_operation() has something to dispatch to once the
   //  idle loop starts. Without this, miptr stays nullptr until the user's
   //  first menu click and nothing draws in the meantime.
   change_graph_state(0) ;

   //  Claude 08/17/26 - size the graphics frame to the initial dialog and
   //  fill/border it; resize_dialog_and_workspace() keeps this in sync from
   //  here on, but a live-drag or WM_SIZE isn't guaranteed to fire before
   //  the dialog is first shown, so do it explicitly here too. Actual demo
   //  content (the intro X, initially) is drawn separately, once the idle
   //  loop starts -- see draw_gframe_contents()'s comment.
   resize_gframe() ;

   // Claude 08/14/26 - the real, permanent floor for WM_GETMINMAXINFO.
   // Same shape as resize_dialog_and_workspace's live layout math, just solved for
   // the smallest acceptable listview height (MIN_LISTVIEW_VISIBLE_DY)
   // instead of the current one. Computed once, here, and never touched
   // again -- see the comment on the variable itself.
   min_application_window_height = get_terminal_top() + MIN_LISTVIEW_VISIBLE_DY
      + MainStatusBar->height() + (uint) get_dy_offset() + (uint) dy_frame ;

#ifdef USE_WIDTH_RESIZE
   // Claude 08/16/26 - same shape as min_application_window_height above,
   // computed once here and never touched again.
   min_application_window_width = MIN_TERMINAL_VISIBLE_DX + (uint) dx_frame ;
#endif

   //****************************************************************
   //  create/configure working space
   //****************************************************************
   
   //  restore previously-saved window size/position from the .ini file. 
   restore_dialog_settings(hwnd);

   //  Claude 08/17/26 - IDB_CLOSE is the only WS_TABSTOP control in this
   //  dialog, so the dialog manager gives it default keyboard focus on
   //  startup, and nothing ever moves focus away from it afterward. That's
   //  a separate mechanism from IsDialogMessage() (removed from the main
   //  loop already) -- the built-in Button control class has its own
   //  internal handling of Space on whichever button currently has focus,
   //  independent of dialog navigation entirely. Moving focus to hwndGFrame
   //  (a plain Static, which does nothing with keyboard input at all) is
   //  what actually stops it. Paired with returning FALSE from
   //  WM_INITDIALOG below, per the standard convention for a dialog proc
   //  that sets focus itself instead of letting the system pick a default.
   SetFocus(hwndGFrame) ;
}

//********************************************************************************************
//  okay, this function originally gave inaccurate results,
//  because the rectangle passed by WM_SIZING was from GetWindowRect(),
//  which included the unwanted border area, rather than from
//  GetClientRect(), which works with get_bottom_line().
//********************************************************************************************
static void resize_dialog_and_workspace()
{
   RECT myRect ;
   // char msgstr[81] ;
   // syslog("resize terminal, drag=%s\n", (resize_on_drag) ? "true" : "false") ;

   //  if resizing on drag-and-drop, re-read main-dialog size
   // BOOL gcr_ok = 
   GetClientRect(hwndMain, &myRect) ;
   uint new_window_height = (uint) (myRect.bottom - myRect.top) ;
#ifdef USE_WIDTH_RESIZE
   uint new_window_width  = (uint) (myRect.right - myRect.left) ;
#endif
   // syslog("resize: cyClient: %u, new_window_height: %u, rect=(%ld,%ld,%ld,%ld), gcr_ok=%d, err=%lu\n",
   //    cyClient, new_window_height,
   //    (long) myRect.left, (long) myRect.top, (long) myRect.right, (long) myRect.bottom,
   //    (int) gcr_ok, gcr_ok ? 0ul : (unsigned long) GetLastError());

   if (new_window_height == 0) {
       return ;
   }

#ifdef USE_WIDTH_RESIZE
   // Claude 08/16/26 - unlike the fixed-width case, a live drag can now
   // change either dimension independently (or both), so bail only when
   // neither has actually moved.
   bool height_changed = (cyClient != new_window_height) ;
   bool width_changed  = (cxClient != new_window_width  &&  new_window_width != 0) ;
   if (!height_changed  &&  !width_changed) {
      return ;
   }
   cyClient = new_window_height ;
   if (width_changed) {
      cxClient = new_window_width ;
   }
#else
   if (cyClient == new_window_height) {
      return ;
   }
   cyClient = new_window_height ;
#endif

   // int dy_offset = get_dy_offset() ;

   MainStatusBar->MoveToBottom(cxClient, cyClient-1) ;
#ifdef USE_WIDTH_RESIZE
   //  status-bar part boundaries are proportional to cxClient --
   //  redo them whenever width actually moved
   if (width_changed) {
      update_statusbar_parts() ;
   }
#endif
   //  Claude 08/17/26 - re-solve the graphics frame's position/size for the
   //  new cxClient/cyClient (width and/or height may have moved) and redraw
   //  the evaluation X on top of it.
   resize_gframe() ;

   save_cfg_file();
}

//*************************************************************************************
// Claude: WM_SIZE — this is the only place you actually move/resize child controls. 
// Dialogs don't auto-relayout children on resize; you compute the height delta 
// and grow the listview by exactly that much, leaving the top controls alone.
//*************************************************************************************
// static const char *size_type_name(WPARAM wParam)
// {
//    switch (wParam) {
//    case SIZE_RESTORED:  return "SIZE_RESTORED" ;
//    case SIZE_MINIMIZED: return "SIZE_MINIMIZED" ;
//    case SIZE_MAXIMIZED: return "SIZE_MAXIMIZED" ;
//    case SIZE_MAXSHOW:   return "SIZE_MAXSHOW" ;
//    case SIZE_MAXHIDE:   return "SIZE_MAXHIDE" ;
//    default:             return "SIZE_??" ;
//    }
// }

static bool do_size(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   // syslog("do_size: type=%s, lParam dx=%d dy=%d, IsIconic=%d\n",
   //    size_type_name(wParam), (int) LOWORD(lParam), (int) HIWORD(lParam),
   //    (int) IsIconic(hwnd));
   resize_dialog_and_workspace();
   return true ;
}

//*************************************************************************************
// Claude: WM_SIZING itself isn't needed for this shape of problem — 
// it's for constraining to an aspect ratio or snapping to a grid during the drag. 
// Locking width via WM_GETMINMAXINFO is simpler and sufficient here.
//*************************************************************************************
// static 
bool do_sizing(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   //  handle main-dialog resizing
   switch (message) {
   case WM_SIZING:
      switch (wParam) {
      case WMSZ_BOTTOMLEFT:
      case WMSZ_BOTTOMRIGHT:
      case WMSZ_TOPLEFT:
      case WMSZ_TOPRIGHT:
      case WMSZ_LEFT:
      case WMSZ_RIGHT:
      case WMSZ_TOP:
      case WMSZ_BOTTOM:
         resize_dialog_and_workspace();
         return true;

      default:
         break;
      }
      break;
   }  //lint !e744
   return false ;
}

//*************************************************************************************
//  DDM 01/29/17 - These minima are not actually working;
//  Perhaps this is due to Windowblinds ??
//  Yes; this works fine on standard Windows 7
//*************************************************************************************
//  Claude 08/12/26
//  WM_GETMINMAXINFO — this is where you lock the width and bound the height.
//  Setting ptMinTrackSize.x == ptMaxTrackSize.x (both equal to the dialog's current
//  width) is enough to make the left/right borders un-draggable — you don't need
//  WM_SIZING for that. Height min comes from your own "smallest useful layout"
//  calculation; height max comes from SystemParametersInfo(SPI_GETWORKAREA, ...) 
//  so the dialog can't be dragged off the bottom of the screen.
//*************************************************************************************
static bool do_getminmaxinfo(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LPMINMAXINFO lpTemp = (LPMINMAXINFO) lParam; //  NOLINT(performance-no-int-to-ptr)
   POINT        ptTemp;
   // syslog("set minimum to %ux%u\n", cxClient, cyClient);
   
   //  Claude 08/14/26 - cxClient is a CLIENT-area size; ptMinTrackSize/
   //  ptMaxTrackSize must be WINDOW sizes (border+caption included), so add
   //  the frame delta captured at init. Width is pinned min==max to lock
   //  horizontal resize; that pin must land on the real current window
   //  width or Windows will fight the live window size every time this
   //  fires and can degenerate the rect mid-drag.
   //
   //  Height floor comes from min_application_window_height.
   //  min_application_window_height is computed once in do_init_dialog 
   //  and never changes, which is what a track-size floor needs to be.
   
#ifdef USE_WIDTH_RESIZE
   // Claude 08/16/26 - width is no longer pinned min==max; it gets its own
   // floor/ceiling the same way height already does.
   //  set minimum dimensions
   ptTemp.x = (LONG) min_application_window_width ;
   ptTemp.y = (LONG) min_application_window_height ;
   lpTemp->ptMinTrackSize = ptTemp;
   //  set maximum dimensions
   ptTemp.x = (LONG) get_screen_width() ;
   ptTemp.y = (LONG) get_screen_height() ;
   lpTemp->ptMaxTrackSize = ptTemp;
#else
   //  set minimum dimensions
   ptTemp.x = (LONG) cxClient + dx_frame ;
   ptTemp.y = (LONG) min_application_window_height ;
   lpTemp->ptMinTrackSize = ptTemp;
   // uint dxmin = ptTemp.x ;
   // uint dymin = ptTemp.y ;
   //  set maximum dimensions
   ptTemp.x = (LONG) cxClient + dx_frame ;
   ptTemp.y = get_screen_height() ;
   lpTemp->ptMaxTrackSize = ptTemp;
#endif
   // lpTemp->ptMaxSize = ptTemp;
   // syslog("gmmi: dxmin: %u, dxmax: %u, dymin: %u, dymax: %ld\n", dxmin, ptTemp.x, dymin, (long) ptTemp.y);
   return true ;
}

//***********************************************************************
static LRESULT CALLBACK DialogProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   //***************************************************
   //  debug: log all windows messages
   //***************************************************
   if (dbg_flags & DBG_WINMSGS) {
      switch (message) {
      //  list messages to be ignored
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORSTATIC:
      case WM_CTLCOLOREDIT:
      case WM_CTLCOLORDLG:
      case WM_MOUSEMOVE:
      case 295:  //  WM_CHANGEUISTATE
      case WM_NCMOUSEMOVE:
      case WM_NCMOUSELEAVE:
      case WM_NCHITTEST:
      case WM_SETCURSOR:
      case WM_ERASEBKGND:
      case WM_TIMER:
      case WM_NOTIFY:
      case WM_COMMAND:  //  prints its own msgs below
         break;
      default:
         syslog("TOP [%s]\n", lookup_winmsg_name(message)) ;
         break;
      }
   }

   switch(message) {
   case WM_INITDIALOG:
      do_init_dialog(hwnd) ;
      //  Claude 08/17/26 - FALSE, not TRUE: do_init_dialog() calls
      //  SetFocus(hwndGFrame) itself (see that function's closing comment),
      //  so we tell the dialog manager not to override it with its own
      //  default focus assignment (which would otherwise land back on
      //  IDB_CLOSE, the only WS_TABSTOP control, undoing the whole point).
      return FALSE;

   case WM_PAINT:
      //  Claude 08/17/26 - BeginPaint/EndPaint must run unconditionally, 
      //  no matter what cxClient/cyClient are -- that's what validates the
      //  region and stops Windows from re-sending WM_PAINT. The cxClient/
      //  cyClient check only guards whether there's anything meaningful to
      //  paint (currently nothing -- display_current_operation() is still
      //  commented out below); it must never gate validation itself, or
      //  this becomes the same idle-loop-starving bug gframe_drawn_once
      //  was fixing, just for a theoretical cxClient==0 case instead.
      {
      PAINTSTRUCT ps;
      // HDC hdc = 
      BeginPaint (hwnd, &ps) ;
      if (cxClient != 0 && cyClient != 0) {
         // display_current_operation() ;
      }
      EndPaint (hwnd, &ps) ;
      }
      //  see gframe_drawn_once comment: this is the earliest point at which
      //  hwndGFrame is guaranteed to actually be visible, so this is where
      //  the first real draw belongs.
      if (!gframe_drawn_once) {
         gframe_drawn_once = true ;
         resize_gframe() ;
      }
      return 0 ;
    
   // case WM_NOTIFY:
   //    return term_notify(hwnd, lParam) ;


   case WM_EXITSIZEMOVE:
      {
      RECT rect ;
      GetWindowRect(hwnd, &rect);
      window_top = rect.top ;
      window_left = rect.left ;
      save_cfg_file();
      }
      break ;
   
   case WM_GETMINMAXINFO:
      do_getminmaxinfo(hwnd, message, wParam, lParam) ;
      return FALSE;

   case WM_SIZE:
      do_size(hwnd, message, wParam, lParam) ;
      return TRUE ;

#ifndef USE_WIDTH_RESIZE
   //  this is only required if width is fixed in dialog -- 
   //  it's what clamps out the WindowBlinds side-to-side wobble.
   //  Not applicable once width is actually allowed to change.
   case WM_WINDOWPOSCHANGING:
      {
      WINDOWPOS* pos = (WINDOWPOS*)lParam;
      if (!(pos->flags & SWP_NOSIZE))
         pos->cx = cxClient;
      break;
      }      
      return TRUE ;
#endif

   //***********************************************************************************************
   case WM_COMMAND:
      {  //  create local context
      DWORD cmd = HIWORD (wParam) ;
      DWORD target = LOWORD(wParam) ;

      switch (cmd) {
      case FVIRTKEY:  //  keyboard accelerators: WARNING: same code as CBN_SELCHANGE !!
         //  fall through to BM_CLICKED, which uses same targets
      case BN_CLICKED:
         //  Claude 08/17/26 - menu-item WM_COMMANDs land here too: a menu
         //  selection's notification code (HIWORD) is 0, same as BN_CLICKED,
         //  so IDM_MAINMENU's items (gtstuff.rc) are handled by the same
         //  switch as the dialog's buttons.
         switch(target) {
         
         case IDB_CLOSE:
         case IDM_FILE_CLOSE:
            PostMessageA(hwnd, WM_CLOSE, 0, 0);
            break;

         case IDM_CIRCLES:
         case IDM_SQUARES:
         case IDM_LIGHTNING:
         case IDM_RECT:
         case IDM_PIXELS:
         case IDM_CLRBARS:
         case IDM_XPAL:
         case IDM_BITBLT:
         case IDM_XNPAL:
         case IDM_XRECT:
         case IDM_GPAL:
         case IDM_TRIANGLE:
         case IDM_RAINBOW:
         case IDM_LINES:
         case IDM_LGAMES:
         case IDM_COLORS:
         case IDM_FLAMES:
         case IDM_FACES:
         case IDM_ASCII:
         case IDM_SGLASS:
         case IDM_WINCOLORS:
            change_graph_state(target);
            break;
         } //lint !e744  switch target
         return true;
      } //lint !e744  switch cmd
      break;
      }  //lint !e438 !e10  end local context

   //********************************************************************
   //  application shutdown handlers
   //********************************************************************
   case WM_CLOSE:
      DestroyWindow(hwnd);
      break;

   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   // default:
   //    return false;
   }  //lint !e744  switch(message) 

   return false;
}

//***********************************************************************
//lint -esym(1784, WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
   g_hinst = hInstance;
   load_exec_filename() ;     //  get our executable name

   HWND hwnd = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, (DLGPROC) DialogProc) ;
   if (hwnd == NULL) {
      syslog("CreateDialog: %s\n", get_system_message()) ;
      return 0;
   }

   //  Claude 08/17/26 - PeekMessage-based idle loop, same shape as the
   //  original gstuff main loop, restoring free-running-page capability
   //  without WM_TIMER. GetMessage() blocks until a message arrives, which
   //  is exactly what was preventing idle-time drawing; PeekMessage() with
   //  PM_REMOVE doesn't block, so when the queue is empty we fall through to
   //  display_current_operation() instead.
   //
   //  display_current_operation() returning false means nothing needed
   //  drawing this pass (current page is static and already up to date) --
   //  block via WaitMessage() instead of spinning a CPU core for nothing.
   //  That's a deliberate improvement over the original: gstuff was a
   //  dedicated full-screen demo where spinning was fine; this app shares
   //  the machine with whatever else the user is running.
   //
   //  Claude 08/17/26 - no IsDialogMessage() here: this app has no keyboard
   //  interface at all by design (a single button, a menu, and the graphics
   //  frame), and IsDialogMessage() is what was implementing Tab/Enter/Space
   //  dialog-navigation conventions automatically -- specifically, Space
   //  activating whatever control currently has keyboard focus (IDB_CLOSE,
   //  the only tab-stop control in the dialog), which was closing the
   //  program on every press. Menu mnemonics (Alt+F, etc.) don't depend on
   //  IsDialogMessage() -- those work through ordinary WM_SYSKEYDOWN/
   //  WM_SYSCHAR dispatch, so removing this doesn't affect them.
   MSG Msg;
   for (;;) {
      if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
         if (Msg.message == WM_QUIT) {
            break ;
         }
         TranslateMessage(&Msg);
         DispatchMessage(&Msg);
      }
      else {
         if (!display_current_operation()) {
            WaitMessage() ;
         }
      }
   }

   return (int) Msg.wParam ;
}  //lint !e715

