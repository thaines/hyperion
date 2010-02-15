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

#include "eos/stereo/sad_seg_stereo.h"

#include "eos/stereo/sad.h"
#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
SadSegStereo::SadSegStereo()
:minDisp(-30),maxDisp(30),maxDiff(1e2),maxRad(3),minSeg(0),out(null<svt::Var*>())
{}

SadSegStereo::~SadSegStereo()
{
 delete out;
}

void SadSegStereo::SetSegments(nat32 count,const svt::Field<nat32> & s)
{
 segments = count;
 segs = s;
}

void SadSegStereo::AddField(const svt::Field<real32> & left,const svt::Field<real32> & right)
{
 nat32 ind = in.Size();
 in.Size(ind+1);
 in[ind].first = left;
 in[ind].second = right;
}

void SadSegStereo::SetRange(int32 minD,int32 maxD)
{
 minDisp = minD;
 maxDisp = maxD;
}

void SadSegStereo::SetMaxAD(real32 mad)
{
 maxDiff = mad;
}

void SadSegStereo::SetMinCluster(nat32 th)
{
 minSeg = th;
}

void SadSegStereo::SetMaxRadius(nat32 mr)
{
 maxRad = mr;
}

void SadSegStereo::Run(time::Progress * prog)
{
 prog->Push();
  // Firstly create the output data structures, initialising 
  // them to default values...
   out = new svt::Var(in[0].first.GetVar()->GetCore());
    out->Setup2D(segs.Size(0),segs.Size(1));
    real32 dispIni = 0;
    out->Add("disp",dispIni);
    bit validIni = false; 
    out->Add("valid",validIni);
   out->Commit();


  // Setup the segment information data structure...
   sed.Size(segments);

   for (nat32 i=0;i<sed.Size();i++)
   {
    sed[i].min = maxDisp;
    sed[i].max = minDisp;
    sed[i].validCount = 0;
    sed[i].invalidCount = 0;
   }
   
   for (nat32 y=0;y<segs.Size(1);y++)
   {
    for (nat32 x=0;x<segs.Size(0);x++)
    {
     sed[segs.Get(x,y)].invalidCount += 1;
    }
   }
   
  // Set the minRange and maxRange values correctly.
   maxRange = math::Max(math::Abs(minDisp),math::Abs(maxDisp));
   minRange = -maxRange;

  // Then iterate through a pass for each window size, thats
  // all folks...
   for (nat32 i=1;i<=maxRad;i++)
   {
    prog->Report(i-1,maxRad);
    DoPass(prog,i);
   }
   prog->Report(maxRad,maxRad);

 prog->Pop();
}

void SadSegStereo::GetDisparity(svt::Field<real32> & disp)
{
 svt::Field<real32> temp;
 out->ByName("disp",temp);
 disp.CopyFrom(temp);
}

void SadSegStereo::GetMask(svt::Field<bit> & mask)
{
 svt::Field<bit> temp;
 out->ByName("valid",temp);
 mask.CopyFrom(temp);
}

//------------------------------------------------------------------------------
void SadSegStereo::DoPass(time::Progress * prog,nat32 radius)
{
 prog->Push();

 // Before we can begin we need data, and a lot of data at that, to store the 
 // dis-similarity measure...
  svt::Var diSim(out->GetCore());
   diSim.Setup3D(out->Size(0),out->Size(1),maxRange+1-minRange);
   real32 dsiIni = 0.0;
   diSim.Add("dsi",dsiIni);
  diSim.Commit(false);


 // Obtain all the fields required...
  svt::Field<real32> dsi; diSim.ByName("dsi",dsi);
  svt::Field<real32> disp; out->ByName("disp",disp);
  svt::Field<bit> valid; out->ByName("valid",valid);


 // We have to use sad with this window size to get a big fat set of 
 // dis-similarity measures...
  prog->Report(0,5);
  Sad sad;
   for (nat32 i=0;i<in.Size();i++) sad.AddField(in[i].first,in[i].second);
   sad.SetRadius(radius);
   sad.SetRange(minRange,maxRange);
   sad.SetMaxDiff(maxDiff);
   sad.SetOutput(dsi);

  sad.Run(prog);


 // For all invalid pixels in unreliable segments calculate a disparity and store
 // it if it passes the left-right consistancy check...
  prog->Report(1,5);
  for (nat32 y=radius;y<segs.Size(1)-radius;y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    if (valid.Get(x,y)) continue;
    nat32 s = segs.Get(x,y);
    if (sed[s].Valid()) continue;

    // Determine the best disparity...
     int32 sdi = minDisp;
     real32 best = dsi.Get(x,y,0);
     for (int32 i=minDisp+1-minRange;i<maxDisp+1-minRange;i++)
     {
      if (dsi.Get(x,y,i)<best)
      {
       sdi = i + minRange;
       best = dsi.Get(x,y,i);
      }
     }


    // Check no other dis-similaritys are basically the same,
    // i.e. we are guessing between them...
     bit badChoice = false;
     for (int32 i=minDisp+1-minRange;i<maxDisp+1-minRange;i++)
     {
      if (math::Equal(dsi.Get(x,y,i),best)&&(sdi!=(i+minRange))) {badChoice = true; break;}
     }
     if (badChoice) continue;


    // Perform left-right consistancy check...
     int32 start = math::Max<int32>(0,x+sdi-maxDisp);
     int32 end = math::Min<int32>(segs.Size(0)-1,x+sdi-minDisp);
     bit failed = false;
      for (int32 i=start;i<=end;i++)
      {
       if (dsi.Get(i,y,((x+sdi)-i)-minRange)<best) {failed = true; break;}
      }
     if (failed) continue;

    // Write the disparity and its validity out...
     valid.Get(x,y) = true;
     disp.Get(x,y) = sdi;
   }
  }


 // Reject isolated pixels, factor remaining pixels into there segment min/max
 // disparities and update validity of segments...
  prog->Report(2,5);
  KillIsolated(valid,disp);


 // For all invalid pixels in each reliable segment calculate a disparity within 
 // the reduced search range, store it if it passes the left-right consistancy
 // check...
  prog->Report(3,5);
  for (nat32 y=radius;y<segs.Size(1)-radius;y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    if (valid.Get(x,y)) continue;
    nat32 s = segs.Get(x,y);
    if (!sed[s].Valid()) continue;

    // Determine the best disparity...
     int32 sdi = sed[s].min;
     real32 best = dsi.Get(x,y,sdi-minRange);
     for (int32 i=sed[s].min+1-minRange;i<=sed[s].max-minRange;i++)
     {
      if (dsi.Get(x,y,i)<best)
      {
       sdi = i + minRange;
       best = dsi.Get(x,y,i);
      }
     }

    // Perform left-right consistancy check...
     int32 start = math::Max<int32>(0,x+sdi-sed[s].max);
     int32 end = math::Min<int32>(segs.Size(0)-1,x+sdi-sed[s].min);
     bit failed = false;
      for (int32 i=start;i<=end;i++)
      {    
       if (dsi.Get(i,y,((x+sdi)-i)-minRange)<best) {failed = true; break;}
      }
     if (failed) continue;

    // Write the disparity and its validity out...
     valid.Get(x,y) = true;
     disp.Get(x,y) = sdi;
   }
  }


 // Reject isolated pixels, factor remaining pixels into there segment min/max
 // disparities adn update segment validities...
  prog->Report(4,5);
  KillIsolated(valid,disp);

 prog->Pop();
}

void SadSegStereo::KillIsolated(svt::Field<bit> & valid,svt::Field<real32> & disp)
{
 // Set all valid/invalid counts to 0...
  for (nat32 i=0;i<sed.Size();i++) 
  {
   sed[i].validCount = 0;
   sed[i].invalidCount = 0;
  }
  
 // Construct a forest where every node is lonely...
  ds::Array2D<Node> forest(segs.Size(0),segs.Size(1));
  for (nat32 y=0;y<forest.Height();y++)
  {
   for (nat32 x=0;x<forest.Width();x++)
   {
    forest.Get(x,y).parent = null<Node*>();
    forest.Get(x,y).size = 1;
   }
  }

 // Do a single pass and merge all nodes that are applicable...
 // (We use 4 way connectivity.)
  for (nat32 y=0;y<forest.Height();y++)
  {
   for (nat32 x=0;x<forest.Width();x++)
   {
    if (valid.Get(x,y)==false) continue;

    // Check across...
     if ((x!=forest.Width()-1)&&valid.Get(x+1,y)&&(math::Equal(disp.Get(x,y),disp.Get(x+1,y))))
     {
      Node * a = forest.Get(x,y).Head();
      Node * b = forest.Get(x+1,y).Head();
      if (a!=b)
      {
       b->parent = a;
       a->size += b->size;
      }
     }

    // Check down...
     if ((y!=forest.Height()-1)&&valid.Get(x,y+1)&&(math::Equal(disp.Get(x,y),disp.Get(x,y+1))))
     {
      Node * a = forest.Get(x,y).Head();
      Node * b = forest.Get(x,y+1).Head();
      if (a!=b)
      {
       b->parent = a;
       a->size += b->size;
      }
     }
   }
  }

 // Do a second pass and destroy all entrys that belong to small clusters...
  for (nat32 y=0;y<forest.Height()-1;y++)
  {
   for (nat32 x=0;x<forest.Width()-1;x++)
   {
    Seg * t = &sed[segs.Get(x,y)];
    if (valid.Get(x,y)==false)
    {
     t->invalidCount += 1;
     continue;
    }
    
    if (forest.Get(x,y).Head()->size<minSeg)
    {
     valid.Get(x,y) = false;
     t->invalidCount += 1;
    }
    else
    {
     // Its a perfectly valid pixel, and will remain so for an eternity, nows
     // the time to factor it into its segments min/max disparity...
      t->min = math::Max(math::Min(t->min,int32(disp.Get(x,y))-1),minDisp);
      t->max = math::Min(math::Max(t->max,int32(disp.Get(x,y))+1),maxDisp);
      t->validCount += 1;      
    }
   }
  }
}

//------------------------------------------------------------------------------
 };
};
