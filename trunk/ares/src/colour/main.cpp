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

#include "colour/main.h"

using namespace eos;

//------------------------------------------------------------------------------
void DoColour(const bs::ColourRGB & rgb)
{
 bs::ColourL l = rgb;
 bs::ColourLuv luv = rgb;
 bs::ColourXYZ xyz = rgb;

 printf("Colour RGB: (%f,%f,%f)\n",rgb.r,rgb.g,rgb.b);
 printf("Colour L: (%f)\n",l.l);
 printf("Colour Luv: (%f,%f,%f)\n",luv.l,luv.u,luv.v);
 printf("Colour XYZ: (%f,%f,%f)\n",xyz.x,xyz.y,xyz.z);
 printf("\n");
}

int main()
{
 DoColour(bs::ColourRGB(0.0,0.0,0.0));
 DoColour(bs::ColourRGB(1.0,0.0,0.0));
 DoColour(bs::ColourRGB(0.0,1.0,0.0));
 DoColour(bs::ColourRGB(0.0,0.0,1.0));
 DoColour(bs::ColourRGB(0.0,1.0,1.0));
 DoColour(bs::ColourRGB(1.0,0.0,1.0));
 DoColour(bs::ColourRGB(1.0,1.0,0.0));
 DoColour(bs::ColourRGB(1.0,1.0,1.0));
 
 return 0;
}

//------------------------------------------------------------------------------
