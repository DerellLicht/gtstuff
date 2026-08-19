/*----------------------
   EZFONT.H header file
  ----------------------*/
  
#define  MAX_PATH_LEN  1024

//***************************************************************
#define EZ_ATTR_NORMAL        0
#define EZ_ATTR_BOLD          1
#define EZ_ATTR_ITALIC        2
#define EZ_ATTR_UNDERLINE     4
#define EZ_ATTR_STRIKEOUT     8

// HFONT build_font(char *fname, unsigned fheight, unsigned fweight, unsigned fitalic, unsigned funderline, unsigned fstrikeout);
HFONT build_font(char const * const fname, unsigned fheight, unsigned fbold, unsigned fitalic, unsigned funderline, unsigned fstrikeout);
void init_msg_window(HWND hwnd, unsigned x0, unsigned y0, unsigned dx, unsigned dy);
void clear_message_area(void);
void gotoxy(HDC hdc, int x, int y);
void gotoxy(HWND hwnd, int x, int y);
void set_text_color(unsigned new_color);
void set_text_attributes(unsigned new_attribs);
int  set_text_font(char *fontname, int psize);
void set_default_font(void);
void set_text_size(int psize);
void set_text_angle(unsigned degs);
int  textheight(HDC hdc, unsigned selected);
int  textwidth(HDC hdc, char *str);
void dputs(HDC hdc, char *str);
void dputs(HDC hdc, char *str, unsigned attr);
void dprints(HDC hdc, int x, int y, unsigned attr, char *str);
void dprints(HDC hdc, int x, int y, unsigned fg, unsigned bg, char *str);
void dprints_centered_x(HDC hdc, long y, unsigned attr, char *str);
void dprints_centered_x(HDC hdc, long y, unsigned fg, unsigned bg, char *str);
void dprints_centered_x(HWND hwnd, long y, unsigned attr, char *str);
void dprints_centered_x(HWND hwnd, long y, unsigned fg, unsigned bg, char *str);

