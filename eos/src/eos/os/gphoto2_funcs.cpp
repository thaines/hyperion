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

#include "eos/os/gphoto2_funcs.h"

#include "eos/str/functions.h"
#include "eos/file/dlls.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
EOS_VAR_DEF int EOS_STDCALL (*gp_port_info_list_new)(GPPortInfoList ** list);
EOS_VAR_DEF int EOS_STDCALL (*gp_port_info_list_free)(GPPortInfoList * list);
EOS_VAR_DEF int EOS_STDCALL (*gp_port_info_list_load)(GPPortInfoList * list);
EOS_VAR_DEF int EOS_STDCALL (*gp_port_info_list_lookup_path)(GPPortInfoList * list,const char * path);
EOS_VAR_DEF int EOS_STDCALL (*gp_port_info_list_get_info)(GPPortInfoList * list,int n,GPPortInfo * info);

EOS_VAR_DEF int EOS_STDCALL (*gp_log_add_func)(GPLogLevel level,GPLogFunc func,void * data);

EOS_VAR_DEF GPContext * EOS_STDCALL (*gp_context_new)(void);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_ref)(GPContext * context);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_unref)(GPContext * context);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_idle_func)(GPContext * context,GPContextIdleFunc func,void * data);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_progress_funcs)(GPContext * context,GPContextProgressStartFunc start_func,GPContextProgressUpdateFunc update_func,GPContextProgressStopFunc stop_func,void * data);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_error_func)(GPContext * context,GPContextErrorFunc func,void * data);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_status_func)(GPContext * context,GPContextStatusFunc func,void *data);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_question_func)(GPContext * context,GPContextQuestionFunc func, void * data);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_cancel_func)(GPContext * context,GPContextCancelFunc func,void * data);
EOS_VAR_DEF void EOS_STDCALL (*gp_context_set_message_func)(GPContext * context,GPContextMessageFunc func,void *data);

EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_new)(GPCameraAbilitiesList ** list);
EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_free)(GPCameraAbilitiesList * list);
EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_load)(GPCameraAbilitiesList * list,GPContext * context);
EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_detect)(GPCameraAbilitiesList * list,GPPortInfoList * info_list, GPCameraList * l,GPContext*);
EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_count)(GPCameraAbilitiesList * list);
EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_lookup_model)(GPCameraAbilitiesList * list,const char * model);
EOS_VAR_DEF int EOS_STDCALL (*gp_abilities_list_get_abilities)(GPCameraAbilitiesList * list,int index,GPCameraAbilities * abilities);

EOS_VAR_DEF int EOS_STDCALL (*gp_list_new)(GPCameraList ** list);
EOS_VAR_DEF int EOS_STDCALL (*gp_list_free)(GPCameraList * list);
EOS_VAR_DEF int EOS_STDCALL (*gp_list_count)(GPCameraList * list);
EOS_VAR_DEF int EOS_STDCALL (*gp_list_get_name)(GPCameraList * list,int index,const char ** name);
EOS_VAR_DEF int EOS_STDCALL (*gp_list_get_value)(GPCameraList * list,int index,const char ** value);

EOS_VAR_DEF int EOS_STDCALL (*gp_file_new)(GPCameraFile ** file);
EOS_VAR_DEF int EOS_STDCALL (*gp_file_free)(GPCameraFile * file);
EOS_VAR_DEF int EOS_STDCALL (*gp_file_save)(GPCameraFile * file,const char * filename);

EOS_VAR_DEF int EOS_STDCALL (*gp_widget_new)(GPCameraWidgetType type,const char * label,GPCameraWidget ** widget);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_free)(GPCameraWidget * widget);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_ref)(GPCameraWidget * widget);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_unref)(GPCameraWidget * widget);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_count_children)(GPCameraWidget * widget);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_child)(GPCameraWidget * widget,int child_number,GPCameraWidget ** child);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_set_value)(GPCameraWidget * widget,const void * value);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_value)(GPCameraWidget * widget,void * value);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_name)(GPCameraWidget * widget,const char ** name);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_info)(GPCameraWidget * widget,const char ** info);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_id)(GPCameraWidget * widget,int * id);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_type)(GPCameraWidget * widget,GPCameraWidgetType * type);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_label)(GPCameraWidget * widget,const char ** label);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_range)(GPCameraWidget * range,float * min,float * max,float * increment);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_count_choices)(GPCameraWidget * widget);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_get_choice)(GPCameraWidget * widget,int choice_number,const char ** choice);
EOS_VAR_DEF int EOS_STDCALL (*gp_widget_set_changed)(GPCameraWidget * widget,int changed);

EOS_VAR_DEF int EOS_STDCALL (*gp_camera_new)(GPCamera ** camera);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_set_abilities)(GPCamera * camera,GPCameraAbilities abilities);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_set_port_info)(GPCamera * camera,GPPortInfo info);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_init)(GPCamera * camera,GPContext * context);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_exit)(GPCamera * camera,GPContext * context);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_free)(GPCamera * camera);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_get_config)(GPCamera * camera,GPCameraWidget ** window,GPContext * context);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_set_config)(GPCamera * camera,GPCameraWidget * window,GPContext * context);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_capture)(GPCamera * camera,GPCameraCaptureType type,GPCameraFilePath * path,GPContext *context);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_file_get)(GPCamera * camera,const char * folder,const char * file,GPCameraFileType type,GPCameraFile * camera_file,GPContext * context);
EOS_VAR_DEF int EOS_STDCALL (*gp_camera_file_delete)(GPCamera * camera,const char * folder,const char * file,GPContext * context);


//------------------------------------------------------------------------------
EOS_FUNC void EOS_STDCALL GphotoLogFunc(GPLogLevel level,const char * domain,const char * format,va_list args,void * data)
{
 switch (level)
 {
  case GP_LOG_ERROR:
  {
   char s[4096];
   vsprintf(s,format,args);
   LogAlways("[gphoto.error]" << LogDiv() << domain << LogDiv() << s);
  }
  break;
  case GP_LOG_VERBOSE:
  case GP_LOG_DEBUG:
  {
   char s[4096];
   vsprintf(s,format,args);
   LogAlways("[gphoto.debug]" << LogDiv() << domain << LogDiv() << s);
  }
  break;
  default: break;
 }
}

//------------------------------------------------------------------------------
// A function that checks if the libgphoto2 library has been loaded, and if not
// trys to load it. Returns true if its all ready to go, false if not...
bit LoadGPhoto()
{
 if (gp_context_new) return true; // Allready loaded.

 file::Dll gport;
  gport.Load("gphoto2_port");
 if (!gport.Active()) {LogAlways("Could not load gphoto_port library"); return false;}


 (void*&)gp_port_info_list_new = gport.Get("gp_port_info_list_new");
 (void*&)gp_port_info_list_free = gport.Get("gp_port_info_list_free");
 (void*&)gp_port_info_list_load = gport.Get("gp_port_info_list_load");
 (void*&)gp_port_info_list_lookup_path = gport.Get("gp_port_info_list_lookup_path");
 (void*&)gp_port_info_list_get_info = gport.Get("gp_port_info_list_get_info");

 (void*&)gp_log_add_func = gport.Get("gp_log_add_func");


 file::Dll gphoto;
  gphoto.Load("gphoto2");
 if (!gphoto.Active()) {LogAlways("Could not load gphoto library"); return false;}


 (void*&)gp_context_new = gphoto.Get("gp_context_new");
 (void*&)gp_context_ref = gphoto.Get("gp_context_ref");
 (void*&)gp_context_unref = gphoto.Get("gp_context_unref");
 (void*&)gp_context_set_idle_func = gphoto.Get("gp_context_set_idle_func");
 (void*&)gp_context_set_progress_funcs = gphoto.Get("gp_context_set_progress_funcs");
 (void*&)gp_context_set_error_func = gphoto.Get("gp_context_set_error_func");
 (void*&)gp_context_set_status_func = gphoto.Get("gp_context_set_status_func");
 (void*&)gp_context_set_question_func = gphoto.Get("gp_context_set_question_func");
 (void*&)gp_context_set_cancel_func = gphoto.Get("gp_context_set_cancel_func");
 (void*&)gp_context_set_message_func = gphoto.Get("gp_context_set_message_func");

 (void*&)gp_abilities_list_new = gphoto.Get("gp_abilities_list_new");
 (void*&)gp_abilities_list_free = gphoto.Get("gp_abilities_list_free");
 (void*&)gp_abilities_list_load = gphoto.Get("gp_abilities_list_load");
 (void*&)gp_abilities_list_detect = gphoto.Get("gp_abilities_list_detect");
 (void*&)gp_abilities_list_count = gphoto.Get("gp_abilities_list_count");
 (void*&)gp_abilities_list_lookup_model = gphoto.Get("gp_abilities_list_lookup_model");
 (void*&)gp_abilities_list_get_abilities = gphoto.Get("gp_abilities_list_get_abilities");

 (void*&)gp_list_new = gphoto.Get("gp_list_new");
 (void*&)gp_list_free = gphoto.Get("gp_list_free");
 (void*&)gp_list_count = gphoto.Get("gp_list_count");
 (void*&)gp_list_get_name = gphoto.Get("gp_list_get_name");
 (void*&)gp_list_get_value = gphoto.Get("gp_list_get_value");

 (void*&)gp_file_new = gphoto.Get("gp_file_new");
 (void*&)gp_file_free = gphoto.Get("gp_file_free");
 (void*&)gp_file_save = gphoto.Get("gp_file_save");

 (void*&)gp_widget_new = gphoto.Get("gp_widget_new");
 (void*&)gp_widget_free = gphoto.Get("gp_widget_free");
 (void*&)gp_widget_ref = gphoto.Get("gp_widget_ref");
 (void*&)gp_widget_unref = gphoto.Get("gp_widget_unref");
 (void*&)gp_widget_count_children = gphoto.Get("gp_widget_count_children");
 (void*&)gp_widget_get_child = gphoto.Get("gp_widget_get_child");
 (void*&)gp_widget_set_value = gphoto.Get("gp_widget_set_value");
 (void*&)gp_widget_get_value = gphoto.Get("gp_widget_get_value");
 (void*&)gp_widget_get_name = gphoto.Get("gp_widget_get_name");
 (void*&)gp_widget_get_info = gphoto.Get("gp_widget_get_info");
 (void*&)gp_widget_get_id = gphoto.Get("gp_widget_get_id");
 (void*&)gp_widget_get_type = gphoto.Get("gp_widget_get_type");
 (void*&)gp_widget_get_label = gphoto.Get("gp_widget_get_label");
 (void*&)gp_widget_get_range = gphoto.Get("gp_widget_get_range");
 (void*&)gp_widget_count_choices = gphoto.Get("gp_widget_count_choices");
 (void*&)gp_widget_get_choice = gphoto.Get("gp_widget_get_choice");
 (void*&)gp_widget_set_changed = gphoto.Get("gp_widget_set_changed");

 (void*&)gp_camera_new = gphoto.Get("gp_camera_new");
 (void*&)gp_camera_set_abilities = gphoto.Get("gp_camera_set_abilities");
 (void*&)gp_camera_set_port_info = gphoto.Get("gp_camera_set_port_info");
 (void*&)gp_camera_init = gphoto.Get("gp_camera_init");
 (void*&)gp_camera_exit = gphoto.Get("gp_camera_exit");
 (void*&)gp_camera_free = gphoto.Get("gp_camera_free");
 (void*&)gp_camera_get_config = gphoto.Get("gp_camera_get_config");
 (void*&)gp_camera_set_config = gphoto.Get("gp_camera_set_config");
 (void*&)gp_camera_capture = gphoto.Get("gp_camera_capture");
 (void*&)gp_camera_file_get = gphoto.Get("gp_camera_file_get");
 (void*&)gp_camera_file_delete = gphoto.Get("gp_camera_file_delete");

 if ((gp_port_info_list_new==0)||(gp_port_info_list_free==0)||(gp_port_info_list_load==0)||
     (gp_port_info_list_lookup_path==0)||(gp_port_info_list_get_info==0)||(gp_log_add_func==0)||
     (gp_context_new==0)||(gp_context_ref==0)||(gp_context_unref==0)||(gp_context_set_idle_func==0)||
     (gp_context_set_progress_funcs==0)||(gp_context_set_error_func==0)||(gp_context_set_status_func==0)||
     (gp_context_set_question_func==0)||(gp_context_set_question_func==0)||
     (gp_context_set_message_func==0)||(gp_abilities_list_new==0)||(gp_abilities_list_free==0)||
     (gp_abilities_list_load==0)||(gp_abilities_list_detect==0)||(gp_abilities_list_count==0)||
     (gp_abilities_list_lookup_model==0)||(gp_abilities_list_get_abilities==0)||(gp_list_new==0)||
     (gp_list_free==0)||(gp_list_count==0)||(gp_list_get_name==0)||(gp_list_get_value==0)||
     (gp_file_new==0)||(gp_file_free==0)||(gp_file_save==0)||(gp_widget_new==0)||(gp_widget_free==0)||
     (gp_widget_ref==0)||(gp_widget_unref==0)||(gp_widget_count_children==0)||(gp_widget_get_child==0)||
     (gp_widget_set_value==0)||(gp_widget_get_value==0)||(gp_widget_get_name==0)||
     (gp_widget_get_info==0)||(gp_widget_get_id==0)||(gp_widget_get_type==0)||(gp_widget_get_label==0)||
     (gp_widget_get_range==0)||(gp_widget_count_choices==0)||(gp_widget_get_choice==0)||
     (gp_widget_set_changed==0)||(gp_camera_new==0)||(gp_camera_set_abilities==0)||
     (gp_camera_set_port_info==0)||(gp_camera_init==0)||(gp_camera_exit==0)||(gp_camera_free==0)||
     (gp_camera_get_config==0)||(gp_camera_set_config==0)||(gp_camera_capture==0)||
     (gp_camera_file_get==0)||(gp_camera_file_delete==0))
 {
  return false;
  LogAlways("Could not load all gphoto functions");
 }

 // Setup low level logging capture...
  #ifdef EOS_DEBUG
   gp_log_add_func(GP_LOG_DEBUG,GphotoLogFunc,0);
  #else
   gp_log_add_func(GP_LOG_ERROR,GphotoLogFunc,0);
  #endif

 gport.UnloadKeep();
 gphoto.UnloadKeep();
 return true;
}

//------------------------------------------------------------------------------
EOS_FUNC void EOS_STDCALL ErrorFunc(GPContext * context,const char * format,va_list args,void * data)
{
 char s[4096];
 vsprintf(s,format,args);
 LogAlways("[gphoto.error]" << LogDiv() << s);
}

EOS_FUNC void EOS_STDCALL StatusFunc(GPContext * context,const char * format,va_list args,void * data)
{
 char s[4096];
 vsprintf(s,format,args);
 LogDebug("[gphoto.status]" << LogDiv() << s);
}

EOS_FUNC void EOS_STDCALL MessageFunc(GPContext * context,const char * format,va_list args,void * data)
{
 char s[4096];
 vsprintf(s,format,args);
 LogDebug("[gphoto.message]" << LogDiv() << s);
}
//------------------------------------------------------------------------------
 };
};
