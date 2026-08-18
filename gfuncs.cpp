//************************************************************************
//  File: DEMO.C, the Windows portion
//  This is a main program which is used to illustrate use of 
//  drawing routines in the text of 
//  "Advanced Programmer's Guide to SuperVGA".                         
//                         
//  12/06/01 07:15 - 
//  It now demonstrates drawing routines in the Windows GDI interface.
//************************************************************************

#include <windows.h>

#include "common.h"
#include "gfuncs.h"
#include "gtstuff.h"
#include "palettes.h"

//  duplicate standard DOS colors
// unsigned dos_cref[16] = {
// 0x00000000,
// 0x00AA0000,
// 0x0000AA00,
// 0x00AAAA00,
// 0x000000AA,
// 0x00AA00AA,
// 0x000055AA,
// 0x00AAAAAA,
// 0x00555555,
// 0x00FF5555,
// 0x0055FF55,
// 0x00FFFF55,
// 0x005555FF,
// 0x00FF55FF,
// 0x0055FFFF,
// 0x00FFFFFF
// } ;

//***********************************************************************
unsigned fill_patterns[6] = {
   HS_HORIZONTAL,
   HS_VERTICAL,
   HS_FDIAGONAL,
   HS_BDIAGONAL,
   HS_CROSS,
   HS_DIAGCROSS
};

//**************************************************************************
//  turn random number into a floating-point percentage
//  this is used by rainbow
//**************************************************************************
double random_part(void)
{
   return (double) rand() / (RAND_MAX + 1.0) ;
}

/************************************************************************/
COLORREF random_palette_ref(void)
{
   return get_palette_entry(random_int(256)) ;
}

/************************************************************************/
int random_int(int n)
{
   //  turn random number into a floating-point percentage
   double d = (double) rand() / (RAND_MAX + 1.0) ;
   //  then calculate result as that random percentage of the target value
   return (int) (n * d) ;
}

/************************************************************************/
COLORREF random_colorref(void)
{
   return RGB(random_int(256), random_int(256), random_int(256)) ;
}

/************************************************************************/
void Clear_Window(HDC hdc, unsigned Color)
{
   RECT   rect ;
   SetRect (&rect, 0, 0, cxGFrame, cyGFrame) ;
   HBRUSH hBrush = CreateSolidBrush (Color) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

/************************************************************************/
//  clear window using specified color (given hwnd)
/************************************************************************/
void Clear_Window(HWND hwnd, unsigned Color)
{
   // RECT rect ;
   HDC hdc = GetDC (hwnd) ;
   Clear_Window(hdc, Color) ;
   // SetRect (&rect, 0, 0, cxGFrame, cyGFrame) ;
   // HBRUSH hBrush = CreateSolidBrush (Color) ;
   // FillRect (hdc, &rect, hBrush) ;
   // DeleteObject (hBrush) ;
   ReleaseDC (hwnd, hdc) ;
}

/************************************************************************/
//  clear window using specified color (given hdc)
/************************************************************************/
//lint -esym(714, Clear_Screen)
//lint -esym(759, Clear_Screen)
//lint -esym(765, Clear_Screen)
void Clear_Screen(HDC hdc, BYTE Color)
{
   RECT   rect ;
   SetRect (&rect, 0, 0, cxGFrame, cyGFrame) ;
   HBRUSH hBrush = CreateSolidBrush (get_palette_entry(Color)) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

/************************************************************************/
//  clear window using system background color
/************************************************************************/
void Clear_Screen(HDC hdc)
{
   RECT   rect ;
   SetRect (&rect, 0, 0, cxGFrame, cyGFrame) ;
   HBRUSH hBrush = CreateSolidBrush (GetSysColor(COLOR_WINDOW)) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

//************************************************************************
void Solid_Rect(HDC hdc, int xl, int yu, int xr, int yl, BYTE Color)
{
   RECT   rect ;
   SetRect (&rect, xl, yu, xr, yl) ;
   HBRUSH hBrush = CreateSolidBrush (get_palette_entry(Color)) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

//************************************************************************
void SolidRect(HDC hdc, int xl, int yu, int xr, int yl, COLORREF Color)
{
   RECT   rect ;
   SetRect (&rect, xl, yu, xr, yl) ;
   HBRUSH hBrush = CreateSolidBrush (Color) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

/************************************************************************/
// ;* Description: Draw line from (x0,y0) to (x1,y1) using color 'Color'   *
/************************************************************************/
void Line(HDC hdc, int x0, int y0, int x1, int y1, BYTE Color)
{
   HPEN hPen = CreatePen(PS_SOLID, 1, get_palette_entry(Color)) ;
   SelectObject(hdc, hPen) ;

   MoveToEx(hdc, x0, y0, NULL) ;
   LineTo  (hdc, x1, y1) ;

   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;
}

/************************************************************************/
void LineCR(HDC hdc, int x0, int y0, int x1, int y1, COLORREF Color)
{
   HPEN hPen = CreatePen(PS_SOLID, 1, Color) ;
   SelectObject(hdc, hPen) ;

   MoveToEx(hdc, x0, y0, NULL) ;
   LineTo  (hdc, x1, y1) ;

   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;
}

/************************************************************************/
void Box(HDC hdc, int x0, int y0, int x1, int y1, COLORREF Color)
{
   HPEN hPen = CreatePen(PS_SOLID, 1, Color) ;
   SelectObject(hdc, hPen) ;

   MoveToEx(hdc, x0, y0, NULL) ;
   LineTo  (hdc, x1, y0) ;
   LineTo  (hdc, x1, y1) ;
   LineTo  (hdc, x0, y1) ;
   LineTo  (hdc, x0, y0) ;

   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;
}

//************************************************************************
// Arguments:   Source X, Source Y, Destination X, Destination Y,       *
//              Width, Height                                           *
//************************************************************************
void dBitBlt(HDC hdc, int srcX, int srcY, int destX, int destY, 
             int width, int height, unsigned Color)
{
   BitBlt(hdc, destX, destY, width, height, 
          hdc, srcX, srcY, Color) ;
}

