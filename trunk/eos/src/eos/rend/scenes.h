#ifndef EOS_REND_SCENES_H
#define EOS_REND_SCENES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file scenes.h
/// Provides translation of an XMl file into a set of Job's, which can then be 
/// rendered. (One Job per camera view.)

/// \page rend_scene File Format of eos::rend::Scene XML Files
/// Evidently these are XMl files, with assorted nodes to define a scene to be 
/// rendered. The basic structure of a scene hierachy is allways:
///
/// <scene ...>\n	<objects...>\n</scene>
///
/// The objects form a hierachy with <transform .../> nodes indicating 
/// transformations and various other nodes at the leaves defining 
/// objects. A special <camera .../> node indicates that something is to 
/// be rendered, and defines a camera, details of the renderer to be used
/// and the outputs to save. Each camera node in a scene becomes a job.
///
/// Documentation of each node type now follows, except for <scene/> which is
/// simply a container type with no attributes.
///
/// \section scene_background <background/>
///
/// \section scene_transform <transform/>
///
/// \section scene_camera <camera/>
///
/// \section scene_sphere <sphere/>
///
/// \section scene_material <material/>
///
/// \section scene_texture <texture/>
///

#include "eos/types.h"
#include "eos/ds/arrays.h"
#include "eos/ds/stacks.h"
#include "eos/bs/dom.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// This represents a stack of Transform objects, it allows you to grab a 
/// pointer to an OpTran for the top stack level, which it will automatically
/// register with a previously given Job.
class EOS_CLASS TransformStack
{
 public:
  /// All OpTran objects requested are registered with this Job, this being the
  /// job currently worked on.
   TransformStack(Job * regWith);
   
  /// &nbsp;
   ~TransformStack();


  /// You can push the inverse transformation on if you set invert to true,
  /// useful when working back from the camera to the root before iterating 
  /// all the objects.
   void Push(const Transform & in,bit invert = false);
   
  /// &nbsp;
   void Pop();
   
   
  /// Returns the full transformation of the current location. This object 
  /// returned will be registered the Job. If called multiple times without
  /// a Push/Pop it will return the same pointer each time.
   OpTran * Current();


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::TransformStack";}


 private:
  Job * regWith;

  struct Node
  {
   Transform t; // Multiplied with all earlier transforms on the stack.
   OpTran * tran; // Null if it hasn't been generated yet.
  };
  ds::Stack<Node> stack;
};

//------------------------------------------------------------------------------
/// This class represents a scene, where a scene can have multiple views and 
/// therefore consists of multiple jobs. Its most valuable function is the Load 
/// method, which takes an XMl file describing a scene and creates all relevent 
/// jobs. See the \link rend_scene scene XML page \endlink for details of the 
/// file format it loads.
class EOS_CLASS Scene
{
 public:
  /// &nbsp;
   Scene();
   
  /// Will delete all contained Jobs.
   ~Scene();
   
  
  /// The fills the Scene object, deleting any previous Jobs, with
  /// the contents of the given scene.
  /// Returns true if it parsed correctly, false if it failed to parse.
  /// Note that it will still return true if it doesn't recognise nodes, 
  /// it will just ignore them, as long as there not problamatic to 
  /// understanding everything as a whole.
  /// It writes all problems and warnings to the log.
  /// See \link rend_scene here \endlink for details on the file format.
   bit Load(bs::Element * scene);


  /// Returns how many jobs exist.
   nat32 Jobs() const;
   
  /// Provides access to any given job.
   Job * operator[] (nat32 i);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::Scene";}


 protected:
  ds::Array<Job*> data;
};

//------------------------------------------------------------------------------
 };
};
#endif
