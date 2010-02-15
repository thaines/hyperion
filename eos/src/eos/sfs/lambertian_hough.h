#ifndef EOS_SFS_LAMBERTIAN_HOUGH_H
#define EOS_SFS_LAMBERTIAN_HOUGH_H
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


/// \file lambertian_hough.h
/// Uses a hough transform on an image with provided orientation information to
/// find lighting direction and albedos.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays2d.h"
#include "eos/ds/lists.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// Estimates possible lighting directions and albedos for an image using the
/// hough transform. (Technically a generalised variety, though its not much 
/// different from the line variety.)
/// You provide data in the form of (weighted) irradiance/orientation pairs,
/// it then draws onto a 2D image where one axis represents orientations
/// (Generated with the vertices of a sub-divided icosahedron.) and another axis
/// represents albedo. The maximas of the final summed image are where lighting
/// models are likelly to be be, a method is provided to extract a list of these
/// maximas, as vectors to dot with the surface orientation to get irradiance.
class EOS_CLASS LamHough
{
 public:
  /// &nbsp;
   LamHough();
   
  /// &nbsp;
   ~LamHough();
   
   
  /// Sets the size of the output accumilator.
  /// \param subDivs Number of subdivisions of the icosahedron, specifies the x axis size of the accumilator image. Defaults to 7.
  /// \param minAlb Minimum albedo considered. Defaults to 0.0.
  /// \param maxAlb Maximum albedo considered. Defaults to 200.0.
  /// \param albDivs Divisions of albedo, size of the y axis of the accumilator image. Defaults to 200.
   void Set(nat32 subDivs,real32 minAlb,real32 maxAlb,nat32 albDivs);
   
  /// Sets the falloff, i.e. blur, of the weights being drawn, standard deviation
  /// for albedo, plus the hard cutoff in number of standard deviations.
  /// Defaults to 2.0 and 2.0 respectivly.
   void Set(real32 albSd,real32 cutoff);
   
  /// Sets the prunning parameters, used to prune the maximas founds down to a
  /// more managable number.
  /// bestCap is the number of best to take from each segment, defaults to 3.
  /// maxErr is the maximum error to accept per pixel, defaults to 2.0.
   void Set(nat32 bestCap,real32 maxErr);


  /// Adds a single sample to the database of samples.
   void Add(real32 irr,const bs::Normal & dir,real32 weight = 1.0,nat32 segment = 0);
   
  /// Adds an entire image to the database of samples. A mask may be provided.
   void Add(const svt::Field<real32> & irr,const svt::Field<bs::Normal> & norm,
            const svt::Field<real32> & weight = svt::Field<real32>(),
            const svt::Field<nat32> & segment = svt::Field<nat32>(),
            const svt::Field<bit> & mask = svt::Field<bit>());


  /// Runs the algorithm, very fast really, you will probably spend comparable time
  /// in the Add methods.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// After calling run this returns the number of maximas found.
   nat32 Maximas() const;
   
  /// Returns a given maxima, as a vector that for a matching sample should, when
  /// dotted with the direction vector, calculate the albedo.
   const bs::Normal & Maxima(nat32 ind) const;
   
  /// Returns the weight as obtained from thenhough transform result.
   real32 Weight(nat32 ind) const;
   
  /// Returns the number of segments for which the total error is within the
  /// prunning limit, weighted by position. Higher is better basically.
   nat32 MaximaValue(nat32 ind) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::LamHough";}


 private:
  // Inputs...
   nat32 subDivs;
   real32 minAlb;
   real32 maxAlb;
   nat32 albDivs;
   real32 albSd;
   real32 cutoff;
   nat32 bestCap;
   real32 maxErr;

   struct Sample
   {
    real32 irr;
    bs::Normal dir;
    real32 weight;
    nat32 segment;
   };
   ds::List<Sample> in;


  // Outputs...
   struct Model
   {
    bs::Normal mod; // Model itself.
    real32 weight;
    nat32 num; // Number of segments for which it is a best fit, weighted by the position.
   };
   ds::Array<Model> out; // List of models - maximas taken from Hough transform.
   
   
  // Runtime...
   struct ModError
   {
    Model * mod;
    bs::Normal modVec; // Dup from mod->mod, for cache coherance.
    real32 error;
    
    bit operator < (const ModError & rhs) const
    {return error < rhs.error;}
   };
};

//------------------------------------------------------------------------------
 };
};
#endif
