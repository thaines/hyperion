#ifndef EOS_FILE_MESHES_H
#define EOS_FILE_MESHES_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file meshes.h
/// Provides generic loading and saving of 3D models - simply a router for the
/// various model types suported by eos in general.

#include "eos/types.h"#include "eos/str/strings.h"
#include "eos/sur/mesh.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// Saves a Mesh, if the file extension is .obj it saves it using the wavefront
/// file format, any other extension and it uses the .ply format.
/// Returns true on success.
EOS_FUNC bit SaveMesh(const sur::Mesh & mesh,cstrconst filename,bit overwrite = false,
                      time::Progress * prog = null<time::Progress*>());

/// Saves a Mesh, if the file extension is .obj it saves it using the wavefront
/// file format, any other extension and it uses the .ply format.
/// Returns true on success.
EOS_FUNC bit SaveMesh(const sur::Mesh & mesh,const str::String & filename,bit overwrite = false,
                      time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// Loads a mesh, works out what kind of file it is and uses the correct loader
/// accordingly.
/// The user is responsable for calling delete on the Mesh when they are done
/// with it.
EOS_FUNC sur::Mesh * LoadMesh(cstrconst filename,
                              time::Progress * prog = null<time::Progress*>(),
                              str::TokenTable * tt = null<str::TokenTable*>());

/// Loads a mesh, works out what kind of file it is and uses the correct loader
/// accordingly.
/// The user is responsable for calling delete on the Mesh when they are done
/// with it.
EOS_FUNC sur::Mesh * LoadMesh(const str::String & filename,
                              time::Progress * prog = null<time::Progress*>(),
                              str::TokenTable * tt = null<str::TokenTable*>());

//------------------------------------------------------------------------------
 };
};
#endif
