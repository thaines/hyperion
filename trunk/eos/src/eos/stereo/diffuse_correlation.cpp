//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines
#include "eos/stereo/diffuse_correlation.h"

#include "eos/ds/sort_lists.h"
#include "eos/ds/priority_queues.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
DiffusionWeight::DiffusionWeight()
{}
  
DiffusionWeight::~DiffusionWeight()
{}

void DiffusionWeight::Create(const bs::LuvRangeImage & img, const bs::LuvRangeDist & dist, real32 distMult, time::Progress * prog)
{
 prog->Push();
 
 data.Resize(img.Width(),img.Height());
 for (nat32 y=0;y<img.Height();y++)
 {
  prog->Report(y,img.Height());
  for (nat32 x=0;x<img.Width();x++)
  {
   if (img.Valid(x,y))
   {
    Weight & weight = data.Get(x,y);

    // +ve x...
     if ((x+1<img.Width())&&(img.Valid(x+1,y))) weight.dir[0] = dist(img.Get(x,y),img.Get(x+1,y));
                                           else weight.dir[0] = math::Infinity<real32>();

    // +ve y...
     if ((y+1<img.Height())&&(img.Valid(x,y+1))) weight.dir[1] = dist(img.Get(x,y),img.Get(x,y+1));
                                            else weight.dir[1] = math::Infinity<real32>();

    // -ve x...
     if ((x>0)&&(img.Valid(x-1,y))) weight.dir[2] = dist(img.Get(x,y),img.Get(x-1,y));
                               else weight.dir[2] = math::Infinity<real32>();

    // -ve y...
     if ((y>0)&&(img.Valid(x,y-1))) weight.dir[3] = dist(img.Get(x,y),img.Get(x,y-1));
                               else weight.dir[3] = math::Infinity<real32>();

    // Convert and normalise...
     real32 low = math::Infinity<real32>();
     for (nat32 d=0;d<4;d++) low = math::Min(low,weight.dir[d]);
     
     real32 weightSum = 0.0;
     for (nat32 d=0;d<4;d++)
     {
      if (math::IsFinite(weight.dir[d]))
      {
       weight.dir[d] = -math::Exp(distMult * (weight.dir[d]-low));
       weightSum += weight.dir[d];
      }
      else weight.dir[d] = 0.0;
     }
     
     if (!math::IsZero(weightSum))
     {
      for (nat32 d=0;d<4;d++) weight.dir[d] /= weightSum;
     }
   }
   else
   {
    for (nat32 d=0;d<4;d++) data.Get(x,y).dir[d] = 0.0;
   }
  }
 }
 
 prog->Pop();
}

//------------------------------------------------------------------------------
RangeDiffusionSlice::RangeDiffusionSlice()
:steps(0)
{}

RangeDiffusionSlice::~RangeDiffusionSlice()
{}

void RangeDiffusionSlice::Create(nat32 yy, nat32 stps, const bs::LuvRangeImage & img, const DiffusionWeight & dw, time::Progress * prog)
{
 prog->Push();
 
 // First we need to calculate the offset array, and how large we need the
 // result storage to be...
  prog->Report(0,img.Width()+1);
  y = yy;
  steps = stps;
  offset.Resize(steps*2+1,steps*2+1);
  nat32 valueCount = 0;
  for (int32 v=0;v<int32(offset.Height());v++)
  {
   for (int32 u=0;u<int32(offset.Width());u++)
   {
    if (math::Abs(u-int32(steps)) + math::Abs(v-int32(steps)) <= int32(steps))
    {
     offset.Get(u,v) = valueCount;
     valueCount += 1;
    }
   }
  }
 
 // We need some tempory arrays for doing the diffusion in...
  ds::Array2D<real32> bufA(offset.Width(),offset.Height());
  ds::Array2D<real32> bufB(offset.Width(),offset.Height());

 // Helpful little arrays...
  int32 du[4] = {1,0,-1, 0};
  int32 dv[4] = {0,1, 0,-1};


 // Now iterate the pixels and calculate and store the diffusion weights for
 // each...
  if ((data.Width()!=img.Width())||(valueCount!=data.Height()))
  {
   data.Resize(img.Width(),valueCount);
  }
  
  for (nat32 x=0;x<img.Width();x++)
  {
   prog->Report(x+1,img.Width()+1);
   
   if (!img.Valid(x,y))
   {
    for (nat32 v=0;v<valueCount;v++)  data.Get(x,v) = 0.0;
    continue;
   }
   
   // Do the diffusion - we have to bounce between two buffers; when zeroing
   // the old buffer only clear the range to be used next time...
    ds::Array2D<real32> * from = &bufA;
    ds::Array2D<real32> * to = &bufB;
    
    from->Get(steps,steps) = 1.0;
    
    for (nat32 s=0;s<steps;s++)
    {
     // Zero out relevant range of to buffer...
      for (int32 v=-int32(s)-1;v<=int32(s)+1;v++)
      {
       for (int32 u=-int32(s)-1;u<=int32(s)+1;u++) to->Get(u+steps,v+steps) = 0.0;
      }
      
     // Do diffusion step...
      for (int32 v=-int32(s);v<=int32(s);v++)
      {
       for (int32 u=-int32(s);u<=int32(s);u++)
       {
        int32 ax = int32(x) + u;
        int32 ay = int32(y) + v;
        
        if (((math::Abs(v)+math::Abs(u))<=int32(s))&&
            (ax>=0)&&(ay>=0)&&(ax<int32(img.Width()))&&(ay<int32(img.Height())))
        {
         real32 val = from->Get(u+steps,v+steps);
         for (nat32 d=0;d<4;d++)
         {
          to->Get(u+steps+du[d],v+steps+dv[d]) += val * dw.Get(ax,ay,d);
         }
        }
       }
      }
     
     // Swap buffers...
      math::Swap(from,to);
    }


   // Copy result into storage - result is in from...
    for (nat32 v=0;v<from->Height();v++)
    {
     for (nat32 u=0;u<from->Width();u++)
     {
      int32 ru = int32(u) - int32(steps);
      int32 rv = int32(v) - int32(steps);
      if (math::Abs(ru) + math::Abs(rv) <= int32(steps))
      {
       data.Get(x,offset.Get(u,v)) = from->Get(u,v);
      }
     }
    }
    
   // Normalise - it should be already, but numerical error will creep in...
    real32 sum = 0.0;
    for (nat32 v=0;v<data.Height();v++) sum += data.Get(x,v);
    
    if (sum>0.5) // sum could be zero in the case of a pixel surrounded by masked pixels.
    {
     for (nat32 v=0;v<data.Height();v++) data.Get(x,v) /= sum;
    }
  }

 
 prog->Pop();
}

nat32 RangeDiffusionSlice::Width() const
{
 return data.Width();
}

nat32 RangeDiffusionSlice::Steps() const
{
 return steps;
}

real32 RangeDiffusionSlice::Get(nat32 x,int32 u,int32 v) const
{
 nat32 man = math::Abs(u) + math::Abs(v);
 if (man>steps) return 0.0;
 
 nat32 os = offset.Get(u+int32(steps),v+int32(steps));
 return data.Get(x,os);
}

//------------------------------------------------------------------------------
DiffuseCorrelation::DiffuseCorrelation()
:dist(null<bs::LuvRangeDist*>()),img1(null<bs::LuvRangeImage*>()),dif1(null<RangeDiffusionSlice*>()),img2(null<bs::LuvRangeImage*>()),dif2(null<RangeDiffusionSlice*>())
{}

DiffuseCorrelation::~DiffuseCorrelation()
{}

void DiffuseCorrelation::Setup(const bs::LuvRangeDist & d, real32 dc, const bs::LuvRangeImage & i1, const RangeDiffusionSlice & f1, const bs::LuvRangeImage & i2, const RangeDiffusionSlice & f2)
{
 log::Assert(f1.Steps()==f2.Steps());

 dist = &d;
 distCap = dc;
 
 img1 = &i1;
 dif1 = &f1;
 img2 = &i2;
 dif2 = &f2;
}

nat32 DiffuseCorrelation::Width1() const
{
 return img1->Width();
}

nat32 DiffuseCorrelation::Width2() const
{
 return img2->Width();
}

real32 DiffuseCorrelation::Cost(nat32 xx1,nat32 xx2) const
{
 real32 ret = 0.0;
 
 int32 steps = dif1->Steps();
 for (int32 v=-steps;v<=steps;v++)
 {
  for (int32 u=-steps;u<=steps;u++)
  {
   int32 x1 = int32(xx1) + u;
   int32 y1 = int32(dif1->Y()) + v;
   int32 x2 = int32(xx2) + u;
   int32 y2 = int32(dif2->Y()) + v;
   
   real32 weight = dif1->Get(xx1,u,v) + dif2->Get(xx2,u,v);
   
   if (img1->ValidExt(x1,y1)&&img2->ValidExt(x2,y2))
   {
    ret += weight * math::Min((*dist)(img1->Get(x1,y1),img2->Get(x2,y2)),distCap);
   }
   else
   {
    ret += weight * distCap;
   }
  }
 }

 return ret/2.0;
}

real32 DiffuseCorrelation::DistanceCap() const
{
 return distCap;
}

//------------------------------------------------------------------------------
DiffusionCorrelationImage::DiffusionCorrelationImage()
:distMult(1.0),minimaLimit(8),baseDistCap(1.0),distCapMult(2.0),distCapThreshold(0.5),range(2),steps(5)
{}

DiffusionCorrelationImage::~DiffusionCorrelationImage()
{}

void DiffusionCorrelationImage::Set(const bs::LuvRangeDist & d, real32 dm, const bs::LuvRangePyramid & l, const bs::LuvRangePyramid & r)
{
 dist = &d;
 distMult = dm;
 left = &l;
 right = &r;
}

void DiffusionCorrelationImage::Set(nat32 ml, real32 bdc, real32 dcm, real32 dct, nat32 r, nat32 s)
{
 minimaLimit = ml;
 baseDistCap = bdc;
 distCapMult = dcm;
 distCapThreshold = dct;
 range = r;
 steps = s;
}

void DiffusionCorrelationImage::Run(time::Progress * prog)
{
 prog->Push();
 
 // Find out how many levels we are going to do, construct the data structure to
 // store our state. Also make the correlation object ready for use.
  nat32 levels = math::Min(left->Levels(),right->Levels());
  nat32 step = 0;
  nat32 steps = 4 + levels;
  
  ds::ArrayDel< ds::SortList<Match> > matches(levels);
  
  DiffusionWeight leftDiff;
  DiffusionWeight rightDiff;
  RangeDiffusionSlice leftSliceLow;
  RangeDiffusionSlice leftSliceHigh;
  RangeDiffusionSlice rightSliceLow;
  RangeDiffusionSlice rightSliceHigh;
  DiffuseCorrelation dcLow;
  DiffuseCorrelation dcHigh;
  
  ds::Array<real32> distCap(levels);
  distCap[0] = baseDistCap;
  for (nat32 l=1;l<levels;l++) distCap[l] = distCap[l-1] * distCapMult;


 // Create result for highest level - its low enough resolution that we can brute force...
  prog->Report(step++,steps);
  prog->Push();
  prog->Report(0,left->Level(0).Height()+1);
  
  nat32 l = levels-1;  
  leftDiff.Create(left->Level(l),*dist,distMult);
  rightDiff.Create(right->Level(l),*dist,distMult);
  
  for (nat32 y=0;y<left->Level(l).Height();y++)
  {
   prog->Report(y+1,left->Level(l).Height()+1);
   leftSliceLow.Create(y,steps,left->Level(l),leftDiff);
   rightSliceLow.Create(y,steps,right->Level(l),rightDiff);
   dcLow.Setup(*dist,distCap[l],left->Level(l),leftSliceLow,right->Level(l),rightSliceLow);
   
   for (nat32 xLeft=0;xLeft<left->Level(l).Width();xLeft++)
   {
    for (nat32 xRight=0;xRight<right->Level(l).Width();xRight++)
    {
     Match m;
     m.y = y;
     m.xLeft = xLeft;
     m.xRight = xRight;
     m.score = dcLow.Cost(xLeft,xRight);
     
     matches[l].Add(m);
    }
   }
  }
  prog->Pop();


 // Now iterate down to the lower levels, only considering pairings the higher
 // levels consider to be good enough...
  while(true)
  {
   prog->Report(step++,steps);
   // Move to the level we need to proccess...
    l -= 1;
    nat32 step2 = 0,steps2 = matches[l+1].Size()+1;
    prog->Push();
    prog->Report(step2++,steps2);
    
   // Get the images, setup the diffusion weights...
    const bs::LuvRangeImage & leftImg = left->Level(l);
    const bs::LuvRangeImage & rightImg = right->Level(l);
    
    leftDiff.Create(leftImg,*dist,distMult);
    rightDiff.Create(rightImg,*dist,distMult);
    
   // Iterate every match in the above layer...
    int32 prevY = -1; // Dummy value, so we always reset for the first entry.
    if (matches[l+1].Size()==0) break; // So we don't crash if the parameters are too fussy.
    ds::SortList<Match>::Cursor targ = matches[l+1].FrontPtr();
    while (!targ.Bad())
    {
     prog->Report(step2++,steps2);
     
     // Get the match...
      Match & m = *targ;
       
     // Only consider this match if it good enough...
      if (m.score<(distCap[l+1]*distCapThreshold))
      {
       // If the match has a different y coordinate we need to recalculate the
       // slices and update the correlation calculation objects...
        if (m.y!=prevY)
        {
         prevY = m.y;
         
         if (left->HalfHeight())
         {
          leftSliceLow.Create(m.y*2,steps,leftImg,leftDiff);
          rightSliceLow.Create(m.y*2,steps,rightImg,rightDiff);
          dcLow.Setup(*dist,distCap[l],leftImg,leftSliceLow,rightImg,rightSliceLow);

          leftSliceHigh.Create(m.y*2+1,steps,leftImg,leftDiff);
          rightSliceHigh.Create(m.y*2+1,steps,rightImg,rightDiff);
          dcHigh.Setup(*dist,distCap[l],leftImg,leftSliceHigh,rightImg,rightSliceHigh);
         }
         else
         {
          leftSliceLow.Create(m.y,steps,leftImg,leftDiff);
          rightSliceLow.Create(m.y,steps,rightImg,rightDiff);
          dcLow.Setup(*dist,distCap[l],leftImg,leftSliceLow,rightImg,rightSliceLow);
         }
        }

       // Iterate the region around the match in the current level, calculating
       // correlation values where they currently don't exist...
        int32 lowLeftX = math::Clamp<int32>(m.xLeft*2-int32(range),0,leftImg.Width()-1);
        int32 highLeftX = math::Clamp<int32>(m.xLeft*2+1+int32(range),0,leftImg.Width()-1);
        int32 lowRightX = math::Clamp<int32>(m.xRight*2-int32(range),0,rightImg.Width()-1);
        int32 highRightX = math::Clamp<int32>(m.xRight*2+1+int32(range),0,rightImg.Width()-1);
       
        for (int32 xLeft=lowLeftX;xLeft<=highLeftX;xLeft++)
        {
         for (int32 xRight=lowRightX;xRight<=highRightX;xRight++)
         {
          if (left->HalfHeight())
          {
           Match mNew;
           mNew.y = m.y*2;
           mNew.xLeft = xLeft;
           mNew.xRight = xRight;
           mNew.score = dcLow.Cost(xLeft,xRight);
     
           matches[l].Add(mNew);
           
           mNew.y += 1;
           mNew.score = dcHigh.Cost(xLeft,xRight);
           
           matches[l].Add(mNew);
          }
          else
          {
           Match mNew;
           mNew.y = m.y;
           mNew.xLeft = xLeft;
           mNew.xRight = xRight;
           mNew.score = dcLow.Cost(xLeft,xRight);
     
           matches[l].Add(mNew);
          }
         }
        }
      }
 
     // To the next...   
      ++targ;
    }
    prog->Pop();
    
   // Break if we have just done the full resolution level...
    if (l==0) break;
  }
 
 
 // At this point we have the correlation scores at the final resolution - copy 
 // them into our final data structure...
  // First pass to store the maximas we are interested in. We store them using
  // the a sort list of the Match data structure, even though it has stuff we
  // don't actually need...
  // (Offset into minima array for each pixel by y*width+x)
   prog->Report(step++,steps);
   ds::ArrayDel<ds::PriorityQueue<Disp> > minimaLeft(left->Level(0).Width() * left->Level(0).Height());
   ds::ArrayDel<ds::PriorityQueue<Disp> > minimaRight(right->Level(0).Width() * right->Level(0).Height());

   prog->Push();
   if (matches[0].Size()!=0)
   {
    ds::SortList<Match>::Cursor targ = matches[0].FrontPtr();
    nat32 step2 = 0, steps2 = matches[0].Size();
    while (!targ.Bad())
    {
     prog->Report(step2++,steps2);
     // Get the match...
      Match & m = *targ;
     
     // Determine if its a minima...
      real32 minScore = m.score;
      bit ok = m.score < (baseDistCap*distCapThreshold);
      
      // Check negative for left image...
       if (ok) 
       {
        for (nat32 i=0;i<range;i++)
        {
         m.xLeft -= 1;
         Match * p = matches[0].Get(m);
         if (p==null<Match*>()) ok = false;
                           else ok = ok && (p->score > minScore);
        }
        m.xLeft += range;
       }
 
      // Check positive for left image...
       if (ok) 
       {
        for (nat32 i=0;i<range;i++)
        {
         m.xLeft += 1;
         Match * p = matches[0].Get(m);
         if (p==null<Match*>()) ok = false;
                           else ok = ok && (p->score > minScore);
        }
        m.xLeft -= range;
       }
       
      // Check negative for right image...
       if (ok) 
       {
        for (nat32 i=0;i<range;i++)
        {
         m.xRight -= 1;
         Match * p = matches[0].Get(m);
         if (p==null<Match*>()) ok = false;
                           else ok = ok && (p->score > minScore);
        }
        m.xRight += range;
       }
 
      // Check positive for right image...
       if (ok) 
       {
        for (nat32 i=0;i<range;i++)
        {
         m.xRight += 1;
         Match * p = matches[0].Get(m);
         if (p==null<Match*>()) ok = false;
                           else ok = ok && (p->score > minScore);
        }
        m.xRight -= range;
       }
       
      // If all tests have been passed store it...
       if (ok)
       {
        Disp l;
        l.d = m.xRight - m.xLeft;
        l.score = m.score;
        minimaLeft[m.y*left->Level(0).Width() + m.xLeft].Add(l);

        Disp r;
        r.d = m.xLeft - m.xRight;
        r.score = m.score;
        minimaRight[m.y*right->Level(0).Width() + m.xRight].Add(r);
       }


     // To next...
      ++targ;
    }
   }
   prog->Pop();


  // Now prepare the offset data structures, and set the size of the others...
   prog->Report(step++,steps);
   offsetLeft.Size(left->Level(0).Width()*left->Level(0).Height() + 1);
   offsetRight.Size(right->Level(0).Width()*right->Level(0).Height() + 1);
   
   offsetLeft[0] = 0;
   for (nat32 i=1;i<offsetLeft.Size();i++)
   {
    offsetLeft[i] = math::Min(minimaLeft[i-1].Size(),minimaLimit) + offsetLeft[i-1];
   }
   
   offsetRight[0] = 0;
   for (nat32 i=1;i<offsetRight.Size();i++)
   {
    offsetRight[i] = math::Min(minimaRight[i-1].Size(),minimaLimit) + offsetRight[i-1];
   }


   dispLeft.Size(offsetLeft[offsetLeft.Size()-1]);
   dispRight.Size(offsetRight[offsetRight.Size()-1]);
   
   scoreLeft.Size(dispLeft.Size() * (range*2+1));
   scoreRight.Size(dispRight.Size() * (range*2+1));


  // For each pixel output its minima into the data structure, using the sorting
  // obtained from the priority queues...
   // Left...
    prog->Report(step++,steps);
    prog->Push();
    for (nat32 y=0;y<left->Level(0).Height();y++)
    {
     prog->Report(y,left->Level(0).Height());
     for (nat32 x=0;x<left->Level(0).Width();x++)
     {
      nat32 c = y*left->Level(0).Width() + x;
      for (nat32 i=offsetLeft[c];i<offsetLeft[c+1];i++)
      {
       Disp d = minimaLeft[c].Peek();
       minimaLeft[c].Rem();
       
       dispLeft[i] = d.d;
       
       for (int32 r=-int32(range);r<=int32(range);r++)
       {
        real32 val = baseDistCap;
        
        Match dm;
        dm.y = y;
        dm.xLeft = x;
        dm.xRight = x + d.d + r;
        
        Match * m = matches[0].Get(dm);
        if (m) val = m->score;
       
        scoreLeft[i*(range*2+1) + range + r] = val;
       }
      }
     }
    }
    prog->Pop();
   
   // Right...
    prog->Report(step++,steps);
    prog->Push();
    for (nat32 y=0;y<right->Level(0).Height();y++)
    {
     prog->Report(y,right->Level(0).Height());
     for (nat32 x=0;x<right->Level(0).Width();x++)
     {
      nat32 c = y*right->Level(0).Width() + x;
      for (nat32 i=offsetRight[c];i<offsetRight[c+1];i++)
      {
       Disp d = minimaRight[c].Peek();
       minimaRight[c].Rem();
       
       dispRight[i] = d.d;
       
       for (int32 r=-int32(range);r<=int32(range);r++)
       {
        real32 val = baseDistCap;
        
        Match dm;
        dm.y = y;
        dm.xLeft = x + d.d + r;
        dm.xRight = x;
        
        Match * m = matches[0].Get(dm);
        if (m) val = m->score;
       
        scoreRight[i*(range*2+1) + range + r] = val;
       }
      }
     }
    }
    prog->Pop();

 
 prog->Pop();
}

int32 DiffusionCorrelationImage::Range() const
{
 return range;
}

nat32 DiffusionCorrelationImage::CountLeft(nat32 x,nat32 y) const
{
 nat32 index = y*left->Level(0).Width() + x;
 return offsetLeft[index+1] - offsetLeft[index];
}

int32 DiffusionCorrelationImage::DisparityLeft(nat32 x,nat32 y,nat32 i) const
{
 nat32 index = y*left->Level(0).Width() + x;
 nat32 offset = offsetLeft[index];
 return dispLeft[offset+i];
}

real32 DiffusionCorrelationImage::ScoreLeft(nat32 x,nat32 y,nat32 i,int32 offset) const
{
 nat32 index = y*left->Level(0).Width() + x;
 int32 os = (offsetLeft[index] + i) * (range*2+1) + range;
 return scoreLeft[os + offset];
}

nat32 DiffusionCorrelationImage::CountRight(nat32 x,nat32 y) const
{
 nat32 index = y*right->Level(0).Width() + x;
 return offsetRight[index+1] - offsetRight[index];
}

int32 DiffusionCorrelationImage::DisparityRight(nat32 x,nat32 y,nat32 i) const
{
 nat32 index = y*right->Level(0).Width() + x;
 nat32 offset = offsetRight[index];
 return dispRight[offset+i];
}

real32 DiffusionCorrelationImage::ScoreRight(nat32 x,nat32 y,nat32 i,int32 offset) const
{
 nat32 index = y*right->Level(0).Width() + x;
 int32 os = (offsetRight[index] + i) * (range*2+1) + range;
 return scoreRight[os + offset];
}

//------------------------------------------------------------------------------
DiffCorrStereo::DiffCorrStereo()
:useHalfX(true),useHalfY(true),useCorners(true),halfHeight(true),
distMult(0.1),minimaLimit(8),baseDistCap(4.0),distCapMult(2.0),distCapThreshold(0.5),dispRange(2),diffSteps(5),
doLR(true),distCapDifference(0.25),distSdMult(0.1)
{}

DiffCorrStereo::~DiffCorrStereo()
{}

void DiffCorrStereo::SetImages(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r)
{
 left = l;
 right = r;
}

void DiffCorrStereo::SetMasks(const svt::Field<bit> & lm,const svt::Field<bit> & rm)
{
 leftMask = lm;
 rightMask = rm;
}

void DiffCorrStereo::SetPyramid(bit ux, bit uy, bit uc, bit hh)
{
 useHalfX = ux;
 useHalfY = uy;
 useCorners = uc;
 halfHeight = hh;
}

void DiffCorrStereo::SetDiff(real32 dm,nat32 ds)
{
 distMult = dm;
 diffSteps = ds;
}

void DiffCorrStereo::SetCorr(nat32 ml,real32 bdc,real32 dcm,real32 dct,nat32 dr)
{
 minimaLimit = ml;
 baseDistCap = bdc;
 distCapMult = dcm;
 distCapThreshold = dct;
 dispRange = dr;
}

void DiffCorrStereo::SetRefine(bit lr,real32 dcd,real32 dsm)
{
 doLR = lr;
 distCapDifference = dcd;
 distSdMult = dsm;
}

void DiffCorrStereo::Run(time::Progress * prog)
{
 prog->Push();
 
 // First we need luv range pyramids for both images...
  prog->Report(0,3);
  bs::LuvRangePyramid leftP;
  leftP.Create(left,leftMask,useHalfX,useHalfY,useCorners,true,halfHeight);

  bs::LuvRangePyramid rightP;
  rightP.Create(right,rightMask,useHalfX,useHalfY,useCorners,true,halfHeight);
  

 // Now do the real work - construct the maxima sets...
  prog->Report(1,3);
  bs::BasicLRD dist; // Latter on the ability to choose this will probably need adding.
  DiffusionCorrelationImage dci;
  
  dci.Set(dist,distMult,leftP,rightP);
  dci.Set(minimaLimit,baseDistCap,distCapMult,distCapThreshold,dispRange,diffSteps);

  dci.Run(prog);


 // Finally, do the work this class is meant to do - for each pixel calculate a
 // refined disparity value based on the information, when confidence is high
 // enough...
  prog->Report(2,3);
  
  prog->Push();
  disp.Resize(left.Size(0),left.Size(1));
  sd.Resize(left.Size(0),left.Size(1));
  for (nat32 y=0;y<disp.Height();y++)
  {
   prog->Report(y,disp.Height());
   for (nat32 x=0;x<disp.Width();x++)
   {
    // Decide if the match is good enough - if not indicate it is masked with a
    // negative standard deviation, otherwise refine the disparity and assign
    // a standard deviation...
     // Set it to masked values, so we can just continue on it failing a test...
      disp.Get(x,y) = 0.0;
      sd.Get(x,y) = -1.0;
      
     // Check it actually has a minima to consider...
      if (dci.CountLeft(x,y)==0) continue;
      nat32 rx = int32(x) + dci.DisparityLeft(x,y,0);
      nat32 ry = y;
     
     // Left-right consistancy check...
      if (doLR)
      {
       if (dci.CountRight(rx,ry)==0) continue;
       if (int32(rx)+dci.DisparityRight(rx,ry,0)!=int32(x)) continue;
      }
     
     // Sufficiently better than alternative matches in *both* images...
      real32 thresh = baseDistCap * distCapDifference;
      
      if (dci.CountLeft(x,y)>1)
      {
       if (dci.ScoreLeft(x,y,0,0)-dci.ScoreLeft(x,y,1,0) < thresh) continue;
      }
     
      if (dci.CountRight(rx,ry)>1)
      {
       if (dci.ScoreRight(rx,ry,0,0)-dci.ScoreRight(rx,ry,1,0) < thresh) continue;
      }

     
     // Ok - its passed - next step is to refine the disparity value via
     // polynomial fitting, we take an average of the offset values from both
     // images to maintain symmetry. The fitting itself is done by matching area
     // under the curve, as our correlations are for regions...
      real32 p = 0.5*(dci.ScoreLeft(x,y,0,-1) + dci.ScoreRight(rx,ry,0,-1));
      real32 q = 0.5*(dci.ScoreLeft(x,y,0, 0) + dci.ScoreRight(rx,ry,0, 0));
      real32 r = 0.5*(dci.ScoreLeft(x,y,0, 1) + dci.ScoreRight(rx,ry,0, 1));
      
      real32 dOS = (p-r)/(2.0*(p+r) - 4.0*q);
      disp.Get(x,y) = real32(dci.DisparityLeft(x,y,0)) + dOS;
     
     // Finally, calculate a standard deviation, based on nearby matches...
      sd.Get(x,y) = 1.0; // Code me! ********************************************
   }
  }
  prog->Pop();
 
 prog->Pop();
}

nat32 DiffCorrStereo::Width() const
{
 return disp.Width();
}
   
nat32 DiffCorrStereo::Height() const
{
 return disp.Height();
}
   
nat32 DiffCorrStereo::Size(nat32 x, nat32 y) const
{
 if (sd.Get(x,y)>0.0) return 1;
                 else return 0;
}
   
real32 DiffCorrStereo::Disp(nat32 x, nat32 y, nat32 i) const
{
 return disp.Get(x,y);
}
   
real32 DiffCorrStereo::Cost(nat32 x, nat32 y, nat32 i) const
{
 return 0.0;
}
   
real32 DiffCorrStereo::Prob(nat32 x, nat32 y, nat32 i) const
{
 return 1.0;
}
  
real32 DiffCorrStereo::DispWidth(nat32 x, nat32 y, nat32 i) const
{
 return 0.5;
}

void DiffCorrStereo::GetDisp(svt::Field<real32> & d) const
{
 for (nat32 y=0;y<disp.Height();y++)
 {
  for (nat32 x=0;x<disp.Width();x++)
  {
   d.Get(x,y) = disp.Get(x,y);
  }
 }
}

void DiffCorrStereo::GetSd(svt::Field<real32> & s) const
{
 for (nat32 y=0;y<sd.Height();y++)
 {
  for (nat32 x=0;x<sd.Width();x++) s.Get(x,y) = sd.Get(x,y);
 }
}

void DiffCorrStereo::GetMask(svt::Field<bit> & m) const
{
 for (nat32 y=0;y<disp.Height();y++)
 {
  for (nat32 x=0;x<disp.Width();x++)
  {
   m.Get(x,y) = sd.Get(x,y)>0.0;
  }
 }
}

cstrconst DiffCorrStereo::TypeString() const
{
 return "eos::stereo::DiffCorrStereo";
}

//------------------------------------------------------------------------------
DiffCorrRefine::DiffCorrRefine()
:useHalfX(true),useHalfY(true),useCorners(true),
distMult(0.1),diffSteps(5),cap(4.0),prune(0.1)
{}

DiffCorrRefine::~DiffCorrRefine()
{}

void DiffCorrRefine::SetImages(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r)
{
 left = l;
 right = r;
}

void DiffCorrRefine::SetMasks(const svt::Field<bit> & l,const svt::Field<bit> & r)
{
 leftMask = l;
 rightMask = r;
}

void DiffCorrRefine::SetDisparity(const svt::Field<real32> & d)
{
 disp = d;
}

void DiffCorrRefine::SetFlags(bit x, bit y, bit c)
{
 useHalfX = x;
 useHalfY = y;
 useCorners = c;
}

void DiffCorrRefine::SetDiff(real32 dm,nat32 ds)
{
 distMult = dm;
 diffSteps = ds;
}

void DiffCorrRefine::SetDist(real32 c,real32 p)
{
 cap = c;
 prune = p;
}

void DiffCorrRefine::Run(time::Progress * prog)
{
 prog->Push();
 
 // Distance object to use...
  bs::BasicLRD dist;
  
 // Generate range images from the inputs...
  prog->Report(0,disp.Size(1)+2);
  bs::LuvRangeImage l;
  l.Create(left,leftMask,useHalfX,useHalfY,useCorners);
  
  DiffusionWeight lw;
  lw.Create(l,dist,distMult,prog);


  prog->Report(1,disp.Size(1)+2);
  bs::LuvRangeImage r;
  r.Create(right,rightMask,useHalfX,useHalfY,useCorners);
  
  DiffusionWeight rw;
  rw.Create(r,dist,distMult,prog);
  
  
 // Iterate the disparity map and refine...
  out.Resize(disp.Size(0),disp.Size(1));

  RangeDiffusionSlice ls;
  RangeDiffusionSlice rs;
  DiffuseCorrelation dc;
  
  for (int32 y=0;y<int32(out.Height());y++)
  {
   prog->Report(2+y,disp.Size(1)+2);
   prog->Push();
   
   // Create the diffusion maps for this scanline...
    prog->Report(0,3);
    ls.Create(y,diffSteps,l,lw,prog);
    
    prog->Report(1,3);
    rs.Create(y,diffSteps,r,rw,prog);
    

   // Do the scanline...
    prog->Report(2,3);
    dc.Setup(dist,cap,l,ls,r,rs);
    for (int32 x=0;x<int32(out.Width());x++)
    {
     // Get the discrete x value for the other image, rounding if needed...
      int32 x2 = x + int32(math::Round(disp.Get(x,y)));
      out.Get(x,y) = math::Infinity<real32>();
      
     // Check the masking is safe - we throw away values that have bad masking...
      if ((l.ValidExt(x   ,y)==false)||
          (l.ValidExt(x+1 ,y)==false)||
          (l.ValidExt(x-1 ,y)==false)||
          (r.ValidExt(x2  ,y)==false)||
          (r.ValidExt(x2+1,y)==false)||
          (r.ValidExt(x2-1,y)==false)) continue;
     
     // Get the 5 needed correlation values - the current pixel and offset 
     // by 1 for each image...
      real32 centre = dc.Cost(x,x2);
      real32 negL = dc.Cost(x-1,x2);
      real32 posL = dc.Cost(x+1,x2);
      real32 negR = dc.Cost(x,x2-1);
      real32 posR = dc.Cost(x,x2+1);
     
     // Verify that the pixel is sufficiently better than the rest...
      if (((negL-centre)<prune)||
          ((posL-centre)<prune)||
          ((negR-centre)<prune)||
          ((posR-centre)<prune)) continue;
     
     // Refine the disparity position and store it...
      real32 p = 0.5*(negL+negR);
      real32 q = centre;
      real32 r = 0.5*(posL+posR);
      
      real32 dOS = (p-r)/(2.0*(p+r) - 4.0*q);
      out.Get(x,y) = real32(x2-x) + dOS;
    }

   prog->Pop();
  }
    
 
 prog->Pop();
}

void DiffCorrRefine::GetDisp(svt::Field<real32> & d) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   if (math::IsFinite(out.Get(x,y))) d.Get(x,y) = out.Get(x,y);
                                else d.Get(x,y) = 0.0;
  }
 }
}

void DiffCorrRefine::GetMask(svt::Field<bit> & mask) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   mask.Get(x,y) = math::IsFinite(out.Get(x,y));
  }
 }
}

//------------------------------------------------------------------------------
 };
};
