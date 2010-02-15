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


#include "cyclops/warp.h"

//------------------------------------------------------------------------------
Warp::Warp(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
imageVar(null<svt::Var*>()),disparityVar(null<svt::Var*>()),viewVar(null<svt::Var*>())
{
 // Create default image and disparity...
  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  bs::ColourRGB colIni(0.0,0.0,0.0);
  imageVar->Add("rgb",colIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);

  disparityVar = new svt::Var(cyclops.Core());
  disparityVar->Setup2D(320,240);
  real32 dispIni = 0.0;
  disparityVar->Add("disp",dispIni);
  disparityVar->Commit();
  disparityVar->ByName("disp",disparity);
  
 // Create the view...
  viewVar = new svt::Var(cyclops.Core());
  viewVar->Setup2D(320,240);
  bs::ColRGB cIni(0,0,0);
  viewVar->Add("rgb",cIni);
  viewVar->Commit();
  viewVar->ByName("rgb",view);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Forward Warp");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+48);
   
   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);
   
   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   amount = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Load Disparity...");
   but3->SetChild(lab3); lab3->Set("Save Image...");
   amount->Set("1.0");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(amount,false);
   horiz1->AttachRight(but3,false);

   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);
   
   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);
   
   canvas->SetSize(image.Size(0),image.Size(1));

 // Event handlers...
  win->OnDeath(MakeCB(this,&Warp::Quit));
  canvas->OnResize(MakeCB(this,&Warp::Resize));
  but1->OnClick(MakeCB(this,&Warp::LoadImage));
  but2->OnClick(MakeCB(this,&Warp::LoadObf));
  amount->OnChange(MakeCB(this,&Warp::AmountChange));
  but3->OnClick(MakeCB(this,&Warp::SaveImage));
}

Warp::~Warp()
{
 delete win;
 delete imageVar;
 delete disparityVar;
 delete viewVar;
}

void Warp::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Warp::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - view.Size(0))/2;
  nat32 sy = (canvas->P().Height() - view.Size(1))/2; 
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(view.Size(0),view.Size(1))),bs::Pos(sx,sy),view);

 // Update...
  canvas->Update();
}

void Warp::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  cstr filename = fn.ToStr();
  svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),filename);
  mem::Free(filename);
  if (floatVar==null<svt::Var*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
   return;
  }
  
  delete imageVar;
  imageVar = floatVar;
  imageVar->ByName("rgb",image);
  
  Update();
 }
}

void Warp::LoadObf(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity","*.obf,*.dis",fn))
 {
  cstr filename = fn.ToStr();
   svt::Node * floatNode = svt::Load(cyclops.Core(),filename);
  mem::Free(filename);
  
  if (floatNode==null<svt::Node*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load disparity");
   return;
  }
  
  if (str::Compare(typestring(*floatNode),"eos::svt::Var")!=0)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a disparity map");
   delete floatNode;
   return;
  }
  svt::Var * floatVar = static_cast<svt::Var*>(floatNode);

  nat32 dispInd;
  if ((floatVar->Dims()!=2)||
      (floatVar->GetIndex(cyclops.TT()("disp"),dispInd)==false)||
      (floatVar->FieldType(dispInd)!=cyclops.TT()("eos::real32")))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a disparity map");
   delete floatVar;
   return;
  }
  
  delete disparityVar;
  disparityVar = floatVar;
  disparityVar->ByName("disp",disparity);
  disparityVar->ByName("mask",mask); // Will often fail.

  Update();
 }
}

void Warp::SaveImage(gui::Base * obj,gui::Event * event)
{
 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Image",fn))
  {  
   // Create a tempory image...
    svt::Var tempVar(image);
    bs::ColourRGB colIni(0.0,0.0,0.0);
    tempVar.Add("rgb",colIni);
    tempVar.Commit();
    svt::Field<bs::ColourRGB> temp(&tempVar,"rgb");

   // Extract the warping amount...
    real32 t = amount->Get().ToReal32();

   // Warp...
    stereo::ForwardWarp(image,disparity,temp,5.0,t,mask.Valid()?(&mask):null<svt::Field<bit>*>());


   // Save the image...
    if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImage(temp,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
    }
    mem::Free(ts); 
  }
}

void Warp::AmountChange(gui::Base * obj,gui::Event * event)
{
 Update();
}

void Warp::Update()
{
 // If disparity and image don't match do nothing...
  if ((disparity.Size(0)!=image.Size(0))||(disparity.Size(1)!=image.Size(1))) return;

 // Extract the warping amount...
  real32 t = amount->Get().ToReal32();

 // Create a tempory image...
  svt::Var tempVar(image);
  bs::ColourRGB colIni(0.0,0.0,0.0);
  tempVar.Add("rgb",colIni);
  tempVar.Commit();
  svt::Field<bs::ColourRGB> temp(&tempVar,"rgb");

 // Warp...
  stereo::ForwardWarp(image,disparity,temp,5.0,t,mask.Valid()?(&mask):null<svt::Field<bit>*>());

 // Adjust format of view if need be...
  if ((view.Size(0)!=image.Size(0))||(view.Size(1)!=image.Size(1)))
  {
   delete viewVar;
   viewVar = new svt::Var(image);
   bs::ColRGB cIni(0,0,0);
   viewVar->Add("rgb",cIni);
   viewVar->Commit();
   viewVar->ByName("rgb",view);
  }
 
 // Copy tempory image into view...
  for (nat32 y=0;y<view.Size(1);y++)
  {
   for (nat32 x=0;x<view.Size(0);x++)
   {
    view.Get(x,y) = temp.Get(x,y);
   }
  }
  
 canvas->SetSize(view.Size(0),view.Size(1));
 canvas->Redraw();
}

//------------------------------------------------------------------------------
