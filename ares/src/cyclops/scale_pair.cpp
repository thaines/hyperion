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


#include "cyclops/scale_pair.h"

//------------------------------------------------------------------------------
ScalePair::ScalePair(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>())
{
 // Create default images...
  bs::ColRGB colIni(0,0,0);
  bs::ColourRGB colourIni(0.0,0.0,0.0);

  leftImgVar = new svt::Var(cyclops.Core());
  leftImgVar->Setup2D(320,240);
  leftImgVar->Add("rgb",colourIni);
  leftImgVar->Commit();
  leftImgVar->ByName("rgb",leftImg);

  rightImgVar = new svt::Var(cyclops.Core());
  rightImgVar->Setup2D(320,240);
  rightImgVar->Add("rgb",colourIni);
  rightImgVar->Commit();
  rightImgVar->ByName("rgb",rightImg);

  leftVar = new svt::Var(cyclops.Core());
  leftVar->Setup2D(320,240);
  leftVar->Add("rgb",colIni);
  leftVar->Commit();
  leftVar->ByName("rgb",leftImage);

  rightVar = new svt::Var(cyclops.Core());
  rightVar->Setup2D(320,240);
  rightVar->Add("rgb",colIni);
  rightVar->Commit();
  rightVar->ByName("rgb",rightImage);


 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Scale Image Pair");
  cyclops.App().Attach(win);
  win->SetSize(leftImage.Size(0)+rightImage.Size(0)+24,math::Max(leftImage.Size(1),rightImage.Size(1))+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Left Image...");
   but2->SetChild(lab2); lab2->Set("Load Right Image...");

   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert2,false);
   vert2->AttachBottom(but1);
   vert2->AttachBottom(but2);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but3->SetChild(lab3); lab3->Set("Save Left Scaled...");
   but4->SetChild(lab4); lab4->Set("Save Right Scaled...");

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(but3);
   vert3->AttachBottom(but4);


   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but5->SetChild(lab5); lab5->Set("Update Pair Config...");
   horiz1->AttachRight(but5,false);

   newHeight = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   newDim = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   newHeight->Set(str::String("240"));

   gui::Vertical * vert4 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert4,false);
   vert4->AttachBottom(newHeight);
   vert4->AttachBottom(newDim);


   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2);

   gui::Panel * panel1 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   gui::Panel * panel2 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   horiz2->AttachRight(panel1);
   horiz2->AttachRight(panel2);

   left = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   right = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel1->SetChild(left);
   panel2->SetChild(right);

   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   right->SetSize(rightImage.Size(0),rightImage.Size(1));


 // Handlers...
  win->OnDeath(MakeCB(this,&ScalePair::Quit));
  left->OnResize(MakeCB(this,&ScalePair::ResizeLeft));
  right->OnResize(MakeCB(this,&ScalePair::ResizeRight));

  but1->OnClick(MakeCB(this,&ScalePair::LoadLeft));
  but2->OnClick(MakeCB(this,&ScalePair::LoadRight));
  but3->OnClick(MakeCB(this,&ScalePair::SaveLeft));
  but4->OnClick(MakeCB(this,&ScalePair::SaveRight));
  but5->OnClick(MakeCB(this,&ScalePair::UpdatePair));
  newHeight->OnChange(MakeCB(this,&ScalePair::EditHeight));
}

ScalePair::~ScalePair()
{
 delete win;
 delete leftVar;
 delete rightVar;
 delete leftImgVar;
 delete rightImgVar;
}

void ScalePair::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void ScalePair::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(leftImage.Size(0),leftImage.Size(1))),bs::Pos(sx,sy),leftImage);

 // Update...
  left->Update();
}

void ScalePair::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(rightImage.Size(0),rightImage.Size(1))),bs::Pos(sx,sy),rightImage);

 // Update...
  right->Update();
}

void ScalePair::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

   delete leftImgVar;
   leftImgVar = floatVar;
   leftImgVar->ByName("rgb",leftImg);

  // Image loaded, but its not in a format suitable for fast display - convert...
   leftVar->Setup2D(leftImgVar->Size(0),leftImgVar->Size(1));
   leftVar->Commit();
   leftVar->ByName("rgb",leftImage);

   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     leftImage.Get(x,y) = leftImg.Get(x,y);
    }
   }

  // Update the editbox and printed dimensions...
   str::String s;
   s << leftImage.Size(1);
   newHeight->Set(s);
   UpdateDim();

  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();
 }
}

void ScalePair::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

   delete rightImgVar;
   rightImgVar = floatVar;
   rightImgVar->ByName("rgb",rightImg);

  // Image loaded, but its not in a format suitable for fast display - convert...
   rightVar->Setup2D(rightImgVar->Size(0),rightImgVar->Size(1));
   rightVar->Commit();
   rightVar->ByName("rgb",rightImage);

   for (nat32 y=0;y<rightImage.Size(1);y++)
   {
    for (nat32 x=0;x<rightImage.Size(0);x++)
    {
     rightImage.Get(x,y) = rightImg.Get(x,y);
    }
   }

  // Update the editbox and printed dimensions...
   str::String s;
   s << rightImage.Size(1);
   newHeight->Set(s);
   UpdateDim();


  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
 }
}

void ScalePair::SaveLeft(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Scaled Left Image",fn))
  {
   if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
   // Calculate the scale factor and the new image size - kept seperate as we
   // obviously need it to be correct...
    real32 factor = newHeight->Get().ToReal32();
    if (factor>1.0) factor /= real32(leftImage.Size(1));

    int32 width  = int32(factor*leftImage.Size(0));
    int32 height = int32(factor*leftImage.Size(1));


   // Create a data structure to contain it...
    svt::Var civ(cyclops.Core());
     civ.Setup2D(width,height);
     bs::ColourRGB iniRGB(0.0,0.0,0.0);
     civ.Add("rgb",iniRGB);
    civ.Commit(false);

    svt::Field<bs::ColourRGB> ci(&civ,"rgb");


   // Create an input and output mask, fill in the input mask...
    svt::Var inMaskVar(cyclops.Core());
    inMaskVar.Setup2D(leftImg.Size(0),leftImg.Size(1));
    bit maskIni = true;
    inMaskVar.Add("mask",maskIni);
    inMaskVar.Commit();
    svt::Field<bit> inMask(&inMaskVar,"mask");

    svt::Var outMaskVar(cyclops.Core());
    outMaskVar.Setup2D(width,height);
    outMaskVar.Add("mask",maskIni);
    outMaskVar.Commit();
    svt::Field<bit> outMask(&outMaskVar,"mask");

    for (nat32 y=0;y<leftImg.Size(1);y++)
    {
     for (nat32 x=0;x<leftImg.Size(0);x++)
     {
      if (math::Equal(leftImg.Get(x,y).r,real32(1.0))&&
          math::Equal(leftImg.Get(x,y).g,real32(0.0))&&
          math::Equal(leftImg.Get(x,y).b,real32(1.0))) inMask.Get(x,y) = false;
     }
    }


   // Fill data structure...
    filter::ScaleExtend(leftImg,inMask,ci,outMask,factor);


   // Use the output mask to pink-out as needed...
    for (int32 y=0;y<height;y++)
    {
     for (int32 x=0;x<width;x++)
     {
      if (outMask.Get(x,y)==false) ci.Get(x,y) = bs::ColourRGB(1.0,0.0,1.0);
     }
    }


   // Save it...
    cstr ts = fn.ToStr();
     if (!filter::SaveImage(ci,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving scaled image.");
     }
    mem::Free(ts);
  }
}

void ScalePair::SaveRight(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Scaled Right Image",fn))
  {
   if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
   // Calculate the scale factor and the new image size - kept seperate as we
   // obviously need it to be correct...
    real32 factor = newHeight->Get().ToReal32();
    if (factor>1.0) factor /= real32(rightImage.Size(1));

    int32 width  = int32(factor*rightImage.Size(0));
    int32 height = int32(factor*rightImage.Size(1));


   // Create a data structure to contain it...
    svt::Var civ(cyclops.Core());
     civ.Setup2D(width,height);
     bs::ColourRGB iniRGB(0.0,0.0,0.0);
     civ.Add("rgb",iniRGB);
    civ.Commit(false);

    svt::Field<bs::ColourRGB> ci(&civ,"rgb");


   // Create an input and output mask, fill in the input mask...
    svt::Var inMaskVar(cyclops.Core());
    inMaskVar.Setup2D(rightImg.Size(0),rightImg.Size(1));
    bit maskIni = true;
    inMaskVar.Add("mask",maskIni);
    inMaskVar.Commit();
    svt::Field<bit> inMask(&inMaskVar,"mask");

    svt::Var outMaskVar(cyclops.Core());
    outMaskVar.Setup2D(width,height);
    outMaskVar.Add("mask",maskIni);
    outMaskVar.Commit();
    svt::Field<bit> outMask(&outMaskVar,"mask");

    for (nat32 y=0;y<rightImg.Size(1);y++)
    {
     for (nat32 x=0;x<rightImg.Size(0);x++)
     {
      if (math::Equal(rightImg.Get(x,y).r,real32(1.0))&&
          math::Equal(rightImg.Get(x,y).g,real32(0.0))&&
          math::Equal(rightImg.Get(x,y).b,real32(1.0))) inMask.Get(x,y) = false;
     }
    }


   // Fill data structure...
    filter::ScaleExtend(rightImg,inMask,ci,outMask,factor);


   // Use the output mask to pink-out as needed...
    for (int32 y=0;y<height;y++)
    {
     for (int32 x=0;x<width;x++)
     {
      if (outMask.Get(x,y)==false) ci.Get(x,y) = bs::ColourRGB(1.0,0.0,1.0);
     }
    }


   // Save it...
    cstr ts = fn.ToStr();
     if (!filter::SaveImage(ci,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving scaled image.");
     }
    mem::Free(ts);
  }
}

void ScalePair::UpdatePair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera Pair File","*.pcc",fn))
 {
  cam::CameraPair pair;
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load camera pair file");
  }
  else
  {
   // Get the factor...
    real32 factor = newHeight->Get().ToReal32();
    if (factor>1.0) factor /= real32(leftImage.Size(1));
    LogDebug("Scaling .pcc by a factor of " << factor);

   // Apply to both sides...
    pair.ScaleLeft(pair.leftDim[0]*factor,pair.leftDim[1]*factor);
    pair.ScaleRight(pair.rightDim[0]*factor,pair.rightDim[1]*factor);

   // Save back the file...
    str::String fn2;
    if (cyclops.App().SaveFileDialog("Save Camera Pair File...",fn2))
    {
     if (!fn2.EndsWith(".pcc")) fn2 += ".pcc";
     if (!pair.Save(fn2,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving back the file.");
      return;
     }
    }
  }
 }
}

void ScalePair::EditHeight(gui::Base * obj,gui::Event * event)
{
 UpdateDim();
}

void ScalePair::UpdateDim()
{
 // Get factor...
  real32 factor = newHeight->Get().ToReal32();
  if (factor>1.0) factor /= real32(rightImage.Size(1));

 // Create dimension string...
  str::String s;
  s << "left = (" << nat32(leftImage.Size(0)*factor) << "," << nat32(leftImage.Size(1)*factor);
  s << "); right = (" << nat32(rightImage.Size(0)*factor) << "," << nat32(rightImage.Size(1)*factor) << ")";

 // Display...
  newDim->Set(s);
}

//------------------------------------------------------------------------------
