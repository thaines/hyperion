//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "eos/stereo/simple.h"

#include "eos/math/functions.h"
#include "eos/math/gaussian_mix.h"
#include "eos/mem/packer.h"
#include "eos/ds/priority_queues.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
HDSC::HDSC()
:dsc(null<DSC*>())
{}

HDSC::~HDSC()
{
 delete dsc;
}

void HDSC::Set(DSC * d)
{
 delete dsc;
 dsc = d->Clone();
 
 lMask.SetInvalid();
 rMask.SetInvalid();
 
 MakeHierachy();
}

void HDSC::Set(DSC * d,const svt::Field<bit> & lm,const svt::Field<bit> & rm)
{
 delete dsc;
 dsc = d->Clone();
 
 lMask = lm;
 rMask = rm;
 
 MakeHierachy();
}

nat32 HDSC::Levels() const
{
 return leftMask.Size();
}

nat32 HDSC::WidthLeft(nat32 l) const
{
 return leftMask[l].Width();
}

nat32 HDSC::WidthRight(nat32 l) const
{
 return rightMask[l].Width();
}

nat32 HDSC::Height(nat32 l) const
{
 return leftMask[l].Height();
}

void HDSC::CloneMe(HDSC & target) const
{
 delete target.dsc;
 target.dsc = dsc->Clone();
 target.lMask = lMask;
 target.rMask = rMask;
 
 target.MakeHierachy();
}

byte * HDSC::Left(nat32 level,nat32 x,nat32 y) const
{
 return leftCol[level][y].Ptr() + dsc->Bytes() * x;
}

byte * HDSC::Right(nat32 level,nat32 x,nat32 y) const
{
 return rightCol[level][y].Ptr() + dsc->Bytes() * x;
}

bit HDSC::LeftMask(nat32 level,nat32 x,nat32 y) const
{
 return leftMask[level].Get(x,y);
}

bit HDSC::RightMask(nat32 level,nat32 x,nat32 y) const
{
 return rightMask[level].Get(x,y);
}

void HDSC::MakeHierachy()
{
 LogTime("eos::stereo::HDSC::MakeHierachy");

 // Asserts...
  log::Assert(dsc->HeightLeft()==dsc->HeightRight());
  log::Assert((!lMask.Valid())||((lMask.Size(0)==dsc->WidthLeft())&&(lMask.Size(1)==dsc->HeightLeft())));
  log::Assert((!rMask.Valid())||((rMask.Size(0)==dsc->WidthRight())&&(rMask.Size(1)==dsc->HeightRight())));



 // Calculate how many levels are required...
  int32 levels = math::Max(math::TopBit(dsc->WidthLeft()),
                           math::TopBit(dsc->HeightLeft()),
                           math::TopBit(dsc->WidthRight()));



 // Create the data structures...
  leftMask.Size(levels);
  rightMask.Size(levels);
  
  leftMask[0].Resize(dsc->WidthLeft(),dsc->HeightLeft());
  rightMask[0].Resize(dsc->WidthRight(),dsc->HeightRight());
  for (int32 l=1;l<levels;l++)
  {
   nat32 widthLeft  = (leftMask[l-1].Width()/2)  + ((leftMask[l-1].Width()&1)?1:0);
   nat32 widthRight = (rightMask[l-1].Width()/2) + ((rightMask[l-1].Width()&1)?1:0);
   nat32 height     = (leftMask[l-1].Height()/2) + ((leftMask[l-1].Height()&1)?1:0);
  
   leftMask[l].Resize(widthLeft,height);
   rightMask[l].Resize(widthRight,height);
  }


  leftCol.Size(levels);
  rightCol.Size(levels);
  for (int32 l=0;l<levels;l++)
  {
   leftCol[l].Size(leftMask[l].Height());
   rightCol[l].Size(leftMask[l].Height());

   for (nat32 y=0;y<leftMask[l].Height();y++)
   {
    leftCol[l][y]  = new byte[dsc->Bytes() * leftMask[l].Width()];
    rightCol[l][y] = new byte[dsc->Bytes() * rightMask[l].Width()];
   }
  }



 // Fill in the first level of the mask...
  if (lMask.Valid())
  {
   for (nat32 y=0;y<leftMask[0].Height();y++)
   {
    for (nat32 x=0;x<leftMask[0].Width();x++) leftMask[0].Get(x,y) = lMask.Get(x,y);
   }
  }
  else
  {
   for (nat32 y=0;y<leftMask[0].Height();y++)
   {
    for (nat32 x=0;x<leftMask[0].Width();x++) leftMask[0].Get(x,y) = true;
   }
  }
  
  if (rMask.Valid())
  {
   for (nat32 y=0;y<rightMask[0].Height();y++)
   {
    for (nat32 x=0;x<rightMask[0].Width();x++) rightMask[0].Get(x,y) = rMask.Get(x,y);
   }
  }
  else
  {
   for (nat32 y=0;y<rightMask[0].Height();y++)
   {
    for (nat32 x=0;x<rightMask[0].Width();x++) rightMask[0].Get(x,y) = true;
   }
  }



 // Fill all deeper levels of the mask...
  for (int32 l=1;l<levels;l++)
  {
   for (nat32 y=0;y<leftMask[l].Height();y++)
   {
    for (nat32 x=0;x<leftMask[l].Width();x++)
    {
     nat32 prevX = x*2;
     nat32 prevY = y*2;

     bit val = leftMask[l-1].Get(prevX,prevY);
     if ((prevX+1)!=leftMask[l-1].Width()) val |= leftMask[l-1].Get(prevX+1,prevY);
     if ((prevY+1)!=leftMask[l-1].Height()) val |= leftMask[l-1].Get(prevX,prevY+1);
     if (((prevX+1)!=leftMask[l-1].Width())&&((prevY+1)!=leftMask[l-1].Height()))
        val |= leftMask[l-1].Get(prevX+1,prevY+1);
     leftMask[l].Get(x,y) = val;
    }
     
    for (nat32 x=0;x<rightMask[l].Width();x++)
    {
     nat32 prevX = x*2;
     nat32 prevY = y*2;

     bit val = rightMask[l-1].Get(prevX,prevY);
     if ((prevX+1)!=rightMask[l-1].Width()) val |= rightMask[l-1].Get(prevX+1,prevY);
     if ((prevY+1)!=rightMask[l-1].Height()) val |= rightMask[l-1].Get(prevX,prevY+1);
     if (((prevX+1)!=rightMask[l-1].Width())&&((prevY+1)!=rightMask[l-1].Height()))
        val |= rightMask[l-1].Get(prevX+1,prevY+1);
     rightMask[l].Get(x,y) = val;
    }     
   }
  }



 // Fill in the first level of the colour map...
  for (nat32 y=0;y<leftMask[0].Height();y++)
  {
   for (nat32 x=0;x<leftMask[0].Width();x++) dsc->Left(x,y,leftCol[0][y] + dsc->Bytes() * x);
   for (nat32 x=0;x<rightMask[0].Width();x++) dsc->Right(x,y,rightCol[0][y] + dsc->Bytes() * x);
  }



 // Fill in all deeper levels of the colour map...
 {
  mem::StackPtr<byte> temp[2];
  for (nat32 i=0;i<2;i++) temp[i] = new byte[dsc->Bytes()];
  
  for (int32 l=1;l<levels;l++)
  {
   for (nat32 y=0;y<leftMask[l].Height();y++)
   {
    for (nat32 x=0;x<leftMask[l].Width();x++)
    {
     nat32 prevX = x*2;
     nat32 prevY = y*2;
      
     // Get pointers to the 4 extraction points...
      const byte * ep[4];

      if (leftMask[l-1].Get(prevX,prevY)==false) ep[0] = null<byte*>();
         else ep[0] = leftCol[l-1][prevY] + dsc->Bytes() * prevX;
      if (((prevX+1)==leftMask[l-1].Width())||
          (leftMask[l-1].Get(prevX+1,prevY)==false)) ep[1] = null<byte*>();
         else ep[1] = leftCol[l-1][prevY] + dsc->Bytes() * (prevX+1);
      if (((prevY+1)==leftMask[l-1].Height())||
          (leftMask[l-1].Get(prevX,prevY+1)==false)) ep[2] = null<byte*>();
         else ep[2] = leftCol[l-1][prevY+1] + dsc->Bytes() * prevX;
      if (((prevX+1)==leftMask[l-1].Width())||
          ((prevY+1)==leftMask[l-1].Height())||
          (leftMask[l-1].Get(prevX+1,prevY+1)==false)) ep[3] = null<byte*>();
         else ep[3] = leftCol[l-1][prevY+1] + dsc->Bytes() * (prevX+1);

     // Merge and store...
      dsc->Join(4,ep,leftCol[l][y] + dsc->Bytes() * x);
    }
     
    for (nat32 x=0;x<rightMask[l].Width();x++)
    {
     nat32 prevX = x*2;
     nat32 prevY = y*2;
      
     // Get pointers to the 4 extraction points...
      const byte * ep[4];

      if (rightMask[l-1].Get(prevX,prevY)==false) ep[0] = null<byte*>();
         else ep[0] = rightCol[l-1][prevY] + dsc->Bytes() * prevX;
      if (((prevX+1)==rightMask[l-1].Width())||
          (rightMask[l-1].Get(prevX+1,prevY)==false)) ep[1] = null<byte*>();
         else ep[1] = rightCol[l-1][prevY] + dsc->Bytes() * (prevX+1);
      if (((prevY+1)==rightMask[l-1].Height())||
          (rightMask[l-1].Get(prevX,prevY+1)==false)) ep[2] = null<byte*>();
         else ep[2] = rightCol[l-1][prevY+1] + dsc->Bytes() * prevX;
      if (((prevX+1)==rightMask[l-1].Width())||
          ((prevY+1)==rightMask[l-1].Height())||
          (rightMask[l-1].Get(prevX+1,prevY+1)==false)) ep[3] = null<byte*>();
         else ep[3] = rightCol[l-1][prevY+1] + dsc->Bytes() * (prevX+1);

     // Merge and store...
      dsc->Join(4,ep,rightCol[l][y] + dsc->Bytes() * x);
    }
   }
  }
 }
}

//------------------------------------------------------------------------------
BiLatAD::BiLatAD()
:radius(4),distSd(1.5),colScale(4.0)
{}

BiLatAD::~BiLatAD()
{}

HDSC * BiLatAD::Clone() const
{
 BiLatAD * ret = new BiLatAD();
 CloneMe(*ret);
 
 ret->radius = radius;
 ret->distSd = distSd;
 ret->colScale = colScale;
 
 return ret;
}

void BiLatAD::SetParas(nat32 r,real32 sd,real32 sc)
{
 radius = r;
 distSd = sd;
 colScale = sc;
}

real32 BiLatAD::Cost(nat32 level,int32 leftX,int32 rightX,int32 bothY) const
{
 LogTime("eos::stereo::BiLatAD::Cost");
 
 real32 totalCost = 0.0;
 real32 totalWeight = 0.0;
 
 bit lcMask = (leftX>=0)&&(leftX<int32(WidthLeft(level)))&&(bothY>=0)&&(bothY<int32(Height(level)));
 bit rcMask = (rightX>=0)&&(rightX<int32(WidthRight(level)))&&(bothY>=0)&&(bothY<int32(Height(level)));
 
 lcMask = lcMask&&LeftMask(level,leftX,bothY);
 rcMask = rcMask&&RightMask(level,rightX,bothY);


 for (int32 v=-radius;v<=radius;v++)
 {
  for (int32 u=-radius;u<=radius;u++)
  {
   // Check if the relevant coordinates are masked or not...
    bit lMask = ((leftX+u)>=0)&&((leftX+u)<int32(WidthLeft(level)))&&
                ((bothY+v)>=0)&&((bothY+v)<int32(Height(level)));
    bit rMask = ((rightX+u)>=0)&&((rightX+u)<int32(WidthRight(level)))&&
                ((bothY+v)>=0)&&((bothY+v)<int32(Height(level)));

    lMask = lMask&&LeftMask(level,leftX+u,bothY+v);
    rMask = rMask&&RightMask(level,rightX+u,bothY+v);
    
    if ((!lMask)||(!rMask)) continue;


   // Calculate the colour differences...
    real32 leftDiff = (lMask&&lcMask)?
                      GetDSC()->Cost(Left(level,leftX,bothY),Left(level,leftX+u,bothY+v)):
                      math::Infinity<real32>();
    real32 rightDiff = (rMask&&rcMask)?
                       GetDSC()->Cost(Right(level,rightX,bothY),Right(level,rightX+u,bothY+v)):
                       math::Infinity<real32>();

    real32 diff = math::Min(leftDiff,rightDiff);
    if (!math::IsFinite(diff)) diff = 0.0;

    real32 cost = GetDSC()->Cost(Left(level,leftX+u,bothY+v),Right(level,rightX+u,bothY+v));


   // Use the distance from the centre and the colour differences to calculate the weight...
    real32 dist = math::Sqrt(math::Sqr<real32>(u) + math::Sqr<real32>(v));
    real32 weight = math::Gaussian(real32(radius)/distSd,dist) * math::Gaussian(colScale,diff);


   // Incrimentally add it to the final cost value...
    if (!math::IsZero(weight))
    {
     totalWeight += weight;
     totalCost += (weight * (cost-totalCost)) / totalWeight;
    }
  }
 }

 return totalCost;
}

cstrconst BiLatAD::TypeString() const
{
 return "eos::stereo::BiLatAD";
}

//------------------------------------------------------------------------------
SimpleStereo::SimpleStereo()
:hdsc(null<HDSC*>()),limit(2)
{}

SimpleStereo::~SimpleStereo()
{
 delete hdsc;
}

void SimpleStereo::Set(const HDSC * h)
{
 delete hdsc;
 hdsc = h->Clone();
}

void SimpleStereo::Set(nat32 l)
{
 limit = l;
}

void SimpleStereo::Run(time::Progress * prog)
{
 LogTime("eos::stereo::SimpleStereo::Run");
 prog->Push();
 
 // Create an index and packer for each layer of the hierachy...
  prog->Report(0,hdsc->Levels()+2);
  ds::ArrayDel< ds::Array2D<DispList> > index(hdsc->Levels());
  ds::ArrayDel< mem::StackPtr<mem::Packer> > storage(hdsc->Levels());
  for (nat32 i=0;i<storage.Size();i++) storage[i] = new mem::Packer(blockSize);
  
  
 // Create the layer limit data structure and some helper storage...
  ds::Array<nat32> layerLimit(hdsc->Levels());
  layerLimit[0] = limit;
  for (nat32 i=1;i<layerLimit.Size();i++) layerLimit[i] = layerLimit[i-1]*2;
  
  // Stores all passed down values each iteration - exists so we can sort 'em...
   ds::Array<DispCost> dcArray(layerLimit[layerLimit.Size()-1]);
   
  // For grabbing the n best, used in a strange way.
   ds::PriorityQueue<DispCost> dcHeap(layerLimit[layerLimit.Size()-1]);


 // Initialise the top layer with all disparities...
  prog->Report(1,hdsc->Levels()+2);
  nat32 topL = hdsc->Levels()-1;
  index[topL].Resize(hdsc->WidthLeft(topL),hdsc->Height(topL));
  prog->Push();
  for (nat32 y=0;y<index[topL].Height();y++)
  {
   prog->Report(y,index[topL].Height());
   for (nat32 x=0;x<index[topL].Width();x++)
   {
    // Check if its masked, if so we want nothing...
     if (hdsc->LeftMask(topL,x,y)==false)
     {
      index[topL].Get(x,y).size = 0;
      index[topL].Get(x,y).data = null<DispCost*>();
      continue;
     }
     
    // Count the number of disparities that need storing...
     int32 dispCount = 0;
     for (nat32 x2=0;x2<hdsc->WidthRight(topL);x2++)
     {
      if (hdsc->RightMask(topL,x2,y)) ++dispCount;
     }

    // Create the data structure...
     DispCost * out = storage[topL]->Malloc<DispCost>(dispCount);
     index[topL].Get(x,y).size = dispCount;
     index[topL].Get(x,y).data = out;
    
    // Second pass to fill it in...
     for (nat32 x2=0;x2<hdsc->WidthRight(topL);x2++)
     {
      if (hdsc->RightMask(topL,x2,y))
      {
       out->disp = int32(x2) - int32(x);
       out->cost = 0.0;
       ++out;
      }
     }
   }
  }
  prog->Pop();


 // Iterate all layers but the top, transfering from the layer above the best 
 // disparities in the implied ranges...
  for (int32 l=hdsc->Levels()-2;l>=0;l--)
  {
   prog->Report(hdsc->Levels()-l,hdsc->Levels()+2);
   // Size the index for this new layer...
    index[l].Resize(hdsc->WidthLeft(l),hdsc->Height(l));
   
   // Iterate each pixel one at a time and determine its disparity values for
   // further consideration...
    prog->Push();
    for (nat32 y=0;y<index[l].Height();y++)
    {
     prog->Report(y,index[l].Height());
     for (nat32 x=0;x<index[l].Width();x++)
     {
      nat32 parentSize = index[l+1].Get(x/2,y/2).size;
      DispCost * parent = index[l+1].Get(x/2,y/2).data;

      // Check if we are masked, if so do nothing...
       if (parentSize==0)
       {
        index[l].Get(x,y).size = 0;
        index[l].Get(x,y).data = null<DispCost*>();
        continue;
       }

      // Iterate the disparities from the above layer, for each one project it 
      // to this layer and iterate the surrounding search range. For every cost
      // found record the n best, where n is limit * 2^l...
       int32 disp = parent[0].disp - int32(expand);
       dcHeap.MakeEmpty();
       for (nat32 i=0;i<parentSize;i++)
       {
        int32 start = math::Max(disp,parent[i].disp - int32(expand));
        int32 end = parent[i].disp + int32(expand);
        for (int32 d=start;d<=end;d++)
        {
         real32 cost = hdsc->Cost(l,x,x+d,y);

         if (dcHeap.Size()==layerLimit[l])
         {
          if (dcHeap.Peek().cost<cost) continue;
          dcHeap.Rem();
         }
         
         DispCost dc;
         dc.disp = d;
         dc.cost = cost;
         dcHeap.Add(dc);
        }
        disp = end+1;
       }
       
      // We have all the relevant disparities for this transfer - we need them
      // sorted by disparity rather than cost however, so transfer to array and
      // sort...
       nat32 num = dcHeap.Size();
       for (nat32 i=0;i<num;i++)
       {
        dcArray[i] = dcHeap.Peek();
        dcHeap.Rem();
       }

       dcArray.SortRange<SortDC>(0,num-1);

      // Now we have the array of disparities create the storage object,
      // and store them...
       DispCost * out = storage[topL]->Malloc<DispCost>(num);
       index[l].Get(x,y).size = num;
       index[l].Get(x,y).data = out;
       
       for (nat32 i=0;i<num;i++) out[i] = dcArray[i];
     }
    }
    prog->Pop();
   
   // Terminate the previous layers memory, to keep usage down...
    //index[l+1].Resize(0,0);
    //storage[l+1]->Reset(); ************************************* Causes a crash. ***********************
  }


 // Extract from the bottom layer into the output data structure...
  prog->Report(hdsc->Levels()+1,hdsc->Levels()+2);
  data.Size(index[0].Height());
  prog->Push();
  for (nat32 y=0;y<data.Size();y++)
  {
   prog->Report(y,data.Size());
   // Fill in the scanline index in a first pass...
    data[y].index.Size(index[0].Width()+1);
    data[y].index[0] = 0;
    for (nat32 x=0;x<index[0].Width();x++)
    {
     data[y].index[x+1] = data[y].index[x] + index[0].Get(x,y).size;
    }

   // Fill in the data in a second pass...
    data[y].data.Size(data[y].index[index[0].Width()]);
    nat32 pos = 0;
    for (nat32 x=0;x<index[0].Width();x++)
    {
     for (nat32 i=0;i<index[0].Get(x,y).size;i++)
     {
      data[y].data[pos] = index[0].Get(x,y).data[i];
      ++pos;
     }
    }
  }
  prog->Pop();

 prog->Pop();
}

nat32 SimpleStereo::Width() const
{
 return hdsc->WidthLeft(0);
}

nat32 SimpleStereo::Height() const
{
 return hdsc->Height(0);
}

nat32 SimpleStereo::Size(nat32 x,nat32 y) const
{
 return data[y].index[x+1] - data[y].index[x];
}

real32 SimpleStereo::Disp(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].disp;
}

real32 SimpleStereo::Cost(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].cost;
}

real32 SimpleStereo::DispWidth(nat32 x,nat32 y,nat32 i) const
{
 return 0.5;
}

cstrconst SimpleStereo::TypeString() const
{
 return "eos::stereo::SimpleStereo";
}

//------------------------------------------------------------------------------
 };
};
