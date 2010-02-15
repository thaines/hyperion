#ifndef EOS_SVT_FILE_H
#define EOS_SVT_FILE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file svt/file.h
/// Defines the IO system for the SVT system. This is rather complex, due to its
/// generic nature.


/// \page svt_io Single Variable Type File Format
/// The SVT i/o system is a layered structure, each layer is documented in turn below.
///
/// \section Object Block File Structure.
/// An Object Block file, extension .obf, consists of a single object, where an 
/// object consists of one or more blocks.
/// The multiple blocks per object structure is designed to suport inheritance,
/// multiple inheritance if necesary.
/// (A shared grandparent in the inheritance tree will have to be duplicated however.)
///
/// The structure of a block is as follows:
/// - 3 byte block magic number, bmn.
/// - 3 byte object magic number, omn.
/// - 5 byte intel order unsigned integer block size in bytes, bs.
/// - 5 byte intel order unsigned integer object size in bytes, os.
/// - (bs - 16) bytes of block data. This can include further objects.
///
/// The block size is the offset from the start of this header to the next block 
/// header, or the end of the file. The object size is the offset from the start of 
/// this header to the start of the first header of the next object, or the end of file.
/// No padding is ever done to the sizes given, though padding can happen internally in blocks.
///
/// A complete object consists of a sequence of blocks, one after the other. All blocks
/// in the sequence will share the same object magic number.
/// Block types are simply object types, existing in the same namespace.
/// The final block in the sequence will have its block magic number and object magic number equal.
/// Any block in the file that is a child type must have its parent type(s) inheritance trees
/// immediately before it in the file, in a loader dependent order.
/// The structure is designed to be optimal if loading the final object type, but not to inhibit
/// the loading of one of its parent object types instead of the actual object.
/// If doing this due to an unrecognised object type then the standard is to load the last 
/// recognised block type in the block sequence. Whilst obviously the correct behaviour for
/// single inheritance this fact has to be kept in mind when deciding the write order of parents
/// when multiple inheritance is used.
///
/// \section Node
/// The Node object has a magic number of "HON", as in hierachy of nodes, and has no 
/// parent type.
/// A node object's internal structure is simply a 4 byte intel order child count 
/// followed by that many objects.
///
/// \section Meta
/// This is a child type of the Node object, and so will allways follow a HON.
/// Its magic number is "SID" for single instance data.
/// Its internal structure is a 4 byte item count followed by that many items.
///
/// Each item consists of 3 data lumps, each data lump being a 2 byte lump size
/// followed by that many bytes of data.
/// The first lump is a UTF-8 string name for the item, the next lump is a UTF-8
/// type for the lump, so it can be interpreted whilst the final lump is the actual 
/// lump data.
///
/// The standardised item types suported at this time are as follows:
/// - int - The data lump will contain an intel order signed number, it will ushally be 4 bytes.
/// - real - The data lump will contain a IEEE floating point number, the data lump must be either 4 or 8 bytes in size.
/// - string - The data lump will contain a UTF-8 string.
/// - token - Identical to string. This is used to imply that the string contained is frequently used, and could be stored as a pointer to a central table to avoid duplication.
///
/// If unknown types are found there handling can be application specific, 
/// but ushall behavour is to either ignore them or to present them to the user as hex.
///
/// \section Var
/// This is a child type of the Meta object, and so will allways follow a SID.
/// Its magic number is "MID" for multiple instance data.
/// The Vars data block consists of 3 parts.
///
/// \subsection var_array Array Structure
/// This specifies a multi-dimensional array.
/// Consists simply of a 4 byte dimension count followed by 4 bytes for each dimension,
/// indicating the size of the dimensions.
/// For instance, a 2D image that is 640 by 480 would be the sequence of 32 bit 
/// integers 2,640,480.
///
/// \subsection var_field Field List
/// This is the same as the item list in the Meta object bit instead of actual data it has
/// the default data.
/// Headed with a 4 byte field count followed by that many Items.
/// In this object an item uses the same lump structure, with the name lump, the type lump and
/// lastly the default lump. The type lumps here come from a different namespace,
/// and use fully qualified eos names, such as "eos::int32", "eos::real32" or "eos::bs::ColourRGB",
/// for example. The default lump is identical to the Meta item data, except its a default value 
/// that is used to initialise instances of the type in question rather than the actual data.
///
/// \subsection var_data Field Data
/// To explain this data block first define the field block. A field block is the size of all
/// the fields added together. It contains each field defined in the field list, in the order
/// given. The field data is then a tightly packed linear array, the size being the product of
/// all the dimension sizes given. If looping over the array for each dimension the inner loop
/// is the first dimension given in the dimension list.
///
/// \section File Structure
/// A file containing a SVT hierachy is given the extension .svt.
/// It has as root an additional object type, TAV, which provides the file format
/// type and version information.
/// The version object has no parent type, and consists of the following:
/// - 4 bytes, JSVT, to confirm that this is indeed a file containning Just a SVT hierachy. 
///   This could be something else if I choose to use the same structure to store some other data blocks.
/// - 4 byte revision number.
/// - 4 byte extension number.
/// - 2 bytes size plus data lump, containing a UTF-8 string indicating the program that wrote the file.
///   Can optionally be left empty, but is considered good form so measures can be taken to resolve 
///   incompatabilities if they are unfortunate enough to arise.
///   Due to this purpose it should idealy be program build unique.
/// - The true root of the file, an SVT object, either a HON, SID or MID.
///
/// The revision number changes if the actual file format changes fundamentally, 
/// such that no previous program can reliably open it. Should be 1.
/// The extension number is incrimented for each change that adds something 
/// without breaking previous loaders of the current revision. This is ushally 
/// taken to indicate that the file could contain unrecognised blocks, or blocks with 
/// extra details that don't interfere with previous loaders (Appended data.). Should be 0.
/// Note, despite all the ramblings about multiple inheritance it has not been used,
/// consider revision 1 of the file format to not suport it. This makes implimenting 
/// a version one loader considerably easier, as fallback for unrecognised types
/// is less painful this way. Additionally, support for only 4 of the 5 bytes in header 
/// sizes can be considered acceptable for a version 1 loader.


#include "eos/types.h"
#include "eos/svt/var.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
// The TaV (type and version) object, the root of a SVT file. Exists simply as
// a conveniance object used internally by the SVT io system.
class EOS_CLASS TaV : public Deletable
{
 public:
  /// Initialises it to contain the correct version numbers and version strings etc.
   TaV();
  
  // Does nothing to the root.
  ~TaV();


  /// Returns true if the magic number, revision and extension are all suported.
   bit Check() const;

  /// Returns true on success.
   bit Write(io::OutVirt<io::Binary> & out);
   
  /// Returns true on total success, false on failure of some level.
  /// It can still be ok on returning false however if root!=0, 
  /// that just means that it didn't recognise the entire file and 
  /// some objects have been downgraded to simpler versions.
   bit Read(Core & core,io::InVirt<io::Binary> & in);
   
   
  /// Should be "JSVT".
   cstrchar magic[4];

  /// The revision number, should be 1.
   nat32 revision;

  /// The extension number, should be 0.
   nat32 extension;
   
  /// The program name string, unique to the program hopefully.
  /// Defaults to the EOS version details and compilation date.
  /// This is mem::Free'ed when the class dies.
   cstr prog;
   
  /// The root of the actual SVT hierachy.
   Node * root;
};

//------------------------------------------------------------------------------
// I/O functions for the basic SVT objects, so they can be registered with the
// core object by the core when the core is created...
// (Includes the Create functions as well.)

EOS_FUNC Node * CreateNode(Core & core,Node * parent);
EOS_FUNC Node * LoadNode(Core & core,io::InVirt<io::Binary> & in,Node * self);

EOS_FUNC Node * CreateMeta(Core & core,Node * parent);
EOS_FUNC Node * LoadMeta(Core & core,io::InVirt<io::Binary> & in,Node * self);

EOS_FUNC Node * CreateVar(Core & core,Node * parent);
EOS_FUNC Node * LoadVar(Core & core,io::InVirt<io::Binary> & in,Node * self);

//------------------------------------------------------------------------------
/// Loads an SVT file. (Traditionally a .svt extension.)
/// Returns the root of the newly loaded hierachy on success or null on total 
/// failure. As SVT files suport loading of newer file versions by old loaders
/// you can get a warning via the optional warning pointer if parts of the file
/// were unsuported and have hence been ignored.
EOS_FUNC Node * Load(Core & core,cstrconst fn, bit * warning = null<bit*>());

/// Saves a SVT file, returns true on success and false on failure.
EOS_FUNC bit Save(cstrconst fn,Node * root,bit overwrite = false);

//------------------------------------------------------------------------------
 };
};
#endif
