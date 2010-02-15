//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/bs/luv_range.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
LuvRangeImage::LuvRangeImage()
{}

LuvRangeImage::~LuvRangeImage()
{}
   
void LuvRangeImage::Create(const svt::Field<bs::ColourLuv> & img, const svt::Field<bit> & msk, bit useHalfX, bit useHalfY, bit useCorners)
{
 // First fill it in from the centre points only...
  data.Resize(img.Size(0),img.Size(1));
  mask.Resize(img.Size(0),img.Size(1));
  for (nat32 y=0;y<data.Height();y++)
  {
   for (nat32 x=0;x<data.Width();x++)
   {
    data.Get(x,y) = img.Get(x,y);
    if (msk.Valid()) mask.Get(x,y) = msk.Get(x,y);
                else mask.Get(x,y) = true;
   }
  }
 
 // Now do the halfs and corners, as needed...
  if (useHalfX||useHalfY||useCorners)
  {
   for (nat32 y=0;y<data.Height();y++)
   {
    for (nat32 x=0;x<data.Width();x++)
    {
     if (!mask.Get(x,y)) continue;
     
     bit safeX = ((x+1)<data.Width())  && mask.Get(x+1,y);
     bit safeY = ((y+1)<data.Height()) && mask.Get(x,y+1);
     
     if (useHalfX && safeX)
     {
      bs::ColourLuv half;
      half.l = 0.5 * (img.Get(x,y).l + img.Get(x+1,y).l);
      half.u = 0.5 * (img.Get(x,y).u + img.Get(x+1,y).u);
      half.v = 0.5 * (img.Get(x,y).v + img.Get(x+1,y).v);
      
      data.Get(x  ,y) += half;
      data.Get(x+1,y) += half;
     }
     
     if (useHalfY && safeY)
     {
      bs::ColourLuv half;
      half.l = 0.5 * (img.Get(x,y).l + img.Get(x,y+1).l);
      half.u = 0.5 * (img.Get(x,y).u + img.Get(x,y+1).u);
      half.v = 0.5 * (img.Get(x,y).v + img.Get(x,y+1).v);
      
      data.Get(x,y  ) += half;
      data.Get(x,y+1) += half;
     }
     
     // You could code this to handle one entry being masked, but seems safer to
     // just drop such scenarios altogether.
     if (useCorners && safeX && safeY && mask.Get(x+1,y+1))
     {
      bs::ColourLuv quater;
      quater.l = 0.25 * (img.Get(x,y).l + img.Get(x+1,y).l + img.Get(x,y+1).l + img.Get(x+1,y+1).l);
      quater.u = 0.25 * (img.Get(x,y).u + img.Get(x+1,y).u + img.Get(x,y+1).u + img.Get(x+1,y+1).u);
      quater.v = 0.25 * (img.Get(x,y).v + img.Get(x+1,y).v + img.Get(x,y+1).v + img.Get(x+1,y+1).v);
      
      data.Get(x  ,y  ) += quater;
      data.Get(x+1,y  ) += quater;
      data.Get(x  ,y+1) += quater;
      data.Get(x+1,y+1) += quater;
     }
    }
   }
  }
}
   
void LuvRangeImage::Create(const LuvRangeImage & img, bit halfWidth, bit halfHeight)
{
 // Calculate dimensions...
  nat32 width = img.Width();
  nat32 height = img.Height();
  if (halfWidth) width = (width>>1) + (width&1);
  if (halfHeight) height = (height>>1) + (height&1);
 
 // Setup mask with all entries invalid...
  mask.Resize(width,height);
  for (nat32 y=0;y<mask.Height();y++)
  {
   for (nat32 x=0;x<mask.Width();x++) mask.Get(x,y) = false;
  }
 
 // Copy data over...
  data.Resize(width,height);
  for (nat32 y=0;y<img.Height();y++)
  {
   for (nat32 x=0;x<img.Width();x++)
   {
    if (!img.Valid(x,y)) continue;
    
    nat32 tx = x;
    nat32 ty = y;
    
    if (halfWidth) tx /= 2;
    if (halfHeight) ty /= 2;
    
    if (mask.Get(tx,ty)) data.Get(tx,ty) += img.Get(x,y);
    else
    {
     data.Get(tx,ty) = img.Get(x,y);
     mask.Get(tx,ty) = true;
    }
   }
  }
}
  
nat32 LuvRangeImage::Width() const
{
 return data.Width();
}

nat32 LuvRangeImage::Height() const
{
 return data.Height();
}

bit LuvRangeImage::Valid(nat32 x,nat32 y) const
{
 return mask.Get(x,y);
}

bit LuvRangeImage::ValidExt(int32 x,int32 y) const
{
 if (x<0) return false;
 if (y<0) return false;
 if (x>=int32(data.Width())) return false;
 if (y>=int32(data.Height())) return false;
 return mask.Get(x,y);
}

const LuvRange & LuvRangeImage::Get(nat32 x,nat32 y) const
{
 return data.Get(x,y);
}

//------------------------------------------------------------------------------
LuvRangePyramid::LuvRangePyramid()
:halfWidth(false),halfHeight(false)
{}
  
LuvRangePyramid::~LuvRangePyramid()
{}
   
void LuvRangePyramid::Create(const svt::Field<bs::ColourLuv> & img, const svt::Field<bit> & mask, bit useHalfX, bit useHalfY, bit useCorners, bit halfW, bit halfH)
{
 halfWidth = halfW;
 halfHeight = halfH;
 
 // Calculate how many levels are required...
  nat32 levels = 1;
  {
   nat32 width = img.Size(0);
   nat32 height = img.Size(1);
   while (true)
   {
    bit done = true;
    
    if (halfWidth&&(width!=1))
    {
     done = false;
     width = (width>>1) + (width&1);
    }
    
    if (halfHeight&&(height!=1))
    {
     done = false;
     height = (height>>1) + (height&1);
    }
    
    if (done) break;
         else levels += 1;
   }
  }

 // Build 'em...
  data.Size(levels);
  data[0].Create(img,mask,useHalfX,useHalfY,useCorners);
  
  for (nat32 l=1;l<data.Size();l++)
  {
   data[l].Create(data[l-1],halfWidth,halfHeight);
  }
  
 // Loging...
  LogDebug("pyramid {levels}" << LogDiv() << levels);
  for (nat32 l=0;l<data.Size();l++)
  {
   LogDebug("level {n,width,height}" << LogDiv() << l << LogDiv() << data[l].Width() << LogDiv() << data[l].Height());
  }
}
   
nat32 LuvRangePyramid::Levels() const
{
 return data.Size();
}

bit LuvRangePyramid::HalfWidth() const
{
 return halfWidth;
}

bit LuvRangePyramid::HalfHeight() const
{
 return halfHeight;
}

const LuvRangeImage & LuvRangePyramid::Level(nat32 l) const
{
 return data[l];
}

//------------------------------------------------------------------------------
LuvRangeDist::~LuvRangeDist()
{}

//------------------------------------------------------------------------------
BasicLRD::~BasicLRD()
{}
   
real32 BasicLRD::operator () (const LuvRange & lhs,const LuvRange & rhs) const
{
 return lhs ^ rhs;
}
   
cstrconst BasicLRD::TypeString() const
{
 return "eos::bs::BasicLRD";
}

//------------------------------------------------------------------------------
LuvRangeHierachy::LuvRangeHierachy(const svt::Field<bs::ColourLuv> & img,time::Progress * prog)
{
 prog->Push();

 // Basics...
   width = img.Size(0);
  height = img.Size(1);
  levels = math::Max(math::TopBit(width),math::TopBit(height));
    data = new LuvRange*[levels];


 // Create layer zero...
  prog->Report(0,levels);
  data[0] = new LuvRange[width*height];
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    LuvRange & targ = data[0][y*width+x];
    targ = img.Get(x,y);
    
    if (x!=0)
    {
     bs::ColourLuv half = img.Get(x,y);
     half.Mix(0.5,img.Get(x-1,y));
     targ += half;
    }
    
    if (x!=width-1)
    {
     bs::ColourLuv half = img.Get(x,y);
     half.Mix(0.5,img.Get(x+1,y));
     targ += half;
    }
    
    if (y!=0)
    {
     bs::ColourLuv half = img.Get(x,y);
     half.Mix(0.5,img.Get(x,y-1));
     targ += half;
    }
    
    if (y!=height-1)
    {
     bs::ColourLuv half = img.Get(x,y);
     half.Mix(0.5,img.Get(x,y+1));
     targ += half;
    }
   }
  }


 // Create the hierachy layers...
  nat32 prevWidth  = width;
  nat32 prevHeight = height;
  for (nat32 l=1;l<levels;l++)
  {
   prog->Report(l,levels);
   // Size...
    nat32 locWidth  = width>>l;  if (locWidth==0)  locWidth  = 1;
    nat32 locHeight = height>>l; if (locHeight==0) locHeight = 1;
   
   // Allocate...
    data[l] = new LuvRange[locWidth*locHeight];
   
   // Sample from previous... 
    for (nat32 y=0;y<locHeight;y++)
    {
     for (nat32 x=0;x<locWidth;x++)
     {
      LuvRange & targ = data[l][y*locWidth+x];
     
      nat32 xx = x*2;
      nat32 yy = y*2;

      bit safeX = (xx+1)<prevWidth;
      bit safeY = (yy+1)<prevHeight;
      
      targ = data[l-1][yy*prevWidth + xx];
      if (safeX) targ += data[l-1][yy*prevWidth + xx+1];
      if (safeY) targ += data[l-1][(yy+1)*prevWidth + xx];
      if (safeX&&safeY) targ += data[l-1][(yy+1)*prevWidth + xx+1];
     }
    }
   
   
   // Bookkeeping...
    prevWidth  = locWidth;
    prevHeight = prevHeight;
  }
  
 prog->Pop();
}

LuvRangeHierachy::~LuvRangeHierachy()
{
 for (nat32 l=0;l<levels;l++) delete[] data[l];
 delete[] data;
}

nat32 LuvRangeHierachy::Levels() const
{
 return levels;
}

nat32 LuvRangeHierachy::Width(nat32 level) const
{
 nat32 ret = width>>level;
 if (ret!=0) return ret;
        else return 1;
}

nat32 LuvRangeHierachy::Height(nat32 level) const
{
 nat32 ret = height>>level;
 if (ret!=0) return ret;
        else return 1;
}

const LuvRange & LuvRangeHierachy::Get(nat32 level,nat32 x,nat32 y) const
{
 nat32 locWidth = width>>level;
 if (locWidth==0) locWidth = 1;
 return data[level][y*locWidth + x];
}

//------------------------------------------------------------------------------
 };
};
