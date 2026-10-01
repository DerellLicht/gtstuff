//*****************************************************************************
//  gobjects - a class to manage graphics objects for my gstuff program.
//
//  Written by:   Derell Licht
//*****************************************************************************

#include <string>
#include <limits.h>

class graph_object {
public:
   graph_object() = default;
   //  disable copy operators for this polymorphic base
   graph_object& operator=(const graph_object &src) = delete;
   graph_object(const graph_object&) = delete;
   //  disable move operators too (rule of five --
   //  cppcoreguidelines-special-member-functions)
   // graph_object(graph_object&&) = delete;
   // graph_object& operator=(graph_object&&) = delete;

   virtual ~graph_object() = default ;
   virtual void update_display(void) = 0 ;

protected:
   //  Claude 08/17/26 - centralized, clip-safe access to hwndGFrame's DC,
   //  inherited by every subclass -- call get_gframe_dc()/release_gframe_dc()
   //  exactly like GetDC()/ReleaseDC(), unqualified, from any subclass's
   //  update_display(). No subclass needs to know hwndGFrame exists at all.
   //  protected, not public: subclasses need this, nothing outside the
   //  hierarchy should be calling it. See gobjects.cpp for why the clip
   //  region has to be re-established on every single call, not just once.
   static HDC get_gframe_dc(void) ;
   static void release_gframe_dc(HDC hdc) ;

   //  Claude 08/17/26 - grants draw_intro_graphics() (alg_selector.cpp)
   //  access to the two methods above, without it being part of the
   //  graph_object hierarchy at all -- mirrors gstuff's original design,
   //  where the on-screen main-menu screen wasn't a gobjects subclass
   //  either. A single, deliberate, named exception -- same philosophy as
   //  get_hwndGFrame() in gtstuff.h.
   friend void draw_intro_graphics(void) ;
} ;

//*******************************************************
class circles: public graph_object {
private:

public:
   circles() = default;
   ~circles() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class squares: public graph_object {
private:

public:
   squares() = default;
   ~squares() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class polygon: public graph_object {
private:

public:
   polygon() = default;
   ~polygon() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class rect: public graph_object {
private:

public:
   rect();
   ~rect() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class pixels: public graph_object {
private:
   unsigned dp_char_width = 0;
   unsigned dp_char_height = 0;
   unsigned rows = 0;
   unsigned columns = 0;
   unsigned color = 0;
   void log_pixel_dimens();

public:
   pixels() = default;
   ~pixels() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class colorbars: public graph_object {
private:

public:
   colorbars() = default;
   ~colorbars() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class xpalette: public graph_object {
private:

public:
   xpalette() = default;
   ~xpalette() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class bitblt: public graph_object {
private:

   //  private functions
   void Concentric_Rect(HDC hdc, int l, int t, int width, int height);

public:
   bitblt() = default;
   ~bitblt() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class xnpalette: public graph_object {
private:

public:
   xnpalette() = default;
   ~xnpalette() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class xrect: public graph_object {
private:

   //  private functions
   void Solid_XRect(HDC hdc, int xl, int yu, int xr, int yl, int Color);

public:
   xrect() = default;
   ~xrect() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class gpalettes: public graph_object {
private:

public:
   gpalettes() = default;
   ~gpalettes() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class triangles: public graph_object {
private:
   //  shared pen cache, one pen per palette entry, created lazily
   //  and flushed when the active palette changes (only one gobject
   //  instance is ever active, so sharing is safe)
   static inline HPEN s_pens[256] = {} ;
   //  UINT_MAX = "no palette selected yet", so the first redraw
   //  always counts as a change and populates s_pens
   unsigned current_palette_index = UINT_MAX ;
   void release_cached_pens() ;
   
public:
   triangles() = default;
   ~triangles() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class rainbow: public graph_object {
private:
   double X = 0.0;
   double Y = 0.0;
   double B = 0.0;
   double thold_limit = 60.0;
   unsigned xbase = 1;
   unsigned xdiff = 1; 
   unsigned ybase = 1;
   unsigned ydiff = 1;

   //  private functions
   void rainbow_plot_pixel(HDC hdc, int pcolor, double thold_angle, unsigned primary);
   void update_gtimer(HDC hdc);

public:
   rainbow() = default;
   ~rainbow() override = default;

   void update_display(void) override ;
   void update_boundaries(unsigned xClient, unsigned yClient);
} ;

//*******************************************************
class lines: public graph_object {
private:
   unsigned orient = 0;  //  0=horiz, 1=vert

public:
   lines() = default;
   ~lines() override = default;

   void update_display(void) override ;
} ;

/************************************************************************/
typedef struct vector_s {
   unsigned x ;
   unsigned x_dir ;
   unsigned y ;
   unsigned y_dir ;
   //  mode3 angle management vars
   unsigned theta ;
   double tan_theta ;
   double dx ;
   unsigned prev_dx ;
   unsigned x_changed ;
   double dy ;
   unsigned prev_dy ;
   unsigned y_changed ;
} vector_t, *vector_p ;

//*******************************************************
class line_games: public graph_object {
private:
   vector_t start = {};
   vector_t finish = {};
   unsigned state = 0;
   unsigned delay = 0;
   unsigned color = 0;
   unsigned line_algorithm = 3;

   //  private functions
   void move_point(vector_p vector);
   void init_vector(vector_p vector);

public:
   line_games() = default;
   ~line_games() override = default;

   void update_display(void) override ;
   void update_line_algorithm(void);
} ;

//*******************************************************
class rcolors: public graph_object {
private:
   unsigned char_width = 0;
   unsigned char_height = 0;
   unsigned rows = 0;
   unsigned columns = 0;
   
   void log_char_dimens();

public:
   rcolors() = default;
   ~rcolors() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
class flames: public graph_object {
private:
   rgb_t fire_palette[256] = {};
   unsigned fire_palette_init = 0;
   u8 *fire_palette_record = {};
   unsigned fire_char_width = 0;
   unsigned fire_char_height = 0;
   unsigned fire_rows = 0;
   unsigned fire_cols = 0;

   //  private functions
   void dump_fire_palette(void);
   void set_fire_palette(unsigned index, u8 red, u8 green, u8 blue);
   unsigned get_record_index(unsigned x, unsigned y);
   void set_palette_index(unsigned x, unsigned y, u8 color_idx);
   unsigned get_palette_index(unsigned x, unsigned y);
   void update_global_palette28(void);
   void init_fire_palette(void);
   COLORREF get_fire_palette(unsigned index);
   void draw_fire_element(HDC hdc, unsigned x, unsigned y, unsigned color);

public:
   flames() = default;
   ~flames() override = default;

   void update_display(void) override ;
} ;

//*******************************************************
typedef struct face_s {
   u8  fchar ;
   COLORREF  attr ;
   unsigned  dir ;   //  0-7 representing one of 8 linear directions
   unsigned  row ;
   unsigned  col ;
} face_t, *face_p ;

#define  FACE_COUNT  30

class face_trap: public graph_object {
private:
   char *busy_bfr = {};
   unsigned char_width = 0;
   unsigned char_height = 0;
   unsigned dft_columns = 0;
   unsigned dft_rows = 0;
   face_t faces[FACE_COUNT] = {};  //  convert to <vector>

   //  private functions
   void move_a_face(HDC hdc, face_p fp);
   void redraw_face_traps(HDC hdc);
   unsigned pick_new_dir(unsigned free_flags, unsigned free_count);
   unsigned get_free_count(unsigned busy_flags);
   unsigned get_free_flags(int x, int y);
   unsigned is_cell_free(int column, int row);
   void dputc(HDC hdc, unsigned x, unsigned y, char outchr, COLORREF attr);
   unsigned max_char_width(HDC hdc);

public:
   face_trap() = default;
   ~face_trap() override = default;

   void update_display(void) override ;
} ;


//*******************************************************
class ascii: public graph_object {
private:

public:
   ascii() = default;
   ~ascii() override = default;

   void update_display(void) override ;
   void set_font_name(char const *new_font_name);
   char *get_font_name(void);
} ;

//*******************************************************
class sglass: public graph_object {
private:
   int max_col = 0;
   int max_row = 0;
   int x = 0;
   int y = 0;
   int distfact = 2;    //  multiplier for distance 
   int size = 75;       //  max size to grow to     
   int osize = 50;      //  same                    
   int limit = 1;       //  min. size of box - one row/col
   int in_size = 1;     //  starting size             
   int o_size = 0;      //  same                      
   int unoo = 1;        //  one - changed from + to - 
   int onoo = 1;        //  same, for other drawing   

   void box_box (HDC hdc, int col_inpt, int row_inpt, int siz);
   void box_point (HDC hdc, int ccol, int rrow, int ssiz);

public:
   sglass() = default;
   ~sglass() override = default;
   
   void update_display(void) override ;
} ;

//*******************************************************
class wincolors: public graph_object {
private:

public:
   wincolors() = default;
   ~wincolors() override = default;

   void update_display(void) override ;
} ;
