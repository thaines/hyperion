//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/bs/luv_range.h"

namespace eos
{
 namespace bs
 {
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
