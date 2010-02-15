#ifndef EOS_FILE_DIRS_H
#define EOS_FILE_DIRS_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file dirs.h
/// Provides the ability to navigate the directory structure.

#include "eos/types.h"

#include "eos/str/strings.h"
#include "eos/ds/lists.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// SpecialDir enum, indicates several 'special' positions.
enum SpecialDir {WorkDir, ///< The running directory of the executable.
                 RootDir, ///< Root of the filesystem.
                 ConfDir, ///< Where to save global config files.
                 HomeDir, ///< Where to save configuration files for the logged in user.
                 DocsDir, ///< Where to save normal files for the logged in user.
                };

//------------------------------------------------------------------------------
/// DirType enum, indicates type of filesystem node.
enum DirType {TypeDir = 1,     ///< A directory.
              TypeFile = 2,    ///< A file.
              TypeLink = 4,    ///< A link to any other type of entity.
              TypeMeta = 8,    ///< Meta data, such entrys contain extra details about the file system, mostly immutable.
              TypeUnknown = 16 ///< An unknown entity from the filesystem.
             };

//------------------------------------------------------------------------------
/// The file system is represented as a single type, this representing 
/// any point in the system, including files, directorys, links and anything else
/// the filesystem may offer. (Yes, its ushally a directory so its called 'Dir',
/// but it does other stuff as well.) A very simple approach is applied, where you can
/// get a list of all children of any object and the objects parent, as nodes,
/// and extract information about the current node, of any type you like.
/// Note that under windows a virtual root filesystem is created, which contains
/// all the drives /c, /d etc.
class EOS_CLASS Dir
{
 public:
  /// On construction you give one of several posible starting positions
   Dir(SpecialDir pos);

  /// You can construct using a filename in any ushall format, it will be converted
  /// to a format this class likes.
   Dir(cstrconst pos);

  /// You can construct using a filename in any ushall format, it will be converted
  /// to a format this class likes. 
   Dir(const str::String & pos);

  /// &nbsp;
   Dir(const Dir & rhs); 

  /// &nbsp;
   ~Dir();

   
  /// &nbsp;
   Dir & operator = (const Dir & rhs);


  /// Returns true if the location is valid, this can return false if you 
  /// navigate to somewhere that dosn't exist or someone deletes the thing you
  /// are currently at.
   bit Valid() const;
   
  /// Returns the type of the current location, this dictates what you can do
  /// with it. Note that for all types it returns just the type, except for 
  /// TypeLink where it returns TypeLink | (type of thing it links to.).
   DirType Type() const;
   
  
  /// If the current location is invalid this trys to create it, as a directory.
  /// Returns true on success.
   bit Create();
   
  /// Returns true if you have the right to create things in the current directory.
   bit Editable();
   
  
  /// Returns true if the entity exists in the current location.
   bit Exists(cstrconst obj);
   
  /// Returns true if the entity exists in the current location.
   bit Exists(const str::String & obj);

  /// Returns an OS independent path for the current location.
   const str::String & Path() const;
   
  /// Returns an OS specific path for the current location.
   const str::String & RealPath() const;
  
  
  /// Navigates up from the current position, to the directory above.
  /// Does nothing if this is not an allowed operation.
   void Up();

  /// This moves into a child directory/object. Note that it is irrelevent
  /// if it exists or not - it will ignore this fact until it becomes a 
  /// problem.  
   void Go(cstrconst obj);

  /// This moves into a child directory/object. Note that it is irrelevent
  /// if it exists or not - it will ignore this fact until it becomes a 
  /// problem.  
   void Go(const str::String & obj);


  /// This allows you to obtain a list of all children of this location,
  /// with a set of restrictions on which children types to allow in the
  /// list.
  /// \param out A linked list to which all children found are added as strings that will work with the Go methods.
  /// \param pattern Pattern of filenames to accept, things such as "*.bmp" etc.
  /// \param type Types to output, or'ed together.
   void Children(ds::List<cstr,mem::KillDel<cstr> > & out,
                 cstrconst pattern = null<cstrconst>(),DirType type = DirType(TypeDir | TypeFile | TypeLink));


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::file::Dir";}


 protected:
  str::String path; // Allways a full unix style path, i.e. starts with /.
  str::String osPath; // Same as path under Linux, but has the windows crap under windows.
  
  void Optimise(); // Makes the path as short as possible, removing things like x/.. etc.
  void MakeOS(); // Makes osPath match the meaning of path.
  void MakePath(); // Makes path match osPath.
};

//------------------------------------------------------------------------------
 };
};
#endif
