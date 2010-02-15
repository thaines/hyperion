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


#include "cyclops/to_orient.h"

//------------------------------------------------------------------------------
ToOrient::ToOrient(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
asReal(null<svt::Var*>()),asImg(null<svt::Var*>())
{
 // Create default disparity array...
  asReal = new svt::Var(cyclops.Core());
  asReal->Setup2D(320,240);
  real32 dispIni = 0.0;
  bit maskIni = true;
  asReal->Add("disp",dispIni);
  asReal->Add("mask",maskIni);
  asReal->Commit();
  asReal->ByName("disp",disp);
  asReal->ByName("mask",dispMask);

 // Create default image...
  asImg = new svt::Var(cyclops.Core());
  asImg->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  asImg->Add("rgb",colIni);
  asImg->Commit();
  asImg->ByName("rgb",image);

 // Create default calibration...
  pair.SetDefault(320,240);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Disparity To Needle map");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+64);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but2->SetChild(lab2); lab2->Set("Load Disparity...");
   but3->SetChild(lab3); lab3->Set("Load Calibration...");
   but4->SetChild(lab4); lab4->Set("Save Needle Map...");

   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);



   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));

 // Event handlers...
  win->OnDeath(MakeCB(this,&ToOrient::Quit));
  canvas->OnResize(MakeCB(this,&ToOrient::Resize));
  but2->OnClick(MakeCB(this,&ToOrient::LoadSvt));
  but3->OnClick(MakeCB(this,&ToOrient::LoadPair));
  but4->OnClick(MakeCB(this,&ToOrient::SaveNeedle));
}

ToOrient::~ToOrient()
{
 delete win;
 delete asImg;
 delete asReal;
}

void ToOrient::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void ToOrient::Resize(gui::Base * obj,gui::Event * event)
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

void ToOrient::LoadSvt(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity File","*.obf,*.dis",fn))
 {
  // Load svt, verify it is a disparity map...
   cstr filename = fn.ToStr();
   svt::Node * floatNode = svt::Load(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatNode==null<svt::Node*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load file.");
    delete floatNode;
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


  // Move the floatVar into the asReal array, deleting previous...
   delete asReal;
   asReal = floatVar;
   if (!asReal->Exists("mask"))
   {
    bit maskIni = true;
    asReal->Add("mask",maskIni);
    asReal->Commit();
   }
   asReal->ByName("disp",disp);
   asReal->ByName("mask",dispMask);
   real32 maxDisp = 0.1;
   real32 minDisp = 0.0;
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (dispMask.Get(x,y))
     {
      minDisp = math::Min(minDisp,disp.Get(x,y));
      maxDisp = math::Max(maxDisp,disp.Get(x,y));
     }
    }
   }


  // Now convert asReal into asImg, for display...
   asImg->Setup2D(floatVar->Size(0),floatVar->Size(1));
   asImg->Commit();
   asImg->ByName("rgb",image);

   maxDisp = 255.0/(maxDisp-minDisp);
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     if (dispMask.Get(x,y))
     {
      byte v = byte((disp.Get(x,y)-minDisp)*maxDisp);
      image.Get(x,y) = bs::ColRGB(v,v,v);
     }
     else image.Get(x,y) = bs::ColRGB(0,0,255);
    }
   }


  // Refresh the display...
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
 }
}

void ToOrient::LoadPair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
 }
}



void ToOrient::SaveNeedle(gui::Base * obj,gui::Event * event)
{
 LogBlock("ToOrient::SaveNeedle","-");

 // Ask for a filename...
  str::String fn;
  if (cyclops.App().SaveFileDialog("Save image...",fn))
  {
   time::Progress * prog = cyclops.BeginProg();

   // Create tempory image...
    svt::Var civ(cyclops.Core());
    civ.Setup2D(disp.Size(0),disp.Size(1));
    bs::ColourRGB iniRGB(0.0,0.0,0.0);
    civ.Add("rgb",iniRGB);
    civ.Commit(false);
    svt::Field<bs::ColourRGB> ci(&civ,"rgb");


   // Create array of 3D positions...
    cam::CameraPair tPair = pair;
    tPair.LeftToDefault();

    prog->Report(0,2);
    ds::Array2D<bs::Vert> pos(disp.Size(0),disp.Size(1));
    for (nat32 y=0;y<pos.Height();y++)
    {
     for (nat32 x=0;x<pos.Width();x++)
     {
      if (dispMask.Get(x,y))
      {
       math::Vect<4,real64> p;
       tPair.Triangulate(x,y,disp.Get(x,y),p);
       p /= p[3];

       for (nat32 i=0;i<3;i++) pos.Get(x,y)[i] = p[i];
      }
     }
    }


   // Calculate and fill in the normals...
    prog->Report(1,2);
    for (int32 y=0;y<int32(ci.Size(1));y++)
    {
     for (int32 x=0;x<int32(ci.Size(0));x++)
     {
      if (dispMask.Get(x,y)==false) ci.Get(x,y) = bs::ColourRGB(0.5,0.5,1.0);
      else
      {
       bs::Vert xLow,xHigh,yLow,yHigh;

       if (dispMask.Get(math::Max<int32>(x-1,0),y)) xLow = pos.Get(math::Max<int32>(x-1,0),y);
                                               else xLow = pos.Get(x,y);
       if (dispMask.Get(math::Min<int32>(x+1,ci.Size(0)-1),y)) xHigh = pos.Get(math::Min<int32>(x+1,ci.Size(0)-1),y);
                                                          else xHigh = pos.Get(x,y);
       if (dispMask.Get(x,math::Max<int32>(y-1,0))) yLow = pos.Get(x,math::Max<int32>(y-1,0));
                                               else yLow = pos.Get(x,y);
       if (dispMask.Get(x,math::Min<int32>(y+1,ci.Size(1)-1))) yHigh = pos.Get(x,math::Min<int32>(y+1,ci.Size(1)-1));
                                                          else yHigh = pos.Get(x,y);

       xHigh -= xLow;
       yHigh -= yLow;
       if (math::IsZero(xHigh.LengthSqr())) xHigh = bs::Vert(1.0,0.0,0.0);
       if (math::IsZero(yHigh.LengthSqr())) yHigh = bs::Vert(0.0,1.0,0.0);

       bs::Vert norm;
       math::CrossProduct(xHigh,yHigh,norm);
       norm.Normalise();

       if (norm[2]<0.0) ci.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
                   else ci.Get(x,y) = bs::ColourRGB((1.0 + norm[0])*0.5,(1.0 + norm[1])*0.5,norm[2]);
      }
     }
    }


   // Save the image...
    if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImage(ci,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving needle map.");
    }
    mem::Free(ts);

   cyclops.EndProg();
  }
}

//------------------------------------------------------------------------------
