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


#include "bessel/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 
 if (argc!=3)
 {
  con << "Calculates the modified bessel function of the first kind.\n";
  con << "Usage: bessel order value\n";
  con << "order - The order, times two, an integer.\n";
  con << "value - The value to calculate the bessel function for\n.";
 }
 
 nat32 orderX2 = str::ToInt32(argv[1]);
 real32 x = str::ToReal32(argv[2]);
 
 real32 res = math::ModBesselFirst(orderX2,x);
 con << res << "\n";
 
 if (orderX2==0)
 {
  real32 ires = math::InverseModBesselFirstOrder0(res);
  con << "(Input by inverse " << ires << ")\n";
 }


 return 0;
}

//------------------------------------------------------------------------------

