//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "eos/os/cameras.h"

#include "eos/str/functions.h"
#include "eos/file/csv.h"
#include "eos/os/gphoto2_funcs.h"

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
Camera::Camera()
:cam(null<void*>())
{}

Camera::~Camera()
{
 Deactivate();
}

bit Camera::Active() const
{
 if (cam==null<void*>()) return false;
 return true;
}

void Camera::Deactivate()
{
 if (cam)
 {
  GPContext * context = gp_context_new();
  if (context!=null<GPContext*>())
  {
   gp_context_ref(context);
   gp_context_set_error_func(context,ErrorFunc,0);
   gp_context_set_status_func(context,StatusFunc,0);
   gp_context_set_message_func(context,MessageFunc,0);
  }

  if (gp_camera_exit((GPCamera*)cam,context)<0) {LogAlways("[gphoto] Error disconnecting camera");}
  if (gp_camera_free((GPCamera*)cam)<0) {LogAlways("[gphoto] Error deleting camera");}

  if (context) gp_context_unref(context);

  cam = null<void*>();
 }
}

// Helper for below...
bs::Element * WidgetToElem(str::TokenTable & tt,GPCameraWidget * widget)
{
 // Element to represent self...
  // Create the element, with the correct type...
   GPCameraWidgetType type;
   if (gp_widget_get_type(widget,&type)<0) {LogAlways("[gphoto] Error getting widget type"); return null<bs::Element*>();}
   cstrconst typeStr;
   switch (type)
   {
    case GP_WIDGET_WINDOW: typeStr = "window"; break;
    case GP_WIDGET_SECTION: typeStr = "section"; break;
    case GP_WIDGET_TEXT: typeStr = "text"; break;
    case GP_WIDGET_RANGE: typeStr = "range"; break;
    case GP_WIDGET_TOGGLE: typeStr = "toggle"; break;
    case GP_WIDGET_RADIO: typeStr = "radio"; break;
    case GP_WIDGET_MENU: typeStr = "menu"; break;
    case GP_WIDGET_BUTTON: return null<bs::Element*>(); // Unsupported.
    case GP_WIDGET_DATE: typeStr = "date"; break;
    default: LogAlways("[gphoto] Unrecognised widget type"); return null<bs::Element*>();
   }
   bs::Element * ret = new bs::Element(tt,typeStr);

  // Name...
   cstrconst name;
   if (gp_widget_get_name(widget,&name)<0) {LogAlways("[gphoto] Error getting widget name"); name = "error";}
   ret->SetAttribute("name",name);

  // Info...
   cstrconst info;
   if (gp_widget_get_info(widget,&info)<0) {LogAlways("[gphoto] Error getting widget info"); info = "error";}
   ret->SetAttribute("info",info);

  // Label...
   cstrconst label;
   if (gp_widget_get_label(widget,&label)<0) {LogAlways("[gphoto] Error getting widget label"); label = "error";}
   ret->SetAttribute("label",label);

  // Id...
   int id;
   if (gp_widget_get_id(widget,&id)<0) {LogAlways("[gphoto] Error getting widget id"); id = -1;}
   ret->SetAttribute("id",int32(id));

  // Value, plus value specific...
   switch (type)
   {
    case GP_WIDGET_TEXT:
    {
     char * value;
     if (gp_widget_get_value(widget,&value)<0) {LogAlways("[gphoto] Error getting widget value");}
     else
     {
      ret->SetAttribute("value",value);
     }
    }
    break;
    case GP_WIDGET_RANGE:
    {
     float value;
     if (gp_widget_get_value(widget,&value)<0) {LogAlways("[gphoto] Error getting widget value");}
     else
     {
      ret->SetAttribute("value",real32(value));
      float min,max,inc;
      if (gp_widget_get_range(widget,&min,&max,&inc)<0) {LogAlways("[gphoto] Error getting widget range");}
      else
      {
       ret->SetAttribute("min",real32(min));
       ret->SetAttribute("max",real32(max));
       ret->SetAttribute("increment",real32(inc));
      }
     }
    }
    break;
    case GP_WIDGET_TOGGLE:
    {
     int value;
     if (gp_widget_get_value(widget,&value)<0) {LogAlways("[gphoto] Error getting widget value");}
     else
     {
      ret->SetAttribute("value",real32(value));
     }
    }
    break;
    case GP_WIDGET_RADIO:
    {
     char * value;
     if (gp_widget_get_value(widget,&value)<0) {LogAlways("[gphoto] Error getting widget value");}
     else
     {
      ret->SetAttribute("value",value);
      int choices = gp_widget_count_choices(widget);
      if (choices<0) {LogAlways("[gphoto] Error getting choice count");}
      str::String s;
      for (int i=0;i<choices;i++)
      {
       cstrconst c;
       if (gp_widget_get_choice(widget,i,&c)<0) {LogAlways("[gphoto] Error getting choice");}
       else
       {
        if (s.Size()!=0) s += ",";
        s += c;
       }
      }
      ret->SetAttribute("choices",s);
     }
    }
    break;
    case GP_WIDGET_MENU:
    {
     char * value;
     if (gp_widget_get_value(widget,&value)<0) {LogAlways("[gphoto] Error getting widget value");}
     else
     {
      ret->SetAttribute("value",value);
      int choices = gp_widget_count_choices(widget);
      if (choices<0) {LogAlways("[gphoto] Error getting choice count");}
      str::String s;
      for (int i=0;i<choices;i++)
      {
       cstrconst c;
       if (gp_widget_get_choice(widget,i,&c)<0) {LogAlways("[gphoto] Error getting choice");}
       else
       {
        if (s.Size()!=0) s += ",";
        s += c;
       }
      }
      ret->SetAttribute("choices",s);
     }
    }
    break;
    case GP_WIDGET_DATE:
    {
     int value;
     if (gp_widget_get_value(widget,&value)<0) {LogAlways("[gphoto] Error getting widget value");}
     else
     {
      ret->SetAttribute("value",real32(value));
     }
    }
    break;
    default:
    break;
   }

 // Iterate children...
  if ((type==GP_WIDGET_WINDOW)||(type==GP_WIDGET_SECTION))
  {
   int childCount = gp_widget_count_children(widget);
   if (childCount<0) {LogAlways("[gphoto] Error getting widget child count"); return ret;}
   for (int i=0;i<childCount;i++)
   {
    GPCameraWidget * child;
    if (gp_widget_get_child(widget,i,&child)<0) {LogAlways("[gphoto] Error getting child widget");}
    else
    {
     bs::Element * c = WidgetToElem(tt,child);
     if (c) ret->AppendChild(c);
    }
   }
  }

 return ret;
}

bs::Element * Camera::GetConf(str::TokenTable & tt) const
{
 GPContext * context = gp_context_new();
 if (context!=null<GPContext*>())
 {
  gp_context_ref(context);
  gp_context_set_error_func(context,ErrorFunc,0);
  gp_context_set_status_func(context,StatusFunc,0);
  gp_context_set_message_func(context,MessageFunc,0);
 }

  GPCameraWidget * widget;
  if (gp_camera_get_config((GPCamera*)cam,&widget,context)<0) {LogAlways("[gphoto] Error getting camera configuration"); return null<bs::Element*>();}
  bs::Element * ret = WidgetToElem(tt,widget);
  if (gp_widget_free(widget)<0) {LogAlways("[gphoto] Error destroying widget"); return null<bs::Element*>();}

 if (context) gp_context_unref(context);

 return ret;
}

// Helper for below...
void ElemToWidget(const bs::Element * elem,GPCameraWidget * widget)
{
 // Get and switch on type...
  GPCameraWidgetType type;
  if (gp_widget_get_type(widget,&type)<0) {LogAlways("[gphoto] Error getting widget type"); return;}
  switch (type)
  {
   case GP_WIDGET_WINDOW:
   case GP_WIDGET_SECTION:
   {
    // Iterate children, recurse on those we have the relevent info for...
     int childCount = gp_widget_count_children(widget);
     if (childCount<0) {LogAlways("[gphoto] Error getting widget child count");}
     for (int i=0;i<childCount;i++)
     {
      GPCameraWidget * child;
      if (gp_widget_get_child(widget,i,&child)<0) {LogAlways("[gphoto] Error getting child widget");}
      else
      {
       // Get child id...
        int id;
        if (gp_widget_get_id(child,&id)<0) {LogAlways("[gphoto] Error getting widget id");}
        else
        {
         str::String s; s << int32(id);
         bs::Attribute attr(elem->TokTab()("id"),s);

         // Get child type string...
          GPCameraWidgetType type;
          if (gp_widget_get_type(widget,&type)<0) {LogAlways("[gphoto] Error getting widget type");}
          else
          {
           cstrconst typeStr = null<cstrconst>();
           switch (type)
           {
            case GP_WIDGET_WINDOW: typeStr = "window"; break;
            case GP_WIDGET_SECTION: typeStr = "section"; break;
            case GP_WIDGET_TEXT: typeStr = "text"; break;
            case GP_WIDGET_RANGE: typeStr = "range"; break;
            case GP_WIDGET_TOGGLE: typeStr = "toggle"; break;
            case GP_WIDGET_RADIO: typeStr = "radio"; break;
            case GP_WIDGET_MENU: typeStr = "menu"; break;
            case GP_WIDGET_BUTTON: break; // Unsupported.
            case GP_WIDGET_DATE: typeStr = "date"; break;
            default: LogAlways("[gphoto] Unrecognised widget type"); break;
           }
           if (typeStr)
           {
            bs::Element * cm = elem->FindChild(typeStr,attr);
            if (cm) ElemToWidget(cm,child);
           }
          }
        }
      }
     }
   }
   break;
   case GP_WIDGET_TEXT:
   case GP_WIDGET_RADIO:
   case GP_WIDGET_MENU:
   {
    bs::Attribute * attr = elem->GetAttribute("value");
    if (attr)
    {
     cstr value = attr->AsString().ToStr();
     if (gp_widget_set_value(widget,&value)<0) {LogAlways("[gphoto] Error setting widget (text)");}
     mem::Free(value);
    }
   }
   break;
   case GP_WIDGET_RANGE:
   {
    bs::Attribute * attr = elem->GetAttribute("value");
    if (attr)
    {
     float value = attr->AsReal();
     if (gp_widget_set_value(widget,&value)<0) {LogAlways("[gphoto] Error setting widget (real)");}
    }
   }
   break;
   case GP_WIDGET_TOGGLE:
   case GP_WIDGET_DATE:
   {
    bs::Attribute * attr = elem->GetAttribute("value");
    if (attr)
    {
     int value = attr->AsInt();
     if (gp_widget_set_value(widget,&value)<0) {LogAlways("[gphoto] Error setting widget (int)");}
    }
   }
   break;
   case GP_WIDGET_BUTTON:
   return; // Unsupported.
   default:
    LogAlways("[gphoto] Unrecognised widget type");
   return;
  }
}

void Camera::SetConf(const bs::Element * root)
{
 // Approach taken is to get the current configuration
 // and to then iterate it, for every type found an attempt is
 // then made to find its relevent Element in the given dom
 // and to update it if possible...
  // So errors end up in the log...
   GPContext * context = gp_context_new();
   if (context!=null<GPContext*>())
   {
    gp_context_ref(context);
    gp_context_set_error_func(context,ErrorFunc,0);
    gp_context_set_status_func(context,StatusFunc,0);
    gp_context_set_message_func(context,MessageFunc,0);
   }

  // Extract current configuration...
   GPCameraWidget * widget;
   if (gp_camera_get_config((GPCamera*)cam,&widget,context)<0) {LogAlways("[gphoto] Error getting camera configuration (b)"); return;}

  // Update it with the given dom...
   ElemToWidget(root,widget);

  // Write the configuration back...
   if (gp_camera_set_config((GPCamera*)cam,widget,context)<0) {LogAlways("[gphoto] Error setting camera configuration"); return;}

  // Clean up...
   if (gp_widget_free(widget)<0) {LogAlways("[gphoto] Error destroying widget"); return;}
   if (context) gp_context_unref(context);
}

file::ImageRGB * Camera::Capture() const
{
 LogBlock("file::ImageRGB * Camera::Capture() const","-");

 // Before we start we need a context for logging...
  GPContext * context = gp_context_new();
  if (context!=null<GPContext*>())
  {
   gp_context_ref(context);
   gp_context_set_error_func(context,ErrorFunc,0);
   gp_context_set_status_func(context,StatusFunc,0);
   gp_context_set_message_func(context,MessageFunc,0);
  }
  LogDebug("[gphoto] Context created");

 // First step - actually do the capture...
  GPCameraFile * file;
  GPCameraFilePath path;
  if (gp_file_new(&file)<0) {LogAlways("[gphoto] Error creating file object"); return null<file::ImageRGB*>();}
  if (gp_camera_capture((GPCamera*)cam,GP_CAPTURE_IMAGE,&path,context)<0) {LogAlways("[gphoto] Error capturing image"); return null<file::ImageRGB*>();}
  LogDebug("[gphoto] Photo taken, capture is on camera. {folder,file}" << LogDiv() << path.folder << LogDiv() << path.name);
  if (gp_camera_file_get((GPCamera*)cam,path.folder,path.name,GP_FILE_TYPE_NORMAL,file,context)<0) {LogAlways("[gphoto] Error transfering image from camera"); return null<file::ImageRGB*>();}
  LogDebug("[gphoto] Capture performed");

 // Second step, save the captured file to somewhere on disk,
 // this is a major flaw in the gphoto library design...
  cstr filename = file::TemporyFilename();
  if (gp_file_save(file,filename)<0) {LogAlways("[gphoto] Error saving tempory file"); return null<file::ImageRGB*>();}
  LogDebug("[gphoto] Capture saved to tempory file");

 // Third step - load back in the file just saved...
  file::ImageRGB * ret = new file::ImageRGB();
  if (ret->Load(filename)==false)
  {
   LogAlways("[gphoto] Error reloading image");
   delete ret;
   ret = null<file::ImageRGB*>();
  }
  LogDebug("[gphoto] Tempory file reloaded");

 // Delete the files, clean up, and return the result...
  file::DeleteFile(filename);
  if (gp_camera_file_delete((GPCamera*)cam,path.folder,path.name,context)<0) {LogAlways("[gphoto] Could not delete tempory file on camera - you will have to do it yourself");}

  mem::Free(filename);
  if (context) gp_context_unref(context);

 return ret;
}

//------------------------------------------------------------------------------
CameraDetector::CameraDetector()
:ptpAbilities(null<void*>())
{}

CameraDetector::~CameraDetector()
{
 EmptyData();
}

// This has memory leaks on error - should fix sometime.
bit CameraDetector::Refresh()
{
 EmptyData();

 // First find out if the library is loaded, and load it if needed...
  if (LoadGPhoto()==false) return false;

 // Create a context object, so we can capture errors to the log...
  GPContext * context = gp_context_new();
  if (context==0) return false;
  gp_context_ref(context);
  gp_context_set_error_func(context,ErrorFunc,0);
  gp_context_set_status_func(context,StatusFunc,0);
  gp_context_set_message_func(context,MessageFunc,0);

 // Create a list of all cameras...
  GPCameraAbilitiesList * al;
  if (gp_abilities_list_new(&al)<0) {LogAlways("[gphoto] Creating camera list failed"); return false;}
  if (gp_abilities_list_load(al,context)<0) {LogAlways("[gphoto] Loading camera list failed"); return false;}

 // Create a list of all ports avaliable...
  GPPortInfoList * pl;
  if (gp_port_info_list_new(&pl)<0) {LogAlways("[gphoto] Creating port list failed"); return false;}
  if (gp_port_info_list_load(pl)<0) {LogAlways("[gphoto] Loading port list failed"); return false;}

 // Detect all connected cameras...
  GPCameraList * cl;
  if (gp_list_new(&cl)<0) {LogAlways("[gphoto] Creating detected camera list failed"); return false;}
  if (gp_abilities_list_detect(al,pl,cl,context)<0) {LogAlways("[gphoto] Detecting connected cameras failed"); return false;}

 // Copy it all over to our data structure, extracting all needed details...
  int n = gp_list_count(cl);
  if (n<0) {LogAlways("[gphoto] Error getting camera list length"); return false;}
  LogDebug("[gphoto] Found " << int32(n) << " cameras");

  data.Size(n);
  for (int i=0;i<n;i++) {data[i].abilities = 0; data[i].port = 0;}

  for (int i=0;i<n;i++)
  {
   const char * model;
   const char * port;
   if (gp_list_get_name(cl,i,&model)<0) {LogAlways("[gphoto] List error A"); return false;}
   if (gp_list_get_value(cl,i,&port)<0) {LogAlways("[gphoto] List error B"); return false;}
   LogDebug("[gphoto] Found Camera. {model,port}" << LogDiv() << model << LogDiv() << port);

   data[i].abilities = mem::Malloc<GPCameraAbilities>();
   data[i].port = mem::Malloc<GPPortInfo>();

   int aInd = gp_abilities_list_lookup_model(al,model);
   int pInd = gp_port_info_list_lookup_path(pl,port);
   if ((aInd<0)||(pInd<0)) {LogAlways("[gphoto] List error C {aInd,pInd}" << LogDiv() << int32(aInd) << LogDiv() << int32(pInd)); return false;}

   if (gp_abilities_list_get_abilities(al,aInd,(GPCameraAbilities*)data[i].abilities)<0) {LogAlways("[gphoto] List error D"); return false;}
   if (gp_port_info_list_get_info(pl,pInd,(GPPortInfo*)data[i].port)<0) {LogAlways("[gphoto] List error E"); return false;}
  }

 // Seperatly store the ptp camera driver...
  int ptpInd = gp_abilities_list_lookup_model(al,"USB PTP Class Camera");
  if (ptpInd<0) {LogAlways("[gphoto] Error finding ptp driver"); return false;}
  ptpAbilities = mem::Malloc<GPCameraAbilities>();
  if (gp_abilities_list_get_abilities(al,ptpInd,(GPCameraAbilities*)ptpAbilities)<0) {LogAlways("[gphoto] Error getting ptp driver"); return false;}

 // Clean up...
  gp_list_free(cl);
  gp_port_info_list_free(pl);
  gp_abilities_list_free(al);
  gp_context_unref(context);

 return true;
}

nat32 CameraDetector::Size() const
{
 return data.Size();
}

cstrconst CameraDetector::Name(nat32 i) const
{
 return static_cast<GPCameraAbilities*>(data[i].abilities)->model;
}

cstrconst CameraDetector::Port(nat32 i) const
{
 return static_cast<GPPortInfo*>(data[i].port)->path;
}

bit CameraDetector::Search(cstrconst name,cstrconst port,nat32 & out) const
{
 for (out=0;out<data.Size();out++)
 {
  bit ok = true;
   if ((name)&&(str::Compare(Name(out),name)!=0)) ok = false;
   if ((port)&&(str::Compare(Port(out),port)!=0)) ok = false;
  if (ok) return true;
 }
 return false;
}

bit CameraDetector::Link(nat32 index,Camera * cam,bit ptp) const
{
 cam->Deactivate();

 // Create a context...
  GPContext * context = gp_context_new();
  if (context)
  {
   gp_context_ref(context);
   gp_context_set_error_func(context,ErrorFunc,0);
   gp_context_set_status_func(context,StatusFunc,0);
   gp_context_set_message_func(context,MessageFunc,0);
  }

 // Create the camera...
  GPCamera * c;
  if (gp_camera_new(&c)<0) {LogAlways("[gphoto] Error creating camera"); return false;}
  if (gp_camera_set_abilities(c,*(GPCameraAbilities*)(ptp?ptpAbilities:data[index].abilities))<0) {LogAlways("[gphoto] Error setting camera model"); gp_camera_free(c); return false;}
  if (gp_camera_set_port_info(c,*(GPPortInfo*)data[index].port)<0) {LogAlways("[gphoto] Error setting camera port"); gp_camera_free(c); return false;}
  if (gp_camera_init(c,context)<0) {LogAlways("[gphoto] Error connecting camera"); gp_camera_free(c); return false;}

  cam->cam = c;

 if (context) gp_context_unref(context);
 return true;
}

bit CameraDetector::Link(cstrconst name,cstrconst port,Camera * cam,bit ptp) const
{
 nat32 index;
 if (Search(name,port,index)==false) return false;
 return Link(index,cam,ptp);
}

void CameraDetector::EmptyData()
{
 for (nat32 i=0;i<data.Size();i++)
 {
  mem::Free(data[i].abilities);
  mem::Free(data[i].port);
 }
 data.Size(0);

 mem::Free(ptpAbilities);
 ptpAbilities = null<void*>();
}

//------------------------------------------------------------------------------
 };
};
