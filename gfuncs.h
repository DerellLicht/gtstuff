/************************************************************************/
/*                         */
/* File: gfuncs.h                     */
/* Description: declarations for common funcs  */
/*                         */
/************************************************************************/

#define  WIN_BLACK      (0x00000000U)
#define  WIN_BLUE       (0x00AA0000U)
#define  WIN_GREEN      (0x0000AA00U)
#define  WIN_CYAN       (0x00AAAA00U)
#define  WIN_RED        (0x000000AAU)
#define  WIN_MAGENTA    (0x00AA00AAU)
#define  WIN_BROWN      (0x000055AAU)
#define  WIN_WHITE      (0x00AAAAAAU)
#define  WIN_GREY       (0x00555555U)
#define  WIN_BBLUE      (0x00FF5555U)
#define  WIN_BGREEN     (0x0055FF55U)
#define  WIN_BCYAN      (0x00FFFF55U)
#define  WIN_BRED       (0x005555FFU)
#define  WIN_BMAGENTA   (0x00FF55FFU)
#define  WIN_YELLOW     (0x0055FFFFU)
#define  WIN_BWHITE     (0x00FFFFFFU)

#define  ROP_SRC   SRCCOPY
#define  ROP_AND   SRCAND
#define  ROP_OR    SRCPAINT
#define  ROP_XOR   SRCINVERT

//  width/height of graphics frame
extern uint cxGFrame ;
extern uint cyGFrame ;

// extern int maxx, maxy ;
// extern unsigned dos_cref[16] ;
// extern char tempstr[260] ;

//lint -esym(843, fill_patterns)
extern unsigned fill_patterns[6] ;

//  gfuncs.cpp
int      random_int(int n);
double   random_part(void);
COLORREF random_colorref(void);
COLORREF random_palette_ref(void);

void Clear_Screen(HDC hdc);
void Clear_Screen(HDC hdc, BYTE Color);
// void Clear_Window(HWND hwnd, BYTE Color);
void Clear_Window(HWND hwnd, unsigned Color);
void Clear_Window(HDC hdc, unsigned Color);

void Solid_Rect(HDC hdc, int xl, int yu, int xr, int yl, BYTE Color);
void SolidRect(HDC hdc, int xl, int yu, int xr, int yl, COLORREF Color);
void Line(HDC hdc, int x0, int y0, int x1, int y1, BYTE Color);
void LineCR(HDC hdc, int x0, int y0, int x1, int y1, COLORREF Color);
void Box(HDC hdc, int x0, int y0, int x1, int y1, COLORREF Color);
void dBitBlt(HDC hdc, int srcX, int srcY, int destX, int destY, 
             int width, int height, unsigned Color);

