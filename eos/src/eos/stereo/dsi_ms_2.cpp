//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/stereo/dsi_ms_2.h"

#include "eos/mem/safety.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
SparseDSI2::SparseDSI2()
:occCost(1.0),pixLim(1),vertCost(1.0),vertMult(0.2),
dsc(null<DSC*>())
{}

SparseDSI2::~SparseDSI2()
{
 delete dsc;
}

void SparseDSI2::Set(real32 occC,real32 vCost,real32 vMult,nat32 pixL)
{
 occCost = occC;
 vertCost = vCost;
 vertMult = vMult;
 pixLim = pixL;
}

void SparseDSI2::Set(DSC * d)
{
 delete dsc;
 dsc = d->Clone();
}

void SparseDSI2::Set(const svt::Field<bit> & ml,const svt::Field<bit> & mr)
{
 maskLeft = ml;
 maskRight = mr;
}

void SparseDSI2::Run(time::Progress * prog)
{
 LogBlock("eos::stereo::SparseDSI2::Run","-");
 prog->Push();
 #ifdef EOS_DEBUG
 nat32 dispCount = 0;
 #endif


 // Create data structures needed during the process, plus other initialisation
 // stuff...
  // Store the image sizes...
   widthLeft = dsc->WidthLeft();
   heightLeft = dsc->HeightLeft();
   widthRight = dsc->WidthRight();
   heightRight = dsc->HeightRight();

   log::Assert(heightLeft==heightRight);
   log::Assert((!maskLeft.Valid())||((int32(maskLeft.Size(0))==widthLeft)&&(int32(maskLeft.Size(1))==heightLeft)));
   log::Assert((!maskRight.Valid())||((int32(maskRight.Size(0))==widthRight)&&(int32(maskRight.Size(1))==heightRight)));


  // Prepare the output array...
   data.Size(heightLeft);
   for (int32 y=0;y<heightLeft;y++) data[y].index.Size(widthLeft+1);


  // Calculate the height of the hierachy we are going to work on for each scanline...
   int32 levels = math::Min(math::TopBit(widthLeft),math::TopBit(widthRight));


  // Create a data structure to store the masking for a single scanline...
   ds::ArrayDel< ds::Array<bit> > leftMask(levels);
   ds::ArrayDel< ds::Array<bit> > rightMask(levels);
   for (int32 l=0;l<levels;l++)
   {
    leftMask[l].Size(widthLeft>>l);
    rightMask[l].Size(widthRight>>l);
   }


  // Create a data structure to store a feature hierachy for a single scanline...
   ds::ArrayDel< mem::StackPtr<byte> > leftCol(levels);
   ds::ArrayDel< mem::StackPtr<byte> > rightCol(levels);
   for (int32 l=0;l<levels;l++)
   {
    leftCol[l] = new byte[dsc->Bytes() * (widthLeft>>l)];
    rightCol[l] = new byte[dsc->Bytes() * (widthRight>>l)];
   }


  // The previous row cost structure...
   ds::ArrayDel< ds::InplaceKdTree<byte*,2> > prevRow(levels);
   for (int32 l=0;l<levels;l++) prevRow[l].Shrinks(false);


  // Storage for the current ordered match set. Stored in an evergrowing array
  // to avoid memory bashing. Defaults to being the size of the left image,
  // most cases will ultimatly make it bigger once it hits the lower levels in
  // the hierachy...
   ds::ArrayNS<Match> matchSet(widthLeft);

  // Various buffers etc so we don't have to allocate stuff regularly...
   ds::ArrayDel<ds::SparseBitArray> expandTemp(widthLeft+widthRight);

   ds::Array<real32> pruneLeft(widthLeft);
   ds::Array<real32> pruneRight(widthRight);

   ds::Array<BestCost> leftCost(widthRight);
   ds::Array<BestCost> rightCost(widthLeft);

   ds::Array<BestMatch> leftBest(widthLeft);
   ds::Array<BestMatch> rightBest(widthRight);
   
   ds::MultiNth nth;

  // Storage of colour information for vertical consistancy, lots of byte
  // buffers, one for each level, all sized to the maximum possible storage
  //requirement...
   ds::ArrayDel< mem::StackPtr<byte> > colBuf(levels);
   for (int32 l=0;l<levels;l++)
   {
    colBuf[l] = new byte[dsc->Bytes() * 2 * math::Min(widthLeft>>l,widthRight>>l)];
   }



 // Process the image a row at a time...
  for (int32 y=0;y<heightLeft;y++)
  {
   prog->Report(y,heightLeft);

   // Calculate the mask hierachy...
    // Left...
    {
     // Start...
      if (maskLeft.Valid())
      {
       for (int32 x=0;x<widthLeft;x++) leftMask[0][x] = maskLeft.Get(x,y);
      }
      else
      {
       for (int32 x=0;x<widthLeft;x++) leftMask[0][x] = true;
      }

     // Rest of hierachy - 'or' together the lower levels...
      for (int32 l=1;l<levels;l++)
      {
       int32 prevWidth = widthLeft>>(l-1);
       int32 currWidth = widthLeft>>l;

       for (int32 x=0;x<currWidth;x++)
       {
        if ((x*2+1)!=prevWidth) leftMask[l][x] = leftMask[l-1][x*2] | leftMask[l-1][x*2+1];
                           else leftMask[l][x] = leftMask[l-1][x*2];
       }
      }
    }

    // Right...
    {
     // Start...
      if (maskRight.Valid())
      {
       for (int32 x=0;x<widthRight;x++) rightMask[0][x] = maskRight.Get(x,y);
      }
      else
      {
       for (int32 x=0;x<widthRight;x++) rightMask[0][x] = true;
      }

     // Rest of hierachy - 'or' together the lower levels...
      for (int32 l=1;l<levels;l++)
      {
       int32 prevWidth = widthRight>>(l-1);
       int32 currWidth = widthRight>>l;

       for (int32 x=0;x<currWidth;x++)
       {
        if ((x*2+1)!=prevWidth) rightMask[l][x] = rightMask[l-1][x*2] | rightMask[l-1][x*2+1];
                           else rightMask[l][x] = rightMask[l-1][x*2];
       }
      }
    }


   // Calculate the entire feature hierachy for efficiency...
   // (We have to handle masking whilst we are at it, painful.)
    // Left...
     {
      // Starting level...
       byte * targ = leftCol[0].Ptr();
       for (int32 x=0;x<widthLeft;x++)
       {
        dsc->Left(x,y,targ);
        targ += dsc->Bytes();
       }

      // Rest of hierachy...
       for (int32 l=1;l<levels;l++)
       {
        int32 prevWidth = widthLeft>>(l-1);
        int32 currWidth = widthLeft>>l;

        byte * prevTarg = leftCol[l-1].Ptr();
        byte * currTarg = leftCol[l].Ptr();
        for (int32 x=0;x<currWidth;x++)
        {
         if ((x*2+1)!=prevWidth)
         {
          if (leftMask[l-1][x*2+1]==false) mem::Copy(currTarg,prevTarg,dsc->Bytes());
          else
          {
           if (leftMask[l-1][x*2]==false) mem::Copy(currTarg,prevTarg+dsc->Bytes(),dsc->Bytes());
                                     else dsc->Join(prevTarg,prevTarg+dsc->Bytes(),currTarg);
          }
         }
         else mem::Copy(currTarg,prevTarg,dsc->Bytes());

         prevTarg += dsc->Bytes() * 2;
         currTarg += dsc->Bytes();
        }
       }
     }

    // Right...
     {
      // Starting level...
       byte * targ = rightCol[0].Ptr();
       for (int32 x=0;x<widthRight;x++)
       {
        dsc->Right(x,y,targ);
        targ += dsc->Bytes();
       }

      // Rest of hierachy...
       for (int32 l=1;l<levels;l++)
       {
        int32 prevWidth = widthRight>>(l-1);
        int32 currWidth = widthRight>>l;

        byte * prevTarg = rightCol[l-1].Ptr();
        byte * currTarg = rightCol[l].Ptr();
        for (int32 x=0;x<currWidth;x++)
        {
         if ((x*2+1)!=prevWidth)
         {
          if (rightMask[l-1][x*2+1]==false) mem::Copy(currTarg,prevTarg,dsc->Bytes());
          else
          {
           if (rightMask[l-1][x*2]==false) mem::Copy(currTarg,prevTarg+dsc->Bytes(),dsc->Bytes());
                                      else dsc->Join(prevTarg,prevTarg+dsc->Bytes(),currTarg);
          }
         }
         else mem::Copy(currTarg,prevTarg,dsc->Bytes());

         prevTarg += dsc->Bytes() * 2;
         currTarg += dsc->Bytes();
        }
       }
     }


   // Prepare the starting ordered match set, we match every pixel to every
   // other in the top level, with consideration of masking of course...
   // (The hierachy is of course set so the highest level has only 1 range in
   // one of the images.)
   {
    int32 leftWid = widthLeft>>(levels-1);
    int32 rightWid = widthRight>>(levels-1);

    matchSet.Size(leftWid*rightWid);

    nat32 pos = 0;
    for (int32 sum=0;sum<=leftWid+rightWid-2;sum++)
    {
     for (int32 left=math::Max(int32(0),sum-rightWid+1);left<math::Min(leftWid,sum+1);left++)
     {
      int32 right = sum-left;
      if (leftMask[levels-1][left]&&rightMask[levels-1][right])
      {
       matchSet[pos].left = left;
       matchSet[pos].right = right;
       pos++;
      }
     }
    }

    matchSet.Size(pos);
   }


   // Iterate throught the levels, calculating each and passing to the next...
    for (int32 l=levels-1;l>=0;l--)
    {
     // Calculate the cost of the matchSet - 3 passes...
      CostPass(matchSet,leftCol[l].Ptr(),rightCol[l].Ptr(),
               ((y!=0)&&(prevRow[l].Size()!=0))?(&prevRow[l]):null<ds::InplaceKdTree<byte*,2>*>());
      IncPass(l,matchSet,leftCost,rightCost);
      DecPass(l,matchSet,leftCost,rightCost);

     // Prune out matches with costs that are too high...
      Prune(l,matchSet,pruneLeft,pruneRight,nth);


     // If enabled create the previous row data structure at this resolution
     // ready for the next level...
      if (!math::IsZero(vertMult))
      {
       // Reset the needed parts of the leftBest and rightBest buffers...
        for (int32 i=0;i<(widthLeft>>l);i++)
        {
         leftBest[i].other = -1;
         leftBest[i].cost = math::Infinity<real32>();
        }

        for (int32 i=0;i<(widthRight>>l);i++)
        {
         rightBest[i].other = -1;
         rightBest[i].cost = math::Infinity<real32>();
        }


       // Find the index of the minimum cost member for each pixel in the left
       // and right sides...
        for (nat32 i=0;i<matchSet.Size();i++)
        {
         real32 matchCost = matchSet[i].TotalCost();

         if (matchCost<leftBest[matchSet[i].left].cost)
         {
          leftBest[matchSet[i].left].other = matchSet[i].right;
          leftBest[matchSet[i].left].cost  = matchCost;
         }

         if (matchCost<rightBest[matchSet[i].right].cost)
         {
          rightBest[matchSet[i].right].other = matchSet[i].left;
          rightBest[matchSet[i].right].cost  = matchCost;
         }
        }


       // Work out how many best costs there are going to be from the above
       // calculated data structure...
        nat32 costCount = 0;
        for (int32 i=0;i<(widthLeft>>l);i++)
        {
         if (leftBest[i].other!=-1)
         {
          if (rightBest[leftBest[i].other].other==i) costCount += 1;
         }
        }


       // Iterate and store all the best costs, with each one we store both its
       // left and right colour, such that we can adjust the vertical
       // consistancy cost by vertical colour difference...
        prevRow[l].Size(costCount);
        costCount = 0;
        for (nat32 i=0;i<matchSet.Size();i++)
        {
         if ((leftBest[matchSet[i].left].other==matchSet[i].right)&&
             (rightBest[matchSet[i].right].other==matchSet[i].left))
         {
          math::Vect<2> pos;
          pos[0] = matchSet[i].left;
          pos[1] = matchSet[i].right;
          prevRow[l].SetPos(costCount,pos);

          nat32 cs = dsc->Bytes();
          byte * targ = colBuf[l].Ptr() + cs * 2 * costCount;
          prevRow[l][costCount] = targ;
          mem::Copy(targ   ,leftCol[l].Ptr()  + cs*matchSet[i].left ,cs);
          mem::Copy(targ+cs,rightCol[l].Ptr() + cs*matchSet[i].right,cs);

          costCount += 1;
         }
        }
        log::Assert(costCount==prevRow[l].Size());

       // Build it, so we can use it next time around...
        prevRow[l].Build();
      }


     // Unless this is the last level expand out the matches for the next level
     // below in the hierachy...
      if (l!=0) Expand(l-1,matchSet,leftMask[l-1],rightMask[l-1],expandTemp);
    }


   // Extract the final result for this scanline...
    Extract(matchSet,data[y]);

    #ifdef EOS_DEBUG
    dispCount += data[y].data.Size();
    #endif
  }


 LogDebug("[SparseStereo 2] {left pixels,disparities,d/p}" << LogDiv()
            << (widthLeft*heightLeft) << LogDiv() << dispCount << LogDiv()
            << real32(dispCount)/real32(widthLeft*heightLeft));
 prog->Pop();
}

nat32 SparseDSI2::Size(nat32 x,nat32 y) const
{
 return data[y].index[x+1] - data[y].index[x];
}

real32 SparseDSI2::Disp(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].disp;
}

real32 SparseDSI2::Cost(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].cost;
}

void SparseDSI2::SortByDisp(time::Progress * prog)
{
 prog->Push();
  // Simply sort each pixel by a suitable cost function...
  // (Lots of lil' sorts.)
   for (int32 y=0;y<heightLeft;y++)
   {
    prog->Report(y,heightLeft);
    for (int32 x=0;x<widthLeft;x++)
    {
     data[y].data.SortRange<DispCost::SortDisp>(data[y].index[x],data[y].index[x+1]-1);
    }
   }
 prog->Pop();
}

void SparseDSI2::SortByCost(time::Progress * prog)
{
 prog->Push();
  // Simply sort each pixel by a suitable cost function...
  // (Lots of lil' sorts.)
   for (int32 y=0;y<heightLeft;y++)
   {
    prog->Report(y,heightLeft);
    for (int32 x=0;x<widthLeft;x++)
    {
     data[y].data.SortRange<DispCost::SortCost>(data[y].index[x],data[y].index[x+1]-1);
    }
   }
 prog->Pop();
}

//------------------------------------------------------------------------------
void SparseDSI2::CostPass(ds::ArrayNS<Match> & matchSet,
                          byte * leftCol,byte * rightCol,
                          ds::InplaceKdTree<byte*,2> * prevRow)
{
 LogTime("eos::stereo::SparseDSI2::CostPass");

 // Iterate each match and calculate its cost, this is simply the matching cost
 // plus the vertical consistancy cost as applicable. We optimise calculation
 // speed for the vertical cost quite a lot as otherwise it takes too long...
  for (nat32 i=0;i<matchSet.Size();i++)
  {
   matchSet[i].cost = dsc->Cost(leftCol  + dsc->Bytes()*matchSet[i].left,
                                rightCol + dsc->Bytes()*matchSet[i].right);

   if ((prevRow)&&(!math::IsZero(vertMult)))
   {
    math::Vect<2> pos;
    pos[0] = matchSet[i].left;
    pos[1] = matchSet[i].right;

    real32 dist;
    byte * col = (*prevRow)[prevRow->NearestMan(pos,&dist)];
    real32 colCost = dsc->Cost(leftCol   + dsc->Bytes()*matchSet[i].left ,col) +
                     dsc->Cost(rightCol  + dsc->Bytes()*matchSet[i].right,col+dsc->Bytes());

    matchSet[i].cost += vertMult * math::Exp(-colCost*vertCost) * dist;
   }
  }
}

void SparseDSI2::IncPass(int32 level,ds::ArrayNS<Match> & matchSet,
                         ds::Array<BestCost> & leftCost,
                         ds::Array<BestCost> & rightCost)
{
 LogTime("eos::stereo::SparseDSI2::IncPass");

 int32 leftWidth  = widthLeft>>level;
 int32 rightWidth = widthRight>>level;


 // Reset the leftCost and rightCost to there starting positions.
  for (int32 i=0;i<rightWidth;i++)
  {
   leftCost[i].cost[0] = occCost*real32(i);
   leftCost[i].pos[0] = 0;
  }

  for (int32 i=0;i<leftWidth;i++)
  {
   rightCost[i].cost[0] = occCost*real32(i);
   rightCost[i].pos[0] = 0;
  }


 // Iterate the entire matchSet, setting the incCost for each and updating
 // the best arrays as we go - we can do this because of the order in which
 // the matchSet is presented...
 // Essentially we have to iterate each left+right set twice, once to update
 // costs and a second to update the best arrays.
 // The cost arrays are offset +1 to make the code neater, this makes the
 // connection with the basic idea kinda lose.
  int32 basePos = 0;
  while (true)
  {
   int32 dist = matchSet[basePos].left + matchSet[basePos].right;

   // Cost update pass...
    int32 pos = basePos;
    while ((pos!=int32(matchSet.Size()))&&(dist==(matchSet[pos].left+matchSet[pos].right)))
    {
     nat32 il = (matchSet[pos].left<leftCost[matchSet[pos].right].pos[0])?1:0;
     log::Assert(leftCost[matchSet[pos].right].pos[il]<=matchSet[pos].left);
     real32 cl = leftCost[matchSet[pos].right].cost[il] +
                 occCost * real32(matchSet[pos].left-leftCost[matchSet[pos].right].pos[il]);

     nat32 ir = (matchSet[pos].right<rightCost[matchSet[pos].left].pos[0])?1:0;
     log::Assert(rightCost[matchSet[pos].left].pos[ir]<=matchSet[pos].right);
     real32 cr = rightCost[matchSet[pos].left].cost[ir] +
                 occCost * real32(matchSet[pos].right-rightCost[matchSet[pos].left].pos[ir]);

     matchSet[pos].incCost = math::Min(cl,cr);

     pos += 1;
    }

   // Best cost array update pass...
    pos = basePos;
    while ((pos!=int32(matchSet.Size()))&&(dist==(matchSet[pos].left+matchSet[pos].right)))
    {
     int32 nl = matchSet[pos].left + 1;
     int32 nr = matchSet[pos].right + 1;
     real32 incCost = matchSet[pos].incCost + matchSet[pos].cost;

     // Left...
      if (nr!=rightWidth)
      {
       leftCost[nr].cost[1] = leftCost[nr].cost[0];
       leftCost[nr].pos[1] = leftCost[nr].pos[0];

       real32 cl = leftCost[nr].cost[0] + occCost*real32(nl-leftCost[nr].pos[0]);
       leftCost[nr].cost[0] = math::Min(cl,incCost);
       leftCost[nr].pos[0] = nl;
      }

     // Right...
      if (nl!=leftWidth)
      {
       rightCost[nl].cost[1] = rightCost[nl].cost[0];
       rightCost[nl].pos[1] = rightCost[nl].pos[0];

       real32 cr = rightCost[nl].cost[0] + occCost*real32(nr-rightCost[nl].pos[0]);
       rightCost[nl].cost[0] = math::Min(cr,incCost);
       rightCost[nl].pos[0] = nr;
      }

     pos += 1;
    }

   // To next subset or break...
    if (pos==int32(matchSet.Size())) break;
    basePos = pos;
  }
}

void SparseDSI2::DecPass(int32 level,ds::ArrayNS<Match> & matchSet,
                         ds::Array<BestCost> & leftCost,
                         ds::Array<BestCost> & rightCost)
{
 LogTime("eos::stereo::SparseDSI2::DecPass");

 int32 leftWidth  = widthLeft>>level;
 int32 rightWidth = widthRight>>level;


 // Reset the leftCost and rightCost to there starting positions.
  for (int32 i=0;i<rightWidth;i++)
  {
   leftCost[i].cost[0] = occCost*real32(rightWidth-1-i);
   leftCost[i].pos[0] = leftWidth-1;
  }

  for (int32 i=0;i<leftWidth;i++)
  {
   rightCost[i].cost[0] = occCost*real32(leftWidth-1-i);
   rightCost[i].pos[0] = rightWidth-1;
  }


 // Iterate the entire matchSet, setting the decCost for each and updating
 // the best arrays as we go - we can do this because of the order in which
 // the matchSet is presented...
 // Essentially we have to iterate each left+right set twice, once to update
 // costs and a second to update the best arrays.
 // The cost arrays are offset -1 to make the code neater, this makes the
 // connection with the basic idea kinda lose.
  int32 basePos = matchSet.Size()-1;
  while (true)
  {
   int32 dist = matchSet[basePos].left + matchSet[basePos].right;

   // Cost update pass...
    int32 pos = basePos;
    while ((pos!=-1)&&(dist==(matchSet[pos].left+matchSet[pos].right)))
    {
     nat32 il = (matchSet[pos].left>leftCost[matchSet[pos].right].pos[0])?1:0;
     real32 cl = leftCost[matchSet[pos].right].cost[il] +
                 occCost * real32(leftCost[matchSet[pos].right].pos[il]-matchSet[pos].left);

     nat32 ir = (matchSet[pos].right>rightCost[matchSet[pos].left].pos[0])?1:0;
     real32 cr = rightCost[matchSet[pos].left].cost[ir] +
                 occCost * real32(rightCost[matchSet[pos].left].pos[ir]-matchSet[pos].right);

     matchSet[pos].decCost = math::Min(cl,cr);

     pos -= 1;
    }

   // Best cost array update pass...
    pos = basePos;
    while ((pos!=-1)&&(dist==(matchSet[pos].left+matchSet[pos].right)))
    {
     int32 nl = matchSet[pos].left - 1;
     int32 nr = matchSet[pos].right - 1;
     real32 decCost = matchSet[pos].decCost + matchSet[pos].cost;

     // Left...
      if (nr!=-1)
      {
       leftCost[nr].cost[1] = leftCost[nr].cost[0];
       leftCost[nr].pos[1] = leftCost[nr].pos[0];

       real32 cl = leftCost[nr].cost[0] + occCost*real32(leftCost[nr].pos[0]-nl);
       leftCost[nr].cost[0] = math::Min(cl,decCost);
       leftCost[nr].pos[0] = nl;
      }

     // Right...
      if (nl!=-1)
      {
       rightCost[nl].cost[1] = rightCost[nl].cost[0];
       rightCost[nl].pos[1] = rightCost[nl].pos[0];

       real32 cr = rightCost[nl].cost[0] + occCost*real32(rightCost[nl].pos[0]-nr);
       rightCost[nl].cost[0] = math::Min(cr,decCost);
       rightCost[nl].pos[0] = nr;
      }

     pos -= 1;
    }

   // To next subset or break...
    if (pos==-1) break;
    basePos = pos;
  }
}

void SparseDSI2::Prune(int32 level,ds::ArrayNS<Match> & matchSet,
                       ds::Array<real32> & bestLeft,ds::Array<real32> & bestRight,
                       ds::MultiNth & nth)
{
 LogTime("eos::stereo::SparseDSI2::Prune");
 // Set the relevant section of the two arrays to infinity...
  int32 leftWidth = widthLeft>>level;
  int32 rightWidth = widthRight>>level;
  for (int32 i=0;i<leftWidth;i++) bestLeft[i] = math::Infinity<real32>();
  for (int32 i=0;i<rightWidth;i++) bestRight[i] = math::Infinity<real32>();


 // Run through and find the lowest costs for each pixel in each image...
  for (nat32 i=0;i<matchSet.Size();i++)
  {
   real32 cost = matchSet[i].TotalCost();
   bestLeft[matchSet[i].left] = math::Min(bestLeft[matchSet[i].left],cost);
   bestRight[matchSet[i].right] = math::Min(bestRight[matchSet[i].right],cost);
  }


 // If needed calculate the tolerance, so that it only allows a maximum of pixLim matches per pixel.
  real32 tolerance;
  if (pixLim>1)
  {
   nth.Reset(widthLeft+widthRight,pixLim-1);
    for (nat32 i=0;i<matchSet.Size();i++)
    {
     real32 cost = matchSet[i].TotalCost();
            cost -= math::Min(bestLeft[matchSet[i].left],bestRight[matchSet[i].right]);
     
     nth.Add(matchSet[i].left,cost);
     nth.Add(widthLeft+matchSet[i].right,cost);
    }
   tolerance = nth.Result();
  }
  else tolerance = 0.0;


 // Final pass, keep all matches that are within the tolerance, terminate the
 // rest...
  nat32 writePos = 0;
  for (nat32 i=0;i<matchSet.Size();i++)
  {
   real32 lim = tolerance + math::Min(bestLeft[matchSet[i].left],bestRight[matchSet[i].right]);
   if ((matchSet[i].TotalCost()<=lim)||(math::Equal(matchSet[i].TotalCost(),lim)))
   {
    if (writePos!=i) matchSet[writePos] = matchSet[i];
    writePos += 1;
   }
  }
  matchSet.Size(writePos);
}

void SparseDSI2::Expand(int32 newLevel,ds::ArrayNS<Match> & matchSet,
                        const ds::Array<bit> & leftMask,
                        const ds::Array<bit> & rightMask,
                        ds::ArrayDel<ds::SparseBitArray> & temp)
{
 LogTime("eos::stereo::SparseDSI2::Expand");

 // Temp represents matches such that left+right is the offset into the temp
 // array whilst the bit position is left. This means that the array is in the
 // order that it needs to be written out in.
  int32 newLeftWidth = widthLeft>>newLevel;
  int32 newRightWidth = widthRight>>newLevel;

 // Iterate the entire matchSet, flip the bits in temp to indicate the matches
 // that need to be done. Check the mask as we do this...
  for (nat32 i=0;i<matchSet.Size();i++)
  {
   int32 cL = matchSet[i].left * 2;
   int32 cR = matchSet[i].right * 2;

   for (int32 nr=math::Max(cR-range,int32(0));nr<=math::Min(cR+range+1,newRightWidth-1);nr++)
   {
    if (rightMask[nr]==false) continue;
    for (int32 nl=math::Max(cL-range,int32(0));nl<=math::Min(cL+range+1,newLeftWidth-1);nl++)
    {
     if (leftMask[nl]==false) continue;
     temp[nl+nr].Set(nl);
    }
   }
  }


 // Resize the matchSet based on the data in temp...
 {
  nat32 totalSize = 0;
  for (int32 i=0;i<newLeftWidth + newRightWidth - 1;i++)
  {
   totalSize += temp[i].Size();
  }
  matchSet.Size(totalSize);
 }


 // Fill up matchSet from temp...
 {
  nat32 offset = 0;
  for (int32 i=0;i<newLeftWidth + newRightWidth - 1;i++)
  {
   nat32 loc = 0;
   while (temp[i].NextInc(loc))
   {
    matchSet[offset].left  = int32(loc);
    matchSet[offset].right = i - int32(loc);

    loc += 1;
    offset += 1;
   }
  }
  log::Assert(offset==matchSet.Size(),"bit array mismatch");
 }


 // Reset temp, ready for the next time it is used...
  for (int32 i=0;i<newLeftWidth + newRightWidth - 1;i++) temp[i].Reset();
}

void SparseDSI2::Extract(const ds::ArrayNS<Match> & matchSet,Scanline & out)
{
 LogTime("eos::stereo::SparseDSI2::Extract");

 // Zeroeth pass - set out.index to all zeros...
  for (nat32 i=0;i<out.index.Size();i++) out.index[i] = 0;


 // First pass - count how many matches are assigned to each pixel...
  for (nat32 i=0;i<matchSet.Size();i++) out.index[matchSet[i].left] += 1;


 // Second pass - convert from size to offsets, and simultaneously
 // offset all to the first position of the next entry, so we can subtract them
 // back into position in the final pass...
  for (nat32 i=1;i<out.index.Size();i++) out.index[i] += out.index[i-1];


 // Third pass - iterate the matches and copy them into the structure,
 // correcting the offsets as we go...
  out.data.Size(out.index[out.index.Size()-1]);
  for (nat32 i=0;i<matchSet.Size();i++)
  {
   nat32 & ind = out.index[matchSet[i].left];
   ind -= 1;
   out.data[ind].disp = matchSet[i].Disp();
   out.data[ind].cost = matchSet[i].TotalCost()/real32(widthLeft+widthRight);
  }
}

//------------------------------------------------------------------------------
 };
};
