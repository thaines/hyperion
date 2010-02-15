//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


#include "cyclops/undistorter.h"

//------------------------------------------------------------------------------
Undistorter::Undistorter(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),floatVar(null<svt::Var*>()),var(null<svt::Var*>())
{
 // Create default images...
  floatVar = new svt::Var(cyclops.Core());
  floatVar->Setup2D(320,240);
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  floatVar->Add("rgb",colourIni);
  floatVar->Commit();
  floatVar->ByName("rgb",floatImage);

  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);

  ccValid = false;
  imageValid = false;


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Undistorter");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+48);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);


   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Load Intrinsic...");
   but3->SetChild(lab3); lab3->Set("Undistort");
   but4->SetChild(lab4); lab4->Set("Save Image...");
   but5->SetChild(lab5); lab5->Set("Save Intrinsic...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   horiz1->AttachRight(but5,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&Undistorter::Quit));
  canvas->OnResize(MakeCB(this,&Undistorter::Resize));
  but1->OnClick(MakeCB(this,&Undistorter::LoadImage));
  but2->OnClick(MakeCB(this,&Undistorter::LoadIntrinsic));
  but3->OnClick(MakeCB(this,&Undistorter::Undistort));
  but4->OnClick(MakeCB(this,&Undistorter::SaveImage));
  but5->OnClick(MakeCB(this,&Undistorter::SaveIntrinsic));
}

Undistorter::~Undistorter()
{
 delete win;
 delete floatVar;
 delete var;
}

void Undistorter::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Undistorter::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // If the image is un-distorted render the distortion grid...
  if (imageValid&&ccValid)
  {
   // Divide the image up into 50 pixel chunks, take the image coordinates and
   // pretend there undistroted, distort, and draw the lines...
    static const int32 div = 50;

    // Horizontal...
     for (int32 y=-2;y<=int32(image.Size(1))/div+2;y++)
     {
      math::Vect<2,real64> undis;
      undis[1] = real64(y*div);

      math::Vect<2,real64> prev;
      for (int32 x=-2;x<=int32(image.Size(0))/div+2;x++)
      {
       undis[0] = real64(x*div);

       math::Vect<2,real64> curr;
       cc.radial.Dis(undis,curr);
       if (x!=-2) RenderLine(prev,curr,bs::ColourRGB(1.0,0.0,1.0));
       prev = curr;
      }
     }

    // Vertical...
     for (int32 x=-2;x<=int32(image.Size(0))/div+2;x++)
     {
      math::Vect<2,real64> undis;
      undis[0] = real64(x*div);

      math::Vect<2,real64> prev;
      for (int32 y=-2;y<=int32(image.Size(1))/div+2;y++)
      {
       undis[1] = real64(y*div);

       math::Vect<2,real64> curr;
       cc.radial.Dis(undis,curr);
       if (y!=-2) RenderLine(prev,curr,bs::ColourRGB(1.0,0.0,1.0));
       prev = curr;
      }
     }
  }


 // Update...
  canvas->Update();
}

void Undistorter::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * tempVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (tempVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }
   delete floatVar;
   floatVar = tempVar;
   floatVar->ByName("rgb",floatImage);


  // Image loaded, but its not in a format suitable for fast display - convert...
   var->Setup2D(floatVar->Size(0),floatVar->Size(1));
   var->Commit();
   var->ByName("rgb",image);

   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     image.Get(x,y) = floatImage.Get(x,y);
    }
   }

  // Refresh the display...
   imageValid = true;
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
 }
}

void Undistorter::LoadIntrinsic(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Intrinsic Calibration","*.icd",fn))
 {
  if (cc.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
  }
  else
  {
   ccValid = true;
   canvas->Redraw();
  }
 }
}

void Undistorter::Undistort(gui::Base * obj,gui::Event * event)
{
 if (ccValid)
 {
  if (imageValid)
  {
   time::Progress * prog = cyclops.BeginProg();
   prog->Report(0,3);

   // First work out the extents of the new image...
    real32 coordX[3];
    coordX[0] = 0.0;
    coordX[1] = cc.radial.centre[0];
    coordX[2] = cc.dim[0]-1.0;

    real32 coordY[3];
    coordY[0] = 0.0;
    coordY[1] = cc.radial.centre[1];
    coordY[2] = cc.dim[1]-1.0;

    int32 minX = int32(cc.dim[0]);
    int32 maxX = 0;
    int32 minY = int32(cc.dim[1]);
    int32 maxY = 0;

    for (nat32 v=0;v<3;v++)
    {
     for (nat32 u=0;u<3;u++)
     {
      math::Vect<2,real64> dis;
      dis[0] = coordX[u];
      dis[1] = coordY[v];

      math::Vect<2,real64> undis;
      cc.radial.UnDis(dis,undis);

      minX = math::Min(minX,int32(math::RoundDown(undis[0])));
      maxX = math::Max(maxX,int32(math::RoundUp(undis[0])));
      minY = math::Min(minY,int32(math::RoundDown(undis[1])));
      maxY = math::Max(maxY,int32(math::RoundUp(undis[1])));
     }
    }
    LogDebug("Undistort dimensions {minX,maxX,minY,maxY}" << LogDiv()
             << minX << LogDiv() << maxX << LogDiv() << minY << LogDiv() << maxY);


   // Create the new image...
    svt::Var * udVar = new svt::Var(cyclops.Core());
    udVar->Setup2D(maxX-minX+1,maxY-minY+1);
    bs::ColourRGB colourIni(0.0,0.0,0.0);
    udVar->Add("rgb",colourIni);
    udVar->Commit(false);
    svt::Field<bs::ColourRGB> udImage(udVar,"rgb");


   // Create a mask for the old image, so we correctly handle any masked out regions...
    svt::Var maskVar(cyclops.Core());
    maskVar.Setup2D(image.Size(0),image.Size(1));
    bit maskIni = true;
    maskVar.Add("mask",maskIni);
    maskVar.Commit();
    svt::Field<bit> mask(&maskVar,"mask");

    for (nat32 y=0;y<mask.Size(1);y++)
    {
     for (nat32 x=0;x<mask.Size(0);x++)
     {
      if (math::Equal(floatImage.Get(x,y).r,real32(1.0))&&
          math::Equal(floatImage.Get(x,y).g,real32(0.0))&&
          math::Equal(floatImage.Get(x,y).b,real32(1.0))) mask.Get(x,y) = false;
     }
    }


   // Sample from the old image into the new image...
    prog->Report(1,3);
    prog->Push();
    for (int32 y=0;y<int32(udImage.Size(1));y++)
    {
     prog->Report(y,udImage.Size(1));
     for (int32 x=0;x<int32(udImage.Size(0));x++)
     {
      // Find where we are sampling...
       math::Vect<2,real64> loc;
       loc[0] = real64(x+minX);
       loc[1] = real64(y+minY);
       cc.radial.Dis(loc,loc);

      // Take the sample...
       if (svt::SampleColourRGBLin2D(udImage.Get(x,y),loc[0],loc[1],floatImage,&mask)==false)
       {
	udImage.Get(x,y) = bs::ColourRGB(1.0,0.0,1.0);
       }
     }
    }
    prog->Pop();


   // Book-keeping...
    prog->Report(2,3);
    delete floatVar;
    floatVar = udVar;
    floatImage = udImage;

    var->Setup2D(floatVar->Size(0),floatVar->Size(1));
    var->Commit();
    var->ByName("rgb",image);

    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      image.Get(x,y) = floatImage.Get(x,y);
     }
    }


   // Refresh the display...
    imageValid = false;
    canvas->SetSize(image.Size(0),image.Size(1));
    canvas->Redraw();

   cyclops.EndProg();
  }
  else
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Current image allready undistorted or you have not yet loaded an image.");
  }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"No calibration file loaded");
 }
}

void Undistorter::SaveImage(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Image",fn))
  {
   if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";

   // Save it...
    cstr ts = fn.ToStr();
     if (!filter::SaveImage(floatImage,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image.");
     }
    mem::Free(ts);
  }
}

void Undistorter::SaveIntrinsic(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Intrinsic",fn))
  {   
   // Work out the extents of the new image...
    real32 coordX[3];
    coordX[0] = 0.0;
    coordX[1] = cc.radial.centre[0];
    coordX[2] = cc.dim[0]-1.0;

    real32 coordY[3];
    coordY[0] = 0.0;
    coordY[1] = cc.radial.centre[1];
    coordY[2] = cc.dim[1]-1.0;

    int32 minX = int32(cc.dim[0]);
    int32 maxX = 0;
    int32 minY = int32(cc.dim[1]);
    int32 maxY = 0;

    for (nat32 v=0;v<3;v++)
    {
     for (nat32 u=0;u<3;u++)
     {
      math::Vect<2,real64> dis;
      dis[0] = coordX[u];
      dis[1] = coordY[v];

      math::Vect<2,real64> undis;
      cc.radial.UnDis(dis,undis);

      minX = math::Min(minX,int32(math::RoundDown(undis[0])));
      maxX = math::Max(maxX,int32(math::RoundUp(undis[0])));
      minY = math::Min(minY,int32(math::RoundDown(undis[1])));
      maxY = math::Max(maxY,int32(math::RoundUp(undis[1])));
     }
    }
   
   
   // Modify the calibration...
    cam::CameraCalibration modCC = cc;
    modCC.intrinsic.Offset(-minX,-minY);
    for (nat32 i=0;i<4;i++) modCC.radial.k[i] = 0.0;
    modCC.dim[0] = maxX-minX+1;
    modCC.dim[1] = maxY-minY+1;


   // Save it...
    if (!fn.EndsWith(".icd")) fn << ".icd";
    if (!modCC.Save(fn))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving intrinsic calibration.");
    }
  }
}

void Undistorter::RenderLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - image.Size(0))/2;
 nat32 sy = (canvas->P().Height() - image.Size(1))/2;

 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);

 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);

 canvas->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col);
}

//------------------------------------------------------------------------------
