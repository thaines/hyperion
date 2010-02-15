//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
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
