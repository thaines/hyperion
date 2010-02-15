#ifndef EOS_GUI_CALLBACKS_H
#define EOS_GUI_CALLBACKS_H
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


/// \file callbacks.h
/// Provides a system to represent a callback function, implimented so as to
/// provide extensive capability. Designed for the gui system exclusivly.

#include "eos/types.h"
#include "eos/gui/base.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
/// The event type, from which all events inherit.
class EOS_CLASS Event : public Deletable
{
 public:
  /// &nbsp;
   Event() {}
   
  /// &nbsp;
   ~Event() {}


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// Null Event, for when there is no data in relation to the event and all that
/// matters is that it happened.
class EOS_CLASS NullEvent : public Event
{
 public:
  /// &nbsp;
   NullEvent() {}
   
  /// &nbsp;
   ~NullEvent() {}
  

  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Event for when a mouse down/up event is sent.
class EOS_CLASS MouseButtonEvent : public Event
{
 public:
  /// &nbsp;
   MouseButtonEvent() {}
   
  /// &nbsp;
   ~MouseButtonEvent() {}
  

  /// &nbsp;
   cstrconst TypeString() const;
   
   
  /// Enum of mouse buttons.
   enum Button {LMB, ///< Left Mouse Button.
                MMB, ///< Middle Mouse Button.
                RMB  ///< Right Mouse Button.
               };


  Button button; ///< Which button has changed state.   
  int32 x; ///< The location of the mouse cursor.
  int32 y; ///< The location of the mouse cursor.     
  bit down; ///< true if the button has gone from up to down, false if it has gone from down to up.
};

//------------------------------------------------------------------------------
/// Event for when the mouse wheel is moved.
class EOS_CLASS MouseWheelEvent : public Event
{
 public:
  /// &nbsp;
   MouseWheelEvent() {}
   
  /// &nbsp;
   ~MouseWheelEvent() {}
  

  /// &nbsp;
   cstrconst TypeString() const;


  int32 x; ///< The location of the mouse cursor.
  int32 y; ///< The location of the mouse cursor.
  int32 deltaX; ///< The delta of the wheel in the x direction, most mice don't suport this.
  int32 deltaY; ///< The delta of the wheel in the y direction, thw ushal wheel motion.
};

//------------------------------------------------------------------------------
/// Event for when responding to every mouse movement.
class EOS_CLASS MouseMoveEvent : public Event
{
 public:
  /// &nbsp;
   MouseMoveEvent() {}
   
  /// &nbsp;
   ~MouseMoveEvent() {}
  

  /// &nbsp;
   cstrconst TypeString() const;
   
   
  int32 x; ///< The x position in object relevent coordinates.
  int32 y; ///< The y position in object relevent coordinates.  
   
  bit lmb; ///< true if the left mouse button is down.
  bit mmb; ///< true if the middle mouse button is down.
  bit rmb; ///< true if the right mouse button is down.
};

//------------------------------------------------------------------------------
/// Event for when a window close request is sent.
class EOS_CLASS DeathEvent : public Event
{
 public:
  /// &nbsp;
   DeathEvent():doDeath(true) {}
   
  /// &nbsp;
   ~DeathEvent() {}
  

  /// &nbsp;
   cstrconst TypeString() const;
  
  /// An output field, set to true to carry out the death, false to throw
  /// a last minite reprieve into the mix. Defaults to true.
   bit doDeath;
};

//------------------------------------------------------------------------------
/// Event for when a choice is made on a yes/no or ok/cancel dialog
class EOS_CLASS ChoiceEvent : public Event
{
 public:
  /// &nbsp;
   ChoiceEvent() {}
   
  /// &nbsp;
   ~ChoiceEvent() {}
  

  /// &nbsp;
   cstrconst TypeString() const;
  
  /// The result of he choice, one button will equate to true, the other to
  /// false.
   bit result;
};

//------------------------------------------------------------------------------
/// Event for when a choice is made on a yes/no or ok/cancel dialog
class EOS_CLASS FileEvent : public Event
{
 public:
  /// &nbsp;
   FileEvent():accept(false) {}
   
  /// &nbsp;
   ~FileEvent() {}


  /// &nbsp;
   cstrconst TypeString() const;

  /// Set to true if they accepted a file, false if they did not.
   bit accept;

  /// Contains the filename of the file they accepted, if they did accept one.
   str::String filename;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// This defines a callback as used by the gui system, a virtual class that
/// provides the same interface regardless, we then define flat templated
/// children of this type for all possible callback scenarios.
class EOS_CLASS Callback : public Deletable
{
 public:
  /// &nbsp;
   Callback() {}

  /// &nbsp;
   ~Callback() {}


  /// Makes a clone of the object that can be deleted using the delete operator,
  /// so we can internally store passed in callbacks.
   virtual Callback * Clone() const = 0;

  /// We call this method to execute the callback, it takes an event object
  /// which will contain data relevent to the callback. Any returning of 
  /// information is done by editting the event itself.
   virtual void Call(Base * obj,Event * event) = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// A strange entity - a callback that does nothing. This exists however as a 
/// conveniant way of blocking, i.e. by setting a callback on an object the 
/// signal dosn't reach its children, that might be the only purpose, in which 
/// case this class makes sense. Alternativly the default answer and the default
/// answer with a callback might be different.
class EOS_CLASS NullCallback : public Callback
{
 public:
  /// &nbsp;
   NullCallback() {}
   
  /// &nbsp;
   ~NullCallback() {}


  /// &nbsp;
   Callback * Clone() const
   {
    return new NullCallback();
   }

   void Call(Base * obj,Event * event) {}


  /// &nbsp;
   cstrconst TypeString() const {return "eos::gui::NullCallback";}
};

//------------------------------------------------------------------------------
/// The classical callback scenario - a function that takes the object and event
/// as well as one extra item, ushally a pointer to some object.
template <typename T>
class EOS_CLASS FunctionCallback : public Callback
{
 public:
  /// &nbsp;
   FunctionCallback(void (*F)(Base * obj,Event * event,T srt),T srt)
   :Func(F),data(srt)
   {}
   
  /// &nbsp;
   ~FunctionCallback() {}


  /// &nbsp;
   Callback * Clone() const
   {
    return new FunctionCallback(Func,data);
   }

   void Call(Base * obj,Event * event)
   {
    Func(obj,event,data);
   }


  /// &nbsp;
   cstrconst TypeString() const {return "eos::gui::FunctionCallback<>";}


 private:
  void (*Func)(Base * obj,Event * event,T srt);
  T data;
};

//------------------------------------------------------------------------------
/// The more mature callback scenario, an object pointer and a method to be 
/// called, which takes the object and event.
template <typename T>
class EOS_CLASS MethodCallback : public Callback
{
 public:
  /// &nbsp;
   MethodCallback(T * obj,void (T:: *M)(Base * obj,Event * event))
   :ptr(obj),Method(M)
   {}
   
  /// &nbsp;
   ~MethodCallback() {}


  /// &nbsp;
   Callback * Clone() const
   {
    return new MethodCallback(ptr,Method);
   }

   void Call(Base * obj,Event * event)
   {
    (ptr->*Method)(obj,event);
   }


  /// &nbsp;
   cstrconst TypeString() const {return "eos::gui::MethodCallback<>";}


 private:
  T * ptr;
  void (T:: *Method)(Base * obj,Event * event);
};

//------------------------------------------------------------------------------
/// This allocates and returns a FunctionCallback, provided to make neat code
/// as you can't do the implicit template parameter thing with classes.
template <typename T>
inline Callback * MakeCB(void (*F)(Base * obj,Event * event,T srt),T srt)
{
 return new FunctionCallback<T>(F,srt);
}

/// This allocates and returns a MethodCallback, provided to make neat code
/// as you can't do the implicit template parameter thing with classes.
template <typename T>
inline Callback * MakeCB(T * obj,void (T:: *M)(Base * obj,Event * event))
{
 return new MethodCallback<T>(obj,M);
}

//------------------------------------------------------------------------------
 };
};
#endif
