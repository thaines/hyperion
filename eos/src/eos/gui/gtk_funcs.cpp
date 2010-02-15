//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
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

#include "eos/gui/gtk_funcs.h"

#include "eos/file/dlls.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
// Where the fuction pointers memory is allocated...
EOS_VAR_DEF void EOS_STDCALL (*gtk_init)(int * argc,char *** argv);
EOS_VAR_DEF void EOS_STDCALL (*gtk_main)();
EOS_VAR_DEF unsigned int EOS_STDCALL (*gtk_idle_add)(int (*)(void *),void * data);
EOS_VAR_DEF int EOS_STDCALL (*gtk_events_pending)();
EOS_VAR_DEF int EOS_STDCALL (*gtk_main_iteration)();
EOS_VAR_DEF void EOS_STDCALL (*gtk_main_quit)();

EOS_VAR_DEF unsigned long EOS_STDCALL (*gtk_signal_connect_full)(void * object,const char * name,void*,void*,void*,void*,int,int);

EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_destroy)(GtkWidget * widget);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_show)(GtkWidget * widget);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_hide)(GtkWidget * widget);
EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_widget_ref)(GtkWidget * widget);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_unref)(GtkWidget * widget);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_add_events)(GtkWidget * widget,int events);
EOS_VAR_DEF GdkWindow * EOS_STDCALL (*gtk_widget_get_parent_window)(GtkWidget * widget);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_queue_draw_area)(GtkWidget * widget,int x,int y,int width,int height);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_set_size_request)(GtkWidget * widget,int width,int height);
EOS_VAR_DEF void EOS_STDCALL (*gtk_widget_modify_base)(GtkWidget * widget,GtkStateType state,const GdkColor * color);

EOS_VAR_DEF void EOS_STDCALL (*gtk_container_add)(GtkContainer * container,GtkWidget * widget);
EOS_VAR_DEF void EOS_STDCALL (*gtk_container_remove)(GtkContainer * container,GtkWidget * widget);
EOS_VAR_DEF unsigned int EOS_STDCALL (*gtk_container_get_border_width)(GtkContainer * container);
EOS_VAR_DEF void EOS_STDCALL(*gtk_container_set_border_width)(GtkContainer * container,unsigned int border_width);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_bin_get_child)(GtkBin * bin);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_window_new)(GtkWindowType type);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_get_size)(GtkWindow * window,int * width,int * height);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_resize)(GtkWindow * window,int width,int height);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_set_resizable)(GtkWindow * window,int resizable);
EOS_VAR_DEF int EOS_STDCALL (*gtk_window_get_resizable)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_iconify)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_deiconify)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_maximize)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_unmaximize)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_fullscreen)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_unfullscreen)(GtkWindow * window);
EOS_VAR_DEF void EOS_STDCALL (*gtk_window_set_title)(GtkWindow * window,const char * title);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_button_new)();

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_label_new)(const char * str);
EOS_VAR_DEF void EOS_STDCALL (*gtk_label_set_text)(GtkLabel * label,const char * str);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_drawing_area_new)();

EOS_VAR_DEF GdkPixmap * EOS_STDCALL (*gdk_pixmap_new)(GdkDrawable * drawable,int width,int height,int depth);
EOS_VAR_DEF GdkDrawable * EOS_STDCALL (*gdk_drawable_ref)(GdkDrawable * drawable);
EOS_VAR_DEF void EOS_STDCALL (*gdk_drawable_unref)(GdkDrawable * drawable);

EOS_VAR_DEF GdkGC * EOS_STDCALL (*gdk_gc_new)(GdkDrawable * drawable);
EOS_VAR_DEF GdkGC * EOS_STDCALL (*gdk_gc_ref)(GdkGC * gc);
EOS_VAR_DEF void EOS_STDCALL (*gdk_gc_unref)(GdkGC * gc);
EOS_VAR_DEF void EOS_STDCALL (*gdk_gc_set_rgb_fg_color)(GdkGC * gc,const GdkColor * color);
EOS_VAR_DEF void EOS_STDCALL (*gdk_gc_set_rgb_bg_color)(GdkGC *gc,const GdkColor * color);

EOS_VAR_DEF void EOS_STDCALL (*gdk_draw_point)(GdkDrawable * drawable,GdkGC * gc,int x,int y);
EOS_VAR_DEF void EOS_STDCALL (*gdk_draw_line)(GdkDrawable * drawable,GdkGC * gc,int x1,int y1,int x2,int y2);
EOS_VAR_DEF void EOS_STDCALL (*gdk_draw_rectangle)(GdkDrawable * drawable,GdkGC * gc,int filled,int x,int y,int width,int height);
EOS_VAR_DEF void EOS_STDCALL (*gdk_draw_drawable)(GdkDrawable * drawable,GdkGC * gc,GdkDrawable * src,int xsrc,int ysrc,int xdest,int ydest,int width,int height);
EOS_VAR_DEF void EOS_STDCALL (*gdk_draw_rgb_image)(GdkDrawable * drawable,GdkGC * gc,int x,int y,int width,int height,GdkRgbDither dith,byte * rgb_buf,int rowstride);

EOS_VAR_DEF GdkCursor * EOS_STDCALL (*gdk_cursor_new)(int cursor_type);
EOS_VAR_DEF GdkCursor * EOS_STDCALL (*gdk_cursor_ref)(GdkCursor * cursor);
EOS_VAR_DEF void EOS_STDCALL (*gdk_cursor_unref)(GdkCursor * cursor);
EOS_VAR_DEF void EOS_STDCALL (*gdk_window_set_cursor)(GdkWindow * window,GdkCursor * cursor);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_frame_new)(const char * label);
EOS_VAR_DEF void EOS_STDCALL (*gtk_frame_set_label)(GtkFrame * frame,const char * label);
EOS_VAR_DEF void EOS_STDCALL (*gtk_frame_set_label_align)(GtkFrame * frame,float xalign,float yalign);
EOS_VAR_DEF void EOS_STDCALL (*gtk_frame_set_shadow_type)(GtkFrame * frame,GtkShadowType type);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_scrolled_window_new)(void*,void*);
EOS_VAR_DEF void EOS_STDCALL (*gtk_scrolled_window_set_policy)(GtkScrolledWindow *scrolled_window,GtkPolicyType hscrollbar_policy,GtkPolicyType vscrollbar_policy);
EOS_VAR_DEF void EOS_STDCALL (*gtk_scrolled_window_add_with_viewport)(GtkScrolledWindow * scrolled_window,GtkWidget * child);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_hbox_new)(int homogeneous,int spacing);
EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_vbox_new)(int homogeneous,int spacing);
EOS_VAR_DEF void EOS_STDCALL (*gtk_box_pack_start)(GtkBox * box,GtkWidget * child,int expand,int fill,unsigned int padding);
EOS_VAR_DEF void EOS_STDCALL (*gtk_box_pack_end)(GtkBox * box,GtkWidget * child,int expand,int fill,unsigned int padding);
EOS_VAR_DEF int EOS_STDCALL (*gtk_box_get_spacing)(GtkBox * box);
EOS_VAR_DEF void EOS_STDCALL (*gtk_box_set_spacing)(GtkBox * box,int spacing);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_table_new)(unsigned int rows,unsigned int columns,int homogeneous);
EOS_VAR_DEF void EOS_STDCALL (*gtk_table_resize)(GtkTable * table,unsigned int rows,unsigned int columns);
EOS_VAR_DEF void EOS_STDCALL (*gtk_table_attach)(GtkTable * table,GtkWidget *child,unsigned int left_a,unsigned int right_a,unsigned int top_attach,unsigned int bottom_attach,
                                                 GtkAttachOptions xoptions,GtkAttachOptions yoptions,unsigned int xpadding,unsigned int ypadding);
EOS_VAR_DEF void EOS_STDCALL (*gtk_table_set_row_spacings)(GtkTable * table,unsigned int spacing);
EOS_VAR_DEF void EOS_STDCALL (*gtk_table_set_col_spacings)(GtkTable * table,unsigned int spacing);
EOS_VAR_DEF unsigned int EOS_STDCALL (*gtk_table_get_row_spacing)(GtkTable *table,unsigned int row);
EOS_VAR_DEF unsigned int EOS_STDCALL (*gtk_table_get_col_spacing)(GtkTable *table,unsigned int column);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_file_chooser_dialog_new)(const char * title,GtkWindow * parent,GtkFileChooserAction action,const char * first_button_text,...);
EOS_VAR_DEF int EOS_STDCALL (*gtk_dialog_run)(GtkDialog * dialog);
EOS_VAR_DEF int EOS_STDCALL (*gtk_file_chooser_set_current_name)(GtkFileChooser * chooser,const char * name);
EOS_VAR_DEF int EOS_STDCALL (*gtk_file_chooser_set_filename)(GtkFileChooser * chooser,const char *name);
EOS_VAR_DEF char * EOS_STDCALL (*gtk_file_chooser_get_filename)(GtkFileChooser * chooser);
EOS_VAR_DEF int EOS_STDCALL (*gtk_file_chooser_set_current_folder)(GtkFileChooser * chooser,const char * name);
EOS_VAR_DEF char * EOS_STDCALL (*gtk_file_chooser_get_current_folder)(GtkFileChooser * chooser);
EOS_VAR_DEF void EOS_STDCALL (*gtk_file_chooser_add_filter)(GtkFileChooser * chooser,GtkFileFilter * filter);
EOS_VAR_DEF GtkFileFilter * EOS_STDCALL (*gtk_file_filter_new)();
EOS_VAR_DEF void EOS_STDCALL (*gtk_file_filter_set_name)(GtkFileFilter * filter,const char *name);
EOS_VAR_DEF void EOS_STDCALL (*gtk_file_filter_add_pattern)(GtkFileFilter * filter,const char * pattern);
EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_message_dialog_new)(GtkWindow *parent,GtkDialogFlags flags,GtkMessageType type,GtkButtonsType buttons,const char * message_format,...);

EOS_VAR_DEF void EOS_STDCALL (*g_free)(void * mem);
EOS_VAR_DEF void * EOS_STDCALL (*g_object_ref)(void * object);
EOS_VAR_DEF void EOS_STDCALL (*g_object_unref)(void * object);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_text_view_new_with_buffer)(GtkTextBuffer * buffer);
EOS_VAR_DEF void EOS_STDCALL (*gtk_text_view_set_editable)(GtkTextView * text_view,int setting);
EOS_VAR_DEF GtkTextBuffer * EOS_STDCALL (*gtk_text_buffer_new)(GtkTextTagTable * table);
EOS_VAR_DEF int EOS_STDCALL (*gtk_text_buffer_get_line_count)(GtkTextBuffer * buffer);
EOS_VAR_DEF void EOS_STDCALL (*gtk_text_buffer_insert)(GtkTextBuffer * buffer,GtkTextIter * iter,const char * text,int len);
EOS_VAR_DEF void EOS_STDCALL (*gtk_text_buffer_delete)(GtkTextBuffer * buffer,GtkTextIter * start,GtkTextIter * end);
EOS_VAR_DEF char * EOS_STDCALL (*gtk_text_buffer_get_text)(GtkTextBuffer * buffer,const GtkTextIter * start,const GtkTextIter * end,int include_hidden_chars);
EOS_VAR_DEF void EOS_STDCALL (*gtk_text_buffer_get_iter_at_line)(GtkTextBuffer * buffer,GtkTextIter * iter,int line_number);
EOS_VAR_DEF void EOS_STDCALL (*gtk_text_buffer_get_start_iter)(GtkTextBuffer * buffer,GtkTextIter * iter);
EOS_VAR_DEF void EOS_STDCALL (*gtk_text_buffer_get_end_iter)(GtkTextBuffer * buffer,GtkTextIter * iter);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_check_button_new)();
EOS_VAR_DEF int EOS_STDCALL (*gtk_toggle_button_get_active)(GtkToggleButton * toggle_button);
EOS_VAR_DEF void EOS_STDCALL (*gtk_toggle_button_set_active)(GtkToggleButton * toggle_button,int is_active);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_combo_box_new_text)();
EOS_VAR_DEF int EOS_STDCALL (*gtk_combo_box_get_active)(GtkComboBox * combo_box);
EOS_VAR_DEF void EOS_STDCALL (*gtk_combo_box_set_active)(GtkComboBox * combo_box,int index_);
EOS_VAR_DEF void EOS_STDCALL (*gtk_combo_box_append_text)(GtkComboBox * combo_box,const char * text);
EOS_VAR_DEF void EOS_STDCALL (*gtk_combo_box_insert_text)(GtkComboBox * combo_box,int position,const char * text);
EOS_VAR_DEF void EOS_STDCALL (*gtk_combo_box_prepend_text)(GtkComboBox * combo_box,const char * text);
EOS_VAR_DEF void EOS_STDCALL (*gtk_combo_box_remove_text)(GtkComboBox * combo_box,int position);
EOS_VAR_DEF char * EOS_STDCALL (*gtk_combo_box_get_active_text)(GtkComboBox * combo_box);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_entry_new)();
EOS_VAR_DEF void EOS_STDCALL (*gtk_entry_set_text)(GtkEntry * entry,const char * text);
EOS_VAR_DEF const char * EOS_STDCALL (*gtk_entry_get_text)(GtkEntry *entry);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_progress_bar_new)();
EOS_VAR_DEF void EOS_STDCALL (*gtk_progress_bar_set_text)(GtkProgressBar * pbar,const char * text);
EOS_VAR_DEF void EOS_STDCALL (*gtk_progress_bar_set_fraction)(GtkProgressBar * pbar,double fraction);
EOS_VAR_DEF void EOS_STDCALL (*gtk_progress_bar_set_orientation)(GtkProgressBar * pbar,GtkProgressBarOrientation orientation);

EOS_VAR_DEF GtkWidget * EOS_STDCALL (*gtk_expander_new)(const char * label);
EOS_VAR_DEF void EOS_STDCALL (*gtk_expander_set_expanded)(GtkExpander * expander,int expanded);
EOS_VAR_DEF void EOS_STDCALL (*gtk_expander_set_label)(GtkExpander * expander,const char * label);

//------------------------------------------------------------------------------
#define LoadGtkFunc(f,s) ((void*&)f) = (void*)gtk.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Gtk function {func}" << LogDiv() << s); gtk_init = 0; return false;}

#ifdef EOS_LINUX
#define LoadGdkFunc(f,s) ((void*&)f) = gtk.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Gdk function {func}" << LogDiv() << s); gtk_init = 0; return false;}
#define LoadGlibFunc(f,s) ((void*&)f) = gtk.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Glib function {func}" << LogDiv() << s); gtk_init = 0; return false;}
#define LoadGobjFunc(f,s) ((void*&)f) = gtk.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Gobj function {func}" << LogDiv() << s); gtk_init = 0; return false;}
#else
#define LoadGdkFunc(f,s) ((void*&)f) = gdk.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Gdk function {func}" << LogDiv() << s); gtk_init = 0; return false;}
#define LoadGlibFunc(f,s) ((void*&)f) = glib.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Glib function {func}" << LogDiv() << s); gtk_init = 0; return false;}
#define LoadGobjFunc(f,s) ((void*&)f) = gobj.Get(s); if (f==0) {LogAlways("[gui.gtk] Error loading Gobj function {func}" << LogDiv() << s); gtk_init = 0; return false;}
#endif

bit LoadGtk()
{
 if (gtk_init) return true;

 // First load the dll(s)...
  #ifdef EOS_LINUX
   file::Dll gtk;
   gtk.Load("gtk-x11-2.0");
   if (!gtk.Active()) {LogAlways("[gui.gtk] Could not load gtk-x11-2.0"); return false;}
  #else
   file::Dll gtk;
   gtk.Load("libgtk-win32-2.0-0.dll");
   if (!gtk.Active()) {LogAlways("[gui.gtk] Could not load libgtk-win32-2.0-0"); return false;}

   file::Dll gdk;
   gdk.Load("libgdk-win32-2.0-0.dll");
   if (!gdk.Active()) {LogAlways("[gui.gtk] Could not load libgdk-win32-2.0-0"); return false;}

   file::Dll glib;
   glib.Load("libglib-2.0-0.dll");
   if (!glib.Active()) {LogAlways("[gui.gtk] Could not load libglib-2.0-0"); return false;}

   file::Dll gobj;
   gobj.Load("libgobject-2.0-0.dll");
   if (!gobj.Active()) return false;
  #endif


 // Now load each and every function...
  LoadGtkFunc(gtk_init,"gtk_init");
  LoadGtkFunc(gtk_main,"gtk_main");
  LoadGtkFunc(gtk_events_pending,"gtk_events_pending");
  LoadGtkFunc(gtk_main_iteration,"gtk_main_iteration");
  LoadGtkFunc(gtk_idle_add,"gtk_idle_add");
  LoadGtkFunc(gtk_main_quit,"gtk_main_quit");

  LoadGtkFunc(gtk_signal_connect_full,"gtk_signal_connect_full");

  LoadGtkFunc(gtk_widget_destroy,"gtk_widget_destroy");
  LoadGtkFunc(gtk_widget_show,"gtk_widget_show");
  LoadGtkFunc(gtk_widget_hide,"gtk_widget_hide");
  LoadGtkFunc(gtk_widget_ref,"gtk_widget_ref");
  LoadGtkFunc(gtk_widget_unref,"gtk_widget_unref");
  LoadGtkFunc(gtk_widget_add_events,"gtk_widget_add_events");
  LoadGtkFunc(gtk_widget_get_parent_window,"gtk_widget_get_parent_window");
  LoadGtkFunc(gtk_widget_queue_draw_area,"gtk_widget_queue_draw_area");
  LoadGtkFunc(gtk_widget_set_size_request,"gtk_widget_set_size_request");
  LoadGtkFunc(gtk_widget_modify_base,"gtk_widget_modify_base");

  LoadGtkFunc(gtk_container_add,"gtk_container_add");
  LoadGtkFunc(gtk_container_remove,"gtk_container_remove");
  LoadGtkFunc(gtk_container_get_border_width,"gtk_container_get_border_width");
  LoadGtkFunc(gtk_container_set_border_width,"gtk_container_set_border_width");

  LoadGtkFunc(gtk_bin_get_child,"gtk_bin_get_child");

  LoadGtkFunc(gtk_window_new,"gtk_window_new");
  LoadGtkFunc(gtk_window_get_size,"gtk_window_get_size");
  LoadGtkFunc(gtk_window_resize,"gtk_window_resize");
  LoadGtkFunc(gtk_window_set_resizable,"gtk_window_set_resizable");
  LoadGtkFunc(gtk_window_get_resizable,"gtk_window_get_resizable");
  LoadGtkFunc(gtk_window_iconify,"gtk_window_iconify");
  LoadGtkFunc(gtk_window_deiconify,"gtk_window_deiconify");
  LoadGtkFunc(gtk_window_maximize,"gtk_window_maximize");
  LoadGtkFunc(gtk_window_unmaximize,"gtk_window_unmaximize");
  LoadGtkFunc(gtk_window_fullscreen,"gtk_window_fullscreen");
  LoadGtkFunc(gtk_window_unfullscreen,"gtk_window_unfullscreen");
  LoadGtkFunc(gtk_window_set_title,"gtk_window_set_title");

  LoadGtkFunc(gtk_button_new,"gtk_button_new");

  LoadGtkFunc(gtk_label_new,"gtk_label_new");
  LoadGtkFunc(gtk_label_set_text,"gtk_label_set_text");

  LoadGtkFunc(gtk_drawing_area_new,"gtk_drawing_area_new");

  LoadGdkFunc(gdk_pixmap_new,"gdk_pixmap_new");
  LoadGdkFunc(gdk_drawable_ref,"gdk_drawable_ref");
  LoadGdkFunc(gdk_drawable_unref,"gdk_drawable_unref");

  LoadGdkFunc(gdk_gc_new,"gdk_gc_new");
  LoadGdkFunc(gdk_gc_ref,"gdk_gc_ref");
  LoadGdkFunc(gdk_gc_unref,"gdk_gc_unref");
  LoadGdkFunc(gdk_gc_set_rgb_fg_color,"gdk_gc_set_rgb_fg_color");
  LoadGdkFunc(gdk_gc_set_rgb_bg_color,"gdk_gc_set_rgb_bg_color");

  LoadGdkFunc(gdk_draw_point,"gdk_draw_point");
  LoadGdkFunc(gdk_draw_line,"gdk_draw_line");
  LoadGdkFunc(gdk_draw_rectangle,"gdk_draw_rectangle");
  LoadGdkFunc(gdk_draw_drawable,"gdk_draw_drawable");
  LoadGdkFunc(gdk_draw_rgb_image,"gdk_draw_rgb_image");

  LoadGdkFunc(gdk_cursor_new,"gdk_cursor_new");
  LoadGdkFunc(gdk_cursor_ref,"gdk_cursor_ref");
  LoadGdkFunc(gdk_cursor_unref,"gdk_cursor_unref");
  LoadGdkFunc(gdk_window_set_cursor,"gdk_window_set_cursor");

  LoadGtkFunc(gtk_frame_new,"gtk_frame_new");
  LoadGtkFunc(gtk_frame_set_label,"gtk_frame_set_label");
  LoadGtkFunc(gtk_frame_set_label_align,"gtk_frame_set_label_align");
  LoadGtkFunc(gtk_frame_set_shadow_type,"gtk_frame_set_shadow_type");

  LoadGtkFunc(gtk_scrolled_window_new,"gtk_scrolled_window_new");
  LoadGtkFunc(gtk_scrolled_window_set_policy,"gtk_scrolled_window_set_policy");
  LoadGtkFunc(gtk_scrolled_window_add_with_viewport,"gtk_scrolled_window_add_with_viewport");

  LoadGtkFunc(gtk_hbox_new,"gtk_hbox_new");
  LoadGtkFunc(gtk_vbox_new,"gtk_vbox_new");
  LoadGtkFunc(gtk_box_pack_start,"gtk_box_pack_start");
  LoadGtkFunc(gtk_box_pack_end,"gtk_box_pack_end");
  LoadGtkFunc(gtk_box_get_spacing,"gtk_box_get_spacing");
  LoadGtkFunc(gtk_box_set_spacing,"gtk_box_set_spacing");

  LoadGtkFunc(gtk_table_new,"gtk_table_new");
  LoadGtkFunc(gtk_table_resize,"gtk_table_resize");
  LoadGtkFunc(gtk_table_attach,"gtk_table_attach");
  LoadGtkFunc(gtk_table_set_row_spacings,"gtk_table_set_row_spacings");
  LoadGtkFunc(gtk_table_set_col_spacings,"gtk_table_set_col_spacings");
  LoadGtkFunc(gtk_table_get_row_spacing,"gtk_table_get_row_spacing");
  LoadGtkFunc(gtk_table_get_col_spacing,"gtk_table_get_col_spacing");

  LoadGtkFunc(gtk_file_chooser_dialog_new,"gtk_file_chooser_dialog_new");
  LoadGtkFunc(gtk_dialog_run,"gtk_dialog_run");
  LoadGtkFunc(gtk_file_chooser_set_current_name,"gtk_file_chooser_set_current_name");
  LoadGtkFunc(gtk_file_chooser_set_filename,"gtk_file_chooser_set_filename");
  LoadGtkFunc(gtk_file_chooser_get_filename,"gtk_file_chooser_get_filename");
  LoadGtkFunc(gtk_file_chooser_set_current_folder,"gtk_file_chooser_set_current_folder");
  LoadGtkFunc(gtk_file_chooser_get_current_folder,"gtk_file_chooser_get_current_folder");
  LoadGtkFunc(gtk_file_chooser_add_filter,"gtk_file_chooser_add_filter");
  LoadGtkFunc(gtk_file_filter_new,"gtk_file_filter_new");
  LoadGtkFunc(gtk_file_filter_set_name,"gtk_file_filter_set_name");
  LoadGtkFunc(gtk_file_filter_add_pattern,"gtk_file_filter_add_pattern");
  LoadGtkFunc(gtk_message_dialog_new,"gtk_message_dialog_new");

  LoadGlibFunc(g_free,"g_free");
  LoadGobjFunc(g_object_ref,"g_object_ref");
  LoadGobjFunc(g_object_unref,"g_object_unref");

  LoadGtkFunc(gtk_text_view_new_with_buffer,"gtk_text_view_new_with_buffer");
  LoadGtkFunc(gtk_text_view_set_editable,"gtk_text_view_set_editable");
  LoadGtkFunc(gtk_text_buffer_new,"gtk_text_buffer_new");
  LoadGtkFunc(gtk_text_buffer_get_line_count,"gtk_text_buffer_get_line_count");
  LoadGtkFunc(gtk_text_buffer_insert,"gtk_text_buffer_insert");
  LoadGtkFunc(gtk_text_buffer_delete,"gtk_text_buffer_delete");
  LoadGtkFunc(gtk_text_buffer_get_text,"gtk_text_buffer_get_text");
  LoadGtkFunc(gtk_text_buffer_get_iter_at_line,"gtk_text_buffer_get_iter_at_line");
  LoadGtkFunc(gtk_text_buffer_get_start_iter,"gtk_text_buffer_get_start_iter");
  LoadGtkFunc(gtk_text_buffer_get_end_iter,"gtk_text_buffer_get_end_iter");

  LoadGtkFunc(gtk_check_button_new,"gtk_check_button_new");
  LoadGtkFunc(gtk_toggle_button_get_active,"gtk_toggle_button_get_active");
  LoadGtkFunc(gtk_toggle_button_set_active,"gtk_toggle_button_set_active");

  LoadGtkFunc(gtk_combo_box_new_text,"gtk_combo_box_new_text");
  LoadGtkFunc(gtk_combo_box_get_active,"gtk_combo_box_get_active");
  LoadGtkFunc(gtk_combo_box_set_active,"gtk_combo_box_set_active");
  LoadGtkFunc(gtk_combo_box_append_text,"gtk_combo_box_append_text");
  LoadGtkFunc(gtk_combo_box_insert_text,"gtk_combo_box_insert_text");
  LoadGtkFunc(gtk_combo_box_prepend_text,"gtk_combo_box_prepend_text");
  LoadGtkFunc(gtk_combo_box_remove_text,"gtk_combo_box_remove_text");
  LoadGtkFunc(gtk_combo_box_get_active_text,"gtk_combo_box_get_active_text");

  LoadGtkFunc(gtk_entry_new,"gtk_entry_new");
  LoadGtkFunc(gtk_entry_set_text,"gtk_entry_set_text");
  LoadGtkFunc(gtk_entry_get_text,"gtk_entry_get_text");

  LoadGtkFunc(gtk_progress_bar_new,"gtk_progress_bar_new");
  LoadGtkFunc(gtk_progress_bar_set_text,"gtk_progress_bar_set_text");
  LoadGtkFunc(gtk_progress_bar_set_fraction,"gtk_progress_bar_set_fraction");
  LoadGtkFunc(gtk_progress_bar_set_orientation,"gtk_progress_bar_set_orientation");

  LoadGtkFunc(gtk_expander_new,"gtk_expander_new");
  LoadGtkFunc(gtk_expander_set_expanded,"gtk_expander_set_expanded");
  LoadGtkFunc(gtk_expander_set_label,"gtk_expander_set_label");


 // And final steps...
  gtk.UnloadKeep();
  #ifndef EOS_LINUX
   gdk.UnloadKeep();
   glib.UnloadKeep();
   gobj.UnloadKeep();
  #endif

  int num = 1;
  char * ptr = "eos";
  char ** ptp = &ptr;
  gtk_init(&num,&ptp);

  return true;
}

//------------------------------------------------------------------------------
 };
};
