#ifndef EOS_MEM_ALLOC_H
#define EOS_MEM_ALLOC_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file alloc.h
/// Contains templated equivalents to C style memory functions.

/// \namespace eos::mem
/// Provides basic malloc/free, memory managers to deal with rapid fixed
/// size allocations/delocations and a generic object factory. Interfaces with 
/// the logging system to allow leak and integrity detection. Also overrides
/// the global new/delete operators, to cicumvent the many memory spaces problem
/// associated with shared librarys by simply forcing all allocation to be in a
/// single memory space.

#include "eos/types.h"

#include <malloc.h>

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
// We firsly need a set of 'normal' style malloc/free functions, these have
// to be exported from the class because if inline they could use the 
// malloc/free of another executable unit...

EOS_FUNC void * EOS_STDCALL BasicMalloc(nat32 size);
EOS_FUNC void EOS_STDCALL BasicFree(void * ptr);

//------------------------------------------------------------------------------
// Then we need a set of inline templated, and inturn typed, equivalents, with
// more typical names...

/// eos provides a typed malloc, where you specify the type to be returned and 
/// the number of items you want. So instead of typing
/// <code>= (type*)malloc(sizeof(type)*n)</code> you enter 
/// <code>= malloc<type>(n)</code>, or <code>malloc<type>()</code> when 
/// <code>n==1</code>. This stricter typing is generally useful for catching 
/// certain bugs, and prevents a certain kind of bastardisation.
template <typename T>
inline T * Malloc(int32 n = 1)
{
 return static_cast<T*>(BasicMalloc(sizeof(T)*n));
}

/// To compliment the typed malloc a typed free is also provided, though it can
/// be used in the traditional sense as the compiler will just work out which
/// variant to use, so it adds little. Provided for completeness really.
template <typename T>
inline void Free(T * ptr)
{
 BasicFree(ptr);
}

//------------------------------------------------------------------------------
 };
};

//------------------------------------------------------------------------------
// The global new/delete operators, including [] versions, to make *everything*
// use the above malloc/free...

/// Overloaded version of new to use own malloc/free, to bypass memory space 
/// problems in regards to multiple dlls.
inline void * operator new(size_t size)
{
 return eos::mem::BasicMalloc(size);   
}

/// A placement new, for creating new objects in allready allocated memory.
/// Remember to call delete without destruction, i.e. my_obj->~T();
template <typename T>
inline void * operator new(size_t size,T * mem)
{
 return mem;
}

/// Overloaded version of new[] to use own malloc/free, to bypass memory space 
/// problems in regards to multiple dlls.
inline void * operator new[](size_t size)
{
 return eos::mem::BasicMalloc(size);   
}

/// Overloaded version of delete to use own malloc/free, to bypass memory space 
/// problems in regards to multiple dlls.
inline void operator delete(void * ptr)
{
 eos::mem::BasicFree(ptr);   
}

/// Overloaded version of delete[] to use own malloc/free, to bypass memory space 
/// problems in regards to multiple dlls.
inline void operator delete[](void * ptr)
{
 eos::mem::BasicFree(ptr);   
}

//------------------------------------------------------------------------------
#endif
