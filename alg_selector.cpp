//***************************************************************************************
//  alg_selector.cpp 
//  Present onscreen menu of program options, and display the selected option.
//  Possibly, a better name for this file/header might have been menu.cpp/menu.h
//***************************************************************************************

#include <windows.h>
#include <stdio.h>   //  sprintf, for %*s syntax
#include <math.h>
#include <vector>

#include "common.h"     //  u8, etc
#include "resource.h"  
#include "gtstuff.h"    //  
#include "palettes.h"   //  24-bit palette functions - required by gobjects.h
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"       //  demo declarations
#include "images.h"  

bool we_should_redraw = 1 ;
bool pause_the_race = false ;
bool use_solid_pattern = false ;
bool run_custom_op = false ;

unsigned cycle_count = 0 ;

//***********************************************************************
//  Claude 08/17/26 - the "intro" placeholder graphic: an X marking the
//  frame's extent while no algorithm is selected. Formerly hardcoded into
//  draw_gframe_contents() (gtstuff.cpp) on every repaint; moved here as
//  menu_items[0]'s draw_func so it goes through the same dispatch as every
//  other demo. draw_gframe_contents() still owns fill+border (that's frame
//  housekeeping, not demo content) -- this just adds the X on top of that.
//***********************************************************************
void draw_intro_graphics(void)
{
   HDC hdc = graph_object::get_gframe_dc() ;
   LineCR(hdc, 0, 0,                  (int) cxGFrame - 1, (int) cyGFrame - 1, WIN_BWHITE) ;
   LineCR(hdc, 0, (int) cyGFrame - 1, (int) cxGFrame - 1, 0,                  WIN_BWHITE) ;
   graph_object::release_gframe_dc(hdc) ;
}

//***********************************************************************
//  instantiate each of the classes
//***********************************************************************
// NOLINTBEGIN(bugprone-throwing-static-initialization)
static circles circles0 ;
static squares squares0 ;
static polygon polygon0 ;
static rect rect0 ;
static pixels pixels0 ;
static colorbars colorbars0 ;
static xpalette xpalette0 ;
static bitblt bitblt0 ;
static xnpalette xnpalette0 ;
static xrect xrect0 ;
static gpalettes gpalettes0 ;
static triangles triangles0 ;
static rainbow rainbow0 ;
static lines lines0 ;
static line_games lgames0 ;
static rcolors rcolors0 ;
static flames flames0 ;
static face_trap faces0 ;
//  ascii is exported because it is used by font.dialog.cpp
ascii ascii0 ;
static sglass sglass0 ;
static wincolors wincolors0 ;
// NOLINTEND(bugprone-throwing-static-initialization)

//************************************************************************
typedef struct menu_items_s {
   graph_object  *go ;
   uint           menu_id ;
   bool           write_once ;
   bool           use_pause ;
   bool           solid_fill_option ;
   bool           use_pal_select ;
   bool           use_custom_option ;
   char const    *title ;
   void (*draw_func)(void);
} menu_items_t, *menu_items_p ;

static menu_items_p miptr = nullptr ;

//************************************************************************
std::vector<menu_items_t> menu_items {
//                                                       pal
// gobject      menu_id        write_once  pause  solid  select custom
 { 0,           0,             true,       false, false, false, false, "Graphics demos",         draw_intro_graphics }
,{ &circles0,   IDM_CIRCLES,   false,      true,  true,  false, false, "Psychedelic Raindrops",  0 }
,{ &squares0,   IDM_SQUARES,   false,      true,  true,  false, false, "Boxing Lessons",         0 }
,{ &polygon0,   IDM_LIGHTNING, false,      true,  false, false, false, "Temporal Lightning",     0 }
,{ &rect0,      IDM_RECT,      true,       false, false, false, false, "Palette Boxes",          0 }
,{ &pixels0,    IDM_PIXELS,    false,      true,  true,  true,  false, "Pixel-packing",          0 }
,{ &colorbars0, IDM_CLRBARS,   true,       false, false, false, false, "Color Bars",             0 }
,{ &xpalette0,  IDM_XPAL,      true,       false, false, false, false, "XWindows Palette",       0 }
,{ &bitblt0,    IDM_BITBLT,    true,       false, false, false, false, "BitBlt Demo",            0 }
,{ &xnpalette0, IDM_XNPAL,     true,       false, false, false, true,  "Named XWindows Palette", 0 }
,{ &xrect0,     IDM_XRECT,     true,       false, false, false, false, "XWindows Palette Boxes", 0 }
,{ &gpalettes0, IDM_GPAL,      true,       false, false, false, true,  "More Palettes",          0 }
,{ &triangles0, IDM_TRIANGLE,  false,      true,  false, false, false, "Line Triangle",          0 }
,{ &rainbow0,   IDM_RAINBOW,   false,      true,  false, true,  false, "Rainbow !!",             0 }
,{ &lines0,     IDM_LINES,     false,      true,  false, false, false, "Lines",                  0 }
,{ &lgames0,    IDM_LGAMES,    false,      true,  false, true,  true,  "Lines Games",            0 }
,{ &rcolors0,   IDM_COLORS,    false,      true,  true,  true,  false, "Raining characters",     0 }
,{ &flames0,    IDM_FLAMES,    true,       false, false, false, false, "Fire tricks",            0 }
,{ &faces0,     IDM_FACES,     true,       false, false, false, false, "Face traps",             0 }
,{ &ascii0,     IDM_ASCII,     true,       false, false, false, true,  "Font Toys",              0 }
,{ &sglass0,    IDM_SGLASS,    false,      true,  false, true,  false, "Stained Glass",          0 }
,{ &wincolors0, IDM_WINCOLORS, true,       false, false, false, false, "Windows Colors",         0 }
} ;

//***********************************************************************
static void button_enable_disable(HWND hwnd)
{
   static HWND hwndPause  = nullptr ;
   static HWND hwndSolid  = nullptr ;
   static HWND hwndCustom = nullptr ;
   if (hwndPause == nullptr) {
      hwndPause  = GetDlgItem(hwnd, IDB_PAUSE ) ;
      hwndSolid  = GetDlgItem(hwnd, IDB_PSOLID) ;
      hwndCustom = GetDlgItem(hwnd, IDB_CUSTOM) ;
   }
   EnableWindow(hwndPause,       (miptr->use_pause)         ? TRUE : FALSE);
   EnableWindow(hwndSolid,       (miptr->solid_fill_option) ? TRUE : FALSE);
   EnableWindow(hwndCustom,      (miptr->use_custom_option) ? TRUE : FALSE);
   EnableWindow(hwndPaletteSpin, (miptr->use_pal_select)    ? TRUE : FALSE);
   EnableWindow(hwndPalette,     (miptr->use_pal_select)    ? TRUE : FALSE);
}

//***********************************************************************
void fill_gobject_combobox(HWND hwnd, unsigned init_idx)
{
   // for (uint j=0; menu_items[j] != 0; j++) {
   for (auto &mitem : menu_items) {
      // if (mitem.menu_id == 0) {
      //    continue ;
      // }
      LRESULT result = SendMessageA(hwnd, CB_ADDSTRING, 0, (LPARAM) mitem.title);
      switch (result) {
      case CB_ERR:
         syslog("CB_ADDSTRING: CB_ERR: %s\n", get_system_message()) ;
         break;
      case CB_ERRSPACE:
         syslog("CB_ADDSTRING: CB_ERRSPACE: %s\n", get_system_message()) ;
         break;

      default:
         // wsprintfA(msgstr, "CB_ADDSTRING returned %u\n", result) ;
         // OutputDebugStringA(msgstr) ;
         break;
      }
   }
   SendMessageA(hwnd, CB_SETCURSEL, (WPARAM) init_idx, 0);
}

//***********************************************************************
//  handle button presses
//***********************************************************************
void toggle_pause_req()
{
   pause_the_race = !pause_the_race ;
   draw_ledb(hwndPauseState, pause_the_race);
}

void toggle_solid_pattern()
{
   use_solid_pattern = !use_solid_pattern ;
   draw_ledb(hwndSolidState, use_solid_pattern);
}

//***********************************************************************
void handle_custom_req(HWND hwnd)
{
   if (miptr->menu_id == IDM_ASCII) {
      read_a_font(hwnd);
   }
   else {
      run_custom_op = !run_custom_op ;
      we_should_redraw = 1 ;
   }
}

//***********************************************************************
void run_selected_gobject(HWND hwndGObjList)
{
   char tempstr[81];
   uint sel = SendMessageA(hwndGObjList, CB_GETCURSEL, 0, 0);
   // sel++ ;  // list box index is off by one vs menu_items list
   //  the initial landing slot in the dialog, is a label field;
   //  it is not intended to be executable, and has a resource ID of 0.
   // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
   uint target = menu_items[sel].menu_id; 
   if (target == 0) {
      return ;
   }
   
   // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
   wsprintf(tempstr, "[%u: %u] %s", sel, target, menu_items[sel].title);
   status_message(tempstr);
   change_graph_state(target);
}

//***********************************************************************
// display the cycle counter
//***********************************************************************
static u64 ti = 0 ;
static uint elapsed_secs = 0 ;

static void display_cycle_counter(void)
{
   char tempstr[81];
   unsigned tf_msec = (unsigned) (proc_time () - ti) / get_clocks_per_msec ();
   if (tf_msec == 0) {
      tf_msec = 1 ;
   }
   uint esecs = tf_msec / 1000 ;
   if (esecs != elapsed_secs) {
      elapsed_secs = esecs ;
      unsigned cycles_per_msec = cycle_count * 1000 / tf_msec ;   
      // wsprintf(tempstr, "cycle_count=%u, %u msec, %u cycles/sec", 
      //    cycle_count, tf_msec, cycles_per_msec) ;
      wsprintf(tempstr, "%u cycles/sec", cycles_per_msec) ;
      status_message(tempstr);
   }
}

//**************************************************************************
//  notes on how we_should_redraw is used:
//  
//  The flag is set after resize/redraw (in gtstuff.cpp),
//  or when a new gobject is selected (via change_graph_state()).
//  
//  The flag is cleared by display_current_operation()
//  after the first call to the drawing function is executed.
//  After that, display_current_operation() will continue to be executed
//  repeatedly from the message-handler loop.
//  gobjects which intend to execute just once, will check
//  we_should_redraw and return without action if it is false.
//  gobjects which are *not* execute-once objects, simply ignore the flag.
//**************************************************************************
void change_graph_state(uint graph_id)
{
   for(auto &mentry : menu_items) {
      if (mentry.menu_id == graph_id) {
         // demo_state = j ;
         // miptr = &menu_items[demo_state] ;
         miptr = &mentry ;
         show_graph_desc(miptr->title);
         status_message(" ");
         status_message(1, " ");
         status_message(2, " ");
         cycle_count = 0 ;
         we_should_redraw = 1 ;
         ti = proc_time();
         elapsed_secs = 0 ;
         pause_the_race = false ;
         use_solid_pattern = false ;
         run_custom_op = false ;
         button_enable_disable(get_main_dialog_handle());
         draw_ledb(hwndPauseState, false);
         draw_ledb(hwndSolidState, false);
         return ;         
      }
   }
   
   //  we didn't find the requested ID
   char msgstr[81];
   sprintf(msgstr, "ERROR: ID %u not found !!", graph_id);
   show_graph_desc(msgstr);
}

//***********************************************************************
bool display_current_operation(void)
{
   if (miptr == nullptr) {
      return false;
   }
   if (miptr->go != nullptr) {
      miptr->go->update_display() ;
      we_should_redraw = 0 ;
      if (!miptr->write_once) {
         display_cycle_counter();
      }
      return true ;
   } 
   else if (miptr->draw_func != nullptr) {
      //  Claude 08/17/26 - draw-once page: only redraw when something has
      //  flagged it necessary (see draw_gframe_contents() in gtstuff.cpp,
      //  and change_graph_state() above), then go quiet again -- unlike the
      //  go != nullptr branch above, which redraws unconditionally every
      //  idle pass because it's meant to be continuously running.
      if (we_should_redraw) {
         (miptr->draw_func)() ;
         we_should_redraw = 0 ;
         return true ;
      }
      return false ;
   }
   else {
      we_should_redraw = 0 ;
      return false ;
   }
}

