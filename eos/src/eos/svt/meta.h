#ifndef EOS_SVT_META_H
#define EOS_SVT_META_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


/// \file meta.h
/// Specifies a simple data container. Provides some quite complicated 
/// facilities however, as it has to provide read-only meta data and 
/// ints/reals/strings.

#include "eos/types.h"
#include "eos/str/strings.h"
#include "eos/ds/sort_lists.h"
#include "eos/svt/node.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
/// The Meta class provides a hash table of key -> data, where data can be int,
/// real, token or string. If you request the wrong type for any given key it converts
/// as best it can. It allows desendent classes to integrate to export variables
/// and capture attempt to change them for its own nefarious purposes.
/// The ability to extract a list of all keys, stored and exported alike, is 
/// also provided, and the ability to query write access of variables.
/// See the \link svt_io SVT IO page \endlink for details on the results of
/// IO with this class.
class EOS_CLASS Meta : public Node
{
 public:
  /// &nbsp;
   Meta(Core & c):Node(c) {}
   
  /// This extends a Node into a Meta, deleting the passed in Node such
  /// that the Meta object created here replaces it.
  /// No other objects are hurt in the construction of this one.
   Meta(Node * node):Node(node) {delete node;}
   
  // This shallow copies the passed in Meta and then neuters it, such 
  // that the shallow copy still works when the meta is deleted.
  // Allows a new Meta object to replace an existing Meta object 
  // without any construction/detruction outside of the two objects.
  // Hidden, as for use be child objects only really.
   Meta(Meta * meta);

  /// &nbsp;
  ~Meta() {}

  
  /// An enum used to indicate the types of data that are actually stored in a Key.
   enum Type {Aint,   ///< &nbsp;
              Areal,  ///< &nbsp;
              Atoken, ///< &nbsp;
              Astring ///< &nbsp;
             };

  /// Internal class that represents the result of requesting an 
  /// individual key. Allows sophisticated querying of type and value,
  /// and setting the data when applicable, including changing data 
  /// type.
   class EOS_CLASS Item
   {
    public:
     ~Item() {if (is==Astring) delete asStr;}

    private:
      Item(Meta & owner,str::Token tok)
      :meta(owner),exp(owner.IsExport(tok)),key(tok),is(Meta::Aint),asInt(0)
      {}

     friend class Meta;
     Meta & meta;
     bit exp; //  If true its an export - go via meta, not the below.    
    
     str::Token key;
     Meta::Type is;
    
     union
     {
      int32 asInt;
      real32 asReal;
      str::Token asTok;
      str::String * asStr;     
     };

    public:
     bit operator < (const Item & rhs) const {return key < rhs.key;} // Simply to sort it in the structure.
  
     /// Returns the token that will return this Item.
      str::Token Key() const {return key;}

     /// Returns the type of the data stored within.
      Meta::Type Is() const {if (exp) return meta.OnGetType(key); else return is;}

     /// Returns the data as an integer.
      int32 AsInt() const;

     /// Returns the data as a real.
     real32 AsReal() const;

     /// Returns the data as a token.
      str::Token AsToken() const;

     /// Outputs the data as a string.
      void AsString(str::String & out) const;


     /// Sets the data to be the given integer, will return true if the change
     /// has been made, false if it has not. Can only return false for exported keys.
      bit Set(int32 rhs);

     /// Sets the data to be the given real, will return true if the change
     /// has been made, false if it has not. Can only return false for exported keys.
      bit Set(real32 rhs);

     /// Sets the data to be the given token, will return true if the change
     /// has been made, false if it has not. Can only return false for exported keys.
      bit Set(str::Token rhs);

     /// Sets the data to be the given string, will return true if the change
     /// has been made, false if it has not. Can only return false for exported keys.
      bit Set(const str::String & rhs);

     /// Sets the data to be the given string, will return true if the change
     /// has been made, false if it has not. Can only return false for exported keys.
      bit Set(cstrconst rhs);

     /// Returns true if its an exported key, that maps to some detail of the object.
     /// Exported keys can not allways be editted.
      bit IsExport() const {return exp;}

     /// Returns true if the key can be changed, will allways be true for non-exported
     /// keys, note that it can return true for exported keys and still not accept changes
     /// because they are out of range/invalid etc for whatever validation is happenning in
     /// the object.
      bit CanSet() const {return !exp || meta.CanSet(key);}

     /// Returns a description of this key, ushally it will be an empty string, but exported
     /// Items can have this set, to explain validation detals and the like.
      cstrconst Description() const {return meta.GetDescription(key);}
   };
   friend class Item;


  /// Returns a pointer to the Item associated with a key if it allready exists,
  /// or null if its dosn't exist. To create an Item that does not exist use
  /// operator [].
   Item * Get(str::Token tok) const;

  /// &nbsp;
   Item * Get(cstrconst str) const {return Get(GetCore().GetTT()(str));}

  /// Returns a reference to the item associated with the given key, if the Key does
  /// not yet exist it creates a new one, which will be set ot an integer of value 0.
   Item & operator[] (str::Token tok);

  /// &nbsp;
   Item & operator[] (cstrconst str) {return (*this)[GetCore().GetTT()(str)];}

  /// Returns how many keys are stored within the data structure.
   nat32 Size() const;
   
  /// Returns how many keys are exported, these are the first entrys when using
  /// Entry and can so be eailly skipped.
   nat32 SizeExports() const {return ExportSize();}

  /// Allows you to access the data structure as an array and obtain a list of all 
  /// the Items. Note that you shouldn't be adding data whilst iterating this 
  /// array, as the ordering is not in the form of appending.
   Item & Entry(nat32 i);

 
  /// &nbsp;
   nat32 Memory() const;
   
  /// &nbsp;
   nat32 WriteSize() const;
   
  /// &nbsp;
   nat32 TotalWriteSize() const;


  /// &nbsp;
   cstrconst MagicString() const;
  
  /// &nbsp;
   nat32 Write(io::OutVirt<io::Binary> & out) const;
   
  /// &nbsp;
   void ReadBlock(io::InVirt<io::Binary> & in);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::svt::Meta";}


 protected:
  /// Can be overloaded to return how many keys have been exported.
   virtual nat32 ExportSize() const {return 0;}

  /// This can be overloaded to provide access to a list of exported keys.
   virtual str::Token Export(nat32 i) const {return str::NullToken;}

  /// Should return true if given a key that is exported, false otherwise.
   virtual bit IsExport(str::Token tok) const {return false;}

  /// Will only be called with keys that return true from the IsExport method.
  /// Should return true if the OnSet.. methods can work.
   virtual bit CanSet(str::Token tok) const {return true;}

  /// This is called for all tokens to request a description of the token, it 
  /// should return an empty string by default. Useful way to explain the 
  /// validation details for a given Item.
   virtual cstrconst GetDescription(str::Token tok) const {return "";}


  /// Will be called with an exported key, should return the type of the 
  /// exported variable...
   virtual Type OnGetType(str::Token tok) const {return Aint;}


  /// Will be called to get an exported variable as an integer, will only be
  /// given exported keys, should return true on success, false otherwise.
   virtual bit OnGetInt(str::Token tok,int32 & out) const {return false;}

  /// Will be called to get an exported variable as a real, will only be
  /// given exported keys, should return true on success, false otherwise.
   virtual bit OnGetReal(str::Token tok,real32 & out) const {return false;}

  /// Will be called to get an exported variable as a token, will only be
  /// given exported keys, should return true on success, false otherwise.
   virtual bit OnGetToken(str::Token tok,str::Token & out) const {return false;}

  /// Will be called to get an exported variable as a string, will only be
  /// given exported keys, should return true on success, false otherwise.
   virtual bit OnGetString(str::Token tok,str::String & out) const {return false;}


  /// Will be called to set an exported variable to an integer, will only be
  /// given exported variables, return true if the input was accepted, false if
  /// no change has occured.
   virtual bit OnSetInt(str::Token tok,int32 in) {return false;}

  /// Will be called to set an exported variable to a real, will only be
  /// given exported variables, return true if the input was accepted, false if
  /// no change has occured.
   virtual bit OnSetReal(str::Token tok,real32 in) {return false;}

  /// Will be called to set an exported variable to a token, will only be
  /// given exported variables, return true if the input was accepted, false if
  /// no change has occured.
   virtual bit OnSetToken(str::Token tok,str::Token in) {return false;}

  /// Will be called to set an exported variable to a string, will only be
  /// given exported variables, return true if the input was accepted, false if
  /// no change has occured.
   virtual bit OnSetString(str::Token tok,const str::String & in) {return false;}


 private:
  ds::SortList<Item*,ds::SortPtrOp<Item*>,mem::KillDel<Item> > list;
  mutable ds::SortList<Item*,ds::SortPtrOp<Item*>,mem::KillDel<Item> > expList; // Export list, stores the exported nodes that have been constructed.

 // Helpers for the memory method, as its quite hard to calculate...
  mutable nat32 memory;
  void SumMem(Item *& item) {if (item->is==Astring) memory += item->asStr->Memory();}
  
 // Another helper, similar to above, for the WriteSize method.
 // (Uses the above memory variable.)
  void SumWriteItems(Item *& item)
  {
   memory += 6;
   memory += str::Length(core.GetTT().Str(item->key));
   switch (item->is)
   {
    case Aint:
     memory += 3 + 4;
    break;
    case Areal:
     memory += 4 + 4;
    break;
    case Atoken:
     memory += 5 + str::Length(core.GetTT().Str(item->asTok));
    break;
    case Astring:
     memory += 6 + item->asStr->Size();
    break;
   }
  }
};
  
//------------------------------------------------------------------------------
 };
};
#endif
