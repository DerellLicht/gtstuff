//**************************************************************************
//  bitmap/image control variables
//**************************************************************************

//lint -esym(769, SQR_IDX_GREEN, SQR_IDX_BLUE, SQR_IDX_YELLOW)

//  LED-image indices
enum {   // NOLINT(cppcoreguidelines-use-enum-class, performance-enum-size)
LED_IDX_IDLE = 0,
LED_IDX_OFF,
LED_IDX_BLUE,
LED_IDX_GREEN,
LED_IDX_RED,
LED_IDX_YELLOW
} ;

//***********************************************************************

//  images.cpp
bool load_led_images(void);
void release_led_images(void);
HWND resize_led_control(HWND hwnd, unsigned ctrl_id);
HWND resize_led_control(HWND hwnd, unsigned ctrl_id, bool indicator_active);
void draw_led(HWND hwndBmp, unsigned idx);
void draw_ledb(HWND hwndBmp, bool idx);
