#include <windows.h>

#include "gstuff.h"     //  u8, etc
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

//*******************************************************
class gtemplate: public graph_object {
private:

   //  private functions
      
public:
   gtemplate(char *title_text) ;
   void update_display(HWND hwnd) ;
   bool process_key(unsigned key_id) ;
} ;

//***********************************************************************
gtemplate::gtemplate(char *title_text) 
: graph_object(title_text) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void gtemplate::update_display(HWND hwnd)
{
}

//************************************************************************
//  return TRUE if we handled this key,
//  FALSE if we did not
//************************************************************************
bool gtemplate::process_key(unsigned key_id)
{
   return FALSE ;
}

