#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h" 
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"
#include "rgb_data.h"

//***********************************************************************
xpalette::xpalette(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void xpalette::update_display()
{
   HBRUSH hBrush ;
   RECT   rect ;
   char tempstr[81];

   if (!we_should_redraw) 
      return ;

   //  xn = number of squares in x direction
   //  xg = size of gap between squares in X
   //  xo = size of gapes outside squares
   //  xs = size of square in X direction (calculated)
   //  xt = intermediate value: space not consumed by gaps,
   //                           i.e., space used by squares.
   //  xx = number of pixels left over
   //  xthold = ratio of extra pixels to number of squares
   //         = number of squares which will get an extra pixel
   //  
   //  if it works for x, it works for y
   int yn, yg, yo, ys, yt, yx ;
   int xn, xg, xo, xs, xt, xx ;
   int rowIdx, colIdx, idx ;
   int xl, yu, xr, yl ;
   double xthold, xrun, ythold, yrun ;
   int xruni, xprev, yruni, yprev ;

   HDC hdc = get_gframe_dc() ;
   Clear_Window(hdc, 0);

   //*********************************************************
   //                  do my computations                     
   //                                                         
   //  First model is a 16x16 matrix of squares, with gaps    
   //  between squares, 256 shades of one color.              
   //  Modifying CreateSolidBrush allowed observing           
   //  transitions from one primary color to another.         
   //*********************************************************

   //  calc horizontal layout
   xn = 24 ;   //  number of horiz squares
   xo = 10 ;   //  size of outside gap
   xg =  4 ;   //  size of inside gap
   xt = (cxGFrame - ((xn-1)*xg) - (2 * xo)) ;
   xs = xt / xn ;
   xx = xt % xn ;
   xthold = (double) xx / (double) xn ;

   //  calc vertical layout
   yn = NBR_RGB_ITEMS / xn;   //  number of vertical squares
   if ((NBR_RGB_ITEMS % xn) != 0) //lint !e774
      yn++ ;
   yo = 10 ;
   yg =  4 ;
   yt = (cyGFrame - ((yn-1)*yg) - (2 * yo)) ;
   ys = yt / yn ;
   yx = yt % yn ;
   ythold = (double) yx / (double) yn ;

   //*****************************************************
   //  now, draw some squares!!
   //*****************************************************
   idx = 0 ;
   yu = yo ;
   yl = yu + ys ;

   //  initialize the fill vars
   yrun = 0.0 ;
   yprev = 0 ;
   // yruni = 0 ;

   for (rowIdx=0; rowIdx<yn; rowIdx++) {
      xrun = 0.0 ;
      xprev = 0 ;
      // xruni = 0 ;
      xl = xo ;
      xr = xl + xs ;
      for (colIdx=0; colIdx<xn; colIdx++) {

         if (idx <= NBR_RGB_ITEMS) {
            hBrush = CreateSolidBrush (drgb[idx].value) ;
            SetRect (&rect, xl, yu, xr, yl) ;
         
            FillRect (hdc, &rect, hBrush) ;
            DeleteObject (hBrush) ;

            //  write the color index within the block
            SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT)) ;
            wsprintf(tempstr, "%d", idx) ;
            // dprints(hdc, xl+5, yu+5, 0x1C, tempstr) ;
            SetBkMode(hdc, TRANSPARENT) ;
            SetTextColor(hdc, WIN_BLACK) ;
            TextOut (hdc, xl+5, yu+5, tempstr, strlen(tempstr));
            SetBkMode(hdc, OPAQUE) ;

         }

         xl = xr + xg ;
         xr = xl + xs ;

         //  update the fill vars
         xrun += xthold ;
         xruni = (int) xrun ;
         if (xruni > xprev)
            xr++ ;
         xprev = xruni ;

         idx++ ;
      }  //  loop thru all columns on one bar
      yu = yl + yg ;
      yl = yu + ys ;

      //  update the fill vars
      yrun += ythold ;
      yruni = (int) yrun ;
      if (yruni > yprev)
         yl++ ;
      yprev = yruni ;
   }  //  loop over all horizontal bars

   release_gframe_dc(hdc) ;
}
