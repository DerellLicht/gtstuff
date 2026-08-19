## next steps in developing this program

- CRITICAL !!!
We need to go back over all the other `gobjects`, and make sure none of them 
have `return` after `get_gframe_dc()` without `release_gframe_dc()`

- implement these flags via user controls
extern int pause_the_race ;  
extern unsigned use_solid_pattern ;  
extern unsigned cycle_count ;  
unsigned fill_patterns[6] = {  
  
   add a button to toggle modes on related pages  

- implement elapsed time(msec) and cycles/second

- we need custom functions for some gobjects

- `rainbow` bogs the mouse down badly;
  it is also *far* slower than `gstuff` , which does *not* bog down the mouse.
  
- `line games` doesn't clear screen on startup

- `stained glass` doesn't work at all, and bogs down the mouse in the process...

