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


#include "cyclops/disp_clean.h"

//------------------------------------------------------------------------------
DispClean::DispClean(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),var(null<svt::Var*>())
{
 // Create default image and disparity...
  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,255);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);

  dispVar = new svt::Var(cyclops.Core());
  dispVar->Setup2D(320,240);
  real32 dispIni = 0.0;
  bit maskIni = false;
  bit userMaskIni = true;
  dispVar->Add("disp",dispIni);
  dispVar->Add("mask",maskIni);
  dispVar->Add("user_mask",userMaskIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);
  dispVar->ByName("user_mask",userMask);

  RenderDisp();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Disparity Cleaner");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+12,image.Size(1)+80);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Disparity...");
   but2->SetChild(lab2); lab2->Set("Remove Spikes");
   but3->SetChild(lab3); lab3->Set("Smooth");
   but4->SetChild(lab4); lab4->Set("Save Disparity...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);


   gui::Expander * expander1 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(expander1,false);
   expander1->Set("Spike Removal Parameters");
   expander1->Expand(false);

   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   expander1->SetChild(horiz2);

   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   srWindow    = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   srRange     = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   srTolerance = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab5->Set(" Window Radius:");     srWindow->Set("6.0");   srWindow->SetSize(32,24);
   lab6->Set("   Disparity Range:"); srRange->Set("1.0");    srRange->SetSize(32,24);
   lab7->Set("   Tolerance:");       srTolerance->Set("64"); srTolerance->SetSize(32,24);

   horiz2->AttachRight(lab5,false);
   horiz2->AttachRight(srWindow,false);
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(srRange,false);
   horiz2->AttachRight(lab7,false);
   horiz2->AttachRight(srTolerance,false);


   gui::Expander * expander2 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(expander2,false);
   expander2->Set("Smoothing Parameters");
   expander2->Expand(false);

   gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   expander2->SetChild(horiz3);

   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab8->Set("Source Sd:");
   lab9->Set("Smooth Sd:");
   lab10->Set("Iterations:");

   smoothSource = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); smoothSource->SetSize(32,24);
   smoothSmooth = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); smoothSmooth->SetSize(32,24);
   smoothIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); smoothIters->SetSize(32,24);

   horiz3->AttachRight(lab8,false);
   horiz3->AttachRight(smoothSource,false);
   horiz3->AttachRight(lab9,false);
   horiz3->AttachRight(smoothSmooth,false);
   horiz3->AttachRight(lab10,false);
   horiz3->AttachRight(smoothIters,false);

   smoothSource->Set("2.0");
   smoothSmooth->Set("0.5");
   smoothIters->Set("250");


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&DispClean::Quit));
  canvas->OnResize(MakeCB(this,&DispClean::Resize));

  but1->OnClick(MakeCB(this,&DispClean::LoadDisp));
  but2->OnClick(MakeCB(this,&DispClean::RemoveSpikes));
  but3->OnClick(MakeCB(this,&DispClean::Smooth));
  but4->OnClick(MakeCB(this,&DispClean::SaveDisp));
}

DispClean::~DispClean()
{
 delete win;
 delete var;
}

void DispClean::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void DispClean::Resize(gui::Base * obj,gui::Event * event)
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

void DispClean::LoadDisp(gui::Base * obj,gui::Event * event)
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

  delete dispVar;
  dispVar = newDispVar;

  if ((!dispVar->Exists("mask"))||(!dispVar->Exists("user_mask")))
  {
   bit maskIni = true;
   if (!dispVar->Exists("mask")) dispVar->Add("mask",maskIni);
   if (!dispVar->Exists("user_mask")) dispVar->Add("user_mask",maskIni);
   dispVar->Commit();
  }

  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);
  dispVar->ByName("user_mask",userMask);

  RenderDisp();
  canvas->Redraw();
 }
}

void DispClean::RemoveSpikes(gui::Base * obj,gui::Event * event)
{
 // Extract parameters...
  real32 windowRad = math::Max(srWindow->Get().ToReal32(),real32(0.001));
  real32 range = math::Max(srRange->Get().ToReal32(),real32(0.001));
  int32 tolerance = srTolerance->Get().ToInt32();


 time::Progress * prog = cyclops.BeginProg();
 prog->Report(0,3);

 // Mean shift the disparity map...
  // Before we mean shift we have to screw with the disparit map a bit as it has
  // no masking suport, simply wack the disparity values so far out there
  // irrelevant in the masked areas...
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (mask.Get(x,y)==false) disp.Get(x,y) = (-disp.Size(0) - range)*2.0;
    }
   }

  // Initialisation...
   alg::MeanShift ms;
   ms.AddFeature(disp,1.0/range);
   ms.AddFeature(0,1.0/windowRad);
   ms.AddFeature(1,1.0/windowRad);

  // Run...
   ms.Run(prog);


 // Cluster the result to create a segmentation...
  prog->Report(1,3);
  // Extract the results into a tempory variable...
   real32 tempIni = 0.0;
   nat32 segIni = 0;

   svt::Var temp(disp);
   temp.Add("disp",tempIni);
   temp.Add("x",tempIni);
   temp.Add("y",tempIni);
   temp.Add("seg",segIni);
   temp.Commit();

   svt::Field<real32> resDisp(&temp,"disp");
   svt::Field<real32> resX(&temp,"x");
   svt::Field<real32> resY(&temp,"y");
   svt::Field<nat32> seg(&temp,"seg");

   ms.Get(disp,resDisp);
   ms.Get(0,resX);
   ms.Get(1,resY);

  // Segment it all, extract the segmentation...
   filter::Segmenter sm;
   sm.AddField(resDisp,1.0/range);
   sm.AddField(resX,1.0/windowRad);
   sm.AddField(resY,1.0/windowRad);

   sm.Run(prog);

   sm.GetOutput(seg);
   nat32 segCount = sm.Segments();


 // Record the size of each segment, terminate all disparity values belonging to
 // segments that are smaller than the tolerance...
  prog->Report(2,3);
  // Count segment size...
   ds::Array<int32> segSize(segCount);
   for (nat32 i=0;i<segSize.Size();i++) segSize[i] = 0;

   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++) segSize[seg.Get(x,y)] += 1;
   }

  // Update user mask...
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++)
    {
     userMask.Get(x,y) = segSize[seg.Get(x,y)]>=tolerance;
    }
   }


 // Redraw...
  cyclops.EndProg();
  RenderDisp();
  canvas->Redraw();
}

void DispClean::Smooth(gui::Base * obj,gui::Event * event)
{
 // Get parameters....
  real32 paraSmoothSource = smoothSource->Get().ToReal32();
  real32 paraSmoothSmooth = smoothSmooth->Get().ToReal32();
  nat32 paraSmoothIters = smoothIters->Get().ToInt32();


 // Smooth the disparity map...
  // Setup...
   inf::IntegrateBP ibp(disp.Size(0),disp.Size(1));
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (mask.Get(x,y)&&userMask.Get(x,y))
     {
      if (math::IsZero(paraSmoothSource)) ibp.SetLocked(x,y,disp.Get(x,y));
                                     else ibp.SetVal(x,y,disp.Get(x,y),1.0/paraSmoothSource);
     }

     for (nat32 i=0;i<4;i++) ibp.SetRel(x,y,i,1.0,0.0,1.0/paraSmoothSmooth);
    }
   }

   ibp.SetIters(paraSmoothIters);

  // Run...
   ibp.Run(cyclops.BeginProg());
   cyclops.EndProg();

  // Extract results...
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     userMask.Get(x,y) = ibp.Defined(x,y);
     mask.Get(x,y) = userMask.Get(x,y);
     if (userMask.Get(x,y)) disp.Get(x,y) = ibp.Expectation(x,y);
    }
   }


 // Redraw...
  RenderDisp();
  canvas->Redraw();
}

void DispClean::SaveDisp(gui::Base * obj,gui::Event * event)
{
 // Create the result svt...
  real32 dispIni = 0.0;
  bit maskIni = false;

  svt::Var * result = new svt::Var(cyclops.Core());
  result->Setup2D(disp.Size(0),disp.Size(1));
  result->Add("disp",dispIni);
  result->Add("mask",maskIni);
  result->Commit();


 // Fill the result svt...
  svt::Field<real32> dispTemp(result,"disp");
  svt::Field<bit> maskTemp(result,"mask");

  dispTemp.CopyFrom(disp);
  for (nat32 y=0;y<maskTemp.Size(1);y++)
  {
   for (nat32 x=0;x<maskTemp.Size(0);x++) maskTemp.Get(x,y) = mask.Get(x,y) && userMask.Get(x,y);
  }


 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Disparity Map",fn))
  {
   if (!fn.EndsWith(".dis")) fn << ".dis";
   cstr ts = fn.ToStr();
   if (!svt::Save(ts,result,true))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving svt file.");
   }
   mem::Free(ts);
  }


 // Clean up...
  delete result;
}

void DispClean::RenderDisp()
{
 if ((image.Size(0)!=disp.Size(0))||(image.Size(1)!=disp.Size(1)))
 {
  var->Setup2D(disp.Size(0),disp.Size(1));
  var->Commit();
  var->ByName("rgb",image);

  canvas->SetSize(image.Size(0),image.Size(1));
 }

 real32 minDisp = 0.0;
 real32 maxDisp = 0.0;
 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++)
  {
   if (mask.Get(x,y))
   {
    minDisp = math::Min(minDisp,disp.Get(x,y));
    maxDisp = math::Max(maxDisp,disp.Get(x,y));
   }
  }
 }

 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++)
  {
   if (mask.Get(x,y))
   {
    if (userMask.Get(x,y))
    {
     nat8 val = nat8(math::Clamp<real32>(255.0*(disp.Get(x,y)-minDisp)/(maxDisp-minDisp),0,255));
     image.Get(x,y) = bs::ColRGB(val,val,val);
    }
    else image.Get(x,y) = bs::ColRGB(255,0,255);
   }
   else image.Get(x,y) = bs::ColRGB(0,0,255);
  }
 }
}

//------------------------------------------------------------------------------
