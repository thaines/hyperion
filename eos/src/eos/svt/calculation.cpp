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

#include "eos/svt/calculation.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
Algorithm::Algorithm(nat32 in,nat32 out)
:inputs(in),io(in+out)
{
 for (nat32 i=0;i<io.Size();i++) io[i] = null<Node*>();
}

Algorithm::~Algorithm()
{}

void Algorithm::SetInput(nat32 input,Node * node)
{
 io[input] = node;
}

Node * Algorithm::GetOutput(nat32 output)
{
 return io[inputs+output];
}

Node * Algorithm::GetInput(nat32 input)
{
 return io[input];
}

void Algorithm::SetOutput(nat32 output,Node * node)
{
 io[inputs+output] = node;
}

//------------------------------------------------------------------------------
MetaAlgorithm::~MetaAlgorithm()
{}

bit MetaAlgorithm::InputOptional(nat32 input) const
{
 return false;
}

bit MetaAlgorithm::OutputOptional(nat32 output) const
{
 return false;
}

bit MetaAlgorithm::GoodDom(bs::Element * elem) const
{
 return true;
}

//------------------------------------------------------------------------------
 };
};
