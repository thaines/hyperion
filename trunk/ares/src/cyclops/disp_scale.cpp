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


#include "cyclops/disp_scale.h"

//------------------------------------------------------------------------------
DispScale::DispScale(Cyclops & cyc)
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
  dispVar->Add("disp",dispIni);
  dispVar->Add("mask",maskIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);

  RenderDisp();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Scale Disparity");
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

   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   scale = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   but1->SetChild(lab1); lab1->Set("Load Disparity...");
   but2->SetChild(lab2); lab2->Set("Save Scaled Disparity...");
   but3->SetChild(lab3); lab3->Set("Update Pair...");

   lab4->Set("New Height:");
   scale->SetSize(48,24);
   scale->Set("240");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(lab4,false);
   horiz1->AttachRight(scale,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&DispScale::Quit));
  canvas->OnResize(MakeCB(this,&DispScale::Resize));

  but1->OnClick(MakeCB(this,&DispScale::LoadDisp));
  but2->OnClick(MakeCB(this,&DispScale::SaveDisp));
  but3->OnClick(MakeCB(this,&DispScale::UpdatePair));
}

DispScale::~DispScale()
{
 delete win;
 delete var;
}

void DispScale::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void DispScale::Resize(gui::Base * obj,gui::Event * event)
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

void DispScale::LoadDisp(gui::Base * obj,gui::Event * event)
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

  if (!dispVar->Exists("mask"))
  {
   bit maskIni = true;
   if (!dispVar->Exists("mask")) dispVar->Add("mask",maskIni);
   dispVar->Commit();
  }

  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);

  str::String ts;
  ts << disp.Size(1);
  scale->Set(ts);

  RenderDisp();
  canvas->Redraw();
 }
}

void DispScale::SaveDisp(gui::Base * obj,gui::Event * event)
{
 // Get the new dimensions...
  real32 factor = scale->Get().ToReal32();
  if (factor>1.0) factor /= real32(disp.Size(1));

  int32 width  = int32(factor*disp.Size(0));
  int32 height = int32(factor*disp.Size(1));


 // Create the result svt...
  real32 dispIni = 0.0;
  bit maskIni = false;

  svt::Var * result = new svt::Var(cyclops.Core());
  result->Setup2D(width,height);
  result->Add("disp",dispIni);
  result->Add("mask",maskIni);
  result->Commit();


 // Fill the result svt...
  svt::Field<real32> dispTemp(result,"disp");
  svt::Field<bit> maskTemp(result,"mask");

  // Offset the source to get x positions...
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (mask.Get(x,y)) disp.Get(x,y) += real32(x);
    }
   }

  // Scale the source into the output...
   for (nat32 y=0;y<dispTemp.Size(1);y++)
   {
    for (nat32 x=0;x<dispTemp.Size(0);x++)
    {
     maskTemp.Get(x,y) = svt::SampleRealLin2D(dispTemp.Get(x,y),real32(x)/factor,real32(y)/factor,disp,&mask);
    }
   }

  // Remove the offsets from the source...
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (mask.Get(x,y)) disp.Get(x,y) -= real32(x);
    }
   }

  // Correct for scaling and remove the offsets from the output...
   for (nat32 y=0;y<dispTemp.Size(1);y++)
   {
    for (nat32 x=0;x<dispTemp.Size(0);x++)
    {
     if (maskTemp.Get(x,y)) dispTemp.Get(x,y) = dispTemp.Get(x,y)*factor - real32(x);
    }
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

void DispScale::UpdatePair(gui::Base * obj,gui::Event * event)
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
    real32 factor = scale->Get().ToReal32();
    if (factor>1.0) factor /= real32(disp.Size(1));
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

void DispScale::RenderDisp()
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
    nat8 val = nat8(math::Clamp<real32>(255.0*(disp.Get(x,y)-minDisp)/(maxDisp-minDisp),0,255));
    image.Get(x,y) = bs::ColRGB(val,val,val);
   }
   else image.Get(x,y) = bs::ColRGB(0,0,255);
  }
 }
}

//------------------------------------------------------------------------------
