#ifndef EOS_REND_FUNCTIONS_H
#define EOS_REND_FUNCTIONS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::rend
/// Non-real-time rendering capability, everything from simple line drawing to
/// raytracers and global illumination rendering.

/// \file rend/functions.h
/// Provides various simple functions for rendering to images and other
/// helper functions.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"
#include "eos/bs/geo2d.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Sets a pixel, does nothing if given an out of bound value.
template <typename CT>
inline void SetPixel(svt::Field<CT> & out,const bs::Pnt & pix,const CT & col)
{
 int32 x = int32(math::Round(pix.X()));
 int32 y = int32(math::Round(pix.Y()));
 if ((x<0)||(x>=int32(out.Size(0)))||(y<0)||(y>=int32(out.Size(1)))) return;
 out.Get(x,y) = col;
}

/// Renders a line, will handle out of bounds coordinates correctly.
template <typename CT>
inline void Line(svt::Field<CT> & out,const bs::Pnt & start,const bs::Pnt & end,const CT & col)
{
 int32 x1 = int32(math::Round(start.X()));
 int32 y1 = int32(math::Round(start.Y()));
 int32 x2 = int32(math::Round(end.X()));
 int32 y2 = int32(math::Round(end.Y()));
 
 // Check for degenerate situations - ie horizontal and vertical lines...
  if (x1==x2)
  {
   if (y1>y2) for (int32 y=y2;y<=y1;y++) SetPixel(out,bs::Pnt(x1,y),col);
         else for (int32 y=y1;y<=y2;y++) SetPixel(out,bs::Pnt(x1,y),col);
   return;
  }
  if (y1==y2)
  {
   if (x1>x2) for (int32 x=x2;x<=x1;x++) SetPixel(out,bs::Pnt(x,y1),col);
         else for (int32 x=x1;x<=x2;x++) SetPixel(out,bs::Pnt(x,y1),col);
   return;	  
  }
 
 	
 // The variables required...
  int32 slope;
  int32 dx, dy, incE, incNE, d, x, y;
  
 // Decide if we need to do x/y or y/x, and do one or the other...
  if (math::Abs(x1-x2)>abs(y1-y2))
  {
   if (x1 > x2)
   {
    int32 t = x1; x1 = x2; x2 = t;
    t = y1; y1 = y2; y2 = t;
   }
   
   dx = x2 - x1;
   dy = y2 - y1;
   
   if (dy < 0)
   {
    slope = -1;
    dy = -dy;
   }
   else slope = 1;

   // Setup...
    incE = 2 * dy;
    incNE = 2 * dy - 2 * dx;
    d = 2 * dy - dx;
    y = y1;
   
   // Run...
    for (x = x1; x <= x2; x++)
    {
     SetPixel(out,bs::Pnt(x,y),col);
     if (d <= 0)
     {
      d += incE;
     }
     else
     {
      d += incNE;
      y += slope;
     }
    }	  	  
  }
  else
  {
   if (y1 > y2)
   {
    int32 t = x1; x1 = x2; x2 = t;
    t = y1; y1 = y2; y2 = t;
   }
   
   dx = x2 - x1;
   dy = y2 - y1;
   
   if (dx < 0)
   {
    slope = -1;
    dx = -dx;
   }
   else slope = 1;

   // Setup...
    incE = 2 * dx;
    incNE = 2 * dx - 2 * dy;
    d = 2 * dx - dy;
    x = x1;
   
   // Run...
    for (y = y1; y <= y2; y++)
    {
     SetPixel(out,bs::Pnt(x,y),col);
     if (d <= 0)
     {
      d += incE;
     }
     else
     {
      d += incNE;
      x += slope;
     }
    }	  	  
  }
}

/// Renders an arrow, handles out of bounds correctly. The arrow head has lines comming off 
/// at 45 degrees. size gives the length of the arror heads as a multiplier of line length,
/// defaults to 5%.
template <typename CT>
inline void Arrow(svt::Field<CT> & out,const bs::Pnt & start,const bs::Pnt & end,const CT & col,real32 size = 0.05)
{
 rend::Line(out,start,end,col);
 
 math::Vect<2> back;
  back[0] = (start.X() - end.X())*size*math::Sqrt(2);
  back[1] = (start.Y() - end.Y())*size*math::Sqrt(2);
 
 math::Vect<2> arrA;
  arrA[0] = back[0] + back[1];
  arrA[1] = back[1] - back[0];
  
 math::Vect<2> arrB;
  arrB[0] = back[0] - back[1];
  arrB[1] = back[1] + back[0];  
  
 rend::Line(out,end,bs::Pnt(end.X()+arrA[0],end.Y()+arrA[1]),col);
 rend::Line(out,end,bs::Pnt(end.X()+arrB[0],end.Y()+arrB[1]),col);
}

//------------------------------------------------------------------------------
 };
};
#endif
