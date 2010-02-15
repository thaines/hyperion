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


#include "mg_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 data::Random rand;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();

 static const real32  mu = 1.0;

 con << "Preping multigrid...\n";
 alg::Multigrid2D mg;
 mg.SetSize(83,101,5);
 
 mg.SetOffset(0,0, 0, 0);
 mg.SetOffset(0,1, 1, 0);
 mg.SetOffset(0,2, 0, 1);
 mg.SetOffset(0,3,-1, 0);
 mg.SetOffset(0,4, 0,-1);
 mg.SpreadFirstStencil();
 
 for (nat32 l=0;l<mg.Levels();l++)
 {
  real32 h = math::Pow<real32>(2,l);
  real32 invH2 = 1.0/math::Sqr(h);
  for (nat32 y=0;y<mg.Height(l);y++)
  {
   for (nat32 x=0;x<mg.Width(l);x++)
   {
    mg.SetA(l,x,y,0,4.0*invH2 + mu);
    mg.SetA(l,x,y,1,-invH2);
    mg.SetA(l,x,y,2,-invH2);
    mg.SetA(l,x,y,3,-invH2);
    mg.SetA(l,x,y,4,-invH2);
   }
  }
 }
 
 for (nat32 y=0;y<mg.Height(0);y++)
 {
  for (nat32 x=0;x<mg.Width(0);x++) mg.SetB(0,x,y,0.0);
 }
 
 mg.SetB(0,mg.Width(0)/3,mg.Height(0)/3,10.0);
 mg.SetB(0,2*mg.Width(0)/3,2*mg.Height(0)/3,60.0);


 con << "Solving multigrid...\n";
 mg.ReRun(&con.BeginProg());
 con.EndProg();


 con << "Solution:\n";
 for (nat32 y=0;y<mg.Height(0);y++)
 {
  for (nat32 x=0;x<mg.Width(0);x++) con << mg.Get(x,y) << " ";
  con << "\n";
 }
 con << "\n";

 return 0;
}

//------------------------------------------------------------------------------

