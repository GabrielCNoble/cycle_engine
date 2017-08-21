#include "gui.h"
#include "input.h"
#include "gmath/vector.h"
#include "draw.h"
#include "console.h"
#include "pew.h"

extern input_cache input;
extern renderer_t renderer;
extern console_font font;

int widget_count;

#define MIN_WIDGET_WIDTH 32.0
#define MIN_WIDGET_HEIGHT 32.0 
#define SCROLLER_FIXED_DIMENSION 0.34



widget_t *widgets = NULL;
widget_t *last = NULL;
widget_t *top_widget = NULL;


swidget_t *active_swidget = NULL;

unsigned int param_pool_byte_offset = 0;
unsigned char *param_pool[512];


#ifdef __cplusplus
extern "C"
{
#endif


/*
=============
gui_Init
=============
*/
PEWAPI void gui_Init()
{
	
	widgets = (widget_t *)malloc(sizeof(widget_t));
	widgets->type = WIDGET_ROOT;
	widgets->next = NULL;
	widgets->prev = NULL;
	widgets->name = strdup("root");
	//widgets->affect = NULL;
	widgets->x = 0.0;
	widgets->y = 0.0;
	widgets->w = 0.0;
	widgets->h = 0.0;
	last = widgets;	
	widget_count = 0;
	//gelem_a.gelems=NULL;
	//gelem_a.gelem_count=0;
	
	/*window_classes.classes=NULL;
	window_classes.class_count=0;*/
	
	//gui_ResizeGelemArray(8);
	//gui_ResizeWindowClassArray(8);
	
}

/*
=============
gui_Finish
=============
*/
PEWAPI void gui_Finish()
{
	widget_t *t = widgets->next;
	widget_t *q;
	widget_t *r;
	widget_t *s;
	while(t)
	{
		q = t->next;
		free(t->name);
		/*r = t->w_widgets;
		while(r)
		{
			s = r->next;
			free(r->name);
			free(r);
			r = s;
		}*/
		free(t);
		t = q;
	}
	free(widgets);
	//free(gelem_a.gelems);
}


PEWAPI widget_t *gui_CreateWidget(char *name, int bm_flags, float x, float y, float w, float h, float r, float g, float b, float a, unsigned int tex_handle, int b_focused, void (*widget_callback)(widget_t *))
{
	widget_t *temp;
	float hw = w / 2.0;
	float hh = h / 2.0;
	temp = (widget_t *)malloc(sizeof(widget_t));
	temp->name = strdup(name);
	temp->type = WIDGET_BASE;						/* to it be updated in the first frame */
	temp->bm_flags = bm_flags | WIDGET_VISIBLE | WIDGET_GRABBED_HEADER;
	//temp->affect = NULL;
	
	if(w < MIN_WIDGET_WIDTH) w = MIN_WIDGET_WIDTH;
	if(h < MIN_WIDGET_HEIGHT) h = MIN_WIDGET_HEIGHT;
	temp->x = x;
	temp->y = y;		
	temp->w = w;
	temp->h = h;
	
	temp->cw = w;
	temp->ch = h;
	
	temp->r = r;
	temp->g = g;
	temp->b = b;
	temp->a = a;
	
	temp->hx = hw;
	temp->lx = hw;
	temp->hy = hh;
	temp->ly = hh; 
	
	//temp->text_buffer = (char *)malloc(renderer.screen_width * renderer.screen_height);
	
	temp->tex_handle = tex_handle;
	
	temp->next = NULL;
	temp->prev = last;
	
	last->next = temp;
	last = temp;
	
	
	temp->sub_widgets = NULL;
	temp->last_added = NULL;
	temp->sub_widgets_count = 0;
	temp->widget_callback = widget_callback;
	temp->active_borders = WIDGET_LEFT_BORDER | WIDGET_RIGHT_BORDER | WIDGET_TOP_BORDER | WIDGET_BOTTOM_BORDER;
	temp->left_border = NULL;
	temp->right_border = NULL;
	temp->top_border = NULL;
	temp->bottom_border = NULL;
	
	widget_count++;
	
	if(!widget_count || b_focused)
	{
		gui_SetFocused(temp);
	}

	
	
	
	
	return temp;
}

PEWAPI void gui_ShareBorders(widget_t *a, widget_t *b, int a_border, int b_border)
{
	if(a && b)
	{
		
		switch(a_border)
		{
			case WIDGET_LEFT_BORDER:
			case WIDGET_RIGHT_BORDER:
				if(!(b_border & (WIDGET_TOP_BORDER | WIDGET_BOTTOM_BORDER)))
				{
					switch(b_border)
					{
						case WIDGET_LEFT_BORDER:
							b->left_border = a;
							b->active_borders &= ~WIDGET_LEFT_BORDER;
						break;
						
						case WIDGET_RIGHT_BORDER:
							b->right_border = a;
							b->active_borders &= ~WIDGET_RIGHT_BORDER;
						break;
					}
				}
			break;
			
			case WIDGET_TOP_BORDER:
			case WIDGET_BOTTOM_BORDER:
				if(!(b_border & (WIDGET_LEFT_BORDER | WIDGET_RIGHT_BORDER)))
				{
					switch(b_border)
					{
						case WIDGET_TOP_BORDER:
							b->top_border = a;
							b->active_borders &= ~WIDGET_TOP_BORDER;
						break;
						
						case WIDGET_BOTTOM_BORDER:
							b->bottom_border = a;
							b->active_borders &= ~WIDGET_BOTTOM_BORDER;
						break;
					}
				}
			break;

		}
	
	}
}

PEWAPI void gui_AddButton(widget_t *widget, char *name, int bm_flags, int bm_button_flags, float x, float y, float w, float h, float r, float g, float b, float a, void *data, void (*widget_callback)(swidget_t *, void *))
{
	wbutton_t *btn;
	
	float hh;
	float hw;
	
	if(widget)
	{
		btn = (wbutton_t *)malloc(sizeof(wbutton_t));
		btn->swidget.name = strdup(name);
		btn->swidget.bm_flags = bm_flags;
		btn->swidget.x = x;
		btn->swidget.y = y;
		btn->swidget.w = w;
		btn->swidget.h = h;
		
		btn->swidget.cx = x;
		btn->swidget.cy = y;
		btn->swidget.cw = w;
		btn->swidget.ch = h;
		
		//btn->swidget.r = r;
		//btn->swidget.g = g;
		//btn->swidget.b = b;
		//btn->swidget.a = a;
		btn->swidget.next = NULL;
		btn->swidget.type = WIDGET_BUTTON;
		
		btn->swidget.widget_callback = widget_callback;
		btn->swidget.data = data;
		btn->swidget.bm_border_hooks = 0;
		
		btn->button_flags = bm_button_flags;
		
		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)btn;
			widget->last_added = (swidget_t *)btn;
		}
		else
		{
			widget->last_added->next = (swidget_t *)btn;
			widget->last_added = (swidget_t *)btn;
		}
		widget->sub_widgets_count++;
		
		hw = w / 2.0;
		hh = h / 2.0;
		
		if(btn->swidget.y + hh > widget->hy)
		{
			widget->hy = btn->swidget.y + hh;
		}
		
		if(btn->swidget.y - hh < widget->ly)
		{
			widget->ly = btn->swidget.y - hh;
		}
		
		
		return;
	}
}

PEWAPI void gui_AddVar(widget_t *widget, char *name, int bm_flags, int var_flags, int type, float x, float y, float w, float h,  void *var)
{
	wvar_t *v;
	if(widget)
	{
		v = (wvar_t *)malloc(sizeof(wvar_t));
		v->swidget.name = strdup(name);
		
		v->swidget.x = x;
		v->swidget.y = y;
		v->swidget.w = w;
		v->swidget.h = h;
		
		
		v->swidget.cx = x;
		v->swidget.cy = y;
		v->swidget.cw = w;
		v->swidget.ch = h;
		
		v->swidget.type = WIDGET_VAR;
		v->swidget.bm_flags = bm_flags;
		v->swidget.widget_callback = NULL;
		v->swidget.next = NULL;
		v->swidget.bm_border_hooks = 0;
		
		v->var = var;
		v->var_flags = var_flags;
		v->var_type = type;
		

		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)v;
			widget->last_added = (swidget_t *)v;
		}
		else
		{
			widget->last_added->next = (swidget_t *)v;
			widget->last_added = (swidget_t *)v;
		}
		widget->sub_widgets_count++;
		
	}
}

PEWAPI wdropdown_t *gui_AddDropDown(widget_t *widget, char *name, int bm_flags, int bm_border_hooks, float x_offset, float y_offset, float w, void *data, void (*dropdown_callback)(swidget_t *, void *, int))
{
	wdropdown_t *t = NULL;
	if(widget)
	{
		t = (wdropdown_t *)malloc(sizeof(wdropdown_t));
		t->swidget.name = strdup(name);
		
		//t->swidget.x = x;
		//t->swidget.y = y;
		t->swidget.w = w;
		t->swidget.h = OPTION_HEIGHT;
		
		if(bm_border_hooks & WIDGET_TOP_BORDER)
		{
			t->swidget.y = (widget->h / 2.0) - (t->swidget.h / 2.0) - y_offset;
			t->swidget.ch = (widget->h / 2.0) - t->swidget.y;
		}
		else if(bm_border_hooks & WIDGET_BOTTOM_BORDER)
		{
			t->swidget.y = (-widget->h / 2.0) + (t->swidget.h / 2.0) + y_offset;
			t->swidget.ch = (-widget->h / 2.0) - t->swidget.y;
		}
		else
		{
			t->swidget.y = y_offset;
		}
		
		if(bm_border_hooks & WIDGET_LEFT_BORDER)
		{
			t->swidget.x = (-widget->w / 2.0) + (t->swidget.w / 2.0) + x_offset;
			t->swidget.cw = (-widget->w / 2.0) - t->swidget.x;
		}
		else if(bm_border_hooks & WIDGET_RIGHT_BORDER)
		{
			t->swidget.x = (widget->w / 2.0) - (t->swidget.w / 2.0) - x_offset;
			t->swidget.cw = (widget->w / 2.0) - t->swidget.x;
		}
		else
		{
			t->swidget.x = x_offset;
		}
		
		
		t->swidget.cx = t->swidget.x;
		t->swidget.cy = t->swidget.y;
		//t->swidget.cw = w;
		//t->swidget.ch = OPTION_HEIGHT;
		
		t->swidget.type = WIDGET_DROP_DOWN;
		t->swidget.bm_flags &= ~(WIDGET_GRABBABLE | WIDGET_HEADER | WIDGET_RESIZABLE);
		t->swidget.widget_callback = NULL;
		t->swidget.next = NULL;
		t->swidget.bm_border_hooks = bm_border_hooks;
		
		t->cur_option = 0;
		t->max_options = 4;
		t->active_option = NULL;
		t->options = (woption_t *)malloc(sizeof(woption_t) * 4);
		t->option_count = 0;
		t->dropdown_callback = dropdown_callback;
		t->data = data;
		t->bm_flags = bm_flags;
		
		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		else
		{
			widget->last_added->next = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		widget->sub_widgets_count++;
	}
	
	return t;
}

PEWAPI void gui_AddOption(wdropdown_t *dropdown, char *name)
{
	woption_t *o;
	if(dropdown)
	{
		if(dropdown->option_count >= dropdown->max_options)
		{
			o = (woption_t *)malloc(sizeof(woption_t) * (dropdown->max_options + 4));
			memcpy(o, dropdown->options, sizeof(woption_t) * dropdown->max_options);
			free(dropdown->options);
			dropdown->options = o;
			dropdown->max_options += 4;
		}
		
		o = &dropdown->options[dropdown->option_count];		
		o->name = strdup(name);
		o->bm_flags = 0;
		o->nested = NULL;
		
		if(!dropdown->option_count)
		{
			dropdown->active_option = o;
			dropdown->cur_option = 0;
		}
		
		dropdown->option_count++;
	}
}

PEWAPI wdropdown_t *gui_NestleDropDown(wdropdown_t *dropdown, int option_index, char *name, int bm_flags, int width, void *data, void (*dropdown_callback)(swidget_t *, void *, int))
{
	wdropdown_t *t = NULL;
	if(dropdown)
	{
		if(option_index >= 0 && option_index < dropdown->option_count)
		{
			t = (wdropdown_t *)malloc(sizeof(wdropdown_t));
			t->swidget.name = strdup(name);
			
			t->swidget.x = dropdown->swidget.x + dropdown->swidget.w / 2.0 + width / 2;
			t->swidget.y = dropdown->swidget.y - OPTION_HEIGHT * (option_index + 1);
			t->swidget.w = width;
			t->swidget.h = OPTION_HEIGHT;
			
			
			t->swidget.cx = t->swidget.x;
			t->swidget.cy = t->swidget.y;
			t->swidget.cw = width;
			t->swidget.ch = OPTION_HEIGHT;
			
			t->swidget.type = WIDGET_DROP_DOWN;
			t->swidget.bm_flags &= ~(WIDGET_GRABBABLE | WIDGET_HEADER | WIDGET_RESIZABLE);
			t->swidget.widget_callback = NULL;
			t->swidget.next = NULL;
			t->swidget.bm_border_hooks = 0;
			
			t->cur_option = 0;
			t->max_options = 4;
			t->active_option = NULL;
			t->options = (woption_t *)malloc(sizeof(woption_t) * 4);
			t->option_count = 0;
			t->dropdown_callback = dropdown_callback;
			t->data = data;
			t->bm_flags = bm_flags | DROP_DOWN_DROPPED;
			
			dropdown->options[option_index].nested = (swidget_t *)t;
		}
	}
	
	return t;
	
}


PEWAPI wtabbar_t *gui_AddTabBar(widget_t *widget, char *name, int bm_flags, int bm_border_hooks, float x_offset, float y_offset, float w, float h, void (*tabbar_callback)(swidget_t *, void *, int ))
{
	wtabbar_t *t = NULL;
	if(widget)
	{
		t = (wtabbar_t *)malloc(sizeof(wtabbar_t));
		t->swidget.name = strdup(name);
		
		//t->swidget.x = x;
		//t->swidget.y = y;
		t->swidget.w = w;
		t->swidget.h = h;
		
		if(bm_border_hooks & WIDGET_TOP_BORDER)
		{
			t->swidget.y = (widget->h / 2.0) - (t->swidget.h / 2.0) - y_offset;
			t->swidget.ch = (widget->h / 2.0) - t->swidget.y;
		}
		else if(bm_border_hooks & WIDGET_BOTTOM_BORDER)
		{
			t->swidget.y = (-widget->h / 2.0) + (t->swidget.h / 2.0) + y_offset;
			t->swidget.ch = (-widget->h / 2.0) - t->swidget.y;
		}
		else
		{
			t->swidget.y = y_offset;
		}
		
		if(bm_border_hooks & WIDGET_LEFT_BORDER)
		{
			t->swidget.x = (-widget->w / 2.0) + (t->swidget.w / 2.0) + x_offset;
			t->swidget.cw = (-widget->w / 2.0) - t->swidget.x;
		}
		else if(bm_border_hooks & WIDGET_RIGHT_BORDER)
		{
			t->swidget.x = (widget->w / 2.0) - (t->swidget.w / 2.0) - x_offset;
			t->swidget.cw = (widget->w / 2.0) - t->swidget.x;
		}
		else
		{
			t->swidget.x = x_offset;
		}

		
		t->swidget.cx = t->swidget.x;
		t->swidget.cy = t->swidget.y;
		//t->swidget.cw = w;
		//t->swidget.ch = h;
		
		t->swidget.type = WIDGET_TAB_BAR;
		t->swidget.bm_flags = bm_flags;
		t->swidget.widget_callback = NULL;
		t->swidget.next = NULL;
		t->swidget.bm_border_hooks = bm_border_hooks;
		
		t->active_tab = NULL;
		t->max_tabs = 8;
		t->tabs = (wtab_t *)malloc(sizeof(wtab_t) * t->max_tabs);
		t->tab_count = 0;
		t->tabbar_callback = tabbar_callback;
		
		

		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		else
		{
			widget->last_added->next = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		widget->sub_widgets_count++;
	}
	
	return t;
}

PEWAPI int gui_AddTab(wtabbar_t *tabbar, char *name, int tab_flags)
{
	int tab_index = -1;
	wtab_t *temp;
	if(tabbar)
	{		
		if(tabbar->tab_count >= tabbar->max_tabs)
		{
			temp = (wtab_t *)malloc(sizeof(wtab_t) * tabbar->max_tabs + 8);
			memcpy(temp, tabbar->tabs, sizeof(wtab_t) * tabbar->max_tabs);
			free(tabbar->tabs);
			tabbar->tabs = temp;
			tabbar->max_tabs += 8;
		}
		tab_index = tabbar->tab_count;
		
		tabbar->tabs[tab_index].name = strdup(name);
		tabbar->tabs[tab_index].swidgets = NULL;
		tabbar->tabs[tab_index].last = NULL;
		tabbar->tabs[tab_index].swidget_count = 0;
		tabbar->tabs[tab_index].bm_flags = tab_flags;
		
		if(!tabbar->active_tab)
		{
			tabbar->active_tab = &tabbar->tabs[0];
			tabbar->tabs[0].bm_flags |= TAB_SELECTED;
		}
		
		tabbar->tab_count++;
	}
	
	return tab_index;
}

PEWAPI void gui_AddVarToTab(wtabbar_t *tabbar, int tab_index, char *name, int bm_flags, int var_flags, int type, float x, float y, float w, float h,  void *var)
{
	wvar_t *v;
	wtab_t *tab;
	if(tabbar)
	{
		if(tab_index >= 0 && tab_index < tabbar->tab_count)
		{
			
			tab = &tabbar->tabs[tab_index];
			
			if(tab->bm_flags & TAB_NO_SUB_WIDGETS)
			{
				return;
			}
			
			v = (wvar_t *)malloc(sizeof(wvar_t));
			v->swidget.name = strdup(name);
			
			v->swidget.x = x;
			v->swidget.y = y;
			v->swidget.w = w;
			v->swidget.h = h;
			
			
			v->swidget.cx = x;
			v->swidget.cy = y;
			v->swidget.cw = w;
			v->swidget.ch = h;
			
			v->swidget.type = WIDGET_VAR;
			v->swidget.bm_flags = bm_flags;
			v->swidget.widget_callback = NULL;
			v->swidget.next = NULL;
			v->swidget.bm_border_hooks = 0;
			
			v->var = var;
			v->var_flags = var_flags;
			v->var_type = type;
	
			if(!tab->swidgets)
			{
				tab->swidgets = (swidget_t *)v;
				tab->last = (swidget_t *)v;
			}
			else
			{
				tab->last->next = (swidget_t *)v;
				tab->last = (swidget_t *)v;
			}
			tab->swidget_count++;
		}
		
	}
}

PEWAPI wslider_t *gui_AddSlider(widget_t *widget, char *name, short bm_flags, float x, float y, float width, float pos, void *data, void (*slider_callback)(swidget_t *, void *, float))
{
	wslider_t *t = NULL;
	if(widget)
	{
		t = (wslider_t *)malloc(sizeof(wslider_t));
		t->swidget.name = strdup(name);
		
		t->swidget.x = x;
		t->swidget.y = y;
		t->swidget.w = width;
		t->swidget.h = SLIDER_OUTER_HEIGHT;
		
		
		t->swidget.cx = x;
		t->swidget.cy = y;
		t->swidget.cw = width;
		t->swidget.ch = SLIDER_OUTER_HEIGHT;
		
		t->swidget.type = WIDGET_SLIDER;
		t->swidget.bm_flags = 0;
		t->swidget.widget_callback = NULL;
		t->swidget.next = NULL;
		t->swidget.bm_border_hooks = 0;
		
		t->bm_flags = bm_flags;
		t->data = data;
		t->slider_callback = slider_callback;
		t->pos = pos;		
		t->last_pos = pos;

		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		else
		{
			widget->last_added->next = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		widget->sub_widgets_count++;
	}
	
	return t;
}

PEWAPI wslidergroup_t *gui_AddSliderGroup(widget_t *widget, char *name, short bm_flags, float x, float y, float width, int slider_count_hint, void *data, void (*slider_group_callback)(swidget_t *, void *, int))
{
	wslidergroup_t *t = NULL;
	wslider_t *s;
	float yc;
	float yp;
	int i;
	if(widget)
	{
		t = (wslidergroup_t *)malloc(sizeof(wslidergroup_t));
		t->swidget.name = strdup(name);
		
		t->swidget.type = WIDGET_SLIDER_GROUP;
		t->swidget.bm_flags = 0;
		t->swidget.widget_callback = NULL;
		t->swidget.next = NULL;
		
		//t->bm_flags = bm_flags;
		t->data = data;
		t->slider_group_callback = slider_group_callback;
		
		t->swidget.x = x;
		t->swidget.y = y;
		t->swidget.w = width;
		t->swidget.h = SLIDER_INNER_HEIGHT;
		
		t->swidget.cx = x;
		t->swidget.cy = y;
		t->swidget.cw = width;
		t->swidget.ch = SLIDER_INNER_HEIGHT;
		t->swidget.bm_border_hooks = 0;
		
		if(slider_count_hint > 0)
		{
			t->sliders = (wslider_t *)malloc(sizeof(wslider_t ) * slider_count_hint);
			t->max_sliders = slider_count_hint;
		}
		else
		{
			t->sliders = NULL;
			t->max_sliders = 0;
		}
		
		t->slider_count = 0;

		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		else
		{
			widget->last_added->next = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		widget->sub_widgets_count++;
	}
	
	return t;
}

PEWAPI wslider_t *gui_AddSliderToGroup(wslidergroup_t *slider_group, char *name, float pos, float max, float min, short bm_flags, void *data, void (*slider_callback)(swidget_t *, void *, float))
{
	wslider_t *t = NULL;
	int i;
	float ys;
	float yc;
	if(slider_group)
	{
		if(slider_group->slider_count >= slider_group->max_sliders)
		{
			t = (wslider_t *)malloc(sizeof(wslider_t ) * (slider_group->max_sliders + 4));
			memcpy(t, slider_group->sliders, sizeof(wslider_t *) * slider_group->max_sliders);
			free(slider_group->sliders);
			slider_group->sliders = t;
			
			for(i = 0; i < slider_group->slider_count - 1; i++)
			{
				slider_group->sliders[i].swidget.next = &slider_group->sliders[i + 1].swidget;
			}
			slider_group->max_sliders += 4;
		}
		
		
		//t = (wslider_t *)malloc(sizeof(wslider_t));
		
		t = &slider_group->sliders[slider_group->slider_count];
		t->swidget.name = strdup(name);
		
		t->swidget.x = 0;
		t->swidget.w = slider_group->swidget.w;
		t->swidget.h = SLIDER_OUTER_HEIGHT;
		
		t->swidget.cx = 0;
		t->swidget.cw = slider_group->swidget.w;
		t->swidget.ch = SLIDER_OUTER_HEIGHT;
		
		
		
		t->swidget.type = WIDGET_SLIDER;
		t->swidget.bm_flags = 0;
		t->swidget.widget_callback = NULL;
		t->swidget.next = NULL;
		t->swidget.bm_border_hooks = 0;
		
		t->bm_flags = bm_flags;
		t->data = data;
		t->slider_callback = slider_callback;
		t->pos = pos;		
		t->last_pos = pos;
		t->max = max;
		t->min = min;
		
		slider_group->sliders[slider_group->slider_count] = *t;
		
		if(slider_group->slider_count > 0)
		{
			slider_group->sliders[slider_group->slider_count - 1].swidget.next = (swidget_t *)t;
		}
		slider_group->slider_count++;
		
		slider_group->swidget.h = ((SLIDER_OUTER_HEIGHT + 8.0) * slider_group->slider_count) / 2.0;
		slider_group->swidget.ch = slider_group->swidget.h;
		
		ys = SLIDER_OUTER_HEIGHT + 8.0;
		
		if(slider_group->slider_count == 1)
		{
			yc = 0;
		}
		else
		{
			yc = slider_group->swidget.h;	
		}
		
		for(i = 0; i < slider_group->slider_count; i++)
		{
			slider_group->sliders[i].swidget.y = yc;
			yc -= ys;
		}
		
	}
	
	return t;
}


PEWAPI wsurface_t *gui_AddSurface(widget_t *widget, char *name, short bm_flags, float x, float y, float width, float height, unsigned int src_id, void (*surface_callback)(swidget_t *, int))
{
	wsurface_t *t = NULL;
	if(widget)
	{
		t = (wsurface_t *)malloc(sizeof(wsurface_t));
		t->swidget.name = strdup(name);
		
		t->swidget.x = x;
		t->swidget.y = y;
		t->swidget.w = width;
		t->swidget.h = height;
		
		
		t->swidget.cx = x;
		t->swidget.cy = y;
		t->swidget.cw = width;
		t->swidget.ch = height;
		
		t->swidget.type = WIDGET_SURFACE;
		t->swidget.bm_flags = bm_flags;
		t->swidget.widget_callback = NULL;
		t->swidget.next = NULL;
		t->swidget.bm_border_hooks = 0;
		
		//t->bm_flags = bm_flags;
		//t->data = data;
		t->surface_callback = surface_callback;
		t->src_id = src_id;

		if(!widget->sub_widgets)
		{
			widget->sub_widgets = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		else
		{
			widget->last_added->next = (swidget_t *)t;
			widget->last_added = (swidget_t *)t;
		}
		widget->sub_widgets_count++;
	}
	
	return t;
}

PEWAPI void gui_DeleteWidgetByName(char *name)
{
	swidget_t *w;

}

PEWAPI void gui_DeleteWidget(widget_t *widget)
{
	swidget_t *w;
	swidget_t *p;
	wtab_t *tab; 
	wslider_t *slider;
	wslidergroup_t *slidergroup;
	int i;
 	int stack_top = -1;
	swidget_t *stack[64];
	w = widget->sub_widgets;
	
	_recursive_delete:

	while(w)
	{
		switch(w->type)
		{
			case WIDGET_TAB_BAR:
				if(((wtabbar_t *)w)->tab_count)
				{
					
					for(i = 0; i < ((wtabbar_t *)w)->tab_count; i++)
					{
						tab = &((wtabbar_t *)w)->tabs[i];
						
						if(tab->bm_flags != 0xffffffff)
						{
							free(tab->name);
							tab->bm_flags = 0xffffffff;
							if(tab->swidget_count)
							{
								stack_top++;
								stack[stack_top] = w;
								w = tab->swidgets;
								goto _recursive_delete;	
							}
						}
					}
				}
				free(w->name);
				
				p = w->next;
				free(w->name);
				free(((wtabbar_t *)w)->tabs);
				free(w);		
				w = p;
			break;
			
			
			case WIDGET_DROP_DOWN:	
				for(i = 0; i < ((wdropdown_t *)w)->option_count; i++)
				{
					if(((wdropdown_t *)w)->options[i].nested)
					{
						stack_top++;
						stack[stack_top] = w;
						p = ((wdropdown_t *)w)->options[i].nested;
						((wdropdown_t *)w)->options[i].nested = NULL;
						w = p;
						goto _recursive_delete;
					}
				}
				//free(w->name);
				//w->name = NULL;
				free(((wdropdown_t *)w)->options);
			case WIDGET_BUTTON:
			case WIDGET_VAR:
			case WIDGET_SLIDER:
				
				p = w->next;
				free(w->name);
				free(w);
				w = p;
			break;
			
			case WIDGET_SLIDER_GROUP:
				slidergroup = (wslidergroup_t *)w;
				slider = slidergroup->sliders;
				
				p = w->next;
				free(w->name);
				free(w);
				w = p;
				
				if(slider)
				{
					stack_top++;
					stack[stack_top] = w;
					w = (swidget_t *)slider;
					goto _recursive_delete;
				}
			break;
			
			default:
				w = w->next;
			break;
		}
		
		if(!w)
		{
			if(stack_top >= 0)
			{
				w = stack[stack_top--];
			}
		}

	}
	
	if(widget == last)
	{
		last = last->prev;
	}
	
	if(widget->prev)
	{
		widget->prev->next = widget->next;
	}
	if(widget->next)
	{
		widget->next->prev = widget->prev;
	}
	
	widget_count--;
	
	gui_SetFocused(last);
	
	
	
	
	
	free(widget->name);
	free(widget);
	
}

PEWAPI void gui_MarkForDeletion(widget_t *widget)
{
	if(widget)
	{
		widget->bm_flags |= WIDGET_DELETE;
	}
}

PEWAPI widget_t *gui_GetWidget(char *name)
{
	
}

PEWAPI void gui_ShowWidget(widget_t *widget)
{
	widget->bm_flags |= WIDGET_VISIBLE;
	
}

PEWAPI void gui_HideWidget(widget_t *widget)
{
	widget->bm_flags &= ~WIDGET_VISIBLE;
}

void gui_SetFocused(widget_t *widget)
{
	if(widget)
	{
		/* if there's just one widget, there's no need
		to update anything... */
		if(widget_count > 1)
		{
			top_widget->bm_flags &= ~WIDGET_MOUSE_OVER;
			if(widget != widgets->next)
			{
				
				if(widget == last)
				{
					last = last->prev; 
				}
				
				
				if(widget->next)
				{
					widget->next->prev = widget->prev;
				}
				
				widget->prev->next = widget->next;
				
				widget->prev = widgets;
				widget->next = widgets->next;
				widget->next->prev = widget;
				widgets->next = widget;
				
			}

		}
		
		top_widget = widget;
	}
}

void gui_ProcessWidgets()
{
	widget_t *cwidget;
	widget_t *p;
	swidget_t *cswidget;
	wbutton_t *button;
	wtabbar_t *tabbar;
	wdropdown_t *dropdown;
	wtab_t *tab;
	wslider_t *slider;
	wslidergroup_t *sslider;
	int stack_top = -1;
	swidget_t *swidget_stack[64];
	vec2_t pos_stack[64];
	int mouse_over_widgets = 0;
	
	float x_scale = 2.0 / renderer.width;
	float y_scale = 2.0 / renderer.height;
	
	float rel_x;
	float rel_y;
	float tab_label_width;
	float option_height;
	
	int tab_index;
	int option_index;
	int pw;
	int ph;
	
	float rw;
	float rh;
	float rx;
	float ry;
	
	int rel_ix;
	int rel_iy;
	
	float dw;
	float c_dw;
	float old_d;
	float dx;
	float dy;
	
	float x;
	float y;
	float hw;
	float hh;
	float cw;
	float ch;
	
	input.bm_mouse &= ~MOUSE_OVER_WIDGET;
	
	/*if(widget_count > 1)
	{
		cwidget = top_widget;
		goto _go;
	}
	else
	{*/
	cwidget = widgets->next;
	//}
	
	//active_swidget = NULL;
	
	while(cwidget)
	{
		
		//_go:
		
		if(!(cwidget->bm_flags & WIDGET_VISIBLE))
		{
			goto _skip;
		}
			
		cwidget->relative_mouse_x = ((cwidget->x * x_scale - input.normalized_mouse_x) * -2.0) / (cwidget->w * x_scale);
		cwidget->relative_mouse_y = ((cwidget->y * y_scale - input.normalized_mouse_y) * -2.0) / (cwidget->h * y_scale);
		
		cw = cwidget->w / cwidget->cw;
		ch = cwidget->h / cwidget->ch;
		
		/* If this isn't the top widget and the top widget doesn't have the mouse over it, they don't
		overlap, which means that is necessary to calculate this widget's relative mouse position. Otherwise, 
		this widget is the top widget, and HAS to have it's stuff updated... */	
		if(((cwidget == top_widget) || (!(top_widget->bm_flags & WIDGET_MOUSE_OVER))) &&
		        cwidget->relative_mouse_x <= 1.0 && cwidget->relative_mouse_x >= -1.0)
		{
			if(cwidget->relative_mouse_y <= 1.0 && cwidget->relative_mouse_y >= -1.0)
			{
				pw = renderer.screen_width * cwidget->w * x_scale * 0.5;
				ph = renderer.screen_height * cwidget->h * y_scale * 0.5;
				
				rel_ix = pw * (cwidget->relative_mouse_x * 0.5 + 0.5);
				rel_iy = ph * (cwidget->relative_mouse_y * 0.5 + 0.5);
				
				cwidget->bm_flags |= WIDGET_MOUSE_OVER;
				
				if(!(cwidget->bm_flags & WIDGET_IGNORE_MOUSE))
				{
					input.bm_mouse |= MOUSE_OVER_WIDGET;
					
					if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
					{
						/* There's an active subwidget from another widget. Only allow this
						widget to become the top widget if the mouse is not on top of the
						active subwidget... */
						if(active_swidget)
						{
							if(active_swidget->bm_flags & WIDGET_MOUSE_OVER)
							{
								//goto _skip1;
								/* well... this label doesn't have function scope...
								this might create problems later on... */	
								asm("jmp _skip1\n");
							}
						}
						
						cwidget->bm_flags |= WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
						if(cwidget != top_widget)
						{
							gui_SetFocused(cwidget);
							active_swidget == NULL;
						}
						
						//_skip1:
						asm("_skip1:\n");
					}
					else
					{
						cwidget->bm_flags &= ~WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
					}
					
					if(input.bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
					{
						cwidget->bm_flags |= WIDGET_RECEIVED_RIGHT_BUTTON_DOWN;
					}
					else
					{
						cwidget->bm_flags &= ~WIDGET_RECEIVED_RIGHT_BUTTON_DOWN;
					}
					
				}
				
				mouse_over_widgets++;
				
				

				if(rel_iy >= ph - WIDGET_BORDER_PIXEL_WIDTH && (cwidget->active_borders & WIDGET_TOP_BORDER))
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_TOP_BORDER;
				}
				else if(rel_iy <= WIDGET_BORDER_PIXEL_WIDTH && (cwidget->active_borders & WIDGET_BOTTOM_BORDER))
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_BOTTOM_BORDER;
				}
				else
				{
					cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER_TOP_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER);
				}
				
				
				if(rel_ix >= pw - WIDGET_BORDER_PIXEL_WIDTH && (cwidget->active_borders & WIDGET_RIGHT_BORDER))
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_RIGHT_BORDER;
				}
				else if(rel_ix <= WIDGET_BORDER_PIXEL_WIDTH && (cwidget->active_borders & WIDGET_LEFT_BORDER))
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_LEFT_BORDER;
				}
				else
				{
					cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_RIGHT_BORDER);
				}	
				
				if(cwidget->bm_flags & WIDGET_HEADER)
				{
					if(rel_iy >= ph - WIDGET_HEADER_PIXEL_HEIGHT || (cwidget->bm_flags & WIDGET_GRABBED_HEADER))
					{
						cwidget->bm_flags |= WIDGET_MOUSE_OVER_HEADER;
						cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER_TOP_BORDER | WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_RIGHT_BORDER);
					}
					else
					{
						cwidget->bm_flags &= ~WIDGET_MOUSE_OVER_HEADER;
					}
				}
				
			}	
			else
			{
				cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_MOUSE_OVER_HEADER | WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_RIGHT_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER | WIDGET_MOUSE_OVER_TOP_BORDER);
			}
		}
		else
		{
			cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_MOUSE_OVER_HEADER | WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_RIGHT_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER | WIDGET_MOUSE_OVER_TOP_BORDER);
		}
		
		/* If this widget is not the top widget and the top widget doesn't have the mouse over it, it means they
		don't overlap, so it's safe to allow the pointer to change. If this is the top widget, then it's also safe
		to set the cursor, for the top widget has the final word about the cursor used... */
		if(((cwidget == top_widget) || (!(top_widget->bm_flags & WIDGET_MOUSE_OVER))))
		{
			
			if(cwidget->bm_flags & WIDGET_RESIZABLE)
			{
				switch(cwidget->bm_flags & (WIDGET_MOUSE_OVER_TOP_BORDER | 
									WIDGET_MOUSE_OVER_BOTTOM_BORDER |
									WIDGET_MOUSE_OVER_LEFT_BORDER | 
									WIDGET_MOUSE_OVER_RIGHT_BORDER))
									
				{
					case WIDGET_MOUSE_OVER_BOTTOM_BORDER:
					case WIDGET_MOUSE_OVER_TOP_BORDER:
						input_SetCursor(CURSOR_VERTICAL_ARROWS);
					break;
					
					case WIDGET_MOUSE_OVER_LEFT_BORDER:
					case WIDGET_MOUSE_OVER_RIGHT_BORDER:
						input_SetCursor(CURSOR_HORIZONTAL_ARROWS);
					break;	
					
					case WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_TOP_BORDER:
					case WIDGET_MOUSE_OVER_RIGHT_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER:
						input_SetCursor(CURSOR_LEFT_DIAGONAL_ARROWS);
					break;
					
					
					case WIDGET_MOUSE_OVER_RIGHT_BORDER | WIDGET_MOUSE_OVER_TOP_BORDER:
					case WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER:
						input_SetCursor(CURSOR_RIGHT_DIAGONAL_ARROWS);
					break;
					
					default:
						input_SetCursor(CURSOR_ARROW);
					break;
						
				}
			}
			
			if(cwidget->widget_callback)
			{
				cwidget->widget_callback(cwidget);
			}
		}
								
		
		
		if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_HEADER)
			{
				cwidget->bm_flags |= WIDGET_GRABBED_HEADER;
			}
			else
			{
				cwidget->bm_flags &= ~WIDGET_GRABBED_HEADER;
				
				if(cwidget->bm_flags & WIDGET_MOUSE_OVER_TOP_BORDER)
				{
					cwidget->bm_flags |= WIDGET_GRABBED_TOP_BORDER;
				}
				else
				{
					cwidget->bm_flags &= ~WIDGET_GRABBED_TOP_BORDER;
				}
			}
			
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_LEFT_BORDER)
			{
				cwidget->bm_flags |= WIDGET_GRABBED_LEFT_BORDER;
			}
			else
			{
				cwidget->bm_flags &= ~WIDGET_GRABBED_LEFT_BORDER;
			}
			
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_RIGHT_BORDER)
			{
				cwidget->bm_flags |= WIDGET_GRABBED_RIGHT_BORDER;
			}
			else
			{
				cwidget->bm_flags &= ~WIDGET_GRABBED_RIGHT_BORDER;
			}
			
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_BOTTOM_BORDER)
			{
				cwidget->bm_flags |= WIDGET_GRABBED_BOTTOM_BORDER;
			}
			else
			{
				cwidget->bm_flags &= ~WIDGET_GRABBED_BOTTOM_BORDER;
			}
			
			
		}
		
		
		if((cwidget->bm_flags & WIDGET_GRABBABLE) && (cwidget->bm_flags & WIDGET_MOVABLE))
		{
			
			if(cwidget->bm_flags & WIDGET_GRABBED_HEADER)
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cwidget->bm_flags &= ~WIDGET_GRABBED_HEADER;
				}
				else
				{
					cwidget->x += input.mouse_dx / x_scale;
					cwidget->y += input.mouse_dy / y_scale;
				}
			}
			
			if(cwidget->bm_flags & (WIDGET_GRABBED_LEFT_BORDER | WIDGET_GRABBED_RIGHT_BORDER))
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cwidget->bm_flags &= ~(WIDGET_GRABBED_LEFT_BORDER | WIDGET_GRABBED_RIGHT_BORDER);
				}
				else
				{
					old_d = cwidget->w;	
					cwidget->w += (input.mouse_dx / x_scale) * (1.0 - (2.0 * ((cwidget->bm_flags & WIDGET_GRABBED_LEFT_BORDER) && 1)));
					dx = 0.0;
					if(cwidget->w < MIN_WIDGET_WIDTH)
					{
						dx = MIN_WIDGET_WIDTH - cwidget->w;
						dx /= old_d - cwidget->w;
						cwidget->w = MIN_WIDGET_WIDTH;
					}
					cwidget->x += ((input.mouse_dx * (1.0 - dx)) * 0.5) / x_scale;
				}
			}		
				
			if(cwidget->bm_flags & (WIDGET_GRABBED_BOTTOM_BORDER | WIDGET_GRABBED_TOP_BORDER))
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cwidget->bm_flags &= ~(WIDGET_GRABBED_BOTTOM_BORDER | WIDGET_GRABBED_TOP_BORDER);
				}
				
				old_d = cwidget->h;	
				cwidget->h += (input.mouse_dy / y_scale) * (1.0 - (2.0 * ((cwidget->bm_flags & WIDGET_GRABBED_BOTTOM_BORDER) && 1)));
				dy = 0.0;
				if(cwidget->h < MIN_WIDGET_HEIGHT)
				{
					dy = MIN_WIDGET_HEIGHT - cwidget->h;
					dy /= old_d - cwidget->h;
					cwidget->h = MIN_WIDGET_HEIGHT;
				}
				cwidget->y += ((input.mouse_dy * (1.0 - dy)) * 0.5) / y_scale;
			}
			
			
			if(cwidget->bottom_border)
			{
				hh = cwidget->bottom_border->h / 2.0;
				y = cwidget->bottom_border->y + hh;
				
				hh = cwidget->h / 2.0;
				
				dy = cwidget->y - hh - y;
				
				old_d = cwidget->h;
				
				cwidget->y -= dy * 0.5;
				cwidget->h += dy;
				
				if(cwidget->h < MIN_WIDGET_HEIGHT)
				{
					dy = MIN_WIDGET_HEIGHT - cwidget->h;
					cwidget->h = MIN_WIDGET_HEIGHT;
					cwidget->y -= dy * 0.5;
					
					cwidget->bottom_border->h -= dy;
					cwidget->bottom_border->y -= dy * 0.5;					
				}
				
			}
			
			if(cwidget->top_border)
			{
				hh = cwidget->top_border->h / 2.0;
				y = cwidget->top_border->y - hh;
				
				hh = cwidget->h / 2.0;
				
				dy = cwidget->y + hh - y;
				
				cwidget->y -= dy * 0.5;
				cwidget->h -= dy;
				
				if(cwidget->h < MIN_WIDGET_HEIGHT)
				{
					dy = MIN_WIDGET_HEIGHT - cwidget->h;
					cwidget->h = MIN_WIDGET_HEIGHT;
					cwidget->y += dy * 0.5;
					
					cwidget->top_border->h -= dy;
					cwidget->top_border->y += dy * 0.5;					
				}
				
			}
			 
			
			if(cwidget->right_border)
			{
				hw = cwidget->right_border->w / 2.0;
				x = cwidget->right_border->x - hw;
				
				hw = cwidget->w / 2.0;
				
				dx = cwidget->x + hw - x;
				
				cwidget->x -= dx * 0.5;
				cwidget->w -= dx;
				
				if(cwidget->w < MIN_WIDGET_WIDTH)
				{
					dx = MIN_WIDGET_WIDTH - cwidget->w;
					cwidget->w = MIN_WIDGET_WIDTH;
					cwidget->x += dx * 0.5;
					
					cwidget->right_border->w -= dx;
					cwidget->right_border->x += dx * 0.5;					
				}
				
			}
			
			
			if(cwidget->left_border)
			{
				hw = cwidget->left_border->w / 2.0;
				x = cwidget->left_border->x + hw;
				
				hw = cwidget->w / 2.0;
				
				dx = cwidget->x - hw - x;
				
				cwidget->x -= dx * 0.5;
				cwidget->w += dx;
				
				if(cwidget->w < MIN_WIDGET_WIDTH)
				{
					dx = MIN_WIDGET_WIDTH - cwidget->w;
					cwidget->w = MIN_WIDGET_WIDTH;
					cwidget->x -= dx * 0.5;
					
					cwidget->left_border->w -= dx;
					cwidget->left_border->x -= dx * 0.5;					
				}
				
			}
			
		}
		
		x = cwidget->x;
		y = cwidget->y;
		
		cswidget = cwidget->sub_widgets;
		
		while(cswidget)
		{
			_nestled_swidgets:
				
			
			if(cswidget->bm_border_hooks & WIDGET_BOTTOM_BORDER)
			{
				dy = cswidget->ch;
				dy = -(cwidget->h / 2.0) - cswidget->y - dy;
				cswidget->y += dy;
			}
			
			
			if(cswidget->bm_border_hooks & WIDGET_LEFT_BORDER)
			{
				dx = cswidget->cw;
				dx = -(cwidget->w / 2.0) - cswidget->x - dx;
				
				//printf("%f\n", d)
				
				cswidget->x += dx;
			}
			
			/*if(cswidget->bm_border_hooks)
			{
				switch(cswidget->bm_border_hooks)
				{
					case WIDGET_BOTTOM_BORDER:
						dy = cswidget->ch;
						dy = -(cwidget->h / 2.0) - cswidget->y - dy;
						cswidget->y += dy;
					break;
				}
			}	*/
			
			/*if(cswidget->bm_flags & WIDGET_KEEP_RELATIVE_X_POSITION)
			{
				cswidget->x = cswidget->cx * cw;
				
				if(!(cswidget->bm_flags & WIDGET_LOCK_X_SCALE))
				{
					cswidget->w = cswidget->cw + (cwidget->w - cwidget->cw);
				}
			}
			
			if(cswidget->bm_flags & WIDGET_KEEP_RELATIVE_Y_POSITION)
			{
				cswidget->y = cswidget->cy * ch;
			}*/
			
			ry = cswidget->y;
			rh = cswidget->h;
			rw = cswidget->w;
			
			
			/* if this subwidget is a drop down box and it's open,
			increase its height and move its origin so the [-1.0, 1.0]
			range spans across the whole subwidget. This allow correct relative
			mouse position calculation regardless of it being open or closed... */
			if(cswidget->type == WIDGET_DROP_DOWN)
			{				
				if(((wdropdown_t *)cswidget)->bm_flags & DROP_DOWN_DROPPED)
				{
					rh += (OPTION_HEIGHT * ((wdropdown_t *)cswidget)->option_count);
					ry -= (OPTION_HEIGHT * ((wdropdown_t *)cswidget)->option_count) / 2.0;
				}
			}
			/* if this subwidget is a slider increase its width
			by half of its outer height. This allow correct
			picking even if the cursor border spans out of the
			subwidget boundaries... */
			else if(cswidget->type == WIDGET_SLIDER)
			{
				rw += SLIDER_OUTER_HEIGHT / 2.0;
			}
			
			
			//printf(" %f  %f\n", ry, rh);
			
			cswidget->relative_mouse_x = (((cswidget->x + x) * x_scale - input.normalized_mouse_x) * -2.0) / (rw * x_scale);
			cswidget->relative_mouse_y = (((ry + y) * y_scale - input.normalized_mouse_y) * -2.0) / (rh * y_scale);
			
			//printf("%s  %f %f\n", cswidget->name, cswidget->relative_mouse_x, cswidget->relative_mouse_y);
			 
			if((active_swidget == cswidget) || (cwidget->bm_flags & WIDGET_MOUSE_OVER) && !(cwidget->bm_flags & (WIDGET_MOUSE_OVER_HEADER |
																			   						  WIDGET_MOUSE_OVER_BOTTOM_BORDER|
																			   						  WIDGET_MOUSE_OVER_TOP_BORDER|
																			   						  WIDGET_MOUSE_OVER_LEFT_BORDER|
																			   						  WIDGET_MOUSE_OVER_RIGHT_BORDER)) ||
																			   						(!(cwidget->bm_flags & WIDGET_NO_BORDERS)))
			{
				if(cswidget->relative_mouse_x <= 1.0 && cswidget->relative_mouse_x >= -1.0)
				{
					if(cswidget->relative_mouse_y <= 1.0 && cswidget->relative_mouse_y >= -1.0)
					{
						cswidget->bm_flags |= WIDGET_MOUSE_OVER;
						
						if(cwidget->bm_flags & WIDGET_IGNORE_MOUSE)
						{
							input.bm_mouse |= MOUSE_OVER_WIDGET;
						}
						
						//active_swidget = cswidget;
						//cwidget->active_swidget = cswidget;
						
						if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
						{
							cswidget->bm_flags |= (WIDGET_RECEIVED_LEFT_BUTTON_DOWN | WIDGET_LEFT_BUTTON_DOWN);
							cwidget->active_swidget = cswidget;
							
							if(cswidget->type == WIDGET_DROP_DOWN)
							{
								active_swidget = cswidget;	
							}
							
							
							
							/* in the case the widget that contains the current
							subwidget being processed is not the top widget. Setting
							it as top widget will draw it on top of any other 
							widget/subwidget */
							if(top_widget != cwidget)
							{
								gui_SetFocused(cwidget);
							}
							
						}
						else
						{
							cswidget->bm_flags &= ~WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
						}
						
						if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_RELEASED)
						{
							cswidget->bm_flags |= WIDGET_RECEIVED_LEFT_BUTTON_UP;
							cswidget->bm_flags &= ~WIDGET_LEFT_BUTTON_DOWN;
							cwidget->active_swidget = NULL;
						}
						else
						{
							cswidget->bm_flags &= ~WIDGET_RECEIVED_LEFT_BUTTON_UP;
						}
						
			
					}
					else
					{
						cswidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_RECEIVED_LEFT_BUTTON_UP | WIDGET_RECEIVED_LEFT_BUTTON_DOWN);
					}

				}
				else
				{
					cswidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_RECEIVED_LEFT_BUTTON_UP | WIDGET_RECEIVED_LEFT_BUTTON_DOWN);
				}
			}
			else
			{
				cswidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_RECEIVED_LEFT_BUTTON_UP | WIDGET_RECEIVED_LEFT_BUTTON_DOWN);
			}
			
			if(!(cswidget->bm_flags & WIDGET_MOUSE_OVER))
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cswidget->bm_flags &= ~WIDGET_LEFT_BUTTON_DOWN;
				}
			}
			
			
			switch(cswidget->type)
			{
				case WIDGET_BUTTON:
					button = (wbutton_t *)cswidget;
					
					if(cswidget->bm_flags & WIDGET_MOUSE_OVER)
					{
						if(cswidget->bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_DOWN)
						{
							if(button->button_flags & BUTTON_CHECK_BOX)
							{
								if(button->button_flags & BUTTON_CHECK_BOX_CHECKED)
								{
									button->button_flags &= ~BUTTON_CHECK_BOX_CHECKED;
								}
								else
								{
									button->button_flags |= BUTTON_CHECK_BOX_CHECKED;
								}
							}
							/* a normal button... */
							else
							{
								if(button->button_flags & BUTTON_TOGGLE)
								{
									if(button->button_flags & BUTTON_TOGGLED)
									{
										button->button_flags &= ~BUTTON_TOGGLED;
									}
									else
									{
										button->button_flags |= BUTTON_TOGGLED;
									}
								}
								else
								{
									button->button_flags |= BUTTON_TOGGLED;
								}
							}
										
							if(cswidget->widget_callback)
							{
								cswidget->widget_callback(cswidget, cswidget->data);
							}
										
						}
						else if(cswidget->bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_UP)
						{
										
							if(button->button_flags & BUTTON_TOGGLE)
							{
													
							}
							else
							{
								button->button_flags &= ~BUTTON_TOGGLED;
							}
										
						}
					}
									
					
									
									
				break;
							
				case WIDGET_TAB_BAR:
					tabbar = (wtabbar_t *)cswidget;
					
					if(cswidget->bm_flags & WIDGET_MOUSE_OVER)
					{
						if(tabbar->tab_count)
						{
							tab_index = (int)((tabbar->swidget.relative_mouse_x * 0.5 + 0.5) * tabbar->tab_count);									
										
							if(tabbar->swidget.bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_DOWN)
							{
								if(tabbar->active_tab != &tabbar->tabs[tab_index])
								{
									if(tabbar->active_tab)
									{
										tabbar->active_tab->bm_flags &= ~TAB_SELECTED;
									}
								}
											
								tabbar->active_tab = &tabbar->tabs[tab_index];
								tabbar->active_tab->bm_flags |= TAB_SELECTED;
											
								if(tabbar->tabbar_callback)
								{
									tabbar->tabbar_callback(cswidget, NULL, tab_index);
								}			
							}
										
							if(!(tabbar->active_tab->bm_flags & TAB_NO_SUB_WIDGETS) && tabbar->active_tab->swidgets)
							{
								stack_top++;
								swidget_stack[stack_top] = cswidget;
								pos_stack[stack_top].x = x;
								pos_stack[stack_top].y = y;
											
								x += tabbar->swidget.x;
								y += tabbar->swidget.y;
								cswidget = tabbar->active_tab->swidgets;
								goto _nestled_swidgets;
							}
											
										
						}
					}
					
					
				break;
							
				case WIDGET_DROP_DOWN:
					dropdown = (wdropdown_t *)cswidget;		
					
					if(cswidget->bm_flags & WIDGET_MOUSE_OVER)
					{
						if(dropdown->option_count && dropdown->bm_flags & DROP_DOWN_DROPPED)
						{
										
							option_index = (int)((1.0 - (dropdown->swidget.relative_mouse_y * 0.5 + 0.5)) * (dropdown->option_count + 1));
							
							if(!(dropdown->bm_flags & DROP_DOWN_NO_HEADER))
							{
								option_index--;
							}
										
							if(dropdown->active_option)
							{
								dropdown->active_option->bm_flags &= ~OPTION_MOUSE_OVER;
							}
							
							//printf("option index: %d\n", option_index);
										
							if(option_index >= 0 && option_index < dropdown->option_count)
							{
											
								dropdown->options[dropdown->cur_option].bm_flags &= ~OPTION_MOUSE_OVER;
								dropdown->cur_option = option_index;
											
								if(cswidget->bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_DOWN)
								{										
									dropdown->active_option = &dropdown->options[option_index];
									if(dropdown->dropdown_callback)
									{
										dropdown->dropdown_callback(cswidget, dropdown->data, option_index);
									}
												
								}
											
								dropdown->options[option_index].bm_flags |= OPTION_MOUSE_OVER;
							}
		
						}
									
						if(cswidget->bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_DOWN)
						{
							if(dropdown->bm_flags & DROP_DOWN_DROPPED)
							{
								dropdown->bm_flags &= ~DROP_DOWN_DROPPED;
							}
							else
							{
								dropdown->bm_flags |= DROP_DOWN_DROPPED;
							}
										
							cswidget->bm_flags &= ~WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
										
						}
					}
					else
					{
						if(dropdown->option_count > 0)
						{
							if(dropdown->cur_option >= 0 && dropdown->cur_option < dropdown->option_count)
							{
								if(!dropdown->options[dropdown->cur_option].nested)
								{
									dropdown->options[dropdown->cur_option].bm_flags &= ~OPTION_MOUSE_OVER;
								}
							}
						}
						
						/* close this drop down if the click happens outside
						of it... */
						/*if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
						{
							dropdown->bm_flags &= ~DROP_DOWN_DROPPED;
							active_swidget = NULL;
						}*/
						
					}			
					
								
					if(dropdown->options[dropdown->cur_option].nested)
					{
						stack_top++;
						swidget_stack[stack_top] = cswidget;
						cswidget = dropdown->options[dropdown->cur_option].nested;
						((wdropdown_t *)cswidget)->bm_flags |= DROP_DOWN_DROPPED;
						goto _nestled_swidgets;
					}
								
				break;
				
				case WIDGET_SLIDER:
					slider = (wslider_t *)cswidget;	
					if(cswidget->bm_flags & WIDGET_LEFT_BUTTON_DOWN)
					{
						slider->pos = cswidget->relative_mouse_x * 0.5 + 0.5;
						if(slider->pos < 0.0) slider->pos = 0.0;
						else if(slider->pos > 1.0) slider->pos = 1.0;
							
						if(slider->pos + 0.0001 > slider->last_pos || slider->pos - 0.0001 < slider->last_pos)
						{
							if(slider->slider_callback)
							{
								slider->slider_callback(cswidget, slider->data, slider->pos);
							}
						}
						slider->last_pos = slider->pos;
					}
				break;
				
				case WIDGET_SLIDER_GROUP:
					sslider = (wslidergroup_t *)cswidget;
					
					if(sslider->sliders && sslider->slider_count)
					{
						if(sslider->slider_group_callback)
						{
							sslider->slider_group_callback(cswidget, NULL, 0);
						}
						
						stack_top++;
						swidget_stack[stack_top] = cswidget;
						pos_stack[stack_top].x = x;
						pos_stack[stack_top].y = y;
												
						x += sslider->swidget.x;
						y += sslider->swidget.y;
						cswidget = (swidget_t *)sslider->sliders;
						goto _nestled_swidgets;
					}
					
					
					
					
				break;
			}
			
			cswidget = cswidget->next;
			
			if(!cswidget)
			{
				if(stack_top != -1)
				{
					cswidget = swidget_stack[stack_top];
					x = pos_stack[stack_top].x;
					y = pos_stack[stack_top].y;
					stack_top--;
					cswidget = cswidget->next;
				}
			}
		}
		
		_skip:
			
		if(cwidget->bm_flags & WIDGET_DELETE)
		{
			p = cwidget->next;
			gui_DeleteWidget(cwidget);
			cwidget = p;	
		}
		else
		{
			cwidget = cwidget->next;
		}
		
			
		
	}
}


void gui_test_CloseWidget(widget_t *widget)
{
	//gui_HideWidget(widget);
	pew_Exit();
}

void gui_test_CloseConsole(widget_t *widget)
{
	console_RollUpConsole();
}

void gui_test_ToggleVolumetricLights(widget_t *widget)
{
	if(renderer.renderer_flags & RENDERFLAG_DRAW_LIGHT_VOLUMES)
	{
		console_PassParam("disable render_flag use_light_volumes");
	}
	else
	{
		console_PassParam("enable render_flag use_light_volumes");
	}
}

void gui_test_ToggleShadows(widget_t *widget)
{
	if(renderer.renderer_flags & RENDERFLAG_USE_SHADOW_MAPS)
	{
		console_PassParam("disable render_flag use_shadow_maps");
	}
	else
	{
		console_PassParam("enable render_flag use_shadow_maps");
	}
}

void gui_test_ToggleBloom(widget_t *widget)
{
	if(renderer.renderer_flags & RENDERFLAG_USE_BLOOM)
	{
		console_PassParam("disable render_flag use_bloom");
	}
	else
	{
		console_PassParam("enable render_flag use_bloom");
	}
}


#ifdef __cplusplus
}
#endif












