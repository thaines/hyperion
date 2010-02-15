//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/svt/meta.h"

#include "eos/data/blocks.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
Meta::Meta(Meta * meta)
:Node(static_cast<Node*>(meta))
{
 list = meta->list;
 meta->list.MakeEmptyUnsafe();
}

Meta::Item * Meta::Get(str::Token tok) const
{
 if (IsExport(tok))
 {
  Item dummy(const_cast<Meta&>(*this),tok);
  Item ** item = expList.Get(&dummy);  
  if (item) return *item;
  else
  {
   Item * ret = new Item(*const_cast<Meta*>(this),tok);
   expList.Add(ret);
   return ret;
  }
 }
 else
 {
  Item dummy(const_cast<Meta&>(*this),tok);
  Item ** item = list.Get(&dummy);
  if (item) return *item;
       else return null<Item*>();
 }
}

Meta::Item & Meta::operator[] (str::Token tok)
{
 if (IsExport(tok))
 {
  Item dummy(*this,tok);
  Item ** ret = expList.Get(&dummy);
  if (ret==0)
  {
   Item * ret = new Item(*this,tok);
   expList.Add(ret);
   return *ret;
  }
  else return **ret;  
 }
 else
 {
  Item dummy(*this,tok);
  Item ** ret = list.Get(&dummy);
  if (ret==0)
  {
   Item * ret = new Item(*this,tok);
   list.Add(ret);
   return *ret;
  }
  else return **ret;
 }
}

nat32 Meta::Size() const
{
 return list.Size() + ExportSize();
}

Meta::Item & Meta::Entry(nat32 i)
{
 if (i<ExportSize())
 {
  Item dummy(*this,Export(i));
  Item ** ret = list.Get(&dummy);
  if (ret==0)
  {
   Item * ret = new Item(*this,Export(i));
   list.Add(ret);
   return *ret;
  }
  else return **ret; 
 }
 else return *list[i-ExportSize()];
}

nat32 Meta::Memory() const
{
 memory = Node::Memory() + list.Memory() + sizeof(nat32) + sizeof(Item)*list.Size();
 list.Iter<Meta,&Meta::SumMem>((Meta*)this);
 return memory;
}

nat32 Meta::WriteSize() const
{
 memory = Node::WriteSize();
  memory += 16 + 4;
  list.Iter<Meta,&Meta::SumWriteItems>((Meta*)this);
 return memory;
}

nat32 Meta::TotalWriteSize() const
{
 return WriteSize();
}

cstrconst Meta::MagicString() const
{
 return "SID";
}

nat32 Meta::Write(io::OutVirt<io::Binary> & out) const
{
 nat32 ret = Node::Write(out);

  // Header...
   nat32 ws = WriteSize();
   nat32 bs = ws - ret;
   nat32 os = TotalWriteSize()-ret;
   nat8 zb = 0;

   ret += out.Write("SID",3);
   ret += out.Write(MagicString(),3);
   ret += out.Write(&bs,4);
   ret += out.Write(&zb,1);
   ret += out.Write(&os,4);
   ret += out.Write(&zb,1);

  // Field count...
   nat32 fc = list.Size();
   ret += out.Write(&fc,4);

  // Fields...
   for (nat32 i=0;i<fc;i++)
   {
    Item * targ = list[i];
    // Name lump...
     cstrconst name = core.GetTT().Str(targ->key);
     nat16 nameSize = str::Length(name);
     ret += out.Write(&nameSize,2);
     ret += out.Write(name,nameSize); 

    // Type lump & Data lump...
     switch (targ->is)
     {
      case Aint:
      {
       nat16 typeSize = 3;
       ret += out.Write(&typeSize,2);
       ret += out.Write("int",typeSize);
       
       typeSize = 4;
       ret += out.Write(&typeSize,2);
       ret += out.Write(&targ->asInt,4);
      }
      break;
      case Areal:
      {
       nat16 typeSize = 4;
       ret += out.Write(&typeSize,2);
       ret += out.Write("real",typeSize);
       
       typeSize = 4;
       ret += out.Write(&typeSize,2);
       ret += out.Write(&targ->asReal,4);       
      }
      break;
      case Atoken:
      {
       nat16 typeSize = 5;
       ret += out.Write(&typeSize,2);
       ret += out.Write("token",typeSize);
       
       cstrconst str = core.GetTT().Str(targ->asTok);
       nat16 strSize = str::Length(str);
       ret += out.Write(&strSize,2);
       ret += out.Write(str,strSize);        
      }
      break;
      case Astring:
      {
       nat16 typeSize = 6;
       ret += out.Write(&typeSize,2);
       ret += out.Write("string",typeSize);
       
       typeSize = targ->asStr->Size();
       ret += out.Write(&typeSize,2);
       out << *targ->asStr;
       ret += typeSize;
      }
      break;      
     }
   }

 if (ret!=ws) out.SetError(true);
 return ret;
}

void Meta::ReadBlock(io::InVirt<io::Binary> & in)
{
 struct Head
 {
  cstrchar bm[3];
  cstrchar om[3];
  nat32 bSize;
  nat8 bExt;
  nat32 oSize;
  nat8 oExt;
 } head;
 
 nat32 rsf = 0;
  rsf += in.Read(head.bm,3);
  rsf += in.Read(head.om,3);
  rsf += in.Read(&head.bSize,4);
  rsf += in.Read(&head.bExt,1);
  rsf += in.Read(&head.oSize,4);
  rsf += in.Read(&head.oExt,1);
  
  if ((head.bm[0]!='S')||(head.bm[1]!='I')||(head.bm[2]!='D')) {in.SetError(true); return;}
  if ((head.bExt!=0)||(head.oExt!=0)) {in.SetError(true); return;}

  // Loaded header, time for the body...
   nat32 fields;
   rsf += in.Read(&fields,4);
   if (rsf!=20) {in.SetError(true); return;}
   
   data::Block block(128); 
   for (nat32 i=0;i<fields;i++)
   {
    // Read in the name...
     nat16 size = 0;
     rsf += in.Read(&size,2);
     if (block.Size()<nat32(size)+1) block.SetSize(size+1);
     rsf += in.Read(block.Ptr(),size);
     block[size] = 0;
     str::Token key = core.GetTT()((cstrconst)block.Ptr());
    
    // Read in the type...
     size = 0;
     rsf += in.Read(&size,2);
     if (block.Size()<nat32(size)+1) block.SetSize(size+1);
     rsf += in.Read(block.Ptr(),size);
     block[size] = 0;
     Type type = Astring;
     if (str::Compare((cstrconst)block.Ptr(),"int")==0) type = Aint;
     else
     {
      if (str::Compare((cstrconst)block.Ptr(),"real")==0) type = Areal;
      else
      {
       if (str::Compare((cstrconst)block.Ptr(),"token")==0) type = Atoken;
      }
     }
    
    // Read in the data...
     size = 0;
     rsf += in.Read(&size,2);
     if (block.Size()<nat32(size)+1) block.SetSize(size+1);
     rsf += in.Read(block.Ptr(),size);
    
    // Create and store the item...
     switch (type)
     {
      case Aint: (*this)[key].Set(*(int32*)block.Ptr()); break;
      case Areal: (*this)[key].Set(*(real32*)block.Ptr()); break;
      case Atoken: (*this)[key].Set(core.GetTT()((cstrconst)block.Ptr())); break;
      case Astring: (*this)[key].Set((cstrconst)block.Ptr()); break;
     }
   }

 in.SetError(rsf!=head.bSize);
}

//------------------------------------------------------------------------------
int32 Meta::Item::AsInt() const
{
 if (exp)
 {
  int32 ret = 0;
   meta.OnGetInt(key,ret);
  return ret;
 }
 else
 {
  switch (is)
  {
   case Aint: return asInt;
   case Areal: return (int32)asReal;
   case Atoken: return str::ToInt32(meta.core.GetTT().Str(asTok));
   case Astring: return asStr->ToInt32();
  }
  return 0;
 }
}

real32 Meta::Item::AsReal() const
{
 if (exp)
 {
  real32 ret = 0.0;
   meta.OnGetReal(key,ret);
  return ret;
 }
 else
 {
  switch (is)
  {
   case Aint: return (real32)asInt;
   case Areal: return asReal;
   case Atoken: return str::ToReal32(meta.core.GetTT().Str(asTok));
   case Astring: return asStr->ToReal32();
  }
  return 0.0;
 }
}

str::Token Meta::Item::AsToken() const
{
 if (exp)
 {
  str::Token ret = str::NullToken;
   meta.OnGetToken(key,ret);
  return ret;
 }
 else
 {
  switch (is)
  {
   case Aint:
   {
    cstrchar temp[12];
    str::ToStr(asInt,temp);
    return meta.core.GetTT()(temp);
   }
   case Areal:
   {
    cstrchar temp[64];
    str::ToStr(asReal,temp);
    return meta.core.GetTT()(temp);
   }
   case Atoken: return asTok;
   case Astring: return meta.core.GetTT()(*asStr);
  }
  return 0;
 }
}

void Meta::Item::AsString(str::String & out) const
{
 if (exp)
 {
  if (meta.OnGetString(key,out)==false) out.SetSize(0);
 }
 else
 {
  out.SetSize(0);
  switch (is)
  {
   case Aint: out << asInt; break;
   case Areal: out << asReal; break;
   case Atoken: out << meta.core.GetTT().Str(asTok); break;
   case Astring: out << asStr;break;
  }
 }
}

bit Meta::Item::Set(int32 rhs)
{
 if (exp) return meta.OnSetInt(key,rhs);
 else
 {
  if (is!=Aint)
  {
   if (is==Astring) delete asStr;
   is = Aint;
  }
  asInt = rhs;
  return true;
 }
}

bit Meta::Item::Set(real32 rhs)
{
 if (exp) return meta.OnSetReal(key,rhs);
 else
 {
  if (is!=Areal)
  {
   if (is==Astring) delete asStr;
   is = Areal;
  }
  asReal = rhs;
  return true;
 }
}

bit Meta::Item::Set(str::Token rhs)
{
 if (exp) return meta.OnSetToken(key,rhs);
 else
 {
  if (is!=Atoken)
  {
   if (is==Astring) delete asStr;
   is = Atoken;
  }
  asTok = rhs;
  return true;
 }
}

bit Meta::Item::Set(const str::String & rhs)
{
 if (exp) return meta.OnSetString(key,rhs);
 else
 {
  if (is!=Astring)
  {
   is = Astring;
   asStr = new str::String();
  }
  *asStr = rhs;
  return true;
 }
}

bit Meta::Item::Set(cstrconst rhs)
{
 if (exp)
 {
  str::String str(rhs);
  return meta.OnSetString(key,rhs);
 }
 else
 {
  if (is!=Astring)
  {
   is = Astring;
   asStr = new str::String(rhs);
  }
  else
  {
   *asStr = rhs;
  }
  return true;
 }
}
//------------------------------------------------------------------------------
 };
};
