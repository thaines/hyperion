//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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


#include "cyclops/comp_needle.h"

//------------------------------------------------------------------------------
CompNeedle::CompNeedle(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),capVal(10.0),
truthVar(null<svt::Var*>()),guessVar(null<svt::Var*>()),imageVar(null<svt::Var*>())
{
 // Create default disparity maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);

  truthVar = new svt::Var(cyclops.Core());
  truthVar->Setup2D(320,240);
  truthVar->Add("rgb",colourIni);
  truthVar->Commit();
  truthVar->ByName("rgb",truthImage);

  guessVar = new svt::Var(cyclops.Core());
  guessVar->Setup2D(320,240);
  guessVar->Add("rgb",colourIni);
  guessVar->Commit();
  guessVar->ByName("rgb",guessImage);


 // Create default image...
  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  imageVar->Add("rgb",colIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Needle Map Comparator");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   cap = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   resLab = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   resLab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   resLab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Truth...");
   but2->SetChild(lab2); lab2->Set("Load Guess...");
   lab3->Set("    Error Cap (Degrees):");
   cap->SetSize(48,24);
   cap->Set("10.0");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(lab3,false);
   horiz1->AttachRight(cap,false);
   horiz1->AttachRight(resLab,false);

   vert1->AttachBottom(resLab2,false);
   vert1->AttachBottom(resLab3,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&CompNeedle::Quit));
  canvas->OnResize(MakeCB(this,&CompNeedle::Resize));
  but1->OnClick(MakeCB(this,&CompNeedle::LoadTruth));
  but2->OnClick(MakeCB(this,&CompNeedle::LoadGuess));
  cap->OnChange(MakeCB(this,&CompNeedle::OnCapChange));
}

CompNeedle::~CompNeedle()
{
 delete win;
 delete truthVar;
 delete guessVar;
 delete imageVar;
}

void CompNeedle::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void CompNeedle::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // Update...
  canvas->Update();
}

void CompNeedle::LoadTruth(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Truth","*.bmp,*.jpg,*.png,*.tif",fn))
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
   
  // Stick in right variables...
   delete truthVar;
   truthVar = tempVar;
   truthVar->ByName("rgb",truthImage);

  // Update the display...
   Update();
 }
}

void CompNeedle::LoadGuess(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Guess","*.bmp,*.jpg,*.png,*.tif",fn))
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
   
  // Stick in right variables...
   delete guessVar;
   guessVar = tempVar;
   guessVar->ByName("rgb",guessImage);

  // Update the display...
   Update();
 }
}

void CompNeedle::OnCapChange(gui::Base * obj,gui::Event * event)
{
 capVal = cap->Get().ToReal32();
 Update();
}

void CompNeedle::Update()
{
 // Check the disparities match...
  resLab->Set("");
  resLab2->Set("");
  resLab3->Set("");
  if (truthImage.Size(0)!=guessImage.Size(0)) return;
  if (truthImage.Size(1)!=guessImage.Size(1)) return;


 // Create a new image...
  imageVar->Setup2D(truthImage.Size(0),truthImage.Size(1));
  imageVar->Commit();
  imageVar->ByName("rgb",image);


 // Iterate and colour by state, summing error as we go...
  real64 inlierSum = 0.0;
  real64 outlierSum = 0.0;
  real64 inlierErrSum = 0.0;
  for (nat32 y=0;y<image.Size(1);y++)
  {
   for (nat32 x=0;x<image.Size(0);x++)
   {
    bit noTruth = math::IsZero(truthImage.Get(x,y).r)&&
                  math::IsZero(truthImage.Get(x,y).g)&&
                  math::IsZero(truthImage.Get(x,y).b);
    bit noGuess = math::IsZero(guessImage.Get(x,y).r)&&
                  math::IsZero(guessImage.Get(x,y).g)&&
                  math::IsZero(guessImage.Get(x,y).b); 
    if (noTruth)
    {
     if (noGuess) image.Get(x,y) = bs::ColRGB(0,0,255);
             else image.Get(x,y) = bs::ColRGB(0,255,0);
    }
    else
    {
     if (noGuess)
     {
      image.Get(x,y) = bs::ColRGB(255,0,0);
      outlierSum += 1.0;
     }
     else
     {
      bs::Normal truth(truthImage.Get(x,y).r*2.0-1.0,truthImage.Get(x,y).g*2.0-1.0,truthImage.Get(x,y).b);
      bs::Normal guess(guessImage.Get(x,y).r*2.0-1.0,guessImage.Get(x,y).g*2.0-1.0,guessImage.Get(x,y).b);
      
      truth.Normalise();
      guess.Normalise();

      if (!math::IsFinite(truth.LengthSqr())||(!math::IsFinite(guess.LengthSqr())))
      {
       if (math::IsFinite(truth.LengthSqr())) outlierSum += 1.0;
       image.Get(x,y) = bs::ColRGB(0,255,255);
      }
      else
      {
       real32 angle = math::InvCos(truth * guess);
       if (math::IsFinite(angle))
       {
        angle *= 180.0/math::pi;
      
        if (angle<=capVal)
        {
         // Inlier...
          inlierSum += 1.0;
          inlierErrSum += angle;
        }
        else
        {
         // Outlier...
          outlierSum += 1.0;
        }
      
        byte v = byte(math::Clamp(255.0*angle/capVal,0.0,255.0));
        image.Get(x,y) = bs::ColRGB(v,v,v);  
       }
       else
       {
        image.Get(x,y) = bs::ColRGB(255,0,255);
       }
      }
     }
    }
   }
  }  


 // Stick the error sum and other stuff in the labels...
  {
   str::String s;
   real64 avgError = inlierErrSum/inlierSum;
   s << "Average inlier error = " << avgError << ";";
   resLab->Set(s);
  }
  {
   real64 errorSum = inlierErrSum + outlierSum*capVal;
   real64 percentage = 100.0*errorSum/((inlierSum+outlierSum)*capVal);

   str::String s;
   s << "Error sum = " << errorSum << "; Error sum percentage of max = " << percentage << ";";
   resLab2->Set(s);
  }
  {
   real64 outliers = 100.0*outlierSum/(inlierSum+outlierSum);

   str::String s;
   s << "Inlier Percentage = " << (100.0-outliers) << "; (Outlier percentage = " << outliers << ")";
   resLab3->Set(s);
  }

 // Redraw...
  canvas->SetSize(image.Size(0),image.Size(1));
  canvas->Redraw();
}

//------------------------------------------------------------------------------
