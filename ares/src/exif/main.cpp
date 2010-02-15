//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "exif/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();

 if (argc<2)
 {
  con << "Usage:\nexif [image]\n";
  con << "Prints out recognised exif information\n";
  return 1;
 }


 file::Exif exif;
 
 if (exif.Load(argv[1])==false)
 {
  con << "Error loading jpeg\n";
  return 1;
 }
 
 con << "Found " << exif.TagCount() << " exif tags\n";

 if (exif.HasMake())
    con << "Make = " << exif.GetMake() << "\n";

 if (exif.HasModel())
    con << "Model = " << exif.GetModel() << "\n";

 if (exif.HasDateTime())
    con << "Date/time = " << exif.GetDateTime() << "\n";
 
 if (exif.HasExposureTime())
    con << "Exposure time = " << exif.GetExposureTime() << "s (1/" << (1.0/exif.GetExposureTime()) << "s)\n";
 
 if (exif.HasFStop())
    con << "F-Stop = " << exif.GetFStop() << "\n";

 if (exif.HasISO())
    con << "ISO = " << exif.GetISO() << "\n";
    
 if (exif.HasFocalLength())
    con << "Focal Length = " << exif.GetFocalLength() << "mm\n";

 if (exif.HasFlash())
 {
  if (exif.GetFlash()) con << "Flash was used\n";
                  else con << "Flash was not used\n";
 }

 return 0;
}

//------------------------------------------------------------------------------
