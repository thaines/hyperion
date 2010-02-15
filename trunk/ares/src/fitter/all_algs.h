#ifndef FITTER_ALL_ALGS_H
#define FITTER_ALL_ALGS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
// Contains a data structure that covers every algorithm possibility, allowing
// each algorithm to fit a sphere to a given data set with a single call, and 
// the extraction of all the stats to a stream, again with a single call.

#include "fit_algs.h"

//------------------------------------------------------------------------------
// This constructs all combinations of direction mod...
template <void (*To)(const Sphere & in,math::Vector<real32> & out),
          void (*From)(const math::Vector<real32> & in,Sphere & out),
          real32 (*PosError)(const math::Vector<real32> & sphere,const math::Vect<3> & pos),
          real32 (*DirError)(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew),
          real32 (*PosMod)(real32 x)>
struct DirModSet
{
 Fitter<To,From,PosError,DirError,PosMod,LinearMod> linear;
 Fitter<To,From,PosError,DirError,PosMod,SquaredMod> squared;
 Fitter<To,From,PosError,DirError,PosMod,BlakeZissermanMod> blake;
 Fitter<To,From,PosError,DirError,PosMod,PsuedoHuberMod> psuedo;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  linear.Add(ds,iniMode);
  squared.Add(ds,iniMode);
  blake.Add(ds,iniMode);
  psuedo.Add(ds,iniMode);
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Linear Dir:\n"; linear.Print(out,ident+2);
  out.Pad(ident); out << "Squared Dir:\n"; squared.Print(out,ident+2);
  out.Pad(ident); out << "Blake-Zisserman Dir:\n"; blake.Print(out,ident+2);
  out.Pad(ident); out << "Psuedo Huber Dir:\n"; psuedo.Print(out,ident+2);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.dirMod = "Linear"; linear.GetBest(self,out);  
  self.dirMod = "Squared"; squared.GetBest(self,out);
  self.dirMod = "Blake-Zisserman"; blake.GetBest(self,out);
  self.dirMod = "Psuedo Huber"; psuedo.GetBest(self,out);
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations of error modifier...
template <void (*To)(const Sphere & in,math::Vector<real32> & out),
          void (*From)(const math::Vector<real32> & in,Sphere & out),
          real32 (*PosError)(const math::Vector<real32> & sphere,const math::Vect<3> & pos),
          real32 (*DirError)(const math::Vector<real32> & sphere,const math::Vect<5> & dir,real32 ew)>
struct ModSet
{
 DirModSet<To,From,PosError,DirError,LinearMod> linear;
 DirModSet<To,From,PosError,DirError,SquaredMod> squared;
 DirModSet<To,From,PosError,DirError,BlakeZissermanMod> blake;
 DirModSet<To,From,PosError,DirError,PsuedoHuberMod> psuedo;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  linear.Add(ds,iniMode);
  squared.Add(ds,iniMode);
  blake.Add(ds,iniMode);
  psuedo.Add(ds,iniMode);
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Linear Pos:\n"; linear.Print(out,ident+2);
  out.Pad(ident); out << "Squared Pos:\n"; squared.Print(out,ident+2);
  out.Pad(ident); out << "Blake-Zisserman Pos:\n"; blake.Print(out,ident+2);
  out.Pad(ident); out << "Psuedo Huber Pos:\n"; psuedo.Print(out,ident+2);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.posMod = "Linear"; linear.GetBest(self,out);  
  self.posMod = "Squared"; squared.GetBest(self,out);
  self.posMod = "Blake-Zisserman"; blake.GetBest(self,out);
  self.posMod = "Psuedo Huber"; psuedo.GetBest(self,out);
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations of error metric for the (centre,radius)
// representation...
struct CentreRadius
{
 ModSet<ToCR,FromCR,PosErrAlgCR,DirErrPlaneCR> ms11;
 ModSet<ToCR,FromCR,PosErrAlgCR,DirErrPlaneNormCR> ms12;
 ModSet<ToCR,FromCR,PosErrAlgCR,DirErrAngCR> ms13;
 ModSet<ToCR,FromCR,PosErrAlgCR,DirErrArcCR> ms14;
 
 ModSet<ToCR,FromCR,PosErrGeoCR,DirErrPlaneCR> ms21;
 ModSet<ToCR,FromCR,PosErrGeoCR,DirErrPlaneNormCR> ms22;
 ModSet<ToCR,FromCR,PosErrGeoCR,DirErrAngCR> ms23;
 ModSet<ToCR,FromCR,PosErrGeoCR,DirErrArcCR> ms24;
 
 ModSet<ToCR,FromCR,PosErrDepthCR,DirErrPlaneCR> ms31;
 ModSet<ToCR,FromCR,PosErrDepthCR,DirErrPlaneNormCR> ms32;
 ModSet<ToCR,FromCR,PosErrDepthCR,DirErrAngCR> ms33;
 ModSet<ToCR,FromCR,PosErrDepthCR,DirErrArcCR> ms34;
 
 ModSet<ToCR,FromCR,PosErrPlaneCR,DirErrPlaneCR> ms41;
 ModSet<ToCR,FromCR,PosErrPlaneCR,DirErrPlaneNormCR> ms42;
 ModSet<ToCR,FromCR,PosErrPlaneCR,DirErrAngCR> ms43;
 ModSet<ToCR,FromCR,PosErrPlaneCR,DirErrArcCR> ms44;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  ms11.Add(ds,iniMode); ms12.Add(ds,iniMode); ms13.Add(ds,iniMode); ms14.Add(ds,iniMode);
  ms21.Add(ds,iniMode); ms22.Add(ds,iniMode); ms23.Add(ds,iniMode); ms24.Add(ds,iniMode);
  ms31.Add(ds,iniMode); ms32.Add(ds,iniMode); ms33.Add(ds,iniMode); ms34.Add(ds,iniMode);
  ms41.Add(ds,iniMode); ms42.Add(ds,iniMode); ms43.Add(ds,iniMode); ms44.Add(ds,iniMode);      
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {  
  out.Pad(ident); out << "Algorithmic Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
  
  out.Pad(ident); out << "Geometric Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms21.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms22.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms23.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms24.Print(out,ident+4);
  
  out.Pad(ident); out << "Depth Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms31.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms32.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms33.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms34.Print(out,ident+4);
  
  out.Pad(ident); out << "Plane Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.pos = "Algorithmic";
  self.dir = "Plane";            ms11.GetBest(self,out);
  self.dir = "Normalised Plane"; ms12.GetBest(self,out);
  self.dir = "Angle";            ms13.GetBest(self,out);
  self.dir = "Arc";              ms14.GetBest(self,out);
  
  self.pos = "Geometric";
  self.dir = "Plane";            ms21.GetBest(self,out);
  self.dir = "Normalised Plane"; ms22.GetBest(self,out);
  self.dir = "Angle";            ms23.GetBest(self,out);
  self.dir = "Arc";              ms24.GetBest(self,out);
  
  self.pos = "Depth";
  self.dir = "Plane";            ms31.GetBest(self,out);
  self.dir = "Normalised Plane"; ms32.GetBest(self,out);
  self.dir = "Angle";            ms33.GetBest(self,out);
  self.dir = "Arc";              ms34.GetBest(self,out);
  
  self.pos = "Plane"; 
  self.dir = "Plane";            ms41.GetBest(self,out);
  self.dir = "Normalised Plane"; ms42.GetBest(self,out);
  self.dir = "Angle";            ms43.GetBest(self,out);
  self.dir = "Arc";              ms44.GetBest(self,out);      
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations of error metric for the (centre,radius)
// representation...
struct CentreRadiusSqr
{
 ModSet<ToCRS,FromCRS,PosErrAlgCRS,DirErrPlaneCRS> ms11;
 ModSet<ToCRS,FromCRS,PosErrAlgCRS,DirErrPlaneNormCRS> ms12;
 ModSet<ToCRS,FromCRS,PosErrAlgCRS,DirErrAngCRS> ms13;
 ModSet<ToCRS,FromCRS,PosErrAlgCRS,DirErrArcCRS> ms14;
 
 ModSet<ToCRS,FromCRS,PosErrGeoCRS,DirErrPlaneCRS> ms21;
 ModSet<ToCRS,FromCRS,PosErrGeoCRS,DirErrPlaneNormCRS> ms22;
 ModSet<ToCRS,FromCRS,PosErrGeoCRS,DirErrAngCRS> ms23;
 ModSet<ToCRS,FromCRS,PosErrGeoCRS,DirErrArcCRS> ms24;
 
 ModSet<ToCRS,FromCRS,PosErrDepthCRS,DirErrPlaneCRS> ms31;
 ModSet<ToCRS,FromCRS,PosErrDepthCRS,DirErrPlaneNormCRS> ms32;
 ModSet<ToCRS,FromCRS,PosErrDepthCRS,DirErrAngCRS> ms33;
 ModSet<ToCRS,FromCRS,PosErrDepthCRS,DirErrArcCRS> ms34;
 
 ModSet<ToCRS,FromCRS,PosErrPlaneCRS,DirErrPlaneCRS> ms41;
 ModSet<ToCRS,FromCRS,PosErrPlaneCRS,DirErrPlaneNormCRS> ms42;
 ModSet<ToCRS,FromCRS,PosErrPlaneCRS,DirErrAngCRS> ms43;
 ModSet<ToCRS,FromCRS,PosErrPlaneCRS,DirErrArcCRS> ms44;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  ms11.Add(ds,iniMode); ms12.Add(ds,iniMode); ms13.Add(ds,iniMode); ms14.Add(ds,iniMode);
  ms21.Add(ds,iniMode); ms22.Add(ds,iniMode); ms23.Add(ds,iniMode); ms24.Add(ds,iniMode);
  ms31.Add(ds,iniMode); ms32.Add(ds,iniMode); ms33.Add(ds,iniMode); ms34.Add(ds,iniMode);
  ms41.Add(ds,iniMode); ms42.Add(ds,iniMode); ms43.Add(ds,iniMode); ms44.Add(ds,iniMode);      
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Algorithmic Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
  
  out.Pad(ident); out << "Geometric Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms21.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms22.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms23.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms24.Print(out,ident+4);
  
  out.Pad(ident); out << "Depth Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms31.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms32.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms33.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms34.Print(out,ident+4);
  
  out.Pad(ident); out << "Plane Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.pos = "Algorithmic";
  self.dir = "Plane";            ms11.GetBest(self,out);
  self.dir = "Normalised Plane"; ms12.GetBest(self,out);
  self.dir = "Angle";            ms13.GetBest(self,out);
  self.dir = "Arc";              ms14.GetBest(self,out);
  
  self.pos = "Geometric";
  self.dir = "Plane";            ms21.GetBest(self,out);
  self.dir = "Normalised Plane"; ms22.GetBest(self,out);
  self.dir = "Angle";            ms23.GetBest(self,out);
  self.dir = "Arc";              ms24.GetBest(self,out);
  
  self.pos = "Depth";
  self.dir = "Plane";            ms31.GetBest(self,out);
  self.dir = "Normalised Plane"; ms32.GetBest(self,out);
  self.dir = "Angle";            ms33.GetBest(self,out);
  self.dir = "Arc";              ms34.GetBest(self,out);
  
  self.pos = "Plane"; 
  self.dir = "Plane";            ms41.GetBest(self,out);
  self.dir = "Normalised Plane"; ms42.GetBest(self,out);
  self.dir = "Angle";            ms43.GetBest(self,out);
  self.dir = "Arc";              ms44.GetBest(self,out);      
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations of error metric for the (centre,1.0/radius)
// representation...
struct CentreOneDivRadius
{
 ModSet<ToCRDO,FromCRDO,PosErrAlgCRDO,DirErrPlaneCRDO> ms11;
 ModSet<ToCRDO,FromCRDO,PosErrAlgCRDO,DirErrPlaneNormCRDO> ms12;
 ModSet<ToCRDO,FromCRDO,PosErrAlgCRDO,DirErrAngCRDO> ms13;
 ModSet<ToCRDO,FromCRDO,PosErrAlgCRDO,DirErrArcCRDO> ms14;
 
 ModSet<ToCRDO,FromCRDO,PosErrGeoCRDO,DirErrPlaneCRDO> ms21;
 ModSet<ToCRDO,FromCRDO,PosErrGeoCRDO,DirErrPlaneNormCRDO> ms22;
 ModSet<ToCRDO,FromCRDO,PosErrGeoCRDO,DirErrAngCRDO> ms23;
 ModSet<ToCRDO,FromCRDO,PosErrGeoCRDO,DirErrArcCRDO> ms24;
 
 ModSet<ToCRDO,FromCRDO,PosErrDepthCRDO,DirErrPlaneCRDO> ms31;
 ModSet<ToCRDO,FromCRDO,PosErrDepthCRDO,DirErrPlaneNormCRDO> ms32;
 ModSet<ToCRDO,FromCRDO,PosErrDepthCRDO,DirErrAngCRDO> ms33;
 ModSet<ToCRDO,FromCRDO,PosErrDepthCRDO,DirErrArcCRDO> ms34;
 
 ModSet<ToCRDO,FromCRDO,PosErrPlaneCRDO,DirErrPlaneCRDO> ms41;
 ModSet<ToCRDO,FromCRDO,PosErrPlaneCRDO,DirErrPlaneNormCRDO> ms42;
 ModSet<ToCRDO,FromCRDO,PosErrPlaneCRDO,DirErrAngCRDO> ms43;
 ModSet<ToCRDO,FromCRDO,PosErrPlaneCRDO,DirErrArcCRDO> ms44;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  ms11.Add(ds,iniMode); ms12.Add(ds,iniMode); ms13.Add(ds,iniMode); ms14.Add(ds,iniMode);
  ms21.Add(ds,iniMode); ms22.Add(ds,iniMode); ms23.Add(ds,iniMode); ms24.Add(ds,iniMode);
  ms31.Add(ds,iniMode); ms32.Add(ds,iniMode); ms33.Add(ds,iniMode); ms34.Add(ds,iniMode);
  ms41.Add(ds,iniMode); ms42.Add(ds,iniMode); ms43.Add(ds,iniMode); ms44.Add(ds,iniMode);      
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Algorithmic Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
  
  out.Pad(ident); out << "Geometric Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms21.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms22.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms23.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms24.Print(out,ident+4);
  
  out.Pad(ident); out << "Depth Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms31.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms32.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms33.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms34.Print(out,ident+4);
  
  out.Pad(ident); out << "Plane Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.pos = "Algorithmic";
  self.dir = "Plane";            ms11.GetBest(self,out);
  self.dir = "Normalised Plane"; ms12.GetBest(self,out);
  self.dir = "Angle";            ms13.GetBest(self,out);
  self.dir = "Arc";              ms14.GetBest(self,out);
  
  self.pos = "Geometric";
  self.dir = "Plane";            ms21.GetBest(self,out);
  self.dir = "Normalised Plane"; ms22.GetBest(self,out);
  self.dir = "Angle";            ms23.GetBest(self,out);
  self.dir = "Arc";              ms24.GetBest(self,out);
  
  self.pos = "Depth";
  self.dir = "Plane";            ms31.GetBest(self,out);
  self.dir = "Normalised Plane"; ms32.GetBest(self,out);
  self.dir = "Angle";            ms33.GetBest(self,out);
  self.dir = "Arc";              ms34.GetBest(self,out);
  
  self.pos = "Plane"; 
  self.dir = "Plane";            ms41.GetBest(self,out);
  self.dir = "Normalised Plane"; ms42.GetBest(self,out);
  self.dir = "Angle";            ms43.GetBest(self,out);
  self.dir = "Arc";              ms44.GetBest(self,out);      
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations of error metric for the implicit
// representation...
struct Implicit
{
 ModSet<ToIM,FromIM,PosErrAlgIM,DirErrPlaneIM> ms11;
 ModSet<ToIM,FromIM,PosErrAlgIM,DirErrPlaneNormIM> ms12;
 ModSet<ToIM,FromIM,PosErrAlgIM,DirErrAngIM> ms13;
 ModSet<ToIM,FromIM,PosErrAlgIM,DirErrArcIM> ms14;
 
 ModSet<ToIM,FromIM,PosErrGeoIM,DirErrPlaneIM> ms21;
 ModSet<ToIM,FromIM,PosErrGeoIM,DirErrPlaneNormIM> ms22;
 ModSet<ToIM,FromIM,PosErrGeoIM,DirErrAngIM> ms23;
 ModSet<ToIM,FromIM,PosErrGeoIM,DirErrArcIM> ms24;
 
 ModSet<ToIM,FromIM,PosErrDepthIM,DirErrPlaneIM> ms31;
 ModSet<ToIM,FromIM,PosErrDepthIM,DirErrPlaneNormIM> ms32;
 ModSet<ToIM,FromIM,PosErrDepthIM,DirErrAngIM> ms33;
 ModSet<ToIM,FromIM,PosErrDepthIM,DirErrArcIM> ms34;
 
 ModSet<ToIM,FromIM,PosErrPlaneIM,DirErrPlaneIM> ms41;
 ModSet<ToIM,FromIM,PosErrPlaneIM,DirErrPlaneNormIM> ms42;
 ModSet<ToIM,FromIM,PosErrPlaneIM,DirErrAngIM> ms43;
 ModSet<ToIM,FromIM,PosErrPlaneIM,DirErrArcIM> ms44;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  ms11.Add(ds,iniMode); ms12.Add(ds,iniMode); ms13.Add(ds,iniMode); ms14.Add(ds,iniMode);
  ms21.Add(ds,iniMode); ms22.Add(ds,iniMode); ms23.Add(ds,iniMode); ms24.Add(ds,iniMode);
  ms31.Add(ds,iniMode); ms32.Add(ds,iniMode); ms33.Add(ds,iniMode); ms34.Add(ds,iniMode);
  ms41.Add(ds,iniMode); ms42.Add(ds,iniMode); ms43.Add(ds,iniMode); ms44.Add(ds,iniMode);      
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Algorithmic Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
  
  out.Pad(ident); out << "Geometric Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms21.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms22.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms23.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms24.Print(out,ident+4);
  
  out.Pad(ident); out << "Depth Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms31.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms32.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms33.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms34.Print(out,ident+4);
  
  out.Pad(ident); out << "Plane Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.pos = "Algorithmic";
  self.dir = "Plane";            ms11.GetBest(self,out);
  self.dir = "Normalised Plane"; ms12.GetBest(self,out);
  self.dir = "Angle";            ms13.GetBest(self,out);
  self.dir = "Arc";              ms14.GetBest(self,out);
  
  self.pos = "Geometric";
  self.dir = "Plane";            ms21.GetBest(self,out);
  self.dir = "Normalised Plane"; ms22.GetBest(self,out);
  self.dir = "Angle";            ms23.GetBest(self,out);
  self.dir = "Arc";              ms24.GetBest(self,out);
  
  self.pos = "Depth";
  self.dir = "Plane";            ms31.GetBest(self,out);
  self.dir = "Normalised Plane"; ms32.GetBest(self,out);
  self.dir = "Angle";            ms33.GetBest(self,out);
  self.dir = "Arc";              ms34.GetBest(self,out);
  
  self.pos = "Plane"; 
  self.dir = "Plane";            ms41.GetBest(self,out);
  self.dir = "Normalised Plane"; ms42.GetBest(self,out);
  self.dir = "Angle";            ms43.GetBest(self,out);
  self.dir = "Arc";              ms44.GetBest(self,out);      
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations of error metric for the polar 
// coordinate/distance to surface, radius representation...
struct PolarCentre
{
 ModSet<ToNC,FromNC,PosErrAlgNC,DirErrPlaneNC> ms11;
 ModSet<ToNC,FromNC,PosErrAlgNC,DirErrPlaneNormNC> ms12;
 ModSet<ToNC,FromNC,PosErrAlgNC,DirErrAngNC> ms13;
 ModSet<ToNC,FromNC,PosErrAlgNC,DirErrArcNC> ms14;
 
 ModSet<ToNC,FromNC,PosErrGeoNC,DirErrPlaneNC> ms21;
 ModSet<ToNC,FromNC,PosErrGeoNC,DirErrPlaneNormNC> ms22;
 ModSet<ToNC,FromNC,PosErrGeoNC,DirErrAngNC> ms23;
 ModSet<ToNC,FromNC,PosErrGeoNC,DirErrArcNC> ms24;
 
 ModSet<ToNC,FromNC,PosErrDepthNC,DirErrPlaneNC> ms31;
 ModSet<ToNC,FromNC,PosErrDepthNC,DirErrPlaneNormNC> ms32;
 ModSet<ToNC,FromNC,PosErrDepthNC,DirErrAngNC> ms33;
 ModSet<ToNC,FromNC,PosErrDepthNC,DirErrArcNC> ms34;
 
 ModSet<ToNC,FromNC,PosErrPlaneNC,DirErrPlaneNC> ms41;
 ModSet<ToNC,FromNC,PosErrPlaneNC,DirErrPlaneNormNC> ms42;
 ModSet<ToNC,FromNC,PosErrPlaneNC,DirErrAngNC> ms43;
 ModSet<ToNC,FromNC,PosErrPlaneNC,DirErrArcNC> ms44;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  ms11.Add(ds,iniMode); ms12.Add(ds,iniMode); ms13.Add(ds,iniMode); ms14.Add(ds,iniMode);
  ms21.Add(ds,iniMode); ms22.Add(ds,iniMode); ms23.Add(ds,iniMode); ms24.Add(ds,iniMode);
  ms31.Add(ds,iniMode); ms32.Add(ds,iniMode); ms33.Add(ds,iniMode); ms34.Add(ds,iniMode);
  ms41.Add(ds,iniMode); ms42.Add(ds,iniMode); ms43.Add(ds,iniMode); ms44.Add(ds,iniMode);      
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Algorithmic Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
  
  out.Pad(ident); out << "Geometric Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms21.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms22.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms23.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms24.Print(out,ident+4);
  
  out.Pad(ident); out << "Depth Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms31.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms32.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms33.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms34.Print(out,ident+4);
  
  out.Pad(ident); out << "Plane Pos:\n";
  out.Pad(ident+2); out << "Plane Dir:\n";            ms11.Print(out,ident+4);
  out.Pad(ident+2); out << "Normalised Plane Dir:\n"; ms12.Print(out,ident+4);
  out.Pad(ident+2); out << "Angle Dir:\n";            ms13.Print(out,ident+4);
  out.Pad(ident+2); out << "Arc Length Dir:\n";       ms14.Print(out,ident+4);
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.pos = "Algorithmic";
  self.dir = "Plane";            ms11.GetBest(self,out);
  self.dir = "Normalised Plane"; ms12.GetBest(self,out);
  self.dir = "Angle";            ms13.GetBest(self,out);
  self.dir = "Arc";              ms14.GetBest(self,out);
  
  self.pos = "Geometric";
  self.dir = "Plane";            ms21.GetBest(self,out);
  self.dir = "Normalised Plane"; ms22.GetBest(self,out);
  self.dir = "Angle";            ms23.GetBest(self,out);
  self.dir = "Arc";              ms24.GetBest(self,out);
  
  self.pos = "Depth";
  self.dir = "Plane";            ms31.GetBest(self,out);
  self.dir = "Normalised Plane"; ms32.GetBest(self,out);
  self.dir = "Angle";            ms33.GetBest(self,out);
  self.dir = "Arc";              ms34.GetBest(self,out);
  
  self.pos = "Plane"; 
  self.dir = "Plane";            ms41.GetBest(self,out);
  self.dir = "Normalised Plane"; ms42.GetBest(self,out);
  self.dir = "Angle";            ms43.GetBest(self,out);
  self.dir = "Arc";              ms44.GetBest(self,out);      
 }
};

//------------------------------------------------------------------------------
// This constructs all combinations possible, in one mother of a final class...
struct AllAlgs
{
 CentreRadius cr;
 CentreRadiusSqr crs;
 CentreOneDivRadius cdo;
 Implicit im;
 PolarCentre nc;
 
 void Add(const DataSet & ds,nat32 iniMode)
 {
  cr.Add(ds,iniMode);   
  crs.Add(ds,iniMode);  
  cdo.Add(ds,iniMode);
  im.Add(ds,iniMode);
  nc.Add(ds,iniMode);
 }
 
 void Print(io::OutVirt<io::Text> & out,nat32 ident = 0)
 {
  out.Pad(ident); out << "Centre Radius Representation:\n"; cr.Print(out,ident+2);
  out.Pad(ident); out << "Centre Radius Squared Representation:\n"; crs.Print(out,ident+2);
  out.Pad(ident); out << "Centre 1/Radius Representation:\n"; cdo.Print(out,ident+2);
  out.Pad(ident); out << "Implicit Representation:\n"; cdo.Print(out,ident+2);
  out.Pad(ident); out << "Polar-Surface Radius Representation:\n"; nc.Print(out,ident+2);  
 }
 
 void GetBest(AlgRep & self,BestAlg & out)
 {
  self.rep = "Centre Radius"; cr.GetBest(self,out);
  self.rep = "Centre Radius Squared"; crs.GetBest(self,out); 
  self.rep = "Centre 1.0/Radius"; cdo.GetBest(self,out);
  self.rep = "Implicit"; im.GetBest(self,out);
  self.rep = "Polar-Surface Radius"; nc.GetBest(self,out);  
 }
};

//------------------------------------------------------------------------------
#endif
