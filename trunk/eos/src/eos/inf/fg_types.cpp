//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/inf/fg_types.h"

#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
void Frequency::MakeClass(nat32 labels,MessageClass & out)
{
 out.scale = labels;
 out.Size = &Frequency::Size;
 out.Flatline = &Frequency::Flatline;
 out.FlatlineLn = &Frequency::FlatlineLn;
 out.Mult = &Frequency::Mult;
 out.InplaceMult = &Frequency::InplaceMult;
 out.InplaceAdd = &Frequency::InplaceAdd;
 out.InplaceNeg = &Frequency::InplaceNeg;
 out.FromNegLn = &Frequency::FromNegLn;
 out.Norm = &Frequency::Norm;
 out.Drop = &Frequency::Drop;
}

nat32 Frequency::Size(const MessageClass & mc)
{
 return sizeof(real32)*mc.scale;
}

void Frequency::Flatline(const MessageClass & mc,void * obj)
{
 real32 * targ = static_cast<real32*>(obj);

 for (nat32 i=0;i<mc.scale;i++) targ[i] = 1.0;
}

void Frequency::FlatlineLn(const MessageClass & mc,void * obj)
{
 real32 * targ = static_cast<real32*>(obj);
 
 for (nat32 i=0;i<mc.scale;i++) targ[i] = 0.0;
}

void Frequency::Mult(const MessageClass & mc,void * objA,void * objB,void * objC)
{
 real32 * targA = static_cast<real32*>(objA);
 real32 * targB = static_cast<real32*>(objB);
 real32 * targC = static_cast<real32*>(objC);  

 for (nat32 i=0;i<mc.scale;i++) targA[i] = targB[i]*targC[i];
}

void Frequency::InplaceMult(const MessageClass & mc,void * objA,void * objB)
{
 real32 * targA = static_cast<real32*>(objA);
 real32 * targB = static_cast<real32*>(objB);

 for (nat32 i=0;i<mc.scale;i++) targA[i] *= targB[i];
}

void Frequency::InplaceAdd(const MessageClass & mc,void * objA,void * objB)
{
 real32 * targA = static_cast<real32*>(objA);
 real32 * targB = static_cast<real32*>(objB);

 for (nat32 i=0;i<mc.scale;i++) targA[i] += targB[i];
}

void Frequency::InplaceNeg(const MessageClass & mc,void * objA,void * objB)
{
 real32 * targA = static_cast<real32*>(objA);
 real32 * targB = static_cast<real32*>(objB);

 for (nat32 i=0;i<mc.scale;i++) targA[i] -= targB[i];
}

void Frequency::FromNegLn(const MessageClass & mc,void * obj)
{
 real32 * targ = static_cast<real32*>(obj);

 real32 min = 0.0;
 for (nat32 i=0;i<mc.scale;i++) min = math::Min(min,targ[i]);
 if (min<0.0)
 {
  for (nat32 i=0;i<mc.scale;i++) targ[i] -= min;
 }

 for (nat32 i=0;i<mc.scale;i++) targ[i] = math::Exp(-targ[i]);
 Norm(mc,obj);
}

void Frequency::Norm(const MessageClass & mc,void * obj)
{
 real32 * targ = static_cast<real32*>(obj);

 real32 val = 0.0;
 for (nat32 i=0;i<mc.scale;i++) val += targ[i];
 if (math::Equal(val,real32(0.0))) return;
 val = 1.0/val;
 for (nat32 i=0;i<mc.scale;i++) targ[i] *= val;
}

void Frequency::Drop(const MessageClass & mc,void * obj)
{
 real32 * targ = static_cast<real32*>(obj);
 real32 min = targ[0];
 for (nat32 i=1;i<mc.scale;i++) min = math::Min(min,targ[i]);
 for (nat32 i=0;i<mc.scale;i++) targ[i] -= min;
}

//------------------------------------------------------------------------------
 };
};
