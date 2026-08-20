//****************************************************************************
//  Copyright (c) 2008-2026  Derell Licht
//  images.cpp - handle ImageList functions for LED bitmaps
//****************************************************************************
#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "common.h"
#include "gtstuff.h" 
#include "images.h"

//lint -esym(714, resize_square_control, draw_square)
//lint -esym(759, resize_square_control, draw_square)
//lint -esym(765, resize_square_control, draw_square)

//  led-image data
static HIMAGELIST hiSpriteList = 0 ;
static HICON hiLeds[6] = { 0, 0, 0, 0, 0, 0 } ;
static int image_height = 0 ;
static int el_width = 0 ;

//****************************************************************************
HWND resize_led_control(HWND hwnd, unsigned ctrl_id)
{
   HWND hwndBmp = GetDlgItem(hwnd, ctrl_id) ;
   SetWindowPos(hwndBmp, NULL, 0, 0, el_width, image_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
   draw_led(hwndBmp, LED_IDX_OFF) ;
   return hwndBmp ;
}

//****************************************************************************
HWND resize_led_control(HWND hwnd, unsigned ctrl_id, bool indicator_active)
{
   HWND hwndBmp = GetDlgItem(hwnd, ctrl_id) ;
   SetWindowPos(hwndBmp, NULL, 0, 0, el_width, image_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
   if (indicator_active) 
      draw_led(hwndBmp, LED_IDX_GREEN) ;
   else
      draw_led(hwndBmp, LED_IDX_OFF) ;
   return hwndBmp ;
}

//****************************************************************************
bool load_led_images(void)
{
   unsigned idx ;
   //  load the LED images
   hiSpriteList = ImageList_LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_LEDBMPS), image_height, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION) ;
   if (hiSpriteList == NULL) {
      syslog("ImageList_LoadBitmap: %s\n", get_system_message()) ;
      return false;
   }
   ImageList_GetIconSize(hiSpriteList, &el_width, &image_height) ;

   for (idx=0; idx<6; idx++) {
      hiLeds[idx] = ImageList_ExtractIcon(NULL, hiSpriteList, idx) ;
      if (hiLeds[idx] == NULL) {
         syslog("ImageList_ExtractIcon: %s\n", get_system_message()) ;
      }
   }

   // syslog("image dims: LED: W=%d, H=%d: SQUARE: W=%d, H=%d\n", el_width, image_height, sqr_el_width, sqr_image_height) ;
   // syslog("LED image list dimensions: width=%d, height=%d\n", el_width, image_height) ;
   //  FYI: ImageList_GetImageCount() returns number of elements in list
   return true;
}

//***********************************************************************
HIMAGELIST get_image_list(void)
{
   return hiSpriteList ;
}

//***********************************************************************
void release_led_images(void)
{
   unsigned idx ;

   //  release LED-image resources
   for (idx=0; idx<6; idx++) {
      if (hiLeds[idx] != NULL) {
         DestroyIcon(hiLeds[idx]) ;
         hiLeds[idx] = NULL ;
      }
   }
   if (hiSpriteList != 0) {
      // DeleteObject(hiSpriteList) ;  //  delete image??
      hiSpriteList = 0 ;
   }

}

//***********************************************************************
void draw_led(HWND hwndBmp, unsigned idx)
{
   if (hiLeds[idx] != NULL)
      // SendMessageA(hwndBmp, STM_SETIMAGE, (WPARAM) IMAGE_ICON,(LPARAM) hiLeds[idx]);
      PostMessageA(hwndBmp, STM_SETIMAGE, (WPARAM) IMAGE_ICON,(LPARAM) hiLeds[idx]);
}

//***********************************************************************
void draw_ledb(HWND hwndBmp, bool idx)
{
   if (idx)
      PostMessageA(hwndBmp, STM_SETIMAGE, (WPARAM) IMAGE_ICON,(LPARAM) hiLeds[LED_IDX_GREEN]);
   else
      PostMessageA(hwndBmp, STM_SETIMAGE, (WPARAM) IMAGE_ICON,(LPARAM) hiLeds[LED_IDX_OFF]);
}
