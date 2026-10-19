#include "bitmaps/bmpdata.c"
//#include <stdarg.h>

#define WHITE 0xffffff
#define BLACK 0
#define ALMOST_BLACK 0x000001
#define LIGHT_GRAY 0xa3a3a3
#define LIGHTER_GRAY 0xcccaca
#define PALE_RED 0x694545
#define DARK_RED 0x510000

#define STACK_POINTER 0xFFFFFF

#define fori(x) for(int i=0;i<x;i++)

#define RED 0xff0000
#define ORANGE 0xff7f00
#define YELLOW 0xffff00
#define GREEN 0x00FF00
#define BLUE 0x0000ff
#define PURPLE 0xb300ff
#define NUMBER_COLOUR 0xa151b9
#define OS_KEYWORD_COLOUR 0x039b5e
#define BRICK 0xc14a09
#define NAVY 0x000b06
#define COMMENT_COLOUR 0xe4e6b0


#define TAB_COLOUR 0xff4d4d
#define HIGHLIGHT_TAB_COLOUR 0xfa717f

#define true 1
#define false 0

#define BASE_ADDRESS 0x300000 //programs reside here
#define PROGRAM_HEAP 0x250000

#define STRINGS_ADDRESS 0x150000

#define STR_LEN_ADDRESS 0x600000
#define MALLOC_ADDRESS 0x600500
#define STR_CAT_ADDRESS 0x601000
#define STR_CPY_ADDRESS 0x601500
#define INT2STR_ADDRESS 0x602000


#define DRAW_ADDRESS 0x602500
#define OUTLINE_RECT_ADDRESS 0x603000
#define DRAW_CIRCLE_ADDRESS 0x603500
#define DRAW_LINE_ADDRESS 0x604000
#define REFRESH_ADDRESS 0x604500
#define TIMER_ADDRESS 0x605000
#define RNG_ADDRESS 0x605500
#define DRAW_PIXEL_ADDRESS 0x606000


#define NULL ((void*)0)

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182 


char CLIPBOARD[1024*1024]={0}; //1 MB

volatile char CURRENT_KEY=' ';

volatile int TIMER_TICKS=0;

volatile _Bool INTERRUPT=false;

volatile _Bool MOUSE_HELD=false;

unsigned int COLOUR;

char FILE_BUF[1024*1024*2]={0}; //2 MB file buffer

_Bool bg_image_copied=0;
int background_image_width,background_image_height;
unsigned int * background_image_buf;

void* TEMP_MEM[64];
int TEMP_MEM_INDEX;

char* CURR_TEMP8=NULL;


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

uint32_t framebuffer_add;

uint8_t FILE_INFO_BUF[40];

//uint32_t DRAW_PIXEL_ADDRESS;

//char@ ptr=0x45678;
//uint8_t* program_memory=(uint8_t*)0x600000;//where programs reside in memory

char* heap=(char*)0x400000;//real heap ~4 mb in
int heap_offset=0;

void create_global_message(char* msg);

typedef struct {
    uint32_t size;
    uint32_t addr;
    uint32_t length;
    uint32_t type;

}__attribute__ ((packed)) multiboot_mmap_entry_t;

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
} __attribute__((packed)) multiboot_info_t;

typedef struct block{
    int size;
    int free;
    struct block* next;

}block_t;

typedef struct {
    uint32_t sector;
    uint8_t data[512];

}sector_t;

/*typedef struct{
    char* 

}file_entry_t;*/

block_t* free_list=NULL;

//#define BLOCK_SIZE sizeof(block_t);

void* CREATE_BLOCK(int size){
    if (heap_offset>=0xfffffff)return NULL;

    void* ptr=&heap[heap_offset];

    heap_offset+=size;

    return ptr;

}
int ghjk=10;
block_t* FIND_FREE_BLOCK(int size){
    block_t* current=free_list;

    block_t* small=NULL;

    while (current!=NULL){
        if (current->size>=size && current->free)
            if (small==NULL || current->size<small->size)
                small=current;
        
        current=current->next;

    }

    return small;

}

block_t* REQUEST_NEW_BLOCK(int size){
    block_t* block=CREATE_BLOCK(sizeof(block_t)+size);

    block->free=false;
    block->next=NULL;
    block->size=size;

    return block;
}

void REMOVE_FREE(block_t* block){
    if(free_list==block){
        free_list=block->next;
        return;
    }
    block_t* current=free_list;

    while(current!=NULL && current->next!=block){
        current=current->next;
    }

    if (current!=NULL){
        current->next=block->next;
    }


}

void* MALLOC(int size){
    if (size<=0){
        return NULL;
    }

    block_t* block=FIND_FREE_BLOCK(size);

    if (block==NULL){
        
        block=REQUEST_NEW_BLOCK(size);
        
    }
    else{
        //write_rowcol_text(8,4,"A FREE BLOCK WAS FOUND",GREEN);
        block->free=0;

        REMOVE_FREE(block);

    }
    
    return ((void*)(block+1));


}

//void* REALLOC(void)

void FREE(void* address){
    if(address==NULL){
        //create_global_message("Could not free - address is NULL!");
        return;
    }
    
    block_t* block=(block_t*)address-1;

    block->free=true;

    block->next=free_list;
    free_list=block;

}


void cminus();
void setup_interface();
void draw_close_box();
void home();
void bit_maker();
void shell();
void files();
void textpad();
void unhighlight_char();
_Bool type_char(uint8_t* character);

static inline unsigned char inb(unsigned short port);


_Bool running_program=0;


uint32_t* framebuffer;
uint32_t ppr;//(pixels per row)

multiboot_mmap_entry_t* mmap_entry;
multiboot_info_t* multiboot_info;

uint32_t text_colour = 0;
uint32_t bg_colour = 0;
uint32_t bg_highlight_colour = 0xffffff;
uint32_t text_highlight_colour = 0x008cff;

uint32_t GLOBAL_BRUSH_COLOUR=0;

//DIMENSIONS OF SCREEN TEXTBOX=101x42

char scroll_bounds[2]={0,42};

char shell_tokens[64][64] = {0};
char char_buffer[101*42] = {'\0'};
int char_buffer_index = 0;
int shell_token_index = 0;

volatile int CURRENT_RAND;


int min_cursorx = 0;

static int mouse_x = 500;
static int mouse_y = 500;
static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

uint32_t background_buffer[20*20];

int old_mouse_pos[2] = {500,500};

_Bool done = 0;


// PS/2 Keyboard Scancode Set 1 -> ASCII (index = scancode)
char character_table_shift[128] = {
    0,   0,   '!', '"', '£', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   0,   // 0x00-0x0F
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,   'A', 'S', // 0x10-0x1F
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '\\', 0,   '|', 'Z', 'X', 'C', 'V', // 0x20-0x2F
    'B', 'N', 'M', '<', '>', '?', ' ', '*', ' ', ' ', ' ',   0,   0,   0,   0,   0,   // 0x30-0x3F
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40-0x4F
    '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x50-0x5F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x60-0x6F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0    // 0x70-0x7F
};

char character_table[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,   // 0x00-0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,   0,   'a', 's', // 0x10-0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','~', 0,   '#','z', 'x', 'c', 'v', // 0x20-0x2F
    'b', 'n', 'm', ',', '.', '/', ' ',   '*', ' ',   ' ', 0,   0,   0,   0,   0,   0,   // 0x30-0x3F
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40-0x4F
    '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x50-0x5F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x60-0x6F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0    // 0x70-0x7F
};



int cursorx = 1;
int cursory = 0;
int last_cursorx = 0;
int last_cursory = 0;

_Bool shift_pressed = 0; 
_Bool at_start = 1;
_Bool on_tabs = 0;
_Bool shell_selected = 1;
_Bool files_selected = 0;
_Bool cm_selected = 0;
_Bool home_selected = 1;

_Bool done_shell = 0;
_Bool done_cm = 0;

_Bool on_blink = 1;

struct text_button {
    char text[32];
    uint32_t fg;
    uint32_t bghighlightcolour;
    char command[32];
    int width;
    int height;
    int x;
    int y;
    uint32_t bgcolour;

};

typedef struct {
    uint8_t* instructions,stack;
    int index;
    int eax,ebx,ecx,edx,esi,edi,ebp,esp;

}Process;

typedef struct{
    char filename[32];
    char size[4];
    char first_sector[4];

}FileEntry;

typedef struct{
    char file_count[4];
    char sector_ptr[4];

}MetaData;

typedef struct {
    char* msg;
    void (*function)(void);
    int index;
    char* buffer;
    int capacity;

}Question;

Question* question;

struct button {
    int x;
    int y;
    int width;
    int height;

    uint32_t bgcolour;
    uint32_t textcolour;
    uint32_t bghighlightcolour;

    char* text;
    char* arg1;

    _Bool is_file;
    _Bool is_program;
    _Bool is_image;

    void (*on_click)(void);
    void (*on_click_arg)(char* arg);

};

typedef struct{
    int x;
    int y;

    int height;

    struct button* up;
    struct button* down;
    

}Scrollbar;

struct break_point{
    int index;
    int address;
};

typedef struct{

    int x;
    int y;
    int width;
    int height;
    uint32_t* bg_buf;

}Background;

typedef struct{
    int x;
    int y;

    int width;
    int height;

    char* text;

    uint32_t colour;

}TextSlab;

typedef struct{

    uint32_t bg;
    uint32_t fg;

    _Bool editable,is_console;

    int x;
    int y;
    int width;
    int height;

    char* title;

    char* buffer;
    int index;
    int capacity;
    int cursorx;
    int cursory;

    Background* background;

    struct button* close_button;

    struct button** buttons;
    int buttons_index;
    int buttons_capacity;

    struct button** tab;
    int tab_index;
    int tab_capacity;

    int scroll_bounds;
    int MAX;



}TextBox;

typedef struct {

    int x;
    int y;
    int width;
    int height;

    Background* bg;

}Rectangle;

typedef struct{
    int width;
    int height;

    uint32_t* bitmap;

}__attribute__ ((packed))Image;

typedef struct{
    int row,col;
    struct button* button;


}ScrollButton;


typedef struct{
    int row,col;
    Image* image;
    

}ScrollImage;

typedef struct{
    int row,col;
    char* text;
    _Bool malloced;

}ScrollText;

typedef struct{
    int x,y,width,height,ID,scroll_bounds,bg,fg;

    _Bool editable;

    char* title;
    
    ScrollText** text;
    int index,capacity;

    ScrollButton** buttons;
    int buttons_capacity,buttons_index;

    ScrollImage** images;
    int images_index,images_capacity;

    Background* background;


}ScrollArea;

struct DropDown;

typedef struct{
    uint32_t fg;
    uint32_t bg;
    uint32_t fg_highlight;
    uint32_t bg_highlight;

    char* name;

    char* buffer;
    char* visual_buffer;
    int buffer_index;
    int buffer_capacity;

    _Bool editable;

    int scroll_bounds;

    struct button** buttons;
    int buttons_index;
    int buttons_capacity;

    Scrollbar* scrollbar;

    int cursorx;
    int cursory;

    int selection_start;
    int selection_end;
    _Bool selecting;

    TextSlab** text_slabs;
    int text_slabs_capacity;
    int text_slabs_index;

    Rectangle** rectangles;
    int rectangles_index;
    int rectangles_capacity;

    struct DropDown* drop_down;

    Background** temps;
    int temps_index;
    int temps_capacity;

    ScrollArea** scroll_areas;
    int scroll_index,scroll_capacity;


    struct button* close_button;
    struct button** tab;
    int tab_index;
    int tab_capacity;

}TextArea;

typedef struct {
    int x;
    int y;
    int width;
    int height;

    struct button** buttons;

    int scroll_bounds;

    char* buffer;
    struct button* close_button;
    char* name;

    uint32_t fg;
    uint32_t bg;
    uint32_t fg_highlight;
    uint32_t bg_highlight;


}PopupWindow;

typedef struct{
    char* message;
    int x, y, width, height,blinks;
    _Bool done;
    Background* background;



}global_message;

typedef struct{
    uint32_t x,y,width,height,colour,brush_size,pixel_size;
    _Bool show_pixels;
    uint32_t* bitmap;

}DrawArea;

typedef struct{
    int x;
    int y;
    int width;
    int height;
    struct button* buttons;
    int buttons_index;

}DropDown;

volatile global_message GLOBAL_MESSAGE;

TextArea* textarea;

DrawArea* drawarea=NULL;

TextBox* textbox=NULL;

Image SEAGULL;
Image GOS_LOGO;

int buttons_capacity=32;
struct button** buttons;

int buttons_index = 0;

int text_buttons_index=0;

volatile _Bool OS_LEFT_CLICK;

volatile int* OS_KEY=0x490000; //important

void handle_key_press(unsigned char scancode);

void ask_question(Question* q);
//void print_message(char msg[]);

_Bool mouse_intersects(int x, int y,int width, int height);

int write_rowcol_text(int row,int column, const char* msg, uint32_t colour);
int write_text(int row, int column, const char* msg, uint32_t colour);
void draw_rect(int x,int y,int width, int height, uint32_t colour);
void draw_bordered_rect(int x,int y,int width, int height,uint32_t colour);
void draw_mouse(void);
void restore_20x20_background();
void save_20x20_background();
void save_mouse();
void restore_mouse();
void draw_scrollarea(ScrollArea* area);

void save_fe_file(char* file_extension);

void display_bar();

void timer_handler(void);
void keyboard_handler(void);
void default_handler(void);
void disk_handler(void);
void mouse_handler(void);
void syscall_handler(void);

void read_sector_poll(unsigned int lba, unsigned char* buffer);
void write_sector_poll(unsigned int lba, unsigned char* buffer);

_Bool rect_contains(int pointx, int pointy, int rectx, int recty, int rectwidth, int rectheight);
_Bool compare_strings(char string1[], char string2[]);
void save_text_file(void);
void open_text_file(void);
void open_file(char* filename);

void run_command();

void run_program();

void write_char_buffer_index();

struct button* create_button(int xc, int yc, int widthval, int heightval, uint32_t bgcol, uint32_t textcol, uint32_t bghighlightcol, char *textval, void (*function)(void));
void create_scrollbar();
void restore_background(Background *bg);
void save_background(Background *bg);

char *str_cat(char *str1, char *str2);
char *str_copy(char *str);
int str_len(char* str,_Bool include);

volatile _Bool mouse_drawing_enabled=true;
volatile _Bool mouse_needs_redrawn=false;

struct Error** errors;
uint64_t error_index = 0;
int error_capacity = 64;

TextBox* console;
TextBox* error_box;

char* console_buf;
char* error_buf;

int console_index;
int console_capacity;


volatile _Bool should_blink=0;
volatile _Bool quarter_second=0;
volatile _Bool redraw_screen=0;
volatile _Bool hovering=false;

void MAIN_LOOP(){
    _Bool running=running_program;
    running_program=false;
    if (hovering){
        hovering = 0;
        highlight_buttons();
    }
    if (should_blink){
        should_blink = 0;
        blink_cursor();
        
        display_bar();

        read_time();
    }
    
    if (mouse_needs_redrawn && mouse_drawing_enabled){
        draw_mouse();
        mouse_needs_redrawn = false;
    }
    process_pending_buttons();
    __asm__ __volatile__("hlt");
    running_program=running;
}

int write_xy_text(int x, int y, char *msg, uint32_t colour);


char current_file_name[32] = {'\0'};
char resave_file_name[32]= {'\0'};

void refresh_all_scrollareas(){
    for (int i=0;i<textarea->scroll_index;i++){
        draw_scrollarea(textarea->scroll_areas[i]);
    }

}

int len(char* text){
    int len;
    for (len=0;text[len]!='\0';len++);
    return len;
}

void clear_current_file_name(){
    for (int i=0;i<32;i++){
        current_file_name[i]='\0';
    }
}

void clear_resave_file_name(){
    for (int i=0;i<32;i++){
        resave_file_name[i]='\0';
    }
}

void realloc(void* buf1,int index,int* capacity){ //???
    *capacity*=2;
    //void* realloc_arr=MALLOC(*capacit)
    for (int i=0;i<index;i++){

    }


}

int abs(int n){
    return n<0?-n:n;
}

int centre_x(int width){
    return (ppr/2)-(width/2);

}

int centre_y(int height){
    return ((768/2)-(height/2));

}


void* memcpy32(uint32_t* dst, uint32_t* src, uint32_t count){
    __asm__ __volatile__ (
        "rep movsl"
        : "+D"(dst), "+S"(src), "+c"(count)
        :
        : "memory"


    );

    return dst;

}

/*
    memcpy8
*/

void* memcpy(uint32_t* dst,  uint32_t* src, uint32_t count){
    __asm__ __volatile__ (
        "rep movsb"
        : "+D"(dst), "+S"(src), "+c"(count)
        :
        : "memory"


    );


    return dst;
}

char* sprintf(char* exp ){
    /*va_list args;
    va_start(args,exp);*/

}

static inline unsigned short inw(unsigned short port);

void memset8(uint32_t* dst, uint8_t val, uint32_t count){
    __asm__ __volatile__(
        "rep stosb"
        : "+D"(dst), "+c"(count)
        : "a"(val)
        : "memory"
    );

}

void memset32(uint32_t *dst, uint32_t val, uint32_t count){
    //while (inw(0x3DA)&8);
    //while (!(inw(0x3DA)&8));
    
    __asm__ __volatile__(
        "rep stosl"
        : "+D"(dst), "+c"(count)
        : "a"(val)
        : "memory"
    );

}

void memset(void* s, int c, uint64_t n){
    memset32(s,c,n);

}

void recolour_area(int x,int y, int width, int height, uint32_t old,uint32_t new){
    restore_mouse();
    

    for (int i=x;i<x+width;i++){
        for (int j=y;j<y+height;j++){
            if (framebuffer[j*ppr+i]==old)framebuffer[j*ppr+i]=new;
        }
    }
    save_mouse();

}

uint32_t get_complementary_colour(uint32_t colour);

void highlight_button(struct button* btn) {    
    recolour_area(btn->x,btn->y,btn->width,btn->height,btn->bgcolour,btn->bghighlightcolour);
    recolour_area(btn->x,btn->y,btn->width,btn->height,btn->textcolour,get_complementary_colour(btn->textcolour));

}

void draw_button(struct button* btn) {
    //restore_20x20_background();
    draw_rect(btn->x, btn->y, btn->width, btn->height, btn->bgcolour);

    int length=str_len(btn->text,false);

    write_xy_text(btn->x+((btn->width/2)-((10*len(btn->text))/2)),btn->y+3,btn->text,btn->textcolour);
    //save_20x20_background();
    //draw_mouse();

}

void draw_xy_button(int x, int y, struct button* btn) {
    draw_rect(x,y,btn->width,btn->height,btn->bgcolour);
    write_xy_text((x+(btn->width/2))-((10*len(btn->text))/2),y+3,btn->text,btn->textcolour);
}

uint32_t get_complementary_colour(uint32_t colour){
    int R=colour>>16;
    int G=(colour&0x00ff00)>>8;
    int B=colour&0x0000ff;

    R=255-R;
    G=255-G;
    B=255-B;

    return (R<<16)|(G<<8)|B;
}

void draw_text_button(struct text_button btn){
    restore_mouse();

    draw_rect(btn.x,btn.y,btn.width,btn.height,bg_colour);
    write_xy_text(btn.x,btn.y+((btn.height/2)-9),btn.text,btn.fg);

    save_mouse();
}

void highlight_text_button(struct text_button btn){
    restore_mouse();

    draw_rect(btn.x,btn.y,btn.width,btn.height,btn.bghighlightcolour);
    write_xy_text(btn.x,btn.y+((btn.height/2)-9),btn.text,btn.fg);

    save_mouse();
}

void draw_text_box(TextBox* box){
    restore_mouse();
    save_background(box->background);


    draw_bordered_rect(box->x,box->y,box->width,box->height,box->bg);
    draw_bordered_rect(box->x,box->y,box->width,25,LIGHT_GRAY);

    int titlex=box->x+5;
    int titley=box->y+((25/2)-5);

    write_xy_text(titlex,titley,box->title,BLACK);

    //draw_button(textbox->close_button);

    save_mouse();


}

void close_text_box(){
    restore_mouse();

    /*if (textbox == NULL) {
        save_mouse();

        return;
    }*/

    for (int i=0;i<textbox->buttons_index;i++){
        
        FREE(textbox->buttons[i]);
    }

    textarea->buttons_index--;

    //textbox->close_button=NULL;

    restore_background(textbox->background);
    
    //FREE(textbox->buffer);
    FREE(textbox->buttons);
    FREE(textbox->close_button);
    FREE(textbox->background->bg_buf);
    FREE(textbox->background);
    //FREE(textbox->buffer);
    FREE(textbox);
    

    textbox=NULL;
    
    save_mouse();

    //refresh_text();

}

void draw_text_area(){
    restore_mouse();

    draw_rect(0,25,1024,768-25,textarea->bg);
    draw_bordered_rect(0,25,1024,25,LIGHT_GRAY);

    write_xy_text(10,30,textarea->name,BLACK);

    draw_button(textarea->close_button);
    
    save_mouse();
}

void free_text_area(){
    clear_resave_file_name();

    FREE(textarea->buffer);

    if (drawarea!=NULL){
        FREE(drawarea->bitmap);
        FREE(drawarea);
        drawarea=NULL;
    }
    
    for (int i=0;i<textarea->scroll_index;i++){
        ScrollArea* area=textarea->scroll_areas[i];
        for (int j=0;j<area->buttons_index;j++){
            ScrollButton* btn=area->buttons[j];

            FREE(btn->button);
            FREE(btn);

        }
        for (int j=0;j<area->images_index;j++){
            ScrollImage* img=area->images[j];

            FREE(img->image->bitmap);
            FREE(img->image);


            
        }
        for (int j=0;j<area->index;j++){
            if (area->text[j]->malloced){
                FREE(area->text[j]->text);
            }
            
            FREE(area->text[j]);
            
        }
        FREE(area->background->bg_buf);
        FREE(area->background);

        FREE(area->buttons);
        FREE(area->text);
        FREE(area->images);
        FREE(area);
    }
    FREE(textarea->scroll_areas);

    FREE(textarea->tab);

    for(int i=0;i<textarea->buttons_index&&textarea->buttons[i]!=NULL;i++)
        FREE(textarea->buttons[i]);
    
    FREE(textarea->buttons);

    //for (int i=0;i<textarea->rectangles_index;i++)FREE(textarea->rectangles[i]);
    //FREE(textarea->rectangles);

    //for(int i=0;i<textarea->temps_index;i++)FREE(textarea->temps[i]->bg_buf);
    for(int i=0;i<textarea->temps_index;i++)
        FREE(textarea->temps[i]);
    
    FREE(textarea->temps);

    FREE(textarea);
    
}

void destroy_global_message();

void create_text_area(uint32_t bg, uint32_t fg,uint32_t fg_highlight,uint32_t bg_highlight,char* name,_Bool editable){
    if (!GLOBAL_MESSAGE.done)
        destroy_global_message();

    if (textbox!=NULL)
        close_text_box();

    if (textarea!=NULL)
        free_text_area();

    //init_idt();

    textarea=MALLOC(sizeof(TextArea));
    /*TextArea* ta=MALLOC(sizeof(TextArea));
    textarea=ta;*/

    textarea->name=name;

    textarea->bg=bg;
    textarea->fg=fg;

    uint32_t bgcomp=get_complementary_colour(bg);
    uint32_t fgcomp=get_complementary_colour(fg);

    textarea->bg_highlight=bgcomp==fg?bgcomp+1:bgcomp;
    textarea->fg_highlight=fgcomp==bg?fgcomp-1:fgcomp;

    textarea->tab_capacity=256;
    textarea->tab_index=0;
    textarea->tab=MALLOC(textarea->tab_capacity*sizeof(struct button*));
    textarea->tab[0]=NULL;

    textarea->editable=editable;

    textarea->buffer_capacity=1024;
    textarea->buffer_index=0;
    textarea->buffer=MALLOC(textarea->buffer_capacity);

    textarea->selection_start=0;
    textarea->selection_end=0;
    textarea->selecting=false;

    //textarea->text_slabs=MALLOC(512*sizeof(TextSlab));

    
    textarea->scroll_bounds=0;

    fill_array(textarea->buffer,textarea->buffer_capacity,'\0');
    
    textarea->buttons_capacity=128;
    textarea->buttons_index=0;
    textarea->buttons=MALLOC(textarea->buttons_capacity*sizeof(struct button*));
    
    textarea->cursorx = 0;
    textarea->cursory = 0;


    textarea->temps_index=0;
    textarea->temps_capacity=512;
    textarea->temps=MALLOC(textarea->temps_capacity*sizeof(Background));
    
    
    textarea->close_button=create_button(1024-27, 27, 25, 21, RED, BLACK, ORANGE, "X", home);
    
    textarea->scroll_capacity=32;
    textarea->scroll_index=0;
    textarea->scroll_areas=MALLOC(textarea->scroll_capacity*sizeof(ScrollArea*));

    draw_text_area();
    //configure_tabs();
    //create_scrollbar();

}
void draw_scrollarea(ScrollArea* scrollarea);

void refresh_draw_area(DrawArea* area);

DrawArea* create_draw_area(int x, int y, int width, int height){
    DrawArea* area=MALLOC(sizeof(DrawArea));
    area->brush_size=4;
    area->colour=BLACK;
    area->width=width;
    area->height=height;
    area->x=x;
    area->y=y;
    area->pixel_size=1;
    area->show_pixels=true;
    area->bitmap=MALLOC(width*height*sizeof(uint32_t));
    memset32(area->bitmap,WHITE,width*height);
    refresh_draw_area(area);
    return area;
}

void refresh_draw_area(DrawArea* area){
    restore_mouse();
    for (int i=0;i<area->height;i++){
        int ind=((area->y+i)*ppr)+area->x;
        memcpy32(framebuffer+ind,area->bitmap+(i*area->width),area->width);
    }
    draw_rect_outline(area->x,area->y,area->width*area->pixel_size,area->height*area->pixel_size,BLACK);
    save_mouse();
}

void refresh_text_area(){
    for (int i=0;i<textarea->temps_index;i++){
        restore_background(textarea->temps[i]);
        FREE(textarea->temps[i]->bg_buf);
        FREE(textarea->temps[i]);
    }
    textarea->temps_index=0;

    /*for (int i=0;i<textarea->scroll_index;i++){
        restore_background(textarea->scroll_areas[i]->background);
        draw_scrollarea(textarea->scroll_areas[i]);
    }*/
}

char* quick_int_str(int n);

void create_text_box(int x,int y, int width, int height, uint32_t bg, uint32_t fg, char* title,_Bool is_console){
    if (textbox!=NULL)
        close_text_box();
    
    TextBox* re=MALLOC(sizeof(TextBox));

    re->x=x;
    re->y=y;
    re->width=width;
    re->height=height;
    re->bg=bg;
    re->fg=fg;
    re->title=title;
    re->cursorx=0;
    re->cursory=0;
    re->scroll_bounds=0;
    re->editable=true;
    re->is_console=is_console;

    re->MAX=(re->height/18)-3;


    //re->close_button=buttons[buttons_index-1];

    Background* background=MALLOC(sizeof(Background));


    background->x=x;
    background->y=y;
    background->width=width;
    background->height=height;

    background->bg_buf=(uint32_t*)MALLOC(width*height*sizeof(uint32_t));

    save_background(background);

    re->background = background;

    textbox=re;
    
    //draw_text_box(textbox);
    draw_text_box(textbox);
    
    textbox->close_button =create_button((x+width)-25,y,25,25,RED,BLACK,ORANGE,"X",close_text_box);

    textbox->buttons_capacity=128;
    textbox->buttons_index=0;
    textbox->buttons=MALLOC(textbox->buttons_capacity*sizeof(struct button*));

    textbox->index=0;
    textbox->capacity=512;
    textbox->buffer=MALLOC(textbox->capacity);

    fill_array(textbox->buffer,textbox->capacity,'\0');

    
    
    //draw_button(textbox->close_button);
}

Background* create_rectangle_bg(int x,int y,int width, int height){
    Background* bg=MALLOC(sizeof(Background));

    bg->x=x;
    bg->y=y;
    bg->width=width;
    bg->height=height;

    save_background(bg);

    return bg;



}

void draw_rect_outline(int x,int y, int width, int height,uint32_t colour){
    //Background* bg=MALLOC(sizeof(Background));

    /*bg->x=x;
    bg->y=y;
    bg->width=width;
    bg->height=height;
    save_background(bg);*/


    for (int i=x;i<x+width;i++){
        draw_pixel(i,y,colour);
        draw_pixel(i,y+height,colour);
    }
    for (int i=y;i<y+height;i++){
        draw_pixel(x,i,colour);
        draw_pixel(x+width,i,colour);
    }

    //return bg;

}
int get_line_size(char* line){
    int i;
    for (i=1;line[i]!='\n'&&line[i]!='\0';i++);
    return i;

}

Background* draw_text_slab(TextSlab* slab){
    char* text=slab->text;
    int x=slab->x;
    int y=slab->y;
    uint32_t colour=slab->colour;


    int width=0;
    for (int i=0;text[i]!='\0';i++){
        if (text[i]=='\n'||!i){
            if (get_line_size(text+i)>width)
                width=get_line_size(text+i+1);

        }
    }

    

    int height=18;
    for (int i=0;text[i]!='\0';i++)
        if(text[i]=='\n')
            height+=18;


    Background* bg=create_rectangle_bg(x,y,width*10+10,height);
    draw_rect_outline(x,y,width*10+10,height,colour);
    int colx=0;
    int coly=0;

    for (int i=0;text[i]!='\0';i++){
        if (text[i]=='\n'){
            coly++;
            colx=0;
            continue;
        }

        write_char(x+(width/2/10)+(colx++*10),y+2+coly*18,font8x16[text[i]],colour);
    }

    FREE(slab->text);
    FREE(slab);
    return bg;
}

void draw_sized_text_slab(int x,int y,int width, int height, char* text, uint32_t colour){
    if (width==-1)
        width=len(text);
    if (height==-1)
        height=20;

    draw_rect_outline(x,y,width,height,colour);

    x=(x+(width/2))-((len(text)*10)/2);

    write_xy_text(x,y+1,text,colour);


}

Background* create_text_slab(int x,int y,char* text,uint32_t colour){
    TextSlab* slab=MALLOC(sizeof(TextSlab));
     

    slab->colour=colour;
    slab->x=x;
    slab->y=y;
    slab->text=text;

    return draw_text_slab(slab);
    


}


void draw_image(int x,int y,Image* image){
    uint32_t* bitmap=image->bitmap;

    for (int i=0;i<image->height;i++){
        for (int j=0;j<image->width;j++){
            int bm=bitmap[i*image->width+j];
            if(!bm)
                continue;
            
            bm>>=8;
            

            draw_pixel(x+j,y+i,bm);

        }
    }

}

Image* create_image(int width, int height,uint32_t* bitmap){
    Image* image=MALLOC(sizeof(Image));

    image->bitmap=bitmap;
    image->width=width;
    image->height=height;

    return image;

}

struct button* create_undrawn_button(int xc, int yc, int widthval, int heightval, uint32_t bgcol, uint32_t textcol, uint32_t bghighlightcol,char* textval,_Bool is_file, _Bool is_program, _Bool is_image, void (*function)(void),char* arg1) {
    struct button* btn=MALLOC(sizeof(struct button));

    btn->x = xc;
    btn->y = yc;
    btn->width = widthval==-1?len(textval)*10:widthval;
    btn->height = heightval==-1?20:heightval;
    btn->bgcolour = bgcol;
    btn->textcolour = textcol;
    btn->bghighlightcolour = bghighlightcol;
    btn->text = textval;
    btn->is_file=is_file;
    btn->is_image=is_image;
    btn->is_program=is_program;
    btn->arg1=arg1;

    btn->on_click=function;

    textarea->buttons[textarea->buttons_index++] = btn;

    return btn;



}

struct button* create_textbox_button(uint32_t bgcol, uint32_t textcol, uint32_t bghighlightcol,char* textval,void (*function)(char* args),char* arg){
    struct button* btn=MALLOC(sizeof(struct button));
    int j=0;

    
    if (textbox->buttons[0]!=NULL){
        for (int i=0;i<textbox->buttons_index;i++){
            j+=textbox->buttons[i]->width;
            j+=10;
            
        }
    }
    

    btn->x=textbox->x+(10*len(textbox->title)+10)+j+10;
    btn->y=textbox->y+4;
    btn->width=0;
    for (int i=0;textval[i]!='\0';i++)
        btn->width+=10;
    
    
    btn->bgcolour=LIGHT_GRAY;
    btn->bghighlightcolour=LIGHTER_GRAY;

    btn->on_click=function;

    btn->height=19;
    btn->is_file=false;
    btn->is_image=false;
    btn->is_program=false;
    btn->textcolour=textcol;
    btn->text=textval;
    
    

    if (arg!=NULL){
        btn->arg1=arg;
        //write_rowcol_text(12,10,str_cat("arg= ",btn->arg1),ORANGE);
        btn->on_click_arg=function;
    }
    else{
        btn->arg1=NULL;
    }

    draw_button(btn);
    textbox->buttons[textbox->buttons_index++]=btn;
    //textarea->buttons[textarea->buttons_index++]=btn;

    return btn;

}



void create_global_message(char* msg){
    if (!GLOBAL_MESSAGE.done){
        destroy_global_message();
    }
    GLOBAL_MESSAGE.message=msg;

    GLOBAL_MESSAGE.width=10*len(msg);
    GLOBAL_MESSAGE.height=20;

    GLOBAL_MESSAGE.x=(1024/2)-(GLOBAL_MESSAGE.width);
    GLOBAL_MESSAGE.y=100;

    GLOBAL_MESSAGE.blinks=0;
    GLOBAL_MESSAGE.done=false;
    GLOBAL_MESSAGE.background=create_rectangle_bg(GLOBAL_MESSAGE.x,GLOBAL_MESSAGE.y,GLOBAL_MESSAGE.width,GLOBAL_MESSAGE.height);


}

struct button* create_button(int xc, int yc, int widthval, int heightval, uint32_t bgcol, uint32_t textcol, uint32_t bghighlightcol,char* textval,void (*function)(void)) {
    struct button* btn=MALLOC(sizeof(struct button));

    btn->x = xc;
    btn->y = yc;
    btn->width = widthval==-1?len(textval)*10:widthval;
    btn->height = heightval==-1?18:heightval;
    btn->bgcolour = bgcol;
    btn->textcolour = textcol;
    btn->bghighlightcolour = get_complementary_colour(bgcol);
    btn->text = textval;
    btn->is_file=false;
    btn->is_program=false;
    btn->is_image=false;
    btn->arg1=NULL;

    btn->on_click=function;

    /*if (buttons == NULL) {
        buttons_capacity = 32;
        buttons = MALLOC(buttons_capacity * sizeof(struct button*));
        buttons_index = 0;
    }*/

    textarea->buttons[textarea->buttons_index++] = btn;
    draw_button(btn);

    return btn;



}
void pop_and_delete_button(){
    textarea->buttons[--textarea->buttons_index]=NULL;
}
void pop_button(){
    --textarea->buttons_index;
}

char* quick_int_str(int text);

struct button* create_tab_button(uint32_t textcol,char* text, void(*function)(void)){
    struct button* btn=MALLOC(sizeof(struct button));
    int j=0;

    
    if (textarea->tab[0]!=NULL){
        for (int i=0;i<textarea->tab_index;i++){
            //write_rowcol_text(i+10+textarea->tab_index,20,str_cat("WIDTH= ",quick_int_str(textarea->tab[i]->width)),PURPLE);
            j+=textarea->tab[i]->width;
            j+=15;
            
        }
    }
    

    btn->x=100+j;
    btn->y=27;
    btn->width=0;
    for (int i=0;text[i]!='\0';i++)
        btn->width+=10;
    
    
    btn->bgcolour=LIGHT_GRAY;
    btn->bghighlightcolour=LIGHTER_GRAY;
    btn->on_click=function;

    btn->height=21;
    btn->is_file=false;
    btn->is_image=false;
    btn->is_program=false;
    btn->textcolour=textcol;
    btn->text=text;
    btn->arg1=NULL;

    draw_button(btn);
    textarea->tab[textarea->tab_index++]=btn;
    textarea->buttons[textarea->buttons_index++]=btn;

    return btn;
}





volatile uint64_t rng_state=0;


void unhighlight_button(struct button* btn){

    recolour_area(btn->x,btn->y,btn->width,btn->height,btn->bghighlightcolour,btn->bgcolour);
    recolour_area(btn->x,btn->y,btn->width,btn->height,get_complementary_colour(btn->textcolour),btn->textcolour);
    
}

static inline unsigned long long rdtsc() {
    unsigned int lo, hi;

    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));//get the whole time stamp counter value

    return ((unsigned long long )hi<<32)|lo;//HHHH0000 | 0000LLLL essentially
}


void init_rng(){
    rng_state=rdtsc()^TIMER_TICKS;

    if (rng_state==0)rng_state=1;

}

int gen_rand(){
    rng_state^=rng_state<<13;
    rng_state^=rng_state>>7;
    rng_state^=rng_state<<17;

    return rng_state^0xffffffff;
}

static inline unsigned short inw(unsigned short port) {
    unsigned short result;
    __asm__ __volatile__("inw %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

static inline void outw(unsigned short port, unsigned short value) {
    __asm__ __volatile__("outw %%ax, %%dx" :: "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "d"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

char* get_disk_info(uint16_t* data){
    outb(0x1F6,0xA0);
    outb(0x1F7,0xEC);

    int timeout=15000000;
    while (timeout-- && !(inb(0x1F7) & 8)); //poll

    for (int i=0;i<256;i++){
        data[i]=inw(0x1F0);
    }

    return str_copy(data);


}

int abs_difference(int num1, int num2){
    return (num1-num2 < 0)?num2-num1:num1-num2;
}

void draw_pixel(int x, int y, uint32_t colour) {

    framebuffer[y * ppr + x] = colour;
    
}

_Bool is_prime(int value){
    if (value%2==0)
        return true;
    
    for (int i=2;i<value;i++){
        if (value%i==0){
            return 0;
        }
    }
    return 1;
}

int number_of_instances(int* list, int value){
    int number=0;
    for (int i=0;i<sizeof(list);i++){
        if (list[i]==value)number++;
    }
    return number;
}

double sqrt(int val){
    double x=val/2.0;

    for (int i=0;i<10;i++){
        x=(x+val/x)/2.0;//newton
    }
    return x;
}

void draw_straight_line(int x1,int y1, int x2, int y2, uint32_t colour){
    restore_mouse();




    save_mouse();

}

void write_char(int x, int y, unsigned char character[], uint32_t colour);

void write_rowcol_char(int col,int row, char character, uint32_t colour) {
    write_char(col * 10, row * 18, font8x16[character], colour);
}

void clear_all_rows_textbox(){
    int old=textbox->cursory;
    while (textbox->cursory<(textbox->height/18)-2){
        clear_row_textbox();
        textbox->cursory++;
    }
    textbox->cursory=old;
}

int count_lines(char* buf){
    int c=0;
    for (int i=0;buf[i]!='\0';i++){
        if (buf[i]=='\n')
            c++;
    }
    return c;
}

void write_char(int x, int y, unsigned char character[], uint32_t colour) {
    restore_mouse();

    int xval = x;
    for (int i = 0; i < 16; i++) {
        unsigned char row = character[i];
        xval = x;
        for (int j = 0; j < 16; j++) {//let row=0b00011000

            if (row & 0x80) {
                draw_pixel(xval, y, colour);
            }

            row = row << 1;
            xval++;
        }
        y++;

    }
    save_mouse();

}

int get_line_length(char* buf,int* index){
    int l=0;
    if (buf[*index]=='\n')(*index)++;

    while (buf[*index]!='\n' && buf[*index]!='\0'){l++;(*index)++;}
    if (buf[*index]=='\n')(*index)--;

    return l;
}

int get_textbox_line(int line){
    int nc=0;
    int i=0;

    for (i=0;i<textbox->capacity&&nc!=line;i++){
        if (textbox->buffer[i]=='\n')nc++;

    }

    

    return i;

}

char* get_last_line(char* buf){
    int i=0;
    while (buf[i]!='\0')i++;
    
    while (buf[i]!='\n' && i>0)i--;
    i--;
    while (buf[i]!='\n' && i>0)i--;

    return buf+i;
}

int get_line(int line){
    int nc=0;
    int i=0;

    for (i=0;i<textarea->buffer_capacity&&nc!=line;i++){
        if (textarea->buffer[i]=='\n')nc++;
    }
    return i;
}

int get_line_number(){
    int nc=0;
    for (int i=0;i<textarea->buffer_index&&textarea->buffer[i]!='\0';i++){
        if (textarea->buffer[i]=='\n')nc++;

    }
    return nc;

}

_Bool is_line_textbox(){
    for (int i=textbox->index;textbox->buffer[i]!='\0';i++){
        if (textbox->buffer[i]=='\n')return true;
    }
    return false;

}

_Bool is_line(){
    for (int i=textarea->buffer_index;textarea->buffer[i]!='\0';i++){
        if (textarea->buffer[i]=='\n')return true;
    }
    return false;

}

void scroll_up_scrollareas(){
    for(int i=0;i<textarea->scroll_index;i++){
        int x=textarea->scroll_areas[i]->x;
        int y=textarea->scroll_areas[i]->y;
        int width=textarea->scroll_areas[i]->width;
        int height=textarea->scroll_areas[i]->height;
        
        if (!rect_contains(mouse_x,mouse_y,x,y,width,height))
            continue;

        if (textarea->scroll_areas[i]->scroll_bounds==0)
            continue;

        textarea->scroll_areas[i]->scroll_bounds--;
        draw_scrollarea(textarea->scroll_areas[i]);
    }
    
}

void scroll_up_textbox(){
    if (textbox->scroll_bounds<=0 || !textbox->editable)
        return;

    textbox->scroll_bounds--;

    //textbox->index=textbox->cursory<textbox->MAX?get_textbox_line(textbox->scroll_bounds+(textbox->cursory+1)):get_textbox_line(textbox->scroll_bounds+textbox->cursory);
    textbox->index=get_textbox_line(textbox->scroll_bounds+textbox->cursory+1);
    //textbox->index=get_textbox_line(textbox->scroll_bounds);
    textbox->cursorx=0;
    //clear_row_textbox();

    refresh_textbox();

}

void scroll_up(){
    if (textbox){
        scroll_up_textbox();
        return;
    }
    scroll_up_scrollareas();
    if (textarea->scroll_bounds==0||!textarea->editable)return;

    textarea->scroll_bounds--;

    textarea->buffer_index=get_line(textarea->scroll_bounds);
    textarea->cursorx=0;

    refresh_text();
}

void scroll_down_scrollareas(){
    for(int i=0;i<textarea->scroll_index;i++){
        int x=textarea->scroll_areas[i]->x;
        int y=textarea->scroll_areas[i]->y;
        int width=textarea->scroll_areas[i]->width;
        int height=textarea->scroll_areas[i]->height;

        if (!rect_contains(mouse_x,mouse_y,x,y,width,height))
            continue;
        textarea->scroll_areas[i]->scroll_bounds++;

        draw_scrollarea(textarea->scroll_areas[i]);
    }

    

}

void scroll_down_textbox(){
    if (!textbox->editable || (!is_line_textbox() && textbox->cursory==0) )
        return;

    textbox->scroll_bounds++;


    //textbox->index=textbox->cursory>0?get_textbox_line(textbox->scroll_bounds+(textbox->cursory-1)):get_textbox_line(textbox->scroll_bounds+textbox->cursory);
    textbox->index=get_textbox_line(textbox->scroll_bounds+(textbox->cursory-1));
    //textbox->index=get_textbox_line(textbox->scroll_bounds);
    textbox->cursorx=0;
    
    
    

    refresh_textbox();


}

void scroll_down(){
    
    if (textbox){
        scroll_down_textbox();
        return;
    }
    scroll_down_scrollareas();
    if ((!is_line()&&textarea->cursory==0)||!textarea->editable)return;

    textarea->scroll_bounds++;

    textarea->buffer_index=get_line(textarea->scroll_bounds);
    textarea->cursorx=0;

    refresh_text();
}

void draw_scrollbar(Scrollbar* bar){
    //draw_bordered_rect(bar->x,bar->y,12,bar->height,LIGHT_GRAY);

    draw_button(bar->down);
    draw_button(bar->up);

}

void create_scrollbar(){

    Scrollbar* bar=MALLOC(sizeof(Scrollbar));

    int x=textbox==NULL?1024-12:(textbox->x+textbox->width)-12;
    int y=textbox==NULL?50:textbox->y+25;
    int height=(textbox==NULL?768-50:textbox->height-25);

    //draw_bordered_rect(x,y,12,height,LIGHT_GRAY);

    create_button(x,y,12,20,PALE_RED,BLACK,TAB_COLOUR,"^",scroll_up);

    create_button(x,(textbox==NULL?768-20:textbox->y+textbox->width-20),12,20,PALE_RED,BLACK,TAB_COLOUR,"V",scroll_down);

    bar->down=textarea->buttons[textarea->buttons_index-1];
    bar->up=textarea->buttons[textarea->buttons_index-2];

    bar->x=x;
    bar->y=y;

    bar->height=height;



    textarea->scrollbar=bar;

}

ScrollText* create_scrolltext(ScrollArea* area,int row, int col,char* text, _Bool malloced){
    ScrollText* st=MALLOC(sizeof(ScrollText));
    st->text=text;
    st->col=col;
    st->row=row;
    st->malloced=malloced;

    area->text[area->index++]=st;

    draw_scrollarea(area);

    return st;

}

ScrollImage* create_scrollimage(ScrollArea* area,int row, int col, Image* img){
    ScrollImage* image=MALLOC(sizeof(ScrollImage));

    image->col=col;
    image->row=row;
    image->image=img;

    area->images[area->images_index++]=image;

    draw_scrollarea(area);

    return image;
    

}

ScrollButton* create_scrollbutton(ScrollArea* area,int row,int col, struct button* btn){
    ScrollButton* but=MALLOC(sizeof(ScrollButton));

    but->button=btn;
    but->row=row;
    but->col=col;

    area->buttons[area->buttons_index++]=but;


    draw_scrollarea(area);

    return but;

}

void draw_scrollarea(ScrollArea* scrollarea){
    restore_background(scrollarea->background);
    FREE(scrollarea->background->bg_buf);
    FREE(scrollarea->background);

    int R=(scrollarea->bg >> 16);
    int G=(scrollarea->bg >> 8)&0xff;
    int B=(scrollarea->bg)&0xff;

    int brightness=(R*299)+(G*587)+(B*114)/1000;
    int finalbw=brightness<=128?WHITE:BLACK;

    scrollarea->background=create_rectangle_bg(scrollarea->x,scrollarea->y,scrollarea->width,scrollarea->height);

    Background* right=create_rectangle_bg(scrollarea->x+2+scrollarea->width,scrollarea->y,1024-(scrollarea->x+scrollarea->width),scrollarea->height);
    Background* left=create_rectangle_bg(0,scrollarea->y,scrollarea->x,scrollarea->height);
    Background* down=create_rectangle_bg(scrollarea->x,scrollarea->y+2+scrollarea->height,scrollarea->width,768-(scrollarea->y+scrollarea->height));
    Background* up=create_rectangle_bg(scrollarea->x,0,scrollarea->width,scrollarea->y-20);

    draw_rect_outline(scrollarea->x,scrollarea->y,scrollarea->width,scrollarea->height,get_complementary_colour(finalbw));
    if (scrollarea->bg)draw_rect(scrollarea->x+2,scrollarea->y+2,scrollarea->width-4,scrollarea->height-4,scrollarea->bg);

    int MAX=scrollarea->scroll_bounds+(scrollarea->height/18); //max row
    
    for (int j = 0; j < scrollarea->index; j++){
        if (scrollarea->text[j]->row<scrollarea->scroll_bounds || scrollarea->text[j]->row > MAX){
            continue;
        }
        write_xy_text(scrollarea->x +2+ scrollarea->text[j]->col * 10, scrollarea->y + (scrollarea->text[j]->row-scrollarea->scroll_bounds) * 18, scrollarea->text[j]->text, scrollarea->fg);
    }

    for (int j = 0; j < scrollarea->buttons_index; j++){
        if (scrollarea->buttons[j]->row < scrollarea->scroll_bounds || scrollarea->buttons[j]->row > MAX)
            continue;
        //draw_rect(scrollarea->x + 2 + scrollarea->buttons[j]->col * 10, scrollarea->y + (scrollarea->buttons[j]->row-scrollarea->scroll_bounds) * 18,scrollarea->buttons[j]->button->width,scrollarea->buttons[j]->button->height,GREEN);
        draw_xy_button(scrollarea->x + 2 + scrollarea->buttons[j]->col * 10, scrollarea->y + (scrollarea->buttons[j]->row-scrollarea->scroll_bounds) * 18, scrollarea->buttons[j]->button);
    }

    for (int j = 0; j < scrollarea->images_index; j++){
        if (scrollarea->images[j]->row<scrollarea->scroll_bounds || scrollarea->images[j]->row>=MAX)
            continue;
        
        ScrollImage* img=scrollarea->images[j];

        if (img->col>(scrollarea->width/10)||img->col<0)
            continue;
        
        if (img->row<scrollarea->scroll_bounds){
            Image* new=MALLOC(sizeof(Image));
            new->width=img->image->width;
            new->height=img->image->height-(18*abs_difference(scrollarea->scroll_bounds,img->row));

            uint32_t* bmp=MALLOC(new->width*new->height*4);

            for (int i=abs_difference(scrollarea->scroll_bounds,img->row);i<new->width*new->height;i++){
                bmp[i]=img->image->bitmap[i];
            }
            new->bitmap=bmp;

            draw_image(scrollarea->x+2+img->col*10,scrollarea->y+2,new);

            FREE(bmp);
            FREE(new);
            continue;
        }

        if (((img->row-scrollarea->scroll_bounds)*18)+(img->image->height)>scrollarea->y+scrollarea->height){
            Image* new=MALLOC(sizeof(Image));
            new->width=img->image->width;
            new->height=abs_difference(img->row*18,scrollarea->y+scrollarea->height);
            new->bitmap=MALLOC(new->width*4*new->height);


            for (int i=0;i<new->height*new->width;i++){
                new->bitmap[i]=img->image->bitmap[i];

            }
            draw_image((scrollarea->x+2+img->col*10),(scrollarea->y+scrollarea->height)-new->height,new);

            FREE(new->bitmap);
            FREE(new);
            continue;

        }
        
        draw_image(scrollarea->x +2+ scrollarea->images[j]->col * 10, scrollarea->y + (scrollarea->images[j]->row-scrollarea->scroll_bounds) * 18, scrollarea->images[j]->image);
    }

    restore_background(right);
    restore_background(left);
    restore_background(down);
    restore_background(up);

    draw_rect(scrollarea->x,scrollarea->y-20,scrollarea->width,20,finalbw);
    
    
    draw_sized_text_slab(scrollarea->x,scrollarea->y-20,scrollarea->width,20,scrollarea->title,get_complementary_colour(finalbw));

    FREE(right->bg_buf);
    FREE(left->bg_buf);
    FREE(down->bg_buf);
    FREE(up->bg_buf);

    FREE(up);
    FREE(right);
    FREE(left);
    FREE(down);

}

ScrollArea* create_scrollarea(char* title,int x,int y, int width, int height,int bg,int fg){
    ScrollArea* scrollarea=MALLOC(sizeof(ScrollArea));

    scrollarea->x=x;
    scrollarea->y=y;
    scrollarea->width=width;
    scrollarea->height=height;
    scrollarea->bg=bg;
    scrollarea->fg=fg;
    scrollarea->title=title;
    scrollarea->ID=textarea->scroll_index;
    scrollarea->index=0;
    scrollarea->capacity=512;
    scrollarea->scroll_bounds=0;

    scrollarea->text=MALLOC(scrollarea->capacity*sizeof(ScrollText*));
    
    scrollarea->buttons_index=0;
    scrollarea->buttons_capacity=128;
    scrollarea->buttons=MALLOC(scrollarea->buttons_capacity*sizeof(ScrollButton*));

    scrollarea->images_index=0;
    scrollarea->images_capacity=128;
    scrollarea->images=MALLOC(scrollarea->images_capacity*sizeof(ScrollImage*));

    scrollarea->background=create_rectangle_bg(x,y-10,width,height+10);

    textarea->scroll_areas[textarea->scroll_index++]=scrollarea;
    
    draw_scrollarea(scrollarea);

    
    return scrollarea;
}

int write_text_to_scrollarea(ScrollArea* area, int row, char* text){
    int MAX=area->width/10;
    int length=len(text);
    int i=0;
    while (i<length){
        char buf[MAX+1];
        int j;
        for (j=0;j<MAX;j++){
            if (text[i]==' '){
                int k=i;
                int k2=j;
                _Bool ok=true;
                do{
                    k++;
                    k2++;

                }while (text[k]!=' ' && text[k]!='\0');
                ok=k2<MAX;

                if (!ok){
                    i++;
                    break;
                }

            }
            if (text[i]=='\n'){
                i++;
                break;
            }
            buf[j]=text[i];
            i++;
        }
        buf[j]=0;
        create_scrolltext(area,row,0,str_copy(buf),true);
        row++;
    }
    return row;



}

int get_textbox_cursory(){
    int c=0;
    for (int i=0;i<textbox->index && textbox->buffer[i]!='\0';i++){
        if (textbox->buffer[i]=='\n')c++;
    }
    return c-textbox->scroll_bounds;

}

void destroy_char_textbox();

void clear_row_textbox(){
    int x=textbox->cursorx;

    while (textbox->cursorx<((textbox->width/10)-1)){
        destroy_char_textbox();
        textbox->cursorx++;
    }
    textbox->cursorx=x;
    
}

void refresh_textbox(){
    //draw_text_box(textbox);

    int oldx=textbox->cursorx;
    int oldy=textbox->cursory;

    int nc=0;


    //clear_all_rows_textbox();


    textbox->cursorx=0;
    textbox->cursory=0;

    draw_rect(textbox->x+2,textbox->y+25,textbox->width-4,textbox->height-25-2,textbox->bg);

    //textbox->scroll_bounds0=0;
    //textbox->scroll_bounds1=42;
    int line_index=get_textbox_line(textbox->scroll_bounds);

    for (int i=line_index;textbox->buffer[i]!='\0';i++){
        if (textbox->cursory>textbox->MAX)
            break;
        //clear_row_textbox();
        

        //destroy_char_textbox();

        if ((textbox->cursorx>=((textbox->width/10)-2) || textbox->buffer[i]=='\n')){
            textbox->cursorx=0;
            textbox->cursory++;
            
            continue;
        }

        type_char(font8x16[textbox->buffer[i]]);
    }

    textbox->cursorx=oldx;
    textbox->cursory=get_textbox_cursory();
    

}

int get_cursory(){
    int c=0;
    for (int i=0;i<textarea->buffer_index;i++){
        if (textarea->buffer[i]=='\n')c++;
    }

    return c-textarea->scroll_bounds;
}

void clear_row(){
    int x=textarea->cursorx;

    while (textarea->cursorx<101){
        destroy_char();
        textarea->cursorx++;
    }
    textarea->cursorx=x;

}

void highlight_syntax();

void refresh_text(){
    restore_mouse();
    if (textbox!=NULL){
        refresh_textbox();
        return;
    }

    int oldx=textarea->cursorx;
    int oldy=textarea->cursory;

    textarea->cursorx=0;
    textarea->cursory=0;

    memset32(framebuffer+(50*ppr),textarea->bg,1024*(768-50));

    
    for (int i=get_line(textarea->scroll_bounds);textarea->buffer[i]!=0 && textarea->cursory<41;i++){
        
        if (textarea->buffer[i]=='\n'){
            textarea->cursorx=0;
            textarea->cursory++;
            continue;
        }

        type_char(font8x16[textarea->buffer[i]]);
        

    }

    textarea->cursorx=oldx;
    textarea->cursory=get_cursory();


    highlight_syntax();
    save_mouse();
}

char* number_to_string(int num) {
    static char numstring[32];
    int i = 0;
    while (num != 0) {
        int temp = num % 10;
        numstring[i++] = '0' + temp;
        num /= 10;
    }
    numstring[i] = '\0';

    return numstring;
}

_Bool rect_contains(int pointx, int pointy, int rectx, int recty, int rectwidth, int rectheight) {
    return pointx>=rectx && pointx<=rectx+rectwidth && pointy>=recty && pointy<=recty+rectheight;
}

void restore_background(Background* bg){
    int inds=0;
    restore_mouse();

    for (int i=bg->x;i<bg->x+bg->width;i++){
        for (int j=bg->y;j<bg->y+bg->height;j++){
            framebuffer[j*ppr+i]=bg->bg_buf[inds++];
        }
    }

    /*FREE(bg->bg_buf);
    FREE(bg);*/
    
    save_mouse();
}

void save_background(Background* bg){
    bg->bg_buf=MALLOC(bg->width*bg->height*sizeof(uint32_t));

    int inds=0;
    restore_mouse();  // Hide cursor before saving to avoid capturing cursor pixels

    for (int i=bg->x;i<bg->x+bg->width;i++){
        for (int j=bg->y;j<bg->y+bg->height;j++){
            bg->bg_buf[inds++]=framebuffer[j*ppr+i];
        }
    }
    save_mouse();  // Restore cursor after saving
}

void restore_mouse(){
    if (!mouse_drawing_enabled) return;  // Already hidden, avoid nested calls
    mouse_drawing_enabled=false;
    restore_20x20_background();
}

void save_mouse(){
    if (mouse_drawing_enabled) return;  // Already shown, avoid nested calls
    save_20x20_background();
    mouse_drawing_enabled=true;
    draw_mouse();
}

void restore_20x20_background(void) {
    int inds = 0;
    for (int i = old_mouse_pos[0]; i < old_mouse_pos[0] + 20; i++) {
        for (int j = old_mouse_pos[1]; j < old_mouse_pos[1] + 20; j++) {
            framebuffer[j * ppr + i] = background_buffer[inds++];
        }
    }
}

void save_20x20_background(void) {
    old_mouse_pos[0] = mouse_x;
    old_mouse_pos[1] = mouse_y;

    int ind = 0;
    for (int i = mouse_x; i < mouse_x + 20; i++) {
        for (int j = mouse_y; j < mouse_y + 20; j++) {
            //if (framebuffer[j*ppr+i]=bg_highlight_colour){}
            background_buffer[ind++] = framebuffer[j * ppr + i];
        }
    }
}

_Bool dont_restore = 0;
_Bool typing=0;

char* concatenate_strings(char str1[], char str2[]) {
    static char new_string[sizeof(str1)+sizeof(str2)-1];
    int ind = 0;
    for (int i = 0; str1[i] != '\0'; i++) {
        new_string[ind++] = str1[i];
    }
    for (int i = 0; str2[i] != '\0'; i++) {
        new_string[ind++] = str2[i];
    }
    new_string[ind] = '\0';
    
    return new_string;
}

_Bool tab_highlighted = 0;

_Bool str_eq(char *str1, char *str2);

void highlight_buttons(){
    for (int i=0;i<textarea->buttons_index;i++){
        struct button* btn=textarea->buttons[i];
        if (mouse_intersects(btn->x,btn->y,btn->width,btn->height)){
            
            highlight_button(btn);
            
        }else{
            
            unhighlight_button(btn);
            
        }
    }

    for (int i=0;i<textbox->buttons_index&&textbox!=NULL;i++){
        struct button* btn=textbox->buttons[i];

        if (mouse_intersects(btn->x,btn->y,btn->width,btn->height)){
            highlight_button(btn);
        }
        else{
            unhighlight_button(btn);
        }
    }

    for (int i=0;i<textarea->scroll_index;i++){
        for (int j=0;j<textarea->scroll_areas[i]->buttons_index;j++){
            ScrollButton* btn=textarea->scroll_areas[i]->buttons[j];

            if (btn->row < textarea->scroll_areas[i]->scroll_bounds || btn->row>(textarea->scroll_areas[i]->scroll_bounds+(textarea->scroll_areas[i]->height/18)) || btn->col<0 || btn->col > (textarea->scroll_areas[i]->width/10))
                continue;

            int finalx=textarea->scroll_areas[i]->x+btn->col*10;
            int finaly=textarea->scroll_areas[i]->y+(btn->row - textarea->scroll_areas[i]->scroll_bounds)*18;


            btn->button->x=finalx;
            btn->button->y=finaly;

            if (finaly<0 || finaly>(textarea->scroll_areas[i]->y + textarea->scroll_areas[i]->height))
                continue; 

            if (mouse_intersects(finalx,finaly,btn->button->width,btn->button->height)){
                
                highlight_button(btn->button);
            }else{
                unhighlight_button(btn->button);

            }
        }
    }
}

void WINDOW_MANAGER(){
    draw_rect(0,50,1024,768-50,textarea->bg);

    for (int i=0;i<textarea->text_slabs_index;i++){
        draw_text_slab(textarea->text_slabs[i]);
    }

    refresh_text();

    for (int i=0;i<textarea->buttons_index;i++){
        draw_button(textarea->buttons[i]);
    }

    if (textbox!=NULL){
        draw_text_box(textbox);
    }


    //todo


}





void hover_mouse() {
    //highlight_buttons();
    hovering=true;
}

typedef enum{
    HOME,
    CMINUS,
    TEXTPAD,
    FILES

}PROGRAM_TYPE;

PROGRAM_TYPE program=HOME;

#define PENDING_BUTTONS_CAP 32
struct button* pending_buttons[PENDING_BUTTONS_CAP];
int pending_buttons_index = 0;

void open_program(char* filename);

void process_temp_mem(){
    for (int i=0;i<TEMP_MEM_INDEX;i++){
        FREE(TEMP_MEM[i]);
    }
    TEMP_MEM_INDEX=0;
}

void open_image(char* filename);

void process_pending_buttons(void) {
    while (pending_buttons_index > 0) {
        struct button* b = pending_buttons[--pending_buttons_index];
        
        if (b == NULL) 
            continue;
        
        if ((b->on_click!=NULL||b->on_click_arg!=NULL)&&!b->is_file&&!b->is_image&&!b->is_program) {
            restore_mouse();

            if (b->arg1!=NULL){
                //write_rowcol_text(11,10,b->arg1,GREEN);
                //write_rowcol_text(10,30,str_cat("Processing pending button -- ",b->text),RED);

                b->on_click_arg(b->arg1);
            }
            else{
                //__asm__ __volatile__("sti");
                b->on_click();
                running_program=false;
            }

            save_mouse();
        } else if (b->is_file) {
            restore_mouse();
            open_file(b->text);
            save_mouse();
        }
        else if(b->is_program){
            open_program(b->text);
            running_program=false;
        }
        else if (b->is_image){
            open_image(b->text);

        }

    }
}

void right_click_mouse(){
    if (drawarea!=NULL){
        drawarea->colour=framebuffer[mouse_y*ppr+mouse_x];

    }

}

void left_click_mouse() {
    //restore_20x20_background();
    
    for (int i = 0; i < textarea->buttons_index; i++) {
        struct button* btn2 = textarea->buttons[i];
        if (mouse_intersects(btn2->x,btn2->y,btn2->width,btn2->height)) {
            pending_buttons[pending_buttons_index++] = btn2;
        }
    }

    if (textbox){
        for (int i=0;i<textbox->buttons_index;i++){
            struct button* btn=textbox->buttons[i];
            if (mouse_intersects(btn->x,btn->y,btn->width,btn->height)){
                pending_buttons[pending_buttons_index++]=btn;
            }
        }
    }

    for (int i=0;i<textarea->scroll_index;i++){
        for (int j=0;j<textarea->scroll_areas[i]->buttons_index;j++){
            ScrollButton* btn=textarea->scroll_areas[i]->buttons[j];

            if (btn->row < textarea->scroll_areas[i]->scroll_bounds || btn->row > (textarea->scroll_areas[i]->scroll_bounds+(textarea->scroll_areas[i]->height/18)) || btn->col<0 || btn->col > (textarea->scroll_areas[i]->width/10))
                continue;

            int finalx=textarea->scroll_areas[i]->x+btn->col*10;
            int finaly=textarea->scroll_areas[i]->y+(btn->row - textarea->scroll_areas[i]->scroll_bounds) * 18;

            if (mouse_intersects(finalx,finaly,btn->button->width,btn->button->height)){
                pending_buttons[pending_buttons_index++]=btn->button;
            }

        }
    }

    
    //draw_mouse();

    
}

void textareabuf_str(){
    add_char_to_char_buf('\0');

}

int str_hex_to_int(char* str){
    str+=2;
    int num=0;
    for (int i=0;str[i]!='\0';i++){
        uint8_t dig=str[i];
        char c='A';
        _Bool is_alpha=false;
        for (int i=0;i<6;i++,c++){
            int val=i+10;
            if (dig==c || dig==(c+abs('A'-'a'))){
                num*=16;
                num+=val;
                is_alpha=true;
                break;
            }

        }
        if (is_alpha)continue;
        num*=16;
        num+=dig-'0';
        
    }
    return num;

}

int str_int(char *str) {

    if (str[0]=='0' && (str[1]=='x' || str[1]=='X'))
        return str_hex_to_int(str);
    
    if (!str || str[0] == '\0') return 0;
    int sign = 1;
    int i = 0;
    if (str[0] == '-') { sign = -1; i = 1; }
    else if (str[0] == '+') { i = 1; }

    int num = 0;
    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            num = num * 10 + (str[i] - '0');
        } else {
            break;
        }
    }
    return sign * num;
}

void char_str(char c, char* buf){
    buf[0]=c;
    buf[1]=0;


}

void int_str(int value, char *buf) {
    int i = 0;
    _Bool is_negative = false;

    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {
        buf[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }
}

char* quick_int_str(int value){
    char buf[32];

    int_str(value,buf);

    return str_copy(buf);

}

char *str_cat(char *str1, char *str2) {
    int sz1 = 0;
    int sz2 = 0;

    for (int i = 0; str1[i] != '\0'; i++)sz1++;
    for (int i = 0; str2[i] != '\0'; i++)sz2++;

    char *retarr = MALLOC(sz1 + sz2 + 1);
    int ind = 0;

    for (int i = 0; i < sz1; i++)retarr[ind++] = str1[i];
    for (int i = 0; i < sz2; i++)retarr[ind++] = str2[i];

    retarr[ind] = '\0';
    

    return retarr;
}

int str_contains(char *haystack, char *needle) {
    if (!haystack || !needle) return 0;
    while (*haystack) {
        char *h = haystack;
        char *n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (*n == '\0') return 1;
        haystack++;
    }
    return 0;
}

void fill_array(char* array, int capacity,char val){
    for (int i=0;i<capacity;i++){
        array[i]=val;
    }
}

_Bool str_eq(char *str1, char *str2) {
    int i = 0;

    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i])return false;
        i++;
    }
    return str1[i] == '\0' && str2[i] == '\0';
}

void draw_mouse(void) {
    
    restore_20x20_background();

    save_20x20_background();
    
    mouse_drawing_enabled = true;

    int dcx = mouse_x;
    int dcy = mouse_y;
    
    dcx++;
    for (int i = 0; i < 12; i++) {
        
        draw_pixel(dcx, dcy++, 0);
        
    }
    dcx--;
    draw_pixel(++dcx, --dcy, 0);
    draw_pixel(++dcx, --dcy, 0);

    for (int i = 1; i <= 5; i++) {
        draw_pixel(++dcx, dcy, 0);
    }

    dcx -= 2;

    for (int i = 0; i < 6; i++) {
        if (i % 2 == 0 && i != 0) {
            dcx++;
        }
        draw_pixel(dcx, ++dcy, 0);
    }
    draw_pixel(++dcx, ++dcy, 0);
    draw_pixel(++dcx, dcy, 0);
    
    dcx++;
    dcy--;

    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0 && i != 0) {
            dcx--;
        }
        draw_pixel(dcx, dcy--, 0);
    }
    for (int i = 0; i < 3; i++) {
        draw_pixel(++dcx, dcy, 0);
    }
    draw_pixel(++dcx, ++dcy, 0);

    for (int i = 0; i < 10; i++) {
        draw_pixel(dcx--, dcy--, 0);
    }

    //fill

    dcx = mouse_x+2;
    dcy = mouse_y+1;

    for (int i = 0; i < 9; i++) {
        draw_pixel(dcx, dcy++, get_complementary_colour(COLOUR));
    }
    dcx++;
    dcy--;
    for (int i = 0; i < 8; i++) {
        draw_pixel(dcx, dcy--, get_complementary_colour(COLOUR));
    }
    dcx++;
    dcy+=2;
    for (int i = 0; i < 7; i++) {
        draw_pixel(dcx, dcy++, get_complementary_colour(COLOUR));
    }
    
    dcx++;
    dcy--;
    for (int i = 0; i < 6; i++) {
        draw_pixel(dcx, dcy--, get_complementary_colour(COLOUR));
    }
    dcx++;
    dcy+=2;
    for (int i = 0; i < 5; i++) {
        draw_pixel(dcx, dcy++, get_complementary_colour(COLOUR));
    }
    dcx++;
    dcy-=2;
    for (int i = 0; i < 3; i++) {
        draw_pixel(dcx, dcy--, get_complementary_colour(COLOUR));
    }
    dcx++;
    dcy += 2;
    draw_pixel(dcx, dcy, get_complementary_colour(COLOUR));
    dcx -= 2;
    dcy += 4;
    for (int i = 0; i < 6; i++) {
        if (i % 2 == 0 && i != 0) {
            dcx++;
        }
        draw_pixel(dcx, dcy++, get_complementary_colour(COLOUR));
    }
    dcy--;
    dcx++;
    for (int i = 0; i < 6; i++) {
        if (i % 2 == 0 && i != 0) {
            dcx--;
        }
        draw_pixel(dcx, dcy--,get_complementary_colour(COLOUR));
    }
}

_Bool type_textbox(uint8_t* character){
    if ((textbox->cursorx>=(textbox->width/10)-2))
        return false;
    if (textbox->cursory<0 || textbox->cursory>textbox->MAX)return;
    restore_mouse();


    int xval=textbox->x+5+10*textbox->cursorx;
    int y=textbox->y+30+18*textbox->cursory;

    textbox->cursorx++;

    

    for (int i = 0; i < 16; i++) {
        unsigned char row = character[i];
        
        for (int j = 0; j < 8; j++) {//row=0b00011000
            if (row & 0x80) {
                draw_pixel(xval+j, y, textbox->fg);
            }
            row = row << 1;
        }
        y++;

    }
    //unhighlight_char();

    save_mouse();
    
    return true;

}

_Bool type_char(uint8_t* character) {
    if(textbox!=NULL)return type_textbox(character);

    if (textarea->cursorx>=101)return false;


    restore_mouse();


    int xval = textarea->cursorx*10;
    int y=(textarea->cursory+3)*18;
    destroy_char();

    for (int i = 0; i < 16; i++) {
        unsigned char row = character[i];
        
        for (int j = 0; j < 8; j++) {//row=0b00011000
            if (row & 0x80) {
                draw_pixel(xval+j, y, textarea->fg);
            }
            row <<=1;
        }
        y++;

    }
    //unhighlight_char();

    save_mouse();

    
    textarea->cursorx++;

    

    return true;
    
}

__asm__(
    ".global disk_handler_asm\n"
    "disk_handler_asm:\n"
    "   pushal\n"
    "   call disk_handler\n"
    "   popal\n"
    "   iretl\n"
);

extern void disk_handler_asm(void);

__asm__ (
    ".global timer_handler_asm\n"
    "timer_handler_asm:\n"
    "   pushal\n"
    "   call timer_handler\n"
    "   popal\n"
    "   iretl\n"
);
extern void timer_handler_asm(void);

__asm__(
    ".global keyboard_handler_asm\n"
    "keyboard_handler_asm:\n"
    "   pushal\n"
    "   call keyboard_handler\n"
    "   popal\n"
    "   iretl\n"
);

extern void keyboard_handler_asm(void);

__asm__(
    ".global default_handler_asm\n"
    "default_handler_asm:\n"
    "   pushal\n"
    "   call default_handler\n"
    "   popal\n"
    "   iretl\n"
);

extern void default_handler_asm(void);

__asm__(
    ".global mouse_handler_asm\n"
    "mouse_handler_asm:\n"
    "   pushal\n"
    "   call mouse_handler\n"
    "   popal\n"
    "   iretl\n"

);

extern void mouse_handler_asm(void);

__asm__(
    
    ".global syscall_handler_asm\n"
    "syscall_handler_asm:\n"
    "   pushal\n"
    "   call syscall_handler\n"
    "   popal\n"
    "   iretl\n"

);
extern void syscall_handler_asm(void);


_Bool is_numeric(char c) {
    if (c=='0'||c=='1'||c=='2'||c=='3'||c=='4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9'){
        return 1;
    }
    return 0;
}

uint32_t address_offset;

char* str_copy_input(char* str){
    char* ptr=(char*)STRINGS_ADDRESS+address_offset;
    address_offset+=len(str)+1;

    int i;
    for (i=0;str[i]!='\0';i++){
        ptr[i]=str[i];
    }
    ptr[i]='\0';

    return ptr;



}



void write_time(char character, int x) {
    draw_rect(x * 10, 5, 8, 16, TAB_COLOUR);
    write_char(x * 10, 5, font8x16[character], COLOUR);
}

char *str_copy(char *str);


int yy=10;

void debug(){

    int val;
    __asm__ __volatile__("" : "=D"(val));

    char vb[32];

    int_str(val,vb);

    write_rowcol_text(yy++,4,str_cat("[esp]= ",vb),BLACK);
}

void add_char_to_console(char c){
    if (console_index>=console_capacity-1){
        
        console_capacity*=2;
        char* re=MALLOC(console_capacity);
        for (int i=0;i<console_index;i++){
            re[i]=console_buf[i];
        }
        FREE(console_buf);
        console_buf=re;
    }
    console_buf[console_index++]=c;

}

void add_text_to_console(char* text){
    for (int i=0;text[i]!='\0';i++){
        add_char_to_console(text[i]);
    }
    add_char_to_console(' ');
}

int yyyyy=0;
void OS_PRINT(){
    //write_rowcol_text(30,30,"iehfiuehugheirhgijehgijehg",RED);
    //console_buf=MALLOC()

    uint32_t addr;
    char type;

    __asm__ __volatile__("" : "=D"(addr));
    __asm__ __volatile__("" : "=S"(type));

   
    
    uint8_t* ptr=(uint8_t*)addr;

    //write_rowcol_text(30,30,ptr,RED);
    for (int i=0;ptr[i]!='\0';i++){
        add_char_to_console(ptr[i]);

    }

    if (type==1){
        add_char_to_console('\n');
    }
    
    console_buf[console_index] = '\0';

}



void OS_DRAW(void){
    int type;

    __asm__ __volatile__("": "=D"(type));



}
int* INPUT_VALUE=0x353535;
char INPUT_TYPE;

void SUBMIT_INPUT(){
    if (INPUT_TYPE==0){
        *INPUT_VALUE=str_int(textbox->buffer);
    }
    else{
        *INPUT_VALUE=str_copy_input(textbox->buffer);
        
    }

    close_text_box();


}

void INPUT_HANG(){
    __asm__ __volatile__("sti");
    while (textbox!=NULL){
        if (hovering){
            hovering=0;
            highlight_buttons();
        }
        if (should_blink){
            should_blink=0;
            blink_cursor();
            read_time();
        }
        if (quarter_second){
            quarter_second=false;
        }
        
        if (mouse_needs_redrawn&&mouse_drawing_enabled){
            draw_mouse();
            mouse_needs_redrawn=false;
        }
        
        process_pending_buttons();
        __asm__ __volatile__("hlt");


    }

}

void OS_INPUT(void){

    __asm__ __volatile__("" : "=D"(INPUT_TYPE));
    char* title;
    __asm__ __volatile__ ("" : "=S"(title));

    create_text_box(200,400,len(title)*10+200,200,textarea->bg,textarea->fg,title,false);

    //pop_and_delete_button();

    create_textbox_button(GREEN, BLACK, YELLOW, "Submit", SUBMIT_INPUT,NULL);

    running_program=false;
    
    INPUT_HANG();
    

    running_program=true;
    

}


/////////////cmcompilerstart//////////////////////////////////////


struct ASTNode *parse_identifier_increment();

struct ASTNode *parse_expression();

struct ASTNode *parse_statement();

struct ASTNode *parse_parameter();



uint64_t token_index = 0;
uint64_t token_count;

uint64_t nodes_index = 0;

typedef enum{
    SYNTAX_ERROR,
    VARIABLE_ERROR,
    TYPE_ERROR
    
}ErrorType;

struct Error {
    char* type;
    char* message;

    uint32_t line_num;
};




uint64_t token_capacity;
uint64_t node_capacity;


uint64_t asm_count;
uint64_t asm_capacity;

uint32_t global_address = BASE_ADDRESS;//important
uint32_t program_heap=PROGRAM_HEAP;

typedef struct {
    uint8_t* bytes;
    int size;
    int capacity;

}code_buffer;

int byte_count;
int byte_capacity=256;

struct x86_data_instruction {
    uint32_t address;

    char *value;
    int size;
    char *label;
};

struct helper_function {
    uint32_t address;
    char* name;

    struct x86_instruction** instructions;

};

struct helper_function** helpers;
int helpers_count=0;
int helpers_size=64;

uint32_t address_offset = 0;

struct x86_data_instruction **data_instructions;

int data_count = 0;
int data_capacity = 100;

typedef enum {
    OPERAND_REGISTER,
    OPERAND_IMMEDIATE,
    OPERAND_MEMORY,
    OPERAND_MEMORY_DIRECT,
    OPERAND_LABEL,
    OPERAND_NONE
} OperandType;

typedef struct {
    OperandType type;

    union {
        int reg;           // register number
        int imm;       // immediate value
        struct {
            int base_reg;
            int index_reg;
            int scale;
            int disp;
        } mem;
        uint32_t addr;
    };
}Operand;


Operand parse_operand(const char *str);

struct x86_instruction {
    char *label;

    char *mnemonic;
    char *operand1;
    char *operand2;
    char* operand3;


    uint64_t address;
};

struct x86_instruction **instructions;
struct SymbolTable;

struct patch_point{
    uint32_t address;
    int asm_point;

};

struct ASTNode {
    char *type;
    char *value;
    char* fraction;

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode* data_type;

    char *identifier;
    char *var_type;
    char* class_name;

    struct ASTNode *param1;
    struct ASTNode *param2;
    struct ASTNode *param3;
    struct ASTNode *param4;

    struct ASTNode *condition;

    struct ASTNode **children;

    struct ASTNode **elements;
    struct ASTNode **array_sizes;
    uint32_t array_dimensions;
    uint32_t element_count;

    struct ASTNode **indexes;


    struct ASTNode *fordeclaration;
    struct ASTNode *forcondition;
    struct ASTNode *forincrement;

    struct SymbolTable *scope;

    struct ASTNode* operation;

    char *op;

    char *return_type;

    struct ASTNode **params;
    int param_count;


    struct ASTNode* mnemonic;
    struct ASTNode* operand1;
    struct ASTNode* operand2;
};

struct Token {
    char *type;
    char *value;
};


struct Symbol {
    char *name;
    char *type;
    char* struct_name;
    _Bool is_global;
    uint32_t addr;

    uint64_t size;
    uint32_t *dimensions;
    uint32_t num_dimensions;
    int stack_offset;
    struct Symbol *params;
    struct SymbolTable *child_table;
    uint32_t element_size;

    int locals_offset;
    uint32_t func_address;
    int param_count;

    struct Symbol** fields;

    

    
};

struct SymbolTable {
    struct Symbol *symbols;
    struct SymbolTable *parent;
    uint64_t capacity;
    uint64_t count;
    int start_address;
    int end_address;
};


_Bool in_function=false;

struct SymbolTable* function_table=NULL;
struct Symbol* current_function=NULL;


struct Token *tokens;
struct ASTNode **nodes;

char *evaluate_string(struct ASTNode *node);

char *find_identifier_value(char *identifier);

int evaluate_const_expr(struct ASTNode *node);

void generate_call(struct ASTNode* node,struct SymbolTable* table);

void write_program_to_memory(code_buffer* buf,uint32_t address);

void assemble_instructions(struct x86_instruction** instrs,code_buffer* buf);


char* get_v_addr(char* id, struct SymbolTable* table);

struct ASTNode *parse_bitwise_or();


_Bool is_float=false;


int evaluate_variable_expr(struct ASTNode *node) {
    //printf("Node->type in evalvarexpr=%s\n", node->type);
    if (str_eq(node->type, "NUMBER")) {
        return str_int(node->value);
    }
    if (str_eq(node->type, "IDENTIFIER")) {
        return str_int(find_identifier_value(node->identifier));
    }
    if (str_eq(node->type, "char")) {
        return str_int(node->value);
    }
    if (str_eq(node->type,"string")) {
        //error
    }

    if (str_eq(node->type, "PLUS")) {
        return evaluate_variable_expr(node->left) + evaluate_variable_expr(node->right);
    }
    if (str_eq(node->type, "MINUS")) {
        return evaluate_variable_expr(node->left) - evaluate_variable_expr(node->right);
    }
    if (str_eq(node->type, "MULTIPLY")) {
        return evaluate_variable_expr(node->left) * evaluate_variable_expr(node->right);
    }
    if (str_eq(node->type, "DIVIDE")) {
        return evaluate_variable_expr(node->left) / evaluate_variable_expr(node->right);
    }
    return 0;
    //error
}

int evaluate_bool_expr(struct ASTNode *node) {
    if (str_eq(node->type, "NUMBER")) {
        return str_int(node->value);
    }
    if (str_eq(node->type, "BOOLEAN")) {
        return str_eq(node->value, "true") ? 1 : 0;
    }
    if (str_eq(node->type, "char")) {
        return str_int(node->value);
    }
    if (str_eq(node->type, "IDENTIFIER")) {
        return str_int(find_identifier_value(node->identifier));
    }
    if (str_eq(node->type, "PLUS")) {
        return evaluate_const_expr(node->left) + evaluate_const_expr(node->right);
    }
    if (str_eq(node->type, "MINUS")) {
        return evaluate_const_expr(node->left) - evaluate_const_expr(node->right);
    }
    if (str_eq(node->type, "MULTIPLY")) {
        return evaluate_const_expr(node->left) * evaluate_const_expr(node->right);
    }
    if (str_eq(node->type, "DIVIDE")) {
        return evaluate_const_expr(node->left) / evaluate_const_expr(node->right);
    }

    if (str_eq(node->type, ">")) {
        return evaluate_bool_expr(node->left) > evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, "<")) {
        return evaluate_bool_expr(node->left) < evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, "<=")) {
        return evaluate_bool_expr(node->left) <= evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, ">=")) {
        return evaluate_bool_expr(node->left) >= evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, "!=")) {
        return evaluate_bool_expr(node->left) != evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, "==")) {
        return evaluate_bool_expr(node->left) == evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, "&&")) {
        return evaluate_bool_expr(node->left) && evaluate_bool_expr(node->right);
    }
    if (str_eq(node->type, "||")) {
        return evaluate_bool_expr(node->left) || evaluate_bool_expr(node->right);
    }
    //error
}

int evaluate_const_expr(struct ASTNode *node) {
    if (str_eq(node->type, "NUMBER")) {
        return str_int(node->value);
    }
    if (str_eq(node->type, "char")) {
        return str_int(node->value);
    }

    if (str_eq(node->type, "string")) {
        //error
    }

    if (str_eq(node->type, "MULTIPLY")) {
        return evaluate_const_expr(node->left) * evaluate_const_expr(node->right);
    }
    if (str_eq(node->type, "PLUS")) {
        return evaluate_const_expr(node->left) + evaluate_const_expr(node->right);
    }
    if (str_eq(node->type, "MINUS")) {
        return evaluate_const_expr(node->left) - evaluate_const_expr(node->right);
    }
    if (str_eq(node->type, "DIVIDE")) {
        return evaluate_const_expr(node->left) / evaluate_const_expr(node->right);
    }
    return 0;
}

struct SymbolTable *create_symbol_table(struct SymbolTable *parent) {
    struct SymbolTable *table = MALLOC(sizeof(struct SymbolTable));
    table->capacity = 100;
    table->count = 0;
    table->symbols = MALLOC(table->capacity * sizeof(struct Symbol));
    table->parent = parent;
    return table;
}

void symbol_table_insert(struct SymbolTable *table, struct Symbol symbol) {
    if (table->count == table->capacity) {
        table->capacity *= 2;
        struct Symbol *new_symbols = MALLOC(table->capacity * sizeof(struct Symbol));

        for (uint64_t i = 0; i < table->count; i++) {
            new_symbols[i] = table->symbols[i];
        }
        table->symbols = new_symbols;
    }

    table->symbols[table->count++] = symbol;
}

struct Symbol *symbol_table_lookup_local(struct SymbolTable *table, char *name) {
    for (uint64_t i = 0; i < table->count; i++) {
        if (str_eq(table->symbols[i].name, name)) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

void count_array_dimensions(struct ASTNode* node, uint32_t* dim, int depth) {
    //printf("IN COUNT ARRAY DIM\n");
    if (node->elements == NULL)return;

    int count=0;
    for (int i=0;node->elements[i]!=NULL;i++) {
        count++;
    }

    dim[depth] = count;

    if (node->elements[0]!=NULL&&node->elements[0]->elements!=NULL) {
        count_array_dimensions(node->elements[0], dim, depth+1);
    }
}


struct Symbol *symbol_table_lookup(struct SymbolTable *table, char *name) {
    //printf("IN SYMBOL TABLE LOOKUP\n");

    struct Symbol *symbol = symbol_table_lookup_local(table, name);

    //printf("looked up %s\n",symbol->name);

    if (symbol != NULL) {
        //printf("RETURNING %s\n",symbol->name);

        return symbol;
    }

    if (table->parent != NULL)return symbol_table_lookup(table->parent, name);

    return NULL;
}

uint32_t program_alloc(int amount){
    uint32_t addr=program_heap;
    program_heap+=amount;
    return addr;
    

}

int total_allocated_size=4;
int total_array_size=0;

//int tem=10;

void build_symbol_table(struct ASTNode *node, struct SymbolTable *table, int *offset) {
    if (node == NULL)return;

    if (str_eq(node->type, "int_decl")) {
        struct Symbol symbol;
        symbol.name = node->identifier;
        symbol.type = "int";
        symbol.size = 4;
        symbol.element_size=0;
        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(4);
        else{
            symbol.stack_offset = -(*offset);
            *offset += 4;
            total_allocated_size+=4;
        }

        symbol_table_insert(table, symbol);
        
    } 
    else if (str_eq(node->type, "char_decl")) {
        struct Symbol symbol;
        symbol.type = "char";
        symbol.size = 1;
        symbol.element_size=0;
        symbol.name = node->identifier;
        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(1);
        else{
            symbol.stack_offset = -(*offset);
            *offset += 4;
            total_allocated_size+=4;
        }

        symbol_table_insert(table, symbol);
    } else if (str_eq(node->type, "bool_decl")) {
        struct Symbol symbol;
        symbol.name = node->identifier;
        symbol.type = "bool";
        symbol.size = 1;
        symbol.element_size=0;
        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(1);
        else{
            symbol.stack_offset = -(*offset);
            *offset += 4;
            total_allocated_size+=4;
        }
        symbol_table_insert(table, symbol);

    } 
    else if(str_eq(node->type,"float")){
        struct Symbol symbol;
        symbol.type="float";
        symbol.name=node->identifier;
        symbol.size=4;
        symbol.element_size=0;
        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(4);
        else{
            symbol.stack_offset = -(*offset);
            *offset += 4;
            total_allocated_size+=4;
        }
        symbol_table_insert(table,symbol);

    }
    else if (str_eq(node->type, "for")) {
        struct SymbolTable *for_table = create_symbol_table(table);

        node->scope = for_table;

        struct Symbol for_symbol;
        for_symbol.name = node->fordeclaration->identifier;
        for_symbol.type = "int";
        for_symbol.size = 4;
        for_symbol.element_size=0;
        for_symbol.is_global=!in_function;

        if (for_symbol.is_global)
            for_symbol.addr=program_alloc(4);
        else{
            for_symbol.stack_offset = -(*offset);
            *offset += 4;
            total_allocated_size+=4;
        }

        symbol_table_insert(for_table, for_symbol);

        for (int64_t i = 0; node->children[i] != NULL; i++) {
            build_symbol_table(node->children[i], for_table, offset);
        }

        return;
    } 
    else if(str_eq(node->type,"fori")){
        struct SymbolTable* fori_table=create_symbol_table(table);

        node->scope=fori_table;

        struct Symbol for_symbol;
        for_symbol.name = "i";
        for_symbol.type = "int";
        for_symbol.size = 4;
        for_symbol.is_global=!in_function;

        if (for_symbol.is_global)
            for_symbol.addr=program_alloc(4);
        else{
            for_symbol.stack_offset = -(*offset);
            *offset += 4;
            total_allocated_size+=4;
        }

        symbol_table_insert(fori_table, for_symbol);

        for (int64_t i = 0; node->children[i] != NULL; i++) {
            build_symbol_table(node->children[i], fori_table, offset);
        }

        return;

        

    }
    
    else if (str_eq(node->type, "if") || str_eq(node->type, "elif") || str_eq(node->type, "while") || str_eq(
                   node->type, "else")) {
        struct SymbolTable *loop_table = create_symbol_table(table);

        node->scope = loop_table;

        for (int64_t i = 0; node->children[i] != NULL; i++) {
            build_symbol_table(node->children[i], loop_table, offset);
        }

        return;
    } 
    else if (str_eq(node->type, "int_arr_decl") || str_eq(node->type, "str_arr_decl") ||
               str_eq(node->type, "bool_arr_decl") || str_eq(node->type, "char_arr_decl")) {
        struct Symbol symbol;

        //printf("BUILDING ARRAY SYMBOL");

        symbol.name = node->identifier;

        if (str_eq(node->type, "int_arr_decl")){
            symbol.type = "int_arr";
            symbol.element_size=4;
        }
        if (str_eq(node->type, "char_arr_decl")){
            symbol.type = "char_arr";
            symbol.element_size=1;
        }
        if (str_eq(node->type, "str_arr_decl")){
            symbol.type = "str_arr";
            symbol.element_size=4;
        }
        if (str_eq(node->type, "bool_arr_decl")){
            symbol.type = "bool_arr";
            symbol.element_size=1;
        }else{

        }

        symbol.num_dimensions = node->array_dimensions;

        int size = 1;

        if (node->array_sizes[0] != NULL) {
            symbol.dimensions = MALLOC(sizeof(uint32_t) * node->array_dimensions);

            for (int64_t i = 0; node->array_sizes[i] != NULL; i++) {
                int dim = evaluate_variable_expr(node->array_sizes[i]);
                //printf("\nDEBUG: evaluated as: %d\n", dim);
                symbol.dimensions[i] = dim;
                size *= dim;
            }
            size *= symbol.element_size;
        } else if (node->right != NULL) {
            symbol.dimensions=MALLOC(sizeof(uint32_t) * node->array_dimensions);
            symbol.num_dimensions = node->array_dimensions;


            count_array_dimensions(node->right,symbol.dimensions,0);

            size=1;
            for (int i=0;i<symbol.dimensions[i];i++) {
                size*=(int)symbol.dimensions[i];
            }
            size*=symbol.element_size;


        }
        else {

        }

        symbol.size = size;

        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(symbol.size);
        else{
            symbol.stack_offset = -(*offset);
            *offset += symbol.size;
            total_allocated_size+=symbol.size;
        }

        symbol_table_insert(table, symbol);

    } 
    else if(str_eq(node->type,"def")){
        struct Symbol symbol;

        symbol.name=node->identifier;

        symbol.type="struct";

        symbol.fields=MALLOC(node->array_dimensions*sizeof(struct Symbol*));

        int size=0;
        int struct_offset=0;
        int i;
        for(i=0;i<node->array_dimensions;i++){
            struct Symbol* field=MALLOC(sizeof(struct Symbol));
            field->name=node->children[i]->identifier;

            field->type=node->children[i]->type;

            field->stack_offset=struct_offset;
            //write_rowcol_text(tem++,5,quick_int_str(struct_offset),ORANGE);

            field->size=4;

            if (str_contains(field->type,"_ptr")){ //is a ptr
                if (str_contains(field->type,"char")||str_contains(field->type,"bool"))
                    field->element_size=1;

                else {
                    if (str_eq(field->type,"struct_ptr")){
                        field->struct_name=node->children[i]->value;
                        field->element_size=str_eq(field->struct_name,symbol.name)?-1:symbol_table_lookup(table,field->struct_name)->size; //check if self referential patch later

                    }
                    else{
                        field->element_size=4;
                    }
                }

                field->num_dimensions=node->children[i]->array_dimensions;
            }
            else if(str_contains(field->type,"_arr")){ //is an arr
                field->dimensions = MALLOC(sizeof(uint32_t) * node->children[i]->array_dimensions);
                int sz = 1;

                int element_size;

                if (str_eq(field->type,"struct_arr")){
                    field->struct_name=node->children[i]->value;
                    element_size=symbol_table_lookup(table,field->struct_name)->size;
                }
                else{
                    element_size=4;
                }
                field->element_size=element_size;

                for (int j = 0; node->children[i]->array_sizes[j] != NULL; j++){
                    int dim = evaluate_variable_expr(node->children[i]->array_sizes[j]);
                    field->dimensions[j] = dim;

                    sz *= dim;
                }
                sz*=field->element_size;

                field->size = sz;
                field->num_dimensions=node->children[i]->array_dimensions;
            } 
            else if(str_eq(field->type,"struct")){
                field->size=symbol_table_lookup(table,node->children[i]->value)->size;
                field->struct_name=node->children[i]->value;
            }     

            symbol.fields[i]=field;

            size+=4;
            struct_offset+=4;
        }

        symbol.size=size;
        symbol.num_dimensions=i;

        for (int j=0;j<i;j++){ //patch self referential struct ptrs with calculated sz
            if (symbol.fields[j]->element_size==-1 && str_eq(symbol.fields[i]->struct_name,symbol.name)){
                symbol.fields[j]->element_size=symbol.size;
            }
        }

        //total_allocated_size+=symbol.size;

        symbol_table_insert(table,symbol);

    }
    else if(str_eq(node->type,"struct")){
        struct Symbol symbol;

        symbol.type="struct";
        symbol.struct_name=node->value;
        symbol.name=node->identifier;

        symbol.size=symbol_table_lookup(table,symbol.struct_name)->size;
        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(symbol.size);
        else{
            symbol.stack_offset = -(*offset);
            *offset += symbol.size;
            total_allocated_size+=symbol.size;
        }

        symbol_table_insert(table,symbol);

    }
    else if(str_eq(node->type,"struct_arr")){
        struct Symbol symbol;

        symbol.type="struct_arr";
        symbol.name=node->identifier;
        symbol.num_dimensions=node->array_dimensions;
        symbol.struct_name=node->value;
        symbol.stack_offset=-(*offset);

        int struct_sz=symbol_table_lookup(table,symbol.struct_name)->size;
        symbol.element_size=struct_sz;
        
        int sz=1;
        symbol.dimensions=MALLOC(sizeof(uint32_t)*node->array_dimensions);

        for (int i=0;node->array_sizes[i]!=NULL;i++){
            int dim=evaluate_variable_expr(node->array_sizes[i]);
            symbol.dimensions[i]=dim;
            sz*=dim;
        }
        sz*=struct_sz;
        symbol.size=sz;
        

        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(symbol.size);
        else{
            symbol.stack_offset = -(*offset);
            *offset += symbol.size;
            total_allocated_size+=symbol.size;
        }

        symbol_table_insert(table,symbol);

    }
    else if(str_eq(node->type,"struct_ptr")){
        struct Symbol symbol;

        symbol.type="struct_ptr";
        symbol.name=node->identifier;
        symbol.num_dimensions=node->array_dimensions;
        symbol.struct_name=node->value;
        symbol.stack_offset=-(*offset);

        symbol.size=4;
        symbol.element_size=symbol_table_lookup(table,symbol.struct_name)->size;
        
        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(symbol.size);
        else{
            symbol.stack_offset = -(*offset);
            *offset += symbol.size;
            total_allocated_size+=symbol.size;
        }

        symbol_table_insert(table,symbol);

    }
    else if(str_contains(node->type, "ptr_decl")){
        struct Symbol symbol;

        if (str_eq(node->type, "int_ptr_decl")){
            symbol.type = "int_ptr";
            symbol.element_size=4;
        }
        else if (str_eq(node->type, "char_ptr_decl")){
            symbol.type = "char_ptr";
            symbol.element_size=1;
        }
        else if (str_eq(node->type, "str_ptr_decl")){
            symbol.type = "str_ptr";
            symbol.element_size=4;
        }
        else{

        }

        symbol.name=node->identifier;
        symbol.num_dimensions=node->array_dimensions;
        symbol.stack_offset=-(*offset);
        symbol.size=4;

        symbol.is_global=!in_function;

        if (symbol.is_global)
            symbol.addr=program_alloc(symbol.size);
        else{
            symbol.stack_offset = -(*offset);
            *offset += symbol.size;
            total_allocated_size+=symbol.size;
        }

        symbol_table_insert(table,symbol);

    }
    
    
    else if (str_eq(node->type, "function")) {
        struct Symbol symbol;
        symbol.name = node->identifier;
        //symbol.type = node->return_type;
        symbol.stack_offset = 0;
        symbol.type="function";
        symbol.param_count=0;
        symbol.size=0;

        in_function=true;

        struct SymbolTable *func_table = create_symbol_table(table);

        int params_offset = 8; // return address is at [EBP+4], first param at [EBP+8]

        if (node->params) {
            for (int64_t i = 0; i<node->param_count; i++) {
                struct Symbol param;
                symbol.param_count++;
                param.name = node->params[i]->identifier;
                param.type = node->params[i]->data_type->type;
                param.element_size=-1;
                //param.element_size=1;

                if (str_contains(param.type,"ptr")){
                    param.num_dimensions=node->params[i]->data_type->array_dimensions;
                    //write_rowcol_text(32,0,str_cat("in build=",quick_int_str(node->params[i]->array_dimensions)),ORANGE);

                    if (str_contains(param.type,"char")||str_contains(param.type,"bool")){
                        //write_rowcol_text(30+params_index,5-1,quick_int_str(params[params_index-1]->data_type->array_dimensions),ORANGE);
                        param.element_size=1;

                    }
                    else if (str_contains(param.type,"int")){
                        param.element_size=4;
                        
                    }
                    else if (str_contains(param.type,"int64")){
                        param.element_size=8;
                        
                    }
                    else if(str_contains(param.type,"struct")){
                        param.element_size=symbol_table_lookup(table,node->params[i]->data_type->value)->size;
                        param.struct_name=node->params[i]->data_type->value;

                    }
                    else{
                        param.element_size=4;
                    }

                    param.size=4;
                }
                else if(str_contains(param.type,"char") || str_contains(param.type,"bool")){
                    param.size=1;
                }
                else{
                    param.size=4;
                }


                //write_rowcol_text(13,2,param.type,RED);
                param.stack_offset = params_offset;
                param.is_global=false;

                symbol_table_insert(func_table, param);
                params_offset += 4;
                symbol.size+=param.size;
            }
        }

        int local_offset = 4;

        for (int64_t i = 0; node->children[i] != NULL; i++) {
            build_symbol_table(node->children[i], func_table, &local_offset);
        }

        in_function=false;

        symbol.locals_offset = local_offset;
        symbol.child_table = func_table;

        symbol_table_insert(table, symbol);

        return;
    }
    else if (str_eq(node->type,"LABEL")){
        struct Symbol symbol;
        symbol.type="label";
        symbol.name=node->identifier;

        symbol_table_insert(table,symbol);

    }

}

char delimiters[] = {
    ' ', ';', '(', ')', '{', '}', '\'', ':', '"', '+', '-', '*', '/', '%', '[', ']', '=', '>', '<', ',', '&', '|', '!', '\n', '@', '^','.','$','?', '~',
    '#',
};

char asm_keywords[][32]={
    "MOV", "EAX", "EBX", "ECX", "EDX", "EDI", "ESI", "EBP","ESP", "MOVZX", "INT", "SUB", "ADD", "IMUL", "IDIV", "CDQ", "RDTSC",
    "AL", "BL", "CL", "DL","AX","BX","CX","DX", "TEST", "CMP", "JE", "JNE", "JG","JGE","JL","JLE","JZ","NOT","NEG","PUSH","POP","CALL", "RET", "AND",
    "OR", "XOR", "JMP","PUSHAD","POPAD","LABEL", "HLT", "REP", "STOSB","STOSD", "MOVSB", "KEY", "GETMOUSE", "IN", "OUT", "MOVSS", "CVTSI2SS", "CVTTSS2SI", "UCOMISS",
    "SQRTSS", "DIVSS", "MULSS", "SUBSS", "ADDSS", "MOVD", "XMM0", "XMM1", "XMM2", "XMM3", "XMM4", "XMM5", "XMM6", "XMM7", "MOVSD", "CLD", "CLI","STI", 
};

char os_keywords[][32]={
    "malloc", "free", "FRAMEBUF", "PPR", "MAINLOOP",
};

char keywords[][32] = {
    "int", "str", "for", "if", "while", "char", "ret", "bool", "elif", "func", "void", "else",
    "float", "struct", "malloc", "fori", "sizeof", "def", "NULL","MOV", "EAX", "EBX", "ECX", "EDX", 
    "EDI", "ESI", "EBP","ESP", "MOVZX", "INT", "SUB", "ADD", "IMUL", "IDIV", "CDQ", "RDTSC",
    "AL", "BL", "CL", "DL","AX","BX","CX","DX", "TEST", "CMP", "JE", "JNE", "JG","JGE","JL","JLE","JZ","NOT","NEG","PUSH","POP","CALL", "RET", "AND",
    "OR", "XOR", "JMP", "PUSHAD","POPAD","LABEL","HLT", "REP","STOSB", "STOSD", "MOVSB","include", "break", "continue", "KEY",
    "IN", "OUT", "MOVSS", "CVTSI2SS", "CVTTSS2SI", "UCOMISS", "SQRTSS", "DIVSS", "MULSS", "SUBSS", "ADDSS", "MOVD", "XMM0", "XMM1", "XMM2",
    "XMM3", "XMM4", "XMM5", "XMM6", "XMM7", "MOVSD", "CLD", "CLI","STI","goto",
};

char operators[][2] = {
    "+", "-", "*", "/", "%", "++", "--", "+=", "-=", "/=", "*=", "&&", "||", "!=", "==", "^", "&","|", "!", "?","$",":", "=", ">", "<", ">=", "<=", "@"
    "~", "#", ".",
};



void access_global_variable_from_func(struct Symbol* symbol,struct SymbolTable* table);

void resize_node_array() {
    node_capacity *= 2;
    struct ASTNode **new_nodes = (struct ASTNode **) MALLOC(sizeof(struct ASTNode *) * node_capacity);

    for (int i = 0; i < nodes_index; i++) {
        new_nodes[i] = nodes[i];
    }
    nodes = new_nodes;
}

void resize_token_array() {
    token_capacity *= 2;
    struct Token *new_tokens = (struct Token *) MALLOC(sizeof(struct Token) * token_capacity);

    for (int i = 0; i < token_index; i++) {
        new_tokens[i] = tokens[i];
    }

    tokens = new_tokens;
}

void clear_buffer(char *buf, int *index) {
    *index = 0;
    for (int i = 0; i < 256; i++) {
        buf[i] = '\0';
    }
}

void strcpy(char* buf,char* str){
    int i;
    for (i=0;str[i]!='\0';i++)
        buf[i]=str[i];
    buf[i]='\0';
    
}

char *str_copy(char *str) {

    int sz = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        sz++;

    }
    char *buff = MALLOC(sz + 1);
    for (int i = 0; str[i] != '\0'; i++) {

        buff[i] = str[i];
    }
    buff[sz] = '\0';

    return buff;
}

int str_len(char *str, _Bool include) {
    int sz = 0;
    for (int i = 0; str[i] != '\0'; i++)sz++;
    if (include)return sz + 1;
    return sz;
}


void uint32_t_str(uint32_t value, char *buf) {
    int i = 0;
    _Bool is_negative = false;


    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {
        //printf("1");
        buf[i++] = (char)(value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }
}

_Bool is_bin(char* str){
    if (str[0]!='0')return false;
    for (int i=2;str[i]!='\0';i++){
        if(str[i]>'1'||str[i]<'0')return false;
    }
    return true;

}

_Bool is_hex(char* str){
    if (str[0]!='0')return false;

    for (int i=2;str[i]!='\0';i++){
        if ((str[i]<'a')&&str[i]>'F')return false;
        if ((str[i]<'A'&&str[i]>'9'))return false;
        if (str[i]<'0')return false;
        if (str[i]>'f')return false;
        
    }
    return true;

}

_Bool is_number(char* str) {  
    if (str[1]=='b')return is_bin(str);
    if (str[1]=='x')return is_hex(str);

    for (int i=0;str[i]!='\0';i++){
        if (str[i]<'0'||(str[i]>'9'))return false;
    }
    return true;
}

_Bool is_delimiter(char c) {
    for (int i = 0; i < sizeof(delimiters)/sizeof(delimiters[0]); i++) {
        if (c == delimiters[i]) {
            return true;
        }
    }
    return false;
}

_Bool is_keyword(char *buffer) {
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (str_eq(buffer, keywords[i]) && !is_number(buffer))return true;
    }
    return false;
}

_Bool is_operator(char *buffer) {
    for (int i = 0; i < sizeof(operators) / sizeof(operators[0]); i++) {
        if (str_eq(buffer, operators[i]))return true;
    }
    return false;
}

_Bool is_os_keyword(char* buf){
    for (int i=0;i<sizeof(os_keywords)/sizeof(os_keywords[0]);i++){
        if (str_eq(buf,os_keywords[i]))
            return true;
    }
    return false;
}

_Bool is_asm_keyword(char* buf){
    for (int i=0;i<sizeof(asm_keywords)/sizeof(asm_keywords[0]);i++){
        if (str_eq(buf,asm_keywords[i]))
            return true;
    }
    return false;
}

void emit_token(char *buffer) {
    struct Token token;

    int value_sz = 0;

    for (int i = 0; buffer[i] != '\0'; i++) value_sz++;


    char *buf = MALLOC(value_sz + 1);

    for (int i = 0; i < value_sz; i++)buf[i] = buffer[i];
    //printf("Buffer = %s\n", buffer);

    buf[value_sz] = '\0';

    token.value = buf;
    //printf("Buf[0]=%c", buf[0]);

    if (str_eq(buf, "false") || str_eq(buf, "true"))token.type = "BOOLEAN";
    else if(is_os_keyword(buf))token.type="OS_KEYWORD";
    else if(is_asm_keyword(buf))token.type="ASM";
    else if (is_keyword(buf))token.type = "KEYWORD";
    else if (is_number(buf))token.type= "NUMBER";
    else token.type = "IDENTIFIER";

    tokens[token_index++] = token;
}

struct Token advance() {
    //if (token_index!=token_count) {
    struct Token token = tokens[token_index++];
    return token;
    //}
}

struct Token peek() {
    //if (token_index!=token_count) {
    return tokens[token_index];
    //}
}

_Bool match(char *value) {
    if (str_eq(peek().value, value)) {
        return true;
    }
    return false;
}

void emit_error(char* type,char* message){
    struct Error* error=MALLOC(sizeof(struct Error));
    error->type=type;
    error->message=message;

    errors[error_index++]=error;

}

void get_error_line_num(){

}

void expect(char *type, char *value) {
    if (str_eq("IDENTIFIER", type)) {
        if (!str_eq(advance().type, type)) {
            emit_error("EXPECTED TOKEN",value);
            return;
        }
    } else if (!str_eq(peek().type, type) || !str_eq(peek().value, value)) {
        emit_error("EXPECTED TOKEN",value);
        
    }
    advance();
}

struct ASTNode* parse_malloc(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    node->type="malloc";

    advance();
    expect("PARENTHESIS","(");
    node->right=parse_bitwise_or();
    expect("PARENTHESIS",")");
    
    return node;

}

struct ASTNode *parse_call() {
    //call(func(x,y));
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    advance();//->foo

    char *identifier = advance().value;//->( - CHANGE TO PARSE_PRIMARY()
    

    expect("PARENTHESIS", "(");//->) | param 

    uint64_t param_index = 0;
    uint64_t param_capacity = 100;
    struct ASTNode **params = MALLOC(param_capacity * sizeof(struct ASTNode *));
    node->param_count=0;

    while (!match(")")&&!error_index) {
        if (param_index == param_capacity) {
            param_capacity *= 2;
            struct ASTNode **new_params = MALLOC(param_capacity * sizeof(struct ASTNode *));
            for (int i = 0; i < param_index; i++) {
                new_params[i] = params[i];
            }
            params = new_params;
        }

        params[param_index++] = parse_expression();
        //printf("AFTER PE=%s", peek().value);

        node->param_count++;

        if (match(","))advance();

    }
    node->type = "call";
    node->identifier = identifier;
    node->params = params;
    
    advance(); //->

    //write_rowcol_text(10,70,str_cat("OUTSIDE LOOP PARSE CALL=",peek().value),PURPLE);

    return node;
}

struct ASTNode* parse_primary();

struct ASTNode* parse_input() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    node->type = advance().value;
    expect("PARENTHESIS","(");
    node->right=parse_primary();
    expect("PARENTHESIS",")");

    expect("SEMICOLON", ";");

    return node;


}

struct ASTNode* parse_type(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->array_dimensions=NULL;
    node->array_sizes=NULL;
    
    node->type=advance().value;
    node->value=NULL;
    //write_rowcol_text(10,15,peek().value,RED);

    if (str_eq(node->type,"struct")){
        
        node->value=advance().value;
        

    }

    //write_rowcol_text(11,15,peek().value,RED);

    int ptr_count=0;
    while (match("*")){
        ptr_count++;
        advance();
    }

    if (ptr_count>0){
        node->type=str_cat(node->type,"_ptr");
        node->array_dimensions=ptr_count;
    }
    

    int arr_count=0;

    struct ASTNode** sizes=MALLOC(128*sizeof(struct ASTNode));
    int i=0;
    while(match("[")){
        //write_rowcol_text(12+i,25,peek().value,RED);

        advance();
        arr_count++;
        sizes[i++]=parse_bitwise_or();
        expect("BRACKET","]");
        if (!match("["))break;

    }
    if (arr_count>0){
        node->type=str_cat(node->type,"_arr");
        node->array_dimensions=arr_count;
        node->array_sizes=sizes;
    }else{
        FREE(sizes);
    }
    
    //write_rowcol_text(12,15,node->type,RED);

    return node;

}

int exp(int num,int power){
    if (power==0)return 1;
    int save=num;
    for (int i=1;i<power;i++){
        num*=save;
    }
    return num;
}

char* strbintointstr(char* num){
    int ind;

    for (ind=0;num[ind+1]!='\0';ind++);

    int res=0;
    int stride=0;

    for (int i=ind;i>1;i--){
        int val=num[i]-'0';
        if (val>1){
            //er?ror
        }
        res+=(val*exp(2,stride++));
        
    }

    char buf[32];
    int_str(res,buf);

    return str_copy(buf);

}

char* inttostrhex(int num){
    char buf[32];

    int i=0;

    while(num>0){
        int digit=num%16;
        num/=16;
        
        char c;

        if (digit >= 10)
        {
            switch (digit)
            {
            case 10:
                c = 'A';
                break;
            case 11:
                c = 'B';
                break;
            case 12:
                c = 'C';
                break;
            case 13:
                c = 'D';
                break;
            case 14:
                c = 'E';
                break;
            case 15:
                c = 'F';
                break;
            }
        }
        else{
            c='0'+digit;
        }
        buf[i++]=c;

    }
    buf[i]='\0';

    int j=0;

    for (j=0;buf[j+1]!='\0';j++);

    int k=0;

    char buf2[32];
    while (j>=0){
        buf2[k++]=buf[j--];


    }

    buf2[k]='\0';
    return str_cat("0x",buf2);


}

char* strhextointstr(char* num){
    int ind;

    for (ind=0;num[ind+1]!='\0';ind++);

    int stride=0;
    int res=0;

    for (int i=ind;i>1;i--){
        int val=num[i]-'0';

        if (val>9){
            switch(num[i]){
                case 'a':{
                    val=10;
                    break;
                }
                case 'A':{
                    val=10;
                    break;
                }
                case 'b':{
                    val=11;
                    break;
                }
                case 'B':{
                    val=11;
                    break;
                }
                case 'c':{
                    val=12;
                    break;
                }
                case 'C':{
                    val=12;
                    break;
                }
                case 'd':{
                    val=13;
                    break;
                }
                case 'D':{
                    val=13;
                    break;
                }
                case 'e':{
                    val=14;
                    break;
                }
                case 'E':{
                    val=14;
                    break;
                }
                case 'f':{
                    val=15;
                    break;
                }
                case 'F':{
                    val=15;
                    break;
                }
                default: break;
                


            }


        }

        res+=(val*exp(16,stride++));

    }
    char buf[32];
    int_str(res,buf);


    return str_copy(buf);

}

struct ASTNode* parse_dereference();

struct ASTNode *parse_identifier(){
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    node->type = "IDENTIFIER";
    node->identifier = advance().value;

    if (match("[")){
        int element_index = 0;
        int element_capacity = 100;
        struct ASTNode **indexes = MALLOC(element_capacity * sizeof(struct ASTNode *));

        advance();
        // printf("\nAFTER MATCH [=%s\n", peek().value);

        while (!match("]") && !error_index){
            if (element_index == element_capacity){
                element_capacity *= 2;
                struct ASTNode **new_elements = MALLOC(element_capacity * sizeof(struct ASTNode *));
                for (int i = 0; i < element_index; i++){
                    new_elements[i] = indexes[i];
                }
                indexes = new_elements;
            }
            indexes[element_index++] = parse_bitwise_or();

            expect("BRACKET", "]");
            if (!match("["))
                break;

            advance();
        }
        node->indexes = indexes;
        node->type = "array_element";
        
    }

    if (match(".") || match("->")){ //p.x
        struct ASTNode *node2 = MALLOC(sizeof(struct ASTNode));

        node2->left=node;

        node2->type=match(".")?"member_access":"member_ptr_access";

        advance();

        node2->right=parse_identifier();

        //FREE(node);
        node=node2;
    }

    return node;
}

struct ASTNode* parse_sizeof();

struct ASTNode *parse_primary() {
    if (str_eq(peek().type, "STRING")) {
        struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
        node->type = "string";
        node->value = advance().value;

        return node;
    }

    if (str_eq(peek().type, "CHARACTER")) {
        struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
        node->type = "char";
        char *value = advance().value;
        
        int cv = (int) value[0];

        char buf[32];

        int_str(cv, buf);

        //printf("CHAR VALUE=%s", buf);

        node->value = str_copy(buf);


        return node;
    }

    if (str_eq(peek().type, "BOOLEAN")) {
        struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
        node->type = "BOOLEAN";
        node->value = advance().value;

        return node;
    }
    if (match("KEY")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="KEY";
        advance();
        return node;
    }
    if (match("malloc")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="__malloc";

        advance();
        expect("PARENTHESIS","(");
        node->right=parse_bitwise_or();
        expect("PARENTHESIS",")");
        return node;

    }
    if (match("FRAMEBUF")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="__FRAMEBUF";

        advance();
        return node;

    }
    
    if (match("PPR")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="__PPR";
        advance();
        return node;
    }

    if (match("EAX") || match("EBX") || match("ECX") || match("EDX") || match("EDI") || match("ESI") || match("EBP") || match("ESP")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="REGISTER";
        node->value=advance().value;
        return node;

    }
    if (match("AL") || match("BL") || match("CL") || match("DL")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="reg8";
        node->value=advance().value;
        return node;
    }
    if (match("AX") || match("BX") || match("CX") || match("DX")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="reg16";
        node->value=advance().value;
        return node;

    }
    if (str_eq(peek().type, "NUMBER")) {
        struct ASTNode *node = (struct ASTNode *) MALLOC(sizeof(struct ASTNode));
        node->type = "NUMBER";
        char* num= advance().value;

        if (num[1]=='b'){
            num=strbintointstr(num);
        }
        else if(num[1]=='x'){
            num=strhextointstr(num);
        }
        node->value=num;

        if (match(".")){
            node->type="FLOATING POINT";
            advance();
            node->fraction=advance().value;
        }

        return node;
    }
    if (match("sizeof")){
        return parse_sizeof();
    }
    if (match("?")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

        node->type="ternary";
        advance();

        node->condition=parse_bitwise_or();
        expect("OPERATOR","?");
        node->left=parse_bitwise_or();
        expect("COLON",":");
        node->right=parse_bitwise_or();
        return node;
    }

    if (match("~")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        advance();
        node->type="~";
        node->right=parse_primary();

        return node;

    }    
    if (match("$")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

        node->type="$";
        advance();

        return node;
    }
    if (str_eq(peek().type, "IDENTIFIER")) {
        //write_rowcol_text(38,3,tokens[token_index].value,ORANGE); p[5]->x

        /*if (str_eq(tokens[token_index+1].value,".")||str_eq(tokens[token_index+1].value,"->")){
            
            
            struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
            node->type="parent_member_access";
            node->identifier=peek().value;
            node->right=parse_identifier();
            return node;
        }*/
        return parse_identifier();
    }
    if (match("(")) {
        advance();

        if (str_eq(peek().type,"KEYWORD")){
            struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
            node->type="cast";
            node->left=parse_type();
            expect("PARENTHESIS",")");
            node->right=parse_primary();

            return node;
        }

        struct ASTNode *node = parse_bitwise_or();

        expect("PARENTHESIS", ")");

        return node;
    }
    if (match("-")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="negation";
        advance();

        node->right=parse_primary();

        return node;
    }
    if (match("!")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

        node->type="not";
        advance();

        node->right=parse_primary();

        return node;
        
    }
    if (match("#")) {
        return parse_call();
    }
    if (match("*")){
        return parse_dereference();

    }

    if (match("@")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

        advance();

        node->type="addressof";
        node->identifier=advance().value;
        

        return node;
    }


    emit_error("UNEXPECTED TOKEN",peek().value);
    return NULL;
}

/*
 *
 *
 *expression → term (("+" | "-") term)*
term → primary (("*" | "/") primary)*
primary → NUMBER | IDENT | "(" expression ")"
*/

//int x=1+2*3;
struct ASTNode *parse_term() {
    struct ASTNode *node = parse_primary();

    while ((match("*") || match("/")||match("%")||match("<<")||match(">>")) && str_eq(peek().type,"OPERATOR")) {
        struct ASTNode *new_node = MALLOC(sizeof(struct ASTNode));

        if (match("*")){
            new_node->type="MULTIPLY";
        }else if(match("/")){
            new_node->type="DIVIDE";
        }
        else if(match(">>")||match("<<")){
            new_node->type=peek().value;
        }
        else{
            new_node->type="MOD";
        }

        advance();

        new_node->left = node;
        new_node->right = parse_primary();

        node = new_node;
    }
    return node;
}

struct ASTNode *parse_expression() {
    //int x=1+2*3
    struct ASTNode *node = parse_term();

    while ((match("+") || match("-")) && str_eq(peek().type,"OPERATOR")) {
        struct ASTNode *new_node = MALLOC(sizeof(struct ASTNode));
        new_node->type = str_eq(peek().value, "+") ? "PLUS" : "MINUS";

        advance();

        new_node->left = node;
        new_node->right = parse_term();

        node = new_node;
    }
    return node;
}

/*struct ASTNode *parse_string() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    node->value = peek().value;
    node->type=advance().type;


    while (match("+")) {
        struct ASTNode *new_node = MALLOC(sizeof(struct ASTNode));
        advance();//->x
        new_node->type="string_concat";

        new_node->right=parse_expression();
        new_node->left = node;

        node = new_node;
    }

    return node;
}*/

struct ASTNode *parse_comparison() {
    struct ASTNode *node = parse_expression();

    while ((match(">") || match("<") || match("==") || match("!=") || match(">=") || match("<=")) && str_eq(peek().type,"OPERATOR")) {
        struct ASTNode *new_node = MALLOC(sizeof(struct ASTNode));

        new_node->type = advance().value;

        new_node->left = node;

        new_node->right = parse_expression();

        node = new_node;
    }
    return node;
}

struct ASTNode *parse_logical_and() {
    struct ASTNode *node = parse_comparison();

    while (match("&&") && str_eq(peek().type,"OPERATOR")) {
        struct ASTNode *new_node = MALLOC(sizeof(struct ASTNode));
        new_node->type = "&&";
        new_node->left = node;

        advance();

        new_node->right = parse_comparison();

        node = new_node;
    }
    return node;
}

struct ASTNode *parse_bool() {
    struct ASTNode *node = parse_logical_and();


    while (match("||") && str_eq(peek().type,"OPERATOR")) {
        struct ASTNode *new_node = MALLOC(sizeof(struct ASTNode));
        new_node->type = advance().value;

        new_node->left = node;

        new_node->right = parse_logical_and();

        node = new_node;
    }

    return node;
}

struct ASTNode* parse_bitwise_and(){
    struct ASTNode* node=parse_bool();

    while((match("&")||match("^"))  && str_eq(peek().type,"OPERATOR")){
        struct ASTNode* new_node=MALLOC(sizeof(struct ASTNode));
        new_node->type=advance().value;
        new_node->left=node;
        new_node->right=parse_bool();
        node=new_node;
    }
    return node;

}

struct ASTNode* parse_bitwise_or(){
    struct ASTNode* node=parse_bitwise_and();

    while(match("|") && str_eq(peek().type,"OPERATOR")){
        struct ASTNode* new_node=MALLOC(sizeof(struct ASTNode));

        new_node->type=advance().value;
        new_node->left=node;
        new_node->right=parse_bitwise_and();

        node=new_node;

    }
    return node;

}

struct ASTNode *parse_char() {
    char *type = peek().type;

    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    node->right = parse_expression();
    return node;
}

struct ASTNode *parse_array() {
    expect("BRACE", "{");

    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    int count = 0;
    //printf("\nPEEK B4=%s\n", peek().value);

    uint64_t elements_capacity = 100;

    struct ASTNode **elements = MALLOC(elements_capacity * sizeof(struct ASTNode *));


    while (!match("}")&&!error_index) {
        if (count == elements_capacity) {
            elements_capacity *= 2;
            struct ASTNode **new_node = MALLOC(elements_capacity * sizeof(struct ASTNode *));
            for (int i = 0; i < count; i++) {
                new_node[i] = elements[i];
            }
            elements = new_node;
        }

        //printf("IN LOOP %s\n", peek().value);

        if (match("{")) {
            elements[count++] = parse_array();
        } else {
            //printf("IN ELSE %s", peek().value);
            elements[count++] = parse_expression();
            //printf("IN END ELSE=%s\n", peek().value);
        }

        //printf("pEEK = %s", peek().value);
        if (match(","))advance();
    }
    //printf("PEEK AFTER LOOP=%s\n", peek().value);
    expect("BRACE", "}");
    node->element_count = count;
    node->elements = elements;
    node->type = "array";
    return node;
}

struct ASTNode* parse_pointer(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    
    node->right=parse_expression();
    



    return node;

}


struct ASTNode *parse_declaration() {
    char *type = advance().value; //-->x
    char *type2 = str_cat(type, "_decl");

    int dimensions = 0;
    struct ASTNode **array_sizes = MALLOC(100 * sizeof(struct ASTNode *));
    int array_sizes_index = 0;

    while(match("*")){
        dimensions++;
        advance();


    }

    if (dimensions>0){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->array_dimensions=dimensions;

        node->type=str_cat(str_cat(type,"_ptr"),"_decl");

        char* identifier=advance().value;
        node->identifier=identifier;

        if (!match("=")){
            expect("SEMICOLON",";");
            return node;
        }

        expect("OPERATOR","=");

        node->right=parse_expression();
        
        expect("SEMICOLON",";");
        return node;


    }
    dimensions=0;

    while (match("[")) {
        dimensions++;
        advance();

        if (!match("]")) {
            array_sizes[array_sizes_index++] = parse_expression();
        }

        expect("BRACKET", "]");
    }
    //printf("AFTER WHILE DIMENSIONS=%s", peek().value);

    if (dimensions > 0) {
        struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
        int sz = 0;

        char *arrtype = str_cat(type, "_arr");

        node->type = arrtype;
        node->type = str_cat(node->type, "_decl");

        char *identifier = advance().value;
        node->identifier = identifier;
        node->array_dimensions = dimensions;
        node->array_sizes = array_sizes;

        if (match("=")) {
            advance();
            node->right = parse_array();
            expect("SEMICOLON", ";");
        } else {
            expect("SEMICOLON", ";");
        }
        return node;
    }


    char *identifier = advance().value; //--> =|;

    if (match(";")) {
        advance();
        struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
        node->type = type2;
        node->identifier = identifier;
        return node;
    }else if(!match("=")){
        emit_error("EXPECTED TOKEN","=");
    }

    advance(); //-->y

    struct ASTNode *node = parse_bitwise_or();
    struct ASTNode *decl = MALLOC(sizeof(struct ASTNode));

    decl->type = type2;
    decl->identifier = identifier;
    decl->left = NULL;
    decl->right = node;

    expect("SEMICOLON", ";"); // advance past ;
    return decl;

}
int y2=6;
struct ASTNode *parse_print_statement() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    node->type = advance().value;
    expect("PARENTHESIS", "(");
    node->right = parse_expression();
    expect("PARENTHESIS", ")");
    expect("SEMICOLON", ";");

    //write_rowcol_text(y2++,40,str_cat("Peek.value in parse_print() = ",peek().value),RED);

    return node;
}

struct ASTNode *parse_wfill() {
    char *type = advance().value; //->(

    expect("PARENTHESIS", "(");

    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    node->type = type;

    node->param1 = parse_expression();
    expect("COMMA", ",");

    node->param2 = parse_expression();
    expect("COMMA", ",");

    node->param3 = parse_expression();
    expect("COMMA", ",");

    node->param4 = parse_expression();
    expect("PARENTHESIS", ")"); //->;
    if(match(";"))advance(); //->

    return node;
}

struct ASTNode *parse_if() {
    char *type = advance().value; //->(

    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    node->type = type;

    expect("PARENTHESIS","("); //->x
    node->condition = parse_bitwise_or();
    expect("PARENTHESIS", ")"); //->{
    expect("BRACE","{"); //->statement1

    uint64_t statements_capacity = 1000;
    uint64_t statements_index = 0;

    struct ASTNode **statements = MALLOC(statements_capacity * sizeof(struct ASTNode *));

    while (!match("}")&&!error_index) {
        if (statements_index == statements_capacity) {
            //resize
            statements_capacity *= 2;
            struct ASTNode **realloc_arr = MALLOC(statements_capacity * sizeof(struct ASTNode *));

            for (int i = 0; i < statements_index; i++) {
                realloc_arr[i] = statements[i];
            }
            statements = realloc_arr;
        }

        statements[statements_index++] = parse_statement();
    }
    node->children = statements;
    advance();

    return node;
}

struct ASTNode *parse_for() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    advance(); //->(
    expect("PARENTHESIS","("); //->c1

    struct ASTNode *declaration = parse_declaration();

    //expect("SEMICOLON",";");

    //printf("after declaration=%s\n", peek().value);

    struct ASTNode *condition = parse_bitwise_or();

    expect("SEMICOLON",";");

    //printf("after condition=%s\n", peek().value);

    
    struct ASTNode *increment = parse_identifier_increment();
    //printf("after increment=%s\n", peek().value);
    expect("PARENTHESIS",")"); //-> { | ;
    if (!match("{")){
        //advance();
        node->type = "for";
        node->fordeclaration = declaration;
        node->forcondition = condition;
        node->forincrement = increment;
        node->children[0] = parse_statement();
        //expect("SEMICOLON",";");
        return node;
    }
    expect("BRACE","{"); //->statement1
    //printf("after advancements=%s\n", peek().value);
    uint64_t statements_capacity = 100;
    uint64_t statements_index = 0;

    struct ASTNode **statements = MALLOC(statements_capacity * sizeof(struct ASTNode *));
    while (!match("}")&&!error_index) {
        //printf("fijeife");
        if (statements_index == statements_capacity) {
            statements_capacity *= 2;
            struct ASTNode **realloc_arr = (struct ASTNode **) MALLOC(statements_capacity * sizeof(struct ASTNode *));
            for (int i = 0; i < statements_index; i++) {
                realloc_arr[i] = statements[i];
            }
            statements = realloc_arr;
        }
        statements[statements_index++] = parse_statement();

        if (token_index>=token_count&&!match("}")){
            emit_error("EXPECTED TOKEN","}");
            break;
        }
        

    }


    advance(); //->

    node->type = "for";
    node->fordeclaration = declaration;
    node->forcondition = condition;
    node->forincrement = increment;
    node->children = statements;

    return node;
}

struct ASTNode* parse_fori(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->type="fori";
    

    advance();
    expect("PARENTHESIS","(");

    node->right=parse_bitwise_or();

    expect("PARENTHESIS",")");
    expect("BRACE","{");

    uint64_t statements_capacity = 100;
    uint64_t statements_index = 0;

    struct ASTNode **statements = MALLOC(statements_capacity * sizeof(struct ASTNode *));

    while (!match("}")&&!error_index) {
        if (statements_index == statements_capacity) {
            statements_capacity *= 2;
            struct ASTNode **realloc_arr = (struct ASTNode **) MALLOC(statements_capacity * sizeof(struct ASTNode *));
            for (int i = 0; i < statements_index; i++) {
                realloc_arr[i] = statements[i];
            }
            statements = realloc_arr;
        }
        statements[statements_index++] = parse_statement();
    }
    node->children=statements;

    expect("BRACE","}");

    return node;

}

struct ASTNode *parse_array_assignment() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    advance(); //consume '['

    int indexes_index = 0;
    int indexes_size = 100;
    struct ASTNode **indexes = MALLOC(indexes_size * sizeof(struct ASTNode *));

    while (!match("]")&&!error_index) {
        if (indexes_index == indexes_size) {
            indexes_size *= 2;
            struct ASTNode **realloc_arr = MALLOC(indexes_size * sizeof(struct ASTNode *));
            for (int i = 0; i < indexes_index; i++) {
                realloc_arr[i] = indexes[i];
            }
            indexes = realloc_arr;
        }
        indexes[indexes_index++] = parse_expression();
        expect("BRACKET", "]");
        if (!match("["))break;
        advance();
    }
    indexes[indexes_index]=NULL;

    char *op = advance().value;
    node->indexes = indexes;
    node->right = parse_expression();
    node->type = "array_assignment";
    node->op = op;
    expect("SEMICOLON", ";");
    return node;
}

struct ASTNode *parse_identifier_increment() {
    /*struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    char *identifier = advance().value; //->[ | op

    if (match("[")) {
        struct ASTNode *assignment_node = parse_array_assignment();
        assignment_node->identifier = identifier;
        return assignment_node;
    }*/


    struct ASTNode* node=parse_identifier();

    char *op = advance().value;
    node->type = str_contains(node->type,"array")?
        "array_assignment":str_contains(node->type,"member")?
            node->type:"operation";
    
    node->op = op;


    if (str_eq(op, "++") || str_eq(op, "--")) {
        node->operation = NULL;

    } 
    else {
        //write_rowcol_text(30,5,peek().value,ORANGE);
        node->operation = parse_bitwise_or();
        //write_rowcol_text(31,5,peek().value,ORANGE);
        
    }

    //write_rowcol_text(20,17,peek().value,BLUE);
    


    return node;
}


struct ASTNode *parse_parameter() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    node->type = "parameter";


    node->data_type= parse_type();

    node->identifier = advance().value;

    return node;
}

struct ASTNode *parse_function() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    advance(); //->id

    char *identifer = advance().value; //->(
    advance(); //->x

    uint64_t params_capacity = 100;
    uint64_t params_index = 0;

    struct ASTNode **params = MALLOC(params_capacity * sizeof(struct ASTNode *));
    node->param_count=0;

    while (!match(")")&&!error_index) {
        node->param_count++;
        if (params_index == params_capacity) {
            params_capacity *= 2;
            struct ASTNode **realloc_arr = (struct ASTNode **) MALLOC(params_capacity * sizeof(struct ASTNode *));
            for (int i = 0; i < params_index; i++) {
                params[i] = realloc_arr[i];
            }
            params = realloc_arr;
        }

        params[params_index++] = parse_parameter();
        
        if (match(","))advance();
    }

    if (params_index == 0) {
        node->params = NULL;
    }

    advance(); //->{
    advance(); //->statement1

    uint64_t statements_capacity = 100;
    uint64_t statements_index = 0;
    struct ASTNode **statements = (struct ASTNode **) MALLOC(statements_capacity * sizeof(struct ASTNode *));

    while (!match("}")&&!error_index) {
        if (statements_index == statements_capacity) {
            statements_capacity *= 2;
            struct ASTNode **realloc_arr = MALLOC(statements_capacity * sizeof(struct ASTNode *));

            for (int i = 0; i < statements_index; i++) {
                realloc_arr[i] = statements[i];
            }
            statements = realloc_arr;
        }
        if (match("}"))break;
        statements[statements_index++] = parse_statement();
    }
    expect("BRACE","}"); //->
    node->identifier = identifer;
    //node->return_type = return_type;
    node->params = params;
    node->type = "function";
    node->children = statements;
    return node;
}

struct ASTNode *parse_return() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    advance(); //->expression

    if (!match(";"))node->right = parse_bitwise_or();
    else node->right = NULL;

    node->type = "return";
    return node;
}

struct ASTNode *parse_continue() {
    advance();

    expect("SEMICOLON", ";");

    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    node->type = "continue";

    return node;
}

struct ASTNode *parse_break() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    advance();

    expect("SEMICOLON", ";");

    node->type = "break";
    return node;
}

struct ASTNode *parse_else() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));
    node->type = "else";


    uint64_t children_capacity = 100;
    uint64_t children_index = 0;
    struct ASTNode **children = MALLOC(children_capacity * sizeof(struct ASTNode *));

    advance(); //->{
    expect("BRACE", "{");
    //printf("\nB4 : %s", peek().value);

    while (!match("}")&&!error_index) {
        if (children_index == children_capacity) {
            children_capacity *= 2;
            struct ASTNode **realloc_arr = MALLOC(children_capacity * sizeof(struct ASTNode *));
            for (int i = 0; i < children_index; i++) {
                realloc_arr[i] = children[i];
            }
            children = realloc_arr;
        }

        children[children_index++] = parse_statement();
    }
    expect("BRACE", "}");

    node->children = children;

    return node;
}

struct ASTNode* parse_circle() {
    struct ASTNode *node = MALLOC(sizeof(struct ASTNode));

    node->type=advance().value;

    expect("PARENTHEIS","(");

    node->param1=parse_expression();
    expect("COMMA",",");
    node->param2=parse_expression();
    expect("COMMA",",");
    node->param3=parse_expression();
    expect("PARENTHESIS",")");
    expect("SEMICOLON",";");

    return node;
}
int y=6;

struct ASTNode* parse_colour(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    advance();
    expect("PARENTHESIS","(");
    node->param1=parse_expression();
    expect("COMMA",",");
    node->param2=parse_expression();
    expect("COMMA",",");
    node->param3=parse_expression();
    expect("PARENTHESIS",")");

    expect("SEMICOLON",";");

    node->type="colour";

    return node;
    
}

struct ASTNode* parse_sleep(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->type="sleep";

    advance();
    expect("PARENTHESIS","(");

    node->right=parse_bitwise_or();

    expect("PARENTHESIS",")");

    expect("SEMICOLON",";");

    return node;

}

struct ASTNode* parse_dereference(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    int dimensions=0;

    while(match("*")){

        dimensions++;
        advance();
    }

    node->type="dereference";
    node->array_dimensions=dimensions;
    node->right=parse_primary();

    return node;
    

}

struct ASTNode* parse_rand(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    node->type="rand";

    advance();
    expect("PARENTHESIS","(");

    node->param1=parse_bitwise_or();
    expect("COMMA",",");
    node->param2=parse_bitwise_or();
    expect("PARENTHESIS",")");

    return node;

}

struct ASTNode* parse_struct(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    node->type="struct";

    advance();//-->id

    node->value=advance().value;

    int dimensions=0;


    while(match("*")){
        dimensions++;
        advance();
    }
    if (dimensions){
        node->type=str_cat(node->type,"_ptr");
        node->array_dimensions=dimensions;
    }

    struct ASTNode** sizes=MALLOC(128*sizeof(struct ASTNode*));
    int i=0;

    while(match("[")){
        advance();

        sizes[i++]=parse_bitwise_or();
        dimensions++;

        expect("BRACKET","]");
    }

    if (i){
        node->type=str_cat(node->type,"_arr");
        node->array_sizes=sizes;
        node->array_dimensions=dimensions;
    }else{
        //FREE(sizes);
    }


    

    node->identifier=advance().value;

    if (match(";")){
        advance();
        return node;
    }
    expect("OPERATOR","=");

    node->right=parse_bitwise_or();
    expect("SEMICOLON",";");


    return node;

}

struct ASTNode* parse_free(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->type="free";

    advance();

    expect("PARENTHESIS","(");

    node->right=parse_primary();

    if (!str_eq(node->right->type,"IDENTIFIER"))emit_error("UNEXPECTED TYPE",node->right->type);

    expect("PARENTHESIS",")");

    expect("SEMICOLON",";");

    return node;

}

struct ASTNode* parse_key(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    node->type="KEY";
    advance();
    char* op=advance().value;
    node->right=parse_bitwise_or();
    expect("SEMICOLON",";");
    return node;
}

struct ASTNode* parse_fields(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    

}

struct ASTNode* parse_def(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->type="def";

    advance();

    node->value=advance().value;

    node->identifier=advance().value;
    
    expect("BRACE","{");

    int ind=0;
    int cap=512;
    struct ASTNode** children=MALLOC(cap*sizeof(struct ASTNode*));

    int field_count=0;

    while(!match("}")&&error_index==0){
        children[ind]=parse_type();
        children[ind++]->identifier=advance().value;
        expect("SEMICOLON",";");

        field_count++;

        

    }

    expect("BRACE","}");

    node->children=children;
    node->array_dimensions=field_count;

    return node;
    
}

struct ASTNode* parse_asm_statement(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    
    //struct ASTNode* operand2=MALLOC(sizeof(struct ASTNode));

    node->type="asm";
    node->value=advance().value;

    if (str_eq(node->value,"LABEL")){
        node->type="LABEL";
        node->identifier=advance().value;
        expect("SEMICOLON",";");
        return node;
    }
    if (match(";")){
        advance();
        node->operand1=NULL;
        return node;
    }
    if (match("[")){
        advance();
        node->operand1=MALLOC(sizeof(struct ASTNode));
        node->operand1->op="mem";
        node->operand1->type="asm";
        node->operand1->value=advance().value;
        while (!match("]")&&!error_index){
            node->operand1->value=str_cat(node->operand1->value,advance().value);
        }
        expect("BRACKET","]");
    }
    else if(str_eq(peek().type,"NUMBER")||match("-")){
        _Bool is_neg=false;
        if (match("-")){
            advance();
            is_neg=true;
        }

        node->operand1=MALLOC(sizeof(struct ASTNode));
        node->operand1->type="imm";
        char* str=advance().value;
        node->operand1->value=is_neg?str_cat("-",str):str;
        //write_rowcol_text(10,34,str_cat("== ",peek().value),ORANGE);
        
    }
    else if (is_asm_keyword(peek().value)){
        
        node->operand1=MALLOC(sizeof(struct ASTNode));

        node->operand1->type="asm";
        node->operand1->value=advance().value;
    }
    
    else{

        node->operand1=parse_bitwise_or();

    }
    
    if (match(";")){
        advance();
        node->operand2=NULL;
        return node;
    }
    
    if (match("[")){
        advance();
        node->operand2=MALLOC(sizeof(struct ASTNode));
        node->operand2->op="mem";
        node->operand2->type="asm";
        node->operand2->value=advance().value;
        while (!match("]")&&!error_index){
            node->operand2->value=str_cat(node->operand2->value,advance().value);
        }
        expect("BRACKET","]");

    }
    else if(str_eq(peek().type,"NUMBER") || match("-")){
        _Bool is_neg=false;
        if (match("-")){
            advance();
            is_neg=true;
        }
        
        node->operand2=MALLOC(sizeof(struct ASTNode));
        node->operand2->type="imm";
        char* str=advance().value;
        node->operand2->value=is_neg?str_cat("-",str):str;

    }
    else if (str_eq(peek().type,"ASM")){
        node->operand2=MALLOC(sizeof(struct ASTNode));
        node->operand2->type="asm";
        node->operand2->value=advance().value;
        //write_rowcol_text(10,10,"ASM",RED);

    }
    else{
        node->operand2=parse_bitwise_or();
    }
    //expect("SEMICOLON",";");
    return node;

}

struct ASTNode* parse_callad(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->type="callad";

    advance();
    expect("PARENTHESIS","(");
    
    node->right=parse_bitwise_or();


}

struct ASTNode* parse_sizeof(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));

    node->type="sizeof";

    advance();
    expect("PARENTHESIS","(");

    if (str_eq(peek().type,"IDENTIFIER")){
        node->value="id";
        node->data_type=parse_identifier();
    }
    else{
        node->value="type";
        node->data_type=parse_type();
    }

    expect("PARENTHESIS",")");

    return node;

}

struct ASTNode* parse___free(){
    struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
    node->type="__free";
    advance();
    expect("PARENTHESIS","(");
    node->right=parse_primary();
    expect("PARENTHESIS",")");
    expect("SEMICOLON",";");
    return node;

}

//goto parsestatement

struct ASTNode *parse_statement() {
    //write_rowcol_text(y++,10,peek().value,BLUE);
    if (str_eq(tokens[token_index].type, "KEYWORD")) {
        if (match("char") || match("int") || match("bool") || match("float")) {
            //write_rowcol_text(36,77,"ENTERING PARSE_DECLARATION",RED);
            return parse_declaration();
        }
        if (match("if") || match("while") || match("elif")) {
            return parse_if();
        }
        if (match("for")) {
            return parse_for();
        }
        if (match("fori")){
            return parse_fori();
        }
        if (match("func")) {
            //write_rowcol_text(31,77,"ENTERING PARSE_func",RED);
            return parse_function();
        }
        if (match("ret")) {
            return parse_return();
        }
        if (match("continue")) {
            return parse_continue();
        }
        if (match("break")) {
            return parse_break();
        }
        
        if (match("else")) {
            //write_rowcol_text(30+y++,2,"in else",RED);
            return parse_else();
        }
        if (match("sleep")){
            return parse_sleep();
        }
        if (match("struct")){

            return parse_struct();
        }
        if (match("def")){
            return parse_def();

        }
        if (match("goto")){
            struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
            advance();
            node->type="goto";
            node->right=parse_bitwise_or();
            expect("SEMICOLON",";");
            return node;
        }

    } else if (str_eq(peek().type, "IDENTIFIER")) {
        struct ASTNode* node= parse_identifier_increment();
        expect("SEMICOLON",";");
        return node;
    }
    else if(match("*")){
        struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
        node->type="dereference_assignment";

        node->array_dimensions=0;

        while (match("*")){
            advance();
            node->array_dimensions++;
        }

        node->left=parse_primary();
        node->op=advance().value;
        node->operation=parse_bitwise_or();
        expect("SEMICOLON",";");

        return node;
    }
    else if(match("#")){
        struct ASTNode* node=parse_call();
        expect("SEMICOLON",";");
        return node;
    }
    else if(str_eq(peek().type,"ASM")){
        if (match("KEY"))
            return parse_key();
        return parse_asm_statement();
    }
    else if(str_eq(peek().type,"OS_KEYWORD")){
        if (match("free")){
            return parse___free();

        }
        if (match("MAINLOOP")){
            struct ASTNode* node=MALLOC(sizeof(struct ASTNode));
            node->type="mainloop";
            advance();
            expect("SEMICOLON",";");
            return node;
        }

    }
    
    if (match(";")) {
        advance();
        return parse_statement();
    }
    if(str_eq(peek().type,"NUMBER")){
        emit_error("LONELY NUMBER",peek().value);
    }
    if (str_eq(peek().type,"OPERATOR")){
        emit_error("LONELY OPERATOR",peek().value);

    }
    if (str_eq(peek().type,"PARENTHESIS")||str_eq(peek().type,"COMMA")||str_eq(peek().type,"BRACKET")){
        emit_error("LONELY DELIMITER",peek().value);

    }
    if (match("include")){
        advance();
        advance();
        return parse_statement();
        
    }
    
    

    return NULL;
}

void parse_program() {
    token_count = token_index;
    token_index = 0;

    node_capacity = 512;
    nodes =MALLOC(node_capacity * sizeof(struct ASTNode *));
    nodes_index = 0;
    //printf("Token count before=%llu", token_count);
    //exit(0);


    //write_rowcol_text(20,10,str_cat("ENTERING TOKEN= ",peek().value),ORANGE);
    while (token_index < token_count&&!error_index) {
        //printf("Token index=%llu\n", token_index);

        if (nodes_index >= node_capacity) {
            //printf("resized\n");
            resize_node_array();
        }
        nodes[nodes_index++] = parse_statement();
    }
}

void lex_cm(char *file) {
    int buffer_index = 0;
    _Bool string_literal = 0;
    _Bool character_literal = 0;

    char buffer[256] = {'\0'};

    for (int i = 0; file[i] != '\0'; i++) {
        if (token_index >= token_capacity) {
            resize_token_array();
        }
        char c = file[i];


        if (c=='/'&&file[i+1]=='/'){
            while (file[i+1]!='\n'&&file[i+1]!='\0'){
                i++;
            }
            continue;
        }

        if (c=='/' && file[i+1]=='*'){
            while (file[i]!='*' || file[i+1]!='/'){
                i++;

            }
            i++;
            continue;
        }


        if (!is_delimiter(c)) {
            buffer[buffer_index++] = c;
        } else {
            if (c == '\'') {
                if (!character_literal) {
                    character_literal = true;
                } else {
                    character_literal = false;

                    struct Token token;

                    token.type = "CHARACTER";

                    int value_sz = 0;

                    for (int j = 0; buffer[j] != '\0'; j++) value_sz++;
                    if (value_sz>1&&buffer[0]!='\\'){
                        emit_error("SYNTAX ERROR","multi character character literal");
                    }
                    if (value_sz==0){
                        emit_error("SYNTAX ERROR","empty character literal");
                    }

                    char *buf = MALLOC(value_sz + 1);

                    for (int j = 0; j < value_sz; j++)buf[j] = buffer[j];

                    buf[value_sz] = '\0';

                    token.value = buf;

                    tokens[token_index++] = token;

                    clear_buffer(buffer, &buffer_index);
                }
                continue;
            }
            if (c == '"') {
                if (!string_literal) {
                    string_literal = true;

                } 
                else {
                    string_literal = false;

                    struct Token token;

                    int value_sz = 0;

                    for (int j = 0; buffer[j] != '\0'; j++) value_sz++;

                    

                    char *buf = MALLOC(value_sz + 1);

                    for (int j = 0; j < value_sz; j++) {
                        buf[j] = buffer[j];
                    }

                    buf[value_sz] = '\0';
                    


                    token.value = buf;

                    token.type = "STRING";

                    tokens[token_index++] = token;

                    clear_buffer(buffer, &buffer_index);
                }
                continue;
            }
            if (string_literal || character_literal) {
                buffer[buffer_index++] = c;
                continue;
            }
            if (buffer[0] != '\0')emit_token(str_copy(buffer));

            clear_buffer(buffer, &buffer_index);


            struct Token token;
            switch (c) {
                case '(': {
                    token.type = "PARENTHESIS";
                    token.value = "(";
                    tokens[token_index++] = token;
                    continue;
                }
                case ')': {
                    token.type = "PARENTHESIS";
                    token.value = ")";
                    tokens[token_index++] = token;
                    continue;
                }
                case ' ': {
                    continue;
                }
                case ';': {
                    token.type = "SEMICOLON";
                    token.value = ";";
                    tokens[token_index++] = token;
                    continue;
                }
                case ':': {
                    token.type="COLON";
                    token.value=":";
                    tokens[token_index++]=token;
                    continue;
                }
                case '\'': {
                    character_literal = !character_literal;
                    continue;
                }
                case '{': {
                    token.type = "BRACE";
                    token.value = "{";
                    tokens[token_index++] = token;
                    continue;
                }
                case '}': {
                    token.type = "BRACE";
                    token.value = "}";
                    tokens[token_index++] = token;
                    continue;
                }
                case '[': {
                    token.type = "BRACKET";
                    token.value = "[";
                    tokens[token_index++] = token;
                    continue;
                }
                case ']': {
                    token.type = "BRACKET";
                    token.value = "]";
                    tokens[token_index++] = token;
                    continue;
                }
                case ',': {
                    token.type = "COMMA";
                    token.value = ",";
                    tokens[token_index++] = token;
                    continue;
                }
            }

            token.type = "OPERATOR";

            switch (c) {
                case '+': {
                    switch (file[i + 1]) {
                        case '=': {
                            token.value = "+=";
                            i++;
                            tokens[token_index++] = token;
                            continue;
                        }
                        case '+': {
                            token.value = "++";
                            i++;
                            tokens[token_index++] = token;
                            continue;
                        }
                        default: {
                            token.value = "+";
                            tokens[token_index++] = token;
                            continue;
                        }
                    }
                }
                case '-': {
                    switch (file[i + 1]) {
                        case '>':{
                            token.value="->";
                            i++;
                            tokens[token_index++]=token;

                            continue;
                        }
                        case '-': {
                            token.value = "--";
                            i++;
                            tokens[token_index++] = token;
                            continue;
                        }
                        case '=': {
                            token.value = "-=";
                            i++;
                            tokens[token_index++] = token;
                            continue;
                        }
                        default: {
                            token.value = "-";
                            tokens[token_index++] = token;
                            continue;
                        }
                    }
                }
                case '*': {
                    if (file[i + 1] == '=') {
                        token.value = "*=";
                        i++;
                        tokens[token_index++] = token;
                        continue;
                    }
                    token.value = "*";
                    tokens[token_index++] = token;
                    continue;
                }
                case '/': {
                    if (file[i + 1] == '=') {
                        token.value = "/=";
                        i++;
                        tokens[token_index++] = token;
                        continue;
                    }
                    token.value = "/";
                    tokens[token_index++] = token;
                    continue;
                }
                case '&': {
                    if (file[i+1]=='&'){
                        token.value="&&";
                        i++;
                        tokens[token_index++]=token;
                        continue;
                    }
                    token.value = "&";
                    tokens[token_index++] = token;
                    continue;
                }
                case '|': {
                    if (file[i+1]=='|'){
                        token.value="||";
                        i++;
                        tokens[token_index++]=token;
                        continue;
                    }
                    token.value = "|";
                    tokens[token_index++] = token;
                    continue;
                }
                case '=': {
                    if (file[i + 1] == '=') {
                        token.value = "==";
                        i++;
                        tokens[token_index++] = token;
                        continue;
                    }
                    token.value = "=";
                    tokens[token_index++] = token;
                    continue;
                }
                case '<': {
                    if (file[i + 1] == '=') {
                        token.value = "<=";
                        i++;
                        tokens[token_index++] = token;
                        continue;
                    }
                    if(file[i+1]=='<'){
                        token.value="<<";
                        i++;
                        tokens[token_index++]=token;
                        continue;
                    }
                    token.value = "<";
                    tokens[token_index++] = token;
                    continue;
                }
                case '>': {
                    if (file[i + 1] == '=') {
                        token.value = ">=";
                        i++;
                        tokens[token_index++] = token;
                        continue;
                    }
                    if(file[i+1]=='>'){
                        token.value=">>";
                        i++;
                        tokens[token_index++]=token;
                        continue;
                    }
                    token.value = ">";
                    tokens[token_index++] = token;
                    continue;
                }
                case '!': {
                    if (file[i + 1] == '=') {
                        token.value = "!=";
                        i++;
                        tokens[token_index++] = token;
                        continue;
                    }
                    token.value = "!";
                    tokens[token_index++] = token;
                    continue;
                }
                case '~': {
                    token.value="~";
                    tokens[token_index++]=token;
                    continue;
                }
                case '%': {
                    token.value="%";
                    tokens[token_index++]=token;
                    continue;
                }
                case '@': {
                    
                    token.value="@";
                    tokens[token_index++]=token;
                    continue;
                }
                case '^':{
                    token.value="^";
                    tokens[token_index++]=token;
                    continue;
                }
                case '?':{
                    token.value="?";
                    tokens[token_index++]=token;
                    continue;
                }
                case '.':{
                    token.value=".";
                    tokens[token_index++]=token;
                    continue;

                }
                case '$':{
                    token.value="$";
                    tokens[token_index++]=token;
                    continue;
                }
                case '#':{
                    token.value="#";
                    tokens[token_index++]=token;
                    continue;
                }
            }
        }
    }

    if (string_literal||character_literal) {
        emit_error("UNCLOSED STRING OR CHARACTER","");
        
    }
    /*for (int i = 0; i < token_index; i++) {
        //printf("Token type: %s\n", tokens[i].type);
        //printf("Token value:%s\n", tokens[i].value);
    }*/
}

struct SymbolTable *global_table;

void semantic_analysis() {
    //printf("IN SEMANTIC_ANALYSIS();\n");

    global_table = create_symbol_table(NULL);
    int offset = 4;
    for (uint64_t i = 0; nodes[i] != NULL; i++) {
        build_symbol_table(nodes[i], global_table, &offset);
    }

    //printf("\n\n\n\n======SYMBOL TABLE===========\n\n");
    /*for (uint64_t i = 0; i < global_table->count; i++) {
        //printf("%s: offset %d , size: %llu\n",
               global_table->symbols[i].name,
               global_table->symbols[i].stack_offset,
               global_table->symbols[i].size);
    }*/
}

char *find_identifier_value(char *identifier) {
    for (int i = 0; nodes[i] != NULL && i < nodes_index; i++) {
        struct ASTNode *node = nodes[i];

        _Bool is_identifier = str_eq(node->type, "str_decl") || str_eq(node->type, "int_decl") || str_eq(
                                  node->type, "char_decl");

        if (is_identifier) {
            if (str_eq(node->type, "int_decl") && str_eq(node->identifier, identifier)) {
                char num[32];

                int_str(evaluate_variable_expr(node->right), num);

                return str_copy(num);
            }
            if (str_eq(node->type, "char_decl") && str_eq(node->identifier, identifier)) {
                char num[32];
                int_str(evaluate_variable_expr(node->right), num);
                return str_copy(num);
            }
            if (str_eq(node->type, "str_decl") && str_eq(node->identifier, identifier)) {
                return evaluate_string(node->right);
            }
            if (str_eq(node->type, "bool_decl") && str_eq(node->identifier, identifier)) {
                char bool[32];
                int_str(evaluate_bool_expr(node->right), bool);
                return str_copy(bool);
            }
        }
    }
    return NULL;
}

uint32_t calculate_flat_index(struct ASTNode **indexes, uint32_t *sizes, uint32_t dimensions);


void emit_data_instruction(char *value) {
    if (data_capacity == data_count) {
        data_capacity *= 2;
        struct x86_data_instruction **realloc_arr = MALLOC(data_capacity * sizeof(struct x86_data_instruction *));
        for (long i = 0; i < data_count; i++) {
            realloc_arr[i] = data_instructions[i];
        }
        data_instructions = realloc_arr;
    }

    int size = len(value)+1;

    struct x86_data_instruction *data_instruction = MALLOC(sizeof(struct x86_data_instruction));

    
    data_instruction->address = STRINGS_ADDRESS + address_offset;
    data_instruction->size = size;

    data_instruction->value = value;

    data_instructions[data_count++] = data_instruction;

    address_offset += size;

    char* ptr=(char*)data_instruction->address;

    int i=0;
    for (;data_instruction->value[i]!='\0';i++) {
        ptr[i]=data_instruction->value[i];

        
    }
    ptr[i]=0;
}

struct x86_instruction *get_last_instruction() {
    return instructions[asm_count - 1];
}

_Bool is_32_bit(char* op1){
    char num_buf[64]={'\0'};
    int ind = 0;
    for (int i = 5; op1[i] != ']' && op1[i] != '\0'; i++)
    { //[ebp-12345]

        num_buf[ind++] = op1[i];
    }
    int val = str_int(str_copy(num_buf));

    if (val >= 128 || val < -128){
        return true;
    }
    return false;
}

int is_reg32(char *s)
{
    return s && (str_eq(s, "EAX") || str_eq(s, "EBX") ||
                 str_eq(s, "ECX") || str_eq(s, "EDX") ||
                 str_eq(s, "ESP") || str_eq(s, "EBP") ||
                 str_eq(s, "ESI") || str_eq(s, "EDI"));
}

// Helper function to check if string is an 8-bit register
int is_reg8(char *s)
{
    return s && (str_eq(s, "AL") || str_eq(s, "BL") ||
                 str_eq(s, "CL") || str_eq(s, "DL") ||
                 str_eq(s, "AH") || str_eq(s, "BH") ||
                 str_eq(s, "CH") || str_eq(s, "DH"));
}

int is_reg16(char* s){
    return s && (str_eq(s, "AX") || str_eq(s, "BX") ||
                 str_eq(s, "CX") || str_eq(s, "DX"));
}

// Helper function to check if operand is memory reference
int is_memory(char *s)
{
    return s && s[0] == '[';
}

// Helper function to check if operand is immediate value
int is_immediate(char *s)
{
    return s && ((s[0] >= '0' && s[0] <= '9') || s[0] == '-');
}

// Helper to determine if immediate fits in 8 bits
int fits_in_imm8(char *s){
    if (!is_immediate(s))
        return 0;
    int val = str_int(s);
    return (val >= -128 && val <= 127);
}

int get_disp_from_mem(char *mem_operand) {
    if (!mem_operand || mem_operand[0] != '[') return 0;
    
    char num_buf[64] = {'\0'};
    int ind = 0;
    
    // Find the start of the displacement (after EBP)
    char *ptr = mem_operand;
    while (*ptr && *ptr != '+' && *ptr != '-') ptr++;
    
    if (*ptr == '\0' || *ptr == ']') return 0; // No displacement
    
    // Copy the number including sign
    while (*ptr && *ptr != ']') {
        num_buf[ind++] = *ptr;
        ptr++;
    }
    
    
    return str_int(num_buf);
}

int fits_in_disp8(char *mem_operand) {
    int disp = get_disp_from_mem(mem_operand);
    return (disp >= -128 && disp <= 127);
}

_Bool is_xmm(char* op){
    if (!op)return false;

    char reg[5]={0};
    reg[0]='X';
    reg[1]='M';
    reg[2]='M';

    for (int i=0;i<8;i++){
        reg[3]='0'+i;
        if (str_eq(op,reg))return true;
    }
    return false;

}

int get_instruction_size(struct x86_instruction *instruction) {
    char *opcode = instruction->mnemonic;
    char *op1 = instruction->operand1;
    char *op2 = instruction->operand2;
    char *op3 = instruction->operand3;

    
    if (str_eq(opcode, "JMP")) {
        if (is_reg32(op1)){
            return 2;
        }
        return 5; // E9 + imm32
    }

    if (str_eq(opcode, "CLD") || str_eq(opcode,"STI") || str_eq(opcode,"CLI"))
        return 1;

    if (str_eq(opcode,"REP")){
        return 2;
        /*if (str_eq(op1,"STOSD") || str_eq(op1,"STOSB")){
            return 2;
        }
        if (str_eq(op1,"MOVSB") || str_eq(op1,"MOVSD"))
            return 2;
        */
    }

    if (str_eq(opcode,"IN")){
        if (is_reg8(op1)){
            return 1;
        }
        if (is_reg16(op1)){
            return 2;
        }
        if (is_reg16(op1) && is_imm(op2)){
            return 3;
        }
        return 3;

    }
    if (str_eq(opcode,"OUT")){
        if (is_reg8(op2))
            return 1;
        if (is_reg16(op2))
            return 2;
        if (is_immediate(op1) && is_reg8(op2))
            return 2;
        
        return 3;

    }

    if (str_eq(opcode, "MOVSS")){
        if (is_xmm(op1) && is_xmm(op2))
            return 4;

        char *mem = is_memory(op1) ? op1 : op2;

        if (str_contains(mem, "ESP")){
            return 9;
        }

        return 8; // Prefix(1) + Opcode(2) + ModRM(1) + Disp32(4)
    }

    if (str_eq(opcode, "MOVD")){
        if (is_xmm(op1) && is_reg32(op2))
            return 4;
        if (is_reg32(op1) && is_xmm(op2))
            return 4;

        char *mem = is_memory(op1) ? op1 : op2;

        int size = 4;

        if (str_contains(mem, "ESP")){
            size ++;
        }

        size += 4;

        return size;
    }

    if (str_eq(opcode, "CVTSI2SS") || str_eq(opcode, "CVTTSS2SI")){
        if (is_xmm(op1) && is_reg32(op2))
            return 4;
        if (is_reg32(op1) && is_xmm(op2))
            return 4;

        char *mem = is_memory(op1) ? op1 : op2;

        // Prefix (F3) + Opcode (0F 2A/2C) + ModRM = 4 bytes base
        int size = 4;

        if (str_contains(mem, "ESP"))
            size ++; // SIB byte
        size += 4;     // Forced Long Displacement

        return size;
    }

    if (str_eq(opcode, "ADDSS") || str_eq(opcode, "SUBSS") || 
        str_eq(opcode, "MULSS") || str_eq(opcode, "DIVSS") || 
        str_eq(opcode, "SQRTSS")) {

        if (is_xmm(op1) && is_xmm(op2)) return 4;

        char* mem = op2; 
        int size = 4;

        if (str_contains(mem, "ESP")) size += 1;
        size += 4; 

        return size;
    }

    if (str_eq(opcode, "UCOMISS")){
        if (is_xmm(op1) && is_xmm(op2))
            return 3; // Opcode(2) + ModRM(1)

        char *mem = op2;
        int size = 3; // Opcode(2) + ModRM(1)
        if (str_contains(mem, "ESP"))
            size += 1; // SIB
        size += 4;     // Forced Long
        return size;
    }

    if (str_eq(opcode, "JE") || str_eq(opcode, "JNE") ||
        str_eq(opcode, "JG") || str_eq(opcode, "JL") ||
        str_eq(opcode, "JGE") || str_eq(opcode, "JLE") ||
        str_eq(opcode, "JA") || str_eq(opcode, "JB") ||
        str_eq(opcode, "JAE") || str_eq(opcode, "JBE") ||
        str_eq(opcode, "JZ") || str_eq(opcode, "JNZ") ||
        str_eq(opcode, "JS") || str_eq(opcode, "JNS")) {

        // Always force long conditional jump encoding (0F 8x + imm32)
        return 6; // 0F 8x + imm32
    }

    if (str_eq(opcode, "CALL")) {
        if (is_reg32(op1))
            return 2;
        return 5; // E8 + 4 bytes
    }

    if (str_eq(opcode, "RET") || str_eq(opcode,"PUSHAD") || str_eq(opcode,"POPAD")) return 1; // C3

    
    if (str_eq(opcode, "PUSH")) {
        if (is_reg32(op1)) return 1; // 50+r
        if (is_immediate(op1)) {
            if (fits_in_imm8(op1)) return 2; // 6A + imm8
            return 5; // 68 + imm32
        }
        if (is_memory(op1)) return 6; // FF /6 + addressing
    }

    if (str_eq(opcode, "POP")) {
        if (is_reg32(op1)) return 1; // 58+r
        if (is_memory(op1)) return 6; // 8F /0 + addressing
    }

    
    if (str_eq(opcode, "MOV")) {
        // MOV reg32, reg32
        if (is_reg32(op1) && is_reg32(op2)) return 2; // 89/8B + ModR/M

        // MOV reg32, imm32
        if (is_reg32(op1) && is_immediate(op2)) return 5; // B8+r + imm32

        // MOV reg8, reg8
        if (is_reg8(op1) && is_reg8(op2)) return 2; // 88/8A + ModR/M

        // MOV reg8, imm8
        if (is_reg8(op1) && is_immediate(op2)) return 2; // B0+r + imm8

        if (is_reg16(op1) && is_immediate(op2)) return 3;

        if (is_reg16(op1) & is_reg16(op2)) return 3;

        // MOV reg32, [mem]
        if (is_reg32(op1) && is_memory(op2)) {
            Operand mem_op = parse_operand(op2);
            if (mem_op.type == OPERAND_MEMORY_DIRECT) return 6; // modrm + disp32
            int base = mem_op.mem.base_reg;
            int disp = mem_op.mem.disp;
            int size = 1 + 1; // opcode + modrm
            if (base == 4) size += 1; // SIB
            if (!(disp == 0 && base != 5)) {
                if (disp >= -128 && disp <= 127) size += 1; else size += 4;
            }
            return size;
        }

        // MOV [mem], reg32
        if (is_memory(op1) && is_reg32(op2)) {
            Operand mem_op = parse_operand(op1);
            if (mem_op.type == OPERAND_MEMORY_DIRECT) return 6;
            int base = mem_op.mem.base_reg;
            int disp = mem_op.mem.disp;
            int size = 1 + 1; // opcode + modrm
            if (base == 4) size += 1; // SIB
            if (!(disp == 0 && base != 5)) {
                if (disp >= -128 && disp <= 127) size += 1; else size += 4;
            }
            return size;
        }

        // MOV [mem], reg8
        if (is_memory(op1) && is_reg8(op2)) {
            Operand mem_op = parse_operand(op1);
            if (mem_op.type == OPERAND_MEMORY_DIRECT) return 6;
            int base = mem_op.mem.base_reg;
            int disp = mem_op.mem.disp;
            int size = 1 + 1; // opcode (0x88) + modrm
            if (base == 4) size += 1; // SIB
            if (!(disp == 0 && base != 5)) {
                if (disp >= -128 && disp <= 127) size += 1; else size += 4;
            }
            return size;
        }

        // MOV [mem], imm32
        if (is_memory(op1) && is_immediate(op2)) {
            Operand mem_op = parse_operand(op1);
            if (mem_op.type == OPERAND_MEMORY_DIRECT) return 10; // modrm + disp32 + imm32
            int base = mem_op.mem.base_reg;
            int disp = mem_op.mem.disp;
            int size = 1 + 1; // opcode + modrm
            if (base == 4) size += 1; // SIB
            if (!(disp == 0 && base != 5)) {
                if (disp >= -128 && disp <= 127) size += 1; else size += 4;
            }
            size += 4; // imm32
            return size;
        }

        // MOV reg8, [mem]
        if (is_reg8(op1) && is_memory(op2)) {
            if (str_contains(op2, "EBP"))
            {
                if (fits_in_disp8(op2))
                {
                    return 3; // 8A + ModR/M + disp8
                }
                return 6; // 8A + ModR/M + disp32
            }
            return 3;
        } // 8A + ModR/M + disp8
    }

    if (str_eq(opcode, "MOVZX")) {
        // MOVZX reg32, reg8
        if (is_reg32(op1) && is_reg8(op2)) return 3; // 0F B6 + ModR/M
        if (is_reg32(op1) && is_reg16(op2)) return 3;
        // MOVZX reg32, [mem8]
        if (is_reg32(op1) && is_memory(op2)) {
            if (str_contains(op2, "EBP"))
            {
                if (fits_in_disp8(op2))
                {
                    return 4; // 0F B6 + ModR/M + disp8
                }
                return 7; // 0F B6 + ModR/M + disp32
            }
            return 4;

        } // 0F B6 + ModR/M + disp
    }

    if (str_eq(opcode, "MOVSX")) {
        if (is_reg32(op1) && is_reg8(op2)) return 3; // 0F BE + ModR/M
        if (is_reg32(op1) && is_memory(op2)){
            if (str_contains(op2, "EBP"))
            {
                if (fits_in_disp8(op2))
                {
                    return 4;
                }
                return 7;
            }
            return 4;
        }
    }

    if (str_eq(opcode, "LEA")) {
        // LEA reg32, [mem]
        if (is_reg32(op1) && is_memory(op2)) {
            if (str_contains(op2, "EBP"))
            {
                if (fits_in_disp8(op2))
                    return 3;
                return 6;
            }
            return 6; // 8D + ModR/M + disp32
        }
    }

    
    if (str_eq(opcode, "ADD") || str_eq(opcode, "SUB") ||
        str_eq(opcode, "AND") || str_eq(opcode, "OR") ||
        str_eq(opcode, "XOR")) {
        
        // op reg32, reg32
        if (is_reg32(op1) && is_reg32(op2)) return 2; // opcode + ModR/M

        if (is_reg8(op1) && is_reg8(op2)) return 2; // opcode + ModR/M

        // op reg32, imm
        if (is_reg32(op1) && is_immediate(op2)) {
            // We force imm32 encoding for ESP in the emitter; reflect that here
            if (str_eq(op1, "ESP")) {
                return 6; // 81 + ModR/M + imm32
            }
            // For EBP and others, choose the smaller encoding when possible
            if (fits_in_imm8(op2)) return 3; // 83 + ModR/M + imm8
            return 6; // 81 + ModR/M + imm32
        }

        // op [mem], imm
        if (is_memory(op1) && is_immediate(op2))
        {
            // Compute precise size for memory + immediate by parsing the memory operand
            Operand mem_op = parse_operand(op1);
            int imm8 = fits_in_imm8(op2);

            int imm_size = imm8 ? 1 : 4;
            int opcode_size = 1; // 0x83 (imm8) or 0x81 (imm32) - opcode byte present

            if (mem_op.type == OPERAND_MEMORY_DIRECT) {
                return opcode_size + 1 /*modrm*/ + 4 /*disp32*/ + imm_size;
            }

            int base = mem_op.mem.base_reg;
            int disp = mem_op.mem.disp;

            int size = opcode_size + 1; // opcode + modrm
            if (base == 4) size += 1; // SIB
            if (!(disp == 0 && base != 5)) {
                if (disp >= -128 && disp <= 127) size += 1; else size += 4;
            }
            size += imm_size;
            return size;
        }

        // op [mem], reg32
        if (is_memory(op1) && is_reg32(op2))
        {
            if (str_contains(op1, "EBP"))
            {
                if (!fits_in_disp8(op1))
                    return 6;
                return 3;
            }
            return 6;
        }

        // op reg32, [mem]
        if (is_reg32(op1) && is_memory(op2)) {
            if (str_contains(op2, "EBP")) {
                if (!fits_in_disp8(op2))return 6;
                
                return 3;
            }
            return 6;
        }
    }

    if (str_eq(opcode, "IMUL")) {
        // IMUL reg32, imm   (two-operand IMUL with immediate)
        if (is_reg32(op1) && is_immediate(op2)) {
            if (fits_in_imm8(op2)) return 3; // 6B + ModR/M + imm8
            return 6; // 69 + ModR/M + imm32
        }

        // IMUL reg32, reg32
        if (is_reg32(op1) && is_reg32(op2)) return 3; // 0F AF + ModR/M
        
        // IMUL reg32, [mem]
        if (is_reg32(op1) && is_memory(op2)) return 4; // 0F AF + addressing
        
        // IMUL reg32, reg32, imm (three-operand form)
        if (is_reg32(op1) && is_reg32(op2) && is_immediate(op3)) {
            if (fits_in_imm8(op3)) return 3; // 6B + ModR/M + imm8
            return 6; // 69 + ModR/M + imm32
        }
    }

    if (str_eq(opcode, "IDIV") || str_eq(opcode, "DIV")) {
        if (is_reg32(op1)) return 2; // F7 /7 or F7 /6
        if (is_memory(op1)) return 3; // F7 + addressing
    }

    if (str_eq(opcode, "INC") || str_eq(opcode, "DEC")) {
        if (is_reg32(op1)) return 1; // 40+r or 48+r
        if (is_memory(op1)) return 3; // FF /0 or FF /1 + addressing
    }

    if (str_eq(opcode, "NEG") || str_eq(opcode, "NOT")) {
        if (is_reg32(op1)) return 2; // F7 + ModR/M
        if (is_memory(op1)) return 3; // F7 + addressing
    }

    
    if (str_eq(opcode, "CMP")) {
        // CMP reg32, reg32
        if (is_reg32(op1) && is_reg32(op2)) return 2; // 39/3B + ModR/M

        // CMP reg32, imm
        if (is_reg32(op1) && is_immediate(op2)) {
            if (fits_in_imm8(op2)) return 3; // 83 + ModR/M + imm8
            if (str_eq(op1, "EAX")) return 5; // 3D + imm32 (special encoding)
            return 6; // 81 + ModR/M + imm32
        }

        // CMP [mem], imm
        if (is_memory(op1) && is_immediate(op2)) {
            if (str_contains(op1, "EBP"))
            {
                if (fits_in_disp8(op1) && fits_in_imm8(op2))
                {
                    return 4; // 83 + ModR/M + disp8 + imm8
                }
                if (fits_in_disp8(op1))
                {
                    return 7; // 81 + ModR/M + disp8 + imm32
                }
                return 10; // 81 + ModR/M + disp32 + imm32
            }
            if (fits_in_imm8(op2)) return 4; // 83 + addressing + imm8
            return 7; // 81 + addressing + imm32
        }

        // CMP [mem], reg32
        if (is_memory(op1) && is_reg32(op2))
        {
            if (str_contains(op1, "EBP"))
            {
                if (fits_in_disp8(op1))
                {
                    return 3; // 39 + ModR/M + disp8
                }
                return 6; // 39 + ModR/M + disp32
            }
            return 3;
        }

        // CMP reg32, [mem]
        if (is_reg32(op1) && is_memory(op2))
        {
            if (str_contains(op2, "EBP"))
            {
                if (fits_in_disp8(op2))
                {
                    return 3; // 3B + ModR/M + disp8
                }
                return 6; // 3B + ModR/M + disp32
            }
            return 3;
        }
    }

    if (str_eq(opcode, "TEST")) {

        // TEST reg32, reg32
        if (is_reg32(op1) && is_reg32(op2)) return 2; // 85 + ModR/M

        // TEST reg8, reg8
        if (is_reg8(op1) && is_reg8(op2)) return 2; // 84 + ModR/M

        // TEST reg32, imm
        if (is_reg32(op1) && is_immediate(op2)) {
            if (str_eq(op1, "EAX")) return 5; // A9 + imm32
            return 6; // F7 + ModR/M + imm32
        }

        // TEST reg8, imm
        if (is_reg8(op1) && is_immediate(op2)) {
            if (str_eq(op1, "AL")) return 2; // A8 + imm8
            return 3; // F6 + ModR/M + imm8
        }
        
    }

    
    if (str_eq(opcode, "SETE") || str_eq(opcode, "SETNE") ||
        str_eq(opcode, "SETG") || str_eq(opcode, "SETL") ||
        str_eq(opcode, "SETGE") || str_eq(opcode, "SETLE") ||
        str_eq(opcode, "SETA") || str_eq(opcode, "SETB") ||
        str_eq(opcode, "SETAE") || str_eq(opcode, "SETBE") ||
        str_eq(opcode, "SETZ") || str_eq(opcode, "SETNZ") ||
        str_eq(opcode, "SETS") || str_eq(opcode, "SETNS")) {
        return 3; // 0F 9x + ModR/M
    }

    
    if (str_eq(opcode, "SHL") || str_eq(opcode, "SHR") ||
        str_eq(opcode, "SAL") || str_eq(opcode, "SAR") ||
        str_eq(opcode, "ROL") || str_eq(opcode, "ROR")) {
        
        // shift reg32, 1
        if (is_reg32(op1) && op2 && str_eq(op2, "1")) return 2; // D1 + ModR/M
        
        // shift reg32, CL
        if (is_reg32(op1) && op2 && str_eq(op2, "CL")) return 2; // D3 + ModR/M
        
        // shift reg32, imm8
        if (is_reg32(op1) && is_immediate(op2)) return 3; // C1 + ModR/M + imm8
    }

    
    if (str_eq(opcode, "LODSB") || str_eq(opcode, "STOSB") ||
        str_eq(opcode, "MOVSB") || str_eq(opcode, "SCASB")) {
        return 1;
    }

    if (str_eq(opcode, "LODSD") || str_eq(opcode, "STOSD") ||
        str_eq(opcode, "MOVSD") || str_eq(opcode, "SCASD")) {
        return 1;
    }

    if (str_eq(opcode, "REP") || str_eq(opcode, "REPE") ||
        str_eq(opcode, "REPNE")) {
        return 1; // Prefix byte
    }

    
    if (str_eq(opcode, "CDQ") || str_eq(opcode, "CWD")) return 1; // 99

    if (str_eq(opcode, "NOP")) return 1; // 90

    if (str_eq(opcode, "RDTSC")) return 2; // 0F 31

    if (str_eq(opcode, "INT")) return 2; // CD + imm8

    if (str_eq(opcode, "XCHG")) {
        if (is_reg32(op1) && is_reg32(op2)) {
            if (str_eq(op1, "EAX") || str_eq(op2, "EAX")) return 1; // 90+r
            return 2; // 87 + ModR/M
        }
    }

    
    // DWORD PTR prefix
    if (str_eq(op1, "DWORDPTR")) return 7;

    
    return 6;
}

int emit_false_instruction(char *mnemonic, char *operand1, char *operand2) {
    struct x86_instruction *instruction = MALLOC(sizeof(struct x86_instruction));

    instruction->mnemonic = str_copy(mnemonic);
    instruction->operand1 = str_copy(operand1);
    instruction->operand2 = str_copy(operand2);

    return get_instruction_size(instruction);
}

void emit_long_helper_instruction(char* mnemonic,char* op1,char*op2,char*op3,struct x86_instruction** ins,int*index,uint32_t*address) {
    struct x86_instruction *instruction = MALLOC(sizeof(struct x86_instruction));
    instruction->mnemonic = str_copy(mnemonic);
    instruction->operand1 = str_copy(op1);
    instruction->operand2 = str_copy(op2);
    instruction->operand3 = str_copy(op3);


    ins[*index]=instruction;
    (*index)++;

    *address+=get_instruction_size(instruction);
}

void emit_helper_instruction(char* mnemonic, char* operand1, char* operand2, struct x86_instruction** ins,int* index,uint32_t* address) {
    struct x86_instruction *instruction = MALLOC(sizeof(struct x86_instruction));
    instruction->mnemonic = str_copy(mnemonic);
    instruction->operand1 = str_copy(operand1);
    instruction->operand2 = str_copy(operand2);


    ins[*index]=instruction;
    (*index)++;

    *address+=get_instruction_size(instruction);
}

void emit_helper_function(struct x86_instruction** instructions,uint32_t address,char* name) {
    if (helpers_count==helpers_size) {
        helpers_size *= 2;
        struct helper_function** realloc_arr=MALLOC(helpers_size*sizeof(struct helper_function*));

        for (int i=0;i<helpers_count;i++) {

            realloc_arr[i]=helpers[i];
        }
        helpers = realloc_arr;
    }

    struct helper_function* helper=MALLOC(sizeof(struct helper_function));

    helper->address=address;
    helper->name=name;
    helper->instructions=instructions;

    helpers[helpers_count++]=helper;


}
int v=0;

void emit_byte_instruction(char *mnemonic, char *operand1, char *operand2) {
    if (asm_capacity == asm_count) {
        asm_capacity *= 2;
        struct x86_instruction **realloc_arr = MALLOC(asm_capacity * sizeof(struct x86_instruction *));
        for (long i = 0; i < asm_count; i++) {
            realloc_arr[i] = instructions[i];
        }
        instructions = realloc_arr;
    }

    struct x86_instruction *instruction = MALLOC(sizeof(struct x86_instruction));


    instruction->mnemonic = str_copy(mnemonic);
    instruction->operand1 = str_copy(operand1);
    instruction->operand2 = str_copy(operand2);
    instruction->operand3 = "BYTE";

    instructions[asm_count++] = instruction;

    uint32_t instruction_size=get_instruction_size(instruction);

    char bf[32];

    uint32_t_str(instruction_size,bf);

    //write_rowcol_text(4+v++,90,bf,ORANGE);


    global_address +=instruction_size;
}

void emit_instruction(char *mnemonic, char *operand1, char *operand2) {
    if (asm_capacity == asm_count) {
        asm_capacity *= 2;
        struct x86_instruction **realloc_arr = MALLOC(asm_capacity * sizeof(struct x86_instruction *));
        for (long i = 0; i < asm_count; i++) {
            realloc_arr[i] = instructions[i];
        }
        instructions = realloc_arr;
    }

    struct x86_instruction *instruction = MALLOC(sizeof(struct x86_instruction));


    instruction->mnemonic = str_copy(mnemonic);
    instruction->operand1 = str_copy(operand1);
    instruction->operand2 = str_copy(operand2);

    instructions[asm_count++] = instruction;

    uint32_t instruction_size=get_instruction_size(instruction);

    char bf[32];

    uint32_t_str(instruction_size,bf);

    //write_rowcol_text(4+v++,90,bf,ORANGE);


    global_address +=instruction_size;
}

_Bool is_const_bool_expr(struct ASTNode *node) {
    if (node == NULL) {
        return false;
    }

    if (str_eq(node->type, "NUMBER")) {
        return true;
    }

    if (str_eq(node->type, "IDENTIFIER")) {
        return false;
    }
    if (str_eq(node->type, "char")) {
        return true;
    }
    if (str_eq(node->type, "BOOLEAN")) {
        return true;
    }

    if (str_eq(node->type, "&&") || str_eq(node->type, "||") || str_eq(node->type, "==") || str_eq(node->type, "<") ||
        str_eq(node->type, ">") || str_eq(node->type, "<=") || str_eq(node->type, ">=") || str_eq(node->type, "!=")) {
        return is_const_bool_expr(node->left) && is_const_bool_expr(node->right);
    }
    //error
}

_Bool is_const_expr(struct ASTNode *node) {
    if (node == NULL) {
        return false;
    }
    if (str_eq(node->type, "NUMBER")) {
        return true;
    }
    if (str_eq(node->type, "IDENTIFIER"))return false;

    if (str_eq(node->type, "PLUS") || str_eq(node->type, "MINUS") || str_eq(node->type, "MULTIPLY") || str_eq(
            node->type, "DIVIDE")) {
        return is_const_expr(node->left) && is_const_expr(node->right);
    }
    return false;
}

void generate_str_expr(struct ASTNode *node, struct SymbolTable *table) {
    if (node == NULL)return;

    if (str_eq(node->type, "string")) {
        emit_data_instruction(node->value);

        struct x86_data_instruction *data = data_instructions[data_count - 1];

        char addr_buff[32];

        uint32_t_str(data->address, addr_buff);
        emit_instruction("MOV","EAX",addr_buff);
    }

    else if (str_eq(node->type,"IDENTIFIER")) {
        struct Symbol* symbol=symbol_table_lookup(table,node->identifier);

        int offset=symbol->stack_offset;

        char offset_buff[32];


        char* operand2;
        if (offset<0) {
            int_str(-offset,offset_buff);
            operand2= str_cat(str_cat("[EBP-",offset_buff),"]");

        }
        else {
            int_str(offset,offset_buff);
            operand2=str_cat(str_cat("[EBP+",offset_buff),"]");
        }

        emit_instruction("MOV","EAX",operand2);


    }
    else if (str_eq(node->type,"NUMBER")) {

    }
}

int generate_false_expr(struct ASTNode *node, struct SymbolTable *table, int offset) {
    if (node == NULL)return 0;

    if (str_eq(node->type, "BOOLEAN")) {
        if (str_eq(node->value, "true")) {
            offset += emit_false_instruction("MOV", "EAX", "1");
        } else {
            offset += emit_false_instruction("MOV", "EAX", "0");
        }
        return offset;
    }
    if (str_eq(node->type, "NUMBER")) {
        offset += emit_false_instruction("MOV", "EAX", node->value);
        return offset;
    }
    if (str_eq(node->type, "string")) {
        //error
        return 0;
    }

    if (str_eq(node->type, "char")) {
        offset += emit_false_instruction("MOV", "EAX", node->value);
        return offset;
    }


    if (str_eq(node->type, "IDENTIFIER")) {
        struct Symbol *symbol = symbol_table_lookup(table, node->identifier);

        //printf("DEBUG: Loading identifier '%s', offset=%d\n", node->identifier, symbol->stack_offset);

        if (str_eq(symbol->type, "str")) {
            //error
        }

        int soffset = symbol->stack_offset;
        char buf[32];

        char *addr;
        if (soffset < 0) {
            int_str(-soffset, buf);
            addr = str_cat(str_cat("[EBP-", buf), "]");
        } else {
            int_str(soffset, buf);
            addr = str_cat(str_cat("[EBP+", buf), "]");
        }

        offset += emit_false_instruction("MOV", "EAX", addr);
        return offset;
    }

    if (str_eq(node->type, "PLUS")) {
        int soffset = generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");

        generate_false_expr(node->right, table, soffset);
        offset += emit_false_instruction("POP", "EBX", "");
        offset += emit_false_instruction("ADD", "EAX", "EBX");
        return offset;
    }
    if (str_eq(node->type, "MINUS")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");

        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");
        offset += emit_false_instruction("SUB", "EAX", "EBX");
        return offset;
    }
    if (str_eq(node->type, "MULTIPLY")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");

        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");
        offset += emit_false_instruction("IMUL", "EAX", "EBX");
        return offset;
    }
    if (str_eq(node->type, "DIVIDE")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");

        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("MOV", "ECX", "EAX");
        offset += emit_false_instruction("POP", "EAX", "");
        offset += emit_false_instruction("DIV", "ECX", "");
        return offset;
    }

    if (str_eq(node->type, "&&")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");

        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("TEST", "EBX", "EBX");
        offset += emit_false_instruction("SETNZ", "BL", "");

        offset += emit_false_instruction("TEST", "EAX", "EAX");
        offset += emit_false_instruction("SETNZ", "AL", "");

        offset += emit_false_instruction("AND", "AL", "BL");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, "||")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");

        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("TEST", "EBX", "EBX");
        offset += emit_false_instruction("SETNZ", "BL", "");

        offset += emit_false_instruction("TEST", "EAX", "EAX");
        offset += emit_false_instruction("SETNZ", "AL", "");

        offset += emit_false_instruction("OR", "AL", "BL");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, ">")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");
        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("CMP", "EBX", "EAX");
        offset += emit_false_instruction("SETG", "AL", "");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, "<")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");
        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("CMP", "EBX", "EAX");
        offset += emit_false_instruction("SETL", "AL", "");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, ">=")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");
        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("CMP", "EBX", "EAX");
        offset += emit_false_instruction("SETGE", "AL", "");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, "<=")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");
        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("CMP", "EBX", "EAX");
        offset += emit_false_instruction("SETLE", "AL", "");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, "==")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");
        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("CMP", "EBX", "EAX");
        offset += emit_false_instruction("SETE", "AL", "");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    if (str_eq(node->type, "!=")) {
        generate_false_expr(node->left, table, offset);
        offset += emit_false_instruction("PUSH", "EAX", "");
        generate_false_expr(node->right, table, offset);
        offset += emit_false_instruction("POP", "EBX", "");

        offset += emit_false_instruction("CMP", "EBX", "EAX");
        offset += emit_false_instruction("SETNE", "AL", "");
        offset += emit_false_instruction("MOVZX", "EAX", "AL");
        return offset;
    }
    return offset;
}

struct Symbol* parent_struct=NULL;
_Bool struct_is_param;
struct Symbol** fields;
int fields_index=0;

struct Symbol* get_struct_field(char* name){//p.x.y
    struct Symbol* field=parent_struct;

    for (int i=1;i<fields_index;i++){
        
        for (int j=0;j<field->num_dimensions;j++){
            if (field->fields[j]==fields[i])
                field=field->fields[j];
        }
        
    }
    return field;

}
int jk=20;
struct Symbol* derive_field(char* name){
    
    struct Symbol* field=parent_struct;

    for (int i=0;i<field->num_dimensions;i++){

        //write_rowcol_text(21+i,30,"iiii",PURPLE);
        
        if (str_eq(field->fields[i]->name,name))
            
            return field->fields[i];
    }

    return field;
}

char* get_v_abs_addr(char* id, struct SymbolTable* table);

void generate_member_access(struct ASTNode* node,struct SymbolTable* table, _Bool get_value){
    _Bool has_dereferenced = false;

    parent_struct = symbol_table_lookup(table, symbol_table_lookup(table, node->left->identifier)->struct_name);

    //write_rowcol_text(22,10,parent_struct->name,YELLOW);
    struct Symbol *symbol = symbol_table_lookup(table, node->left->identifier);

    int offset = symbol->stack_offset;
    char ob[32];

    // fields=MALLOC(128*4);
    // fields[fields_index++]=symbol_table_lookup(table,parent_struct->struct_name);

    char* operand2=get_v_abs_addr(node->left->identifier,table);
    _Bool positive=symbol->is_global || (!symbol->is_global && symbol->stack_offset>0);

    emit_instruction("MOV", "EAX", operand2);

    if (str_eq(symbol->type, "struct_ptr")){
        positive = true; // now regardless, any computed offsets are added to EAX, not subbed, for calculating the final address

        emit_instruction("MOV", "EAX", "[EAX]");
    }

    if (str_eq(node->left->type, "array_element")){
            if (str_contains(symbol->type, "ptr")){
                char esb[32];
                int_str(symbol->element_size, esb);

                emit_instruction("MOV", "EDI", "EAX");
                emit_instruction("MOV", "EBX", esb);
                //emit_instruction("PUSH","EAX","");
                generate_ptr_index(node->left->indexes, symbol->num_dimensions, table);

                if (str_contains(node->type,"ptr")){
                    emit_instruction("MOV","EAX","[EAX]");
                    positive=true;

                }
                //emit_instruction("POP","EBX","");
            }
            else{
                emit_instruction("PUSH", "EAX", "");
                generate_array_index(node->left->indexes, symbol->dimensions, symbol->num_dimensions, table);

                emit_instruction("POP", "EBX", "");
                char szb[32];
                int_str(symbol->element_size,szb);
                emit_instruction("MOV","ECX",szb);
                emit_instruction("IMUL","EAX","ECX");

                emit_instruction(positive?"ADD":"SUB", "EBX", "EAX");
                emit_instruction("MOV","EAX","EBX");
            }
        }



    while (str_eq(node->right->type, "member_access") || str_eq(node->right->type, "member_ptr_access")){ // int x=p.x.y;
        node = node->right;

        _Bool is_ptr = str_eq(node->type, "member_ptr_access");

        //write_rowcol_text(jk++,4,str_cat("FINDING: ",node->left->identifier),PURPLE);

        struct Symbol *field = derive_field(node->left->identifier);


        if (field == NULL)
            emit_error("UNKNOWN FIELD", node->left->identifier);

        char ob[32];
        int_str(field->stack_offset, ob);

        emit_instruction(positive? "ADD" : "SUB", "EAX", ob);

        if (is_ptr){
            emit_instruction("MOV", "EAX", "[EAX]");
            positive = true;
        }

        if (str_eq(node->left->type, "array_element")){
            if (str_contains(field->type, "ptr")){
                char esb[32];
                int_str(field->element_size, esb);

                emit_instruction("MOV", "EDI", "EAX");
                emit_instruction("MOV", "EBX", esb);
                //emit_instruction("PUSH","EAX","");
                generate_ptr_index(node->left->indexes, field->num_dimensions, table);
                //emit_instruction("POP","EBX","");
            }
            else{
                emit_instruction("PUSH", "EAX", "");
                generate_array_index(node->left->indexes, field->dimensions, field->num_dimensions, table);

                emit_instruction("POP", "EBX", "");
                char szb[32];
                int_str(field->element_size,szb);
                emit_instruction("MOV","ECX",szb);
                emit_instruction("IMUL","EAX","ECX");
                emit_instruction(positive?"ADD":"SUB", "EBX", "EAX");
                emit_instruction("MOV","EAX","EBX");
            }
        }

        parent_struct = symbol_table_lookup(table, field->struct_name);
    }

    struct Symbol *final_field = derive_field(node->right->identifier);
    int final = final_field->stack_offset;

    if (final_field == NULL)
        emit_error("UNKNOWN FIELD", node->right->identifier);

    char offb[32];
    int_str(final, offb);

    if (str_contains(node->type,"ptr")){
        
    }

    //write_rowcol_text(36,10,str_cat("FINAL FIELD : ",node->right->identifier),GREEN);

    emit_instruction(positive ? "ADD" : "SUB", "EAX", offb);

    if (str_eq(node->right->type, "array_element")){
        if (str_contains(final_field->type, "ptr")){
            char esb[32];
            int_str(final_field->element_size, esb);

            emit_instruction("MOV", "EDI", "[EAX]");
            emit_instruction("MOV", "EBX", esb);
            generate_ptr_index(node->right->indexes, final_field->num_dimensions, table);
        }
        else{
            emit_instruction("PUSH", "EAX", "");
            generate_array_index(node->right->indexes, final_field->dimensions, final_field->num_dimensions, table);

            char eszb[32];
            int_str(final_field->element_size, eszb);
            emit_instruction("MOV", "ECX", eszb);
            emit_instruction("IMUL", "EAX", "ECX");

            emit_instruction("POP", "EBX", "");
            emit_instruction(positive? "ADD" : "SUB", "EBX", "EAX");
            emit_instruction("MOV", "EAX", "EBX");
        }
    }

    if (get_value)
        emit_instruction("MOV", "EAX", "[EAX]");

}

void generate_float_expr(struct ASTNode* node, struct SymbolTable* table){


}

_Bool is_float_expr(struct ASTNode* node, struct SymbolTable* table){
    if (str_eq(node->type,"FLOATING POINT")){
        return true;
    }
    if (str_eq(node->type,"IDENTIFIER")){
        struct Symbol* id=symbol_table_lookup(table,node->identifier);
        if (str_eq(id->type,"float"))
            return true;
    }
    if (node->right)
        return is_float_expr(node->right, table);
    if (node->left)
        return is_float_expr(node->left,table);

}

void get_sizeof(struct ASTNode* node, struct SymbolTable* table){
    char* size;
    if (str_eq(node->value,"type")){
        if (str_contains(node->data_type->type,"_ptr"))size="4";
        else if (str_eq(node->data_type->type,"int"))size="4";
        else if (str_eq(node->data_type->type,"bool") || str_eq(node->data_type->type,"char"))size="1";

        else if (str_eq(node->data_type->type,"struct")){
            struct Symbol* symbol=symbol_table_lookup(table,node->data_type->value);

            char buf[32];
            uint32_t_str(symbol->size,buf);

            size=str_copy(buf);

        }

    }
    else{
        struct Symbol* symbol=symbol_table_lookup(table, node->data_type->identifier);
        if (str_eq(node->data_type->type,"IDENTIFIER")){
            char buf[32];
            uint32_t_str(symbol->size,buf);

            size=str_copy(buf);
        }

    }

    emit_instruction("MOV","EAX",size);

}

int uiui=8;
void generate_expr(struct ASTNode *node, struct SymbolTable *table) {
    if (node == NULL){
        return;
    }

    /*if (is_float_expr(node, table)){
        generate_float_expr(node,table);
        
        return;

    }*/

    if (str_eq(node->type, "BOOLEAN")) {
        emit_instruction("MOV","EAX",str_eq(node->value,"true")?"1":"0");
        return;
    }

    if (str_eq(node->type, "NUMBER") || str_eq(node->type,"char")) {
        emit_instruction("MOV", "EAX", node->value);
        return;
    }
    if (str_eq(node->type,"$")){
        emit_instruction("MOV","EAX",quick_int_str(global_address));
        return;

    }
    if (str_eq(node->type,"KEY")){
        uint32_t addr=(uint32_t)(OS_KEY);
        char addb[32];
        uint32_t_str(addr,addb);

        emit_instruction("MOV","EBX",addb);
        emit_instruction("MOV","EAX","[EBX]");
        //emit_instruction("MOV","[EBX]","0");
        return;
    }

    if (str_eq(node->type,"__FRAMEBUF")){
        char buf[32];
        uint32_t_str((uint32_t)framebuffer,buf);
        emit_instruction("MOV","EAX",buf);
    }

    if (str_eq(node->type,"__PPR")){
        char buf[32];
        uint32_t_str(ppr,buf);
        emit_instruction("MOV","EAX",buf);
    }

    if (str_eq(node->type,"__malloc")){
        uint32_t addr=(uint32_t)(&MALLOC);
       

        generate_expr(node->right,table);
        emit_instruction("PUSH","EAX","");

        emit_instruction("CALL","-1","");
        char addrbuf[32];
        uint32_t_str(addr-global_address,addrbuf);
        instructions[asm_count-1]->operand1=str_copy(addrbuf);
        emit_instruction("ADD","ESP","4");


    }

    if (str_eq(node->type,"REGISTER")){
        if (!str_eq(node->value,"EAX"))
            emit_instruction("MOV","EAX",node->value);
        
        return;
    }
    
    if (str_eq(node->type,"reg8") || str_eq(node->type, "reg16")){
        emit_instruction("MOVZX","EAX",node->value);
        return;
    }

    if (str_eq(node->type,"inint") || str_eq(node->type,"instr")){
        _Bool isint=str_eq(node->type,"inint");
        emit_instruction("MOV","EAX","1");
        emit_instruction("MOV","EDI",isint?"0":"1");

        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);
        emit_instruction("MOV","EBX","EAX");

        emit_instruction("POP","EAX","");

        emit_instruction("MOV","ESI","EBX");
        emit_instruction("INT","100","");

        emit_instruction("MOV","EAX",quick_int_str((int)&INPUT_VALUE));
        emit_instruction("MOV","EAX","[EAX]");

        return;
    }

    if (str_eq(node->type,"ternary")){
        generate_expr(node->condition,table);

        emit_instruction("CMP","EAX","0");
        emit_instruction("JE","-1","");

        uint32_t a1a=global_address;
        int a1=asm_count-1;

        generate_expr(node->left,table);
        emit_instruction("JMP","-1","");

        uint32_t a2a=global_address;
        int a2=asm_count-1;

        generate_expr(node->right,table);

        instructions[a1]->operand1=quick_int_str(a2a-a1a);
        instructions[a2]->operand1=quick_int_str(global_address-a2a);
        

    }
    if (str_eq(node->type,"string")) {

        emit_instruction("MOV","EBX","0x145000");
        emit_instruction("MOV","EAX","[EBX]");
        emit_instruction("MOV","EBX","0x150000");
        emit_instruction("ADD","EAX","EBX");

        emit_instruction("PUSH","EAX","");
        emit_instruction("MOV","EBX","0x145000");
        emit_instruction("MOV","ECX","[EBX]");
        
        int i;
        for (i=0;node->value[i]!='\0';i++){
            int val=(int)node->value[i];
            emit_instruction("MOV","[EAX]",quick_int_str(val));
            emit_instruction("ADD","EAX","1");


            emit_instruction("ADD","ECX","1");

            
        }
        emit_instruction("MOV","[EAX]","0");
        emit_instruction("ADD","ECX","1");
        emit_instruction("MOV","EBX","0x145000");
        emit_instruction("MOV","[EBX]","ECX");

        emit_instruction("POP","EAX","");
        
        return;
    }

    if (str_eq(node->type,"call")){
        generate_call(node,table);
        return;
    }

    if (str_eq(node->type,"dereference")){

        int dim=node->array_dimensions;

        // emit_instruction("MOV","EAX",operand1);

        generate_expr(node->right,table);

        for (int i=0;i<dim;i++){
            emit_instruction("MOV","EAX","[EAX]");
        }
        return;
        
    }

    if (str_eq(node->type, "array_element")) {
        struct Symbol *symbol = symbol_table_lookup(table, node->identifier);
        int offset = symbol->stack_offset;

        _Bool is_byte=symbol->element_size==1;

        char sizebuf[32];
        if (!str_contains(symbol->type,"ptr")){
            generate_array_index(node->indexes,symbol->dimensions,symbol->num_dimensions,table);

            int_str(symbol->element_size,sizebuf);
            
            emit_instruction("MOV","EBX",sizebuf);
            emit_instruction("IMUL","EAX","EBX");
        }


        if (str_contains(symbol->type,"ptr")){
            char* operand1=get_v_addr(node->identifier,table);

            char sizebuf[32];
            int_str(symbol->element_size,sizebuf);
            
            emit_instruction("MOV","EDI",operand1);
            emit_instruction("MOV","EBX",sizebuf);

            generate_ptr_index(node->indexes,symbol->num_dimensions,table);//now have calculated address in EAX

            //emit_instruction("MOV","ECX","[EAX]");
            emit_instruction("MOV","EAX","[EAX]");
            if (is_byte)
                emit_instruction("MOVZX","EAX","AL");
            
            return;


        }
        char* operand1;

        if (symbol->is_global){
            operand1=get_v_addr(node->identifier,table);
            emit_instruction("ADD", "EDX", "EAX");
            operand1="[EDX]";
        }
        else {
            operand1=get_v_addr(node->identifier,table);
            emit_instruction("MOV","EDX",operand1);
            if (symbol->stack_offset>0)
                emit_instruction("ADD", "EDX", "EAX");
            else
                emit_instruction("SUB","EDX","EAX");
            
            operand1="[EDX]";
        }



        emit_instruction("MOV","EAX",operand1);
        if (is_byte)
            emit_instruction("MOVZX","EAX","AL");
        
        return;
    }
    if (str_eq(node->type,"addressof")){
        struct Symbol* symbol=symbol_table_lookup(table, node->identifier);

        if (str_eq(symbol->type,"function")){
            char buf[32];
            uint32_t_str(symbol->func_address,buf);
            emit_instruction("MOV","EAX",buf);
            return;
        }

        if (symbol->is_global){
            char buf[32];
            int_str(symbol->addr,buf);
            

            emit_instruction("MOV","EAX",buf);
        }
        else{
            
            char buf[32];
            if (symbol->stack_offset<0){
                int_str(-symbol->stack_offset,buf);
                emit_instruction("MOV","EAX","EBP");
                emit_instruction("SUB","EAX",buf);
            }
            else{
                int_str(symbol->stack_offset,buf);
                emit_instruction("MOV","EAX","EBP");
                emit_instruction("ADD","EAX",buf);

            }
        }

        return;

    }

    if (str_eq(node->type,"sizeof")){
        get_sizeof(node,table);
        return;
    }

    if (str_eq(node->type, "IDENTIFIER")) {
        struct Symbol *symbol = symbol_table_lookup(table, node->identifier);

        _Bool is_byte=symbol->size==1 || str_eq(symbol->type,"bool") || str_eq(symbol->type,"char");


        if (symbol==NULL){
            emit_error("UNKNOWN IDENTIFIER",node->identifier);
            return;
        }

        char* addr=get_v_addr(node->identifier,table);

        emit_instruction("MOV", "EAX", addr);

        if (is_byte)
            emit_instruction("MOVZX","EAX","AL");
        
        
        return;
    }

    if (str_eq(node->type, "PLUS")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");

        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");
        emit_instruction("ADD", "EAX", "EBX");
        return;
    }

    if (str_eq(node->type,"member_access") || str_eq(node->type,"member_ptr_access")){//int x=p.x->y;
        generate_member_access(node,table,true);
        return;

    }
    
    if (str_eq(node->type, "MINUS")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");

        generate_expr(node->right, table);
        emit_instruction("MOV", "EBX", "EAX");
        emit_instruction("POP", "EAX", "");
        emit_instruction("SUB", "EAX", "EBX");
        return;
    }
    if (str_eq(node->type, "MULTIPLY")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");

        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");
        emit_instruction("IMUL", "EAX", "EBX");
        return;
    }
    if (str_eq(node->type,"MOD")){
        emit_instruction("PUSH","EDX","");
        generate_expr(node->left,table);

        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);

        emit_instruction("MOV","EBX","EAX");
        emit_instruction("POP","EAX","");
        emit_instruction("CDQ","","");

        emit_instruction("IDIV","EBX","");
        emit_instruction("MOV","EAX","EDX");
        emit_instruction("POP","EDX","");

        return;
    }
    
    if (str_eq(node->type, "DIVIDE")) {
        emit_instruction("PUSH","EDX","");
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");

        generate_expr(node->right, table);
        emit_instruction("MOV", "EBX", "EAX");
        emit_instruction("POP", "EAX", "");
        emit_instruction("CDQ", "", "");
        emit_instruction("IDIV", "EBX", "");
        emit_instruction("POP", "EDX", "");
        return;
    }

    if (str_eq(node->type,"~")){
        generate_expr(node->right,table);
        emit_instruction("NOT","EAX","");
        return;
    }

    if (str_eq(node->type,"not")){
        generate_expr(node->right,table);

        emit_instruction("CMP","EAX","0");
        emit_instruction("SETE","AL","");
        emit_instruction("MOVZX","EAX","AL");

        return;
    }
    if (str_eq(node->type,"negation")){
        generate_expr(node->right,table);

        emit_instruction("NEG","EAX","");
        
        return;
    }

    if (str_eq(node->type, "&&")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");

        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("TEST", "EBX", "EBX");
        emit_instruction("SETNZ", "BL", "");

        emit_instruction("TEST", "EAX", "EAX");
        emit_instruction("SETNZ", "AL", "");

        emit_instruction("AND", "AL", "BL");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, "||")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");

        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("TEST", "EBX", "EBX");
        emit_instruction("SETNZ", "BL", "");

        emit_instruction("TEST", "EAX", "EAX");
        emit_instruction("SETNZ", "AL", "");

        emit_instruction("OR", "AL", "BL");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, ">")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");
        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("CMP", "EBX", "EAX");
        emit_instruction("SETG", "AL", "");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, "<")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");
        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("CMP", "EBX", "EAX");
        emit_instruction("SETL", "AL", "");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, ">=")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");
        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("CMP", "EBX", "EAX");
        emit_instruction("SETGE", "AL", "");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, "<=")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");
        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("CMP", "EBX", "EAX");
        emit_instruction("SETLE", "AL", "");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, "==")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");
        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("CMP", "EBX", "EAX");
        emit_instruction("SETE", "AL", "");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type, "!=")) {
        generate_expr(node->left, table);
        emit_instruction("PUSH", "EAX", "");
        generate_expr(node->right, table);
        emit_instruction("POP", "EBX", "");

        emit_instruction("CMP", "EBX", "EAX");
        emit_instruction("SETNE", "AL", "");
        emit_instruction("MOVZX", "EAX", "AL");
        return;
    }
    if (str_eq(node->type,"&")){
        generate_expr(node->left,table);
        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);
        emit_instruction("POP","EBX","");
        emit_instruction("AND","EAX","EBX");

        return;
    }
    if (str_eq(node->type,"^")){
        generate_expr(node->left,table);
        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);
        emit_instruction("POP","EBX","");
        emit_instruction("XOR","EAX","EBX");

        return;
    }
    if (str_eq(node->type,"|")){
        generate_expr(node->left,table);
        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);
        emit_instruction("POP","EBX","");
        emit_instruction("OR","EAX","EBX");

        return;
    }
    if (str_eq(node->type,">>")){
        generate_expr(node->left,table);
        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);
        emit_instruction("MOV","ECX","EAX");

        emit_instruction("POP","EAX","");
        emit_instruction("SHR","EAX","CL");

        return;
    }
    if (str_eq(node->type,"<<")){
        generate_expr(node->left,table);
        emit_instruction("PUSH","EAX","");

        generate_expr(node->right,table);
        emit_instruction("MOV","ECX","EAX");

        emit_instruction("POP","EAX","");
        emit_instruction("SHL","EAX","CL");

        return;
    }

}

_Bool is_const_string(struct ASTNode *node, struct SymbolTable *table) {
    if (str_eq(node->type, "string")) {
        //printf("%s:%d\n", node->type, 1);
        return true;
    }
    if (str_eq(node->type, "IDENTIFIER")) {
        //printf("%s:%d\n", node->type, 0);
        return false;
    }
    if (str_eq(node->type, "NUMBER")) {
        //printf("%s:%d\n", node->type, 1);
        return true;
    }
    if (str_eq(node->type, "char")) {
        //printf("%s:%d\n", node->type, 1);
        return true;
    }
    if (str_eq(node->type, "PLUS") || str_eq(node->type, "string_concat")) {
        //printf("IN PLUS ICS\n");
        if (is_const_string(node->left, table) && is_const_string(node->right, table)) {
            //printf("PLUS = %d\n", 1);
        } else {
            //printf("PLUS = %d\n", 0);
        }
        return is_const_string(node->left, table) && is_const_string(node->right, table);
    }
    if (str_eq(node->type, "MINUS") || str_eq(node->type, "MULTIPLY") || str_eq(node->type, "DIVIDE")) {
        return is_const_string(node->left, table) && is_const_string(node->right, table);
    }
    //error
    return false;
}

void generate_str(struct ASTNode* node,struct SymbolTable* table) {
    //printf("IN GEN STR\n");
    char strcpy[32];
    char strcat[32];
    char strlen[32];
    char mallocbuf[32];
    char int2str[32];
    char heapptr[32];

    uint32_t_str(STR_CPY_ADDRESS,strcpy);
    uint32_t_str(STR_CAT_ADDRESS,strcat);
    uint32_t_str(STR_LEN_ADDRESS ,strlen);
    uint32_t_str(MALLOC_ADDRESS,mallocbuf);
    uint32_t_str(INT2STR_ADDRESS,int2str);

    if (str_eq(node->type,"string")) {
        emit_data_instruction(str_copy(node->value));

        uint32_t address=data_instructions[data_count-1]->address;
        int size=data_instructions[data_count-1]->size;
        char size_buf[32];
        int_str(size,size_buf);
        char addr_buf[32];
        uint32_t_str(address,addr_buf);

        emit_instruction("MOV","EAX",addr_buf);
        
        return;
    }
    if (str_eq(node->type,"call")){
        generate_call(node,table);
        return;
    }
    if (str_eq(node->type,"IDENTIFIER")) {
        struct Symbol* symbol=symbol_table_lookup(table,node->identifier);
        int offset=symbol->stack_offset;

        if (!str_eq(symbol->type,"str")) {
            //error
        }

        char offset_buf[32];
        if (offset<0) {
            int_str(-offset,offset_buf);

            emit_instruction("MOV","EAX",str_cat(str_cat("[EBP-",offset_buf),"]"));
        }
        else {
            int_str(offset,offset_buf);

            emit_instruction("MOV","EAX",str_cat(str_cat("[EBP+",offset_buf),"]"));
        }
        return;

    }
    if (str_eq(node->type,"char")){

        int i=str_int(node->value);

        char strb[2];


        char str=(char)i;
        strb[0]=str;
        strb[1]='\0';

        emit_data_instruction(strb);
        uint32_t add=data_instructions[data_count-1];


        char addb[32];
        uint32_t_str(add, addb);


        emit_instruction("MOV","EAX",addb);
        return;
    }
    if (str_eq(node->type,"PLUS")) {
        generate_str(node->left,table);

        emit_instruction("PUSH","EAX","");

        generate_str(node->right,table);

        emit_instruction("MOV","EBX","EAX");
        emit_instruction("POP","EAX","");

        char strcat2[32];

        int_str(STR_CAT_ADDRESS-(global_address+5),strcat2);

        emit_instruction("CALL",strcat2,"");

        return;

    }
    //error



}

char *evaluate_string(struct ASTNode *node) {
    if (str_eq(node->type, "string")) {
        return node->value;
    }
    if (str_eq(node->type, "NUMBER")) {
        //printf("IN NM");
        return node->value;
    }
    if (str_eq(node->type, "char")) {
        return node->value;
    }
    if (str_eq(node->type, "IDENTIFIER")) {
        return find_identifier_value(node->identifier)==NULL?NULL:find_identifier_value(node->identifier);
    }
    if ( str_eq(node->type, "string_concat")) {
        return str_cat(evaluate_string(node->left), evaluate_string(node->right));
    }
    if (str_eq(node->type, "MULTIPLY")) {
        char buf[32];
        int_str(evaluate_const_expr(node->left) * evaluate_const_expr(node->right), buf);
        return str_copy(buf);
    }
    if (str_eq(node->type, "PLUS")) {
        char buf[32];
        int_str(evaluate_const_expr(node->left) + evaluate_const_expr(node->right), buf);
        return str_copy(buf);
    }
    if (str_eq(node->type, "DIVIDE")) {
        char buf[32];
        int_str(evaluate_const_expr(node->left) / evaluate_const_expr(node->right), buf);
        return str_copy(buf);
    }
    if (str_eq(node->type, "MINUS")) {
        char buf[32];
        int_str(evaluate_const_expr(node->left) - evaluate_const_expr(node->right), buf);
        return str_copy(buf);
    }
    //error
    return NULL;
}

_Bool contains_string(struct ASTNode *node,struct SymbolTable* table) {
    if (node == NULL)return false;

    if (str_eq(node->type, "IDENTIFIER")) {
        struct Symbol* symbol=symbol_table_lookup(table,node->identifier);
        if (str_eq(symbol->type,"str")) {
            return true;
        }
        return false;
    }
    if (str_eq(node->type, "string")) {
        return true;
    }
    if (str_eq(node->type, "NUMBER")) {
        return false;
    }
    if (str_eq(node->type, "char")) {
        return false;
    }
    if (str_eq(node->type, "bool"))return false;

    if (node->left) {
        return contains_string(node->left,table);
    }
    if (node->right) {
        return contains_string(node->right,table);
    }
}

/*void emit_error() {
    emit_instruction("MOV", "EAX", "0");
    emit_instruction("INT", "0x80", ""); //maybe
}*/

void emit_epilogue() {
    emit_instruction("MOV", "ESP", "EBP");
    emit_instruction("POP", "EBP", "");
    emit_instruction("RET", "", "");
}

void emit_prologue(int space) {
    emit_instruction("PUSH", "EBP", "");
    emit_instruction("MOV", "EBP", "ESP");


    char space_buffer[32];
    
    int locals_size = space - 4;
    if (locals_size > 0) {
        int_str(locals_size, space_buffer);
        emit_instruction("SUB", "ESP", space_buffer);
    }
}

int get_child_length(struct ASTNode** indexes){
    int num=0;

    while (indexes[num]!=NULL)num++;

    return num;


}

/*
  * takes base address in edi,
  * and takes element size in ebx.
*/
void generate_ptr_index(struct ASTNode**indexes, int dimensions,struct SymbolTable* table){
    //int size=get_child_length(indexes);
    emit_instruction("PUSH","EBX","");
    //emit_instruction("PUSH","ECX","");
    //write_rowcol_text(30,0,str_cat("the dimensions=",quick_int_str(dimensions)),PURPLE);
    for (int i=0;i<dimensions;i++){
        if (indexes[i]==NULL){
            //emit_instruction("POP","ECX","");
            emit_instruction("POP","EBX","");
            emit_instruction("MOV","EAX","EDI");
            

            return;
        }

        if (i<dimensions-1){
            
            generate_expr(indexes[i],table);
            emit_instruction("MOV","ECX","4");
            emit_instruction("IMUL","EAX","ECX");
            emit_instruction("ADD","EAX","EDI");
            

            if (indexes[i+1]!=NULL)
                emit_instruction("MOV","EDI","[EAX]");
            else 
                emit_instruction("MOV","EDI","EAX");

        }
        else {
            
            generate_expr(indexes[i],table);
            //emit_instruction("POP","ECX","");
            emit_instruction("POP","EBX","");
            emit_instruction("IMUL","EAX","EBX");
            emit_instruction("ADD","EAX","EDI");
            return;
            
        }

        
    }


}

void generate_array_index(struct ASTNode**indexes, uint32_t* sizes, uint32_t dimensions,struct SymbolTable* table){
    emit_instruction("MOV","EDI","0");
    for (int i=0;i<dimensions;i++){
        generate_expr(indexes[i],table); // result -> EAX

        //write_rowcol_text(22,0,indexes[i]->value,PURPLE);

        emit_instruction("MOV","EBX","1");
        for (int j=i+1;j<dimensions;j++){
            char sizesbuf[32];
            uint32_t_str(sizes[j],sizesbuf);
            emit_instruction("MOV","ECX",sizesbuf);
            emit_instruction("IMUL","EBX","ECX");
        }

        emit_instruction("IMUL","EAX","EBX");
        emit_instruction("ADD","EDI","EAX");
    }
    emit_instruction("MOV","EAX","EDI");


}

uint32_t calculate_flat_index(struct ASTNode **indexes, uint32_t *sizes, uint32_t dimensions) {
    uint32_t result = 0;

    for (int i = 0; i < dimensions; i++) {
        //printf("I=%d\n", i);
        uint32_t stride = 1;
        for (int j = i + 1; j < dimensions; j++) {
            ///printf("J=%d\n", j);
            uint32_t size = sizes[j];
            stride *= size;
        }

        //printf("ENTERING EVAL VAR EXPR\n");
        int index = evaluate_variable_expr(indexes[i]);
        //printf("LEFT EVAL VAR EXPR\n");
        result += index * stride;
    }
    //printf("\nleft CALC flat index");

    return result;
}

char* get_v_abs_addr(char* id,struct SymbolTable* table){
    struct Symbol *symbol = symbol_table_lookup(table,id);
    char *operand1="EDX";
    if (symbol->is_global){
        char buf[32];
        uint32_t_str(symbol->addr, buf);
        emit_instruction("MOV","EDX",buf);
    }
    else{
        char buf[32];

        if (symbol->stack_offset < 0){
            uint32_t_str(-symbol->stack_offset, buf);
            emit_instruction("MOV","EDX","EBP");
            emit_instruction("SUB","EDX",buf);
            
        }
        else{
            uint32_t_str(symbol->stack_offset, buf);
            emit_instruction("MOV","EDX","EBP");
            emit_instruction("ADD","EDX",buf);
        }
    }
    return operand1;

}

char* get_v_addr(char* id, struct SymbolTable* table){
    struct Symbol *symbol = symbol_table_lookup(table,id);
    char *operand1;
    if (symbol->is_global){
        char buf[32];
        uint32_t_str(symbol->addr, buf);
        emit_instruction("MOV","EDX",buf);
        operand1="[EDX]";
    }
    else{
        char buf[32];

        if (symbol->stack_offset < 0){
            uint32_t_str(-symbol->stack_offset, buf);
            operand1 = str_cat(str_cat("[EBP-", buf), "]");
        }
        else{
            uint32_t_str(symbol->stack_offset, buf);
            operand1 = str_cat(str_cat("[EBP+", buf), "]");
        }
    }
    return operand1;
}

char* get_operand1(struct SymbolTable* table, struct ASTNode* node){
    struct Symbol* id=symbol_table_lookup(table,node->identifier);
    int offset=id->stack_offset;
    char* operand1;
    if (offset < 0) {
        char offset_buf[32];
        int_str(-offset, offset_buf);
        operand1 = str_cat(str_cat("[EBP-", offset_buf), "]");
    } else {
        char offset_buf[32];
        int_str(offset, offset_buf);
        operand1 = str_cat(str_cat("[EBP+", offset_buf), "]");
    }

    if (in_function && symbol_table_lookup_local(table, node->identifier) == NULL) {
        struct SymbolTable *table2 = table;
        struct Symbol *found = NULL;

        while (table2) {
            found = symbol_table_lookup_local(table2, node->identifier);
            if (found) break;
            if (table2 == function_table) break;
            table2 = table2->parent;
        }

        if (found == NULL) {
            char offset_buf[32];
            if (offset < 0) {
                int_str(-offset, offset_buf);
                operand1 = str_cat(str_cat("[EDX-", offset_buf), "]");
            } else {
                int_str(offset, offset_buf);
                operand1 = str_cat(str_cat("[EDX+", offset_buf), "]");
            }
        }
    }

    

    if (str_eq(node->type, "array_assignment")&&!str_contains(id->type,"ptr")) {

        generate_array_index(node->indexes,id->dimensions,id->num_dimensions,table);

        char sizebf[32];
        uint32_t_str(id->element_size,sizebf);
        emit_instruction("MOV","EBX",str_copy(sizebf));
        emit_instruction("IMUL","EAX","EBX");

        if (offset < 0)
        {
            char offset_buf[32];
            int_str(-offset, offset_buf);

            emit_instruction("MOV","ECX","EBP");
            emit_instruction("SUB","ECX",str_copy(offset_buf));
            emit_instruction("SUB","ECX","EAX");
            
            operand1 = "[ECX]";
        }
        else
        {
            char offset_buf[32];
            int_str(offset, offset_buf);

            emit_instruction("MOV","ECX","EBP");
            emit_instruction("ADD","ECX",offset_buf);
            emit_instruction("ADD","ECX","EAX");

            operand1 = "[ECX]";
        }

        if (in_function && symbol_table_lookup_local(table, node->identifier) == NULL)
        {
            struct SymbolTable *table2 = table;
            struct Symbol *found = NULL;

            while (table2)
            {
                found = symbol_table_lookup_local(table2, node->identifier);
                if (found)
                    break;
                if (table2 == function_table)
                    break;
                table2 = table2->parent;
            }

            if (found == NULL)
            {
                char offset_buf[32];
                if (offset < 0)
                {
                    int_str(-offset, offset_buf);
                    operand1 = str_cat(str_cat("[EDX-", offset_buf), "]");
                }
                else
                {
                    int_str(offset, offset_buf);
                    emit_instruction("MOV","ECX","EDX");
                    emit_instruction("SUB","ECX",offset_buf);
                    emit_instruction("SUB","ECX","EAX");
                    operand1 = "[ECX]";
                }
            }
        }

        //operand1="ECX";

        

    }

    if (str_eq(node->type,"array_assignment")&&str_contains(id->type,"ptr")){
        //write_rowcol_text(10,10,"IN IF statement",RED);
        
        emit_instruction("MOV","EDI",operand1);
        char sizebf[32];
        int_str(id->element_size,sizebf);

        emit_instruction("MOV","EBX",str_copy(sizebf));


        generate_ptr_index(node->indexes,id->num_dimensions,table);
        //write_rowcol_text(11,10,"DONE",RED);


        emit_instruction("MOV","ECX","EAX");
        operand1="[ECX]";


    }
    
    if (str_eq(node->type,"dereference")){
        int dim=node->array_dimensions;

        emit_instruction("MOV","ECX",operand1);

        for (int i=1;i<dim;i++){
            emit_instruction("MOV","ECX","[ECX]");

        }

        operand1="[ECX]";

    }
    //write_rowcol_text(30,30,node->type,ORANGE);

    if (str_eq(node->type,"member_access") || str_eq(node->type,"member_ptr_access")){
        //write_rowcol_text(30,30,"IN MEMBER ACCESS",ORANGE);
        generate_member_access(node,table,false);
        emit_instruction("MOV","ECX","EAX");


        operand1="[ECX]";
        



    }
    return operand1;
}


void generate_operation(struct ASTNode *node, struct SymbolTable *table) {

    struct Symbol *id = symbol_table_lookup(table, node->identifier);
    

   // printf("IDTYPE=%s\n", id->type);
    int offset = id->stack_offset;

    char *operand1=get_v_addr(node->identifier,table);

    //write_rowcol_text(25,50-5,str_cat(id->type,str_cat("ELEMENT SIZE ======",quick_int_str(id->element_size))),ORANGE);

    _Bool is_byte= (id->element_size==1 && str_contains(id->type,"_arr")) || id->size==1 || (str_contains(id->type,"ptr") && id->element_size==1 && get_child_length(node->indexes)>=id->num_dimensions);


    if (str_eq(node->type, "array_assignment") && !str_contains(id->type,"ptr")) {

        generate_array_index(node->indexes,id->dimensions,id->num_dimensions,table);

        char sizebf[32];
        uint32_t_str(id->element_size,sizebf);
        emit_instruction("MOV","EBX",str_copy(sizebf));
        emit_instruction("IMUL","EAX","EBX");

        operand1="[ECX]";

        if (id->is_global){
            char buf[32];
            int_str(id->addr,buf);
            emit_instruction("MOV","ECX",buf);
            emit_instruction("ADD","ECX","EAX");

        }
        else{
            if (offset < 0){
                char offset_buf[32];
                int_str(-offset, offset_buf);

                emit_instruction("MOV","ECX","EBP");
                emit_instruction("SUB","ECX",str_copy(offset_buf));
                emit_instruction("SUB","ECX","EAX");
                
            }
            else{
                char offset_buf[32];
                int_str(offset, offset_buf);

                emit_instruction("MOV","ECX","EBP");
                emit_instruction("ADD","ECX",offset_buf);
                emit_instruction("ADD","ECX","EAX");

            }
        }
        

    }

    if (str_eq(node->type,"array_assignment") && str_contains(id->type,"ptr")){
        emit_instruction("MOV","EDI",operand1);

        char sizebf[32];
        int_str(id->element_size,sizebf);

        emit_instruction("MOV","EBX",sizebf);

        generate_ptr_index(node->indexes,id->num_dimensions,table);


        emit_instruction("MOV","ECX","EAX");
        operand1="[ECX]";


    }
    
    if (str_eq(node->type,"dereference_assignment")){
        int dim=node->array_dimensions;

        generate_expr(node->left,table);

        emit_instruction("MOV","ECX","EAX");

        for (int i=1;i<dim;i++){
            emit_instruction("MOV","ECX","[ECX]");
        }

        operand1="[ECX]";

    }
    //write_rowcol_text(30,30,node->type,ORANGE);

    if (str_eq(node->type,"member_access") || str_eq(node->type,"member_ptr_access")){
        //write_rowcol_text(30,30,"IN MEMBER ACCESS",ORANGE);
        generate_member_access(node,table,false);
        emit_instruction("MOV","ECX","EAX");


        operand1="[ECX]";
        



    }

    if (str_eq(node->type,"KEY")){
        uint32_t addr=(uint32_t)(OS_KEY);
        char addb[32];
        uint32_t_str(addr,addb);
        emit_instruction("MOV","ECX",addb);
        node->operation=node->right;

        operand1="[ECX]";
    }


    char *op = node->op; 

    if (node->operation && !str_eq(op,"++") && !str_eq(op,"--")) {
        generate_expr(node->operation,table);
    } 

    if (str_eq(op, "=")) {
        
    } else if (str_eq(op, "+=")) {
        //write_rowcol_text(35,5,str_cat("Offset=",ofb),RED);
        //write_rowcol_text(36,5,operand1,RED);
        
        emit_instruction("MOV","EBX",operand1);
        emit_instruction("ADD","EAX","EBX");

        
    } else if (str_eq(op, "-=")) {
        emit_instruction("MOV","EBX",operand1);
        emit_instruction("SUB","EBX","EAX");
        emit_instruction("MOV","EAX","EBX");
        

    } else if (str_eq(op, "*=")) {
        emit_instruction("PUSH", "EAX", "");
        emit_instruction("MOV", "EAX", operand1);
        emit_instruction("POP", "EBX", "");
        emit_instruction("IMUL", "EAX", "EBX");
        
    } else if (str_eq(op, "/=")) {
        emit_instruction("PUSH", "EAX", "");
        emit_instruction("MOV", "EAX", operand1);
        emit_instruction("CDQ", "", "");
        emit_instruction("POP", "EBX", ""); //10/2
        emit_instruction("IDIV", "EBX", "");
        
    } else if (str_eq(op, "++")) {
        //emit_instruction("XOR","EAX","EAX");
        emit_instruction("MOV", "EAX", operand1);
        emit_instruction("ADD", "EAX", "1");
        
    } else if (str_eq(op, "--")) {
        char ofb[32];
        int_str(offset,ofb);
        //write_rowcol_text(35,5,str_cat("Offset=",ofb),RED);
        //write_rowcol_text(36,5,operand1,RED);
        emit_instruction("MOV", "EAX", operand1);
        emit_instruction("SUB", "EAX", "1");
        


    }
    
    /*if (str_eq(operand4,"BYTE")){
        emit_instruction("MOV",operand1,"AL");
        return;
    }*/

    emit_instruction("MOV", operand1, is_byte?"AL":"EAX"); //[ebp-4], [ebp-5], [ebp-6], [ebp-7], [ebp-8], [ebp-9] <--

}

void patch_if_else_instruction(struct patch_point **asm_points, uint32_t address, uint64_t asm_final) {
    for (uint64_t i = 0; i < asm_final; i++) {
        char addr_buf[32];
        int_str(address-(asm_points[i]->address), addr_buf);

        instructions[asm_points[i]->asm_point]->operand1 = str_copy(addr_buf);
    }
}

uint64_t x86_node_index = 0; //important

struct ASTNode **__current_child_list = NULL;
int __current_child_index = -1;

void patch_jump_instruction(int address, uint64_t asm_point) {
    char addr_buf[32];
    int_str(address, addr_buf);

    instructions[asm_point]->operand1 = str_copy(addr_buf);
}

void flatten_and_store_array(struct ASTNode *node, struct SymbolTable *table, int *offset,struct Symbol* symbol) {

    //printf("IN FLATTEN AND STORE ARRAY\n");
    if (node->elements == NULL) {
        //
        
        generate_expr(node, table);
        
        

        char offset_buf[32];
        if (*offset < 0) {
            int_str(-(*offset), offset_buf);
            emit_instruction("MOV", str_cat(str_cat("[EBP-", offset_buf), "]"), "EAX");
            *offset-=symbol->element_size;
        }
        else {
            int_str(*offset, offset_buf);
            emit_instruction("MOV", str_cat(str_cat("[EBP+", offset_buf), "]"), "EAX");
            *offset+=symbol->element_size;
        }

        return;
    }
    for (int i=0;node->elements[i]!=NULL;i++) {
        flatten_and_store_array(node->elements[i], table, offset,symbol);
    }
}

void generate_call(struct ASTNode* node,struct SymbolTable* table){
    struct Symbol *function = symbol_table_lookup(table, node->identifier);
    struct SymbolTable *functable = function->child_table;
    uint32_t addr = function->func_address;

    int totalsize = 0;

    char addr_buf[32];

    int param_count = function->param_count;

    for (int i = 0; i<node->param_count; i++){
        generate_expr(node->params[node->param_count - 1 - i], table);
        emit_instruction("PUSH", "EAX", "");
    }

    //write_rowcol_text(10,30,str_cat("PARAM COUNT = ", quick_int_str(param_count)),BLUE);

    uint32_t call_addr = global_address;
    int_str(addr - (call_addr + 5), addr_buf);

    emit_instruction("CALL", addr_buf, "");

    if (node->param_count > 0){
        char cleanup_buf[32];
        int_str(node->param_count*4, cleanup_buf);
        write_rowcol_text(13,13,str_cat("The param count * 4 when calling this function was == ", cleanup_buf),GREEN);
        emit_instruction("ADD", "ESP", cleanup_buf);
    }
}


void access_global_variable_from_func(struct Symbol* symbol,struct SymbolTable* table){
    int offset=symbol->stack_offset;

    char offset_buf[32];
    int_str(-offset,offset_buf);

    emit_instruction("MOV","EAX","[EBP]");
    emit_instruction("MOV","EBX",str_cat(str_cat("[EAX-",offset_buf),"]"));
    emit_instruction("MOV","EAX","EBX");


}

void generate_instruction(struct ASTNode *node, struct SymbolTable *table) {
    //printf("NODE TYPE IN GEN INSTRUCTION=%s\n",node->type);

    /*if (node->identifier&&!str_contains(node->type,"decl")) {
        struct Symbol* proposed_identifier=symbol_table_lookup(table,node->identifier);

        if (proposed_identifier==NULL) {
            emit_error("REFERENCE ERROR",node->identifier);
        }
    }*/

    if (node == NULL)return;

    //write_rowcol_text(asm_count+3,36,node->type,ORANGE);


    if (str_eq(node->type, "int_decl")) {
        generate_expr(node->right, table);

        char* operand1=get_v_addr(node->identifier,table);

        emit_instruction("MOV",operand1,"EAX");

    } else if (str_eq(node->type, "prnt") || str_eq(node->type, "prntnln")) {//EAX= interrupt type. EDI=param 1, ESI=param 2 etc...
        /*generate_expr(node->right,table);
        emit_instruction("INT","100","");
        return;*/


        generate_expr(node->right,table);
        emit_instruction("XOR","EDI","EDI");
        emit_instruction("MOV","EDI","EAX");//EDI contains string pointer

        if (str_eq(node->type,"prnt")) {
            emit_instruction("MOV","ESI","0");//0 for prnt
        }
        else {
            emit_instruction("MOV","ESI","1");//1 for \n at end (prntnln)
        }

        emit_instruction("MOV","EAX","0");//0 for output interrupt type
        emit_instruction("INT","100","");



    } else if (str_eq(node->type, "char_decl")) {
        char* operand1=get_v_addr(node->identifier,table);
        generate_expr(node->right,table);
        emit_instruction("MOV",operand1,"AL");

    } else if (str_eq(node->type, "bool_decl")) {
        char* operand1=get_v_addr(node->identifier,table);
        generate_expr(node->right,table);
        emit_instruction("MOV",operand1,"AL");
    } 
    else if(str_eq(node->type,"float_decl")){

    }
    else if (str_eq(node->type, "function")) {
        if (in_function){
            emit_error("SEMANTIC ERROR","cannot have a nested function");
            return;
        }

        in_function=true;
        

        struct Symbol *func = symbol_table_lookup(table, node->identifier);
        
        current_function=func;

        emit_instruction("JMP","-1","");
        int asm_point=asm_count-1;

        func->func_address = global_address;

        struct SymbolTable *child_table = func->child_table;

        function_table=child_table;

        int space = func->locals_offset;

        emit_prologue(space);


        for (int i = 0; node->children[i] != NULL; i++) {
            __current_child_list = node->children;
            __current_child_index = i;
            
            generate_instruction(node->children[i], child_table);
        }
        __current_child_list = NULL;
        __current_child_index = -1;



        emit_epilogue();
        uint32_t current_address=global_address;
        instructions[asm_point]->operand1=quick_int_str(current_address-func->func_address);
        current_function=NULL;
        in_function=false;

    } else if (str_eq(node->type, "return")) {

        if (node->right){
            generate_expr(node->right,table);
            if (str_eq(node->right->type,"IDENTIFIER")){
                struct Symbol* symbol=symbol_table_lookup(table,node->right->identifier);
                if (str_contains(symbol->type,"struct")){
                    current_function->struct_name=symbol->struct_name;
                    
                }
            }
        }
        emit_epilogue();

    } 
    else if (str_eq(node->type, "call")) {
        generate_call(node,table);

    } else if (str_eq(node->type, "operation")) {

        generate_operation(node, table);

    } else if (str_eq(node->type, "for")) {
        
        //for(int i=0;i<10;i+=x|i++){}
        
        struct ASTNode *init = node->fordeclaration;
        struct ASTNode *condition = node->forcondition;
        struct ASTNode *operation = node->forincrement;

        struct SymbolTable *for_table = node->scope;

        struct Symbol *init_symbol = symbol_table_lookup(for_table, init->identifier);

        generate_expr(init->right, for_table);

        char* operand1=get_v_addr(init->identifier,for_table);

        emit_instruction("MOV", operand1, "EAX");

        uint32_t start_point = global_address;

        
        ///debug///
        char stp[32];
        uint32_t_str(start_point,stp);
        //write_rowcol_text(20,5,str_cat("STARTPOINT=",stp),RED);
        ///debug///

        char sp[32];

        generate_expr(condition, for_table);

        emit_instruction("CMP", "EAX", "0");
        
       
        emit_instruction("JE", "-1", ""); //patchable instruction


        uint32_t je_addr=global_address;
        
        uint64_t asm_point = asm_count - 1;

        

        struct break_point** break_points=MALLOC(512*sizeof(struct break_point));
        struct break_point** continue_points=MALLOC(512*sizeof(struct break_point));

        int continue_index=0;
        int break_index=0;
        
        struct ASTNode** save1=__current_child_list;
        int save2=__current_child_index;
        if (node->children[0]!=NULL){
            for (int i = 0; node->children[i] != NULL; i++) {
                __current_child_list = node->children;
                __current_child_index = i;

                if (str_eq(node->children[i]->type,"continue")){
                    emit_instruction("JMP","-1","");

                    continue_points[continue_index]=MALLOC(sizeof(struct break_point));
                    continue_points[continue_index]->address=global_address;
                    continue_points[continue_index]->index=asm_count-1;
                    continue;
                }


                if (str_eq(node->children[i]->type,"break")){
                    emit_instruction("JMP","-1","");

                    break_points[break_index]=MALLOC(sizeof(struct break_point));
                    break_points[break_index]->address=global_address;
                    break_points[break_index]->index=asm_count-1;
                    break_index++;
                    continue;

                }
                
                generate_instruction(node->children[i], for_table);
            }
        }
        __current_child_list =save1;
        __current_child_index = save2;

        generate_operation(operation, for_table);


        emit_instruction("JMP", "-1", "");
        uint32_t jmp_addr = global_address; // address after JMP
        uint64_t jmp_asm_point = asm_count - 1;

        uint32_t end_point = global_address;

        patch_jump_instruction(end_point - je_addr, asm_point);

        patch_jump_instruction(start_point - jmp_addr, jmp_asm_point);

        for (int i=0;i<break_index;i++){
            instructions[break_points[i]->index]->operand1=quick_int_str(global_address-break_points[i]->address);
        }

        for (int i=0;i<continue_index;i++){
            instructions[continue_points[i]->index]->operand1=quick_int_str(start_point-continue_points[i]->address);
        }

    } 

    else if(str_eq(node->type,"fori")){
        
        struct SymbolTable* for_table=node->scope;

        struct Symbol* init_symbol=symbol_table_lookup(for_table,"i");


        char* operand1=get_v_addr("i",for_table);
        
        emit_instruction("MOV",operand1,"0");


        struct ASTNode* condition=MALLOC(sizeof(struct ASTNode));
        condition->type="<";
        condition->left=MALLOC(sizeof(struct ASTNode));
        condition->left->type="IDENTIFIER";
        condition->left->identifier="i";
        condition->right=node->right;

        //emit_instruction("PUSH","ESP","");

        uint32_t start=global_address;

        generate_expr(condition,for_table);

        //write_rowcol_text(27,22,"Out of gen expr",BLUE);

        emit_instruction("CMP","EAX","0");
        emit_instruction("JE","-1","");

        uint32_t jmpstart=global_address;

        int asmp=asm_count-1;

        struct ASTNode** save1=__current_child_list;
        int save2=__current_child_index;
        for (int i=0;node->children[i]!=NULL;i++){
            __current_child_list = node->children;
            __current_child_index = i;
            
            generate_instruction(node->children[i],for_table);

        }
        __current_child_list = save1;
        __current_child_index = save2;

        operand1=get_v_addr("i",for_table);
        emit_instruction("ADD",operand1,"1");

        emit_instruction("JMP","-1","");



        uint32_t end=global_address;
        char buf3[32];

        //emit_instruction("POP","ESP","");

        int_str(start-end,buf3);
        instructions[asm_count-1]->operand1=str_copy(buf3);

        char buf2[32];
        int_str(end-jmpstart,buf2);
        instructions[asmp]->operand1=str_copy(buf2);
        


    }
    else if (str_eq(node->type, "if")) {

        struct patch_point** patch_points;
        
        struct ASTNode *condition = node->condition;

        struct SymbolTable *if_table = node->scope;

        generate_expr(condition, table);
        emit_instruction("CMP", "EAX", "0");

        

        emit_instruction("JE", "-1", ""); //patchable instruction
        uint32_t start_point=global_address;

        uint64_t asm_point = asm_count - 1; //patch point

        uint64_t asm_points_capacity = 64;
        int asm_points_count = 0;

        struct patch_point **asm_points = MALLOC(asm_points_capacity * sizeof(struct patch_point*));

        //.printf("!1");

        struct ASTNode** save1=__current_child_list;
        int save2=__current_child_index;
        for (int i = 0; node->children[i] != NULL; i++) {
            //printf("1");
            __current_child_list = node->children;
            __current_child_index = i;
            generate_instruction(node->children[i], if_table);
        }

        __current_child_index=save2;
        __current_child_list=save1;
        
        uint32_t if_patch_point_address=global_address;

        emit_instruction("JMP", "-2", "");

        uint32_t end_point = global_address;

        patch_jump_instruction(end_point-(start_point), asm_point);

        struct patch_point* if_patch_point=MALLOC(sizeof(struct patch_point));
        if_patch_point->address=end_point;
        if_patch_point->asm_point=asm_count-1;

        asm_points[asm_points_count++] = if_patch_point;

        uint64_t ni = x86_node_index + 1;

        //printf("B4 while");
        while (nodes[ni]!=NULL&&str_eq(nodes[ni]->type, "elif")) {
            //write_rowcol_text(y++,20,"ELIFF_____",PURPLE);

            //printf("1\n");
            struct ASTNode *elifnode = nodes[ni++];
            x86_node_index++;

            struct ASTNode *elif_condition = elifnode->condition;

            struct SymbolTable *elif_table = elifnode->scope;

            generate_expr(elif_condition, elif_table);

            emit_instruction("CMP", "EAX", "0");

           

            emit_instruction("JE", "-1", "");
            uint32_t elif_entry=global_address;

            uint64_t elif_point = asm_count - 1;

            struct ASTNode** save1=__current_child_list;
            int save2=__current_child_index;
            for (int i = 0; elifnode->children[i] != NULL; i++) {
                __current_child_list = elifnode->children;
                __current_child_index = i;
                generate_instruction(elifnode->children[i], elif_table);
            }
            __current_child_list = save1;
            __current_child_index = save2;

            
            emit_instruction("JMP", "-2", "");
            uint32_t elif_patch_addr=global_address;

            struct patch_point* elif_patch_point=MALLOC(sizeof(struct patch_point));

            elif_patch_point->address=elif_patch_addr;
            elif_patch_point->asm_point=asm_count-1;

            asm_points[asm_points_count++] = elif_patch_point;

            uint32_t ep = global_address;
            patch_jump_instruction(ep-(elif_entry), elif_point);

        }


        while (__current_child_list!=NULL&&str_eq(__current_child_list[__current_child_index+1]->type,"elif")) {

            //write_rowcol_text(y++,20,"ELIFF_____",PURPLE);

            //printf("1\n");
            struct ASTNode *elifnode = __current_child_list[1+__current_child_index++];
            //x86_node_index++;

            struct ASTNode *elif_condition = elifnode->condition;

            struct SymbolTable *elif_table = elifnode->scope;

            generate_expr(elif_condition, elif_table);

            emit_instruction("CMP", "EAX", "0");

           

            emit_instruction("JE", "-1", "");
            uint32_t elif_entry=global_address;

            uint64_t elif_point = asm_count - 1;


            struct ASTNode** save1=__current_child_list;
            int save2=__current_child_index;
            for (int i = 0; elifnode->children[i] != NULL; i++) {
                __current_child_list = elifnode->children;
                __current_child_index = i;
                generate_instruction(elifnode->children[i], elif_table);
            }
            __current_child_list = save1;
            __current_child_index = save2;

            
            emit_instruction("JMP", "-2", "");
            uint32_t elif_patch_addr=global_address;

            struct patch_point* elif_patch_point=MALLOC(sizeof(struct patch_point));

            elif_patch_point->address=elif_patch_addr;
            elif_patch_point->asm_point=asm_count-1;

            asm_points[asm_points_count++] = elif_patch_point;

            uint32_t ep = global_address;
            patch_jump_instruction(ep-(elif_entry), elif_point);

        }




        //printf("AFTer while");
        if (__current_child_list != NULL) {
            if (__current_child_index + 1 >= 0 && __current_child_list[__current_child_index + 1] != NULL && str_eq(__current_child_list[__current_child_index + 1]->type, "else")) {
                __current_child_index++;
                
                struct ASTNode *else_node = __current_child_list[__current_child_index];
                struct SymbolTable *else_table = else_node->scope;
                for (int i = 0; else_node->children[i] != NULL; i++) {
                    generate_instruction(else_node->children[i], else_table);
                }
                uint32_t final = global_address;
                patch_if_else_instruction(asm_points, final, asm_points_count);
                return;
            }
        } else {
            if (nodes[x86_node_index+1]!=NULL) {
                if (str_eq(nodes[x86_node_index + 1]->type, "else")) {
                    
                    x86_node_index++;

                    struct ASTNode *else_node = nodes[x86_node_index];

                    struct SymbolTable *else_table = else_node->scope;


                    struct ASTNode** save1=__current_child_list;
                    int save2=__current_child_index;
                    for (int i = 0; else_node->children[i] != NULL; i++) {
                        __current_child_list=else_node->children;
                        __current_child_index=i;
                        generate_instruction(else_node->children[i], else_table);
                    }
                    __current_child_index=save2;
                    __current_child_list=save1;

                    uint32_t final = global_address;

                    patch_if_else_instruction(asm_points, final, asm_points_count);
                    return;
                }
            }
        }
        //printf("ELSE");
        uint32_t final = global_address;
        patch_if_else_instruction(asm_points, final, asm_points_count);

    } else if (str_eq(node->type, "while")) {
        struct ASTNode *condition = node->condition;

        struct SymbolTable *while_table = node->scope;

        uint32_t entry = global_address;

        generate_expr(condition, table);


        emit_instruction("CMP", "EAX", "0");

        uint32_t comparison_addr = global_address;
        emit_instruction("JE", "-1", "");
        uint32_t je_addr = global_address; // address after JE
        uint64_t je = asm_count - 1;


        struct ASTNode** save1=__current_child_list;
        int save2=__current_child_index;
        for (int i = 0; node->children[i] != NULL; i++) {
            __current_child_list=node->children;
            __current_child_index=i;

            generate_instruction(node->children[i], while_table);
        }
        __current_child_list=save1;
        __current_child_index=save2;

        emit_instruction("JMP", "-1", "");
        int asmp=asm_count-1;

        uint32_t exit = global_address; // address after JMP

        // compute offsets relative to the address following the branch instruction
        patch_jump_instruction(entry - exit, asmp);
        patch_jump_instruction(exit - je_addr, je);

    }
    else if (str_eq(node->type,"dereference_assignment")){
        generate_operation(node,table);
        return;

    }
    else if (str_eq(node->type, "array_assignment")) {
        //printf("12345aa");
        struct Symbol *symbol = symbol_table_lookup(table, node->identifier);
        //printf("after");

        generate_operation(node, table);

    } else if (str_eq(node->type, "int_arr_decl") || str_eq(node->type, "char_arr_decl")
               || str_eq(node->type, "str_arr_decl") || str_eq(node->type, "bool_arr_decl")) {

        char *type = node->type;


        //printf("IN GENERARE ARRAY INSTRUCTIONS\n");

        struct Symbol *symbol = symbol_table_lookup(table, node->identifier);

        int offset = symbol->stack_offset;

        int size = (int) symbol->size;

        char size_buf[32];

        uint32_t_str(size, size_buf);

        /*if (!in_function)
            emit_instruction("SUB", "ESP", str_copy(size_buf));*/

        if (node->right && !symbol->is_global) {
            if (node->right->elements[0] != NULL) {
                int current_offset = offset;
                //printf("ENTERING FASA");
                flatten_and_store_array(node->right, table, &current_offset,symbol);
            } else {
                //?e?r?r?o?r?
            }
        }
    }
    else if(str_contains(node->type,"ptr_decl")){        
        generate_expr(node->right,table);

        char*operand1=get_v_addr(node->identifier,table);

        emit_instruction("MOV",operand1,"EAX");
        
    }
    else if (str_eq(node->type,"inint")) {

        emit_instruction("MOV","EAX","1");//EAX=interrupt type
        emit_instruction("MOV","EDI","0");//param1 (0) therefore = data type

        emit_instruction("INT","100","");

    }
    else if (str_eq(node->type,"inchar")) {

    }
    else if (str_eq(node->type,"instr")) {
        struct Symbol* symbol=symbol_table_lookup(table,node->identifier);

        int offset=symbol->stack_offset;

        emit_instruction("MOV","EAX","1");
        emit_instruction("MOV","EDI","1");//1 for string data type
        emit_instruction("INT","100","");

    }
    else if (str_eq(node->type,"wdraw")) {
        generate_expr(node->param1,table);
        emit_instruction("PUSH","EAX","");
        generate_expr(node->param2,table);
        emit_instruction("POP","EBX","");

        emit_instruction("ADD","EAX","50");


        char pprbb[32];
        char fbb[32];
        int_str(ppr*4,pprbb);
        uint32_t_str((uint32_t)framebuffer,fbb);

        emit_instruction("MOV","ESI","1024");
        emit_instruction("IMUL","EAX","ESI");
        emit_instruction("ADD","EAX","EBX");
        emit_instruction("MOV","EBX","4");
        emit_instruction("IMUL","EAX","EBX");

        emit_instruction("MOV","EBX",fbb);
        emit_instruction("ADD","EAX","EBX");
        emit_instruction("MOV","EDI","EAX");
        
        emit_instruction("PUSH","EDI","");

        generate_expr(node->param3,table);
        emit_instruction("MOV","ECX","EAX");
        emit_instruction("PUSH","ECX","");
        generate_expr(node->param4,table);
        emit_instruction("POP","ECX","");

        emit_instruction("MOV","ESI","EAX");

        emit_instruction("MOV","EAX","[0xccc000]");

        emit_instruction("POP","EDI","");

        uint32_t start=global_address;

        emit_instruction("PUSHAD","","");

        

        emit_instruction("REP","STOSD","");

        emit_instruction("POPAD","","");
        
        emit_instruction("SUB","ESI","1");

        emit_instruction("CMP","ESI","0");
        emit_instruction("JLE","1","");

        int i1=asm_count-1;
        uint32_t a1=global_address;

        emit_instruction("ADD","EDI","4096");
        emit_instruction("JMP","1","");
        instructions[asm_count-1]->operand1=quick_int_str(start-global_address);
        instructions[i1]->operand1=quick_int_str(global_address-a1);

        return;
        
        /*char framead[32];
        uint32_t_str(framebuffer_add,framead);
        char pprbuf[32];
        int_str(ppr*4,pprbuf);


        generate_expr(node->param4,table);//sp=height
        emit_instruction("PUSH","EAX","");//sp=width
        generate_expr(node->param3,table);//sp=y
        emit_instruction("PUSH","EAX","");//sp=x
        generate_expr(node->param2,table);
        emit_instruction("PUSH","EAX","");
        generate_expr(node->param1,table);
        emit_instruction("PUSH","EAX","");

        char adb[32];
        

        emit_instruction("CALL","-1","");//sp=RETADDRESS
        

        int_str(DRAW_ADDRESS-global_address,adb);

        //write_rowcol_text(17,0,adb,RED);
        instructions[asm_count-1]->operand1=str_copy(adb);//start*/
        

    }
    else if(str_eq(node->type,"wfill")){
        

    }
    else if(str_eq(node->type,"drawcircle")){

    }
    else if(str_eq(node->type,"fillcircle")){

    }
    else if(str_eq(node->type,"line")){

        generate_expr(node->param2,table);
        emit_instruction("PUSH","EAX","");
        generate_expr(node->param1,table);
        emit_instruction("PUSH","EAX","");

        emit_instruction("CALL","-1","");

        char adrbuf[32];
        int_str(DRAW_LINE_ADDRESS-global_address,adrbuf);
        instructions[asm_count-1]->operand1=str_copy(adrbuf);


    }
    else if (str_eq(node->type,"colour")) {
        struct Symbol* symbol=symbol_table_lookup(table,"colour");



        int offset=symbol->stack_offset;

        char offset_buf[32];

        int_str(-offset,offset_buf);


        

        generate_expr(node->param1,table);

        emit_instruction("SHL","EAX","16");
        emit_instruction("PUSH","EAX","");

        generate_expr(node->param2,table);

        emit_instruction("SHL","EAX","8");
        emit_instruction("PUSH","EAX","");

        generate_expr(node->param3,table);
        
        emit_instruction("POP","EBX","");
        emit_instruction("OR","EAX","EBX");
        emit_instruction("POP","EBX","");
        emit_instruction("OR","EAX","EBX");

        emit_instruction("MOV","[0xCCC000]","EAX");

        /*if (in_function){
            emit_instruction("MOV", str_cat(str_cat("[EDX-", offset_buf), "]"), "EAX");
            return;
        }

        emit_instruction("MOV",str_cat(str_cat("[EBP-",offset_buf),"]"),"EAX");*/


    }
    else if(str_eq(node->type,"dereference")){
        generate_operation(node,table);
    }

    else if(str_eq(node->type,"sleep")){
        generate_expr(node->right,table);
        emit_instruction("MOV","EBX","EAX");
        emit_instruction("MOV","EAX","2");
        emit_instruction("INT","100","");
    }
    else if(str_eq(node->type,"free")){
        struct Symbol* symbol=symbol_table_lookup(table,node->right->identifier);
        
        int offset=symbol->stack_offset;

        char buf[32];
        char* operand1;

        if (offset<0){
            int_str(-offset,buf);
            emit_instruction("MOV","EAX",str_cat(str_cat("[EBP-",buf),"]"));

        }
        else{
            int_str(offset,buf);
            emit_instruction("MOV","EAX",str_cat(str_cat("[EBP+",buf),"]"));

        }


        if (in_function && symbol_table_lookup_local(table, node->identifier) == NULL){
            struct SymbolTable *table2 = table;
            struct Symbol *found = NULL;

            while (table2)
            {
                found = symbol_table_lookup_local(table2, node->identifier);
                if (found)
                    break;
                if (table2 == function_table)
                    break;
                table2 = table2->parent;
            }

            if (found == NULL)
            {
                char offset_buf[32];
                int_str(-offset, offset_buf);
                emit_instruction("MOV","EAX",str_cat(str_cat("[EDX-",offset_buf),"]"));
                
                
                
            }
        }

        //emit_instruction("MOV","EAX",operand1);

        char abuf[32];
        emit_instruction("PUSH","EBP","");
        emit_instruction("MOV","EBP","ESP");

        emit_instruction("PUSH","EAX","");


        emit_instruction("CALL",abuf,"");
        int_str(((uint32_t)(&FREE))-global_address,abuf);
        instructions[asm_count-1]->operand1=abuf;

        emit_instruction("MOV","ESP","EBP");
        emit_instruction("POP","EBP","");

    }
    else if(str_eq(node->type,"struct_ptr")){
        struct Symbol* symbol=symbol_table_lookup(table,node->identifier);
        int offset=symbol->stack_offset;
        char ob[32];
        int_str(-offset,ob);
        char* operand1=get_v_addr(node->identifier,table);

        if (node->right){
            generate_expr(node->right,table);

            emit_instruction("MOV",operand1,"EAX");
            
        }
    }
    else if(str_eq(node->type,"member_access")||str_eq(node->type,"member_ptr_access")){
        generate_operation(node,table);
    }
    else if(str_eq(node->type,"asm")){
        char* mnemonic=node->value;
        char* operand1=NULL;
        char* operand2=NULL;


        if (node->operand1==NULL){
            emit_instruction(mnemonic,"","");
            return;
        }
        

        if (str_eq(node->operand1->op,"mem")){
            operand1=str_cat(str_cat("[",node->operand1->value),"]");

        }
        else if (str_eq(node->operand1->type,"asm")||str_eq(node->operand1->type,"imm")){
            write_rowcol_text(10,3,node->operand1->value,RED);
            operand1=node->operand1->value;

        }
        else
        {
            if (node->operand2==NULL){
                generate_expr(node->operand1,table);
                operand1="EAX";
            }
            else{
                operand1=get_operand1(table,node->operand1);
            }
        }

        if(node->operand2==NULL){
            emit_instruction(mnemonic,operand1, "");
            struct Symbol* s=symbol_table_lookup(table,node->operand1->identifier);
            if (str_eq(s->type,"label")){
                instructions[asm_count-1]->operand1=quick_int_str(s->func_address-global_address);
            }
            return;
        }

        if (str_eq(node->operand2->op,"mem")){
            operand2=str_cat(str_cat("[",node->operand2->value),"]");
        }
        else if (str_eq(node->operand2->type,"asm")||str_eq(node->operand2->type,"imm")){
            operand2=node->operand2->value;

        }
        else{
            if (str_contains(node->operand1->value,"EAX"))
                emit_instruction("PUSH","EAX","");
            
            generate_expr(node->operand2,table);
            
            if (str_contains(node->operand1->value,"EAX")){
                emit_instruction("MOV","EBX","EAX");
                emit_instruction("POP","EAX","");
                operand2="EBX";
                
            }
            else{
                operand2="EAX";
            }


            

            

        }

        emit_instruction(mnemonic,operand1,operand2);


    }
    else if(str_eq(node->type,"LABEL")){
        struct Symbol* sym=symbol_table_lookup(table,node->identifier);

        sym->func_address=global_address;
        return;
    }

    else if(str_eq(node->type,"KEY")){
        generate_operation(node,table);

    }

    else if(str_eq(node->type,"__free")){
        uint32_t addr=(uint32_t)(&FREE);

        generate_expr(node->right,table);
        emit_instruction("PUSH","EAX","");

        emit_instruction("CALL","-1","");
        char addrbuf[32];
        uint32_t_str(addr-global_address,addrbuf);
        instructions[asm_count-1]->operand1=str_copy(addrbuf);
        emit_instruction("ADD","ESP","4");
    }

    else if (str_eq(node->type,"goto")){
        generate_expr(node->right,table);
        emit_instruction("JMP","EAX","");

    }

    else if (str_eq(node->type,"mainloop")){
        char buf[32];
        uint32_t_str((uint32_t)&MAIN_LOOP,buf);

        emit_instruction("MOV","EAX",buf);
        emit_instruction("STI","","");
        emit_instruction("CALL","EAX","");
    }

    
}

void generate_x86() {
    asm_capacity = 100;

    instructions = MALLOC(asm_capacity * sizeof(struct x86_instruction));
    data_instructions = MALLOC(data_capacity * sizeof(struct x86_instruction));
    asm_count = 0;
    data_count = 0;

    global_address = BASE_ADDRESS;
    address_offset = 0;

    /*emit_instruction("PUSH","EBP","");
    emit_instruction("MOV","EBP","ESP");
    //emit_instruction("SUB","ESP","32");

    /*emit_instruction("MOV","EDI","[ESP]");
    emit_instruction("INT","100","");*/


    emit_instruction("MOV","EDX","EBP");

    emit_instruction("MOV","ECX","0x55555");

    char hob[32];
    uint32_t_str((uint32_t)heap_offset,hob);

    emit_instruction("MOV","EBX","0x50000");
    emit_instruction("MOV","EBX","[EBX]");
    emit_instruction("MOV","[ECX]","EBX");

    emit_instruction("MOV","EBX","0x145000");
    emit_instruction("MOV","[EBX]","0");


    for (x86_node_index = 0; x86_node_index<nodes_index;x86_node_index++) {
        generate_instruction(nodes[x86_node_index], global_table);
    }

    /*emit_instruction("MOV","ESP","EBP");
    emit_instruction("POP","EBP","");


    /*emit_instruction("MOV","EDI","[ESP]");
    emit_instruction("INT","100","");*/
    
    char tas[32];
    int_str(total_array_size,tas);
    emit_instruction("ADD","ESP",tas);
    emit_instruction("RET","","");

    if (asm_count < asm_capacity) {
        instructions[asm_count] = NULL;
    }
    if (data_count < data_capacity) {
        data_instructions[data_count] = NULL;
    }
    //asm_count=0;
    data_count=0;
    x86_node_index=0;
    //printf("\n\n FINAL ADDRESS=%u", fake_address);
}

void generate_data() {
    for (int i=0;data_instructions[i]!=NULL;i++) {
        uint8_t* addr=(uint8_t*)data_instructions[i]->address;

        int ind=0;
        for (;data_instructions[i]->value[ind]!='\0';ind++) {
            addr[ind]=data_instructions[i]->value[ind];
            
        }
        addr[ind]='\0';
    }

}

void generate_strcpy() {
    uint32_t curr_address=STR_CPY_ADDRESS;

    struct x86_instruction** helper_instructions=MALLOC(64*sizeof(struct x86_instruction*));

    int ind=0;

    emit_helper_instruction("PUSH","ESI","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("PUSH","EDI","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","EDI","EAX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","ESI","EAX",helper_instructions,&ind,&curr_address);

    helper_instructions[ind] = NULL;

    struct helper_function* helper_func=MALLOC(sizeof(struct helper_function));

    helper_func->name="malloc";
    helper_func->address=MALLOC_ADDRESS;
    helper_func->instructions=helper_instructions;

    helpers[helpers_count++]=helper_func;
    emit_helper_instruction("TEST","AL","AL",helper_instructions,&ind,&curr_address);

    //int_str(loop-(curr_address+6), addr_buf);

    //emit_helper_instruction("JNZ",addr_buf,"",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("POP","EDI","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("POP","ESI","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("RET","","",helper_instructions,&ind,&curr_address);

    helper_instructions[ind] = NULL;

    struct helper_function* helper_function=MALLOC(sizeof(struct helper_function));
    helper_function->name="strcpy";
    helper_function->address=STR_CPY_ADDRESS;
    helper_function->instructions=helper_instructions;

    helpers[helpers_count++] = helper_function;

}



void generate_strlen() {
    //printf("ENTERED GENSTRLEN\n");
    uint32_t curr_address=STR_LEN_ADDRESS;


    struct x86_instruction** helper_instructions=MALLOC(64*sizeof(struct x86_instruction*));
    int ind=0;

    //printf("ENTERED GEN STRLEN\n");

    emit_helper_instruction("PUSH","EBX","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","EBX","EAX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("XOR","EAX","EAX",helper_instructions,&ind,&curr_address);

    uint32_t start_addr=curr_address;
    emit_helper_instruction("MOV","AL","[EBX]",helper_instructions,&ind,&curr_address);
    int start=ind-1;
    emit_helper_instruction("TEST","AL","AL",helper_instructions,&ind,&curr_address);//test \0
    emit_helper_instruction("JE","-1","",helper_instructions,&ind,&curr_address);

    uint32_t je_addr=curr_address;

    int je=ind-1;

    emit_helper_instruction("ADD","EAX","1",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("ADD","EBX","1",helper_instructions,&ind,&curr_address);

    char start_buf[32];
    //printf("BEFORE UINT_T_STR GEN STRLEN\n");
    uint32_t_str(start_addr-(curr_address+5),start_buf);
    //printf("AFTER UINT32_T_STR GEN STRLEN\n");

    emit_helper_instruction("JMP",start_buf,"",helper_instructions,&ind,&curr_address);

    uint32_t end_addr=curr_address;
    char end_addr_buf[32];
    int_str(end_addr-je_addr,end_addr_buf);
    ///printf("ADTER SECOND UINT32_t_str=%u\n",end_addr);

    ///printf("HELPER INSTRUCTIONS JE=%p\n",helper_instructions[je]);
    helper_instructions[je]->operand1=str_copy(end_addr_buf);
    //printf("AFTER STRCOPY\n");

    emit_helper_instruction("POP","EBX","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("RET","","",helper_instructions,&ind,&curr_address);

    helper_instructions[ind] = NULL;

    struct helper_function* helper_func=MALLOC(sizeof(struct helper_function));

    helper_func->name="strlen";
    helper_func->address=STR_LEN_ADDRESS;
    helper_func->instructions=helper_instructions;

    helpers[helpers_count++]=helper_func;



}

void generate_strcat() {
    uint32_t curr_address=STR_CAT_ADDRESS;

    struct x86_instruction** helper_instructions=MALLOC(64*sizeof(struct x86_instruction*));

    int ind=0;

    emit_helper_instruction("PUSH","EAX","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("PUSH","EBX","",helper_instructions,&ind,&curr_address);

    char strlenbuf[32];
    int_str(STR_LEN_ADDRESS-(curr_address+5),strlenbuf);

    emit_helper_instruction("CALL",strlenbuf,"",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","ECX","EAX",helper_instructions,&ind,&curr_address);

    emit_helper_instruction("MOV","EAX","[ESP]",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("CALL",strlenbuf,"",helper_instructions,&ind,&curr_address);

    emit_helper_instruction("ADD","EAX","ECX",helper_instructions,&ind,&curr_address);

    char mallocbuf[32];
    

    emit_helper_instruction("ADD","EAX","1",helper_instructions,&ind,&curr_address);
    int_str(MALLOC_ADDRESS-(curr_address+5),mallocbuf);
    emit_helper_instruction("CALL",mallocbuf,"",helper_instructions,&ind,&curr_address);

    emit_helper_instruction("MOV","EDI","EAX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("POP","EBX","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("POP","ESI","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("PUSH","EDI","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","EAX","EDI",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","EBX","ESI",helper_instructions,&ind,&curr_address);

    char strcpy[32];
    int_str(STR_CPY_ADDRESS-(curr_address+5),strcpy);

    emit_helper_instruction("CALL",strcpy,"",helper_instructions,&ind,&curr_address);

    emit_helper_instruction("POP","EAX","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("ADD","EAX","ECX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("POP","EBX","",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("CALL",strcpy,"",helper_instructions,&ind,&curr_address);

    emit_helper_instruction("SUB","EAX","ECX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("RET","","",helper_instructions,&ind,&curr_address);

    helper_instructions[ind] = NULL;

    struct helper_function* helper_func=MALLOC(sizeof(struct helper_function));

    helper_func->name="strcat";
    helper_func->address=STR_CAT_ADDRESS;
    helper_func->instructions=helper_instructions;

    helpers[helpers_count++]=helper_func;

}

/*void generate_malloc() {
    uint32_t curr_address=MALLOC_ADDRESS;

    struct x86_instruction** helper_instructions=MALLOC(64*sizeof(struct x86_instruction*));

    int ind=0;

    char addr_buf[32];
    uint32_t_str(&heap,addr_buf);



    emit_helper_instruction("MOV","EBX",str_cat(str_cat("[",addr_buf),"]"),helper_instructions,&ind,&curr_address);
    emit_helper_instruction("ADD",str_cat(str_cat("[",addr_buf),"]"),"EAX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("MOV","EAX","EBX",helper_instructions,&ind,&curr_address);
    emit_helper_instruction("RET","","",helper_instructions,&ind,&curr_address);


    struct helper_function* helper_func=MALLOC(sizeof(struct helper_function));

    helper_func->name="malloc";
    helper_func->address=MALLOC_ADDRESS;
    helper_func->instructions=helper_instructions;

    helpers[helpers_count++]=helper_func;

}*/

void emit_general_instruction(char* mnemonic, char* operand1, char* operand2, struct x86_instruction** list,int* index, int* capacity,uint32_t* address){
    if (*index>=*capacity){
        *capacity*=2;

        struct x86_instruction** re=MALLOC(*capacity*sizeof(struct x86_instruction*));

        for (int i=0;i<*index;i++){
            re[i]=list[i];
        }
        list=re;
        
    }


    struct x86_instruction* instruction=MALLOC(sizeof(struct x86_instruction));

    instruction->mnemonic=mnemonic;
    instruction->operand1=operand1;
    instruction->operand2=operand2;

    list[*index]=instruction;
    (*index)++;

    *address+=get_instruction_size(instruction);


}

void generate_rng(){
    int capacity=128;
    int ind=0;

    struct x86_instruction** ins=MALLOC(capacity*sizeof(struct x86_instruction*));

    uint32_t address=RNG_ADDRESS;
    uint32_t gen_rand_address=&gen_rand;

    
    
    emit_general_instruction("POP","ESI","",ins,&ind,&capacity,&address); //&
    emit_general_instruction("POP","EBX","",ins,&ind,&capacity,&address); //param2
    emit_general_instruction("POP","ECX","",ins,&ind,&capacity,&address); //param1
    
    emit_general_instruction("PUSH","ESI","",ins,&ind,&capacity,&address); //&

    emit_general_instruction("PUSH","EBX","",ins,&ind,&capacity,&address);
    emit_general_instruction("PUSH","ECX","",ins,&ind,&capacity,&address);

    emit_general_instruction("PUSH","EBP","",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","EBP","ESP",ins,&ind,&capacity,&address);

    emit_general_instruction("CALL","-1","",ins,&ind,&capacity,&address);

    char adb[32];
    int_str(gen_rand_address-address,adb);
    ins[ind-1]->operand1=str_copy(adb);

    emit_general_instruction("MOV","ESP","EBP",ins,&ind,&capacity,&address);
    emit_general_instruction("POP","EBP","",ins,&ind,&capacity,&address);

    emit_general_instruction("POP","ECX","",ins,&ind,&capacity,&address);
    emit_general_instruction("POP","EBX","",ins,&ind,&capacity,&address);

    emit_general_instruction("CDQ","","",ins,&ind,&capacity,&address);

    emit_general_instruction("PUSH","EDX","",ins,&ind,&capacity,&address);

    emit_general_instruction("SUB","EBX","ECX",ins,&ind,&capacity,&address);
    emit_general_instruction("IDIV","EBX","",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","EAX","EDX",ins,&ind,&capacity,&address);
    emit_general_instruction("POP","EDX","",ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","EAX","ECX",ins,&ind,&capacity,&address);

    emit_general_instruction("RET","","",ins,&ind,&capacity,&address);
    

    code_buffer* rngbuf=MALLOC(sizeof(code_buffer));
    rngbuf->capacity=512;
    rngbuf->bytes=MALLOC(rngbuf->capacity);
    rngbuf->size=0;

    assemble_instructions(ins,rngbuf);

    write_program_to_memory(rngbuf,RNG_ADDRESS);

}

void generate_draw_line(){//todo
    int capacity=64;
    int ind=0;

    struct x86_instruction** ins=MALLOC(capacity*sizeof(struct x86_instruction*));

    uint32_t address=DRAW_LINE_ADDRESS;

    emit_general_instruction("POP","ESI","",ins,&ind,&capacity,&address); //caller address
    emit_general_instruction("POP","EAX","",ins,&ind,&capacity,&address); //x
    emit_general_instruction("POP","EBX","",ins,&ind,&capacity,&address); //y




}

void generate_malloc(){
    int capacity=64;
    int ind=0;

    struct x86_instruction** ins=MALLOC(capacity*sizeof(struct x86_instruction*));

    uint32_t address=MALLOC_ADDRESS;

    long* hp=&heap_offset;
    uint32_t HEAP_PTR=(uint32_t)hp;

    char heap_ptr_buf[32];
    uint32_t_str(HEAP_PTR,heap_ptr_buf);

    char addrbuf[32];
    uint32_t_str((uint32_t)heap,addrbuf);

    emit_general_instruction("MOV","ECX",str_copy(heap_ptr_buf),ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","EBX","[ECX]",ins,&ind,&capacity,&address);
   
    
    emit_general_instruction("ADD","[ECX]","EAX",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","EAX","EBX",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","EBX",addrbuf,ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","EAX","EBX",ins,&ind,&capacity,&address);

    emit_general_instruction("RET","","",ins,&ind,&capacity,&address);


    code_buffer* mallocbuf=MALLOC(sizeof(code_buffer));
    mallocbuf->capacity=512;
    mallocbuf->bytes=MALLOC(mallocbuf->capacity);
    mallocbuf->size=0;

    assemble_instructions(ins,mallocbuf);

    write_program_to_memory(mallocbuf,MALLOC_ADDRESS);


}

generate_draw_pixel(){
    int capacity=64;
    int ind=0;

    struct x86_instruction** ins=MALLOC(capacity*sizeof(struct x86_instruction*));

    uint32_t address=DRAW_PIXEL_ADDRESS;

    

    char frame_buf[32];
    uint32_t_str(framebuffer_add,frame_buf);
    
    char ppr_str[32];
    int_str(ppr*4,ppr_str);

    emit_general_instruction("POP","ESI","",ins,&ind,&capacity,&address); // caller address
    emit_general_instruction("POP","EAX","",ins,&ind,&capacity,&address); //x
    emit_general_instruction("POP","EBX","",ins,&ind,&capacity,&address); //y

    emit_general_instruction("PUSH","ESI","",ins,&ind,&capacity,&address); //push caller address

    emit_general_instruction("MOV","ESI",frame_buf,ins,&ind,&capacity,&address);
    emit_general_instruction("SHL","EBX","2",ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","EAX","ESI",ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","EAX","EBX",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","EBX","[0xCCC000]",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","[EAX]","EBX",ins,&ind,&capacity,&address);
    emit_general_instruction("RET","","",ins,&ind,&capacity,&address);

    


    code_buffer* drawbuf=MALLOC(sizeof(code_buffer));
    drawbuf->capacity=512;
    drawbuf->bytes=MALLOC(drawbuf->capacity);
    drawbuf->size=0;

    assemble_instructions(ins,drawbuf);

    write_program_to_memory(drawbuf,DRAW_PIXEL_ADDRESS);

}




void generate_draw(){
    int capacity=128;
    int ind=0;

    struct x86_instruction** ins=MALLOC(capacity*sizeof(struct x86_instruction*));

    uint32_t address=DRAW_ADDRESS;

    char frame_buf[32];
    uint32_t_str(framebuffer_add,frame_buf);
    
    char ppr_str[32];
    int_str(ppr*4,ppr_str);

//drawrect:
//  [RET] [x] [y] [width] [height]  <--

    emit_general_instruction("POP","ESI","",ins,&ind,&capacity,&address); // save caller address

    emit_general_instruction("POP","EAX","",ins,&ind,&capacity,&address); // x
    emit_general_instruction("POP","EBX","",ins,&ind,&capacity,&address); // y
    emit_general_instruction("POP","ECX","",ins,&ind,&capacity,&address); // width
    emit_general_instruction("POP","EDI","",ins,&ind,&capacity,&address); // height

    emit_general_instruction("ADD","ECX","EAX",ins,&ind,&capacity,&address); //endx
    emit_general_instruction("ADD","EDI","EBX",ins,&ind,&capacity,&address); //endy

    emit_general_instruction("PUSH","ESI","",ins,&ind,&capacity,&address); //push caller address onto stack
    emit_general_instruction("PUSH","EAX","",ins,&ind,&capacity,&address); //oldx

    //[oldx] [RET]

//.loop

    uint32_t loopstart=address;
    emit_general_instruction("CALL","-1","",ins,&ind,&capacity,&address);
    uint32_t call1=address;
    int asmpoint1=ind-1;
    emit_general_instruction("ADD","EAX","1",ins,&ind,&capacity,&address);
    emit_general_instruction("CMP","EAX","ECX",ins,&ind,&capacity,&address);

    char adb1[32];
    

    emit_general_instruction("JL","-1","",ins,&ind,&capacity,&address);
    int_str(loopstart-(address),adb1);
    ins[ind-1]->operand1=str_copy(adb1);

    emit_general_instruction("MOV","EAX","[ESP]",ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","EBX","1",ins,&ind,&capacity,&address);
    emit_general_instruction("CMP","EBX","EDI",ins,&ind,&capacity,&address);

    char adb2[32];
    

    emit_general_instruction("JL","-1","",ins,&ind,&capacity,&address);
    int_str(loopstart-(address),adb2);
    ins[ind-1]->operand1=str_copy(adb2);

    emit_general_instruction("POP","EAX","",ins,&ind,&capacity,&address);
    emit_general_instruction("RET","","",ins,&ind,&capacity,&address);

    uint32_t drawpixel=address;

    char adb3[32];
    int_str(drawpixel-call1,adb3);

    ins[asmpoint1]->operand1=str_copy(adb3);

//drawpixel: 

//DRAW_PIXEL_ADDRESS=address;

//  [RET] [EAX] [ESI]

    emit_general_instruction("PUSH","EBX","",ins,&ind,&capacity,&address);
    emit_general_instruction("PUSH","ESI","",ins,&ind,&capacity,&address);
    emit_general_instruction("PUSH","EAX","",ins,&ind,&capacity,&address);

//  [EAX] [ESI] [EBX] [RET] [EAX] [ESI]
    
    emit_general_instruction("MOV","ESI",ppr_str,ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","EBX","50",ins,&ind,&capacity,&address);
    emit_general_instruction("IMUL","EBX","ESI",ins,&ind,&capacity,&address);
    
    //emit_general_instruction("SHL","EBX","2",ins,&ind,&capacity,&address);
    

    emit_general_instruction("MOV","ESI",frame_buf,ins,&ind,&capacity,&address);

    emit_general_instruction("SHL","EAX","2",ins,&ind,&capacity,&address);
    emit_general_instruction("ADD","ESI","EAX",ins,&ind,&capacity,&address);
    
    emit_general_instruction("ADD","ESI","EBX",ins,&ind,&capacity,&address);

    emit_general_instruction("MOV","EBX","[0xCCC000]",ins,&ind,&capacity,&address);
    emit_general_instruction("MOV","[ESI]","EBX",ins,&ind,&capacity,&address);

    emit_general_instruction("POP","EAX","",ins,&ind,&capacity,&address);
    emit_general_instruction("POP","ESI","",ins,&ind,&capacity,&address);
    emit_general_instruction("POP","EBX","",ins,&ind,&capacity,&address);
    emit_general_instruction("RET","","",ins,&ind,&capacity,&address);

    code_buffer* drawbuf=MALLOC(sizeof(code_buffer));
    drawbuf->capacity=1024;
    drawbuf->bytes=MALLOC(drawbuf->capacity);
    drawbuf->size=0;

    ins[ind] = NULL;
    assemble_instructions(ins,drawbuf);

    write_program_to_memory(drawbuf,DRAW_ADDRESS);


}

void generate_helper_functions() {
    // ensure helpers array is allocated and reset
    if (helpers == NULL) {
        helpers = MALLOC(helpers_size * sizeof(struct helper_function*));
    }
    helpers_count = 0;

    generate_draw();
    generate_draw_pixel();
    generate_malloc();
    generate_rng();


    //generate_heap_ptr();

    /*generate_strlen();

    //printf("LEFT STRLEN GEN\n");

    generate_strcat();

    generate_strcpy();

    generate_malloc();





    //printf("LAST HELPER: %s\n",helpers[helpers_count-1]->name);

   /* for (int i=0;helpers[helpers_count-1]->instructions[i]!=NULL;i++) {

        /printf("%s %s %s\n",helpers[helpers_count-1]->instructions[i]->mnemonic,helpers[helpers_count-1]->instructions[i]->operand1,helpers[helpers_count-1]->instructions[i]->operand2);
    }*/
    //printf("\n\n");


}

//void print_all_nodes();

int parse_register(char* operand) {
    if (str_eq(operand,"EAX") || str_eq(operand, "XMM0"))return 0;
    if (str_eq(operand,"ECX") || str_eq(operand, "XMM1"))return 1;
    if (str_eq(operand,"EDX") || str_eq(operand, "XMM2"))return 2;
    if (str_eq(operand,"EBX") || str_eq(operand, "XMM3"))return 3;
    if (str_eq(operand,"ESP") || str_eq(operand, "XMM4"))return 4;
    if (str_eq(operand,"EBP") || str_eq(operand, "XMM5"))return 5;
    if (str_eq(operand,"ESI") || str_eq(operand, "XMM6"))return 6;
    if (str_eq(operand,"EDI") || str_eq(operand, "XMM7"))return 7;


    if (str_eq(operand,"AL") || str_eq(operand,"AX"))return 0;
    if (str_eq(operand,"CL") || str_eq(operand,"CX"))return 1;
    if (str_eq(operand,"DL") || str_eq(operand,"DX"))return 2;
    if (str_eq(operand,"BL") || str_eq(operand,"BX"))return 3;

    return -1;
}
uint8_t modrm(uint8_t mod, uint8_t reg, uint8_t rm) {
    return (mod<<6)|(reg<<3)|rm;

}

uint8_t sib(uint8_t scale, uint8_t index, uint8_t base) {
    return (scale<<6)|(index<<3)|base;

}


void emit_byte(code_buffer* buffer,uint8_t byte) {
    if (buffer->size==buffer->capacity) {
        buffer->capacity*=2;
        uint8_t* new_data=MALLOC(buffer->capacity);

        for (int i=0;i<buffer->size;i++) {
            new_data[i]=buffer->bytes[i];
        }

        buffer->bytes=new_data;



    }
    buffer->bytes[buffer->size++]=byte;
}








//////////////codegenstart///////


int my_strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *my_strcpy(char *dest, const char *src) {
    char *original_dest = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return original_dest;
}

char *my_strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

char *my_strchr(const char *str, int c) {
    while (*str) {
        if (*str == c) return (char *)str;
        str++;
    }
    return (c == '\0') ? (char *)str : NULL;
}

char *my_strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (!*n) return (char *)haystack;
        haystack++;
    }
    return NULL;
}

int my_isdigit(int c) {
    return (c >= '0' && c <= '9');
}

int my_atoi(const char *str) {
    int result = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t' || *str == '\n') str++;

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

long long my_strtoll(const char *str, char **endptr, int base) {
    long long result = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t' || *str == '\n') str++;

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Handle hex prefix
    if (base == 0 || base == 16) {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            str += 2;
            base = 16;
        } else if (base == 0) {
            base = 10;
        }
    }

    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'f') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'F') {
            digit = *str - 'A' + 10;
        } else {
            break;
        }

        if (digit >= base) break;

        result = result * base + digit;
        str++;
    }

    if (endptr) *endptr = (char *)str;
    return sign * result;
}

unsigned long my_strtoul(const char *str, char **endptr, int base) {
    unsigned long result = 0;

    while (*str == ' ' || *str == '\t' || *str == '\n') str++;

    if (*str == '+') str++;

    // Handle hex prefix
    if (base == 0 || base == 16) {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            str += 2;
            base = 16;
        } else if (base == 0) {
            base = 10;
        }
    }

    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'f') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'F') {
            digit = *str - 'A' + 10;
        } else {
            break;
        }

        if (digit >= base) break;

        result = result * base + digit;
        str++;
    }

    if (endptr) *endptr = (char *)str;
    return result;
}

void buffer_emit_byte(code_buffer *buf, uint8_t byte) {
    if (buf->size >= buf->capacity) {
        // Can't realloc, so just allocate bigger buffer and copy
        int new_capacity = buf->capacity * 2;
        uint8_t *new_data = (uint8_t *)MALLOC(new_capacity);

        // Copy old data
        for (int i = 0; i < buf->size; i++) {
            new_data[i] = buf->bytes[i];
        }

        buf->bytes = new_data;
        buf->capacity = new_capacity;
    }
    buf->bytes[buf->size++] = byte;
}

void buffer_emit_word(code_buffer *buf, short word) {
    buffer_emit_byte(buf, word & 0xFF);
    buffer_emit_byte(buf, (word >> 8) & 0xFF);
}

void buffer_emit_dword(code_buffer *buf, int dword) {
    char ob[32];
    int_str(dword,ob);

    //write_rowcol_text(20,3,ob,GREEN);
    

    buffer_emit_byte(buf, dword & 0xFF);
    buffer_emit_byte(buf, (dword >> 8) & 0xFF);
    buffer_emit_byte(buf, (dword >> 16) & 0xFF);
    buffer_emit_byte(buf, (dword >> 24) & 0xFF);
}



Operand parse_memory_operand(const char *str) {
    Operand op = {0};
    op.type = OPERAND_MEMORY;
    op.mem.base_reg = -1;
    op.mem.index_reg = -1;
    op.mem.scale = 1;
    op.mem.disp = 0;

    const char *ptr = str;
    if (my_strstr(ptr, "PTR") != NULL) {
        ptr = my_strchr(ptr, '[');
        if (!ptr) return op;
    }

    const char *start = my_strchr(ptr, '[');
    const char *end = my_strchr(ptr, ']');

    if (!start || !end) return op;

    start++;

    char inside[256];
    int len = end - start;
    my_strncpy(inside, start, len);
    inside[len] = '\0';

    if (my_isdigit(inside[0]) || (inside[0] == '0' && inside[1] == 'x')) {
        op.type = OPERAND_MEMORY_DIRECT;
        op.addr = my_strtoul(inside, NULL, 0);
        return op;
    }

    char *plus = my_strchr(inside, '+');
    char *minus = my_strchr(inside, '-');

    if (plus || minus) {
        char reg_name[32];
        char *op_pos = plus ? plus : minus;
        int reg_len = op_pos - inside;
        my_strncpy(reg_name, inside, reg_len);
        reg_name[reg_len] = '\0';

        char *reg_start = reg_name;
        while (*reg_start == ' ') reg_start++;
        char *reg_end = reg_start + my_strlen(reg_start) - 1;
        while (reg_end > reg_start && *reg_end == ' ') {
            *reg_end = '\0';
            reg_end--;
        }

        op.mem.base_reg = parse_register(reg_start);

        int offset = my_atoi(op_pos + 1);
        if (minus) offset = -offset;
        op.mem.disp = offset;
    } else {
        char reg_name[32];
        my_strcpy(reg_name, inside);

        char *reg_start = reg_name;
        while (*reg_start == ' ') reg_start++;
        char *reg_end = reg_start + my_strlen(reg_start) - 1;
        while (reg_end > reg_start && *reg_end == ' ') {
            *reg_end = '\0';
            reg_end--;
        }

        op.mem.base_reg = parse_register(reg_start);
    }

    return op;
}

Operand parse_operand(const char *str) {
    Operand op = {0};

    if (!str || my_strlen(str) == 0) {
        op.type = OPERAND_NONE;
        return op;
    }

    if (my_strchr(str, '[') != NULL) {
        return parse_memory_operand(str);
    }

    int reg = parse_register(str);
    if (reg >= 0) {
        op.type = OPERAND_REGISTER;
        op.reg = reg;
        return op;
    }

    if (my_isdigit(str[0]) || str[0] == '-' || (str[0] == '0' && str[1] == 'x')) {
        op.type = OPERAND_IMMEDIATE;
        op.imm = my_strtoll(str, NULL, 0);
        
        return op;
    }

    op.type = OPERAND_NONE;
    return op;
}

void encode_mov_reg_reg(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x89);
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_mov_reg8_reg8(code_buffer *buf, int dst, int src) {
    // MOV r8, r8 -> 0x8A /r (MOV r8, r/m8)
    buffer_emit_byte(buf, 0x8A);
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_mov_reg8_imm(code_buffer* buf, int reg, int imm){
    buffer_emit_byte(buf, 0xB0+reg);
    buffer_emit_byte(buf, (char)imm);

}

void encode_mov_reg_imm(code_buffer *buf, int reg, int imm) {
    buffer_emit_byte(buf, 0xB8 + reg);
    buffer_emit_dword(buf, imm);
}

void encode_mov_reg_imm8(code_buffer *buf, int reg, int imm8) {
    buffer_emit_byte(buf, 0xB0 + reg);
    buffer_emit_byte(buf, (uint8_t)imm8);
}

void encode_mov_reg_mem(code_buffer *buf, int reg, Operand *mem_op) {
    buffer_emit_byte(buf, 0x8B);

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, reg, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_mov_mem_reg(code_buffer *buf, Operand *mem_op, int reg) {
    buffer_emit_byte(buf, 0x89);

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, reg, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_mov_mem_imm(code_buffer *buf, Operand *mem_op, int imm) {
    buffer_emit_byte(buf, 0xC7);

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, 0, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, 0, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, 0, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, 0, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_dword(buf, disp);
        }
    }
    buffer_emit_dword(buf, imm);
}

void encode_add_reg_reg(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x01);
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_add_reg_imm(code_buffer *buf, int reg, int imm) {
    // Force imm32 encoding for ESP to avoid potential low-level issues
    if (reg == parse_register("ESP")) {
        buffer_emit_byte(buf, 0x81);
        buffer_emit_byte(buf, modrm(3, 0, reg));
        buffer_emit_dword(buf, imm);
        return;
    }

    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83);
        buffer_emit_byte(buf, modrm(3, 0, reg));
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_byte(buf, 0x81);
        buffer_emit_byte(buf, modrm(3, 0, reg));
        buffer_emit_dword(buf, imm);
    }
}

void encode_add_mem_imm(code_buffer *buf, Operand *mem_op, int imm) {
    int start_size = buf->size;
    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83);
    } else {
        buffer_emit_byte(buf, 0x81);
    }

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, 0, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, 0, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, 0, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, 0, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_dword(buf, disp);
        }
    }

    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_dword(buf, imm);
    }

    // Debug: if base is EDX, print emitted bytes for inspection
    if (mem_op->type != OPERAND_MEMORY_DIRECT) {
        int base = mem_op->mem.base_reg;
        if (base == parse_register("EDX")) {
            int cols = 20;
            int x = 2;
            int y = 20;
            char out[128];
            out[0] = '\0';
            for (int i = start_size; i < buf->size; i++) {
                char h[4];
                byte_to_hex(buf->bytes[i], h);
                out[my_strlen(out)] = '\0';
                // append with space
                str_cat(out, h);
                str_cat(out, " ");
            }
            write_rowcol_text(y, x, out, RED);
        }
    }
}

void encode_sub_reg_reg(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x29);
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_sub_reg_imm(code_buffer *buf, int reg, int imm) {
    // Force imm32 encoding for ESP to avoid potential low-level issues
    if (reg == parse_register("ESP")) {
        buffer_emit_byte(buf, 0x81);
        buffer_emit_byte(buf, modrm(3, 5, reg));
        buffer_emit_dword(buf, imm);
        return;
    }

    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83);
        buffer_emit_byte(buf, modrm(3, 5, reg));
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_byte(buf, 0x81);
        buffer_emit_byte(buf, modrm(3, 5, reg));
        buffer_emit_dword(buf, imm);
    }
}

void encode_imul_reg_reg(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0xAF);
    buffer_emit_byte(buf, modrm(3, dst, src));
}

void encode_idiv_reg(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0xF7);
    buffer_emit_byte(buf, modrm(3, 7, reg));
}

void encode_neg_reg(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0xF7);
    buffer_emit_byte(buf, modrm(3, 3, reg));
}

void encode_not_reg(code_buffer *buf, int reg) {
    // NOT r/m32 -> F7 /2
    buffer_emit_byte(buf, 0xF7);
    buffer_emit_byte(buf, modrm(3, 2, reg));
}

void encode_mov_reg16_imm(code_buffer* buf, int reg, int imm){
    buffer_emit_byte(buf,0x66);
    buffer_emit_byte(buf, 0xB8+reg);
    buffer_emit_byte(buf, (char)imm);

}

void encode_mov_reg16_reg16(code_buffer* buf, int reg1, int reg2){
    buffer_emit_byte(buf, 0x66);
    buffer_emit_byte(buf, 0x89);
    buffer_emit_byte(buf, modrm(3,reg2,reg1));
}

void encode_imul_reg_imm(code_buffer *buf, int reg, int imm) {
    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x6B); // IMUL r/m32, imm8 -> 6B /5 ib
        buffer_emit_byte(buf, modrm(3, 5, reg));
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_byte(buf, 0x69); // IMUL r/m32, imm32 -> 69 /5 id
        buffer_emit_byte(buf, modrm(3, 5, reg));
        buffer_emit_dword(buf, imm);
    }
}

void encode_in_reg8(code_buffer* buf){
    //IN AL, DX
    buffer_emit_byte(buf, 0xEC);

}

void encode_in_reg16(code_buffer* buf){
    //IN AX, DX
    buffer_emit_byte(buf, 0x66);
    buffer_emit_byte(buf, 0xED);

}

void encode_in_reg8_imm(code_buffer* buf, int imm){
    //IN AL, imm
    buffer_emit_byte(buf, 0xE4);
    buffer_emit_byte(buf, (char)imm);

}

void encode_in_reg16_imm(code_buffer* buf, int imm){
    //IN AX, imm
    buffer_emit_byte(buf, 0x66);
    buffer_emit_byte(buf, 0xE5);
    buffer_emit_byte(buf, (char)imm);
}

void encode_out_reg8(code_buffer* buf){
    //OUT DX, AL
    buffer_emit_byte(buf, 0xEE);

}

void encode_out_reg16(code_buffer* buf){
    //OUT DX, AX
    buffer_emit_byte(buf, 0x66); //for AX
    buffer_emit_byte(buf, 0xEF);

}

void encode_out_imm8_reg8(code_buffer* buf, int imm){
    //OUT 0x60, AL
    buffer_emit_byte(buf, 0xE6);
    buffer_emit_byte(buf, (char)imm);

}

void encode_out_imm8_reg16(code_buffer* buf, int imm){
    //OUT 0x60, AX
    buffer_emit_byte(buf, 0x66);
    buffer_emit_byte(buf, 0xE7);
    buffer_emit_byte(buf, (char)imm);

}

void encode_cdq(code_buffer *buf) {
    buffer_emit_byte(buf, 0x99);
}

void encode_ret(code_buffer *buf) {
    buffer_emit_byte(buf, 0xC3);
}

void encode_push_reg(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x50 + reg);
}

void encode_pop_reg(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x58 + reg);
}

void encode_call_mem(code_buffer *buf, uint32_t addr) {
    buffer_emit_byte(buf, 0xFF);
    buffer_emit_byte(buf, modrm(0, 2, 5));
    buffer_emit_dword(buf, addr);
}

void encode_call_rel(code_buffer *buf, int offset) {
    buffer_emit_byte(buf, 0xE8);
    buffer_emit_dword(buf, offset);
}

void encode_call_reg(code_buffer* buf, int reg){
    buffer_emit_byte(buf, 0xFF);
    buffer_emit_byte(buf, 0xD0 | (reg & 0x07));
}

void encode_shl_reg_imm(code_buffer *buf, int reg, int imm) {
    // SHL r/m32, imm8  -> opcode C1 /4 ib
    buffer_emit_byte(buf, 0xC1);
    buffer_emit_byte(buf, modrm(3, 4, reg));
    buffer_emit_byte(buf, (uint8_t)imm);
}

void encode_shl_reg_one(code_buffer *buf, int reg) {
    // SHL r/m32, 1 -> D1 /4
    buffer_emit_byte(buf, 0xD1);
    buffer_emit_byte(buf, modrm(3, 4, reg));
}

void encode_shl_reg_cl(code_buffer *buf, int reg) {
    // SHL r/m32, CL -> D3 /4
    buffer_emit_byte(buf, 0xD3);
    buffer_emit_byte(buf, modrm(3, 4, reg));
}

void encode_shr_reg_imm(code_buffer *buf, int reg, int imm) {
    // SHR r/m32, imm8 -> opcode C1 /5 ib
    buffer_emit_byte(buf, 0xC1);
    buffer_emit_byte(buf, modrm(3, 5, reg));
    buffer_emit_byte(buf, (uint8_t)imm);
}

void encode_shr_reg_one(code_buffer *buf, int reg) {
    // SHR r/m32, 1 -> D1 /5
    buffer_emit_byte(buf, 0xD1);
    buffer_emit_byte(buf, modrm(3, 5, reg));
}

void encode_shr_reg_cl(code_buffer *buf, int reg) {
    // SHR r/m32, CL -> D3 /5
    buffer_emit_byte(buf, 0xD3);
    buffer_emit_byte(buf, modrm(3, 5, reg));
}

void encode_or_reg_reg(code_buffer *buf, int dst, int src) {
    // OR r/m32, r32 -> 0x09 /r
    buffer_emit_byte(buf, 0x09);
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_or_reg_imm(code_buffer *buf, int reg, int imm) {
    // OR r/m32, imm8/imm32 -> 83/81 with /1 in ModR/M
    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83);
        buffer_emit_byte(buf, modrm(3, 1, reg));
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_byte(buf, 0x81);
        buffer_emit_byte(buf, modrm(3, 1, reg));
        buffer_emit_dword(buf, imm);
    }
}

void encode_jmp_rel(code_buffer *buf, int offset) {
    // Always emit near JMP: 0xE9 imm32
    buffer_emit_byte(buf, 0xE9);
    buffer_emit_dword(buf, offset);
}

void encode_je_rel(code_buffer *buf, int offset) {
    // Always emit long form: 0x0F 0x84 imm32
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x84);
    buffer_emit_dword(buf, offset);
}

void encode_jne_rel(code_buffer *buf, int offset) {
    // Always emit long form: 0x0F 0x85 imm32
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x85);
    buffer_emit_dword(buf, offset);
}

void encode_jz_rel(code_buffer *buf, int offset) {
    encode_je_rel(buf, offset);
}

void encode_cond_rel(code_buffer *buf, int offset, uint8_t cond_opcode) {
    // Emit long conditional jump: 0x0F <cond_opcode> imm32
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, cond_opcode);
    buffer_emit_dword(buf, offset);
}

void encode_movzx_reg_reg8(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0xB6);
    buffer_emit_byte(buf, modrm(3, dst, src));
}

void encode_movzx_reg_reg16(code_buffer* buf, int dst, int src){
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0xB7);
    buffer_emit_byte(buf, modrm(3, dst, src));

}

void encode_movsx_reg_reg8(code_buffer *buf, int dst, int src) {
    // MOVSX r32, r8 -> 0x0F 0xBE /r
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0xBE);
    buffer_emit_byte(buf, modrm(3, dst, src));
}

void encode_movsx_reg_mem(code_buffer *buf, int dst, Operand *mem_op) {
    // MOVSX r32, r/m8 -> 0x0F 0xBE /r
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0xBE);

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, dst, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, dst, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, dst, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, dst, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_test_reg_reg(code_buffer *buf, int reg1, int reg2) {
    buffer_emit_byte(buf, 0x85);
    buffer_emit_byte(buf, modrm(3, reg2, reg1));
}

void encode_test_reg8_imm(code_buffer *buf, int reg, int imm8) {
    // TEST r8, imm8 -> special-case AL: A8 ib, otherwise F6 /0 ib
    if (reg == 0) { // AL
        buffer_emit_byte(buf, 0xA8);
        buffer_emit_byte(buf, (uint8_t)imm8);
    } else {
        // F6 /0 ib
        buffer_emit_byte(buf, 0xF6);
        buffer_emit_byte(buf, modrm(3, 0, reg));
        buffer_emit_byte(buf, (uint8_t)imm8);
    }
}

void encode_cmp_reg_reg(code_buffer *buf, int reg1, int reg2) {
    buffer_emit_byte(buf, 0x39);
    buffer_emit_byte(buf, modrm(3, reg2, reg1));
}

void encode_cmp_reg_imm(code_buffer *buf, int reg, int imm) {
    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83);
        buffer_emit_byte(buf, modrm(3, 7, reg));
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_byte(buf, 0x81);
        buffer_emit_byte(buf, modrm(3, 7, reg));
        buffer_emit_dword(buf, imm);
    }
}

void encode_setnz(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x95);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_sete(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x94);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_setg(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x9F);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_setge(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x9D);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_setl(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x9C);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_setle(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x9E);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_setne(code_buffer *buf, int reg) {
    buffer_emit_byte(buf, 0x0F);
    buffer_emit_byte(buf, 0x95);
    buffer_emit_byte(buf, modrm(3, 0, reg));
}

void encode_int(code_buffer *buf, uint8_t interrupt) {
    buffer_emit_byte(buf, 0xCD);
    buffer_emit_byte(buf, interrupt);
}

void encode_sub_mem_imm(code_buffer *buf, Operand *mem_op, int imm) {
    int start_size = buf->size;
    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83);
    } else {
        buffer_emit_byte(buf, 0x81);
    }

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, 5, 5));  // Note: reg field is 5 for SUB
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, 5, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, 5, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, 5, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_dword(buf, disp);
        }
    }

    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_dword(buf, imm);
    }

    // Debug: if base is EDX, print emitted bytes for inspection
    if (mem_op->type != OPERAND_MEMORY_DIRECT) {
        int base = mem_op->mem.base_reg;
        if (base == parse_register("EDX")) {
            int x = 2;
            int y = 23;
            char out[128];
            out[0] = '\0';
            for (int i = start_size; i < buf->size; i++) {
                char h[4];
                byte_to_hex(buf->bytes[i], h);
                str_cat(out, h);
                str_cat(out, " ");
            }
            write_rowcol_text(y, x, out, RED);
        }
    }
}

void encode_sub_mem_reg(code_buffer *buf, Operand *mem_op, int reg) {
    buffer_emit_byte(buf, 0x29); // SUB r/m32, r32

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, reg, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_add_mem_reg(code_buffer *buf, Operand *mem_op, int reg) {
    buffer_emit_byte(buf, 0x01); // ADD r/m32, r32

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, reg, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_lodsb(code_buffer *buf) {
    buffer_emit_byte(buf, 0xAC); // LODSB
}

void encode_stosb(code_buffer *buf) {
    buffer_emit_byte(buf, 0xAA); // STOSB
}

void encode_xor_reg_reg(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x31); // XOR r/m32, r32
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_xor_reg_imm(code_buffer *buf, int reg, int imm) {
    if (imm >= -128 && imm <= 127) {
        buffer_emit_byte(buf, 0x83); // XOR r/m32, imm8
        buffer_emit_byte(buf, modrm(3, 6, reg));
        buffer_emit_byte(buf, (uint8_t)imm);
    } else {
        buffer_emit_byte(buf, 0x81); // XOR r/m32, imm32
        buffer_emit_byte(buf, modrm(3, 6, reg));
        buffer_emit_dword(buf, imm);
    }
}

void encode_and_reg8_reg8(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x20); // AND r/m8, r8
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_and_reg_reg(code_buffer *buf, int dst, int src) {
    buffer_emit_byte(buf, 0x21); // AND r/m32, r32
    buffer_emit_byte(buf, modrm(3, src, dst));
}

void encode_mov_mem_reg8(code_buffer *buf, Operand *mem_op, int reg) {
    // MOV r/m8, r8 -> 0x88 /r
    buffer_emit_byte(buf, 0x88);

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, reg, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, reg, base));

            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, reg, base));
            if (base == 4) buffer_emit_byte(buf, sib(0, 4, 4));
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_mov_reg8_mem(code_buffer *buf, int reg, Operand *mem_op) {
    // MOV r8, [mem] -> 0x8A /r
    buffer_emit_byte(buf, 0x8A);

    if (mem_op->type == OPERAND_MEMORY_DIRECT) {
        buffer_emit_byte(buf, modrm(0, reg, 5));
        buffer_emit_dword(buf, mem_op->addr);
    } else {
        int base = mem_op->mem.base_reg;
        int disp = mem_op->mem.disp;

        if (disp == 0 && base != 5) {
            buffer_emit_byte(buf, modrm(0, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
        } else if (disp >= -128 && disp <= 127) {
            buffer_emit_byte(buf, modrm(1, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_byte(buf, (uint8_t)disp);
        } else {
            buffer_emit_byte(buf, modrm(2, reg, base));
            if (base == 4) {
                buffer_emit_byte(buf, sib(0, 4, 4));
            }
            buffer_emit_dword(buf, disp);
        }
    }
}

void encode_rep_stosd(code_buffer* buf){
    buffer_emit_byte(buf,0xF3);
    buffer_emit_byte(buf,0xAB);
}

void encode_rep_stosb(code_buffer* buf){
    buffer_emit_byte(buf,0xF3);
    buffer_emit_byte(buf,0xAA);

}

void encode_rep_movsb(code_buffer* buf){
    buffer_emit_byte(buf,0xF3);
    buffer_emit_byte(buf, 0xA4);
}

void encode_movd_xmm_reg(code_buffer* buf, int dst, int src){
    buffer_emit_byte(buf,0x66);
    buffer_emit_byte(buf,0x0F);
    buffer_emit_byte(buf,0x6E);
    buffer_emit_byte(buf,modrm(0b11,dst,src));

}

void encode_movd_reg_xmm(code_buffer* buf, int dst, int src){
    buffer_emit_byte(buf,0x66);
    buffer_emit_byte(buf,0x0F);
    buffer_emit_byte(buf,0x7E);
    buffer_emit_byte(buf,modrm(0b11,dst,src));

}

void encode_addss_xmm_xmm(code_buffer* buf, int dst, int src){

}

void encode_subss_xmm_xmm(code_buffer* buf, int dst, int src){
    
}

void encode_mulss_xmm_xmm(code_buffer* buf, int dst, int src){
    
}

void encode_divss_xmm_xmm(code_buffer* buf, int dst, int src){
    
}

void encode_movss_xmm_xmm(code_buffer* buf, int dst, int src){
    buffer_emit_byte(buf,0xF3);
    buffer_emit_byte(buf,0x0F);
    buffer_emit_byte(buf,0x10);
    buffer_emit_byte(buf,modrm(0b11,dst,src));

}

void encode_rep_movsd(code_buffer* buf){
    buffer_emit_byte(buf,0xF3);
    buffer_emit_byte(buf,0xA5);

}

void encode_jmp_reg(code_buffer* buf, int reg){
    buffer_emit_byte(buf,0xFF);
    buffer_emit_byte(buf,modrm(0b11,0b100,reg));

}


void encode_instruction(code_buffer *buf, struct x86_instruction *instr) {
    if (!instr->mnemonic) return;



    if (my_strcmp(instr->mnemonic, "MOV") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            if (is_reg8(instr->operand1) && is_reg8(instr->operand2)) {
                encode_mov_reg8_reg8(buf, op1.reg, op2.reg);
            } 
            else if(is_reg16(instr->operand1) && is_reg16(instr->operand2)){
                encode_mov_reg16_reg16(buf, op1.reg, op2.reg);
            }
            else {
                encode_mov_reg_reg(buf, op1.reg, op2.reg);
            }
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            if (is_reg8(instr->operand1)) {
                encode_mov_reg8_imm(buf, op1.reg, op2.imm);
            }
            else if(is_reg16(instr->operand1)){
                encode_mov_reg16_imm(buf, op1.reg, op2.imm);
            }
            else {
                encode_mov_reg_imm(buf, op1.reg, op2.imm);
            }
        } else if (op1.type == OPERAND_REGISTER && (op2.type == OPERAND_MEMORY || op2.type == OPERAND_MEMORY_DIRECT)) {
            if (is_reg8(instr->operand1)) {
                encode_mov_reg8_mem(buf, op1.reg, &op2);
            } else {
                encode_mov_reg_mem(buf, op1.reg, &op2);
            }
        } else if ((op1.type == OPERAND_MEMORY || op1.type == OPERAND_MEMORY_DIRECT) && op2.type == OPERAND_REGISTER) {
            if (is_reg8(instr->operand2)) {
                encode_mov_mem_reg8(buf, &op1, op2.reg);
            } else {
                encode_mov_mem_reg(buf, &op1, op2.reg);
            }
        } else if ((op1.type == OPERAND_MEMORY || op1.type == OPERAND_MEMORY_DIRECT) && op2.type == OPERAND_IMMEDIATE) {
            encode_mov_mem_imm(buf, &op1, op2.imm);
        }
        
    }
    else if(str_eq(instr->mnemonic,"CLD")){
        buffer_emit_byte(buf,0xFC);
    }
    else if (str_eq(instr->mnemonic,"STI")){
        buffer_emit_byte(buf,0xFB);
    }
    else if (str_eq(instr->mnemonic,"CLI")){
        buffer_emit_byte(buf,0xFA);
    }

    else if(str_eq(instr->mnemonic,"MOVD")){
        Operand op1=parse_operand(instr->operand1);
        Operand op2=parse_operand(instr->operand2);
        if (is_xmm(instr->operand1) && is_reg32(instr->operand2)){
            encode_movd_xmm_reg(buf,op1.reg,op2.reg);

        }
        else if(is_reg32(instr->operand1) && is_xmm(instr->operand1)){
            encode_movd_reg_xmm(buf,op1.reg,op2.reg);
        }
    }

    else if(str_eq(instr->mnemonic,"MOVSS")){
        Operand op1=parse_operand(instr->operand1);
        Operand op2=parse_operand(instr->operand2);

        if (is_xmm(instr->operand1) && is_xmm(instr->operand2)){
            encode_movss_xmm_xmm(buf, op1.reg, op2.reg);

        }
    }
    

    else if(str_eq(instr->mnemonic, "IN")){
        Operand op2=parse_operand(instr->operand2);
        if (is_immediate(instr->operand2)){
            encode_in_reg8_imm(buf,op2.imm);
        }
        else if(is_reg8(instr->operand1)){
            encode_in_reg8(buf);
        }
        else if(is_reg16(instr->operand1) && !is_immediate(instr->operand2)){
            encode_in_reg16(buf);
        }
        else{
            encode_in_reg16_imm(buf,op2.imm);
        }

    }
    else if(str_eq(instr->mnemonic,"OUT")){
        Operand op1=parse_operand(instr->operand1);
        Operand op2=parse_operand(instr->operand2);

        if (op1.type == OPERAND_IMMEDIATE && is_reg8(instr->operand2)){
            encode_out_imm8_reg8(buf,op1.imm);

        }
        else if(op1.type == OPERAND_IMMEDIATE && is_reg16(instr->operand2)){
            encode_out_imm8_reg16(buf,op1.imm);
        }
        else if(op1.type == OPERAND_REGISTER && is_reg8(instr->operand2)){
            encode_out_reg8(buf);
        }
        else{
            encode_out_reg16(buf);
        }


    }

    else if (my_strcmp(instr->mnemonic, "AND") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        
        if (is_reg8(instr->operand1) && is_reg8(instr->operand2)) { 
            encode_and_reg8_reg8(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_and_reg_reg(buf, op1.reg, op2.reg);
        }
        
    }
    else if (my_strcmp(instr->mnemonic, "LODSB") == 0) {
        encode_lodsb(buf);
    }
    else if(str_eq(instr->mnemonic,"REP")){
        if (str_eq(instr->operand1, "STOSB")){
            encode_rep_stosb(buf);
        }
        else if (str_eq(instr->operand1,"STOSD")){
            encode_rep_stosd(buf);
        }
        else if(str_eq(instr->operand1,"MOVSB")){
            encode_rep_movsb(buf);
        }
        else if (str_eq(instr->operand1,"MOVSD")){
            encode_rep_movsd(buf);
        }
    }
    else if (my_strcmp(instr->mnemonic, "STOSB") == 0) {
        encode_stosb(buf);
    }
    else if (my_strcmp(instr->mnemonic, "XOR") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_xor_reg_reg(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            encode_xor_reg_imm(buf, op1.reg, op2.imm);
        }
    }
    else if (my_strcmp(instr->mnemonic, "MOVZX") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            if (is_reg16(instr->operand2))
                encode_movzx_reg_reg16(buf,op1.reg,op2.reg);
            else
                encode_movzx_reg_reg8(buf, op1.reg, op2.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "MOVSX") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_movsx_reg_reg8(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && (op2.type == OPERAND_MEMORY || op2.type == OPERAND_MEMORY_DIRECT)) {
            encode_movsx_reg_mem(buf, op1.reg, &op2);
        }
    }
    else if (my_strcmp(instr->mnemonic, "SHL") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);
        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            if (op2.imm == 1) encode_shl_reg_one(buf, op1.reg);
            else encode_shl_reg_imm(buf, op1.reg, op2.imm);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            // CL is register 1
            if (op2.reg == 1) encode_shl_reg_cl(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "SHR") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);
        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            if (op2.imm == 1) encode_shr_reg_one(buf, op1.reg);
            else encode_shr_reg_imm(buf, op1.reg, op2.imm);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            // CL is register 1
            if (op2.reg == 1) encode_shr_reg_cl(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "OR") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);
        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_or_reg_reg(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            encode_or_reg_imm(buf, op1.reg, op2.imm);
        }
    }
    else if (my_strcmp(instr->mnemonic, "ADD") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (instr->operand1 && my_strcmp(instr->operand1, "DWORDPTR") == 0) {
            Operand mem_op = parse_operand(instr->operand2);
            Operand reg_op = parse_operand(instr->operand3);

            if ((mem_op.type == OPERAND_MEMORY || mem_op.type == OPERAND_MEMORY_DIRECT) && reg_op.type == OPERAND_REGISTER) {
                encode_add_mem_reg(buf, &mem_op, reg_op.reg);
            }
        } else {
            if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
                encode_add_reg_reg(buf, op1.reg, op2.reg);
            } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
                encode_add_reg_imm(buf, op1.reg, op2.imm);
            } else if ((op1.type == OPERAND_MEMORY || op1.type == OPERAND_MEMORY_DIRECT) && op2.type == OPERAND_IMMEDIATE) {
                encode_add_mem_imm(buf, &op1, op2.imm);
            }else if ((op1.type == OPERAND_MEMORY || op1.type == OPERAND_MEMORY_DIRECT) && op2.type == OPERAND_REGISTER) {
                encode_add_mem_reg(buf, &op1, op2.reg);
            }
        }
    }
    else if (my_strcmp(instr->mnemonic, "SUB") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_sub_reg_reg(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            encode_sub_reg_imm(buf, op1.reg, op2.imm);
        }
        else if ((op1.type == OPERAND_MEMORY || op1.type == OPERAND_MEMORY_DIRECT) && op2.type == OPERAND_IMMEDIATE){
            encode_sub_mem_imm(buf, &op1, op2.imm);
        }
        else if ((op1.type == OPERAND_MEMORY || op1.type == OPERAND_MEMORY_DIRECT) && op2.type == OPERAND_REGISTER){
            encode_sub_mem_reg(buf, &op1, op2.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "IMUL") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            encode_imul_reg_imm(buf, op1.reg, op2.imm);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_imul_reg_reg(buf, op1.reg, op2.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "IDIV") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_REGISTER) {
            encode_idiv_reg(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "NEG") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_REGISTER) {
            encode_neg_reg(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "NOT") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) {
            encode_not_reg(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "CDQ") == 0) {
        encode_cdq(buf);
    }
    else if (my_strcmp(instr->mnemonic, "RDTSC") == 0) {
        // RDTSC -> 0F 31
        buffer_emit_byte(buf, 0x0F);
        buffer_emit_byte(buf, 0x31);
    }
    else if (my_strcmp(instr->mnemonic, "RET") == 0) {
        encode_ret(buf);
    }
    else if (my_strcmp(instr->mnemonic, "PUSH") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_REGISTER) {
            encode_push_reg(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "POP") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_REGISTER) {
            encode_pop_reg(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "CALL") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_MEMORY_DIRECT) {
            encode_call_mem(buf, op1.addr);
        } else if (op1.type == OPERAND_IMMEDIATE) {
            encode_call_rel(buf, op1.imm);
        }
        else if(op1.type==OPERAND_REGISTER){
            encode_call_reg(buf, op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "JMP") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_IMMEDIATE) {
            encode_jmp_rel(buf, op1.imm);
        }
        else if (op1.type == OPERAND_REGISTER){
            encode_jmp_reg(buf,op1.reg);
        }
    }
    else if (my_strcmp(instr->mnemonic, "JE") == 0 || my_strcmp(instr->mnemonic, "JZ") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_IMMEDIATE) {
            encode_je_rel(buf, op1.imm);
        }
    }
    else if (my_strcmp(instr->mnemonic, "JNE") == 0) {
        Operand op1 = parse_operand(instr->operand1);

        if (op1.type == OPERAND_IMMEDIATE) {
            encode_jne_rel(buf, op1.imm);
        }
    }
    else if (my_strcmp(instr->mnemonic, "JNZ") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x85);
    }
    else if (my_strcmp(instr->mnemonic, "JB") == 0 || my_strcmp(instr->mnemonic, "JC") == 0 || my_strcmp(instr->mnemonic, "NAE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x82);
    }
    else if (my_strcmp(instr->mnemonic, "JBE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x86);
    }
    else if (my_strcmp(instr->mnemonic, "JA") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x87);
    }
    else if (my_strcmp(instr->mnemonic, "JAE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x83);
    }
    else if (my_strcmp(instr->mnemonic, "JS") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x88);
    }
    else if (my_strcmp(instr->mnemonic, "JNS") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x89);
    }
    else if (my_strcmp(instr->mnemonic, "JL") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x8C);
    }
    else if (my_strcmp(instr->mnemonic, "JGE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x8D);
    }
    else if (my_strcmp(instr->mnemonic, "JLE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x8E);
    }
    else if (my_strcmp(instr->mnemonic, "JG") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_cond_rel(buf, op1.imm, 0x8F);
    }
    else if (my_strcmp(instr->mnemonic, "TEST") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_test_reg_reg(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            if (is_reg8(instr->operand1)) {
                encode_test_reg8_imm(buf, op1.reg, op2.imm);
            } else {
                // TEST r/m32, imm32 -> F7 /0 id
                buffer_emit_byte(buf, 0xF7);
                buffer_emit_byte(buf, modrm(3, 0, op1.reg));
                buffer_emit_dword(buf, op2.imm);
            }
        }
    }
    else if (my_strcmp(instr->mnemonic, "CMP") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        Operand op2 = parse_operand(instr->operand2);

        if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_REGISTER) {
            encode_cmp_reg_reg(buf, op1.reg, op2.reg);
        } else if (op1.type == OPERAND_REGISTER && op2.type == OPERAND_IMMEDIATE) {
            encode_cmp_reg_imm(buf, op1.reg, op2.imm);
        }
    }
    else if (my_strcmp(instr->mnemonic, "SETNZ") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_setnz(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "SETE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_sete(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "SETG") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_setg(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "SETGE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_setge(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "SETL") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_setl(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "SETLE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_setle(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "SETNE") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_REGISTER) encode_setne(buf, op1.reg);
    }
    else if (my_strcmp(instr->mnemonic, "INT") == 0) {
        Operand op1 = parse_operand(instr->operand1);
        if (op1.type == OPERAND_IMMEDIATE) encode_int(buf, op1.imm);
    }
    else if(str_eq(instr->mnemonic,"PUSHAD")){
        buffer_emit_byte(buf,0x60);
    }
    else if(str_eq(instr->mnemonic,"POPAD")){
        buffer_emit_byte(buf,0x61);
    }
}


////////codegenend///////////////////////////////////////////////////////



void assemble_instructions(struct x86_instruction** instrs,code_buffer* buf) {
    for (int i=0;instrs[i]!=NULL;i++) {
        //printf("1i");
        encode_instruction(buf,instrs[i]);
    }

}

void write_program_to_memory(code_buffer* buf,uint32_t address){
    uint8_t* ptr=(uint8_t*)address;

    uint8_t* buffer=buf->bytes;
    int i;
    for (i=0;i<buf->size;i++){
        ptr[i]=buffer[i];
    }
    ptr[i]='\0';


}

void byte_to_hex(uint8_t byte, char *output) {
    const char hex_chars[] = "0123456789ABCDEF";
    output[0] = hex_chars[(byte >> 4) & 0x0F];  // High nibble
    output[1] = hex_chars[byte & 0x0F];          // Low nibble
    output[2] = '\0';
}

code_buffer* program_buffer;

void generate_bytes() {
    program_buffer=MALLOC(sizeof(code_buffer));//main programs buffer
    program_buffer->size=0;
    program_buffer->capacity=1024;
    program_buffer->bytes=MALLOC(program_buffer->capacity);



    code_buffer* strcpy_buf=MALLOC(sizeof(code_buffer));
    strcpy_buf->size=0;
    strcpy_buf->capacity=1024;
    strcpy_buf->bytes=MALLOC(strcpy_buf->capacity);

    code_buffer* strcat_buf=MALLOC(sizeof(code_buffer));
    strcat_buf->size=0;
    strcat_buf->capacity=1024;
    strcat_buf->bytes=MALLOC(strcat_buf->capacity);

    code_buffer* asm_malloc_buf=MALLOC(sizeof(code_buffer));
    asm_malloc_buf->size=0;
    asm_malloc_buf->capacity=1024;
    asm_malloc_buf->bytes=MALLOC(asm_malloc_buf->capacity);

    code_buffer* strlen_buf=MALLOC(sizeof(code_buffer));
    strlen_buf->size=0;
    strlen_buf->capacity=1024;
    strlen_buf->bytes=MALLOC(strlen_buf->capacity);


    for (int i=0;i<10;i++) {//gen helper funcs
        //printf("\nhelpers[i]->name=%s\n",helpers[i]->name);
        if (str_eq(helpers[i]->name,"strcpy")) {
            assemble_instructions(helpers[i]->instructions,strcpy_buf);
            //printf("successfully assembled strcpy\n");


        }
         if (str_eq(helpers[i]->name,"strcat")) {
            assemble_instructions(helpers[i]->instructions,strcat_buf);
            //printf("successfully assembled strcat\n");

        }
         if (str_eq(helpers[i]->name,"malloc")) {
            //printf("FOUND malloc\n");
            assemble_instructions(helpers[i]->instructions,asm_malloc_buf);
            //printf("successfully assembled malloc\n");
            break;

        }
         if (str_eq(helpers[i]->name,"strlen")) {
            assemble_instructions(helpers[i]->instructions,strlen_buf);
            //printf("successfully assembled strlen\n");

        }


    }
    ///printf("Out of for loop\n");



    //////////////////////////////////////////////////////////////



    assemble_instructions(instructions,program_buffer);//main program
    int x=93;
    int y=3;

    /*for (int i=0;i<buffer->size;i++){
        char buf[10];
        byte_to_hex(buffer->bytes[i],buf);
        write_rowcol_text(y++,x,buf,BLUE);
        if (y>=40){
            y=3;
            x+=3;
        }
    }*/


    write_program_to_memory(program_buffer,BASE_ADDRESS);

    write_program_to_memory(strcpy_buf,STR_CPY_ADDRESS);

    write_program_to_memory(strcat_buf,STR_CAT_ADDRESS);

    write_program_to_memory(asm_malloc_buf,MALLOC_ADDRESS);

    write_program_to_memory(strlen_buf,STR_LEN_ADDRESS);

    FREE(program_buffer);
    FREE(strcpy_buf);
    FREE(strcat_buf);
    FREE(asm_malloc_buf);
    FREE(strlen_buf);


}

void compile_cm(char *file) {
    int y=3;

    //write_rowcol_text(y++,0,"lexxing cm...",BLUE);
    lex_cm(file);

    if (error_index)return;

    /*for (int i=0;i<30;i++){
        char* msg=str_cat(str_cat(tokens[i].type," , "),tokens[i].value);
        write_rowcol_text(i+5,65,msg,BLUE);
    }*/
    

    //write_rowcol_text(y++,0,"parsing...",BLUE);

    parse_program();
    if (error_index)return;

    //printf("\n\n\n\n");
    //print_all_nodes();

    //write_rowcol_text(y++,0,"semantically analysing...",BLUE);
    semantic_analysis();
    if (error_index)return;



    ///printf("ENTERING GEN HELPER FUNCS\n");
    //write_rowcol_text(y++,0,"generating helper functions...",BLUE);
    
    
    
    //printf("LEFT GEN HELPER FUNCS\n");
    //write_rowcol_text(y++,0,"generating x86...",BLUE);
    generate_x86();
    if (error_index)return;

    //generate_data();
    //write_rowcol_text(y++,0,"finally, generating bytes...",BLUE);

    generate_bytes();
    //if (error_index)return;
    //draw_rect(0,0,100,20,BLUE);

}

/*void print_ast(struct ASTNode *node, int depth) {
    if (node == NULL) {
        return;
    }

    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // Print node info
    printf("Node: type='%s'", node->type ? node->type : "NULL");

    if (node->value) {
        printf(", value='%s'", node->value);
    }
    if (node->identifier) {
        printf(", id='%s'", node->identifier);
    }
    if (node->var_type) {
        printf(", var_type='%s'", node->var_type);
    }
    if (node->op) {
        printf(", op='%s'", node->op);
    }
    if (node->return_type) {
        printf(", return type='%s'", node->return_type);
    }
    if (node->array_dimensions) {
        printf(", dimensions='%d'", node->array_dimensions);
    }


    printf("\n");

    // Print children
    if (node->left) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("LEFT:\n");
        print_ast(node->left, depth + 1);
    }

    if (node->right) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("RIGHT:\n");
        print_ast(node->right, depth + 1);
    }

    if (node->condition) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("CONDITION:\n");
        print_ast(node->condition, depth + 1);
    }

    if (node->param1) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("PARAM1:\n");
        print_ast(node->param1, depth + 1);
    }

    if (node->param2) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("PARAM2:\n");
        print_ast(node->param2, depth + 1);
    }

    if (node->param3) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("PARAM3:\n");
        print_ast(node->param3, depth + 1);
    }

    if (node->param4) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("PARAM4:\n");
        print_ast(node->param4, depth + 1);
    }

    if (node->fordeclaration) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("FOR_INIT:\n");
        print_ast(node->fordeclaration, depth + 1);
    }

    if (node->forcondition) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("FOR_CONDITION:\n");
        print_ast(node->forcondition, depth + 1);
    }


    if (node->forincrement) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("FOR_INCREMENT:\n");
        print_ast(node->forincrement, depth + 1);
    }
    if (node->indexes) {
        for (int i = 0; node->indexes[i] != NULL; i++) {
            for (int j = 0; j < depth; j++) printf("  ");
            printf("INDEXES[%d]:\n", i);
            print_ast(node->indexes[i], depth + 1);
        }
    }
    if (node->params) {
        for (int i = 0; node->params[i] != NULL; i++) {
            for (int j = 0; j < depth; j++) printf("  ");
            printf("PARAMS[%d]:\n", i);
            print_ast(node->params[i], depth + 1);
        }
    }

    if (node->children) {
        for (int i = 0; node->children[i] != NULL; i++) {
            for (int j = 0; j < depth; j++) printf("  ");
            printf("CHILD[%d]:\n", i);
            print_ast(node->children[i], depth + 1);
        }
    }
    if (node->elements) {
        for (int i = 0; node->elements[i] != NULL; i++) {
            for (int j = 0; j < depth; j++) printf("  ");
            printf("ELEMENT[%d]:\n", i);
            print_ast(node->elements[i], depth + 1);
        }
    }
    if (node->array_sizes) {
        for (int i = 0; node->array_sizes[i] != NULL; i++) {
            for (int j = 0; j < depth; j++) printf("  ");
            printf("ARRAY_SIZES[%d]:\n", i);
            print_ast(node->array_sizes[i], depth + 1);
        }
    }
}

void print_all_nodes() {
    printf("\n\n========== AST ==========\n");
    for (int i = 0; i < nodes_index; i++) {
        printf("\n--- Statement %d ---\n", i);
        print_ast(nodes[i], 0);
    }
    printf("=========================\n\n");
}*/

int get_file_info(char* filename);

char* catfile(char* filename, char* dst){
    int size;

    load_file(filename,FILE_BUF,&size);
    FILE_BUF[size-1]='\0';
    char* retfile=str_cat(FILE_BUF,dst);
    fill_array(FILE_BUF,1024*1024*2,'\0');

    return retfile;
}

char* preprocess(char* file){
    char* orig=file;
    char* x=NULL;
    lex_cm(file);

    for (int i=0;i<token_index;i++){
        //write_rowcol_text(18,18,"IN FOR",RED);
        if (str_eq(tokens[i].value,"include")){
            //write_rowcol_text(19,19,"FOUND 'INCLUDE'",RED);
            i++;
            char* filename=tokens[i].value;

            if (get_file_info(filename)==-1){
                char* msg=str_cat("Could not find file - ",filename);
                emit_error("FILE ERROR", msg);
                FREE(msg);
                return;
            }
            if (x!=NULL)
                FREE(x);

            x=catfile(filename,x==NULL?file:x);
            //return file;

        }
    }
    token_index=0;
    return x==NULL?file:x;

}

void cmmain(char* file) {
    token_capacity = 512;

    program_heap=PROGRAM_HEAP;

    token_index=0;
    x86_node_index=0;

    int data_count = 0;
    int data_capacity = 100;

    address_offset=0;

    nodes_index=0;

    tokens =MALLOC(token_capacity * sizeof(struct Token));

    total_allocated_size=4;
    total_array_size=0;
    
    global_address=BASE_ADDRESS;

    struct ASTNode **__current_child_list = NULL;
    int __current_child_index = -1;
    
    in_function=false;
    function_table=NULL;

    error_capacity=64;
    error_index=0;
    errors=MALLOC(error_capacity*sizeof(struct Error*));

    char* f=preprocess(file);
    if (error_index)return;

    compile_cm(f);

    if (f!=file)
        FREE(f);

    
}



/////////////////////////////////////////////////////////cmcompilerend///



char* setup_graphics(char* str){
    return str_cat("colour(0,0,0);",str);

}

void call_program(){
    __asm__ __volatile__(
        "pushl %%ebp\n"
        "movl %%esp, %%ebp\n"
        "subl %1, %%esp\n"
        "call *%0\n"
        "movl %%ebp, %%esp\n"
        "popl %%ebp\n"
        
        //"ret\n"
        :
        : "r"((uint32_t)BASE_ADDRESS), "r"(total_allocated_size)
        : "memory", "cc"
    );
}

void free_cm(){
    FREE(tokens);
    for (int i=0;i<nodes_index;i++){
        FREE(nodes[i]);
    }
    

    /*FREE(global_table->symbols);
    FREE(global_table);*/

    for (int i=0;i<asm_count;i++){
        FREE(instructions[i]);
    }
    FREE(instructions);

}

void DUMP_x86(){
    uint32_t fake_address=global_address;
    for (int i=0;i<asm_count;i++){
        add_text_to_console(instructions[i]->mnemonic);
        if (instructions[i]->operand1)
            add_text_to_console(instructions[i]->operand1);

        if (instructions[i]->operand2)
            add_text_to_console(instructions[i]->operand2);
        
        char abuf[32];
        uint32_t_str(fake_address, abuf);
        add_text_to_console(str_cat("     __",abuf));
        fake_address+=get_instruction_size(instructions[i]);

        add_char_to_console('\n');
    }

}

void dump_asm(){
    cmmain(textarea->buffer);

    FREE(errors);
    DUMP_x86();
    create_global_message("Check console");

}

int y4=20;

void run_program(){
    if (textbox!=NULL)
        close_text_box();
    
    cmmain(textarea->buffer); //compile file    

    //write_rowcol_text(10,20,"DONE COMPILING!",ORANGE);

    if (error_index>0){
        //write_rowcol_text(20,0,"234897189324914",RED);
        open_errors();

        for (int i=0;i<error_index;i++)FREE(errors[i]);
        FREE(errors);
        free_cm();
        return;
    }
    free_cm();

    FREE(errors);


    running_program=true;
    call_program();
    running_program=false;
    return;

}



_Bool compare_strings(char* string1, char* string2) {
    for (int i = 0; string2[i] != '\0'; i++) {
        if (string1[i] != string2[i]) {
            return 0;
        }
    }
    return 1;
}

_Bool shell_tab = 1;
_Bool files_tab = 0;
_Bool cm_tab = 0;


struct idt_entry {
    unsigned short base_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short base_high;


}__attribute__ ((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
}__attribute__ ((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void set_idt_entry(int index, unsigned int handler_address) {
    idt[index].base_low = handler_address & 0xFFFF;
    idt[index].base_high = (handler_address >> 16) & 0xFFFF;

    // Get the current code segment instead of hardcoding 0x08
    unsigned short cs;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));

    idt[index].selector = cs;
    idt[index].zero = 0;
    idt[index].flags = 0x8E;
}

void load_idt() {
    
    idtp.limit = (sizeof(idt)) - 1;
    idtp.base = (unsigned int)&idt;

    __asm__ __volatile__("lidt (%0)": :"r"(&idtp));
}

void remap_pic(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);  // Master PIC: IRQs start at 32
    outb(0xA1, 0x28);  // Slave PIC: IRQs start at 40
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}



void timer_handler(void){
    TIMER_TICKS++;
    //int* testptr=MALLOC(10);
    

    if (TIMER_TICKS%5==0){
        CURRENT_RAND=gen_rand();
        redraw_screen=true;
    }

    if (TIMER_TICKS%25==0){
        quarter_second=true;
    }

    if (TIMER_TICKS%40==0){
        should_blink=true;
    }
    //FREE(testptr);


    outb(0x20,0x20);
}

volatile _Bool escape=false;

void keyboard_handler(void) {
    unsigned char scancode = inb(0x60);
    *OS_KEY=scancode;

    outb(0x20, 0x20);

    if (!running_program)
        handle_key_press(scancode);
    

    if (scancode==0x01){
        //escape=false;
        escape_program();
    }
}

void default_handler(void) {
    outb(0x20, 0x20);
    
}

void write_rowcol_int(int row, int col, int text, uint32_t fg){
    char buf[32];
    int_str(text,buf);
    write_rowcol_text(row,col,buf,fg);
}

void OS_SLEEP(){
    int time;
    
    __asm__ __volatile__("" : "=b"(time));

    

    int goal=TIMER_TICKS+time;
    int y=4;
    __asm__ __volatile__("sti");

    do{

        //write_rowcol_int(y++,20,TIMER_TICKS,BLACK);
        __asm__ __volatile__("hlt");
      

    }while (TIMER_TICKS<goal);

    __asm__ __volatile__("cli");



}

void OS_PRINT_DEBUG(){
    int value;

    __asm__ __volatile__ ("" : "=a"(value));

    char vbuf[32];
    int_str(value,vbuf);

    for (int i=0;vbuf[i]!='\0';i++){
        console_buf[console_index++]=vbuf[i];

    }
    console_buf[console_index++]='\n';
    console_buf[console_index]='\0';

}

void syscall_handler(void){
    /*OS_PRINT_DEBUG();
    return;*/


    int syscall_number;

    __asm__ __volatile__("" : "=a"(syscall_number));

    

    switch (syscall_number){
        case 0:{
            OS_PRINT();
            break;
        }
        case 1:{
            OS_INPUT();
            break;
        }
        case 2:{
            OS_SLEEP();
            break;
        }
        default: break;
    }

    
}

void mouse_handler(void) {
    uint8_t b = inb(0x60);

    if (mouse_cycle == 0) {
        if ((b & 0x08) == 0) {
            
            outb(0xA0, 0x20);
            outb(0x20, 0x20);
            return;
        }
        mouse_byte[0] = b;
        mouse_cycle = 1;
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }

    if (mouse_cycle == 1) {
        mouse_byte[1] = b;
        mouse_cycle = 2;
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }

    // third byte — full packet
    if (mouse_cycle == 2) {
        mouse_byte[2] = b;

        // Convert to signed 8-bit deltas
        int dx = (int)((int8_t)mouse_byte[1]);
        int dy = (int)((int8_t)mouse_byte[2]);

        
        mouse_x += dx;
        mouse_y -= dy; // typical PS/2: y is inverted relative to screen
        
        hover_mouse();

        if (mouse_drawing_enabled){
            mouse_needs_redrawn=true;
        }

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1024) mouse_x = 1024-50;
        if (mouse_y > 768)  mouse_y = 768-50;

        
        mouse_cycle = 0;

        if (mouse_byte[0] & 0x01) {
            MOUSE_HELD=true;
            
            
        }
        if(!(mouse_byte[0] & 1))
            if(MOUSE_HELD){
                left_click_mouse();
                MOUSE_HELD=false;
            }
            
        
        if (mouse_byte[0]&0b10){
            right_click_mouse();
        }
        
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }
}

uint32_t get_last_sector(){

    uint8_t buffer[512];
    read_sector_poll(1,buffer);

    uint32_t current_free = buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);

    return current_free;
}

uint32_t allocate_sector() {//int current_free=allocate_sector(); write_sector_poll(current_free,data); something like that...
    uint8_t buffer[512];
    
    read_sector_poll(1, buffer);

    uint32_t current_free = buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
    uint32_t next_free = current_free + 1;

    buffer[4] = next_free & 0xff;
    buffer[5] = (next_free >> 8) & 0xff;
    buffer[6] = (next_free >> 16) & 0xff;
    buffer[7] = (next_free >> 24) & 0xff;

    write_sector_poll(1, buffer);
    
    return current_free;

}

int load_file(char* filename, uint8_t* output_buffer, int* size) {
    uint8_t metadata[512];
    read_sector_poll(1, metadata);

    uint32_t file_count = metadata[0] | (metadata[1] << 8) | (metadata[2] << 16) | (metadata[3] << 24);

    for (uint32_t i = 0; i < file_count; i++) {
        int entries_per_sector = 12;
        int table_sector = 2 + (i / entries_per_sector);
        int entry_offset = (i % entries_per_sector) * 40;

        
        uint8_t table[512];
        read_sector_poll(table_sector, table);

        _Bool match = 1;

        char name[32]={'\0'};

        for (int j=0;j<32;j++){
            name[j]=table[entry_offset+j];
        }

        match=str_eq(name,filename);

        if (match) {
            *size = (table[entry_offset + 32]) | (table[entry_offset + 33]<<8) | (table[entry_offset + 34]<<16) | (table[entry_offset + 35]<<24);
            int first_sector = (table[entry_offset + 36]) | (table[entry_offset + 37] << 8) | (table[entry_offset + 38] << 16) | (table[entry_offset + 39] << 24);

            int sectors_needed = (*size + 511) / 512;

            int x = 20;
            int y = 10;
            for (int s = 0; s < sectors_needed; s++) {
                uint8_t buffer[512];
                read_sector_poll(first_sector + s, buffer);

                int bytes_to_copy = 512;
                if (s == sectors_needed - 1) {
                    int remainder = *size % 512;
                    if (remainder != 0) {
                        bytes_to_copy = remainder;
                    }
                }
                
                for (int j = 0; j < bytes_to_copy; j++) {
                    output_buffer[s * 512 + j] = buffer[j];

                }
                if (y >= 40) {
                    x += 3;
                    y = 10;
                }
            }
            return 1;

        }
    }
    return 0;

}

char* get_file_extension(char* file);

void rename_file(char* orig, char* new){
    int s,o;
    char* fex;
    for (int i=2;i<=50;i++){
        s=i;

        uint8_t data[512];
        read_sector_poll(s,data);

        _Bool found=false;
        for (int j=0;j<12;j++){
            o=j*40;
            
            if (str_eq(orig,data+o)){
                fex=get_file_extension(data+o);
                found=true;
                break;
            }
        }
        if (found)break;
    }

    uint8_t data[512];
    read_sector_poll(s,data);
    int i;
    for (i=0;i<31-(1+len(fex)) && new[i]!='\0';i++){
        data[o+i]=new[i];
    }
    data[o+i]='.';
    i++;
    for (int j=0;fex[j]!='\0';j++,i++){
        data[o+i]=fex[j];
    }
    data[o+i]='\0';
    write_sector_poll(s,data);

}

int delete_file(char* filename){
    int found_file=get_file_info(filename);

    if (found_file==-1){
        print_to_console("ERROR: File not found!");
        return;
    }

    uint8_t metadata[512];
    read_sector_poll(1,metadata);
    uint32_t file_count=metadata[0] | (metadata[1]<<8) | (metadata[2]<<16) | (metadata[3]<<24);

    file_count--;

    metadata[0]=file_count & 0xff;
    metadata[1]=(file_count>>8)&0xff;
    metadata[2]=(file_count>>16)&0xff;
    metadata[3]=(file_count>>24)&0xff;

    write_sector_poll(1,metadata);

    int i,offset;
    for (i=2;i<51;i++){
        uint8_t sector[512];
        read_sector_poll(i,sector);

        _Bool found=false;

        for (int j=0;j<12;j++){
            offset=j*40;

            found=str_eq(sector+offset,filename);

            if (found)break;
        }
        if (found)break;
        
    }


    {
        uint8_t sector[512];
        read_sector_poll(i, sector);

        for (int j = offset / 40; j < 11; j++){
            int o1 = j * 40;
            int o2 = (j + 1) * 40;
            memcpy(sector + o1, sector + o2, 40);
        }
        uint8_t sector2[512];
        read_sector_poll(i + 1, sector2);

        memcpy(sector + (11 * 40), sector2, 40);
        write_sector_poll(i, sector);
    }

    for (int j=i+1;j<51;j++){
        uint8_t sector[512];
        uint8_t sector2[512];
        read_sector_poll(j,sector);
        read_sector_poll(j+1,sector);

        for (int k=0;k<11;k++){
            int o1=k*40,o2=(k+1)*40;

            memcpy(sector+o1,sector+o2,40);

        }

        if (j<50){
            memcpy(sector+(11*40),sector2,40);
        }

    }


}

uint32_t get_file_count();

uint32_t get_background_colour(){
    uint8_t buf[512];
    read_sector_poll(1,buf);
    uint32_t colour=buf[49] | (buf[50] << 8) | (buf[51] << 16) | (buf[52] << 24);
    return !colour?DARK_RED:colour;

}

void set_background_colour(uint32_t colour){
    uint8_t buf[512];
    read_sector_poll(1,buf);
    buf[49]=colour&0xff;
    buf[50]=(colour>>8)&0xff;
    buf[51]=(colour>>16)&0xff;
    buf[52]=(colour>>24)&0xff;

}

_Bool get_is_background_set(){

    uint8_t buf[512];
    read_sector_poll(1,buf);

    return buf[8];

}

Image load_image(char* filename);

void load_bg_image(){
    restore_mouse();
    uint8_t metadata[512]={0};
    read_sector_poll(1,metadata);
    char buf[32];
    int i;
    for(i=0;i<31 && metadata[9+i]!='\0';i++){
        buf[i]=metadata[9+i];
    }
    buf[i]='\0';

    
    if (!bg_image_copied){
        Image image=load_image(buf);

        bg_image_copied=true;
        background_image_width=image.width, background_image_height=image.height;
        background_image_buf=MALLOC(image.width * image.height * sizeof(uint32_t));

        for (int i=0;i<image.height;i++){
            memcpy32(framebuffer+((50+i)*ppr),image.bitmap+(i*image.width),image.width);
            memcpy32(background_image_buf+(i*image.width),image.bitmap+(i*image.width),image.width);
        }
    }
    else{
        
        for (int i=0;i<background_image_height;i++){
            memcpy32(framebuffer+((50+i)*ppr),background_image_buf+(i*background_image_width),background_image_width);
        }
    }
    save_mouse();
    
    //while(1)__asm__ __volatile__("hlt");

}

uint32_t get_file_size(char* filename){
    get_file_info(filename);
    
    return FILE_INFO_BUF[32] | (FILE_INFO_BUF[33]<<8) | (FILE_INFO_BUF[34]<<16) | (FILE_INFO_BUF[35]<<24);

}

uint32_t get_first_sector(char* filename){
    get_file_info(filename);
    
    return FILE_INFO_BUF[36] | (FILE_INFO_BUF[37]<<8) | (FILE_INFO_BUF[38]<<16) | (FILE_INFO_BUF[39]<<24);

}

void save_bitmap_file(){
    char* fex=".bmp";
    char* fullname=str_cat(textbox->buffer,fex);
   
    if (get_file_info(fullname)!=-1){
        FREE(fullname);
        create_global_message("File already exists!");
        return;
    }
    else if (len(fullname)>=32){
        create_global_message("Filename too long!");
        FREE(fullname);
        return;
    }

    memcpy(resave_file_name,fullname,32);

    uint32_t width=drawarea->width;
    uint32_t height=drawarea->height;

    uint32_t file_count=get_file_count();
    inc_file_count();

    uint32_t entry_sector=2+(file_count/12);
    uint32_t entry_offset=40*(file_count%12);
    uint32_t total_size=10+(drawarea->width*drawarea->height*sizeof(uint32_t));
    uint32_t int_size=drawarea->width*drawarea->height;

    uint32_t sectors_needed=(511+total_size)/512;
    uint32_t first_sector=allocate_sector();
    uint32_t bmp_ind=0;

    
    for (int i=0;i<sectors_needed;i++){
        uint8_t data[512]={0};
        if (i==0){
            data[0]=width&0xff;
            data[1]=(width>>8)&0xff;
            data[2]=(width>>16)&0xff;
            data[3]=(width>>24)&0xff;
            data[4]='\0';
            data[5]=height&0xff;
            data[6]=(height>>8)&0xff;
            data[7]=(height>>16)&0xff;
            data[8]=(height>>24)&0xff;
            data[9]='\0';

            for (int ind=10;ind<512-3 && bmp_ind<int_size;){
                data[ind]=drawarea->bitmap[bmp_ind]&0xff;
                ind++;
                data[ind]=(drawarea->bitmap[bmp_ind]>>8)&0xff;
                ind++;
                data[ind]=(drawarea->bitmap[bmp_ind]>>16)&0xff;
                ind++;
                data[ind]=(drawarea->bitmap[bmp_ind]>>24)&0xff;
                ind++,bmp_ind++;
            }
            //write_rowcol_int(20,5,first_sector,RED);
            
            write_sector_poll(first_sector,data);
            continue;

        }
        
        for (int ind=0;ind<512-3 && bmp_ind<int_size;){
            data[ind]=drawarea->bitmap[bmp_ind]&0xff;
            ind++;
            data[ind]=(drawarea->bitmap[bmp_ind]>>8)&0xff;
            ind++;
            data[ind]=(drawarea->bitmap[bmp_ind]>>16)&0xff;
            ind++;
            data[ind]=(drawarea->bitmap[bmp_ind]>>24)&0xff;
            ind++,bmp_ind++;
        }
        write_sector_poll(allocate_sector(),data);

    }

    uint8_t toc[512];
    read_sector_poll(entry_sector,toc);
    
    int i;
    for (i=0;fullname[i]!='\0';i++){
        toc[entry_offset+i]=fullname[i];
        
    }
    toc[entry_offset+i]='\0';

    toc[entry_offset+32]=total_size&0xff;
    toc[entry_offset+33]=(total_size>>8)&0xff;
    toc[entry_offset+34]=(total_size>>16)&0xff;
    toc[entry_offset+35]=(total_size>>24)&0xff;

    toc[entry_offset+36]=first_sector&0xff;
    toc[entry_offset+37]=(first_sector>>8)&0xff;
    toc[entry_offset+38]=(first_sector>>16)&0xff;
    toc[entry_offset+39]=(first_sector>>24)&0xff;

    write_sector_poll(entry_sector,toc);

    FREE(fullname);

    if (textbox!=NULL)
        close_text_box();
    
    create_global_message("File saved!");

}

void copy_file(char* orig, char* new){
    get_file_info(orig);

    uint32_t sector=FILE_INFO_BUF[36] | (FILE_INFO_BUF[37]<<8) | (FILE_INFO_BUF[38]<<16) | (FILE_INFO_BUF[39]<<24);
    uint32_t file_size=FILE_INFO_BUF[32] | (FILE_INFO_BUF[33]<<8) | (FILE_INFO_BUF[34]<<16) | (FILE_INFO_BUF[35]<<24);

    uint32_t sectors_needed=(file_size+511)/512;

    uint32_t first_sector=allocate_sector();
    for (int i=0;i<sectors_needed;i++){
        uint8_t data[512];
        read_sector_poll(sector+i,data);
        write_sector_poll(i==0?first_sector:allocate_sector(),data);

    }
    
    uint8_t metadata[512];
    read_sector_poll(1,metadata);
    uint32_t file_count=metadata[0] | (metadata[1]<<8) | (metadata[2]<<16) | (metadata[3]<<24);

    int entry_sector=2+(file_count/12);
    int entry_offset=40*(file_count%12);

    uint8_t toc[512];
    read_sector_poll(entry_sector,toc);

    char* fex=get_file_extension(orig);
    uint8_t newbuf[32]={0};
    newbuf[0]='.';
    for (int i=0;fex[i]!='\0';i++){
        newbuf[1+i]=fex[i];
    }
    char* newname=str_cat(new,newbuf);

    {
        int i;
        for (i=0;i<31 && newname[i]!='\0';i++){
            toc[entry_offset+i]=newname[i];
        }
        toc[entry_offset+i]='\0';
    }

    FREE(newname);

    toc[entry_offset+36]=first_sector&0xff;
    toc[entry_offset+37]=(first_sector>>8)&0xff;
    toc[entry_offset+38]=(first_sector>>16)&0xff;
    toc[entry_offset+39]=(first_sector>>24)&0xff;

    toc[entry_offset+32]=file_size&0xff;
    toc[entry_offset+33]=(file_size>>8)&0xff;
    toc[entry_offset+34]=(file_size>>16)&0xff;
    toc[entry_offset+35]=(file_size>>24)&0xff;

    write_sector_poll(entry_sector,toc);

    file_count++;

    metadata[0]=file_count&0xff;
    metadata[1]=(file_count>>8)&0xff;
    metadata[2]=(file_count>>16)&0xff;
    metadata[3]=(file_count>>24)&0xff;

    write_sector_poll(1,metadata);

}


int defragment_files(){
    uint8_t metadata[512];
    read_sector_poll(1,metadata);

    uint32_t file_count=metadata[0] | (metadata[1] << 8) | (metadata[2] << 16) | (metadata[3] << 24);
    uint32_t sector_count=get_last_sector()-50;

    uint32_t curr_sector=51;

    sector_t* sectors=MALLOC(sector_count*sizeof(sector_t));
    int s_index=0;

    for (int i=0;i<file_count;i++){
        int toc_sector=2+(i/12);
        int entry_offset=40*(i%12);

        uint8_t toc[512];
        read_sector_poll(toc_sector,toc);

        int file_sector=toc[entry_offset+36] | (toc[entry_offset+37]<<8) | (toc[entry_offset+38]<<16) | (toc[entry_offset+39]<<24);
        int file_size=toc[entry_offset+32] | (toc[entry_offset+33]<<8) | (toc[entry_offset+34]<<16) | (toc[entry_offset+35]<<24);

        int new_sector=curr_sector++;
        int old_sector=file_sector;

        int sectors_needed=(file_size+511)/512;

        // print_to_console("\n");
        // print_to_console(str_cat("FILE SIZE: ",quick_int_str(file_size)));

        for (int j=0;j<sectors_needed;j++){
            uint8_t data[512];
            read_sector_poll(old_sector+j,data);

            sector_t new;
            new.sector=j==0?new_sector:curr_sector++;
            memcpy(new.data,data,512);
            sectors[s_index++]=new;

            
            // print_to_console(toc+entry_offset);
            // print_to_console(str_cat("OLD SECTOR: ",quick_int_str(old_sector+j)));
            // print_to_console(str_cat("NEW SECTOR: ",quick_int_str(new.sector)));

        }

        toc[entry_offset+36]=new_sector&0xff;
        toc[entry_offset+37]=(new_sector>>8)&0xff;
        toc[entry_offset+38]=(new_sector>>16)&0xff;
        toc[entry_offset+39]=(new_sector>>24)&0xff;
        
        write_sector_poll(toc_sector,toc);


    }

    metadata[4]=curr_sector&0xff;
    metadata[5]=(curr_sector>>8)&0xff;
    metadata[6]=(curr_sector>>16)&0xff;
    metadata[7]=(curr_sector>>24)&0xff;

    write_sector_poll(1,metadata);


    for (int i=0;i<s_index;i++){
        sector_t sector=sectors[i];
        uint8_t data[512];
        memcpy(data,sector.data,512);
        write_sector_poll(sector.sector,data);

    }
    FREE(sectors);


}

uint32_t get_file_count(){
    uint8_t md[512];
    read_sector_poll(1,md);

    return md[0] | (md[1]>>8) | (md[2]>>16) | (md[3]>>24);
}


void inc_file_count(){
    uint32_t fc=get_file_count();
    fc++;

    uint8_t md[512];

    read_sector_poll(1,md);

    md[0]=fc&0xff;
    md[1]=(fc>>8)&0xff;
    md[2]=(fc>>16)&0xff;
    md[3]=(fc>>24)&0xff;

    write_sector_poll(1,md);

}

int get_file_info(char* filename){
    for (int i=2;i<=50;i++){
        uint8_t data[512];
        read_sector_poll(i,data);

        for (int j=0;j<12;j++){
            if (str_eq(data+j*40,filename)){
                memcpy(FILE_INFO_BUF,data+j*40,40);
                return 1;
            }
        }
        


    }
    return -1;
}

void init_files();

void submit_init_files(){
    _Bool b=false;
    if (str_eq(textbox->buffer,"y") || str_eq(textbox->buffer,"Y"))
        b=true;

    close_text_box();

    if (!b)return;

    refresh_all_scrollareas();
    
    init_files();

}

void ask_yes_no_files(){
    create_text_box(300,350,350,200,textarea->bg,textarea->fg,"Are you sure? (y/n)",false);
    create_textbox_button(NULL,BLACK,NULL,"Submit",submit_init_files,NULL);

}

int get_inp(char* title){
    create_text_box(centre_x(300),50,300,150,textarea->bg,textarea->fg,title,false);
    create_textbox_button(NULL,ORANGE,NULL,"Submit",SUBMIT_INPUT,NULL);

    INPUT_HANG();
    if (textbox!=NULL)close_text_box();
    return *INPUT_VALUE;

}


void init_files() {
    uint8_t buffer[512] = { 0 };

    buffer[0] = 0; //file count
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;

    buffer[4] = 51; //next sector available
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;

    buffer[8]=0; //background image is set?

    /*
        buffer[9] - buffer[48] 
        full background entry
    */

    //background colour (default - dark red)
    buffer[49]=DARK_RED & 0xff;
    buffer[50]=(DARK_RED>>8) & 0xff;
    buffer[51]=(DARK_RED>>16) & 0xff;
    buffer[52]=(DARK_RED>>24) & 0xff;


    write_sector_poll(1, buffer);

    uint8_t empty[512] = { 0 };
    for (int i = 2; i <= 50; i++) {
        write_sector_poll(i, empty);
    }
}

void save_file(uint8_t* filename, uint8_t* data, int size) {
    uint8_t metadata[512];
    read_sector_poll(1, metadata);

    uint32_t file_count = metadata[0] | (metadata[1] << 8) | (metadata[2] << 16) | (metadata[3] << 24);
    

    int sectors_needed = (size + 511) / 512;
    uint32_t first_sector = allocate_sector();

    for (int i = 0; i < sectors_needed; i++) {
        uint8_t buffer[512] = { '\0' };

        int bytes_to_write = 512;
        if (i == sectors_needed - 1) {
            int remainder = size % 512;
            if (remainder != 0) {
                bytes_to_write = remainder;
            }
        }

        for (int j = 0; j < bytes_to_write; j++) {
            buffer[j] = data[i * 512 + j];
        }

        uint32_t sector_to_write = (i == 0) ? first_sector : allocate_sector();

        write_sector_poll(sector_to_write, buffer);
        
        
        
    }

    
    int entries_per_sector = 12;  
    int table_sector = 2 + (file_count / entries_per_sector);  
    int entry_offset = (file_count % entries_per_sector) * 40;

   
    uint8_t table[512];
    read_sector_poll(table_sector, table);

    for (int i = 0; i < 32; i++) {
        table[entry_offset + i] = '\0';
    }
   
    for (int i = 0; i < 32; i++) {
        table[entry_offset + i] = filename[i];
        if (filename[i] == '\0') break;
    }

    
    table[entry_offset + 32] = size & 0xFF;
    table[entry_offset + 33] = (size >> 8) & 0xFF;
    table[entry_offset + 34] = (size >> 16) & 0xFF;
    table[entry_offset + 35] = (size >> 24) & 0xFF;


    table[entry_offset + 36] = first_sector & 0xFF;
    table[entry_offset + 37] = (first_sector >> 8) & 0xFF;
    table[entry_offset + 38] = (first_sector >> 16) & 0xFF;
    table[entry_offset + 39] = (first_sector >> 24) & 0xFF;

    write_sector_poll(table_sector, table);

    
    file_count++;
    read_sector_poll(1, metadata);

    metadata[0] = file_count & 0xFF;
    metadata[1] = (file_count >> 8) & 0xFF;
    metadata[2] = (file_count >> 16) & 0xFF;
    metadata[3] = (file_count >> 24) & 0xFF;

    write_sector_poll(1, metadata);

    create_global_message("File saved!");
}

volatile int disk_ready = 1;

void write_sector_poll(unsigned int lba, unsigned char* buffer) {
    // Wait for disk ready
    int timeout = 1000000;
    while (timeout-- > 0 && (inb(0x1F7) & 0xC0) != 0x40);
    if (timeout == 0) {
        write_text(cursory, 0, "HUNG: Disk not ready for write!", 0x4F);
        return;
    }

    // Send command to write
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x30);

    // Wait for DRQ
    timeout = 1000000;
    while (timeout-- > 0 && !(inb(0x1F7) & 0x08));
    if (timeout == 0) {
        write_text(cursory+1, 0, "HUNG: No DRQ for write!", 0x4F);
        return;
    }

    // Write data
    uint16_t* buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buf[i]);
    }

    // Wait for write to complete
    timeout = 1000000;
    while (timeout-- > 0 && (inb(0x1F7) & 0x80));

}

void read_sector_poll(unsigned int lba, unsigned char* buffer) {
    // Wait for disk ready WITH TIMEOUT
    int timeout = 1000000;
    while (timeout-- > 0 && (inb(0x1F7) & 0xC0) != 0x40);

    if (timeout == 0) {
        return;
    }
    


    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x20);


    // Wait for DRQ WITH TIMEOUT
    timeout = 1000000;
    while (timeout-- > 0 && !(inb(0x1F7) & 0x08));

    if (timeout == 0) {
        return;
    }

    
    uint16_t* buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(0x1F0);
    }

}

char* get_file_extension(char* name){
    int i=len(name)-1;

    char ext[8];
    int j=0;
    for (;name[i]!='.' && i>=0;i--){
        ext[j++]=name[i];


    }
    ext[j]='\0';
    
    char ret[8];
    int k;
    for (k=0;k<j;k++){
        ret[k]=ext[j-1-k];
    }
    ret[k]='\0';
    
    return str_copy(ret);
}

void run_executable(char* filename){
    for (int i=0;filename[i]!='\0';i++){
        if (filename[i]=='.'){
            filename[i]='\0';
            break;
        }
    }


    create_text_area(WHITE,BLACK+1,NULL,NULL,filename,true);

    running_program=true;
    call_program();
    running_program=false;
    open_console();
}



void open_program(char* filename){
    char* prg=(char*)BASE_ADDRESS;
    int size;

    char fn[32];
    int i;
    for  (i=0;filename[i]!='\0';i++){
        fn[i]=filename[i];
    }
    fn[i]='\0';


    int found=load_file(fn,prg,&size);


    if (!found){
        write_rowcol_text(0,60,"NOT FOUND!",YELLOW);
        return;
    }


    run_executable(fn);


}

void list_files() {
    uint8_t metadata[512];
    read_sector_poll(1, metadata);

    uint32_t file_count = metadata[0] | (metadata[1] << 8) | (metadata[2] << 16) | (metadata[3] << 24);

    ScrollArea* documents=create_scrollarea("Documents", 10,100,300,600,0,RED);
    ScrollArea* programs=create_scrollarea("Programs", 350,100,300,600,0,RED);
    ScrollArea* images=create_scrollarea("Images", 700,100,300,600,0,RED);
    
    uint32_t space_used=0;
    
    for (uint32_t i = 0; i < file_count; i++) {
        int entries_per_sector = 512 / 40; 
        int table_sector = 2 + (i / entries_per_sector);
        int entry_offset = (i % entries_per_sector) * 40;  

        uint8_t table[512];
        read_sector_poll(table_sector, table);

        space_used += (table[entry_offset + 32]) | (table[entry_offset + 33]<<8) | (table[entry_offset + 34] << 16) | (table[entry_offset + 35] << 24);

        char filename[40];

        int j;
        for (j=0;j<40&&table[entry_offset+j]!='\0';j++){
            filename[j]=table[entry_offset+j];
        }
        filename[j]='\0';

        char* fex=get_file_extension(filename);

        //write_rowcol_text(i+20,50,filename,GREEN);
        char* fn=str_copy(filename);

        if (str_eq(fex,"cm")||str_eq(fex,"txt")){
            struct button* button=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,fn,true, false,false,NULL,NULL);
            pop_button();
            ScrollButton* btn=create_scrollbutton(documents,documents->buttons_index*2+1,1,button);



        }
        else if(str_eq(fex,"bmp")){
            struct button* button=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,fn,false, false,true,NULL,NULL);
            pop_button();
            ScrollButton* btn=create_scrollbutton(images,images->buttons_index*2+1,1,button);

        }
        else{
            struct button* button=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,fn,false,true,false,NULL,NULL);
            pop_button();
            ScrollButton* btn=create_scrollbutton(programs,programs->buttons_index*2+1,1,button);

        }
        
        FREE(fex);
        //FREE(fn);
        
    }
    char s1[32];
    uint32_t_str(space_used,s1);
    char* s2=str_cat("Total file size: ",s1);
    char* s3=str_cat(s2," Bytes");

    uint8_t sect1[512]={0};
    read_sector_poll(1,sect1);
    uint32_t sectorptr=sect1[4] | (sect1[5]<<8) | (sect1[6] << 16) | (sect1[7] << 24); 
    
    char buf[32];
    uint32_t_str(sectorptr,buf);
    char* s4=str_cat("Sector ptr: ",buf);

    uint16_t disk_info[256];
    get_disk_info(disk_info);

    uint32_t total_sectors=disk_info[60] | (disk_info[61] << 16);
    char total_sectors_buf[32];
    uint32_t_str(total_sectors,total_sectors_buf);


    char* s5=str_cat("Disk capacity: ",total_sectors_buf);
    char* s6=str_cat(s5," sectors");

    int offset=write_rowcol_text(3,2,s4,get_complementary_colour(COLOUR));
    offset=write_rowcol_text(3,offset+3,s3,get_complementary_colour(COLOUR));
    offset=write_rowcol_text(3,offset+3,s6,get_complementary_colour(COLOUR));


    //FREE(s1);
    FREE(s2);
    FREE(s3);
    FREE(s4);
    FREE(s5);
    FREE(s6);

}

void flash_message(char* msg){
    
}

void enable_disk_interrupts() {
    outb(0x3F6, 0x00);
}

void disk_handler(void) {
    disk_ready = 1;
    draw_pixel(0, 1, 0xe00000);

    outb(0xA0,0x20);
    outb(0x20,0x20);
}

void mouse_wait(uint8_t type) {
    for (int i = 0; i < 1000000; i++) {
        uint8_t status = inb(0x64);
        if (type == 0) {
            if (status & 1) return;
        }
        else {
            if (!(status & 2)) return;
        }
    }
}


void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void debug_ps2_config() {
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    uint8_t cfg = inb(0x60);

    int offs = write_text(5, 0, "cfg:", text_colour);
}


void init_mouse() {
    uint8_t status;


    mouse_wait(1);
    outb(0x64, 0xA8);

    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = inb(0x60);

    
    status |= 0x02; 
    status &= ~0x20;


    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);


    mouse_write(0xF6);
    mouse_read();


    mouse_write(0xF4);
    mouse_read();
}

void init_timer(int frequency){
    int divisor=PIT_FREQUENCY/frequency;

    char div[32];
    int_str(divisor,div);

    //write_rowcol_text(44,32,str_cat("divisor=",div),PURPLE);

    outb(PIT_COMMAND,0x36);

    outb(PIT_CHANNEL0,divisor&0xff);
    outb(PIT_CHANNEL0,(divisor>>8)&0xff);
}

void init_idt(void) {
    remap_pic();

    for (int i = 0; i < 256; i++){
        set_idt_entry(i, default_handler_asm);
    }

    set_idt_entry(33, keyboard_handler_asm);
    set_idt_entry(32,timer_handler_asm);
    set_idt_entry(44, mouse_handler_asm);    
    set_idt_entry(100,syscall_handler_asm);

    load_idt();

    // uint8_t master_mask = inb(0x21);
    // master_mask &= ~0x06; 
    // outb(0x21, master_mask);

    // uint8_t slave_mask = inb(0xA1);
    // slave_mask &= ~0x10; 
    // outb(0xA1, slave_mask);

    init_mouse();

    __asm__ __volatile__("sti");
}

void clear_rects(){
    for (int i=0;i<textarea->rectangles_index;i++){
        restore_background(textarea->rectangles[i]->bg);
    }
    textarea->rectangles_index=0;

}

void draw_bordered_rect(int x, int y, int width, int height, uint32_t colour) {
    restore_mouse();
    /*for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            //framebuffer[j * ppr + i] = 0x003b3b3b;
            framebuffer[j * ppr + i] = BLACK;
        }
    }
    for (int i = x + 2; i < x+width - 2; i++) {
        for (int j = y + 2; j < y+height - 2; j++) {
            framebuffer[j * ppr + i] = colour;
        }
    }*/

    draw_rect(x,y,width,height,BLACK);
    draw_rect(x+2,y+2,width-4,height-4,colour);

    save_mouse();
}


void draw_rect(int x, int y, int width, int height,uint32_t colour) {
    restore_mouse();
    /*for (int i = x; i < x + width; i++) {
        for (int j = y; j < height + y; j++) {
            framebuffer[j * ppr + i] = colour;
        }
    }*/
    while (inw(0x3DA) & 8);
    while (!(inw(0x3DA) & 8));

    uint32_t* start=framebuffer+(y*ppr+x);
    for (int i=0;i<height;i++){
        memset32(start,colour,width);
        start+=ppr;
    }

    save_mouse();
}

int write_xy_text(int x, int y, char* msg, uint32_t colour) {
    
    for (int i = 0; msg[i] != '\0'; i++) {
        write_char(x, y, font8x16[msg[i]], colour);
        x += 10;
    }
    return x;
    
}



int write_rowcol_text(int row, int column, const char* msg, uint32_t colour) {
    int coloff = column;
    for (int i = 0; msg[i] != '\0'; i++) {
        write_rowcol_char(coloff++, row, msg[i],colour);
        
    }
    return coloff;
}

void write_centred_text(int row, char* msg, uint32_t colour){
    write_rowcol_text(row,50-(len(msg)),msg,colour);

}

int write_text(int row, int column,const char* msg,uint32_t colour) {

    for (int i = 0; msg[i] != '\0'; i++) {
        uint32_t offset = (row * ppr + column);
        write_char(10*column+3,18*row+5,font8x16[msg[i]],colour);
        column++;
    }
    return column + 1;
}

void configure_program_tab(const char* name) {
    int size = 0;

    for (int i = 0; name[i] != '\0'; i++) {
        size++;
    }

    draw_bordered_rect(0, 25, 1024, 25, 0x00b8b8b8);

    int offset = 20;
    for (int i = 0; name[i] != '\0'; i++) {
        write_char(offset, 30, font8x16[name[i]], 0);
        offset += 10;
    }

    create_button(1024-25,25,25,25,RED,BLACK,HIGHLIGHT_TAB_COLOUR,"c",home);
}

void configure_tabs() {//gullos, v 1.1 |s
    restore_mouse();
    draw_rect(15 * 10 + 20, 0, 2, 25, BLACK);//vertical line
    
    create_button(190,2,80,21,TAB_COLOUR,BLACK,HIGHLIGHT_TAB_COLOUR,"Textpad",textpad);
    create_button(280,2,60,21,TAB_COLOUR,BLACK,HIGHLIGHT_TAB_COLOUR,"Files",files);
    create_button(350,2,70,21,TAB_COLOUR,BLACK,HIGHLIGHT_TAB_COLOUR,"C-",cminus);
    create_button(430,2,80,21,TAB_COLOUR,BLACK,HIGHLIGHT_TAB_COLOUR,"Bitmaker",bit_maker);

    save_mouse();

}

void display_bar(){
    char* heapptr=inttostrhex(((int)heap)+heap_offset);
    char* currfree=inttostrhex((int)free_list);

    char* msg1=str_cat("Heap ptr: ",heapptr);
    char* msg2=str_cat("Current free: ",currfree);

    draw_rect(200,2,800,21,TAB_COLOUR);
    int offset=write_xy_text(210,5,msg1,COLOUR);
    offset=write_xy_text(offset+20,5,msg2,COLOUR);

}

void clear_screen(){


}

void destroy_char_textbox(){
    restore_mouse();

    int x=textbox->x+5+10*textbox->cursorx;
    int y=textbox->y+30+18*textbox->cursory;

    for (int i = x; i < x + 10; i++) {
        
        for (int j = y; j < y + 18; j++) {
            framebuffer[j * ppr + i] = textbox->bg;
        }
    }
    save_mouse();
}

void destroy_char() {
    if (textbox!=NULL){
        destroy_char_textbox();
        return;
    }
    restore_mouse();
    for (int x = textarea->cursorx * 10; x < textarea->cursorx * 10 + 10; x++) {
        int y0 = (textarea->cursory + 3) * 18;
        for (int y = y0; y < y0 + 18; y++) {
            framebuffer[y * ppr + x] = textarea->bg;
        }
    }
    save_mouse();
}

_Bool mouse_graphic_intersects(int x, int y,int width, int height) {
    x -= 20;
    y -= 20;
    width += 20;
    height += 20;
    if (mouse_x>=x&&mouse_y>=y&&mouse_x<=x+width&&mouse_y<=y+height)return true;
    return false;
}

_Bool mouse_intersects(int x, int y,int width, int height) {
    /*x -= 20;
    y -= 20;
    width += 20;
    height += 20;*/
    if (mouse_x>=x&&mouse_y>=y&&mouse_x<=x+width&&mouse_y<=y+height)return true;
    return false;
}

_Bool is_on_char() {
    for (int x = textarea->cursorx * 10; x < textarea->cursorx * 10 + 10; x++) {
        int y0 = (textarea->cursory + 3) * 18;
        for (int y = y0; y < y0 + 18; y++) {
            if (framebuffer[y * ppr + x] != textarea->bg && framebuffer[y * ppr + x] != textarea->bg_highlight) {
                return 1;
            }
        }
    }
    return 0;
}

int get_i_cursory(int i){
    int nc=0;
    int ind=0;
    while (ind!=i){
        if (textarea->buffer[ind]=='\n'){
            nc++;
        }
        ind++;
    }
    
    return nc-textarea->scroll_bounds;

}

int get_i_cursorx(int i){
    int cx=-1;
    do{
        cx++;
        i--;
    }
    while (i>=0 && textarea->buffer[i]!='\n');

    return cx;
}

void unhighlight_char_textbox(){
    if(textbox->cursory<0 || textbox->cursory>textbox->MAX)return;
    restore_mouse();
    int x=textbox->x+5+10*textbox->cursorx;
    int y=textbox->y+30+18*textbox->cursory;
    for (int i = x; i < x + 10; i++) {
        for (int j = y; j < y + 18; j++) {
            if (framebuffer[j * ppr + i] == get_complementary_colour(textbox->fg)) {
                framebuffer[j * ppr + i] = textbox->fg;
            }
            else if (framebuffer[j * ppr + i] == get_complementary_colour(textbox->bg)){
                framebuffer[j * ppr + i] = textbox->bg;
            }
        }
    }
    save_mouse();
}

void unhighlight_char() {
    if (textbox!=NULL){
        unhighlight_char_textbox();
        return;
    }
    restore_mouse();


    if (textarea->selecting){
        
        int start=textarea->selection_start;
        int end=textarea->selection_end;
        
        
        if (start>end){
            int temp=start;
            start=end;
            end=temp;
        }
        

        for (int i=start;i<end;i++){
            if (textarea->buffer[i]=='\n')continue;
            int cx=get_i_cursorx(i);
            int cy=get_i_cursory(i);
            recolour_area(cx*10,(3+cy)*18,10,18,get_complementary_colour(textarea->fg),textarea->fg);
            recolour_area(cx*10,(3+cy)*18,10,18,get_complementary_colour(textarea->bg),textarea->bg);
        }
        
    }


    int y=(textarea->cursory+3);
    for (int i = textarea->cursorx * 10; i < textarea->cursorx * 10+10; i++) {
        for (int j = y * 18; j < y * 18+18; j++) {
            if (framebuffer[j * ppr + i] == get_complementary_colour(textarea->fg)) {
                framebuffer[j * ppr + i] = textarea->fg;
            }
            else if (framebuffer[j * ppr + i] == get_complementary_colour(textarea->bg)) {
                framebuffer[j * ppr + i] = textarea->bg;
            }
        }
    }
    save_mouse();
}

void highlight_char_textbox(){
    if(textbox->cursory<0 || textbox->cursory>textbox->MAX)return;
    restore_mouse();
    int x=textbox->x+5+10*textbox->cursorx;
    int y=textbox->y+30+18*textbox->cursory;
    for (int i = x; i < x+ 10; i++) {
        for (int j = y; j < y + 18; j++) {
            
                if (framebuffer[j * ppr + i] == textbox->fg) {
                    framebuffer[j * ppr + i] = get_complementary_colour(textbox->fg);
                }
                else if (framebuffer[j*ppr+i]==textbox->bg){
                    framebuffer[j * ppr + i] = get_complementary_colour(textbox->bg);
                }
            
        }
    }
    save_mouse();
}

void highlight_char() {
    if (textbox!=NULL){
        highlight_char_textbox();
        return;
    }

    restore_mouse();

    if (textarea->selecting){
        int start=textarea->selection_start;
        int end=textarea->selection_end;

        if (start>end){
            int temp=start;
            start=end;
            end=temp;
        }

        for (int i=start;i<end;i++){
            if (textarea->buffer[i]=='\n')continue;
            int cx=get_i_cursorx(i);
            int cy=get_i_cursory(i);
            recolour_area(cx*10,(3+cy)*18,10,18,textarea->fg,get_complementary_colour(textarea->fg));
            recolour_area(cx*10,(3+cy)*18,10,18,textarea->bg,get_complementary_colour(textarea->bg));
        }
        
    }

    

    int y=(textarea->cursory+3);

    for (int i = textarea->cursorx * 10; i < textarea->cursorx * 10 + 10; i++) {
        for (int j = y * 18; j < y * 18 + 18; j++) {
            
                if (framebuffer[j * ppr + i] == textarea->fg) {
                    framebuffer[j * ppr + i] = get_complementary_colour(textarea->fg);
                }
                else if (framebuffer[j*ppr+i]==textarea->bg){
                    framebuffer[j * ppr + i] = get_complementary_colour(textarea->bg);
                }
            
        }
    }

    save_mouse();

}

void blink_global_message(){
    int bg=on_blink?COLOUR:get_complementary_colour(COLOUR);
    int fg=on_blink?get_complementary_colour(COLOUR):COLOUR;

    draw_rect(GLOBAL_MESSAGE.x,GLOBAL_MESSAGE.y,GLOBAL_MESSAGE.width,GLOBAL_MESSAGE.height,bg);
    write_xy_text(GLOBAL_MESSAGE.x,GLOBAL_MESSAGE.y,GLOBAL_MESSAGE.message,fg);

}

void destroy_global_message(){
    restore_background(GLOBAL_MESSAGE.background);

    FREE(GLOBAL_MESSAGE.background->bg_buf);
    FREE(GLOBAL_MESSAGE.background);

    GLOBAL_MESSAGE.done=true;
}

void blink_cursor(void) {
     //restore_20x20_background();

     if (!GLOBAL_MESSAGE.done && GLOBAL_MESSAGE.blinks<9){
        GLOBAL_MESSAGE.blinks++;
        blink_global_message();
     }
     else if(!GLOBAL_MESSAGE.done){
        destroy_global_message();
     }



     if (on_blink){
        
        highlight_char();
     }else{
        
        unhighlight_char();
     }     

     on_blink = !on_blink;
     
    
}

void refresh_textpad() {//change eventually
    draw_rect(0, 50, 1024, 768 - 50,WHITE);

}

void clear_char_buffer() {
    textarea->buffer_index=0;
    fill_array(textarea->buffer,textarea->buffer_capacity,'\0');
}

void clear_question_buffer() {
    question=NULL;
}


/*void clear_tokens() {
    for (int i = 0; i < (sizeof(tokens) / sizeof(tokens[0])); i++) {
        for (int j = 0; j < sizeof(tokens[i]); j++) {
            tokens[i][j] = 0;
        }
    }
}*/

void char_buffer_101_space_fill() {
    
    for (int i = 0; i < 101; i++) {
        char_buffer[char_buffer_index++] = ' ';
    }
}

_Bool char_buffer_101_increment_is_new() {
    
    for (int i = char_buffer_index; i < char_buffer_index + 101; i++) {
        if (char_buffer[i] == 0) {
            return 1;
        }
    }
    return 0;
}

void find_closest_char_textbox(){
    int pos = textbox->index;
    
    if (pos > 0 && textbox->buffer[pos - 1] == '\n'){
        pos--;
    }
    
    int linecount = 0;

    while (pos > 0 && textbox->buffer[pos - 1] != '\n'){
        pos--;
        linecount++;
    }

    textbox->cursorx = linecount;
    textbox->cursory--;

}

void find_closest_char(void) {
    int i=textarea->buffer_index;

    i--;
    int x=0;
    while (i>=0 && textarea->buffer[i]!='\n'){
        i--;
        x++;
    }
    textarea->cursorx=x;
    textarea->cursory--;


}

void pop_char() {
    char_buffer[char_buffer_index] = ' ';
    if (char_buffer_index > 0) {
        char_buffer_index--;
    }

    /*for (int i = sizeof(char_buffer); i >= 0; i--) {
        if (char_buffer[i] != 0) {
            char_buffer[i] = 0;
            break;
        }
    }*/
}

_Bool is_last_char_textbox(){
    for (int i=textbox->index;textbox->buffer[i]!='\0';i++){
        if (i != ' ')return false;

    }
    return true;
}

_Bool is_last_char(){
    for (int i=textarea->buffer_index;textarea->buffer[i]!='\0';i++){
        if (i != ' ')return false;

    }
    return true;
}

void shift_textbox_buffer_left(){
    for (int i=textbox->index; ; i++){
        textbox->buffer[i]=textbox->buffer[i+1];
        if (textbox->buffer[i]=='\0') break;
    }


}

void shift_buffer_left(){
    for (int i=textarea->buffer_index; ; i++){
        textarea->buffer[i]=textarea->buffer[i+1];
        if (textarea->buffer[i]=='\0') break;
    }
}

void delete_char_textbox(){
    if (textbox->index > 0){
        char deleted_char = textbox->buffer[textbox->index - 1];

        if ((textbox->is_console && textbox->cursorx==0) || (textbox->cursorx==0&&textbox->cursory==0)){
            return;
        }

        destroy_char_textbox();
        textbox->index--;
        shift_textbox_buffer_left();
        

        if (deleted_char == '\n')
        {
            find_closest_char_textbox();
        }
        else
        {
            textbox->cursorx--;
            if (textbox->cursorx < 0)
            {
                find_closest_char_textbox();
            }
        }
        destroy_char_textbox();
        refresh_textbox();
    }
}

void delete_char(void) {
    if (textbox!=NULL){
        delete_char_textbox();
        return;
    }

    else if (question!=NULL){
        if (question->index>0){
            if (textarea->cursorx>0){
                textarea->cursorx--;
                destroy_char();
            }
            question->buffer[question->index--]=' ';
        }
    }


    else {                        
        if (textarea->buffer_index > 0){
            if (textarea->cursorx==0 && textarea->cursory==0)
                return;
            char deleted_char = textarea->buffer[textarea->buffer_index - 1];
            
            destroy_char();
            textarea->buffer_index--;
            shift_buffer_left();

            if (deleted_char == '\n'){
                find_closest_char();
            }
            else {
                textarea->cursorx--;
                if (textarea->cursorx < 0){
                    find_closest_char();
                }
            }
            destroy_char();
            refresh_text();
        }
    }
}


void ask_question(Question* q) {
    //restore_20x20_background();

    unhighlight_char();

    textarea->cursory++;

    textarea->cursorx = 0;

    write_rowcol_text(textarea->cursory + 3, textarea->cursorx, q->msg, RED);
    
    textarea->cursory++;

    //save_20x20_background();

}


/*void parse_print() {
    char msg[256];
    int msgi = 0;
    while (tokens[token_index][0] != 0) {
        token_index++;
        for (int i = 0; tokens[token_index][i] != 0; i++) {
            msg[msgi] = tokens[token_index][i];
            msgi++;
        }
        msg[msgi] = ' ';
        msgi++;
        
        
    }
    print_message(msg);
}*/

/*void parse_mkdir() {

}

void parse_gd() {
    while (tokens[token_index][0] != 0) {
        break;//WAY down the line
    }
}

void parse_rand() {
    unsigned long long number1 = rdtsc() << 32;
    unsigned long long number2 =number1 >> 32;
    print_number((unsigned int)number2*78);
}
void clear_shell() {
    ////////////domsoemthing
}
void parse_tokens() {

    if (compare_strings(tokens[token_index], "gd")) {
        token_index++;
        parse_gd();
    }
    else if (compare_strings(tokens[token_index], "prntnln")||compare_strings(tokens[token_index],"prnt")) {
        parse_print();
    }
    else if (compare_strings(tokens[token_index], "clr")) {
        clear_shell();
    }
    else if (compare_strings(tokens[token_index], "rand")) {
        parse_rand();
    }
    else if (tokens[token_index][0] == 0 || compare_strings(tokens[token_index], " ")) {
        print_message("Nothing entered");
    }
    else {
        print_error(tokens[token_index]);
    }
    
    clear_tokens();
    token_index = 0;
    
}

void lex_buffer(void) {
    

    char chars[512]={0};//initialise the lexers' char buffer
    
    int tc = 0;//initialise token buffer index counter
    int cc = 0;//initialise char buffer index counter
    for (unsigned int i = 0; i<sizeof(char_buffer); i++) {
        if (char_buffer[i] != '(' && char_buffer[i] != ' '&&char_buffer[i]!=';'&&char_buffer[i]!=0&&char_buffer[i]!='/'&&!is_operator(char_buffer[i])) {//consume characters
            chars[cc] = char_buffer[i];
            cc++;
            
        }
        else {
            cc = 0;
            int size = 0;

            for (int j = 0; chars[j] != 0; j++) {//get size of token
                size++;
            }

            for (int j = 0; j < size; j++) {//create the "string" && add it to tokens
                tokens[tc][j] = chars[j];
            }
            

            tc++;//increment index

            for (unsigned int j = 0; j < sizeof(chars); j++) {//reset chars to 0
                chars[j] = 0;
            }

            if (char_buffer[i] == 0) {
                break;
            }

            if (char_buffer[i] == '(') {
                tokens[tc][0] = '(';
                tc++;
            }
            if (char_buffer[i] == '/') {
                tokens[tc][0] = '/';
                tc++;
            }

        }

        
        
    }

    clear_char_buffer();
    
    

}*/

void enter_textbox(){
    if (textbox->cursory>=textbox->MAX-1)
        return;


    add_char_to_textbox('\n');
    textbox->cursorx=0;

    if (textbox->is_console){
        run_command();
        return;
    }

    refresh_textbox();
}

void enter() {

    /*if (question!=NULL){
        question->function();
        refresh_text();
        return;
    }*/

    if (textbox!=NULL){
        enter_textbox();
        return;
    }
    
    add_char_to_char_buf('\n');

    textarea->cursorx = 0;

    refresh_text();
}



int time_offset=72;

unsigned char bcd_to_binary(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void read_time() {
    

    outb(0x70, 0x00);//seconds
    unsigned char seconds = bcd_to_binary(inb(0x71));

    
    outb(0x70, 0x02);//minutes
    unsigned char minutes = bcd_to_binary(inb(0x71));

    
    outb(0x70, 0x04);//hours
    unsigned char hours = bcd_to_binary(inb(0x71));

    char hrsten = (hours/10)+'0';
    char hrs = (hours % 10) + '0';

    char mnsten = (minutes/10)+'0';
    char mns = (minutes % 10) + '0';
    
    char ssten = (seconds / 10) + '0';
    char ss = (seconds%10)+'0';

    
    write_time(hrsten,102-8);
    write_time(hrs, 102-7 );
    write_time(':', 102-6);

    write_time(mnsten, 102-5 );
    write_time(mns, 102-4);
    write_time(':', 102-3);

    write_time(ssten, 102-2);
    write_time(ss, 102-1);


    

}

void shift_textbox_buffer_right(){
    int old=textbox->index;
    int i;

    int len = old;
    while (textbox->buffer[len] != '\0') len++;

    for (int j = len; j >= old; j--) {
        textbox->buffer[j + 1] = textbox->buffer[j];
    }
    
}

void add_char_to_textbox(char character){
    if (textbox->index == textbox->capacity){
        textbox->capacity *= 2;
        uint8_t* realloc = MALLOC(textbox->capacity);

        for (int i = 0; i < textbox->capacity; i++){
            realloc[i] = '\0';
        }

        for (int i = 0; i < textbox->index; i++){
            realloc[i] = textbox->buffer[i];
        }

        FREE(textbox->buffer);
        textbox->buffer = realloc;
    }
    shift_textbox_buffer_right();

    textbox->buffer[textbox->index++] = character;
    refresh_textbox();

}

void right_arrow_textbox();

void append_textbox(char* msg){
    int x=textbox->cursorx;
    int y=textbox->cursory;
    int in=textbox->index;


    while (textbox->buffer[textbox->index]!='\0'){
        right_arrow_textbox();
    }

    /*if (textbox->cursory>=textbox->MAX){
        textbox->cursorx=x;
        textbox->cursory=y;
        textbox->index=in;
        return;
    }*/



    for (int j=0;msg[j]!='\0';j++)
        add_char_to_textbox(msg[j]);


    
    /*int n=len(msg)/(textbox->width/10);
    textbox->cursory+=n+1;*/

    //textbox->index=i;

}

void add_text_to_textbox(char* msg){
    for (int i=0;msg[i]!='\0';i++){
        add_char_to_textbox(msg[i]);
    }
}


void shift_buffer_right(){
    if (textbox!=NULL)
        shift_textbox_buffer_right();

    int old=textarea->buffer_index;
    int i;

    int len = old;
    while (textarea->buffer[len] != '\0') len++;

    for (int j = len; j >= old; j--) {
        textarea->buffer[j + 1] = textarea->buffer[j];
    }

}

void add_char_to_char_buf(char character){
    if (textbox!=NULL){
        add_char_to_textbox(character);
        return;
    }


    if (textarea->buffer_index >= textarea->buffer_capacity){
        textarea->buffer_capacity *= 2;
        uint8_t* realloc = MALLOC(textarea->buffer_capacity);

        for (int i = 0; i < textarea->buffer_capacity; i++){
            realloc[i] = '\0';
        }

        for (int i = 0; i < textarea->buffer_index; i++){
            realloc[i] = textarea->buffer[i];
        }
        FREE(textarea->buffer);

        textarea->buffer = realloc;
    }

    shift_buffer_right();

    textarea->buffer[textarea->buffer_index++] = character;


    if (character=='('){
        type_char(font8x16[')']);
        add_char_to_char_buf(')');
        textarea->cursorx--;
        textarea->buffer_index--;
        
        return;
    }
    if (character=='{'){
        type_char(font8x16['}']);
        add_char_to_char_buf('}');
        textarea->cursorx--;
        textarea->buffer_index--;
        
        return;
    }
    if (character=='"' && textarea->buffer[textarea->buffer_index-2]!='"'){
        type_char(font8x16['"']);
        add_char_to_char_buf('"');
        textarea->cursorx--;
        textarea->buffer_index--;
        
        return;
    }
    if (character=='\'' && textarea->buffer[textarea->buffer_index-2]!='\''){
        type_char(font8x16['\'']);
        add_char_to_char_buf('\'');
        textarea->cursorx--;
        textarea->buffer_index--;
        
        return;
    }
    if (character=='['){
        type_char(font8x16[']']);
        add_char_to_char_buf(']');
        textarea->cursorx--;
        textarea->buffer_index--;
        
        return;
    }

    refresh_text();
}

void adjust_cursor_position(){

}

_Bool can_go_down_textbox(){
    if (textbox->cursory>=textbox->MAX)return false;

    for (int i=textbox->index;textbox->buffer[i]!='\0';i++){
        if (textbox->buffer[i]=='\n')return true;
    }
    return false;
}

_Bool can_go_down(){
    for (int i=textarea->buffer_index;textarea->buffer[i]!='\0';i++){
        if (textarea->buffer[i]=='\n')return true;
    }
    return false;

}

void down_arrow_textbox(){
    if (textbox->is_console)return;
    if (can_go_down_textbox()){
        int linecount=textbox->cursorx;
        while(textbox->buffer[textbox->index]!='\n'){
            textbox->index++;
        }
        
        textbox->cursorx=0; 
        textbox->cursory++;
        textbox->index++;//consume '\n'

        while (textbox->cursorx < linecount && 
               textbox->buffer[textbox->index] != '\0' &&
               textbox->buffer[textbox->index] != '\n'){
            textbox->index++;
            textbox->cursorx++;
        }

    }
}

void down_arrow(){
    if (textbox!=NULL){
        down_arrow_textbox();
        return;
    }
    if (can_go_down()){
        int linecount=textarea->cursorx;
        while(textarea->buffer[textarea->buffer_index]!='\n'){
            textarea->buffer_index++;
        }
        
        textarea->cursorx=0; 
        textarea->cursory++;
        textarea->buffer_index++;//consume '\n'

        while (textarea->cursorx < linecount && 
               textarea->buffer[textarea->buffer_index] != '\0' &&
               textarea->buffer[textarea->buffer_index] != '\n'){
            textarea->buffer_index++;
            textarea->cursorx++;
        }

    }

}

void up_arrow_textbox(){
    if (textbox->is_console)return;
    if (textbox->cursory > 0){
        int target_x = textbox->cursorx;
        
        while (textbox->index > 0 && textbox->buffer[textbox->index - 1] != '\n'){
            textbox->index--;
        }
        
        if (textbox->index > 0){
            textbox->index--; 
        }
        
        int line_start = textbox->index;
        while (line_start > 0 && textbox->buffer[line_start - 1] != '\n'){
            line_start--;
        }
        
        int prev_line_len = textbox->index - line_start;
        
        if (target_x <= prev_line_len){
            textbox->index = line_start + target_x;
            textbox->cursorx = target_x;
        } else {
            textbox->cursorx = prev_line_len;
        }
        
        textbox->cursory--;
    }
}

void up_arrow(){
    if (textbox!=NULL){up_arrow_textbox();return;}
    if (textarea->cursory > 0){
        int target_x = textarea->cursorx;
        
        while (textarea->buffer_index > 0 && textarea->buffer[textarea->buffer_index - 1] != '\n'){
            textarea->buffer_index--;
        }
        
        if (textarea->buffer_index > 0){
            textarea->buffer_index--; 
        }
        
        int line_start = textarea->buffer_index;
        while (line_start > 0 && textarea->buffer[line_start - 1] != '\n'){
            line_start--;
        }
        
        int prev_line_len = textarea->buffer_index - line_start;
        
        if (target_x <= prev_line_len){
            textarea->buffer_index = line_start + target_x;
            textarea->cursorx = target_x;
        } else {
            textarea->cursorx = prev_line_len;
        }
        
        textarea->cursory--;
    }
}

void left_arrow_textbox(){
    if (textbox->index > 0){
        if (textbox->is_console && textbox->cursorx==0)return;

        if (textbox->cursorx==0 && textbox->cursory==0)
            return;
        
        char prev_char = textbox->buffer[textbox->index - 1];
        
        textbox->index--;
        
        if (prev_char == '\n'){
            find_closest_char_textbox();
        }
        else {
            textbox->cursorx--;
            if (textbox->cursorx < 0){
                find_closest_char_textbox();
            }
        }
    }

}

void left_arrow(){
    if (textbox!=NULL){
        left_arrow_textbox();
        return;
    }
    if (textarea->buffer_index > 0){
        char prev_char = textarea->buffer[textarea->buffer_index - 1];

        if (textarea->cursory==0&&textarea->cursorx==0)
                return;
        
        textarea->buffer_index--;
        
        if (prev_char == '\n'){
            find_closest_char();
        } 
        else {
            
            textarea->cursorx--;
            if (textarea->cursorx < 0){
                find_closest_char();
            }
        }
    }
}

void right_arrow_textbox(){
    if (textbox->buffer[textbox->index]!='\0'){

        textbox->index++;
        textbox->cursorx++;
        
        if (/*++textbox->cursorx>=((textbox->width-textbox->x)/10)||*/textbox->buffer[textbox->index-1]=='\n'){
            textbox->cursory++;
            textbox->cursorx=0;
        }
    }

}

void right_arrow(){
    if (textbox!=NULL){
        right_arrow_textbox();
        return;
    }
    if (textarea->buffer[textarea->buffer_index]!='\0'){

        textarea->buffer_index++;
        
        if (++textarea->cursorx>=101||textarea->buffer[textarea->buffer_index-1]=='\n'){
            textarea->cursory++;
            textarea->cursorx=0;
        }
    }

}
//uint32_t escape_address=(uint32_t)(&home);
void escape_program(){
    if (running_program){
        running_program=false;
        __asm__ __volatile__(
            "sti"
            :
            :
            : "memory"
        );
        home();
    }

}

void select_text(){
    
    if (!textarea->selecting){
        textarea->selecting=true;
        textarea->selection_start=textarea->selection_end=textarea->buffer_index;
        return;
    }
    textarea->selection_end=textarea->buffer_index;

}

void copy_text(){
    //memset8(CLIPBOARD,'\0',1024*1024);

    if (textarea->selection_start > textarea->selection_end){
        int temp=textarea->selection_end;
        textarea->selection_end=textarea->selection_start;
        textarea->selection_start=temp;
    }
    int j=0;
    for (int i=textarea->selection_start;i<textarea->selection_end;i++,j++){
        CLIPBOARD[j]=textarea->buffer[i];
    }
    CLIPBOARD[j]='\0';

}

void paste_text(){
    for (int i=0;CLIPBOARD[i]!='\0';i++){
        if (CLIPBOARD[i]=='\n'){
            enter();
            continue;
        }
        //type_char(font8x16[CLIPBOARD[i]]);
        shift_buffer_right();
        textarea->buffer[textarea->buffer_index++]=CLIPBOARD[i];
        if (textarea->buffer_index>=textarea->buffer_capacity-2){
            textarea->buffer_capacity*=2;

            char* re=MALLOC(textarea->buffer_capacity);
            fill_array(re,textarea->buffer_capacity,'\0');
            for (int i=0;textarea->buffer[i]!='\0';i++)
                re[i]=textarea->buffer[i];
            
            FREE(textarea->buffer);
            textarea->buffer=re;
        }
        
    }
    refresh_text_area();

}

void delete_selected(){
    if (!textarea->selecting)return;
    int start=textarea->selection_start;
    int end=textarea->selection_end;
    if (start>end){
        int temp=start;
        start=end;
        end=temp;
    }

    int cx=get_i_cursorx(end);
    int cy=get_i_cursory(end);
    textarea->cursorx=cx;
    textarea->cursory=cy;
    textarea->buffer_index=end;

    while (textarea->buffer_index!=start)delete_char();
    textarea->selecting=false;

}

volatile _Bool ctrl_pressed=false;
volatile _Bool alt_pressed=false;


void handle_key_press(uint8_t scancode) {

    if ((!textarea->editable && textbox==NULL) && scancode!=0x9d && scancode!=0x1d && scancode != 0x48 && scancode != 0x50){
        return;
    }

    

    if (scancode == 0x9D){
        ctrl_pressed = 0;
        return;
    }
    if (scancode==0x1D){
        ctrl_pressed=1;
        return;
    }

    if (scancode==0x38){
        alt_pressed=true;
    }
    if (scancode==0x2C){
        alt_pressed=false;
    }

    if (shift_pressed && !textarea->selecting && (scancode==0x4b || scancode== 0x4d || scancode== 0x48 || scancode== 0x50)){
        select_text();
    }


    write_char_buffer_index();

    int offset2 = (textarea->cursory * 80 + textarea->cursorx) * 2;
    // restore_20x20_background();
    if (scancode < 0x80)
    {   
        

        
        if (scancode == 0x2A || scancode == 0x36)
        { // for shift
            shift_pressed = 1;
        }
        
        else if (scancode == 0x0E)
        { // backspace
            on_blink = 1;
            unhighlight_char();

            if (shift_pressed && ctrl_pressed){
                delete_selected();
                return;
            }

            delete_char();
        }
        else if (scancode == 0x1C)
        { // enter
            unhighlight_char();
            
            enter();
        }

        else if (scancode == 0x4B)
        { // left arrow

            on_blink = 1;

            unhighlight_char();
            left_arrow();
            if (shift_pressed){
                select_text();
            }
        }
        else if (scancode == 0x4D)
        { // right arrow
            on_blink = 1;

            unhighlight_char();

            right_arrow();
            if (shift_pressed)
                select_text();
        }
        else if (scancode == 0x48)
        { // up arrow
            unhighlight_char();
            if (ctrl_pressed)
            {
                scroll_up();
                return;
            }

            up_arrow();
            if (shift_pressed)
                select_text();
        }
        else if (scancode == 0x50)
        { // down arrow
            unhighlight_char();
            if (ctrl_pressed)
            {
                scroll_down();
                return;
            }

            down_arrow();
            if (shift_pressed)
                select_text();
        }
        else if (scancode == 0x0F)
        { // tab
            
            if (textbox != NULL)
            {
                if (textbox->cursorx <= 28 - 4)
                {
                    unhighlight_char_textbox();

                    for (int i = 0; i < 4; i++)
                    {
                        type_textbox(font8x16[' ']);
                        add_char_to_textbox(' ');
                    }
                }
                return;
            }
            if (textarea->cursorx <= 101 - 4)
            {
                unhighlight_char();
                for (int i = 0; i < 4; i++)
                {
                    type_char(font8x16[' ']);
                    add_char_to_char_buf(' ');
                }
            }
        }
        else
        { // acknowledge the key typed, and write to screen
            // destroy_char();
            on_blink = 1;

            if (ctrl_pressed && character_table[scancode]=='c'){
                copy_text();
                return;
            }
            if (ctrl_pressed && character_table[scancode]=='v'){
                paste_text();
                return;
            }

            if (shift_pressed)
            {
                unhighlight_char();
                _Bool can_type = type_char(font8x16[character_table_shift[scancode]]);

                if (can_type)
                    add_char_to_char_buf(character_table_shift[scancode]);
                // textarea->buffer[textarea->buffer_index++] = character_table_shift[scancode];
            }
            else
            {
                unhighlight_char();
                _Bool can_type = type_char(font8x16[character_table[scancode]]);

                

                if (can_type)
                    add_char_to_char_buf(character_table[scancode]);
                // textarea->buffer[textarea->buffer_index++] = character_table[scancode];
            }
            if (textarea->cursorx==100){
                enter();
            }
        }
    }
    else
    {

        int scancode2 = scancode - 0x80;
        if (scancode2 == 0x2A || scancode2 == 0x36)
        {
            shift_pressed = 0;
            unhighlight_char();
            textarea->selecting=false;
            refresh_text_area();
        }
        
    }
}

void draw_close_box() {
    draw_bordered_rect(1024 - 25, 25, 25, 25, 0xf21400);
    write_char(1024 - 15, 30, font8x16['X'], 0);
}

void setup_program(char program_name[],uint32_t bgcol,uint32_t textcol,uint32_t texthighlightcol,uint32_t bghighlightcol,int cursor_x,int cursor_y,int mincursorx) {
    buttons_index=0;
    buttons_capacity=32;
    buttons=MALLOC(buttons_capacity*sizeof(struct button*));


    clear_char_buffer();

    init_idt();//initiate interrupts

    //restore_20x20_background();
    done = 0;

    clear_char_buffer();
    shell_selected = 0;
   


    bg_colour = bgcol;
    text_colour = textcol;
    cursorx = cursor_x;
    cursory = cursor_y;
    min_cursorx = mincursorx;
    bg_highlight_colour = bghighlightcol;
    text_highlight_colour = texthighlightcol;

    draw_rect(0, 25, 1024, 768 - 25, bg_colour);
    draw_bordered_rect(0, 25, 1024, 25, 0xa3a3a3);

    

    int x = 0;

    //write program name
    for (int i = 0; program_name[i] != '\0'; i++) {
        write_char(20+x, 30, font8x16[program_name[i]], 0);
        x += 10;
    }
    
    if (!home_selected) {
        draw_close_box();
    }
    save_20x20_background();
    draw_mouse();
    
   
    
    
}

void setup_interface() {
    generate_helper_functions();//includes a builtin malloc, wdraw, rand etc...

    draw_bordered_rect(0, 0, 1024, 55, TAB_COLOUR);
    
    draw_rect(15 * 10 + 20, 0, 2, 25, BLACK);//vertical line
    //text_colour = 0;
    draw_image(4,2,&SEAGULL);
    int offset = write_text(0, 3, "GullOS,", 0);
    offset = write_text(0, offset, "V 1.1", 0x0026ff);
    

}

void save_fe_file(char* file_extension){
    int name_sz=0;

    char* question_buffer=question->buffer;

    for (int i=0;question_buffer[i]!=0;i++){
        name_sz++;

    }
    int fe_sz=0;
    
    for (int i=0;file_extension[i]!='\0';i++){
        fe_sz++;
    }
    
    uint8_t name[name_sz+fe_sz+1];

    for (int i=0;i<name_sz;i++){
        name[i]=question_buffer[i];
    }

    int index=0;

    for (int i=name_sz;i<name_sz+fe_sz;i++){
        name[i]=file_extension[index++];
    }

    name[name_sz+fe_sz]='\0';

    int data_sz=0;

    for (int i=0;textarea->buffer[i]!=0;i++){
        data_sz++;
    }
    uint8_t data[data_sz];

    for (int i=0;i<data_sz;i++){
        data[i]=textarea->buffer[i];
    }
    
    clear_question_buffer();
    save_file(name,data,data_sz);
    //print_message("File saved!");

   
}

void submit_file();
void save_cminus_file();

void resave_file(){
    if (resave_file_name[0]=='\0'){
        create_global_message("No file selected!");
        return;
    }


    _Bool found=false;
    char sector[512];
    int sector_found;

    for (int i=0;i<=50&&!found;i++){
        read_sector_poll(2+i,sector);

        for (int j=0;j<12;j++){
            char* section=sector+(j*40);
            section[31]='\0';
            if (str_contains(section,resave_file_name)){
                found=true;
                sector_found=i+2;
                break;
            }
        }
    }

    if (!found){
        return;
    }

    int size=len(textarea->buffer)+1;
    int sectors_needed=(size+511)/512;
    int first_sector=allocate_sector();

    for (int i=0;i<sectors_needed;i++){
        char data[512];

        for (int j=0;j<512&&textarea->buffer[j]!='\0';j++){
            data[j]=textarea->buffer[i*512+j];
        }

        int sector_to_write=i==0?first_sector:allocate_sector();

        write_sector_poll(sector_to_write,data);
    }

    int entry_offset=0;
    for (int i=0;i<12;i++,entry_offset+=40){
        char potential[40];

        for(int j=0;j<40;j++){
            potential[j]=sector[i*40+j];
        }
        if (str_contains(potential,resave_file_name))
            break;

    }

    sector[entry_offset+32]=size&0xff;
    sector[entry_offset+33]=(size>>8)&0xff;
    sector[entry_offset+34]=(size>>16)&0xff;
    sector[entry_offset+35]=(size>>24)&0xff;

    sector[entry_offset+36]=first_sector&0xff;
    sector[entry_offset+37]=(first_sector>>8)&0xff;
    sector[entry_offset+38]=(first_sector>>16)&0xff;
    sector[entry_offset+39]=(first_sector>>24)&0xff;

    write_sector_poll(sector_found,sector);

    create_global_message("File saved!");

}

void save_as_cminus(){
    create_text_box(300,400,300,100,WHITE,BLACK+1,"Name your file",false);

    create_textbox_button(GREEN,WHITE,ORANGE,"Save as",submit_file,".cm");

}

void save_as_textpad(){
    create_text_box(300,400,300,100,WHITE,BLACK+1,"Name your file",false);

    create_textbox_button(GREEN,WHITE,ORANGE,"Save as",submit_file,".txt");

}



void save_textpad(){



}


void save_cminus_file(){
    int name_sz=0;

    char* question_buffer=question->buffer;

    for (int i=0;question_buffer[i]!=0;i++){
        name_sz++;

    }
    
    uint8_t name[name_sz+3];

    for (int i=0;i<name_sz;i++){
        name[i]=question_buffer[i];
    }

    name[name_sz]='.';
    name[name_sz+1]='c';
    name[name_sz+2]='m';
    name[name_sz+3]='\0';

    int data_sz=0;

    for (int i=0;textarea->buffer[i]!=0;i++){
        data_sz++;
    }
    uint8_t data[data_sz];

    for (int i=0;i<data_sz;i++){
        data[i]=textarea->buffer[i];
    }
    
    clear_question_buffer();
    
    save_file(name,data,data_sz);
    //print_message("File saved!");




}

void submit_file(char* fex){
    char name[32];

    int j=0;

    for (;textbox->buffer[j]!='\0';j++){
        name[j]=textbox->buffer[j];
    }

    name[j]='\0';
    char* fullname=str_cat(name,fex);

    /*
     *  check filename does not already exist
    */
    for (int i=1;i<=51;i++){
        char data[512];
        read_sector_poll(i,data);
        
        for (int j=0;j<12;j++){
            char filename[32];
            int offset=j*40;
            int k;
            for (k=0;k<32 && data[offset+k]!='\0';k++){
                filename[k]=data[offset+k];
            }
            filename[k]='\0';
            if (str_eq(filename,fullname)){
                create_global_message("Filename already exists!");
                return;
            }
                

        }
    }

    save_file(fullname,textarea->buffer,len(textarea->buffer));

    for (int i=0;fullname[i]!=0;i++){
        resave_file_name[i]=fullname[i];
    }

    FREE(fullname);

    close_text_box();


}

void save_text_file() {
    //write_rowcol_text(10,5,str_copy(question->buffer),ORANGE);

    char name[32];

    int j=0;
    for (;textbox->buffer[j]!='\0';j++){
        name[j]=textbox->buffer[j];
    }
    name[j]='.';
    name[j+1]='t';
    name[j+2]='x';
    name[j+3]='t';
    name[j+4]='\0';
    
    

    save_file(name, textarea->buffer, len(textarea->buffer));

    //print_message("File saved!");
    
}

void print_to_console(char* msg){
    append_textbox(msg);
    add_char_to_textbox('\n');
    textbox->cursorx=0;
    
}

_Bool command_match(char* msg){
    return str_eq(msg,tokens[0].value);
}

void print_commands(){
    print_to_console("print - print \"message\"");
    print_to_console("mkcpy - mkcpy \"filename\" \"copyname\"");
    print_to_console("info - info \"filename\"");
    print_to_console("del - del \"filename\"");
    print_to_console("open - open \"filename\"");
    print_to_console("defrag - defrag");
    print_to_console("rename - rename \"filename\" \"newname\"");
    print_to_console("setbg - setbg \"imagename\"");
    //print_to_console("");


}

void parse_command(){
    //write_centred_text(9,peek().value,ORANGE);

    char* token=tokens[0].value;

    if (command_match("mkcpy")){
        char* filename=tokens[1].value;
        char* cpyname=tokens[2].value;
        //int found=copy_file(filename,cpyname);
        int found=get_file_info(filename);
        if (found==-1){
            char* msg=str_cat("ERROR: could not find file - ",filename);
            print_to_console(msg);
            FREE(msg);
            return;
        }

        char buf[32]={0};
        buf[0]='.';
        char* fex=get_file_extension(filename);

        for (int i=0;fex[i]!='\0';i++)
            buf[1+i]=fex[i];
        
        cpyname=str_cat(cpyname,buf);
        
        if (get_file_info(cpyname)!=-1){
            print_to_console("ERROR: file already exists!");
            return;
        }

        copy_file(filename,cpyname);

        print_to_console("Success!");

    }
    else if (command_match("setbg")){
        char* filename=tokens[1].value;
        if (get_file_info(filename)==-1){
            char* msg=str_cat("ERROR: could not find file - ",filename);
            print_to_console(msg);
            FREE(msg);
            return;
        }
        if (!str_eq(get_file_extension(filename),"bmp")){
            print_to_console("file must be of type image");
            return;
        }

        uint8_t sector[512]={0};
        
        read_sector_poll(1,sector);
        sector[8]=1;
        memcpy(sector+9,FILE_INFO_BUF,40);
        write_sector_poll(1,sector);
        bg_image_copied=false;

        print_to_console("Success!");

    }
    else if (command_match("rename")){
        char* filename=tokens[1].value;
        char* newname=tokens[2].value;
        int found=get_file_info(filename);
        if (found==-1){
            char* msg=str_cat("ERROR: could not find file - ",filename);
            print_to_console(msg);
            FREE(msg);
            return;
        }
        rename_file(filename,newname);
        print_to_console("Success!");
        
    }
    else if (command_match("open")){
        char* filename=tokens[1].value;
        
        if (get_file_info(filename)==-1){
            char* msg=str_cat("ERROR: could not find file - ",filename);
            print_to_console(msg);
            FREE(msg);
            return;
        }

        FREE(tokens);
        __asm__ __volatile__("sti");
        open_file(filename);

    }
    else if (command_match("del")){
        char* filename=tokens[1].value;
        int found=delete_file(filename);
        if (found==-1){
            char* msg=str_cat("ERROR: Could not find file - ",filename);
            print_to_console(msg);
            FREE(msg);
        }
        print_to_console("Success!");

    }
    else if (command_match("info")){
        
        char* filename=tokens[1].value;
        int success=get_file_info(filename);

        if (success==-1){
            char* msg=str_cat("ERROR: Could not find file - ",filename);
            print_to_console(msg);
            FREE(msg);
            return;
        }

        int sector=FILE_INFO_BUF[36] | (FILE_INFO_BUF[37] << 8) | (FILE_INFO_BUF[38] << 16) | (FILE_INFO_BUF[39] << 24);
        int size=FILE_INFO_BUF[32] | (FILE_INFO_BUF[33] << 8) | (FILE_INFO_BUF[34] << 16) | (FILE_INFO_BUF[35] << 24);
        
        char sec[32],sz[32];
        uint32_t_str(sector,sec);
        uint32_t_str(size,sz);

        char* m11=str_cat(sz, " bytes");
        char* m1=str_cat("Filename: ",filename);
        char* m2=str_cat("@ Sector: ", sec);
        char* m3=str_cat("Size: ",m11);

        print_to_console(m1);
        print_to_console(m3);
        print_to_console(m2);


        FREE(m1),FREE(m11),FREE(m2),FREE(m3);

    }
    else if (command_match("print")){
        //write_centred_text(10,"FOUND PRINT",GREEN);
        
        print_to_console(tokens[1].value);
    }
    else if (command_match("help") || command_match("?")){
        print_commands();
    }
    else if (command_match("defrag")){
        int start=TIMER_TICKS;
        int old=get_last_sector();
        defragment_files();
        int end=TIMER_TICKS;
        print_to_console("Success!");
        
        char save_buf[32];
        char time_buf[32];
        int_str(abs(start-end)/100,time_buf);
        int_str(abs(get_last_sector()-old)*512,save_buf);
        char* msg=str_cat("Saved: ",save_buf);
        char* fmsg=str_cat(msg, " bytes!");
        char* msg2=str_cat("Time: ",time_buf);
        char* fmsg2=str_cat(msg2," seconds");

        print_to_console(fmsg);
        print_to_console(fmsg2);

        FREE(msg),FREE(fmsg),FREE(msg2),FREE(fmsg2);
    }
    else{
        char* msg=str_cat("ERROR: Unknown command - ",tokens[0].value);

        print_to_console(msg);
        FREE(msg);

    }
    tokens[0].value=" ";
}

void run_command(){
    token_count=0;
    token_index=0;
    token_capacity=128;
    tokens=MALLOC(token_capacity*sizeof(struct Token));

    lex_cm(get_last_line(textbox->buffer));
    parse_command();

    FREE(tokens);

}

void open_command_line(){
    create_text_box(200,250,500,400,BLACK,GREEN,"Command line",true);

    /*for (int i=0;i<3;i++){
        print_to_console("hello world!");
    }*/


}

void files() {
    home_selected = 0;
    //setup_program("Files", 0xffffff, 0, 0xffd000, 0x242424, 0, 30, 0);

    create_text_area(COLOUR,WHITE,RED,ALMOST_BLACK,"Files",false);

    if (get_is_background_set()){
        load_bg_image();
    }

    create_tab_button(RED,"Init File System / Wipe disk",ask_yes_no_files);
    create_tab_button(GREEN,"Command line",open_command_line);
    create_tab_button(YELLOW,"Refresh",files);

    list_files();


    while(1){
        MAIN_LOOP();
    }
}

void open_image(char* filename){
    int i;
    for (i=0;filename[i]!='\0';i++){
        current_file_name[i]=filename[i];
    }

    current_file_name[i]='\0';

    bit_maker();

}

void open_file(char* filename){
    //restore_20x20_background();
    
    int sz=0;

    for (int i=0;filename[i]!='\0';i++){
        sz++;
    }
    uint8_t file[sz+1];

    for (int i=0;i<sz;i++){
        file[i]=filename[i];
    }
    file[sz]='\0';
    
    int fesz=0;
    for (int i=sz-1;file[i]!='.';i--){
        fesz++;
    }
    
    uint8_t file_extension[fesz+1];

    for (int i=0;i<fesz;i++){
        file_extension[i]=file[sz-(fesz-i)];

    }
    file_extension[fesz]='\0';

    for (int i=0;file[i]!='\0';i++){
        current_file_name[i]=file[i];
    }

    if (compare_strings(file_extension,"txt")){
        
        textpad();
        
    }else if(compare_strings(file_extension,"cm")){
        
        cminus();
         
    }
    else if (str_eq(file_extension,"bmp")){
        bit_maker();
    }
    
}

Image load_image(char* filename){
    Image image;
    get_file_info(filename);

    uint32_t first_sector=FILE_INFO_BUF[36] | (FILE_INFO_BUF[37]<<8) | (FILE_INFO_BUF[38]<<16) | (FILE_INFO_BUF[39]<<24);
    uint32_t file_size=FILE_INFO_BUF[32] | (FILE_INFO_BUF[33]<<8) | (FILE_INFO_BUF[34]<<16) | (FILE_INFO_BUF[35]<<24);
    uint32_t width;
    uint32_t height;

    {
        uint8_t data[512];
        read_sector_poll(first_sector,data);
        width=data[0] | (data[1]<<8) | (data[2]<<16) | (data[3]<<24);
        height=data[5] | (data[6]<<8) | (data[7]<<16) | (data[8]<<24);

    }


    image.width=width,image.height=height;
    image.bitmap=MALLOC(width*height*sizeof(uint32_t));

    uint32_t sectors_needed = (file_size+511)/512;
    uint32_t int_size=width*height;
    uint32_t bmp_ind=0;

    for (int i=0;i<sectors_needed;i++){
        int ind=i==0?10:0;

        uint8_t data[512]={0};
        read_sector_poll(first_sector+i,data);

        while (ind<512-3 && bmp_ind<int_size){
            image.bitmap[bmp_ind++]=data[ind] | (data[ind+1]<<8) | (data[ind+2]<<16) | (data[ind+3]<<24);
            ind+=4;

        }
    }

    memset8(current_file_name,'\0',32);

    return image;

}

void compile_file(){
    char* filename=str_cat(textbox->buffer, ".exe");

    close_text_box();
    
    cmmain(textarea->buffer);


    char table_info[512];
    read_sector_poll(1,table_info);

    int file_count=table_info[0] | (table_info[1] << 8) | (table_info[2] << 16) | (table_info[3] << 24);

    int target_sector=2+((file_count*40)/512);
    int entry_offset=(file_count%12)*40;

    file_count++;
    table_info[0]=file_count&0xff;
    table_info[1]=(file_count>>8)&0xff;
    table_info[2]=(file_count>>16)&0xff;
    table_info[3]=(file_count>>24)&0xff;
    write_sector_poll(1,table_info);
    

    char metadata[512];
    read_sector_poll(target_sector,metadata);

    int i;
    for (i=0;i<32 && filename[i]!='\0';i++){
        metadata[entry_offset+i]=filename[i];

    }
    metadata[i==32?31:entry_offset+i]='\0';

    int first_sector=allocate_sector();
    int size=program_buffer->size;

    metadata[entry_offset+32] = (size&0xff);
    metadata[entry_offset+33] = (size>>8)&0xff;
    metadata[entry_offset+34] = (size>>16)&0xff;
    metadata[entry_offset+35] = (size>>24)&0xff;


    metadata[entry_offset+36]=first_sector&0xff;
    metadata[entry_offset+37]=(first_sector>>8)&0xff;
    metadata[entry_offset+38]=(first_sector>>16)&0xff;
    metadata[entry_offset+39]=(first_sector>>24)&0xff;


    write_sector_poll(target_sector,metadata);

    int sectors_needed=(size+511)/512;
    int offset=0;

    for (i=0;i<sectors_needed;i++){
        char s[512]={'\0'};

        int j=0;
        while (j<512&&offset<size){
            s[j]=program_buffer->bytes[offset];
            offset++;
            j++;

        }

        int to_write=i==0?first_sector:allocate_sector();

        write_sector_poll(to_write,s);


    }

    FREE(filename);

    create_global_message("Compiled!");

}

void save_program(){
    create_text_box(300,300,300,200,textarea->bg,textarea->fg,"Enter program name",false);

    create_textbox_button(WHITE,BLACK+1,LIGHTER_GRAY,"Submit",compile_file,NULL);
}

void open_current_file(){
    char* buf=(char*)0x200000;
    int size;

    load_file(current_file_name,buf,&size);
    buf[size] = '\0';

    clear_char_buffer();
    
    
    for (int i=0;i<size;i++){
        if (i>=textarea->buffer_capacity){
            textarea->buffer_capacity*=2;
            char* re=MALLOC(textarea->buffer_capacity);

            fill_array(re,textarea->buffer_capacity,'\0');

            for (int j=0;j<i;j++){
                re[j]=textarea->buffer[j];
            }

            FREE(textarea->buffer);
            textarea->buffer=re;
        }
        textarea->buffer[i]=buf[i];
        
        //textarea->buffer_index++;
    }

    for (int i=0;current_file_name[i]!='\0';i++){
        resave_file_name[i]=current_file_name[i];
    }

    clear_current_file_name();

    refresh_text();


}

void open_text_file() {
    char* question_buffer=question->buffer;

    int sz = 0;
    for (int i = 0; question_buffer[i] != 0 && i < sizeof(question_buffer); i++) {
        sz++;
    }
    uint8_t name[sz + 1];

    for (int i = 0; i < sz; i++) {
        name[i]=question_buffer[i];
    }
    name[sz] = '\0';
    char suffix[5];

    for (int i=0;i<4;i++){
        suffix[i]=name[sz-(4-i)];
    }
    suffix[5]='\0';

    

    refresh_textpad();//TEMP! MUST CHANGE SOON

    clear_question_buffer();

    uint8_t buf[textarea->buffer_capacity];
    int size=0;

    load_file(name, buf, &size);

    clear_char_buffer();

    for (int i=0;buf[i]!='\0';i++){
        add_char_to_char_buf(buf[i]);
    }
    refresh_text();

    

}

void write_char_buffer_index() {
    draw_rect(1024-25-60-60, 27, (1024-25)-(1024-25-60-60), 21, 0xa3a3a3);

    
    char buf[12];
    int num = textarea->buffer_index;
    int pos = 0;

    
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
    }
    else {
        int temp_pos = 0;
        char temp[12];
        while (num > 0) {
            temp[temp_pos++] = '0' + (num % 10);
            num /= 10;
        }

      
        for (int i = 0; i < temp_pos; i++) {
            buf[i] = temp[temp_pos - 1 - i];
        }
        buf[temp_pos] = '\0';
    }

    char* indexmsg = "Index:";
    

    int x = 1024-25-60;
    int ix = x - 60;

    for (int i = 0; indexmsg[i] != '\0'; i++) {
        write_char(ix + i * 10, 32, font8x16[indexmsg[i]], BLACK);
    }

    for (int i = 0; buf[i] != '\0'; i++) {
        write_char(x, 32, font8x16[buf[i]], BLUE);
        x += 10;
    }
}

void write_text_to_textbox(char* text){
    int i=0;
    while(text[i]!='\0')textbox->buffer[textbox->index++]=text[i++];
    
}

void open_errors(){
    if (textbox!=NULL)
        return;
    
    create_text_box(200,300,400,400,BLACK+1,RED,"Errors",false);

    

    /*textbox->capacity=128;
    textbox->buffer=MALLOC(textbox->capacity);
    
    textbox->index=0;*/

    //textbox->buffer=error_buf;

    for (int i=0;i<error_index&&errors[i]!=NULL;i++){
        write_text_to_textbox("ERROR: ");

        write_text_to_textbox(str_cat(str_cat(errors[i]->type,"  "),errors[i]->message));
        
        write_text_to_textbox("\n");
    }

    refresh_text();

}

void clear_textbox(){
    //write_rowcol_text(10,10,quick_int_str(textbox->capacity),ORANGE);
    textbox->index=0;
    textbox->cursorx=0;
    textbox->cursory=0;
    fill_array(textbox->buffer,textbox->capacity,'\0');
    refresh_textbox();
    textbox->cursorx=0;
    textbox->cursory=0;
    textbox->scroll_bounds=0;
    console_index=0;
    

}

void open_console(){
    if (textbox!=NULL)return;

    create_text_box(300,300,400,300,textarea->bg,textarea->fg,"Console",false);
    //FREE(textbox->buffer);
    textbox->buffer=console_buf;
    textbox->capacity=console_capacity;

    create_textbox_button(ORANGE,BLACK,RED,"Clear",clear_textbox,NULL);
    
    //write_text_to_textbox("123454321");

    refresh_textbox();
    
}

void highlight_syntax(){
    int i=get_line(textarea->scroll_bounds);
    int cx=0;
    int cy=0;

    while (textarea->buffer[i]!=0 && i<textarea->buffer_capacity && cy<41){
        if (textarea->buffer[i]=='/' && textarea->buffer[i+1]=='/'){
            while (textarea->buffer[i]!=0 && textarea->buffer[i]!='\n'){
                recolour_area(cx*10,cy*18+50,10,18,textarea->fg,COMMENT_COLOUR);
                cx++;
                i++;
            }

        }
        if (textarea->buffer[i]=='/' && textarea->buffer[i+1]=='*'){
            recolour_area(cx*10,cy*18+50,20,18,textarea->fg,COMMENT_COLOUR);
            i+=2;
            cx+=2;
            while (textarea->buffer[i]!=0 && !(textarea->buffer[i]=='*' && textarea->buffer[i+1]=='/')){
                if (textarea->buffer[i]=='\n'){
                    cy++;
                    cx=0;
                    i++;
                    continue;
                }
                recolour_area(cx*10,cy*18+50,10,18,textarea->fg,COMMENT_COLOUR);
                cx++;
                i++;
                
            }
            recolour_area(cx*10,cy*18+50,20,18,textarea->fg,COMMENT_COLOUR);
            i+=2;
            cx+=2;
            continue;
        }
        for (int j=2;j<=11;j++){
            
            if (textarea->buffer[i]=='"' || textarea->buffer[i]=='\'' || (textarea->buffer[i]=='/' && (textarea->buffer[i+1]=='/' || textarea->buffer[i+1]=='*'))){
                break;
            }
            char word[j];
            
            for (int k=0;k<j;k++)
                word[k]=0;
            
            for (int k=0;k<j-1;k++)
                word[k]=textarea->buffer[i+k];

            if (is_asm_keyword(word) && (i==0 || is_delimiter(textarea->buffer[i-1])) && is_delimiter(textarea->buffer[i+j-1])){
                recolour_area(cx*10,cy*18+50,(j-1)*10,18,textarea->fg,BLUE);
                /*cx+=j-1;
                i+=j-1;*/
                continue;
            }
            if (is_os_keyword(word) && (i==0 || is_delimiter(textarea->buffer[i-1])) && is_delimiter(textarea->buffer[i+j-1])){
                recolour_area(cx*10,cy*18+50,(j-1)*10,18,textarea->fg,OS_KEYWORD_COLOUR);
                continue;

            }
            
            if (is_keyword(word) && (i==0 || is_delimiter(textarea->buffer[i-1])) && is_delimiter(textarea->buffer[i+j-1])){
                recolour_area(cx*10,cy*18+50,(j-1)*10,18,textarea->fg,RED);
                /*cx+=j-1;
                i+=j-1;*/
                continue;
            }
            if (is_operator(word)){
                recolour_area(cx*10,cy*18+50,(j-1)*10,18,textarea->fg,0x727a69);
                /*cx+=j-1;
                i+=j-1;*/
                continue;
            }
            if (is_number(word) && (i==0 || is_delimiter(textarea->buffer[i-1])) && is_delimiter(textarea->buffer[i+j-1])){
                recolour_area(cx*10,cy*18+50,(j-1)*10,18,textarea->fg,ORANGE);
                /*cx+=j-1;
                i+=j-1;*/
                continue;
            }
            


            
        }

        /*if (textarea->buffer[i]=='/' && (textarea->buffer[i+1]=='/' || textarea->buffer[i+1]=='*')){
            while (textarea->buffer[i]!='*' && textarea->buffer[i+1]!='/' && textarea->buffer[i]!=0){
                recolour_area(cx*10,cy*18+50,10,18,textarea->fg,COMMENT_COLOUR);
                cx++;
                i++;
                if (textarea->buffer[i]=='\n'){
                    cx=0;
                    cy++;
                    i++;
                }
            }
            recolour_area(cx*10,cy*18+50, 20,18,textarea->fg,COMMENT_COLOUR);
            cx++;
            i++;
        }
        else*/
        if (textarea->buffer[i]=='"' || textarea->buffer[i]=='\''){
            _Bool isc=textarea->buffer[i]=='\'';

            cx++;
            i++;
            while (textarea->buffer[i]!='"' && textarea->buffer[i]!=0 && textarea->buffer[i]!='\n' && i<textarea->buffer_capacity){
                if (isc&&textarea->buffer[i]=='\'')
                    break;
                recolour_area(cx*10,cy*18+50,10,18,textarea->fg,GREEN);
                cx++;
                i++;
            }
            

        }
        if (textarea->buffer[i]=='\n'){
            cy++;
            cx=-1;
        }

        
        i++;
        cx++;
    }
    END:

}

void highlight_syntax_f(){
    int cx=0;
    int cy=textarea->cursory+textarea->scroll_bounds;
    int cindex=get_line(cy);

    while (textarea->buffer[cindex]!='\0' && textarea->buffer[cindex]!='\n'){
        /*if (textarea->buffer[cindex]=='\n'){
            cx=0;
            cy++;
            cindex++;
            break;
            continue;
        }*/

        for (int j=2;j<=12;j++){
            int length=j;

            char word[length];

            int k;
            for (k=0;k<length-1;k++)
                word[k]=textarea->buffer[cindex+k];
            
            word[k]='\0';

            if(word[0]=='"'){
                int i=0;
                cindex++;
                cx++;
                while (textarea->buffer[cindex]!='\n' && textarea->buffer[cindex]!=0 && textarea->buffer[cindex]!='"'){
                    recolour_area((cx+i)*10,50+cy*18,10,18,textarea->fg,GREEN);
                    cx++;
                    cindex++;
                }


            }
            
            if(textarea->buffer[cindex-1]=='\''&&textarea->buffer[cindex+1]=='\''){
                recolour_area((cx-2)*10,50+cy*18,30,18,textarea->fg,GREEN);
            }
            else if (is_operator(word)){
                recolour_area(cx*10,50+cy*18,(length-1)*10,18,textarea->fg,BRICK);
            }
            else if(is_delimiter(word[0])){
                recolour_area(cx*10,50+cy*18,10,18,textarea->fg,LIGHT_GRAY);

            }
            else if(is_asm_keyword(word)&&(is_delimiter(textarea->buffer[cindex-1]) || cindex==0)&&(is_delimiter(textarea->buffer[k+cindex]))){
                recolour_area(cx*10,50+cy*18,(length-1)*10,18,textarea->fg,BLUE);

            }
            else if((str_eq(word,"true") || str_eq(word,"false"))&&(is_delimiter(textarea->buffer[cindex-1])||cindex==0)&&is_delimiter(textarea->buffer[k+cindex])){
                recolour_area(cx*10,50+cy*18,(length-1)*10,18,textarea->fg,NUMBER_COLOUR);
            }
            else if(is_keyword(word)&&(is_delimiter(textarea->buffer[cindex-1])||cindex==0)&&is_delimiter(textarea->buffer[k+cindex])){
                recolour_area(cx*10,50+cy*18,(length-1)*10,18,textarea->fg,RED);

            }
            
            else if(is_number(word)){
                _Bool true_number=true;
                for (int i=cindex-1;i>=0;i--){
                    if (!is_numeric(textarea->buffer[i])){
                        if (!is_delimiter(textarea->buffer[i])){
                            true_number=false;
                            
                        }
                        break;

                        
                    }


                }

                for (int i=cindex+length-1;i<textarea->buffer_capacity&&textarea->buffer[i]!='\0';i++){
                    if (!is_numeric(textarea->buffer[i])){
                        if (!is_delimiter(textarea->buffer[i])){
                            true_number=false;
                            
                        }
                        break;
                    }
                }

                if (true_number){
                    recolour_area(cx*10,50+cy*18,(length-1)*10,18,textarea->fg,NUMBER_COLOUR);
                }
            }


            
            

        }




        cindex++;
        cx++;
        

        
    }





    


}

void toggle_theme(){
    int ox=textarea->cursorx;
    int oy=textarea->cursory;

    uint32_t oldbg=textarea->bg;
    textarea->bg=textarea->bg==NAVY?WHITE:NAVY;
    textarea->fg=get_complementary_colour(textarea->bg)+1;

    textarea->bg_highlight=get_complementary_colour(textarea->bg);
    textarea->fg_highlight=get_complementary_colour(textarea->fg);

    recolour_area(0,50,1024,768,oldbg,textarea->bg);

    textarea->cursorx=textarea->cursory=0;


    refresh_text();
    textarea->cursorx=ox;
    textarea->cursory=oy;
    
}


void goto_line(){
    int line=str_int(textbox->buffer);
    //line=3;
    close_text_box();

    if (count_lines(textarea->buffer)<line || line<0){
        create_global_message("Invalid line!");
        return;
    }

    textarea->scroll_bounds=line;
    textarea->cursorx=0;
    textarea->cursory=0;
    textarea->buffer_index=get_line(textarea->scroll_bounds);
    refresh_text();

}

void goto_cminus(){
    create_text_box(300,400,300,100,textarea->bg,textarea->fg,"Enter line number",false);
    create_textbox_button(NULL,PURPLE,NULL,"Go",goto_line,NULL);
}

void cminus(){
    
    //save_20x20_background();
    home_selected = 0;

    //setup_program("CMinus", WHITE, BLACK, GREEN, 0x242424, 0, 3, 0);

    create_text_area(WHITE,BLACK+1,YELLOW,ALMOST_BLACK,"CMinus",true);

    create_tab_button(BLACK, "Save as", save_as_cminus);
    create_tab_button(BLACK, "Save", resave_file);
    create_tab_button(ORANGE, "Run", run_program);
    create_tab_button(BLUE,"Compile",save_program);
    create_tab_button(GREEN,"Console",open_console);
    create_tab_button(RED,"Errors",open_errors);
    create_tab_button(WHITE,"Toggle theme", toggle_theme);
    create_tab_button(PURPLE,"Goto line",goto_cminus);
    create_tab_button(YELLOW,"Dump x86",dump_asm);
    
    //create_scrollbar();

    if (current_file_name[0] != '\0'){
        // restore_20x20_background();
        open_current_file();
        // save_20x20_background();
        // draw_mouse();
    }
    
    console_capacity=512;
    console_index=0;
    console_buf=MALLOC(console_capacity);
    console_buf[0] = '\0';
    
    while(1){
       MAIN_LOOP();
    }
}

void textpad() {//ttttt
    //save_20x20_background();
    home_selected = 0;

    

    //setup_program("Textpad", 0xffffff, 0, 0xffd000, 0x242424, 0, 3, 0);

    create_text_area(WHITE,BLACK+1,YELLOW,ALMOST_BLACK,"Textpad",true);

    create_tab_button(BLACK, "Save as",save_as_textpad);
    create_tab_button(BLACK,"Save", resave_file);

    

    

    if (current_file_name[0]!='\0'){
        
        open_current_file();
    }
    

    while(1){
        MAIN_LOOP();
    }

}

void paint_editor(){


}

int mandelbrot(double c_real, double c_imag){
    int max_iterations=50;

    double z_real=0;
    double z_imag=0;

    for (int i=0;i<max_iterations;i++){

        double z_real_new=z_real*z_real-z_imag*z_imag+c_real;
        double z_imag_new=2*z_real*z_imag+c_imag;

        z_real=z_real_new;
        z_imag=z_imag_new;

        if (z_real*z_real+z_imag*z_imag>4){
            return i;
        }






    }
    return max_iterations;

    
}

Image* create_mandelbrot_image(int width, int height){

    uint32_t** bitmap = MALLOC(height * sizeof(uint32_t*));
    
    int z[2]={0,0};
    for (int i=0;i<height;i++){
        bitmap[i]=MALLOC(width*sizeof(uint32_t));

        for (int j=0;j<width;j++){
            int x_min=-2.5;
            int x_max=1;
            int y_min=-1.25;
            int y_max=1.25;

            double c_real=x_min+((double)j/width)*(x_max-x_min);
            double c_imag=y_min+((double)i/height)*(y_max-y_min);

            int iterations=mandelbrot(c_real,c_imag);

            bitmap[i][j]=iterations*10;


        }

    }

    Image* image=create_image(width,height,bitmap);
    return image;


}
int gg=0;
int heapsave;


void draw_cpu(){
    //heap_offset=heapsave;
    refresh_text_area();
    //textarea->temps[textarea->temps_index++]=draw_rect_outline(200,250,400,400,LIGHT_GRAY);

    int eax;
    int ebx;
    int ecx;
    int edx;
    int edi;
    int esi;
    int ebp;
    int esp;
    int pc;
    int mar;
    int mdr;

    __asm__ __volatile__("":"=a"(eax)); //get eax
    __asm__ __volatile__("":"=b"(ebx)); //get ebx
    __asm__ __volatile__("":"=c"(ecx)); //get ecx
    __asm__ __volatile__("":"=d"(edx)); //get edx
    __asm__ __volatile__("":"=D"(edi)); //get edi
    __asm__ __volatile__("":"=S"(esi));
    __asm__ __volatile__("movl %%ebp, %0" : "=r"(ebp));
    __asm__ __volatile__("movl %%esp, %0" : "=r"(esp));

    /*__asm__ __volatile__("":"=s"(esi)); //get esi
    __asm__ __volatile__("":"=B"(ebp)); //get ebp
    __asm__ __volatile__("":"=S"(esp)); //get esp


    __asm__ __volatile__("":"=a"(eax)); //get pc
    __asm__ __volatile__("":"=a"(eax)); //get mdr
    __asm__ __volatile__("":"=a"(eax)); //get mar*/

    

    char* eaxval=inttostrhex(eax);
    char* ebxval=inttostrhex(ebx);
    char* ecxval=inttostrhex(ecx);
    char* edxval=inttostrhex(edx);
    char* edival=inttostrhex(edi);
    char* espval=inttostrhex(esp);
    char* ebpval=inttostrhex(ebp);
    char* esival=inttostrhex(esi);
    
    textarea->temps[textarea->temps_index++]=create_text_slab(25,100,str_cat("EAX:\n    ",eaxval),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(175,100,str_cat("EBX:\n    ",ebxval),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(25,150,str_cat("ECX:\n    ",ecxval),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(175,150,str_cat("EDX:\n    ",edxval),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(25,200,str_cat("EDI:\n    ",edival),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(175,200,str_cat("ESI:\n    ",edival),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(25,250,str_cat("EBP:\n    ",ebpval),get_complementary_colour(COLOUR));
    textarea->temps[textarea->temps_index++]=create_text_slab(175,250,str_cat("ESP:\n    ",espval),get_complementary_colour(COLOUR));
    

    //TextSlab* slab=create_text_slab(200,350,str_cat("EAX:\n",edival),RED);

    //textarea->text_slabs[textarea->text_slabs_index++]=slab;
    //textarea->temps[textarea->temps_index++]=create_rectangle_bg(slab->x,slab->y,slab->width,slab->height);

    FREE(eaxval);
    FREE(ebxval);
    FREE(ecxval);
    FREE(edxval);
    FREE(edival);
    FREE(esival);
    FREE(espval);
    FREE(ebpval);

}

void add_pixel_to_drawarea(int x, int y){
    if (x>drawarea->width || y>drawarea->height || x<0 || y<0)
        return;
    
    int i=y*drawarea->width+x;
    
    drawarea->bitmap[i]=drawarea->colour;

}

void draw_draw_area(){
    if (!rect_contains(mouse_x,mouse_y,drawarea->x,drawarea->y,drawarea->width*drawarea->pixel_size,drawarea->height*drawarea->pixel_size) || textbox!=NULL)
        return;
    int x=abs(mouse_x-drawarea->x);
    int y=abs(mouse_y-drawarea->y);

    for (int i=0;i<drawarea->brush_size;i++){
        
        for (int j=0;j<drawarea->brush_size;j++){
            
            add_pixel_to_drawarea(x+j,y+i);
        
        } 
    }

    refresh_draw_area(drawarea);

}

void inc_brush_size(){
    drawarea->brush_size+=2;
}

void dec_brush_size(){
    if (drawarea->brush_size<3)return;
    drawarea->brush_size-=2;
}

void toggle_show_pixels(){
    drawarea->show_pixels=!drawarea->show_pixels;
}

void change_brush_colour(){
    uint32_t col=get_inp("Enter colour");
    drawarea->colour=col;

}

void clear_draw_area(){
    memset32(drawarea->bitmap,WHITE,drawarea->width*drawarea->height);
    refresh_draw_area(drawarea);
}



void save_as_bitmaker(){
    create_text_box(centre_x(300),50,300,150,textarea->bg,textarea->fg,"Enter file name",false);
    create_textbox_button(NULL,GREEN,NULL,"Submit",save_bitmap_file,NULL);
    
}

void resave_bitmaker_file(){
    if (resave_file_name[0]=='\0'){
        create_global_message("No file selected!");
        return;
        
    }

    uint32_t file_size=get_file_size(resave_file_name);
    uint32_t first_sector=get_first_sector(resave_file_name);

    uint32_t sectors_needed=(511+file_size)/512;
    uint32_t int_count=drawarea->width*drawarea->height;
    uint32_t bmp_ind=0;
    for (int i=0;i<sectors_needed;i++){
        int ind=i==0?10:0;
        uint8_t data[512];
        read_sector_poll(first_sector+i,data);

        while(ind<512-3 && bmp_ind<int_count){
            data[ind++]=drawarea->bitmap[bmp_ind]&0xff;
            data[ind++]=(drawarea->bitmap[bmp_ind]>>8)&0xff;
            data[ind++]=(drawarea->bitmap[bmp_ind]>>16)&0xff;
            data[ind++]=(drawarea->bitmap[bmp_ind]>>24)&0xff;
            bmp_ind++;

        }
        write_sector_poll(first_sector+i,data);

    }
    create_global_message("File saved!");


}


void bit_maker(){ 
    create_text_area(COLOUR,get_complementary_colour(COLOUR)-1,NULL,NULL,"Bitmaker",false);
    if (get_is_background_set()){
        load_bg_image();
    }

    if (current_file_name[0]!='\0'){
        memcpy(resave_file_name,current_file_name,32);
        Image image=load_image(current_file_name);

        drawarea=create_draw_area(centre_x(image.width),25+centre_y(image.height),image.width,image.height);
        memcpy32(drawarea->bitmap,image.bitmap,image.width*image.height);
        refresh_draw_area(drawarea);
        
    }
    else{
        int width=get_inp("Enter width");
        int height=get_inp("Enter height");
        height=height>768-50?768-50:height;
        width=width>1024?1024:width;

        drawarea=create_draw_area(centre_x(width),25+centre_y(height),width,height);
    }

    create_tab_button(BLACK,"Save as",save_as_bitmaker);
    create_tab_button(BLACK,"Save",resave_bitmaker_file);
    create_tab_button(GREEN,"Inc brush size",inc_brush_size);
    create_tab_button(RED,"Dec brush size",dec_brush_size);
    //create_tab_button(YELLOW,"Toggle pixels",toggle_show_pixels);
    create_tab_button(PURPLE,"Change brush colour",change_brush_colour);
    create_tab_button(WHITE,"Clear",clear_draw_area);


    while(1){
        MAIN_LOOP();
        if (MOUSE_HELD){
            draw_draw_area();
        }
        

    }


}

void change_bg_colour(){
    uint32_t col=get_inp("Enter colour");

    uint8_t buf[512];
    read_sector_poll(1,buf);
    buf[49]=col&0xff;
    buf[50]=(col>>8)&0xff;
    buf[51]=(col>>16)&0xff;
    buf[52]=(col>>24)&0xff;
    write_sector_poll(1,buf);
    COLOUR=col;
    
}

void home() {
    _Bool bg_set=get_is_background_set();
    
    create_text_area(COLOUR,WHITE,YELLOW,ALMOST_BLACK,"Home",false);
    if (bg_set){
        load_bg_image();
    }

    home_selected = 1;
    
    int x=10e9;

    heapsave=heap_offset;
    draw_image(centre_x(200),50,&GOS_LOGO);

    /*ScrollArea* area=create_scrollarea("Getting started",600,100,400,300,RED,WHITE);
    int row=write_text_to_scrollarea(area,0,"\nHello and welcome to GullOS! A 32 bit integrated development experimental operating system. Press CTRL+DOWN to read more.\n");
    row=write_text_to_scrollarea(area,row+1,"Where do I begin?\n\nNavigate to your desired application using the top buttons. I recommend checking out the 'C-' application, which is where you should spend most of your time here. 'C-' is a language I made, which is OS-specific to this operating system. It allows absolute total control over everything, and is very powerful. But with great power comes great responsibility as they say, thus you should be careful as it is VERY EASY to crash this operating system... by design!\n\n About C- \n\nC- is a language which, as you could probably guess from its name, is very similar to c. functionally, loops, if statements, variables etc are essentially the same here. However, there are notable differences. I will aim to outline them below.\n\n1. For loops - the syntax, while appearing the exact same as C, are fundamentally parsed differently. The 'int' in the for loop is absolutely meaningless. No matter what, char, bool, whatever data type you put in, its always treated and stored as a int32_t. For loops also dont allow multiple declarations, or increments, (e.g- for (int i=0,j=0...)), however they do allow multiple conditions.\n\n2. Assignments - C- treats the '=' operator, or any operator which contains '=', as fundamentally different than operators such as '+'. For example, this means something like 'buf[i++]=arr[j++]' is not possible.\n\n3. Functions - functions are relatively different in C-. Return types for functions are not enforced, thus it similar to javascript or python in this sense. An example of a function declaration is as follows - 'func foo(int x, int y){}'. Important note, to call a function, put a hashtag followed by the function's name. Example - '#foo(1,2);'. This just allowed for easier parsing, that's all.\n\n4. Inline assembly - a very cool feature of C- is that it allows you to write assembly and C- interchangably, i.e you can write x86 assembly anywhere. An example of the power of this is 'MOV EAX inint(\"Enter a number\")', which moves user input into EAX directly. While not every x86 instruction is supported, every one that allowed me to write this compiler is, so its safe to assume a wide range of instructions are supported.\n\n5. Small type limitations - 'short' data types, i.e int16_t, do not exist. why? Because I dont see a point to be honest lol. But anyway, unsigned integers do not exist as of yet, but I do plan on eventually adding them in a future update. 'long' data types, i.e int64_t, do not exist, due to the 32 bit nature of this operating system. Not to say it would be impossible, but I do not see a point.\n\n6. The standard library - if you head over to files, you should see 'stdlib.cm' in documents, which is the standard library for C-, and includes a variety of functionality, such as draw, printf, IO etc.\n\n7. ");
    row=write_text_to_scrollarea(area,row+1,"");*/

    mmap_entry=(multiboot_mmap_entry_t*)multiboot_info->mmap_addr;
    int inc=0;
    while ((uint32_t)mmap_entry<(uint32_t)multiboot_info->mmap_addr+(uint32_t)multiboot_info->mmap_length){
        if (mmap_entry->type==1){
            write_rowcol_text(3+inc,0,"FOUND FREE! --> ",YELLOW);

            char buf[32];
            uint32_t_str(mmap_entry->addr,buf);
            write_rowcol_text(3+(inc++),13,buf,YELLOW);
        }
        
        
        mmap_entry=(multiboot_mmap_entry_t*)((uint32_t)mmap_entry+mmap_entry->size+4);
    }

    ScrollArea* apps=create_scrollarea("Applications",750,100,250,400,0,RED);

    struct button* fb=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,"Files",false,false,false,files,NULL);
    pop_button();

    struct button* cmb=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,"C-",false,false,false,cminus,NULL);
    pop_button();

    struct button* tb=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,"Textpad",false,false,false,textpad,NULL);
    pop_button();

    struct button* bp=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,"Bitmaker",false,false,false,bit_maker,NULL);
    pop_button();

    create_scrollbutton(apps,1,1,fb);
    create_scrollbutton(apps,3,1,cmb);
    create_scrollbutton(apps,5,1,tb);
    create_scrollbutton(apps,7,1,bp);


    ScrollArea* settings=create_scrollarea("Settings",50,350,200,300,0,RED);
    struct button* ccb=create_undrawn_button(0,0,-1,-1,COLOUR,get_complementary_colour(COLOUR)-1,RED,"Change bg colour",false,false,false,change_bg_colour,NULL);
    pop_button();

    

    create_scrollbutton(settings,1,1,ccb);

    while(1){
        MAIN_LOOP();
        if (quarter_second){
            draw_cpu();
            quarter_second=false;
        }
        
    }


}

void shell(void) {
    home_selected = 0;
    setup_program("Shell", 0, 0x00e60202,0x00c3ff,0xffffff,1,3,1);
    shell_selected = 1;

    write_char(0, 18 * 3, font8x16['>'], text_colour);

}


void kmaing(uint32_t magic, multiboot_info_t* mbi) {
    
    //framebuffer[y*width+x]=color;
    
    framebuffer = (uint32_t*)mbi->framebuffer_addr;
    framebuffer_add=mbi->framebuffer_addr;
    uint32_t width = mbi -> framebuffer_width;

    uint32_t pitch = mbi->framebuffer_pitch;
    ppr = pitch / 4;

    COLOUR=get_background_colour();

    *OS_KEY=0;

    SEAGULL.bitmap=GULLOS_LOGO;
    SEAGULL.width=22;
    SEAGULL.height=24;

    GOS_LOGO.bitmap=GULLOSLOGO_TYPED_TRANSPARENT;
    GOS_LOGO.width=200;
    GOS_LOGO.height=200;

    multiboot_info=mbi;


    //t=1/f -> T=1/100 == 0.01s per tick. sleep(100)==1s;
    init_timer(100);

    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);//disable the qemu default cursor
    
    init_idt();

    init_rng();

    setup_interface();

    //save_20x20_background();

    restore_mouse();

    home();
    
    
}



