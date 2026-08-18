//***************************************************************************************
//  alg_selector.cpp 
//  Present onscreen menu of program options, and display the selected option.
//  Possibly, a better name for this file/header might have been menu.cpp/menu.h
//***************************************************************************************
//  Original file info:
//  File: DEMO.C
//  This is a main program which is used to illustrate use of drawing routines 
//  from the book "Advanced Programmer's Guide to SuperVGA".
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

int we_should_redraw = 1 ;
int pause_the_race = 0 ;
unsigned use_solid_pattern = 0 ;
unsigned cycle_count = 0 ;

static u64 ti = 0 ;

#define  RESTRICT_ALGS

//***********************************************************************
//  instantiate each of the classes
//***********************************************************************
// NOLINTBEGIN(bugprone-throwing-static-initialization)
static circles circles0("Psychedelic Raindrops") ;
static squares squares0("Boxing Lessons") ;
static polygon polygon0("Temporal Lightning") ;
static rect rect0("Palette Boxes") ;
#ifndef  RESTRICT_ALGS
static pixels pixels0("Pixel-packing") ;
static colorbars colorbars0("Color Bars") ;
static xpalette xpalette0("XWindows palette") ;
static bitblt bitblt0("BitBlt demo and options") ;
static xnpalette xnpalette0("Named XWindows palette") ;
static xrect xrect0("XWindows Palette Boxes") ;
static gpalettes gpalettes0("more palettes") ;
static triangles triangles0("line triangle") ;
static rainbow rainbow0("Rainbow !!") ;
static lines lines0("Lines") ;
static line_games lgames0("Line Games") ;
static rcolors rcolors0("rcolors32") ;
static flames flames0("Fire tricks") ;
static face_trap faces0("Face traps") ;
ascii ascii0("ASCII Table") ;
static sglass sglass0("Stained Glass") ;
static wincolors wincolors0("Windows Colors") ;
#endif
// NOLINTEND(bugprone-throwing-static-initialization)


/************************************************************************/
typedef struct menu_items_s {
   graph_object *go ;
   uint          menu_id ;
   char const   *title ;
   void (*draw_func)(void);
} menu_items_t, *menu_items_p ;

static menu_items_p miptr = nullptr ;
/************************************************************************/

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

/************************************************************************/
std::vector<menu_items_t> menu_items {
 { 0,           0,             "Graphics demos",         draw_intro_graphics }
,{ &circles0,   IDM_CIRCLES,   "Psychedelic Raindrops",  0 }
,{ &squares0,   IDM_SQUARES,   "Boxing Lessons",         0 }
,{ &polygon0,   IDM_LIGHTNING, "Temporal Lightning",     0 }
,{ &rect0,      IDM_RECT,      "Palette Boxes",          0 }
#ifndef  RESTRICT_ALGS
,{ &pixels0,    0,             "Pixel-packing",          0 }
,{ &colorbars0, 0,             "Color Bars",             0 }
,{ &xpalette0,  0,             "XWindows Palette",       0 }
,{ &bitblt0,    0,             "BitBlt demo",            0 }
,{ &xnpalette0, 0,             "Named XWindows Palette", 0 }
,{ &xrect0,     0,             "XWindows Palette Boxes", 0 }
,{ &gpalettes0, 0,             "more palettes",          0 }
,{ &triangles0, 0,             "line triangle",          0 }
,{ &rainbow0,   0,             "Rainbow !!",             0 }
,{ &lines0,     0,             "Lines",                  0 }
,{ &lgames0,    0,             "Lines Games",            0 }
,{ &rcolors0,   0,             "rcolors32",              0 }
,{ &flames0,    0,             "Fire tricks",            0 }
,{ &faces0,     0,             "Face traps",             0 }
,{ &ascii0,     0,             "ASCII table",            0 }
,{ &sglass0,    0,             "Stained Glass",          0 }
,{ &wincolors0, 0,             "Windows Colors",         0 }
#endif
} ;


//***********************************************************************
void change_graph_state(uint graph_id)
{
   for(auto &mentry : menu_items) {
      if (mentry.menu_id == graph_id) {
         // demo_state = j ;
         // miptr = &menu_items[demo_state] ;
         miptr = &mentry ;
         show_graph_desc(miptr->title);
         cycle_count = 0 ;
         we_should_redraw = 1 ;
         ti = proc_time ();
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

