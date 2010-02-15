//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "eos/file/wavefront.h"

#include "eos/file/files.h"
#include "eos/io/to_virt.h"
#include "eos/ds/arrays_resize.h"
#include "eos/ds/dense_hash.h"


namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
Wavefront::Wavefront()
:cornerCount(0),faceCount(0)
{}

Wavefront::~Wavefront()
{}

bit Wavefront::Save(cstrconst fn,bit ow,time::Progress * prog) const
{
 prog->Push();
 File<io::Text> f(fn,ow?way_ow:way_new,mode_write);
 if (!f.Active()) {prog->Pop(); return false;}

 Cursor<io::Text> out = f.GetCursor();


 // Save vertices...
 nat32 step = 0;
 nat32 steps = vert.Size() + norm.Size() + tex.Size() + surface.Size();
 {
  ds::List<bs::Vert>::Cursor targ = vert.FrontPtr();
  while (!targ.Bad())
  {
   prog->Report(step++,steps);
   out << "v " << targ->X() << " " << targ->Y() << " " << targ->Z() << "\n";
   ++targ;
  }
 }


 // Save texture coordinates...
 {
  ds::List<bs::Tex2D>::Cursor targ = tex.FrontPtr();
  while (!targ.Bad())
  {
   prog->Report(step++,steps);
   out << "vt " << targ->U() << " " << targ->V() << "\n";
   ++targ;
  }
 }


 // Save normals...
 {
  ds::List<bs::Normal>::Cursor targ = norm.FrontPtr();
  while (!targ.Bad())
  {
   prog->Report(step++,steps);
   out << "vn " << targ->X() << " " << targ->Y() << " " << targ->Z() << "\n";
   ++targ;
  }
 }


 // Save faces...
  if (faceCount!=0)
  {
   out << "f ";
   ds::List<Corner>::Cursor targ = surface.FrontPtr();
   while (!targ.Bad())
   {
    prog->Report(step++,steps);
    // Write the corner details...
     out << targ->vertInd << "/";
     if (targ->texInd!=0) out << targ->texInd;
     out << "/";
     if (targ->normInd!=0) out << targ->normInd;

    // Handle end of face...
     if (targ->eof)
     {
      if (targ.End()) out << "\n";
                 else out << "\nf ";
     }
     else
     {
      out << " ";
     }

    ++targ;
   }
  }


 prog->Pop();
 return true;
}

bit Wavefront::Save(const str::String & fn,bit ow,time::Progress * prog) const
{
 cstr s = fn.ToStr();
  bit ret = Save(s,ow,prog);
 mem::Free(s);
 return ret;
}

nat32 Wavefront::Add(const bs::Vert & v)
{
 vert.AddBack(v);
 return vert.Size();
}

nat32 Wavefront::Add(const bs::Normal & n)
{
 norm.AddBack(n);
 return norm.Size();
}

nat32 Wavefront::Add(const bs::Tex2D & t)
{
 tex.AddBack(t);
 return tex.Size();
}

nat32 Wavefront::Add(nat32 vertInd,nat32 normInd,nat32 texInd)
{
 Corner nc;
  nc.vertInd = vertInd;
  nc.normInd = normInd;
  nc.texInd = texInd;
  nc.eof = false;
 surface.AddBack(nc);

 ++cornerCount;
 return cornerCount;
}

nat32 Wavefront::Face()
{
 surface.Back().eof = true;
 cornerCount = 0;
 ++faceCount;
 return faceCount;
}

//------------------------------------------------------------------------------
// This represents a pair of indices, one for a vertex, another for a uv coordinate.
// Zero is used to indicate a null.
struct VertUV
{
 nat32 vertInd;
 nat32 uvInd;
};

// The above, extended to have a handle to the matching vertex.
// Sortable, so it can be indexed by a SortList.
struct IndexedVert
{
 nat32 vertInd;
 nat32 uvInd;
 sur::Vertex vert;
 
 bit operator < (const IndexedVert & rhs) const
 {
  if (vertInd!=rhs.vertInd) return vertInd<rhs.vertInd;
  return uvInd<rhs.uvInd;
 }
};

//------------------------------------------------------------------------------
EOS_FUNC sur::Mesh * LoadWavefront(cstrconst filename,time::Progress * prog,str::TokenTable * tt)
{
 LogTime("eos::file::LoadWavefront");
 prog->Push();


 // Open the file...
  File<io::Text> f(filename,way_edit,mode_read);
  if (!f.Active()) {prog->Pop(); return null<sur::Mesh*>();}
  Cursor<io::Text> cur = f.GetCursor();
  io::VirtIn< Cursor<io::Text> > virtCur = io::VirtIn< Cursor<io::Text> >(cur);
  nat32 fSize = f.Size();
  prog->Report(0,fSize);



 // Create linked lists of the read in vertices and uv's, plus use an 
 // index/data structure for the faces...
  ds::List<bs::Vert> vertList;
  ds::List<bs::Tex2D> uvList;
  
  ds::List<nat32> faceIndexList;
  faceIndexList.AddBack(0); // Always 1 larger than number of faces for dummy.
  ds::List<VertUV> faceDataList;



 // Read everything into the data structure - parse the file line by line...
  str::String line;
  while (line.GetLine(virtCur))
  {
   prog->Report(fSize-cur.Avaliable(),fSize);
   if (line.StartsWith("v "))
   {
    // Vertex...
     str::String::Cursor cur = line.GetCursor();
     cur.Skip(2);

     bs::Vert v;
     cur >> v[0] >> v[1] >> v[2];
     if (!cur.Error())
     {
      vertList.AddBack(v);
     }
   }
   else
   {
    if (line.StartsWith("vt "))
    {
     // UV-coordinate...
      str::String::Cursor cur = line.GetCursor();
      cur.Skip(3);

      bs::Tex2D uv;
      cur >> uv[0] >> uv[1];
      if (!cur.Error())
      {
       uvList.AddBack(uv);
      }
    }
    else
    {
     if (line.StartsWith("f "))
     {
      // Face...
       str::String::Cursor cur = line.GetCursor();
       cur.Skip(2);

       nat32 faceSize = 0;
       while (true)
       {
        int32 vn;
        cur >> vn;
        if (cur.Error()) break;
        byte t[2];
        if (cur.Peek(t,2)!=2) break;
        if (t[0]!='/') break;
        
        int32 uvn = 0;
        if (t[1]!='/')
        {
         cur.Skip(1);
         cur >> uvn;
         if (cur.Error()) break;
        }
        
        cur.SkipTo(' ');
        
        VertUV fv;
        if (vn>=0) fv.vertInd = vn;
              else fv.vertInd = int32(vertList.Size()) + vn;
        if (uvn>=0) fv.uvInd = uvn;
               else fv.uvInd = int32(uvList.Size()) + uvn;
        
        if (vn==0)
        {
         LogDebug("[wavefront] Face has bad vertex index. (" << faceSize << ")");
         return null<sur::Mesh*>();
        }

        faceDataList.AddBack(fv);
        ++faceSize;
       }
       
       if (faceSize<3)
       {
        LogDebug("[wavefront] Face has insufficient vertices. (" << faceSize << ")");
        return null<sur::Mesh*>();
       }

       faceIndexList.AddBack(faceIndexList.Back() + faceSize);
     }
    }
   }
  }
  
  
  
 // Convert constructed lists into arrays...
  ds::Array<bs::Vert> vertArray(vertList.Size());  
  {
   ds::List<bs::Vert>::Cursor targ = vertList.FrontPtr();
   for (nat32 i=0;i<vertArray.Size();i++)
   {
    vertArray[i] = *targ;
    ++targ;
   }
  }

  ds::Array<bs::Tex2D> uvArray(uvList.Size());
  {
   ds::List<bs::Tex2D>::Cursor targ = uvList.FrontPtr();
   for (nat32 i=0;i<uvArray.Size();i++)
   {
    uvArray[i] = *targ;
    ++targ;
   }
  }
  
  ds::Array<nat32> faceIndex(faceIndexList.Size()); 
  {
   ds::List<nat32>::Cursor targ = faceIndexList.FrontPtr();
   for (nat32 i=0;i<faceIndex.Size();i++)
   {
    faceIndex[i] = *targ;
    ++targ;
   }
  }
  
  ds::Array<VertUV> faceData(faceDataList.Size());
  {
   ds::List<VertUV>::Cursor targ = faceDataList.FrontPtr();
   for (nat32 i=0;i<faceData.Size();i++)
   {
    faceData[i] = *targ;
    ++targ;
   }
  }



 // Create the mesh, add uv coordinates if they have been supplied...
  sur::Mesh * ret = new sur::Mesh(tt);
  data::Property<sur::Vertex,real32> propU;
  data::Property<sur::Vertex,real32> propV;
  
  if ((tt!=null<str::TokenTable*>())&&(uvArray.Size()!=0))
  {
   real32 realIni = 0.0;
   ret->AddVertProp("u",realIni);
   ret->AddVertProp("v",realIni);
   ret->Commit();
   
   propU = ret->GetVertProp<real32>("u");
   propV = ret->GetVertProp<real32>("v");
  }


 // Create tracking data structure, to track each vertex/uv pair that 
 // is created so no duplicates are made...
  ds::SortList<IndexedVert> tds;


 // Iterate all the faces, creating there vertice as they request them
 // and avoiding duplicates using the tracking data structure...
  ds::Array<sur::Vertex> verts;
  for (nat32 i=0;i<faceIndex.Size()-1;i++)
  {
   // Go throgh the faces vertices and fill in the vertex array,
   // creating new vertices as needed...
    verts.Size(faceIndex[i+1]-faceIndex[i]);
    for (nat32 j=faceIndex[i];j<faceIndex[i+1];j++)
    {
     IndexedVert dummy;
     dummy.vertInd = faceData[j].vertInd;
     dummy.uvInd = faceData[j].uvInd;
     
     IndexedVert * found = tds.Get(dummy);
     if (found)
     {
      //  Already created - reuse...
       verts[j-faceIndex[i]] = found->vert;
     }
     else
     {
      // Doesn't exist - create it...
       dummy.vert = ret->NewVertex(vertArray[dummy.vertInd-1]);
       if ((dummy.uvInd!=0)&&(propU.Valid()&&propV.Valid()))
       {
        propU.Get(dummy.vert) = uvArray[dummy.vertInd-1][0];
        propV.Get(dummy.vert) = uvArray[dummy.vertInd-1][1];
       }
       
       tds.Add(dummy);
       verts[j-faceIndex[i]] = dummy.vert;
     }
    }
  
   // Create the face...
    ret->NewFace(verts);
  }


 // Clean up and return...
  f.Close();
  prog->Pop();
  return ret;
}

//------------------------------------------------------------------------------
 };
};
