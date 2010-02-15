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


#include "bp_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 
 str::TokenTable tt;
 svt::Core core(tt);
  
 inf::BinBP2D bp;
 bp.SetSize(32,32);
 bp.SetMomentum(0.01);


 for (nat32 y=0;y<bp.Height();y++)
 {
  for (nat32 x=0;x<bp.Width();x++)
  {
   bp.SetCost(x,y,(((x/2)+(y/2))%2)==0,1.5);
   bp.SetCostX(x,y,false,true,1.0);
   bp.SetCostX(x,y,true,false,1.0);
   bp.SetCostY(x,y,false,true,1.0);
   bp.SetCostY(x,y,true,false,1.0);
  }
 } 
 
 
 bp.Run(&con.BeginProg());
 con.EndProg();


 for (nat32 y=0;y<bp.Height();y++)
 {
  con << "|";
  for (nat32 x=0;x<bp.Width();x++)
  {
   if (bp.State(x,y)) con << "+";
                 else con << " ";
   con << "|";
  }
  con << "\n";
 }
 con << "\n";


 return 0;
}

//------------------------------------------------------------------------------

