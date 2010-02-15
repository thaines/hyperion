//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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

#include "eos/file/meshes.h"

#include "eos/file/wavefront.h"
#include "eos/file/ply.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
EOS_FUNC bit SaveMesh(const sur::Mesh & mesh,cstrconst filename,
                      bit overwrite,time::Progress * prog)
{
 if (str::AtEnd(filename,".obj"))
 {
  Wavefront out;
  mesh.Store(out);
  return out.Save(filename,overwrite,prog);
 }
 else
 {
  Ply out;
  mesh.Store(out);
  return out.Save(filename,overwrite,prog);  
 }
}

EOS_FUNC bit SaveMesh(const sur::Mesh & mesh,const str::String & filename,
                       bit overwrite,time::Progress * prog)
{
 cstr fn = filename.ToStr();
 bit ret = SaveMesh(mesh,fn,overwrite,prog);
 mem::Free(fn);
 return ret;
}

//------------------------------------------------------------------------------
EOS_FUNC sur::Mesh * LoadMesh(cstrconst filename,
                              time::Progress * prog,str::TokenTable * tt)
{
 if (str::AtEnd(filename,".obj"))
 { 
  return LoadWavefront(filename,prog,tt);
 }
 else
 {
  return LoadPly(filename,prog,tt);
 }
}

EOS_FUNC sur::Mesh * LoadMesh(const str::String & filename,
                              time::Progress * prog,str::TokenTable * tt)
{
 cstr fn = filename.ToStr();
 sur::Mesh * ret = LoadMesh(fn,prog,tt);
 mem::Free(fn);
 return ret;
}

//------------------------------------------------------------------------------
 };
};
