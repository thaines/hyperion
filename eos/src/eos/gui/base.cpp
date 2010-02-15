//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/gui/base.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
Base::Base()
:parent(null<Base*>()) 
{}

Base::~Base() 
{}

Base * Base::Parent() const 
{
 return parent;
}

Base * Base::SuperParent() const 
{
 if (parent==null<Base*>()) return const_cast<Base*>(this); 
                       else return parent->SuperParent();
}

void Base::SetParent(Base * np)
{
 parent = np;
}

void Base::Detach(Base * child)
{}

//------------------------------------------------------------------------------
Factory::Factory(str::TokenTable & tokTab)
:mem::ObjectFactory<Base,Factory*>(tokTab)
{}

Factory::~Factory()
{}

//------------------------------------------------------------------------------
 };
};
