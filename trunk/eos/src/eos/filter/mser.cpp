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

#include "eos/filter/mser.h"

#include "eos/ds/lists.h"
#include "eos/rend/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
Mser::Mser()
:minArea(32),maxAreaMult(0.1),delta(32)
{}

Mser::~Mser()
{}

void Mser::Set(nat32 d,nat32 minA,real32 maxAM)
{
 delta = d;	
 minArea = minA;
 maxAreaMult = maxAM;
}

void Mser::Set(const svt::Field<bs::ColourL> & im)
{
 img = im;
}

void Mser::Run(time::Progress * prog)
{
 LogBlock("void eos::filter::Mser::Run()","-");
 prog->Push();


 // Data structures used in both steps...
  prog->Report(0,4);
  ds::Array2D<Turnip> forest(img.Size(0),img.Size(1));
  ds::List<Node> store;
  
 
 // Set the maximum mser area from image size...
  maxArea = nat32(real32(img.Size(0)*img.Size(1))*maxAreaMult);


 // We use a bucket sort on the assumption of 256 colours...
 // Simultaneously initialise some values in the data structure, the rest are done latter.
  Turnip * bucket[256];
  for (nat32 i=0;i<256;i++) bucket[i] = null<Turnip*>();
  prog->Push();
   for (nat32 y=0;y<img.Size(1);y++)
   {
    prog->Report(y,img.Size(1));
    for (nat32 x=0;x<img.Size(0);x++)
    {
     Turnip & targ = forest.Get(x,y);

     targ.x = x;
     targ.y = y;
     targ.l = img.Get(x,y).l;
     targ.bucket = nat32(math::Round(targ.l*255.0));
     log::Assert((targ.bucket>=0)&&(targ.bucket<=255));
     
     targ.next = bucket[targ.bucket];
     bucket[targ.bucket] = &targ;
     
     targ.area = 0;
    }
   }
  prog->Pop();


 // Go from dark to light...
  prog->Report(1,4);
  prog->Push();
   for (nat32 i=0;i<256;i++)
   {
    prog->Report(i,256);
    DoList(forest,bucket[i],store,false);
   }   
   Analyse(*forest.Get(0,0).Head(),store,false); // We don't want to miss the longest chain.
  prog->Pop();


 // Null all the area values, to effectivly start again...
  for (nat32 y=0;y<img.Size(1);y++)
  {
   for (nat32 x=0;x<img.Size(0);x++)
   {
    forest.Get(x,y).area = 0;
   }
  }
  
  
 // Go from light to dark...
  prog->Report(2,4);
  prog->Push();
   prog->Report(0,257);
   for (nat32 i=0;i<256;i++)
   {
    prog->Report(i,256);
    DoList(forest,bucket[255-i],store,true);
   }
   Analyse(*forest.Get(0,0).Head(),store,true); // We don't want to miss the longest chain.
  prog->Pop();


 // Move the store linked list into the data array...
  prog->Report(3,4);
  data.Size(store.Size());
  for (nat32 i=0;i<data.Size();i++)
  {
   data[i] = store.Front();
   store.RemFrontKill();
  }


 prog->Pop();        
}

nat32 Mser::Size() const
{
 return data.Size();        
}

bit Mser::Maximal(nat32 i) const
{
 return data[i].maximal;        
}

const bs::Pos & Mser::Pixel(nat32 i) const
{
 return data[i].pixel;        
}

real32 Mser::Threshold(nat32 i) const
{
 return data[i].threshold;        
}

nat32 Mser::Area(nat32 i) const
{
 return data[i].area;
}

const bs::Pnt & Mser::Centre(nat32 i) const
{
 return data[i].centre;        
}

const math::Mat<2> & Mser::Covariance(nat32 i) const
{
 return data[i].covariance;        
}

const math::Mat<2> & Mser::Affine(nat32 i) const
{
 return data[i].affine;        
}

void Mser::VisualiseRegions(svt::Field<bs::ColourRGB> & out) const
{
 // Create the data structure, both a queue and an indication of allready visited nodes...
  ds::Array2D<Cabbage> field(img.Size(0),img.Size(1));
  Cabbage * top;
  Cabbage * last;
  for (nat32 y=0;y<field.Height();y++)
  {
   for (nat32 x=0;x<field.Width();x++)
   {
    field.Get(x,y).x = x;
    field.Get(x,y).y = y;
    field.Get(x,y).ind = nat32(-1);
   }
  }


 // Iterate all mser's, rendering 'em one at a time using the dta structure to
 // efficiently outline the fills...
  for (nat32 i=0;i<data.Size();i++)
  {
   top = &field.Get(data[i].pixel[0],data[i].pixel[1]);
   top->ind = i;
   last = top;
   last->next = null<Cabbage*>();
   while (top)
   {
    if ((top->x!=0)&&(field.Get(top->x-1,top->y).ind!=i))
    {
     field.Get(top->x-1,top->y).ind = i;
     real32 gl = img.Get(top->x-1,top->y).l;
     if ((( data[i].maximal)&&(gl>=data[i].threshold))||
         ((!data[i].maximal)&&(gl<=data[i].threshold)))
     {
      last->next = &field.Get(top->x-1,top->y);
      last = last->next;
      last->next = null<Cabbage*>();
     }
     else
     {
      out.Get(top->x-1,top->y) = bs::ColourRGB(1.0,1.0,1.0);
     }
    }

    if ((top->x!=img.Size(0)-1)&&(field.Get(top->x+1,top->y).ind!=i))
    {
     field.Get(top->x+1,top->y).ind = i;
     real32 gl = img.Get(top->x+1,top->y).l;
     if ((( data[i].maximal)&&(gl>=data[i].threshold))||
         ((!data[i].maximal)&&(gl<=data[i].threshold)))
     {
      last->next = &field.Get(top->x+1,top->y);
      last = last->next;
      last->next = null<Cabbage*>();
     }
     else
     {
      out.Get(top->x+1,top->y) = bs::ColourRGB(1.0,1.0,1.0);
     }
    }

    if ((top->y!=0)&&(field.Get(top->x,top->y-1).ind!=i))
    {
     field.Get(top->x,top->y-1).ind = i;
     real32 gl = img.Get(top->x,top->y-1).l;
     if ((( data[i].maximal)&&(gl>=data[i].threshold))||
         ((!data[i].maximal)&&(gl<=data[i].threshold)))
     {
      last->next = &field.Get(top->x,top->y-1);
      last = last->next;
      last->next = null<Cabbage*>();
     }
     else
     {
      out.Get(top->x,top->y-1) = bs::ColourRGB(1.0,1.0,1.0);
     }
    }

    if ((top->y!=img.Size(1)-1)&&(field.Get(top->x,top->y+1).ind!=i))
    {
     field.Get(top->x,top->y+1).ind = i;
     real32 gl = img.Get(top->x,top->y+1).l;
     if ((( data[i].maximal)&&(gl>=data[i].threshold))||
         ((!data[i].maximal)&&(gl<=data[i].threshold)))
     {
      last->next = &field.Get(top->x,top->y+1);
      last = last->next;
      last->next = null<Cabbage*>();
     }
     else
     {
      out.Get(top->x,top->y+1) = bs::ColourRGB(1.0,1.0,1.0);
     }
    }
    
    top = top->next;
   }
  }
}

void Mser::VisualiseFrames(svt::Field<bs::ColourRGB> & out) const
{
 // The points we need...
  bs::Pnt ce;   
  bs::Pnt ic[2][2];
  bs::Pnt oc[2][2];
   
  ce[0] = 0.0;
  ce[1] = 3.0;
   
  ic[0][0][0] = -1.0;
  ic[0][0][1] = -1.0;
  ic[0][1][0] = -1.0;
  ic[0][1][1] =  1.0;
  ic[1][0][0] =  1.0;
  ic[1][0][1] = -1.0;
  ic[1][1][0] =  1.0;
  ic[1][1][1] =  1.0;
   
  oc[0][0][0] = -2.0;
  oc[0][0][1] = -2.0;
  oc[0][1][0] = -2.0;
  oc[0][1][1] =  3.0;
  oc[1][0][0] =  3.0;
  oc[1][0][1] = -2.0;
  oc[1][1][0] =  3.0;
  oc[1][1][1] =  3.0;

 // Loop 'em all...	
  for (nat32 i=0;i<Size();i++)
  {
   // Transform the points...
    bs::Pnt tce;
    bs::Pnt tic[2][2];
    bs::Pnt toc[2][2];
    
    math::MultVect(Affine(i),ce,tce); tce += Centre(i);
    math::MultVect(Affine(i),ic[0][0],tic[0][0]); tic[0][0] += Centre(i);
    math::MultVect(Affine(i),ic[0][1],tic[0][1]); tic[0][1] += Centre(i);
    math::MultVect(Affine(i),ic[1][0],tic[1][0]); tic[1][0] += Centre(i);
    math::MultVect(Affine(i),ic[1][1],tic[1][1]); tic[1][1] += Centre(i);
    math::MultVect(Affine(i),oc[0][0],toc[0][0]); toc[0][0] += Centre(i);
    math::MultVect(Affine(i),oc[0][1],toc[0][1]); toc[0][1] += Centre(i);
    math::MultVect(Affine(i),oc[1][0],toc[1][0]); toc[1][0] += Centre(i);
    math::MultVect(Affine(i),oc[1][1],toc[1][1]); toc[1][1] += Centre(i);

   // Draw the lines...
    rend::Line(out,Centre(i),tce,bs::ColourRGB(1.0,1.0,1.0));
    
    rend::Line(out,tic[0][0],tic[0][1],bs::ColourRGB(1.0,0.0,1.0));
    rend::Line(out,tic[1][1],tic[0][1],bs::ColourRGB(1.0,0.0,1.0));
    rend::Line(out,tic[1][1],tic[1][0],bs::ColourRGB(1.0,0.0,1.0));
    rend::Line(out,tic[0][0],tic[1][0],bs::ColourRGB(1.0,0.0,1.0));
    
    rend::Line(out,toc[0][0],toc[0][1],bs::ColourRGB(0.0,1.0,1.0));
    rend::Line(out,toc[1][1],toc[0][1],bs::ColourRGB(0.0,1.0,1.0));
    rend::Line(out,toc[1][1],toc[1][0],bs::ColourRGB(0.0,1.0,1.0));
    rend::Line(out,toc[0][0],toc[1][0],bs::ColourRGB(0.0,1.0,1.0));    
  }	
}

bs::Element * Mser::AsXML(str::TokenTable & tt) const
{
 bs::Element * ret = new bs::Element(tt,"features");

 for (nat32 i=0;i<Size();i++)
 {
  bs::Element * targ = ret->NewChild("mser");

  targ->SetAttribute("maximal",Maximal(i));

  str::String pos;
  pos << Pixel(i);
  targ->SetAttribute("pos",pos);

  targ->SetAttribute("threshold",Threshold(i));

  targ->SetAttribute("area",int32(Area(i)));

  str::String centre;
  centre << Centre(i);
  targ->SetAttribute("centre",centre);

  str::String covariance;
  covariance << Covariance(i);
  targ->SetAttribute("covariance",covariance);

  str::String affine;
  affine << Affine(i);
  targ->SetAttribute("affine",affine);    
 }

 return ret;
}

void Mser::DoList(ds::Array2D<Turnip> & forest,Turnip * targ,ds::List<Node> & store,bit dir)
{
 // Loop all the nodes in the list...
  while (targ)
  {
   // Run the turnip through the forests initiation rite - give it area!..
    targ->parent = null<Turnip*>();
    targ->previous = null<Turnip*>();
    targ->nextOther = targ;
    targ->prevOther = targ;
    targ->area = 1;


   // Check the (possibly) 4 neighbours, merge with 'em as needed...
    if (targ->x!=0)
    {
     Turnip & other = forest.Get(targ->x-1,targ->y);
     if (other.area!=0) Merge(*targ,other,store,dir);
    }

    if (targ->x!=forest.Width()-1)
    {
     Turnip & other = forest.Get(targ->x+1,targ->y);
     if (other.area!=0) Merge(*targ,other,store,dir);
    }
  
    if (targ->y!=0)
    {
     Turnip & other = forest.Get(targ->x,targ->y-1);
     if (other.area!=0) Merge(*targ,other,store,dir);
    }
  
    if (targ->y!=forest.Height()-1)
    {
     Turnip & other = forest.Get(targ->x,targ->y+1);
     if (other.area!=0) Merge(*targ,other,store,dir);
    }
   
   // To next... 
    targ = targ->next;
  }
}

void Mser::Merge(Turnip & ta,Turnip & tb,ds::List<Node> & store,bit dir)
{
 // Get heads for the two clusters...
  Turnip * ha = ta.Head();
  Turnip * hb = tb.Head();
  if (ha==hb) return;


 // Decide which node has the smallest cluster, and run the Collect method on 
 // that path...
  if (ha->area>hb->area) mem::PtrSwap(ha,hb);
  Analyse(*ha,store,dir);
  
 
 // Arrange for ha->previous to be null, so we can safely overwrite it below...
  ha->Collapse();


 // Make the smallest cluster the largest clusters bitch...
  ha->previous = hb;
  hb->parent = ha;
  ha->area += hb->area;
 
 // Compress the ha path, makes latter code faster and removes the possibility 
 // of a stack overflow...
  ha->Compress();
}

void Mser::Analyse(Turnip & head,ds::List<Node> & store,bit dir)
{
 // Check for a situation where we can't do jack, i.e. there is absolutly 
 // no chance of us finding a valid mser, if so return...
  nat32 minRange = delta*2 + 3;
  if ((head.area<minArea)||
      ((!dir)&&(head.bucket<=minRange))||
      ((dir)&&(255-head.bucket<=minRange))) return;


 // Area chart - we first fill it in then check for minima 
 // with a rolling window...
 // Have to include the relevent turnip so we can store the correct one when found.
  nat32 area[256];
  Turnip * node[256];

  
 // Fill in the area chart and set ranges...
  nat32 startBucket;
  nat32 endBucket;
  if (dir)
  {
   // Maximal...	  
    Turnip * targ = &head;
    nat32 bucket = targ->bucket;

    startBucket = bucket+1; // Skip first - liable to be flawed.
    endBucket = 255;

    while (targ)
    {
     if (targ->bucket!=bucket)
     {
      while (bucket<targ->bucket)
      {
       ++bucket;
       area[bucket] = targ->area;
       node[bucket] = targ;
      }
     }
     targ = targ->previous;	    
    }
    if (bucket==head.bucket) return;

    while (bucket<255)
    {
     area[bucket+1] = area[bucket];
     node[bucket+1] = node[bucket];     
     ++bucket;
    }
  }
  else
  {
   // Minimal...
    Turnip * targ = &head;
    nat32 bucket = targ->bucket;

    startBucket = 0;
    endBucket = bucket-1; // Skip last - liable to be flawed.

    while (targ)
    {
     if (targ->bucket!=bucket)
     {
      while (bucket>targ->bucket)
      {
       --bucket;
       area[bucket] = targ->area;
       node[bucket] = targ;
      }
     }
     targ = targ->previous;	    
    }
    if (bucket==head.bucket) return;

    while (bucket!=0)
    {
     area[bucket-1] = area[bucket];
     node[bucket-1] = node[bucket];
     --bucket;
    }
  }

 // Iterate the given range, extract any minimas and send them on to the store method...
 // Sliding window alert.
  real32 window[3];
  for (nat32 i=0;i<3;i++) window[i] = 0.0;

  startBucket += delta;
  endBucket -= delta;
  for (nat32 i=startBucket;i<=endBucket;i++)
  {
   window[0] = window[1];
   window[1] = window[2];
   window[2] = math::Abs(real32(area[i+delta])-real32(area[i-delta]))/real32(area[i]);
   if ((window[1]<window[0])&&
       (window[1]<window[2])&&
       (area[i-1]>=minArea)&&(area[i-1]<=maxArea)) Store(*node[i-1],store,dir);
  }
}

void Mser::Store(const Turnip & head,ds::List<Node> & store,bit dir)
{
 LogTime("eos::filter::Mser::Store");
 Node node;


 // Basics...
  node.maximal = dir;
  node.pixel[0] = head.x;
  node.pixel[1] = head.y;
  node.area = head.area;


 // Calculate mean and threshold in a single pass...
  nat32 n = 0;
  node.centre[0] = 0.0;
  node.centre[1] = 0.0;
  real32 sumX = 0.0;
  real32 sumY = 0.0;
  node.threshold = head.l;
  
  head.CalcTolMean(dir,node.threshold,n,node.centre[0],node.centre[1],sumX,sumY);
  
  if (n)
  {
   node.centre[0] += sumX/real32(n);	  
   node.centre[1] += sumY/real32(n);
  }


 // Calculate covariance in a second pass...  
  node.covariance[0][0] = 0.0;
  node.covariance[0][1] = 0.0;
  node.covariance[1][1] = 0.0;
  head.CalcCovar(node.centre,node.covariance);
  node.covariance[0][0] /= real32(node.area);
  node.covariance[0][1] /= real32(node.area);
  node.covariance[1][1] /= real32(node.area);
  node.covariance[1][0] = node.covariance[0][1];
  


 // Generate an affine frame...
  // Create a transformation upto an unknown rotation...
   math::Mat<2> temp;
   math::Mat<2> affineInv = node.covariance;
   math::Inverse(affineInv,temp);
   math::Sqrt22(affineInv);
   node.affine = affineInv;
   math::Inverse(node.affine,temp);

   
  // In a third pass find the furthest point, under the affine frame...
   real32 best = 0.0;
   bs::Pnt furthest;
   head.CalcExt(node.centre,affineInv,best,furthest);

  // Rotate (A Givens rotation is used.)...
   math::Mat<2> rot;
   real32 c;
   real32 s;
   if (math::IsZero(furthest[1]))
   {
    c = 1.0;
    s = 0.0;	     
   }
   else
   {
    if (math::Abs(furthest[1])>math::Abs(furthest[0]))
    {
     real32 r = -furthest[0]/furthest[1];
     s = math::InvSqrt(1.0+math::Sqr(r));
     c = s*r;
    }
    else
    {
     real32 r = -furthest[1]/furthest[0];
     c = math::InvSqrt(1.0+math::Sqr(r));
     s = c*r;
    }
   }
     
   rot[0][0] = c;  rot[0][1] = s;
   rot[1][0] = -s; rot[1][1] = c;
   math::Mult(node.affine,rot,temp);
   node.affine = temp;


 // Store...
  store.AddBack(node);
}

//------------------------------------------------------------------------------
MserKeys::MserKeys()
{}
   
MserKeys::~MserKeys()
{}

void MserKeys::Set(const svt::Field<bs::ColourRGB> & im)
{
 img = im;	
}

void MserKeys::Run(time::Progress * prog)
{
 LogBlock("void eos::filter::MserKeys::Run(time::Progress * prog)","-");
 prog->Push();
  prog->Report(0,2);
   Mser::Run(prog);
  prog->Report(1,2);
   prog->Push();

    key.Size(Size());   
    for (nat32 i=0;i<Size();i++)
    {
     prog->Report(i,Size());
     MserKey & targ = key[i];
     targ.index = i;
     
     // Iterate and grab each pixel...
      for (nat32 y=0;y<21;y++)
      {
       for (nat32 x=0;x<21;x++)
       {
	// Get pixel position...
	 bs::Pnt bp;
	 bs::Pnt pp;
	 
	 bp[0] = -2.0 + (x*5.0/20.0);
	 bp[1] = -2.0 + (y*5.0/20.0);
	 math::MultVect(Affine(i),bp,pp);
	 pp += Centre(i);

	// Clamp to the 4 surrounding pixels, grab the mixing ratios...
	 bs::Pos pix[2][2];
	 pix[0][0][0] = math::Clamp<int32>(int32(math::RoundDown(pp[0])),0,img.Size(0)-1);
	 pix[0][0][1] = math::Clamp<int32>(int32(math::RoundDown(pp[1])),0,img.Size(1)-1);
	 pix[0][1][0] = math::Clamp<int32>(int32(math::RoundDown(pp[0])),0,img.Size(0)-1);
	 pix[0][1][1] = math::Clamp<int32>(int32(math::RoundUp(pp[1])),0,img.Size(1)-1);
	 pix[1][0][0] = math::Clamp<int32>(int32(math::RoundUp(pp[0])),0,img.Size(0)-1);
	 pix[1][0][1] = math::Clamp<int32>(int32(math::RoundDown(pp[1])),0,img.Size(1)-1);
	 pix[1][1][0] = math::Clamp<int32>(int32(math::RoundUp(pp[0])),0,img.Size(0)-1);
	 pix[1][1][1] = math::Clamp<int32>(int32(math::RoundUp(pp[1])),0,img.Size(1)-1);

	 bs::Pnt t;
	 t[0] = math::Mod1(pp[0]);
	 t[1] = math::Mod1(pp[1]);

	// Sum to get final colour...
	 bs::ColourRGB final;
	  final.r = (1.0-t[0])*(1.0-t[1])*img.Get(pix[0][0][0],pix[0][0][1]).r +
	            (1.0-t[0])*t[1]*img.Get(pix[0][1][0],pix[0][1][1]).r +
	            t[0]*(1.0-t[1])*img.Get(pix[1][0][0],pix[1][0][1]).r +
	            t[0]*t[1]*img.Get(pix[1][1][0],pix[1][1][1]).r;
	  final.g = (1.0-t[0])*(1.0-t[1])*img.Get(pix[0][0][0],pix[0][0][1]).g +
	            (1.0-t[0])*t[1]*img.Get(pix[0][1][0],pix[0][1][1]).g +
	            t[0]*(1.0-t[1])*img.Get(pix[1][0][0],pix[1][0][1]).g +
	            t[0]*t[1]*img.Get(pix[1][1][0],pix[1][1][1]).g;
	  final.b = (1.0-t[0])*(1.0-t[1])*img.Get(pix[0][0][0],pix[0][0][1]).b +
	            (1.0-t[0])*t[1]*img.Get(pix[0][1][0],pix[0][1][1]).b +
	            t[0]*(1.0-t[1])*img.Get(pix[1][0][0],pix[1][0][1]).b +
	            t[0]*t[1]*img.Get(pix[1][1][0],pix[1][1][1]).b;
	            
	// Put into relevant position in output key...
	 nat32 offset = targ.Offset(x,y);
	 targ[offset] = final.r;
	 targ[offset+1] = final.g;
	 targ[offset+2] = final.b;
       }
      }
     
     // Normalise - 3 passes...
      // Mean...
       for (nat32 j=0;j<3;j++) targ.mean[j] = 0;
       for (nat32 j=0;j<21*21;j++)
       {
	targ.mean[0] += targ[j*3];
	targ.mean[1] += targ[j*3+1];
	targ.mean[2] += targ[j*3+2];
       }
       for (nat32 j=0;j<3;j++) targ.mean[j] /= 21.0*21.0;

      // Sd...
       for (nat32 j=0;j<3;j++) targ.sd[j] = 0;
       for (nat32 j=0;j<21*21;j++)
       {
	targ.sd[0] += math::Sqr(targ.mean[0]-targ[j*3]);
	targ.sd[1] += math::Sqr(targ.mean[1]-targ[j*3+1]);
	targ.sd[2] += math::Sqr(targ.mean[2]-targ[j*3+2]);
       }
       for (nat32 j=0;j<3;j++)
       {
        targ.sd[j] = math::Sqrt(targ.sd[j]/21.0*21.0);
        if (math::IsZero(targ.sd[j])) targ.sd[j] = 1.0;
       }
      
      // Apply...
       for (nat32 j=0;j<21*21;j++)
       {
        targ[j*3]   = (targ[j*3]  -targ.mean[0])/targ.sd[0];
        targ[j*3+1] = (targ[j*3+1]-targ.mean[1])/targ.sd[1];
        targ[j*3+2] = (targ[j*3+2]-targ.mean[2])/targ.sd[2];
       }
    }

   prog->Pop();
 prog->Pop();	
}

const MserKey & MserKeys::Key(nat32 i) const
{
 return key[i];
}

svt::Var * MserKeys::KeyImage(svt::Core & core) const
{
 // First create the image, 21*Size() by 21 pixels in size...	
  svt::Var * ret = new svt::Var(core);
  ret->Setup2D(21*Size(),21);
  bs::ColourRGB colIni(0.0,0.0,0.0);
  ret->Add("rgb",colIni);
  ret->Commit(false);
  
  svt::Field<bs::ColourRGB> rgb(ret,"rgb");
 
 // Iterate all keys, writting 'em one by one...
  for (nat32 i=0;i<Size();i++)
  {
   for (nat32 y=0;y<21;y++)
   {
    for (nat32 x=0;x<21;x++)
    {
     key[i].Extract(x,y,rgb.Get(21*i + x,y));
    }	   
   }
  }
 
 // Return...
  return ret;
}

//------------------------------------------------------------------------------
 };
};
