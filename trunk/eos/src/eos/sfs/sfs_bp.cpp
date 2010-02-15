//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sfs/sfs_bp.h"

#include "eos/ds/arrays.h"
#include "eos/filter/grad_walk.h"
#include "eos/filter/kernel.h"
#include "eos/inf/bin_bp_2d.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
SfS_BP::SfS_BP()
:iters(8),maxLevels(32),doML(true)
{}

SfS_BP::~SfS_BP()
{}

void SfS_BP::SetSize(nat32 width,nat32 height)
{
 data.Resize(width,height);
 horiz.Resize(width,height);
 vert.Resize(width,height);
 dist.Resize(width,height);
 distMax.Resize(width,height);
 
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   data.Get(x,y) = math::FisherBingham();
   horiz.Get(x,y) = 0.0;
   vert.Get(x,y) = 0.0;
   dist.Get(x,y) = math::FisherBingham();
   distMax.Get(x,y) = bs::Normal(0.0,0.0,1.0);
  }
 }
}

void SfS_BP::SetHoriz(nat32 x,nat32 y,real32 k)
{
 horiz.Get(x,y) = k;
}

void SfS_BP::SetVert(nat32 x,nat32 y,real32 k)
{
 vert.Get(x,y) = k;
}

void SfS_BP::SetDist(nat32 x,nat32 y,const math::FisherBingham & fb)
{
 data.Get(x,y) = fb;
}

void SfS_BP::MultDist(nat32 x,nat32 y,const math::Fisher & f)
{
 data.Get(x,y) *= f;
}

void SfS_BP::MultDist(nat32 x,nat32 y,const math::Bingham & b)
{
 data.Get(x,y) *= b;
}

void SfS_BP::MultDist(nat32 x,nat32 y,const math::FisherBingham & fb)
{
 data.Get(x,y) *= fb;
}

void SfS_BP::SetCone(nat32 x,nat32 y,real32 irr,real32 albedo,const bs::Normal & light,real32 k)
{
 if (math::IsZero(irr)) data.Get(x,y) = math::FisherBingham();
 else
 {
  bs::Normal dir = light; dir.Normalise();
  real32 ang = math::InvCos(math::Min(irr/(albedo*light.Length()),real32(1.0)));
  data.Get(x,y) = math::FisherBingham(dir,ang,k);
 }
}

void SfS_BP::SetCone(const svt::Field<real32> & irr,const svt::Field<real32> & albedo,const svt::Field<bs::Normal> & light,const svt::Field<real32> & k)
{
 log::Assert((irr.Size(0)==albedo.Size(0))&&(irr.Size(0)==light.Size(0))&&(irr.Size(0)==k.Size(0)));
 log::Assert((irr.Size(1)==albedo.Size(1))&&(irr.Size(1)==light.Size(1))&&(irr.Size(1)==k.Size(1)));

 SetSize(irr.Size(0),irr.Size(1));
 
 for (nat32 y=0;y<irr.Size(1);y++)
 {
  for (nat32 x=0;x<irr.Size(0);x++) SetCone(x,y,irr.Get(x,y),albedo.Get(x,y),light.Get(x,y),k.Get(x,y));
 }
}

void SfS_BP::SetHorizVert(const svt::Field<real32> & h,const svt::Field<real32> & v)
{
 log::Assert((horiz.Width()==h.Size(0))&&(vert.Width()==v.Size(0)));
 log::Assert((horiz.Height()==h.Size(1))&&(vert.Height()==v.Size(1)));

 for (nat32 y=0;y<horiz.Height();y++)
 {
  for (nat32 x=0;x<horiz.Width();x++)
  {
   horiz.Get(x,y) = h.Get(x,y);
   vert.Get(x,y) = v.Get(x,y);
  }
 }
}

void SfS_BP::MultDist(const svt::Field<math::Fisher> & pr)
{
 log::Assert((data.Width()==pr.Size(0))&&(data.Height()==pr.Size(1)));
 
 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++) data.Get(x,y) *= pr.Get(x,y);
 }
}

void SfS_BP::MultDist(const svt::Field<math::Bingham> & pr)
{
 log::Assert((data.Width()==pr.Size(0))&&(data.Height()==pr.Size(1)));
 
 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++) data.Get(x,y) *= pr.Get(x,y);
 }
}

void SfS_BP::MultDist(const svt::Field<math::FisherBingham> & pr)
{
 log::Assert((data.Width()==pr.Size(0))&&(data.Height()==pr.Size(1)));
 
 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++) data.Get(x,y) *= pr.Get(x,y);
 }
}

void SfS_BP::SetIters(nat32 it,nat32 lev)
{
 iters = it;
 maxLevels = lev;
}

void SfS_BP::DisableNorms()
{
 doML = false;
}

void SfS_BP::Run(time::Progress * prog)
{
 prog->Push();
 
 // Calculate the parameters of the hierachy, create its data structure...
  nat32 levels = math::Min(math::TopBit(math::Max(data.Width(),data.Height())),maxLevels);
  ds::ArrayDel< ds::Array2D<MsgSet> > level(levels);
  
  nat32 step = 0;
  nat32 steps = (levels*2-1) + 3 + (doML?1:0);
  
  prog->Report(step++,steps);
  prog->Push();
  prog->Report(0,levels);
  level[0].Resize(data.Width(),data.Height());
  for (nat32 l=1;l<levels;l++)
  {
   prog->Report(l,levels);
   nat32 nWidth  = (level[l-1].Width()/2) + (((level[l-1].Width()%2)==0)?0:1);
   nat32 nHeight = (level[l-1].Height()/2) + (((level[l-1].Height()%2)==0)?0:1);
   level[l].Resize(nWidth,nHeight);
  }
  prog->Pop();



 // Fill in the distributions from the data array, zero out the messages at the
 // same time and fill in the send flags...
  prog->Report(step++,steps);
  prog->Push();
  prog->Report(0,levels);
  prog->Push();
  for (nat32 y=0;y<data.Height();y++)
  {
   prog->Report(y,data.Height());
   for (nat32 x=0;x<data.Width();x++)
   {
    level[0].Get(x,y).data = data.Get(x,y);
    for (nat32 d=0;d<4;d++)
    {
     level[0].Get(x,y).in[d] = math::FisherBingham();
      
     int32 ox = x;
     int32 oy = y;
     switch (d)
     {
      case 0: ++ox; break;
      case 1: ++oy; break;
      case 2: --ox; break;
      case 3: --oy; break;
     }
     
     if ((ox>=0)&&(ox<int32(level[0].Width()))&&(oy>=0)&&(oy<int32(level[0].Height())))
     {
      switch (d)
      {
       case 0: level[0].Get(x,y).send[d] = horiz.Get(x,y); break;
       case 1: level[0].Get(x,y).send[d] = vert.Get(x,y); break;
       case 2: level[0].Get(x,y).send[d] = horiz.Get(ox,oy); break;
       case 3: level[0].Get(x,y).send[d] = vert.Get(ox,oy); break;
      }
     }
     else
     {
      level[0].Get(x,y).send[d] = -1.0;
     }
    }
   }
  }
  prog->Pop();
  
  for (nat32 l=1;l<levels;l++)
  {
   prog->Report(l,levels);
   prog->Push();
   for (nat32 y=0;y<level[l].Height();y++)
   {
    prog->Report(y,level[l].Height());
    for (nat32 x=0;x<level[l].Width();x++)
    {
     nat32 oldX = x*2;
     nat32 oldY = y*2;
     bit incX = (oldX+1)<level[l-1].Width();
     bit incY = (oldY+1)<level[l-1].Height();
     
     real32 k[4];
     real32 div = 1.0;
     level[l].Get(x,y).data = level[l-1].Get(oldX,oldY).data;
     for (nat32 d=0;d<4;d++) k[d] = level[l-1].Get(oldX,oldY).send[d];
     
     if (incX)
     {
      level[l].Get(x,y).data *= level[l-1].Get(oldX+1,oldY).data;
      div += 1.0;
      for (nat32 d=0;d<4;d++) k[d] += level[l-1].Get(oldX+1,oldY).send[d];
     }
     if (incY)
     {
      level[l].Get(x,y).data *= level[l-1].Get(oldX,oldY+1).data;
      div += 1.0;
      for (nat32 d=0;d<4;d++) k[d] += level[l-1].Get(oldX,oldY+1).send[d];
     }
     if (incX&&incY)
     {
      level[l].Get(x,y).data *= level[l-1].Get(oldX+1,oldY+1).data;
      div += 1.0;
      for (nat32 d=0;d<4;d++) k[d] += level[l-1].Get(oldX+1,oldY+1).send[d];
     }
     
     level[l].Get(x,y).data /= div;

     for (nat32 d=0;d<4;d++)
     {
      level[l].Get(x,y).in[d] = math::FisherBingham();
      
      int32 ox = x;
      int32 oy = y;
      switch (d)
      {
       case 0: ++ox; break;
       case 1: ++oy; break;
       case 2: --ox; break;
       case 3: --oy; break;
      }
      
      if ((ox>=0)&&(ox<int32(level[l].Width()))&&(oy>=0)&&(oy<int32(level[l].Height())))
      {
       level[l].Get(x,y).send[d] = k[d]/div;
      }
      else
      {
       level[l].Get(x,y).send[d] = -1.0;
      }
     }
    }
   }
   prog->Pop();
  }
  prog->Pop();


 // Iterate the hierachy, passing messages at each level and then transfering down...
  for (nat32 l=levels-1;;l--)
  {
   // Iterate the level, passing messages...
    prog->Report(step++,steps);
    prog->Push();
    for (nat32 it=0;it<iters;it++)
    {
     prog->Report(it,iters);
     prog->Push();
     for (nat32 y=0;y<level[l].Height();y++)
     {
      prog->Report(y,level[l].Height());
      for (nat32 x=(it+y)%2;x<level[l].Width();x+=2)
      {
       MsgSet & ms = level[l].Get(x,y);
       //LogDebug("Calculating (" << x << "," << y << ") at level " << l);
       //LogDebug("Data term" << LogDiv() << ms.data);
       for (nat32 d=0;d<4;d++)
       {
        //LogDebug("Message from dir " << d << LogDiv() << ms.in[d]);
        if (ms.send[d]>0.0)
        {
         // Calculate the coordinates of the neighbour...
          int32 ox = x;
          int32 oy = y;
          switch (d)
          {
           case 0: ++ox; break;
           case 1: ++oy; break;
           case 2: --ox; break;
           case 3: --oy; break;
          }
         
         // Calculate the message...
          math::FisherBingham msg = ms.data;
          for (nat32 i=0;i<d;i++) msg *= ms.in[i];
          for (nat32 i=d+1;i<4;i++) msg *= ms.in[i];
          
          if (msg.Bad())
          {
           LogDebug("pre conv nan" << LogDiv() << l << LogDiv() << it << LogDiv() << x << LogDiv() 
                    << y << LogDiv() << d << LogDiv() << msg.fisher << ";" << msg.bingham);
          }          
          
          //LogDebug("Pre-convolution" << LogDiv() << msg);
          msg.Convolve(ms.send[d]);
          
          if (msg.Bad())
          {
           LogDebug("post conv nan" << LogDiv() << l << LogDiv() << it << LogDiv() << x << LogDiv() 
                    << y << LogDiv() << d << LogDiv() << msg.fisher << ";" << msg.bingham);
          }
                   
         // Send...
          level[l].Get(ox,oy).in[(d+2)%4] = msg;
          //LogDebug("Sent message for direction " << d << LogDiv() << msg);
        }
       }
      }
     }
     prog->Pop();
    }
    prog->Pop();


   // Break if last level...
    if (l==0) break;
    
    
   // Pass down to level below, ready for its iterations...
    prog->Report(step++,steps);
    prog->Push();
    for (nat32 y=0;y<level[l-1].Height();y++)
    {
     prog->Report(y,level[l-1].Height());
     for (nat32 x=0;x<level[l-1].Width();x++)
     {
      nat32 hX = x/2;
      nat32 hY = y/2;
      for (nat32 d=0;d<4;d++) level[l-1].Get(x,y).in[d] = level[l].Get(hX,hY).in[d];
     }
    }
    prog->Pop();
  }


 // Extract the final beliefs and store in the dist data structure...
  prog->Report(step++,steps);
  prog->Push();
  for (nat32 y=0;y<dist.Height();y++)
  {
   prog->Report(y,dist.Height());
   for (nat32 x=0;x<dist.Width();x++)
   {
    dist.Get(x,y) = level[0].Get(x,y).data;
    for (nat32 d=0;d<4;d++) dist.Get(x,y) *= level[0].Get(x,y).in[d];
   }
  }
  prog->Pop();


 // Calculate the distMax data structure...
  if (doML)
  {
   prog->Report(step++,steps);
   prog->Push();
   for (nat32 y=0;y<distMax.Height();y++)
   {
    prog->Report(y,distMax.Height());
    for (nat32 x=0;x<distMax.Width();x++)
    {
     dist.Get(x,y).Maximum(distMax.Get(x,y));
     if (!math::IsFinite(distMax.Get(x,y).Length()))
     {
      LogDebug("NaN maximum {x,y,dir,fish,bing}" << LogDiv() << x << LogDiv() << y << LogDiv()
               << distMax.Get(x,y) << LogDiv() << dist.Get(x,y).fisher << LogDiv() << dist.Get(x,y).bingham);
     }
    }
   }
   prog->Pop();
  }

 prog->Pop();
}

void SfS_BP::GetDist(nat32 x,nat32 y,math::FisherBingham & out) const
{
 out = dist.Get(x,y);
}

void SfS_BP::GetDist(svt::Field<math::FisherBingham> & out) const
{
 for (nat32 y=0;y<dist.Height();y++)
 {
  for (nat32 x=0;x<dist.Width();x++) out.Get(x,y) = dist.Get(x,y);
 }
}

void SfS_BP::GetDir(nat32 x,nat32 y,bs::Normal & out) const
{
 out = distMax.Get(x,y);
}

void SfS_BP::GetDir(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<distMax.Height();y++)
 {
  for (nat32 x=0;x<distMax.Width();x++) out.Get(x,y) = distMax.Get(x,y);
 }
}

cstrconst SfS_BP::TypeString() const
{
 return "eos::sfs::SfS_BP";
}

//------------------------------------------------------------------------------
SfS_BP_Nice::SfS_BP_Nice()
:blur(math::Sqrt(2.0)),gradLength(8),gradExp(12.0),gradStopChance(0.0), 
simK(4.5),coneK(16.0),fadeTo(0.0),   
gradDisc(4.0),gradBias(4.0),borderK(0.0),   
project(false),iters(8),
toLight(0.0,0.0,1.0)
{}

SfS_BP_Nice::~SfS_BP_Nice()
{}

void SfS_BP_Nice::SetImage(const svt::Field<real32> & i)
{
 image = i;
}

void SfS_BP_Nice::SetAlbedo(const svt::Field<real32> & a)
{
 albedo = a;
}

void SfS_BP_Nice::SetLight(bs::Normal & norm)
{
 toLight = norm;
 toLight.Normalise();
}

void SfS_BP_Nice::SetParasGrad(real32 b,nat32 length,real32 exp,real32 stopChance)
{
 blur = b;
 gradLength = length; 
 gradExp = exp;
 gradStopChance = stopChance;
}

void SfS_BP_Nice::SetParasCore(real32 sK,real32 cK,real32 fT,nat32 i)
{
 simK = sK;
 coneK = cK;
 fadeTo = fT;
 iters = i;
}

void SfS_BP_Nice::SetParasExtra(real32 gDisc,real32 gBias,real32 bK,bit p)
{
 gradDisc = gDisc;
 gradBias = gBias;
 borderK = bK;
 project = p;
}

void SfS_BP_Nice::Run(time::Progress * prog)
{
 prog->Push();
 nat32 step = 0;
 nat32 steps = 5;

 // Apply the blur to the input image...
  prog->Report(step++,steps);
  real32 realIni = 0.0;
  svt::Var temp(image);
  temp.Add("l",realIni);
  temp.Add("dx",realIni);
  temp.Add("dy",realIni);
  temp.Commit();
  
  svt::Field<real32> blurImage(&temp,"l");
  
  if (math::IsZero(blur)) blurImage.CopyFrom(image);
  else
  {
   filter::KernelVect kernel(math::Max(nat32(2),nat32(math::RoundUp(2.0*blur))));
   kernel.MakeGaussian(math::Sqrt(2.0));

   kernel.Apply(image,blurImage);
  }


 // Calculate the gradient from the blured image...
  prog->Report(step++,steps);
  svt::Field<real32> dx(&temp,"dx");
  svt::Field<real32> dy(&temp,"dy");
  filter::GradWalkPerfect(blurImage,dx,dy,gradLength,gradExp,gradStopChance,prog);
 
 
 // Create and fill in the SfS object we are abstracting...
  prog->Report(step++,steps);
  sfs::SfS_BP sfsbp;
  sfsbp.SetSize(image.Size(0),image.Size(1));
  
  prog->Push();
  for (nat32 y=0;y<image.Size(1);y++)
  {
   prog->Report(y,image.Size(1));
   for (nat32 x=0;x<image.Size(0);x++)
   {
    // Similarity terms...
     sfsbp.SetHoriz(x,y,simK);
     sfsbp.SetVert(x,y,simK);

    // Cone term...
     real32 corL = math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0));
     sfsbp.SetCone(x,y,image.Get(x,y),albedo.Get(x,y),toLight,coneK*(1.0 - corL*(1.0-fadeTo)));
     
    // Gradiant disc term...
    {
     math::Vect<3,real32> dir;
     dir[0] = dy.Get(x,y) - toLight[0];
     dir[1] = -dx.Get(x,y) - toLight[1];
     dir[2] = 0.0;
     real32 len = dir.Length();
     if (!math::IsZero(len))
     {
      dir.Normalise();
      math::FisherBingham fb(len*gradDisc,dir);
      sfsbp.MultDist(x,y,fb);
     }
    }

    // Gradiant bias term...
    {
     math::Vect<3,real32> dir;
     dir[0] = dx.Get(x,y);
     dir[1] = dy.Get(x,y);
     dir[2] = 0.0;
     real32 len = dir.Length();
     if (!math::IsZero(len))
     {
      dir.Normalise();
      if (gradBias>0.0) dir *= -1.0;
      
      // Rotate vector onto cone...
       bs::Normal grad;
       CrossProduct(toLight,dir,grad);

       real32 coneAng = math::InvCos(math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0)));
       
       math::Mat<3,3> rotMat;
       AngAxisToRotMat(grad,coneAng,rotMat);
       MultVect(rotMat,toLight,dir);
      
      // Now add the fisher distribution...
       math::Fisher f(dir,math::Abs(len*gradBias));
       sfsbp.MultDist(x,y,f);
     }
    }
    
    // Border term...
     if (!math::IsZero(image.Get(x,y)))
     {
      bit boundary = false;
      boundary |= (x!=0) && (math::IsZero(image.Get(x-1,y)));
      boundary |= (x+1<image.Size(0)) && (math::IsZero(image.Get(x+1,y)));
      boundary |= (y!=0) && (math::IsZero(image.Get(x,y-1)));
      boundary |= (y+1<image.Size(1)) && (math::IsZero(image.Get(x,y+1)));
     
      if (boundary)
      {
       math::Fisher fish;
       fish[0] = -dx.Get(x,y);
       fish[1] = -dy.Get(x,y);
       fish[2] = 0.0;
       
       real32 len = fish.Length();
       if (!math::IsZero(len))
       {
        fish *= borderK/len;
        sfsbp.MultDist(x,y,fish);
       }
      }
     }
   }
  }
  prog->Pop();
 
 
 // Run...
  prog->Report(step++,steps);
  sfsbp.SetIters(iters);
  sfsbp.Run(prog);


 // Extract the results...
  prog->Report(step++,steps);
  result.Resize(image.Size(0),image.Size(1));
  for (nat32 y=0;y<result.Height();y++)
  {
   for (nat32 x=0;x<result.Width();x++) sfsbp.GetDir(x,y,result.Get(x,y));
  }
  
 
 // If requested project the results back onto the cones...
  if (project)
  {
   for (nat32 y=0;y<result.Height();y++)
   {
    for (nat32 x=0;x<result.Width();x++)
    {
     // Rotate vector onto cone...
      bs::Normal grad;
      CrossProduct(toLight,result.Get(x,y),grad);

      real32 coneAng = math::InvCos(math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0)));
      
      math::Mat<3,3> rotMat;
      AngAxisToRotMat(grad,coneAng,rotMat);
      MultVect(rotMat,toLight,result.Get(x,y));   
    }
   }
  }

 prog->Pop();
}

void SfS_BP_Nice::GetNeedle(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = result.Get(x,y);
 }
}

cstrconst SfS_BP_Nice::TypeString() const
{
 return "eos::sfs::SfS_BP_Nice";
}

//------------------------------------------------------------------------------
NeedleFromFB::NeedleFromFB()
:momentum(0.01),angMult(1.0)
{}

NeedleFromFB::~NeedleFromFB()
{}

void NeedleFromFB::Fill(const svt::Field<math::FisherBingham> & source)
{
 in.Resize(source.Size(0),source.Size(1));
 out.Resize(source.Size(0),source.Size(1));
 
 for (nat32 y=0;y<in.Height();y++)
 {
  for (nat32 x=0;x<in.Width();x++) in.Get(x,y) = source.Get(x,y);
 }
}

void NeedleFromFB::Fill(const SfS_BP & source)
{
 in.Resize(source.Width(),source.Height());
 out.Resize(source.Width(),source.Height());
 
 for (nat32 y=0;y<in.Height();y++)
 {
  for (nat32 x=0;x<in.Width();x++) source.GetDist(x,y,in.Get(x,y));
 }
}

void NeedleFromFB::SetParas(real32 am,real32 m)
{
 momentum = m;
 angMult = am;
}

void NeedleFromFB::Run(time::Progress * prog)
{
 prog->Push();
 
 // First step is to extract all the directions...
  prog->Report(0,3);
  ds::Array2D<DirPair> dirs(in.Width(),in.Height());
  prog->Push();
  for (nat32 y=0;y<in.Height();y++)
  {
   prog->Report(y,in.Height());
   prog->Push();
   for (nat32 x=0;x<in.Width();x++)
   {
    prog->Report(x,in.Width());
    math::Vect<3> vect[6];
    LogDebug("{x,y,fb}" << LogDiv() << x << LogDiv() << y << LogDiv() 
             << "[" << in.Get(x,y).fisher << ";" << in.Get(x,y).bingham << "]");
    nat32 num = in.Get(x,y).Critical(vect);
    switch(num)
    {
     case 0:
      dirs.Get(x,y)[0] = bs::Normal(0.0,-1.0,0.0);
      dirs.Get(x,y)[1] = bs::Normal(0.0,1.0,0.0);
     break;
     case 1:
      dirs.Get(x,y)[0] = vect[0];
      dirs.Get(x,y)[1] = vect[0];
     break;
     case 2:
      dirs.Get(x,y)[0] = vect[0];
      dirs.Get(x,y)[1] = vect[1];
     break;
     default:
     {
      real32 cost[6];
      for (nat32 i=0;i<num;i++) cost[i] = in.Get(x,y).Cost(vect[i]);
      
      nat32 ind[2];
      if (cost[0]<cost[1])
      {
       ind[0] = 0; ind[1] = 1;
      }
      else
      {
       ind[0] = 1; ind[1] = 0;
      }
      
      for (nat32 i=2;i<num;i++)
      {
       if (cost[i]<cost[ind[1]])
       {
        if (cost[i]<cost[ind[0]])
        {
         ind[1] = ind[0];
         ind[0] = i;
        }
        else
        {
         ind[1] = i;
        }
       }
      }
      
      dirs.Get(x,y)[0] = vect[ind[0]];
      dirs.Get(x,y)[1] = vect[ind[1]];
     }
     break;
    }
   }
   prog->Pop();
  }
  prog->Pop();


 // Second step is to build the belief propagation solver, filling in all the 
 // relevant parameters...
  prog->Report(1,3);
  inf::BinBP2D bp;
  bp.SetSize(in.Width(),in.Height());
  bp.SetMomentum(momentum);
  bp.SetEnd(1e-4,(in.Width()+in.Height())*2);
  
  prog->Push();
  for (nat32 y=0;y<in.Height();y++)
  {
   prog->Report(y,in.Height());
   prog->Push();
   for (nat32 x=0;x<in.Width();x++)
   {
    prog->Report(x,in.Width());
    math::FisherBingham & fb = in.Get(x,y);
    
    // The probabilities of the assigning either of the labels...
    {
     real32 normDiv = fb.NormDiv();
     real32 probFalse = -math::Ln(fb.UnnormProb(dirs.Get(x,y)[0])/normDiv);
     real32 probTrue  = -math::Ln(fb.UnnormProb(dirs.Get(x,y)[1])/normDiv);
     real32 minProb = math::Min(probFalse,probTrue);
     bp.SetCost(x,y,false,probFalse-minProb);
     bp.SetCost(x,y,true ,probTrue-minProb);
    }
    
    // The probabilities on the x axis...
     if (x+1<in.Width())
     {
      for (nat32 i=0;i<2;i++)
      {
       for (nat32 j=0;j<2;j++)
       {
        bp.SetCostX(x,y,i==1,j==1,angMult*math::InvCos(dirs.Get(x,y)[i]*dirs.Get(x+1,y)[j]));
       }
      }
     }

    // The probabilities on the y axis...
     if (y+1<in.Height())
     {
      for (nat32 i=0;i<2;i++)
      {
       for (nat32 j=0;j<2;j++)
       {
        bp.SetCostY(x,y,i==1,j==1,angMult*math::InvCos(dirs.Get(x,y)[i]*dirs.Get(x,y+1)[j]));
       }
      }
     }
   }
   prog->Pop();
  }
  prog->Pop();
  
 
 // Third step is to solve and extract the result...
  prog->Report(2,3);
  bp.Run(prog);
  
  out.Resize(in.Width(),in.Height());
  for (nat32 y=0;y<in.Height();y++)
  {
   for (nat32 x=0;x<in.Width();x++)
   {
    out.Get(x,y) = dirs.Get(x,y)[bp.State(x,y)?1:0];
   }
  }
 
 prog->Pop();
}

void NeedleFromFB::GetDir(nat32 x,nat32 y,bs::Normal & o) const
{
 o = out.Get(x,y);
}

void NeedleFromFB::GetDir(svt::Field<bs::Normal> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) o.Get(x,y) = out.Get(x,y);
 }
}

cstrconst NeedleFromFB::TypeString() const
{
 return "eos::sfs::NeedleFromFB";
}

//------------------------------------------------------------------------------
SfS_BP_Nice2::SfS_BP_Nice2()
:simK(4.5),coneK(16.0),fadeTo(1.0),iters(8),
boundK(2.0),boundLength(8),boundExp(6.0),
gradK(0.0),gradLength(8),gradExp(6.0),
angMult(1.0),momentum(0.01),
toLight(0.0,0.0,1.0)
{}

SfS_BP_Nice2::~SfS_BP_Nice2()
{}

void SfS_BP_Nice2::SetImage(const svt::Field<real32> & i)
{
 image = i;
}

void SfS_BP_Nice2::SetAlbedo(const svt::Field<real32> & a)
{
 albedo = a;
}

void SfS_BP_Nice2::SetLight(bs::Normal & norm)
{
 toLight = norm;
 toLight.Normalise();
}

void SfS_BP_Nice2::SetParasCore(real32 sk,real32 ck,real32 ft,nat32 i)
{
 simK = sk;
 coneK = ck;
 fadeTo = ft;
 iters = i;
}

void SfS_BP_Nice2::SetParasBound(real32 k,nat32 l,real32 e)
{
 boundK = k;
 boundLength = l;
 boundExp = e;
}

void SfS_BP_Nice2::SetParasGrad(real32 k,nat32 l,real32 e)
{
 gradK = k;
 gradLength = l;
 gradExp = e;
}

void SfS_BP_Nice2::SetParasExtract(real32 am,real32 m)
{
 angMult = am;
 momentum = m;
}

void SfS_BP_Nice2::Run(time::Progress * prog)
{
 prog->Push();
 nat32 step = 0;
 nat32 steps = 3;
 if ((!math::IsZero(boundK))||(!math::IsZero(gradK))) steps += 1;
 if (!math::IsZero(gradK)) steps += 1;


 // If doing boundaries or gradients we need a smoothed version of the input image...
  svt::Var temp(image);
  if ((!math::IsZero(boundK))||(!math::IsZero(gradK)))
  {
   prog->Report(step++,steps);
   real32 realIni = 0.0;
   temp.Add("l",realIni);
   temp.Commit();
  
   svt::Field<real32> blurImage(&temp,"l");
   filter::KernelVect kernel(3);
   kernel.MakeGaussian(math::Sqrt(2.0));
   kernel.Apply(image,blurImage);
  }


 // If doing gradients we need gradient information for the entire image...
  if (!math::IsZero(gradK))
  {
   prog->Report(step++,steps);
   
   real32 realIni = 0.0;
   temp.Add("dx",realIni);
   temp.Add("dy",realIni);
   temp.Commit();
   
   svt::Field<real32> blurImage(&temp,"l");
   svt::Field<real32> dx(&temp,"dx");
   svt::Field<real32> dy(&temp,"dy");
   filter::GradWalkPerfect(blurImage,dx,dy,gradLength,gradExp,0.0,prog);
  }


 // Create and fill in the SfS object we are abstracting...
  prog->Report(step++,steps);
  sfs::SfS_BP sfsbp;
  sfsbp.SetSize(image.Size(0),image.Size(1));
  
  filter::GradWalkSelect gws;
  {
   svt::Field<real32> blurImage(&temp,"l");
   gws.SetInput(blurImage);
   gws.SetParas(boundLength,boundExp);
  }
  
  
  svt::Field<real32> dx(&temp,"dx");
  svt::Field<real32> dy(&temp,"dy");
  prog->Push();
  for (nat32 y=0;y<image.Size(1);y++)
  {
   prog->Report(y,image.Size(1));
   for (nat32 x=0;x<image.Size(0);x++)
   {
    // Similarity terms...
     sfsbp.SetHoriz(x,y,simK);
     sfsbp.SetVert(x,y,simK);  

    // Cone term...
     real32 corL = math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0));
     sfsbp.SetCone(x,y,image.Get(x,y),albedo.Get(x,y),toLight,coneK*(1.0 - corL*(1.0-fadeTo)));
     
    // Boundary term...
     if ((!math::IsZero(boundK))&&(math::IsZero(image.Get(x,y))))
     {
      bit boundary = false;
      boundary |= (x>0)&&(!math::IsZero(image.Get(x-1,y)));
      boundary |= (y>0)&&(!math::IsZero(image.Get(x,y-1)));
      boundary |= (x+1<image.Size(0))&&(!math::IsZero(image.Get(x+1,y)));
      boundary |= (y+1<image.Size(1))&&(!math::IsZero(image.Get(x,y+1)));
      
      if (boundary)
      {
       real32 dx,dy;
       gws.Query(x,y,dx,dy);
       
       math::Fisher fish;
       fish[0] = -dx;
       fish[1] = -dy;
       fish[2] = 0.0;
       
       real32 len = fish.Length();
       if (!math::IsZero(len))
       {
        fish *= boundK/len;
        sfsbp.MultDist(x,y,fish);
       }
      }
     }
     
    // Gradient term...
     if (!math::IsZero(gradK))
     {
      math::Vect<3,real32> dir;
      dir[0] = dy.Get(x,y);
      dir[1] = -dx.Get(x,y);
      dir[2] = 0.0;
      real32 len = dir.Length();
      if (!math::IsZero(len))
      {
       dir /= len;
       math::FisherBingham fb(len*gradK,dir);
       sfsbp.MultDist(x,y,fb);
      }
     }
   }
  }
  prog->Pop();


 // Run...
  prog->Report(step++,steps);
  sfsbp.SetIters(iters);
  sfsbp.Run(prog);
  
 
 // Extract the results with belief propagation based selection between maxima...
  prog->Report(step++,steps);
  NeedleFromFB nffb;
  nffb.Fill(sfsbp);
  nffb.SetParas(angMult,momentum);
  
  nffb.Run(prog);
  
  result.Resize(image.Size(0),image.Size(1));
  for (nat32 y=0;y<image.Size(1);y++)
  {
   for (nat32 x=0;x<image.Size(0);x++)
   {
    sfsbp.GetDir(x,y,result.Get(x,y));
   }
  }


 prog->Pop();
}

void SfS_BP_Nice2::GetNeedle(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = result.Get(x,y);
 }
}

cstrconst SfS_BP_Nice2::TypeString() const
{
 return "eos::sfs::SfS_BP_Nice2";
}

//------------------------------------------------------------------------------
 };
};
