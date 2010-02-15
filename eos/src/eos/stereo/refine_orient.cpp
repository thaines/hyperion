//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/stereo/refine_orient.h"

#include "eos/filter/kernel.h"
#include "eos/alg/fitting.h"
#include "eos/math/gaussian_mix.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
RefineOrient::RefineOrient()
:radius(2),damping(1.0),stepCost(0.5),iters(100),duds(false)
{}

RefineOrient::~RefineOrient()
{}

void RefineOrient::Set(nat32 r,real32 d,real32 s,nat32 i)
{
 radius = r;
 damping = d;
 stepCost = s;
 iters = i;
}

void RefineOrient::Set(const DSI & d,const SparsePos & s)
{
 dsi = &d;
 spos = &s;
}

void RefineOrient::Set(const svt::Field<nat32> & s)
{
 seg = s;
}

void RefineOrient::Set(const svt::Field<bs::Normal> & n)
{
 needle = n;
}

void RefineOrient::Set(const cam::CameraPair & p)
{
 pair = p;
}

void RefineOrient::Duds(bit enable)
{
 duds = enable;
}

void RefineOrient::Run(time::Progress * prog)
{
 LogBlock("eos::stereo::RefineOrient::Run","-");
 prog->Push();
 
 // If data does not allready contain a 'point-surface' extract one from the 
 // spos. We require our point surface to have w=1 for all homogenous
 // coordinates...
  prog->Report(0,3);
  if ((data.Width()==0)||(data.Height()==0))
  {
   data.Resize(spos->Width(),spos->Height());
   for (nat32 y=0;y<spos->Height();y++)
   {
    for (nat32 x=0;x<spos->Width();x++)
    {
     real32 best = math::Infinity<real32>();
     for (nat32 i=0;i<spos->Size(x,y);i++)
     {
      if (spos->Cost(x,y,i)<best) data.Get(x,y) = spos->Centre(x,y,i);
     }
    }
   }
  }



 // Create a data structure of normals - if provided use them, otherwise 
 // plane fit each segment to obtain the normals...
  prog->Report(1,3);
  ds::Array2D<bs::Normal> norm(spos->Width(),spos->Height());
  if (needle.Valid())
  {
   // We have been given normals - easy case as a copy will surfice...
    for (nat32 y=0;y<spos->Height();y++)
    {
     for (nat32 x=0;x<spos->Width();x++)
     {
      norm.Get(x,y) = needle.Get(x,y);
     }
    }
  }
  else
  {
   // We are without normals, plane fit each segment...
    // Count how many segments we have...
     nat32 segCount = 1;
     for (nat32 y=0;y<seg.Size(1);y++)
     {
      for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
     }

    // Create plane fitters for each segment...
     ds::ArrayDel<alg::LinePlaneFit> planeFit(segCount);
    
    // Fill in the data...
     for (nat32 y=0;y<seg.Size(1);y++)
     {
      for (nat32 x=0;x<seg.Size(0);x++) planeFit[seg.Get(x,y)].Add(data.Get(x,y),data.Get(x,y),1.0);
     }

    // Run the fitters...
     prog->Push();
      for (nat32 i=0;i<segCount;i++)
      {
       prog->Report(i,segCount);
       planeFit[i].Run();
      }
     prog->Pop();

    // Assign the normals...
     for (nat32 y=0;y<seg.Size(1);y++)
     {
      for (nat32 x=0;x<seg.Size(0);x++)
      {
       norm.Get(x,y) = planeFit[seg.Get(x,y)].Plane().n;
       norm.Get(x,y).Normalise();
       if (norm.Get(x,y)[2]<0.0) norm.Get(x,y) *= -1.0;
      }
     }
  }



 // Calculate the centre of the left camera and the projection matrix to take a
 // position to its right image rectified coordinate...
  math::Vect<4> leftCentre;
  {
   math::Vect<4,real64> centre;
   pair.lp.Centre(centre);
   centre.Normalise();
   if (!math::IsZero(centre[3])) centre /= centre[3];
   for (nat32 i=0;i<4;i++) leftCentre[i] = centre[i];
  }
  
  math::Mat<3,4> projRight;
  {
   math::Mat<3,3> rectRight;
   math::Mat<3,3> temp;
   
   math::Assign(rectRight,pair.unRectRight);
   math::Inverse(rectRight,temp);
   
   math::Mult(rectRight,pair.rp,projRight);
  }



 // Do the iterations, jiggling arround the point surface using the normals
 // to end up with a smooth surface which reasonably matches the data sources... 
  ds::Array2D<bs::Vertex> temp(data.Width(),data.Height());
  ds::Array2D<int32> prevDisp(data.Width(),data.Height());
  ds::Array2D<real32> weight(data.Width(),data.Height());
    
  for (nat32 y=0;y<prevDisp.Height();y++)
  {
   for (nat32 x=0;x<prevDisp.Width();x++) prevDisp.Get(x,y) = math::min_int_32;
  }
  
  prog->Report(2,3);
  prog->Push();
  for (nat32 i=0;i<iters;i++)
  {
   LogTime("eos::stereo::RefineOrient::Run - iter");
   prog->Report(i,iters);
   prog->Push();
   // Prep pointers...
    ds::Array2D<bs::Vertex> * in;
    ds::Array2D<bs::Vertex> * out;
    if ((i%2)==0)
    {
     in = &data;
     out = &temp;
    }
    else
    {
     in = &temp;
     out = &data;    
    }


   // Iterate the entire image and calculate the weight assigned to each pixel, 
   // as we don't want to have to calcalate such a thing repeatedly in the 
   // below...
    for (nat32 y=0;y<out->Height();y++)
    {
     prog->Report(y,out->Height()*2);
     for (nat32 x=0;x<out->Width();x++)
     {
      if ((duds==false)||(seg.Get(x,y)!=0))
      {
       // Project to the right image to discover the disparity...
        math::Vect<3> pp;
        math::MultVect(projRight,in->Get(x,y),pp);
        pp /= pp[2];
        int32 disp = int32(math::Round(pp[0]-real32(x)));
        
       // If the disparity doesn't match the cached disparity we need to
       // re-calculate the weight...
        if (prevDisp.Get(x,y)!=disp)
        {
         prevDisp.Get(x,y) = disp;
         
         weight.Get(x,y) = math::Infinity<real32>();
         for (nat32 j=0;j<dsi->Size(x,y);j++)
         {
          real32 cost = dsi->Cost(x,y,j) + stepCost*math::Abs(dsi->Disp(x,y,j)-real32(disp));
          weight.Get(x,y) = math::Min(weight.Get(x,y),cost);
         }
         weight.Get(x,y) = math::Exp(-weight.Get(x,y));
        }
      }
     }
    }


   // Iterate the entire image and for each pixel optimise its position...
    for (nat32 y=0;y<out->Height();y++)
    {
     prog->Report(out->Height()+y,out->Height()*2);
     for (nat32 x=0;x<out->Width();x++)
     {
      if (duds&&(seg.Get(x,y)==0)) out->Get(x,y) = in->Get(x,y);
                              else CalcPos(x,y,*in,norm,leftCentre,weight,out->Get(x,y));
     }
    }
   prog->Pop();
  }
  prog->Pop();
  
 // If the number of iterations are odd copy over temp...
  if ((iters%2)==1)
  {
   for (nat32 y=0;y<data.Height();y++)
   {
    for (nat32 x=0;x<data.Width();x++) data.Get(x,y) = temp.Get(x,y);
   }
  }

 prog->Pop();
}

void RefineOrient::PosMap(svt::Field<bs::Vertex> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = data.Get(x,y);
  }
 }
}

void RefineOrient::NeedleMap(svt::Field<bs::Normal> & out,real32 sd) const
{
 // Use central difference to fill in the output, with duplication for boundaries...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++)
   {
    nat32 sX = math::Clamp<nat32>(x,1,out.Size(0)-2);
    nat32 sY = math::Clamp<nat32>(y,1,out.Size(1)-2);
   
    bs::Vertex nx = data.Get(sX-1,sY); nx /= nx[3];
    bs::Vertex px = data.Get(sX+1,sY); px /= px[3];
    bs::Vertex ny = data.Get(sX,sY-1); ny /= ny[3];
    bs::Vertex py = data.Get(sX,sY+1); py /= py[3];
    
    math::Vect<3> dx;
    dx[0] = px[0]-nx[0];
    dx[1] = px[1]-nx[1];
    dx[2] = px[2]-nx[2];

    math::Vect<3> dy;
    dy[0] = py[0]-ny[0];
    dy[1] = py[1]-ny[1];
    dy[2] = py[2]-ny[2];
    
    math::CrossProduct(dx,dy,out.Get(x,y));
    out.Get(x,y).Normalise();
   }
  }


 // If requested smooth the needle map...
  if (!math::IsZero(sd))
  {
   // Subdivide into 3 real32 fields...
    svt::Field<real32> nx; out.SubField(sizeof(real32) * 0,nx);
    svt::Field<real32> ny; out.SubField(sizeof(real32) * 1,ny);
    svt::Field<real32> nz; out.SubField(sizeof(real32) * 2,nz);
   
   // Apply gaussian kernel to each...
    filter::KernelVect kernel(nat32(math::RoundUp(sd*2.0)));
    kernel.MakeGaussian(sd);

    kernel.ApplyRepeat(nx,nx);
    kernel.ApplyRepeat(ny,ny);
    kernel.ApplyRepeat(nz,nz);

   // Re-normalise...
    for (nat32 y=0;y<out.Size(1);y++)
    {
     for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y).Normalise();
    }
  }
}

void RefineOrient::DispMap(svt::Field<real32> & out) const
{
 // Calculate the right rectification matrix...
  math::Mat<3,3,real64> temp;
  math::Mat<3,3,real64> rectRight = pair.unRectRight;
  math::Inverse(rectRight,temp);


 // Iterate every pixel, project back the position and 
 // rectify it for the right view before taking the x difference to get the 
 // displarity...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++)
   {
    // Project back onto the right view...
     math::Vect<3,real64> urr;
     math::MultVect(pair.rp,data.Get(x,y),urr);
     
    // Rectify it...
     math::Vect<3,real64> rr;
     math::MultVect(rectRight,urr,rr);
     rr /= rr[2];
    
    // Store the difference in the x-coordinate...
     out.Get(x,y) = rr[0] - real32(x);
   }
  }
}

void RefineOrient::CalcPos(nat32 x,nat32 y,
                           const ds::Array2D<bs::Vertex> & pos,const ds::Array2D<bs::Normal> & norm,
                           const math::Vect<4> & centre,const ds::Array2D<real32> & weight,
                           bs::Vertex & out) const
{
 LogTime("eos::stereo::RefineOrient::CalcPos");
 
 // Calculate a direction vector from are starting position, this with pos will
 // define the axis which we are working along...
  math::Vect<3> linePos;
  math::Vect<3> lineDir;
  math::AssignVectRemHG(linePos,pos.Get(x,y));
  for (nat32 i=0;i<3;i++) lineDir[i] = centre[i] - linePos[i];
  lineDir.Normalise();


 // Calculate the range of the window...
  int32 bx = math::Max(int32(x) - int32(radius),int32(0));
  int32 ex = math::Min(int32(x) + int32(radius),int32(pos.Width()-1));
  int32 by = math::Max(int32(y) - int32(radius),int32(0));
  int32 ey = math::Min(int32(y) + int32(radius),int32(pos.Height()-1)); 


 // Iterate the window, project every position/normal onto the line of the pixel,
 // store the relative depths in the gaussian mixture...    
  real32 samples = 0.0;
  real32 offset = 0.0;
  nat32 s = seg.Get(x,y);
  for (int32 v=by;v<=ey;v++)
  {
   for (int32 u=bx;u<=ex;u++)
   {
    if (((u!=int32(x))||(v!=int32(y)))&&(seg.Get(u,v)==s))
    {
     // Project to the line...
      bs::Plane plane;
      plane.n = norm.Get(u,v);
      plane.d = -(pos.Get(u,v)[0] * plane.n[0] + 
                  pos.Get(u,v)[1] * plane.n[1] +
                  pos.Get(u,v)[2] * plane.n[2]);
      
      math::Vect<4> loc;
      plane.LineIntercept(centre,pos.Get(x,y),loc);
   
     // Add to the gaussian mixture, assuming its not infinitly distant...
      if (!math::IsZero(loc[3]))
      {
       loc /= loc[3];
       
       real32 os = lineDir[0]*(loc[0]-linePos[0]) + 
                   lineDir[1]*(loc[1]-linePos[1]) + 
                   lineDir[2]*(loc[2]-linePos[2]);
       
       if (math::IsFinite(os)&&(math::Abs(os)<dispWindow))
       {
        samples += weight.Get(u,v);
        offset += os*weight.Get(u,v);
       }
      }
    }
   }
  }


 // Extract the mode from the gaussian mixture,construct the final output position...
  if (!math::IsZero(samples))
  {
   offset /= samples;
   offset *= damping; // Applys damping.
   out[0] = linePos[0] + offset * lineDir[0];
   out[1] = linePos[1] + offset * lineDir[1];
   out[2] = linePos[2] + offset * lineDir[2];
   out[3] = 1.0;
  }
  else
  {
   out = pos.Get(x,y);
  }
}

//------------------------------------------------------------------------------
 };
};
