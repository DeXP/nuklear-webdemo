#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


#include <GL/glew.h>
/*#define GLFW_INCLUDE_ES2*/
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS 
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

int result = 1;

GLFWwindow* win;
struct nk_context *ctx;
struct nk_color background;

int inFullscreen = 0;
int wasFullscreen = 0;
int width = 0;
int height = 0;

void render();
void error_callback(int error, const char* description);

#include "style.c"

#include "overview.c"
#include "calculator.c"
#include "node_editor.c"

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


void render() {
   static int selected_item = 0;

   /* Input */
   glfwPollEvents();
   nk_glfw3_new_frame();

   /* GUI */
   {
   node_editor(ctx);
   overview(ctx);
   calculator(ctx);

   if (nk_begin(ctx, "Demo", nk_rect(450, 10, 230, 250),
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
   {float bg[4];
   nk_color_fv(bg, background);
   glfwGetWindowSize(win, &width, &height);
   glViewport(0, 0, width, height);
   glClear(GL_COLOR_BUFFER_BIT);
   glClearColor(bg[0], bg[1], bg[2], bg[3]);
   /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
    * with blending, scissor, face culling, depth test and viewport and
    * defaults everything back into a default state.
    * Make sure to either a.) save and restore or b.) reset your own state after
    * rendering the UI. */
   nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
   glfwSwapBuffers(win);}
}

void error_callback(int error, const char* description)
{
   fprintf(stderr, "Error %d: %s\n", error, description);
}

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
#ifdef __EMSCRIPTEN__
   int isInFullscreen = EM_ASM_INT_V(return !!(document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement));
#else
   int isInFullscreen = 0;
#endif
   if (isInFullscreen && !wasFullscreen)
   {
      printf("Successfully transitioned to fullscreen mode!\n");
      wasFullscreen = isInFullscreen;
   }

   if (wasFullscreen && !isInFullscreen)
   {
      printf("Exited fullscreen. Test succeeded.\n");
      result = 1;
      #ifdef REPORT_RESULT
      REPORT_RESULT();
      #endif
      wasFullscreen = isInFullscreen;
#ifdef __EMSCRIPTEN__
      emscripten_cancel_main_loop();
#endif
      return;
   }
}

int main()
{
   /* Setup win */
   glfwSetErrorCallback(error_callback);
   if (!glfwInit())
   {
      result = 0;
      printf("Could not create window. Test failed.\n");
      #ifdef REPORT_RESULT
      REPORT_RESULT();
      #endif
      return -1;
   }
   glfwWindowHint(GLFW_RESIZABLE , 1);
   win = glfwCreateWindow(1024, 1024, "GLFW resizing test - windowed", NULL, NULL);
   if (!win)
   {
      result = 0;
      printf("Could not create window. Test failed.\n");
      #ifdef REPORT_RESULT
      REPORT_RESULT();
      #endif
      glfwTerminate();
      return -1;
   }
   glfwMakeContextCurrent(win);

   /* Install callbacks */
   glfwSetWindowSizeCallback(win, windowSizeCallback);

   /* Nuklear */
   ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
   /* Load Fonts: if none of these are loaded a default font will be used  */
   {struct nk_font_atlas *atlas;
   nk_glfw3_font_stash_begin(&atlas);
   /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
   /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
   /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
   /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
   /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
   /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
   nk_glfw3_font_stash_end();
   /*nk_style_set_font(ctx, &droid->handle);*/}
   background = nk_rgb(20, 20, 20);

   /* Main loop */
#ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(render, 0, 1);
#endif

   nk_glfw3_shutdown();
   glfwTerminate();

   return 0;
}
