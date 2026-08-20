//****************************************************************************
//  gtstuff.h - public declarations for graphics program
//  
//  Written by:   Derell Licht
//****************************************************************************

//****************************************************************************
//  debug: message-reporting data 
//****************************************************************************

#define  DBG_VERBOSE       0x01
#define  DBG_WINMSGS       0x02

extern HINSTANCE g_hinst ;

extern uint cxClient ;
extern uint cyClient ;

// These HWND fields are defined in gtstuff.cpp, 
// but used in alg_selector.cpp
extern HWND hwndPalette ;
extern HWND hwndPaletteSpin ;

extern HWND hwndPauseState ;
extern HWND hwndSolidState ;
//**************************************************************
//  function prototypes
//**************************************************************

//  gtstuff.cpp
void status_message(char *msgstr);
void status_message(uint idx, char *msgstr);
HWND get_main_dialog_handle();

void show_graph_desc(char const *desc);

//  Claude 08/17/26 - hwndGFrame itself stays `static` (private) to
//  gtstuff.cpp -- this getter is the one deliberate crack in that, used
//  only by graph_object::get_gframe_dc()/release_gframe_dc() in
//  gobjects.cpp. Nothing else should call it; algorithm subclasses should
//  never see an HWND at all, only the HDC those two functions hand back.
HWND get_hwndGFrame(void) ;

