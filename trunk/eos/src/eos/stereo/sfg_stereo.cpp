//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/sfg_stereo.h"

#include "eos/math/functions.h"
#include "eos/math/gaussian_mix.h"
#include "eos/ds/arrays2d.h"
#include "eos/file/csv.h"
#include "eos/inf/fig_variables.h"
#include "eos/inf/fig_factors.h"
#include "eos/filter/kernel.h"


namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
DisparityOrient::DisparityOrient()
:labels(0),depth(null<real32*>()),hon(0),lt(null<Inst*>())
{
 for (nat32 i=0;i<4;i++) dist[i] = null<real32*>();
}

DisparityOrient::~DisparityOrient()
{
 for (nat32 i=0;i<4;i++) delete[] dist[i];
 delete[] lt;
 delete[] depth;
}

void DisparityOrient::Set(real32 maxC,real32 angM,nat32 l,real32 * d,const alg::HemiOfNorm & h)
{
 maxCost = maxC;
 angMult = angM/h.Spacing();

 labels = l;
 log::Assert(labels<0xFFFF);
 
 delete[] depth;
 depth = new real32[labels];
 for (nat32 i=0;i<labels;i++) depth[i] = d[i];

 hon = h;
 log::Assert(hon.Norms()<0xFFFF);
 LogDebug("[DisparityOrient] Set {maxCost,angMult,labels,hon.Norms()}" << LogDiv()
          << maxCost << LogDiv() << angMult << LogDiv()
          << labels << LogDiv() << hon.Norms());
}

nat32 DisparityOrient::Links() const
{
 return 4;
}

void DisparityOrient::LinkType(nat32 ind,inf::MessageClass & out) const
{
 if (ind==3) inf::Frequency::MakeClass(hon.Norms(),out);
        else inf::Frequency::MakeClass(labels,out);
}

void DisparityOrient::ToSP()
{
 log::Assert(false,"sum-product not supported by DisparityOrient");
}

void DisparityOrient::SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{}

void DisparityOrient::SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{}

void DisparityOrient::SendAllSP(nat32 instance,const inf::MessageSet & ms)
{}

void DisparityOrient::ToMS()
{
 LogBlock("void DisparityOrient::ToMS()","-");
 // Hijacked for the purpose of creating the lookup tables, this is rather slow
 // as the lookup tables are rather complex in design, not to mention 
 // converted into an interpreted single instruction 'set' to accelerate 
 // runtime...
  // First build a linked list of all costs less than maxCost, this involves 
  // iterating every single combination and is painfully slow,
  // O(labels^3 * hon.Norms()) to be precise. Latter steps are not so bad as
  // once this list exists we can iterate it rather than the entire parameter
  // space. The fact that the output of this proccess is sorted is important...
   ds::List<Node> uoi;
   for (nat32 d0=0;d0<labels;d0++)
   {
    real32 bd = depth[d0];
    for (nat32 d1=0;d1<labels;d1++)
    {
     real32 dx = depth[d1] - bd;
     for (nat32 d2=0;d2<labels;d2++)
     {
      real32 dy = depth[d2] - bd;

      bs::Normal norm;
       norm[0] = -dx;
       norm[1] = -dy;
       norm[2] = 1.0;
      norm *= math::InvSqrt(math::Sqr(dx)+math::Sqr(dy)+1.0);
      
      for (nat32 o3=0;o3<hon.Norms();o3++)
      {
       real32 ang = math::InvCos(norm * hon.Norm(o3));
       real32 cost = ang*angMult;
       if (cost<maxCost)
       {
        Node nn;
         nn.cost = cost;
         nn.index[0] = nat16(d0); for (nat32 i=0;i<4;i++)
         nn.index[1] = nat16(d1);
         nn.index[2] = nat16(d2);
         nn.index[3] = nat16(o3);
        uoi.AddBack(nn);
       }
      }
     }
    }
   }
   LogDebug("[DisparityOrient] Created unsorted instruction list {size,outOf}" << LogDiv()
            << uoi.Size() << LogDiv() << labels*labels*labels*hon.Norms());


  // Construct the data structure to store the output program in...
   ltSize = uoi.Size();
   lt = new Inst[ltSize];


  // Marginal distribution storage...
   for (nat32 i=0;i<3;i++)
   {
    dist[i] = new real32[labels];
    for (nat32 j=0;j<labels;j++) dist[i][j] = maxCost;
   }

   dist[3] = new real32[hon.Norms()];
   for (nat32 i=0;i<hon.Norms();i++) dist[3][i] = maxCost;


  // Fill it up, simultaneously extracting 'marginal distributions'...
   for (nat32 i=0;i<ltSize;i++)
   {
    Node & targ = uoi.Front();
     lt[i].cost = targ.cost;
     for (nat32 j=0;j<4;j++)
     {
      nat8 ind = targ.index[j];
      lt[i].index[j] = ind;
      dist[j][ind] = math::Min(dist[j][ind],targ.cost);
     }
    uoi.RemFrontKill();
   }
   

  // Optimise the unwind values, to maximise execution speed, 
  // also compensate for bias...
   for (nat32 i=0;i<4;i++) lt[0].unwind[i] = 3;

   for (nat32 i=1;i<ltSize;i++)
   {
    bit check[5];
    for (nat32 j=0;j<4;j++) check[j] = lt[i-1].index[j]!=lt[i].index[j];
    check[4] = true;

    for (nat32 j=0;j<4;j++)
    {
     lt[i].unwind[j] = 3;
     nat32 ind = 0;
     while (true)
     {
      if (ind==j) ++ind;
      if (check[ind]) break;
      lt[i].unwind[j] -= 1;
      ++ind;
     }
    }
    
    // Bias correction, I should probably work out how this works...
     lt[i].cost -= math::Max(dist[0][lt[i].index[0]],
                             dist[1][lt[i].index[1]],
                             dist[2][lt[i].index[2]],
                             dist[3][lt[i].index[3]]);
   }
}

void DisparityOrient::SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{
 log::Assert(false,"non-loopy not supported by DisparityOrient");
}

void DisparityOrient::SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{
 log::Assert(false,"non-loopy not supported by DisparityOrient");
}

void DisparityOrient::SendAllMS(nat32 instance,const inf::MessageSet & ms)
{
 // Extract the 4 in's and 4 out's...
  inf::Frequency in[4];
  inf::Frequency out[4];
  for (nat32 i=0;i<4;i++)
  {
   in[i] = ms.GetIn<inf::Frequency>(i);
   out[i] = ms.GetOut<inf::Frequency>(i); 
  }


 // Calculate the maximal value that any cost may take...
  real32 highCost = maxCost;
  for (nat32 i=0;i<4;i++)
  {
   real32 min = in[i][0];
   for (nat32 j=1;j<in[i].Labels();j++) min = math::Min(min,in[i][j]);
   highCost += min;
  }


 // Set all outputs to the maximal value...
  for (nat32 i=0;i<4;i++)
  {
   for (nat32 j=0;j<out[i].Labels();j++) out[i][j] = highCost;
  }


 // For each of the 4 outputs run the relevent 'program' to calculate the 
 // output...
  const nat32 uia[4][3] = {{1,2,3},{0,2,3},{0,1,3},{0,1,2}};
  real32 costStack[3];
  for (nat32 i=0;i<4;i++)
  {
   Inst * targ = lt;
   for (nat32 j=0;j<ltSize;j++)
   {
    // Update costStack, with minimal calculation...
     switch (targ->unwind[i])
     {
      case 3: costStack[0] = in[uia[i][0]][targ->index[uia[i][0]]];
      case 2: costStack[1] = costStack[0] + in[uia[i][1]][targ->index[uia[i][1]]];
      case 1: costStack[2] = costStack[1] + in[uia[i][2]][targ->index[uia[i][2]]];
      default: break;
     }
     
    // Calculate final cost, onwards only if its low enough...
     real32 cost = costStack[2] + targ->cost;
     if (cost<highCost)
     {
      // Update the relevant output...
       real32 & to = out[i][targ->index[i]];
       to = math::Min(to,cost);
     }
     
    // Onwards!..
     ++targ;
   }

   out[i].Drop();
  }
}

cstrconst DisparityOrient::TypeString() const
{
 return "eos::stereo::DisparityOrient";
}

//------------------------------------------------------------------------------
GridDisparityOrient2D::GridDisparityOrient2D()
:depth(null<real32*>()),hon(0)
{
 pipe[0] = null<inf::Grid2D*>();
 pipe[1] = null<inf::Grid2D*>();
}

GridDisparityOrient2D::~GridDisparityOrient2D()
{
 delete[] depth;
 delete pipe[0];
 delete pipe[1];
}

void GridDisparityOrient2D::Set(real32 maxC,real32 angM,
                                const cam::DispConv & dc,int32 minDisp,int32 maxDisp,
                                const alg::HemiOfNorm & h)
{
 maxCost = maxC;
 angMult = angM;

 labels = maxDisp-minDisp+1;
 delete[] depth;
 depth = new real32[labels];
 for (nat32 i=0;i<labels;i++)
 {
  depth[i] = dc.DispToDepth(0.0,0.0,real32(i)+real32(minDisp));
 }

 hon = h;
}

nat32 GridDisparityOrient2D::PipeCount() const
{
 return 2;
}

bit GridDisparityOrient2D::PipeIsSet(nat32 ind) const
{
 return pipe[ind]!=null<inf::Grid2D*>();
}

const inf::VariablePattern & GridDisparityOrient2D::PipeGet(nat32 ind) const
{
 return *pipe[ind];
}

bit GridDisparityOrient2D::PipeSet(nat32 ind,const inf::VariablePattern & vp)
{
 if (str::Compare(typestring(vp),"eos::inf::Grid2D")!=0) return false;
 if (pipe[ind])
 {
  if (*pipe[ind]==vp) return true;
                 else return false;
 }

 switch (ind)
 {
  case 0:
  {
   const math::Vector<nat32> & paras = vp.Paras();
   if (paras[0]!=labels) return false;

   delete pipe[0];
   delete pipe[1];
   
   pipe[0] = static_cast<inf::Grid2D*>(vp.Clone());
   pipe[1] = new inf::Grid2D(hon.Norms(),paras[1],paras[2]);
  }
  return true;
  case 1:
  {
   const math::Vector<nat32> & paras = vp.Paras();
   if (paras[0]!=hon.Norms()) return false;
   
   delete pipe[0];
   delete pipe[1];
   
   pipe[0] = new inf::Grid2D(labels,paras[1],paras[2]);
   pipe[1] = static_cast<inf::Grid2D*>(vp.Clone());   
  }
  return true;
  default:
  return false;
 }
}

void GridDisparityOrient2D::Construct(nat32 level,inf::FactorConstruct & fc) const
{
 // Calculate size...
  const math::Vector<nat32> & paras = pipe[0]->Paras();
  nat32 width = paras[1]>>level; if (width==0) width = 1;
  nat32 height = paras[2]>>level; if (height==0) height = 1;


 // Create function, setting all parameters as necesary...
  DisparityOrient * func = new DisparityOrient();
  func->Set(maxCost,angMult,labels,depth,hon);
  nat32 hand = fc.MakeFuncs(func,(width-1)*(height-1));


 // Wire it up...
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    // Link 0...
     if ((x!=width-1)&&(y!=height-1)) fc.Link(hand,y*(width-1)+x,0,0,y*width+x,0);

    // Link 1...
     if ((x!=0)&&(y!=height-1)) fc.Link(hand,y*(width-1)+x-1,1,0,y*width+x,1);

    // Link 2...
     if ((x!=width-1)&&(y!=0)) fc.Link(hand,(y-1)*(width-1)+x,2,0,y*width+x,2);

    // Link 3...
     if ((x!=width-1)&&(y!=height-1)) fc.Link(hand,y*(width-1)+x,3,1,y*width+x,3);
   }
  }
}

cstrconst GridDisparityOrient2D::TypeString() const
{
 return "eos::stereo::GridDisparityOrient2D";
}

//------------------------------------------------------------------------------
AlbedoOrient::AlbedoOrient()
:angAlias(0.25),maxCost(1.0),angRes(90),
toLight(0.0,0.0,1.0),hon(0),
orToCCA(null<ConeCosAng*>())
{
 cca[0] = null<real32*>();
 cca[1] = null<real32*>();
}

AlbedoOrient::~AlbedoOrient()
{
 delete[] orToCCA;
 delete[] cca[1]; 
 delete[] cca[0];
}

void AlbedoOrient::Set(real32 angA,real32 maxC,nat32 angR,
                       const bs::Normal & toL,const alg::HemiOfNorm & h,
                       const svt::Field<real32> & ir)
{
 angAlias = angA;
 maxCost = maxC;
 angRes = angR;
 toLight = toL;
 hon = h;
 irr = ir;
}

nat32 AlbedoOrient::Links() const
{
 return 2;
}

void AlbedoOrient::LinkType(nat32 ind,inf::MessageClass & out) const
{
 inf::Frequency::MakeClass(hon.Norms(),out);
}

void AlbedoOrient::ToSP()
{
 log::Assert(false,"stereo::AlbedoOrient can not be used with sum-product");
}

void AlbedoOrient::SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{}

void AlbedoOrient::SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{}

void AlbedoOrient::SendAllSP(nat32 instance,const inf::MessageSet & ms)
{}

void AlbedoOrient::ToMS()
{
 // Create the orToCCA lookup table...
  orToCCA = new ConeCosAng[hon.Norms()];
  real32 diffMult = angAlias*real32(angRes);
  for (nat32 i=0;i<hon.Norms();i++)
  {
   // Calculate the relevant cos-angle, map it into the cca array...
    real32 ang = hon.Norm(i)*toLight;
    real32 pos = real32(angRes)*ang;

   // Split it into its two neighbours, handling out of bounds, anti-alias...
    orToCCA[i].index[0] = nat32(math::RoundDown(pos));
    orToCCA[i].index[1] = orToCCA[i].index[0]+1;

    if ((orToCCA[i].index[0]>=0)&&(orToCCA[i].index[0]<int32(angRes)))
    {
     orToCCA[i].cost[0] = diffMult*(pos-real32(orToCCA[i].index[0]));
    }
    else
    {
     orToCCA[i].index[0] = 0;
     orToCCA[i].cost[0] = maxCost;
    }
    
    if ((orToCCA[i].index[1]>=0)&&(orToCCA[i].index[1]<int32(angRes)))
    {
     orToCCA[i].cost[1] = diffMult*(real32(orToCCA[i].index[1])-pos);
    }
    else
    {
     orToCCA[i].index[1] = 0;
     orToCCA[i].cost[1] = maxCost;
    }    
  }
  
 // Declare the cca variable, ready for action...
  cca[0] = new real32[angRes];
  cca[1] = new real32[angRes];
}

void AlbedoOrient::SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{
 log::Assert(false,"stereo::AlbedoOrient can not be used with non-loopy belief propagation");
}

void AlbedoOrient::SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{
 log::Assert(false,"stereo::AlbedoOrient can not be used with non-loopy belief propagation");
}

void AlbedoOrient::SendAllMS(nat32 instance,const inf::MessageSet & ms)
{
 // Get the inputs and outputs...
  inf::Frequency in[2];
  inf::Frequency out[2];
  for (nat32 i=0;i<2;i++)
  {
   in[i] = ms.GetIn<inf::Frequency>(i);
   out[i] = ms.GetOut<inf::Frequency>(i); 
  }


 // Get the irradiance values for each of the two nodes in question...
  real32 ir[2];
  if (instance>=((irr.Size(0)-1)*irr.Size(1)))
  {
   instance -= (irr.Size(0)-1)*irr.Size(1);
   nat32 bX = instance/(irr.Size(1)-1);
   nat32 bY = instance%(irr.Size(1)-1);
   
   ir[0] = irr.Get(bX,bY);
   ir[1] = irr.Get(bX,bY+1);
  }
  else
  {
   nat32 bX = instance%(irr.Size(0)-1);
   nat32 bY = instance/(irr.Size(0)-1);

   ir[0] = irr.Get(bX,bY);
   ir[1] = irr.Get(bX+1,bY);
  }
  bit doAng = (!math::IsZero(ir[0]))&&(!math::IsZero(ir[1]));


 // Generate the out[0] message...
  // Calculate the maximum cost allowed...
   real32 highCost = in[1][0];
   for (nat32 i=1;i<hon.Norms();i++) highCost = math::Min(highCost,in[1][i]);
   highCost += maxCost;

  // Set the angle array to be the maximum cost allowed...
   for (nat32 i=0;i<angRes;i++) cca[0][i] = highCost;

  // Update the angle array with all the input data...
   for (nat32 i=0;i<hon.Norms();i++)
   {
    cca[0][orToCCA[i].index[0]] = math::Min(cca[0][orToCCA[i].index[0]],in[1][i]+orToCCA[i].cost[0]);
    cca[0][orToCCA[i].index[1]] = math::Min(cca[0][orToCCA[i].index[1]],in[1][i]+orToCCA[i].cost[1]);
   }

  // Transform the angle array with the irradiance ratio...
   real32 ratio;
   if (doAng) ratio = ir[1]/ir[0];
         else ratio = 1.0;

   for (nat32 i=0;i<angRes;i++)
   {
    int32 from = int32(math::Round(real32(i)*ratio));
    if (from<int32(angRes)) cca[1][i] = cca[0][from];
                       else cca[1][i] = highCost;
   }

  // Pull from the angle array into the output, and drop...
   for (nat32 i=0;i<hon.Norms();i++)
   {
    out[0][i] = math::Min(cca[1][orToCCA[i].index[0]]+orToCCA[i].cost[0],
                          cca[1][orToCCA[i].index[1]]+orToCCA[i].cost[1]);
   }
   out[0].Drop();


 // Generate the out[1] message...
  // Calculate the maximum cost allowed...
   highCost = in[0][0];
   for (nat32 i=1;i<hon.Norms();i++) highCost = math::Min(highCost,in[0][i]);
   highCost += maxCost;

  // Set the angle array to be the maximum cost allowed...
   for (nat32 i=0;i<angRes;i++) cca[0][i] = highCost;

  // Update the angle array with all the input data...
   for (nat32 i=0;i<hon.Norms();i++)
   {
    cca[0][orToCCA[i].index[0]] = math::Min(cca[0][orToCCA[i].index[0]],in[0][i]+orToCCA[i].cost[0]);
    cca[0][orToCCA[i].index[1]] = math::Min(cca[0][orToCCA[i].index[1]],in[0][i]+orToCCA[i].cost[1]);
   }

  // Transform the angle array with the irradiance ratio...
   if (doAng) ratio = ir[0]/ir[1];
         else ratio = 1.0;

   for (nat32 i=0;i<angRes;i++)
   {
    int32 from = int32(math::Round(real32(i)*ratio));
    if (from<int32(angRes)) cca[1][i] = cca[0][from];
                       else cca[1][i] = highCost;
   }

  // Pull from the angle array into the output, and drop...
   for (nat32 i=0;i<hon.Norms();i++)
   {
    out[1][i] = math::Min(cca[1][orToCCA[i].index[0]]+orToCCA[i].cost[0],
                          cca[1][orToCCA[i].index[1]]+orToCCA[i].cost[1]);
   }
   out[1].Drop();
}

cstrconst AlbedoOrient::TypeString() const
{
 return "eos::stereo::AlbedoOrient";
}

//------------------------------------------------------------------------------
GridAlbedoOrient2D::GridAlbedoOrient2D()
:angAlias(0.25),maxCost(1.0),angRes(90),toLight(0.0,0.0,1.0),hon(0),pipe(null<inf::Grid2D*>())
{}

GridAlbedoOrient2D::~GridAlbedoOrient2D()
{
 delete pipe;
 for (nat32 i=0;i<cache.Size();i++)
 {
  delete cache[i].var;
 }
}

void GridAlbedoOrient2D::Set(real32 angA,real32 maxC,nat32 angR,
                             const bs::Normal & toL,const alg::HemiOfNorm & h,
                             const svt::Field<real32> & ir)
{
 angAlias = angA;
 maxCost = maxC;
 angRes = angR;
 toLight = toL;
 hon = h;
 irr = ir;

 delete pipe;
 pipe = new inf::Grid2D(hon.Norms(),irr.Size(0),irr.Size(1));
}

nat32 GridAlbedoOrient2D::PipeCount() const
{
 return 1;
}

bit GridAlbedoOrient2D::PipeIsSet(nat32 ind) const
{
 return pipe!=null<inf::Grid2D*>();
}

const inf::VariablePattern & GridAlbedoOrient2D::PipeGet(nat32 ind) const
{
 return *pipe;
}

bit GridAlbedoOrient2D::PipeSet(nat32 ind,const inf::VariablePattern & vp)
{
 log::Assert(pipe);
 if (*pipe==vp) return true;
           else return false;
}

void GridAlbedoOrient2D::Construct(nat32 level,inf::FactorConstruct & fc) const
{
 // Cap level...
  if (level>pipe->MaxLevel()) level = pipe->MaxLevel();

  
 // Calculate size...
  const math::Vector<nat32> & paras = pipe->Paras();
  nat32 width = paras[1]>>level; if (width==0) width = 1;
  nat32 height = paras[2]>>level; if (height==0) height = 1;


 // If needed extend array of down-sized irradiance maps...
  nat32 cacheLevel = cache.Size();
  if (cacheLevel<=level)
  {
   cache.Size(level+1);
   if (cacheLevel==0)
   {
    cache[0].var = null<svt::Var*>();
    cache[0].irr = irr;
    ++cacheLevel;
   }

   real32 irrIni = 0.0;
   for (;cacheLevel<=level;cacheLevel++)
   {
    cache[cacheLevel].var = new svt::Var(irr);
    cache[cacheLevel].var->Setup2D(cache[cacheLevel-1].irr.Size(0)>>1,cache[cacheLevel-1].irr.Size(1)>>1);
    cache[cacheLevel].var->Add("irr",irrIni);
    cache[cacheLevel].var->Commit();
    
    cache[cacheLevel].var->ByName("irr",cache[cacheLevel].irr);
    
    svt::Field<real32> & in  = cache[cacheLevel-1].irr;
    svt::Field<real32> & out = cache[cacheLevel].irr;
    for (nat32 y=0;y<cache[cacheLevel].irr.Size(1);y++)
    {
     for (nat32 x=0;x<cache[cacheLevel].irr.Size(0);x++)
     {
      nat32 x2 = math::Max(x+1,cache[cacheLevel].irr.Size(0)-1);
      nat32 y2 = math::Max(y+1,cache[cacheLevel].irr.Size(1)-1);
      out.Get(x,y) = 0.25*(in.Get(x,y) + in.Get(x2,y) + in.Get(x,y2) + in.Get(x2,y2));
     }
    }
   }
  }


 // Create function, setting all parameters as necesary...
  AlbedoOrient * func = new AlbedoOrient();
  func->Set(angAlias,maxCost,angRes,toLight,hon,cache[level].irr);
  nat32 hand = fc.MakeFuncs(func,(width-1)*height + width*(height-1));


 // Wire it up...
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    // Dx...
     if (x!=width-1) fc.Link(hand,y*(width-1)+x,0,0,y*width+x,0);
     if (x!=0) fc.Link(hand,y*(width-1)+x-1,1,0,y*width+x,1);

    // Dy...
     if (y!=height-1) fc.Link(hand,(width-1)*height + x*(height-1)+y,0,0,y*width+x,0);
     if (y!=0) fc.Link(hand,(width-1)*height + x*(height-1)+y-1,1,0,y*width+x,1);
   }
  }
}

cstrconst GridAlbedoOrient2D::TypeString() const
{
 return "eos::stereo::GridAlbedoOrient2D";
}

//------------------------------------------------------------------------------
BruteDisparitySmoothSFS::BruteDisparitySmoothSFS()
:depth(null<real32*>()),lt(null<byte**>())
{}

BruteDisparitySmoothSFS::~BruteDisparitySmoothSFS()
{
 if (lt)
 {
  for (nat32 i=0;i<irrRes*2-1;i++) delete[] lt[i];
  delete[] lt;
 }
 delete[] depth;
}

void BruteDisparitySmoothSFS::Set(bit di,nat32 l,real32 * d,const bs::Normal & toL,
                                  real32 c,nat32 s,const svt::Field<real32> & ir,nat32 irrR)
{
 dir = di;
 labels = l;
 depth = new real32[labels];
 for (nat32 i=0;i<labels;i++) depth[i] = d[i];
 toLight = toL;
 cost = c;
 stride = s;
 irr = ir;
 irrRes = irrR;
}

byte * BruteDisparitySmoothSFS::GetLT() const
{
 const_cast<BruteDisparitySmoothSFS*>(this)->ToMS();
 
 nat32 mltSize = math::Sqr(math::Sqr(labels));
 mltSize = (mltSize>>3) + ((mltSize&7)?1:0);
 
 byte * ret = new byte[(irrRes*2-1)*mltSize];
 
 for (nat32 i=0;i<irrRes*2-1;i++) mem::Copy(ret + i*mltSize,lt[i],mltSize);
 
 return ret;
}

void BruteDisparitySmoothSFS::SetLT(byte * ltc)
{
 if (lt==null<byte**>())
 {
  lt = new byte*[irrRes*2-1];
  nat32 mltSize = math::Sqr(math::Sqr(labels));
  mltSize = (mltSize>>3) + ((mltSize&7)?1:0);
  
  for (nat32 i=0;i<irrRes*2-1;i++)
  {
   lt[i] = new byte[mltSize];
   mem::Copy(lt[i],ltc + mltSize*i,mltSize);
  }
 }
}

nat32 BruteDisparitySmoothSFS::Links() const
{
 return 4;
}

void BruteDisparitySmoothSFS::LinkType(nat32 ind,inf::MessageClass & out) const
{
 return inf::Frequency::MakeClass(labels,out);
}

void BruteDisparitySmoothSFS::ToSP()
{
 log::Assert(false,"BruteDisparitySmoothSFS is not sum-product compatable");
}

void BruteDisparitySmoothSFS::SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{}

void BruteDisparitySmoothSFS::SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{}

void BruteDisparitySmoothSFS::SendAllSP(nat32 instance,const inf::MessageSet & ms)
{}

void BruteDisparitySmoothSFS::ToMS()
{
 LogBlock("void BruteDisparitySmoothSFS::ToMS()","-");
 if (lt==null<byte**>())
 {
  // Generate the lookup table - iterate every single disparity combination,
  // for every single irradiance arrangment to consider. Let user make tea whilst
  // this is happenning...
   // Create the data structure...
    lt = new byte*[irrRes*2-1];
    nat32 mltSize = math::Sqr(math::Sqr(labels));
     mltSize = (mltSize>>3) + ((mltSize&7)?1:0);
    for (nat32 i=0;i<irrRes*2-1;i++)
    {
     lt[i] = new byte[mltSize];
     mem::Null(lt[i],mltSize);
    }

   // Do the first half, top albedo being greater than...
    for (nat32 i=1;i<irrRes-1;i++)
    {
     real32 val = real32(irrRes-1-i)/real32(irrRes-1);
     if (dir)
     {
      CalcLT(1.0/val,1,lt[irrRes-i]);
      CalcLT(val,2,lt[irrRes-i]);
     }
     else
     {
      CalcLT(1.0/val,0,lt[irrRes-i]);
      CalcLT(val,3,lt[irrRes-i]);    
     }
    }

   // Do the centre...
    if (dir)
    {
     CalcLT(1.0,1,lt[irrRes]);
     CalcLT(1.0,2,lt[irrRes]);   
    }
    else
    {
     CalcLT(1.0,0,lt[irrRes]);
     CalcLT(1.0,3,lt[irrRes]);
    }

   // Do the second half, top albedo being less than...
    for (nat32 i=1;i<irrRes-1;i++)
    {
     real32 val = real32(irrRes-1-i)/real32(irrRes-1);
     if (dir)
     {
      CalcLT(val,1,lt[irrRes+i]);
      CalcLT(1.0/val,2,lt[irrRes+i]);
     }
     else
     {
      CalcLT(val,0,lt[irrRes+i]);
      CalcLT(1.0/val,3,lt[irrRes+i]);    
     }
    }
   
   // Testing pass - gather statistics for the lookup tables and log them, so
   // I can decide on a sensible compression scheme...
    for (nat32 i=0;i<irrRes*2-1;i++)
    {
     LogDebug("Albedo Smoothing Lookup {dir,labels,bitcount}" << LogDiv()
              << dir << LogDiv() << labels << LogDiv() << math::BitCount(lt[i],mltSize));
    }
 }
}

void BruteDisparitySmoothSFS::SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{
 log::Assert(false,"BruteDisparitySmoothSFS is not non-loopy compatable.");
}

void BruteDisparitySmoothSFS::SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{
 log::Assert(false,"BruteDisparitySmoothSFS is not non-loopy compatable.");
}

void BruteDisparitySmoothSFS::SendAllMS(nat32 instance,const inf::MessageSet & ms)
{
 // Extract the 4 in's and 4 out's...
  inf::Frequency in[4];
  inf::Frequency out[4];
  for (nat32 i=0;i<4;i++)
  {
   in[i] = ms.GetIn<inf::Frequency>(i);
   out[i] = ms.GetOut<inf::Frequency>(i); 
  }


 // Calculate the maximal value that any cost may take...
  real32 maxCost = cost;
  for (nat32 i=0;i<4;i++)
  {
   real32 min = in[i][0];
   for (nat32 j=1;j<labels;j++) min = math::Min(min,in[i][j]);
   maxCost += min;
  }


 // Set all outputs to the maximal value...
  for (nat32 i=0;i<4;i++)
  {
   for (nat32 j=0;j<labels;j++) out[i][j] = maxCost;
  }


 // Iterate the entire 4 dimensional lookup table, whenever we find a set bit
 // we update the outputs as relevant...
  nat32 x = instance%stride;
  nat32 y = instance/stride;
 
  nat32 bit = 0;
  byte * targ;
  
  real32 irA;
  real32 irB;
  if (dir) {irA = irr.Get(x+1,y); irB = irr.Get(x,y+1);}
      else {irA = irr.Get(x,y); irB = irr.Get(x+1,y+1);}
  if (math::IsZero(irA)||math::IsZero(irB))
  {
   for (nat32 i=0;i<4;i++) out[i].Drop();
   return;
  }

  if (irB<irA) targ = lt[math::Clamp(nat32(math::RoundDown((irB/irA)*irrRes)),nat32(1),irrRes-1)-1];
          else targ = lt[irrRes*2-math::Clamp(nat32(math::RoundDown((irA/irB)*irrRes)),nat32(1),irrRes-1)];
  
  for (nat32 d0=0;d0<labels;d0++)
  {
   real32 costA = in[0][d0];
   for (nat32 d1=0;d1<labels;d1++)
   {
    real32 costB = costA + in[1][d1];
    for (nat32 d2=0;d2<labels;d2++)
    {
     real32 costC = costB + in[2][d2];
     for (nat32 d3=0;d3<labels;d3++)
     {
      real32 costD = costC + in[3][d3];

      if ((targ[0]>>bit)&1)
      {
       // Update each of the 4 outputs with the relevant cost...
        out[0][d0] = math::Min(out[0][d0],costD-in[0][d0]);
        out[1][d1] = math::Min(out[1][d1],costD-in[1][d1]);
        out[2][d2] = math::Min(out[2][d2],costD-in[2][d2]);
        out[3][d3] = math::Min(out[3][d3],costD-in[3][d3]);
      }
      
      ++bit;
      if (bit==8)
      {
       bit = 0;
       ++targ;
      }
     }
    }
   }
  }


 // Drop the outputs...
  for (nat32 i=0;i<4;i++) out[i].Drop();
}

cstrconst BruteDisparitySmoothSFS::TypeString() const
{
 return "eos::inf::BruteDisparitySmoothSFS";
}

void BruteDisparitySmoothSFS::CalcLT(real32 irrRatio,nat32 outC,byte * out)
{
 LogTime("void BruteDisparitySmoothSFS::CalcLT(...)");

 nat32 labels2 = math::Sqr(labels);
 nat32 labels3 = labels2*labels;
 mem::PrefetchRead(depth); // Need to actually check this is advantageous.

 switch (outC)
 {
  case 0:
  {
   for (nat32 d1=0;d1<labels;d1++)
   {
    nat32 bitA = labels2*d1;
    for (nat32 d2=0;d2<labels;d2++)
    {
     nat32 bitB = bitA + labels*d2;
     for (nat32 d3=0;d3<labels;d3++)
     {
      nat32 bitC = bitB + d3;

      real32 dx = depth[d2]-depth[d3];
      real32 dy = depth[d1]-depth[d3];
      real32 ang  = -toLight[0]*dx - toLight[1]*dy + toLight[2];
             ang *= math::InvSqrt(math::Sqr(dx)+math::Sqr(dy)+1.0);
      if (ang<0.0) continue;

      ang *= irrRatio;
      real32 best = math::Infinity<real32>();
      nat32 bestInd = nat32(-1);
       for (nat32 d0=0;d0<labels;d0++)
       {
        real32 ndx = depth[d1]-depth[d0];
        real32 ndy = depth[d2]-depth[d0];
        real32 nAng = -toLight[0]*ndx - toLight[1]*ndy + toLight[2];
               nAng *= math::InvSqrt(math::Sqr(ndx)+math::Sqr(ndy)+1.0);
        if (nAng<0.0) continue;

        real32 angDiff = math::Abs(nAng-ang);
        if (angDiff<best)
        {
         best = angDiff;
         bestInd = d0;
        }
       }
      if (bestInd!=nat32(-1))
      {
       nat32 bitD = bitC + labels3*bestInd;
       out[bitD>>3] |= 1 << (bitD&7);
      }
     }
    }
   }
  }
  break;
  case 1:
  {
   for (nat32 d0=0;d0<labels;d0++)
   {
    nat32 bitA = labels3*d0;
    for (nat32 d2=0;d2<labels;d2++)
    {
     nat32 bitB = bitA + labels*d2;
     for (nat32 d3=0;d3<labels;d3++)
     {
      nat32 bitC = bitB + d3;
      
      real32 dx = depth[d3]-depth[d2];
      real32 dy = depth[d2]-depth[d0];
      real32 ang  = -toLight[0]*dx - toLight[1]*dy + toLight[2];
             ang *= math::InvSqrt(math::Sqr(dx)+math::Sqr(dy)+1.0);
      if (ang<0.0) continue;
      
      ang *= irrRatio;
      real32 best = math::Infinity<real32>();
      nat32 bestInd = nat32(-1);
       for (nat32 d1=0;d1<labels;d1++)
       {
        real32 ndx = depth[d1]-depth[d0];
        real32 ndy = depth[d3]-depth[d1];
        real32 nAng = -toLight[0]*ndx - toLight[1]*ndy + toLight[2];
               nAng *= math::InvSqrt(math::Sqr(ndx)+math::Sqr(ndy)+1.0);
        if (nAng<0.0) continue;
     
        real32 angDiff = math::Abs(nAng-ang);
        if (angDiff<best)
        {
         best = angDiff;
         bestInd = d1;
        }
       }
      if (bestInd!=nat32(-1))
      {
       nat32 bitD = bitC + labels2*bestInd;
       out[bitD>>3] |= 1 << (bitD&7);
      }
     }
    }
   }
  }
  break;
  case 2:
  {
   for (nat32 d0=0;d0<labels;d0++)
   {
    nat32 bitA = labels3*d0;
    for (nat32 d1=0;d1<labels;d1++)
    {
     nat32 bitB = bitA + labels2*d1;
     for (nat32 d3=0;d3<labels;d3++)
     {
      nat32 bitC = bitB + d3;
      
      real32 dx = depth[d1]-depth[d0];
      real32 dy = depth[d3]-depth[d1];
      real32 ang  = -toLight[0]*dx - toLight[1]*dy + toLight[2];
             ang *= math::InvSqrt(math::Sqr(dx)+math::Sqr(dy)+1.0);
      if (ang<0.0) continue;
      
      ang *= irrRatio;
      real32 best = math::Infinity<real32>();
      nat32 bestInd = nat32(-1);
       for (nat32 d2=0;d2<labels;d2++)
       {
        real32 ndx = depth[d3]-depth[d2];
        real32 ndy = depth[d2]-depth[d0];
        real32 nAng = -toLight[0]*ndx - toLight[1]*ndy + toLight[2];
               nAng *= math::InvSqrt(math::Sqr(ndx)+math::Sqr(ndy)+1.0);
        if (nAng<0.0) continue;
     
        real32 angDiff = math::Abs(nAng-ang);
        if (angDiff<best)
        {
         best = angDiff;
         bestInd = d2;
        }
       }
      if (bestInd!=nat32(-1))
      {
       nat32 bitD = bitC + labels*bestInd;
       out[bitD>>3] |= 1 << (bitD&7);
      }
     }
    }
   }
  }
  break;
  case 3:
  {
   for (nat32 d0=0;d0<labels;d0++)
   {
    nat32 bitA = labels3*d0;
    for (nat32 d1=0;d1<labels;d1++)
    {
     nat32 bitB = bitA + labels2*d1;
     for (nat32 d2=0;d2<labels;d2++)
     {
      nat32 bitC = bitB + labels*d2;
      
      real32 dx = depth[d1]-depth[d0];
      real32 dy = depth[d2]-depth[d0];
      real32 ang  = -toLight[0]*dx - toLight[1]*dy + toLight[2];
             ang *= math::InvSqrt(math::Sqr(dx)+math::Sqr(dy)+1.0);
      if (ang<0.0) continue;
      
      ang *= irrRatio;
      real32 best = math::Infinity<real32>();
      nat32 bestInd = nat32(-1);
       for (nat32 d3=0;d3<labels;d3++)
       {
        real32 ndx = depth[d3]-depth[d2];
        real32 ndy = depth[d3]-depth[d1];
        real32 nAng = -toLight[0]*ndx - toLight[1]*ndy + toLight[2];
               nAng *= math::InvSqrt(math::Sqr(ndx)+math::Sqr(ndy)+1.0);
        if (nAng<0.0) continue;
     
        real32 angDiff = math::Abs(nAng-ang);
        if (angDiff<best)
        {
         best = angDiff;
         bestInd = d3;
        }
       }
      if (bestInd!=nat32(-1))
      {
       nat32 bitD = bitC + bestInd;
       out[bitD>>3] |= 1 << (bitD&7);
      }
     }
    }
   }
  }
  break;
 }
}

//------------------------------------------------------------------------------
DisparitySmoothSFS::DisparitySmoothSFS()
:depth(null<real32*>())
{
 for (nat32 i=0;i<4;i++)
 {
  dispToAng[i] = null<real32*>();
  angToDisp[i] = null<nat8*>();
 }
}

DisparitySmoothSFS::~DisparitySmoothSFS()
{ 
 for (nat32 i=0;i<4;i++)
 {
  delete[] dispToAng[i];
  delete[] angToDisp[i];
 }

 delete[] depth; 
}

void DisparitySmoothSFS::Set(nat32 l,real32 * d,const bs::Normal & toL,
                             real32 c,nat32 angR,nat32 s,
                             const svt::Field<real32> & ir)
{
 labels = l;
 delete[] depth;
 depth = new real32[labels];
 for (nat32 i=0;i<labels;i++) depth[i] = d[i];
 
 toLight = toL;
 cost = c;
 angRes = angR;
 stride = s;
 irr = ir;
}

nat32 DisparitySmoothSFS::Links() const
{
 return 4;
}

void DisparitySmoothSFS::LinkType(nat32 ind,inf::MessageClass & out) const
{
 return inf::Frequency::MakeClass(labels,out);
}

void DisparitySmoothSFS::ToSP()
{
 log::Assert(false,"DisparitySmoothSFS is not sum-product compatable");
}

void DisparitySmoothSFS::SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{}

void DisparitySmoothSFS::SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{}

void DisparitySmoothSFS::SendAllSP(nat32 instance,const inf::MessageSet & ms)
{}

void DisparitySmoothSFS::ToMS()
{
 const real32 mX[4] = {1,-1,1,-1};
 const real32 mY[4] = {1,1,-1,-1};

 // dispToAng lookup table...
  for (nat32 i=0;i<4;i++)
  {
   dispToAng[i] = new real32[labels*labels*labels];
   real32 * targ = dispToAng[i];
   for (nat32 dyi=0;dyi<labels;dyi++)
   {
    for (nat32 dxi=0;dxi<labels;dxi++)
    {
     for (nat32 ci=0;ci<labels;ci++)
     {
      real32 dx = mX[i] * (depth[dxi]-depth[ci]);
      real32 dy = mY[i] * (depth[dyi]-depth[ci]);
      *targ = -dx*toLight[0] -dy*toLight[1] +toLight[2];
      if (*targ<0.0) *targ = 0.0;
                else *targ *= math::InvSqrt(math::Sqr(dx)+math::Sqr(dy)+1.0);

      ++targ;
     }
    }
   }
  }


 // angToDisp lookup table...
  real32 invAngRes = 1.0/angRes;
  for (nat32 i=0;i<4;i++)
  {
   angToDisp[i] = new nat8[labels*labels*angRes];
   nat8 * targ = angToDisp[i];
   for (nat32 dxi=0;dxi<labels;dxi++)
   {
    for (nat32 dyi=0;dyi<labels;dyi++)
    {
     real32 * dta = dispToAng[3-i] + labels*(labels*dyi + dxi);
     for (nat32 ai=0;ai<angRes;ai++)
     {
      real32 ang = real32(ai)/real32(angRes);
      // We essentially brute force this - try every disparity and select the 
      // closest. Not very efficient, but the efficient solution would not be
      // much faster and would take considerably more code...
       *targ = 0;
       real32 best = math::Abs(ang-dta[0]);
       for (nat32 j=1;j<labels;j++)
       {
        real32 score = math::Abs(ang-dta[j]);
        if (score<best)
        {
         best = score;
         *targ = j;
        }
       }
       if (best>invAngRes) *targ = nat8(-1);
       
      ++targ;
     }
    }
   }
  }
}

void DisparitySmoothSFS::SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc)
{
 log::Assert(false,"DisparitySmoothSFS is not non-loopy compatable.");
}

void DisparitySmoothSFS::SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti)
{
 log::Assert(false,"DisparitySmoothSFS is not non-loopy compatable.");
}

void DisparitySmoothSFS::SendAllMS(nat32 instance,const inf::MessageSet & ms)
{
 // Extract the 4 in's and 4 out's...
  inf::Frequency in[4];
  inf::Frequency out[4];
  for (nat32 i=0;i<4;i++)
  {
   in[i] = ms.GetIn<inf::Frequency>(i);
   out[i] = ms.GetOut<inf::Frequency>(i); 
  }


 // Calculate the maximal value that any cost may take...
  real32 maxCost = cost;
  for (nat32 i=0;i<4;i++)
  {
   real32 min = in[i][0];
   for (nat32 j=1;j<labels;j++) min = math::Min(min,in[i][j]);
   maxCost += min;
  }


 // For each output iterate the relevant 3 inputs, calculating which output 
 // disparity is relevent for each in turn and improving its cost if possible.
 // This is bloody slow to put into a damned message calculation method, no 
 // choice unfortunatly...
  const nat32 xo[4] = {2,3,0,1};
  const nat32 yo[4] = {1,0,3,2};

  nat32 bX = instance%stride;
  nat32 bY = instance/stride;
  real32 ir[4];
   ir[0] = irr.Get(bX  ,bY);
   ir[1] = irr.Get(bX+1,bY);
   ir[2] = irr.Get(bX  ,bY+1);
   ir[3] = irr.Get(bX+1,bY+1);

  for (nat32 i=0;i<4;i++)
  {
   // Set all outputs to the max possible cost...
    for (nat32 j=0;j<labels;j++) out[i][j] = maxCost;
   
   // Iterate all label combos that matter, for each combo sum the costs, 
   // with some pruning, then calculate where the damned cost should go in the 
   // output array...
    real32 * d2aTarg = dispToAng[3-i];
    real32 irrS = ir[i];
    real32 irrD = ir[3-i];
    if (math::IsZero(irrD)||math::IsZero(irrS)) continue;
    real32 irrMult = irrS/irrD;

    nat8 * a2dTargA = angToDisp[3-i];
    for (nat32 dyi=0;dyi<labels;dyi++)
    {
     real32 costA = in[yo[i]][dyi];
     if (costA>=maxCost) continue;
     nat8 * a2dTargB = &a2dTargA[dyi*labels*angRes];
     for (nat32 dxi=0;dxi<labels;dxi++)
     {
      real32 costB = costA + in[xo[i]][dxi];
      if (costB>=maxCost) continue;
      nat8 * a2dTargC = &a2dTargB[dxi*angRes];
      mem::PrefetchRead(a2dTargC);
      for (nat32 ci=0;ci<labels;ci++)
      {
       real32 costC = costB + in[3-i][ci];
       if (costC<maxCost)
       {
        // Convert the angle across...
         real32 ang = irrMult * (*d2aTarg);
        
        // Figgure out where in the discretization the angle sits, 
        // exit if its out of range...
         if ((ang>=0.0)&&(ang<1.0))
         {
          nat32 ca = nat32(math::RoundDown(real32(angRes)*ang));

          // Use the second lookup table to determine which output disparity to 
          // update...
           nat8 dtu = a2dTargC[ca];
           if (dtu!=nat8(-1))
           {
            // Set the relevent output disparity to the minimum of its current value
            // and the obtained cost...
             out[i][dtu] = math::Min(out[i][dtu],costC);
           }
         }
       }
       ++d2aTarg;
      }
     }
    }
    
   out[i].Drop();
  }
  
  // **************************************
 /*LogDebug("msg {in0,in1,in2,in3,out0,out1,out2,out3}" << LogDiv() 
          << in[0] << LogDiv() << in[1] << LogDiv() << in[2] << LogDiv() << in[3] << LogDiv()
          << out[0] << LogDiv() << out[1] << LogDiv() << out[2] << LogDiv() << out[3]);*/
}

cstrconst DisparitySmoothSFS::TypeString() const
{
 return "eos::stereo::DisparitySmoothSFS";
}

//------------------------------------------------------------------------------
DisparityAlbedoCost::DisparityAlbedoCost()
:cost(1.0),angRes(45),labels(1),
dispToDepth(null<real32*>()),doBrute(false),
vp(null<inf::Grid2D*>()),sIrr(null<Node*>()),
lt(null<byte*>())
{}

DisparityAlbedoCost::~DisparityAlbedoCost()
{
 delete[] lt;
 for (nat32 i=0;i<=vp->MaxLevel();i++) delete sIrr[i].var;
 delete[] sIrr;
 delete vp;
 delete dispToDepth;
}

void DisparityAlbedoCost::Set(real32 c,const bs::Normal & toL,
                              nat32 angR,const svt::Field<real32> & ir,
                              const cam::DispConv & dc,int32 minDisp,int32 maxDisp)
{
 if (!dc.PosInd()) log::Assert(false,"Only position independent disparity convertors can be used with the DisparityAlbedoCost object type");

 labels = maxDisp-minDisp+1;
 delete[] dispToDepth;
 dispToDepth = new real32[labels];
 for (nat32 i=0;i<labels;i++)
 {
  dispToDepth[i] = dc.DispToDepth(0.0,0.0,real32(i)+real32(minDisp));
 }

 cost = c;
 toLight = toL;

 angRes = angR;
 irr = ir;
 
 delete vp;
 vp = new inf::Grid2D(labels,irr.Size(0),irr.Size(1));
}

void DisparityAlbedoCost::EnableBrute(nat32 irrR)
{
 doBrute = true;
 irrRes = irrR;
}

nat32 DisparityAlbedoCost::PipeCount() const
{
 return 1;
}

bit DisparityAlbedoCost::PipeIsSet(nat32 ind) const
{
 return true;
}

const inf::VariablePattern & DisparityAlbedoCost::PipeGet(nat32 ind) const
{
 return *vp;
}

bit DisparityAlbedoCost::PipeSet(nat32 ind,const inf::VariablePattern & ovp)
{
 if (str::Compare(typestring(*vp),typestring(ovp))!=0) return false;
 if (vp->Paras()!=ovp.Paras()) return false;
 return true;
}

void DisparityAlbedoCost::Construct(nat32 level,inf::FactorConstruct & fc) const
{
 LogBlock("void DisparityAlbedoCost::Construct(...) const","{level}" << LogDiv() << level);
 // Adjust the level for overshooting, calculate the width/height
 // we will be working at...
  if (level>vp->MaxLevel()) level = vp->MaxLevel();

  const math::Vector<nat32> & paras = vp->Paras();
  nat32 width = paras[1]>>level;
  nat32 height = paras[2]>>level;
  if ((width==0)||(height==0)) return;

  nat32 aWidth = width-1;
  nat32 aHeight = height-1;

 // Generate scaled irradiance maps if they are not allready avaliable...
  if (sIrr==null<Node*>())
  {
   sIrr = new Node[vp->MaxLevel()+1];

   sIrr[0].var = null<svt::Var*>();
   sIrr[0].field = irr;
   for (nat32 i=1;i<=vp->MaxLevel();i++)
   {
    nat32 nWidth  = irr.Size(0)>>i;
    nat32 nHeight = irr.Size(1)>>i;
    if ((nWidth==0)||(nHeight==0))
    {
     sIrr[i].var = null<svt::Var*>();
     continue;
    }
    
    sIrr[i].var = new svt::Var(irr);
    sIrr[i].var->Setup2D(nWidth,nHeight);
    real32 irrIni = 0.0;
    sIrr[i].var->Add("irr",irrIni);
    sIrr[i].var->Commit(false);
    sIrr[i].var->ByName("irr",sIrr[i].field);
    
    for (nat32 y=0;y<sIrr[i].field.Size(1);y++)
    {
     for (nat32 x=0;x<sIrr[i].field.Size(0);x++)
     {
      sIrr[i].field.Get(x,y) = 0.25*(sIrr[i-1].field.Get(x*2  ,y*2) + 
                                     sIrr[i-1].field.Get(x*2+1,y*2) +
                                     sIrr[i-1].field.Get(x*2  ,y*2+1) +
                                     sIrr[i-1].field.Get(x*2+1,y*2+1));
     }
    }
   }
  }


 // Create the factor, fill in all the details, create handle...
  nat32 hand;
  if (doBrute)
  {
   BruteDisparitySmoothSFS * dsf = new BruteDisparitySmoothSFS();
   dsf->Set(false,labels,dispToDepth,toLight,cost,aWidth,sIrr[level].field,irrRes);
   if (lt) dsf->SetLT(lt);
      else lt = dsf->GetLT();
   hand = fc.MakeFuncs(dsf,aWidth*aHeight);
  }
  else
  {
   DisparitySmoothSFS * dsf = new DisparitySmoothSFS();
   dsf->Set(labels,dispToDepth,toLight,cost,angRes,aWidth,sIrr[level].field);
   hand = fc.MakeFuncs(dsf,aWidth*aHeight);
  }


 // Wire it up...
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    // 0:
     if ((y!=aHeight)&&(x!=aWidth)) fc.Link(hand,y*aWidth+x,0,0,y*width+x,0);

    // 1:
     if ((y!=aHeight)&&(x!=0)) fc.Link(hand,y*aWidth+x-1,1,0,y*width+x,1);

    // 2:
     if ((y!=0)&&(x!=aWidth)) fc.Link(hand,(y-1)*aWidth+x,2,0,y*width+x,2);

    // 3:
     if ((y!=0)&&(x!=0)) fc.Link(hand,(y-1)*aWidth+x-1,3,0,y*width+x,3);               
   }
  }
}

cstrconst DisparityAlbedoCost::TypeString() const
{
 return "eos::stereo::DisparityAlbedoCost";
}

//------------------------------------------------------------------------------
SfgsDisparity1::SfgsDisparity1()
:minDisp(-30),maxDisp(30),
alphaMult(0.1),alphaMax(1.0),
betaMult(0.1),betaMax(1.0),
gammaMult(0.05),gammaMax(1.0),
oobd(math::Exp(-2.0)),max(2.0),
diffErrLevel(0.1),
smoothMult(0.5),smoothMax(2.0),smoothOverride(false),
albedoOverride(false),saCost(1.0),saAngRes(45),bruteAlbedo(false),bruteIrrRes(255),
orientOverride(false),orientsubDivs(2),orientMaxCost(1.0),orientAngMult(1.0),orientEqCost(2.0),
orientOverrideSFS(false),orientSFSangAlias(0.25),orientSFSmaxCost(1.0),orientSFSangRes(90)
{
 dc[0] = null<cam::DispConv*>();
 dc[1] = null<cam::DispConv*>();
}

SfgsDisparity1::~SfgsDisparity1()
{}

void SfgsDisparity1::SetDispRange(int32 minD,int32 maxD)
{
 minDisp = minD;
 maxDisp = maxD;
}

void SfgsDisparity1::SetMatchParas(real32 aMult,real32 aMax,
                                   real32 bMult,real32 bMax,
                                   real32 gMult,real32 gMax,
                                   real32 dMult,real32 dMax,
                                   real32 oo,real32 m)
{
 alphaMult = aMult; alphaMax = aMax;
 betaMult  = bMult; betaMax  = bMax;
 gammaMult = gMult; gammaMax = gMax;
 deltaMult = dMult; deltaMax = dMax;
 oobd = oo; max = m;
}

void SfgsDisparity1::SetDiffParas(real32 errLevel)
{
 diffErrLevel = errLevel;
}

void SfgsDisparity1::SetSmoothParas(real32 sMult,real32 sMax)
{
 smoothMult = sMult;
 smoothMax = sMax;
}

void SfgsDisparity1::SmoothOverride(bit override)
{
 smoothOverride = override;
}

void SfgsDisparity1::SmoothAlbedo(bit smoothAlbedo,real32 cost,nat32 angRes,bit brute,nat32 irrRes)
{
 albedoOverride = smoothAlbedo;
 saCost = cost;
 saAngRes = angRes;
 bruteAlbedo = brute;
 bruteIrrRes = irrRes;
}

void SfgsDisparity1::SmoothOrient(bit smoothOrient,nat32 subDivs,real32 maxCost,real32 angMult,real32 eqCost)
{
 orientOverride = smoothOrient;
 orientsubDivs = subDivs;
 orientMaxCost = maxCost;
 orientAngMult = angMult;
 orientEqCost = eqCost;
}

void SfgsDisparity1::SmoothOrientSFS(bit smoothSFS,real32 angAlias,real32 maxCost,nat32 angRes)
{
 orientOverrideSFS = smoothSFS;
 orientSFSangAlias = angAlias; 
 orientSFSmaxCost = maxCost;
 orientSFSangRes = angRes;
}

void SfgsDisparity1::SetDispConv(const cam::DispConv & left,const cam::DispConv & right)
{
 dc[0] = &left;
 dc[1] = &right;
}

void SfgsDisparity1::SetIr(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right)
{
 ir[0] = left;
 ir[1] = right;
}

void SfgsDisparity1::SetAlbedo(const svt::Field<real32> & left,const svt::Field<real32> & right)
{
 albedo[0] = left;
 albedo[1] = right;
}

void SfgsDisparity1::SetLight(const bs::Normal & left,const bs::Normal & right)
{
 light[0] = left;
 light[1] = right;
}

void SfgsDisparity1::SetDisp(svt::Field<real32> & left,svt::Field<real32> & right)
{
 disp[0] = left;
 disp[1] = right;
}

void SfgsDisparity1::SetConfidence(svt::Field<real32> & left,svt::Field<real32> & right)
{
 confidence[0] = left;
 confidence[1] = right;
}

void SfgsDisparity1::SetDispDx(svt::Field<real32> & left,svt::Field<real32> & right)
{
 dispDx[0] = left;
 dispDx[1] = right;
}

void SfgsDisparity1::SetDispDy(svt::Field<real32> & left,svt::Field<real32> & right)
{
 dispDy[0] = left;
 dispDy[1] = right;
}

void SfgsDisparity1::SetNeedle(const svt::Field<bs::Normal> & left,const svt::Field<bs::Normal> & right)
{
 needle[0] = left;
 needle[1] = right;
}

void SfgsDisparity1::Run(time::Progress * prog)
{
 LogBlock("void SfgsDisparity1::Run(...)","-");
 prog->Push();
  prog->Report(0,2);
  RunSingle(0,prog);
  prog->Report(1,2);  
  RunSingle(1,prog);
 prog->Pop();
}

void SfgsDisparity1::RunSingle(nat32 which,time::Progress * prog)
{
 LogBlock("void SfgsDisparity1::RunSingle()","{which}" << LogDiv() << which);
 prog->Push();
 
 // Some simple stuff...
  prog->Report(0,3);
  nat32 labels = maxDisp - minDisp + 1;

  int32 localMinDisp;
  int32 localMaxDisp;
  if (which==0) localMinDisp = minDisp;
           else localMinDisp = -maxDisp;
  if (which==0) localMaxDisp = maxDisp;
           else localMaxDisp = -minDisp;

  nat32 oi = (which+1)%2;

  nat32 width = disp[which].Size(0);
  nat32 height = disp[which].Size(1);


 // Make a field graph with which to construct the relevent factor graph...
  inf::FieldGraph fg(true);


 // Create the matching cost function (DSI)...
  inf::GridFreq2D * dsi = new inf::GridFreq2D(labels,width,height);
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    for (nat32 l=0;l<labels;l++)
    {
     int32 ox = int32(x) + int32(l) + localMinDisp;
     if ((ox>=0)&&(ox<int32(disp[oi].Size(0))))
     {
      real32 c1 = math::Min(alphaMult*math::Abs(albedo[which].Get(x,y)-albedo[oi].Get(ox,y)),alphaMax);
       
      real32 uSqr = math::Sqr(ir[which].Get(x,y).u-ir[oi].Get(ox,y).u);
      real32 vSqr = math::Sqr(ir[which].Get(x,y).v-ir[oi].Get(ox,y).v);
      real32 c2 = math::Min(betaMult*math::Sqrt(uSqr + vSqr),betaMax);
       
      real32 a1 = math::Min(ir[which].Get(x,y).l/albedo[which].Get(x,y),real32(1.0));
      real32 a2 = math::Min(ir[oi].Get(ox,y).l/albedo[oi].Get(ox,y),real32(1.0));
      real32 c3 = math::Min(gammaMult*math::Abs(math::InvCos(a1)-math::InvCos(a2)),gammaMax);

      bs::ColourL la = ir[which].Get(x,y);
      bs::ColourL lb = ir[oi].Get(ox,y);
      real32 c4 = math::Min<real32>(deltaMult*math::Abs(la.l-lb.l),deltaMax);

      dsi->SetCost(x,y,l,math::Min(c1+c2+c3+c4,max));
     }
     else dsi->SetCost(x,y,l,oobd);
    }
   }
  }


 // If we are in SmoothOrient mode, do that, rather than any other mode...
  if (orientOverride)
  {
   // Create the disparity to surface orientation field...
    alg::HemiOfNorm hon(orientsubDivs);

    GridDisparityOrient2D * dispOrient = new GridDisparityOrient2D();
    dispOrient->Set(orientMaxCost,orientAngMult,*dc[which],localMinDisp,localMaxDisp,hon);


   // Create handles then link it all up...
    nat32 dsiH = fg.NewFP(dsi);
    nat32 orientH = fg.NewFP(dispOrient);

    nat32 dispV = fg.NewVP();
    nat32 orientV = fg.NewVP();

    fg.Lay(dsiH,0,dispV);
    fg.Lay(orientH,0,dispV);
    fg.Lay(orientH,1,orientV);


   // Do either a Potts field or a SFS based smoothing field, depending on flags...
    if (orientOverrideSFS)
    {
     // SFS based smoothing...
      GridAlbedoOrient2D * ao = new GridAlbedoOrient2D();
      svt::Field<real32> irr;
      ir[which].SubField(0,irr);
      ao->Set(orientSFSangAlias,orientSFSmaxCost,orientSFSangRes,light[which],hon,irr);
      
      nat32 aoH = fg.NewFP(ao);
      fg.Lay(aoH,0,orientV);
    }
    else
    {
     // Potts...
      inf::GridSmoothPotts2D * sx = new inf::GridSmoothPotts2D(false);
      inf::GridSmoothPotts2D * sy = new inf::GridSmoothPotts2D(true);
      sx->Set(orientEqCost);
      sy->Set(orientEqCost);
      
      nat32 sxH = fg.NewFP(sx);
      nat32 syH = fg.NewFP(sy);
      
      fg.Lay(sxH,0,orientV);
      fg.Lay(syH,0,orientV);      
    }


   // Run...
    prog->Report(1,3);
    fg.Run(prog);


   // Extract the resulting disparity...
    prog->Report(2,3);
    for (nat32 y=0;y<disp[which].Size(1);y++)
    {
     for (nat32 x=0;x<disp[which].Size(0);x++)
     {
      disp[which].Get(x,y) = fg.Get(dispV,y*width+x).Maximum() + localMinDisp;
     }
    }
  
   // If wanted extract the disparity confidence...
    if (confidence[which].Valid())
    {
     for (nat32 y=0;y<disp[which].Size(1);y++)
     {
      for (nat32 x=0;x<disp[which].Size(0);x++)
      {
       confidence[which].Get(x,y) = fg.Get(dispV,y*width+x).Confidence();
      }
     }   
    }
    
   // If wanted extract the orientation...
    if (needle[which].Valid())
    {
     for (nat32 y=0;y<needle[which].Size(1);y++)
     {
      for (nat32 x=0;x<needle[which].Size(0);x++)
      {
       needle[which].Get(x,y) = hon.Norm(fg.Get(orientV,y*width+x).Highest());
      }
     }
    }
  }
  else
  {
   // If peicewise constant albedo smoothing is being used ignore the else and do
   // something else enirelly...
    if (albedoOverride)
    { 
     DisparityAlbedoCost * smooth = new DisparityAlbedoCost();
     svt::Field<real32> irL; ir[which].SubField(0,irL);
     smooth->Set(saCost,light[which],saAngRes,irL,*dc[which],localMinDisp,localMaxDisp);
     if (bruteAlbedo) smooth->EnableBrute(bruteIrrRes);
   
     nat32 dsiH = fg.NewFP(dsi);
     nat32 smoothH = fg.NewFP(smooth);
     nat32 dispV = fg.NewVP();
   
     fg.Lay(dsiH,0,dispV);
     fg.Lay(smoothH,0,dispV);


     // Solve this mother of a factor graph...
      prog->Report(1,3);
      fg.Run(prog);


     // Extract the resulting disparity...
      prog->Report(2,3);
      for (nat32 y=0;y<disp[which].Size(1);y++)
      {
       for (nat32 x=0;x<disp[which].Size(0);x++)
       {
        disp[which].Get(x,y) = fg.Get(dispV,y*width+x).Maximum() + localMinDisp;
       }
      }
  
     // If wanted extract the disparity confidence...
      if (confidence[which].Valid())
      {
       for (nat32 y=0;y<disp[which].Size(1);y++)
       {
        for (nat32 x=0;x<disp[which].Size(0);x++)
        {
         confidence[which].Get(x,y) = fg.Get(dispV,y*width+x).Confidence();
        }
       }   
      }
    }
    else
    {
     // Create the differential factor fields...
      inf::GridDiff2D * dx = smoothOverride?null<inf::GridDiff2D*>():new inf::GridDiff2D(false);
      inf::GridDiff2D * dy = smoothOverride?null<inf::GridDiff2D*>():new inf::GridDiff2D(true);

      if (!smoothOverride)
      {
       dx->SetLambda(diffErrLevel);
       dy->SetLambda(diffErrLevel);
      }


     // Create the differential smoothing fields...
      inf::GridSmoothLaplace2D * sDxX = new inf::GridSmoothLaplace2D(false);
      inf::GridSmoothLaplace2D * sDxY = new inf::GridSmoothLaplace2D(true);
      inf::GridSmoothLaplace2D * sDyX = smoothOverride?null<inf::GridSmoothLaplace2D*>():(new inf::GridSmoothLaplace2D(false));
      inf::GridSmoothLaplace2D * sDyY = smoothOverride?null<inf::GridSmoothLaplace2D*>():(new inf::GridSmoothLaplace2D(true));

      sDxX->SetMS(smoothMult,smoothMax);
      sDxY->SetMS(smoothMult,smoothMax);
      if (!smoothOverride)
      {
       sDyX->SetMS(smoothMult,smoothMax);
       sDyY->SetMS(smoothMult,smoothMax);
      }


     // Wire the entire mess up...
      nat32 dsiH = fg.NewFP(dsi);
      nat32 dxH = smoothOverride?0:fg.NewFP(dx);
      nat32 dyH = smoothOverride?0:fg.NewFP(dy);
      nat32 sDxXH = fg.NewFP(sDxX);
      nat32 sDxYH = fg.NewFP(sDxY);
      nat32 sDyXH = smoothOverride?0:fg.NewFP(sDyX);
      nat32 sDyYH = smoothOverride?0:fg.NewFP(sDyY);

      nat32 dispV = fg.NewVP();
      nat32 dispDxV = smoothOverride?0:fg.NewVP();
      nat32 dispDyV = smoothOverride?0:fg.NewVP();
  
      fg.Lay(dsiH,0,dispV);

      if (!smoothOverride)
      {
       fg.Lay(dxH,0,dispV);
       fg.Lay(dyH,0,dispV);
       fg.Lay(dxH,1,dispDxV);
       fg.Lay(dyH,1,dispDyV);
      }

      if (smoothOverride)
      {
       fg.Lay(sDxXH,0,dispV);
       fg.Lay(sDxYH,0,dispV);
      }
      else
      {
       fg.Lay(sDxXH,0,dispDxV);
       fg.Lay(sDxYH,0,dispDxV);
       fg.Lay(sDyXH,0,dispDyV);
       fg.Lay(sDyYH,0,dispDyV);
      }


     // Solve this mother of a factor graph...
      prog->Report(1,3);
      fg.Run(prog);


     // Extract the resulting disparity...
      prog->Report(2,3);
      for (nat32 y=0;y<disp[which].Size(1);y++)
      {
       for (nat32 x=0;x<disp[which].Size(0);x++)
       {
        disp[which].Get(x,y) = fg.Get(dispV,y*width+x).Maximum() + localMinDisp;
       }
      }
  
     // If wanted extract the disparity confidence...
      if (confidence[which].Valid())
      {
       for (nat32 y=0;y<disp[which].Size(1);y++)
       {
        for (nat32 x=0;x<disp[which].Size(0);x++)
        {
         confidence[which].Get(x,y) = fg.Get(dispV,y*width+x).Confidence();
        }
       }   
      }
  
     // If wanted extract the resulting differentials of disparity...
      if (dispDx[which].Valid()&&(!smoothOverride))
      {
       for (nat32 y=0;y<height;y++)
       {
        for (nat32 x=0;x<width;x++)
        {
         dispDx[which].Get(x,y) = fg.Get(dispDxV,y*width+x).Maximum() - labels + 1;
        }
       }
      }

      if (dispDy[which].Valid()&&(!smoothOverride))
      {
       for (nat32 y=0;y<height;y++)
       {
        for (nat32 x=0;x<width;x++)
        {
         dispDy[which].Get(x,y) = fg.Get(dispDyV,y*width+x).Maximum() - labels + 1;
        }
       }
      }
    }
  }

 prog->Pop();
}

//------------------------------------------------------------------------------
SfgsAlbedo1::SfgsAlbedo1()
:labels(100),
albedoSd(3.0),albedoMinCorruption(0.2),albedoCorruptionAngle(math::pi*0.1),
neighbourMinProb(1.1),neighbourMaxProb(2.0),neighbourAlpha(0.5),neighbourBeta(0.5),
toLight(0.0,0.0,1.0)
{}

SfgsAlbedo1::~SfgsAlbedo1()
{}

void SfgsAlbedo1::SetLabelCount(nat32 l)
{
 labels = l;
}

void SfgsAlbedo1::SetAlbedoParas(real32 sd,real32 minCorruption,real32 corruptionAngle)
{
 albedoSd = sd;
 albedoMinCorruption = minCorruption;
 albedoCorruptionAngle = corruptionAngle;
}

void SfgsAlbedo1::SetNeighbourParas(real32 minProb,real32 maxProb,real32 alpha,real32 beta)
{
 neighbourMaxProb = maxProb;
 neighbourMinProb = minProb;
 neighbourAlpha = alpha;
 neighbourBeta = beta;
}

void SfgsAlbedo1::SetLight(const bs::Normal & tl)
{
 toLight = tl;
}

void SfgsAlbedo1::SetIr(const svt::Field<bs::ColourLuv> & i)
{
 ir = i;
}

void SfgsAlbedo1::SetNeedle(const svt::Field<bs::Normal> & n)
{
 needle = n;
}

void SfgsAlbedo1::SetAlbedo(svt::Field<real32> & a)
{
 albedo = a;
}

void SfgsAlbedo1::Run(time::Progress * prog)
{
 LogBlock("void SfgsAlbedo1::Run(...)","-");
 prog->Push();
 prog->Report(0,3);
 prog->Push();
 prog->Report(0,4);

 // Simple stuff, work out the mapping from labels to albedo values...
  // Multiply a label index to get an albedo, noting to add 1 to the 
  // label first as 0 is not suported.
   real32 albMult = 100.0/real32(labels);
  
  nat32 width = albedo.Size(0);
  nat32 height = albedo.Size(1);


 // Create a field graph object...
  inf::FieldGraph fg(true);
  
 
 // Create the distribution field...
  prog->Report(1,4);
  inf::GridFreq2D * dist = new inf::GridFreq2D(labels,width,height);
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    real32 lightNormDot = toLight * needle.Get(x,y);
    if (math::IsZero(lightNormDot)||(lightNormDot<0.0))
    {
     for (nat32 l=0;l<labels;l++) dist->SetFreq(x,y,l,1.0);
    }
    else
    {
     // Calculate the expected albedo...
      real32 expAlb = ir.Get(x,y).l/lightNormDot;     

     // Calculate the less-than albedo corruption level...
      real32 lowCorruption = albedoMinCorruption;
      real32 angChange = math::InvCos(lightNormDot);
      if (angChange<albedoCorruptionAngle)
      {
       lowCorruption += (1.0 - lowCorruption)*(1.0 - math::Sqr(angChange/albedoCorruptionAngle));
      }

     // Set all the labels frequencies...
      for (nat32 l=0;l<labels;l++)
      {
       real32 alb = (l+1.0)/albMult;
       real32 p = (alb<expAlb)?lowCorruption:albedoMinCorruption;
       p = math::Max(p,math::Gaussian<real32>(albedoSd,alb-expAlb));
       dist->SetFreq(x,y,l,p);
      }
    }
   }
  }


 // Create the smoothing fields...
  prog->Report(2,4);
  inf::GridSmoothPotts2D * dx = new inf::GridSmoothPotts2D(false,false,width,height);  
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width-1;x++)
   {
    real32 probEqual = neighbourMaxProb;
    probEqual -= neighbourAlpha * math::InvCos(needle.Get(x,y) * needle.Get(x+1,y));
    real32 uDiff = ir.Get(x,y).u - ir.Get(x+1,y).u;
    real32 vDiff = ir.Get(x,y).v - ir.Get(x+1,y).v;
    probEqual -= neighbourBeta * math::Sqrt(math::Sqr(uDiff)+math::Sqr(vDiff));
    dx->Set(x,y,math::Max(probEqual,neighbourMinProb));
   }
  }

  inf::GridSmoothPotts2D * dy = new inf::GridSmoothPotts2D(true,false,width,height);
  for (nat32 y=0;y<height-1;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    real32 probEqual = neighbourMaxProb;
    probEqual -= neighbourAlpha * math::InvCos(needle.Get(x,y) * needle.Get(x,y+1));
    real32 uDiff = ir.Get(x,y).u - ir.Get(x,y+1).u;
    real32 vDiff = ir.Get(x,y).v - ir.Get(x,y+1).v;
    probEqual -= neighbourBeta * math::Sqrt(math::Sqr(uDiff)+math::Sqr(vDiff));
    dy->Set(x,y,math::Max(probEqual,neighbourMinProb));
   }
  }


 // Convert the functions, create the variable field, link it all together...
  prog->Report(3,4);
  nat32 diffHand = fg.NewFP(dist);
  nat32 dxHand = fg.NewFP(dx);
  nat32 dyHand = fg.NewFP(dy);
  
  nat32 var = fg.NewVP();
  
  fg.Lay(diffHand,0,var);
  fg.Lay(dxHand,0,var);
  fg.Lay(dyHand,0,var);


 // Solve...
  prog->Pop();
  prog->Report(1,3);
  fg.Run(prog);


 // Extract the albedo map...
  prog->Report(2,3);
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    inf::Frequency freq = fg.Get(var,y*width+x);    
    albedo.Get(x,y) = freq.Maximum() * albMult;
   }
  }


 prog->Pop();
}

//------------------------------------------------------------------------------
Sfgs1::Sfgs1()
:iCount(3),
dcLeft(null<cam::DispConv*>()),dcRight(null<cam::DispConv*>()),
needleSmooth(0.0)
{}

Sfgs1::~Sfgs1()
{
 delete dcLeft;
 delete dcRight;
}

void Sfgs1::SetDispRange(int32 minDisp,int32 maxDisp)
{
 disp.SetDispRange(minDisp,maxDisp);
}

void Sfgs1::SetMatchParas(real32 alphaMult,real32 alphaMax,real32 betaMult,real32 betaMax,real32 gammaMult,real32 gammaMax,real32 deltaMult,real32 deltaMax,real32 oobd,real32 max)
{
 disp.SetMatchParas(alphaMult,alphaMax,betaMult,betaMax,gammaMult,gammaMax,deltaMult,deltaMax,oobd,max);
}

void Sfgs1::SetDiffParas(real32 errLevel)
{
 disp.SetDiffParas(errLevel);
}

void Sfgs1::SetSmoothParas(real32 smoothMult,real32 smoothMax)
{
 disp.SetSmoothParas(smoothMult,smoothMax);
}

void Sfgs1::EnableSmoothOverride()
{
 disp.SmoothOverride(true);
}

void Sfgs1::EnableAlbedoSmoothOverride(real32 cost,nat32 angRes,bit brute,nat32 irrRes)
{
 disp.SmoothAlbedo(true,cost,angRes,brute,irrRes);
}

void Sfgs1::EnableSmoothOrient(nat32 subDivs,real32 maxCost,real32 angMult,real32 eqCost)
{
 disp.SmoothOrient(true,subDivs,maxCost,angMult,eqCost);
}

void Sfgs1::EnableSmoothOrientEx(real32 angAlias,real32 maxCost,nat32 angRes)
{
 disp.SmoothOrientSFS(true,angAlias,maxCost,angRes);
}

void Sfgs1::SetAlbedoLabelCount(nat32 labels)
{
 alb.SetLabelCount(labels);
}

void Sfgs1::SetAlbedoParas(real32 sd,real32 minCorruption,real32 corruptionAngle)
{
 alb.SetAlbedoParas(sd,minCorruption,corruptionAngle);
}

void Sfgs1::SetNeighbourParas(real32 minProb,real32 maxProb,real32 alpha,real32 beta)
{
 alb.SetNeighbourParas(minProb,maxProb,alpha,beta);
}

void Sfgs1::SetNeedleSmooth(real32 sd)
{
 needleSmooth = sd;
}

void Sfgs1::SetIr(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right)
{
 leftIr = left;
 rightIr = right;
 
 disp.SetIr(left,right);
}

void Sfgs1::SetLight(const bs::Normal & left,const bs::Normal & right)
{
 leftLight = left;
 rightLight = right;
 disp.SetLight(left,right);
}

void Sfgs1::SetAlbedo(svt::Field<real32> & left,svt::Field<real32> & right)
{
 leftAlbedo = left;
 rightAlbedo = right;
 disp.SetAlbedo(left,right);
}

void Sfgs1::SetDisp(svt::Field<real32> & left,svt::Field<real32> & right)
{
 leftDisp = left;
 rightDisp = right;
 disp.SetDisp(left,right);
}

void Sfgs1::SetNeedle(svt::Field<bs::Normal> & left,svt::Field<bs::Normal> & right)
{
 leftNeedle = left;
 rightNeedle = right;
}

void Sfgs1::SetConfidence(svt::Field<real32> & left,svt::Field<real32> & right)
{
 disp.SetConfidence(left,right);
}

void Sfgs1::SetDispDx(svt::Field<real32> & left,svt::Field<real32> & right)
{
 disp.SetDispDx(left,right);
}

void Sfgs1::SetDispDy(svt::Field<real32> & left,svt::Field<real32> & right)
{
 disp.SetDispDy(left,right);
}

void Sfgs1::SetIterCount(nat32 iC)
{
 iCount = iC;
}

void Sfgs1::SetNeedleCreator(cam::DispConv * dcL,cam::DispConv * dcR)
{
 delete dcLeft;  dcLeft = dcL;
 delete dcRight; dcRight = dcR;
 disp.SetDispConv(*dcLeft,*dcRight);
}

void Sfgs1::Run(time::Progress * prog)
{
 LogBlock("void Sfgs1::Run(...)","-");
 prog->Push();
  // Initialise the albedo map to be white...
   prog->Report(0,iCount*2+2);
   for (nat32 y=0;y<leftAlbedo.Size(1);y++)
   {
    for (nat32 x=0;x<leftAlbedo.Size(0);x++) leftAlbedo.Get(x,y) = 100.0;
   }

   for (nat32 y=0;y<rightAlbedo.Size(1);y++)
   {
    for (nat32 x=0;x<rightAlbedo.Size(0);x++) rightAlbedo.Get(x,y) = 100.0;
   }


  // Do the iterations, shape estmation then albedo estimation...
   for (nat32 i=0;i<iCount;i++)
   {
    prog->Report(i*2+1,iCount*2+2);
    disp.Run(prog);

    prog->Report(i*2+2,iCount*2+2);
    {
     prog->Push();
      prog->Report(0,3);
       prog->Push();
        prog->Report(0,2);
         dcLeft->Convert(leftDisp,leftNeedle);
         if (!math::IsZero(needleSmooth))
         {
          svt::Field<real32> nx; leftNeedle.SubField(sizeof(real32)*0,nx);
          svt::Field<real32> ny; leftNeedle.SubField(sizeof(real32)*1,ny);
          svt::Field<real32> nz; leftNeedle.SubField(sizeof(real32)*2,nz);
          
          filter::KernelVect kernel = filter::KernelVect(nat32(math::RoundUp(needleSmooth*2.0)));
          kernel.MakeGaussian(needleSmooth);
          
          kernel.Apply(nx,nx);
          kernel.Apply(ny,ny);
          kernel.Apply(nz,nz);
          
          for (nat32 y=0;y<leftNeedle.Size(1);y++)
          {
           for (nat32 x=0;x<leftNeedle.Size(0);x++)
           {
            leftNeedle.Get(x,y).Normalise();
           }
          }
         }
        prog->Report(1,2);
         dcRight->Convert(rightDisp,rightNeedle);
         if (!math::IsZero(needleSmooth))
         {
          svt::Field<real32> nx; rightNeedle.SubField(sizeof(real32)*0,nx);
          svt::Field<real32> ny; rightNeedle.SubField(sizeof(real32)*1,ny);
          svt::Field<real32> nz; rightNeedle.SubField(sizeof(real32)*2,nz);
          
          filter::KernelVect kernel = filter::KernelVect(nat32(math::RoundUp(needleSmooth*2.0)));
          kernel.MakeGaussian(needleSmooth);
          
          kernel.ApplyRepeat(nx,nx);
          kernel.ApplyRepeat(ny,ny);
          kernel.ApplyRepeat(nz,nz);
          
          for (nat32 y=0;y<rightNeedle.Size(1);y++)
          {
           for (nat32 x=0;x<rightNeedle.Size(0);x++)
           {
            rightNeedle.Get(x,y).Normalise();
           }
          }
         }        
       prog->Pop();

      prog->Report(1,3);
       alb.SetLight(leftLight);
       alb.SetIr(leftIr);
       alb.SetNeedle(leftNeedle);
       alb.SetAlbedo(leftAlbedo);
       alb.Run(prog);      
      prog->Report(2,3);
       alb.SetLight(rightLight);
       alb.SetIr(rightIr);
       alb.SetNeedle(rightNeedle);
       alb.SetAlbedo(rightAlbedo);
       alb.Run(prog);           
     
     prog->Pop();
    }
   }


  // Final shape estimation - its the last thing we do before calling it a wrap...
   prog->Report(iCount*2+1,iCount*2+2);
   if (iCount==0) disp.SetNeedle(leftNeedle,rightNeedle);
   disp.Run(prog);
 
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
