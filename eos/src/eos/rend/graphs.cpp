//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/rend/graphs.h"

#include "eos/bs/colours.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
EOS_FUNC svt::Var * RenderSimpleGraph(svt::Core & core,math::Vector<real32> * samples)
{ 
 // Create the db...
  bs::ColourRGB rgbIni(1.0,1.0,1.0);

  svt::Var * ret = new svt::Var(core);
  ret->Setup2D(samples->Size(),100);
  ret->Add("rgb",rgbIni);
  ret->Commit();
  
  svt::Field<bs::ColourRGB> image(ret,"rgb");


 // Find maximum...
  real32 max = 0.0;
  for (nat32 i=0;i<samples->Size();i++) max = math::Max(max,(*samples)[i]);
  real32 mult = 99.0/max;


 // Render...
  for (nat32 i=0;i<samples->Size();i++)
  {
   int32 top = math::Min(int32(math::Round((*samples)[i] * mult)),int32(99));
   for (int32 j=0;j<top;j++) image.Get(i,j) = bs::ColourRGB(real32(j)/real32(top-1),0.0,0.0);
  }


 // Return...
  return ret;
}

//------------------------------------------------------------------------------
 };
};
