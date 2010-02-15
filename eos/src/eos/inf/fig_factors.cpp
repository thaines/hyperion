//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/inf/fig_factors.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
GridFreq2D::GridFreq2D(nat32 l,nat32 w,nat32 h)
:width(w),height(h),labels(l),vp(new Grid2D(l,w,h)),
ms(false),calcLevel(0),data(vp->MaxLevel()+1)
{
 LogBlock("GridFreq2D::GridFreq2D()","{labels,width,height}" << LogDiv() << l << LogDiv() << w << LogDiv() << h);
 for (nat32 i=0;i<data.Size();i++)
 {
  data[i] = mem::Malloc<real32>(vp->Vars(i)*labels);
 }
}

GridFreq2D::~GridFreq2D()
{
 for (nat32 i=0;i<data.Size();i++) mem::Free(data[i]);
 delete vp;
}

void GridFreq2D::SetFreq(nat32 x,nat32 y,nat32 label,real32 freq)
{
 data[0][(y*width+x)*labels+label] = freq;
}

void GridFreq2D::SetCost(nat32 x,nat32 y,nat32 label,real32 cost)
{
 ms = true;
 data[0][(y*width+x)*labels+label] = cost;
}

nat32 GridFreq2D::PipeCount() const
{
 return 1;
}

bit GridFreq2D::PipeIsSet(nat32) const
{
 return true;
}

const VariablePattern & GridFreq2D::PipeGet(nat32 ind) const
{
 return *vp;
}

bit GridFreq2D::PipeSet(nat32 ind,const VariablePattern & ovp)
{
 if (str::Compare(typestring(*vp),typestring(ovp))!=0) return false;
 if (vp->Paras()!=ovp.Paras()) return false;
 return true;
}

void GridFreq2D::Construct(nat32 level,FactorConstruct & fc) const
{
 // Adjust the level for overshooting...
  if (level>vp->MaxLevel()) level = vp->MaxLevel();

 
 // Check the hierachy goes deep enough, if not then deepen it...
  for (;calcLevel<level;calcLevel++)
  {
   nat32 clw   = width>>calcLevel;      if (clw==0) clw = 1;
   nat32 clwP1 = width>>(calcLevel+1);  if (clwP1==0) clwP1 = 1;
   nat32 clhP1 = height>>(calcLevel+1); if (clhP1==0) clhP1 = 1;

   real32 * out = data[calcLevel+1];
   real32 * in = data[calcLevel];
   
   for (nat32 y=0;y<clhP1;y++)
   {
    for (nat32 x=0;x<clwP1;x++)
    {
     for (nat32 l=0;l<labels;l++)
     {
      out[0] = 0.25*(in[0] + in[1] + in[clw] + in[clw+1]); // Multiplication by .25 not necesary, used to resolve potential numerical stability problems.
      out += 1;
      in += 1;
     }
     in += labels;
    }
    in += labels*clw;
   }
  }


 // Create the function to store it all...
  nat32 lWidth = width>>level;   if (lWidth==0) lWidth = 1;
  nat32 lHeight = height>>level; if (lHeight==0) lHeight = 1;
  Distribution * match = new Distribution(lWidth*lHeight,labels);


 // Copy in the parameters...
  real32 * targ = data[level];
  if (ms)
  {
   for (nat32 y=0;y<lHeight;y++)
   {
    for (nat32 x=0;x<lWidth;x++)
    {
     for (nat32 l=0;l<labels;l++)
     {
      match->SetCost(y*lWidth+x,l,*targ);
      ++targ;
     }
    }
   }
  }
  else
  {
   for (nat32 y=0;y<lHeight;y++)
   {
    for (nat32 x=0;x<lWidth;x++)
    {
     for (nat32 l=0;l<labels;l++)
     {
      match->SetFreq(y*lWidth+x,l,*targ);
      ++targ;
     }
    }
   }
  }


 // Convert from object to handle...
  nat32 hand = fc.MakeFuncs(match,lWidth*lHeight);


 // Wire it up...
  for (nat32 y=0;y<lHeight;y++)
  {
   for (nat32 x=0;x<lWidth;x++)
   {
    nat32 ind = y*lWidth+x;
    fc.Link(hand,ind,0,0,ind,0);
   }
  }
}

cstrconst GridFreq2D::TypeString() const
{
 return "eos::inf::GridFreq2D";
}
   
//------------------------------------------------------------------------------
GridJointPair2D::GridJointPair2D(nat32 l0,nat32 l1,nat32 w,nat32 h,bit mss)
:labels0(l0),labels1(l1),width(w),height(h),ms(mss),data(1)
{
 pipe[0] = new Grid2D(labels0,width,height);
 pipe[1] = new Grid2D(labels1,width,height);
 
 data[0] = new real32[width*height*labels0*labels1];
}

GridJointPair2D::~GridJointPair2D()
{
 delete pipe[0];
 delete pipe[1];
}

real32 & GridJointPair2D::Val(nat32 x,nat32 y,nat32 lab0,nat32 lab1)
{
 return data[0][labels0*(labels1*(y*width + x) + lab1) + lab0];
}

nat32 GridJointPair2D::PipeCount() const
{
 return 2;
}

bit GridJointPair2D::PipeIsSet(nat32 ind) const
{
 return true;
}

const VariablePattern & GridJointPair2D::PipeGet(nat32 ind) const
{
 return *pipe[ind];
}

bit GridJointPair2D::PipeSet(nat32 ind,const VariablePattern & vp)
{
 return *pipe[ind]==vp;
}

void GridJointPair2D::Construct(nat32 level,FactorConstruct & fc) const
{
 // Adjust level, calculate appropriate width/height...
  if (level>pipe[0]->MaxLevel()) level = pipe[0]->MaxLevel();
  nat32 lWidth = width>>level; if (lWidth==0) lWidth = 1;
  nat32 lHeight = height>>level; if (lHeight==0) lHeight = 1;


 // If we don't have the data at the correct resolution down-sample...
  if (data.Size()<=level)
  {
   nat32 i = data.Size();
   data.Size(level+1);
   for (;i<data.Size();i++)
   {
    nat32 pWidth  = width>>(i-1);  if (pWidth==0)  pWidth = 1;
    nat32 pHeight = height>>(i-1); if (pHeight==0) pHeight = 1;

    nat32 aWidth  = width>>i;  if (aWidth==0)  aWidth = 1;
    nat32 aHeight = height>>i; if (aHeight==0) aHeight = 1;

    data[i] = new real32[aWidth*aHeight*labels0*labels1];
    
    for (nat32 y=0;y<aHeight;y++)
    {
     for (nat32 x=0;x<aWidth;x++)
     {
      real32 * out = data[i] + (y*aWidth + x)*labels1*labels0;
      real32 * in[4];
       in[0] = data[i-1] + (y*2*pWidth + 2*x)*labels1*labels0;
       in[1] = data[i-1] + (y*2*pWidth + math::Min(2*x+1,pWidth-1))*labels1*labels0;
       in[2] = data[i-1] + (math::Min(y*2+1,pHeight-1)*pWidth + 2*x)*labels1*labels0;
       in[3] = data[i-1] + (math::Min(y*2+1,pHeight-1)*pWidth + math::Min(2*x+1,pWidth-1))*labels1*labels0;
      
      for (nat32 j=0;j<labels0*labels1;j++)
      {
       out[j] = 0.25*(in[0][j]+in[1][j]+in[2][j]+in[3][j]);
      }
     }
    }
   }
  }


 // Create factor, fill in details, convert to handle...
  DualDistribution * dd = new DualDistribution(lWidth*lHeight,labels0,labels1,ms);
  for (nat32 i=0;i<lWidth*lHeight;i++)
  {
   for (nat32 l0=0;l0<labels0;l0++)
   {
    for (nat32 l1=0;l1<labels1;l1++)
    {
     dd->Val(i,l0,l1) = data[level][labels0*(labels1*i + l1) + l0];
    }
   }
  }  
  nat32 hand = fc.MakeFuncs(dd,lWidth*lHeight);


 // Wire her up...
  for (nat32 y=0;y<lHeight;y++)
  {
   for (nat32 x=0;x<lWidth;x++)
   {
    fc.Link(hand,y*lWidth+x,0,0,y*lWidth+x,0);
    fc.Link(hand,y*lWidth+x,1,1,y*lWidth+x,0);
   }
  }
}

cstrconst GridJointPair2D::TypeString() const
{
 return "eos::inf::GridJointPair2D";
}

//------------------------------------------------------------------------------
GridDiff2D::GridDiff2D(bit d)
:dir(d),lambda(0.0)
{
 pipe[0] = null<Grid2D*>();
 pipe[1] = null<Grid2D*>();
}

GridDiff2D::~GridDiff2D()
{
 delete pipe[0];
 delete pipe[1];
}

void GridDiff2D::SetLambda(real32 l)
{
 lambda = l;
}

nat32 GridDiff2D::PipeCount() const
{
 return 2;
}

bit GridDiff2D::PipeIsSet(nat32 ind) const
{
 return pipe[ind]!=null<Grid2D*>();
}

const VariablePattern & GridDiff2D::PipeGet(nat32 ind) const
{
 return *pipe[ind];
}

bit GridDiff2D::PipeSet(nat32 ind,const VariablePattern & vp)
{
 // Check its an eos::inf::Grid2D - we accept nothing else...
  if (str::Compare(typestring(vp),"eos::inf::Grid2D")!=0) return false;

 // Depending on its slot set the other node, possible rejection exists for tube 1
 // as it has to have an odd number of labels greater than 1.
  switch (ind)
  {
   case 0:
   {
    delete pipe[0];
    delete pipe[1];
    
    const math::Vector<nat32> & paras = vp.Paras();
    pipe[0] = static_cast<Grid2D*>(vp.Clone());
    pipe[1] = new Grid2D(paras[0]*2-1,paras[1],paras[2]);
   }
   break;
   case 1:
   {
    if (((vp.Labels()&1)==0)||(vp.Labels()==1)) return false;
    
    delete pipe[0];
    delete pipe[1];
    
    const math::Vector<nat32> & paras = vp.Paras();
    pipe[1] = static_cast<Grid2D*>(vp.Clone());
    pipe[0] = new Grid2D((paras[0]+1)/2,paras[1],paras[2]);
   }
   break;
  }
 
 return true;
}

void GridDiff2D::Construct(nat32 level,FactorConstruct & fc) const
{
 // Adjust the level for overshooting, calculate size...
  if (level>pipe[0]->MaxLevel()) level = pipe[0]->MaxLevel();
  const math::Vector<nat32> & paras = pipe[0]->Paras();

  nat32 width = paras[1]>>level; if (width==0) width = 1;
  nat32 height = paras[2]>>level; if (height==0) height = 1;


 // Create the function, going straight to handle...
  nat32 hand = fc.MakeFuncs(new DifferencePotts(paras[0],lambda),width*height);


 // Wire it up - this is where it gets painful...
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    nat32 ind = y*width+x;

    // Pipe 0...
     if (dir)
     {
      if (y!=height-1) fc.Link(hand,ind,1,0,ind,0);
      if (y!=0)        fc.Link(hand,ind-width,2,0,ind,1);
     }
     else
     {
      if (x!=width-1) fc.Link(hand,ind,1,0,ind,0);
      if (x!=0)       fc.Link(hand,ind-1,2,0,ind,1);
     }
     
    // Pipe 1...
     fc.Link(hand,ind,0,1,ind,0);
   }
  }
}

cstrconst GridDiff2D::TypeString() const
{
 return "eos::inf::GridDiff2D";
}

//------------------------------------------------------------------------------
GridSmoothPotts2D::GridSmoothPotts2D(bit d,bit c,nat32 w,nat32 h)
:dir(d),con(c),width(w),height(h),vp(null<Grid2D*>()),calcLevel(0)
{
 if (con)
 {
  equal = 1.0;
  data.Size(0);
 }
 else
 {
  data.Size(math::Max(math::TopBit(width),math::TopBit(height)));
  
  for (nat32 i=0;i<data.Size();i++)
  {
   nat32 lWidth = width>>i;   if (lWidth==0) lWidth = 1;
   nat32 lHeight = height>>i; if (lHeight==0) lHeight = 1;
   
   data[i] = new real32[lWidth*lHeight];
  }
  
  for (nat32 i=0;i<width*height;i++)
  {
   data[0][i] = 1.0;
  }
 }
}

GridSmoothPotts2D::~GridSmoothPotts2D()
{
 delete vp;
 for (nat32 i=0;i<data.Size();i++)
 {
  delete[] data[i];
 }
}

void GridSmoothPotts2D::Set(real32 eq)
{
 equal = eq;
}

void GridSmoothPotts2D::Set(nat32 x,nat32 y,real32 eq)
{
 data[0][y*width + x] = eq;
}

nat32 GridSmoothPotts2D::PipeCount() const
{
 return 1;
}

bit GridSmoothPotts2D::PipeIsSet(nat32 ind) const
{
 return vp!=null<Grid2D*>();
}

const VariablePattern & GridSmoothPotts2D::PipeGet(nat32 ind) const
{
 return *vp;
}

bit GridSmoothPotts2D::PipeSet(nat32 ind,const VariablePattern & vpp)
{
 // Check its an eos::inf::Grid2D - we accept nothing else...
  if (str::Compare(typestring(vpp),"eos::inf::Grid2D")!=0) return false;

 // If con is false it must be the correct dimensions...
  if (con==false)
  {
   const math::Vector<nat32> & paras = vpp.Paras();
   if ((width!=paras[1])||(height!=paras[2])) return false;
  }
  
 // Tests passed - accept...
  delete vp;
  vp = static_cast<Grid2D*>(vpp.Clone());
  {
   const math::Vector<nat32> & paras = vp->Paras();
   width = paras[1];
   height = paras[2];
  }
  return true;
}

void GridSmoothPotts2D::Construct(nat32 level,FactorConstruct & fc) const
{
 // Adjust the level for overshooting...
  if (level>vp->MaxLevel()) level = vp->MaxLevel();

  nat32 lWidth = width>>level;   if (lWidth==0) lWidth = 1;
  nat32 lHeight = height>>level; if (lHeight==0) lHeight = 1;
  
  nat32 aWidth = lWidth;
  nat32 aHeight = lHeight;
  if (dir) --aHeight;
      else --aWidth;


 // If con==false then check the hierachy is deep enough...
  if (con==false)
  {
   for (;calcLevel<level;calcLevel++)
   {
    // Width at calcLevel...
     nat32 widthP0 = width>>calcLevel;   if (widthP0==0) widthP0 = 1;
     
    // Dimensions at calcLevel+1...
     nat32 widthP1 = width>>(calcLevel+1);   if (widthP1==0) widthP1 = 1;
     nat32 heightP1 = height>>(calcLevel+1); if (heightP1==0) heightP1 = 1;

    // Iterate calcLevel+1 and transfer in from the level below...
     real32 * in = data[calcLevel];
     real32 * out = data[calcLevel+1];
     for (nat32 y=0;y<heightP1;y++)
     {
      for (nat32 x=0;x<widthP1;x++)
      {
       out[0] = 0.25*(in[0] + in[1] + in[widthP0] + in[widthP0+1]);

       in += 2;
       out += 1;
      }
      in += widthP0;
     }
   }
  }


 // Create the function to store it all...
  EqualPotts * smooth = new EqualPotts(con?1:(aWidth*aHeight),vp->Labels());


 // If con==false we have to set all the instances, otherwise we simply
 // set the one instance...
  if (con)
  {
   smooth->Set(0,equal);
  }
  else
  {
   for (nat32 y=0;y<aHeight;y++)
   {
    for (nat32 x=0;x<aWidth;x++)
    {
     smooth->Set(y*aWidth+x,data[level][y*lWidth+x]);
    }
   }
  }


 // Convert from object to handle...
  nat32 hand = fc.MakeFuncs(smooth,aWidth*aHeight);
  

 // Wire it up...
  if (dir) // y direction...
  {
   for (nat32 y=0;y<lHeight;y++)
   {
    for (nat32 x=0;x<lWidth;x++)
    {
     if (y!=aHeight) fc.Link(hand,y*aWidth+x,0,0,y*lWidth+x,0);     
     if (y!=0)       fc.Link(hand,(y-1)*aWidth+x,1,0,y*lWidth+x,1);
    }
   }
  }
  else // x direction...
  {
   for (nat32 y=0;y<lHeight;y++)
   {
    for (nat32 x=0;x<lWidth;x++)
    {
     if (x!=aWidth) fc.Link(hand,y*aWidth+x,0,0,y*lWidth+x,0);
     if (x!=0)      fc.Link(hand,y*aWidth+x-1,1,0,y*lWidth+x,1);
    }
   }  
  }
}

cstrconst GridSmoothPotts2D::TypeString() const
{
 return "eos::inf::GridSmoothPotts2D";
}

//------------------------------------------------------------------------------
GridSmoothLaplace2D::GridSmoothLaplace2D(bit d,bit c,nat32 w,nat32 h)
:dir(d),con(c),width(w),height(h),vp(null<Grid2D*>()),ms(false),calcLevel(0)
{
 if (!con)
 {
  data.Size(math::Max(math::TopBit(width),math::TopBit(height)));
  
  for (nat32 i=0;i<data.Size();i++)
  {
   nat32 lWidth = width>>i;   if (lWidth==0) lWidth = 1;
   nat32 lHeight = height>>i; if (lHeight==0) lHeight = 1;
   
   data[i] = new Para[lWidth*lHeight];
  }
 }
}

GridSmoothLaplace2D::~GridSmoothLaplace2D()
{
 delete vp;
 for (nat32 i=0;i<data.Size();i++)
 {
  delete[] data[i];
 }
}

void GridSmoothLaplace2D::Set(real32 corruption,real32 sd)
{
 para.corruption = corruption;
 para.sd = sd;
}

void GridSmoothLaplace2D::SetMS(real32 mult,real32 max)
{
 ms = true;
 para.corruption = max;
 para.sd = mult;
}

void GridSmoothLaplace2D::Set(nat32 x,nat32 y,real32 corruption,real32 sd)
{
 nat32 ind = y*width + x;
 
 data[0][ind].corruption = corruption;
 data[0][ind].sd = sd;
}

void GridSmoothLaplace2D::SetMS(nat32 x,nat32 y,real32 mult,real32 max)
{
 ms = true;
 nat32 ind = y*width + x;
 
 data[0][ind].corruption = max;
 data[0][ind].sd = mult;
}

nat32 GridSmoothLaplace2D::PipeCount() const
{
 return 1;
}

bit GridSmoothLaplace2D::PipeIsSet(nat32 ind) const
{
 return vp!=null<Grid2D*>();
}

const VariablePattern & GridSmoothLaplace2D::PipeGet(nat32 ind) const
{
 return *vp;
}

bit GridSmoothLaplace2D::PipeSet(nat32 ind,const VariablePattern & vpp)
{
 // Check its an eos::inf::Grid2D - we accept nothing else...
  if (str::Compare(typestring(vpp),"eos::inf::Grid2D")!=0) return false;

 // If con is false it must be the correct dimensions...
  if (con==false)
  {
   const math::Vector<nat32> & paras = vpp.Paras();
   if ((width!=paras[1])||(height!=paras[2])) return false;
  }
  
 // Tests passed - accept...
  delete vp;
  vp = static_cast<Grid2D*>(vpp.Clone());
  {
   const math::Vector<nat32> & paras = vp->Paras();
   width = paras[1];
   height = paras[2];
  }
  return true;
}

void GridSmoothLaplace2D::Construct(nat32 level,FactorConstruct & fc) const
{
 // Adjust the level for overshooting...
  if (level>vp->MaxLevel()) level = vp->MaxLevel();

  nat32 lWidth = width>>level;   if (lWidth==0) lWidth = 1;
  nat32 lHeight = height>>level; if (lHeight==0) lHeight = 1;
  
  nat32 aWidth = lWidth;
  nat32 aHeight = lHeight;
  if (dir) --aHeight;
      else --aWidth;


 // If con==false then check the hierachy is deep enough...
  if (con==false)
  {
   for (;calcLevel<level;calcLevel++)
   {
    // Width at calcLevel...
     nat32 widthP0 = width>>calcLevel;   if (widthP0==0) widthP0 = 1;
     
    // Dimensions at calcLevel+1...
     nat32 widthP1 = width>>(calcLevel+1);   if (widthP1==0) widthP1 = 1;
     nat32 heightP1 = height>>(calcLevel+1); if (heightP1==0) heightP1 = 1;

    // Iterate calcLevel+1 and transfer in from the level below...
     Para * in = data[calcLevel];
     Para * out = data[calcLevel+1];
     for (nat32 y=0;y<heightP1;y++)
     {
      for (nat32 x=0;x<widthP1;x++)
      {
       out->corruption = 0.25*(in[0].corruption + in[1].corruption + 
                               in[widthP0].corruption + in[widthP0+1].corruption);
       out->sd = 0.25*(in[0].sd + in[1].sd + 
                       in[widthP0].sd + in[widthP0+1].sd);

       in += 2;
       out += 1;
      }
      in += widthP0;
     }
   }
  }


 // Create the function to store it all...
  EqualLaplace * smooth = new EqualLaplace(con?1:(aWidth*aHeight),vp->Labels());


 // If con==false we have to set all the instances, otherwise we simply
 // set the one instance...
  if (ms)
  {
   if (con)
   {
    smooth->SetMS(0,para.sd,para.corruption);
   }
   else
   {
    for (nat32 y=0;y<aHeight;y++)
    {
     for (nat32 x=0;x<aWidth;x++)
     {
      Para & targ = data[level][y*lWidth+x];
      smooth->SetMS(y*aWidth+x,targ.sd,targ.corruption);
     }
    }
   }
  }
  else
  {
   if (con)
   {
    smooth->Set(0,para.corruption,para.sd);
   }
   else
   {
    for (nat32 y=0;y<aHeight;y++)
    {
     for (nat32 x=0;x<aWidth;x++)
     {
      Para & targ = data[level][y*lWidth+x];
      smooth->Set(y*aWidth+x,targ.corruption,targ.sd);
     }
    }
   }
  }


 // Convert from object to handle...
  nat32 hand = fc.MakeFuncs(smooth,aWidth*aHeight);
  

 // Wire it up...
  if (dir) // y direction...
  {
   for (nat32 y=0;y<lHeight;y++)
   {
    for (nat32 x=0;x<lWidth;x++)
    {
     if (y!=aHeight) fc.Link(hand,x*aHeight+y,0,0,y*lWidth+x,0);     
     if (y!=0)       fc.Link(hand,x*aHeight+y-1,1,0,y*lWidth+x,1);
    }
   }
  }
  else // x direction...
  {
   for (nat32 y=0;y<lHeight;y++)
   {
    for (nat32 x=0;x<lWidth;x++)
    {
     if (x!=aWidth) fc.Link(hand,y*aWidth+x,0,0,y*lWidth+x,0);
     if (x!=0)      fc.Link(hand,y*aWidth+x-1,1,0,y*lWidth+x,1);
    }
   }  
  }
}

cstrconst GridSmoothLaplace2D::TypeString() const
{
 return "eos::inf::GridSmoothLaplace2D";
}

//------------------------------------------------------------------------------
GridSmoothVonMises2D::GridSmoothVonMises2D()
:k(1.0),cutMult(0.0),vp(null<Grid2D*>())
{}

GridSmoothVonMises2D::~GridSmoothVonMises2D()
{
 delete vp;
}

void GridSmoothVonMises2D::Set(real32 kk,real32 cutM)
{
 k = kk;
 cutMult = cutM;
}

nat32 GridSmoothVonMises2D::PipeCount() const
{
 return 1;
}

bit GridSmoothVonMises2D::PipeIsSet(nat32 ind) const
{
 return vp!=null<Grid2D*>();
}

const VariablePattern & GridSmoothVonMises2D::PipeGet(nat32 ind) const
{
 return *vp;
}

bit GridSmoothVonMises2D::PipeSet(nat32 ind,const VariablePattern & vpp)
{
 if (vp)
 {
  return vpp==*vp;
 }
 else
 {
  if (str::Compare(typestring(vpp),"eos::inf::Grid2D")!=0) return false;
  vp = static_cast<Grid2D*>(vpp.Clone());
  return true;
 }
}

void GridSmoothVonMises2D::Construct(nat32 level,FactorConstruct & fc) const
{
 // Get the relevant pipe details, factor in level...
  const math::Vector<nat32> & paras = vp->Paras();
  nat32 labels = paras[0];
  nat32 width = paras[1] >> level;  if (width==0) width = 1;
  nat32 height = paras[2] >> level; if (height==0) height = 1;

 // Create the factors, turn them into a handle...
  EqualVonMises * von = new EqualVonMises(1,labels);
  von->Set(0,k,cutMult);
  nat32 hand = fc.MakeFuncs(von,(width-1)*height + width*(height-1));

 // Wire it up...
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    nat32 ind = y*width+x;

    if (x!=width-1) fc.Link(hand,y*(width-1)+x,0,0,ind,0);
    if (x!=0) fc.Link(hand,y*(width-1)+x-1,1,0,ind,1);

    if (y!=height-1) fc.Link(hand,(width-1)*height + x*(height-1)+y,0,0,ind,0);
    if (y!=0) fc.Link(hand,(width-1)*height + x*(height-1)+y-1,1,0,ind,1);
   }
  }
}

cstrconst GridSmoothVonMises2D::TypeString() const
{
 return "eos::inf::GridSmoothVonMises2D";
}
 
//------------------------------------------------------------------------------
 };
};
