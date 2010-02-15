//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/dsi_ms.h"

#include "eos/ds/queues.h"
#include "eos/ds/lists.h"
#include "eos/cam/triangulation.h"
#include "eos/alg/fitting.h"
#include "eos/alg/depth_plane.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
SparseDSI::SparseDSI()
:occCost(1.0),horizCost(1.0),vertCost(1.0),vertMult(0.2),errLim(0.1),
dsc(null<DSC*>())
{}

SparseDSI::~SparseDSI()
{
 delete dsc;
}

void SparseDSI::Set(real32 occC,real32 vCost,real32 vMult,real32 errL)
{
 occCost = occC;
 horizCost = vCost;
 vertCost = vCost;
 vertMult = vMult;
 errLim = errL;
}

void SparseDSI::Set(DSC * d)
{
 delete dsc;
 dsc = d->Clone();
}

void SparseDSI::Run(time::Progress * prog)
{
 LogBlock("eos::stereo::SparseDSI::Run","-");
 prog->Push();

  // Basic preparation - declare some data structures...
   width  = dsc->WidthLeft();
   height = dsc->HeightLeft();
   widthRight  = dsc->WidthRight();
   heightRight = dsc->HeightRight();
   data.Size(height);
   for (nat32 y=0;y<height;y++) data[y].index.Size(width+1);
   
   
   ds::ArrayDel<DoubleNatWindow> pass(dsc->WidthRight());


  // Create data structure for storing the colour range hierachy...
   byte ** col[2];
   byte ** oldCol[2];
   nat32 levels = math::Min(math::TopBit(dsc->WidthLeft()),math::TopBit(dsc->WidthRight()));

   col[0] = new byte*[levels];
   col[1] = new byte*[levels];
   oldCol[0] = new byte*[levels];
   oldCol[1] = new byte*[levels];
   for (nat32 l=0;l<levels;l++)
   {
    col[0][l] = new byte[dsc->Bytes() * (dsc->WidthLeft()>>l)];
    col[1][l] = new byte[dsc->Bytes() * (dsc->WidthRight()>>l)];
    oldCol[0][l] = new byte[dsc->Bytes() * (dsc->WidthLeft()>>l)];
    oldCol[1][l] = new byte[dsc->Bytes() * (dsc->WidthRight()>>l)];    
   }
   
  // Stores a hierachy of scanlines for the previous row, except for the first 
  // as thats saved out and can be accessed anyway. Allows vertical costs to be 
  // applied.
   ds::ArrayDel<Scanline> prevRow(levels);


  // Iterate each row, doing its entire hierachy in one go - good for the 
  // progress bar & memory consumption...
   nat32 dispCount = 0;
   ds::ArrayNS<Node> dsiProg(dsc->WidthLeft()*2);
   for (nat32 y=0;y<height;y++)
   {
    prog->Report(y,height);


    // Swap the oldCol and col buffers - pointers only...
      math::Swap(col[0],oldCol[0]);
      math::Swap(col[1],oldCol[1]);


    // Create the colour range hierachy, for the row from each image...
     // Left...
     {
      // Starting level...
       byte * targ = col[0][0];
       for (nat32 x=0;x<dsc->WidthLeft();x++)
       {
        dsc->Left(x,y,targ);
        targ += dsc->Bytes();
       }
       
      // Rest of hierachy...
       for (nat32 l=1;l<levels;l++)
       {
        nat32 prevWidth = dsc->WidthLeft()>>(l-1);
        nat32 currWidth = dsc->WidthLeft()>>l;

        byte * prevTarg = col[0][l-1];
        byte * currTarg = col[0][l];
        for (nat32 x=0;x<currWidth;x++)
        {
         if ((x*2+1)!=prevWidth) dsc->Join(prevTarg,prevTarg+dsc->Bytes(),currTarg);
                            else mem::Copy(currTarg,prevTarg,dsc->Bytes());
         prevTarg += dsc->Bytes() * 2;
         currTarg += dsc->Bytes();
        }
       }
     }
     
     // Right...
     {
      // Starting level...
       byte * targ = col[1][0];
       for (nat32 x=0;x<dsc->WidthRight();x++)
       {
        dsc->Right(x,y,targ);
        targ += dsc->Bytes();
       }
       
      // Rest of hierachy...
       for (nat32 l=1;l<levels;l++)
       {
        nat32 prevWidth = dsc->WidthRight()>>(l-1);
        nat32 currWidth = dsc->WidthRight()>>l;
        
        byte * prevTarg = col[1][l-1];
        byte * currTarg = col[1][l];
        for (nat32 x=0;x<currWidth;x++)
        {
         if ((x*2+1)!=prevWidth) dsc->Join(prevTarg,prevTarg+dsc->Bytes(),currTarg);
                            else mem::Copy(currTarg,prevTarg,dsc->Bytes());
         prevTarg += dsc->Bytes() * 2;
         currTarg += dsc->Bytes();
        }
       }
     }


    // Create the scanline hierachy of the previous row, so we can add a term 
    // in for the difference with the previous row...
     if (y!=0)
     {
      prevRow[0] = data[y-1];
      for (nat32 i=1;i<prevRow.Size();i++) HalfScanline(prevRow[i-1],prevRow[i]);     
      for (nat32 i=0;i<prevRow.Size();i++) PropagateScanline(prevRow[i]);
     }


    // Prepare the first level...
     int32 wid[2];
      wid[0] = dsc->WidthLeft()>>(levels-1);
      wid[1] = dsc->WidthRight()>>(levels-1);

     dsiProg.Size(wid[0]*wid[1]);
     for (int32 i=0;i<wid[0];i++)
     {
      for (int32 j=0;j<wid[1];j++)
      {
       dsiProg[i*wid[1] + j].left = i;
       dsiProg[i*wid[1] + j].right = j;
      }
     }


    // Iterate the levels, calculating each...
     for (int32 l=levels-1;l>=0;l--)
     {
      // Prepare the prog...
       // Convert offsets into a disparity set...
        if (l+1!=int32(levels)) PrepProg(l,dsiProg);
       
       // Fill in the costs...
        nat32 vertOffset = 0;
        for (nat32 i=0;i<dsiProg.Size();i++)
        {
         // Pixel to pixel matching cost...
          dsiProg[i].cost = dsc->Cost(col[0][l] + dsc->Bytes()*dsiProg[i].left,
                                      col[1][l] + dsc->Bytes()*dsiProg[i].right);

         // Vertical consistancy cost...
          if (y!=0)
          {
           // Find the adjacent data position, done this way as whilst fiddly it
           // makes for an approximatly O(n) algorithm...
            vertOffset = math::Max(vertOffset,prevRow[l].index[dsiProg[i].left]);
            while (true)
            {
             if (prevRow[l].data[vertOffset].disp>=dsiProg[i].right-dsiProg[i].left) break;
             if (vertOffset+1>=prevRow[l].index[dsiProg[i].left+1]) break;
             vertOffset += 1;
            }

           if ((vertOffset>=prevRow[l].index[dsiProg[i].left])&&(vertOffset<prevRow[l].index[dsiProg[i].left+1]))
           {
            // Calculate the vertical cost...
             real32 bestVert;
             if (prevRow[l].data[vertOffset].disp==(dsiProg[i].right-dsiProg[i].left))
             {
              bestVert = prevRow[l].data[vertOffset].cost;
             }
             else
             {
              bestVert = prevRow[l].data[vertOffset].cost + 
                         vertCost * math::Abs(prevRow[l].data[vertOffset].disp - (dsiProg[i].right-dsiProg[i].left));

              if (vertOffset>prevRow[l].index[dsiProg[i].left])
              {
               bestVert = math::Min(bestVert,prevRow[l].data[vertOffset-1].cost +
                                    vertCost * math::Abs(prevRow[l].data[vertOffset-1].disp
                                    - (dsiProg[i].right-dsiProg[i].left)));
              }
             }
          
            // Add the vertical cost, including a term to take into account vertical similarity...
             real32 pixMatch = dsc->Cost(col[0][l] + dsc->Bytes()*dsiProg[i].left,
                                         oldCol[0][l] + dsc->Bytes()*dsiProg[i].left);
             pixMatch += dsc->Cost(col[1][l] + dsc->Bytes()*dsiProg[i].right,
                                   oldCol[1][l] + dsc->Bytes()*dsiProg[i].right);

             dsiProg[i].cost += math::Exp(-vertMult*pixMatch) * vertMult * bestVert;
           }
          }
        }
       
      // Run through the passes...
       FirstPass(l,dsiProg,pass);
       SecondPass(l,dsiProg,pass);
       ExtractPass(l,dsiProg);
     }


    // Extract the final result from the final prog...
     Extract(dsiProg,data[y].index,data[y].data);
     dispCount += data[y].data.Size();
   }


  // Clean up...
   for (nat32 i=0;i<2;i++)
   {
    for (nat32 l=0;l<levels;l++)
    {
     delete[] col[i][l];
     delete[] oldCol[i][l];
    }
    delete[] col[i];
    delete[] oldCol[i];
   }
   
   LogDebug("[SparseStereo] {pixels,disparities,d/p}" << LogDiv() 
            << (width*height) << LogDiv() << dispCount << LogDiv() 
            << real32(dispCount)/real32(width*height));


 prog->Pop();
}

nat32 SparseDSI::Size(nat32 x,nat32 y) const
{
 return data[y].index[x+1] - data[y].index[x];
}

real32 SparseDSI::Disp(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].disp;
}

real32 SparseDSI::Cost(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].cost;
}

void SparseDSI::SortByCost(time::Progress * prog)
{
 prog->Push();
  // Simply sort each pixel by a suitable cost function...
  // (Lots of lil' sorts.)
   for (nat32 y=0;y<height;y++)
   {
    prog->Report(y,height);
    for (nat32 x=0;x<width;x++)
    {
     data[y].data.SortRange<ds::SortOp<DispCost> >(data[y].index[x],data[y].index[x+1]-1);
    }
   }
 prog->Pop();
}

void SparseDSI::FirstPass(nat32 level,ds::ArrayNS<Node> & prog,ds::ArrayDel<DoubleNatWindow> & pass)
{
 LogTime("eos::stereo::SparseDSI::FirstPass");

 // Get the size of the search grid...
  int32 size[2];
  size[0] = dsc->WidthLeft()>>level;
  size[1] = dsc->WidthRight()>>level;


 // Prepare the data structure used to pass on scores...
  for (int32 i=0;i<size[1];i++) pass[i].MakeEmpty();
  int32 passLeft  = 0; // The current pixel line being proccessed, so when it changes we know to act.
  int32 passRight = 0; // Current position, exclusive, we have propagated upto for the current pixel.
  int32 passStart = 0; // When we move to the next pixel, we start propagating from here.


 // Iterate every last node, as we are doing this we propagate through the pass
 // array and set each nodes cost based on the pass array...
  for (nat32 i=0;i<prog.Size();i++)
  {
   // Check if we have moved to the next pixel set, if so fiddle...
    if (prog[i].left!=passLeft)
    {
     passLeft = prog[i].left;
     passRight = math::Min(passStart,prog[i].right);
     passStart = prog[i].right;
    }
   
   // Propagate the pass data structure upto the point we are currently at...
   // We do not propagate the current pixel line.
    while (passRight<prog[i].right)
    {
     if (pass[passRight].Size()>0)
     {
      if (prog[pass[passRight][0]].left!=prog[i].left)
      {
       if ((pass[passRight+1].Size()==0)||(IncCost(prog[pass[passRight][0]])<IncCost(prog[pass[passRight+1][0]])))
       {
        pass[passRight+1].Push(pass[passRight][0]);
       }
      }
      else
      {
       if (pass[passRight].Size()>1)
       {
        if ((pass[passRight+1].Size()==0)||(IncCost(prog[pass[passRight][1]])<IncCost(prog[pass[passRight+1][0]])))
        {
         pass[passRight+1].Push(pass[passRight][1]);
        }
       }
      }
     }
     ++passRight;
    }

   // Extract the best score to this node from the pass data structure, set this
   // nodes cost to be that plus its matching cost...
    if (prog[i].right==0)
    {
     prog[i].incCost = occCost*real32(prog[i].left) + prog[i].cost;
    }
    else
    {
     real32 passCost;
     if (pass[prog[i].right-1].Size()>0)
     {
      if (prog[pass[prog[i].right-1][0]].left!=prog[i].left)
      {
       passCost = IncCost(prog[pass[prog[i].right-1][0]],prog[i]);
      }
      else
      {
       if (pass[prog[i].right-1].Size()>1)
       {
        passCost = IncCost(prog[pass[prog[i].right-1][1]],prog[i]);
       }
       else passCost = math::Infinity<real32>();
      }
     }
     else passCost = math::Infinity<real32>();

     prog[i].incCost = math::Min(occCost*real32(prog[i].left + prog[i].right),passCost) + prog[i].cost;
    }

   // Insert this node into the pass data structure, if its good enough...
    if ((pass[prog[i].right].Size()==0)||(IncCost(prog[i])<IncCost(prog[pass[prog[i].right][0]])))
    {
     pass[prog[i].right].Push(i);
    }
  }
}

void SparseDSI::SecondPass(nat32 level,ds::ArrayNS<Node> & prog,ds::ArrayDel<DoubleNatWindow> & pass)
{
 LogTime("eos::stereo::SparseDSI::SecondPass");


 // Get the size of the search grid...
  int32 size[2];
  size[0] = dsc->WidthLeft()>>level;
  size[1] = dsc->WidthRight()>>level;


 // Prepare the data structure used to pass on scores...
  for (int32 i=0;i<size[1];i++) pass[i].MakeEmpty();
  int32 passLeft = size[0]-1;
  int32 passRight = size[1]-1;
  int32 passStart = size[1]-1;


 // Iterate every last node, as we are doing this we propagate through the pass
 // array and set each nodes cost based on the pass array.
  for (int32 i=prog.Size()-1;i>=0;i--)
  {
   // Check if we have moved to the next pixel set, if so fiddle...
    if (prog[i].left!=passLeft)
    {
     passLeft = prog[i].left;
     passRight = math::Max(passStart,prog[i].right);
     passStart = prog[i].right;
    }
 
   // Propagate the pass data structure upto the point we are currently at...
   // We do not propagate the current pixel line.
    while (passRight>prog[i].right)
    {
     if (pass[passRight].Size()>0)
     {
      if (prog[pass[passRight][0]].left!=prog[i].left)
      {
       if ((pass[passRight-1].Size()==0)||(DecCost(prog[pass[passRight][0]])<DecCost(prog[pass[passRight-1][0]])))
       {
        pass[passRight-1].Push(pass[passRight][0]);
       }
      }
      else
      {
       if (pass[passRight].Size()>1)
       {
        if ((pass[passRight-1].Size()==0)||(DecCost(prog[pass[passRight][1]])<DecCost(prog[pass[passRight-1][0]])))
        {
         pass[passRight-1].Push(pass[passRight][1]);
        }
       }
      }      
     }
     --passRight;
    }

   // Extract the best score to this node from the pass data structure, set this
   // nodes cost to be that plus its matching cost...
    if (prog[i].right+1==size[1])
    {
     prog[i].decCost = occCost*real32((size[0]-1-prog[i].left)) + prog[i].cost;
    }
    else
    {
     real32 passCost;
     if (pass[prog[i].right+1].Size()>0)
     {
      if (prog[pass[prog[i].right+1][0]].left!=prog[i].left)
      {
       passCost = DecCost(prog[pass[prog[i].right+1][0]],prog[i]);
      }
      else
      {
       if (pass[prog[i].right+1].Size()>1)
       {
        passCost = DecCost(prog[pass[prog[i].right+1][1]],prog[i]);
       }
       else passCost = math::Infinity<real32>();
      }
     }
     else passCost = math::Infinity<real32>();

     prog[i].decCost = math::Min(occCost*real32((size[0]-1-prog[i].left) + (size[1]-1-prog[i].right)),passCost)
                     + prog[i].cost;
    }
    
   // Insert this node into the pass data structure, if its good enough...
    if ((pass[prog[i].right].Size()==0)||(DecCost(prog[i])<DecCost(prog[pass[prog[i].right][0]])))
    {
     pass[prog[i].right].Push(i);
    }
  }
}

void SparseDSI::ExtractPass(nat32 level,ds::ArrayNS<Node> & prog)
{
 LogTime("eos::stereo::SparseDSI::ExtractPass");

 nat32 scanWidth = dsc->WidthLeft()>>level;
 real32 errTot = errLim * real32(scanWidth);

 // Iterate all nodes - we by definition have to make the array shorter,
 // so we just have to keep the nodes that will do. Have to do two passes
 // of each pixel to find the minimum and then to prune...
  nat32 io = 0;
  int32 lastLeft = 0xFFFFFFFF;
  real32 min = 0.0;
  for (nat32 ii=0;ii<prog.Size();ii++)
  {
   // If needed find the current pixels minimum value...
    if (lastLeft!=prog[ii].left)
    {
     lastLeft = prog[ii].left;
     min = prog[ii].incCost + prog[ii].decCost - prog[ii].cost;
     for (nat32 ji=ii+1;(ji<prog.Size())&&(prog[ji].left==lastLeft);ji++)
     {
      min = math::Min(min,prog[ji].incCost + prog[ji].decCost - prog[ji].cost);
     }
     min += errTot;
    }

   // Loop through, keeping entrys with a low enough cost...
    real32 cost = prog[ii].incCost + prog[ii].decCost - prog[ii].cost;
    if ((cost<=min)||(math::Equal(cost,min))) // math::Equal(...) required for windows. Don't ask.
    {
     prog[io] = prog[ii];
     ++io;	    
    }
  }
  prog.Size(io);
}

void SparseDSI::PrepProg(nat32 level,ds::ArrayNS<Node> & prog)
{
 LogTime("eos::stereo::SparseDSI::PrepProg");

 // Basics... 
  int32 wid[2];
   wid[0] = dsc->WidthLeft()>>level;
   wid[1] = dsc->WidthRight()>>level;

 // Declare a queue - we write our output into this...
  ds::Queue<Node> newProg;

 // Iterate all the possible pixel values, for each one iterate the relevant
 // part of the prog to extract the position range required...
  int32 base[2];
   base[0] = 0;
   base[1] = 0;

  int32 lastParent = 0;
  for (int32 x=0;x<wid[0];x++)
  {
   int32 lastPos = -1;
   int32 parent = math::Min(x>>1,(wid[0]>>1) - 1);
   if (parent!=lastParent)
   {
    base[0] = base[1];
    lastParent = parent;
   }
   
   nat32 targ = base[0];
   while ((targ<prog.Size())&&(prog[targ].left==parent))
   {
    for (int32 j=-range;j<=range;j++)
    {
     int32 pos = prog[targ].right*2 + j;
     if ((pos<=lastPos)||(pos>=wid[1])) continue;
     lastPos = pos;
    
     Node newNode;
      newNode.left = x;
      newNode.right = pos;
     newProg.Add(newNode);
    }
    ++targ;
   }
   base[1] = targ;
  }

 // Replace the old program with the new program...
  prog.Size(newProg.Size());
  for (nat32 i=0;i<prog.Size();i++)
  {
   prog[i] = newProg.Peek();
   newProg.Rem();
  }
}

void SparseDSI::Extract(ds::ArrayNS<Node> & prog,ds::Array<nat32> & index,ds::Array<DispCost> & out)
{
 LogTime("eos::stereo::SparseDSI::Extract");


 // First pass to extract index, which includes the final size...
  index[0] = 0;
  nat32 pos = 0;
  for (int32 i=0;i<int32(width);i++)
  {
   index[i+1] = index[i];
   while ((pos<prog.Size())&&(prog[pos].left==i))
   {
    index[i+1] += 1;
    ++pos;
   }   
  }
  log::Assert(index[width]==prog.Size());


 // Resize output data structure...
  out.Size(prog.Size());


 // Second pass, to fill in the returned data structure...
  for (nat32 i=0;i<prog.Size();i++)
  {
   out[i].disp = prog[i].right-prog[i].left;
   out[i].cost = (prog[i].incCost+prog[i].decCost-prog[i].cost)/real32(dsc->WidthLeft());
  }
}

void SparseDSI::HalfScanline(const Scanline & in,Scanline & out)
{
 LogTime("eos::stereo::SparseDSI::HalfScanline");
 nat32 width = in.index.Size()-1;
 nat32 newWidth = width>>1;


 // First pass to create the index and find out how big we need to make out.data...
  out.index.Size(newWidth+1);
  out.index[0] = 0;
  for (nat32 i=0;i<newWidth;i++)
  {
   nat32 size = 0;

   nat32 j1 = in.index[i*2];
   nat32 j2 = in.index[i*2+1];
   int32 prevDisp = math::min_int_32;
   while (true)
   {
    bit safe1 = j1<in.index[i*2+1];
    bit safe2 = j2<in.index[i*2+2];
    if ((safe1||safe2)==false) break;
    
    if ((!safe2)||((safe1)&&(in.data[j1].disp<(in.data[j2].disp+1))))
    {
     int32 targDisp = (i*2 + in.data[j1].disp)/2 - i;
     if (targDisp!=prevDisp)
     {
      prevDisp = targDisp;
      size += 1;
     }
    
     j1 += 1;
    }
    else
    {
     int32 targDisp = (i*2+1 + in.data[j2].disp)/2 - i;
     if (targDisp!=prevDisp)
     {
      prevDisp = targDisp;
      size += 1;
     }
    
     j2 += 1;
    }
   }

   out.index[i+1] = out.index[i] + size;
  }


 // Second pass to fill in out.data...
  out.data.Size(out.index[out.index.Size()-1]);
  for (nat32 i=0;i<newWidth;i++)
  {
   nat32 offset = out.index[i];
   bit first = true;

   nat32 j1 = in.index[i*2];
   nat32 j2 = in.index[i*2+1];
   while (true)
   {
    bit safe1 = j1<in.index[i*2+1];
    bit safe2 = j2<in.index[i*2+2];
    if ((safe1||safe2)==false) break;
    
    int32 targDisp;
    real32 targCost;
    if ((!safe2)||((safe1)&&(in.data[j1].disp<(in.data[j2].disp+1))))
    {
     targDisp = (i*2 + in.data[j1].disp)/2 - i;
     targCost = in.data[j1].cost;
     j1 += 1;
    }
    else
    {
     targDisp = (i*2+1 + in.data[j2].disp)/2 - i;
     targCost = in.data[j2].cost;
     j2 += 1;
    }
    
    if (first)
    {
     out.data[offset].disp = targDisp;
     out.data[offset].cost = targCost;
     first = false;
    }
    else
    {
     if (targDisp==out.data[offset].disp)
     {
      out.data[offset].cost = math::Min(out.data[offset].cost,targCost);
     }
     else
     {
      ++offset;
      out.data[offset].disp = targDisp;
      out.data[offset].cost = targCost;
     }
    }
   }
   log::Assert((offset+1==out.index[i+1])||(out.index[i]==out.index[i+1]),"[SparseDSI] Vertical cost synch");
  }
}

void SparseDSI::PropagateScanline(Scanline & sl) const
{
 LogTime("eos::stereo::SparseDSI::PropagateScanline");
 for (nat32 i=0;i<sl.index.Size()-1;i++)
 {
  // Forwards...
   for (nat32 j=sl.index[i]+1;j<sl.index[i+1];j++)
   {
    sl.data[j].cost = math::Min(sl.data[j].cost,
                                sl.data[j-1].cost + vertCost*math::Abs(sl.data[j].disp-sl.data[j-1].disp));
   }
  
  // Backwards...
   for (nat32 j=sl.index[i+1]-1;j>sl.index[i];j--)
   {
    sl.data[j-1].cost = math::Min(sl.data[j-1].cost,
                                  sl.data[j].cost + vertCost*math::Abs(sl.data[j].disp-sl.data[j-1].disp));
   }
 }
}

//------------------------------------------------------------------------------
DispSelect::DispSelect()
:radius(1),mult(1.0),sdsi(null<SparseDSI*>())
{}

DispSelect::~DispSelect()
{}

void DispSelect::Set(nat32 r,real32 m)
{
 radius = r;
 mult = m;
}

void DispSelect::Set(const SparseDSI & d)
{
 sdsi = &d;
}

void DispSelect::Run(time::Progress * prog)
{
 prog->Push();

 // A window of indexes, constantly needed...
  ds::Array2D<nat32> ind(radius*2+1,radius*2+1);

 // Calculate for each pixel...
  disp.Resize(sdsi->Width(),sdsi->Height());
  for (int32 y=0;y<int32(sdsi->Height());y++)
  {
   prog->Report(y,sdsi->Height());
   for (int32 x=0;x<int32(sdsi->Width());x++)
   {
    // We need to find the index of the disparity with minimum cost...
     real32 minCost = math::Infinity<real32>();
     nat32 bestInd = 0;

    // Calculate the window range to use...
     int32 minU = math::Max(x-int32(radius),int32(0)) - x + int32(radius);
     int32 maxU = math::Min(x+int32(radius),int32(sdsi->Width()-1)) - x + int32(radius);
     int32 minV = math::Max(y-int32(radius),int32(0)) - y + int32(radius);
     int32 maxV = math::Min(y+int32(radius),int32(sdsi->Height()-1)) - y + int32(radius);

    // Set the index window to 0's...
     for (int32 v=minV;v<=maxV;v++)
     {
      for (int32 u=minU;u<=maxU;u++) ind.Get(u,v) = 0;
     }

    // Iterate each disparity, calculate its cost, see if its the best...
     for (nat32 i=0;i<sdsi->Size(x,y);i++)
     {
      real32 cost = 0.0;
      int32 disparity = int32(sdsi->Disp(x,y,i));
      
      // Iterate the window, sum the costs...
       for (int32 v=minV;v<=maxV;v++)
       {
        for (int32 u=minU;u<=maxU;u++)
        {
         int32 offU = u-int32(radius);
         int32 offV = v-int32(radius);
         
         // Advance the index till we hit the end or the next node is greater 
         // than the current disparity...
          while (((ind.Get(u,v)+1)!=sdsi->Size(x+offU,y+offV))&&
                 (int32(sdsi->Disp(x+offU,y+offV,ind.Get(u,v)+1))<=disparity))
          {
           ind.Get(u,v) += 1;
          }
          
         // Sum in the relevant cost - either the minimum of the neighbour(s)+cost
         // or the direct intercept...
          int32 indDisp = int32(sdsi->Disp(x+offU,y+offV,ind.Get(u,v)));
          if (indDisp==disparity) cost += sdsi->Cost(x+offU,y+offV,ind.Get(u,v));
          else
          {
           real32 lowCost = sdsi->Cost(x+offU,y+offV,ind.Get(u,v)) + mult*math::Abs(indDisp-disparity);
           if ((indDisp<disparity)&&((ind.Get(u,v)+1)!=sdsi->Size(x+offU,y+offV)))
           {
            real32 highCost = sdsi->Cost(x+offU,y+offV,ind.Get(u,v)+1) 
                            + mult*math::Abs(int32(sdsi->Disp(x+offU,y+offV,ind.Get(u,v)+1))-disparity);
            cost += math::Min(lowCost,highCost);
           }
           else
           {
            cost += lowCost;
           }
          }
        }
       }
      
      // Check if its the best...
       if (cost<minCost)
       {
        minCost = cost;
        bestInd = i;
       }
     }
    
    // Store the relevant disparity...
     disp.Get(x,y) = sdsi->Disp(x,y,bestInd);
   }
  }
 
 prog->Pop();
}

void DispSelect::Get(svt::Field<real32> & out)
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = disp.Get(x,y);
  }
 }
}

//------------------------------------------------------------------------------
SparsePos::SparsePos(nat32 c)
:cap(c)
{}

SparsePos::~SparsePos()
{}

void SparsePos::Set(const DSI & sdsi,const cam::CameraPair & pair,time::Progress * prog)
{
 prog->Push();
 
 width = sdsi.Width();
 height = sdsi.Height();

 // Create the data structure, simultaneously filling it up with data...
  data.Size(height);
  for (nat32 y=0;y<height;y++)
  {
   prog->Report(y,height);
   data[y].index.Size(width+1);
   data[y].index[0] = 0;
   for (nat32 x=0;x<width;x++) data[y].index[x+1] = data[y].index[x] + math::Min(cap,sdsi.Size(x,y));
   
   data[y].data.Size(data[y].index[width]);
   for (nat32 x=0;x<width;x++)
   {
    for (nat32 i=0;i<math::Min(cap,sdsi.Size(x,y));i++)
    {
     Node & targ = data[y].data[data[y].index[x]+i];
     
     // Calculate the three image coordinate pairs...
      math::Vect<3,real64> li[3];
      real32 area = sdsi.DispWidth(x,y,i);
      li[0][0] = x-area; li[0][1] = y; li[0][2] = 1.0;
      li[1][0] = x;      li[1][1] = y; li[1][2] = 1.0;
      li[2][0] = x+area; li[2][1] = y; li[2][2] = 1.0;

      math::Vect<3,real64> ri[3];
      ri[0] = li[0];  ri[1] = li[1]; ri[2] = li[2];
      ri[0][0] += sdsi.Disp(x,y,i);
      ri[1][0] += sdsi.Disp(x,y,i);
      ri[2][0] += sdsi.Disp(x,y,i);

     // Unrectify...
      math::Vect<3,real64> lp[3];
      math::Vect<3,real64> rp[3];
      math::MultVect(pair.unRectLeft,li[0],lp[0]);
      math::MultVect(pair.unRectLeft,li[1],lp[1]);
      math::MultVect(pair.unRectLeft,li[2],lp[2]);
      math::MultVect(pair.unRectRight,ri[0],rp[0]);
      math::MultVect(pair.unRectRight,ri[1],rp[1]);
      math::MultVect(pair.unRectRight,ri[2],rp[2]);
     
     // Triangulate...
      math::Vect<4,real64> loc[3];
      cam::Triangulate(lp[0],rp[0],pair.lp,pair.rp,loc[0]);
      cam::Triangulate(lp[1],rp[1],pair.lp,pair.rp,loc[1]);
      cam::Triangulate(lp[2],rp[2],pair.lp,pair.rp,loc[2]);

      targ.start = loc[0];
      targ.centre = loc[1];
      targ.end = loc[2];

      targ.start /= targ.start[3];      
      targ.end   /= targ.end[3];
      targ.centre /= targ.centre[3];

     targ.cost = sdsi.Cost(x,y,i);
    }
   }
  }
 
 prog->Pop();
}

nat32 SparsePos::Width() const
{
 return width;
}

nat32 SparsePos::Height() const
{
 return height;
}

nat32 SparsePos::Size(nat32 x,nat32 y) const
{
 return data[y].index[x+1] - data[y].index[x];
}

const bs::Vertex & SparsePos::Start(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].start;
}

const bs::Vertex & SparsePos::End(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].end;
}

const bs::Vertex & SparsePos::Centre(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].centre;
}

real32 SparsePos::Cost(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].cost;
}

//------------------------------------------------------------------------------
SparsePlane::SparsePlane()
:duds(false),spos(null<SparsePos*>())
{}

SparsePlane::~SparsePlane()
{}

void SparsePlane::Set(const SparsePos & sp)
{
 spos = &sp;
}

void SparsePlane::Set(const svt::Field<nat32> & s)
{
 seg = s;
}

void SparsePlane::Duds(bit enable)
{
 duds = enable;
}

void SparsePlane::Run(time::Progress * prog)
{
 prog->Push();
 
 // Count how many segments there are...
  prog->Report(0,2);
  nat32 segCount = 1;
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
  }
 
 // Create a plane fitter for each...
  ds::ArrayDel<alg::LinePlaneFit> fit(segCount);

 // Run through and fill in the relevant data for each fitter...
  prog->Push();
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    prog->Report(y,seg.Size(1));
    for (nat32 x=0;x<seg.Size(0);x++)
    {
     nat32 s = seg.Get(x,y);
     if ((!duds)||(s!=0))
     {
      for (nat32 i=0;i<spos->Size(x,y);i++)
      {
       fit[s].Add(spos->Start(x,y,i),spos->End(x,y,i),math::Exp(-spos->Cost(x,y,i)));
      }
     }
    }
   }
  prog->Pop();

 // Run 'em all, extracting as we go...
  prog->Report(1,2);
  prog->Push();
  plane.Size(segCount);
  if (duds)
  {
   plane[0].n[0] = 0.0;
   plane[0].n[1] = 0.0;
   plane[0].n[2] = 0.0;
   plane[0].d = 1.0;
  }
   
  for (nat32 i=duds?1:0;i<fit.Size();i++)
  {
   prog->Report(i,fit.Size());
   fit[i].Run();
   plane[i] = fit[i].Plane();
   plane[i].Normalise();
  }
  prog->Pop();
 
 prog->Pop();
}

nat32 SparsePlane::Segments() const
{
 return plane.Size();
}

const bs::Plane SparsePlane::Seg(nat32 i) const
{
 return plane[i];
}

void SparsePlane::PosMap(const cam::CameraPair & pair,svt::Field<bs::Vertex> & out) const
{
 // Calculate the centre of the left camera...
  math::Vect<4,real64> centre;
  pair.lp.Centre(centre);
  if (!math::IsZero(centre[3])) centre /= centre[3];
  
 // Calculate the psuedo-inverse of the left camera matrix - has its uses...
  math::Mat<4,3,real64> invLP;
  math::PseudoInverse(pair.lp,invLP);


 // Iterate every pixel, for each calculate the ray it is on. Intercept that
 // way with the relevant plane to get the position we then store...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++)
   {
    // Calculate ray - we have the centre, we now need another point, which may
    // be provided by the psuedo inverse, after un-rectification of course...
     math::Vect<3,real64> rp;
      rp[0] = x;
      rp[1] = y;
      rp[2] = 1.0;
     
     math::Vect<3,real64> urp;
     math::MultVect(pair.unRectLeft,rp,urp);
     
     math::Vect<4,real64> to;
     math::MultVect(invLP,urp,to);
     if (!math::IsZero(to[3])) to /= to[3];
     
    
    // Intercept with plane...
     const bs::Plane & p = plane[seg.Get(x,y)];
     math::Vect<4,real64> loc;
     p.LineIntercept(centre,to,loc);
     out.Get(x,y) = loc;
   }
  }
}

void SparsePlane::DispMap(const cam::CameraPair & pair,svt::Field<real32> & out) const
{
 // Calculate the centre of the left camera...
  math::Vect<4,real64> centre;
  pair.lp.Centre(centre);
  
 // Calculate the psuedo-inverse of the left camera matrix - has its uses...
  math::Mat<4,3,real64> invLP;
  math::PseudoInverse(pair.lp,invLP);
  
 // Calculate the right rectification matrix...
  math::Mat<3,3,real64> temp;
  math::Mat<3,3,real64> rectRight = pair.unRectRight;
  math::Inverse(rectRight,temp);


 // Iterate every pixel, for each calcualte the ray it is on. Intercept that
 // way with the relevant plane to get the position, then project back and 
 // rectify it for the right view before taking the x difference to get the 
 // displarity...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++)
   {
    // Calculate ray - we have the centre, we now need another point, which may
    // be provided by the psuedo inverse, after un-rectification of course...
     math::Vect<3,real64> rl;
      rl[0] = x;
      rl[1] = y;
      rl[2] = 1.0;

     math::Vect<3,real64> url;
     math::MultVect(pair.unRectLeft,rl,url);

     math::Vect<4,real64> to;
     math::MultVect(invLP,url,to);


    // Intercept with plane...
     const bs::Plane & p = plane[seg.Get(x,y)];
     math::Vect<4,real64> loc;
     p.LineIntercept(centre,to,loc);


    // Project back into the right view...
     math::Vect<3,real64> urr;
     math::MultVect(pair.rp,loc,urr);
     
    // Rectify it...
     math::Vect<3,real64> rr;
     math::MultVect(rectRight,urr,rr);
     rr /= rr[2];
    
    // Store the difference in the x-coordinate...
     out.Get(x,y) = rr[0] - rl[0];
   }
  }
}

//------------------------------------------------------------------------------
LocalSparsePlane::LocalSparsePlane()
:duds(false),radius(3),spos(null<SparsePos*>())
{}

LocalSparsePlane::~LocalSparsePlane()
{}

void LocalSparsePlane::Set(nat32 r)
{
 radius = int32(r);
}

void LocalSparsePlane::Set(const SparsePos & sp)
{
 spos = &sp;
}

void LocalSparsePlane::Set(const svt::Field<nat32> & s)
{
 seg = s;
}

void LocalSparsePlane::Set(const cam::CameraPair & p)
{
 pair = p;
}

void LocalSparsePlane::Duds(bit enable)
{
 duds = enable;
}

void LocalSparsePlane::Run(time::Progress * prog)
{
 prog->Push();
 
 // Some do-once stuff...
  // Calculate the centre of the left camera...
   math::Vect<4,real64> centre;
   pair.lp.Centre(centre);
   if (!math::IsZero(centre[3])) centre /= centre[3];

  // Calculate the psuedo-inverse of the left camera matrix - has its uses...
   math::Mat<4,3,real64> invLP;
   math::PseudoInverse(pair.lp,invLP);


 // Iterate every single pixel, for each do a plane fitting...
  data.Resize(seg.Size(0),seg.Size(1));
  for (int32 y=0;y<int32(seg.Size(1));y++)
  {
   prog->Report(y,seg.Size(1));
   for (int32 x=0;x<int32(seg.Size(0));x++)
   {
    nat32 s = seg.Get(x,y);
    
    // Set or calculate the plane...
     bs::Plane plane;
     if (duds&&(s==0))
     {
      plane.n[0] = 0.0;
      plane.n[1] = 0.0;
      plane.n[2] = 0.0;
      plane.d = 1.0;
     }
     else
     {
      alg::LinePlaneFit lpf;
    
      // Iterate the window and add all relevant pixels...
       real32 distMult = 1.0/real32(radius+1);
       for (int32 v=math::Max<int32>(y-radius,0);v<=math::Min<int32>(y+radius,int32(seg.Size(1))-1);v++)
       {
        for (int32 u=math::Max<int32>(x-radius,0);u<=math::Min<int32>(x+radius,int32(seg.Size(0))-1);u++)
        {
         if (seg.Get(u,v)==s)
         {
          for (nat32 i=0;i<spos->Size(u,v);i++)
          {
           real32 dist = math::Sqrt(math::Sqr(u-x)+math::Sqr(v-y));
           real32 mult = math::Max(0.0,1.0-distMult*dist);
           lpf.Add(spos->Start(u,v,i),spos->End(u,v,i),mult*math::Exp(-spos->Cost(u,v,i)));
          }
         }
        }
       }
    
      // Find the plane...
       lpf.Run();
       plane = lpf.Plane();
       plane.Normalise();
     }

    // Calculate and store the position and orientation...
     // Calculate ray - we have the centre, we now need another point, which may
     // be provided by the psuedo inverse, after un-rectification of course...
      math::Vect<3,real64> rp;
       rp[0] = x;
       rp[1] = y;
       rp[2] = 1.0;
     
      math::Vect<3,real64> urp;
      math::MultVect(pair.unRectLeft,rp,urp);
     
      math::Vect<4,real64> to;
      math::MultVect(invLP,urp,to);
      if (!math::IsZero(to[3])) to /= to[3];
    
     // Intercept with plane...
      math::Vect<4,real64> loc;
      plane.LineIntercept(centre,to,loc);
      data.Get(x,y).pos = loc;
      if (!math::IsZero(data.Get(x,y).pos[3])) data.Get(x,y).pos /= data.Get(x,y).pos[3];
      
     // Orientation from plane is somewhat easier...
      data.Get(x,y).dir = plane.n;
      if (plane.n[2]<0.0) data.Get(x,y).dir *= -1.0;
   }
  }
 
 prog->Pop();
}

void LocalSparsePlane::PosMap(svt::Field<bs::Vertex> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = data.Get(x,y).pos;
  }
 }
}

void LocalSparsePlane::NeedleMap(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = data.Get(x,y).dir;
  }
 }
}

void LocalSparsePlane::DispMap(svt::Field<real32> & out) const
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
     math::MultVect(pair.rp,data.Get(x,y).pos,urr);
     
    // Rectify it...
     math::Vect<3,real64> rr;
     math::MultVect(rectRight,urr,rr);
     rr /= rr[2];
    
    // Store the difference in the x-coordinate...
     out.Get(x,y) = rr[0] - real32(x);
   }
  }
}

//------------------------------------------------------------------------------
OrientSparsePlane::OrientSparsePlane()
:duds(false),radius(3),spos(null<SparsePos*>())
{}

OrientSparsePlane::~OrientSparsePlane()
{}

void OrientSparsePlane::Set(nat32 r)
{
 radius = int32(r);
}

void OrientSparsePlane::Set(const SparsePos & sp)
{
 spos = &sp;
}

void OrientSparsePlane::Set(const svt::Field<nat32> & s)
{
 seg = s;
}

void OrientSparsePlane::Set(const svt::Field<bs::Normal> & n)
{
 needle = n;
}

void OrientSparsePlane::Set(const cam::CameraPair & p)
{
 pair = p;
}

void OrientSparsePlane::Duds(bit enable)
{
 duds = enable;
}

void OrientSparsePlane::Run(time::Progress * prog)
{
 prog->Push();
 
 // Some do-once stuff...
  // Calculate the centre of the left camera...
   math::Vect<4,real64> centre;
   pair.lp.Centre(centre);
   if (!math::IsZero(centre[3])) centre /= centre[3];

  // Calculate the psuedo-inverse of the left camera matrix - has its uses...
   math::Mat<4,3,real64> invLP;
   math::PseudoInverse(pair.lp,invLP);


 // Iterate every single pixel, for each do a plane fitting...
  data.Resize(seg.Size(0),seg.Size(1));
  for (int32 y=0;y<int32(seg.Size(1));y++)
  {
   prog->Report(y,seg.Size(1));
   for (int32 x=0;x<int32(seg.Size(0));x++)
   {
    nat32 s = seg.Get(x,y);
    
    // Set or calculate the plane...
     bs::Plane plane;
     if (duds&&(s==0))
     {
      plane.n[0] = 0.0;
      plane.n[1] = 0.0;
      plane.n[2] = 0.0;
      plane.d = 1.0;
     }
     else
     {
      alg::DepthPlane lpf;
      lpf.Set(needle.Get(x,y));
    
      // Iterate the window and add all relevant pixels...
       for (int32 v=math::Max<int32>(y-radius,0);v<=math::Min<int32>(y+radius,int32(seg.Size(1))-1);v++)
       {
        for (int32 u=math::Max<int32>(x-radius,0);u<=math::Min<int32>(x+radius,int32(seg.Size(0))-1);u++)
        {
         if (seg.Get(u,v)==s)
         {
          for (nat32 i=0;i<spos->Size(u,v);i++)
          {
           lpf.Add(spos->Centre(x,y,i));
          }
         }
        }
       }
    
      // Find the plane...
       lpf.Run();
       plane = lpf.Plane();
       plane.Normalise();
     }

    // Calculate and store the position and orientation...
     // Calculate ray - we have the centre, we now need another point, which may
     // be provided by the psuedo inverse, after un-rectification of course...
      math::Vect<3,real64> rp;
       rp[0] = x;
       rp[1] = y;
       rp[2] = 1.0;
     
      math::Vect<3,real64> urp;
      math::MultVect(pair.unRectLeft,rp,urp);
     
      math::Vect<4,real64> to;
      math::MultVect(invLP,urp,to);
      if (!math::IsZero(to[3])) to /= to[3];
    
     // Intercept with plane...
      math::Vect<4,real64> loc;
      plane.LineIntercept(centre,to,loc);
      data.Get(x,y).pos = loc;
      if (!math::IsZero(data.Get(x,y).pos[3])) data.Get(x,y).pos /= data.Get(x,y).pos[3];
      
     // Orientation from plane is somewhat easier...
      data.Get(x,y).dir = plane.n;
      if (plane.n[2]<0.0) data.Get(x,y).dir *= -1.0;
   }
  }
 
 prog->Pop();
}

void OrientSparsePlane::PosMap(svt::Field<bs::Vertex> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   //LogDebug("(" << x << "," << y << ") {old,new}" << LogDiv() << out.Get(x,y) << LogDiv() << data.Get(x,y).pos);
   out.Get(x,y) = data.Get(x,y).pos;
  }
 }
}

void OrientSparsePlane::NeedleMap(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = data.Get(x,y).dir;
  }
 }
}

void OrientSparsePlane::DispMap(svt::Field<real32> & out) const
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
     math::MultVect(pair.rp,data.Get(x,y).pos,urr);
     
    // Rectify it...
     math::Vect<3,real64> rr;
     math::MultVect(rectRight,urr,rr);
     rr /= rr[2];
    
    // Store the difference in the x-coordinate...
     out.Get(x,y) = rr[0] - real32(x);
   }
  }
}

//------------------------------------------------------------------------------
 };
};
