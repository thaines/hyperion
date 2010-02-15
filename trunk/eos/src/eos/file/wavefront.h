#ifndef EOS_FILE_WAVEFRONT_H
#define EOS_FILE_WAVEFRONT_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file wavefront.h
/// Provides capability for saving/loading the wavefront 3D file format.

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
/// Class that represents a wavefront file for saving, allows you to add data
/// and Save the file. Only suports text files. The capability is limited to
/// simply vertices/edges/faces data only, with no suport for advanced surfaces
/// or materials/textures, though it does allow output of UV coordinates.
/// The vertex convention is anti-clockwise when you are looking at the front of
/// a face. If normals are not provided for a face the face is rendered flat.
/// All indexes are 1 based instead of 0 based, as thats what the wavefront file
/// format does and allows 0 to be used as a dummy.
class EOS_CLASS Wavefront
{
 public:
  /// &nbsp;
   Wavefront();

  /// &nbsp;
   ~Wavefront();


  /// Saves the wavefront file, traditionaly such files have a .obj extension.
  /// Set ow to true to overwrite and existing file. Returns true on success.
   bit Save(cstrconst fn,bit ow = false,time::Progress * prog = null<time::Progress*>()) const;

  /// Saves the wavefront file, traditionaly such files have a .obj extension.
  /// Set ow to true to overwrite and existing file. Returns true on success.
   bit Save(const str::String & fn,bit ow = false,time::Progress * prog = null<time::Progress*>()) const;


  /// Adds a new vertex, returning its index number.
   nat32 Add(const bs::Vert & vert);

  /// Adds a new normal, returning its index number.
   nat32 Add(const bs::Normal & norm);

  /// Adds a new texture coordinate, returning its index number.
   nat32 Add(const bs::Tex2D & tex);


  /// This adds a vertex for the current face, returning the
  /// number of vertices that have been added to the face so far.
  /// \param vertInd The number returned by Add for a bs::Vert.
  /// \param normInd The number returned by Add for a bs::Normal, or 0 to indicate it should use the obvious normal.
  /// \param texInd The number returned by Add for a bs::Tex2D, or 0 to indicate none suplied.
   nat32 Add(nat32 vertInd,nat32 normInd = 0,nat32 texInd = 0);


  /// This ends the current face, so you can start on another, returns
  /// the index of the surface just finished. The face will contain all the
  /// vertInd's Add'ed since the last surface end call.
   nat32 Face();


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::file::Wavefront";}


 private:
  ds::List<bs::Vert> vert;
  ds::List<bs::Normal> norm;
  ds::List<bs::Tex2D> tex;

  struct Corner
  {
   nat32 vertInd;
   nat32 normInd;
   nat32 texInd;
   bit eof; // true if End of face, next Corner will start another.
  };

  nat32 cornerCount;
  nat32 faceCount;
  ds::List<Corner> surface;
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
/// If suplied with a token table and the file has UV's they will be loaded.
EOS_FUNC sur::Mesh * LoadWavefront(cstrconst filename,
                                   time::Progress * prog = null<time::Progress*>(),
                                   str::TokenTable * tt = null<str::TokenTable*>());

//------------------------------------------------------------------------------
 };
};
#endif
#endif
