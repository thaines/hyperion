#ifndef EOS_SVT_CALCULATION_H
#define EOS_SVT_CALCULATION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file calculation.h
/// Provides an abstraction of a proccess that takes in a set of svt::Field objects
/// and a set of parameters in the form of a svt::Meta and outputs a set of
/// svt::Field/svt::Meta results. Provides for reflection, factory integration
/// and self-documentation without construction. This forms the basis of the
/// aegle plugin system, with all plugins implimenting this system.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/bs/dom.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
/// This is an algorithm. All instances are presumed run once, taking a standard
/// pattern of supplying input and output svt's then calling the Run method.
/// Designed for ease of use by the implimenter of the algorithm more so than
/// the caller of the algorithm.
class EOS_CLASS Algorithm : public Deletable
{
 public:
  /// Has to be constructed with the number of inputs and outputs.
   Algorithm(nat32 inputs,nat32 outputs);

  /// &nbsp;
   ~Algorithm();


  /// Sets an input for the algorithm, must be called before Run is called for
  /// all required inputs.
  /// The given pointer must remain valid until after Run has returned.
   void SetInput(nat32 input,Node * node);

  /// Runs the algorithm, you supply a progress feedback object.
   virtual void Run(time::Progress * prog = null<time::Progress*>()) = 0;

  /// Returns the output of the algorithm for the given index.
  /// Must be called after Run, its just an array access but the user is
  /// responsible for deleting all outputs so ultimatly each output must have
  /// delete called once only.
   Node * GetOutput(nat32 output);


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;


 protected:
  /// Allows the Run implimentation to get each input. They can be requested
  /// repetadly, its just an array lookup.
   Node * GetInput(nat32 input);

  /// Allows the Run implimentation to set its outputs, must be done for all
  /// non-optional outputs, not doing so is considered an error.
  /// Must be done once for each as otherwise a memory leak will occur.
   void SetOutput(nat32 output,Node * node);


 private:
  nat32 inputs;
  ds::Array<Node*> io; // Contains inputs and then outputs, size is inputs+outputs of constructor.
};

//------------------------------------------------------------------------------
/// This provides all details on an algorithm, primarilly its inputs, outputs
/// and help details. It also acts as a factory for the algorithm object itself.
/// When scaning a plugin directory what Aegle will do is obtain a set of these,
/// rather than the algorithms themselves, as these will be very lightweight.
class EOS_CLASS MetaAlgorithm : public Deletable
{
 public:
  /// &nbsp;
   ~MetaAlgorithm();


  /// Creates a new instance of the related Algorithm object, to be used then
  /// discarded.
   virtual Algorithm * Make() const = 0;


  /// Returns the name of the algorithm , fully qualified.
  /// This is used to request the algorithm by users of the system.
   virtual cstrconst Name() const = 0;

  /// Returns general help for the algorithm.
   virtual cstrconst Help() const = 0;


  /// Returns how many inputs there are.
   virtual nat32 Inputs() const = 0;

  /// Returns the name for an input, used when referencing it by the user.
   virtual cstrconst InputName(nat32 input) const = 0;

  /// Returns true if the given input is optional, false if it must be supplied.
  /// Defaults to false for all.
   virtual bit InputOptional(nat32 input) const;

  /// Returns an arbitary block of text detailing the given input for the user.
   virtual cstrconst InputHelp(nat32 input) const = 0;

  /// Returns a Type object that the input should conform to.
  /// Can be null for no constraint, but this is not recomended.
   virtual const Type * InputType(nat32 input) const = 0;


  /// Returns how many outputs there are.
   virtual nat32 Outputs() const = 0;

  /// Returns the name for an output, used when referencing it by the user.
   virtual cstrconst OutputName(nat32 output) const = 0;

  /// Returns true if the given output is optional, false if it will allways be supplied.
  /// Defaults to false for all.
   virtual bit OutputOptional(nat32 output) const;

  /// Returns an arbitary block of text detailing the given output for the user.
   virtual cstrconst OutputHelp(nat32 output) const = 0;

  /// Returns a Type object that the output will conform to.
  /// Can be null for no constraint, but this is not recomended as it will
  /// result in the relevant link being unchecked.
   virtual const Type * OutputType(nat32 output) const = 0;


  /// Given an arbitary block of XML that will be given to the algorithm this
  /// checks its validity, returns true for ok, false if there is a problem.
  /// Must support extra information that the algorithm will not use, primarilly
  /// so the XML can include position information for representing a set of
  /// connected algorithms graphically.
  /// Default version returns true, as most algorithms will not use this feature.
  /// The element passed will generally be the element that states the algorithm
  /// etc, it is its child elements you should be checking.
   virtual bit GoodDom(bs::Element * elem) const;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
 };
};
#endif
