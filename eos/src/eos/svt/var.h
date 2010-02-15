#ifndef EOS_SVT_VAR_H
#define EOS_SVT_VAR_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file var.h
/// Contains the meat of the system, the multi-dimensional arbitary-field storing
/// son of a data type.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/ds/sparse_hash.h"
#include "eos/svt/meta.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
// A pre-decleration of the field class...
template <typename T>
class Field;

//------------------------------------------------------------------------------
/// The part of the svt that stores large quantities of data. You have two 
/// independent sets of meta data which describe the data itself. Firstly you 
/// have the number of dimensions and the size of each dimension, for instance a
/// dimension count of 2 with sizes of 1024 and 768 would represent one common 
/// resolution of image. Seperatly, you have the field list, which describes a 
/// list of fields stored in this data structure. Each field has a name, a type,
/// a size, and default starting data, the size and starting data prefably being
/// deteremined by type. Any number of fields can be stored of any number of types.
/// For instance, to continue the image example you could have one field that contains
/// a colour object, for instance eos::bs::ColourRGB would do, or 3 fields of type
/// eos::real32 named r, g and b. <br><br>
/// Due to the time consuming nature of changing the meta data of the class a 
/// change-commit model is used, you call the methods to make the changes and then
/// call Commit to make the changes. A change of the dims/size data will allways
/// delete all previous data, adding/removing fields does not effect the current set
/// of fields, assuming a dim/size change does not happen at the same time. <br><br>
/// All access to non-meta data is provided by the Field class, you request a Field 
/// by name and the type you want to access it as and it returns a suitable Field
/// class to access all members of that field in the data structure. <br><br>
/// The meta-data for the class is published in the inherited Meta component, this
/// includes the following list:
/// - dims - the number of dimensions of the class, int.
/// - size[x] - the size of each dimensions, using array notation, int.
/// - stride[x] - stride[0] provides the sum of all the field sizes, stride[1] is stride[0]*size[0], stride[2] is stride[1]*size[1] and so on, upto the total memory consumption of the data part of the structure as a whole.
/// - fields - how many fields exist.
/// - field[x].name - name of each field, the array is in storage order, token.
/// - field[x].type - the type of each field, token.
/// - field[x].size - the size of each field, int.
/// - field[x].default - the default data in each field, as a string, in hex.
///
/// See the \link svt_io SVT IO page \endlink for details on the results of
/// IO with this class.
class EOS_CLASS Var : public Meta
{
 public:
  /// &nbsp;
   Var(Core & c);

  /// This var construction option builds the var from a field, it does not 
  /// actualy commit or add any fields to the Var, all it does is take the 
  /// Core from the field and set itself so it will have the same dimensions
  /// as the field after calling Commit(). You can override the dimensions
  /// by calling one of the setup methods, so this can be used to simply
  /// 'extract' a Core object for consistancy.
   template <typename T>
   Var(const Field<T> & f);
   
  /// This takes a Meta object and creates a new var object to replace it.
  /// This new object replaces the Meta object, updating all pointers etc.
  /// It calls delete on the Meta object.
   Var(Meta * meta);
   
  // Shallow copies the given object, and updates the given object such that 
  // when it is deleted it won't hurt any of the shallow copied data.
  // Used by children objects to do the type extension constructors.
  // The passed in object is royally fucked on return, and should only be deleted.
   Var(Var * var);

  /// &nbsp;
   ~Var();

  /// &nbsp;
   Var & operator= (const Var & rhs);


  /// This deletes any data currently stored in the class and sets it to have the
  /// specified number of dimensions and sizes, independently of fields which are
  /// maintained over this operation.
  /// \param dims Number of dimensions.
  /// \param size Pointer to an array of sizes, where size[0] is the size of the first dimension etc.
   void Setup(nat32 dims,const nat32 * size);

  /// Shortcut version of Setup, for 1D.
   void Setup1D(nat32 a);

  /// Shortcut version of Setup, for 2D.
   void Setup2D(nat32 a,nat32 b);

  /// Shortcut version of Setup, for 3D.
   void Setup3D(nat32 a,nat32 b,nat32 c);

  /// Shortcut version of Setup, for 4D.
   void Setup4D(nat32 a,nat32 b,nat32 c,nat32 d);


  /// This adds a field, the direct access version, you will ushally use the
  /// neater one that determines type etc automatically. Takes a token as the 
  /// name to access the field by, a token to indicate the type of the field,
  /// how many bytes the field is and a pointer to a block of data which is 
  /// the default value. If the field name allready exists it is deleted and
  /// replaced with the new field.
   void Add(str::Token name,str::Token type,nat32 size,const void * ini);

  /// This adds a field, all you provide is the name of the field and an 
  /// object, the typename of this object is then taken as the type, the size
  /// of the object as the size of the field and the object passed in to be 
  /// the default data. The prefered way of creating a field.
   template <typename T>
   void Add(str::Token name,const T & ini)
   {
    Add(name,core.GetTT()(typestring<T>()),sizeof(T),(void*)&ini);
   }

  /// This adds a field, all you provide is the name of the field and an 
  /// object, the typename of this object is then taken as the type, the size
  /// of the object as the size of the field and the object passed in to be 
  /// the default data. The prefered way of creating a field.
   template <typename T>
   void Add(cstrconst name,const T & ini)
   {
    Add(core.GetTT()(name),ini);
   }

  /// This removes a field.
   void Rem(str::Token name);
   
  /// This removes a field.
   void Rem(cstrconst name) {Rem(core.GetTT()(name));}


  /// This actually applies all changes made by the dim/size and field editting
  /// methods. Note that once you have called any such methods until you call Commit
  /// none of the other methods can be expected to work correctly, and can even cause
  /// crashes. useDefault indicates if it should overwrite new fields with there
  /// default values, you can set this to false if you will be filling them in imminently.
  /// Note that this can be quite slow for the case where Setup hasn't been called, as 
  /// it has to regig the data quite intricatly. Note that calling Setup with the same
  /// details as before will cause all data to be wipped and this to run faster, so if 
  /// just adjusting the structure ready for a complete blit of new data which happens 
  /// to be the same dimensions/size call Setup anyway, as it will be faster overall.
   void Commit(bit useDefault = true);


  /// Returns the number of dimensions.
   nat32 Dims() const{return dims;}

  /// Returns the size of dimension n.
   nat32 Size(nat32 n) const {return size[n];}
   
  /// Returns an array of sizes, suitable for passing into the Setup() method.
   const nat32 * Sizes() const {return size;}

  /// Returns the stride at the given scale. For a value of 0 this is the sum 
  /// total of all the fields, i.e. the size of a single entry in the multidimensional
  /// array. For further levels it is first the size of an entire row in bytes, then a 
  /// plane, then a cube and so on, assuming it actually gets that far. The highest
  /// requestable is Stride(Dims()), which returns how many bytes of memory the data 
  /// alone is consuming. If this is not a large number you are not using this class 
  /// right.
   nat32 Stride(nat32 l) const {return stride[l];}
   
  /// Returns an array of strides. See Stride() for explanation.
   nat32 * Strides() const {return stride;}

  /// Returns how many items are being stored in the Var, i.e. what you get if you multiply
  /// Size(0..Dims()-1) together.
   nat32 Count() const {return stride[dims]/stride[0];}


  /// Returns the number of fields provided.
   nat32 Fields() const {return fields;}

  /// Returns the name of field i.
   str::Token FieldName(nat32 i) const {return fi[i]->name;}

  /// Returns the type of field i.
   str::Token FieldType(nat32 i) const {return fi[i]->type;}

  /// Returns the size of field i.
   nat32 FieldSize(nat32 i) const {return fi[i]->size;}

  /// Outputs the default data of field i.
   const void * FieldDef(nat32 i) const {return fi[i]->ini;}

  /// Outputs the default data of field i.
   void FieldDefault(nat32 i,void * out) const {mem::Copy((byte*)out,fi[i]->ini,fi[i]->size);}


  /// Returns true if the given field exists, false if it does not. 
   bit Exists(str::Token name) const
   {
    return byName.Get(name)!=null<nat32*>();	   
   }
   
  /// Returns true if the given field exists, false if it does not. 
   bit Exists(cstrconst name) const
   {
    return Exists(core.GetTT()(name));  
   }
   
  /// Returns true if the given field name exists and it has the given type.
   bit Exists(str::Token name,str::Token type) const
   {
    nat32 * ind = byName.Get(name);
    return ind && (fi[*ind]->type==type);
   }
  
  /// Returns true if the given field name exists and it has the given type.
   bit Exists(cstrconst name,cstrconst type) const
   {
    return Exists(core.GetTT()(name),core.GetTT()(type));
   }

   
  /// Given a field name this outputs its index, assuming it exists. Returns false
  /// if it does not exist.
   bit GetIndex(str::Token name,nat32 & out) const 
   {
    nat32 * ind = byName.Get(name);
    if (ind) {out = *ind; return true;}
        else return false;
   }


  /// This outputs a Field for accessing a field indicated by its name.
  /// Returns true on success, or false on failure, i.e. the field does
  /// not exist.
   template <typename T>
   bit ByName(str::Token name,Field<T> & out);
   
  /// &nbsp;
   template <typename T>
   bit ByName(str::Token name,const Field<T> & out) const;

  /// This outputs a Field for accessing a field indicated by its name.
  /// Returns true on success, or false on failure, i.e. the field does
  /// not exist.
   template <typename T>
   bit ByName(cstrconst name,Field<T> & out);
   
  /// &nbsp;
   template <typename T>
   bit ByName(cstrconst name,const Field<T> & out) const;  

  /// This outputs a Field for accessing a field indicated by its index.
   template <typename T>
   void ByInd(nat32 ind,Field<T> & out);
   
  /// &nbsp;
   template <typename T>
   void ByInd(nat32 ind,const Field<T> & out) const;
   
   
  /// Returns the size of the given field index, in bytes...
   nat32 FieldMemory(nat32 ind) const;
   
  /// Given a data block of FieldMemory size this writes out the entire field, 
  /// raw, into that data block.
   void GetRaw(nat32 ind,byte * out) const;


  /// Returns a pointer to the given field index at the given 1D offset.
  /// Obviously not safe, play nice.
   void * Ptr(nat32 ind,nat32 x)
   {return data + fi[ind]->offset + x*stride[0];}
   
  /// Returns a pointer to the given field index at the given 1D offset.
  /// Obviously not safe, play nice.
   void * Ptr(nat32 ind,nat32 x,nat32 y)
   {return data + fi[ind]->offset + x*stride[0] + y*stride[1];}
   
  /// Returns a pointer to the given field index at the given 1D offset.
  /// Obviously not safe, play nice.
   void * Ptr(nat32 ind,nat32 x,nat32 y,nat32 z)
   {return data + fi[ind]->offset + x*stride[0] + y*stride[1] + z*stride[2];}

  /// Returns a pointer to the given field index at the given 1D offset.
  /// Obviously not safe, play nice.
   void * Ptr(nat32 ind,nat32 x,nat32 y,nat32 z,nat32 t)
   {return data + fi[ind]->offset + x*stride[0] + y*stride[1] + z*stride[2] + t*stride[3];}


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
   inline cstrconst TypeString() const {return "eos::svt::Var";}


 protected:
  // The virtual functions inherited from Meta, used to export a whole stack of
  // meta data.
   nat32 ExportSize() const;
   str::Token Export(nat32 i) const;
   bit IsExport(str::Token tok) const;
   bit CanSet(str::Token tok) const;
   cstrconst GetDescription(str::Token tok) const;
   Meta::Type OnGetType(str::Token tok) const;
   bit OnGetInt(str::Token tok,int32 & out) const;
   bit OnGetReal(str::Token tok,real32 & out) const;
   bit OnGetToken(str::Token tok,str::Token & out) const;
   bit OnGetString(str::Token tok,str::String & out) const;
   bit OnSetInt(str::Token tok,int32 in);
   bit OnSetReal(str::Token tok,real32 in);
   bit OnSetToken(str::Token tok,str::Token in);
   bit OnSetString(str::Token tok,const str::String & in);


 private:
  // Dims & size meta-data...
   bit changed; // Set to true when a change is made, set back to false when the change is commited.
   nat32 dims; // How many dism there are.
   nat32 * size; // An array of size dims, size of each dimension.
   nat32 * stride; // An array of size dims+1, contains the pre-multiplied up skip sizes, to make access fast.


  // Fields meta-data...
   class Entry // represents a field, not called that as it would be a name clash.
   {
    public:
      Entry():ini(null<byte*>()) {}

      Entry(const Entry & rhs)
      :name(rhs.name),type(rhs.type),size(rhs.size),ini(mem::Malloc<byte>(rhs.size)),
      offset(rhs.offset),state(rhs.state)
      {mem::Copy(ini,rhs.ini,size);}

     ~Entry() {mem::Free(ini);}

     str::Token name;
     str::Token type;
     nat32 size;
     byte * ini; // Malloc'ed.

     nat32 offset; // Offset into any given set of fields to get to this one.

     enum State {Stored, // It is currently in the data structure, no change needed.
                 Added, // It does not currently exist and must be added next Commit.
                 Deleted // It is to be deleted at the next Commit.
                } state;
   };

   nat32 fields; // Number of fields contained within.
   Entry ** fi; // Pointer to an array containing all the fields.

   ds::SparseHash<nat32> byName; // Indexes into the fi array indexed by the names of fields, so ByName can be implimented.


  // Actual data, everything is stored in a single buffer...
   byte * data; // Malloc'ed.
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// This little section is to get arround a circular linkage problem...
#include "eos/svt/field.h"

namespace eos
{
 namespace svt
 {
  template <typename T>
  Var::Var(const Field<T> & f)
  :Meta(f.GetVar()->GetCore()),
  changed(true),dims(0),size(null<nat32*>()),stride(null<nat32*>()),
  fields(0),fi(null<Entry**>()),byName(3),
  data(null<byte*>())
  {
   Setup(f.Dims(),f.Sizes());
  }
 
  template <typename T>
  bit Var::ByName(str::Token name,Field<T> & out)
  {
   nat32 * ind = byName.Get(name);
   if (ind) {out.Set(this,data+fi[*ind]->offset,stride); return true;}
       else {out.SetInvalid(); return false;}
  }

  template <typename T>
  bit Var::ByName(str::Token name,const Field<T> & out) const
  {
   nat32 * ind = byName.Get(name);
   if (ind) {out.Set(this,data+fi[*ind]->offset,stride); return true;}
       else {out.SetInvalid(); return false;}
  }
  
  template <typename T>
  bit Var::ByName(cstrconst name,Field<T> & out)
  {
   return ByName(core.GetTT()(name),out);
  }

  template <typename T>
  bit Var::ByName(cstrconst name,const Field<T> & out) const
  {
   return ByName(core.GetTT()(name),out);
  }
  
  template <typename T>
  void Var::ByInd(nat32 ind,Field<T> & out)
  {
   out.Set(this,data+fi[ind]->offset,stride);
  }

  template <typename T>
  void Var::ByInd(nat32 ind,const Field<T> & out) const
  {
   out.Set(this,data+fi[ind]->offset,stride);
  }

 };
};

//------------------------------------------------------------------------------
#endif
