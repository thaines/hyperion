//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


#include <stdio.h>
#include <iostream>

#include "rend/main.h"

using namespace eos;

//------------------------------------------------------------------------------
class MyProg : public eos::time::Progress
{
 public:
  MyProg():lastUpdate(0) {}

  void OnChange()
  {
   nat64 now = eos::time::MilliTime();
   if (now-100<lastUpdate) return;
   lastUpdate = now;

   nat32 x,y;
   Part(Depth()-1,x,y);
   for (nat32 i=0;i<Depth();i++)
   {
    nat32 x,y;
    Part(i,x,y);
    std::cout << i << ": [" << x << " of " << y << "]\n";
   }
   std::cout << "\n";

  }
  
 private:
  nat64 lastUpdate;
};

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 MyProg prog;

 if (argc<2)
 {
  std::cout << "Usage: rend [scene.xml]\n";
  std::cout << "Outputs the final rendered image, including a SVT object with no loss of detail.\n";
  return 1;
 }

 str::TokenTable tt;
 svt::Core core(tt);


 // Load the XML file...
  std::cout << "Loading scene description...\n";
  mem::StackPtr<bs::Element> scene = file::LoadXML(tt,argv[1]);
  if (scene==null<bs::Element*>())
  {
   std::cout << "Error loading XML file.\n";
   return 1;
  }


 // Convert it into a set of render jobs...
  std::cout << "Analysing scene description...\n";
  rend::Scene sceneDesc;
  if (sceneDesc.Load(scene)==false)
  {
   std::cout << "Error parsing scene description.\n";
   return 1;
  }


 // Do the job(s), saving the output as we go along...
  std::cout << "Rendering...\n";
  for (nat32 i=0;i<sceneDesc.Jobs();i++)
  {
   prog.Report(i,sceneDesc.Jobs());
   
   // ********************************
   
  }


 return 0;
}

//------------------------------------------------------------------------------
