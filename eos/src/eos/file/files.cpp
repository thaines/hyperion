//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/file/files.h"

#include "eos/math/functions.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef EOS_WIN32
 #include <io.h>
#endif
#include <unistd.h>
#include <string.h>

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
EOS_FUNC cstr TemporyFilename()
{
 return str::Duplicate(tmpnam(0));
}

EOS_FUNC void DeleteFile(cstrconst fn)
{
 ::unlink(fn);
}

//------------------------------------------------------------------------------
#ifdef EOS_WIN32
 #define O_LARGEFILE    O_BINARY
 #define O_SYNC         0
 #define O_DIRECT       0
 
 #define FILE_PERMS     S_IWRITE | S_IREAD
#else
 #define FILE_PERMS     S_IWUSR | S_IRUSR | S_IRGRP
#endif
//------------------------------------------------------------------------------
static const int wayFlag[4] = {O_CREAT | O_EXCL | O_LARGEFILE,
                               O_CREAT | O_TRUNC | O_LARGEFILE,
                               O_LARGEFILE,
                               O_CREAT | O_APPEND | O_LARGEFILE};

static const int modeFlag[3] = {O_RDONLY,O_WRONLY,O_RDWR};

static const int safeFlag[2] = {0,
                                O_SYNC | O_DIRECT};

//------------------------------------------------------------------------------
FileCode::FileCode()
:handle(-1)
{}

FileCode::FileCode(cstrconst fn,Way w,Mode m,Safe s)
:way(w),mode(m),safe(s),handle(open(fn,wayFlag[w] | modeFlag[m] | safeFlag[s],FILE_PERMS))
{}

FileCode::FileCode(const str::String & fn,Way w,Mode m,Safe s)
:way(w),mode(m),safe(s),handle(-1)
{
 cstr fnStr = fn.ToStr();
 handle = open(fnStr,wayFlag[w] | modeFlag[m] | safeFlag[s],FILE_PERMS);
 mem::Free(fnStr);
}

FileCode::FileCode(const Dir & dir,cstrconst fn,Way w,Mode m,Safe s)
:way(w),mode(m),safe(s),handle(-1)
{
 str::String ffn = dir.RealPath(); ffn += fn;
 cstr fnStr = ffn.ToStr();
 handle = open(fnStr,wayFlag[w] | modeFlag[m] | safeFlag[s],FILE_PERMS);
 mem::Free(fnStr);
}

FileCode::FileCode(const Dir & dir,const str::String & fn,Way w,Mode m,Safe s)
:way(w),mode(m),safe(s),handle(-1)
{
 str::String ffn = dir.RealPath(); ffn += fn;
 cstr fnStr = ffn.ToStr();
 handle = open(fnStr,wayFlag[w] | modeFlag[m] | safeFlag[s],FILE_PERMS);
 mem::Free(fnStr);
}

FileCode::~FileCode()
{
 Close();
}

bit FileCode::Active() const
{
 return handle!=-1;
}

bit FileCode::CanRead() const
{
 return Active() || (mode!=mode_write);
}

bit FileCode::CanWrite() const
{
 return Active() || (mode!=mode_read);
}

Way FileCode::GetWay() const
{
 return way;
}

Mode FileCode::GetMode() const
{
 return mode;
}

Safe FileCode::GetSafe() const
{
 return safe;
}

void FileCode::Flush() const
{
 #ifdef EOS_WIN32
  _commit(handle);
 #else
  fsync(handle);
 #endif
}
  
bit FileCode::Open(cstrconst fn,Way w,Mode m,Safe s)
{
 Close();
 
 way = w;
 mode = m;
 safe = s;
 handle = open(fn,wayFlag[w] | modeFlag[m] | safeFlag[s],FILE_PERMS);
 
 return Active();
}

bit FileCode::Open(const str::String & fn,Way w,Mode m,Safe s)
{
 Close();
 
 way = w;
 mode = m;
 safe = s;
 
 cstr fnStr = fn.ToStr();
 handle = open(fnStr,wayFlag[w] | modeFlag[m] | safeFlag[s],FILE_PERMS);
 mem::Free(fnStr);
 
 return Active();
}

void FileCode::Close()
{
 close(handle);
 handle = -1;
}

nat32 FileCode::Size() const
{
 struct stat info;
 if (fstat(handle,&info)==-1) return 0;
 
 return info.st_size;
}

nat32 FileCode::SetSize(nat32 size)
{
 if (ftruncate(handle,size)==0) return size;
                           else return Size();
}

nat32 FileCode::Read(nat32 pos,void * data,nat32 amount) const
{
 lseek(handle,pos,SEEK_SET);
 return read(handle,data,amount);
}

nat32 FileCode::Write(nat32 pos,const void * data,nat32 amount)
{
 lseek(handle,pos,SEEK_SET);
 return write(handle,data,amount);
}

nat32 FileCode::Pad(nat32 pos,byte item,nat32 amount)
{
 static const nat32 buf_size = 32;
 byte buf[buf_size]; memset(buf,item,buf_size);
 lseek(handle,pos,SEEK_SET); 
 nat32 ret = 0;
  while (amount!=ret)
  {
   nat32 toDo = math::Min(amount-ret,buf_size);
   nat32 done = write(handle,buf,toDo);
   ret += done;
   if (done!=toDo) break;
  }
 return ret;
}

//------------------------------------------------------------------------------
 };
};
