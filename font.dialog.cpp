//********************************************************************************
//  this is used *only* by ascii class
//********************************************************************************

#undef  __STRICT_ANSI__

//  used by ascii class
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <tchar.h>

#include "resource.h"  
#include "common.h"  
#include "gtstuff.h" 
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"
#include "alg_selector.h"

//  ascii.cpp
// extern void set_font_name(char *new_font_name);
// extern char *get_font_name(void);
extern ascii ascii0 ;

//*********************************************************
static HWND hWndComboBox = 0 ;

//*********************************************************
typedef struct font_list_s {
   std::string name ;
   DWORD combo_box_idx {};
} font_list_t, *font_list_p ;

static std::vector<font_list_s> font_list ;

//*********************************************************
//  This intermediate function is used because I want
//  merge_sort() to accept a passed parameter,
//  but in this particular application the initial
//  list is global.  This function sets up the global
//  comparison-function pointer and passes the global
//  list pointer to merge_sort().
//*********************************************************
static void sort_font_list(void)
{
   // std::sort(font_list.begin(), font_list.end(), sort_name);

   std::sort(font_list.begin(), font_list.end(), [](const font_list_s& a, const font_list_s& b) {
      return (_stricmp(a.name.c_str(), b.name.c_str()) < 0) ;
   } ) ;
}

//***********************************************************************
static bool check_for_dupe(char *face_name)
{
   for(auto &fptr : font_list) {
      if (_tcscmp(face_name, fptr.name.c_str()) == 0)
         return true;
   }
   return false;
}

//***********************************************************************
//  this needs to drop duplicate entries, though...
//***********************************************************************
static void add_font_to_list(char *facename)
{
   if (check_for_dupe(facename))
      return ;
   font_list_p fptr = &font_list.emplace_back();
   fptr->name = facename ;
}

//***********************************************************************
// typedef struct tagENUMLOGFONTEX {
//   LOGFONT  elfLogFont;
//   TCHAR  elfFullName[LF_FULLFACESIZE];
//   TCHAR  elfStyle[LF_FACESIZE];
//   TCHAR  elfScript[LF_FACESIZE];
// } ENUMLOGFONTEX, *LPENUMLOGFONTEX;
static int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, 
                                           int FontType, LPARAM lParam )
{
   // LOGFONT *lfptr = &lpelfe->elfLogFont ;
   // printf( "%s, charset=%u, paf=%u\n", lfptr->lfFaceName, lfptr->lfCharSet, lfptr->lfPitchAndFamily );
   // printf( "%s, style=%s, script=%s\n", lpelfe->elfFullName, lpelfe->elfStyle, lpelfe->elfScript) ;
   add_font_to_list((char *) lpelfe->elfFullName) ;
   return 1;
}  //lint !e715

//***********************************************************************
static void build_font_list(void)
{
   HDC hDC = GetDC( NULL );
   LOGFONT lf = { 0, 0, 0, 0, 0, 0, 0, 0, 
      // ANSI_CHARSET,  //  lfCharSet
      DEFAULT_CHARSET,  //  lfCharSet - read everything, all languages
      0, 0, 0, 
      DEFAULT_PITCH,    //  lfPitchAndFamily
      // "Courier New" };
      { 0 } };              //  lfFaceName
   EnumFontFamiliesEx( hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, 0 );
   ReleaseDC( NULL, hDC );
}

//****************************************************************************
static void populate_combo_box(void)
{
   if (hWndComboBox == 0)
      return ;

   for(auto &fptr : font_list) {
      fptr.combo_box_idx = SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM) fptr.name.c_str()) ;
   }
}

//****************************************************************************
static DWORD get_current_font_index(void)
{
   char *cfptr = ascii0.get_font_name();

   for(auto &fptr : font_list) {
      if (_tcscmp(cfptr, fptr.name.c_str()) == 0) {
         return fptr.combo_box_idx ;
      }
   }
   return 0 ;
}

//****************************************************************************
static char const *get_selected_font(void)
{
   LRESULT cbresult = SendMessage(hWndComboBox, CB_GETCURSEL, 0, 0);
   if (cbresult == CB_ERR)
      return 0;
   for(auto &fptr : font_list) {
      if (fptr.combo_box_idx == (DWORD) cbresult) {
         return fptr.name.c_str();
      }
   }
   return 0;
}

//****************************************************************************
static BOOL CALLBACK FontDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   int iTemp ;
   LRESULT cblen ;

   switch (message) {
   case WM_INITDIALOG:
      //  move focus to exit button
      iTemp = IDC_FSELECT ;
      SetFocus (GetDlgItem (hDlg, iTemp)) ;

      //  populate the combo box
      hWndComboBox = GetDlgItem(hDlg, IDC_FCOMBO) ;
      populate_combo_box() ;
      SendMessage(hWndComboBox, CB_SETCURSEL, get_current_font_index(), 0);
      //  unfortunately, CB_GETDROPPEDWIDTH just returned the original creation
      //  width of the combo box, rather than data based on the contents...
      //  Thus, it accomplished nothing...
      //  Later note: This was because CB_GETDROPPEDWIDTH is only valid for
      //  boxes with CBS_DROPDOWN or CBS_DROPDOWNLIST style, which this box originally lacked.

//       cblen = SendMessage(hWndComboBox, CB_GETDROPPEDWIDTH, 0, 0);
//       if (cblen == CB_ERR) {
//          wsprintf(tempstr, "CB_GETDROPPEDWIDTH: %s\n", get_system_message()) ;
//          OutputDebugString(tempstr) ;
//       } else {
         // wsprintf(tempstr, "new ComboBox len=%u bytes\n", (unsigned) cblen) ;
         // OutputDebugString(tempstr) ;
         cblen = 300 ;
         SendMessage(hWndComboBox, CB_SETDROPPEDWIDTH, cblen, 0);
//       }
      //  this initially works, but some other message is causing the
      //  dialog to get closed again afterwards...
      // SendMessage(hWndComboBox, CB_SHOWDROPDOWN, TRUE, 0);  
      break;
        
   // case WM_SIZE:  //  nope, that's not it...
   //    // OutputDebugString("WM_SIZE\n") ;
   //    cxClient = LOWORD (lParam) ;
   //    cyClient = HIWORD (lParam) ;
   //    syslog("WM_SIZE was received\n") ;
   //    return FALSE;

   //  this *does* make CB_SHOWDROPDOWN work, but also trashes
   //  the entire application!!!
   // case WM_PAINT:
   //    syslog("WM_PAINT was received\n") ;
   //    SendMessage(hWndComboBox, CB_SHOWDROPDOWN, TRUE, 0);  
   //    break;
    
   case WM_COMMAND:
      switch (LOWORD (wParam)) {
      case IDC_FSELECT:
         // iCurrentColor  = iRace ;
         // iCurrentFigure = iFigure ;
         ascii0.set_font_name(get_selected_font());
         EndDialog (hDlg, TRUE) ;
         return TRUE ;
           
      }  //lint !e744  switch on target checkbox
      break;

   default:
      // wsprintf(tempstr, "%u was received\n", message) ;
      // OutputDebugString(tempstr) ;
      break;
   }  //lint !e744
   return FALSE ;
}  //lint !e715

//***********************************************************************
void display_font_list(void)
{
   // font_list_p fptr ;
   syslog("found %u fonts\n", font_list.size()) ;
   // OutputDebugString(tempstr) ;
   // for (fptr=font_list; fptr != 0; fptr = fptr->next) 
   //    puts(fptr->name) ;
}

//****************************************************************************
int read_a_font(HWND hwnd)
{
   if (font_list.empty()) {
      build_font_list() ;
      sort_font_list() ;
      // display_font_list() ;
   }

   if (DialogBox ((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), TEXT ("FontBox"), hwnd, FontDlgProc)) {
        InvalidateRect (hwnd, NULL, TRUE) ;
   }

   return 0;
}

