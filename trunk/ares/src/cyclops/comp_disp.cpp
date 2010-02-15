//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/comp_disp.h"

//------------------------------------------------------------------------------
CompDisp::CompDisp(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),capVal(1.0),
truthVar(null<svt::Var*>()),guessVar(null<svt::Var*>()),imageVar(null<svt::Var*>())
{
 // Create default disparity maps...
  real32 dispIni = 0.0;

  truthVar = new svt::Var(cyclops.Core());
  truthVar->Setup2D(320,240);
  truthVar->Add("disp",dispIni);
  truthVar->Commit();
  truthVar->ByName("disp",truthDisp);

  guessVar = new svt::Var(cyclops.Core());
  guessVar->Setup2D(320,240);
  guessVar->Add("disp",dispIni);
  guessVar->Commit();
  guessVar->ByName("disp",guessDisp);


 // Create default image...
  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  imageVar->Add("rgb",colIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Disparity Comparator");
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
   lab3->Set("    Error Cap:");
   cap->SetSize(48,24);
   cap->Set("1.0");

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
  win->OnDeath(MakeCB(this,&CompDisp::Quit));
  canvas->OnResize(MakeCB(this,&CompDisp::Resize));
  but1->OnClick(MakeCB(this,&CompDisp::LoadTruth));
  but2->OnClick(MakeCB(this,&CompDisp::LoadGuess));
  cap->OnChange(MakeCB(this,&CompDisp::OnCapChange));
}

CompDisp::~CompDisp()
{
 delete win;
 delete truthVar;
 delete guessVar;
 delete imageVar;
}

void CompDisp::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void CompDisp::Resize(gui::Base * obj,gui::Event * event)
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

void CompDisp::LoadTruth(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity","*.obf,*.dis",fn))
 {
  cstr filename = fn.ToStr();
   svt::Node * newDisp = svt::Load(cyclops.Core(),filename);
  mem::Free(filename);

  if (newDisp==null<svt::Node*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load disparity");
   return;
  }

  if (str::Compare(typestring(*newDisp),"eos::svt::Var")!=0)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a disparity map");
   delete newDisp;
   return;
  }
  svt::Var * newDispVar = static_cast<svt::Var*>(newDisp);

  nat32 dispInd;
  if ((newDispVar->Dims()!=2)||
      (newDispVar->GetIndex(cyclops.TT()("disp"),dispInd)==false)||
      (newDispVar->FieldType(dispInd)!=cyclops.TT()("eos::real32")))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a disparity map");
   delete newDispVar;
   return;
  }

  delete truthVar;
  truthVar = newDispVar;
  truthVar->ByName("disp",truthDisp);
  truthVar->ByName("mask",truthMask); // Will often fail.

  Update();
 }
}

void CompDisp::LoadGuess(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity","*.obf,*.dis",fn))
 {
  cstr filename = fn.ToStr();
   svt::Node * newDisp = svt::Load(cyclops.Core(),filename);
  mem::Free(filename);

  if (newDisp==null<svt::Node*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load disparity");
   return;
  }

  if (str::Compare(typestring(*newDisp),"eos::svt::Var")!=0)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a disparity map");
   delete newDisp;
   return;
  }
  svt::Var * newDispVar = static_cast<svt::Var*>(newDisp);

  nat32 dispInd;
  if ((newDispVar->Dims()!=2)||
      (newDispVar->GetIndex(cyclops.TT()("disp"),dispInd)==false)||
      (newDispVar->FieldType(dispInd)!=cyclops.TT()("eos::real32")))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a disparity map");
   delete newDispVar;
   return;
  }

  delete guessVar;
  guessVar = newDispVar;
  guessVar->ByName("disp",guessDisp);
  guessVar->ByName("mask",guessMask); // Will often fail.

  Update();
 }
}

void CompDisp::OnCapChange(gui::Base * obj,gui::Event * event)
{
 capVal = cap->Get().ToReal32();
 Update();
}

void CompDisp::Update()
{
 // Check the disparities match...
  resLab->Set("");
  resLab2->Set("");
  resLab3->Set("");
  if (truthDisp.Size(0)!=guessDisp.Size(0)) return;
  if (truthDisp.Size(1)!=guessDisp.Size(1)) return;


 // Create a new image...
  imageVar->Setup2D(truthDisp.Size(0),truthDisp.Size(1));
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
    bit tm = truthMask.Valid()?truthMask.Get(x,y):true;
    bit gm = guessMask.Valid()?guessMask.Get(x,y):true;
    if (tm==false)
    {
     if (gm==true) image.Get(x,y) = bs::ColRGB(0,255,0);
              else image.Get(x,y) = bs::ColRGB(0,0,255);
    }
    else
    {
     if (gm==true)
     {
      real32 err = math::Abs(truthDisp.Get(x,y)-guessDisp.Get(x,y));
      if ((!math::IsFinite(err))||(err>capVal))
      {
       outlierSum += 1.0;
       err = capVal;
      }
      else
      {
       inlierSum += 1.0;
       inlierErrSum += err;
      }
      
      byte v = byte(math::Clamp(255.0*err/capVal,0.0,255.0));
      image.Get(x,y) = bs::ColRGB(v,v,v);      
     }
     else
     {
      image.Get(x,y) = bs::ColRGB(255,0,0);
      outlierSum += 1.0;
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
