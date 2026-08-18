//*****************************************************************************
//  gobjects - a class to manage graphics objects for my gstuff program.
//
//  Written by:   Derell Licht
//*****************************************************************************

#include <string>

class graph_object {
protected:
   std::string title ;

public:
   explicit graph_object(std::string title_text) ;
   //  disable copy operators for this polymorphic base
   graph_object& operator=(const graph_object &src) = delete;
   graph_object(const graph_object&) = delete;
   //  disable move operators too (rule of five --
   //  cppcoreguidelines-special-member-functions)
   graph_object(graph_object&&) = delete;
   graph_object& operator=(graph_object&&) = delete;

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
   circles(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   circles &operator=(const circles &src) = delete;
   circles(const circles&) = delete;
   circles(circles&&) = delete;
   circles& operator=(circles&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class squares: public graph_object {
private:

public:
   squares(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   squares &operator=(const squares &src) = delete;
   squares(const squares&) = delete;
   squares(squares&&) = delete;
   squares& operator=(squares&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class polygon: public graph_object {
private:

public:
   polygon(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   polygon &operator=(const polygon &src) = delete;
   polygon(const polygon&) = delete;
   polygon(polygon&&) = delete;
   polygon& operator=(polygon&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class rect: public graph_object {
private:

public:
   rect(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   rect &operator=(const rect &src) = delete;
   rect(const rect&) = delete;
   rect(rect&&) = delete;
   rect& operator=(rect&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class pixels: public graph_object {
private:
   unsigned dp_char_width ;
   unsigned dp_char_height ;
   unsigned rows ;
   unsigned columns ;
   unsigned color ;

public:
   pixels(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   pixels &operator=(const pixels &src) = delete;
   pixels(const pixels&) = delete;
   pixels(pixels&&) = delete;
   pixels& operator=(pixels&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class colorbars: public graph_object {
private:

public:
   colorbars(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   colorbars &operator=(const colorbars &src) = delete;
   colorbars(const colorbars&) = delete;
   colorbars(colorbars&&) = delete;
   colorbars& operator=(colorbars&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class xpalette: public graph_object {
private:

public:
   xpalette(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   xpalette &operator=(const xpalette &src) = delete;
   xpalette(const xpalette&) = delete;
   xpalette(xpalette&&) = delete;
   xpalette& operator=(xpalette&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class bitblt: public graph_object {
private:

   //  private functions
   void Concentric_Rect(HDC hdc, int l, int t, int width, int height);

public:
   bitblt(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   bitblt &operator=(const bitblt &src) = delete;
   bitblt(const bitblt&) = delete;
   bitblt(bitblt&&) = delete;
   bitblt& operator=(bitblt&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class xnpalette: public graph_object {
private:

public:
   xnpalette(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   xnpalette &operator=(const xnpalette &src) = delete;
   xnpalette(const xnpalette&) = delete;
   xnpalette(xnpalette&&) = delete;
   xnpalette& operator=(xnpalette&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class xrect: public graph_object {
private:

   //  private functions
   void Solid_XRect(HDC hdc, int xl, int yu, int xr, int yl, int Color);

public:
   xrect(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   xrect &operator=(const xrect &src) = delete;
   xrect(const xrect&) = delete;
   xrect(xrect&&) = delete;
   xrect& operator=(xrect&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class gpalettes: public graph_object {
private:

public:
   gpalettes(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   gpalettes &operator=(const gpalettes &src) = delete;
   gpalettes(const gpalettes&) = delete;
   gpalettes(gpalettes&&) = delete;
   gpalettes& operator=(gpalettes&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class triangles: public graph_object {
private:

public:
   triangles(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   triangles &operator=(const triangles &src) = delete;
   triangles(const triangles&) = delete;
   triangles(triangles&&) = delete;
   triangles& operator=(triangles&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class rainbow: public graph_object {
private:
   double X, Y, B ;
   double thold_limit ;
   unsigned xbase, xdiff, ybase, ydiff ;

   //  private functions
   void rainbow_plot_pixel(HDC hdc, int pcolor, double thold_angle, unsigned primary);
   void update_gtimer(HDC hdc);

public:
   rainbow(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   rainbow &operator=(const rainbow &src) = delete;
   rainbow(const rainbow&) = delete;
   rainbow(rainbow&&) = delete;
   rainbow& operator=(rainbow&&) = delete;

   // ~faces() {} ;
   void update_display(void) override ;
   void update_boundaries(unsigned xClient, unsigned yClient);
} ;

//*******************************************************
class lines: public graph_object {
private:
   unsigned orient ;  //  0=horiz, 1=vert

public:
   lines(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   lines &operator=(const lines &src) = delete;
   lines(const lines&) = delete;
   lines(lines&&) = delete;
   lines& operator=(lines&&) = delete;

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
   vector_t start ;
   vector_t finish ;
   unsigned state ;
   unsigned delay ;
   unsigned color ;
   unsigned line_algorithm ;

   //  private functions
   void move_point(vector_p vector);
   void init_vector(vector_p vector);

public:
   //  disable copy and assignment operators
   //  for classes with pointer members
   line_games &operator=(const line_games &src) = delete;
   line_games(const line_games&) = delete;
   line_games(line_games&&) = delete;
   line_games& operator=(line_games&&) = delete;

   line_games(std::string title_text) ;
   void update_display(void) override ;
   void update_line_algorithm(void);
} ;

//*******************************************************
class rcolors: public graph_object {
private:
   unsigned char_width ;
   unsigned char_height ;
   unsigned rows ;
   unsigned columns ;

public:
   rcolors(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   rcolors &operator=(const rcolors &src) = delete;
   rcolors(const rcolors&) = delete;
   rcolors(rcolors&&) = delete;
   rcolors& operator=(rcolors&&) = delete;

   void update_display(void) override ;
} ;

//*******************************************************
class flames: public graph_object {
private:
   rgb_t fire_palette[256] ;
   unsigned fire_palette_init ;
   u8 *fire_palette_record ;
   unsigned fire_char_width ;
   unsigned fire_char_height ;
   unsigned fire_rows ;
   unsigned fire_cols ;

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
   flames(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   flames &operator=(const flames &src) = delete;
   flames(const flames&) = delete;
   flames(flames&&) = delete;
   flames& operator=(flames&&) = delete;

   // ~flames() {} ;
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
   char *busy_bfr ;
   unsigned char_width ;
   unsigned char_height ;
   unsigned dft_columns ;
   unsigned dft_rows ;
   face_t faces[FACE_COUNT] ;  //  convert to <vector>

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
   face_trap(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   face_trap &operator=(const face_trap &src) = delete;
   face_trap(const face_trap&) = delete;
   face_trap(face_trap&&) = delete;
   face_trap& operator=(face_trap&&) = delete;

   // ~faces() {} ;
   void update_display(void) override ;
} ;


//*******************************************************
class ascii: public graph_object {
private:

public:
   ascii(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   ascii &operator=(const ascii &src) = delete;
   ascii(const ascii&) = delete;
   ascii(ascii&&) = delete;
   ascii& operator=(ascii&&) = delete;

   void update_display(void) override ;
   void set_font_name(char *new_font_name);
   char *get_font_name(void);
} ;

//*******************************************************
class sglass: public graph_object {
private:
   int max_col, max_row, x, y;
   int distfact ;    /* multiplier for distance */
   int size ;        /* max size to grow to     */
   int osize ;       /* same                    */
   int limit ;       /* min. size of box - one row/col */
   int in_size ;     /* starting size                */
   int o_size ;      /* same                         */
   int unoo ;        /* one - changed from + to -    */
   int onoo ;        /* same, for other drawing      */

   void box_box (HDC hdc, int col_inpt, int row_inpt, int siz);
   void box_point (HDC hdc, int ccol, int rrow, int ssiz);

public:
   sglass(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   sglass& operator=(sglass const &src) = delete;
   sglass(const sglass&) = delete;
   sglass(sglass&&) = delete;
   sglass& operator=(sglass&&) = delete;

   virtual ~sglass() = default;
   void update_display(void) override ;
} ;

//*******************************************************
class wincolors: public graph_object {
private:

public:
   wincolors(std::string title_text) ;
   //  disable copy and assignment operators
   //  for classes with pointer members
   wincolors &operator=(const wincolors &src) = delete;
   wincolors(const wincolors&) = delete;
   wincolors(wincolors&&) = delete;
   wincolors& operator=(wincolors&&) = delete;

   void update_display(void) override ;
} ;
