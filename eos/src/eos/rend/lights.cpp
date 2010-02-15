//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/rend/lights.h"

#include "eos/ds/lists.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void InfiniteLight::Calc(const bs::Normal & outDir,const Intersection & inter,const Material & mat,
                         const class RenderableDB & db,bs::ColourRGB & out) const
{
 out = bs::ColourRGB(0.0,0.0,0.0);

 // Check the first step the ray takes - if we are going into the object 
 // we are lighting up or not. If so we have to adjust the loss according to its
 // transparency details, or give up if its solid.
  bs::ColourRGB loss(1.0,1.0,1.0);
  bs::Ray ray;
   ray.s = inter.point;
   ray.n = toLight;
   
  bs::Normal fromLight = toLight; 
  fromLight.Neg();

  ds::List<const Material*> inside; // Linked list of materials it is currently inside of. (Should really be optimised out. A todo I guess.)
  if ((toLight * inter.norm)<0.0)
  {
   // The ray is fired back into the material, makes the next code lump handle
   // this, or break if its solid.
    if (mat.Transparency()==false) return;
    inside.AddFront(&mat);
  }
 
 // Fire a ray from the point of intersection towards the light source, 
 // and see if it hits anything. If we bump into transparent objects extract 
 // there multipliers and keep track of the transmission rate. Give up if
 // too little light is transmitted, or we hit a solid object...
 // (This of course ignores refraction, but not much too be done about that.)  
  while (true)
  {
   // Find the next intersection point, break if it does not exist...
    Intersection inter;
    Renderable * obj;
    if (db.Intercept(ray,obj,inter)==false) break;

   // Update the loss for the materials we are within...
    bs::FiniteLine line;
     line.s = ray.s;
     line.e = inter.point;
    
    ds::List<const Material*>::Cursor targ = inside.FrontPtr();
    while (!targ.Bad())
    {
     bs::ColourRGB temp;    
      (*targ)->TransparencyLoss(line,temp);
     loss *= temp;

     ++targ;
    }

   // Update the material list and loss for the material surface we just hit....
    Renderable::MatSpec & ms = obj->mat[inter.coord.material];
    if (ms.mat->Transparency()==false) return;
    
    bs::ColourRGB temp; 
    if (ms.im) ms.im->Modify(inter);
    ms.mat->BDRF(fromLight,toLight,inter,temp); 
    loss *= temp;
    
    if ((toLight * inter.norm)<0.0)
    {
     // We are entering the object...
      inside.AddFront(ms.mat);
    }
    else
    {
     // We are exitting the object...
      ds::List<const Material*>::Cursor targ = inside.FrontPtr();
      while (!targ.Bad())
      {
       if (*targ==ms.mat) {targ.RemKillNext(); break;}    
       ++targ;
      }
    }  
   
   // Update the ray for the next casting...
    ray.s = inter.point;
  }
  
 // If we are still inside any object at this point the object must be infinite,
 // and therefore no light can get through - give up...
 // (More likelly something has gone wrong, in which case making dodgy artifacts is good to.)
  if (inside.Size()!=0) return;
  
 // If we reach this point then some light reaches and affects the object - the 
 // output is simply the strength of the light, multiplied by the tranmissive 
 // factors of the objects intersected en-route multiplied by the BDRF of the 
 // material at intersection...
  mat.BDRF(outDir,toLight,inter,out);
  out *= loss;
  out *= col;
}

//------------------------------------------------------------------------------
 };
};
