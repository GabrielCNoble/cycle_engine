#include "gui.h"
#include "input.h"
#include "gmath/vector.h"
#include "draw.h"
#include "console.h"
#include "pew.h"

//extern gelem_array gelem_a;
//extern window_class_array window_classes;
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


PEWAPI widget_t *gui_CreateWidget(char *name, int bm_flags, float x, float y, float w, float h, float r, float g, float b, float a, unsigned int tex_handle, int b_focused)
{
	widget_t *temp;
	
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
	
	temp->text_buffer = (char *)malloc(renderer.screen_width * renderer.screen_height);
	
	temp->tex_handle = tex_handle;
	
	temp->next = NULL;
	temp->prev = last;
	
	temp->sub_widgets = NULL;
	temp->last_added = NULL;
	temp->sub_widgets_count = 0;
	
	if(!widget_count)
	{
		gui_SetFocused(temp);
		//top_widget = temp;
	}
	//temp->base = temp;						/* base widget points to itself. */
	//temp->affect = NULL;
	//temp->w_widgets = NULL;
	//temp->w_last = NULL;
	//temp->widget_count = 0;

	
	
	
	/*if(widget_count == 0)
	{
		top_widget = temp;
	}
	else
	{
		if(b_focused)
		{
			
		}
		else
		{*/
	last->next = temp;
	last = temp;
	/*	}
	}*/
	
	widget_count++;
	
	return temp;
}

PEWAPI void gui_AddButton(widget_t *widget, char *name, int bm_flags, int bm_button_flags, float x, float y, float w, float h, float r, float g, float b, float a, void (*widget_callback)(swidget_t *, void *))
{
	wbutton_t *btn;
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
		
		btn->swidget.r = r;
		btn->swidget.g = g;
		btn->swidget.b = b;
		btn->swidget.a = a;
		btn->swidget.next = NULL;
		btn->swidget.type = WIDGET_BUTTON;
		
		btn->swidget.widget_callback = widget_callback;
		
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
		
		return;
	}
}

PEWAPI void gui_AddSubWidget(widget_t *base, int bm_flags, short type, char *name, float x, float y, float w, float h, float scroller_max, float scroller_min, float r, float g, float b, float a, unsigned int tex_handle, widget_t *affected_widget, void *affect_function)
{
	#if 0
	//wbase_t *btemp;
	wbutton_t *btemp;
	wvscroller_t *vstemp;
	//wwindow_t *wtemp;
	if(base)
	{
		switch(type)
		{
			
			case WIDGET_BUTTON:
				btemp = (wbutton_t *)malloc(sizeof(wbutton_t));
			
				btemp->base.type = WIDGET_BUTTON;
				btemp->base.name = strdup(name);
				//btemp->base.w_widgets = NULL;
				//btemp->base.w_last = NULL;
				//btemp->base.base = base;
				//btemp->base.affect = affected_widget;
				
				
				btemp->base.next = NULL;
				//btemp->base.prev = base->w_last;
				
				
				btemp->base.bm_flags = bm_flags & ~(WIDGET_GRABBABLE);
				btemp->base.r = r;
				btemp->base.g = g;
				btemp->base.b = b;
				btemp->base.a = a;
				
				btemp->base.x = x;
				btemp->base.y = y;
				
				btemp->base.tex_handle = tex_handle;
				
				//btemp->base.relative_x = x * base->w * 0.5;
				//btemp->base.relative_y = y * base->h * 0.5;
				
				/* scaling is relative to the widget base. A scale of 
				1.0 means this scroller goes from the top to the bottom
				of the widget. */
				btemp->base.w = w;
				btemp->base.h = h;
				
				//btemp->base.relative_w = w;
				//btemp->base.relative_h = h;
				
				btemp->affect_fn = (void(*)(wbase_t *))affect_function;
				
				/*if(!base->w_widgets)
				{
					base->w_widgets = (wbase_t *)btemp;
					base->w_last = base->w_widgets;
				}
				else
				{
					base->w_last->next = (wbase_t*) btemp;
					base->w_last = (wbase_t *) btemp;
				}
				base->widget_count++;*/
			
				return (wbase_t *)btemp;
			break;
			
			case WIDGET_VERTICAL_SCROLLER:
				vstemp = (wvscroller_t *)malloc(sizeof(wvscroller_t));
			
				vstemp->base.type = WIDGET_VERTICAL_SCROLLER;
				vstemp->base.name = strdup(name);
				//vstemp->base.w_widgets = NULL;
				//vstemp->base.w_last = NULL;
				//vstemp->base.base = base;
				//vstemp->base.affect = (wbase_t *)affected_widget;
				
				
				vstemp->base.next = NULL;
				//vstemp->base.prev = base->w_last;
				
				
				vstemp->base.bm_flags = bm_flags & ~(WIDGET_GRABBABLE);
				vstemp->base.r = r;
				vstemp->base.g = g;
				vstemp->base.b = b;
				vstemp->base.a = a;
				
				vstemp->base.x = x;
				vstemp->base.y = y;
				//vstemp->base.relative_x = x * base->w * 0.5;
				//vstemp->base.relative_y = y * base->h * 0.5;
				
				vstemp->base.w = SCROLLER_FIXED_DIMENSION;
				vstemp->base.h = h;							/* this height is relative to the top of the widget,
															  				 and scales up and down with the window */
				//vstemp->base.relative_w = SCROLLER_FIXED_DIMENSION;
				//vstemp->base.relative_h = h * base->h * 0.5;
															   
				vstemp->cur = 0.5;
				vstemp->max = scroller_max;
				vstemp->min = scroller_min;
				
				/*if(!base->w_widgets)
				{
					base->w_widgets = (wbase_t *)vstemp;
					base->w_last = base->w_widgets;
				}
				else
				{
					base->w_last->next = (wbase_t*) vstemp;
					base->w_last = (wbase_t *) vstemp;
				}
				base->widget_count++;*/
			
				return (wbase_t *)vstemp;
			break;
			
		}
	}
	#endif 
}

PEWAPI void gui_DeleteWidget(char *name)
{
	
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
				if(widget->next)
				{
					widget->next->prev = widget->prev;
					widget->prev->next = widget->next;
				}
				else 
				{
					widget->prev->next = NULL;
				}
				
				widget->prev = widgets;
				widget->next = widgets->next;
				widgets->next = widget;
				widget->next->prev = widget;
			}
			
		}
		
		top_widget = widget;
	}
}

void gui_ProcessWidgets()
{
	widget_t *cwidget;
	swidget_t *cswidget;
	wbutton_t *button;
	int mouse_over_widgets = 0;
	
	float x_scale = 2.0 / renderer.width;
	float y_scale = 2.0 / renderer.height;
	
	float rel_x;
	float rel_y;
	
	int pw;
	int ph;
	
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
	
	while(cwidget)
	{
		
		//_go:
			
		cwidget->relative_mouse_x = ((cwidget->x * x_scale - input.normalized_mouse_x) * -2.0) / (cwidget->w * x_scale);
		cwidget->relative_mouse_y = ((cwidget->y * y_scale - input.normalized_mouse_y) * -2.0) / (cwidget->h * y_scale);
		
		cw = cwidget->w / cwidget->cw;
		ch = cwidget->h / cwidget->ch;
		
		if((cwidget == top_widget || (!(top_widget->bm_flags & WIDGET_MOUSE_OVER))) && 
		        cwidget->relative_mouse_x <= 1.0 && cwidget->relative_mouse_x >= -1.0)
		{
			if(cwidget->relative_mouse_y <= 1.0 && cwidget->relative_mouse_y >= -1.0)
			{
				pw = renderer.screen_width * cwidget->w * x_scale * 0.5;
				ph = renderer.screen_height * cwidget->h * y_scale * 0.5;
				
				rel_ix = pw * (cwidget->relative_mouse_x * 0.5 + 0.5);
				rel_iy = ph * (cwidget->relative_mouse_y * 0.5 + 0.5);
				
				cwidget->bm_flags |= WIDGET_MOUSE_OVER;
				input.bm_mouse |= MOUSE_OVER_WIDGET;
				
				mouse_over_widgets++;

				if(rel_iy >= ph - WIDGET_BORDER_PIXEL_WIDTH)
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_TOP_BORDER;
				}
				else if(rel_iy <= WIDGET_BORDER_PIXEL_WIDTH)
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_BOTTOM_BORDER;
				}
				else
				{
					cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER_TOP_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER);
				}
				
				
				if(rel_ix >= pw - WIDGET_BORDER_PIXEL_WIDTH)
				{
					cwidget->bm_flags |= WIDGET_MOUSE_OVER_RIGHT_BORDER;
				}
				else if(rel_ix <= WIDGET_BORDER_PIXEL_WIDTH)
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
				
				
				if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
				{
					cwidget->bm_flags |= WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
					if(cwidget != top_widget)
					{
						gui_SetFocused(cwidget);
					}
					
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
			else
			{
				cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_MOUSE_OVER_HEADER | WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_RIGHT_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER | WIDGET_MOUSE_OVER_TOP_BORDER);
			}
		}
		else
		{
			cwidget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_MOUSE_OVER_HEADER | WIDGET_MOUSE_OVER_LEFT_BORDER | WIDGET_MOUSE_OVER_RIGHT_BORDER | WIDGET_MOUSE_OVER_BOTTOM_BORDER | WIDGET_MOUSE_OVER_TOP_BORDER);
		}
		
		if(cwidget == top_widget || (!(top_widget->bm_flags & WIDGET_MOUSE_OVER)))
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
								
		
		
		if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_HEADER)
			{
				cwidget->bm_flags |= WIDGET_GRABBED_HEADER;
			}
			else
			{
				cwidget->bm_flags &= ~WIDGET_GRABBED_HEADER;
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
			
			
			else if(cwidget->bm_flags & WIDGET_GRABBED_LEFT_BORDER)
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cwidget->bm_flags &= ~WIDGET_GRABBED_LEFT_BORDER;
				}
				else
				{
					old_d = cwidget->w;	
					cwidget->w -= input.mouse_dx / x_scale;
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
			
			
			
			else if(cwidget->bm_flags & WIDGET_GRABBED_RIGHT_BORDER)
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cwidget->bm_flags &= ~WIDGET_GRABBED_RIGHT_BORDER;
				}
				else
				{
					old_d = cwidget->w;	
					cwidget->w += input.mouse_dx / x_scale;
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
			
			
			if(cwidget->bm_flags & WIDGET_GRABBED_BOTTOM_BORDER)
			{
				if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
				{
					cwidget->bm_flags &= ~WIDGET_GRABBED_BOTTOM_BORDER;
				}
				else
				{
					old_d = cwidget->h;	
					cwidget->h -= input.mouse_dy / y_scale;
					dy = 0.0;
					if(cwidget->h < MIN_WIDGET_HEIGHT)
					{
						dy = MIN_WIDGET_HEIGHT - cwidget->h;
						dy /= old_d - cwidget->h;
						cwidget->h = MIN_WIDGET_HEIGHT;
					}
					cwidget->y += ((input.mouse_dy * (1.0 - dy)) * 0.5) / y_scale;
				}
			}
		}
		
		x = cwidget->x;
		y = cwidget->y;
		
		cswidget = cwidget->sub_widgets;
		
		while(cswidget)
		{
			
			if(cswidget->bm_flags & WIDGET_KEEP_RELATIVE_X_POSITION)
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
			}
			
			cswidget->relative_mouse_x = (((cswidget->x + x) * x_scale - input.normalized_mouse_x) * -2.0) / (cswidget->w * x_scale);
			cswidget->relative_mouse_y = (((cswidget->y + y) * y_scale - input.normalized_mouse_y) * -2.0) / (cswidget->h * y_scale);
			
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER && !(cwidget->bm_flags & (WIDGET_MOUSE_OVER_HEADER |
																			   WIDGET_MOUSE_OVER_BOTTOM_BORDER|
																			   WIDGET_MOUSE_OVER_TOP_BORDER|
																			   WIDGET_MOUSE_OVER_LEFT_BORDER|
																			   WIDGET_MOUSE_OVER_RIGHT_BORDER)))
			{
				if(cswidget->relative_mouse_x <= 1.0 && cswidget->relative_mouse_x >= -1.0)
				{
					if(cswidget->relative_mouse_y <= 1.0 && cswidget->relative_mouse_y >= -1.0)
					{
						cswidget->bm_flags |= WIDGET_MOUSE_OVER;
						
						if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
						{
							cswidget->bm_flags |= WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
						}
						else
						{
							cswidget->bm_flags &= ~WIDGET_RECEIVED_LEFT_BUTTON_DOWN;
						}
						
						if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_RELEASED)
						{
							cswidget->bm_flags |= WIDGET_RECEIVED_LEFT_BUTTON_UP;
						}
						else
						{
							cswidget->bm_flags &= ~WIDGET_RECEIVED_LEFT_BUTTON_UP;
						}
					}
					else
					{
						cswidget->bm_flags &= ~WIDGET_MOUSE_OVER;
					}
					
					
					//if(cswidget->bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_DOWN)
					//{
					switch(cswidget->type)
					{
						case WIDGET_BUTTON:
							button = (wbutton_t *)cswidget;
								
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
									cswidget->widget_callback(cswidget, NULL);
								}
								
							}
							else if(cswidget->bm_flags & WIDGET_RECEIVED_LEFT_BUTTON_UP)
							{
								if(button->button_flags & BUTTON_CHECK_BOX)
								{
										
								}
								else
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
					}
					
					//if(cswidget->bm_flags & WIDGET_MOUSE_OVER)
					//{
					//	if(cswidget->widget_callback)
					//	{
					//		cswidget->widget_callback(cswidget, NULL);
					//	}
				//	}	
					
						
					//}
					
					
					
				}
				else
				{
					cswidget->bm_flags &= ~WIDGET_MOUSE_OVER;
				}
				
				
				
				
				
			}
			else
			{
				cswidget->bm_flags &= ~WIDGET_MOUSE_OVER;
			}
			
			
			
			cswidget = cswidget->next;
		}
		
		cwidget = cwidget->next;	
		
	}
}

PEWAPI void gui_PrintOnWidget(widget_t *widget, char *str)
{
	char *p = widget->text_buffer;
	char *q = str;
	
	while(*q)
	{
		*p++ = *q++;
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
/*
=============
gui_ResizeWindowArray
=============
*/
/*PEWAPI void gui_ResizeGelemArray(int new_size)
{
	gelem_t *temp=(gelem_t *)calloc(new_size, sizeof(gelem_t));
	if(gelem_a.gelems)
	{
		memcpy(temp, gelem_a.gelems, sizeof(gelem_t)*gelem_a.gelem_count);
		free(gelem_a.gelems);
	}
	gelem_a.gelems=temp;
	gelem_a.gelem_count=new_size;
}*/


/*
=============
gui_ResizeWindowClassArray
=============
*/
/*PEWAPI void gui_ResizeWindowClassArray(int new_size)
{
	/*window_class *temp=calloc(new_size, sizeof(window_class));
	if(window_classes.classes)
	{
		memcpy(temp, window_classes.classes, sizeof(window_class)*window_classes.class_count);
		free(window_classes.classes);
	}
	window_classes.classes=temp;
	window_classes.array_size=new_size;
}*/


/*
=============
gui_RegisterWindowClass
=============
*/
/*PEWAPI void gui_RegisterWindowClass(window_class *win_class)
{
	if(win_class)
	{
		if(window_classes.class_count>=window_classes.array_size)
		{
			gui_ResizeWindowClassArray(window_classes.array_size+10);
		}
		window_classes.classes[window_classes.class_count++]=*win_class;
		return;
	}
}*/


/*
=============
gui_GetWindowClassIndex
=============
*/
/*PEWAPI int gui_GetWindowClassIndex(char *name)
{
	register int i;
	register int c;
	c=window_classes.class_count;
	
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, window_classes.classes[i].class_name))
		{
			return i;
		}
	}
	return -1;
}*/


/*PEWAPI gelem_t *gui_GetGelem(char *name)
{
	register int i;
	register int c;
	c=gelem_a.gelem_count;
	
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, gelem_a.gelems[i].name))
		{
			return &gelem_a.gelems[i];
		}
	}
	return NULL;
}*/



/*
=============
gui_CreateWindow
=============
*/
/*PEWAPI int gui_CreateGelem(gelem_t *gelem)
{
	if(gelem)
	{
		if(gelem_a.gelem_count>=gelem_a.array_size)
		{
			gui_ResizeGelemArray(gelem_a.gelem_count+10);
		}
		gelem->bm_status=0;
		gelem_a.gelems[gelem_a.gelem_count]=*gelem;
		if(gelem_a.gelems[gelem_a.gelem_count].gelem_strt_fn) gelem_a.gelems[gelem_a.gelem_count].gelem_strt_fn(&gelem_a.gelems[gelem_a.gelem_count]);
		gelem_a.gelem_count++;
		return gelem_a.gelem_count-1;
	}
	return-1;
}



PEWAPI void gui_ProcessGelems()
{
	register int i;
	register int c;
	int j;
	int k;
	float half_width;
	float half_height;
	vec2_t rpos;
	c=gelem_a.gelem_count;
	//var_t *var;
	for(i=0; i<c; i++)
	{
		rpos=gui_GetRelativeMouse(&gelem_a.gelems[i]);	
		if(rpos.floats[0]>-1.0 && rpos.floats[0]<1.0)
		{
			if(rpos.floats[1]>-1.0 && rpos.floats[1]<1.0)
			{
				gelem_a.gelems[i].bm_status|=WINDOW_MOUSE_OVER;
				
				if(gelem_a.gelems[i].bm_flags&WINDOW_HAS_HEADER)
				{
					if(rpos.floats[1]>0.9)gelem_a.gelems[i].bm_status|=WINDOW_MOUSE_OVER_HEADER;
					else gelem_a.gelems[i].bm_status&= ~WINDOW_MOUSE_OVER_HEADER;
				}
				
			}else gelem_a.gelems[i].bm_status&= ~WINDOW_MOUSE_OVER;
			
		}else gelem_a.gelems[i].bm_status&= ~WINDOW_MOUSE_OVER;
		
		gelem_a.gelems[i].rmouse_x=rpos.floats[0];
		gelem_a.gelems[i].rmouse_y=rpos.floats[1];
		
		if(gelem_a.gelems[i].gelem_proc_fn) gelem_a.gelems[i].gelem_proc_fn(&gelem_a.gelems[i]);
	}
}*/

/*PEWAPI var_t *gui_CreateWindowVar(window_t *window, int var_type, char *var_name, var_t_value initial_value)
{
	if(window)
	{
		if(window->var_count>=window->max_vars)
		{
			gui_ExpandWindowVars(window, window->max_vars+10);
		}
		window->vars[window->var_count].name=var_name;
		window->vars[window->var_count].type=var_type;
		window->vars[window->var_count].value=initial_value;
		window->var_count++;
	}
}*/


/*PEWAPI void gui_ExpandWindowVars(window_t *window, int new_count)
{
	var_t *temp;
	if(window)
	{
		temp=calloc(new_count, sizeof(var_t));
		if(window->vars)
		{
			memcpy(temp, window->vars, sizeof(window->vars)*window->max_vars);
			free(window->vars);
		}
		window->max_vars=new_count;
		window->vars=temp;
	}
}*/


/*PEWAPI var_t *gui_GetWindowVar(window_t *window, char *name)
{
	register int i;
	register int c;
	c=window->var_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, window->vars[i].name))
		{
			return &window->vars[i];
		}
	}
	return NULL;
}*/


/*PEWAPI vec2_t gui_GetRelativeMouse(gelem_t *gelem)
{
	vec2_t w_mouse_pos;
	vec2_t s_mouse_pos=vec2(input.normalized_mouse_x, input.normalized_mouse_y);
	vec2_t w_center=vec2(gelem->x, gelem->y);
	w_mouse_pos=sub2(w_center, s_mouse_pos);
	
	w_mouse_pos.floats[0]*= -2.0/gelem->width;
	w_mouse_pos.floats[1]*= -2.0/gelem->height;
	
	return w_mouse_pos;
}


PEWAPI vec2_t gui_GetAbsoluteMouse(gelem_t *gelem)
{
	vec2_t s_mouse_pos;
	s_mouse_pos.floats[0]=gelem->x + gelem->rmouse_x*(gelem->width/2.0); 
	s_mouse_pos.floats[1]=gelem->y + gelem->rmouse_y*(gelem->height/2.0);
	return s_mouse_pos;
}


PEWAPI vec2_t gui_GetRelativePosition(gelem_t *gelem, vec2_t absolute_pos)
{
	vec2_t w_mouse_pos;
	vec2_t w_center=vec2(gelem->x, gelem->y);
	w_mouse_pos=sub2(w_center, absolute_pos);
	
	w_mouse_pos.floats[0]*= -2.0/gelem->width;
	w_mouse_pos.floats[1]*= -2.0/gelem->height;
	
	return w_mouse_pos;
}


PEWAPI vec2_t gui_GetAbsolutePosition(gelem_t *gelem, vec2_t relative_pos)
{
	vec2_t s_mouse_pos;
	s_mouse_pos.floats[0]=gelem->x + relative_pos.floats[0]*(gelem->width/2.0); 
	s_mouse_pos.floats[1]=gelem->y + relative_pos.floats[1]*(gelem->height/2.0);
	return s_mouse_pos;
}


PEWAPI void gui_SetWindowVisible(gelem_t *gelem)
{
	gelem->bm_status|=WINDOW_VISIBLE;
}


PEWAPI void gui_SetWindowInvisible(gelem_t *gelem)
{
	gelem->bm_status&= ~WINDOW_VISIBLE;
}*/


/*PEWAPI void gui_DrawChar(char ch, gelem_t *gelem, vec2_t pos, vec3_t color)
{
	vec2_t v=gui_GetAbsolutePosition(gelem, pos);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glUseProgram(0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glColor3f(color.floats[0], color.floats[1], color.floats[2]);
	glRasterPos3f(v.floats[0], v.floats[1], -0.11);
	glBitmap(12, 20, 6, 10, 0, 0, (unsigned char *)font.chars[ch-32].start);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}*/


/*PEWAPI var_t_value f_value(float f)
{
	var_t_value r;
	r.f_val=f;
	return r;
}*/

/*PEWAPI var_t_value i_value(int i)
{
	var_t_value r;
	r.i_val=i;
	return r;
}*/


/*void winfn(window_t *window)
{
	var_t *timer=gui_GetWindowVar(window, "my_int");
	vec2_t mpos;
	timer->value.i_val++;
	
	if(window->bm_status&WINDOW_MOUSE_OVER)
	{
		window->alpha=fabs(window->rmouse_x);
	}
	
	if(timer->value.i_val>60)
	{
		if(window->alpha<1.0) window->alpha=1.0;
		else window->alpha=0.2;
		timer->value.i_val=0;
	}
	
	
}*/


#ifdef __cplusplus
}
#endif












