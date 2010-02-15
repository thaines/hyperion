#ifndef EOS_FILE_PLY_H
#define EOS_FILE_PLY_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file ply.h
/// Provides capability for saving the ply file format.

#include "eos/types.h"
#include "eos/file/files.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/lists.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// Class that represents a ply file for saving, allows you to add data
/// and Save the file. Always outputs as binary.
/// The vertex convention is anti-clockwise when you are looking at the front of
/// a face.
/// Requires uv's for every vertex.
class EOS_CLASS Ply
{
 public:
  /// &nbsp;
   Ply();

  /// &nbsp;
   ~Ply();


  /// Saves the wavefront file, traditionaly such files have a .obj extension.
  /// Set ow to true to overwrite and existing file. Returns true on success.
   bit Save(cstrconst fn,bit ow = false,time::Progress * prog = null<time::Progress*>()) const;

  /// Saves the wavefront file, traditionaly such files have a .obj extension.
  /// Set ow to true to overwrite and existing file. Returns true on success.
   bit Save(const str::String & fn,bit ow = false,time::Progress * prog = null<time::Progress*>()) const;


  /// Adds a new vertex, returning its index number. You must also supply a
  /// uv-coordinate, which defaults to (0,0) if you do not.
   nat32 Add(const bs::Vert & vert,const bs::Tex2D & tex = bs::Tex2D(0.0,0.0));


  /// This adds a vertex for the current face, supply the index provided by Add.
  /// Returns the number of vertices added to the face so far.
   nat32 Add(nat32 vertInd);


  /// This ends the current face, so you can start on another, returns
  /// the index of the surface just finished. The face will contain all the
  /// vertInd's Add'ed since the last surface end call.
   nat32 Face();


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::file::Ply";}


 private:
  struct V
  {
   bs::Vert pos;
   bs::Tex2D uv;
  };
  ds::List<V> vert;

  nat32 cornerCount;
  nat32 faceCount;
  ds::List<nat32> surface; // Faces are deliminated by nat32(-1).
};

//------------------------------------------------------------------------------
 };
};

#include "eos/sur/mesh.h"

#ifdef EOS_SUR_MESH_H // Dam circular dependancies.
namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// This function loads a wavefront file, sticking the output into sur::Mesh.
/// You must delete the returned mesh when done with it.
/// Returns null on error.
/// This obviously only loads mesh data, any other data in the wavefront file
/// will be ignored.
/// If uv or rgb data is provided it will be loaded, but only if a token table is given.
EOS_FUNC sur::Mesh * LoadPly(cstrconst filename,
                             time::Progress * prog = null<time::Progress*>(),
                             str::TokenTable * tt = null<str::TokenTable*>());

//------------------------------------------------------------------------------
 };
};
#endif
#endif
