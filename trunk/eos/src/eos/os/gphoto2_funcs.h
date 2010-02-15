#ifndef EOS_OS_GPHOTO_FUNCS_H
#define EOS_OS_GPHOTO_FUNCS_H
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

/// \file gphoto2_funcs.h
/// Provides access to the gphoto2, for remote controlling cameras.

#include "eos/types.h"
#include <stdarg.h>

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
// Data structures copied from the GPhoto header files. Legally dodgy, but
// neccesary and permission presumed based on there inclusion being required for
// use.
enum GPContextFeedback {GP_CONTEXT_FEEDBACK_OK,GP_CONTEXT_FEEDBACK_CANCEL};

typedef struct {
        char model [128];			/**< name of camera model */
        int status;		/**< driver quality */

	/** Supported port. */
	int port;
	/** Supported port speeds (terminated with a value of 0). */
        int speed [64];

        /* Supported operations */
        int       operations;	/**< camera operation funcs */
        int   file_operations;  /**< camera file op funcs */
        int folder_operations;/**< camera folder op funcs */

	int usb_vendor;		/**< USB Vendor D */
	int usb_product;	/**< USB Product ID */
	int usb_class;          /**< USB device class */
	int usb_subclass;	/**< USB device subclass */
	int usb_protocol;	/**< USB device protocol */

        /* For core use */
        char library [1024];	/**< \internal */
        char id [1024];		/**< \internal */

	/* Reserved space to use in the future w/out changing the
	 * struct size */
	int reserved1;		/**< reserved space \internal */
        int reserved2;		/**< reserved space \internal */
        int reserved3;		/**< reserved space \internal */
        int reserved4;		/**< reserved space \internal */
        int reserved5;		/**< reserved space \internal */
        int reserved6;		/**< reserved space \internal */
        int reserved7;		/**< reserved space \internal */
        int reserved8;		/**< reserved space \internal */
} GPCameraAbilities;

struct GPPortInfo
{
	int type;
	char name[64];
	char path[64];

	/* Private */
	char library_filename[1024];
};

typedef enum {			/* Value (get/set):	*/
	GP_WIDGET_WINDOW,
	GP_WIDGET_SECTION,
	GP_WIDGET_TEXT,		/* char *		*/
	GP_WIDGET_RANGE,	/* float		*/
	GP_WIDGET_TOGGLE,	/* int			*/
	GP_WIDGET_RADIO,	/* char *		*/
	GP_WIDGET_MENU,		/* char *		*/
	GP_WIDGET_BUTTON,	/* CameraWidgetCallback */
	GP_WIDGET_DATE		/* int			*/
} GPCameraWidgetType;

typedef enum {
	GP_CAPTURE_IMAGE,
	GP_CAPTURE_MOVIE,
	GP_CAPTURE_SOUND
} GPCameraCaptureType;

typedef struct {
	char name [128];
	char folder [1024];
} GPCameraFilePath;

typedef enum {
	GP_FILE_TYPE_PREVIEW,
	GP_FILE_TYPE_NORMAL,
	GP_FILE_TYPE_RAW,
	GP_FILE_TYPE_AUDIO,
	GP_FILE_TYPE_EXIF
} GPCameraFileType;

typedef enum {
	GP_LOG_ERROR = 0,
	GP_LOG_VERBOSE = 1,
	GP_LOG_DEBUG = 2,
	GP_LOG_DATA = 3
} GPLogLevel;


struct GPContext {};
struct GPCameraAbilitiesList {};
struct GPPortInfoList {};
struct GPCameraList {};
struct GPCameraFile {};
struct GPCameraWidget {};
struct GPCamera {};


// Definitions of callback functions given to gphoto...
typedef void EOS_STDCALL (*GPContextIdleFunc)(GPContext * context,void * data);
typedef void EOS_STDCALL (*GPContextErrorFunc)(GPContext * context,const char * format,va_list args,void * data) __attribute__((__format__(printf,2,0)));
typedef void EOS_STDCALL (*GPContextStatusFunc)(GPContext * context,const char * format,va_list args,void * data) __attribute__((__format__(printf,2,0)));
typedef void EOS_STDCALL (*GPContextMessageFunc)(GPContext * context,const char * format,va_list args,void * data) __attribute__((__format__(printf,2,0)));
typedef GPContextFeedback EOS_STDCALL (*GPContextQuestionFunc)(GPContext * context,const char * format,va_list args,void * data) __attribute__((__format__(printf,2,0)));
typedef GPContextFeedback EOS_STDCALL (*GPContextCancelFunc)(GPContext * context,void * data);
typedef unsigned int EOS_STDCALL (*GPContextProgressStartFunc)(GPContext * context,float target,const char * format,va_list args,void * data) __attribute__((__format__(printf,3,0)));
typedef void EOS_STDCALL (*GPContextProgressUpdateFunc)(GPContext * context,unsigned int id,float current,void * data);
typedef void EOS_STDCALL (*GPContextProgressStopFunc)(GPContext * context,unsigned int id,void * data);

typedef void EOS_STDCALL (*GPLogFunc)(GPLogLevel level,const char * domain,
			              const char * format,va_list args,void * data);


//------------------------------------------------------------------------------
// Pointers to all the gphoto2 functions needed...
EOS_VAR int EOS_STDCALL (*gp_port_info_list_new)(GPPortInfoList ** list);
EOS_VAR int EOS_STDCALL (*gp_port_info_list_free)(GPPortInfoList * list);
EOS_VAR int EOS_STDCALL (*gp_port_info_list_load)(GPPortInfoList * list);
EOS_VAR int EOS_STDCALL (*gp_port_info_list_lookup_path)(GPPortInfoList * list,const char * path);
EOS_VAR int EOS_STDCALL (*gp_port_info_list_get_info)(GPPortInfoList * list,int n,GPPortInfo * info);

EOS_VAR int EOS_STDCALL (*gp_log_add_func)(GPLogLevel level,GPLogFunc func,void * data);

EOS_VAR GPContext * EOS_STDCALL (*gp_context_new)(void);
EOS_VAR void EOS_STDCALL (*gp_context_ref)(GPContext * context);
EOS_VAR void EOS_STDCALL (*gp_context_unref)(GPContext * context);
EOS_VAR void EOS_STDCALL (*gp_context_set_idle_func)(GPContext * context,GPContextIdleFunc func,void * data);
EOS_VAR void EOS_STDCALL (*gp_context_set_progress_funcs)(GPContext * context,GPContextProgressStartFunc start_func,GPContextProgressUpdateFunc update_func,GPContextProgressStopFunc stop_func,void * data);
EOS_VAR void EOS_STDCALL (*gp_context_set_error_func)(GPContext * context,GPContextErrorFunc func,void * data);
EOS_VAR void EOS_STDCALL (*gp_context_set_status_func)(GPContext * context,GPContextStatusFunc func,void *data);
EOS_VAR void EOS_STDCALL (*gp_context_set_question_func)(GPContext * context,GPContextQuestionFunc func, void * data);
EOS_VAR void EOS_STDCALL (*gp_context_set_cancel_func)(GPContext * context,GPContextCancelFunc func,void * data);
EOS_VAR void EOS_STDCALL (*gp_context_set_message_func)(GPContext * context,GPContextMessageFunc func,void *data);

EOS_VAR int EOS_STDCALL (*gp_abilities_list_new)(GPCameraAbilitiesList ** list);
EOS_VAR int EOS_STDCALL (*gp_abilities_list_free)(GPCameraAbilitiesList * list);
EOS_VAR int EOS_STDCALL (*gp_abilities_list_load)(GPCameraAbilitiesList * list,GPContext * context);
EOS_VAR int EOS_STDCALL (*gp_abilities_list_detect)(GPCameraAbilitiesList * list,GPPortInfoList * info_list, GPCameraList * l,GPContext*);
EOS_VAR int EOS_STDCALL (*gp_abilities_list_count)(GPCameraAbilitiesList * list);
EOS_VAR int EOS_STDCALL (*gp_abilities_list_lookup_model)(GPCameraAbilitiesList * list,const char * model);
EOS_VAR int EOS_STDCALL (*gp_abilities_list_get_abilities)(GPCameraAbilitiesList * list,int index,GPCameraAbilities * abilities);

EOS_VAR int EOS_STDCALL (*gp_list_new)(GPCameraList ** list);
EOS_VAR int EOS_STDCALL (*gp_list_free)(GPCameraList * list);
EOS_VAR int EOS_STDCALL (*gp_list_count)(GPCameraList * list);
EOS_VAR int EOS_STDCALL (*gp_list_get_name)(GPCameraList * list,int index,const char ** name);
EOS_VAR int EOS_STDCALL (*gp_list_get_value)(GPCameraList * list,int index,const char ** value);

EOS_VAR int EOS_STDCALL (*gp_file_new)(GPCameraFile ** file);
EOS_VAR int EOS_STDCALL (*gp_file_free)(GPCameraFile * file);
EOS_VAR int EOS_STDCALL (*gp_file_save)(GPCameraFile * file,const char * filename);

EOS_VAR int EOS_STDCALL (*gp_widget_new)(GPCameraWidgetType type,const char * label,GPCameraWidget ** widget);
EOS_VAR int EOS_STDCALL (*gp_widget_free)(GPCameraWidget * widget);
EOS_VAR int EOS_STDCALL (*gp_widget_ref)(GPCameraWidget * widget);
EOS_VAR int EOS_STDCALL (*gp_widget_unref)(GPCameraWidget * widget);
EOS_VAR int EOS_STDCALL (*gp_widget_count_children)(GPCameraWidget * widget);
EOS_VAR int EOS_STDCALL (*gp_widget_get_child)(GPCameraWidget * widget,int child_number,GPCameraWidget ** child);
EOS_VAR int EOS_STDCALL (*gp_widget_set_value)(GPCameraWidget * widget,const void * value);
EOS_VAR int EOS_STDCALL (*gp_widget_get_value)(GPCameraWidget * widget,void * value);
EOS_VAR int EOS_STDCALL (*gp_widget_get_name)(GPCameraWidget * widget,const char ** name);
EOS_VAR int EOS_STDCALL (*gp_widget_get_info)(GPCameraWidget * widget,const char ** info);
EOS_VAR int EOS_STDCALL (*gp_widget_get_id)(GPCameraWidget * widget,int * id);
EOS_VAR int EOS_STDCALL (*gp_widget_get_type)(GPCameraWidget * widget,GPCameraWidgetType * type);
EOS_VAR int EOS_STDCALL (*gp_widget_get_label)(GPCameraWidget * widget,const char ** label);
EOS_VAR int EOS_STDCALL (*gp_widget_get_range)(GPCameraWidget * range,float * min,float * max,float * increment);
EOS_VAR int EOS_STDCALL (*gp_widget_count_choices)(GPCameraWidget * widget);
EOS_VAR int EOS_STDCALL (*gp_widget_get_choice)(GPCameraWidget * widget,int choice_number,const char ** choice);
EOS_VAR int EOS_STDCALL (*gp_widget_set_changed)(GPCameraWidget * widget,int changed);

EOS_VAR int EOS_STDCALL (*gp_camera_new)(GPCamera ** camera);
EOS_VAR int EOS_STDCALL (*gp_camera_set_abilities)(GPCamera * camera,GPCameraAbilities abilities);
EOS_VAR int EOS_STDCALL (*gp_camera_set_port_info)(GPCamera * camera,GPPortInfo info);
EOS_VAR int EOS_STDCALL (*gp_camera_init)(GPCamera * camera,GPContext * context);
EOS_VAR int EOS_STDCALL (*gp_camera_exit)(GPCamera * camera,GPContext * context);
EOS_VAR int EOS_STDCALL (*gp_camera_free)(GPCamera * camera);
EOS_VAR int EOS_STDCALL (*gp_camera_get_config)(GPCamera * camera,GPCameraWidget ** window,GPContext * context);
EOS_VAR int EOS_STDCALL (*gp_camera_set_config)(GPCamera * camera,GPCameraWidget * window,GPContext * context);
EOS_VAR int EOS_STDCALL (*gp_camera_capture)(GPCamera * camera,GPCameraCaptureType type,GPCameraFilePath * path,GPContext *context);
EOS_VAR int EOS_STDCALL (*gp_camera_file_get)(GPCamera * camera,const char * folder,const char * file,GPCameraFileType type,GPCameraFile * camera_file,GPContext * context);
EOS_VAR int EOS_STDCALL (*gp_camera_file_delete)(GPCamera * camera,const char * folder,const char * file,GPContext * context);


//------------------------------------------------------------------------------
// So gPhoto can use my logging system...
EOS_FUNC void EOS_STDCALL GphotoLogFunc(GPLogLevel level,const char * domain,const char * format,va_list args,void * data);
EOS_FUNC void EOS_STDCALL ErrorFunc(GPContext * context,const char * format,va_list args,void * data);
EOS_FUNC void EOS_STDCALL StatusFunc(GPContext * context,const char * format,va_list args,void * data);
EOS_FUNC void EOS_STDCALL MessageFunc(GPContext * context,const char * format,va_list args,void * data);


//------------------------------------------------------------------------------
// A function that checks if the libgphoto2 library has been loaded, and if not
// trys to load it. Returns true if its all ready to go, false if not...
bit LoadGPhoto();


//------------------------------------------------------------------------------
 };
};
#endif
