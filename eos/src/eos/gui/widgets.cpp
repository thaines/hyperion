//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/gui/widgets.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
Widget::Widget()
{}

Widget::~Widget()
{}

//------------------------------------------------------------------------------
Wrapper::Wrapper()
{}

Wrapper::~Wrapper()
{}

//------------------------------------------------------------------------------
Layout::Layout() 
{}

Layout::~Layout() 
{}

//------------------------------------------------------------------------------
Horizontal::Horizontal()
{}

Horizontal::~Horizontal()
{}

cstrconst Horizontal::TypeString() const
{
 return "eos::gui::Horizontal";
}

//------------------------------------------------------------------------------
Vertical::Vertical()
{}

Vertical::~Vertical()
{}

cstrconst Vertical::TypeString() const
{
 return "eos::gui::Vertical";
}

//------------------------------------------------------------------------------
Grid::Grid()
{}

Grid::~Grid()
{}

cstrconst Grid::TypeString() const
{
 return "eos::gui::Grid";
}

//------------------------------------------------------------------------------
Panel::Panel()
{}

Panel::~Panel()
{}

cstrconst Panel::TypeString() const 
{
 return "eos::gui::Panel";
}

//------------------------------------------------------------------------------
Frame::Frame()
{}

Frame::~Frame()
{}

void Frame::Set(const str::String & label,real32 pos)
{
 cstr l = label.ToStr();
 Set(l,pos);
 mem::Free(l);
}

cstrconst Frame::TypeString() const 
{
 return "eos::gui::Frame";
}

//------------------------------------------------------------------------------
Expander::Expander()
{}

Expander::~Expander()
{}

void Expander::Set(const str::String & label)
{
 cstr l = label.ToStr();
 Set(l);
 mem::Free(l);
}

cstrconst Expander::TypeString() const
{
 return "eos::gui::Expander";
}

//------------------------------------------------------------------------------
Canvas::Canvas()
:onResize(null<Callback*>()),onClick(null<Callback*>()),onWheel(null<Callback*>()),onMove(null<Callback*>())
{}

Canvas::~Canvas()
{
 delete onResize;
 delete onClick;
 delete onWheel;
 delete onMove;
}

void Canvas::OnResize(Callback * cb)
{
 delete onResize;
 onResize = cb;
}

void Canvas::OnClick(Callback * cb)
{
 delete onClick;
 onClick = cb;
}

void Canvas::OnWheel(Callback * cb)
{
 delete onWheel;
 onWheel = cb;
}

void Canvas::OnMove(Callback * cb)
{
 delete onMove;
 onMove = cb;
}

cstrconst Canvas::TypeString() const
{
 return "eos::gui::Canvas";
}

//------------------------------------------------------------------------------
Label::Label()
{}

Label::~Label()
{}

void Label::Set(const str::String & str)
{
 cstr s = str.ToStr();
 Set(s);
 mem::Free(s);
}

cstrconst Label::TypeString() const
{
 return "eos::gui::Label";
}

//------------------------------------------------------------------------------
ComboBox::ComboBox()
:onChange(null<Callback*>())
{}

ComboBox::~ComboBox()
{}

void ComboBox::OnChange(Callback * cb)
{
 delete onChange;
 onChange = cb;
}

cstrconst ComboBox::TypeString() const
{
 return "eos::gui::ComboBox";
}

//------------------------------------------------------------------------------
EditBox::EditBox()
:onChange(null<Callback*>())
{}

EditBox::~EditBox()
{
 delete onChange;
}

void EditBox::Set(const str::String & s)
{
 cstr ss = s.ToStr();
  Set(ss);
 mem::Free(ss);
}

int32 EditBox::GetInt(int32 def) const
{
 int32 ret;
 str::String tempStr = Get();
 str::String::Cursor tempStrCur = tempStr.GetCursor();
 tempStrCur >> ret;
 if (tempStrCur.Error()) return def;
                    else return ret; 
}

real32 EditBox::GetReal(real32 def) const
{
 real32 ret;
 str::String tempStr = Get();
 str::String::Cursor tempStrCur = tempStr.GetCursor();
 tempStrCur >> ret;
 if (tempStrCur.Error()) return def;
                    else return ret;
}

void EditBox::OnChange(Callback * cb)
{
 delete onChange;
 onChange = cb;
}

cstrconst EditBox::TypeString() const
{
 return "eos::gui::EditBox";
}

//------------------------------------------------------------------------------
Multiline::Multiline()
{}

Multiline::~Multiline()
{}

cstrconst Multiline::TypeString() const
{
 return "eos::gui::Multiline";
}

//------------------------------------------------------------------------------
TickBox::TickBox()
:onChange(null<Callback*>())
{}

TickBox::~TickBox()
{}

void TickBox::OnChange(Callback * cb)
{
 delete onChange;
 onChange = cb;
}

cstrconst TickBox::TypeString() const
{
 return "eos::gui::TickBox";
}

//------------------------------------------------------------------------------
Button::Button()
:onClick(null<Callback*>())
{}

Button::~Button()
{
 delete onClick;
}

void Button::OnClick(Callback * cb)
{
 delete onClick;
 onClick = cb;
}

cstrconst Button::TypeString() const
{
 return "eos::gui::Button";
}

//------------------------------------------------------------------------------
ProgressBar::ProgressBar()
{}

ProgressBar::~ProgressBar()
{}

cstrconst ProgressBar::TypeString() const
{
 return "eos::gui::ProgressBar";
}

//------------------------------------------------------------------------------
Window::Window()
:onDeath(null<Callback*>())
{}
   
Window::~Window()
{
 delete onDeath;
}

void Window::SetTitle(const str::String & s)
{
 cstr ss = s.ToStr();
  SetTitle(ss);
 mem::Free(ss);
}

void Window::OnDeath(Callback * cb)
{
 delete onDeath;
 onDeath = cb;
}

cstrconst Window::TypeString() const
{
 return "eos::gui::Window";
}

//------------------------------------------------------------------------------
App::App()
:onLastDeath(null<Callback*>())
{}
   
App::~App()
{
 delete onLastDeath;
}

void App::OnLastDeath(Callback * cb)
{
 delete onLastDeath; 
 onLastDeath = cb;
}

cstrconst App::TypeString() const
{
 return "eos::gui::App";
}

//------------------------------------------------------------------------------
 };
};
