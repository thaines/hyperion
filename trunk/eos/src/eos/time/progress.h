#ifndef EOS_TIME_PROGRESS_H
#define EOS_TIME_PROGRESS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

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


/// \file progress.h
/// Provides the progress bar class, used to monitor progress for slow 
/// calculations.

#include "eos/types.h"
#include "eos/time/times.h"

namespace eos
{
 namespace time
 {
//------------------------------------------------------------------------------
/// The Progress class is designed to be passed to time consuming algorithms so
/// they can report how far they have got. Ushally passed as a pointer so a null
/// pointer can be passed if no progress reporting is required. It works such 
/// that progress reporting algorithms can call other progress reporting 
/// algorithms as sub-algorithms and it all works nicelly. When calling a
/// sub algorithm the parent algorthm calls Push() before handing over execution,
/// passing in its progress object, after execution it should then call Pop. This
/// creates a new progress environment thats within the current progress level. 
/// This can continue to an arbitary depth of algorithm nesting. There should be 
/// a call to report between each sub algorithm, as the reported progress behaves
/// badly. Needless to say an algorithm should neevr report a loss of work.
class EOS_CLASS Progress
{
 public:
  /// See Reset for an explanation of depth. Note that the timmer starts ticking
  /// the moment the class is constructed.
   Progress(nat32 depth = 8,bit startPaused = false);
   
  /// &nbsp; 
   virtual ~Progress();
  
  /// Resets the progress bar to the starting position, so it can be reused.
  /// The depth value indicates how deep you recon the progress sub-levels 
  /// will get, in the event of an overflow it is resized, but its obviously
  /// more efficient to avoid this scenario. Note that the moment this is 
  /// called the timer starts ticking.
   void Reset(nat32 depth = 8,bit startPaused = false);
  
   
  /// Push to indicate entering a sub-level of proccessing.  
  /// Safe when this is set to null.
   void Push();
   
  /// Pop to indicate exiting a sub-level of proccessing.
  /// Safe when this is set to null.
   void Pop();
  
  /// This is to be called by proccessing code to indicate progress so far.
  /// Indicates having done x work modules out of a total of y work modules to be
  /// done. Calling it with x==0 or x==y are optional and ushally ignored.
  /// Calling with x>y is bad. You can change y between calls at a different
  /// level, and it will adjust accordingly. This is likelly to piss
  /// of anyone who happens to be watching when the progress bar starts jumping
  /// arround like a madman. Increasing y is certainly to be avoided if possible.
  /// Safe when this is set to null.
   void Report(nat32 x,nat32 y);
   
  /// Move the incriments the number of work modules done by 1, a useful conveniance
  /// as the number of work moduels to be done itself ushally dosn't change so much.
   void Next();
  
   
  /// Returns the progress, factoring in all the avaliable information, a value
  /// between 0 and 1, where 0 indicates nothing accheived and 1 indicates task
  /// completion.
   real32 Prog();
     
  /// Returns how deep the stack has been Push -ed to. Returns 1 at the base
  /// level. Indicates the range of indexes given to Parts.
   nat32 Depth();
  
  /// Allows direct access to the x/y pairs that make up the current 
  /// progress report. Given an index, from 0 to Depth()-1 inclusive,
  /// outputs the x and y for that level.
   void Part(nat32 index,nat32 & xOut,nat32 & yOut);
  
  
  /// This outputs how long the algorithm has been running for so far and
  /// how much longer it is predicted to run for. Times use seconds as
  /// would be expected.
  /// remaining will be negative if no prediction can be given.
   void Time(real64 & done,real64 & remaining);
  
  
  /// If an algorithm is paused then this can be called to Stop the timer
  /// so the predicted time continues to make some sense. To impliment
  /// pausing ushally involves implimenting a child class and editting 
  /// the OnChange() method to sleep.
   void Pause();
  
  /// Whilst paused you might want to indicate that time has passed, as 
  /// strange as this sounds this has value if you are measuring progress
  /// that includes remote work, i.e. in another thread, process or machine.
   void AddTime(real64 amount);
   
  /// After pausing the timer this method will resume it.
   void Continue();
   
   
  /// This method is called every time Report(...) is called. Does nothing by
  /// default but a child class can use this to respond as needed. If stopping
  /// an algorithm is a requirement then making this method throw will accheive 
  /// that effect. Remember to impliment the algorithms such that memory leaks 
  /// will not occur if this is done.
   virtual void OnChange();
   
  
  /// &nbsp;
   static inline cstrconst TypeString(){return "eos::time::Progress";} 	


 private:
  bit paused;
  real64 time; // When pasued this is the total time, when un-paused this is the start time minus any time previous.
  
  nat32 size; // How large the array is.
  nat32 depth; // How deep in the stack we currently are.
  
  Pair<nat32,nat32> * data; // first is x, second is y.
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
// IO capability for the Progress class...

template <typename T>
inline T & StreamWrite(T & lhs,const Progress & rhs,Binary)
{
 real64 done;
 real64 remain;
 rhs.Time(done,remain);
 lhs << done;
 
 nat32 depth = rhs.Depth();
 lhs << depth;
 
 for (nat32 i=0;i<depth;i++)
 {
  nat32 x,y;
  rhs.Part(i,x,y);
  lhs << x << y;
 }
 
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,Progress & rhs,Binary)
{
 real64 done;
 lhs >> done;
 
 nat32 depth;
 lhs >> depth;
	
 rhs.Reset(depth,true);
 rhs.AddTime(done);
 
 for (nat32 i=0;i<depth;i++)
 {
  nat32 x,y;
  lhs >> x >> y;
  rhs.Report(x,y);
  if (i!=(depth-1)) rhs.Push();	 
 }

 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const Progress & rhs,Text)
{
 lhs << rhs.Prog()*100.0 << "%";
 return lhs;
}

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
#endif
