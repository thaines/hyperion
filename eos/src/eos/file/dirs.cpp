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

#include "eos/file/dirs.h"

#include "eos/ds/lists.h"

#ifdef EOS_WIN32
 #include <windows.h>
 #include <dir.h>
 #include <dirent.h>
#endif

#ifdef EOS_LINUX
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <sys/types.h> 
 #include <dirent.h>
#endif

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
Dir::Dir(SpecialDir pos)
{
 #ifdef EOS_WIN32
  switch(pos)
  {
   case WorkDir: 
   {
    nat32 size = GetCurrentDirectory(0,0);
    cstr cwd = mem::Malloc<cstrchar>(size);
     GetCurrentDirectory(size,cwd);
     osPath = cwd;
     MakePath();
    mem::Free(cwd);	   
   }
   break;
   case RootDir: 
    path = "/";
   break;
   case ConfDir:
   {
    osPath = "C:\\Documents and Settings\\";
    cstrchar buf[256]; // Stupid arbitary limit - know way of getting an actuall number so no choice however. Shoot a m$ programmer. Go on. You know you want to.
    nat32 size = 256; 
    if (GetUserName(buf,&size)==0) str::Copy(buf,"All Users");
    osPath += buf;
    osPath += "\\";
    MakePath();
   }
   break;
   case HomeDir: 
   {
    osPath = "C:\\Documents and Settings\\";
    cstrchar buf[256]; // Stupid arbitary limit - know way of getting an actuall number so no choice however. Shoot a m$ programmer. Go on. You know you want to.
    nat32 size = 256; 
    if (GetUserName(buf,&size)==0) str::Copy(buf,"All Users");
    osPath += buf;
    osPath += "\\Desktop\\";
    MakePath();
   }   
   break;
   case DocsDir:
   {
    osPath = "C:\\Documents and Settings\\";
    cstrchar buf[256]; // Stupid arbitary limit - know way of getting an actuall number so no choice however. Shoot a m$ programmer. Go on. You know you want to.
    nat32 size = 256; 
    if (GetUserName(buf,&size)==0) str::Copy(buf,"All Users");
    osPath += buf;
    osPath += "\\My Documents\\";
    MakePath();
   }
   break;
  }
 #else
  switch(pos)
  {
   case WorkDir: 
   {
    char * cwd = getcwd(0,0);
    if (cwd) path = cwd;
        else path = "/";
    free(cwd);
   }
   break;
   case RootDir: 
    path = "/";
   break;
   case ConfDir:
   {
    char * cwd = getenv("HOME");
    if (cwd) path = cwd;
        else path = "/";
   }   
   break;
   case HomeDir:
   {
    char * cwd = getenv("HOME");
    if (cwd) path = cwd;
        else path = "/";
   }
   break;
   case DocsDir:
   {
    char * cwd = getenv("HOME");
    if (cwd)
    {
     path = cwd;
     if (cwd[str::Length(cwd)-1]!='/') path += "/";
    }
    else path = "/";
    path += "Desktop";
   }
   break;
  }
 #endif
}

Dir::Dir(cstrconst pos)
{
 if (pos[0]=='/')
 {
  path = pos;
 }
 else if ((pos[0]!=0)&&(pos[1]==':')&&(pos[2]=='\\'))
 {
  osPath = pos;
  MakePath();
 }
 else
 {
  *this = Dir(WorkDir);
  path += pos;
 }
 
 Optimise();
}

Dir::Dir(const str::String & strPos)
{
 cstr pos = strPos.ToStr();

 if (pos[0]=='/')
 {
  path = pos;
 }
 #ifdef WIN32
 else if ((pos[0]!=0)&&(pos[1]==':')&&(pos[2]=='\\'))
 {
  osPath = pos;
  MakePath();
 }
 #endif
 else
 {
  *this = Dir(WorkDir);
  path += pos;
 }
 
 mem::Free(pos);
 
 Optimise();
}

Dir::Dir(const Dir & rhs)
:path(rhs.path)
{}

Dir::~Dir()
{}

Dir & Dir::operator = (const Dir & rhs)
{
 path = rhs.path;
 return *this;
}

bit Dir::Valid() const
{
 cstr p = path.ToStr();
  bit ret = 0==access(p,F_OK);
 mem::Free(p);
 return ret;
}

DirType Dir::Type() const
{
 cstr p = path.ToStr();
  DirType ret = (p[str::Length(p)-1]=='/')?TypeDir:TypeFile;
 mem::Free(p);
 return ret;
}

bit Dir::Create()
{
 bit ret = true;

 // Decompose the entire path into a linked list of strings...
  ds::List<str::String*> dec;
  {
   str::String::Cursor targ = path.GetCursor();
   targ.Skip(1); // The openning '/'.
   str::String * cs = new str::String();
   const nat32 buf_size = 64;
   while (targ.Avaliable())
   {
    cstrchar buf[buf_size];
    nat32 amount = targ.Read(buf,buf_size);
    nat32 base = 0;
    for (nat32 i=0;i<amount;i++)
    {
     if ((buf[i]=='/')||(buf[i]=='\\'))
     {
      cs->Write(buf+base,i-base);
      base = i+1;
      dec.AddBack(cs);
      cs = new str::String();
     }
    }
   }
   dec.AddBack(cs);
  }
  
 // Iterate the list, creating each entry as we go...
 {
  path = "";
  ds::List<str::String*>::Cursor targ = dec.FrontPtr();
  while (!targ.Bad())
  {
   path += "/";
   path += **targ;
   ++targ;
   
   cstr p = path.ToStr();
   if (0!=access(p,F_OK))
   {
    #ifdef EOS_WIN32
     if (CreateDirectory(p,0)) {ret = false; mem::Free(p); break;}
    #else
     if (mkdir(p,S_IXUSR | S_IWUSR | S_IXUSR)!=0) {ret = false; mem::Free(p); break;}
    #endif
   }
   mem::Free(p);
  }
 }
 
 // Clean up...
 {
  ds::List<str::String*>::Cursor targ = dec.FrontPtr();
  while (!targ.Bad())
  {
   delete *targ;
   ++targ;
  }
 }
 
 return ret;
}

bit Dir::Editable()
{
 cstr p = path.ToStr();
  bit ret = 0==access(p,W_OK);
 mem::Free(p);
 return ret; 
}

bit Dir::Exists(cstrconst obj)
{
 str::String ep = path; ep += obj;
 
 cstr p = ep.ToStr();
  bit ret = 0==access(p,F_OK);
 mem::Free(p);
 return ret; 
}

bit Dir::Exists(const str::String & obj)
{
 str::String ep = path; ep += obj;
 
 cstr p = ep.ToStr();
  bit ret = 0==access(p,F_OK);
 mem::Free(p);
 return ret;
}

const str::String & Dir::Path() const
{
 return path;
}

const str::String & Dir::RealPath() const
{
 return path;
}

void Dir::Up()
{
 path += "../";
 Optimise();
}

void Dir::Go(cstrconst obj)
{
 path += obj;
 Optimise();
}

void Dir::Go(const str::String & obj)
{
 path += obj;
 Optimise();
}

void Dir::Children(ds::List<cstr,mem::KillDel<cstr> > & out,cstrconst pattern,DirType type)
{
 cstr p = path.ToStr();
 DIR * dir = opendir(p); 
 mem::Free(p);
 
 if (dir)
 {
  while (true)
  {
   dirent * ent = readdir(dir);
   if (ent==0) break;   
   out.AddBack(str::Duplicate(ent->d_name));
  }
  closedir(dir);
 }
}

void Dir::Optimise()
{
 // Decompose the entire path into a linked list of strings...
  ds::List<str::String*> dec;
  {
   str::String::Cursor targ = path.GetCursor();
   targ.Skip(1); // The openning '/'.
   str::String * cs = new str::String();
   const nat32 buf_size = 64;
   while (targ.Avaliable())
   {
    cstrchar buf[buf_size];
    nat32 amount = targ.Read(buf,buf_size);
    nat32 base = 0;
    for (nat32 i=0;i<amount;i++)
    {
     if ((buf[i]=='/')||(buf[i]=='\\'))
     {
      cs->Write(buf+base,i-base);
      base = i+1;
      dec.AddBack(cs);
      cs = new str::String();
     }
    }
   }
   dec.AddBack(cs);
  }

 // Pass through the linked list, identify special strings that can be folded away and fold...
 {
  ds::List<str::String*>::Cursor targ = dec.FrontPtr();
  while (!targ.Bad())
  {
   if ((**targ=="")&&(!targ.End()))
   {
    delete *targ;
    targ.RemKillNext();    
   }
   if (**targ==".")
   {
    delete *targ;
    targ.RemKillNext();
   }
   else if (**targ=="..")
   {
    ds::List<str::String*>::Cursor prev = targ;
    --prev;
    if (!prev.Bad())
    {
     delete *prev;
     prev.RemKillNext();
    }
    
    delete *targ;
    targ.RemKillNext();
   }
   else ++targ;
  }
 }

 // Reconstruct the path from scratch, using the linked list, clean up as we go...
 {
  ds::List<str::String*>::Cursor targ = dec.FrontPtr();
  path = "";
  while (!targ.Bad())
  {
   path += "/";
   path += **targ;
   delete *targ;
   ++targ;
  }
 }
}

void Dir::MakeOS()
{
 #ifdef EOS_WIN32
  str::String::Cursor targ = path.GetCursor();

  // Peek the first 3 values - they should be /<drive letter>/
  // On error we produce a null string, i.e. this is somewhere 
  // the user can't actually interact with.
   cstrchar head[4];
   if ((targ.Read(head,3)!=3)||(head[0]!='/')||(head[2]!='/'))
   {
    osPath = "";
    return;
   }
  
   head[0] = head[1];
   head[1] = ':'; head[2] = '\\'; head[3] = 0;
   osPath = head;
  
   const nat32 buf_size = 64;
   while (targ.Avaliable()!=0)
   {
    cstrchar buf[buf_size];
    nat32 amount = targ.Read(buf,buf_size);
   
    for (nat32 i=0;i<amount;i++)
    {
     if (buf[i]=='/') buf[i] = '\\';
    }
   
    osPath.Write(buf,amount);
   }
 #else
  osPath = path;
 #endif
}

void Dir::MakePath()
{
 #ifdef EOS_WIN32
  if (osPath.Size()<3) path = "/";
  else
  {
   str::String::Cursor targ = osPath.GetCursor();
   
   cstrchar head[3];
   targ.Read(head,3);
   
   path = "/";
   path.Write(head,1);
   path.Write("/",1);
   
   const nat32 buf_size = 64;
   while (targ.Avaliable()!=0)
   {
    cstrchar buf[buf_size];
    nat32 amount = targ.Read(buf,buf_size);
   
    for (nat32 i=0;i<amount;i++)
    {
     if (buf[i]=='\\') buf[i] = '/';
    }
   
    path.Write(buf,amount);
   }
  }
 #else
  path = osPath;
 #endif	
}

//------------------------------------------------------------------------------
 };
};
