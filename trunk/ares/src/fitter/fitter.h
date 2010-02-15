#ifndef FITTER_FITTER_H
#define FITTER_FITTER_H
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

// Contains the fitting object, templated by all the algorithmic parts required 
// to make the relevent fitter, with support stuff as well.

#include "eos.h"

using namespace eos;

//------------------------------------------------------------------------------
// A sphere representation. Must be like the 5th I've coded.
struct Sphere
{
 math::Vect<3> centre;
 real32 radius;
 
 // Creates a random sphere in this structure.
 void MakeRandom(data::Random & rand,const math::Range & pos,const math::Range & rad);
};

//------------------------------------------------------------------------------
// A data set to fit a sphere to.
struct DataSet
{
 Sphere s; // The actual sphere used to generate the data set. No cheating.
 real32 dw; // Direction weighting, to weight the direction error compared to the position error, which is fixed at 1.
 real32 ew; // An extra parameter for direction error metrics, some require it.

 ds::Array< math::Vect<3> > pos; // x,y,z.
 ds::Array< math::Vect<5> > dir; // x,y,normal. (No Z.)

 // Creates a set of samples, longitude and latitude specify the range of the
 // sphere to sample over.
  void Build(data::Random & rand,const Sphere & s,nat32 posSamples,nat32 dirSamples,
             const math::Range & latitude,const math::Range & longitude);

 // Adds noise, pos is noise in x,y coordinates, depth is noise in z and
 // norm is noise in the normals. All addative gaussian. Normal noise is added
 // to each component then the normal is re-normalised.
  void AddNoise(data::Random & rand,real32 pos,real32 depth,real32 norm);
};

//------------------------------------------------------------------------------
// This indicates a specific algorithm, a group of string pointers for each
// category...
struct AlgRep
{
 AlgRep()
 :best(1e100),rep(null<cstr>()),pos(null<cstr>()),dir(null<cstr>()),posMod(null<cstr>()),dirMod(null<cstr>())
 {}


 real32 best;
 
 cstr rep;
 cstr pos;
 cstr dir;
 cstr posMod;
 cstr dirMod;
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0) 
 {
  out.Pad(ident); out << "Representation = " << rep << "\n";
  out.Pad(ident); out << "Position Error = " << pos << "\n";
  out.Pad(ident); out << "Normal Error   = " << dir << "\n";
  out.Pad(ident); out << "Position Mod   = " << posMod << "\n";
  out.Pad(ident); out << "Normal Mod     = " << dirMod << "\n";
 }
};

//------------------------------------------------------------------------------
// This represents the best algorithm for each of the 4 measurement classes, as
// measured by mean and by mean + sd...
struct BestAlg
{
 AlgRep rep[2][4];
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Best Mean,\n";
  out.Pad(ident+2); out << "by Residual:\n"; rep[0][0].Print(out,ident+4);
  out.Pad(ident+2); out << "by Normalised Residual:\n"; rep[0][1].Print(out,ident+4);
  out.Pad(ident+2); out << "by Centre Distance:\n"; rep[0][2].Print(out,ident+4);
  out.Pad(ident+2); out << "by Radius Difference:\n"; rep[0][3].Print(out,ident+4);
  
  out.Pad(ident); out << "Best Mean + Sd,\n";
  out.Pad(ident+2); out << "by Residual:\n"; rep[1][0].Print(out,ident+4);
  out.Pad(ident+2); out << "by Normalised Residual:\n"; rep[1][1].Print(out,ident+4);
  out.Pad(ident+2); out << "by Centre Distance:\n"; rep[1][2].Print(out,ident+4);
  out.Pad(ident+2); out << "by Radius Difference:\n"; rep[1][3].Print(out,ident+4);  
 }
};

//------------------------------------------------------------------------------
// A fitting statistics object, templated on the algorithm modules to be used,
// each time its called it adds to the 4 math::Sample objects the relevent 
// statistics for that run.
// To - function converting from a normal sphere to the used representation. Must set vector size.
// From - function converting from the used representation to a normal sphere.
// PosError - Returns position error given sphere representation.
// DirError - Returns direction error given sphere representation.
// PosMod - Applys a transformation on the position error before it joins the error vector.
// DirMod - Go figgure.
template <void (*To)(const Sphere & in,math::Vector<real32> & out),
          void (*From)(const math::Vector<real32> & in,Sphere & out),
          real32 (*PosError)(const math::Vector<real32> & sphere,const math::Vect<3> & pos),
          real32 (*DirError)(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew),
          real32 (*PosMod)(real32 x),
          real32 (*DirMod)(real32 x)>
class Fitter
{
 public:
   Fitter(){}
  ~Fitter(){}


  // Does a sphere fitting, with the given initialisation state...
  // 0 = radius 1 sphere at origin.
  // 1 = average of all x & y values, z set to smallest in set, or 0 on none.
  // 2 = the answer. (!)
   void Add(const DataSet & ds,nat32 iniMode);


  // Prints out the current status to a write stream with a given level
  // of indentation, does a new line after the output.
   void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
   {
    out.Pad(ident); out << "Residual      = " << residual << "\n";
    out.Pad(ident); out << "Norm Residual = " << normResidual << "\n";
    out.Pad(ident); out << "Centre Dist   = " << centDist << "\n";
    out.Pad(ident); out << "Radius Dist   = " << radDist << "\n";        
   }
   
  // For finding the best scores...
   void GetBest(AlgRep & self,BestAlg & out)
   {
    self.best = residual.Mean();
    if (math::IsFinite(self.best)&&(self.best<out.rep[0][0].best)) out.rep[0][0] = self;
    self.best = normResidual.Mean();
    if (math::IsFinite(self.best)&&(self.best<out.rep[0][1].best)) out.rep[0][1] = self;
    self.best = centDist.Mean();
    if (math::IsFinite(self.best)&&(self.best<out.rep[0][2].best)) out.rep[0][2] = self;
    self.best = radDist.Mean();
    if (math::IsFinite(self.best)&&(self.best<out.rep[0][3].best)) out.rep[0][3] = self;            
    
    self.best = residual.Mean() + residual.Sd();
    if (math::IsFinite(self.best)&&(self.best<out.rep[1][0].best)) out.rep[1][0] = self;
    self.best = normResidual.Mean() + normResidual.Sd();
    if (math::IsFinite(self.best)&&(self.best<out.rep[1][1].best)) out.rep[1][1] = self;
    self.best = centDist.Mean() + centDist.Sd();
    if (math::IsFinite(self.best)&&(self.best<out.rep[1][2].best)) out.rep[1][2] = self;
    self.best = radDist.Mean() + radDist.Sd();
    if (math::IsFinite(self.best)&&(self.best<out.rep[1][3].best)) out.rep[1][3] = self;      
   }


 // The residual of the answer...
  math::Sample residual;

 // The residual of the answer divided by the residual of the actual sphere
 // used to generate the data set. A normalised residual...
  math::Sample normResidual;

 // The distance of the calculated centre from the real centre...
  math::Sample centDist;
  
 // The absolute difference between the true radius and calculated radius...
  math::Sample radDist;
};

//------------------------------------------------------------------------------
template <void (*To)(const Sphere & in,math::Vector<real32> & out),
          void (*From)(const math::Vector<real32> & in,Sphere & out),
          real32 (*PosError)(const math::Vector<real32> & sphere,const math::Vect<3> & pos),
          real32 (*DirError)(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew),
          real32 (*PosMod)(real32 x),
          real32 (*DirMod)(real32 x)>
inline void Fitter<To,From,PosError,DirError,PosMod,DirMod>::Add(const DataSet & ds,nat32 iniMode)
{
 // Interprete the initialisation mode...
  Sphere start;
  switch (iniMode)
  {
   case 0:
    start.centre[0] = 0.0;
    start.centre[1] = 0.0;
    start.centre[2] = 0.0;        
    start.radius = 1.0;
   break;
   case 1:
    // First the centre...
     start.centre[0] = 0.0;
     start.centre[1] = 0.0;
     start.centre[2] = 0.0;
     
     for (nat32 i=0;i<ds.pos.Size();i++)
     {
      start.centre[0] += ds.pos[i][0]; 
      start.centre[1] += ds.pos[i][1];
      if (i==0) start.centre[2] = ds.pos[i][2];
           else start.centre[2] = math::Min(start.centre[2],ds.pos[i][2]);
     }
     for (nat32 i=0;i<ds.dir.Size();i++)
     {
      start.centre[0] += ds.dir[i][0]; 
      start.centre[1] += ds.dir[i][1];
     }
     start.centre[0] /= real32(ds.pos.Size()+ds.dir.Size());
     start.centre[1] /= real32(ds.pos.Size()+ds.dir.Size());     
    
    // Then the radius...
     start.radius = 1.0;
     for (nat32 i=0;i<ds.pos.Size();i++)
     {
      real32 dist = math::Sqrt(math::Sqr(ds.pos[i][0]-start.centre[0])+
                               math::Sqr(ds.pos[i][1]-start.centre[1])+
                               math::Sqr(ds.pos[i][2]-start.centre[2]));
      start.radius = math::Max(start.radius,dist);
     }
     for (nat32 i=0;i<ds.dir.Size();i++)
     {
      real32 dist = math::Sqrt(math::Sqr(ds.dir[i][0]-start.centre[0])+
                               math::Sqr(ds.dir[i][1]-start.centre[1]));      
      start.radius = math::Max(start.radius,dist);
     }     
   break;
   default:
    start = ds.s;
   break;
  }
 
  
 // The error function...
  struct M
  {
   static void F(const math::Vector<real32> & pv,math::Vector<real32> & err,const DataSet & ds)
   {
    for (nat32 i=0;i<ds.pos.Size();i++)
    {
     err[i] = PosMod(PosError(pv,ds.pos[i]));
    }
    
    for (nat32 i=0;i<ds.dir.Size();i++)
    {
     err[ds.pos.Size() + i] = ds.dw*DirMod(DirError(pv,ds.dir[i],ds.ew));
    }
   }
  };

 
 // Calculate the residual of the real answer...
  math::Vector<real32> result;
  To(ds.s,result);
  math::Vector<> err(ds.pos.Size()+ds.dir.Size());
  M::F(result,err,ds);
  real32 realResidual = err.Length();
  
    
 // Run LM...
  To(start,result);
  real32 calcResidual = LM(ds.pos.Size()+ds.dir.Size(),result,ds,M::F);  
  From(result,start);
 
          
 // Add the results into the samples...
  if (math::IsFinite(start.centre[0])&&math::IsFinite(start.centre[1])&&
      math::IsFinite(start.centre[2])&&math::IsFinite(start.radius))
  {
   residual.Add(calcResidual);
   if (!math::Equal(realResidual,real32(0.0))) normResidual.Add(calcResidual/realResidual);
   centDist.Add(math::Sqrt(math::Sqr(start.centre[0]-ds.s.centre[0])+
                           math::Sqr(start.centre[1]-ds.s.centre[1])+
                           math::Sqr(start.centre[2]-ds.s.centre[2])));
   radDist.Add(math::Abs(start.radius - ds.s.radius));
  }
  else
  {
   residual.AddBad();
   normResidual.AddBad();
   centDist.AddBad();
   radDist.AddBad();
  }
}

//------------------------------------------------------------------------------
#endif
