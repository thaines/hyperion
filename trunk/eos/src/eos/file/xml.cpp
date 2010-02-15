//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/file/xml.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
EOS_FUNC bs::Element * LoadXML(str::TokenTable & tokTab,cstrconst filename)
{
 File<io::Text> file(filename,way_edit,mode_read);
 if (!file.Active()) return null<bs::Element*>();
 Cursor<io::Text> cursor = file.GetCursor();

 io::VirtIn< Cursor<io::Text> > in(cursor);
 io::TextParser<io::InVirt<io::Text>,256> tp(in);   
 tp.CommentTypes(false,false,true);	  
  
 bs::Element * ret = new bs::Element(tokTab,str::NullToken);
 while (true)
 {
  tp.InvSkip("<");
  if (ret->ReadText(tp,false,tokTab("content.start"),tokTab("content.post")))
  {
   if (tp.Peek()=='?')
   {
    tp.Next();
    // We have bumped into a <? ... ?> statement - skip it and try again...
     while (true)
     {
      tp.InvSkip("?");
      if (tp.Next()!='?') {delete ret; return null<bs::Element*>();}
      if (tp.Peek()=='>') {tp.Next(); break;}
     }
   } 
   else {delete ret; return null<bs::Element*>();}
  }
  else return ret;
 }
}

EOS_FUNC bs::Element * LoadXML(str::TokenTable & tokTab,const str::String & filename)
{
 cstr fn = filename.ToStr();
 bs::Element * ret = LoadXML(tokTab,fn);
 mem::Free(fn);
 return ret;
}


EOS_FUNC bit SaveXML(bs::Element * root,cstrconst filename,bit overwrite)
{
 File<io::Text> file(filename,overwrite?way_ow:way_new,mode_write);
 if (!file.Active())
 {
  LogDebug("[io.xml] Error openning file to save to. {filename}" << LogDiv() << filename);
  return false;
 }
 Cursor<io::Text> cursor = file.GetCursor();
 
 cursor << "<?xml version=\"1.0\"?>\n";
 cursor << *root;
 
 return !cursor.Error();
}

EOS_FUNC bit SaveXML(bs::Element * root,const str::String & filename,bit overwrite)
{
 File<io::Text> file(filename,overwrite?way_ow:way_new,mode_write);
 if (!file.Active())
 {
  LogDebug("[io.xml] Error openning file to save to. {filename}" << LogDiv() << filename);
  return false;
 }
 Cursor<io::Text> cursor = file.GetCursor();
 
 cursor << "<?xml version=\"1.0\"?>\n";
 cursor << *root;
 
 return !cursor.Error();
}

//------------------------------------------------------------------------------
 };
};
