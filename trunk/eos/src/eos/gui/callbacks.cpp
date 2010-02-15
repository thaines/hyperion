//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
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
