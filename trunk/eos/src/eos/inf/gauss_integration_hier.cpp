//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/inf/gauss_integration_hier.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
HierIntegrateBP::HierIntegrateBP(nat32 width,nat32 height)
:maxLevels(32),iters(8)
{
 Reset(width,height);
}

HierIntegrateBP::~HierIntegrateBP()
{}

void HierIntegrateBP::Reset(nat32 width,nat32 height)
{
 LogTime("eos::inf::HierIntegrateBP::Reset");
 in.Size(height);
 for (nat32 i=0;i<height;i++) in[i].Size(width);
 out.Resize(width,height);

 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   in[y][x].lock = false;
   in[y][x].mean = 0.0;
   in[y][x].invSd = 0.0;
   for (nat32 i=0;i<4;i++)
   {
    in[y][x].rel[i].m = 1.0;
    in[y][x].rel[i].z = 0.0;
    in[y][x].rel[i].invSd = 0.0;
   }

   out.Get(x,y) = math::Gauss1D();
  }
 }
}

nat32 HierIntegrateBP::Width() const
{
 return out.Width();
}

nat32 HierIntegrateBP::Height() const
{
 return out.Height();
}

void HierIntegrateBP::SetVal(nat32 x,nat32 y,real32 mean,real32 invSd)
{
 in[y][x].lock = false;
 in[y][x].mean = mean;
 in[y][x].invSd = invSd;
}

void HierIntegrateBP::SetLocked(nat32 x,nat32 y,real32 val)
{
 in[y][x].lock = true;
 in[y][x].mean = val;
}

void HierIntegrateBP::SetRel(nat32 x,nat32 y,nat32 dir,real32 m,real32 z,real32 invSd)
{
 in[y][x].rel[dir].m = m;
 in[y][x].rel[dir].z = z;
 in[y][x].rel[dir].invSd = invSd;
}

void HierIntegrateBP::SetIters(nat32 i,nat32 ml)
{
 iters = i;
 maxLevels = ml;
}

void HierIntegrateBP::Run(time::Progress * prog)
{
 LogDebug("eos::inf::HierIntegrateBP::Run");
 prog->Push();

 // Allocate the hierachy containning the input data and the message passing data...
  nat32 levels = math::Min(math::TopBit(math::Max(in[0].Size(),in.Size())),maxLevels);
  
  ds::Array<ds::ArrayDel< ds::Array<Node> > > data(levels);
  for (nat32 l=0;l<levels;l++)
  {
   nat32 width,height;
   if (l!=0)
   {
    width = data[l-1][0].Size(); width = width/2 + (((width%2)==1)?1:0);
    height = data[l-1].Size(); height = height/2 + (((height%2)==1)?1:0);
   }
   else
   {
    width = in[0].Size();
    height = in.Size();
   }
   
   data[l].Size(height);
   for (nat32 y=0;y<height;y++) data[l][y].Size(width);
  }



 // Fill in the bottom of the hierachy...
  for (nat32 y=0;y<data[0].Size();y++)
  {
   for (nat32 x=0;x<data[0][y].Size();x++)
   {
    Node & dest = data[0][y][x];
    Pixel & source = in[y][x];
    
    dest.pix = source;
    if (in[y][x].lock==false)
    {
     dest.exp.InvCoVar() = math::Sqr(dest.pix.invSd);
     dest.exp.InvCoVarMean() = math::Sqr(dest.pix.invSd) * dest.pix.mean;
    }
    else dest.exp = math::Gauss1D();
    
    for (nat32 d=0;d<4;d++) dest.msg[d] = math::Gauss1D();
   }
  }



 // Clever little trick - go through all locked nodes and arrange that they
 // don't pass messages, and that their neighbours don't pass messages to them,
 // then set there sent messages correctly from the locked nodes - we then no 
 // longer have to worry about them and they can be handled by the hierachy 
 // without problem...
  for (nat32 y=0;y<data[0].Size();y++)
  {
   for (nat32 x=0;x<data[0][y].Size();x++)
   {
    Node & targ = data[0][y][x];
    if (targ.pix.lock)
    {
     // Arrange that this node never sends messages...
      for (nat32 d=0;d<4;d++) targ.pix.rel[d].invSd = -1.0;
    
     // Arrange that the neighbours don't send messages to this node, and fill
     // in there messages from this node...
      for (nat32 d=0;d<4;d++)
      {
       int32 u = x;
       int32 v = y;
       switch (d) // Done for (d+2)%4 rather than d - makes d the message position written.
       {
        case 0: u--; break;
        case 1: v--; break;
        case 2: u++; break;
        case 3: v++; break;
       }
       if ((u<0)||(u>=int32(data[0][y].Size()))||(v<0)||(v>=int32(data[0].Size()))) continue;
       
       Pixel::Rel & rel = targ.pix.rel[(d+2)%4];
       if (rel.invSd>0.0)
       {
        Node & nt = data[0][v][u];
        nt.msg[d].InvCoVar() = math::Sqr(rel.invSd);
        nt.msg[d].InvCoVarMean() = nt.msg[d].InvCoVar() * (targ.pix.mean*rel.m + rel.z);
       }
      }
    }
   }
  }



 // Fill in all further levels of the hierachy, each from its lower level, plus
 // finish off the bottom level...
  for (nat32 l=0;l<levels;l++)
  {
   // Copy the level below to this level...
    if (l!=0)
    {
     for (nat32 y=0;y<data[l].Size();y++)
     {
      for (nat32 x=0;x<data[l][y].Size();x++)
      {
       Node & targ = data[l][y][x];

       // Calculate some basic details in regards to the parent nodes...
        nat32 fromX = x*2;
        nat32 fromY = y*2;
        bit safeX = fromX+1 < data[l-1][y].Size();
        bit safeY = fromY+1 < data[l-1].Size();
        
       // Work out in which directions we will be sending messages - don't 
       // bother for such directions, other than to mark our not bothering...
        real32 div = 1.0;
        bit send[4];
        for (nat32 d=0;d<4;d++) send[d] = true;
        
        for (nat32 d=0;d<4;d++) send[d] &= data[l-1][fromX][fromY].pix.rel[d].invSd>0.0;
        if (safeX)
        {
         for (nat32 d=0;d<4;d++) send[d] &= data[l-1][fromX+1][fromY].pix.rel[d].invSd>0.0;
         div += 1.0;
        }
        if (safeY)
        {
         for (nat32 d=0;d<4;d++) send[d] &= data[l-1][fromX][fromY+1].pix.rel[d].invSd>0.0;
         div += 1.0;
        }
        if (safeX&&safeY)
        {
         for (nat32 d=0;d<4;d++) send[d] &= data[l-1][fromX+1][fromY+1].pix.rel[d].invSd>0.0;
         div += 1.0;
        }
       
       // Fill in this node using the parent nodes - this gets quite complicated...
        /*targ.exp = data[l-1][fromX][fromY].exp;
        if (safeX) targ.exp *= data[l-1][fromX+1][fromY].exp;
        if (safeY) targ.exp *= data[l-1][fromX][fromY+1].exp;
        if (safeX&&safeY) targ.exp *= data[l-1][fromX+1][fromY+1].exp;
        targ.exp /= div;
        
        for (nat32 d=0;d<4;d++)
        {
         if (send[d])
         {
          targ.pix.rel[d].m =
          targ.pix.rel[d].z =
          targ.pix.rel[d].invSd =
         }
         targ.msg[d] = data[l-1][fromX][fromY].msg[d];
         if (safeX) targ.msg[d] *= data[l-1][fromX+1][fromY].msg[d];
         if (safeY) targ.msg[d] *= data[l-1][fromX][fromY+1].msg[d];
         if (safeX&&safeY) targ.msg[d] *= data[l-1][fromX+1][fromY+1].msg[d];
         targ.msg[d] /= div;
        }*/
      }
     }
    }
    
    
   // Add boundary constraints, to avoid memory damage...
    
   
   // Remove bias from the relationships between pixels...
   
  
  }



 // Pass messages for each level of the hierachy in turn, before copying down to
 // the lower level as applicable...
 
 
 // Extract the final result...
 
 
 
 
 
 // Create and null the message passing data structure...
 // It also contains the input messages, so fill them in.
  /*prog->Report(0,iters+2);
  ds::ArrayDel< ds::Array<Node> > data(out.Height());
  for (nat32 i=0;i<data.Size();i++) data[i].Size(out.Width());

  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    // Expected value...
     if (in[y][x].lock==false)
     {
      data[y][x].exp.InvCoVar() = math::Sqr(in[y][x].invSd);
      data[y][x].exp.InvCoVarMean() = math::Sqr(in[y][x].invSd) * in[y][x].mean;
     }

    // Messages...
     for (nat32 i=0;i<4;i++) data[y][x].msg[i] = math::Gauss1D();
   }
  }


 // Update the input so that all off-grid relationships are set to invSd==0,
 // such that such memory damaging messages are not sent...
  for (nat32 y=0;y<out.Height();y++)
  {
   in[y][0].rel[2].invSd = 0.0;
   in[y][out.Width()-1].rel[0].invSd = 0.0;
  }

  for (nat32 x=0;x<out.Width();x++)
  {
   in[0][x].rel[3].invSd = 0.0;
   in[out.Height()-1][x].rel[1].invSd = 0.0;
  }


 // Go through and create an adjusted set of relationships between pixels to
 // remove bias from the results...
  prog->Report(1,iters+3);
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    // Horizontal...
     if ((x+1)!=out.Width())
     {
      if (!(math::IsZero(in[y][x].rel[0].invSd)&&math::IsZero(in[y][x+1].rel[2].invSd)))
      {
       real32 invA = math::Sqr(in[y][x].rel[0].invSd);
       real32 invB = math::Sqr(in[y][x+1].rel[2].invSd);
       real32 div = 1.0/(invA+invB);

       in[y][x].rel[0].ubM = (invA*in[y][x].rel[0].m + invB/in[y][x+1].rel[2].m) * div;
       in[y][x].rel[0].ubZ = (invA*in[y][x].rel[0].z - (invB*in[y][x+1].rel[2].z/in[y][x+1].rel[2].m)) * div;

       in[y][x+1].rel[2].ubM = (invB*in[y][x+1].rel[2].m + invA/in[y][x].rel[0].m) * div;
       in[y][x+1].rel[2].ubZ = (invB*in[y][x+1].rel[2].z - (invA*in[y][x].rel[0].z/in[y][x].rel[0].m)) * div;
      }
     }

    // Vertical...
     if ((y+1)!=out.Height())
     {
      if (!(math::IsZero(in[y][x].rel[1].invSd)&&math::IsZero(in[y+1][x].rel[3].invSd)))
      {
       real32 invA = math::Sqr(in[y][x].rel[1].invSd);
       real32 invB = math::Sqr(in[y+1][x].rel[3].invSd);
       real32 div = 1.0/(invA+invB);

       in[y][x].rel[1].ubM = (invA*in[y][x].rel[1].m + invB/in[y+1][x].rel[3].m) * div;
       in[y][x].rel[1].ubZ = (invA*in[y][x].rel[1].z - (invB*in[y+1][x].rel[3].z/in[y+1][x].rel[3].m)) * div;

       in[y+1][x].rel[3].ubM = (invB*in[y+1][x].rel[3].m + invA/in[y][x].rel[1].m) * div;
       in[y+1][x].rel[3].ubZ = (invB*in[y+1][x].rel[3].z - (invA*in[y][x].rel[1].z/in[y][x].rel[1].m)) * div;
      }
     }
   }
  }


 // Pass messages (With a checkboard pattern.)...
  for (nat32 i=0;i<iters;i++)
  {
   prog->Report(2+i,iters+3);
   for (nat32 y=0;y<out.Height();y++)
   {
    for (nat32 x=(i+y)%2;x<out.Width();x+=2)
    {
     if (in[y][x].lock)
     {
      // Message calculation is far easier for locked nodes, simply use the
      // relationship to calculate the absolute value and then apply the invSd to
      // get the message...
       for (nat32 j=0;j<4;j++)
       {
        if (!math::IsZero(in[y][x].rel[j].invSd))
        {
         // Calculate message...
          real32 invVar = math::Sqr(in[y][x].rel[j].invSd);
          real32 val = in[y][x].mean*in[y][x].rel[j].ubM + in[y][x].rel[j].ubZ;

          math::Gauss1D msg;
          msg.InvCoVar() = invVar;
          msg.InvCoVarMean() = invVar * val;

         // Send message...
          switch (j)
          {
           case 0: data[y][x+1].msg[2] = msg; break;
           case 1: data[y+1][x].msg[3] = msg; break;
           case 2: data[y][x-1].msg[0] = msg; break;
           case 3: data[y-1][x].msg[1] = msg; break;
          }
        }
       }
     }
     else
     {
      // Calculate the accumulator of messages, from which we can subtract to get
      // the outputs...
       math::Gauss1D acc = data[y][x].exp;
       for (nat32 j=0;j<4;j++) acc *= data[y][x].msg[j];

      // Pass messages...
       for (nat32 j=0;j<4;j++)
       {
        if (!math::IsZero(in[y][x].rel[j].invSd))
        {
         // Remove the message from the direction we are sending from the
         // expectation...
          math::Gauss1D accSub = acc;
          accSub /= data[y][x].msg[j];

         // Combine the acc with the compatability distribution and marginalise
         // to generate our output message...
          math::Gauss2D msgExt(accSub);

          real32 m = in[y][x].rel[j].ubM;
          real32 z = in[y][x].rel[j].ubZ;
          real32 mult = 0.5 * math::Sqr(in[y][x].rel[j].invSd);

          math::Gauss2D co;
          co.InvCoVar()[0][0] = mult * math::Sqr(m);
          co.InvCoVar()[0][1] = -mult * m;
          co.InvCoVar()[1][0] = -mult * m;
          co.InvCoVar()[1][1] = mult;
          co.InvCoVarMean()[1] = -mult * z * m;
          co.InvCoVarMean()[0] =  mult * z;

          msgExt *= co;
          math::Gauss1D msg;
          msgExt.Marg1(msg);

         // Send message...
          switch (j)
          {
           case 0: data[y][x+1].msg[2] = msg; break;
           case 1: data[y+1][x].msg[3] = msg; break;
           case 2: data[y][x-1].msg[0] = msg; break;
           case 3: data[y-1][x].msg[1] = msg; break;
          }
        }
       }
     }
    }
   }
  }


 // Extract the results...
  prog->Report(2+iters,iters+3);
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    if (!in[y][x].lock)
    {
     out.Get(x,y) = data[y][x].exp;
     for (nat32 i=0;i<4;i++) out.Get(x,y) *= data[y][x].msg[i];
    }
   }
  }
*/

 prog->Pop();
}

bit HierIntegrateBP::Defined(nat32 x,nat32 y) const
{
 if (in[y][x].lock) return true;
               else return out.Get(x,y).Defined();
}

real32 HierIntegrateBP::Expectation(nat32 x,nat32 y) const
{
 if (in[y][x].lock) return in[y][x].mean;
               else return out.Get(x,y).Mean();
}

real32 HierIntegrateBP::StandardDeviation(nat32 x,nat32 y) const
{
 if (in[y][x].lock) return 0.0;
               else return out.Get(x,y).Sd();
}

void HierIntegrateBP::GetDefined(svt::Field<bit> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = Defined(x,y);
  }
 }
}

void HierIntegrateBP::GetExpectation(svt::Field<real32> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = Expectation(x,y);
  }
 }
}

void HierIntegrateBP::GetDeviation(svt::Field<real32> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = StandardDeviation(x,y);
  }
 }
}

//------------------------------------------------------------------------------
 };
};
