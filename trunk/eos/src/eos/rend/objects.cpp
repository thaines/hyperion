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

#include "eos/rend/objects.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void Sphere::Bound(bs::Sphere & out) const
{
 out.c = bs::Vert(0.0,0.0,0.0);
 out.r = 1.0;
}

void Sphere::Bound(bs::Box & out) const
{
 out.c = bs::Vert(-1.0,-1.0,-1.0);
 out.e[0][0] = 2.0; out.e[0][0] = 0.0; out.e[0][0] = 0.0;
 out.e[0][0] = 0.0; out.e[0][0] = 2.0; out.e[0][0] = 0.0;
 out.e[0][0] = 0.0; out.e[0][0] = 0.0; out.e[0][0] = 2.0;
}

bit Sphere::Inside(const bs::Vert & point) const
{
 return point.LengthSqr()<1.0;
}

bit Sphere::Intercept(const bs::Ray & ray,real32 & dist) const
{
 bs::Sphere self;
  self.c = bs::Vert(0.0,0.0,0.0);
  self.r = 1.0;
  
 return self.Intercept(ray,&dist);
}

bit Sphere::Intercept(const bs::Ray & ray,Intersection & out) const
{
 bs::Sphere self;
  self.c = bs::Vert(0.0,0.0,0.0);
  self.r = 1.0;

 if (self.Intercept(ray,out.depth,out.point))
 {
  real32 theta = math::InvTan2(out.point[1],out.point[0]);
  real32 rho = math::InvCos(out.point[2]);
  
  out.norm = out.point;
  
  out.axis[1] = bs::Normal(0.0,1.0,0.0);
  math::CrossProduct(out.axis[1],out.norm,out.axis[0]);
  math::CrossProduct(out.norm,out.axis[0],out.axis[1]);
  
  out.coord[0] = 0.5 + theta/(2.0*math::pi);
  out.coord[1] = 0.5 + rho/math::pi;
  out.coord.material = 0;

  out.velocity = velocity;
  return true;
 }
 else return false;
}

bit Sphere::Intercept(const bs::FiniteLine & line) const
{
 bs::Sphere self;
  self.c = bs::Vert(0.0,0.0,0.0);
  self.r = 1.0;
   
 real32 dist;
 bs::Ray ray;
  ray.s = line.s;
  ray.n = line.e;
  ray.n -= line.s;

 real32 length = ray.n.Length();
 ray.n /= length;
 
 return (self.Intercept(ray,&dist))&&(dist<length);
}

//------------------------------------------------------------------------------
 };
};
