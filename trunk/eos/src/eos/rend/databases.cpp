//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/rend/databases.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
BruteDB::BruteDB()
{}

BruteDB::~BruteDB()
{}

void BruteDB::Add(Renderable * rb)
{
 Node n;
  n.rend = rb;
 data.AddBack(n);
}

void BruteDB::Prepare(time::Progress * prog)
{
 prog->Push();
  nat32 step = 0;
  nat32 steps = data.Size();
 
  ds::List<Node>::Cursor targ = data.FrontPtr();
  while (!targ.Bad())
  {
   prog->Report(step,steps);
   
   bs::Sphere sphere;
   targ->rend->object->Bound(sphere);
   targ->rend->local->ToWorld(sphere,targ->sphere);
   
   targ->rend->object->Prepare();
   
   ++targ;
   ++step;
  }
 prog->Pop();
}

void BruteDB::Unprepare(time::Progress * prog)
{
 prog->Push();
  nat32 step = 0;
  nat32 steps = data.Size();
 
  ds::List<Node>::Cursor targ = data.FrontPtr();
  while (!targ.Bad())
  {
   prog->Report(step,steps);
   
   targ->rend->object->Unprepare();
   
   ++targ;
   ++step;
  }
 prog->Pop();
}

void BruteDB::Inside(const bs::Vert & point,ds::List<Renderable*> & out) const
{
 ds::List<Node>::Cursor targ = data.FrontPtr();
 while (!targ.Bad())
 {
  if (targ->sphere.Inside(point))
  {
   // Transform the point to local object coordinates...
    bs::Vert locP;
    targ->rend->local->ToLocal(point,locP);
   
   // Test it using the objects check - if it passes add it to the list...
    if (targ->rend->object->Inside(locP))
    {
     out.AddBack(targ->rend);
    }
  }
  ++targ;
 }
}

bit BruteDB::Intercept(const bs::Ray & ray,Renderable *& objOut,Intersection & intOut) const
{
 bit ret = false;
 
 // Find the closest object to the start of the ray...
  real32 bestDistance = 0.0;
  ds::List<Node>::Cursor targ = data.FrontPtr();
  while (!targ.Bad())
  {
   if (targ->sphere.Intercept(ray))
   {
    // Transform the ray to local object coordinates...
     bs::Ray locRay;
     targ->rend->local->ToLocal(ray,locRay);
  
    // Intercept, work out the distance if relevent, making sure to scale it correctly...
     real32 distance;
     if (targ->rend->object->Intercept(locRay,distance))
     {
      targ->rend->local->ToWorld(distance,distance);
      if ((ret==false)||(distance<bestDistance))
      {
       ret = true;
       objOut = targ->rend;
       bestDistance = distance;
      }
     }
   }
   ++targ;
  }
 
 // We know which the closest object is, now get a full intersection object....
  if (ret)
  {
   bs::Ray locRay;
   objOut->local->ToLocal(ray,locRay);
   log::Assert(objOut->object->Intercept(locRay,intOut));
   intOut.ToWorld(*objOut->local);
  }

 return ret;
}

bit BruteDB::Intercept(const bs::FiniteLine & line,Renderable *& objOut,Intersection & intOut) const
{
 // Simply convert the line into a ray, then fail if it intercepts past the end of the ray...
  bs::Ray ray;
   ray.s = line.s;
   ray.n = line.e;
   ray.n -= line.s;
   real32 lineLength = ray.n.Length();
   ray.n /= lineLength;
   
  bit ret = Intercept(ray,objOut,intOut);
  real32 objDepth;
  objOut->local->ToWorld(intOut.depth,objDepth);
  if (ret)
  {
   bit ret = objDepth < lineLength;
   if (ret) intOut.ToWorld(*objOut->local);
   return ret;
  }
  else return false;
}

cstrconst BruteDB::TypeString() const
{
 return "eos::rend::BruteDB";
}

//------------------------------------------------------------------------------
 };
};
