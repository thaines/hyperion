//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/sfs/worthington.h"

#include "eos/filter/grad_angle.h"
#include "eos/rend/functions.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
Worthington::Worthington()
:iterCount(200),var(null<svt::Var*>())
{}

Worthington::~Worthington()
{
 delete var;
}

void Worthington::SetImage(const svt::Field<real32> & i)
{
 image = i;
}

void Worthington::SetAlbedo(const svt::Field<real32> & a)
{
 albedo = a;
}

void Worthington::SetMask(const svt::Field<bit> & m)
{
 givenMask = m;
}

void Worthington::SetLight(bs::Normal & norm)
{
 toLight = norm;
 toLight.Normalise();
}

void Worthington::SetIters(nat32 iters)
{
 iterCount = iters;
}

void Worthington::UseIniNeedle(const svt::Field<bs::ColourRGB> & iniN)
{
 iniNeedle = iniN;
}

void Worthington::Run(time::Progress * prog)
{
 prog->Push();
 prog->Report(0,iterCount);
 
 // Create the output needle map...
  var = new svt::Var(image);
   var->Add("needle",toLight);
   bit maskIni = true;
   var->Add("mask",maskIni);
  var->Commit();
  
  var->ByName("needle",needle);
  var->ByName("mask",mask);
  if (givenMask.Valid()) mask.CopyFrom(givenMask);


 // Create intermediate data structures...
  svt::Var lds(image);
   real32 blurIni(0.0);
   lds.Add("blur",blurIni);
   lds.Add("needle",toLight);
  lds.Commit();

  svt::Field<real32> blur(&lds,"blur");
  svt::Field<bs::Normal> needle2(&lds,"needle");
  
 // Make a blured version of the image, for making the gradiant from...
  filter::KernelVect kernel(2);
   kernel.MakeGaussian(1.44);
     
  kernel.Apply(image,blur);

 // Prune from the mask the borders, as they don't have a valid region...
  for (nat32 i=0;i<needle.Size(0);i++)
  {
   mask.Get(i,0) = false;
   needle.Get(i,0) = bs::Normal(0.0,0.0,1.0);
   mask.Get(i,needle.Size(1)-1) = false;    
   needle.Get(i,needle.Size(1)-1) = bs::Normal(0.0,0.0,1.0);    
  }
  
  for (nat32 i=1;i<needle.Size(1)-1;i++)
  {
   mask.Get(0,i) = false;
   needle.Get(0,i) = bs::Normal(0.0,0.0,1.0);
   mask.Get(needle.Size(0)-1,i) = false;    
   needle.Get(needle.Size(0)-1,i) = bs::Normal(0.0,0.0,1.0);    
  }


 // Initialise it...
  for (nat32 y=0;y<needle.Size(1);y++)
  {
   for (nat32 x=0;x<needle.Size(0);x++)
   {
    if (mask.Get(x,y))
    {
     if (!math::IsZero(image.Get(x,y)))
     {
      // Work out the angle of the cone from the image colour and albedo map...
       real32 normR = math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0));
       if (!math::IsFinite(normR)) normR = 1.0;
       real32 coneAng = math::InvCos(normR);
      
      // If provided use the given needle map for initialisation, otherwise the original gradient method...
       bs::Normal grad(0.0,0.0,0.0);
       if (iniNeedle.Valid())
       {
        grad[0] = 2.0 * (iniNeedle.Get(x,y).r - 0.5);
        grad[1] = 2.0 * (iniNeedle.Get(x,y).g - 0.5);
        grad[2] = iniNeedle.Get(x,y).b;
        grad.Normalise();
       }
       else
       {
        // Make a normal perpendicular to the plane in which the gradiant angle and [0,0,1]^T are in...
        // (We use a sobel filter to decide dy and dx.)
         real32 dx = (blur.Get(x+1,y-1) + 2.0*blur.Get(x+1,y) + blur.Get(x+1,y+1)) - 
                     (blur.Get(x-1,y-1) + 2.0*blur.Get(x-1,y) + blur.Get(x-1,y+1));
         real32 dy = (blur.Get(x-1,y+1) + 2.0*blur.Get(x,y+1) + blur.Get(x+1,y+1)) - 
                     (blur.Get(x-1,y-1) + 2.0*blur.Get(x,y-1) + blur.Get(x+1,y-1));
         grad[0] = dx;
         grad[1] = dy;
         if (math::IsZero(grad.Length()))
         {
          // Fallback - just choose a direction at 'random' and hope it gets fixed latter...
           grad = toLight;
           grad[0] = -grad[0];
           grad[1] = -grad[1];
         }
         else grad.Normalise();
       }
       
      // Rotate the light direction to the correct angle away from the gradiant...
       bs::Normal gradPerp;
       CrossProduct(grad,toLight,gradPerp);
       if (!math::IsZero(gradPerp.LengthSqr()))
       {
        math::Mat<3,3> rotMat;
        AngAxisToRotMat(gradPerp,coneAng,rotMat);
        MultVect(rotMat,toLight,needle.Get(x,y));
       }
     }
     else
     {
      mask.Get(x,y) = false;
      needle.Get(x,y) = bs::Normal(0.0,0.0,1.0);      
     }
    }
   }
  }


 // Iterate and smooth/correct the normals...
  for (nat32 i=1;i<iterCount;i++)
  {
   prog->Report(i,iterCount);
  
   // First pass to smooth (and re-normalise)...
    for (nat32 y=0;y<needle.Size(1);y++)
    {
     for (nat32 x=0;x<needle.Size(0);x++)
     {
      if (mask.Get(x,y))
      {
       // Calculate the next normal...
        // Calculate sigma...
         real32 sigma = 0.0;
         nat32 sigCc = 0;
          if (mask.Get(x-1,y))
          {
           bs::Normal dx = needle.Get(x,y); dx -= needle.Get(x-2,y); dx *= 0.5;
           bs::Normal dy = needle.Get(x-1,y+1); dy -= needle.Get(x-1,y-1); dy *= 0.5;
           real32 xc = 0.5*(image.Get(x,y)-image.Get(x-2,y)) - (dx*toLight);
           real32 yc = 0.5*(image.Get(x-1,y+1)-image.Get(x-1,y-1)) - (dy*toLight);
           sigma += math::Exp(-math::Sqr(xc) - math::Sqr(yc));
           ++sigCc;
          }
         
          if (mask.Get(x+1,y))
          {
           bs::Normal dx = needle.Get(x+2,y); dx -= needle.Get(x,y); dx *= 0.5;
           bs::Normal dy = needle.Get(x+1,y+1); dy -= needle.Get(x+1,y-1); dy *= 0.5;
           real32 xc = 0.5*(image.Get(x+2,y)-image.Get(x,y)) - (dx*toLight);
           real32 yc = 0.5*(image.Get(x+1,y+1)-image.Get(x+1,y-1)) - (dy*toLight);
           sigma += math::Exp(-math::Sqr(xc) - math::Sqr(yc));
           ++sigCc;
          }

          if (mask.Get(x,y-1))
          {
           bs::Normal dx = needle.Get(x+1,y-1); dx -= needle.Get(x-1,y-1); dx *= 0.5;
           bs::Normal dy = needle.Get(x,y); dy -= needle.Get(x,y-2); dy *= 0.5;
           real32 xc = 0.5*(image.Get(x+1,y-1)-image.Get(x-1,y-1)) - (dx*toLight);
           real32 yc = 0.5*(image.Get(x,y)-image.Get(x,y-2)) - (dy*toLight);
           sigma += math::Exp(-math::Sqr(xc) - math::Sqr(yc));
           ++sigCc;
          }
            
          if (mask.Get(x,y+1))
          {
           bs::Normal dx = needle.Get(x+1,y+1); dx -= needle.Get(x-1,y+1); dx *= 0.5;
           bs::Normal dy = needle.Get(x,y+2); dy -= needle.Get(x,y); dy *= 0.5;
           real32 xc = 0.5*(image.Get(x+1,y+1)-image.Get(x-1,y+1)) - (dx*toLight);
           real32 yc = 0.5*(image.Get(x,y+2)-image.Get(x,y)) - (dy*toLight);
           sigma += math::Exp(-math::Sqr(xc) - math::Sqr(yc));
           ++sigCc;
          }
         if (sigCc!=0) sigma = (sigmaZero*sigma)/real32(sigCc);
         //sigma = math::Max(sigma,real32(0.0001));
         real32 pis = math::pi/sigma;
         
        // Calculate the differentials, dx and dy, and their 2-norms...
         bs::Normal dx = needle.Get(x+1,y); dx -= needle.Get(x-1,y); dx *= 0.5;
         bs::Normal dy = needle.Get(x,y+1); dy -= needle.Get(x,y-1); dy *= 0.5;
         
         real32 dxN = dx.Length();
         real32 dyN = dy.Length();
         
        // Do the sum of neighbours in x part of the equation...
         real32 mult = math::Tanh(pis*dxN)/dxN;
         bs::Normal vec = needle.Get(x-1,y); vec += needle.Get(x+1,y);
         vec *= mult;
         needle2.Get(x,y) = vec;
        
        // Do the big dx part of the equation...
         mult = (pis*math::Sqr(math::Sech(pis*dxN)))/math::Sqr(dxN);
         mult -= math::Tanh(pis*dxN)/math::Pow<real32>(dxN,3.0);
         mult *= dx * dx;
         dx *= mult;
         needle2.Get(x,y) += dx;
         
        // Do the sum of neighbours in y part of the equation...
         mult = math::Tanh(pis*dyN)/dyN;
         vec = needle.Get(x,y-1); vec += needle.Get(x,y+1);
         vec *= mult;
         needle2.Get(x,y) += vec;
       
        // Do the big dy part of the equation...
         mult = (pis*math::Sqr(math::Sech(pis*dyN)))/math::Sqr(dyN);
         mult -= math::Tanh(pis*dyN)/math::Pow<real32>(dyN,3.0);
         mult *= dy * dy;
         dy *= mult;
         needle2.Get(x,y) += dy;
       
        
       // Check the answer is good - if not fallback to DD1 from DD6...
        real32 length = needle2.Get(x,y).Length();
        if ((!math::IsFinite(length))||(math::IsZero(length)))
        {
         needle2.Get(x,y) = bs::Normal(0.0,0.0,0.0);
         needle2.Get(x,y) += needle.Get(x-1,y);
         needle2.Get(x,y) += needle.Get(x+1,y);
         needle2.Get(x,y) += needle.Get(x,y-1);
         needle2.Get(x,y) += needle.Get(x,y+1);
         
         length = needle2.Get(x,y).Length();
         if ((!math::IsFinite(length))||(math::IsZero(length)))
         {
          needle2.Get(x,y) = needle.Get(x,y); // Fallback to no change if DD1 fails - unlikelly.
          length = needle2.Get(x,y).Length();
         }
        }

       // Renormalise, as numerical error adds up...
        needle2.Get(x,y) /= length;
      }
     }
    }


   // DD1 version of first pass...
    /*for (nat32 y=0;y<needle.Size(1);y++)
    {
     for (nat32 x=0;x<needle.Size(0);x++)
     {
      if (mask.Get(x,y))
      {
       needle2.Get(x,y) = bs::Normal(0.0,0.0,0.0);
       needle2.Get(x,y) += needle.Get(x-1,y);
       needle2.Get(x,y) += needle.Get(x+1,y);
       needle2.Get(x,y) += needle.Get(x,y-1);
       needle2.Get(x,y) += needle.Get(x,y+1);
       
       needle2.Get(x,y).Normalise();
      }
     }
    }*/


   // Second pass to enforce the cone constraint...
    for (nat32 y=0;y<needle.Size(1);y++)
    {
     for (nat32 x=0;x<needle.Size(0);x++)
     {
      if (mask.Get(x,y))
      {
       // Work out the angle of the cone from the image colour and albedo map...
        real32 normR = math::Min(image.Get(x,y)/albedo.Get(x,y),real32(1.0));
        if (!math::IsFinite(normR)) normR = 1.0;
        real32 coneAng = math::InvCos(normR);
        
       // Work out the axis of rotation and the relevent angle...
        bs::Normal axis;
        CrossProduct(needle2.Get(x,y),toLight,axis);
        if (math::IsZero(axis.LengthSqr())) axis = bs::Normal(1.0,0.0,0.0);
        axis.Normalise();
        
        real32 ang = math::InvCos(needle2.Get(x,y)*toLight) - coneAng;    
               
       // Apply the axis of rotation...
        math::Mat<3,3> rotMat;
        AngAxisToRotMat(axis,ang,rotMat);
        MultVect(rotMat,needle2.Get(x,y),needle.Get(x,y));
      }
     }     
    }
  }
  
 // Go through and re-normalise all the needles, just a bit of paranoia...
  for (nat32 y=0;y<needle.Size(1);y++)
  {
   for (nat32 x=0;x<needle.Size(0);x++)
   {
    if (mask.Get(x,y)) needle.Get(x,y).Normalise();
   }
  }

 prog->Pop();
}

void Worthington::GetNeedle(svt::Field<bs::Normal> & out) const
{
 out.CopyFrom(needle);
}

void Worthington::GetMask(svt::Field<bit> & out) const
{
 out.CopyFrom(mask);
}

void Worthington::GetNeedleColour(svt::Field<bs::ColourRGB> & out) const
{
 for (nat32 y=0;y<needle.Size(1);y++)
 {
  for (nat32 x=0;x<needle.Size(0);x++)
  {
   if (needle.Get(x,y).Z()<0.0)
   {
    out.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
   }
   else
   {
    out.Get(x,y).r = math::Clamp(0.5*(needle.Get(x,y).X()+1.0),0.0,1.0);
    out.Get(x,y).g = math::Clamp(0.5*(needle.Get(x,y).Y()+1.0),0.0,1.0);
    out.Get(x,y).b = math::Clamp(real64(math::Abs(needle.Get(x,y).Z())),0.0,1.0);
   }
  }
 }
}

svt::Var * Worthington::GetArrow() const
{
 svt::Var * nv = new svt::Var(var->GetCore());
  nv->Setup2D(needle.Size(0)*(arrowSize*2+1),needle.Size(1)*(arrowSize*2+1));
  bs::ColourRGB rgbIni(1.0,1.0,1.0);
  nv->Add("rgb",rgbIni);
 nv->Commit();
 
 svt::Field<bs::ColourRGB> img(nv,"rgb");

 bs::ColourRGB dc(0.0,0.0,0.0);
 
 for (nat32 y=0;y<needle.Size(1);y++)
 {
  nat32 baseY = y*(arrowSize*2+1)+arrowSize; 
  for (nat32 x=0;x<needle.Size(0);x++)
  {
   nat32 baseX = x*(arrowSize*2+1)+arrowSize;
   rend::Arrow(img,
               bs::Pnt(baseX,baseY),
               bs::Pnt(int32(baseX+needle.Get(x,y)[0]*arrowSize),int32(baseY+needle.Get(x,y)[1]*arrowSize)),
               dc);
  }
 }
 
 return nv;
}

cstrconst Worthington::TypeString() const
{
 return "eos::sfs::Worthington";
}

//------------------------------------------------------------------------------
 };
};
