/*---------------------------------------
   EZFONT.C -- Easy Font Creation
               (c) Charles Petzold, 1996
  ---------------------------------------*/

#include <windows.h>
#include <math.h>
#include <errno.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "gfuncs.h"
#include "ezfont.h"

//lint -esym(714, init_msg_window, clear_message_area, textheight, textwidth, gotoxy)
//lint -esym(759, init_msg_window, clear_message_area, textheight, textwidth, gotoxy)
//lint -esym(765, init_msg_window, clear_message_area, textheight, textwidth, gotoxy)

//lint -esym(714, set_text_attributes, set_default_font, set_text_angle, set_message_area_font)
//lint -esym(759, set_text_attributes, set_default_font, set_text_angle, set_message_area_font)
//lint -esym(765, set_text_attributes, set_default_font, set_text_angle, set_message_area_font)

//lint -esym(714, set_text_color, set_text_font, set_text_size, dputs, dprints, dprints_centered_x)
//lint -esym(759, set_text_color, set_text_font, set_text_size, dputs, dprints, dprints_centered_x)
//lint -esym(765, set_text_color, set_text_font, set_text_size, dputs, dprints, dprints_centered_x)

//*************************************************************************
HFONT build_font(char const * const fname, unsigned fheight, unsigned fbold, unsigned fitalic, unsigned funderline, unsigned fstrikeout)
{
   HFONT hfont = CreateFont(
         //  46, 28, 215, 0,  //  height, width, escapement, orientation
         fheight, 0, 0, 0,
         (fbold == 0) ? FW_NORMAL : FW_BOLD,
         fitalic,
         funderline,
         fstrikeout,
         DEFAULT_CHARSET, 0, 0, 0, DEFAULT_PITCH, //  other stuff
         fname);
   return hfont;
}

//*************************************************************************
static HFONT EzCreateFont(HDC hdc, char * szFaceName, int iDeciPtHeight,
       int iDeciPtWidth, unsigned iAttributes, int textangle, BOOL fLogRes)
{
   FLOAT      cxDpi, cyDpi ;
   HFONT      hFont ;
   LOGFONT    lf ;
   POINT      pt ;
   TEXTMETRIC tm ;

   SaveDC (hdc) ;

   SetGraphicsMode (hdc, GM_ADVANCED) ;
   ModifyWorldTransform (hdc, NULL, MWT_IDENTITY) ;
   SetViewportOrgEx (hdc, 0, 0, NULL) ;
   SetWindowOrgEx   (hdc, 0, 0, NULL) ;

   if (fLogRes) {
      cxDpi = (FLOAT) GetDeviceCaps (hdc, LOGPIXELSX) ;
      cyDpi = (FLOAT) GetDeviceCaps (hdc, LOGPIXELSY) ;
   }
   else {
      cxDpi = (FLOAT) (25.4 * GetDeviceCaps (hdc, HORZRES) /
                              GetDeviceCaps (hdc, HORZSIZE)) ;

      cyDpi = (FLOAT) (25.4 * GetDeviceCaps (hdc, VERTRES) /
                              GetDeviceCaps (hdc, VERTSIZE)) ;
   }

   pt.x = (long) (iDeciPtWidth  * cxDpi / 72) ;
   pt.y = (long) (iDeciPtHeight * cyDpi / 72) ;

   DPtoLP (hdc, &pt, 1) ;

   lf.lfHeight         = - (int) (fabs ((double) pt.y) / 10.0 + 0.5) ;
   lf.lfWidth          = 0 ;
   lf.lfEscapement     = textangle ;
   lf.lfOrientation    = textangle ;
   lf.lfWeight         = iAttributes & EZ_ATTR_BOLD      ? 700 : 0 ;
   lf.lfItalic         = iAttributes & EZ_ATTR_ITALIC    ?   1 : 0 ;
   lf.lfUnderline      = iAttributes & EZ_ATTR_UNDERLINE ?   1 : 0 ;
   lf.lfStrikeOut      = iAttributes & EZ_ATTR_STRIKEOUT ?   1 : 0 ;
   lf.lfCharSet        = 0 ;
   lf.lfOutPrecision   = 0 ;
   lf.lfClipPrecision  = 0 ;
   lf.lfQuality        = 0 ;
   lf.lfPitchAndFamily = 0 ;
   strcpy (lf.lfFaceName, szFaceName) ;
   hFont = CreateFontIndirect (&lf) ;

   if (iDeciPtWidth != 0) {
      hFont = (HFONT) SelectObject (hdc, hFont) ;

      GetTextMetrics (hdc, &tm) ;

      DeleteObject (SelectObject (hdc, hFont)) ;

      lf.lfWidth = (int) (tm.tmAveCharWidth *
                          fabs ((double) pt.x) / fabs ((double) pt.y) + 0.5) ;

      hFont = CreateFontIndirect (&lf) ;
   }
   RestoreDC (hdc, -1) ;
   return hFont ;
}

//*************************************************************************
static HWND hwndMsg = 0 ;
static char ezfontname[MAX_PATH_LEN] = "" ;
static int eztextangle = 0 ;  //  in tenths-of-a-degree
static int ezattribs = 0 ;
#define  DEFAULT_PSIZE  (200)
static int ezpointsize = DEFAULT_PSIZE ;
static unsigned text_currX = 10 ;
static unsigned text_currY = 2 ;
static unsigned row_inc = 10 ;
static unsigned text_color = 0x00AAAA00 ;
static unsigned area_x = 0 ;
static unsigned area_y = 0 ;

static unsigned msg_base_x = 0 ;
static unsigned msg_base_y = 0 ;

//lint -esym(578, y0)
//*************************************************************************
void init_msg_window(HWND hwnd, unsigned x0, unsigned y0, unsigned dx, unsigned dy)
{
   hwndMsg = hwnd ;
   msg_base_x = x0 ;
   msg_base_y = y0 ;
   area_x = dx ;
   area_y = dy ;
   // text_color = GetSysColor(COLOR_WINDOW) ;
}

//*******************************************************************
//  handlers for message window
//*******************************************************************

//*************************************************************************
void clear_message_area(void)
{
   HBRUSH hBrush ;
   RECT   rect ;

   HDC hdc = GetDC (hwndMsg) ;
   SetRect (&rect, msg_base_x, msg_base_y, msg_base_x+area_x, msg_base_y+area_y) ;
   hBrush = CreateSolidBrush (GetSysColor(COLOR_WINDOW)) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
   ReleaseDC (hwndMsg, hdc) ;
}

//*************************************************************************
static void scroll_window(HDC hdc)
{
   unsigned xdest = msg_base_x ;
   unsigned ydest = msg_base_y ;
   unsigned xsrc = msg_base_x ;
   unsigned ysrc = msg_base_y+row_inc ;
   unsigned dx = area_x ;  
   unsigned dy = (msg_base_y+text_currY+row_inc) - ysrc ;
   
   //**********************************************
   // copy lower area to upper area
   //**********************************************
   BitBlt(hdc, xdest, ydest, dx, dy, hdc, xsrc, ysrc, SRCCOPY) ;

   //**********************************************
   //  then we need to clear the bottom area
   //**********************************************
   HBRUSH hBrush ;
   RECT   rect ;

   SetRect (&rect, msg_base_x, msg_base_y+text_currY, 
                   area_x,     msg_base_y+text_currY+row_inc) ;
   hBrush = CreateSolidBrush (GetSysColor(COLOR_WINDOW)) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

//*************************************************************************
static void dnewline(HDC hdc)
{
   if ((text_currY + (2*row_inc)) <= area_y) {
      text_currY += row_inc ;
   } else {
      scroll_window(hdc) ;
   }
}

//*************************************************************************
void gotoxy(HDC hdc, int x, int y)
{
   text_currX = x ;
   text_currY = y ;
   // row_inc = textheight(hdc) ;
}  //lint !e715

//*************************************************************************
void gotoxy(HWND hwnd, int x, int y)
{
   HDC hdc = GetDC (hwnd) ;
   gotoxy(hdc, x, y) ;
   ReleaseDC (hwnd, hdc) ;
}

//****************************************************************************
int textheight(HWND hwnd)
{
   int diffY ;
   TEXTMETRIC tm ;
   HFONT hfont ;
   HDC hdc = GetDC (hwnd) ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   // SelectObject (hdc, CrDataFont) ;
   GetTextMetrics(hdc, &tm) ;
   diffY = tm.tmHeight + tm.tmExternalLeading ;
   // DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
   ReleaseDC (hwnd, hdc) ;
   return diffY ;
}

//*****************************************************************
int textheight(HDC hdc, unsigned selected)
{
   int diffY ;
   TEXTMETRIC tm ;
   HFONT hfont ;

   if (!selected  &&  ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   GetTextMetrics(hdc, &tm) ;
   diffY = tm.tmHeight + tm.tmExternalLeading ;
   if (!selected  &&  ezfontname[0] != 0) 
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
   return diffY ;
}

//*****************************************************************
int textwidth(HDC hdc, char *str)
{
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   SIZE ssize ;
   GetTextExtentPoint32(hdc, str, strlen(str), &ssize) ;
   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
   return ssize.cx ;
}

//*************************************************************************
void set_text_color(unsigned new_color)
{
   text_color = new_color ;
}

//****************************************************************************
void set_text_attributes(unsigned new_attribs)
{
   ezattribs = new_attribs ;
}

//****************************************************************************
void set_text_angle(unsigned degs)
{
   if (degs > 360) 
      degs = degs % 360 ;
   eztextangle = degs * 10 ; //  convert to tents of a degree
}

//****************************************************************************
int set_text_font(char *fontname, int psize)
{
   ezpointsize = (psize == 0) ? DEFAULT_PSIZE : psize ;

   if (fontname == NULL) {
      ezfontname[0] = 0 ;
   } else if ((unsigned) strlen(fontname) >= sizeof(ezfontname)) {
      return EINVAL ;
   } else {
      strcpy(ezfontname, fontname) ;
   }
   return 0 ;
}

//****************************************************************************
void set_default_font(void)
{
   set_text_font("Times New Roman", 120);
}

//****************************************************************************
void set_text_size(int psize)
{
   ezpointsize = (psize == 0) ? DEFAULT_PSIZE : psize ;
}

//*******************************************************************
void set_message_area_font(void)
{
   set_text_font("Caligula", 140) ;
}

//*************************************************************************
void dputs(HDC hdc, char *str)
{
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
      row_inc = textheight(hdc, 1) ;
   }

   SetBkMode(hdc, TRANSPARENT) ;
   SetTextColor(hdc, text_color) ;
   TextOut (hdc, msg_base_x + text_currX, msg_base_y + text_currY, str, strlen(str));
   SetBkMode(hdc, OPAQUE) ;

   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));

   dnewline(hdc) ;
}

//*************************************************************************
void dputs(HDC hdc, char *str, unsigned attr)
{
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
      row_inc = textheight(hdc, 1) ;
   }

   SetBkMode(hdc, TRANSPARENT) ;
   SetTextColor(hdc, attr) ;
   TextOut (hdc, msg_base_x + text_currX, msg_base_y + text_currY, str, strlen(str));
   SetBkMode(hdc, OPAQUE) ;

   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));

   dnewline(hdc) ;
}

//****************************************************************************
void dprints(HDC hdc, int x, int y, unsigned attr, char *str)
{
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   SetBkMode(hdc, TRANSPARENT) ;
   SetTextColor(hdc, attr) ;
   TextOut (hdc, x, y, str, strlen(str));
   SetBkMode(hdc, OPAQUE) ;
   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
}         

//****************************************************************************
void dprints(HDC hdc, int x, int y, unsigned fg, unsigned bg, char *str)
{
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   SetTextColor(hdc, fg) ;
   SetBkColor  (hdc, bg) ;
   TextOut (hdc, x, y, str, strlen(str));
   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
}         

//*****************************************************************
void dprints_centered_x(HDC hdc, long y, unsigned attr, char *str)
{
   SIZE ssize ;
   long xt ;
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   GetTextExtentPoint32(hdc, str, strlen(str), &ssize) ;
   xt = (cxGFrame - ssize.cx) / 2 ;
   dprints(hdc, xt, y-(ssize.cy/2), attr, str) ;
   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
}

//*****************************************************************
void dprints_centered_x(HDC hdc, long y, unsigned fg, unsigned bg, char *str)
{
   SIZE ssize ;
   long xt ;
   HFONT hfont ;

   if (ezfontname[0] != 0) {
      hfont = EzCreateFont (hdc, ezfontname, ezpointsize, 0, ezattribs, eztextangle, TRUE) ;
      SelectObject (hdc, hfont) ;
   }
   GetTextExtentPoint32(hdc, str, strlen(str), &ssize) ;
   xt = (cxGFrame - ssize.cx) / 2 ;
   dprints(hdc, xt, y-(ssize.cy/2), fg, bg, str) ;
   if (ezfontname[0] != 0)
      DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
}

//*****************************************************************
void dprints_centered_x(HWND hwnd, long y, unsigned attr, char *str)
{
   HDC hdc = GetDC (hwnd) ;
   dprints_centered_x(hdc, y, attr, str) ;
   ReleaseDC (hwnd, hdc) ;
}

//*****************************************************************
void dprints_centered_x(HWND hwnd, long y, unsigned fg, unsigned bg, char *str)
{
   HDC hdc = GetDC (hwnd) ;
   dprints_centered_x(hdc, y, fg, bg, str) ;
   ReleaseDC (hwnd, hdc) ;
}

