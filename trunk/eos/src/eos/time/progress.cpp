//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines
#include "eos/time/progress.h"

#include "eos/mem/alloc.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace time
 {
//------------------------------------------------------------------------------
Progress::Progress(nat32 d,bit sp)
:paused(false),time(0.0),size(0),depth(0),data(null< Pair<nat32,nat32>* >())
{
 Reset(d,sp);	
}

Progress::~Progress()
{
 delete[] data;	
}

void Progress::Reset(nat32 d,bit startPaused)
{
 if (size<d)
 {
  delete[] data;
  data = new Pair<nat32,nat32>[d];	 
 }
 
 data[0].first = 0;
 data[0].second = 1;
 
 depth = 0;
 paused = startPaused;
 if (paused) time = 0.0;
        else time = UltraTime();	
}

void Progress::Push()
{
 if (this==null<Progress*>()) return;
 ++depth;
 
 if (depth>=size)
 {
  const nat32 sizeInc = 8;
  	 
  Pair<nat32,nat32> * newData = new Pair<nat32,nat32>[size + sizeInc];
  for (nat32 i=0;i<depth;i++) newData[i] = data[i];
  delete[] data;
  data = newData;
  size += sizeInc;	 
 }
 
 data[depth].first = 0;	
 data[depth].second = 1;
}

void Progress::Pop()
{
 if (this==null<Progress*>()) return;
 --depth;
}

void Progress::Report(nat32 x,nat32 y)
{
 if (this==null<Progress*>()) return;
 log::Assert((x<=y)&&(y!=0));
 data[depth].first  = x;	
 data[depth].second = y;
 OnChange();
}

void Progress::Next()
{
 if (this==null<Progress*>()) return;
 data[depth].first += 1;
 OnChange();
}

real32 Progress::Prog()
{
 real32 ret = 0.0;
 real32 fact = 1.0;
 for (nat32 i=0;i<=depth;i++)
 {
  if (data[i].second!=0)
  {
   ret += fact * real32(data[i].first)/real32(data[i].second);
   fact *= 1.0/real32(data[i].second);
  }
 }
 return ret;	
}

nat32 Progress::Depth()
{
 return depth+1;	
}

void Progress::Part(nat32 index,nat32 & xOut,nat32 & yOut)
{
 xOut = data[index].first;	
 yOut = data[index].second;
}

void Progress::Time(real64 & done,real64 & remaining)
{
 if (paused) done = time; 
        else done = UltraTime()-time;	 
 
 real32 prog = Prog();
 remaining = done*(1.0-prog);	
}

void Progress::Pause()
{
 if (!paused)
 {
  time = UltraTime()-time;
  paused = true;
 }	
}

void Progress::AddTime(real64 amount)
{
 if (paused) time += amount;	 
        else time -= amount; 	
}

void Progress::Continue()
{
 if (paused)
 {
  time = UltraTime()-time;
  paused = false;	 
 }	
}

void Progress::OnChange()
{}

//------------------------------------------------------------------------------
 };
};
