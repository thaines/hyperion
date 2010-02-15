#ifndef EOS_INF_FIG_VARIABLES_H
#define EOS_INF_FIG_VARIABLES_H
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


/// \file fig_variables.h
/// Provides implimentations of variable patterns for the field graph system.

#include "eos/types.h"
#include "eos/inf/field_graphs.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// This specifies an array of variables in a 2D grid. The parameter vector is
/// (labels,width,height), indexing is (width*y + x).
/// The formular for width given the level is width<<level, and likewise for 
/// height. A minimum of 1 exists for both values.
class EOS_CLASS Grid2D : public VariablePattern
{
 public:
  /// &nbsp;
   Grid2D(nat32 labels,nat32 width,nat32 height);
   
  /// &nbsp;
   ~Grid2D();
   
  /// &nbsp;
   Grid2D * Clone() const;


  /// &nbsp;
   const math::Vector<nat32> & Paras() const;
   
  /// &nbsp;
   nat32 Labels() const;
   
  /// &nbsp;
   nat32 Vars(nat32 level) const;
   
  /// &nbsp;
   nat32 MaxLevel() const;

   
  /// &nbsp;
   void Transfer(nat32 level,VariableTransfer & vt) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  math::Vector<nat32> paras;
};

//------------------------------------------------------------------------------
 };
};
#endif
