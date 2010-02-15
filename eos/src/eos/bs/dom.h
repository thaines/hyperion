#ifndef EOS_BS_DOM_H
#define EOS_BS_DOM_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file dom.h
/// Provides a structure to represent the document object model. Consists of two
/// objects, Attribute and Element, each Element can contain other elements and
/// attributes. Each elements has a name as does each Attribute, attributes then
/// have an associated string, plus a set of methods which interprete that
/// string as particular types. Each Element has indexing of its attributes and
/// children elements for fast access. There are 3 levels of interface to the
/// system:
/// - Low Level, all data within each object is exposed as linked list/tree structures, which can be iterated over.
/// - Medium level, you can access data by name, to get to children of the Element directly.
/// - High level, you provide an indexing string, which tells you where to go to get at a particular data item plus a default for if it dosn't exist.
/// The dom is designed for fast access and relativly slow editting, as it has
/// to update all the hash tables. It is all based on the existance of a token
/// table to store all names as tokens instead of as strings. Strings within the
/// table are still stored as names.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/str/strings.h"
#include "eos/str/tokens.h"
#include "eos/ds/sort_lists.h"
#include "eos/io/to_virt.h"
#include "eos/io/parser.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
/// Represents an Item in a linked list as used by the document object model.
/// Simply provides and next and last methods. Allways uses a circular list with
/// a dummy, you will need to obtain a pointer to the dummy to iterate it 
/// effectivly.
class EOS_CLASS Item
{
 public:
  /// &nbsp;
   Item():next(this),last(this) {}

  /// &nbsp;
   ~Item() {Remove();}


  /// &nbsp;
   Item * Next() const {return next;}
   
  /// &nbsp;
   Item * Last() const {return last;}
   

  /// Adds an item in before this item, removing node from its previous position.
   void Prepend(Item * node);
   
  /// Adds an item in after this item, removing node from its previous position.
   void Append(Item * node);
   
   
  /// Removes this item from its current linked list, sticking it in a linked list all of its own.
   void Remove();


 private:
  Item * next;
  Item * last;
};

//------------------------------------------------------------------------------
/// Represents a node within a Tree as used by the document object model.
/// Provides the ushall interface.
class EOS_CLASS Tree : public Item
{
 public:
  /// &nbsp;
   Tree():parent(null<Tree*>()) {}
  
  /// &nbsp;
   ~Tree() 
   {
    Remove();
    MakeEmpty();
   }
   
  /// Removes all children nodes.
   void MakeEmpty()
   {while (Front()!=Bad()) delete Front();}
   
   
  // Overrides of Item methods with extra functionality/sexier interfaces...
  /// &nbsp;
   Tree * Next() const {return static_cast<Tree*>(Item::Next());}
   
  /// &nbsp;
   Tree * Last() const {return static_cast<Tree*>(Item::Last());}
     
   /// &nbsp;
    void Prepend(Tree * node) {node->parent = parent; Item::Prepend(node);}
    
   /// &nbsp;
    void Append(Tree * node) {node->parent = parent; Item::Append(node);}
   
   /// Completly removes self from its current parent, leaving it with only its children.
    void Remove() {Item::Remove(); parent = null<Tree*>();}

     
  // Parent and children methods...
   /// &nbsp;
    Tree * Parent() const {return parent;}

        
   /// &nbsp;
    Tree * Front() const {return static_cast<Tree*>(dummy.Next());}
    
   /// &nbsp;
    Tree * Back() const {return static_cast<Tree*>(dummy.Last());}
    
   /// When a Tree pointer for a direct child of this class == Bad() do not do
   /// anything with it, it means you have a pointer to the dummy. Ushally used
   /// as the break out condition when iterating all children, as in while(Bad()!=targ) {... targ = targ->Next();}.
    const Tree * Bad() const {return static_cast<const Tree*>(&dummy);}
    
 
   /// Prepends a child to the start of the child list. Deals with the problem that 
   /// calling Front()->Prepend(...) would fail if Front()==Bad().
    void PrependChild(Tree * node) {dummy.Append(node); node->parent = this;}   
   
   /// Appends a child to the end of the child list. Deals with the problem that 
   /// calling Back()->Append(...) would fail if Back()==Bad().
    void AppendChild(Tree * node) {dummy.Prepend(node); node->parent = this;}


 private:
  Tree * parent;
  Item dummy; // Dummy node for the linked list of children.
};

//------------------------------------------------------------------------------
/// An Attribute, as stored within an Element. Simply a name and an associated
/// string, with methods to interprete the string as various types. Caches the
/// results of interpretations as other types.
/// Multiple attributes with the same name should not exist within the same
/// element. Several default attributes are defined, though only exist if there is
/// actual content other than whitespace for them:
/// - 'content.start' - The text within the element, upto the first contained Element or the end of the Element if none are within.
/// - 'content.post' - The text that follows the Element in its containning Element. Can not exist if is the root of a hierachy.
/// To obtain all content within an Element you start with its content.start, then for
/// each sub Element in turn append all its content followed by its 
/// content.post attribute. A (recursive) method is provided to do this in the Element class.
class EOS_CLASS Attribute : public Item
{
 public:
  /// Will initialise its data as that of a blank string - you should do something about this shortly thereafter.
   Attribute(str::Token n):name(n),flags(0) {}
 
  /// &nbsp;
   Attribute(str::Token n,const str::String & d):name(n),asString(d),flags(0) {}
   
  /// &nbsp;
   Attribute(const Attribute & rhs):name(rhs.name),asString(rhs.asString),flags(0) {}


  /// Converts both to a string and compares the string.
  bit operator == (const Attribute & rhs) const {return AsString()==rhs.AsString();}
   
   
  /// &nbsp;
   Attribute * Next() const {return static_cast<Attribute*>(Item::Next());}
   
  /// &nbsp;
   Attribute * Last() const {return static_cast<Attribute*>(Item::Last());}
   
  /// &nbsp;
   void Prepend(Attribute * node) {Item::Prepend(node);}
   
  /// &nbsp;
   void Append(Attribute * node) {Item::Append(node);}

   
  /// &nbsp;
   str::Token Name() const {return name;}
   
  // Due to the indexing system used by the DOM this method should not be called.
  // Exists for the i/o system...
   void SetName(str::Token n) {name = n;}

   
  /// &nbsp;
   const str::String & AsString() const {return asString;}

  /// This will return a string you can edit but will invalidate all previous cached data,
  /// you must finish editting before you do anything again.
   str::String & EditString() {flags = 0; return asString;}


  /// &nbsp;
   str::Token AsToken(str::TokenTable & tokTab) const
   {
    if (!(flags&1)) {asToken = tokTab(asString); flags |= 1;}
    return asToken;
   }
   
  /// &nbsp;
   int32 AsInt() const
   {
    if (!(flags&2)) {asInt = asString.ToInt32(); flags |= 2;}
    return asInt;
   }
   
  /// &nbsp;
   real32 AsReal() const
   {
    if (!(flags&4)) {asReal = asString.ToReal32(); flags |= 4;}
    return asReal;
   }
  
  /// &nbsp;
   bit AsBit() const
   {
    if (!(flags&8)) {asBit = asString.ToBit(); flags |= 8;}
    return asBit;
   }


  /// &nbsp;
   void SetString(const str::String & d) {asString = d; flags = 0;}

  /// &nbsp;
   void SetToken(str::TokenTable & tokTab,str::Token d) {asString = tokTab.Str(d); asToken = d; flags = 1;}
   
  /// &nbsp;
   void SetInt(int32 d) {asString.SetSize(0); asString << d; asInt = d; flags = 2;}
   
  /// &nbsp;
   void SetReal(real32 d) {asString.SetSize(0); asString << d; asReal = d; flags = 4;}
   
  /// &nbsp;
   void SetBit(bit d) {asString = d?"true":"false"; asBit = d; flags = 8;}
  

  /// &nbsp;
   cstrconst TypeString() const {return "eos::bs::Attribute";}


 private:
  str::Token name;
  str::String asString;
  
  // Ordered to get best performance assuming alignment to 4 byte boundaries.
   mutable str::Token asToken; // flags&1
   mutable int32 asInt; // flags&2
   mutable real32 asReal; // flags&4
   mutable byte flags;
   mutable bit asBit; // flags&8
};

//------------------------------------------------------------------------------
/// A Document Object Model Element, the main class where all the functionality
/// resides. Provides multiple interface methods and is fully streamable with
/// the I/O system, allowing for both loading/saving of text XML files and for
/// the loading/saving of a proprietry binary XML encoding which can be streamed
/// over sockets for comunication. Provides multiple, some quite advanced, methods
/// of accessing the hierachy, editting is more limited in nature. Indexes of
/// contained data are created on first use, to make the advanced access methods
/// relativly fast. (And re-created on first use after edit.)
/// When an element dies all its children and attributes also experiance the
/// same unhappy ending.
/// Note that there is nothing to stop a linked list occuring at the root, it
/// just shouldn't happen as thats an invalid dom.
/// Note that when reading into this it presumes the stream starts directly
/// at a '<' of the node, and will clear out anything currently contained
/// within. You should not read into a Element when its a child of something
/// else as it won't indicate its changes back so the indexes could be left
/// invalid yet marked as valid. As it only reads the head you need extra
/// code to mange prolog. The file::LoadXML function provides this.
class EOS_CLASS Element : public Tree
{
 public:
  /// &nbsp;
   Element(str::TokenTable & tt,str::Token n):tokTab(tt),name(n),aValid(false),eValid(false) {}
   
  /// &nbsp;
   Element(str::TokenTable & tt,cstrconst n):tokTab(tt),name(tokTab(n)),aValid(false),eValid(false) {}

  /// &nbsp;
   Element(const Element & rhs);

  /// &nbsp;
   ~Element()
   {
    while (FrontAttribute()!=BadAttribute()) delete FrontAttribute();
   }

  /// This removes all attributes and children.
   void MakeEmpty()
   {
    Tree::MakeEmpty();
    while (FrontAttribute()!=BadAttribute()) delete FrontAttribute();
    aValid = false;
    eValid = false;
   }


  /// &nbsp;
   str::TokenTable & TokTab() const {return tokTab;}  

  /// &nbsp;
    str::Token Name() const {return name;}


  // Overrides of parent class methods to work with the prefered types and
  // update index validity flags...
   /// &nbsp;
    Element * Next() const {return static_cast<Element*>(Tree::Next());}
   
   /// &nbsp;
    Element * Last() const {return static_cast<Element*>(Tree::Last());}
     
   /// &nbsp;
    void Prepend(Element * node);
    
   /// &nbsp;
    void Append(Element * node);
   
   /// &nbsp;
    void Remove();

   /// &nbsp;
    Element * Parent() const {return static_cast<Element*>(Tree::Parent());}
        
   /// &nbsp;
    Element * Front() const {return static_cast<Element*>(Tree::Front());}
    
   /// &nbsp;
    Element * Back() const {return static_cast<Element*>(Tree::Back());}
    
   /// &nbsp;
    const Element * Bad() const {return static_cast<const Element*>(Tree::Bad());}
    
   /// &nbsp;
    void PrependChild(Element * node);
   
   /// &nbsp;
    void AppendChild(Element * node);


  
  // Attribute list browsing methods...
   /// &nbsp;
    Attribute * FrontAttribute() const {return static_cast<Attribute*>(attributes.Next());}
    
   /// &nbsp;
    Attribute * BackAttribute() const {return static_cast<Attribute*>(attributes.Last());}
    
   /// &nbsp;
    const Attribute * BadAttribute() const {return static_cast<const Attribute*>(&attributes);}
    
   /// After updating attributes you must call this, unlike the Element editting scheme
   /// there is no way for attributes to feed back they have been editted so the index
   /// data can be marked invalid for update next time its used. Internally it just 
   /// sets a flag, so this method is neglible in even the tightest of loops.
    void OnAttributeChange() {aValid = false;}



  // Element creation conveniance...
   /// Creates a new Element and sticks it after this one in the list, returning it ready for use.
    Element * NewSibling(str::Token name)
    {Element * ret = new Element(tokTab,name); Append(ret); return ret;}
    
   /// Creates a new Element and sticks it after this one in the list, returning it ready for use.
    Element * NewSibling(cstrconst name)
    {Element * ret = new Element(tokTab,name); Append(ret); return ret;}
    
   /// Creates a child element with the given name returning it ready for editting.
    Element * NewChild(str::Token name)
    {Element * ret = new Element(tokTab,name); AppendChild(ret); return ret;}
    
   /// Creates a child element with the given name returning it ready for editting.
    Element * NewChild(cstrconst name)
    {Element * ret = new Element(tokTab,name); AppendChild(ret); return ret;}



  // Attribute creation conveniances...
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,const str::String & data);

   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,cstrconst data) {SetAttribute(name,str::String(data));}

   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,str::Token data);
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,int32 data);
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,real32 data);
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,real64 data) {SetAttribute(name,real32(data));}
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(str::Token name,bit data);

    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,const str::String & data) {SetAttribute(tokTab(name),data);}

   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,cstrconst data) {SetAttribute(tokTab(name),data);}

   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,str::Token data) {SetAttribute(tokTab(name),data);}
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,int32 data) {SetAttribute(tokTab(name),data);}
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,real32 data) {SetAttribute(tokTab(name),data);}
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,real64 data) {SetAttribute(name,real32(data));}    
    
   /// If the attribute dosn't exist it creates it, if it does exist it changes it.
    void SetAttribute(cstrconst name,bit data) {SetAttribute(tokTab(name),data);}
    


  // Level 2 interface, request of attributes and children by name...
   /// Returns the Attribute with the given name, or null if it dosn't exist.
    Attribute * GetAttribute(str::Token name) const;
    
   /// Returns the Attribute with the given name, or null if it dosn't exist.
    Attribute * GetAttribute(cstrconst name) const {return GetAttribute(tokTab(name));}
    
   /// Returns the number of child Elements with the given name.
    nat32 ElementCount(str::Token name) const;
    
   /// Returns the number of child Elements with the given name.
    nat32 ElementCount(cstrconst name) const {return ElementCount(tokTab(name));}
    
   /// Returns the Element with the given name and index, or null if it dosn't exist.
    Element * GetElement(str::Token name,nat32 index = 0) const;
    
   /// Returns the Element with the given name and index, or null if it dosn't exist.
    Element * GetElement(cstrconst name,nat32 index = 0) const {return GetElement(tokTab(name),index);}


   /// Finds a child, you give it a child name and an attribute/value pair,
   /// it then finds and returns, if it exists, the first child that matches
   /// that attribute. Ushally used with things such as ID attributes etc.
    Element * FindChild(str::Token name,const Attribute & toMatch) const;

   /// Finds a child, you give it a child name and an attribute/value pair,
   /// it then finds and returns, if it exists, the first child that matches
   /// that attribute. Ushally used with things such as ID attributes etc.
    Element * FindChild(cstrconst name,const Attribute & toMatch) const
    {return FindChild(tokTab(name),toMatch);}



  // Level 3 interface, request of contained data by indexing strings...
   /// This requests an Attribute by string. By example, the string could
   /// look something like ":element:elem[3].attribute". Elements are always
   /// proceded by ':' whilst Attributes are allways proceded by a '.'.
   /// You can only have 1 Attribute as the last part of the string obviously,
   /// Whitespace is allowed and will be concidered as part of the relevent string.
   /// Attribute names are allowed to contain '.' as that makes for a logical
   /// syntax, Element names are not. The [x] part can be
   /// put on the end of an Element to index when multiple elements are provided,
   /// indexes from 0.
    Attribute * Grab(cstrconst str) const;



  // Defaults interface, an enhancment to both the level 2 and 3 interfaces so
  // default values can be suplied for when the Attribute is returned as null.
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    const str::String & GetString(str::Token name,const str::String & def) const
    {Attribute * attr = GetAttribute(name); return attr?attr->AsString():def;}
  
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    str::Token GetToken(str::Token name,str::Token def) const
    {Attribute * attr = GetAttribute(name); return attr?attr->AsToken(tokTab):def;}
    
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    int32 GetInt(str::Token name,int32 def) const
    {Attribute * attr = GetAttribute(name); return attr?attr->AsInt():def;}
    
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    real32 GetReal(str::Token name,real32 def) const
    {Attribute * attr = GetAttribute(name); return attr?attr->AsReal():def;}
    
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    bit GetBit(str::Token name,bit def) const
    {Attribute * attr = GetAttribute(name); return attr?attr->AsBit():def;}


   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    const str::String & GetString(cstrconst name,const str::String & def) const {return GetString(tokTab(name),def);}
  
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    str::Token GetToken(cstrconst name,str::Token def) const {return GetToken(tokTab(name),def);}
    
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    int32 GetInt(cstrconst name,int32 def) const {return GetInt(tokTab(name),def);}
    
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    real32 GetReal(cstrconst name,real32 def) const {return GetReal(tokTab(name),def);}
    
   /// Accesses an attribute contained within this Element, with a given default
   /// to return if the Attribute dosn't exist.
    bit GetBit(cstrconst name,bit def) const {return GetBit(tokTab(name),def);}


   /// Accesses an attribute by a level 3 index string, with a given default
   /// to return if the index string dosn't work.
    const str::String & GrabString(cstrconst str,const str::String & def) const
    {Attribute * attr = Grab(str); return attr?attr->AsString():def;}
  
   /// Accesses an attribute by a level 3 index string, with a given default
   /// to return if the index string dosn't work.
    str::Token GrabToken(cstrconst str,str::Token def) const
    {Attribute * attr = Grab(str); return attr?attr->AsToken(tokTab):def;}
    
   /// Accesses an attribute by a level 3 index string, with a given default
   /// to return if the index string dosn't work.
    int32 GrabInt(cstrconst str,int32 def) const
    {Attribute * attr = Grab(str); return attr?attr->AsInt():def;}
    
   /// Accesses an attribute by a level 3 index string, with a given default
   /// to return if the index string dosn't work.
    real32 GrabReal(cstrconst str,real32 def) const
    {Attribute * attr = Grab(str); return attr?attr->AsReal():def;}
    
   /// Accesses an attribute by a level 3 index string, with a given default
   /// to return if the index string dosn't work.
    bit GrabBit(cstrconst str,bit def) const
    {Attribute * attr = Grab(str); return attr?attr->AsBit():def;}



  // Misc stuff...
   /// Appends the content of the Element to the given str::String, ignoring
   /// all contained tags. Uses the level 2 interface. (Content means 
   /// all the text that is not part of tags, recursive in nature.)
    void Content(str::String & out) const;
    
   /// This adds new lines and indentation to xml to make it human readable.
   /// If newlines/spaces allready exist it makes no change.
    void MakeHuman(nat32 indent = 0,bit lastChild = true);



  /// &nbsp;
   cstrconst TypeString() const {return "eos::bs::Element";}
   
   
  // Helper functions used by the i/o system...
   void WriteText(io::OutVirt<io::Text> & out,bit doPost,str::Token start,str::Token post) const;
   bit ReadText(io::TextParser<io::InVirt<io::Text>,256> & in,bit doPost,str::Token start,str::Token post);


 private:
  str::TokenTable & tokTab;
  
  // Actual contained data, excluding inherited data obviously, which is the majority...
   str::Token name;
   Item attributes; // Dummy of the attributes.
  
  // Indexes of data plus flags to indicate correctness...  
   mutable bit aValid; // True if the attribute index is up-to-date.
   struct AttrNode
   {
    bit operator < (const AttrNode & rhs) const {return name<rhs.name;}
    
    str::Token name;
    Attribute * attribute;
   };
   mutable ds::SortList<AttrNode> aIndex;
   
   mutable bit eValid; // True if the element index is up-to-date.
   struct ElemNode
   {
    bit operator < (const ElemNode & rhs) const
    {
     if (name==rhs.name) return index<rhs.index;
                    else return name<rhs.name;
    }
    
    str::Token name;
    nat32 index;
    Element * element;
   };
   mutable ds::SortList<ElemNode> eIndex;

  // Methods to build indexes...
   void RebuildA() const; // Rebuilds attribute index.
   void RebuildE() const; // Rebuilds element index.
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


  /*template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Element & rhs,Binary)
  {
   // *******************************************************************************
   return lhs;
  }*/

  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Element & rhs,Text)
  {
   rhs.MakeEmpty();
   io::VirtIn<T> in(lhs);
   io::TextParser<io::InVirt<io::Text>,256> tp(in);   
   tp.CommentTypes(false,false,true);
   lhs.SetError(rhs.ReadText(tp,false,rhs.TokTab()("content.start"),rhs.TokTab()("content.post")));
   return lhs;
  }
 
  /*template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Element & rhs,Binary)
  {
   // *******************************************************************************
   return lhs;
  }*/
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Element & rhs,Text)
  {
   io::VirtOut<T> out(lhs);
   rhs.WriteText(out,false,rhs.TokTab()("content.start"),rhs.TokTab()("content.post"));
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
