//************************************************************************
//  gobjects - a class to manage graphics objects for my gstuff program.
//************************************************************************

#include <windows.h>

#include "common.h"     //  u8, etc
// #include "gtstuff.h"    
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes

graph_object::graph_object(char *title_text) :
   title(NULL) 
{ 
   title = new char[strlen(title_text)+1] ;
   strcpy(title, title_text) ;
}

