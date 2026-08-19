//  requires windows.h

#define  TOTAL_PALETTE_ENTRIES   29

//  rgb color struct
typedef struct rgb_s {
   unsigned char  red ;
   unsigned char  green ;
   unsigned char  blue ;
} rgb_t, *rgb_p ;

//  palette-table struct
typedef struct palette_record_s {
   char *name ;
   rgb_p palette ;
   unsigned entries ;
} palette_record_t, *palette_record_p ;

//  function prototypes
COLORREF get_palette_entry(unsigned Color);
char const *get_palette_name(unsigned record);
void set_DAC_table(unsigned record);
void toggle_DAC_table(void);
void next_DAC_table(void);
void prev_DAC_table(void);
unsigned get_curr_palette(void);
unsigned get_palette_entries(unsigned record);
unsigned get_palette_entries(void);
