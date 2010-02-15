#ifndef EOS_TYPES_H
#define EOS_TYPES_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file types.h
/// This file exists to typedef all the types used by the system, so they can
/// be changed for awkward compilers. It also defines tags to use on classes
/// and functions to be externally visible to users of the dll.

/// \namespace eos
/// The base of the namespace hierachy, into which the entire library is packed.
/// The root namespace only contains the eos basic types, children namespaces
/// contain the library itself. See the rest of this documentation for details.

#include <sys/types.h>

#ifdef EOS_LINUX
#include <stdint.h>
#endif

#ifdef EOS_DEBUG
#include <stdlib.h>
#endif

#include <stdio.h> // Tempory, till real logging is added.

namespace eos
{
//------------------------------------------------------------------------------
#ifdef __LP64__
#define EOS_64BIT
#else
#define EOS_32BIT
#endif

//------------------------------------------------------------------------------
/// \def EOS_CLASS
/// \brief Tag for externally visible classes.
/// All classes are tagged with this define, which puts the relevent tags for
/// the current compiler in to make the class externally visible when compiling
/// the dll, or when linking with the library tell the compiler it needs to 
/// import the class.

/// \def EOS_FUNC
/// \brief Tag for externally visible functions.
/// All externally visible functions are tagged with this label, to externalise
/// them when compiling the dll and import them when linking to the library.

/// \def EOS_VAR
/// All variables exported from eos are declared with this define in the .h file,
/// to indicate there avaliability.

/// \def EOS_VAR_DEF
/// All variables exported from eos are declared with this define for the actual
/// instantiation in a .cpp file.

#ifdef EOS_WIN32
 #ifdef EOS_DLL
  #define EOS_CLASS __declspec(dllexport)
 #else
  #define EOS_CLASS __declspec(dllimport)
 #endif
 #define EOS_STDCALL __stdcall
#else
 #define EOS_CLASS
 #define EOS_STDCALL
#endif
#define EOS_FUNC extern EOS_CLASS
#define EOS_VAR extern EOS_CLASS
#define EOS_VAR_DEF EOS_CLASS

//------------------------------------------------------------------------------

/// \var typedef unsigned __int8  nat8
/// Natural number type, 0..2^8-1

/// \var typedef unsigned __int16 nat16
/// Natural number type, 0..2^16-1

/// \var typedef unsigned __int32 nat32
/// Natural number type, 0..2^32-1

/// \var typedef unsigned __int64 nat64
/// Natural number type, 0..2^64-1

#ifdef EOS_WIN32
 typedef unsigned __int8  nat8;
 typedef unsigned __int16 nat16;
 typedef unsigned __int32 nat32;
 typedef unsigned __int64 nat64;
#else
 typedef uint8_t  nat8;
 typedef uint16_t nat16;
 typedef uint32_t nat32;
 typedef uint64_t nat64;
#endif

//------------------------------------------------------------------------------

/// \var typedef __int8  int8
/// Integer number type, -2^7..2^7-1

/// \var typedef __int16 int16
/// Integer number type, -2^15..2^15-1

/// \var typedef __int32 int32
/// Integer number type, -2^31..2^31-1

/// \var typedef __int64 int64
/// Integer number type, -2^63..2^63-1

#ifdef EOS_WIN32
 typedef __int8  int8;
 typedef __int16 int16;
 typedef __int32 int32;
 typedef __int64 int64;
#else
 typedef int8_t  int8;
 typedef int16_t int16;
 typedef int32_t int32;
 typedef int64_t int64;
#endif

//------------------------------------------------------------------------------

/// \var typedef float  real32
/// \brief 32 bit floating point type.
/// The 32 bit floating point type is concidered to be for fast calculations,
/// as such maths done with these is generally inaccurate, but as fast as 
/// possible. For acurate floating point maths see eos::real64.

/// \var typedef double real64
/// \brief 64 bit floating point type.
/// The 64 bit floating point type is concidered to be for slow but accurate
/// calcualtions. As such maths done with these is a lot slower than eos::real32
/// but probably useful for purposes that are not presentation related.

typedef float  real32;
typedef double real64;

//------------------------------------------------------------------------------

/// \var typedef bool bit
/// The boolean type, for true/false values.

/// \var typedef nat8 byte
/// An alternate name for a ::nat8, used for generic data massaging.
/// This is used for all general data proccessing purposes, when dealing with
/// arbitary blocks of data. Ushally used for pointing to blocks of bytes.

/// \var typedef char cstrchar
/// A single character, for c-style string manipulation.

/// \var typedef cstrchar * cstr
/// A c-style string, for c-style string manipulation.

/// \var typedef const char * cstrconst
/// A constant c-style string, for c-style string manipulation.

/// \fn template<typename T> inline  * null()
/// Used to indicate a null pointer.
/// The crazyness of declaring it as a templated function is to enforce typing,
/// to make sure a null can not be interpreted as an integer 0. This catches 
/// a certain class of, admitedly rare, bugs, and makes the coder think about 
/// what there doing. Which is a good thing. And hate me for making this part 
/// of eos and forcing them to type all those extra letters. Which is not so good.
/// \return Returns 0. The key point is its a typed 0.

typedef bool bit;
typedef nat8 byte;

typedef char cstrchar;
typedef cstrchar * cstr;
typedef const char * cstrconst;

template<typename T> 
inline T null()
{return 0;}

//------------------------------------------------------------------------------
/// A basic swap method. Note that a more advanced one for large memory blocks
/// is avaliable in the mem namespace.
template <typename T>
inline void Swap(T & a,T & b)
{
 T temp = a;
 a = b;
 b = temp;	
}

//------------------------------------------------------------------------------
/// \def EOS_APP_START
/// This translates to either int main() or int Winmain(...) depending on which
/// platform you are on, to be used as the head of the main function for 
/// gui based programs.

#ifdef EOS_LINUX
#define EOS_APP_START int main()
#else
typedef struct HINSTANCE__ {int i;} * HINSTANCE;
#define EOS_APP_START extern "C" int EOS_STDCALL WinMain(HINSTANCE,HINSTANCE,char*,int)
#endif

//------------------------------------------------------------------------------
/// A structure that contains nothing, has its uses when dealing with templates 
/// in certain situations.
struct Nothing
{};

//------------------------------------------------------------------------------
/// A simple pair template, can be quite useful for certain scenarios.
template <typename A,typename B>
class EOS_CLASS Pair
{
 public:
  A first;  ///< &nbsp;
  B second; ///< &nbsp;
};

//------------------------------------------------------------------------------
/// A class with a virtual deconstructor, the base class of most virtual objects
/// in the system, so that deletable objects can be stored together.
class EOS_CLASS Deletable
{
 public:
  /// &nbsp;
   virtual ~Deletable() {}
};

//------------------------------------------------------------------------------
/// A class which suports reference counting, via Acquire/Release methods.
class EOS_CLASS RefCounter : public Deletable
{
 public:
  /// Initialised with a reference count of 0, so you must call Acquire after
  /// construction if you yourself are keeping a pointer to it.
   RefCounter():refCount(0) {}
   
  /// &nbsp;
   ~RefCounter() {}
   
   
  /// &nbsp;
   void Acquire() {++refCount;}
  
  /// Safe to call on null pointers.
   void Release() 
   {
    if (this==null<RefCounter*>()) return;
    if (refCount!=0) --refCount;
    if (refCount==0) delete this;
   }


 private:
  nat32 refCount;
};

//------------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
// An unushall practise, we include the memory handling header here, because it 
// overloads the global new/delete, and to only use it for some parts of the
// library and not others would do nasty things. Like, really nasty things.
// But as even the most basic code requires this header we make sure by 
// including it here.

 #include "eos/mem/alloc.h"
 
// Again, we also include the logging system from here, so its universally
// accesable. (As a side affect most of the io system also becomes avaliable.)

 #include "eos/log/logs.h" 

//------------------------------------------------------------------------------
#endif
