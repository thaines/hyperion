//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void Intersection::ToWorld(const OpTran & tran)
{
 {
  bs::Vert temp = point;
  tran.ToWorld(temp,point);
 }
 
 tran.ToWorld(depth,depth);
 
 {
  bs::Normal temp = norm;
  tran.ToWorld(temp,norm);
  temp = axis[0];
  tran.ToWorld(temp,axis[0]);
  temp = axis[1];
  tran.ToWorld(temp,axis[1]);  
 }
 
 {
  bs::Normal temp = velocity;
  real32 speed = temp.Length();
  temp /= speed;
  
  bs::Normal out;
  tran.ToWorld(temp,out);
  tran.ToWorld(speed,speed);
  
  velocity = out;
  velocity *= speed;
 }
}

//------------------------------------------------------------------------------
RayImage::RayImage(nat32 w,nat32 h,nat32 expR,nat32 blockR)
:width(w),height(h),expRays(expR),blockRays(blockR),
sizeOfThing(sizeof(Head) + sizeof(Node) + sizeof(TaggedRay)*expRays)
{
 data = mem::Malloc<byte>(sizeOfThing*width*height);

 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   Head * t1 = Entry(x,y);
    t1->rays = 0;
    t1->weightSum = 0.0;
    
   Node * t2 = t1->Start();
    t2->next = null<Node*>();
  }
 }
}

RayImage::~RayImage()
{
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   Node * targ = Entry(x,y)->Start();
   while (targ->next)
   {
    Node * victim = targ->next;
    targ->next = targ->next->next;
    mem::Free(victim);
   }
  }
 }
 
 mem::Free(data); 
}

nat32 RayImage::Width() const
{
 return width;
}

nat32 RayImage::Height() const
{
 return height;
}

nat32 RayImage::Rays(nat32 x,nat32 y) const
{
 return Entry(x,y)->rays;
}

real32 RayImage::RaysWeightSum(nat32 x,nat32 y) const
{
 return Entry(x,y)->weightSum;
}

TaggedRay & RayImage::Ray(nat32 x,nat32 y,nat32 index)
{
 Node * targ = Entry(x,y)->Start();
 if (index<expRays) return targ->First()[index];
 
 index -= expRays;
 while (true)
 {
  targ = targ->next;
  if (index<blockRays) return targ->First()[index];
  index -= blockRays;
 }
}

const TaggedRay & RayImage::Ray(nat32 x,nat32 y,nat32 index) const
{
 Node * targ = Entry(x,y)->Start();
 if (index<expRays) return targ->First()[index];
 
 index -= expRays;
 while (true)
 {
  targ = targ->next;
  if (index<blockRays) return targ->First()[index];
  index -= blockRays;
 }
}

void RayImage::RemRay(nat32 x,nat32 y,nat32 index)
{
 Head * head = Entry(x,y); 

 if ((index+1)==head->rays)
 {
  head->weightSum -= Ray(x,y,index).weight;
  head->rays -= 1;
 }
 else
 {
  // Get a pointer to the ray...
   TaggedRay * victim = &Ray(x,y,index);

  // Get a pointer to the last ray stored...
   TaggedRay * newGuy = &Ray(x,y,head->rays-1);

  // Update...
   head->weightSum -= victim->weight;
   mem::Copy(victim,newGuy);
   head->rays -= 1;
 }
}

nat32 RayImage::AddRay(nat32 x,nat32 y,const TaggedRay & in)
{
 Head * head = Entry(x,y);
 nat32 ret = head->rays;
 head->rays += 1;
 head->weightSum += in.weight;


 Node * targ = head->Start();
 nat32 index = ret;
 if (index<expRays)
 {
  mem::Copy(&targ->First()[index],&in);
  return ret;
 }
 
 index -= expRays;
 while (true)
 {
  if (targ->next==null<Node*>()) targ->next = (Node*)(void*)mem::Malloc<byte*>(sizeof(Node) + sizeof(TaggedRay)*blockRays);
  targ = targ->next;
 
  if (index<blockRays)
  {
   mem::Copy(&targ->First()[index],&in);
   return ret;
  }

  index -= blockRays;
 }
}

//------------------------------------------------------------------------------
Job::Job()
:db(null<RenderableDB*>()),renderer(null<Renderer*>()),
bg(null<Background*>()),viewer(null<Viewer*>()),
sampler(null<Sampler*>()),tm(null<ToneMapper*>()),
ri(null<RayImage*>())
{}

Job::~Job()
{
 delete db;
 delete renderer;
 delete bg;
 delete viewer;
 delete sampler;
 delete tm;
}

void Job::Setup(RenderableDB * d,Renderer * r,
                Background * b,Viewer * v,
                Sampler * s,ToneMapper * t)
{
 db = d;
 renderer = r;
 bg = b;
 viewer = v;
 sampler = s;
 tm = t;
}

void Job::Add(PostProcessor * pp)
{
 posts.AddBack(pp);
}

void Job::Add(Light * light)
{
 lights.AddBack(light);
}
 
void Job::Add(Renderable * obj)
{
 db->Add(obj);
 objs.AddBack(obj);
}

void Job::Register(OpTran * tran)
{
 trans.AddBack(tran);
}

void Job::Register(Object * obj)
{
 objects.AddBack(obj);
}

void Job::Register(IntersectionModifier * im)
{
 ims.AddBack(im);
}

void Job::Register(Material * mat)
{
 materials.AddBack(mat);
}

void Job::Register(Texture * tex)
{
 textures.AddBack(tex);
}

void Job::Render(time::Progress * prog)
{
 prog->Push();
  nat32 step = 0;
  nat32 steps = posts.Size()+4;
  
  // Prepare...
   prog->Report(step,steps);
   db->Prepare(prog);
   ++step;   
  
  // Create the ray image...
   prog->Report(step,steps);
   delete ri;
   ri = new RayImage(viewer->Width(),viewer->Height(),sampler->Samples());
   ++step;
  
  // Call the actual renderer...
   prog->Report(step,steps);
   sampler->Render(*this,prog);
   ++step;
 
  // Apply all post-proccessing effects...
   ds::List<PostProcessor*,mem::KillDel<PostProcessor> >::Cursor targ = posts.FrontPtr();
   while (!targ.Bad())
   {
    prog->Report(step,steps);
    (*targ)->Apply(*ri,prog); 
    ++targ;
    ++step;
   }
   
  // Un-prepare...
   prog->Report(step,steps);
   db->Unprepare(prog);   
 
 prog->Pop();
}

void Job::Render(svt::Field<bs::ColourRGB> & out,time::Progress * prog)
{
 prog->Push();
  nat32 step = 0;
  nat32 steps = posts.Size()+5;
  
  // Prepare...
   prog->Report(step,steps);
   db->Prepare(prog);
   ++step;  
  
  // Create the ray image...
   prog->Report(step,steps);
   delete ri;
   ri = new RayImage(viewer->Width(),viewer->Height(),sampler->Samples());
   ++step;
  
  // Call the actual renderer...
   prog->Report(step,steps);
   sampler->Render(*this,prog);
   ++step;
 
  // Apply all post-processing effects...
   ds::List<PostProcessor*,mem::KillDel<PostProcessor> >::Cursor targ = posts.FrontPtr();
   while (!targ.Bad())
   {
    prog->Report(step,steps);
    (*targ)->Apply(*ri,prog); 
    ++targ;
    ++step;
   }
   
  // Apply the tone mapper...
   prog->Report(step,steps);
   tm->Apply(*ri,out,prog);
   ++step;
   
  // Un-prepare...
   prog->Report(step,steps);
   db->Unprepare(prog);   
 
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
