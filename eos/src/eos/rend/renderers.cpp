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

#include "eos/rend/renderers.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void OneHit::Cast(TaggedRay & ray,const RenderableDB & db,const ds::List<Renderable*,
                  mem::KillDel<Light> > & inside,const ds::List<Light*> & ll,const Background & bg) const
{
 if (db.Intercept(ray,ray.hit,ray.inter))
 {
  // Hit an object - respond accordingly...
   ray.irradiance = bs::ColourRGB(0.0,0.0,0.0);

   Renderable::MatSpec & ms = ray.hit->mat[ray.inter.coord.material];
   if (ms.im) ms.im->Modify(ray.inter);
   
   ds::List<Light*>::Cursor targ = ll.FrontPtr();
   while (!targ.Bad())
   {
    bs::ColourRGB out;
    bs::Normal norm = ray.n; norm.Neg();
    (*targ)->Calc(norm,ray.inter,*ms.mat,db,out);
    ray.irradiance += out;
    ++targ;
   }
 }
 else
 {
  // Missed all objects - use the background instead...
   ray.hit = null<Renderable*>();
   bg.Calc(ray,ray.irradiance);
 }
}

//------------------------------------------------------------------------------
 };
};
