//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/svt/type.h"

#include "eos/typestring.h"
#include "eos/str/functions.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
Type::Type():cls(Cnode) {}
Type::~Type() {}

bit Type::MemberOf(const Node & rhs)
{
 cstrconst type = typestring(rhs);
 switch (cls)
 {
  case Cnode: break;
  case Cmeta: if (str::Compare(type,"eos::svt::Node")==0) return false; break;
  case Cvar: if ((str::Compare(type,"eos::svt::Node")==0)||(str::Compare(type,"eos::svt::Meta")==0)) return false; break;
 }

 switch (cls)
 {
  case Cvar:
  {
   const Var * var = static_cast<const Var*>(&rhs);
   // Dim/size checks...
   {
    ds::List<DimReq>::Cursor targ = drList.FrontPtr();
    bit matchedFixed = false;
    bit failedFixed = false;
    bit failedRange = false;
    while (!targ.Bad())
    {
     if (targ->size)
     {
      if (targ->dim>=var->Dims()) return false;
      switch (targ->mode)
      {
       case None: break;
       case Fixed: 
        if (var->Size(targ->dim)==targ->value) matchedFixed = true;
                                          else failedFixed = true;
       break;
       case Minimum:
        if (var->Size(targ->dim)>=targ->value) failedRange = true;
       break;
       case Maximum:
        if (var->Size(targ->dim)<=targ->value) failedRange = true;
       break;
      }
     }
     else
     {
      switch (targ->mode)
      {
       case None: break;
       case Fixed: 
        if (var->Dims()==targ->value) matchedFixed = true;
                                 else failedFixed = true;
       break;
       case Minimum:
        if (var->Dims()>=targ->value) failedRange = true;
       break;
       case Maximum:
        if (var->Dims()<=targ->value) failedRange = true;
       break;
      }
     }
     ++targ;
    }
    if ((!matchedFixed)&&((failedFixed)||(failedRange))) return false;
   }

   // Field checks...
   {
    ds::List<FieldReq>::Cursor targ = frList.FrontPtr();
    while (!targ.Bad())
    {
     nat32 ind;
     if (var->GetIndex(targ->name,ind)==false) return false;
     if (targ->type!=str::NullToken)
     {
      if ((targ->type!=var->FieldType(ind))||(targ->size!=var->FieldSize(ind))) return false;
     }
     ++targ;
    }
   }
  }
  case Cmeta: // Fall through and do the meta checks as well.
  {
   const Meta * meta = static_cast<const Meta*>(&rhs);
   // Key checks...
    ds::List<KeyReq>::Cursor targ = krList.FrontPtr();
    while (!targ.Bad())
    {
     Meta::Item * item = meta->Get(targ->key);
     if (item==null<Meta::Item*>()) return false;
     if ((targ->typeCheck)&&(item->Is()!=targ->type)) return false;
     ++targ;
    }
  }
  case Cnode:
   // Nothing to see here, move along.
  break;
 }

 return true;
}

void Type::RequireMeta()
{
 cls = Cmeta;
}

void Type::RequireVar()
{
 cls = Cvar;
}

void Type::RequireKey(str::Token key)
{
 KeyReq req;
  req.key = key;
  req.typeCheck = false;
  req.type = Meta::Aint;
 krList.AddBack(req);
}

void Type::RequireKeyType(str::Token key,Meta::Type type)
{
 KeyReq req;
  req.key = key;
  req.typeCheck = true;
  req.type = type;
 krList.AddBack(req);
}

void Type::ConstrainDims(Mode mode,nat32 val)
{
 DimReq req;
  req.size = false;
  req.mode = mode;
  req.value = val;
 drList.AddBack(req);
}

void Type::ConstrainSize(nat32 dim,Mode mode,nat32 val)
{
 DimReq req;
  req.size = true;
  req.dim = dim;
  req.mode = mode;
  req.value = val;
 drList.AddBack(req);
}

void Type::RequireField(str::Token name)
{
 FieldReq req;
  req.name = name;
  req.type = str::NullToken;
  req.size = 0;
 frList.AddBack(req);
}

void Type::RequireFieldType(str::Token name,str::Token type,nat32 size)
{
 FieldReq req;
  req.name = name;
  req.type = type;
  req.size = size;
 frList.AddBack(req);
}

//------------------------------------------------------------------------------
 };
};
