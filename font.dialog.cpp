//********************************************************************************
//  this is used *only* by ascii class
//********************************************************************************

#undef  __STRICT_ANSI__

//  used by ascii class
#include <windows.h>

#include "resource.h"  
#include "common.h"  
// #include "gstuff.h"     //  for timer functions
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"
#include "alg_selector.h"

extern HINSTANCE g_hinst ;

//lint -esym(714, display_font_list)
//lint -esym(765, display_font_list)

//  ascii.cpp
// extern void set_font_name(char *new_font_name);
// extern char *get_font_name(void);
extern ascii ascii0 ;

//*********************************************************
static HWND hWndComboBox = 0 ;

//*********************************************************
typedef struct font_list_s {
   struct font_list_s *next ;
   char name[LF_FULLFACESIZE] ;
   DWORD combo_box_idx ;
} font_list_t, *font_list_p ;

static font_list_p font_list = 0 ;
static font_list_p font_tail = 0 ;
static unsigned font_count = 0 ;

//*********************************************************
static int sort_name(font_list_p a, font_list_p b)
{
   return(strcmpi(a->name, b->name)) ;
}

//*********************************************************
static font_list_p z = NULL ;

//*********************************************************
//  This routine merges two sorted linked lists.
//*********************************************************
static font_list_p merge(font_list_p a, font_list_p b)
   {
   font_list_p c = z ;

   do {
      int x = sort_name(a, b) ;
      if (x <= 0)
         {
         c->next = a ;
         c = a ;
         a = a->next ;
         }
      else
         {
         c->next = b ;
         c = b ;
         b = b->next ;
         }
      }
   while ((a != NULL) && (b != NULL));

   if (a == NULL)  c->next = b ;
             else  c->next = a ;
   return z->next ;
   }

//*********************************************************
//  This routine recursively splits linked lists
//  into two parts, passing the divided lists to
//  merge() to merge the two sorted lists.
//*********************************************************
static font_list_p merge_sort(font_list_p c)
   {
   font_list_p a ;
   font_list_p b ;
   font_list_p prev ;
   int pcount = 0 ;
   int j = 0 ;

   if ((c != NULL) && (c->next != NULL))
      {
      a = c ;
      while (a != NULL)
         {
         pcount++ ;
         a = a->next  ;
         }
      a = c ;
      b = c ;
      prev = b ;
      while (j <  pcount/2)
         {
         j++ ;
         prev = b ;
         b = b->next ;
         }
      prev->next = NULL ;  //lint !e771

      return merge(merge_sort(a), merge_sort(b)) ;
      }
   return c ;
   }

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
   if (z == 0) {
      // z = new ffdata ;
      // z = (struct ffdata *) malloc(sizeof(ffdata)) ;
      z = (font_list_p) new font_list_t ;
      if (z == NULL)
         return ;
      memset((char *) z, 0, sizeof(font_list_t)) ;
   }
   font_list = merge_sort(font_list) ;
}

//***********************************************************************
static unsigned check_for_dupe(char *face_name)
{
   font_list_p fptr ;
   for (fptr=font_list; fptr != 0; fptr = fptr->next) {
      if (strcmp(face_name, fptr->name) == 0)
         return 1;
   }
   return 0;
}

//***********************************************************************
//  this needs to drop duplicate entries, though...
//***********************************************************************
static void add_font_to_list(char *facename)
{
   if (check_for_dupe(facename) != 0)
      return ;
   font_list_p fptr = new font_list_t ; // NOLINT(cppcoreguidelines-owning-memory)
   if (fptr == 0) //lint !e774
      return ;
   memset((char *) fptr, 0, sizeof(font_list_t)) ;
   strncpy(fptr->name, facename, LF_FULLFACESIZE) ;
   *(fptr->name + LF_FULLFACESIZE - 1) = 0 ; //  ensure NULL-term on name

   //  add new entry to list
   if (font_list == 0)
      font_list = fptr ;
   else
      font_tail->next = fptr ;
   font_tail = fptr ;
   font_count++ ;
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

   font_list_p fptr ;
   for (fptr=font_list; fptr != 0; fptr = fptr->next) {
      fptr->combo_box_idx = SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM) fptr->name) ;
   }
}

//****************************************************************************
static DWORD get_current_font_index(void)
{
   font_list_p fptr ;
   char *cfptr = ascii0.get_font_name();

   // unsigned fcount = 0 ;
   for (fptr=font_list; fptr != 0; fptr = fptr->next) {
      // wsprintf(tempstr, "%s vs %s, %u, %u\n", cfptr, fptr->name, fptr->combo_box_idx, fcount) ;
      // OutputDebugString(tempstr) ;
      if (strcmp(cfptr, fptr->name) == 0)
         break;
      // fcount++ ;
   }
   // wsprintf(tempstr, "current font=%s (%u), fcount=%u\n", cfptr, fptr->combo_box_idx, fcount) ;
   // OutputDebugString(tempstr) ;
   return (fptr == 0) ? 0 : fptr->combo_box_idx ;
}

//****************************************************************************
static char *get_selected_font(void)
{
   LRESULT cbresult = SendMessage(hWndComboBox, CB_GETCURSEL, 0, 0);
   if (cbresult == CB_ERR)
      return 0;
   font_list_p fptr ;
   for (fptr=font_list; fptr != 0; fptr = fptr->next) {
      if (fptr->combo_box_idx == (DWORD) cbresult)
         return fptr->name;
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
   syslog("found %u fonts\n", font_count) ;
   // OutputDebugString(tempstr) ;
   // for (fptr=font_list; fptr != 0; fptr = fptr->next) 
   //    puts(fptr->name) ;
}

//****************************************************************************
int read_a_font(HWND hwnd)
{
   if (font_count == 0) {
      build_font_list() ;
      sort_font_list() ;
      // display_font_list() ;
   }

   if (DialogBox ((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), TEXT ("FontBox"), hwnd, FontDlgProc)) {
        InvalidateRect (hwnd, NULL, TRUE) ;
   }

   return 0;
}

