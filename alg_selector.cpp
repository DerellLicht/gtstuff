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

#include "common.h"     //  u8, etc
#include "resource.h"  
#include "gtstuff.h"    //  
#include "palettes.h"   //  24-bit palette functions - required by gobjects.h
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
// #include "keywin32.h"
#include "alg_selector.h"       //  demo declarations
// #include "ezfont.h"

int demo_state = 0 ;
int we_should_redraw = 1 ;
int pause_the_race = 0 ;
unsigned use_solid_pattern = 0 ;
unsigned cycle_count = 0 ;

static u64 ti = 0 ;

#define  RESTRICT_ALGS

//***********************************************************************
//  instantiate each of the classes
//***********************************************************************
static circles circles0("Psychedelic Raindrops") ;
static squares squares0("Boxing Lessons") ;
#ifndef  RESTRICT_ALGS
static polygon polygon0("Temporal Lightning") ;
static rect rect0("Palette Boxes") ;
static pixels pixels0("Pixel-packing") ;
static colorbars colorbars0("Color Bars") ;
static xpalette xpalette0("XWindows palette") ;

static sglass sglass0("Stained Glass") ;
static flames flames0("Fire tricks") ;
static face_trap faces0("Face traps") ;
static rainbow rainbow0("Rainbow !!") ;
ascii ascii0("ASCII Table") ;
static rcolors rcolors0("rcolors32") ;
static line_games lgames0("Line Games") ;
static lines lines0("Lines") ;
static gpalettes gpalettes0("more palettes") ;
static xnpalette xnpalette0("Named XWindows palette") ;
static xrect xrect0("XWindows Palette Boxes") ;
static bitblt bitblt0("BitBlt demo and options") ;
static triangles triangles0("line triangle") ;
static wincolors wincolors0("Windows Colors") ;
#endif

/************************************************************************/
typedef struct menu_items_s {
   unsigned menu_attr ;
   graph_object *go ;
   uint     menu_id ;
   char     *menu_text ;
   char     *title ;
   void (*draw_func)(HWND hwnd);
} menu_items_t, *menu_items_p ;

static menu_items_p miptr = nullptr ;
/************************************************************************/

static menu_items_t menu_items[] = {
{ WIN_BLUE,  0,           0,           "  ",                 "Graphics demos",         0 },
{ WIN_BLUE,  &circles0,   IDM_CIRCLES, "a: Raindrops ",      "Psychedelic Raindrops",  0 },
{ WIN_BLUE,  &squares0,   IDM_SQUARES, "b: Boxes ",          "Boxing Lessons",         0 },
#ifndef  RESTRICT_ALGS
{ WIN_BLUE,  &polygon0,   0, "c: Lightning ",                "Temporal Lightning",     0 },
{ WIN_BLUE,  &rect0,      0, "d: 3D Boxes ",                 "Palette Boxes",          0 },
{ WIN_BLUE,  &pixels0,    0, "e: Pixels ",                   "Pixel-packing",          0 },
{ WIN_BLUE,  &colorbars0, 0, "f: Color Bars ",               "Color Bars",             0 },
{ WIN_BLUE,  &xpalette0,  0, "g: Observe XWindows Palette ", "XWindows Palette",       0 },
{ WIN_BLUE,  &bitblt0,    0, "h: bitblt demo ",              "BitBlt demo",            0 },
{ WIN_BLUE,  &xnpalette0, 0, "i: Named XWindows Palette ",   "Named XWindows Palette", 0 },
{ WIN_BLUE,  &xrect0,     0, "j: 3D XWindows Boxes ",        "XWindows Palette Boxes", 0 },
{ WIN_BLUE,  &gpalettes0, 0, "k: more palettes ",            "more palettes",          0 },
{ WIN_BLUE,  &triangles0, 0, "l: line stuff ",               "line triangle",          0 },
{ WIN_BLUE,  &rainbow0,   0, "m: Rainbow !!",                "Rainbow !!",             0 },
{ WIN_BLUE,  &lines0,     0, "n: Lines",                     "Lines",                  0 },
{ WIN_BLUE,  &lgames0,    0, "o: Line Games",                "Lines Games",            0 },
{ WIN_BLUE,  &rcolors0,   0, "p: rcolors32",                 "rcolors32",              0 },
{ WIN_BLUE,  &flames0,    0, "q: Fire tricks",               "Fire tricks",            0 },
{ WIN_BLUE,  &faces0,     0, "r: Face Traps",                "Face traps",             0 },
{ WIN_BLUE,  &ascii0,     0, "s: ASCII table",               "ASCII table",            0 },
{ WIN_BLUE,  &sglass0,    0, "t: Stained Glass",             "Stained Glass",          0 },
{ WIN_BLUE,  &wincolors0, 0, "u: Windows Colors",            "Windows Colors",         0 },
#endif
{ 0, 0, 0, 0, 0, 0 }} ;


//***********************************************************************
void change_graph_state(HWND hwnd, uint graph_id)
{
   for (uint j=0; menu_items[j].menu_text != 0; j++) {
      if (menu_items[j].menu_id == graph_id) {
         demo_state = j ;
         miptr = &menu_items[demo_state] ;
         show_graph_desc(miptr->title);
         cycle_count = 0 ;
         ti = proc_time ();
         return ;         
      }
   }
}

//***********************************************************************
bool display_current_operation(HWND hwnd)
{
   if (miptr == nullptr) {
      return false;
   }
   if (miptr->go != nullptr) {
      miptr->go->update_display(hwnd) ;
      we_should_redraw = 0 ;
      return true ;
   } 
   else {
      we_should_redraw = 0 ;
      return false ;
   }
}

