//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/rend/visualise.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
EOS_FUNC void VisNeedleMap(const svt::Field<bs::Normal> & in,svt::Field<bs::ColourRGB> & out,bit rev)
{
 for (nat32 y=0;y<in.Size(1);y++)
 {
  for (nat32 x=0;x<in.Size(0);x++)
  {
   if (rev)
   {
    if (in.Get(x,y).Z()>0.0)
    {
     out.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
    }
    else
    {
     out.Get(x,y).r = math::Clamp<real32>(0.5*(in.Get(x,y).X()+1.0),0.0,1.0);
     out.Get(x,y).g = math::Clamp<real32>(0.5*(in.Get(x,y).Y()+1.0),0.0,1.0);
     out.Get(x,y).b = math::Clamp<real32>(-in.Get(x,y).Z(),0.0,1.0);
    }   
   }
   else
   {
    if (in.Get(x,y).Z()<0.0)
    {
     out.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
    }
    else
    {
     out.Get(x,y).r = math::Clamp<real32>(0.5*(in.Get(x,y).X()+1.0),0.0,1.0);
     out.Get(x,y).g = math::Clamp<real32>(0.5*(in.Get(x,y).Y()+1.0),0.0,1.0);
     out.Get(x,y).b = math::Clamp<real32>(in.Get(x,y).Z(),0.0,1.0);
    }
   }
  }
 }
}

EOS_FUNC void NeedleMapToModel(const svt::Field<bs::Normal> & in,file::Wavefront & out)
{
 real32 mult = 1.0/in.Size(0);
 real32 halfMult = 0.4*mult; // Not actually half, to leave a gap between squares.
 
 for (nat32 y=0;y<in.Size(1);y++)
 {
  for (nat32 x=0;x<in.Size(0);x++)
  {
   real32 pos[2];
    pos[0] = real32(x)*mult - 0.5;
    pos[1] = real32(y)*mult - 0.5;
  
   // Create corners of face...
    nat32 vert[4];
    bs::Vert v[4];
     v[0][0] = -halfMult; v[0][1] = -halfMult; v[0][2] = 0.0;
     v[1][0] = +halfMult; v[1][1] = -halfMult; v[1][2] = 0.0;
     v[2][0] = +halfMult; v[2][1] = +halfMult; v[2][2] = 0.0;
     v[3][0] = -halfMult; v[3][1] = +halfMult; v[3][2] = 0.0;


    bs::Normal nx;
    bs::Normal ny;
    bs::Normal nz = in.Get(x,y);

    nx[0] = nz[2];
    nx[1] = nz[0];
    nx[2] = nz[1];
    nat32 nxi;
    if (math::Abs(nx[1])>math::Abs(nx[0])) nxi = 1;
                                      else nxi = 0;
    if (math::Abs(nx[2])>math::Abs(nx[nxi])) nxi = 2;
    nx[nxi] = -nx[nxi];
    
    math::CrossProduct(nz,nx,ny);
    math::CrossProduct(ny,nz,nx);
    nx.Normalise();
    ny.Normalise();

    math::Mat<3> mult;
    mult[0][0] = nx[0]; mult[0][1] = ny[0]; mult[0][2] = nz[0];
    mult[1][0] = nx[1]; mult[1][1] = ny[1]; mult[1][2] = nz[1];
    mult[2][0] = nx[2]; mult[2][1] = ny[2]; mult[2][2] = nz[2];
        

    for (nat32 i=0;i<4;i++)
    {
     bs::Vert temp;
     math::MultVect(mult,v[i],temp);
     temp[0] += pos[0];
     temp[1] += pos[1];
     vert[i] = out.Add(temp);
    }

   // Create face...
    for (nat32 i=0;i<4;i++) out.Add(vert[i]);
    out.Face();
  }
 }
}

//------------------------------------------------------------------------------
 };
};
