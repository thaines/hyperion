//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/file/ply.h"

#include "eos/file/files.h"
#include "eos/io/to_virt.h"
#include "eos/io/functions.h"
#include "eos/io/conversion.h"
#include "eos/ds/dense_hash.h"
#include "eos/str/functions.h"
#include "eos/str/tokenize.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
Ply::Ply()
:cornerCount(0),faceCount(0)
{}

Ply::~Ply()
{}

bit Ply::Save(cstrconst fn,bit ow,time::Progress * prog) const
{
 prog->Push();

 File<io::Binary> f(fn,ow?way_ow:way_new,mode_write);
 if (!f.Active()) {prog->Pop(); return false;}
 Cursor<io::Binary> out = f.GetCursor();


 // Save header...
 {
  cstrchar buf[32];

  cstrconst head = "ply\nformat binary_little_endian 1.0\n";
  cstrconst tail = "end_header\n";
  out.Write(head,str::Length(head));

  cstrconst headV = "element vertex ";
  cstrconst tailV = "\nproperty float x\nproperty float y\nproperty float z\nproperty float u\nproperty float v\n";
  out.Write(headV,str::Length(headV));
  str::ToStr(vert.Size(),buf);
  out.Write(buf,str::Length(buf));
  out.Write(tailV,str::Length(tailV));

  cstrconst headF = "element face ";
  cstrconst tailF = "\nproperty list uchar int vertex_index\n";
  out.Write(headF,str::Length(headF));
  str::ToStr(faceCount,buf);
  out.Write(buf,str::Length(buf));
  out.Write(tailF,str::Length(tailF));

  out.Write(tail,str::Length(tail));
 }


 // Save vertices...
 nat32 step = 0;
 nat32 steps = vert.Size() + surface.Size();
 {
  ds::List<V>::Cursor targ = vert.FrontPtr();
  while (!targ.Bad())
  {
   prog->Report(step++,steps);
   if (out.Write(&targ->pos,12)!=12) {prog->Pop(); return false;}
   if (out.Write(&targ->uv,8)!=8) {prog->Pop(); return false;}
   ++targ;
  }
 }

 // Save faces...
 {
  ds::List<nat32>::Cursor targ = surface.FrontPtr();
  bit first = true;
  while (!targ.Bad())
  {
   prog->Report(step++,steps);
   if (first)
   {
    ds::List<nat32>::Cursor ra = targ;
    nat8 n = 0;
    while (!ra.Bad())
    {
     if (*ra==nat32(-1)) break;
                    else n += 1;
     ++ra;
    }
    if (out.Write(&n,1)!=1) {prog->Pop(); return false;}
    first = false;
   }
   if (*targ==nat32(-1)) first = true;
   else
   {
    if (out.Write(&(*targ),4)!=4) {prog->Pop(); return false;}
   }
   ++targ;
  }
 }


 prog->Pop();
 return true;
}

bit Ply::Save(const str::String & fn,bit ow,time::Progress * prog) const
{
 cstr s = fn.ToStr();
  bit ret = Save(s,ow,prog);
 mem::Free(s);
 return ret;
}

nat32 Ply::Add(const bs::Vert & pos,const bs::Tex2D & uv)
{
 V v;
 v.pos = pos;
 v.uv = uv;
 vert.AddBack(v);
 return vert.Size()-1;
}

nat32 Ply::Add(nat32 vertInd)
{
 surface.AddBack(vertInd);
 cornerCount += 1;
 return cornerCount;
}

nat32 Ply::Face()
{
 surface.AddBack(nat32(-1));
 cornerCount = 0;
 ++faceCount;
 return faceCount-1;
}

//------------------------------------------------------------------------------
// Stuff to help the ply loader...
enum PlyEncode {PlyAscii,
                PlyLittleEndian, // Intel
                PlyBigEndian, // Motorola
                PlyUnknownEncode
               };

enum PlyType {PlyChar,
              PlyUChar,
              PlyShort,
              PlyUShort,
              PlyInt,
              PlyUInt,
              PlyFloat,
              PlyDouble,
              PlyList,
              PlyUnknown
             };

PlyType StrToPlyType(cstrconst t)
{
 if (str::Compare(t,"char")==0) return PlyChar;
 else
 {
  if (str::Compare(t,"uchar")==0) return PlyUChar;
  else
  {
   if (str::Compare(t,"short")==0) return PlyShort;
   else
   {
    if (str::Compare(t,"ushort")==0) return PlyUShort;
    else
    {
     if (str::Compare(t,"int")==0) return PlyInt;
     else
     {
      if (str::Compare(t,"uint")==0) return PlyUInt;
      else
      {
       if (str::Compare(t,"float")==0) return PlyFloat;
       else
       {
        if (str::Compare(t,"double")==0) return PlyDouble;
        else
        {
         if (str::Compare(t,"list")==0) return PlyList;
         else
         {
          LogAlways("[file.ply] Unknown type" << LogDiv() << t);
          return PlyUnknown;
         }
        }
       }
      }
     }
    }
   }
  }
 }
}

nat32 PlyTypeSize(PlyType t)
{
 switch (t)
 {
  case PlyChar: return 1;
  case PlyUChar: return 1;
  case PlyShort: return 2;
  case PlyUShort: return 2;
  case PlyInt: return 4;
  case PlyUInt: return 4;
  case PlyFloat: return 4;
  case PlyDouble: return 8;
  default: LogAlways("[file.ply] Size of variable sized type requested."); return 0;
 }
}

nat32 ReadPlyInt(io::InVirt<io::Binary> & cur,PlyEncode e,PlyType t)
{
 if (e==PlyAscii)
 {
  io::ConvertIn<io::InVirt<io::Binary>,io::Text> curT = io::ConvertIn<io::InVirt<io::Binary>,io::Text>(cur);
  nat32 ret = 0;
  curT >> ret;
  return ret;
 }
 else
 {
  switch (t)
  {
   case PlyChar:
   {
    int8 d;
    cur >> d;
    return d;
   }
   case PlyUChar:
   {
    nat8 d;
    cur >> d;
    return d;
   }
   case PlyShort:
   {
    int16 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return d;
   }
   case PlyUShort:
   {
    nat16 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return d;
   }
   case PlyInt:
   {
    int32 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return d;
   }
   case PlyUInt:
   {
    nat32 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return d;
   }
   case PlyFloat:
   {
    real32 d;
    cur >> d;
    return nat32(d);
   }
   case PlyDouble:
   {
    real64 d;
    cur >> d;
    return nat32(d);
   }
   case PlyList: case PlyUnknown: LogAlways("[file.ply] Type can not be represented as an integer."); return 0;
  }
 }
 return 0;
}

real32 ReadPlyReal(io::InVirt<io::Binary> & cur,PlyEncode e,PlyType t)
{
 if (e==PlyAscii)
 {
  io::ConvertIn<io::InVirt<io::Binary>,io::Text> curT = io::ConvertIn<io::InVirt<io::Binary>,io::Text>(cur);
  real64 ret = 0.0;
  curT >> ret;
  return ret;
 }
 else
 {
  switch (t)
  {
   case PlyChar:
   {
    int8 d;
    cur >> d;
    return real32(d)/real32(math::max_int_8);
   }
   case PlyUChar:
   {
    nat8 d;
    cur >> d;
    return real32(d)/real32(math::max_nat_8);
   }
   case PlyShort:
   {
    int16 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return real32(d)/real32(math::max_int_16);
   }
   case PlyUShort:
   {
    nat16 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return real32(d)/real32(math::max_nat_16);
   }
   case PlyInt:
   {
    int32 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return real32(d)/real32(math::max_int_32);
   }
   case PlyUInt:
   {
    nat32 d;
    cur >> d;
    if (e==PlyBigEndian) d = io::FromBigEndian(d);
    return real32(d)/real32(math::max_nat_32);
   }
   case PlyFloat:
   {
    real32 d;
    cur >> d;
    return real32(d);
   }
   case PlyDouble:
   {
    real64 d;
    cur >> d;
    return real32(d);
   }
   case PlyList: case PlyUnknown:  LogAlways("[file.ply] Type can not be represented as a real."); return 0.0;
  }
 }
 return 0.0;
}



struct PlyProperty
{
 str::Token name;
 PlyType type;

 PlyType listLength;
 PlyType listType;
 str::Token listIndex;

 int32 outInd; // For useful entrys this is the valid index to write out the data, or -1 if its to be ignored.
 bit faceList; // True if this is the face list.

 bit Load(const str::Tokenize & line,str::TokenTable & tt)
 {
  if (line.Size()==0) {LogAlways("[file.ply] Empty line is not a property."); return false;}
  if (str::Compare(line[0],"property")!=0) {LogAlways("[file.ply] Line is not a property."); return false;}

  outInd = -1;
  faceList = false;

  if (line.Size()==3)
  {
   // Basic type...
    type = StrToPlyType(line[1]);
    if ((type==PlyUnknown)||(type==PlyList)) {LogAlways("[file.ply] Bad property (0)."); return false;}
    name = tt(line[2]);

    if (name==tt("x")) outInd = 0;
    if (name==tt("y")) outInd = 1;
    if (name==tt("z")) outInd = 2;

    if (name==tt("u")) outInd = 3;
    if (name==tt("v")) outInd = 4;
    
    if (name==tt("r")) outInd = 5;
    if (name==tt("g")) outInd = 6;
    if (name==tt("b")) outInd = 7;
  }
  else
  {
   if (line.Size()==5)
   {
    // List...
     if (str::Compare(line[1],"list")!=0) {LogAlways("[file.ply] Bad property (1)."); return false;}

     name = tt(line[4]);
     type = PlyList;

     listLength = StrToPlyType(line[2]);
     if ((listLength==PlyUnknown)||(listLength==PlyList)) {LogAlways("[file.ply] Bad list length type."); return false;}
     listType = StrToPlyType(line[3]);
     if ((listType==PlyUnknown)||(listType==PlyList)) {LogAlways("[file.ply] Bad list index type."); return false;}

     nat32 inCase = 0;
     if (str::AtEnd(line[4],"_index")) inCase = 1;
     if (str::AtEnd(line[4],"_indices")) inCase = 2;
     if (inCase==0) {LogAlways("[file.ply] Bad index name."); return false;}

     nat32 tl;
     if (inCase==1) tl = str::Length(line[4])-str::Length("_index");
               else tl = str::Length(line[4])-str::Length("_indices");
     cstr t = mem::Malloc<cstrchar>(tl+1);
     mem::Copy(t,line[4],tl);
     t[tl] = 0;
     listIndex = tt(t);
     mem::Free(t);

     if (listIndex==tt("vertex")) faceList = true;
   }
   else {LogAlways("[file.ply] Invalid property line length."); return false;}
  }
  return true;
 }

 void Skip(io::InVirt<io::Binary> & cur,PlyEncode e)
 {
  if (type==PlyList)
  {
   nat32 n = ReadPlyInt(cur,e,listLength);
   for (nat32 i=0;i<n;i++) ReadPlyInt(cur,e,listType);
  }
  else
  {
   ReadPlyReal(cur,e,type);
  }
 }

 void ReadVertex(io::InVirt<io::Binary> & cur,PlyEncode e,real32 data[8])
 {
  if (type==PlyList)
  {
   nat32 n = ReadPlyInt(cur,e,listLength);
   for (nat32 i=0;i<n;i++) ReadPlyInt(cur,e,listType);
  }
  else
  {
   real32 val = ReadPlyReal(cur,e,type);
   if (outInd>=0) data[outInd] = val;
  }
 }

 void ReadFace(io::InVirt<io::Binary> & cur,PlyEncode e,ds::Array<sur::Vertex> & vertDB,sur::Mesh & mesh)
 {
  if (type==PlyList)
  {
   if (faceList)
   {
    ds::Array<sur::Vertex> va(ReadPlyInt(cur,e,listLength));

    bit bad = false;
    for (nat32 i=0;i<va.Size();i++)
    {
     nat32 ind = ReadPlyInt(cur,e,listType);
     if (ind<vertDB.Size()) va[i] = vertDB[ind];
     else
     {
      LogAlways("[file.ply] Bad vertex index." << LogDiv() << ind << LogDiv() << vertDB.Size());
      bad = true;
     }
    }

    if (!bad)
    {
     if (va.Size()>=3) mesh.NewFace(va);
     else {LogAlways("[file.ply] Degenerate face" << LogDiv() << va.Size());}
    }
   }
   else
   {
    nat32 n = ReadPlyInt(cur,e,listLength);
    for (nat32 i=0;i<n;i++) ReadPlyInt(cur,e,listType);
   }
  }
  else
  {
   ReadPlyReal(cur,e,type);
  }
 }
};

struct PlyElement
{
 str::Token name;
 nat32 size;
 ds::List<PlyProperty> prop;

 bit Load(const str::Tokenize & line,str::TokenTable & tt)
 {
  if (line.Size()!=3) {LogAlways("[file.ply] Element line is wrong length."); return false;}
  if (str::Compare(line[0],"element")!=0) {LogAlways("[file.ply] Line is not an element."); return false;}
  name = tt(line[1]);
  size = str::ToInt32(line[2]);
  return true;
 }
};

struct PlyHeader
{
 ~PlyHeader()
 {
  while (elem.Size()!=0)
  {
   delete elem.Front();
   elem.RemFront();
  }
 }

 PlyEncode encode;
 ds::List<PlyElement*> elem;

 bit LoadFormat(const str::Tokenize & line)
 {
  if (line.Size()!=3) {LogAlways("[file.ply] Format line wrong length."); return false;}
  if (str::Compare(line[0],"format")!=0) {LogAlways("[file.ply] Not a format line."); return false;}

  if (str::Compare(line[1],"ascii")==0) encode = PlyAscii;
  else
  {
   if (str::Compare(line[1],"binary_little_endian")==0) encode = PlyLittleEndian;
   else
   {
    if (str::Compare(line[1],"binary_big_endian")==0) encode = PlyBigEndian;
    else
    {
     LogAlways("[file.ply] Unrecognised encoding." << LogDiv() << line[1]);
     return false;
    }
   }
  }

  if (str::Compare(line[2],"1.0")!=0) {LogAlways("[file.ply] Unsuported version." << LogDiv() << line[2]); return false;}
  return true;
 }
};

//------------------------------------------------------------------------------
EOS_FUNC sur::Mesh * LoadPly(cstrconst filename,time::Progress * prog,str::TokenTable * tt)
{
 LogTime("eos::file::LoadPly");
 prog->Push();

 // Open the file...
  File<io::Binary> f(filename,way_edit,mode_read);
  if (!f.Active())
  {
   LogAlways("[file.ply] Could not open file." << LogDiv() << filename);
   prog->Pop();
   return null<sur::Mesh*>();
  }
  Cursor<io::Binary> cur = f.GetCursor();
  io::VirtIn< Cursor<io::Binary> > virtCur = io::VirtIn< Cursor<io::Binary> >(cur);
  nat32 fSize = f.Size();
  prog->Report(0,fSize);



 // We use a token table - either the provided one or a tempory one...
  str::TokenTable * tokTab = tt;
  if (tokTab==null<str::TokenTable*>()) tokTab = new str::TokenTable();



 // Read in the header...
  PlyHeader head;
  bit haveExtra[5]; // u,v,r,g,b.
  for (nat32 i=0;i<5;i++) haveExtra[i] = false;
  

  {
   head.encode = PlyUnknownEncode;
   str::String s;
   s.GetLine(virtCur,true);
   str::Tokenize tok(s);
   if ((tok.Size()!=1)||(str::Compare(tok[0],"ply")!=0)) {LogAlways("[file.ply] Not a ply file." << LogDiv() << tok); return false;}

   while (true)
   {
    prog->Report(fSize-cur.Avaliable(),fSize);
    s.GetLine(virtCur,true);
    tok.Replace(s);
    if (tok.Size()==0) continue; // Skip blank lines.
    if (str::Compare(tok[0],"end_header")==0) break;
    if (str::Compare(tok[0],"format")==0)
    {
     if ((head.encode!=PlyUnknownEncode)||(head.LoadFormat(tok)==false)) {LogAlways("[file.ply] Format error."); return null<sur::Mesh*>();}
    }
    else
    {
     if (str::Compare(tok[0],"comment")==0)
     {
      // Ignore.
     }
     else
     {
      if (str::Compare(tok[0],"element")==0)
      {
       PlyElement * pe = new PlyElement();
       head.elem.AddBack(pe);
       if (pe->Load(tok,*tokTab)==false) {LogAlways("[file.ply] Element error."); return null<sur::Mesh*>();}
      }
      else
      {
       if (str::Compare(tok[0],"property")==0)
       {
        if (head.elem.Size()==0) {LogAlways("[file.ply] Property size error."); return null<sur::Mesh*>();}
        PlyElement * pe = head.elem.Back();
        PlyProperty pp;
        if (pp.Load(tok,*tokTab)==false) {LogAlways("[file.ply] Property error."); return null<sur::Mesh*>();}
        pe->prop.AddBack(pp);
        
        if (pe->name==(*tokTab)("vertex"))
        {
         if (pp.name==(*tokTab)("u")) haveExtra[0] = true;
         if (pp.name==(*tokTab)("v")) haveExtra[1] = true;
         if (pp.name==(*tokTab)("r")) haveExtra[2] = true;
         if (pp.name==(*tokTab)("g")) haveExtra[3] = true;
         if (pp.name==(*tokTab)("b")) haveExtra[4] = true;
        }
       }
       else
       {
        if (str::Compare(tok[0],"obj_info")==0)
        {
         // Ignore.
        }
        else
        {
         // We don't know what it is, quit...
          LogAlways("[file.ply] Unknown header type." << LogDiv() << tok[0]);
          return null<sur::Mesh*>();
        }
       }
      }
     }
    }
   }
  }

  if (head.encode==PlyUnknownEncode) {LogAlways("[file.ply] Header omited format info."); return null<sur::Mesh*>();}


 
 // Create the mesh object, including and uv and rgb properties...
  sur::Mesh * ret = new sur::Mesh(tt);
  data::Property<sur::Vertex,real32> propU;
  data::Property<sur::Vertex,real32> propV;
  data::Property<sur::Vertex,real32> propR;
  data::Property<sur::Vertex,real32> propG;
  data::Property<sur::Vertex,real32> propB;
  if (tt)
  {
   real32 realIni = 0.0;
   if (haveExtra[0]) ret->AddVertProp("u",realIni);
   if (haveExtra[1]) ret->AddVertProp("v",realIni);
   if (haveExtra[2]) ret->AddVertProp("r",realIni);
   if (haveExtra[3]) ret->AddVertProp("g",realIni);
   if (haveExtra[4]) ret->AddVertProp("b",realIni);
   ret->Commit();
   
   if (haveExtra[0]) propU = ret->GetVertProp<real32>("u");
   if (haveExtra[1]) propV = ret->GetVertProp<real32>("v");
   if (haveExtra[2]) propR = ret->GetVertProp<real32>("r");
   if (haveExtra[3]) propG = ret->GetVertProp<real32>("g");
   if (haveExtra[4]) propB = ret->GetVertProp<real32>("b");
  }


 // Do a single pass through the file, building the mesh as we go...
 // Iterate the elements, skip any that arn't vertex or face.
  ds::Array<sur::Vertex> vertDB;
  ds::List<PlyElement*>::Cursor targElem = head.elem.FrontPtr();
  while (!targElem.Bad())
  {
   prog->Report(fSize-cur.Avaliable(),fSize);

   ds::Array<PlyProperty> pa((*targElem)->prop.Size());
   {
    ds::List<PlyProperty>::Cursor targ = (*targElem)->prop.FrontPtr();
    for (nat32 i=0;i<pa.Size();i++)
    {
     pa[i] = *targ;
     ++targ;
    }
   }


   if ((*targElem)->name==(*tokTab)("vertex"))
   {
    // Vertices...
     vertDB.Size((*targElem)->size);
     for (nat32 i=0;i<(*targElem)->size;i++)
     {
      prog->Report(fSize-cur.Avaliable(),fSize);
      
      real32 data[8];
      for (nat32 j=0;j<8;j++) data[j] = 0.0;
      for (nat32 j=0;j<pa.Size();j++) pa[j].ReadVertex(virtCur,head.encode,data);

      vertDB[i] = ret->NewVertex(bs::Vert(data[0],data[1],data[2]));
      
      if (propU.Valid()) propU.Get(vertDB[i]) = data[3];
      if (propV.Valid()) propV.Get(vertDB[i]) = data[4];
      if (propR.Valid()) propR.Get(vertDB[i]) = data[5];
      if (propG.Valid()) propG.Get(vertDB[i]) = data[6];
      if (propB.Valid()) propB.Get(vertDB[i]) = data[7];
     }
   }
   else
   {
    if ((*targElem)->name==(*tokTab)("face"))
    {
     // Faces...
      for (nat32 i=0;i<(*targElem)->size;i++)
      {
       prog->Report(fSize-cur.Avaliable(),fSize);
       for (nat32 j=0;j<pa.Size();j++) pa[j].ReadFace(virtCur,head.encode,vertDB,*ret);
      }
    }
    else
    {
     // Other - skip...
      for (nat32 i=0;i<(*targElem)->size;i++)
      {
       prog->Report(fSize-cur.Avaliable(),fSize);
       for (nat32 j=0;j<pa.Size();j++) pa[j].Skip(virtCur,head.encode);
      }
    }
   }
   ++targElem;
  }



 // Clean up...
  if (tt==null<str::TokenTable*>()) delete tokTab;


 prog->Pop();
 return ret;
}

//------------------------------------------------------------------------------
 };
};
