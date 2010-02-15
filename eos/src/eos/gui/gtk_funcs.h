#ifndef EOS_GUI_GTK_FUNCS_H
#define EOS_GUI_GTK_FUNCS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines and a whole load of people from the GTK team. (gtk.org)
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


// The licensing of this file is extremely dodgy, it was mostly constructed from the
// gtk documentation as it contains all the relevent information, (Such as the
// code with lgpl headers striped.) which itself was presumably constructed from
// the header files of the gtk code base. (The headers were also used directly.)
// This makes this file a derivative work of some non-lgpl content which
// is itself a derivative work of lgpl content. On top of that this is all through
// the headers, which makes the lgpl rules in regards to including headers apply,
// probably. I am assuming that this file is lgpl but in such a way as the rest
// of the code base is not, this is the obvious intention of the gtk team and lgpl
// writers, though its unclear if leaglly this is actually the case.
// (There is of course the other interpretation - I was provided with all the
// relevent information by the owners of the code without lgpl headers, so I
// can do whatever the hell I want with it.) In conclusion then, licenses make
// my head hurt.

/// \file gtk_funcs.h
/// Provides the gtk functions, it does dynamically load them however so gtk is
/// only linked if it is actually used. An extremely dodgy setup, on all counts.

#include "eos/types.h"
#include "eos/file/dlls.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
// Various constants/enums etc that are passed into gtk, straight copys from the
// headers mostly...
typedef enum
{
  G_CONNECT_AFTER	= 1 << 0,
  G_CONNECT_SWAPPED	= 1 << 1
} GConnectFlags;

typedef enum
{
  GTK_WINDOW_TOPLEVEL,
  GTK_WINDOW_POPUP
} GtkWindowType;

typedef enum
{
  GTK_JUSTIFY_LEFT,
  GTK_JUSTIFY_RIGHT,
  GTK_JUSTIFY_CENTER,
  GTK_JUSTIFY_FILL
} GtkJustification;

typedef enum {
	GDK_COLORSPACE_RGB
} GdkColorspace;

typedef enum
{
  GDK_RGB_DITHER_NONE,
  GDK_RGB_DITHER_NORMAL,
  GDK_RGB_DITHER_MAX
} GdkRgbDither;

typedef enum
{
  GDK_SCROLL_UP,
  GDK_SCROLL_DOWN,
  GDK_SCROLL_LEFT,
  GDK_SCROLL_RIGHT
} GdkScrollDirection;

typedef enum
{
  GTK_SHADOW_NONE,
  GTK_SHADOW_IN,
  GTK_SHADOW_OUT,
  GTK_SHADOW_ETCHED_IN,
  GTK_SHADOW_ETCHED_OUT
} GtkShadowType;

typedef enum
{
  GTK_POLICY_ALWAYS,
  GTK_POLICY_AUTOMATIC,
  GTK_POLICY_NEVER
} GtkPolicyType;

typedef enum
{
  GTK_EXPAND = 1 << 0,
  GTK_SHRINK = 1 << 1,
  GTK_FILL   = 1 << 2
} GtkAttachOptions;

typedef enum
{
  GTK_FILE_CHOOSER_ACTION_OPEN,
  GTK_FILE_CHOOSER_ACTION_SAVE,
  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
} GtkFileChooserAction;

typedef enum
{
  GTK_RESPONSE_NONE = -1,

  GTK_RESPONSE_REJECT = -2,
  GTK_RESPONSE_ACCEPT = -3,

  GTK_RESPONSE_DELETE_EVENT = -4,

  GTK_RESPONSE_OK     = -5,
  GTK_RESPONSE_CANCEL = -6,
  GTK_RESPONSE_CLOSE  = -7,
  GTK_RESPONSE_YES    = -8,
  GTK_RESPONSE_NO     = -9,
  GTK_RESPONSE_APPLY  = -10,
  GTK_RESPONSE_HELP   = -11
} GtkResponseType;

typedef enum
{
  GTK_MESSAGE_INFO,
  GTK_MESSAGE_WARNING,
  GTK_MESSAGE_QUESTION,
  GTK_MESSAGE_ERROR
} GtkMessageType;

typedef enum
{
  GTK_BUTTONS_NONE,
  GTK_BUTTONS_OK,
  GTK_BUTTONS_CLOSE,
  GTK_BUTTONS_CANCEL,
  GTK_BUTTONS_YES_NO,
  GTK_BUTTONS_OK_CANCEL
} GtkButtonsType;

typedef enum
{
  GTK_DIALOG_MODAL               = 1 << 0, /* call gtk_window_set_modal (win, TRUE) */
  GTK_DIALOG_DESTROY_WITH_PARENT = 1 << 1, /* call gtk_window_set_destroy_with_parent () */
  GTK_DIALOG_NO_SEPARATOR        = 1 << 2  /* no separator bar above buttons */
} GtkDialogFlags;

typedef enum
{
  GTK_PROGRESS_LEFT_TO_RIGHT,
  GTK_PROGRESS_RIGHT_TO_LEFT,
  GTK_PROGRESS_BOTTOM_TO_TOP,
  GTK_PROGRESS_TOP_TO_BOTTOM
} GtkProgressBarOrientation;

typedef struct {
  nat32 pixel;
  nat16 red;
  nat16 green;
  nat16 blue;
} GdkColor;

typedef struct {
  int x;
  int y;
  int width;
  int height;
} GdkRectangle;

typedef struct {
  int width;
  int height;
} GtkRequisition;

struct GtkAllocation {
  int x;
  int y;
  int width;
  int height;
};

typedef enum
{
  GTK_STATE_NORMAL,
  GTK_STATE_ACTIVE,
  GTK_STATE_PRELIGHT,
  GTK_STATE_SELECTED,
  GTK_STATE_INSENSITIVE
} GtkStateType;

//------------------------------------------------------------------------------
struct GTypeInstance
{
 void * g_class;
};

struct GObject
{
  GTypeInstance  g_type_instance;

  volatile unsigned int ref_count;
  struct GData * qdata;
};

struct GtkObject
{
  GObject parent_instance;
  nat32 flags;
};

struct GtkWidget
{
 GtkObject object;

 nat16 private_flags;
 nat8 state;
 nat8 saved_state;
 char * name;
 struct GtkStyle * style;

 GtkRequisition requisition;
 GtkAllocation allocation;

 struct GdkWindow * window;
 GtkWidget * parent;
};

//------------------------------------------------------------------------------
// Gtk types, often as something totally different as it dosn't matter to a user
// of the library or the compiler what they really contain as only gtk functions
// do anything to them.
struct GClosure {};

struct GtkProgressBar : public GtkWidget {};
struct GtkDrawingArea : public GtkWidget {};
struct GtkMisc : public GtkWidget {};
struct GtkLabel : public GtkMisc {};
struct GtkContainer : public GtkWidget {};
struct GtkEntry : public GtkWidget {};
struct GtkTextView : public GtkContainer {};
struct GtkBox : public GtkContainer {};
struct GtkHBox : public GtkBox {};
struct GtkVBox : public GtkBox {};
struct GtkTable : public GtkContainer {};
struct GtkBin : public GtkContainer {};
struct GtkExpander : public GtkBin {};
struct GtkWindow : public GtkBin {};
struct GtkDialog : public GtkWindow {};
struct GtkMessageDialog : public GtkDialog {};
struct GtkFileChooserDialog : public GtkDialog {};
struct GtkFileChooser : public GtkFileChooserDialog {};
struct GtkComboBox : public GtkBin {};
struct GtkButton : public GtkBin {};
struct GtkToggleButton : public GtkButton {};
struct GtkFrame : public GtkBin {};
struct GtkScrolledWindow : public GtkBin {};
struct GtkTextBuffer {};
struct GtkTextTagTable {};
struct GtkFileFilter {};


struct GdkEvent {};
struct GdkDrawable {};
struct GdkPixmap : public GdkDrawable {};
struct GdkWindow : public GdkDrawable {};
struct GdkGC {};
struct GdkDevice {};
struct GdkRegion {};
struct GdkCursor {};

//------------------------------------------------------------------------------
struct GtkTextIter {
  /*< private >*/
  void * dummy1;
  void * dummy2;
  int dummy3;
  int dummy4;
  int dummy5;
  int dummy6;
  int dummy7;
  int dummy8;
  void * dummy9;
  void * dummy10;
  int dummy11;
  int dummy12;
  /* padding */
  int dummy13;
  void * dummy14;
};

//------------------------------------------------------------------------------
// Gdk event structures...
typedef enum
{
  GDK_NOTHING		= -1,
  GDK_DELETE		= 0,
  GDK_DESTROY		= 1,
  GDK_EXPOSE		= 2,
  GDK_MOTION_NOTIFY	= 3,
  GDK_BUTTON_PRESS	= 4,
  GDK_2BUTTON_PRESS	= 5,
  GDK_3BUTTON_PRESS	= 6,
  GDK_BUTTON_RELEASE	= 7,
  GDK_KEY_PRESS		= 8,
  GDK_KEY_RELEASE	= 9,
  GDK_ENTER_NOTIFY	= 10,
  GDK_LEAVE_NOTIFY	= 11,
  GDK_FOCUS_CHANGE	= 12,
  GDK_CONFIGURE		= 13,
  GDK_MAP		= 14,
  GDK_UNMAP		= 15,
  GDK_PROPERTY_NOTIFY	= 16,
  GDK_SELECTION_CLEAR	= 17,
  GDK_SELECTION_REQUEST = 18,
  GDK_SELECTION_NOTIFY	= 19,
  GDK_PROXIMITY_IN	= 20,
  GDK_PROXIMITY_OUT	= 21,
  GDK_DRAG_ENTER        = 22,
  GDK_DRAG_LEAVE        = 23,
  GDK_DRAG_MOTION       = 24,
  GDK_DRAG_STATUS       = 25,
  GDK_DROP_START        = 26,
  GDK_DROP_FINISHED     = 27,
  GDK_CLIENT_EVENT	= 28,
  GDK_VISIBILITY_NOTIFY = 29,
  GDK_NO_EXPOSE		= 30,
  GDK_SCROLL            = 31,
  GDK_WINDOW_STATE      = 32,
  GDK_SETTING           = 33,
  GDK_OWNER_CHANGE      = 34,
  GDK_GRAB_BROKEN       = 35
} GdkEventType;

typedef enum
{
  GDK_EXPOSURE_MASK		= 1 << 1,
  GDK_POINTER_MOTION_MASK	= 1 << 2,
  GDK_POINTER_MOTION_HINT_MASK	= 1 << 3,
  GDK_BUTTON_MOTION_MASK	= 1 << 4,
  GDK_BUTTON1_MOTION_MASK	= 1 << 5,
  GDK_BUTTON2_MOTION_MASK	= 1 << 6,
  GDK_BUTTON3_MOTION_MASK	= 1 << 7,
  GDK_BUTTON_PRESS_MASK		= 1 << 8,
  GDK_BUTTON_RELEASE_MASK	= 1 << 9,
  GDK_KEY_PRESS_MASK		= 1 << 10,
  GDK_KEY_RELEASE_MASK		= 1 << 11,
  GDK_ENTER_NOTIFY_MASK		= 1 << 12,
  GDK_LEAVE_NOTIFY_MASK		= 1 << 13,
  GDK_FOCUS_CHANGE_MASK		= 1 << 14,
  GDK_STRUCTURE_MASK		= 1 << 15,
  GDK_PROPERTY_CHANGE_MASK	= 1 << 16,
  GDK_VISIBILITY_NOTIFY_MASK	= 1 << 17,
  GDK_PROXIMITY_IN_MASK		= 1 << 18,
  GDK_PROXIMITY_OUT_MASK	= 1 << 19,
  GDK_SUBSTRUCTURE_MASK		= 1 << 20,
  GDK_SCROLL_MASK               = 1 << 21,
  GDK_ALL_EVENTS_MASK		= 0x3FFFFE
} GdkEventMask;

typedef enum
{
  GDK_SHIFT_MASK    = 1 << 0,
  GDK_LOCK_MASK	    = 1 << 1,
  GDK_CONTROL_MASK  = 1 << 2,
  GDK_MOD1_MASK	    = 1 << 3,
  GDK_MOD2_MASK	    = 1 << 4,
  GDK_MOD3_MASK	    = 1 << 5,
  GDK_MOD4_MASK	    = 1 << 6,
  GDK_MOD5_MASK	    = 1 << 7,
  GDK_BUTTON1_MASK  = 1 << 8,
  GDK_BUTTON2_MASK  = 1 << 9,
  GDK_BUTTON3_MASK  = 1 << 10,
  GDK_BUTTON4_MASK  = 1 << 11,
  GDK_BUTTON5_MASK  = 1 << 12,
  GDK_RELEASE_MASK  = 1 << 30,
  GDK_MODIFIER_MASK = GDK_RELEASE_MASK | 0x1fff
} GdkModifierType;

typedef struct {
  GdkEventType type;
  GdkWindow * window;
  int8 send_event;
  GdkRectangle area;
  GdkRegion * region;
  int count; /* If non-zero, how many more events follow. */
} GdkEventExpose;

typedef struct {
  GdkEventType type;
  GdkWindow * window;
  int8 send_event;
  int x, y;
  int width;
  int height;
} GdkEventConfigure;

typedef struct {
  GdkEventType type;
  GdkWindow * window;
  int8 send_event;
  nat32 time;
  double x;
  double y;
  double * axes;
  unsigned int state;
  unsigned int button;
  GdkDevice * device;
  double x_root, y_root;
} GdkEventButton;

typedef struct {
  GdkEventType type;
  GdkWindow * window;
  int8 send_event;
  nat32 time;
  double x;
  double y;
  unsigned int state;
  GdkScrollDirection direction;
  GdkDevice * device;
  double x_root, y_root;
} GdkEventScroll;

typedef struct {
  GdkEventType type;
  GdkWindow * window;
  int8 send_event;
  nat32 time;
  double x;
  double y;
  double *axes;
  unsigned int state;
  int16 is_hint;
  GdkDevice * device;
  double x_root, y_root;
} GdkEventMotion;

//------------------------------------------------------------------------------
// Function pointers to all the functions in gtk...
EOS_VAR void EOS_STDCALL (*gtk_init)(int * argc,char *** argv);
EOS_VAR void EOS_STDCALL (*gtk_main)();
EOS_VAR unsigned int EOS_STDCALL (*gtk_idle_add)(int (*)(void *),void * data);
EOS_VAR int EOS_STDCALL (*gtk_events_pending)();
EOS_VAR int EOS_STDCALL (*gtk_main_iteration)();
EOS_VAR void EOS_STDCALL (*gtk_main_quit)();

EOS_VAR unsigned long EOS_STDCALL (*gtk_signal_connect_full)(void * object,const char * name,void*,void*,void*,void*,int,int);

#define	gtk_signal_connect(object,name,func,func_data) \
   gtk_signal_connect_full ((object), (name), (void*)(func), 0, (func_data), 0, 0, 0)

EOS_VAR void EOS_STDCALL (*gtk_widget_destroy)(GtkWidget * widget);
EOS_VAR void EOS_STDCALL (*gtk_widget_show)(GtkWidget * widget);
EOS_VAR void EOS_STDCALL (*gtk_widget_hide)(GtkWidget * widget);
EOS_VAR GtkWidget * EOS_STDCALL (*gtk_widget_ref)(GtkWidget * widget);
EOS_VAR void EOS_STDCALL (*gtk_widget_unref)(GtkWidget * widget);
EOS_VAR void EOS_STDCALL (*gtk_widget_add_events)(GtkWidget * widget,int events);
EOS_VAR GdkWindow * EOS_STDCALL (*gtk_widget_get_parent_window)(GtkWidget * widget);
EOS_VAR void EOS_STDCALL (*gtk_widget_queue_draw_area)(GtkWidget * widget,int x,int y,int width,int height);
EOS_VAR void EOS_STDCALL (*gtk_widget_set_size_request)(GtkWidget * widget,int width,int height);
EOS_VAR void EOS_STDCALL (*gtk_widget_modify_base)(GtkWidget * widget,GtkStateType state,const GdkColor * color);

EOS_VAR void EOS_STDCALL (*gtk_container_add)(GtkContainer * container,GtkWidget * widget);
EOS_VAR void EOS_STDCALL (*gtk_container_remove)(GtkContainer * container,GtkWidget * widget);
EOS_VAR unsigned int EOS_STDCALL (*gtk_container_get_border_width)(GtkContainer * container);
EOS_VAR void EOS_STDCALL(*gtk_container_set_border_width)(GtkContainer * container,unsigned int border_width);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_bin_get_child)(GtkBin * bin);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_window_new)(GtkWindowType type);
EOS_VAR void EOS_STDCALL (*gtk_window_get_size)(GtkWindow * window,int * width,int * height);
EOS_VAR void EOS_STDCALL (*gtk_window_resize)(GtkWindow * window,int width,int height);
EOS_VAR void EOS_STDCALL (*gtk_window_set_resizable)(GtkWindow * window,int resizable);
EOS_VAR int EOS_STDCALL (*gtk_window_get_resizable)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_iconify)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_deiconify)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_maximize)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_unmaximize)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_fullscreen)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_unfullscreen)(GtkWindow * window);
EOS_VAR void EOS_STDCALL (*gtk_window_set_title)(GtkWindow * window,const char * title);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_button_new)();

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_label_new)(const char * str);
EOS_VAR void EOS_STDCALL (*gtk_label_set_text)(GtkLabel * label,const char * str);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_drawing_area_new)();
EOS_VAR GdkPixmap * EOS_STDCALL (*gdk_pixmap_new)(GdkDrawable * drawable,int width,int height,int depth);
EOS_VAR GdkDrawable * EOS_STDCALL (*gdk_drawable_ref)(GdkDrawable * drawable);
EOS_VAR void EOS_STDCALL (*gdk_drawable_unref)(GdkDrawable * drawable);

EOS_VAR GdkGC * EOS_STDCALL (*gdk_gc_new)(GdkDrawable * drawable);
EOS_VAR GdkGC * EOS_STDCALL (*gdk_gc_ref)(GdkGC * gc);
EOS_VAR void EOS_STDCALL (*gdk_gc_unref)(GdkGC * gc);
EOS_VAR void EOS_STDCALL (*gdk_gc_set_rgb_fg_color)(GdkGC * gc,const GdkColor * color);
EOS_VAR void EOS_STDCALL (*gdk_gc_set_rgb_bg_color)(GdkGC *gc,const GdkColor * color);

EOS_VAR void EOS_STDCALL (*gdk_draw_point)(GdkDrawable * drawable,GdkGC * gc,int x,int y);
EOS_VAR void EOS_STDCALL (*gdk_draw_line)(GdkDrawable * drawable,GdkGC * gc,int x1,int y1,int x2,int y2);
EOS_VAR void EOS_STDCALL (*gdk_draw_rectangle)(GdkDrawable * drawable,GdkGC * gc,int filled,int x,int y,int width,int height);
EOS_VAR void EOS_STDCALL (*gdk_draw_drawable)(GdkDrawable * drawable,GdkGC * gc,GdkDrawable * src,int xsrc,int ysrc,int xdest,int ydest,int width,int height);
EOS_VAR void EOS_STDCALL (*gdk_draw_rgb_image)(GdkDrawable * drawable,GdkGC * gc,int x,int y,int width,int height,GdkRgbDither dith,byte * rgb_buf,int rowstride);

EOS_VAR GdkCursor * EOS_STDCALL (*gdk_cursor_new)(int cursor_type);
EOS_VAR GdkCursor * EOS_STDCALL (*gdk_cursor_ref)(GdkCursor * cursor);
EOS_VAR void EOS_STDCALL (*gdk_cursor_unref)(GdkCursor * cursor);
EOS_VAR void EOS_STDCALL (*gdk_window_set_cursor)(GdkWindow * window,GdkCursor * cursor);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_frame_new)(const char * label);
EOS_VAR void EOS_STDCALL (*gtk_frame_set_label)(GtkFrame * frame,const char * label);
EOS_VAR void EOS_STDCALL (*gtk_frame_set_label_align)(GtkFrame * frame,float xalign,float yalign);
EOS_VAR void EOS_STDCALL (*gtk_frame_set_shadow_type)(GtkFrame * frame,GtkShadowType type);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_scrolled_window_new)(void *,void *);
EOS_VAR void EOS_STDCALL (*gtk_scrolled_window_set_policy)(GtkScrolledWindow * scrolled_window,GtkPolicyType hscrollbar_policy,GtkPolicyType vscrollbar_policy);
EOS_VAR void EOS_STDCALL (*gtk_scrolled_window_add_with_viewport)(GtkScrolledWindow * scrolled_window,GtkWidget * child);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_hbox_new)(int homogeneous,int spacing);
EOS_VAR GtkWidget * EOS_STDCALL (*gtk_vbox_new)(int homogeneous,int spacing);
EOS_VAR void EOS_STDCALL (*gtk_box_pack_start)(GtkBox * box,GtkWidget * child,int expand,int fill,unsigned int padding);
EOS_VAR void EOS_STDCALL (*gtk_box_pack_end)(GtkBox * box,GtkWidget * child,int expand,int fill,unsigned int padding);
EOS_VAR int EOS_STDCALL (*gtk_box_get_spacing)(GtkBox * box);
EOS_VAR void EOS_STDCALL (*gtk_box_set_spacing)(GtkBox * box,int spacing);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_table_new)(unsigned int rows,unsigned int columns,int homogeneous);
EOS_VAR void EOS_STDCALL (*gtk_table_resize)(GtkTable * table,unsigned int rows,unsigned int columns);
EOS_VAR void EOS_STDCALL (*gtk_table_attach)(GtkTable * table,GtkWidget *child,unsigned int left_a,unsigned int right_a,unsigned int top_attach,unsigned int bottom_attach,
                                             GtkAttachOptions xoptions,GtkAttachOptions yoptions,unsigned int xpadding,unsigned int ypadding);
EOS_VAR void EOS_STDCALL (*gtk_table_set_row_spacings)(GtkTable * table,unsigned int spacing);
EOS_VAR void EOS_STDCALL (*gtk_table_set_col_spacings)(GtkTable * table,unsigned int spacing);
EOS_VAR unsigned int EOS_STDCALL (*gtk_table_get_row_spacing)(GtkTable *table,unsigned int row);
EOS_VAR unsigned int EOS_STDCALL (*gtk_table_get_col_spacing)(GtkTable *table,unsigned int column);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_file_chooser_dialog_new)(const char * title,GtkWindow * parent,GtkFileChooserAction action,const char * first_button_text,...);
EOS_VAR int EOS_STDCALL (*gtk_dialog_run)(GtkDialog * dialog);
EOS_VAR int EOS_STDCALL (*gtk_file_chooser_set_current_name)(GtkFileChooser * chooser,const char * name);
EOS_VAR void EOS_STDCALL (*gtk_file_chooser_set_filename)(GtkFileChooser * chooser,const char * name);
EOS_VAR char * EOS_STDCALL (*gtk_file_chooser_get_filename)(GtkFileChooser * chooser);
EOS_VAR void EOS_STDCALL (*gtk_file_chooser_add_filter)(GtkFileChooser * chooser,GtkFileFilter * filter);
EOS_VAR GtkFileFilter * EOS_STDCALL (*gtk_file_filter_new)();
EOS_VAR void EOS_STDCALL (*gtk_file_filter_set_name)(GtkFileFilter * filter,const char *name);
EOS_VAR void EOS_STDCALL (*gtk_file_filter_add_pattern)(GtkFileFilter * filter,const char * pattern);
EOS_VAR GtkWidget * EOS_STDCALL (*gtk_message_dialog_new)(GtkWindow *parent,GtkDialogFlags flags,GtkMessageType type,GtkButtonsType buttons,const char * message_format,...);

EOS_VAR void EOS_STDCALL (*g_free)(void * mem);

EOS_VAR void * EOS_STDCALL (*g_object_ref)(void * object);
EOS_VAR void EOS_STDCALL (*g_object_unref)(void * object);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_text_view_new_with_buffer)(GtkTextBuffer * buffer);
EOS_VAR void EOS_STDCALL (*gtk_text_view_set_editable)(GtkTextView * text_view,int setting);
EOS_VAR GtkTextBuffer * EOS_STDCALL (*gtk_text_buffer_new)(GtkTextTagTable * table);
EOS_VAR int EOS_STDCALL (*gtk_text_buffer_get_line_count)(GtkTextBuffer * buffer);
EOS_VAR void EOS_STDCALL (*gtk_text_buffer_insert)(GtkTextBuffer * buffer,GtkTextIter * iter,const char * text,int len);
EOS_VAR void EOS_STDCALL (*gtk_text_buffer_delete)(GtkTextBuffer * buffer,GtkTextIter * start,GtkTextIter * end);
EOS_VAR char * EOS_STDCALL (*gtk_text_buffer_get_text)(GtkTextBuffer * buffer,const GtkTextIter * start,const GtkTextIter * end,int include_hidden_chars);
EOS_VAR void EOS_STDCALL (*gtk_text_buffer_get_iter_at_line)(GtkTextBuffer * buffer,GtkTextIter * iter,int line_number);
EOS_VAR void EOS_STDCALL (*gtk_text_buffer_get_start_iter)(GtkTextBuffer * buffer,GtkTextIter * iter);
EOS_VAR void EOS_STDCALL (*gtk_text_buffer_get_end_iter)(GtkTextBuffer * buffer,GtkTextIter * iter);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_check_button_new)();
EOS_VAR int EOS_STDCALL (*gtk_toggle_button_get_active)(GtkToggleButton * toggle_button);
EOS_VAR void EOS_STDCALL (*gtk_toggle_button_set_active)(GtkToggleButton * toggle_button,int is_active);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_combo_box_new_text)();
EOS_VAR int EOS_STDCALL (*gtk_combo_box_get_active)(GtkComboBox * combo_box);
EOS_VAR void EOS_STDCALL (*gtk_combo_box_set_active)(GtkComboBox * combo_box,int index_);
EOS_VAR void EOS_STDCALL (*gtk_combo_box_append_text)(GtkComboBox * combo_box,const char * text);
EOS_VAR void EOS_STDCALL (*gtk_combo_box_insert_text)(GtkComboBox * combo_box,int position,const char * text);
EOS_VAR void EOS_STDCALL (*gtk_combo_box_prepend_text)(GtkComboBox * combo_box,const char * text);
EOS_VAR void EOS_STDCALL (*gtk_combo_box_remove_text)(GtkComboBox * combo_box,int position);
EOS_VAR char * EOS_STDCALL (*gtk_combo_box_get_active_text)(GtkComboBox * combo_box);

EOS_VAR GtkWidget * (*gtk_entry_new)();
EOS_VAR void EOS_STDCALL (*gtk_entry_set_text)(GtkEntry * entry,const char * text);
EOS_VAR const char * EOS_STDCALL (*gtk_entry_get_text)(GtkEntry *entry);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_progress_bar_new)();
EOS_VAR void EOS_STDCALL (*gtk_progress_bar_set_text)(GtkProgressBar * pbar,const char * text);
EOS_VAR void EOS_STDCALL (*gtk_progress_bar_set_fraction)(GtkProgressBar * pbar,double fraction);
EOS_VAR void EOS_STDCALL (*gtk_progress_bar_set_orientation)(GtkProgressBar * pbar,GtkProgressBarOrientation orientation);

EOS_VAR GtkWidget * EOS_STDCALL (*gtk_expander_new)(const char * label);
EOS_VAR void EOS_STDCALL (*gtk_expander_set_expanded)(GtkExpander * expander,int expanded);
EOS_VAR void EOS_STDCALL (*gtk_expander_set_label)(GtkExpander * expander,const char * label);

//------------------------------------------------------------------------------
// Function to interface with gtk, returns true if it loaded correctly, false if
// its all gone pear shaped...
bit LoadGtk();

//------------------------------------------------------------------------------
 };
};
#endif
