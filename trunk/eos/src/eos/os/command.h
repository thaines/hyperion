#ifndef EOS_OS_COMMAND_H
#define EOS_OS_COMMAND_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file command.h
/// Allows the forking of another process to execute an arbitary command.
/// Also provides the ability to do default handling of various things by the
/// OS, so openning a directory for viewing with the file manager, openning a 
/// file with the default program etc.

#include "eos/types.h"

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
/// Executes an arbitary command on the current OS, it is released into a 
/// proccess of its own and this function returns instantly - you will never 
/// know what happened.
EOS_FUNC void Execute(cstrconst command);

/// This does the default OS action on the given directory/file - for directorys
/// it will open in a seperate process a filesystem browser, for files it will
/// run the default program for the filetype. Can also cope with url's - 
/// openning them in the default web browser.
/// this is not guaranteed to work - it does its best.
EOS_FUNC void DoSomething(cstrconst thing);

//------------------------------------------------------------------------------
 };
};
#endif
