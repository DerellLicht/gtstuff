//****************************************************************************
//  alg_selector.h - exports graphic drawing functions to the 
//  Windows-interface module.
//****************************************************************************

#define  CONST_SIZE     50

extern int demo_state ;
extern int we_should_redraw ;
extern int pause_the_race ;
extern unsigned use_solid_pattern ;
extern unsigned cycle_count ;

//***********************************************************************
//  demo function prototypes
//***********************************************************************
// int check_for_state_change(HWND hwnd, unsigned inchr);
void change_graph_state(HWND hwnd, uint graph_id);
bool display_current_operation(HWND hwnd);

