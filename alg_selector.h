//****************************************************************************
//  alg_selector.h - exports graphic drawing functions to the 
//  Windows-interface module.
//****************************************************************************

//  used for max width of elements in certain objects
#define  CONST_SIZE     50

extern bool we_should_redraw ;
extern bool pause_the_race ;
extern bool use_solid_pattern ;
extern bool run_custom_op ;

extern unsigned cycle_count ;

//***********************************************************************
//  demo function prototypes
//***********************************************************************

//  alg_selector.cpp
void change_graph_state(uint graph_id);
bool display_current_operation(void);

//  Claude 08/17/26 - not a graph_object subclass, same as gstuff's original
//  on-screen main-menu screen wasn't -- see gobjects.h's friend declaration
//  for how it still reaches get_gframe_dc()/release_gframe_dc(). Referenced
//  as menu_items[0]'s draw_func in alg_selector.cpp.
void draw_intro_graphics(void);

void fill_gobject_combobox(HWND hwnd, unsigned init_idx);
void run_selected_gobject(HWND hwndGObjList);

void handle_custom_req(HWND hwnd);

//  font.dialog.cpp
int read_a_font(HWND hwnd);

