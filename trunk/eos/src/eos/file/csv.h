#ifndef EOS_FILE_CSV_H
#define EOS_FILE_CSV_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file csv.h
/// Provides for the output and input of comma seperated files, i.e. spreadsheets.
/// Specifically used for output by the logging system, as well as anything else
/// that deals in this kind of thing.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/file/files.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
struct CsvEndField {};
struct CsvEndRow {};

/// The field divider function, used to indicate that one field has ended and 
/// another one begun.
inline CsvEndField EndField() {CsvEndField ret; return ret;}

/// The row divided function, used to indicate that one row has ended and 
/// another one begun. Do not call the field divide followed by this - this will
/// close the last field.
inline CsvEndRow EndRow() {CsvEndRow ret; return ret;}

//------------------------------------------------------------------------------
/// The csv file, for output only. Standard streamable interface, except it
/// provides special objects that can be passed in to end fields and rows, 
/// and escapes bad characters as needed.
class EOS_CLASS Csv : public io::Out<io::Text>
{
 public:
  /// &nbsp;
   Csv(cstrconst fn,bit overwrite = false);
   
  /// &nbsp;
   Csv(const str::String & fn,bit overwrite = false);
   
  /// &nbsp;
   Csv(const Dir & dir,cstrconst fn,bit overwrite = false);
   
  /// &nbsp;
   Csv(const Dir & dir,const str::String & fn,bit overwrite = false);
   
  /// &nbsp;
   ~Csv();
   
   
  /// &nbsp;
   bit Active();
    
   
  /// &nbsp;
   nat32 Write(const void * in,nat32 bytes);
  
  /// &nbsp;
   nat32 Pad(nat32 bytes);
   
   
  /// Ends the current field and starts the next.
   void FieldEnd();
  
  /// Ends the current row.
  /// Do not call FieldEnd() then RowEnd() without content - it will create an empty field
  /// on the end of the line. Unless you want that effect of course.
   void RowEnd();
   
   
  /// Required by the logging system, for obvious reasons.
   void Flush();



  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::file::Csv";}


 private:
  bit startOfRow; // true when we should add a " before doing anything else.
  File<io::Text> file;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {
 
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::file::CsvEndField & rhs,Text)
  {
   lhs.FieldEnd();
   return lhs;
  }

  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::file::CsvEndRow & rhs,Text)
  {
   lhs.RowEnd();
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
