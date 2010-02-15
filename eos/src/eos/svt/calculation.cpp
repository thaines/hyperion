//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
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
