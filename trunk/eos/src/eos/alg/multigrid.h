#ifndef EOS_ALG_MULTIGRID_H
#define EOS_ALG_MULTIGRID_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file multigrid.h
/// Impliments multigrid algorithms for solving linear equations, unsuprisingly.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"


namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// Impliments the multigrid algorithm. This solves Ax = z for a sparse, and 
/// probably larger than avaliable memory, A. x is a 2D grid, the multigrid
/// works on the premise that adjacent A's are related and hence can be combined
/// and the algorithm run at various levels. Uses Gauss-Seidel iterations, and 
/// hence requires the A matrix to be by symmetric and positive definite or
/// diagonally dominant.
/// It is assumed that a constant stencil for the entire A matrix with variable
/// entrys for the stencil can define A.
/// You have to fill in the A matrix for all hierachy levels, making that part 
/// of the hierachy the users responsibility. The mapping of x between the 
/// levels of the hierachy is implimented internally, using linear interpolation.
/// Uses the FMG update scheme, for those who care, with symmetric
/// Gauss-Seidel.
class EOS_CLASS Multigrid2D
{
 public:
  /// &nbsp;
   Multigrid2D();

  /// &nbsp;
   ~Multigrid2D();


  /// Sets the size of the problem - width and height of the 2D grid that is the
  /// x 'vector' and the number of entrys in the stencil, including (0,0).
   void SetSize(nat32 width,nat32 height,nat32 stencilSize);
   
  /// After the size is set this returns the number of hierachy levels avaliable.
   nat32 Levels() const;
  
  /// Returns the width for a particular level.
   nat32 Width(nat32 level) const;

  /// Returns the height for a particular level.
   nat32 Height(nat32 level) const;

  /// (se for stencil entry.)
  /// For each stencil index on each level, 0..stencilSize-1, this sets its 
  /// offset from each pixel in the 2D grid. Note that index 0 is allways offset
  /// (0,0) whether you like it or not.
  /// No two stencil entrys may have the same offset pair.
  /// (se for stencil entry.)
   void SetOffset(nat32 level,nat32 se,int32 offX,int32 offY);
   
  /// If the stencil is the same for all levels you may just fill in level 0 and
  /// then finish the rest by calling this method.
   void SpreadFirstStencil();

  
  /// Sets the b value for each position on the grid at each level.
  /// (All levels but level 0 are essentially initialisation values, before 
  /// they get replaced by residuals.)
   void SetB(nat32 level,nat32 x,nat32 y,real32 val);
   
  /// Sets the value for a stencil position. Have to do it for all levels.
  /// It helps if the values are solving the same approximate problem at all 
  /// levels, else it doesn't really work.
  /// Entry 0, corresponding to offset (0,0), must never be 0.
  /// Entrys that are outside the grid are zeroed automatically.
  /// (se for stencil entry.)
   void SetA(nat32 level,nat32 x,nat32 y,nat32 se,real32 val);
   
  /// Zeroes out all A entrys associated with the given location.
  /// usually used with AddA.
   void ZeroA(nat32 level,nat32 x,nat32 y);
  
  /// Same as SetA except it adds rather than sets. 
   void AddA(nat32 level,nat32 x,nat32 y,nat32 se,real32 val);
   
  // Conveniance method, for debugging purposes mostly.
   real32 GetA(nat32 level,nat32 x,nat32 y,nat32 se);
   
  /// Conveniance method. Allows you to fix a value, which you will often have
  /// to do to stop things floating around. Simply updates the b and a entrys 
  /// accordingly. Will have to fix the value on each level.
   void Fix(nat32 level,nat32 x,nat32 y,real32 val);
   
  /// For providing initialisation - optional, as defaults to 0.
  /// (Good for testing, as you can initialise to answer and see if 
  /// it distorts it.)
   void SetX(nat32 level,nat32 x,nat32 y,real32 val);
   
  // Offsets all values of x by the same amount on the given level.
   void OffsetX(nat32 level,real32 offset);

  
  /// Sets the speed, which defaults to 0.5, that being optimal for a general
  /// problem. 1 is probably more traditional however.
   void SetSpeed(real32 speed);
   
  /// Sets the maximum iterations for solving each Ax = b equation, and the 
  /// tolerance under which to stop.
  /// Default tolerance is 0.001, maxIters is 1024.
   void SetIters(real32 tolerance,nat32 maxIters);
  
  /// Runs the algorithm.
  /// Finds a set of x's such that each b is calculated from multiplication of
  /// the x's at each position with the stencil.
  /// Run trashes various internal structures - only call once for each SetSize.
   void Run(time::Progress * prog = null<time::Progress*>());
   
  /// This also runs the algorithm, except it uses the V update scheme rather
  /// than the FMG update scheme. This means that it can be called repeatedly
  /// to refine a solution with the a and b matrics optionally updated between
  /// runs. Useful when approximating something by the first two terms of its 
  /// Taylor expansion.
  /// Uses all levels of the a matrix but just the first level of the b vector.
  /// Uses just the first level of the x matrix if you are providing 
  /// initialisation there.
  /// Can be called after a first call of Run, or without ever calling Run.
   void ReRun(time::Progress * prog = null<time::Progress*>());
   
   
  /// Returns an x value. The x values will all be zero before tha algorithm is first run.
   real32 Get(nat32 x,nat32 y) const
   {
    return data[0].Get(x,y).x;
   }
   
  /// Extracts a 2D array of x values.
  /// Will resize the array if need be.
   void GetX(ds::Array2D<real32> & out) const;
   
  /// Extracts a 2D array of x values.
  /// Input field must be correct size. (Width and height of level 0.)
   void GetX(svt::Field<real32> & out) const;
   
  /// Returns the residual for a single location on a given level, primarilly 
  /// for debug and analysis stuff.
   real32 Residual(nat32 level,nat32 x,nat32 y);


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 speed;
  real32 tolerance;
  nat32 maxIters;
 
  // Structure to store a stencil entry offste details...
   struct Offset
   {
    int32 x;
    int32 y;
   };
   
  // The stencil offsets, entry 0 will always be (0,0) for all levels...
   ds::ArrayDel<ds::Array<Offset> > stencil; // Indexed [level][se].
  
 
  // Structure to store relevant details for each node.
   struct Node
   {
    real32 b;
    real32 x;
    real32 r; // Used as tempory for transfering up, to save repeated calculation of residual.
    real32 a[1]; // Will be extended to the stencil size. sizeof(Node) + sizeof(real32)*(stencil.Size()-1)
   };
   
  // Super structure that stores everything but the stencil shape.
  // Note that its partially trashed at run time.
   ds::ArrayDel<ds::Array2DRS<Node> > data; // Indexed as data[level].Get(x,y).<...>
   
   
  // Helper methods...
   void TransferUp(nat32 from,time::Progress * prog); // Fills in from+1 b's with the residual of from, zeroes from+1 x's.
   void TransferDown(nat32 to,time::Progress * prog); // Offsets to's x's by the interpolated x's of to+1.
   void Solve(nat32 level,time::Progress * prog); // Runs symmetric Gauss-Seidel iterations on the given level till convergance.


  // Debugging methods...
   real32 TotalResidual(nat32 level); // Returns the residual for a given level.
};

//------------------------------------------------------------------------------
 };
};
#endif
