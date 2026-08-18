//*******************************************************************
//  This array of RGB values contains the standard color values
//  from the XConsortium: 
//! $XConsortium: rgb.txt,v 10.41 94/02/20 18:39:36 rws Exp $
//  
//  The reference file is /usr/X11R6/lib/X11/rgb.txt
//*******************************************************************

typedef struct drgb_s {
   unsigned red ;
   unsigned green ;
   unsigned blue ;
   unsigned value ;
   char     *name ;
} drgb_t ;

#define  NBR_RGB_ITEMS  549
extern drgb_t const drgb[NBR_RGB_ITEMS+1] ;

