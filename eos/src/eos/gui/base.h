#ifndef EOS_GUI_BASE_H
#define EOS_GUI_BASE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::gui
/// This provides a gui system. Provides its own
/// abstraction layer to any backend I choose to use, at present the only
/// backend avaliable is gtk. This is very much a work in progress system, 
/// where I just add the features I require when I require them, so its capability
/// is often patchy. There are several reasons for doing this, in part is allows
/// the eos dll to dynamically link to gtk, so you can run an eos based program
/// that has no gui on a computer that has no gui capability. It also allows other
/// systems to provide gui capability, this will latter be extended to html based
/// guis using the same interface, to run under high speed OpenGL based
/// applications where the gtk model is inappropriate and to provide remote gui
/// capability.

/// \file gui/base.h
/// Provides three objects:
/// - An object that all widgets and other gui objects will inherit from.
/// - An object factory for gui objects, abstract so you make one of these for each interface type, gtk, html etc.

#include "eos/types.h"
#include "eos/mem/managers.h"
namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
/// This is a base object in the gui system, from which all parts inherit. It 
/// provides the concept of a parent, as all gui elements must have one, except
/// for the root object which is allways of type App. When an object of type 
/// Base is deleted it is responsable for Release() -ing all its children objects.
class EOS_CLASS Base : public RefCounter
{
 public:
  /// &nbsp;
   Base();

  /// &nbsp;
   ~Base();


  /// &nbsp; 
   Base * Parent() const;
   
  /// Returns the very top of the hierachy its a member of, this should be of type App.
   Base * SuperParent() const;
   
  /// Should only be called when the parent structure(s) is/are updated accordingly.
   void SetParent(Base * np);
   
  /// All parent nodes have to allow there children to disconect themselves, therefore
  /// all gui elements have to provide a Detach method. This should only be called for
  /// children of the object in question. After a call to this the object it is called
  /// on should have no reference to the child in question and the child should have 
  /// had its parent set to n ull, however it should n ot call Release() on the child, as
  /// the caller might want to still play with the child before its potential death.
  /// Therefore call calls to Detach *must* have a matched call to Release() on the child.
   virtual void Detach(Base * child);
   
  // For internal use only, allows 'special data' to be extracted for whatever nefarious
  // purpose is required to make the system work.
   virtual void * Special() = 0;


  /// &nbsp; 
   virtual cstrconst TypeString() const = 0;


 protected:
  Base * parent;
};

//------------------------------------------------------------------------------
/// Provides the ability to construct gui objects by name, even including the 
/// App object. Should accept the TypeString() names of the classes, requires a
/// token table on initialisation. One of these is implimented for each gui type,
/// so that one of these provides gtk objects, whilst another provides html 
/// objects etc. Simply a custom version of the Factory provided by the mem namespace.
/// A gui app will ushally initialise one of these as the second step, after
/// creating a token table for the application. An implimentation of this object
/// is also responsible for initialising and destroying the gui system, hence the
/// Active() method which indicates if the gui type is suported or not.
/// (In addition to supporting the 'eos::gui::thing' name for getting objects just
/// the 'thing' should also be suported.)
class EOS_CLASS Factory : public mem::ObjectFactory<Base,Factory*> 
{
 public:
  /// &nbsp;
   Factory(str::TokenTable & tokTab);

  /// &nbsp;
   virtual ~Factory();

  /// Returns true if the interface type initialised correctly.
   virtual bit Active() const = 0;


  /// &nbsp; 
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
 };
};
#endif
