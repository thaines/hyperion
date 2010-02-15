//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
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
