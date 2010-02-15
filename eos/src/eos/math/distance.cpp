//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
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
