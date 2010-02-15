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


#include "cyclops/colour_balance.h"

//------------------------------------------------------------------------------
ColourBalance::ColourBalance(Cyclops & cyc)
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
  win->SetTitle("Match Image Colours");
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


   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but5->SetChild(lab5); lab5->Set("Match");
   horiz1->AttachRight(but5,false);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but3->SetChild(lab3); lab3->Set("Save Left...");
   but4->SetChild(lab4); lab4->Set("Save Right...");

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(but3);
   vert3->AttachBottom(but4);


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
  win->OnDeath(MakeCB(this,&ColourBalance::Quit));
  left->OnResize(MakeCB(this,&ColourBalance::ResizeLeft));
  right->OnResize(MakeCB(this,&ColourBalance::ResizeRight));

  but1->OnClick(MakeCB(this,&ColourBalance::LoadLeft));
  but2->OnClick(MakeCB(this,&ColourBalance::LoadRight));
  but3->OnClick(MakeCB(this,&ColourBalance::SaveLeft));
  but4->OnClick(MakeCB(this,&ColourBalance::SaveRight));
  but5->OnClick(MakeCB(this,&ColourBalance::Match));
}

ColourBalance::~ColourBalance()
{
 delete win;
 delete leftVar;
 delete rightVar;
 delete leftImgVar;
 delete rightImgVar;
}

void ColourBalance::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void ColourBalance::ResizeLeft(gui::Base * obj,gui::Event * event)
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

void ColourBalance::ResizeRight(gui::Base * obj,gui::Event * event)
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

void ColourBalance::LoadLeft(gui::Base * obj,gui::Event * event)
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

  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();
 }
}

void ColourBalance::LoadRight(gui::Base * obj,gui::Event * event)
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

  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
 }
}

void ColourBalance::SaveLeft(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Left Image",fn))
  {
   if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";

   // Save it...
    cstr ts = fn.ToStr();
     if (!filter::SaveImage(leftImg,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving scaled image.");
     }
    mem::Free(ts);
  }
}

void ColourBalance::SaveRight(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Right Image",fn))
  {
   if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";

   // Save it...
    cstr ts = fn.ToStr();
     if (!filter::SaveImage(rightImg,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving scaled image.");
     }
    mem::Free(ts);
  }
}

void ColourBalance::Match(gui::Base * obj,gui::Event * event)
{
 // Do the matching...
  cyclops.BeginProg()->Report(0,1);
  filter::SimpleImageMatch(leftImg,rightImg,4,2.0);
  cyclops.EndProg();


  // Re-convert left image for fast display...
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


 // Re-convert right image for fast display...
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


  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();

   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
}

//------------------------------------------------------------------------------
