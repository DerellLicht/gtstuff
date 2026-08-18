## next steps in developing this program

- space-bar terminates the program...

- find a way to pause the rolling... maybe click on screen ?

- implement these flags via user controls
extern int we_should_redraw ;
extern int pause_the_race ;
extern unsigned use_solid_pattern ;
extern unsigned cycle_count ;

- One thing worth flagging given our conversation last message: show_graph_desc(char *desc) takes a non-const char*, and its caller now passes miptr->title — which is menu_items_t::title, a char* field initialized from string literals in the menu_items[] table. That's the exact "should probably be const char*" pattern I flagged for menu_items_t a couple messages back, now with a concrete call site touching it too. Not urgent, and not something I'd fix unprompted mid-edit — just flagging it's still on the table whenever you get to that cleanup pass.