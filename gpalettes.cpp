#include <windows.h>
#include <stdio.h>   //  sprintf for %*s 

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

#include "rgb_data.h"
#include "cheetah.def"
#include "hometown.def"

//***********************************************************************
gpalettes::gpalettes(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void gpalettes::update_display()
{
   int xl, yu, idx ;
   // int xr, yl ;
   int maxlen, maxidx, slen, j ;
   // typedef struct tagSIZE { // siz 
   //     LONG cx; 
   //     LONG cy; 
   // } SIZE; 
   SIZE strsz ;
   char tempstr[81];

   if (!we_should_redraw) 
      return ;

   // SetWindowText(hwnd, title) ;

   HDC hdc = get_gframe_dc() ;
   Clear_Screen(hdc, 0) ;
   //*********************************************************
   //  find the size of the longest string, with a blank
   //  char fore and aft.
   //*********************************************************
   maxidx = 0 ;
   maxlen = 0 ;
   for (j=0; j<256; j++) {
      slen = strlen(cheetah[j].name) ;
      if (maxlen < slen) {
         maxlen = slen ;
         maxidx = j ;
      }
   }
   for (j=0; j<256; j++) {
      slen = strlen(hometown[j].name) ;
      if (maxlen < slen) {
         maxlen = slen ;
         maxidx = 256+j ;
      }
   }

   if (maxidx < 256) {
      wsprintf(tempstr, " %s ", cheetah[maxidx].name) ;
   } else {
      wsprintf(tempstr, " %s ", hometown[maxidx-256].name) ;
   }

   if (GetTextExtentPoint32(hdc, tempstr, strlen(tempstr), &strsz) == 0) {
      syslog("GetTextExtentPoint32 failed: %lu\n", GetLastError()) ;
   }
   // wsprintf(tempstr, "string is ( %u, %u )", strsz.cx, strsz.cy) ;
   // MessageBox(hwnd, tempstr, "string size", MB_ICONEXCLAMATION) ;

   //*********************************************************
   //  to make things fit a little better onscreen,
   //  skip all the "grayNN" entries at end of list.
   //*********************************************************

   //  loop over all available rows
   // int amode = 0 ; //  0=drawing first array, 1=drawing second array
   idx = 0 ;
   yu = 0 ;
   while (1) {
      // yl = yu + strsz.cy - 1;
      //  fit as many as fit, on this row
      xl = 0 ;
      while (1) {
         // xr = xl + strsz.cx - 1 ;

         //  draw the string
         SetTextColor(hdc, 0) ;
         // if (amode == 0) {
         if (idx < 256) {
            SetBkColor  (hdc, cheetah[idx].value) ;
            if (!(get_curr_palette() & 1)) {
               sprintf(tempstr, " %-*s ", maxlen, cheetah[idx].name) ; //  for %*s 
            } else {
               wsprintf(tempstr, " (%3d, %3d, %3d) ", 
                  cheetah[idx].red, cheetah[idx].green, cheetah[idx].blue) ;
            }
         }  //  drawing first array
         else {
            SetBkColor  (hdc, hometown[idx-256].value) ;
            // if (get_curr_palette() == 0) {
            if (!(get_curr_palette() & 1)) {
               sprintf(tempstr, " %-*s ", maxlen, hometown[idx-256].name) ; //  for %*s 
            } else {
               wsprintf(tempstr, " (%3d, %3d, %3d) ", 
                  hometown[idx-256].red, hometown[idx-256].green, hometown[idx-256].blue) ;
            }
         }  //  drawing second array
         TextOut (hdc, xl, yu, tempstr, strlen(tempstr));

         idx++ ;
         // if (idx == 256)
         //    amode = 1 ;
         if (idx == 512)
            break;
         xl += strsz.cx + 3 ;
         if ((xl + strsz.cx) > cxGFrame)
            break;
      }  //  draw one row
      if (idx == 512)
         break;
      yu += strsz.cy + 3 ;
      if ((yu + strsz.cy) > cyGFrame)
         break;
   }

   release_gframe_dc(hdc) ;
}
