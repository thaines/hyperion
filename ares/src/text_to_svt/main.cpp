//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


#include "text_to_svt/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;

 // Create basic objects...
  os::Con conObj;
  os::Conversation & con = *conObj.StartConversation();

  str::TokenTable tt;
  svt::Core core(tt);


 // Check we have enough parameters.
  if (argc!=3)
  {
   con << "Usage: text_to_svt [text file] [.obf file]\n";
   return 1;
  }


 // Open the text file...
  file::File<io::Text> textFile(argv[1],file::way_edit,file::mode_read);
  if (!textFile.Active())
  {
   con << "Failed to load text file.\n";
   return 1;
  }
  
  file::Cursor<io::Text> in = textFile.GetCursor();


 // Read in the width and height...
  nat32 width,height;
  in >> width >> height;
  if (in.Error())
  {
   con << "Error reading dimensions.\n";
   return 1;
  }
  con << "width = " << width << "; height = " << height << ";\n";


 // Create the output file data structure...
  svt::Var dispMap(core);
  dispMap.Setup2D(width,height);
  real32 dispIni = 0.0;
  dispMap.Add("disp",dispIni);  
  dispMap.Commit();
  
  svt::Field<real32> disp(&dispMap,"disp");


 // Read in all the data, writting it to the svt data structure as we go...
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    real32 val;
    in >> val;
    if (in.Error())
    {
     con << "Error reading disparity value (" << x << "," << y <<").\n";
     return 1;
    }
    
    disp.Get(x,height-1-y) = val;
   }
  }


 // Save the .svt file...
  if (svt::Save(argv[2],&dispMap,true)==false)
  {
   con << "Error saving .obf file.\n";
   return 1;
  }
  
 
 return 0;
}

//------------------------------------------------------------------------------
