//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


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
