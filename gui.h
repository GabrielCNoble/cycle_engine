#ifndef GUI_H
#define GUI_H

#include "conf.h"
#include "includes.h"
#include "console.h"
#include "framebuffer.h"

#include "gmath/vector.h"
#include "gmath/matrix.h"


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
	WIDGET_SLIDER,
	WIDGET_SLIDER_GROUP,
	WIDGET_SURFACE,
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
	
	//WIDGET_HIGHTLIGHT_BORDERS = 1<<22,			/* whether the borders of the widget should be highlighted when the mouse 
												   //hovers over it */
												   
	WIDGET_IGNORE_MOUSE = 1<<23,				/* wheter this widget should block any mouse action upon the world when
												   the cursor is within its limits... */
												   
	//WIDGET_KEEP_RELATIVE_Y_POSITION = 1<<24,		/* wheter the subwidget should keep its relative position within the widget or not.
												   //The relative position depends on the dimensions of the widget. If the subwidget
												  // keeps relative instead of absolute position within the widget, it will 'slide'
												 //  when the widget gets resized instead of remaining 'stuck' to its original
												 //  position... */
	//WIDGET_KEEP_RELATIVE_X_POSITION = 1<<25,											   
												   			
												   
	//WIDGET_LOCK_X_SCALE = 1 << 26,
	//WIDGET_LOCK_Y_SCALE = 1 << 27,
	WIDGET_NO_BORDERS = 1 << 28,			/* this forces the engine to ignore when the mouse goes over headers and borders (to allow
											   subwidgets to fit perfectly within widgets... */
											   
	WIDGET_DELETE = 1 << 29,			
	WIDGET_LEFT_BUTTON_DOWN = 1 << 30,			
	//WIDGET_HORIZONTAL_BORDERS_SHARED = 1 << 22,
	//WIDGET_VERTICAL_BORDERS_SHARED = 1 << 31,
					   											 										   											   								   											   
};

enum WIDGET_BORDERS
{
	WIDGET_LEFT_BORDER = 1,
	WIDGET_RIGHT_BORDER = 1 << 1,
	WIDGET_TOP_BORDER = 1 << 2,
	WIDGET_BOTTOM_BORDER = 1 << 3,
	WIDGET_HORIZONTAL_BORDERS = 1 << 4,
	WIDGET_VERTICAL_BORDERS = 1 << 5,
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
	DROP_DOWN_TITLE = 1 << 2,
};


#define WIDGET_NO_TEXTURE 0xffffffff
#define WIDGET_BORDER_PIXEL_WIDTH 8
#define WIDGET_HEADER_PIXEL_HEIGHT 12

#define OPTION_HEIGHT 20.0
#define EMPTY_DROP_DOWN_HEIGHT 10.0

#define SLIDER_OUTER_HEIGHT 16.0
#define SLIDER_INNER_HEIGHT 8.0
#define SLIDER_NAME_X0 -100.0
#define SLIDER_NAME_X1 -10.0
#define SLIDER_VALUE_X0 10.0
#define SLIDER_VALUE_X1 80.0


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
	int bm_border_hooks;		/* to which borders this subwidget is hooked... */
	
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
	
	/* values within the range [-1.0, 1.0] are inside the widget, 
	regardless its dimensions... */
	float relative_mouse_x;
	float relative_mouse_y;
	float hx;
	float lx;
	float hy;
	float ly;
	
	float r;
	float g;
	float b;
	float a;
	
	unsigned int tex_handle;			/* if this handle is different from -1, the engine will use as a GL texture handle. */
	struct widget_t *next;
	struct widget_t *prev;
	struct widget_t *left_border;		/* those will be different from NULL in case they share an border... */
	struct widget_t *right_border;
	struct widget_t *top_border;
	struct widget_t *bottom_border;
	int sub_widgets_count;
	swidget_t *sub_widgets;
	swidget_t *last_added;
	swidget_t *active_swidget;
	void (*widget_callback)(widget_t *);
	char *name;
	int bm_flags;
	int active_borders;
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
	swidget_t swidget;
	unsigned int bm_flags;
	float pos;
	float max;
	float min;
	float last_pos;
	void *data;
	void (*slider_callback)(swidget_t *, void *, float);
}wslider_t;

typedef struct
{
	swidget_t swidget;
	int slider_count;
	int max_sliders;
	void *data;
	void (*slider_group_callback)(swidget_t *, void *, int);
	wslider_t *sliders;
}wslidergroup_t;

typedef struct
{
	swidget_t swidget;
	void (*viewport_callback)(swidget_t *, int);
}wviewport_t;

typedef struct
{
	swidget_t swidget;
	unsigned int src_id;
	void (*surface_callback)(swidget_t *, int);
}wsurface_t;


#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void gui_Init();

PEWAPI void gui_Finish();

PEWAPI widget_t *gui_CreateWidget(char *name, int bm_flags, float x, float y, float w, float h, float r, float g, float b, float a, unsigned int tex_handle, int b_focused, void (*widget_callback)(widget_t *));

PEWAPI void gui_ShareBorders(widget_t *a, widget_t *b, int a_border, int b_border);

PEWAPI void gui_AddButton(widget_t *widget, char *name, int bm_flags, int bm_button_flags, float x, float y, float w, float h, float r, float g, float b, float a, void *data, void (*widget_callback)(swidget_t *, void *));

PEWAPI void gui_AddVar(widget_t *widget, char *name, int bm_flags, int var_flags, int type, float x, float y, float w, float h,  void *var);

PEWAPI wdropdown_t *gui_AddDropDown(widget_t *widget, char *name, int bm_flags, int bm_border_hooks, float x_offset, float y_offset, float w, void *data, void (*dropdown_callback)(swidget_t *, void *, int));

PEWAPI void gui_AddOption(wdropdown_t *dropdown, char *name);

PEWAPI wdropdown_t *gui_NestleDropDown(wdropdown_t *dropdown, int option_index, char *name, int bm_flags, int width, void *data, void (*dropdown_callback)(swidget_t *, void *, int));

PEWAPI void gui_AddSeparator(wdropdown_t *dropdown, char *name);

//PEWAPI widget_t *gui_AddNestedDropDown(wdropdown_t *dropdown, int option_index, float width);

PEWAPI wtabbar_t *gui_AddTabBar(widget_t *widget, char *name, int bm_flags, int bm_border_hooks, float x_offset, float y_offset, float w, float h, void (*tabbar_callback)(swidget_t *, void *, int));

PEWAPI int gui_AddTab(wtabbar_t *tabbar, char *name, int tab_flags);

PEWAPI void gui_AddVarToTab(wtabbar_t *tabbar, int tab_index, char *name, int bm_flags, int var_flags, int type, float x, float y, float w, float h,  void *var);

PEWAPI wslider_t *gui_AddSlider(widget_t *widget, char *name, short bm_flags, float x, float y, float width, float pos, void *data, void (*slider_callback)(swidget_t *, void *, float));

PEWAPI wslidergroup_t *gui_AddSliderGroup(widget_t *widget, char *name, short bm_flags, float x, float y, float width, int slider_count_hint, void *data, void (*slider_group_callback)(swidget_t *, void *, int));

PEWAPI wslider_t *gui_AddSliderToGroup(wslidergroup_t *slider_group, char *name, float pos, float max, float min, short bm_flags, void *data, void (*slider_callback)(swidget_t *, void *, float));

PEWAPI wsurface_t *gui_AddSurface(widget_t *widget, char *name, short bm_flags, float x, float y, float width, float height, unsigned int src_id, void (*surface_callback)(swidget_t *, int));
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













