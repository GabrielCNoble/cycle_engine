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
	WIDGET_TAB_BAR,
	WIDGET_DROP_DOWN,
	WIDGET_VAR,
	WIDGET_VERTICAL_SCROLLER,
	WIDGET_HORIZONTAL_SCROLLER
};

enum WIDGET_FLAGS
{
	WIDGET_GRABBABLE = 1,
	WIDGET_MOVABLE = 1<<1,
	WIDGET_RESIZABLE = 1<<2,
	WIDGET_HEADER = 1<<3,						/* just draws a dark header on the top of the widget... */
	WIDGET_MOUSE_OVER = 1<<4,
	WIDGET_MOUSE_OVER_HEADER = 1<<5,
	WIDGET_MOUSE_OVER_LEFT_BORDER = 1<<6,
	WIDGET_MOUSE_OVER_RIGHT_BORDER = 1<<7,
	WIDGET_MOUSE_OVER_TOP_BORDER = 1<<8,
	WIDGET_MOUSE_OVER_BOTTOM_BORDER = 1<<9,
	WIDGET_GRABBED_HEADER = 1<<10,
	WIDGET_GRABBED_LEFT_BORDER = 1<<11,
	WIDGET_GRABBED_RIGHT_BORDER = 1<<12,
	WIDGET_GRABBED_TOP_BORDER = 1<<13,
	WIDGET_GRABBED_BOTTOM_BORDER = 1<<14,
	WIDGET_ON_TOP = 1<<15,
	WIDGET_TRANSLUCENT = 1<<16,
	WIDGET_VISIBLE = 1<<17,
	WIDGET_RECEIVED_LEFT_BUTTON_DOWN = 1<<18,
	WIDGET_RECEIVED_LEFT_BUTTON_UP = 1<<19,
	WIDGET_RECEIVED_RIGHT_BUTTON_DOWN = 1<<20,
	WIDGET_RECEIVED_RIGHT_BUTTON_UP = 1<<21,
	
	WIDGET_HIGHTLIGHT_BORDERS = 1<<22,			/* whether the borders of the widget should be highlighted when the mouse 
												   hovers over it */
												   
	WIDGET_CLICK_TO_FOCUS = 1<<23,				/* whether the user needs to click to focus the widget or just 
												   hover the mouse over it... */
												   
	WIDGET_KEEP_RELATIVE_Y_POSITION = 1<<24,		/* wheter the subwidget should keep its relative position within the widget or not.
												   The relative position depends on the dimensions of the widget. If the subwidget
												   keeps relative instead of absolute position within the widget, it will 'slide'
												   when the widget gets resized instead of remaining 'stuck' to its original
												   position... */
	WIDGET_KEEP_RELATIVE_X_POSITION = 1<<25,											   
												   			
												   
	WIDGET_LOCK_X_SCALE = 1 << 26,
	WIDGET_LOCK_Y_SCALE = 1 << 27,
	WIDGET_NO_BORDERS = 1 << 28,			/* this forces the engine to ignore when the mouse goes over headers and borders (to allow
											   subwidgets to fit perfectly within widgets... */
											   
	WIDGET_DELETE = 1 << 29,										   
												   
												   											   								   											   
};


enum BUTTON_FLAGS
{
	BUTTON_CHECK_BOX = 1,
	BUTTON_CHECK_BOX_CHECKED = 1 << 1,
	BUTTON_CHECK_BOX_SET_VALUE = 1 << 2,		/* if this flag is set, checking this check box will set whatever variable this checkbox modifies
												   to one. Otherwise, it will call a callback... */
	BUTTON_TOGGLE = 1 << 3,
	BUTTON_TOGGLED = 1 << 4
};

enum VAR_FLAGS
{
	VAR_ADDR = 1,								/* if this flag is set, it means the value stored within the wvar_t is a
												   reference to that var type, and thus have to be dereferenced before used... */
	VAR_RW = 1 <<1											   
};

enum VAR_TYPE
{
	VAR_INT_32 = 1,
	VAR_INT_16,
	VAR_INT_8,
	VAR_UINT_32,
	VAR_UINT_16,
	VAR_UINT_8,
	VAR_FLOAT,
	VAR_VEC2T,
	VAR_VEC3T,
	VAR_VEC4T,
	VAR_QUAT,
	VAR_STR,
	VAR_MAT3T,
	VAR_MAT4T
};

enum TAB_FLAGS
{
	TAB_MOUSE_OVER = 1,
	TAB_RECEIVED_LEFT_BUTTON_DOWN = 1 << 2,
	TAB_RECEIVED_LEFT_BUTTON_UP = 1 << 3,
	TAB_RECEIVED_RIGHT_BUTTON_DOWN = 1 << 4,
	TAB_RECEIVED_RIGHT_BUTTON_UP = 1 << 5,
	TAB_SELECTED = 1 << 6,
	TAB_NO_SUB_WIDGETS = 1 << 7
	
};


enum OPTION_FLAGS
{
	OPTION_MOUSE_OVER = 1,
	OPTION_SELECTED = 1 << 1,
	OPTION_NESTLED_DROPDOWN,
};

enum DROP_DOWN_FLAGS
{
	DROP_DOWN_DROPPED = 1,
	DROP_DOWN_NO_HEADER = 1 << 1,
};
#define WIDGET_NO_TEXTURE -1
#define WIDGET_BORDER_PIXEL_WIDTH 8
#define WIDGET_HEADER_PIXEL_HEIGHT 12

#define OPTION_HEIGHT 20.0
#define EMPTY_DROP_DOWN_HEIGHT 10.0


typedef struct
{
	float x;
	float y;
	float w;
	float h;
}wposition_t;

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}wcolor_t;

typedef struct swidget_t
{
	float x;
	float y;
	float w;
	float h;
	float cx;
	float cy;
	float cw;
	float ch;
	
	float relative_mouse_x;
	float relative_mouse_y;
	float r;					//
	float g;					//
	float b;					//
	float a;					// those could be chars...
	char *name;
	int type;
	int bm_flags;
	void (*widget_callback)(swidget_t *, void *);	/* this can gtfo... */
	void *data;
	struct swidget_t *next;
}swidget_t;


typedef struct widget_t
{
	//wposition_t position;
	float x;
	float y;
	float w;
	float h;

	float cw;
	float ch;
	//float relative_x;
	//float relative_y;
	//float relative_w;
	//float relative_h;
	
	/* values within the range [-1.0, 1.0] are inside the widget, 
	regardless its dimensions... */
	float relative_mouse_x;
	float relative_mouse_y;
	
	float r;
	float g;
	float b;
	float a;
	//wcolor_t color;
	unsigned int tex_handle;			/* if this handle is different from -1, the engine will use as a GL texture handle. */
	struct widget_t *next;
	struct widget_t *prev;
	int sub_widgets_count;
	swidget_t *sub_widgets;
	swidget_t *last_added;
	swidget_t *active_swidget;
	char *name;
	int bm_flags;
	short type;
	short widget_count;
	char *text_buffer;
}widget_t;



typedef struct
{
	swidget_t swidget;
	void (*widget_callback)(void *);		/* the 'thing' this button will influence. */
	int button_flags;
}wbutton_t;

typedef struct
{
	swidget_t swidget;
	void *var;
	short var_type;
	short var_flags;
}wvar_t;

typedef struct
{
	char *name;
	int swidget_count;
	swidget_t *swidgets;
	swidget_t *last;
	int bm_flags;
}wtab_t;

typedef struct
{
	swidget_t swidget;
	int tab_count;
	int max_tabs;
	wtab_t *tabs;
	wtab_t *active_tab;
	void (*tabbar_callback)(swidget_t *, void *, int);
}wtabbar_t;

typedef struct
{
	char *name;
	int bm_flags;
	swidget_t *nested;							/* just a single drop down can be nested by option... */
}woption_t;

typedef struct
{
	swidget_t swidget;
	unsigned char bm_flags;
	unsigned char option_count;
	unsigned char max_options;
	unsigned char cur_option;
	void *data;
	woption_t *active_option;
	woption_t *options;
	void (*dropdown_callback)(swidget_t *, void *, int);
}wdropdown_t;

typedef struct
{
	//wbase_t base;
	float min;						/* relative minimum normalized position the scroller can go */
	float cur;						/* current relative normalized position the scroller is*/
	float max;						/* well... */
	//void (*fn)(wbase_t *);		
}wvscroller_t;

typedef struct
{
	//wbase_t base;
	float min;						/* relative minimum normalized position the scroller can go */
	float cur;						/* current relative normalized position the scroller is*/
	float max;						/* well... */
	//void (*affect_fn)(wbase_t *, float, float, float);	
}whscroller_t;

typedef struct
{
	//wbase_t base;
	framebuffer_t framebuffer;
}wdsurface_t;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void gui_Init();

PEWAPI void gui_Finish();

PEWAPI widget_t *gui_CreateWidget(char *name, int bm_flags, float x, float y, float w, float h, float r, float g, float b, float a, unsigned int tex_handle, int b_focused);

PEWAPI void gui_AddButton(widget_t *widget, char *name, int bm_flags, int bm_button_flags, float x, float y, float w, float h, float r, float g, float b, float a, void *data, void (*widget_callback)(swidget_t *, void *));

PEWAPI void gui_AddVar(widget_t *widget, char *name, int bm_flags, int var_flags, int type, float x, float y, float w, float h,  void *var);

PEWAPI wdropdown_t *gui_AddDropDown(widget_t *widget, char *name, int bm_flags, float x, float y, float w, void *data, void (*dropdown_callback)(swidget_t *, void *, int));

PEWAPI void gui_AddOption(wdropdown_t *dropdown, char *name);

PEWAPI wdropdown_t *gui_NestleDropDown(wdropdown_t *dropdown, int option_index, char *name, int bm_flags, int width, void *data, void (*dropdown_callback)(swidget_t *, void *, int));

PEWAPI void gui_AddSeparator(wdropdown_t *dropdown, char *name);

//PEWAPI widget_t *gui_AddNestedDropDown(wdropdown_t *dropdown, int option_index, float width);

PEWAPI wtabbar_t *gui_AddTabBar(widget_t *widget, char *name, int bm_flags, float x, float y, float w, float h, void (*tabbar_callback)(swidget_t *, void *, int));

PEWAPI int gui_AddTab(wtabbar_t *tabbar, char *name, int tab_flags);

PEWAPI void gui_AddVarToTab(wtabbar_t *tabbar, int tab_index, char *name, int bm_flags, int var_flags, int type, float x, float y, float w, float h,  void *var);


//PEWAPI void gui_AddSubWidget(widget_t *base, int bm_flags, short type, char *name, float x, float y, float w, float h, float scroller_max, float scroller_min, float r, float g, float b, float a, unsigned int tex_handle, wbase_t *affected_widget, void *affect_function);

PEWAPI void gui_DeleteWidgetByName(char *name);

PEWAPI void gui_DeleteWidget(widget_t *widget);

PEWAPI void gui_MarkForDeletion(widget_t *widget);

PEWAPI widget_t *gui_GetWidget(char *name);

PEWAPI void gui_ShowWidget(widget_t *widget);

PEWAPI void gui_HideWidget(widget_t *widget);

void gui_SetFocused(widget_t *widget);

void gui_SetActiveTab(wtabbar_t *tabbar, wtab_t *tab);

void gui_ProcessWidgets();

PEWAPI void gui_PrintOnWidget(widget_t *widget, char *str);

void gui_test_CloseWidget(widget_t *widget);

void gui_test_CloseConsole(widget_t *widget);

void gui_test_ToggleVolumetricLights(widget_t *widget);

void gui_test_ToggleShadows(widget_t *widget);

void gui_test_ToggleBloom(widget_t *widget);




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













