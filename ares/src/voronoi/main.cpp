//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "voronoi/main.h"

//------------------------------------------------------------------------------
// Code for cyclops class...
Voronoi::Voronoi()
:tokTab(),core(tokTab),guiFact(tokTab),app(null<gui::App*>()),win(null<gui::Window*>())
{
 if (guiFact.Active()==false) return;

 // Build the gui control panel...
  app = static_cast<gui::App*>(guiFact.Make("App"));
  win = static_cast<gui::Window*>(guiFact.Make("Window"));
    
  win->SetTitle("Voronoi");
  win->SetSize(640,480);
  
  gui::Panel * panel = static_cast<gui::Panel*>(guiFact.Make("Panel"));
  win->SetChild(panel);
   
  canvas = static_cast<gui::Canvas*>(guiFact.Make("Canvas"));
  panel->SetChild(canvas);
  
  canvas->OnResize(MakeCB(this,&Voronoi::Resize));
  canvas->OnClick(MakeCB(this,&Voronoi::Click));  
  
  app->Attach(win);
 
 // Enter the message pump...
  app->Go();
}

Voronoi::~Voronoi()
{
 delete app;	
}

void Voronoi::Resize(gui::Base * obj,gui::Event * event)
{
 rend::Pixels & rc = canvas->P();
 
 // Fill in the background...
  rc.Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(rc.Width()-1,rc.Height()-1)),bs::ColourRGB(0.8,0.8,0.8));

 // Extract all the Pos...
  ds::Array<ds::Delaunay2D<nat32>::Pos> posSet;
  tri.GetAllPos(posSet);

   
 // Draw the voronoi tesselation... 
  ds::Array<ds::Delaunay2D<nat32>::Mid> midSet;
  for (nat32 i=0;i<posSet.Size();i++)
  {
   posSet[i].GetMid(midSet);
   for (nat32 j=0;j<midSet.Size();j++)
   {
    ds::Delaunay2D<nat32>::Mid next = midSet[j].Clockwise(posSet[i]);
    if (midSet[j].HasCentre())
    {
     if (next.HasCentre())
     {
      math::Vect<2,real64> c1;
      math::Vect<2,real64> c2;
      if (midSet[j].Centre(c1)&&next.Centre(c2))
      {
       rc.Line(bs::Pos(nat32(c1[0]),nat32(c1[1])),
               bs::Pos(nat32(c2[0]),nat32(c2[1])),
               bs::ColourRGB(0.65,0.65,0.8));
      }
     }
     else
     {
      // Fire a line out to infinity, or a fair distance at any rate...
       math::Vect<2,real64> c;
       math::Vect<2,real64> dir;
       if (midSet[j].Centre(c))
       {
        next.Direction(midSet[j],dir); 
        rc.Line(bs::Pos(nat32(c[0]),nat32(c[1])),
                bs::Pos(nat32(c[0]+dir[0]*300.0),nat32(c[1]+dir[1]*300.0)),
                bs::ColourRGB(0.65,0.65,0.8));
       }
     }
    }
   }
  }
  
  
 // Draw the voronoi tesselation vertices...
  for (nat32 i=0;i<posSet.Size();i++)
  {
   posSet[i].GetMid(midSet);
   for (nat32 j=0;j<midSet.Size();j++)
   {
    if (midSet[j].HasCentre())
    {
     math::Vect<2,real64> c;
     if (midSet[j].Centre(c))
     {
      rc.Point(bs::Pos(nat32(c[0]),nat32(c[1])),bs::ColourRGB(0.0,0.0,0.0));	  	     
     }
    }	   
   }
  }

  
 // Draw the delaunay triangulation... 
  ds::Array<ds::Delaunay2D<nat32>::Pos> neSet;
  for (nat32 i=0;i<posSet.Size();i++)
  {
   posSet[i].GetPos(neSet);
   for (nat32 j=0;j<neSet.Size();j++)
   {
    if (posSet[i]<neSet[j])
    {
     rc.Line(bs::Pos(nat32(posSet[i].X()),nat32(posSet[i].Y())),
             bs::Pos(nat32(neSet[j].X()),nat32(neSet[j].Y())),
             bs::ColourRGB(0.9,0.6,0.6));
    }
   }
  }

  
 // Draw the vertices that form the triangulation...
  for (nat32 i=0;i<posSet.Size();i++)
  {
   rc.Point(bs::Pos(nat32(posSet[i].X()),nat32(posSet[i].Y())),bs::ColourRGB(0.0,0.0,0.0));	  
  }

  
 canvas->Update();
}

void Voronoi::Click(gui::Base * obj,gui::Event * event)
{
 gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
 if (mbe->down==false) return;
 
 tri.Add(real32(mbe->x),real32(mbe->y),0);

 canvas->Redraw();
 
 // We are ultimatly testing in this code - call various things that should work 
 // but could cause a crash if buggy...	
  //ds::Array<ds::Delaunay2D<nat32>::Mid> mid;
  //tri.GetAllMid(mid);
}

//------------------------------------------------------------------------------
// Program entry point...
EOS_APP_START
{
 Voronoi voronoi;
 return 0;
}

//------------------------------------------------------------------------------
