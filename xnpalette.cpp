#include <windows.h>
#include <stdio.h>      //  sprintf for %*s 

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"
#include "rgb_data.h"

//***********************************************************************
xnpalette::xnpalette(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void xnpalette::update_display()
{
   int xl, yu, idx ;
   int maxlen, maxidx, slen, j ;
   // typedef struct tagSIZE { // siz 
   //     LONG cx; 
   //     LONG cy; 
   // } SIZE; 
   SIZE strsz ;
   char tempstr[81];

   if (!we_should_redraw) 
      return ;

   HDC hdc = get_gframe_dc() ;
   Clear_Window(hdc, 0);

   //*********************************************************
   //  find the size of the longest string, with a blank
   //  char fore and aft.
   //*********************************************************
   maxidx = 0 ;
   // if (dac_mode == 0) {
      maxlen = 0 ;
      for (j=0; drgb[j].name != 0; j++) {
         
         slen = strlen(drgb[j].name) ;
         if (maxlen < slen) {
            maxlen = slen ;
            maxidx = j ;
         }
      }
      wsprintf(tempstr, " %s ", drgb[maxidx].name) ;
   // } else {
   //    wsprintf(tempstr, " (%3d, %3d, %3d) ", 
   //       drgb[maxidx].red, drgb[maxidx].green, drgb[maxidx].blue) ;
   // }
   if (GetTextExtentPoint32(hdc, tempstr, strlen(tempstr), &strsz) == 0) {
      wsprintf(tempstr, "GetTextExtentPoint32 failed: %lu", GetLastError()) ;
      MessageBox(NULL, tempstr, "duh...", MB_ICONEXCLAMATION) ;
   }
   // syslog("string is ( %u, %u )", strsz.cx, strsz.cy) ;

   //*********************************************************
   //  to make things fit a little better onscreen,
   //  skip all the "grayNN" entries at end of list.
   //*********************************************************

   //  loop over all available rows
   idx = 0 ;
   yu = 0 ;
   while (LOOP_FOREVER) {
      // yl = yu + strsz.cy - 1;
      //  fit as many as fit, on this row
      xl = 0 ;
      while (LOOP_FOREVER) {
         // xr = xl + strsz.cx - 1 ;

         //  draw the string
         SetTextColor(hdc, 0) ;
         SetBkColor  (hdc, drgb[idx].value) ;
         //  this should *not* use palette, it should just have a toggle
         // if (!(get_curr_palette() & 1)) {
         if (!run_custom_op) {
            //  wsprintf() not used here, because it doesn't support length argument
            sprintf(tempstr, " %-*s ", maxlen, drgb[idx].name) ;
         } else {
            wsprintf(tempstr, " (%3d, %3d, %3d) ", 
               drgb[idx].red, drgb[idx].green, drgb[idx].blue) ;
         }
         TextOut (hdc, xl, yu, tempstr, strlen(tempstr));

         idx++ ;
         if (drgb[idx].name == 0  ||  strncmp(drgb[idx].name, "gray", 4) == 0)
            break;
         xl += strsz.cx + 3 ;
         if ((xl + strsz.cx) > cxGFrame)
            break;
      }  //  draw one row
      if (drgb[idx].name == 0  ||  strncmp(drgb[idx].name, "gray", 4) == 0)
         break;
      yu += strsz.cy + 3 ;
      if ((yu + strsz.cy) > cyGFrame)
         break;
   }  

   release_gframe_dc(hdc) ;
}
