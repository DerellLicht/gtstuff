//*****************************************************************
//  RAINBOWV.CPP:  A rainbow simulator.                            
//  From "Astronomical Computing"                                  
//  Sky & Telescope Magazine, February 1991                        
//  Original program was in BASIC and EGA 4-color video mode.      
//                                                                 
//  05/27/98
//    Converted to C and VGA/16-color by Dan Miller.                 
//  08/09/02 16:11
//    Converted into Win32 graphics program for more colors
//*****************************************************************

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "common.h"     //  u8, etc
#include "commonw.h"    //  build_font() and constants
#include "gtstuff.h"  
#include "gfuncs.h"     //  graphics primitives
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphic objects
#include "alg_selector.h"

static const double RADS2DEGS = (180.0 / 3.14159) ;

//  winiface.cpp
//  Though these are only used by rainbow.cpp,
//  they need to be updated on WM_SIZE message.
//  Technically, the data should be encapsulated in the class,
//  and a function provided to update them.
// extern unsigned xbase, xdiff, ybase, ydiff ;

//*************************************************
static const unsigned rainbow_index[6] = { 
   WIN_RED,      //  red     
   RGB(255,165,0),   //  orange  
   WIN_YELLOW,     //  yellow  
   WIN_GREEN,      //  green   
   WIN_BLUE,      //  blue    
   WIN_MAGENTA       //  violet  
} ;

//***********************************************************************
rainbow::rainbow(std::string title_text) 
: graph_object(std::move(title_text)) 
, X(0)
, Y(0)
, B(0)
, thold_limit(60.0)
, xbase(1)
, xdiff(1)
, ybase(1)
, ydiff(1)

//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
   // RADS2DEGS = (180.0 / 3.14159) ;
   // thold_limit = 60.0 ;

   //  these initial values are unimporant,
   //  it will be initialized by WM_SIZE in the windows app
   // xbase = xdiff = ybase = ydiff = 1 ;
   //lint -esym(1401, rainbow::X, rainbow::Y, rainbow::B)
}

//************************************************************************
//  this function is currently unused
//************************************************************************
void rainbow::update_gtimer(HDC hdc)
{
   static unsigned gt_interval = 1000000 ;
   static unsigned gtidx = 0 ;
   char str[2] ;
   unsigned x_pixels ;

   if (++gt_interval < 200000)
      return ;
   gt_interval = 0 ;
   if (++gtidx >= 12)
      gtidx = 0 ;

   HFONT progress_font = build_font("Wingdings", 35, EZ_ATTR_NORMAL) ;
   if (progress_font == 0) {
      syslog("progress_font: %s\n", get_system_message()) ;
      return ;
   }
   SelectObject (hdc, progress_font) ;
   // Clear_Window(hdc, 0);
   SetBkMode(hdc, OPAQUE) ;
   SetBkColor  (hdc, 0) ;
   x_pixels = (gtidx < 6  ||  gtidx == 11) ? 0 : 1 ;
         
   str[0] = 183 + gtidx ;  //lint !e734
   str[1] = 0 ;
   SetTextColor(hdc, WIN_BGREEN) ;
   // SetTextColor(hdc, random_palette_ref()) ;
   TextOut (hdc, x_pixels, 0, str, 1);
   DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
}

/************************************************************************/
void rainbow::update_boundaries(unsigned xClient, unsigned yClient)
{
   xbase = xClient / 2 ;        //  originally 320
   xdiff = (xClient-50) / 2 ;   //  originally 320
   ybase = yClient * 7 / 8 ;    //  originally 420
   ydiff = yClient * 5 / 8 ;    //  originally 300
}

/************************************************************************/
void rainbow::rainbow_plot_pixel(HDC hdc, int pcolor, double thold_angle, unsigned primary)
{
   double f1 ;
   unsigned XP, YP ;
   COLORREF old_color ;

   thold_angle = fabs(thold_angle) ;
   if (thold_angle > thold_limit)
      return ;
   f1 = thold_angle / thold_limit ;

   XP = (unsigned) (xbase + xdiff * (f1 *     (X / B))) ;
   YP = (unsigned) (ybase - ydiff * (f1 * fabs(Y / B))) ;
   //  original method *never* trends to white in the central area
   // SetPixel(hdc, XP, YP, rainbow_index[pcolor]) ;

   //  revised method *does* trend to white, but green/blue/violet areas disappear.
   // old_color = GetPixel(hdc, XP, YP) ;
   // old_color |= rainbow_index[pcolor] ;
   // SetPixel(hdc, XP, YP, old_color) ;

   //  This works better, fading from purple to the white center
   if (primary  &&  (thold_angle < 32 + random_int(3))) {
      old_color = GetPixel(hdc, XP, YP) ;
      old_color |= rainbow_index[pcolor] ;
      SetPixel(hdc, XP, YP, old_color) ;
   } else {
      SetPixel(hdc, XP, YP, rainbow_index[pcolor]) ;
   }
}

//****************************************************************************
//  this function plots one primary and one secondary pixel
//****************************************************************************
void rainbow::update_display()
{
   int pcolor ;
   double N, I, R, T1, T2, RS, RP, RB, RC, I1, I2 ;

   if (pause_the_race)
      return ;

   HDC hdc = get_gframe_dc() ;
   //  do I actually want this??
   if (we_should_redraw) {
      Clear_Window(hdc, 0);
      update_boundaries(cxGFrame, cyGFrame) ;
   }

   cycle_count++ ;
   // 30 REM  RANDOM IMPACT PARAMETER
   X = -1 + 2 * random_part() ;
   Y = -1 + 2 * random_part() ;
   B = (double) sqrt(X * X + Y * Y) ; //  square root
   // 50 IF B >= 1 THEN 30
   if (B >= 1.0)
      return ;

   //  select random color and calculate index of refraction
   pcolor = random_int(6) ; //  select 1 thru 6
   N = 1.33 + 0.01 * (double) (pcolor) ;

   // 70 REM  COMPUTE ANGLES
   I  = asin(B) ;     //  angle of incidence
   R  = asin(B / N) ; //  angle of refraction

   //  calculate observation angles for 
   //  primary (T1) and secondary (T2) rainbows.
   T1 = (4 * R - 2 * I) * RADS2DEGS ;
   T2 = (6 * R - 2 * I) * RADS2DEGS - 180 ;

   // 95 REM  INTENSITY FACTORS
   // RS = (sin(I - R) / sin(I + R)) ^ 2 ;
   RS = (double) (sin(I - R) / sin(I + R)) ;
   RS *= RS ;
   RP = (double) (tan(I - R) / tan(I + R)) ;
   RP *= RP ;
   RB = (1 - RP) * (1 - RP) ;
   RC = (1 - RS) * (1 - RS) ;
   I1 = (     RS * RC +      RP + RB) / 2 ;
   I2 = (RS * RS * RC + RP * RP * RB) / 2 ;

   //  Greenler's method: rather than computing intensities
   //  for individual points, we randomly throw away some
   //  computed points in the low-intensity regions.
   if (I1 >= .04 * random_part())
      rainbow_plot_pixel(hdc, pcolor, T1, 1) ;

   if (I2 >= .02 * random_part())
      rainbow_plot_pixel(hdc, pcolor, T2, 0) ;

   // update_gtimer(hdc) ;

   release_gframe_dc(hdc) ;
}
