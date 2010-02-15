//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/bs/dom.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
void Item::Prepend(Item * node)
{
 node->last = last;
 node->next = this;
 
 node->next->last = node;
 node->last->next = node;
}

void Item::Append(Item * node)
{
 node->last = this;
 node->next = next;
 
 node->next->last = node;
 node->last->next = node;
}

void Item::Remove()
{
 next->last = last;
 last->next = next;
 
 next = this;
 last = this;       
}

//------------------------------------------------------------------------------
Element::Element(const Element & rhs)
:tokTab(rhs.tokTab),name(rhs.name),aValid(false),eValid(false)
{
 // Clone all attributes...
 {
  Attribute * targ = rhs.FrontAttribute();
  while (targ!=rhs.BadAttribute())
  {
   attributes.Prepend(new Attribute(*targ));
   targ = targ->Next();
  }
 }
 
 // Clone all children...
 {
  Element * targ = rhs.Front();
  while (targ!=rhs.Bad())
  {
   AppendChild(new Element(*targ));
   targ = targ->Next();
  }
 }
}

void Element::Prepend(Element * node)
{
 // If no other element with the same name exists we can update the parents
 // index data, otherwise we just resort to setting its flag to indicate the
 // index as now invalid...
  if ((Parent()!=null<Element*>())&&(Parent()->eValid))
  {
   ElemNode en;
    en.name = node->Name();
    en.index = 0;
    if (Parent()->eIndex.Get(en)!=null<ElemNode*>()) Parent()->eValid = false;
    else
    {
     en.element = node;
     Parent()->eIndex.Add(en);
    }
  }
  
 // Actually do the update...
  Tree::Prepend(node);
}

void Element::Append(Element * node)
{
 // If no other element with the same name exists we can update the parents
 // index data, otherwise we just resort to setting its flag to indicate the
 // index as now invalid...
  if ((Parent()!=null<Element*>())&&(Parent()->eValid))
  {
   ElemNode en;
    en.name = node->Name();
    en.index = 0;
    if (Parent()->eIndex.Get(en)!=null<ElemNode*>()) Parent()->eValid = false;
    else
    {
     en.element = node;
     Parent()->eIndex.Add(en);
    }
  }
  
 // Actually do the update...
  Tree::Append(node);      
}

void Element::Remove()
{
 // If its the last item in the parent index we can remove it without too much pain,
 // otherwise we just mark the index as invalid...
  if ((Parent()!=null<Element*>())&&(Parent()->eValid))
  {
   ElemNode dummy;
    dummy.name = Name()+1;
    dummy.index = 0;
   ElemNode * targ = Parent()->eIndex.Largest(dummy);
   if (targ->element==this) Parent()->eIndex.Rem(*targ); 
                       else Parent()->eValid = false;
  }
  
 // Do the actual remove...
  Tree::Remove();
}

void Element::PrependChild(Element * node)
{
 // If no other element with the same name exists we can update the
 // index data, otherwise we just resort to setting its flag to indicate the
 // index as now invalid...
  if (eValid)
  {
   ElemNode en;
    en.name = node->Name();
    en.index = 0;
    if (eIndex.Get(en)!=null<ElemNode*>()) eValid = false;
    else
    {
     en.element = node;
     eIndex.Add(en);
    }
  }

 // Do the actual operation...
  Tree::PrependChild(node);        
}

void Element::AppendChild(Element * node)
{
 // If no other element with the same name exists we can update the
 // index data, otherwise we just resort to setting its flag to indicate the
 // index as now invalid...
  if (eValid)
  {
   ElemNode en;
    en.name = node->Name();
    en.index = 0;
    if (eIndex.Get(en)!=null<ElemNode*>()) eValid = false;
    else
    {
     en.element = node;
     eIndex.Add(en);
    }
  }

 // Do the actual operation...
  Tree::AppendChild(node);      
}

void Element::SetAttribute(str::Token name,const str::String & data)
{
 if (!aValid) RebuildA();
 
 AttrNode dummy;
  dummy.name = name;
 AttrNode * targ = aIndex.Get(dummy);
 
 if (targ!=null<AttrNode*>())
 {
  // Attribute allready exists - change its value...
   targ->attribute->SetString(data);
 }
 else
 {
  // Its a new attribute, create and add it in, updating the index in preference to marking it invalid...
   Attribute * attribute = new Attribute(name,data);
   attributes.Prepend(attribute);
   
   dummy.attribute = attribute;
   aIndex.Add(dummy);
 }
}

void Element::SetAttribute(str::Token name,str::Token data)
{
 if (!aValid) RebuildA();
 
 AttrNode dummy;
  dummy.name = name;
 AttrNode * targ = aIndex.Get(dummy);
 
 if (targ!=null<AttrNode*>())
 {
  // Attribute allready exists - change its value...
   targ->attribute->SetToken(tokTab,data);
 }
 else
 {
  // Its a new attribute, create and add it in, updating the index in preference to marking it invalid...
   Attribute * attribute = new Attribute(name);
   attribute->SetToken(tokTab,data);
   attributes.Prepend(attribute);
   
   dummy.attribute = attribute;
   aIndex.Add(dummy);
 }
}
 
void Element::SetAttribute(str::Token name,int32 data)
{
 if (!aValid) RebuildA();
 
 AttrNode dummy;
  dummy.name = name;
 AttrNode * targ = aIndex.Get(dummy);
 
 if (targ!=null<AttrNode*>())
 {
  // Attribute allready exists - change its value...
   targ->attribute->SetInt(data);
 }
 else
 {
  // Its a new attribute, create and add it in, updating the index in preference to marking it invalid...
   Attribute * attribute = new Attribute(name);
   attribute->SetInt(data);
   attributes.Prepend(attribute);
   
   dummy.attribute = attribute;
   aIndex.Add(dummy);
 }       
}

void Element::SetAttribute(str::Token name,real32 data)
{
 if (!aValid) RebuildA();
 
 AttrNode dummy;
  dummy.name = name;
 AttrNode * targ = aIndex.Get(dummy);
 
 if (targ!=null<AttrNode*>())
 {
  // Attribute allready exists - change its value...
   targ->attribute->SetReal(data);
 }
 else
 {
  // Its a new attribute, create and add it in, updating the index in preference to marking it invalid...
   Attribute * attribute = new Attribute(name);
   attribute->SetReal(data);
   attributes.Prepend(attribute);
   
   dummy.attribute = attribute;
   aIndex.Add(dummy);
 }        
}

void Element::SetAttribute(str::Token name,bit data)
{
 if (!aValid) RebuildA();
 
 AttrNode dummy;
  dummy.name = name;
 AttrNode * targ = aIndex.Get(dummy);
 
 if (targ!=null<AttrNode*>())
 {
  // Attribute allready exists - change its value...
   targ->attribute->SetBit(data);
 }
 else
 {
  // Its a new attribute, create and add it in, updating the index in preference to marking it invalid...
   Attribute * attribute = new Attribute(name);
   attribute->SetBit(data);
   attributes.Prepend(attribute);
   
   dummy.attribute = attribute;
   aIndex.Add(dummy);
 }        
}

Attribute * Element::GetAttribute(str::Token name) const
{
 if (!aValid) RebuildA();       
        
 AttrNode dummy;
  dummy.name = name;
 AttrNode * targ = aIndex.Get(dummy);
 
 if (targ) return targ->attribute;
      else return null<Attribute*>();
}

nat32 Element::ElementCount(str::Token name) const
{
 if (!eValid) RebuildE();  
 
 ElemNode dummy;
  dummy.name = name+1;
  dummy.index = 0;
 ElemNode * targ = eIndex.Largest(dummy);
 if ((targ)&&(targ->name==name)) return targ->index+1;
                            else return 0;
}

Element * Element::GetElement(str::Token name,nat32 index) const
{
 if (!eValid) RebuildE();  
 
 ElemNode dummy;
  dummy.name = name;
  dummy.index = index;
 ElemNode * targ = eIndex.Get(dummy);
 if (targ) return targ->element;
      else return null<Element*>();
}

Element * Element::FindChild(str::Token name,const Attribute & toMatch) const
{
 nat32 n = ElementCount(name);
 for (nat32 i=0;i<n;i++)
 {
  Element * targ = GetElement(name,i);
  Attribute * attribute = targ->GetAttribute(toMatch.Name());
  if (*attribute==toMatch) return targ;
 }
 return null<Element*>();
}

Attribute * Element::Grab(cstrconst str) const
{
 // Our behaviour depends on the first character in the string...
  if (str[0]==':')
  {
   // We are indexing into a sub element, find the end of the element name,
   // manage any issues caused by indexing of multiple elements, then get
   // the relevent element. Then restore the string and get recursive.
    ++str;
    // Find the end of the element name...
     cstrconst next = str;
     while ((next[0]!=':')&&(next[0]!='.')&&(next[0]!=0)) ++next;
     if (next[0]==0) return null<Attribute*>(); // You can't end on an Element - bad string.
    
    // Factor in indexing...
     cstrconst end = next;
     nat32 index = 0;
     if (end[-1]==']')
     {
      // We have an indexing scenario - deal with it...
       while ((end[0]!='[')&&(end>str)) --end;
       const_cast<cstrchar&>(next[-1]) = 0;
        index = str::ToInt32(end+1);
       const_cast<cstrchar&>(next[-1]) = ']';
     }
    
    // Get the element in question...
     Element * nextE = GetElement(tokTab(end-str,str),index);
    
    // Recurse...
     if (nextE) return nextE->Grab(next);
           else return null<Attribute*>();
  }
  else if (str[0]=='.')
  {
   // We are getting a particular attribute, the name of which is the rest
   // of the string, simple enough to do...
    return GetAttribute(str+1);
  }
  else
  {
   // Bad string - they must start with a . or :, so we give up and return null...
    return null<Attribute*>();       
  }        
}

void Element::Content(str::String & out) const
{
 Attribute * start = GetAttribute(tokTab("content.start"));
 if (start) out += start->AsString();
 
 Element * targ = Front();
 while (targ!=Bad())
 {
  targ->Content(out);
  
  Attribute * post = targ->GetAttribute(tokTab("content.post"));
  if (post) out += post->AsString();

  targ = targ->Next();       
 }
}

void Element::MakeHuman(nat32 indent,bit lastChild)
{ 
 // If it has children a post openner new line,
 // plus recurse...
  if (Front()!=Bad())
  {
   Attribute * start = GetAttribute(tokTab("content.start"));
   if (!start)
   {
    str::String s("\n");
    for (nat32 i=0;i<=indent;i++) s += " ";
    SetAttribute("content.start",s);
   }
   
   Element * child = Front();
   while (child!=Bad())
   {
    child->MakeHuman(indent+1,child->Next()==Bad());
    child = child->Next();
   }
  }

 // Post tag newline...
  Attribute * post = GetAttribute(tokTab("content.post"));
  if (!post)
  {
   str::String s("\n");
   for (nat32 i=(lastChild?1:0);i<indent;i++) s += " ";
   SetAttribute("content.post",s);
  }
}

void Element::RebuildA() const
{
 aIndex.MakeEmpty();
 Attribute * targ = FrontAttribute();
 while (targ!=BadAttribute())
 {
  AttrNode an;
   an.name = targ->Name();
   an.attribute = targ;
  aIndex.Add(an);
  
  targ = targ->Next();       
 }
 aValid = true;       
}

void Element::RebuildE() const
{
 eIndex.MakeEmpty();
 Element * targ = Front();
 while (targ!=Bad())
 {
  nat32 ind = 0;
  ElemNode dummy;
   dummy.name = targ->Name()+1;
   dummy.index = 0;
  ElemNode * lg = eIndex.Largest(dummy);
  if ((lg)&&(lg->name==targ->Name())) ind = lg->index+1;
  
  ElemNode en;
   en.name = targ->Name();
   en.index = ind;
   en.element = targ;
  eIndex.Add(en);
  
  targ = targ->Next();       
 }
 eValid = true;      
}

//------------------------------------------------------------------------------
void Element::WriteText(io::OutVirt<io::Text> & out,bit doPost,str::Token start,str::Token post) const
{
 io::TextEscape< io::OutVirt<io::Text> > escapeOut(out);

 // Head...
  out << "<" << tokTab.Str(Name());       
  
 // Attributes...
  bit hasStart = false;
  bit hasPost = false;
  {
   Attribute * targ = FrontAttribute();
   while (targ!=BadAttribute())
   {
    if (start==targ->Name()) hasStart = true;
    else if (post==targ->Name()) hasPost = true;
    else 
    {
     out << " " << tokTab.Str(targ->Name()) << "=\"";
     escapeOut << targ->AsString();
     out << "\"";
    }
    targ = targ->Next();
   }
  }

 // If theres nothing to fill out the tag end with a /> and return...
  Element * targ = Front();
  if ((targ==Bad())&&(hasStart==false))
  {
   out << "/>";
   if (doPost&&hasPost) escapeOut << GetAttribute(post)->AsString();
   return;        
  }
  
 // Child elements and content...
  out << ">";
  if (hasStart) escapeOut << GetAttribute(start)->AsString();
  
  while (targ!=Bad())
  {
   targ->WriteText(out,true,start,post);     
   targ = targ->Next();       
  }
 
 // Tail...
  out << "</" << tokTab.Str(Name()) << ">";
  
 // If doPost has been requested and it has a post do it...
  if (doPost&&hasPost) escapeOut << GetAttribute(post)->AsString();
}

bit Element::ReadText(io::TextParser<io::InVirt<io::Text>,256> & in,bit doPost,str::Token start,str::Token post)
{
 // First get the openning < and the name of the element...
  if (in.Next()!='<') return true;
  if (in.Peek()=='/') return true; // Its a closing tag - return so the presumed parent call can detect and handle it.
  if (in.Peek()=='?') return true; // Its not an element, its something that we need to skip as a member of the prolog.
  name = in.ReadToken(tokTab," \t\n/>");

 // Now extract each attribute in turn...
  while (true)
  {
   in.Skip(" \t\n");
   cstrchar nt = in.Peek();
   if ((nt==0)||(nt=='/')||(nt=='>')) break;
   
   // Read in an attribute...
    str::Token tag = in.ReadToken(tokTab," \t\n=");
    in.Skip(" \t\n");
    if (in.Next()!='=') return true;
    in.Skip(" \t\n");
    if (in.Next()!='\"') return true;
    
    str::String data;
    in.ReadString(data,'\"',false,true);
    
    if (in.Next()!='\"') return true;
    SetAttribute(tag,data);
  }
  
 // Handle the end of the head, including when it is in /> form...
 {
  cstrchar tail = in.Next();
  if (tail=='/')
  {
   if (in.Next()!='>') return true;
   // Its a /> style ending - check the doPost condition and return...
    if (doPost)
    {
     str::String postStr;
     in.ReadString(postStr,'<',false,true);
     if (postStr.Size()!=0) SetAttribute(post,postStr);
    }

   return false;	  
  }
  else
  {
   if (tail!='>') return true;
  }
 }

 // Grab the start text...
 {
  str::String startStr;
  in.ReadString(startStr,'<',false,true);
  if (startStr.Size()!=0) SetAttribute(start,startStr);
 }
  
 // Iterate through grabbing child elements, until we find the tail...
  while (true)
  {
   Element * ne = new Element(tokTab,str::NullToken);
   if (ne->ReadText(in,true,start,post))
   {
    delete ne;
    if (in.Next()=='/') break;
                   else return true;
   }
   else
   {
    AppendChild(ne);
   }
  }
 
 // Handle the tail...
  str::Token endName = in.ReadToken(tokTab," \t\n/>");
  if (endName!=name) return true;
  in.Skip(" \t\n");
  if (in.Next()!='>') return true;
 
 // If requested get the post text for the element...
  if (doPost)
  {
   str::String postStr;
   in.ReadString(postStr,'<',false,true);
   if (postStr.Size()!=0) SetAttribute(post,postStr);
  }

 return false;
}

//------------------------------------------------------------------------------
 };
};
