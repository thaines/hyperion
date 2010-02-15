//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/filter/seg_k_mean_grid.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
MeanGridSeg::MeanGridSeg()
:dim(12),colMult(2.0),spatialMult(1.0),maxIters(1000),segments(0)
{}

MeanGridSeg::~MeanGridSeg()
{}

void MeanGridSeg::SetImage(const svt::Field<bs::ColourLuv> & in)
{
 image = in;
}

void MeanGridSeg::SetSize(nat32 d)
{
 dim = d;
}

void MeanGridSeg::SetDist(real32 colM,real32 spatialM)
{
 colMult = colM;
 spatialMult = spatialM;
}

void MeanGridSeg::SetMaxIters(nat32 iters)
{
 maxIters = iters;
}

void MeanGridSeg::Run(time::Progress * prog)
{
 prog->Push();
 
 
 
 prog->Pop();
}

nat32 MeanGridSeg::Segments() const
{
 return segments;
}

void MeanGridSeg::GetSegments(svt::Field<nat32> & o) const
{
 for (nat32 y=0;y<o.Size(1);y++)
 {
  for (nat32 x=0;x<o.Size(0);x++) o.Get(x,y) = out.Get(x,y);
 }
}

//------------------------------------------------------------------------------
 };
};
