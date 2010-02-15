#ifndef EOS_FILE_FILES_H
#define EOS_FILE_FILES_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


/// \file file/files.h
/// Provides basic files for reading/writting to.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/file/dirs.h"
#include "eos/io/seekable.h"
#include "eos/str/strings.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
// Some useful functions...

/// This generates a tempory filename that is certified unique, in an 
/// appropriate temp directory. Note that its unlikelly to be particularly 
/// secure.
/// Remember to mem::Free the returned result.
EOS_FUNC cstr TemporyFilename();

/// Deletes a given filename. Note that it isn't actually deleted until any
/// proccesses accessing it have finished.
EOS_FUNC void DeleteFile(cstrconst fn);

//------------------------------------------------------------------------------
/// An enum of ways of openning a file. (Others could be defined, but I've never
/// used any outside this list.)
enum Way {way_new = 0, ///< If the file dosn't exist create a new file, if the file does exist fail.
          way_ow = 1, ///< If the file dosn't exist create a new file, if the file does exist empty the file and start from scratch.
          way_edit = 2, ///< If the file dosn't exist fail, if the file does exist keep its current contents and start editting.
          way_append = 3 ///< If the file dosn't exist create it, if the file does exist open with original data. All write operations are automatically at the end of the file.
         };

//------------------------------------------------------------------------------
/// An enum of access modes for openning a file.
enum Mode {mode_read = 0, ///< Open the file read only.
           mode_write = 1, ///< Open the file write only.
           mode_wr = 2 ///< Open the file for both reading and writting.
          };

//------------------------------------------------------------------------------
/// An enum of safty modes for indicating how 'safe the file i/o is.
enum Safe {safe_none = 0, ///< No safety is done, just because a file operation completed dosn't mean it has actually reached the hardisk. Most file i/o is perfectly reasonably done like this.
           safe_full = 1 ///< When a file operation method returns there is a guarantee that the data is on disk. Makes things very slow, ushally for when caching etc is being done by the program in a multi-threaded setup.
          };

//------------------------------------------------------------------------------
// Code for the file flat template.
class EOS_CLASS FileCode
{
 public:
   FileCode();
   FileCode(cstrconst fn,Way way,Mode mode,Safe safe = safe_none);
   FileCode(const str::String & fn,Way way,Mode mode,Safe safe = safe_none);
   FileCode(const Dir & dir,cstrconst fn,Way way,Mode mode,Safe safe = safe_none);
   FileCode(const Dir & dir,const str::String & fn,Way way,Mode mode,Safe safe = safe_none);   
  ~FileCode();
  
  bit Active() const;
  bit CanRead() const;
  bit CanWrite() const;
  
  Way GetWay() const;
  Mode GetMode() const;
  Safe GetSafe() const;
  
  void Flush() const;
  
  bit Open(cstrconst fn,Way way,Mode mode,Safe safe = safe_none);
  bit Open(const str::String & fn,Way way,Mode mode,Safe safe = safe_none);
  void Close();
  
  nat32 Size() const;
  nat32 SetSize(nat32 size);
  
  nat32 Read(nat32 pos,void * data,nat32 amount) const;
  nat32 Write(nat32 pos,const void * data,nat32 amount);
  nat32 Pad(nat32 pos,byte item,nat32 amount);


 private:
  Way way;
  Mode mode;
  Safe safe;
  int handle;
};

//------------------------------------------------------------------------------
// A predeclaraction needed for below...
template <typename ET>
class EOS_CLASS File;

//------------------------------------------------------------------------------
/// The File I/O cursor class, specific instances are extracted from the File 
/// class. Where actual i/o happens.
template <typename ET>
class EOS_CLASS Cursor : public io::InOut<ET>
{
 protected:
  friend class File<ET>;
  Cursor(FileCode & f,nat32 p):fc(&f),pos(p) {}
  
 public:

  /// &nbsp;
   Cursor():fc(null<FileCode*>),pos(0) {}
   
  /// &nbsp;
   Cursor(const Cursor<ET> & rhs):fc(rhs.fc),pos(rhs.pos) {}
   
  /// &nbsp;
   ~Cursor() {}


  // Extra functionality...
   /// &nbsp;
    bit Active() {return fc!=null<FileCode*>();}
   
   /// Safe to call when not active, will return false.
    bit CanRead() {if (fc) return fc->CanRead(); else return false;}
    
   /// Safe to call when not active, will return false.
    bit CanWrite() {if (fc) return fc->CanWrite(); else return false;}
        
   /// &nbsp;
    nat32 Size() const {return fc->Size();}


  // From io::In...
   /// &nbsp;
    bit EOS() const {return pos>=fc->Size();}
    
   /// &nbsp;
    nat32 Avaliable() const {if (pos<fc->Size()) return fc->Size()-pos; else return 0;}
    
   /// &nbsp;
    nat32 Read(void * out,nat32 bytes) {nat32 p = fc->Read(pos,out,bytes); pos += p; return p;}
    
   /// &nbsp;
    nat32 Peek(void * out,nat32 bytes) const {return fc->Read(pos,out,bytes);}
    
   /// &nbsp;
    nat32 Skip(nat32 bytes) {pos += bytes; return bytes;}


  // From io::Out...
   /// &nbsp;
    nat32 Write(const void * in,nat32 bytes) {nat32 p = fc->Write(pos,in,bytes); pos += p; return p;}
   /// &nbsp;
    nat32 Pad(nat32 bytes) {nat32 p = fc->Pad(pos,GetPadByte(ET()),bytes); pos += p; return p;}


  /// &nbsp;  
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::file::Cursor<" << typestring<ET>() << ">");
    return ret;
   }


 private:
  FileCode * fc;
  nat32 pos;
};

//------------------------------------------------------------------------------
/// File I/O class provided as a flat template which is templated on the the
/// type of streaming it suports - either io::Binary or io::Text. A very simple
/// class, with all its functionality derived from the eos::io system.
template <typename ET>
class EOS_CLASS File : public io::Seekable<ET>
{
 public:
  /// &nbsp;
   File():fc() {}
   
  /// &nbsp;
   File(cstrconst fn,Way way,Mode mode,Safe safe = safe_none):fc(fn,way,mode,safe) {}
   
  /// &nbsp;
   File(const str::String & fn,Way way,Mode mode,Safe safe = safe_none):fc(fn,way,mode,safe) {}   

  /// &nbsp;
   File(const Dir & dir,cstrconst fn,Way way,Mode mode,Safe safe = safe_none):fc(dir,fn,way,mode,safe) {}
   
  /// &nbsp;
   File(const Dir & dir,const str::String & fn,Way way,Mode mode,Safe safe = safe_none):fc(dir,fn,way,mode,safe) {}   
   
  /// &nbsp;
   ~File() {}
  
  
  /// Returns true on success, if its currently Active() the current file is closed.
   bit Open(cstrconst fn,Way way,Mode mode,Safe safe = safe_none) {return fc.Open(fn,way,mode,safe);}
   
  /// Returns true on success, if its currently Active() the current file is closed.
   bit Open(const str::String & fn,Way way,Mode mode,Safe safe = safe_none) {return fc.Open(fn,way,mode,safe);}   
   
  /// &nbsp;
   void Close() {fc.Close();}


  /// &nbsp;
   bit Active() const {return fc.Active();}
   
  /// &nbsp;
   bit CanRead() const {return fc.CanRead();}
   
  /// &nbsp;
   bit CanWrite() const {return fc.CanWrite();}
   
   /// &nbsp;
    Way GetWay() const {return fc.GetWay();}
    
   /// &nbsp;
    Mode GetMode() const {return fc.GetMode();}
    
   /// &nbsp;
    Safe GetSafe() const {return fc.GetSafe();}
    
    
   /// After this call has returned it is ensured that all data written to the file is on disk and safe.
   /// Not necesary is safe_full is used.
    void Flush() const {fc.Flush();}
 
 
  // From seekable...
   /// &nbsp;
    nat32 Size() const {return fc.Size();}

   /// Returns true.
    bit CanResize() const {return true;}

   /// &nbsp;
    nat32 SetSize(nat32 size) {return fc.SetSize(size);}

   /// Note that because it is not virtual you must used the returned object 
   /// type produced by this method.
    Cursor<ET> GetCursor(nat32 pos = 0) {return Cursor<ET>(fc,pos);} 


  /// &nbsp;  
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::file::File<" << typestring<ET>() << ">");
    return ret;
   }


 private:
  FileCode fc;
};

//------------------------------------------------------------------------------
 };
};
#endif
