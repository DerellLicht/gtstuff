#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"    //  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

//***********************************************************************
colorbars::colorbars(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void colorbars::update_display()
{
   HBRUSH hBrush ;
   RECT   rect ;

   if (!we_should_redraw) 
      return ;

   // SetWindowText(hwnd, title) ;
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
   unsigned max_colors = 256;

   //  calc vertical layout
   yn = 20 ;   //  number of vertical squares
   yo = 10 ;
   yg =  5 ;
   yt = (cyGFrame - ((yn-1)*yg) - (2 * yo)) ;
   ys = yt / yn ;
   yx = yt % yn ;
   ythold = (double) yx / (double) yn ;

   //*****************************************************
   //  now, draw some squares!!
   //*****************************************************
   // idx = 0 ;
   yu = yo ;
   yl = yu + ys ;

   //  initialize the fill vars
   yrun = 0.0 ;
   yprev = 0 ;
   // yruni = 0 ;

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

   for (rowIdx=0; rowIdx<yn; rowIdx++) {
      switch (rowIdx) {
      case  4:  set_DAC_table(0)  ;  max_colors = get_palette_entries(0)  ;  break ;
      case  5:  set_DAC_table(1)  ;  max_colors = get_palette_entries(1)  ;  break ;
      case  6:  set_DAC_table(4)  ;  max_colors = get_palette_entries(4)  ;  break ;
      case  7:  set_DAC_table(6)  ;  max_colors = get_palette_entries(6)  ;  break ;
      case  8:  set_DAC_table(8)  ;  max_colors = get_palette_entries(8)  ;  break ;
      case  9:  set_DAC_table(14) ;  max_colors = get_palette_entries(14)  ;  break ;
      case 10:  set_DAC_table(16) ;  max_colors = get_palette_entries(16)  ;  break ;
      case 11:  set_DAC_table(22) ;  max_colors = get_palette_entries(22)  ;  break ;
      case 12:  set_DAC_table(24) ;  max_colors = get_palette_entries(24)  ;  break ;
      case 13:  set_DAC_table(25) ;  max_colors = get_palette_entries(25)  ;  break ;
      case 17:  set_DAC_table(2)  ;  max_colors = get_palette_entries(2)  ;  break ;
      case 18:  set_DAC_table(5)  ;  max_colors = get_palette_entries(5)  ;  break ;
      case 19:  set_DAC_table(28) ;  max_colors = get_palette_entries(28) ;  break ;

      default:
         max_colors = 256 ;
         break;
      }
      // wsprintf(tempstr, "row %u: max_colors=%u\n", rowIdx, max_colors) ;
      // OutputDebugString(tempstr) ;

      //  calc horizontal layout
      xn = max_colors ;   //  number of horiz squares
      xo = 10 ;   //  outside (boundary) gaps
      xg =  0 ;   //  inside gaps
      xt = (cxGFrame - ((xn-1)*xg) - (2 * xo)) ;
      xs = xt / xn ; //  object space
      xx = xt % xn ; //  leftover space
      // xx = cxClient / xx ; //  extra pixel every xx
      xthold = (double) xx / (double) xn ;

      xrun = 0.0 ;
      xprev = 0 ;
      // xruni = 0 ;
      xl = xo ;
      xr = xl + xs ;
      for (colIdx=0; colIdx<xn; colIdx++) {
         idx = colIdx ;
         switch (rowIdx) {
         //  single-color bars
         case 0: hBrush = CreateSolidBrush (RGB (idx, idx, idx)) ; break;
         case 1: hBrush = CreateSolidBrush (RGB (idx, 0, 0)) ; break;
         case 2: hBrush = CreateSolidBrush (RGB (255, idx, 0)) ; break;
         case 3: hBrush = CreateSolidBrush (RGB (0, 0, idx)) ; break;

         //  special-palette bars
         case  4:
         case  5: 
         case  6: 
         case  7: 
         case  8: 
         case  9: 
         case 10: 
         case 11: 
         case 12: 
         case 13: 
         case 17:
         case 18:
         case 19:
            hBrush = CreateSolidBrush (get_palette_entry(colIdx)) ; break;

         // some AB -> BA transitions
         case 14: hBrush = CreateSolidBrush (RGB (idx, 255-idx, 0)) ; break;
         case 15: hBrush = CreateSolidBrush (RGB (255-idx, 0, idx)) ; break;
         case 16: hBrush = CreateSolidBrush (RGB (0, idx, 255-idx)) ; break;

         //  default, grey-scale fill pattern
         default:
            hBrush = CreateSolidBrush (RGB (idx, idx, idx)) ;
            break;
         }

         SetRect (&rect, xl, yu, xr, yl) ;
         FillRect (hdc, &rect, hBrush) ;
         DeleteObject (hBrush) ;

         xl = xr ;
         xr = xl + xs ;
         // if (colIdx != 0  &&  (colIdx % xx) == 0) 
         //    xr++ ;

         //  update the fill vars
         xrun += xthold ;
         xruni = (int) xrun ;
         if (xruni > xprev)
            xr++ ;
         xprev = xruni ;

         // idx++ ;
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

   //*****************************************************
   //  do cleanup and exit
   //*****************************************************
   set_DAC_table(0) ;
   release_gframe_dc(hdc) ;
}
