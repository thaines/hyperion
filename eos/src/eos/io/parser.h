#ifndef EOS_IO_PARSER_H
#define EOS_IO_PARSER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file parser.h
/// Provides a wrapper for io::In<io::Text> objects that contains a buffering layer
/// combined with various text parsing tools to make passing files containing
/// text particularly easy. Also provides a escape code adding wrapper for
/// output streams.


#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/io/in.h"
#include "eos/str/strings.h"
#include "eos/str/tokens.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// TextParser provides an extra layer of functionality arround an 
/// io::In<io::Text> object, providing buffering of data and an assortment of
/// methods for doing common textual input parsing operations.
/// Designed so that after each operation the contained stream is left just
/// after the data read in, i.e. in the conveniant position. You can still use
/// the input stream contained within directly, as long as before using this
/// wrapper again you call Reset().
/// Note that this transforms all obtained text before parsing such that the
/// only newline character is the standard \\n, i.e. \\r\\n becomes \\n.
template <typename T,nat32 bufSize>
class EOS_CLASS TextParser
{
 public:
  /// &nbsp;
   TextParser(T & stream):offset(0),size(0),lastRead(0),cS(false),cppS(false),xmlS(false),in(stream) {}
   
  /// &nbsp;
   ~TextParser() {}
   
  /// Call this after calling any operation on the contained stream, to flush
  /// the buffer. After a reset it assumes its not in a comments even if it
  /// were before.
   void Reset() 
   {offset = 0; size = 0; lastRead = 0;}


  /// This allows you to turn on automatic comment skipping, for various types
  /// of comment. All comment skiping defaults to off.
  /// \param cStyle Set to true to skip comments in the / * ... * / form.
  /// \param cppStyle Set to true to skip comments in the / / \\n form.
  /// \param xmlStyle Set to true to skip comments in the < !-- -- > form.
   void CommentTypes(bit cStyle,bit cppStyle,bit xmlStyle)
   {cS = cStyle; cppS = cppStyle; xmlS = xmlStyle;}
  
   
  /// Returns the next character on the string without consuming it, or 0 if
  /// there is no next character...
   cstrchar Peek()
   {
    if (size==0)
    {
     FillBuffer();
     if (size==0) return 0;
    }

    return buf[offset];
   }
   
  /// Returns and consumes the next character in the string, or returns 0
  /// if eos.
   cstrchar Next()
   {
    if (size==0)
    {
     FillBuffer();
     if (size==0) return 0;
    }
    
    cstrchar ret = buf[offset];
    ++offset;
    --size;
        
    in.Skip(1);
    --lastRead;
    
    return ret;
   }
    
  /// Given a 256 byte buffer where it is to skip values set to false and
  /// stop when it finds one set to true, leaving that character as the
  /// next one to be read.
   void SkipBuf(bit lt[256])
   {
    // Make sure we have some data to start with...
     if (size==0)
     {
      FillBuffer();
      if (size==0) return;     
     }
     
    // Pass through buffer...
     nat32 i;
     while (true)
     {
      for (i=0;i<size;i++) {if (lt[buf[offset+i]]) break;}
      offset += i;
      size -= i;
      
      if (size==0)
      {      
       FillBuffer();
       if (size==0) return;     
      }
      else break;
     }
     
    // Move file cursor to correct position...
     in.Skip(i);
     lastRead -= i;
   }
   
  /// Given a string of characters to skip this skips all characters until it
  /// finds one not in the given string or is at the end of the stream.
   void Skip(cstrconst toSkip = " \t\n")
   {
    // Construct lookup table...
     bit lt[256];
     for (nat32 i=0;i<256;i++) lt[i] = true;
     while (*toSkip!=0) {lt[byte(*toSkip)] = false; ++toSkip;}
    
    // Call SkipBuf...
     SkipBuf(lt);
   }
   
  /// Given a string of characters to skip this skips all characters until it
  /// finds one in the given string or is at the end of the stream.
   void InvSkip(cstrconst toAccept = " \t\n")
   {
    // Construct lookup table...
     bit lt[256];
     for (nat32 i=0;i<256;i++) lt[i] = false;
     while (*toAccept!=0) {lt[byte(*toAccept)] = true; ++toAccept;}
    
    // Call SkipBuf...
     SkipBuf(lt);
   }   

  /// This reads a str::String from the stream, it stops reading when it
  /// reaches either the end of stream or when one of a set of characters given
  /// in as a c-style string is found. Characters given in breakChar can never
  /// end up in out, it in fact appends to out without emptying it before hand.
   void ReadString(str::String & out,cstrconst breakChar)
   {
    // Construct lookup table...
     bit lt[256];
     for (nat32 i=0;i<256;i++) lt[i] = false;
     while (*breakChar!=0) {lt[byte(*breakChar)] = true; ++breakChar;}
     
    // Make sure we have some data to bitch over...
     if (size==0)
     {
      FillBuffer();
      if (size==0) return;     
     }     
    
    // Pass through the data...
     nat32 i;
     while (true)
     {
      for (i=0;i<size;i++) {if (lt[buf[offset+i]]) break;}
      out.Write(buf+offset,i);
      offset += i;
      size -= i;

      if (size==0)
      {
       FillBuffer();
       if (size==0) return;     
      }
      else break;
     }
         
    // Leave the stream in the correct position...
     in.Skip(i);
     lastRead -= i;
   }

  /// This reads in a string and turns it into a token for returning, it stops
  /// reading on end of stream or on bumping into any of a given set of
  /// characters to break on.
   str::Token ReadToken(str::TokenTable & tokTab,cstrconst breakChar)
   {
    str::String s;
    ReadString(s,breakChar);
    str::Token ret = tokTab(s);
    //LogDebug("[return] io::TextParser::ReadToken {string,return}" << LogDiv() << s << LogDiv() << ret);
    return ret;
   }

  /// This reads in a string till it finds a given character, ushally something
  /// such as ", it manages escape codes in various possible styles as selected
  /// by given flags. Appends to out rather than emptying it first. Leaves
  /// the stream so that the break code is next read. escapeC indicates
  /// to manage C-style escape codes, escapeXML indicates to manage xml-style 
  /// escape codes. break code has to be raw-read, i.e. if read from an escape
  /// code it is ignored and stuck into the output string.
  /// (Note that numerical c-style code are not accepted and that xml codes don't
  /// validate at all, so certain crazy bastardisations will produced bastard 
  /// like results.)
   void ReadString(str::String & out,cstrchar breakCode,bit escapeC,bit escapeXML)
   {
    // Make sure we have some data to bitch over...
     if (size==0)
     {
      FillBuffer();
      if (size==0) return;     
     }
    
    // Pass through the data...
     nat32 i;
     nat32 red; // reduction in buffer size due to deletions.
     nat32 mode = 0; // 0 = normal text, 1 = c-style escape code, 2/3/4 = xml escape code.
     nat32 num = 0; // Used to build up escape codes for xml.
     while (true)
     {
      for (i=0,red=0;i<size;i++)
      {
       if (mode==0)
       {
        if (buf[offset+i]==breakCode) break;
        else
        {
         if ((buf[offset+i]=='\\')&&escapeC)
         {
	  ++red;
	  mode = 1;
         }
         else
         {
          if ((buf[offset+i]=='&')&&escapeXML)
          {
           ++red;
           mode = 2;
           num = 0;
          }
          else
          {
	   buf[offset+i-red] = buf[offset+i];       
          }
         }
        }
       }
       else
       {
	if (mode==1) // c-style...
	{
	 mode = 0;
	 switch (buf[offset+i])
	 {
          case 'n': buf[offset+i-red] = '\n'; break;
          case 'r': buf[offset+i-red] = '\n'; break;
          case 't': buf[offset+i-red] = '\t'; break;
          case 'b': buf[offset+i-red] = '\b'; break;
          case 'f': buf[offset+i-red] = '\f'; break;
          case '0': buf[offset+i-red] = '\0'; break;
          case '\'': buf[offset+i-red] = '\''; break;
          case '\"': buf[offset+i-red] = '\"'; break;
          case '\\': buf[offset+i-red] = '\\'; break;
	  default: buf[offset+i-red] = buf[offset+i]; break;
         }
	}
	else // xml-style...
	{
         if (buf[offset+i]==';') // End of escape code - spit out the bloody character...
         {
          if (mode==2)
          {
           switch (num)
           {
	    case (((((('q'<<8)|'u')<<8)|'o')<<8)|'t'): buf[offset+i-red] = '\"'; break;
	    case (((((('a'<<8)|'p')<<8)|'o')<<8)|'s'): buf[offset+i-red] = '\''; break;
	    case (((('a'<<8)|'m')<<8)|'p'): buf[offset+i-red] = '&'; break;
	    case (('l'<<8)|'t'): buf[offset+i-red] = '<'; break;
	    case (('g'<<8)|'t'): buf[offset+i-red] = '>'; break;
	    default: buf[offset+i-red] = '?'; break;
           }
          }
          else
          {
	   buf[offset+i-red] = byte(num);
          }
	  mode = 0;
         }
	 else 
	 {
          if (mode==2) // Presume character type unless we find a #...
	  {
	   if (buf[offset+i]=='#')
	   {
	    mode = 3;
	    ++red;
	   }
	   else
	   {
	    num <<= 8;
	    num |= buf[offset+i];
	    ++red;
	   } 
	  }
	  else
	  {
	   if (mode==3) // A decimal number sequence escape code...
	   {
	    if (buf[offset+i]=='x')
	    {
	     mode = 4;
	     ++red;	    
	    }
	    else
	    {
	     num *= 10;
	     num += buf[offset+i]-'0';
	     ++red;	    
	    }
	   }
	   else // A hexidecimal number sequence escape code...
	   {
	    num *= 16;
	    if ((buf[offset+i]>='A')&&(buf[offset+i]<='F')) num += buf[offset+i] - 'A' + 10;
	    else if ((buf[offset+i]>='a')&&(buf[offset+i]<='f')) num += buf[offset+i] - 'a' + 10;
	    else num += buf[offset+i] - '0';
	    ++red;
	   }	 
	  }
         }
	}      
       }       
      }
      out.Write(buf+offset,i-red);
      offset += i;
      size -= i;

      if (size==0)
      {
       FillBuffer();
       if (size==0) return;     
      }
      else break;
     }
         
    // Leave the stream in the correct position...
     in.Skip(i);
     lastRead -= i;
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::io::TextParser<" << typestring<T>() << "," << bufSize << ">");
    return ret;
   }


 private: 
  byte buf[bufSize];
  nat32 offset; // Offset to get to data in the buffer.
  nat32 size; // Amount of data in the buffer.
  nat32 lastRead; // last number of bytes read, so they can be skiped in the stream before a FillBuffer is called, which will reset this number.
  
  // Peeks to fill the buffer with data, replacing current contents.
  // Data taken direct from current position of in. Removes coments and \r...
  // Will leave the buffer containing at least 1 byte of data, unless there is
  // no more data to be had. Automatically consumes previously Peek-ed data used
  // to fill the buffer, so any operation only has to Skip data in the buffer as
  // it leaves it, adjusting lastRead accordingly.
   void FillBuffer()
   {
    // Skip previously read data...
     in.Skip(lastRead);
     
    // Fill the buffer...
     offset = 0;
     size = in.Peek(buf,bufSize);     
     lastRead = size;
     if (size==0) return;
         
    // Check if the buffer starts with a comment - if so we have to move into comment
    // skipping mode...
     if (cS&&(size>=2)&&(buf[0]=='/')&&(buf[1]=='*'))
     {
      // Skip c style comment...
       offset += 2; size -= 2;
       while (true)
       {
	while ((size>0)&&(buf[offset]!='*')) {--size; ++offset;}
	if (buf[offset]=='*')
	{
	 if (size>=2)
	 {
	  if (buf[offset+1]=='/') {size -= 2; offset += 2; break;}
	                     else {--size; ++offset;}
	 }
	 else
	 {
	  // Might be the end, might not - get more data...
	   in.Skip(lastRead);
	   buf[0] = '*';
	   offset = 0;
	   lastRead = in.Peek(buf+1,bufSize-1);
	   size = 1 + lastRead;
	   if (lastRead==0) {size = 0; return;}
	 }
        }
	else
	{
	 in.Skip(lastRead);

         offset = 0;
         size = in.Peek(buf,bufSize);     
         lastRead = size;

         if (size==0) return;	
	}
       }
     }
     else if (cppS&&(size>=2)&&(buf[0]=='/')&&(buf[1]=='/'))
     {
      // Skip C++ style comment...	     
       offset += 2; size -= 2;
       while (true)
       {
	while ((size>0)&&(buf[offset]!='\n')) {--size; ++offset;}
	if (buf[offset]=='\n') {--size; ++offset; break;}	
	else
	{
	 in.Skip(lastRead);

         offset = 0;
         size = in.Peek(buf,bufSize);     
         lastRead = size;

         if (size==0) return;	
	}
       }       
     }
     else if (xmlS&&(size>=4)&&(buf[0]=='<')&&(buf[1]=='!')&&(buf[2]=='-')&&(buf[3]=='-'))
     {
      // Skip xml style comment...
       offset += 4; size -= 4;
       while (true)
       {
	while ((size>0)&&(buf[offset]!='-')) {--size; ++offset;}
	if (buf[offset]=='-')
	{
	 if (size<3)
	 {
	  in.Skip(lastRead);
	  for (nat32 i=0;i<size;i++) buf[i] = buf[offset+i];
	  offset = size;
	  lastRead = in.Peek(buf+offset,bufSize-offset);
	  size = offset + lastRead;
	  if (lastRead==0) {size = 0; return;}
	 }
	 
	 if (size<3) {size = 0; return;}
	 
	 if ((buf[offset+1]=='-')&&(buf[offset+2]=='>')) {size -= 3; offset += 3; break;}
	}
	else
	{
	 in.Skip(lastRead);

         offset = 0;
         size = in.Peek(buf,bufSize);     
         lastRead = size;

         if (size==0) return;	
	}
       }
     }
    
    // Search through the buffer, cutting it short on any comments found.
    // (With concideration so possible coments that overflow the buffer are
    // presumed to be actual coments - faster than peeking more data.)
     for (nat32 i=1;i<size;i++)
     {
      if (buf[i]=='/')
      {
       if (cS&&((i==size-1)||(buf[i+1]=='*'))) {lastRead -= size-i; size = i; }
       if (cppS&&((i==size-1)||(buf[i+1]=='/'))) {lastRead -= size-i; size = i; }
      }
      else if (buf[i]=='<')
      {
       if (xmlS)
       {
        switch (size-i)
        {
         default:
          if (buf[i+3]!='-') break;
         case 3:
          if (buf[i+2]!='-') break;
	 case 2:
	  if (buf[i+1]!='!') break;
         case 1:
          {lastRead -= size-i; size = i;}
         break; 
        }
       }
      }
     }
     
    // Final pass to remove any \r's...
     nat32 wb = 0;
     for (nat32 i=0;i<size;i++)
     {
      buf[wb] = buf[i];
      if (buf[i]!='\r') ++wb;
     }
     size = wb;
    
    // Its possible we have made the buffer totally empty - if so we
    // call ourself again, to have another go...
    // (i.e. all data was \r or two coments flush to one another.)
     if (size==0) FillBuffer();
   }   


  bit cS;
  bit cppS;
  bit xmlS;

  T & in;
};

//------------------------------------------------------------------------------
/// This wraps objects of type io::Out<io::Text>, providing a slight variant of
/// the << interface such that all relevent escape codes are inserted as needed.
/// Allows for arbitary setup of escape codes, and provides methods for doing
/// standard sets. You will ushally want to use an instance of this object
/// simultaneously with the contained object so some strings are escaped
/// whilst others are not.
template <typename T>
class EOS_CLASS TextEscape : public Out<io::Text>
{
 public:
  /// &nbsp;
   TextEscape(T & stream)
   :out(stream)
   {
    for (nat32 i=0;i<256;i++) code[i] = null<cstrconst>();	   
   }

  /// &nbsp;
   ~TextEscape() {}

   
  /// This sets the escape code for a particular character.
  /// All characters by default have no escape code until this function
  /// is called, calling it with a null escape code string will reset
  /// the behaviour to that of having no escape code set.
  /// The object internally maintains a pointer to any string passed in,
  /// so make sure they last the lifetime.
   void SetEscape(cstrchar c,cstrconst esc) {code[byte(c)] = esc;}
  
    
  /// This adds in the standard C-style escape codes as are required to be
  /// used when writting out a c-style string, specifically \ and ".
   void AddCstyle()
   {
    SetEscape('\"',"\\\"");
    SetEscape('\\',"\\\\");
   }
   
  /// This adds in the standard xml/html escape codes, specifically escapes for
  /// <,>,&," and '.
   void AddXML()
   {
    SetEscape('<',"&lt;");	   
    SetEscape('>',"&gt;");
    SetEscape('&',"&amp;");
    SetEscape('\"',"&quot;");
    SetEscape('\'',"&apos;");
   }
  

  // From Out...
   /// &nbsp;
    nat32 Write(void * in,nat32 bytes)
    {
     nat32 ret = 0;
     // Loop the data, work on the assumption that escapes are rare and write
     // as much as possible of un-escaped data then have a seperate write for
     // each escape...
      while (bytes!=0)
      {
       // Seek till we reach the end of the data block or till we bump into a
       // character to be escaped...
        nat32 offset = 0;
        while ((offset<bytes)&&(code[static_cast<byte*>(in)[offset]]==null<cstrconst>())) ++offset;
        if (offset!=0) ret += out.Write(in,offset);
        if (offset==bytes) break;        
        
        in = static_cast<byte*>(in) + offset;
        out << code[*static_cast<byte*>(in)]; ++ret;
        in = static_cast<byte*>(in) + 1;
        bytes -= offset + 1;
      }
     return ret;
    }

   /// &nbsp;
    nat32 Pad(nat32 bytes)
    {
     if (code[byte(' ')]==null<cstrconst>()) return out.Pad(bytes);
     else
     {
      for (nat32 i=0;i<bytes;i++) out << code[byte(' ')]; // Inefficient, but I don't expect this code to ever be called.
      return bytes;
     }     
    }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::io::TextEscape<" << typestring<T>() << ">");
    return ret;
   }


 private:
  T & out;
  
  cstrconst code[256]; // Escape code for each character, null to indicate none.
};

//------------------------------------------------------------------------------
 };
};
#endif
