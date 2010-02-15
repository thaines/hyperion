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

#include "fitter/fitter.h"

//------------------------------------------------------------------------------
void Sphere::MakeRandom(data::Random & rand,const math::Range & pos,const math::Range & rad)
{
 centre[0] = rand.Sample(pos);
 centre[1] = rand.Sample(pos);
 centre[2] = rand.Sample(pos);  
    radius = rand.Sample(rad);
}

//------------------------------------------------------------------------------
void DataSet::Build(data::Random & rand,const Sphere & sph,nat32 posSamples,nat32 dirSamples,
                    const math::Range & latitude,const math::Range & longitude)
{
 s = sph;
 pos.Size(posSamples);
 dir.Size(dirSamples);
 
 for (nat32 i=0;i<pos.Size();i++)
 {
  real32 lat = rand.Sample(latitude);
  real32 lon = rand.Sample(longitude);
  
  pos[i][0] = s.radius*math::Cos(lat)*math::Sin(lon) + s.centre[0];
  pos[i][1] = s.radius*math::Sin(lat)*math::Sin(lon) + s.centre[1];
  pos[i][2] = s.radius*math::Cos(lon) + s.centre[2];
 }
 
 for (nat32 i=0;i<dir.Size();i++)
 {
  real32 lat = rand.Sample(latitude);
  real32 lon = rand.Sample(longitude);
   
  dir[i][0] = s.radius*math::Cos(lat)*math::Sin(lon) + s.centre[0];
  dir[i][1] = s.radius*math::Sin(lat)*math::Sin(lon) + s.centre[1];
  dir[i][2] = math::Cos(lat)*math::Sin(lon);
  dir[i][3] = math::Sin(lat)*math::Sin(lon);
  dir[i][4] = math::Cos(lon);
 }
}

void DataSet::AddNoise(data::Random & rand,real32 p,real32 depth,real32 norm)
{
 for (nat32 i=0;i<pos.Size();i++)
 {
  pos[i][0] += rand.Gaussian(p);
  pos[i][1] += rand.Gaussian(p);
  pos[i][2] += rand.Gaussian(depth);
 }
 
 for (nat32 i=0;i<dir.Size();i++)
 {
  dir[i][0] += rand.Gaussian(p);
  dir[i][1] += rand.Gaussian(p);
  dir[i][2] += rand.Gaussian(norm);
  dir[i][3] += rand.Gaussian(norm);
  dir[i][4] += rand.Gaussian(norm);
  
  real32 mult = math::InvSqrt(math::Sqr(dir[i][2]) + math::Sqr(dir[i][3]) + math::Sqr(dir[i][4]));
  dir[i][2] *= mult;
  dir[i][3] *= mult;
  dir[i][4] *= mult;
 }
}

//------------------------------------------------------------------------------
