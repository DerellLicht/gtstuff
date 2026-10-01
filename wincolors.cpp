#include <windows.h>
#include <cmath>        //  Claude: pow(), used by srgb_to_linear()
#include <string>
#include <vector>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

static std::vector<std::string> wincolor_names {
"COLOR_SCROLLBAR",
"COLOR_BACKGROUND",  //  1: use for background, aka COLOR_DESKTOP
"COLOR_ACTIVECAPTION",
"COLOR_INACTIVECAPTION",
"COLOR_MENU",
"COLOR_WINDOW",
"COLOR_WINDOWFRAME",
"COLOR_MENUTEXT",
"COLOR_WINDOWTEXT",  //  8: use for text
"COLOR_CAPTIONTEXT",
"COLOR_ACTIVEBORDER",
"COLOR_INACTIVEBORDER",
"COLOR_APPWORKSPACE",
"COLOR_HIGHLIGHT",
"COLOR_HIGHLIGHTTEXT",
"COLOR_3DFACE",   // aka COLOR_BTNFACE
"COLOR_3DSHADOW", // aka COLOR_BTNSHADOW
"COLOR_GRAYTEXT",
"COLOR_BTNTEXT",
"COLOR_INACTIVECAPTIONTEXT",
"COLOR_3DHILIGHT",   //  aka COLOR_3DHIGHLIGHT, COLOR_BTNHILIGHT, COLOR_BTNHIGHLIGHT
"COLOR_3DDKSHADOW",
"COLOR_3DLIGHT",
"COLOR_INFOTEXT",
"COLOR_INFOBK",
"COLOR_UNUSED",   //  25: not used
"COLOR_HOTLIGHT",
"COLOR_GRADIENTACTIVECAPTION",
"COLOR_GRADIENTINACTIVECAPTION",
"COLOR_MENUHILIGHT"
} ;

#define  COLOR_UNUSED25    25

//************************************************************************
//  Claude: 
//  WCAG is the Web Content Accessibility Guidelines. 
//  Its contrast formula was designed to answer exactly your question, 
//  whether a color is readable against another, which is why it fit so well.
//************************************************************************

//  Claude: minimum WCAG contrast ratio (1.0 = none, 21.0 = black on white)
//  for label text.  4.5 is the WCAG AA standard for normal-size text;
//  3.0 is the lenient threshold.  Tweak to taste.
#define  MIN_TEXT_CONTRAST    4.5

//************************************************************************
//  Claude: convert one 8-bit sRGB channel to linear-light (0.0 - 1.0),
//  per the WCAG definition.  sRGB values are gamma-encoded, so
//  they must be linearized before they can be weighted and summed
//  into a meaningful brightness.
//************************************************************************
static double srgb_to_linear(unsigned c8)
{
   double c = (double) c8 / 255.0 ;
   if (c <= 0.03928) {
      //  the short linear segment near black
      return c / 12.92 ;
   }
   return pow((c + 0.055) / 1.055, 2.4) ;
}

//************************************************************************
//  Claude: relative luminance of a COLORREF: 0.0 = black, 1.0 = white.
//  The weights reflect the eye's sensitivity (green >> red >> blue).
//  Note that COLORREF is 0x00BBGGRR, hence the Get?Value macros.
//************************************************************************
static double rel_luminance(COLORREF cr)
{
   return 0.2126 * srgb_to_linear(GetRValue(cr))
        + 0.7152 * srgb_to_linear(GetGValue(cr))
        + 0.0722 * srgb_to_linear(GetBValue(cr)) ;
}

//************************************************************************
//  Claude: WCAG contrast ratio between two colors, 1.0 to 21.0.
//  Argument order does not matter.
//************************************************************************
static double contrast_ratio(COLORREF a, COLORREF b)
{
   double la = rel_luminance(a) ;
   double lb = rel_luminance(b) ;
   if (la < lb) {
      //  make la the lighter of the two
      double t = la ;
      la = lb ;
      lb = t ;
   }
   return (la + 0.05) / (lb + 0.05) ;
}

//************************************************************************
//  Claude:
//  choose a readable text color for the given background.
//  Prefers the theme color (pref) whenever it meets min_ratio,
//  so the display stays themed where possible.  Otherwise falls
//  back to whichever of pure black / pure white contrasts better.
//************************************************************************
static COLORREF pick_text_color(COLORREF bgnd, COLORREF pref, double min_ratio)
{
   if (contrast_ratio(bgnd, pref) >= min_ratio) {
      return pref ;
   }
   COLORREF black = RGB(0, 0, 0) ;
   COLORREF white = RGB(255, 255, 255) ;
   if (contrast_ratio(bgnd, black) >= contrast_ratio(bgnd, white)) {
      return black ;
   }
   return white ;
}

//************************************************************************
void wincolors::update_display()
{
   if (!we_should_redraw)
      return ;

   set_DAC_table(0) ;
   HDC hdc = get_gframe_dc() ;
   COLORREF canvas = GetSysColor(COLOR_BACKGROUND) ;
   Clear_Window(hdc, canvas);

   unsigned gapx = 30 ;
   unsigned gapy = 20 ;
   unsigned dx =  ((unsigned)cxGFrame - (4 * gapx)) / 3 ;
   unsigned dy = 50 ;
   unsigned idx ;
   char bfr[81] ;

   COLORREF fgnd = GetSysColor(COLOR_WINDOWTEXT) ;
   //  Claude: outer frame line: the theme text color if it stands out
   //  against the canvas, else black/white (same rule as the labels)
   COLORREF outer_frame = pick_text_color(canvas, fgnd, MIN_TEXT_CONTRAST) ;
   unsigned xi = gapx ;
   unsigned yi = 2 * gapy ;

   SetBkMode(hdc, TRANSPARENT);
   int line_idx = 0 ;
   for (idx=0; idx < wincolor_names.size(); idx++) {
      if (idx == COLOR_BACKGROUND ||   //  1
          idx == COLOR_WINDOWTEXT ||   //  8
          idx == COLOR_UNUSED25)       // 25
         continue;
      unsigned xf = xi + dx ;
      unsigned yf = yi + dy ;
      COLORREF bgnd = GetSysColor(idx) ;
      // sprintf(bfr, " %u: %s ", idx, wincolor_names[idx].c_str()) ;
      sprintf(bfr, " %u: %s ", idx, wincolor_names[idx].c_str()) ;
      SolidRect(hdc, xi, yi, xf, yf, bgnd) ;
      //  Claude: the label is drawn against the box color.  Use the theme's
      //  COLOR_WINDOWTEXT if it is readable there, else black/white.
      //  The frame is two lines, because no single color can be visible
      //  against both the box and the canvas: the inner line reuses the
      //  label color (contrasts with the box), and the outer line, drawn
      //  one pixel outside the box, contrasts with the canvas.
      //  Exception: when the label color matches the outer line (dark box,
      //  light label), two identical lines would read as one thick frame,
      //  so the inner line takes the box's own color instead.  That leaves
      //  a single visible line, matching the look of the light boxes.
      COLORREF label = pick_text_color(bgnd, fgnd, MIN_TEXT_CONTRAST) ;
      COLORREF inner_frame = (label == outer_frame) ? bgnd : label ;
      Box(hdc, xi-1, yi-1, xf+1, yf+1, outer_frame) ;
      Box(hdc, xi, yi, xf, yf, inner_frame) ;
      SetTextColor(hdc, label) ;

      TextOut(hdc, xi+10, yi+15, bfr, strlen(bfr)) ;

      xi += (gapx + dx) ;
      if (++line_idx >= 3) {
         line_idx = 0 ;
         xi = gapx ;
         yi += (gapy + dy) ;
      }
   }

   //*****************************************************
   //  do cleanup and exit
   //*****************************************************
   release_gframe_dc(hdc) ;
}
