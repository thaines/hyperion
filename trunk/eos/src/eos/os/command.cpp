//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/os/command.h"

#include "eos/file/csv.h"

#include "eos/str/functions.h"

#ifdef EOS_WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
EOS_FUNC void Execute(cstrconst command)
{
 #ifdef EOS_WIN32
 WinExec(command,0);
 #else
 if (fork()==0)
 {
  char * com_copy = strdup(command);

  const int ARG_COUNT = 256;
  const char * delim = " \t";

  char * arg[ARG_COUNT];
  arg[0] = strtok(com_copy,delim);
  for (int i=1;i<ARG_COUNT;i++)
  {
   arg[i] = strtok(0,delim);
   if (arg[i]==0) break;
  }
  arg[ARG_COUNT-1] = 0;

  execvp(arg[0],arg);

  free(com_copy);
  exit(1);
 }
 #endif
}

EOS_FUNC void DoSomething(cstrconst thing)
{
 #ifdef EOS_WIN32
 struct stat s;
 if (stat(thing,&s)==0)
 {
  cstrchar buf[1024];
  if (GetFullPathName(thing,1024,buf,0)>1024) return;
  if (S_ISDIR(s.st_mode)) ShellExecute(0,"explore",buf,0,0,SW_SHOW);
                     else ShellExecute(0,"open",buf,0,0,SW_SHOW);
 }
 #else
 static const cstr pre = "gnome-open ";
 cstr command = mem::Malloc<cstrchar>(str::Length(pre)+str::Length(thing)+1);
 str::Copy(command,pre);
 str::Append(command,thing);
 Execute(command);
 mem::Free(command);
 #endif
}

//------------------------------------------------------------------------------
 };
};
