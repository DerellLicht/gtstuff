//****************************************************************************
//  gtstuff.h - public declarations for graphics program
//  
//  Written by:   Daniel D. Miller
//****************************************************************************

//****************************************************************************
//  debug: message-reporting data 
//****************************************************************************

#define  DBG_VERBOSE       0x01
#define  DBG_WINMSGS       0x02
#define  DBG_RX_DEBUG      0x04
#define  DBG_CTASK_TRACE   0x08
#define  DBG_SMTP_RECV     0x10
#define  DBG_POLLING       0x20
#define  DBG_ETHERNET      0x40

extern uint cxClient ;
extern uint cyClient ;

//**************************************************************
//  function prototypes
//**************************************************************

//  gtstuff.cpp
void status_message(char *msgstr);
void status_message(uint idx, char *msgstr);

void show_graph_desc(char const *desc);

//  Claude 08/17/26 - hwndGFrame itself stays `static` (private) to
//  gtstuff.cpp -- this getter is the one deliberate crack in that, used
//  only by graph_object::get_gframe_dc()/release_gframe_dc() in
//  gobjects.cpp. Nothing else should call it; algorithm subclasses should
//  never see an HWND at all, only the HDC those two functions hand back.
HWND get_hwndGFrame(void) ;

