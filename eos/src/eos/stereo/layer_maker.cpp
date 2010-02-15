//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/layer_maker.h"

#include "eos/alg/mean_shift.h"
#include "eos/bs/geo_algs.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
LayerMaker::LayerMaker()
{}

LayerMaker::~LayerMaker()
{}

void LayerMaker::Setup(svt::Core & core,PlaneSeg & planeSeg,real32 radius,time::Progress * prog)
{
 prog->Push();

 // Create the data structure to encode all the segment data into...
  svt::Var var(core);
   var.Setup1D(planeSeg.Segments());
   real32 ini = 0.0;
   var.Add("x",ini);
   var.Add("y",ini);
   var.Add("a",ini);
   var.Add("b",ini);
   var.Add("c",ini);
   var.Add("area",ini);
  var.Commit(false);

  svt::Field<real32> fx; var.ByName("x",fx);
  svt::Field<real32> fy; var.ByName("y",fy);
  svt::Field<real32> fa; var.ByName("a",fa);
  svt::Field<real32> fb; var.ByName("b",fb);
  svt::Field<real32> fc; var.ByName("c",fc);
  svt::Field<real32> farea; var.ByName("area",farea);


 // Fill in the data structure...
  for (nat32 i=0;i<planeSeg.Segments();i++)
  {
   real32 x,y;
   planeSeg.GetCenter(i,x,y);
   bs::PlaneABC p;
   planeSeg.GetPlane(i,p);
   nat32 area;
   planeSeg.GetArea(i,area);

   fx.Get(i) = x; 
   fy.Get(i) = y;
   fa.Get(i) = p.a;
   fb.Get(i) = p.b;
   fc.Get(i) = p.c;
   farea.Get(i) = area;
  }


 // Now call mean shift on the segment data to cluster it...
  prog->Report(0,3);
  alg::MeanShift meanShift;
   meanShift.SetCutoff(1e-6,500);
   meanShift.SetDistance(DistFunc,radius);
   meanShift.AddFeature(fx);
   meanShift.AddFeature(fy);
   meanShift.AddFeature(fa);
   meanShift.AddFeature(fb);
   meanShift.AddFeature(fc);
   meanShift.SetWeight(farea);

  meanShift.Run(prog);

  meanShift.Get(fx,fx);
  meanShift.Get(fy,fy);
  meanShift.Get(fa,fa);
  meanShift.Get(fb,fb);
  meanShift.Get(fc,fc);


 // Now build a forest and cluster together all segments within range,
 // take a brute force (O(n^2)) approach as we will never be looking at more 
 // than a few hundred entrys (Can be replaced with a kd tree latter 
 // if needed, and a kd tree is avaliable)...
  prog->Report(1,3);
  // Creation...
   sed.Size(planeSeg.Segments());
   ds::Array<Node> sedInt(sed.Size());
   for (nat32 i=0;i<sedInt.Size();i++)
   {
    sedInt[i].parent = null<Node*>();
    sedInt[i].layer = 0;
   }

  // Clustering...
   for (nat32 i=0;i<sedInt.Size();i++)
   {
    for (nat32 j=i+1;j<sedInt.Size();j++)
    {
     // Check if the two segments need to be merged...
      Node * h1 = sedInt[i].Head();
      Node * h2 = sedInt[j].Head();
      if (h1!=h2)
      {
       bs::Pnt l1; bs::PlaneABC p1;
       l1.X() = fx.Get(i); l1.Y() = fy.Get(i);
       p1.a = fa.Get(i); p1.b = fb.Get(i); p1.c = fc.Get(i);

       bs::Pnt l2; bs::PlaneABC p2;
       l2.X() = fx.Get(j); l2.Y() = fy.Get(j);
       p2.a = fa.Get(j); p2.b = fb.Get(j); p2.c = fc.Get(j);
       
       if (PlaneDistance(l1,p1,l2,p2)<=(radius*0.5)) h2->parent = h1;
      }
    }
   }


 // Count the layers in the forrest and then build the data structures,
 // assigning layer numbers in the proccess and storing them with the
 // appropriate segments...
  prog->Report(2,3);
  nat32 layers = 0;
  for (nat32 i=0;i<sed.Size();i++)
  {
   Node * h = sedInt[i].Head();
   if (h->layer==0)
   {
    ++layers;
    h->layer = layers;
   }
   sed[i] = h->layer - 1;
  }
  layer.Size(layers);


 // Call the rebuild method to setup the planes...
  Rebuild(planeSeg,prog);


 prog->Pop();
}

void LayerMaker::Rebuild(PlaneSeg & planeSeg,time::Progress * prog)
{
 prog->Push();

 for (nat32 i=0;i<layer.Size();i++)
 {
  prog->Report(i,layer.Size());

  LayerSeg layerSeg;
  planeSeg.PrepLayerSeg(layerSeg);

  for (nat32 j=0;j<sed.Size();j++)
  {
   if (sed[j]==i) layerSeg.AddSegment(j);
  }

  layerSeg.Run();
  layerSeg.Result(layer[i]);
 }

 prog->Pop();
}

void LayerMaker::LayerMerge(svt::Core & core,const svt::Field<nat32> & segs,PlaneSeg & planeSeg,real32 radius,time::Progress * prog)
{
 prog->Push();
 
 // Create the data structure to encode all the segment data into...
  prog->Report(0,5);
  svt::Var var(core);
   var.Setup1D(layer.Size());
   real32 ini = 0.0;
   var.Add("x",ini);
   var.Add("y",ini);
   var.Add("a",ini);
   var.Add("b",ini);
   var.Add("c",ini);
   var.Add("area",ini);
  var.Commit(false);

  svt::Field<real32> fx; var.ByName("x",fx);
  svt::Field<real32> fy; var.ByName("y",fy);
  svt::Field<real32> fa; var.ByName("a",fa);
  svt::Field<real32> fb; var.ByName("b",fb);
  svt::Field<real32> fc; var.ByName("c",fc);
  svt::Field<real32> farea; var.ByName("area",farea);


 // Fill in the data structure...
  // Iterate and nullify x,y and area, but fill in the plane details...
   for (nat32 i=0;i<layer.Size();i++)
   {
    fx.Get(i) = 0.0;
    fy.Get(i) = 0.0;
    fa.Get(i) = layer[i].a;
    fb.Get(i) = layer[i].b;
    fc.Get(i) = layer[i].c;
    farea.Get(i) = 0;
   }
     
  // Iterate all pixels, for each one add its position and existance into
  // the relevent layer...
   for (nat32 y=0;y<segs.Size(1);y++)
   {
    for (nat32 x=0;x<segs.Size(0);x++)
    {
     nat32 i = sed[segs.Get(x,y)];
     fx.Get(i) += x; 
     fy.Get(i) += y;
     farea.Get(i) += 1;
    }
   }
   
  // Final iterate and divide through x and y for each layer by the area, 
  // to get actual centres...
   for (nat32 i=0;i<layer.Size();i++)
   {
    if (!math::Equal(farea.Get(i),real32(0.0)))
    {
     fx.Get(i) /= farea.Get(i);
     fy.Get(i) /= farea.Get(i);
    }
   }   


 // Call the mean shift...
  prog->Report(1,5);
  alg::MeanShift meanShift;
   meanShift.SetCutoff(1e-6,500);
   meanShift.SetDistance(DistFunc,radius);
   meanShift.AddFeature(fx);
   meanShift.AddFeature(fy);
   meanShift.AddFeature(fa);
   meanShift.AddFeature(fb);
   meanShift.AddFeature(fc);
   meanShift.SetWeight(farea);

  meanShift.Run(prog);

  meanShift.Get(fx,fx);
  meanShift.Get(fy,fy);
  meanShift.Get(fa,fa);
  meanShift.Get(fb,fb);
  meanShift.Get(fc,fc);


 // Cluster, using a forest. (Used O(n^2) algorithm as data set ushally small, 
 // could be latter improved if becomes a burden.)...
  prog->Report(2,5);
  // Creation...
   ds::Array<Node> laySeg(layer.Size());
   for (nat32 i=0;i<laySeg.Size();i++)
   {
    laySeg[i].parent = null<Node*>();
    laySeg[i].layer = 0;
   }

  // Clustering...
   for (nat32 i=0;i<laySeg.Size();i++)
   {
    for (nat32 j=i+1;j<laySeg.Size();j++)
    {
     // Check if the two layers need to be merged...
      Node * h1 = laySeg[i].Head();
      Node * h2 = laySeg[j].Head();
      if (h1!=h2)
      {
       bs::Pnt l1; bs::PlaneABC p1;
       l1.X() = fx.Get(i); l1.Y() = fy.Get(i);
       p1.a = fa.Get(i); p1.b = fb.Get(i); p1.c = fc.Get(i);

       bs::Pnt l2; bs::PlaneABC p2;
       l2.X() = fx.Get(j); l2.Y() = fy.Get(j);
       p2.a = fa.Get(j); p2.b = fb.Get(j); p2.c = fc.Get(j);
       
       if (PlaneDistance(l1,p1,l2,p2)<=(radius*0.5)) h2->parent = h1;
      }
    }
   }


  // Count the layers in the forrest and assign layer numbers 
  // in the proccess and sorting them with the appropriate segments...  
   prog->Report(3,5);
   nat32 layCount = 0;
   for (nat32 i=0;i<laySeg.Size();i++)
   {
    Node * h = laySeg[i].Head();
    if (h->layer==0)
    {
     ++layCount;
     h->layer = layCount;
    }
   }
   layer.Size(layCount);
       
  // Assign the new layer numbers to each segment...
   for (nat32 i=0;i<sed.Size();i++)
   {
    sed[i] = laySeg[sed[i]].Head()->layer - 1;
   }
   
  // Recreate the planes for the layers, as they are now wrong...
   prog->Report(4,5);
   Rebuild(planeSeg,prog);
   
 prog->Pop();
}

nat32 LayerMaker::Segments()
{
 return sed.Size();
}

nat32 & LayerMaker::SegToLayer(nat32 seg)
{
 return sed[seg];
}

nat32 LayerMaker::Layers()
{
 return layer.Size();
}

const bs::PlaneABC & LayerMaker::Plane(nat32 l)
{
 return layer[l];
}

void LayerMaker::GetLayerSeg(const svt::Field<nat32> & seg,svt::Field<nat32> & laySeg)
{
 for (nat32 y=0;y<laySeg.Size(1);y++)
 {
  for (nat32 x=0;x<seg.Size(0);x++)
  {
   laySeg.Get(x,y) = sed[seg.Get(x,y)];
  }
 }
}

bit LayerMaker::DistFunc(nat32,real32 * fv1,real32 * fv2,real32 pt)
{
 bs::Pnt l1;
 bs::PlaneABC p1;
  l1.X() = fv1[0];
  l1.Y() = fv1[1];
  p1.a = fv1[2];
  p1.b = fv1[3];
  p1.c = fv1[4];

 bs::Pnt l2;
 bs::PlaneABC p2;
  l2.X() = fv2[0];
  l2.Y() = fv2[1];
  p2.a = fv2[2];
  p2.b = fv2[3];
  p2.c = fv2[4];

 real32 dist = PlaneDistance(l1,p1,l2,p2);
 return dist<=pt;
}

//------------------------------------------------------------------------------
 };
};
