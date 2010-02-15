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

#include "eos/alg/depth_plane.h"

#include "eos/ds/arrays.h"
#include "eos/math/stats.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
DepthPlane::DepthPlane()
:norm(0.0,0.0,1.0)
{
 result.n = bs::Normal(0.0,0.0,1.0);
 result.d = 0.0;
}

DepthPlane::~DepthPlane()
{}

void DepthPlane::Set(const bs::Normal & n)
{
 norm = n;
}

void DepthPlane::Add(const bs::Vertex & pos)
{
 list.AddBack(pos);
}

void DepthPlane::Run()
{
 // This works by projecting all points onto the line 
 // through that origin along the orientation - we can then obtain depth and
 // do kernel density estimation...
  // Create the line...
   math::Vect<4> origin;
   origin[0] = 0.0; origin[1] = 0.0; origin[2] = 0.0; origin[3] = 1.0;
   math::Mat<4,4> line;
   math::Plucker(origin,norm,line);


  // Projected points onto the line, assign depths, record in estimator...
   math::UniDensityEstimate ude;
   {
    ds::List<bs::Vertex>::Cursor targ = list.FrontPtr();
    while (!targ.Bad())
    {
     // Project...
      math::Vect<4> proj;
      math::PluckerProj(line,*targ,proj);
      
     // Depth...
      proj.Normalise();
      if (!math::IsZero(proj[3])) // Points at infinity tend to cause problems.
      {
       proj /= proj[3];
       real32 depth = proj[0]*norm[0] + proj[1]*norm[1] + proj[2]*norm[2];
       ude.Add(depth);
      }

     // To next...
      ++targ;
    }
   }


  // Run the estimator...
   ude.Run();
   real32 res = ude.MaxMode();

  // Convert the result into a final plane as our result...
   result[0] = norm[0];
   result[1] = norm[1];
   result[2] = norm[2];
   result[3] = -res;
}

const bs::Plane & DepthPlane::Plane() const
{
 return result;
}

cstrconst DepthPlane::TypeString()
{
 return "eos::alg::DepthPlane";
}

//------------------------------------------------------------------------------
 };
};
