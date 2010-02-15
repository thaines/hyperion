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

#include "eos/bs/geo_algs.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
EOS_FUNC bit IntPlaneLine(const Line & l,const Plane & p,real32 & out)
{ 
 real32 div =  l.n * p.n;
 if (math::Equal(div,real32(0.0))) return false;

 Vert pp = p.n;
 pp *= -p.d;
 pp -= l.p;
 out = (p.n*pp)/div;

 return true;
}

//------------------------------------------------------------------------------
EOS_FUNC real32 PlaneDistance(const Pnt & l1,const PlaneABC & p1,const Pnt & l2,const PlaneABC & p2)
{
 Plane plane1(p1);
 Plane plane2(p2);

 Line line1;
  line1.p[0] = l1.X();
  line1.p[1] = l1.Y();
  line1.p[2] = p1.Z(l1.X(),l1.Y());
  line1.n = plane1.n;

 Line line2; 
  line2.p[0] = l2.X();
  line2.p[1] = l2.Y();
  line2.p[2] = p2.Z(l2.X(),l2.Y());
  line2.n = plane2.n;

 real32 d1; IntPlaneLine(line1,plane2,d1); 
 real32 d2; IntPlaneLine(line2,plane1,d2);
 
 return math::Abs(d1) + math::Abs(d2);
}

//------------------------------------------------------------------------------
 };
};
