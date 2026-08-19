#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

//***********************************************************************
sglass::sglass(std::string title_text) : graph_object(std::move(title_text)) 
, max_col(0)    
, max_row(0)
, x(0)
, y(0)
, distfact(2)   /* multiplier for distance */
, size(75)      /* max size to grow to     */
, osize(50)     /* same                    */
, limit(1)      /* min. size of box - one row/col */
, in_size(1)    /* starting size                */
, o_size(0)     /* same                         */
, unoo(1)       /* one - changed from + to -    */
, onoo(1)       /* same, for other drawing      */

//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
   //  these are initialized in update_display(), after WM_SIZE is processed
   //lint -esym(1401, sglass::max_col, sglass::max_row, sglass::x, sglass::y, sglass::o_size)
}

/*******************************************************************/
/*                         box_box  -  draw boxes                  */
/*******************************************************************/
void sglass::box_box (HDC hdc, int col_inpt, int row_inpt, int siz)
   {
   int newsz, d = 0;

   /* check for limits */
   if (siz > limit &&
      col_inpt + siz < max_col &&
      col_inpt - siz > 0 &&
      row_inpt + siz < max_row &&
      row_inpt - siz > 0)
      {

      // MoveToEx(hdc, x0, y0, NULL) ;
      MoveToEx(hdc, col_inpt + siz, row_inpt - siz, NULL); /* draw box */
      LineTo (hdc, col_inpt + siz, row_inpt + siz);
      LineTo (hdc, col_inpt - siz, row_inpt + siz);
      LineTo (hdc, col_inpt - siz, row_inpt - siz);
      LineTo (hdc, col_inpt + siz, row_inpt - siz);

      d = distfact * siz;       /* d represents 2 * curr size   */
      newsz = siz / 2;          /* halve the size of the box    */

      /* The following is the recursive part of the call              */

      box_box (hdc, col_inpt, row_inpt - d, newsz);  /* to the top   */
      box_box (hdc, col_inpt, row_inpt + d, newsz);  /* to the bottom */
      box_box (hdc, col_inpt + d, row_inpt, newsz);  /* to the right */
      box_box (hdc, col_inpt - d, row_inpt, newsz);  /* to the left  */
      }
   }

/*********************************************************************/
/*                         box_point  - draw diamonds                */
/*                     [same as box_box, essentially                 */
/*********************************************************************/
void sglass::box_point (HDC hdc, int ccol, int rrow, int ssiz)
{
   int news = 0, dd = 0 ;

   if (ssiz > limit &&
      ccol + ssiz < max_col &&
      ccol - ssiz > 0 &&
      rrow + ssiz < max_row &&
      rrow - ssiz > 0)
      {

      MoveToEx (hdc, ccol + ssiz, rrow, NULL);  /* draw diamond */
      LineTo (hdc, ccol, rrow + ssiz);
      LineTo (hdc, ccol - ssiz, rrow);
      LineTo (hdc, ccol, rrow - ssiz);
      LineTo (hdc, ccol + ssiz, rrow);


      dd = distfact * ssiz;
      news = ssiz / 2;

      box_point (hdc, ccol, rrow + dd, news);  /* to the bottom */
      box_point (hdc, ccol, rrow - dd, news);  /* to the top   */
      box_point (hdc, ccol - dd, rrow, news);  /* to the left  */
      box_point (hdc, ccol + dd, rrow, news);  /* to the right */

      }
}

//************************************************************************
void sglass::update_display()
{
   static HPEN hPen ;
   int HC ;
   static unsigned delay = 0 ;

   if (pause_the_race)
      return ;
      
   HDC hdc = get_gframe_dc() ;
   if (we_should_redraw) {
      delay = 0 ;
      Clear_Window(hdc, 0);
   }

   //  in gstuff, delay was 300000
   if (++delay < 300) {
      goto error_exit;
   }
   delay = 0 ;

   cycle_count++ ;
   max_col = cxGFrame;               /* max col on EGA */
   max_row = cyGFrame;               /* max row        */

   x = max_col / 2;             /* middle       */
   y = max_row / 2;
   o_size = cyGFrame / 9 ;                 /* same                         */

   //************************************
   //  draw diamond pattern
   //************************************
   HC = random_int (256);         /* get random color             */
   hPen = CreatePen(PS_SOLID, 1, get_palette_entry(HC)) ;
   SelectObject(hdc, hPen) ;

   box_point (hdc, x, y, in_size);/* call diamond draw            */
   in_size += unoo;          /* add [ or subtract ] one      */
   if (in_size == size)
      unoo = -1;             /* if max, switch to subtract   */
   if (in_size == 1)
      unoo = 1;              /* if min, switch to add        */

   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;

   //************************************
   //  draw box pattern
   //************************************
   HC = random_int (256);         /* get random color             */
   hPen = CreatePen(PS_SOLID, 1, get_palette_entry(HC)) ;
   SelectObject(hdc, hPen) ;

   box_box (hdc, x, y, o_size);   /* call box draw                */
   o_size += onoo;           /* add[or subtract] one          */
   if (o_size == osize)
      onoo = -1;             /* if max, set to subtract      */
   if (o_size == 1)
      onoo = 1;              /* if min, set to add           */

   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;

error_exit:
   release_gframe_dc(hdc) ;
}
