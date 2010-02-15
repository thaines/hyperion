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
 
 if (exif.HasShutterTime()) con << "Shutter time = " << exif.ShutterTime() << "s (1/" << (1.0/exif.ShutterTime()) << "s)\n";

 return 0;
}

//------------------------------------------------------------------------------
