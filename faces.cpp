#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "common.h"     //  u8, etc
#include "commonw.h"    //  build_font() and constants
#include "gtstuff.h"  
#include "gfuncs.h"     //  graphics primitives
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphic objects
#include "alg_selector.h"
// #include "ezfont.h"

// #define  COLOR_DECREMENT   2

//************************************************************************
static char const * const face_trap_ref[] = {
"                  |                                       |                   ",
"                  |                   |                   |                   ",
"                  |                   |                   |                   ",
"                  |                   |                                       ",
"                  |                   |                   |                   ",
"------------ -----+------------- -----+--------------  ---+-------------------",
"                  |                   |                   |                   ",
"                  |                   |                   |                   ",
"                                      |                   |                   ",
"                  |                   |                   |                   ",
"                  |                   |                   |                   ",
"------------------+------------------- -------------------+------- -----------",
"                  |                   |                   |                   ",
"                  |                   |                   |                   ",
"                  |           ------+ |                   |                   ",
"                  |                 | |                   |                   ",
"                                    | |                   |                   ",
" -----------------------------------| | ------------------| ------------------",
"                  |                 | |                   |                   ",
"                  |                 | |                   |                   ",
"                  |           ------+ |                   +----+              ",
"                  |                   |                                       ",
0 } ;

//************************************************************************
//  these are for WingDings
//  An advantage of using WingDings (as opposed to the fixed-point
//  fonts below, is that everyone has it!!
//  If the requested font isn't present, the user will probably end up
//  with a default font such as Arial, which won't have the face chars.
//************************************************************************
static TCHAR * face_font = _T("WingDings") ;
#define  FACE_FONT_SIZE    16
#define  AF_FIRST_CHAR     74
#define  AF_FACE_COUNT      3
#define  AF_WALL_CHAR     110

//  these are for IBMPC
// static char *face_font = "IBMPC" ;
// #define  FACE_FONT_SIZE    20
// #define  AF_FIRST_CHAR      1
// #define  AF_FACE_COUNT      2
// #define  AF_WALL_CHAR     254

//  these are for 
// static char *face_font = "Medievia Sans Mono" ;
// #define  FACE_FONT_SIZE    22
// #define  AF_FIRST_CHAR      1
// #define  AF_FACE_COUNT      2
// #define  AF_WALL_CHAR     233

//************************************************************************
typedef struct direction_data_s {
   int   x_offset ;
   int   y_offset ;
   u8    flag_mask ;
} direction_data_t, *direction_data_p ;

static direction_data_t dir_data[8] = {
   {  1, -1, 0x01 }, // NE
   {  1,  0, 0x02 }, // E
   {  1,  1, 0x04 }, // SE
   {  0,  1, 0x08 }, // S
   { -1,  1, 0x10 }, // SW
   { -1,  0, 0x20 }, // W
   { -1, -1, 0x40 }, // NW
   {  0, -1, 0x80 }, // N
} ;

//***********************************************************************
face_trap::face_trap(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
, busy_bfr(NULL)
, char_width(0)
, char_height(0)
, dft_columns(0)
, dft_rows(0)
, faces{}
{ 
   //lint -esym(1401, face_trap::faces)
}

//************************************************************************
unsigned face_trap::max_char_width(HDC hdc)
{
   char str[2] ;
   str[1] = 0 ;
   SIZE ssize ;
   str[0] = '@' ;
   GetTextExtentPoint32(hdc, str, 1, &ssize) ;
   // wsprintf(tempstr, "@=%u\n", ssize.cx) ;
   // OutputDebugString(tempstr) ;
   unsigned max_width = 0 ;
   unsigned max_idx = 0 ;
   unsigned idx ;
   for (idx=0; idx<256; idx++) {
      str[0] = (char) idx ;
      GetTextExtentPoint32(hdc, str, 1, &ssize) ;
      if (max_width < (unsigned) ssize.cx) {
         
          max_width = (unsigned) ssize.cx ;
          max_idx = idx ;
      }
   }
   // wsprintf(tempstr, "max=%u\n", max_width) ;
   // OutputDebugString(tempstr) ;
   return max_idx;
}

//************************************************************************
void face_trap::dputc(HDC hdc, unsigned x, unsigned y, char outchr, COLORREF attr)
{
   char str[2] ;
   str[0] = outchr ;
   str[1] = 0 ;
   unsigned busy_idx = (y * dft_columns) + x ;
   if (busy_bfr == 0)
      return ;
   busy_bfr[busy_idx] = (outchr == ' ') ? 0 : 1 ;
   unsigned col_pixel = x * char_width ;
   unsigned row_pixel = y * char_height ;
   SetTextColor(hdc, attr) ;
   TextOut (hdc, col_pixel, row_pixel, str, 1);
}

//************************************************************************
unsigned face_trap::is_cell_free(int column, int row)
{
   if (column < 0  ||  row < 0)
      return 0;
   if ((unsigned) column >= dft_columns  ||  (unsigned) row >= dft_rows)
      return 0;
   unsigned busy_idx = ((unsigned) row * dft_columns) + (unsigned) column ;
   if (busy_bfr == 0)
      return 0 ;
   return (busy_bfr[busy_idx]) ? 0 : 1 ;
}

//************************************************************************
unsigned face_trap::get_free_flags(int x, int y)
{
   unsigned busy_mask = 0 ;
   unsigned j ;
   for (j=0; j<8; j++) {
      direction_data_p dptr = &dir_data[j] ;
      if (is_cell_free(x+dptr->x_offset, y+dptr->y_offset))  
         busy_mask |= dptr->flag_mask ;
   }
   return busy_mask ;
}

//************************************************************************
unsigned face_trap::get_free_count(unsigned busy_flags)
{
   unsigned dmask = 1 ;
   unsigned j ;
   unsigned busy_count = 0 ;
   for (j=0; j<8; j++) {
      if (busy_flags & dmask)
         busy_count++ ;
      dmask <<= 1 ;
   }
   return busy_count ;
}

//************************************************************************
unsigned face_trap::pick_new_dir(unsigned free_flags, unsigned free_count)
{
   //  the original random_int(free_count) didn't appear to be very random!!
   unsigned idx = random_int(4 * free_count) / 4 ;
   unsigned dmask = 1 ;
   unsigned j ;
   unsigned found = 0 ;
   // wsprintf(tempstr, "ri: %u of %u\n", idx, free_count) ;
   // OutputDebugString(tempstr) ;
   for (j=0; j<8; j++) {
      if (free_flags & dmask) {
         if (found == idx)
            return j;
         found++ ;
      }
      dmask <<= 1 ;
   }
   return 0 ;  //  this should not occur, since free_count was already checked
}

//************************************************************************
void face_trap::redraw_face_traps(HDC hdc)
{
   unsigned ref_row, row_pixel, col_pixel, busy_idx ;
   unsigned idx ;
   unsigned col, row ;
   char str[2] ;

   Clear_Window(hdc, 0);
   str[0] = (char) max_char_width(hdc) ;
   str[1] = 0 ;
   SIZE ssize ;
   GetTextExtentPoint32(hdc, str, 1, &ssize) ;
   char_width  = (ssize.cx == 0) ? 10 : ssize.cx ;
   char_height = (ssize.cy == 0) ? 20 : ssize.cy ;
   dft_columns = (unsigned) cxGFrame / char_width ;
   dft_rows    = (unsigned) cyGFrame / char_height ;

   // wsprintf(tempstr, "chars: %u columns, %u rows\n", dft_columns, dft_rows) ;
   // OutputDebugString(tempstr) ;

   //**********************************************************************
   //  allocate the "occupied" buffer
   //**********************************************************************
   if (busy_bfr != 0) {
      delete[] busy_bfr ;
      busy_bfr = 0 ;
   }
   busy_bfr = (char *) new char[dft_rows * dft_columns] ;
   memset(busy_bfr, 0, dft_rows * dft_columns) ;

   //**********************************************************************
   //  draw face-trap framework
   //**********************************************************************
   SetBkMode(hdc, OPAQUE) ;
   SetBkColor  (hdc, 0) ;
   SetTextColor(hdc, WIN_GREY) ;

   ref_row = 0 ;
   row_pixel = 0 ;
   busy_idx = 0 ;
   for (row=0; row < dft_rows; row++) {
      col_pixel = 0 ;
      char const *ftrefp = face_trap_ref[ref_row] ;

      //  draw one row
      for (col=0; col<dft_columns; col++) {
         // str[0] = *ftrefp++ ;
         str[0] = *ftrefp++ ;
         if (str[0] != ' ') {
            busy_bfr[busy_idx] = 1 ;
            // str[0] = 159 ;
            str[0] = AF_WALL_CHAR ;
         } else {
            str[0] = ' ' ;
         }
         if (*ftrefp == 0) 
              ftrefp = face_trap_ref[ref_row] ;

         TextOut (hdc, col_pixel, row_pixel, str, 1);
         col_pixel += char_width ;
         busy_idx++ ;
         // if (++ref_col == FTREF_COLS) 
         //    ref_col = 0 ;
      }
      row_pixel += char_height ;

      ref_row++ ;
      if (face_trap_ref[ref_row] == 0)
         ref_row = 0 ;
   }

   //**********************************************************************
   //  put faces in places
   //**********************************************************************
   // typedef struct face_s {
   //    u8  fchar ;
   //    COLORREF  attr ;
   //    unsigned  dir ;   //  0-7 representing one of 8 linear directions
   //    unsigned  row ;
   //    unsigned  col ;
   // } face_t, *face_p ;
   for (idx=0; idx<FACE_COUNT; idx++) {
      face_p fp = &faces[idx] ;
      fp->fchar = AF_FIRST_CHAR + (u8) random_int(AF_FACE_COUNT) ;
      fp->attr = random_colorref() ;
      fp->dir  = random_int(8) ;
      while (1) {
         fp->row = random_int(dft_rows) ;
         fp->col = random_int(dft_columns) ;
         busy_idx = (fp->row * dft_columns) + fp->col ;
         if (busy_bfr[busy_idx] == 0) {
             busy_bfr[busy_idx] = 1 ;
             str[0] = fp->fchar ;
             col_pixel = fp->col * char_width ;
             row_pixel = fp->row * char_height ;
             TextOut (hdc, col_pixel, row_pixel, str, 1);
             break;
         }
      }  //  while not done seeking a clear random position
   }  //  for each face in list
}

//************************************************************************
void face_trap::move_a_face(HDC hdc, face_p fp)
{
   unsigned free_flags, free_count ;
   int next_col = (int) fp->col + dir_data[fp->dir].x_offset ;
   int next_row = (int) fp->row + dir_data[fp->dir].y_offset ;
   // busy_idx = (next_row * dft_columns) + next_col ;
   //  if new location is available, put us there
   if (is_cell_free(next_col, next_row)) {
      //  erase the previous site
      dputc(hdc, fp->col, fp->row, ' ', WIN_GREY) ;

      //  occupy the new site
      fp->col = next_col ;
      fp->row = next_row ;
      dputc(hdc, fp->col, fp->row, fp->fchar, fp->attr) ;
   } 
   //  we have to change directions; figure out where...
   else {
      free_flags = get_free_flags(fp->col, fp->row) ;
      free_count = get_free_count(free_flags) ;

      if (free_count > 0) {
         fp->dir = pick_new_dir(free_flags, free_count) ;
         // wsprintf(tempstr, "%u,%u: f=%02X, c=%u, d=%u\n", fp->col, fp->row, free_flags, free_count, fp->dir) ;
         // OutputDebugString(tempstr) ;
         next_col = (int) fp->col + dir_data[fp->dir].x_offset ;
         next_row = (int) fp->row + dir_data[fp->dir].y_offset ;

         //  erase the previous site
         dputc(hdc, fp->col, fp->row, ' ', WIN_GREY) ;

         //  occupy the new site
         fp->col = next_col ;
         fp->row = next_row ;
         dputc(hdc, fp->col, fp->row, fp->fchar, fp->attr) ;
      }
   }
}

//****************************************************************************
void face_trap::update_display()
{
   static unsigned delay = 0 ;
   if (pause_the_race)
      return ;

   // SetWindowText(hwnd, title) ;
   
   HDC hdc = get_gframe_dc() ;
   HFONT hfont = build_font(face_font, FACE_FONT_SIZE, EZ_ATTR_NORMAL) ;
   SelectObject (hdc, hfont) ;
   if (we_should_redraw) {
      delay = 0 ;
      redraw_face_traps(hdc) ;
   }  //  if we should redraw/re-initialize

   if (++delay > 30000) {
   // if (++delay > 500) {
      delay = 0 ;
      cycle_count++ ;
      SetBkMode(hdc, OPAQUE) ;
      SetBkColor  (hdc, 0) ;
      // for (unsigned idx=0; idx<FACE_COUNT; idx++) {
      //    move_a_face(hdc, &faces[idx]) ;
      // }
      for (auto &face : faces) {
         move_a_face(hdc, &face) ;
      }
   }

   DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
   release_gframe_dc(hdc) ;
}
