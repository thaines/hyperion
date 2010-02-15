//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/sfs/zheng.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
Zheng::Zheng()
:iters(512),mu(1.0),diffDelta(0.0001),toLight(0.01,0.0,1.0)
{
 toLight.Normalise();
}

Zheng::~Zheng()
{}

void Zheng::SetImage(const svt::Field<real32> & i)
{
 image = i;
}

void Zheng::SetAlbedo(const svt::Field<real32> & a)
{
 albedo = a;
}

void Zheng::SetLight(bs::Normal & norm)
{
 toLight = norm;
 if ((math::IsZero(toLight[0]))&&(math::IsZero(toLight[1]))) toLight[0] = 0.01;
 toLight.Normalise();
}

void Zheng::SetParas(real32 m,nat32 i,real32 dd)
{
 mu = m;
 iters = i;
 diffDelta = dd;
}

void Zheng::Run(time::Progress * prog)
{
 prog->Push();
 log::Assert((image.Size(0)==albedo.Size(0))&&(image.Size(1)==albedo.Size(1)));

 // Count how many hierachy levels we are going to be doing...
  nat32 levels = 1;
  {
   nat32 val = math::Min(image.Size(0),image.Size(1));
   while (val>64)
   {
    val = val/2 + (((val%2)==1)?1:0);
    ++levels;
   }
  }


 // Build a hierachy of albedo corrected irradiance and output values...
  prog->Report(0,levels+2);
  prog->Push();
  prog->Report(0,levels);
  ds::ArrayDel<ds::Array2D<real32> > irr(levels);
  ds::ArrayDel<ds::Array2D<Pixel> > data(levels);
  // Do the first layer...
   irr[0].Resize(image.Size(0),image.Size(1));
   data[0].Resize(image.Size(0),image.Size(1));
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     irr[0].Get(x,y) = math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0));

     data[0].Get(x,y).p = 0.0;
     data[0].Get(x,y).q = 0.0;
     data[0].Get(x,y).z = 0.0;
    }
   }
  
  // Do all successive layers...
   for (nat32 l=1;l<levels;l++)
   {
    prog->Report(l,levels);
    // Scale...
     nat32 prevWidth = irr[l-1].Width();
     nat32 prevHeight = irr[l-1].Height();
     nat32 width = prevWidth/2 + (((prevWidth%2)==1)?1:0);
     nat32 height = prevHeight/2 + (((prevHeight%2)==1)?1:0);
     irr[l].Resize(width,height);
     data[l].Resize(width,height);
   
    // Transfer/fill...
     for (nat32 y=0;y<height;y++)
     {
      for (nat32 x=0;x<width;x++)
      {
       // Transfer...
        bit safeX = (x*2+1) < prevWidth;
        bit safeY = (y*2+1) < prevHeight;
        
        real32 div = 1.0;
        real32 val = irr[l-1].Get(x*2,y*2);
         if (safeX) {val += irr[l-1].Get(x*2+1,y*2); ++div;}
         if (safeY) {val += irr[l-1].Get(x*2,y*2+1); ++div;}
         if (safeX&&safeY) {val += irr[l-1].Get(x*2+1,y*2+1); ++div;}
        irr[l].Get(x,y) = val/div;

       // Fill...
        data[l].Get(x,y).p = 0.0;
        data[l].Get(x,y).q = 0.0;
        data[l].Get(x,y).z = 0.0;
      }
     }
   }
  prog->Pop();


 // Iterate the hierachy, doing the iterations and transfering down...
  for (nat32 l=levels-1;;l--)
  {
   prog->Report((levels-1)-l+1,levels+2);
   // Do the iterations for this level...
    ds::Array2D<Pixel> delta(data[l].Width(),data[l].Height());
    DoIterations(irr[l],data[l],delta,prog);
   
   // If its level 0 break...
    if (l==0) break;
    
   // Transfer down for next level...
    for (nat32 y=0;y<data[l-1].Height();y++)
    {
     for (nat32 x=0;x<data[l-1].Width();x++)
     {
      nat32 pX = x/2;
      nat32 pY = y/2;
      
      bit oddX = (x%2)==1;
      bit oddY = (y%2)==1;
      if (oddX&&oddY)
      {
       data[l-1].Get(x,y) = data[l].Get(pX,pY);
       data[l-1].Get(x,y) += data[l].ClampGet(pX+1,pY);
       data[l-1].Get(x,y) += data[l].ClampGet(pX,pY+1);
       data[l-1].Get(x,y) += data[l].ClampGet(pX+1,pY+1);
       data[l-1].Get(x,y).p *= 0.25;
       data[l-1].Get(x,y).q *= 0.25;
       data[l-1].Get(x,y).z *= 0.5;
      }
      else
      {
       if (oddX)
       {
        data[l-1].Get(x,y) = data[l].Get(pX,pY);
        data[l-1].Get(x,y) += data[l].ClampGet(pX+1,pY);
        data[l-1].Get(x,y).p *= 0.5;
        data[l-1].Get(x,y).q *= 0.5;
       }
       else
       {
        if (oddY)
        {
         data[l-1].Get(x,y) = data[l].Get(pX,pY);
         data[l-1].Get(x,y) += data[l].ClampGet(pX,pY+1);
         data[l-1].Get(x,y).p *= 0.5;
         data[l-1].Get(x,y).q *= 0.5;
        }
        else
        {
         data[l-1].Get(x,y) = data[l].Get(pX,pY);
         data[l-1].Get(x,y).z *= 2.0;
        }
       }
      }
     }
    }
  }
 
 
 // Extract the data into a perminant data structure...
  prog->Report(levels+1,levels+2);
  out.Resize(data[0].Width(),data[0].Height());
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++) out.Get(x,y) = data[0].Get(x,y);
  }
 
 prog->Pop();
}

void Zheng::GetNeedle(svt::Field<bs::Normal> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y)[0] = -out.Get(x,y).p; // out.Get(x,y).z - out.ClampGet(x+1,y).z;
   o.Get(x,y)[1] = -out.Get(x,y).q; // out.Get(x,y).z - out.ClampGet(x,y+1).z;
   o.Get(x,y)[2] = 1.0;
   o.Get(x,y).Normalise();
  }
 }
}

void Zheng::GetDepth(svt::Field<real32> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = out.Get(x,y).z;
  }
 }
}

cstrconst Zheng::TypeString() const
{
 return "eos::sfs::Zheng";
}

//------------------------------------------------------------------------------
void Zheng::DoIterations(ds::Array2D<real32> & irr,ds::Array2D<Pixel> & data,ds::Array2D<Pixel> & delta,time::Progress * prog)
{
 prog->Push();
 // Iterate until the cap is hit, unless we break early due to the deltas being zero...
  for (nat32 i=0;i<iters;i++)
  {
   prog->Report(i,iters);

   // Calculate delta on a per-pixel basis...
    for (int32 y=0;y<int32(data.Height());y++)
    {
     for (int32 x=0;x<int32(data.Width());x++)
     {
      // Calculate the many differentials...
       real32 Px = data.ClampGet(x+1,y).p - data.Get(x,y).p;
       real32 Pxx = data.ClampGet(x+1,y).p + data.ClampGet(x-1,y).p - 2.0*data.Get(x,y).p;
       real32 Pyy = data.ClampGet(x,y+1).p + data.ClampGet(x,y-1).p - 2.0*data.Get(x,y).p;

       real32 Qy = data.ClampGet(x,y+1).q - data.Get(x,y).q;
       real32 Qxx = data.ClampGet(x+1,y).q + data.ClampGet(x-1,y).q - 2.0*data.Get(x,y).q;
       real32 Qyy = data.ClampGet(x,y+1).q + data.ClampGet(x,y-1).q - 2.0*data.Get(x,y).q;
       
       real32 Zx = data.ClampGet(x+1,y).z - data.Get(x,y).z;
       real32 Zy = data.ClampGet(x,y+1).z - data.Get(x,y).z;
       real32 Zxx = data.ClampGet(x+1,y).z + data.ClampGet(x-1,y).z - 2.0*data.Get(x,y).z;
       real32 Zyy = data.ClampGet(x,y+1).z + data.ClampGet(x,y-1).z - 2.0*data.Get(x,y).z;
       
       real32 Ixx = irr.ClampGet(x+1,y) + irr.ClampGet(x-1,y) - 2.0*irr.Get(x,y);
       real32 Iyy = irr.ClampGet(x,y+1) + irr.ClampGet(x,y-1) - 2.0*irr.Get(x,y);


      // Calculate the reflectance function, plus differentials...
       real32 R = (-toLight[0]*data.Get(x,y).p - toLight[1]*data.Get(x,y).q + toLight[2])/
                  math::Sqrt(math::Sqr(data.Get(x,y).p) + math::Sqr(data.Get(x,y).q) + 1.0);
              R = math::Max(R,real32(0.0));
       real32 Rp = (-toLight[0]*(data.Get(x,y).p+diffDelta) - toLight[1]*data.Get(x,y).q + toLight[2])/
                   math::Sqrt(math::Sqr(data.Get(x,y).p+diffDelta) + math::Sqr(data.Get(x,y).q) + 1.0);
              Rp = math::Max(Rp,real32(0.0));
              Rp = (Rp - R)/diffDelta;
       real32 Rq = (-toLight[0]*data.Get(x,y).p - toLight[1]*(data.Get(x,y).q+diffDelta) + toLight[2])/
                   math::Sqrt(math::Sqr(data.Get(x,y).p) + math::Sqr(data.Get(x,y).q+diffDelta) + 1.0);
              Rq = math::Max(Rq,real32(0.0));
              Rq = (Rq - R)/diffDelta;
      
      // Calculate the symmetric A matrix...
       real32 A11 = 5.0*math::Sqr(Rp) + 1.25*mu;
       real32 A12 = 5.0*Rp*Rq + 0.25*mu;
       real32 A22 = 5.0*math::Sqr(Rq) + 1.25*mu;
       
      // Calculate some greek letters...
       real32 ep1 = R - irr.Get(x,y); 
       real32 ep2 = Rp*(Pxx+Pyy) + Rq*(Qxx+Qyy) - Ixx - Iyy;
       real32 aDet = A11*A22 - math::Sqr(A12);
       
      // Calculate the C terms...
       real32 C3 = -Px - Qy + Zxx + Zyy;
       real32 C1 = (ep2-ep1)*Rp - mu*(data.Get(x,y).p-Zx) - 0.25*mu*C3;
       real32 C2 = (ep2-ep1)*Rq - mu*(data.Get(x,y).q-Zy) - 0.25*mu*C3;
       
      // And, finally, calculate the deltas...
       delta.Get(x,y).p = (C1*A22 - C2*A12)/aDet;
       delta.Get(x,y).q = (C2*A11 - C1*A12)/aDet;
       delta.Get(x,y).z = (C3 + delta.Get(x,y).p + delta.Get(x,y).q)/4.0;
       
       /*if ((x==int32(data.Width()/2))&&(y==int32(data.Height()/2)))
       {
        LogDebug("Start {x,y,iter}" << LogDiv() << x << LogDiv() << y << LogDiv() << i);
        LogDebug("{Px,Pxx,Pyy}" << LogDiv() << Px << LogDiv() << Pxx << LogDiv() << Pyy);
        LogDebug("{Qy,Qxx,Qyy}" << LogDiv() << Qy << LogDiv() << Qxx << LogDiv() << Qyy);
        LogDebug("{Zx,Zy,Zxx,Zyy}" << LogDiv() << Zx << LogDiv() << Zy << LogDiv() << Zxx << LogDiv() << Zyy);
        LogDebug("{Ixx,Iyy}" << LogDiv() << Ixx << LogDiv() << Iyy);
        LogDebug("{R,Rp,Rq}" << LogDiv() << R << LogDiv() << Rp << LogDiv() << Rq);
        
        LogDebug("{A11,A12,A22}" << LogDiv() << A11 << LogDiv() << A12 << LogDiv() << A22);
        LogDebug("{ep1,ep2,aDet}" << LogDiv() << ep1 << LogDiv() << ep2 << LogDiv() << aDet);
        LogDebug("{C1,C2,C3}" << LogDiv() << C1 << LogDiv() << C2 << LogDiv() << C3);
        LogDebug("End {dp,dq,dz}" << LogDiv() << delta.Get(x,y).p << LogDiv()
                 << delta.Get(x,y).q << LogDiv() << delta.Get(x,y).z);
       }*/
     }
    }
   
   // Apply deltas, keeping track of the total amount of actual change...
    real32 sumDp = 0.0;
    real32 sumDq = 0.0;
    real32 sumDz = 0.0;
    for (nat32 y=0;y<data.Height();y++)
    {
     for (nat32 x=0;x<data.Width();x++)
     {
      if (math::IsFinite(delta.Get(x,y).z))
      {
       data.Get(x,y) += delta.Get(x,y);
       sumDp += math::Abs(delta.Get(x,y).p);
       sumDq += math::Abs(delta.Get(x,y).q);
       sumDz += math::Abs(delta.Get(x,y).z);
      }
      else sumDz += 1.0; // Make sure it doesn't finish this time around.
     }
    }
   
   // If the change from applying the deltas is small break early...
    if ((sumDp<0.001)&&(sumDq<0.001)&&(sumDz<0.001)) break;
  }
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
