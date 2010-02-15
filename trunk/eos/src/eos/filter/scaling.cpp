//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/filter/scaling.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void ScaleExtend(const svt::Field<real32> & in,svt::Field<real32> & out,real32 scaler)
{
 real32 mult = 1.0/scaler;

 for (int32 y=0;y<int32(out.Size(1));y++)
 {
  for (int32 x=0;x<int32(out.Size(0));x++)
  {
   out.Get(x,y) = 0.0;
   real32 weight = 0.0;

   // Calculate the range to be sampled (inclusive both ends)...
    real32 minX = (real32(x)-0.5)*mult;
    real32 maxX = (real32(x)+0.5)*mult;
    real32 minY = (real32(y)-0.5)*mult;
    real32 maxY = (real32(y)+0.5)*mult;

    int32 startX = math::Clamp(int32(math::RoundDown(minX)),int32(0),int32(in.Size(0)-1));
    int32 endX   = math::Clamp(int32(math::RoundDown(maxX)),int32(0),int32(in.Size(0)-1));
    int32 startY = math::Clamp(int32(math::RoundDown(minY)),int32(0),int32(in.Size(1)-1));
    int32 endY   = math::Clamp(int32(math::RoundDown(maxY)),int32(0),int32(in.Size(1)-1));

    if (startX>=endX)
    {
     if (startY>=endY)
     {
      real32 mx = maxX - minX;
      real32 my = maxY - minY;

      weight += mx*my;
      out.Get(x,y) += mx*my*in.Get(endX,endY);
     }
     else
     {
      real32 mx = maxX - minX;
      real32 sy = real32(startY+1) - minY;
      real32 ey = maxY - real32(endY-1);

      // Sum in the end points...
       weight += mx*(sy + ey);
       out.Get(x,y) += mx*(sy*in.Get(endX,startY) + ey*in.Get(endX,endY));

      // Sum in the line...
       for (int32 v=startY+1;v<endY;v++)
       {
        weight += mx;
        out.Get(x,y) += mx*in.Get(endX,v);
       }
     }
    }
    else
    {
     if (startY>=endY)
     {
      real32 sx = real32(startX+1) - minX;
      real32 ex = maxX - real32(endX-1);
      real32 my = maxY - minY;

      // Sum in the end points...
       weight += (sx + ex)*my;
       out.Get(x,y) += (sx*in.Get(startX,endY) + ex*in.Get(endX,endY))*my;

      // Sum in the line...
       for (int32 u=startX+1;u<endX;u++)
       {
        weight += my;
        out.Get(x,y) += my*in.Get(u,endY);
       }
     }
     else
     {
      real32 sx = real32(startX+1) - minX;
      real32 ex = maxX - real32(endX-1);
      real32 sy = real32(startY+1) - minY;
      real32 ey = maxY - real32(endY-1);

      // Sum in the corners...
       weight += sx*sy + sx*ey + ex*sy + ex*ey;
       out.Get(x,y) += sx*sy*in.Get(startX,startY) +
                       sx*ey*in.Get(startX,endY) +
                       ex*sy*in.Get(endX,startY) +
                       ex*ey*in.Get(endX,endY);

      // Sum in the edges...
       for (int32 u=startX+1;u<endX;u++)
       {
        weight += sy + ey;
        out.Get(x,y) += sy*in.Get(u,startY) + ey*in.Get(u,endY);
       }

       for (int32 v=startY+1;v<endY;v++)
       {
        weight += sx + ex;
        out.Get(x,y) += sx*in.Get(startX,v) + ex*in.Get(endX,v);
       }

      // Sum in the soft juicy centre...
       for (int32 v=startY+1;v<endY;v++)
       {
        for (int32 u=startX+1;u<endX;u++)
        {
         out.Get(x,y) += in.Get(u,v);
         weight += 1.0;
        }
       }
     }
    }

   if (!math::IsZero(weight)) out.Get(x,y) /= weight;
  }
 }
}

EOS_FUNC void ScaleExtend(const svt::Field<real32> & in,const svt::Field<bit> & inMask,
                          svt::Field<real32> & out ,svt::Field<bit> & outMask,
                          real32 scaler)
{
 real32 mult = 1.0/scaler;

 for (int32 y=0;y<int32(out.Size(1));y++)
 {
  for (int32 x=0;x<int32(out.Size(0));x++)
  {
   out.Get(x,y) = 0.0;
   real32 weight = 0.0;

   // Calculate the range to be sampled (inclusive both ends)...
    real32 minX = (real32(x)-0.5)*mult;
    real32 maxX = (real32(x)+0.5)*mult;
    real32 minY = (real32(y)-0.5)*mult;
    real32 maxY = (real32(y)+0.5)*mult;

    int32 startX = math::Clamp(int32(math::RoundDown(minX)),int32(0),int32(in.Size(0)-1));
    int32 endX   = math::Clamp(int32(math::RoundDown(maxX)),int32(0),int32(in.Size(0)-1));
    int32 startY = math::Clamp(int32(math::RoundDown(minY)),int32(0),int32(in.Size(1)-1));
    int32 endY   = math::Clamp(int32(math::RoundDown(maxY)),int32(0),int32(in.Size(1)-1));

    if (startX>=endX)
    {
     if (startY>=endY)
     {
      real32 mx = maxX - minX;
      real32 my = maxY - minY;

      if (inMask.Get(endX,endY))
      {
       weight += mx*my;
       out.Get(x,y) += mx*my*in.Get(endX,endY);
      }
     }
     else
     {
      real32 mx = maxX - minX;
      real32 sy = real32(startY+1) - minY;
      real32 ey = maxY - real32(endY-1);

      // Sum in the end points...
       if (inMask.Get(endX,startY))
       {
        weight += mx*sy;
        out.Get(x,y) += mx*sy*in.Get(endX,startY);
       }

       if (inMask.Get(endX,endY))
       {
        weight += mx*ey;
        out.Get(x,y) += mx*ey*in.Get(endX,endY);
       }

      // Sum in the line...
       for (int32 v=startY+1;v<endY;v++)
       {
        if (inMask.Get(endX,v))
        {
         weight += mx;
         out.Get(x,y) += mx*in.Get(endX,v);
        }
       }
     }
    }
    else
    {
     if (startY>=endY)
     {
      real32 sx = real32(startX+1) - minX;
      real32 ex = maxX - real32(endX-1);
      real32 my = maxY - minY;

      // Sum in the end points...
       if (inMask.Get(startX,endY))
       {
        weight += sx*my;
        out.Get(x,y) += sx*in.Get(startX,endY)*my;
       }

       if (inMask.Get(endX,endY))
       {
        weight += ex*my;
        out.Get(x,y) += ex*in.Get(endX,endY)*my;
       }

      // Sum in the line...
       for (int32 u=startX+1;u<endX;u++)
       {
	if (inMask.Get(u,endY))
	{
         weight += my;
         out.Get(x,y) += my*in.Get(u,endY);
        }
       }
     }
     else
     {
      real32 sx = real32(startX+1) - minX;
      real32 ex = maxX - real32(endX-1);
      real32 sy = real32(startY+1) - minY;
      real32 ey = maxY - real32(endY-1);

      // Sum in the corners...
       if (inMask.Get(startX,startY))
       {
	weight += sx*sy;
        out.Get(x,y) += sx*sy*in.Get(startX,startY);
       }

       if (inMask.Get(startX,endY))
       {
	weight += sx*ey;
        out.Get(x,y) += sx*ey*in.Get(startX,endY);
       }

       if (inMask.Get(endX,startY))
       {
	weight += ex*sy;
        out.Get(x,y) += ex*sy*in.Get(endX,startY);
       }

       if (inMask.Get(endX,endY))
       {
	weight += ex*ey;
        out.Get(x,y) += ex*ey*in.Get(endX,endY);
       }

      // Sum in the edges...
       for (int32 u=startX+1;u<endX;u++)
       {
        if (inMask.Get(u,startY))
        {
         weight += sy;
         out.Get(x,y) += sy*in.Get(u,startY);
        }
        if (inMask.Get(u,endY))
        {
         weight += ey;
         out.Get(x,y) += ey*in.Get(u,endY);
        }
       }

       for (int32 v=startY+1;v<endY;v++)
       {
	if (inMask.Get(startX,v))
	{
         weight += sx;
         out.Get(x,y) += sx*in.Get(startX,v);
        }
        if (inMask.Get(endX,v))
	{
         weight += ex;
         out.Get(x,y) += ex*in.Get(endX,v);
        }
       }

      // Sum in the soft juicy centre...
       for (int32 v=startY+1;v<endY;v++)
       {
        for (int32 u=startX+1;u<endX;u++)
        {
         if (inMask.Get(u,v))
         {
          out.Get(x,y) += in.Get(u,v);
          weight += 1.0;
         }
        }
       }
     }
    }

   if (!math::IsZero(weight))
   {
    outMask.Get(x,y) = true;
    out.Get(x,y) /= weight;
   }
   else
   {
    outMask.Get(x,y) = false;
   }
  }
 }
}

EOS_FUNC void ScaleExtend(const svt::Field<bs::ColourRGB> & in,svt::Field<bs::ColourRGB> & out,real32 scaler)
{
 svt::Field<real32> inR;
 svt::Field<real32> inG;
 svt::Field<real32> inB;

 svt::Field<real32> outR;
 svt::Field<real32> outG;
 svt::Field<real32> outB;

 in.SubField(sizeof(real32)*0,inR);
 in.SubField(sizeof(real32)*1,inG);
 in.SubField(sizeof(real32)*2,inB);

 out.SubField(sizeof(real32)*0,outR);
 out.SubField(sizeof(real32)*1,outG);
 out.SubField(sizeof(real32)*2,outB);

 ScaleExtend(inR,outR,scaler);
 ScaleExtend(inG,outG,scaler);
 ScaleExtend(inB,outB,scaler);
}

EOS_FUNC void ScaleExtend(const svt::Field<bs::ColourRGB> & in,const svt::Field<bit> & inMask,
                          svt::Field<bs::ColourRGB> & out,svt::Field<bit> & outMask,
                          real32 scaler)
{
 svt::Field<real32> inR;
 svt::Field<real32> inG;
 svt::Field<real32> inB;

 svt::Field<real32> outR;
 svt::Field<real32> outG;
 svt::Field<real32> outB;

 in.SubField(sizeof(real32)*0,inR);
 in.SubField(sizeof(real32)*1,inG);
 in.SubField(sizeof(real32)*2,inB);

 out.SubField(sizeof(real32)*0,outR);
 out.SubField(sizeof(real32)*1,outG);
 out.SubField(sizeof(real32)*2,outB);

 ScaleExtend(inR,inMask,outR,outMask,scaler);
 ScaleExtend(inG,inMask,outG,outMask,scaler);
 ScaleExtend(inB,inMask,outB,outMask,scaler);
}

//------------------------------------------------------------------------------
 };
};
