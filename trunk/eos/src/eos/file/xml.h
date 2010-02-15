#ifndef EOS_FILE_XML_H
#define EOS_FILE_XML_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file xml.h
/// Provides routines for loading and saving xml files. Simple really. (Dosn't
/// actually do anything, the dom provides all the actual code, this is just a
/// nice wrapper.)

#include "eos/types.h"
#include "eos/file/files.h"
#include "eos/bs/dom.h"
#include "eos/str/strings.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// Loads an XML file, returns the root element if it works or null otherwise.
/// You get no information about the error. You must call delete on the Element
/// when done with it, as ushall for all dom stuff.
EOS_FUNC bs::Element * LoadXML(str::TokenTable & tokTab,cstrconst filename);

/// Loads an XML file, returns the root element if it works or null otherwise.
/// You get no information about the error. You must call delete on the Element
/// when done with it, as ushall for all dom stuff.
EOS_FUNC bs::Element * LoadXML(str::TokenTable & tokTab,const str::String & filename);


/// Saves an XMl file, blah.
/// \param root The root of the tree to save.
/// \param filename The filename to save to.
/// \param overwrite If true it will overwrite an existinmg file, if false it won't.
/// \returns true on success, false if it goes tits up.
EOS_FUNC bit SaveXML(bs::Element * root,cstrconst filename,bit overwrite = false);

/// Saves an XMl file, blah.
/// \param root The root of the tree to save.
/// \param filename The filename to save to.
/// \param overwrite If true it will overwrite an existinmg file, if false it won't.
/// \returns true on success, false if it goes tits up.
EOS_FUNC bit SaveXML(bs::Element * root,const str::String & filename,bit overwrite = false);

//------------------------------------------------------------------------------
 };
};
#endif
