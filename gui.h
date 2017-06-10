#ifndef GUI_H
#define GUI_H

#include "conf.h"
#include "includes.h"
#include "console.h"
#include "framebuffer.h"

#include "gmath/vector.h"
#include "gmath/matrix.h"

/*enum WINDOW_FLAGS
{
	WINDOW_STATIC_SIZE=1,					
	WINDOW_HAS_HEADER=2,
	WINDOW_TRANSPARENT=4,	
};*/

/*enum WINDOW_STATUS
{
	WINDOW_VISIBLE=1,
	WINDOW_SHOULD_CLOSE=2,
	WINDOW_MOUSE_OVER=4,
	WINDOW_MOUSE_OVER_HEADER=8,
	WINDOW_WINDOW_GRABBED=16
};*/

/*enum WINDOW_VAR_TYPE
{
	VAR_CHAR=0,
	VAR_SHORT=1,
	VAR_INT=2,
	VAR_FLOAT=3,
	VAR_PTR=4
};*/


	

/*typedef struct window_node_t
{
	int type;
	struct window_node_t *parent;
	void (*node_fn)(struct window_node_t *, void *);
	int max_children;
	int child_count;
	struct node_t *children;
}window_node_t;*/



/*typedef struct int_var_t
{
	int value;
}int_var_t;

typedef struct float_var_t
{
	float value;
}float_var_t;

typedef struct ptr_var_t
{
	void *value;
}ptr_var_t;

typedef union var_t_value
{
	int i_val;
	float f_val;
	void *ptr_val;
}var_t_value;

typedef struct var_t
{
	char *name;
	int type;
	var_t_value value;
}var_t;*/

/*typedef struct gvar_t
{
	char *name;
	int type;
	void *addr;
}gvar_t;*/

/*typedef struct window_class
{
	short bm_flags;
	char *class_name;
	int var_count;
	int *var_types;
}window_class;*/


/*typedef struct window_t
{
	void (*win_fn)(struct window_t *);
	float rmouse_x;
	float rmouse_y;
	float x;
	float y;
	float width;
	float height;
	float alpha;
	short bm_status;
	short bm_flags;
	int max_vars;
	int var_count;
	char *name;
}window_t;*/

/*typedef struct gelem_t
{
	float rmouse_x;
	float rmouse_y;
	float x;
	float y;
	float width;
	float height;
	float alpha;
	int max_gvar_count;
	int gvar_count;
	char *name;
	gvar_t *gvars;				
	int vars_stack_size;
	void *vars_stack;			
	void (*gelem_proc_fn)(struct gelem_t *);		
	void (*gelem_strt_fn)(struct gelem_t *);		
	short bm_status;
	short bm_flags;
}gelem_t;*/



/*typedef struct gelem_array
{
	int array_size;
	int gelem_count;
	gelem_t *gelems;
}gelem_array;*/


/*typedef struct window_class_array
{
	int array_size;
	int class_count;
	window_class *classes;
}window_class_array;*/

enum WIDGET_TYPES
{
	WIDGET_ROOT,						/* not a real widget, just to avoid updating the list's root pointer... */
	WIDGET_BASE,
	WIDGET_WINDOW,
	WIDGET_TEXT_FIELD,
	WIDGET_IMAGE_AREA,
	WIDGET_BUTTON,
	WIDGET_VERTICAL_SCROLLER,
	WIDGET_HORIZONTAL_SCROLLER
};

enum WIDGET_FLAGS
{
	WIDGET_GRABBABLE = 1,
	WIDGET_MOVABLE = 2,
	WIDGET_RESIZABLE = 4,
	WIDGET_HEADER = 8,						/* just draws a dark header on the top of the widget */
	WIDGET_MOUSE_OVER = 16,
	WIDGET_MOUSE_OVER_HEADER = 32,
	WIDGET_MOUSE_OVER_LEFT_BORDER = 64,
	WIDGET_MOUSE_OVER_RIGHT_BORDER = 128,
	WIDGET_MOUSE_OVER_TOP_BORDER = 256,
	WIDGET_MOUSE_OVER_BOTTOM_BORDER = 512,
	WIDGET_GRABBED_HEADER = 1024,
	WIDGET_GRABBED_LEFT_BORDER = 2048,
	WIDGET_GRABBED_RIGHT_BORDER = 4096,
	WIDGET_GRABBED_TOP_BORDER = 8192,
	WIDGET_GRABBED_BOTTOM_BORDER = 16384,
	WIDGET_ON_TOP = 32768,
	WIDGET_TRANSLUCENT = 65536,
	WIDGET_VISIBLE = 131072,
	WIDGET_RECEIVED_LEFT_BUTTON_DOWN = 262144,
	WIDGET_RECEIVED_LEFT_BUTTON_UP = 524288,
	WIDGET_RECEIVED_RIGHT_BUTTON_DOWN = 1048576,
	WIDGET_RECEIVED_RIGHT_BUTTON_UP = 2097152,
};

#define WIDGET_NO_TEXTURE 0


typedef struct
{
	float x;
	float y;
	float w;
	float h;
}wposition_t;

typedef struct
{
	char r;
	char g;
	char b;
	char a;
}wcolor_t;


typedef struct wbase_t
{
	//wposition_t position;
	float x;
	float y;
	float w;
	float h;
	
	float relative_x;
	float relative_y;
	float relative_w;
	float relative_h;
	
	float relative_mouse_x;
	float relative_mouse_y;
	
	float r;
	float g;
	float b;
	float a;
	//wcolor_t color;
	unsigned int tex_handle;			/* if this handle is different from -1, the engine will use as a GL texture handle. */
	struct wbase_t *next;
	struct wbase_t *prev;
	struct wbase_t *base;				/* so the subwidget can reference its base widget. Also used for checking if a subwidget is not
										being bound to two base widgets */
	struct wbase_t *w_widgets;			/* this is the list of widgets this base has (in the case this widget is a base). */
	struct wbase_t *w_last;
	struct wbase_t *affect;				/* the widget this widget will affect through its functions */
	char *name;
	int bm_flags;
	short type;
	short widget_count;
}wbase_t;


/*typedef struct
{
	wbase_t base;
}wwindow_t;*/


typedef struct
{
	wbase_t base; 
	void (*affect_fn)(wbase_t *);		/* the 'thing' this button will influence. */
}wbutton_t;

typedef struct
{
	wbase_t base;
	float min;						/* relative minimum normalized position the scroller can go */
	float cur;						/* current relative normalized position the scroller is*/
	float max;						/* well... */
	void (*affect_fn)(wbase_t *, float, float, float);		
}wvscroller_t;

typedef struct
{
	wbase_t base;
	float min;						/* relative minimum normalized position the scroller can go */
	float cur;						/* current relative normalized position the scroller is*/
	float max;						/* well... */
	void (*affect_fn)(wbase_t *, float, float, float);	
}whscroller_t;

typedef struct
{
	wbase_t base;
	framebuffer_t framebuffer;
}wdsurface_t;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void gui_Init();

PEWAPI void gui_Finish();

PEWAPI wbase_t *gui_CreateWidget(char *name, int bm_flags, float x, float y, float w, float h, float r, float g, float b, float a, unsigned int tex_handle, int b_focused);

PEWAPI wbase_t *gui_AddSubWidget(wbase_t *base, int bm_flags, short type, char *name, float x, float y, float w, float h, float scroller_max, float scroller_min, float r, float g, float b, float a, unsigned int tex_handle, wbase_t *affected_widget, void *affect_function);

PEWAPI void *gui_DeleteWidget(char *name);

PEWAPI wbase_t *gui_GetWidget(char *name);

PEWAPI void gui_ShowWidget(wbase_t *widget);

PEWAPI void gui_HideWidget(wbase_t *widget);

void gui_SetFocused(wbase_t *widget);

void gui_ProcessWidgets();

void gui_test_CloseWidget(wbase_t *widget);

void gui_test_CloseConsole(wbase_t *widget);

void gui_test_ToggleVolumetricLights(wbase_t *widget);

void gui_test_ToggleShadows(wbase_t *widget);

void gui_test_ToggleBloom(wbase_t *widget);


//PEWAPI void gui_ResizeGelemArray(int new_size);

//PEWAPI void gui_ResizeWindowClassArray(int new_size);

//PEWAPI void gui_RegisterWindowClass(window_class *win_class);

//PEWAPI int gui_GetWindowClassIndex(char *name);

//PEWAPI gelem_t *gui_GetGelem(char *name);

//PEWAPI int gui_CreateGelem(gelem_t *gelem);

//PEWAPI void gui_ProcessGelems();

//PEWAPI var_t *gui_CreateWindowVar(window_t *window, int var_type, char *var_name, var_t_value initial_value);

//PEWAPI void gui_ExpandWindowVars(window_t *window, int new_count);

//PEWAPI var_t *gui_GetWindowVar(window_t *window, char *name);

//PEWAPI vec2_t gui_GetRelativeMouse(gelem_t *gelem);

//PEWAPI vec2_t gui_GetAbsoluteMouse(gelem_t *gelem);

//PEWAPI vec2_t gui_GetRelativePosition(gelem_t *gelem, vec2_t absolute_pos);

//PEWAPI vec2_t gui_GetAbsolutePosition(gelem_t *gelem, vec2_t relative_pos);

//PEWAPI void gui_SetWindowVisible(gelem_t *gelem);

//PEWAPI void gui_SetWindowInvisible(gelem_t *gelem);

//PEWAPI void gui_DrawChar(char ch, gelem_t *gelem, vec2_t pos, vec3_t color);

//PEWAPI var_t_value f_value(float f);

//PEWAPI var_t_value i_value(int i);

#ifdef __cplusplus
}
#endif




#endif /* GUI_H */













