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

#include "eos/rend/scenes.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
TransformStack::TransformStack(Job * rw)
:regWith(rw)
{}

TransformStack::~TransformStack()
{}

void TransformStack::Push(const Transform & in,bit invert)
{
 Node nn;  
  nn.tran = null<OpTran*>();
  
  Transform tr = in;
  if (invert) tr.Invert();
   
  if (stack.Size()==0) nn.t = tr;
  else
  {
   nn.t = stack.Peek().t;
   nn.t *= tr;
  }
  
 stack.Push(nn);
}

void TransformStack::Pop()
{
 stack.Pop(); 
}

OpTran * TransformStack::Current()
{
 Node & targ = stack.Peek();
 if (targ.tran) return targ.tran;
 
 targ.tran = new OpTran(targ.t);
 regWith->Register(targ.tran);
 return targ.tran;
}

//------------------------------------------------------------------------------
Scene::Scene()
{}

Scene::~Scene()
{
 for (nat32 i=0;i<data.Size();i++) delete data[i];
}

bit Scene::Load(bs::Element * scene)
{
 // First 'simple' task, find and parse the background...
 
 // Generate a list of all camera objects, so we can work from that...
 
 
 // For each camera object generate a job, inhabit it with all the objects required...

 // ***************

 return false;
}

nat32 Scene::Jobs() const
{
 return data.Size();
}

Job * Scene::operator[] (nat32 i)
{
 return data[i];
}

//------------------------------------------------------------------------------
 };
};
