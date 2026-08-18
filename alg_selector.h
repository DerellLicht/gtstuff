//****************************************************************************
//  alg_selector.h - exports graphic drawing functions to the 
//  Windows-interface module.
//****************************************************************************

//  used for max width of elements in certain objects
#define  CONST_SIZE     50

extern int we_should_redraw ;
extern int pause_the_race ;
extern unsigned use_solid_pattern ;
extern unsigned cycle_count ;

//***********************************************************************
//  demo function prototypes
//***********************************************************************
// int check_for_state_change(HWND hwnd, unsigned inchr);
void change_graph_state(uint graph_id);
bool display_current_operation(void);

//  Claude 08/17/26 - not a graph_object subclass, same as gstuff's original
//  on-screen main-menu screen wasn't -- see gobjects.h's friend declaration
//  for how it still reaches get_gframe_dc()/release_gframe_dc(). Referenced
//  as menu_items[0]'s draw_func in alg_selector.cpp.
void draw_intro_graphics(void);

