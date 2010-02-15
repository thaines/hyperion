//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/svt/var.h"

#include "eos/str/functions.h"
#include "eos/data/blocks.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
Var::Var(Core & c)
:Meta(c),
changed(true),dims(0),size(null<nat32*>()),stride(null<nat32*>()),
fields(0),fi(null<Entry**>()),byName(3),
data(null<byte*>())
{}

Var::Var(Meta * meta)
:Meta(meta),
changed(true),dims(0),size(null<nat32*>()),stride(null<nat32*>()),
fields(0),fi(null<Entry**>()),byName(3),
data(null<byte*>())
{}

Var::Var(Var * var)
:Meta(static_cast<Meta*>(var)),
changed(var->changed),dims(var->dims),size(var->size),stride(var->stride),
fields(var->fields),fi(var->fi),byName(var->byName),
data(var->data)
{
 var->size = null<nat32*>();
 var->stride = null<nat32*>();
 var->fields = 0;
 var->fi = null<Entry**>();
 var->data = null<byte*>();
}

Var::~Var()
{
 delete[] size;
 delete[] stride;

 for (nat32 i=0;i<fields;i++) delete fi[i];
 delete[] fi;

 mem::Free(data);
}

Var & Var::operator= (const Var & rhs)
{
 // Terminate current contents...
  delete[] size;
  delete[] stride;

  for (nat32 i=0;i<fields;i++) delete fi[i];
  delete[] fi;

  mem::Free(data);
  
 // Copy over the (meta-) data...
  changed = rhs.changed;
  dims = rhs.dims;
  size = new nat32[dims];
  for (nat32 i=0;i<dims;i++) size[i] = rhs.size[i];
  stride = new nat32[dims+1];
  for (nat32 i=0;i<=dims;i++) stride[i] = rhs.stride[i];

  fields = rhs.fields;
  fi = new Entry*[fields];
  for (nat32 i=0;i<fields;i++)
  {
   fi[i] = new Entry(*rhs.fi[i]);
  }
  byName = rhs.byName;

  data = mem::Malloc<byte>(stride[dims]);
  mem::Copy(data,rhs.data,stride[dims]);

 return *this;
}

void Var::Setup(nat32 d,const nat32 * s)
{
 changed = true;
 dims = d;

 delete[] size; size = new nat32[dims];
 delete[] stride; stride = new nat32[dims+1];

 for (nat32 i=0;i<dims;i++) size[i] = s[i];
 // stride is ignored, as it will be recalculated on Commit().
}

void Var::Setup1D(nat32 a)
{
 nat32 s[1];
 s[0] = a;
 Setup(1,s);
}

void Var::Setup2D(nat32 a,nat32 b)
{
 nat32 s[2];
 s[0] = a; s[1] = b;
 Setup(2,s);
}

void Var::Setup3D(nat32 a,nat32 b,nat32 c)
{
 nat32 s[3];
 s[0] = a; s[1] = b; s[2] = c;
 Setup(3,s);
}

void Var::Setup4D(nat32 a,nat32 b,nat32 c,nat32 d)
{
 nat32 s[4];
 s[0] = a; s[1] = b; s[2] = c; s[3] = d;
 Setup(4,s);
}

void Var::Add(str::Token name,str::Token type,nat32 size,const void * ini)
{
 Entry ** nfi = new Entry*[fields+1];
 for (nat32 i=0;i<fields;i++) nfi[i] = fi[i];
 delete[] fi; fi = nfi;

 fi[fields] = new Entry();
  fi[fields]->name = name;
  fi[fields]->type = type;
  fi[fields]->size = size;
  fi[fields]->ini = mem::Malloc<byte>(size);
  mem::Copy(fi[fields]->ini,(byte*)ini,size);
  // offset ignored - its calculated during the commit.
  fi[fields]->state = Entry::Added;

 ++fields;
}

void Var::Rem(str::Token name)
{
 nat32 * ind = byName.Get(name);
 if (ind)
 {
  fi[*ind]->state = Entry::Deleted;
 }
}

void Var::Commit(bit useDefault)
{
 if (changed)
 {
  changed = false;
  // Its a complete re-write, no reorganising of data required. We firstly have to re-calculate
  // the field table, then we construct the data structure and if required copy
  // in the defaults.
   // Recalculate field table...
    nat32 targ = 0;
    nat32 offset = 0;
    for (nat32 i=0;i<fields;i++)
    {
     switch (fi[i]->state)
     {
      case Entry::Stored: 
       fi[i]->offset = offset; offset += fi[i]->size; 
       fi[targ] = fi[i]; ++targ; 
      break;
      case Entry::Added: 
       fi[i]->state = Entry::Stored;
       fi[i]->offset = offset; offset += fi[i]->size; 
       fi[targ] = fi[i]; ++targ; 
      break;
      case Entry::Deleted:
       delete fi[i];
      break;
     }
    }
    if (targ<fields)
    {
     fields = targ;
     Entry ** nfi = new Entry*[fields];
     for (nat32 i=0;i<fields;i++) nfi[i] = fi[i];
     delete[] fi;
     fi = nfi;
    }

   // Rebuild the stride data structure...
    stride[0] = offset;
    for (nat32 i=0;i<dims;i++) stride[i+1] = stride[i]*size[i];

   // Make new data structrue...
    mem::Free(data);
    data = mem::Malloc<byte>(stride[dims]);

   // Copy in defaults...
    if (useDefault)
    {
     byte * temp = mem::Malloc<byte>(stride[0]);
     for (nat32 i=0;i<fields;i++) mem::Copy(temp+fi[i]->offset,fi[i]->ini,fi[i]->size);
      nat32 n = 1; for (nat32 i=0;i<dims;i++) n *= size[i];
      byte * targ = data;
      for (nat32 i=0;i<n;i++)
      {
       mem::Copy(targ,temp,stride[0]);
       targ += stride[0];
      }
     mem::Free(temp);
    }   
 }
 else
 {
  // We have some serious work to do. We start by calculating a copy list, of data to be copied
  // over for each field, and of defaults to copy over if required. We then rebuild the field
  // table and make the new data structure, running the copy list on it.
   // Calculate copy list...
    nat32 cls = 0; // Size of copy list, will be the size of the fields table when done.
    for (nat32 i=0;i<fields;i++) if (fi[i]->state!=Entry::Deleted) ++cls;

    struct CLcode
    {
     bit op; // false == copy in the default, true == copy in from the previous entry.
     nat32 prevOffset; // Offset for previous entry, for when op==true.    
    } * code = new CLcode[cls]; // This will match the field structure to be, so all the other data needed comes from there.

    cls = 0;
    nat32 prevStride = 0;
    for (nat32 i=0;i<fields;i++)
    {
     if (fi[i]->state==Entry::Stored)
     {
      code[cls].op = true;
      code[cls].prevOffset = fi[i]->offset;
      prevStride += fi[i]->size;
      ++cls;
     }
     else if (fi[i]->state==Entry::Added)
     {
      code[cls].op = false;
      ++cls;
     }
     else // Deleted
     {
      prevStride += fi[i]->size;
     }
    }

   // Recalculate field table...
    nat32 targ = 0;
    nat32 offset = 0;
    for (nat32 i=0;i<fields;i++)
    {
     switch (fi[i]->state)
     {
      case Entry::Stored: 
       fi[i]->offset = offset; offset += fi[i]->size; 
       fi[targ] = fi[i]; ++targ; 
      break;
      case Entry::Added: 
       fi[i]->state = Entry::Stored;
       fi[i]->offset = offset; offset += fi[i]->size; 
       fi[targ] = fi[i]; ++targ; 
      break;
      case Entry::Deleted:
       delete fi[i];
      break;
     }
    }
    if (targ<fields)
    {
     fields = targ;
     Entry ** nfi = new Entry*[fields];
     for (nat32 i=0;i<fields;i++) nfi[i] = fi[i];
     delete[] fi;
     fi = nfi;
    }

   // Rebuild the stride data structure...
    stride[0] = offset;
    for (nat32 i=0;i<dims;i++) stride[i+1] = stride[i]*size[i];

   // Build new data structure...
    byte * newData = mem::Malloc<byte>(stride[dims]);

   // Copy over data...
    byte * targIn = data;
    byte * targOut = newData;
    nat32 n = 1; for (nat32 i=0;i<dims;i++) n *= size[i];
    for (nat32 i=0;i<n;i++)
    {
     for (nat32 i=0;i<fields;i++)
     {
      if (code[i].op)
      {
       // Copy from old data...
        mem::Copy(targOut+fi[i]->offset,targIn+code[i].prevOffset,fi[i]->size);
      }
      else if (useDefault)
      {
       // Copy in the default...
        mem::Copy(targOut+fi[i]->offset,fi[i]->ini,fi[i]->size);
      }
     }
     targIn += prevStride;
     targOut += stride[0];
    }

   // Copy in the new structure, terminating the old one...
    mem::Free(data);
    data = newData;

   // Clean up...
    delete[] code;
 }

 // Final step in the algorithm - create the byName hash table so the dam 
 // interface works...
  byName.Reset(3);
  for (nat32 i=0;i<fields;i++)
  {
   byName.Set(fi[i]->name,i);
  }
}

nat32 Var::FieldMemory(nat32 ind) const
{
 nat32 ret = fi[ind]->size;
 for (nat32 i=0;i<dims;i++) ret *= size[i];
 return ret;
}

void Var::GetRaw(nat32 ind,byte * out) const
{
 byte * targ = data + fi[ind]->offset;
 
 nat32 num = 1;
 for (nat32 i=0;i<dims;i++) num *= size[i];
 
 for (nat32 i=0;i<num;i++)
 {
  mem::Copy(out,targ,fi[ind]->size);
  
  out += fi[ind]->size;
  targ += stride[0];
 }
}

nat32 Var::Memory() const
{
 nat32 ret = sizeof(Var);
 ret += sizeof(nat32)*dims;
 ret += sizeof(nat32)*(dims+1);
 
 ret += (sizeof(Entry*)+sizeof(Entry))*fields;
 for (nat32 i=0;i<fields;i++) ret += fi[i]->size;

 ret += byName.Memory() - sizeof(byName);

 ret += stride[dims];

 return ret;
}

nat32 Var::WriteSize() const
{
 nat32 ret = Meta::WriteSize();
  ret += 16;
  ret += 4*(dims+1);

  ret += 4 + 6*fields;
  for (nat32 i=0;i<fields;i++)
  {
   ret += str::Length(core.GetTT().Str(fi[i]->name)) +
          str::Length(core.GetTT().Str(fi[i]->type)) +
          fi[i]->size;
  }
  
  ret += stride[dims];
 return ret;
}

nat32 Var::TotalWriteSize() const
{
 return WriteSize();
}

cstrconst Var::MagicString() const
{
 return "MID";
}

nat32 Var::Write(io::OutVirt<io::Binary> & out) const
{
 nat32 ret = Meta::Write(out);

  // Header...
   nat32 ws = WriteSize();
   nat32 bs = ws - ret;
   nat32 os = TotalWriteSize() - ret;
   nat8 zb = 0;

   ret += out.Write("MID",3);
   ret += out.Write(MagicString(),3);
   ret += out.Write(&bs,4);
   ret += out.Write(&zb,1);
   ret += out.Write(&os,4);
   ret += out.Write(&zb,1);

  // Array structure...
   ret += out.Write(&dims,4);
   for (nat32 i=0;i<dims;i++) ret += out.Write(&size[i],4);

  // Field list...
   ret += out.Write(&fields,4);
   for (nat32 i=0;i<fields;i++)
   {
    cstrconst str = core.GetTT().Str(fi[i]->name);
    nat16 size = str::Length(str);
    ret += out.Write(&size,2);
    ret += out.Write(str,size);
    
    str = core.GetTT().Str(fi[i]->type);
    size = str::Length(str);
    ret += out.Write(&size,2);
    ret += out.Write(str,size); 
    
    size = fi[i]->size;
    ret += out.Write(&size,2);
    ret += out.Write(fi[i]->ini,size);       
   }

  // Data...
   ret += out.Write(data,stride[dims]);

 if (ret!=ws) out.SetError(true);
 return ret;
}

void Var::ReadBlock(io::InVirt<io::Binary> & in)
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
  
  if ((head.bm[0]!='M')||(head.bm[1]!='I')||(head.bm[2]!='D')) {in.SetError(true); return;}
  if ((head.bExt!=0)||(head.oExt!=0)) {in.SetError(true); return;}

  // Loaded header, time for the body...
   // Wipe out any previous storage...
    changed = false;
    delete[] size;
    delete[] stride;
    for (nat32 i=0;i<fields;i++) delete fi[i];
    delete[] fi;
    mem::Free(data);
    
   // The array data...
    rsf += in.Read(&dims,4);
    size = new nat32[dims];
    stride = new nat32[dims+1];
    for (nat32 i=0;i<dims;i++) rsf += in.Read(&size[i],4);
   
   // The field data...
    rsf += in.Read(&fields,4);
    fi = new Entry*[fields];
    data::Block block(128);
    stride[0] = 0;
    for (nat32 i=0;i<fields;i++)
    {
     fi[i] = new Entry();
     
     // Read in the name...
      nat16 lumpSize = 0;
      rsf += in.Read(&lumpSize,2);
      if (block.Size()<nat32(lumpSize)+1) block.SetSize(lumpSize+1);
      rsf += in.Read(block.Ptr(),lumpSize);
      block[lumpSize] = 0;
      fi[i]->name = core.GetTT()((cstrconst)block.Ptr());

     // Read in the type...
      lumpSize = 0;
      rsf += in.Read(&lumpSize,2);
      if (block.Size()<nat32(lumpSize)+1) block.SetSize(lumpSize+1);
      rsf += in.Read(block.Ptr(),lumpSize);
      block[lumpSize] = 0;
      fi[i]->type = core.GetTT()((cstrconst)block.Ptr());
      
     // Read in the size and default data...
      lumpSize = 0;
      rsf += in.Read(&lumpSize,2);
      fi[i]->size = lumpSize;
      fi[i]->ini = mem::Malloc<byte>(fi[i]->size);
      rsf += in.Read(fi[i]->ini,fi[i]->size);
      
     fi[i]->state = Entry::Stored; 
     fi[i]->offset = stride[0];
     stride[0] += fi[i]->size;
    }
   
   // Correct the stride data...
    for (nat32 i=0;i<dims;i++) stride[i+1] = stride[i]*size[i];
   
   // Update byName index...
    byName.Reset(3);
    for (nat32 i=0;i<fields;i++) byName.Set(fi[i]->name,i);
   
   // The actual data...
    data = mem::Malloc<byte>(stride[dims]);
    rsf += in.Read(data,stride[dims]);
  
 in.SetError(rsf!=head.bSize);
}

//------------------------------------------------------------------------------
nat32 Var::ExportSize() const
{
 return 3 + dims*2 + fields*4;
}

str::Token Var::Export(nat32 i) const
{
 if (i<2)
 {
  // The fixed allways-there once...
   if (i==0) return core.GetTT()("dims");
        else return core.GetTT()("fields");
 }
 else if (i<3+dims*2)
 {
  if (i<2+dims)
  {
   // The size once...
    cstrchar str[32];
     str::Copy(str,"size[");
     str::ToStr(i-2,str+str::Length(str));
     str::Append(str,"]");
    return core.GetTT()(str);
  }
  else
  {
   // The stride once...
    cstrchar str[32];
     str::Copy(str,"stride[");
     str::ToStr(i-(2+dims),str+str::Length(str));
     str::Append(str,"]");
    return core.GetTT()(str);
  }
 }
 else
 {
  // The field once...
   i -= 3+dims*2;
   nat32 ind = i/4;
   nat32 wh = i%4;
  
   cstrchar str[64];
    str::Copy(str,"field[");
     str::ToStr(ind,str+str::Length(str));
     switch (wh)
     {
      case 0: str::Append(str,"].name"); break;
      case 1: str::Append(str,"].type"); break;
      case 2: str::Append(str,"].size"); break;
      case 3: str::Append(str,"].default"); break;
     }
   return core.GetTT()(str);
 }
}

bit Var::IsExport(str::Token tok) const
{
 cstrconst str = core.GetTT().Str(tok);
 if (str::AtStart(str,"field["))
 {
  nat32 ind = (nat32)str::ToInt32(str+6); // 6 = chars in 'field['.
  if (ind<fields)
  {
   if (str::AtEnd(str,"].name")) return true;
   if (str::AtEnd(str,"].type")) return true;
   if (str::AtEnd(str,"].size")) return true;
   if (str::AtEnd(str,"].default")) return true;
  }
 }
 else if (str::AtStart(str,"size["))
 {
  if (str::AtEnd(str,"]"))
  {
   nat32 ind = (nat32)str::ToInt32(str+5); // 5 = chars in 'size['.
   if (ind<dims) return true;
  }
 }
 else if (str::AtStart(str,"stride["))
 {
  if (str::AtEnd(str,"]"))
  {
   nat32 ind = (nat32)str::ToInt32(str+7); // 7 = chars in 'stride['.
   if (ind<=dims) return true;
  }
 }
 else if (str::Compare(str,"dims")==0) return true;
 else if (str::Compare(str,"fields")==0) return true;
 
 return false;
}

bit Var::CanSet(str::Token tok) const
{
 if (IsExport(tok)) return false;
               else return true;
}

cstrconst Var::GetDescription(str::Token tok) const
{
 return "";
}

Meta::Type Var::OnGetType(str::Token tok) const
{
 cstrconst str = core.GetTT().Str(tok);
 if (str::AtStart(str,"field["))
 {
  nat32 ind = (nat32)str::ToInt32(str+6); // 6 = chars in 'field['.
  if (ind<fields)
  {
   if (str::AtEnd(str,"].name")) return Meta::Atoken;
   if (str::AtEnd(str,"].type")) return Meta::Atoken;
   if (str::AtEnd(str,"].size")) return Meta::Aint;
   if (str::AtEnd(str,"].default")) return Meta::Astring;
  }
 }
 else if (str::AtStart(str,"size["))
 {
  if (str::AtEnd(str,"]"))
  {
   nat32 ind = (nat32)str::ToInt32(str+5); // 5 = chars in 'size['.
   if (ind<dims) return Meta::Aint;
  }
 }
 else if (str::AtStart(str,"stride["))
 {
  if (str::AtEnd(str,"]"))
  {
   nat32 ind = (nat32)str::ToInt32(str+7); // 7 = chars in 'stride['.
   if (ind<=dims) return Meta::Aint;
  }
 }
 else if (str::Compare(str,"dims")==0) return Meta::Aint;
 else if (str::Compare(str,"fields")==0) return Meta::Aint;
 return Meta::Aint;
}

bit Var::OnGetInt(str::Token tok,int32 & out) const
{
 cstrconst str = core.GetTT().Str(tok);
 if (str::AtStart(str,"field["))
 {
  nat32 ind = (nat32)str::ToInt32(str+6); // 6 = chars in 'field['.
  if (ind<fields)
  {
   if (str::AtEnd(str,"].size"))
   {
    out = fi[ind]->size;
   }
  }
 }
 else if (str::AtStart(str,"size["))
 {
  if (str::AtEnd(str,"]"))
  {
   nat32 ind = (nat32)str::ToInt32(str+5); // 5 = chars in 'size['.
   if (ind<dims)
   {
    out = size[ind];
    return true;
   }
  }
 }
 else if (str::AtStart(str,"stride["))
 {
  if (str::AtEnd(str,"]"))
  {
   nat32 ind = (nat32)str::ToInt32(str+7); // 7 = chars in 'stride['.
   if (ind<=dims)
   {
    out = stride[ind];
    return true;
   }
  }
 }
 else if (str::Compare(str,"dims")==0)
 {
  out = dims;
  return true;
 }
 else if (str::Compare(str,"fields")==0)
 {
  out = fields;
  return true;
 }
 return false;
}

bit Var::OnGetReal(str::Token tok,real32 & out) const {return false;}

bit Var::OnGetToken(str::Token tok,str::Token & out) const
{
 cstrconst str = core.GetTT().Str(tok);
 if (str::AtStart(str,"field["))
 {
  nat32 ind = (nat32)str::ToInt32(str+6); // 6 = chars in 'field['.
  if (ind<fields)
  {
   if (str::AtEnd(str,"].name"))
   {
    out = fi[ind]->name;
   }
   else if (str::AtEnd(str,"].type"))
   {
    out = fi[ind]->type;
   }
  }
 }
 return false;
}

bit Var::OnGetString(str::Token tok,str::String & out) const
{
 cstrconst str = core.GetTT().Str(tok);
 if (!str::AtStart(str,"field[")) return false;
 if (!str::AtEnd(str,"].default")) return false;
 
 nat32 ind = (nat32)str::ToInt32(str+6); // 6 = chars in 'field['.
 if (ind<fields)
 {
  out.SetSize(0);
  out << data::Buffer(fi[ind]->ini,fi[ind]->size);
  return true;
 }
 else return false;
}

bit Var::OnSetInt(str::Token tok,int32 in) {return false;}
bit Var::OnSetReal(str::Token tok,real32 in) {return false;}
bit Var::OnSetToken(str::Token tok,str::Token in) {return false;}
bit Var::OnSetString(str::Token tok,const str::String & in) {return false;}

//------------------------------------------------------------------------------
 };
};
