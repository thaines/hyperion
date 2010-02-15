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

#include "eos/gui/callbacks.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------

cstrconst NullEvent::TypeString() const {return "eos::gui::NullEvent";}
cstrconst MouseButtonEvent::TypeString() const {return "eos::gui::MouseButtonEvent";}
cstrconst MouseWheelEvent::TypeString() const {return "eos::gui::MouseWheelEvent";}
cstrconst MouseMoveEvent::TypeString() const {return "eos::gui::MouseMoveEvent";}
cstrconst DeathEvent::TypeString() const {return "eos::gui::DeathEvent";}
cstrconst ChoiceEvent::TypeString() const {return "eos::gui::ChoiceEvent";}
cstrconst FileEvent::TypeString() const {return "eos::gui::FileEvent";}

//------------------------------------------------------------------------------
 };
};
