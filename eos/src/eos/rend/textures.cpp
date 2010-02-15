//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/rend/textures.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
ConstantTexture::~ConstantTexture()
{}

bit ConstantTexture::Suports(TextureType t) const
{
 return t==type;
}

void ConstantTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 out = value;
}

void ConstantTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 out = colour;
}

void ConstantTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 out = offset;
}

void ConstantTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 out = vector;
}

//------------------------------------------------------------------------------
AddTexture::~AddTexture()
{}

nat32 AddTexture::Parameters() const
{
 return 2;
}

TextureType AddTexture::ParaType(nat32 index) const
{
 return type;
}

cstrconst AddTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "a";
  case 1: return "b";
  default: return "error";
 }
}

void AddTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: a = rhs; break;
  case 1: b = rhs; break; 
 }
}

bit AddTexture::Suports(TextureType t) const
{
 return type==t;
}

void AddTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = 0.0;
  return; 
 }

 real32 temp;
 a->GetValue(coord,inDir,norm,temp);
 b->GetValue(coord,inDir,norm,out);
 out += temp;
}

void AddTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = bs::ColourRGB(0.0,0.0,0.0);
  return; 
 }

 bs::ColourRGB temp;
 a->GetColour(coord,inDir,norm,temp);
 b->GetColour(coord,inDir,norm,out);
 out += temp;
}

void AddTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = bs::Pnt(0.0,0.0);
  return; 
 }

 bs::Pnt temp;
 a->GetOffset(coord,inDir,norm,temp);
 b->GetOffset(coord,inDir,norm,out);
 out += temp;
}

void AddTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = bs::Vert(0.0,0.0,0.0);
  return; 
 }

 bs::Vert temp;
 a->GetVector(coord,inDir,norm,temp);
 b->GetVector(coord,inDir,norm,out);
 out += temp;
}

//------------------------------------------------------------------------------
MultTexture::~MultTexture()
{}

nat32 MultTexture::Parameters() const
{
 return 2;
}

TextureType MultTexture::ParaType(nat32 index) const
{
 return type;
}

cstrconst MultTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "a";
  case 1: return "b";
  default: return "error";
 }
}

void MultTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: a = rhs; break;
  case 1: b = rhs; break; 
 }
}

bit MultTexture::Suports(TextureType t) const
{
 switch (type)
 {
  case TextureOffset: return t==TextureValue;
  case TextureVector: return (t==TextureValue)||(t==TextureVector);
  default: return type==t;
 }
}

void MultTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = 0.0;
  return; 
 }

 if (type==TextureOffset)
 {
  bs::Pnt temp1;
  bs::Pnt temp2;
  a->GetOffset(coord,inDir,norm,temp1);
  b->GetOffset(coord,inDir,norm,temp2);
  out = temp1 * temp2;
  return;
 }
 
 if (type==TextureVector)
 {
  bs::Vert temp1;
  bs::Vert temp2;
  a->GetVector(coord,inDir,norm,temp1);
  b->GetVector(coord,inDir,norm,temp2);
  out = temp1 * temp2;
  return; 
 }
 
 real32 temp;
 a->GetValue(coord,inDir,norm,temp);
 b->GetValue(coord,inDir,norm,out);
 out *= temp;
}

void MultTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = bs::ColourRGB(0.0,0.0,0.0);
  return; 
 }

 bs::ColourRGB temp;
 a->GetColour(coord,inDir,norm,temp);
 b->GetColour(coord,inDir,norm,out);
 out *= temp;
}

void MultTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 out = bs::Pnt(0.0,0.0);
}

void MultTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>()))
 {
  out = bs::Vert(0.0,0.0,0.0);
  return; 
 }

 bs::Vert temp1;
 bs::Vert temp2;
 a->GetVector(coord,inDir,norm,temp1);
 b->GetVector(coord,inDir,norm,temp2);
 math::CrossProduct(temp1,temp2,out);
}

//------------------------------------------------------------------------------
LinearMixTexture::~LinearMixTexture()
{}

nat32 LinearMixTexture::Parameters() const
{
 return 3;
}

TextureType LinearMixTexture::ParaType(nat32 index) const
{
 switch (index)
 {
  case 2: return TextureValue;
  default: return type;
 }
}

cstrconst LinearMixTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "a";
  case 1: return "b";
  case 2: return "mix";
  default: return "error";
 }
}

void LinearMixTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: a = rhs; break;
  case 1: b = rhs; break; 
  case 2: mix = rhs; break;
 }
}

bit LinearMixTexture::Suports(TextureType t) const
{
 return type==t;
}

void LinearMixTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>())||(mix==null<Texture*>()))
 {
  out = 0.0;
  return; 
 }

 real32 av;
 real32 bv;
 real32 mv;

 a->GetValue(coord,inDir,norm,av);
 b->GetValue(coord,inDir,norm,bv);
 mix->GetValue(coord,inDir,norm,mv);

 out = av*(1.0-mv) + bv*mv;
}

void LinearMixTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>())||(mix==null<Texture*>()))
 {
  out = bs::ColourRGB(0.0,0.0,0.0);
  return; 
 }

 bs::ColourRGB av;
 bs::ColourRGB bv;
 real32 mv;
 
 a->GetColour(coord,inDir,norm,av);
 b->GetColour(coord,inDir,norm,bv);
 mix->GetValue(coord,inDir,norm,mv);
 
 av *= 1.0-mv;
 bv *= mv;
 out = av;
 out += bv;
}

void LinearMixTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>())||(mix==null<Texture*>()))
 {
  out = bs::Pnt(0.0,0.0);
  return; 
 }

 bs::Pnt av;
 bs::Pnt bv;
 real32 mv;
 
 a->GetOffset(coord,inDir,norm,av);
 b->GetOffset(coord,inDir,norm,bv);
 mix->GetValue(coord,inDir,norm,mv);

 av *= 1.0-mv;
 bv *= mv;
 out = av;
 out += bv;
}

void LinearMixTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 if ((a==null<Texture*>())||(b==null<Texture*>())||(mix==null<Texture*>()))
 {
  out = bs::Vert(0.0,0.0,0.0);
  return; 
 }

 bs::Vert av;
 bs::Vert bv;
 real32 mv;
 
 a->GetVector(coord,inDir,norm,av);
 b->GetVector(coord,inDir,norm,bv);
 mix->GetValue(coord,inDir,norm,mv);
 
 av *= 1.0-mv;
 bv *= mv;
 out = av;
 out += bv;
}

//------------------------------------------------------------------------------
ToColourTexture::~ToColourTexture()
{}

nat32 ToColourTexture::Parameters() const
{
 return 3;
}

TextureType ToColourTexture::ParaType(nat32 index) const
{
 return TextureValue;
}

cstrconst ToColourTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "r";
  case 1: return "g";
  case 2: return "b";
  default: return "error";
 }
}

void ToColourTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: r = rhs; break;
  case 1: g = rhs; break; 
  case 2: b = rhs; break;
 }
}

bit ToColourTexture::Suports(TextureType t) const
{
 return TextureColour==t;
}

void ToColourTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 out = 0.0;
}

void ToColourTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 if ((r==null<Texture*>())||(g==null<Texture*>())||(b==null<Texture*>()))
 {
  out = bs::ColourRGB(0.0,0.0,0.0);
  return; 
 }

 r->GetValue(coord,inDir,norm,out.r);
 g->GetValue(coord,inDir,norm,out.g);
 b->GetValue(coord,inDir,norm,out.b); 
}

void ToColourTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 out = bs::Pnt(0.0,0.0);
}

void ToColourTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 out = bs::Vert(0.0,0.0,0.0);
}

//------------------------------------------------------------------------------
FromColourTexture::~FromColourTexture()
{}

nat32 FromColourTexture::Parameters() const
{
 return 1;
}

TextureType FromColourTexture::ParaType(nat32 index) const
{
 return TextureColour;
}

cstrconst FromColourTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "in";
  default: return "error";
 }
}

void FromColourTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: in = rhs; break;
 }
}

bit FromColourTexture::Suports(TextureType t) const
{
 return TextureValue==t;
}

void FromColourTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 if (in==null<Texture*>())
 {
  out = 0.0;
  return;
 }
 
 bs::ColourRGB col;
 in->GetColour(coord,inDir,norm,col);
 
 switch (mode)
 {
  case ModeR: out = col.r; break;
  case ModeG: out = col.g; break;
  case ModeB: out = col.b; break;
  case ModeL: 
  {
   bs::ColourL lum = col;
   out = lum.l;
  } 
  break;
 }
}

void FromColourTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 out = bs::ColourRGB(0.0,0.0,0.0);
}

void FromColourTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 out = bs::Pnt(0.0,0.0);
}

void FromColourTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 out = bs::Vert(0.0,0.0,0.0);
}

//------------------------------------------------------------------------------
TransformTexture::TransformTexture()
:in(null<Texture*>())
{
 Identity(matrix);
}

TransformTexture::~TransformTexture()
{}

nat32 TransformTexture::Parameters() const
{
 return 1;
}

TextureType TransformTexture::ParaType(nat32 index) const
{
 return TextureAny;
}

cstrconst TransformTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "in";
  default: return "error";
 }
}

void TransformTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: in = rhs; break;
 }
}

bit TransformTexture::Suports(TextureType t) const
{
 if (in==null<Texture*>()) return true;
                      else return in->Suports(t);
}

void TransformTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 if (in==null<Texture*>())
 {
  out = 0.0;
  return; 
 }
 
 TexCoord nc;
 math::MultVectEH(matrix,coord,nc);
 nc.material = coord.material;

 in->GetValue(nc,inDir,norm,out);
}

void TransformTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 if (in==null<Texture*>())
 {
  out = bs::ColourRGB(0.0,0.0,0.0);
  return; 
 }
 
 TexCoord nc;
 math::MultVectEH(matrix,coord,nc);
 nc.material = coord.material;

 in->GetColour(nc,inDir,norm,out);
}

void TransformTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 if (in==null<Texture*>())
 {
  out = bs::Pnt(0.0,0.0);
  return; 
 }

 TexCoord nc;
 math::MultVectEH(matrix,coord,nc);
 nc.material = coord.material;

 in->GetOffset(nc,inDir,norm,out);
}

void TransformTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 if (in==null<Texture*>())
 {
  out = bs::Vert(0.0,0.0,0.0);
  return; 
 }

 TexCoord nc;
 math::MultVectEH(matrix,coord,nc);
 nc.material = coord.material;

 in->GetVector(nc,inDir,norm,out);
}

//------------------------------------------------------------------------------
DistortionTexture::DistortionTexture(TextureType d)
:dt(d),in(null<Texture*>()),distort(null<Texture*>())
{}

DistortionTexture::~DistortionTexture()
{}

nat32 DistortionTexture::Parameters() const
{
 return 2;
}

TextureType DistortionTexture::ParaType(nat32 index) const
{
 switch (index)
 {
  case 1: return dt;
  default: return TextureAny;  
 }
}

cstrconst DistortionTexture::ParaName(nat32 index) const
{
 switch (index)
 {
  case 0: return "in";
  case 1: return "distortion";
  default: return "error";
 }
}

void DistortionTexture::ParaSet(nat32 index,class Texture * rhs)
{
 switch (index)
 {
  case 0: in = rhs; break;
  case 1: distort = rhs; break;
 }
}

bit DistortionTexture::Suports(TextureType t) const
{
 if (in==null<Texture*>()) return true;
                      else return in->Suports(t);
}

void DistortionTexture::GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const
{
 if ((in==null<Texture*>())||(distort==null<Texture*>()))
 {
  out = 0.0;
  return; 
 }
 
 TexCoord nc = coord;
 switch (dt)
 {
  case TextureValue:
  {
   real32 dq;
   distort->GetValue(coord,inDir,norm,dq);
   nc[0] += dq;
  }
  break;
  case TextureOffset:
  {
   bs::Pnt dq;
   distort->GetOffset(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
  }
  break;
  case TextureVector:
  {
   bs::Vert dq;
   distort->GetVector(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
   nc[2] += dq[2];
  }
  break; 
  default: break;
 }

 in->GetValue(nc,inDir,norm,out);
}

void DistortionTexture::GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const
{
 if ((in==null<Texture*>())||(distort==null<Texture*>()))
 {
  out = bs::ColourRGB(0.0,0.0,0.0);
  return; 
 }
 
 TexCoord nc = coord;
 switch (dt)
 {
  case TextureValue:
  {
   real32 dq;
   distort->GetValue(coord,inDir,norm,dq);
   nc[0] += dq;
  }
  break;
  case TextureOffset:
  {
   bs::Pnt dq;
   distort->GetOffset(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
  }
  break;
  case TextureVector:
  {
   bs::Vert dq;
   distort->GetVector(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
   nc[2] += dq[2];
  }
  break;
  default: break;
 }

 in->GetColour(nc,inDir,norm,out);
}

void DistortionTexture::GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const
{
 if ((in==null<Texture*>())||(distort==null<Texture*>()))
 {
  out = bs::Pnt(0.0,0.0);
  return; 
 }

 TexCoord nc = coord;
 switch (dt)
 {
  case TextureValue:
  {
   real32 dq;
   distort->GetValue(coord,inDir,norm,dq);
   nc[0] += dq;
  }
  break;
  case TextureOffset:
  {
   bs::Pnt dq;
   distort->GetOffset(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
  }
  break;
  case TextureVector:
  {
   bs::Vert dq;
   distort->GetVector(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
   nc[2] += dq[2];
  }
  break;
  default: break;
 }

 in->GetOffset(nc,inDir,norm,out);
}

void DistortionTexture::GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const
{
 if ((in==null<Texture*>())||(distort==null<Texture*>()))
 {
  out = bs::Vert(0.0,0.0,0.0);
  return; 
 }

 TexCoord nc = coord;
 switch (dt)
 {
  case TextureValue:
  {
   real32 dq;
   distort->GetValue(coord,inDir,norm,dq);
   nc[0] += dq;
  }
  break;
  case TextureOffset:
  {
   bs::Pnt dq;
   distort->GetOffset(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
  }
  break;
  case TextureVector:
  {
   bs::Vert dq;
   distort->GetVector(coord,inDir,norm,dq);
   nc[0] += dq[0];
   nc[1] += dq[1];
   nc[2] += dq[2];
  }
  break;
  default: break;
 }

 in->GetVector(nc,inDir,norm,out);
}

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
