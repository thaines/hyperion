#ifndef EOS_VERSION_H
#define EOS_VERSION_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file version.h
/// \brief Defines various functionally irrelevent information.
/// This file contains functions that return the copyright message and version 
/// number. It also documents the defines relevent to compiling the eos library
/// and allows querying of the options used during the librarys compilation.

#include "eos/types.h"
#include "eos/io/inout.h"

namespace eos
{
//------------------------------------------------------------------------------
/// \def EOS_VERSION_INTERFACE
/// The interface version number. Incrimented when it is no longer back compatable with the previos release.

/// \def EOS_VERSION_RELEASE
/// The release version number. Incrimented with each new set of features.

/// \def EOS_VERSION_MINOR
/// The minor version number. Incrimented with each new set of bug fixes.

#define EOS_VERSION_INTERFACE 1
#define EOS_VERSION_RELEASE 0
#define EOS_VERSION_MINOR 0
#define EOS_VERSION_STRING "1.0.0"

//------------------------------------------------------------------------------
/// \struct Version
/// Structure used to indicate a version number.
struct EOS_CLASS Version
{
 int interface; ///< Interface version, incrimented when back compatability is lost.
 int release;   ///< Release version, incrimented with each proper release with new features.
 int minor;     ///< Minor version, incrimented to indicate non-feature-set changes, bug fixes ushally.
 
 /// &nbsp;
  inline static cstrconst TypeString() {return "eos::Version";}
};

// To make it work with streams...
namespace io
{
 template <typename T>
 inline T & StreamRead(T & lhs,Version & rhs,Binary)
 {
  lhs >> rhs.interface >> rhs.release >> rhs.minor;
  return lhs;
 }
 
 template <typename T>
 inline T & StreamWrite(T & lhs,const Version & rhs,Binary)
 {
  lhs << rhs.interface << rhs.release << rhs.minor;
  return lhs;
 }
 
 template <typename T>
 inline T & StreamWrite(T & lhs,const Version & rhs,Text)
 {
  lhs << rhs.interface << "." << rhs.release << "." << rhs.minor;
  return lhs;
 } 
};

/// Provides the dll's version number.
/// \param out The given Version structure is filled with the relevent details.
EOS_FUNC void GetVersion(Version & out);

/// Returns the copyright string for the dll.
/// \return A const copyright message string.
EOS_FUNC cstrconst GetCopyright();

/// Returns a compilation specific string, includes the compilation date as
/// well as the version.
EOS_FUNC cstrconst GetBuild();

//------------------------------------------------------------------------------
/// \enum Os {win32,linux}
/// Indicates the operating system type the library has been compiled for.
enum Os {win32,lin};

/// \enum Chipset {x86,x86_64,power}
/// Indicates the chipset the system has been compiled for.
enum Chipset {x86,x86_64,power};

//------------------------------------------------------------------------------
/// Returns true if it is a debug build, false otherwise.
EOS_FUNC bit IsDebug();

/// \fn EOS_FUNC Os GetOs()
/// Returns the Operating System the dll has been compiled for.
/// \return Returns your (Hopefully) current OS.
EOS_FUNC Os GetOs();

/// \fn EOS_FUNC Chipset GetChipset()
/// Returns the chipset the dll has been compiled for.
/// \return Returns the chipset EOS beleives its running on.
EOS_FUNC Chipset GetChipset();

/// \fn EOS_FUNC boolean WithAsm()
/// Indicates if eos was compiled with assembler optimisations.
/// \return true if compiled with assembler optimisations, false if without such optimisations.
EOS_FUNC bit WithAsm();

//------------------------------------------------------------------------------
// Tempory section, to throw errors if attempts are made to compile with
// (currently) unsuported options...
#ifdef EOS_X86_64
#error X86_64 compilation option not yet suported
#endif

#ifdef EOS_POWER
#error POWER compilation option not yet suported
#endif

//------------------------------------------------------------------------------
};
#endif
