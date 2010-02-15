#ifndef EOS_FILE_XML_H
#define EOS_FILE_XML_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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
