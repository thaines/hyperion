#ifndef FITTER_FIT_ALGS_H
#define FITTER_FIT_ALGS_H
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

// Contains all the algorithm implimentations to be used with the fitter.

#include "fitter.h"

//------------------------------------------------------------------------------
// The pos/dir modifiers...
inline real32 LinearMod(real32 x)
{
 return x;
}

inline real32 SquaredMod(real32 x)
{
 return math::Sqr(x);
}

inline real32 BlakeZissermanMod(real32 x) // Addition to gaussian set to 1.
{
 return -math::Ln(math::Exp(-math::Sqr(x)) + (1.0/math::e));
}

inline real32 PsuedoHuberMod(real32 x) // Angle of straight bits set to 45 degrees.
{
 return 0.5*(math::Sqrt(1.0 + math::Sqr(2.0*x)) - 1.0);
}

//------------------------------------------------------------------------------
// The (centre, radius) encoding...
inline void ToCR(const Sphere & in,math::Vector<real32> & out)
{
 out.SetSize(4);
 out[0] = in.centre[0];
 out[1] = in.centre[1];
 out[2] = in.centre[2]; 
 out[3] = in.radius;
}

inline void FromCR(const math::Vector<real32> & in,Sphere & out)
{
 out.centre[0] = in[0];
 out.centre[1] = in[1];
 out.centre[2] = in[2];
 out.radius = math::Abs(in[3]);
}


inline real32 PosErrAlgCR(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return math::Sqr(pos[0]-sphere[0]) + 
        math::Sqr(pos[1]-sphere[1]) + 
        math::Sqr(pos[2]-sphere[2]) - math::Sqr(sphere[3]);
}

inline real32 PosErrGeoCR(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return math::Sqrt(math::Sqr(pos[0]-sphere[0]) + 
                   math::Sqr(pos[1]-sphere[1]) + 
                   math::Sqr(pos[2]-sphere[2])) - math::Abs(sphere[3]);
}

inline real32 PosErrDepthCR(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 ret = pos[2]-sphere[2];
 real32 xyDist = math::Sqrt(math::Sqr(sphere[0]-pos[0]) + math::Sqr(sphere[1]-pos[1])); 
 if (xyDist<math::Abs(sphere[3]))
 {
  real32 baseHeight = math::Abs(sphere[3])*math::Cos(0.5*math::pi*xyDist/math::Abs(sphere[3]));
  ret = math::Min(ret - baseHeight,ret + baseHeight);
  return ret;
 }
 else
 {
  if (ret<0.0) ret -= xyDist - math::Abs(sphere[3]);
          else ret += xyDist - math::Abs(sphere[3]);
  return ret;
 }
}

inline real32 PosErrPlaneCR(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 relHeight = math::Abs(pos[2] - sphere[2]);
 real32 dist = math::Sqrt(math::Sqr(sphere[0]-pos[0])+math::Sqr(sphere[1]-pos[1]));
 if (relHeight<math::Abs(sphere[3]))
 {
  return dist - math::Abs(sphere[3])*math::Cos(0.5*math::pi*relHeight/math::Abs(sphere[3]));
 }
 else
 {
  return dist + relHeight - math::Abs(sphere[3]);
 }
}

inline real32 DirErrPlaneCR(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 diX = sphere[0] + math::Abs(sphere[3])*dir[2] - dir[0];
 real32 diY = sphere[1] + math::Abs(sphere[3])*dir[3] - dir[1]; 
 return math::Sqrt(math::Sqr(diX) + math::Sqr(diY));
}

inline real32 DirErrPlaneNormCR(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrPlaneCR(sphere,dir,ew)/math::Abs(sphere[3]);
}


inline real32 DirErrAngCR(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 ax = dir[0] - sphere[0];
 real32 ay = dir[1] - sphere[1];
 
 real32 mult = math::InvSqrt(math::Sqr(dir[2]) + math::Sqr(dir[3]));
 mult *= math::InvSqrt(math::Sqr(ax) + math::Sqr(ay));
 
 return math::InvCos(mult*ax*dir[2] + mult*ay*dir[3]);
}

inline real32 DirErrArcCR(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrAngCR(sphere,dir,ew)*math::Abs(sphere[3]);
}

//------------------------------------------------------------------------------
// The (centre, radius squared) encoding...
inline void ToCRS(const Sphere & in,math::Vector<real32> & out)
{
 out.SetSize(4);
 out[0] = in.centre[0];
 out[1] = in.centre[1];
 out[2] = in.centre[2]; 
 out[3] = in.radius*in.radius;
}

inline void FromCRS(const math::Vector<real32> & in,Sphere & out)
{
 out.centre[0] = in[0];
 out.centre[1] = in[1];
 out.centre[2] = in[2];
 out.radius = math::Sqrt(math::Abs(in[3]));
}


inline real32 PosErrAlgCRS(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return math::Sqr(pos[0]-sphere[0]) + 
        math::Sqr(pos[1]-sphere[1]) + 
        math::Sqr(pos[2]-sphere[2]) - math::Abs(sphere[3]);
}

inline real32 PosErrGeoCRS(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return math::Sqrt(math::Sqr(pos[0]-sphere[0]) + 
                   math::Sqr(pos[1]-sphere[1]) + 
                   math::Sqr(pos[2]-sphere[2])) - math::Sqrt(math::Abs(sphere[3]));
}

inline real32 PosErrDepthCRS(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 ret = pos[2]-sphere[2];
 real32 xyDist = math::Sqrt(math::Sqr(sphere[0]-pos[0]) + math::Sqr(sphere[1]-pos[1])); 
 if (xyDist<math::Sqrt(math::Abs(sphere[3])))
 {
  real32 baseHeight = math::Sqrt(math::Abs(sphere[3]))*math::Cos(0.5*math::pi*xyDist/math::Sqrt(math::Abs(sphere[3])));
  ret = math::Min(ret - baseHeight,ret + baseHeight);
  return ret;
 }
 else
 {
  if (ret<0.0) ret -= xyDist - math::Sqrt(math::Abs(sphere[3]));
          else ret += xyDist - math::Sqrt(math::Abs(sphere[3]));
  return ret;
 }
}

inline real32 PosErrPlaneCRS(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 relHeight = math::Abs(pos[2] - sphere[2]);
 real32 dist = math::Sqrt(math::Sqr(sphere[0]-pos[0])+math::Sqr(sphere[1]-pos[1]));
 if (relHeight<math::Sqrt(math::Abs(sphere[3])))
 {
  return dist - math::Sqrt(math::Abs(sphere[3]))*math::Cos(0.5*math::pi*relHeight/math::Sqrt(math::Abs(sphere[3])));
 }
 else
 {
  return dist + relHeight - math::Sqrt(math::Abs(sphere[3]));
 }
}

inline real32 DirErrPlaneCRS(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 diX = sphere[0] + math::Sqrt(math::Abs(sphere[3]))*dir[2] - dir[0];
 real32 diY = sphere[1] + math::Sqrt(math::Abs(sphere[3]))*dir[3] - dir[1]; 
 return math::Sqrt(math::Sqr(diX) + math::Sqr(diY));
}

inline real32 DirErrPlaneNormCRS(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrPlaneCRS(sphere,dir,ew)/math::Sqrt(math::Abs(sphere[3]));
}

inline real32 DirErrAngCRS(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 ax = dir[0] - sphere[0];
 real32 ay = dir[1] - sphere[1];
 
 real32 mult = math::InvSqrt(math::Sqr(dir[2]) + math::Sqr(dir[3]));
 mult *= math::InvSqrt(math::Sqr(ax) + math::Sqr(ay));
 
 return math::InvCos(mult*ax*dir[2] + mult*ay*dir[3]);
}

inline real32 DirErrArcCRS(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrAngCRS(sphere,dir,ew)*math::Sqrt(math::Abs(sphere[3]));
}

//------------------------------------------------------------------------------
// The (centre, one over radius) encoding...
inline void ToCRDO(const Sphere & in,math::Vector<real32> & out)
{
 out.SetSize(4);
 out[0] = in.centre[0];
 out[1] = in.centre[1];
 out[2] = in.centre[2]; 
 out[3] = 1.0/in.radius;
}

inline void FromCRDO(const math::Vector<real32> & in,Sphere & out)
{
 out.centre[0] = in[0];
 out.centre[1] = in[1];
 out.centre[2] = in[2];
 out.radius = 1.0/math::Abs(in[3]);
}


inline real32 PosErrAlgCRDO(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return math::Sqr(pos[0]-sphere[0]) + 
        math::Sqr(pos[1]-sphere[1]) + 
        math::Sqr(pos[2]-sphere[2]) - math::Sqr(1.0/math::Abs(sphere[3]));
}

inline real32 PosErrGeoCRDO(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return math::Sqrt(math::Sqr(pos[0]-sphere[0]) + 
                   math::Sqr(pos[1]-sphere[1]) + 
                   math::Sqr(pos[2]-sphere[2])) - 1.0/math::Abs(sphere[3]);
}

inline real32 PosErrDepthCRDO(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 ret = pos[2]-sphere[2];
 real32 xyDist = math::Sqrt(math::Sqr(sphere[0]-pos[0]) + math::Sqr(sphere[1]-pos[1])); 
 if (xyDist<(1.0/math::Abs(sphere[3])))
 {
  real32 baseHeight = (1.0/math::Abs(sphere[3]))*math::Cos(0.5*math::pi*xyDist/(1.0/math::Abs(sphere[3])));
  ret = math::Min(ret - baseHeight,ret + baseHeight);
  return ret;
 }
 else
 {
  if (ret<0.0) ret -= xyDist - (1.0/math::Abs(sphere[3]));
          else ret += xyDist - (1.0/math::Abs(sphere[3]));
  return ret;
 }
}

inline real32 PosErrPlaneCRDO(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 relHeight = math::Abs(pos[2] - sphere[2]);
 real32 dist = math::Sqrt(math::Sqr(sphere[0]-pos[0])+math::Sqr(sphere[1]-pos[1]));
 if (relHeight<(1.0/math::Abs(sphere[3])))
 {
  return dist - (1.0/math::Abs(sphere[3]))*math::Cos(0.5*math::pi*relHeight/(1.0/math::Abs(sphere[3])));
 }
 else
 {
  return dist + relHeight - (1.0/math::Abs(sphere[3]));
 }
}

inline real32 DirErrPlaneCRDO(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 diX = sphere[0] + (1.0/math::Abs(sphere[3]))*dir[2] - dir[0];
 real32 diY = sphere[1] + (1.0/math::Abs(sphere[3]))*dir[3] - dir[1]; 
 return math::Sqrt(math::Sqr(diX) + math::Sqr(diY));
}

inline real32 DirErrPlaneNormCRDO(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrPlaneCRDO(sphere,dir,ew)/(1.0/math::Abs(sphere[3]));
}

inline real32 DirErrAngCRDO(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 ax = dir[0] - sphere[0];
 real32 ay = dir[1] - sphere[1];
 
 real32 mult = math::InvSqrt(math::Sqr(dir[2]) + math::Sqr(dir[3]));
 mult *= math::InvSqrt(math::Sqr(ax) + math::Sqr(ay));
 
 return math::InvCos(mult*ax*dir[2] + mult*ay*dir[3]);
}

inline real32 DirErrArcCRDO(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrAngCRDO(sphere,dir,ew)*(1.0/math::Abs(sphere[3]));
}

//------------------------------------------------------------------------------
// The implicit a(x^2+y^2+z^2) + bx + cy + dz + e = 0 encoding...
inline void ToIM(const Sphere & in,math::Vector<real32> & out)
{
 out.SetSize(5);
 out[0] = 1.0;
 out[1] = -2.0*in.centre[0];
 out[2] = -2.0*in.centre[1];
 out[3] = -2.0*in.centre[2];
 out[4] = math::Sqr(in.centre[0]) + math::Sqr(in.centre[1]) + math::Sqr(in.centre[2]) - math::Sqr(in.radius);
}

inline void FromIM(const math::Vector<real32> & in,Sphere & out)
{
 out.centre[0] = -0.5*(in[1]/in[0]);
 out.centre[1] = -0.5*(in[2]/in[0]);
 out.centre[2] = -0.5*(in[3]/in[0]);
 out.radius = math::Sqrt(math::Sqr(out.centre[0]) + math::Sqr(out.centre[1]) + math::Sqr(out.centre[2]) - (in[4]/in[0]));
}


inline real32 PosErrAlgIM(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 return sphere[0]*(math::Sqr(pos[0]) + math::Sqr(pos[1]) + math::Sqr(pos[2])) +
        sphere[1]*pos[0] + sphere[2]*pos[1] + sphere[3]*pos[2] + sphere[4];
}

inline real32 PosErrGeoIM(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 mult = 1.0/sphere[0];
 real32 dx = -0.5*sphere[1]*mult - pos[0];
 real32 dy = -0.5*sphere[2]*mult - pos[1];
 real32 dz = -0.5*sphere[3]*mult - pos[2];
 real32 r  = math::Sqrt(0.25*math::Sqr(mult)*(math::Sqr(sphere[1]) + math::Sqr(sphere[2]) + math::Sqr(sphere[3])) - sphere[4]*mult);
 return math::Sqrt(math::Sqr(dx)+math::Sqr(dy)+math::Sqr(dz)) - r;
}

inline real32 PosErrDepthIM(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 px = -0.5*(sphere[1]/sphere[0]);
 real32 py = -0.5*(sphere[2]/sphere[0]);
 real32 pz = -0.5*(sphere[3]/sphere[0]);
 real32 r  = math::Sqrt(math::Sqr(px) + math::Sqr(py) + math::Sqr(pz) - (sphere[4]/sphere[0]));
 
 real32 ret = pos[2]-pz;
 real32 xyDist = math::Sqrt(math::Sqr(px-pos[0]) + math::Sqr(py-pos[1])); 
 if (xyDist<r)
 {
  real32 baseHeight = r*math::Cos(0.5*math::pi*xyDist/r);
  ret = math::Min(ret - baseHeight,ret + baseHeight);
  return ret;
 }
 else
 {
  if (ret<0.0) ret -= xyDist - r;
          else ret += xyDist - r;
  return ret;
 }
}

inline real32 PosErrPlaneIM(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 px = -0.5*(sphere[1]/sphere[0]);
 real32 py = -0.5*(sphere[2]/sphere[0]);
 real32 pz = -0.5*(sphere[3]/sphere[0]);
 real32 r  = math::Sqrt(math::Sqr(px) + math::Sqr(py) + math::Sqr(pz) - (sphere[4]/sphere[0]));
 
 real32 relHeight = math::Abs(pos[2] - pz);
 real32 dist = math::Sqrt(math::Sqr(px-pos[0])+math::Sqr(py-pos[1]));
 if (relHeight<r)
 {
  return dist - r*math::Cos(0.5*math::pi*relHeight/r);
 }
 else
 {
  return dist + relHeight - r;
 }
}

inline real32 DirErrPlaneIM(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 px = -0.5*(sphere[1]/sphere[0]);
 real32 py = -0.5*(sphere[2]/sphere[0]);
 real32 pz = -0.5*(sphere[3]/sphere[0]);
 real32 r  = math::Sqrt(math::Sqr(px) + math::Sqr(py) + math::Sqr(pz) - (sphere[4]/sphere[0]));
 
 real32 diX = px + r*dir[2] - dir[0];
 real32 diY = py + r*dir[3] - dir[1]; 
 return math::Sqrt(math::Sqr(diX) + math::Sqr(diY));
}

inline real32 DirErrPlaneNormIM(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 real32 px = -0.5*(sphere[1]/sphere[0]);
 real32 py = -0.5*(sphere[2]/sphere[0]);
 real32 pz = -0.5*(sphere[3]/sphere[0]);
 real32 r  = math::Sqrt(math::Sqr(px) + math::Sqr(py) + math::Sqr(pz) - (sphere[4]/sphere[0]));
 return DirErrPlaneIM(sphere,dir,ew)/r;
}

inline real32 DirErrAngIM(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 px = -0.5*(sphere[1]/sphere[0]);
 real32 py = -0.5*(sphere[2]/sphere[0]);

 real32 ax = dir[0] - px;
 real32 ay = dir[1] - py; 
 real32 mult = math::InvSqrt(math::Sqr(dir[2]) + math::Sqr(dir[3]));
 mult *= math::InvSqrt(math::Sqr(ax) + math::Sqr(ay));
 
 return math::InvCos(mult*ax*dir[2] + mult*ay*dir[3]);
}

inline real32 DirErrArcIM(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 real32 px = -0.5*(sphere[1]/sphere[0]);
 real32 py = -0.5*(sphere[2]/sphere[0]);
 real32 pz = -0.5*(sphere[3]/sphere[0]);
 real32 r  = math::Sqrt(math::Sqr(px) + math::Sqr(py) + math::Sqr(pz) - (sphere[4]/sphere[0]));
 return DirErrAngIM(sphere,dir,ew)*r;
}

//------------------------------------------------------------------------------
// The direction and distance from origin to surface and radius encoding...
inline void ToNC(const Sphere & in,math::Vector<real32> & out)
{
 out.SetSize(4);
 out[2] = math::Sqrt(math::Sqr(in.centre[0])+math::Sqr(in.centre[1])+math::Sqr(in.centre[2]));
 if (math::Equal(in.centre[0],real32(0.0))&&math::Equal(in.centre[1],real32(0.0))) out[0] = 0.0;
    else out[0] = math::InvTan2(in.centre[1],in.centre[0]);
 if (math::Equal(out[2],real32(0.0))) out[1] = 0.0;
    else out[1] = math::InvCos(in.centre[2]/out[2]);
 
 out[2] -= in.radius;
 out[3] = in.radius;
}

inline void FromNC(const math::Vector<real32> & in,Sphere & out)
{
 real32 dist = in[2] + in[3];
 out.centre[0] = dist*math::Cos(in[0])*math::Sin(in[1]);
 out.centre[1] = dist*math::Sin(in[0])*math::Sin(in[1]);
 out.centre[2] = dist*math::Cos(in[1]);
 out.radius = math::Abs(in[3]);
}


inline real32 PosErrAlgNC(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 r = math::Abs(sphere[3]);
 real32 dist = sphere[2] + sphere[3];
 real32 cx = dist*math::Cos(sphere[0])*math::Sin(sphere[1]);
 real32 cy = dist*math::Sin(sphere[0])*math::Sin(sphere[1]);
 real32 cz = dist*math::Cos(sphere[1]);
 
 return math::Sqr(cx-pos[0]) + math::Sqr(cy-pos[1]) + math::Sqr(cz-pos[2]) - math::Sqr(r);
}

inline real32 PosErrGeoNC(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 r = math::Abs(sphere[3]);
 real32 dist = sphere[2] + sphere[3];
 real32 cx = dist*math::Cos(sphere[0])*math::Sin(sphere[1]);
 real32 cy = dist*math::Sin(sphere[0])*math::Sin(sphere[1]);
 real32 cz = dist*math::Cos(sphere[1]);
 
 return math::Sqrt(math::Sqr(cx-pos[0]) + math::Sqr(cy-pos[1]) + math::Sqr(cz-pos[2])) - r;
}

inline real32 PosErrDepthNC(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 r = math::Abs(sphere[3]);
 real32 dist = sphere[2] + sphere[3];
 real32 cx = dist*math::Cos(sphere[0])*math::Sin(sphere[1]);
 real32 cy = dist*math::Sin(sphere[0])*math::Sin(sphere[1]);
 real32 cz = dist*math::Cos(sphere[1]);;

 real32 ret = pos[2]-cz;
 real32 xyDist = math::Sqrt(math::Sqr(cx-pos[0]) + math::Sqr(cy-pos[1])); 
 if (xyDist<r)
 {
  real32 baseHeight = r*math::Cos(0.5*math::pi*xyDist/r);
  ret = math::Min(ret - baseHeight,ret + baseHeight);
  return ret;
 }
 else
 {
  if (ret<0.0) ret -= xyDist - r;
          else ret += xyDist - r;
  return ret;
 }
}

inline real32 PosErrPlaneNC(const math::Vector<real32> & sphere,const math::Vect<3> & pos)
{
 real32 r = math::Abs(sphere[3]);
 real32 dist = sphere[2] + sphere[3];
 real32 cx = dist*math::Cos(sphere[0])*math::Sin(sphere[1]);
 real32 cy = dist*math::Sin(sphere[0])*math::Sin(sphere[1]);
 real32 cz = dist*math::Cos(sphere[1]);
 
 real32 relHeight = math::Abs(pos[2] - cz);
 real32 pd = math::Sqrt(math::Sqr(cx-pos[0])+math::Sqr(cy-pos[1]));
 if (relHeight<r)
 {
  return pd - r*math::Cos(0.5*math::pi*relHeight/r);
 }
 else
 {
  return pd + relHeight - r;
 }
}

inline real32 DirErrPlaneNC(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 r = math::Abs(sphere[3]);
 real32 dist = sphere[2] + sphere[3];
 real32 cx = dist*math::Cos(sphere[0])*math::Sin(sphere[1]);
 real32 cy = dist*math::Sin(sphere[0])*math::Sin(sphere[1]);

 real32 diX = cx + r*dir[2] - dir[0];
 real32 diY = cy + r*dir[3] - dir[1]; 
 return math::Sqrt(math::Sqr(diX) + math::Sqr(diY));
}

inline real32 DirErrPlaneNormNC(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrPlaneNC(sphere,dir,ew)/math::Abs(sphere[3]);
}


inline real32 DirErrAngNC(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32)
{
 real32 dist = sphere[2] + sphere[3];
 real32 cx = dist*math::Cos(sphere[0])*math::Sin(sphere[1]);
 real32 cy = dist*math::Sin(sphere[0])*math::Sin(sphere[1]);

 real32 ax = dir[0] - cx;
 real32 ay = dir[1] - cy;
 
 real32 mult = math::InvSqrt(math::Sqr(dir[2]) + math::Sqr(dir[3]));
 mult *= math::InvSqrt(math::Sqr(ax) + math::Sqr(ay));
 
 return math::InvCos(mult*ax*dir[2] + mult*ay*dir[3]);
}

inline real32 DirErrArcNC(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)
{
 return DirErrAngNC(sphere,dir,ew)*math::Abs(sphere[3]);
}

//------------------------------------------------------------------------------
#endif
