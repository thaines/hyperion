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

#include "eos/math/distance.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
Distance::~Distance()
{}

//------------------------------------------------------------------------------
EuclideanDistance::~EuclideanDistance()
{}

real32 EuclideanDistance::operator () (nat32 d,const real32 * pa,const real32 * pb) const
{
 real32 ret = 0.0;
 for (nat32 i=0;i<d;i++) ret += math::Sqr(pa[i]-pb[i]);
 return math::Sqrt(ret);
}

cstrconst EuclideanDistance::TypeString() const
{
 return "eos::math::EuclideanDistance";
}

//------------------------------------------------------------------------------
ManhattanDistance::~ManhattanDistance()
{}

real32 ManhattanDistance::operator () (nat32 d,const real32 * pa,const real32 * pb) const
{
 real32 ret = 0.0;
 for (nat32 i=0;i<d;i++) ret += math::Abs(pa[i]-pb[i]);
 return ret;
}

cstrconst ManhattanDistance::TypeString() const
{
 return "eos::math::ManhattanDistance";
}
//------------------------------------------------------------------------------
 };
};
