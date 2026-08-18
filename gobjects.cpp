//************************************************************************
//  gobjects - a class to manage graphics objects for my gstuff program.
//************************************************************************

#include <windows.h>

#include <utility>      //  std::move

#include "common.h"     //  u8, etc
#include "gtstuff.h"    //  get_hwndGFrame() -- see gobjects.h note; this is
                         //  the one other file besides gtstuff.cpp itself
                         //  that knows hwndGFrame exists at all
#include "gfuncs.h"     //  cxGFrame/cyGFrame
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes

graph_object::graph_object(std::string title_text) :
   title(std::move(title_text))
{
}

//****************************************************************************
//  Claude 08/17/26 - GetDC(hwndGFrame) does not actually clip to hwndGFrame's
//  own bounds -- it inherits the parent dialog's clip region instead (see
//  conversation history for the full diagnosis: circles/squares were
//  painting over the menu bar and status bar because of this). hwndGFrame
//  also has no CS_OWNDC, so every GetDC()/ReleaseDC() pair gets a fresh
//  common DC whose attributes -- clip region included -- reset to class
//  defaults on release. That means the clip region has to be re-established
//  on *every* call, not just once at startup; the cost of doing so
//  (CreateRectRgn/SelectClipRgn/DeleteObject on a plain rectangle) is
//  negligible next to the actual drawing work each subclass does per frame.
//****************************************************************************
HDC graph_object::get_gframe_dc(void)
{
   HDC hdc = GetDC(get_hwndGFrame()) ;
   HRGN clip = CreateRectRgn(0, 0, (int) cxGFrame, (int) cyGFrame) ;
   SelectClipRgn(hdc, clip) ;
   DeleteObject(clip) ;
   return hdc ;
}

//****************************************************************************
void graph_object::release_gframe_dc(HDC hdc)
{
   ReleaseDC(get_hwndGFrame(), hdc) ;
}


