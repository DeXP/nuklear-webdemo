#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#define NK_INCLUDE_STANDARD_VARARGS
#define NKC_IMPLEMENTATION
#include "nuklear_cross/nuklear_cross.h"

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

int result = 1;

struct nkc nkcx;
struct nk_context *ctx;
struct nk_color background;

int inFullscreen = 0;
int wasFullscreen = 0;
int width = 0;
int height = 0;


#include "style.c"

#include "overview.c"
#include "calculator.c"
#include "node_editor.c"
#include "extendedx.c"


int style_id[] = {
    999,
    THEME_WHITE,
	THEME_RED,
	THEME_BLUE,
	THEME_DARK
};

const char* style_name[] = {
    "Default",
	"White",
	"Red",
	"Blue",
	"Dark"
};


void render(void* loopArg){
   struct nkc* nkcHandle = (struct nkc*)loopArg;
   struct nk_context *ctx = nkc_get_ctx(nkcHandle);
   static int selected_item = 0;

   /* Input */
   union nkc_event e = nkc_poll_events(nkcHandle);
   if( (e.type == NKC_EWINDOW) && (e.window.param == NKC_EQUIT) ){
       nkc_stop_main_loop(nkcHandle);
   }


   /* GUI */
   {
   node_editor(ctx);
   overview(ctx);
   calculator(ctx);
        basic_demo(ctx, &media);
        button_demo(ctx, &media);
        grid_demo(ctx, &media);


   if (nk_begin(ctx, "Demo", nk_rect(430, 10, 230, 250),
       NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
       NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
   {
       enum {EASY, HARD};
       static int op = EASY;
       static int property = 20;
       nk_layout_row_static(ctx, 30, 80, 1);
       if (nk_button_label(ctx, "button"))
           fprintf(stdout, "button pressed\n");

       nk_layout_row_dynamic(ctx, 30, 2);
       if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
       if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

       nk_layout_row_dynamic(ctx, 25, 1);
       nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

       {;
       nk_layout_row_dynamic(ctx, 20, 1);
       nk_label(ctx, "background:", NK_TEXT_LEFT);
       nk_layout_row_dynamic(ctx, 25, 1);
       if (nk_combo_begin_color(ctx, background, nk_vec2(nk_widget_width(ctx),400))) {

           nk_layout_row_dynamic(ctx, 120, 1);
           background = nk_color_picker(ctx, background, NK_RGBA);
           nk_layout_row_dynamic(ctx, 25, 1);
           background.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1,1);
           background.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1,1);
           background.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1,1);
           background.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1,1);
           nk_combo_end(ctx);
       }}

       nk_layout_row_dynamic(ctx, 25, 2);
	   nk_label(ctx, "GUI skin:", NK_TEXT_LEFT);
	   if (nk_combo_begin_label(ctx, style_name[selected_item], nk_vec2(nk_widget_width(ctx), 200))) {
            int i;
            nk_layout_row_dynamic(ctx, 25, 1);
            for (i = 0; i < sizeof(style_id)/sizeof(style_id[0]) ; ++i)
                if (nk_combo_item_label(ctx, style_name[i], NK_TEXT_LEFT)){
                    selected_item = i;
					set_style(ctx, style_id[i]);
				}
            nk_combo_end(ctx);
        }
   }
   nk_end(ctx);
   }

   /* Draw */
   nkc_render(nkcHandle, background);
}

int main(){
   int width, height;
   background = nk_rgb(20, 20, 20);

   if( nkc_get_desktop_size(&nkcx, &width, &height) ){
	   width -= 20; height -= 20; /* for emscripten canvas' border */
   } else {
	   width = 1280; height = 720;
   }

   if( nkc_init(&nkcx, "Nuklear Demo/Example - Nuklear+ Example", width, height, NKC_WIN_NORMAL) ){
       extendedInit(&nkcx);
       nkc_set_main_loop(&nkcx, render, (void*)&nkcx );
   }
   nkc_shutdown(&nkcx);
   return 0;
}
