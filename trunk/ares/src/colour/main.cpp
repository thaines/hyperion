//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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
