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

#include "eos/filter/matching.h"

#include "eos/filter/conversion.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void MatchNCC(const svt::Field<real32> & imgA,const ds::Array<Corner> & corA,
                       const svt::Field<real32> & imgB,const ds::Array<Corner> & corB,
                       svt::Field<real32> & out,nat32 half,bit addative,
                       time::Progress * prog)
{
 LogBlock("eos::filter::MatchNCC(real32)","-");
 prog->Push();
 
 nat32 winSize = math::Sqr(half*2+1);
 real64 meanMult = 1.0/real64(winSize);

 // Create a tempory structure for image A, containning the window for each corner
 // so as to improve cache coherance, with the mean allready subtracted.
 // Indicate entrys that are too close to the edge with a flag...
  ds::Array<bit> validA(corA.Size());
  ds::Array<real32> cacheA(corA.Size()*winSize);
  for (nat32 i=0;i<corA.Size();i++)
  {
   nat32 lx = nat32(corA[i].loc[0]);
   nat32 ly = nat32(corA[i].loc[1]);
   validA[i] = (lx>=half)&&((lx+half)<imgA.Size(0))&&
               (ly>=half)&&((ly+half)<imgA.Size(1));
   if (validA[i])
   {
    real64 mean = 0.0;
    nat32 offset = i*winSize;
    for (nat32 y=ly-half;y<=ly+half;y++)
    {
     for (nat32 x=lx-half;x<=lx+half;x++)
     {
      real32 val = imgA.Get(x,y);
      mean += val;
      cacheA[offset] = val;
      ++offset;
     }
    }
    
    mean *= meanMult;
    offset = i*winSize;
    for (nat32 j=0;j<winSize;j++)
    {
     cacheA[offset] -= mean;
     ++offset;	    
    }
   }
  }


 // Same as above, for image B...
  ds::Array<bit> validB(corB.Size());
  ds::Array<real32> cacheB(corB.Size()*winSize);
  for (nat32 i=0;i<corB.Size();i++)
  {
   nat32 lx = nat32(corB[i].loc[0]);
   nat32 ly = nat32(corB[i].loc[1]);
   validB[i] = (lx>=half)&&((lx+half)<imgB.Size(0))&&
               (ly>=half)&&((ly+half)<imgB.Size(1));
   if (validB[i])
   {
    real64 mean = 0.0;
    nat32 offset = i*winSize;
    for (nat32 y=ly-half;y<=ly+half;y++)
    {
     for (nat32 x=lx-half;x<=lx+half;x++)
     {
      real32 val = imgB.Get(x,y);
      mean += val;
      cacheB[offset] = val;
      ++offset;
     }
    }
    
    mean *= meanMult;
    offset = i*winSize;
    for (nat32 j=0;j<winSize;j++)
    {
     cacheB[offset] -= mean;
     ++offset;	    
    }
   }
  }


 // Loop all corner pairings...
  for (nat32 v=0;v<corB.Size();v++)
  {
   prog->Report(v,corB.Size());
   if (validB[v])
   {
    // Coordinate in bounds - get ncc'ing...
     for (nat32 u=0;u<corA.Size();u++)
     {
      if (validA[u])
      {
       // No boundary issues - we have to do the ncc...
        // Calculate parts of the final sum...
         real64 top = 0.0;
         real64 div1 = 0.0;
         real64 div2 = 0.0;
         nat32 baseA = winSize*u;
         nat32 baseB = winSize*v;
         for (nat32 t=0;t<winSize;t++)
         {
          real64 vA = cacheA[baseA+t];
          real64 vB = cacheB[baseB+t];
           
          top += vA*vB;
          div1 += math::Sqr(vA);
          div2 += math::Sqr(vB);
         }
       
        // Now calculate the final ncc...
         real32 ncc;
         real64 div = div1*div2;
         if (math::IsZero(div)) ncc = 0.0;
                           else ncc = top*math::InvSqrt(div);
       
        // Store it...
         if (addative) out.Get(u,v) *= ncc;
                  else out.Get(u,v) = ncc;
      }
      else
      {
       out.Get(u,v) = 0.0;	      
      }
     }
   }
   else
   {
    for (nat32 u=0;u<corA.Size();u++) out.Get(u,v) = 0.0;	   
   }
  }
 prog->Pop();
}
                       
//------------------------------------------------------------------------------
EOS_FUNC void MatchNCC(const svt::Field<bs::ColourRGB> & imgA,const ds::Array<Corner> & corA,
                       const svt::Field<bs::ColourRGB> & imgB,const ds::Array<Corner> & corB,
                       svt::Field<real32> & out,nat32 half,
                       time::Progress * prog)
{
 LogBlock("eos::filter::MatchNCC(bs::ColourRGB)","-");
 prog->Push();
  svt::Field<real32> redA;   imgA.SubField(0*sizeof(real32),redA);
  svt::Field<real32> greenA; imgA.SubField(1*sizeof(real32),greenA);
  svt::Field<real32> blueA;  imgA.SubField(2*sizeof(real32),blueA);
 
  svt::Field<real32> redB;   imgB.SubField(0*sizeof(real32),redB);
  svt::Field<real32> greenB; imgB.SubField(1*sizeof(real32),greenB);
  svt::Field<real32> blueB;  imgB.SubField(2*sizeof(real32),blueB);
 
  prog->Report(0,3); MatchNCC(redA,corA,redB,corB,out,half,false,prog);
  prog->Report(1,3); MatchNCC(greenA,corA,greenB,corB,out,half,true,prog);
  prog->Report(2,3); MatchNCC(blueA,corA,blueB,corB,out,half,true,prog);
 prog->Pop();
}

//------------------------------------------------------------------------------
EOS_FUNC void MatchSelect(const svt::Field<real32> & simMat,ds::Array<bs::Pos> & out,real32 ratio)
{
 LogBlock("eos::filter::MatchSelect","-");
 // Set the out array to the maximum possible size, we can shrink it latter...
  out.Size(math::Min(simMat.Size(0),simMat.Size(1)));
  nat32 size = 0;


 // Iterate the matrix in one dimension and start checking, as each 
 // row/column can only be involved in one match...
  for (nat32 y=0;y<simMat.Size(1);y++)
  {
   // Find the x indexes of the highest and second highest scores...
    nat32 bx[2];
    if (simMat.Get(0,y)<simMat.Get(1,y)) {bx[0] = 1; bx[1] = 0;}
                                    else {bx[0] = 0; bx[1] = 1;}

    for (nat32 x=2;x<simMat.Size(0);x++)
    {
     if (simMat.Get(x,y)>simMat.Get(bx[1],y))
     {
      if (simMat.Get(x,y)>simMat.Get(bx[0],y))
      {
       bx[1] = bx[0];
       bx[0] = x;
      }
      else
      {
       bx[1] = x;
      }
     }
    }


   // Do the ratio test, if it fails move on...
    if ((simMat.Get(bx[0],y)*ratio)<simMat.Get(bx[1],y)) continue;


   // Find the highest and second highest score in the other direction
   // for the highest scoring position...
    nat32 by[2];
    if (simMat.Get(bx[0],0)<simMat.Get(bx[0],1)) {by[0] = 1; by[1] = 0;}
                                            else {by[0] = 0; by[1] = 1;}

    for (nat32 y2=2;y2<simMat.Size(1);y2++)
    {
     if (simMat.Get(bx[0],y2)>simMat.Get(bx[0],by[1]))
     {
      if (simMat.Get(bx[0],y2)>simMat.Get(bx[0],by[0]))
      {
       by[1] = by[0];
       by[0] = y2;
      }
      else
      {
       by[1] = y2;
      }
     }
    }


   // Check the highest position matches and the ratio test passes, if either fail move on...
    if (by[0]!=y) continue;
    if ((simMat.Get(bx[0],y)*ratio)<simMat.Get(bx[0],by[1])) continue;


   // We have a usable match - store it...
    out[size][0] = bx[0];
    out[size][1] = y;
    ++size;
  }


 // Shrink the output array to snuggly fit arround the discovered matches...
  out.Size(size);
}

//------------------------------------------------------------------------------
EOS_FUNC void MatchImages(const svt::Field<bs::ColourRGB> & imgA,
                          const svt::Field<bs::ColourRGB> & imgB,
                          ds::Array<Pair<bs::Pnt,bs::Pnt> > & out,
                          nat32 maxMatches,nat32 half,real32 ratio,
                          time::Progress * prog)
{
 LogBlock("eos::filter::MatchImages","-");
 prog->Push();
  // Create grey scale versions for corner detection...
   prog->Report(0,6);
   real32 realIni = 0.0;   

   svt::Var gsVarA(imgA);
    gsVarA.Add("l",realIni);
   gsVarA.Commit();
   
   svt::Field<bs::ColourL> imgAlum(&gsVarA,"l");
   svt::Field<real32> imgAgrey(&gsVarA,"l");
   RGBtoL(imgA,imgAlum);

   svt::Var gsVarB(imgB);
    gsVarB.Add("l",realIni);
   gsVarB.Commit();
   
   svt::Field<bs::ColourL> imgBlum(&gsVarB,"l");
   svt::Field<real32> imgBgrey(&gsVarB,"l");
   RGBtoL(imgB,imgBlum);    


  // Generate corners...
   // First image...
    prog->Report(1,6);
    ds::Array<Corner> corA;
    CornerHarris(imgAgrey,maxMatches,corA,prog);
    

   // Second image...
    prog->Report(2,6);
    ds::Array<Corner> corB;
    CornerHarris(imgBgrey,maxMatches,corB,prog);


  // Apply NCC to get similarity matrix...
   prog->Report(3,6);
   svt::Var simMatVar(imgA);
    simMatVar.Setup2D(corA.Size(),corB.Size());
    simMatVar.Add("sim",realIni);
   simMatVar.Commit(false);
   
   svt::Field<real32> simMat(&simMatVar,"sim");
   MatchNCC(imgA,corA,imgB,corB,simMat,half,prog);


  // Select the best matches...
   prog->Report(4,6);
   ds::Array<bs::Pos> simList;
   MatchSelect(simMat,simList,ratio);


  // Convert them into the output format required...
   prog->Report(5,6);
   out.Size(simList.Size());
   for (nat32 i=0;i<out.Size();i++)
   {
    out[i].first = corA[simList[i][0]].loc;
    out[i].second = corB[simList[i][1]].loc;
   }


 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
