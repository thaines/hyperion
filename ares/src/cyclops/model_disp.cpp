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


#include "cyclops/model_disp.h"

//------------------------------------------------------------------------------
ModelDisp::ModelDisp(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),var(null<svt::Var*>())
{
 // Create default image and calibration...
  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);

  pair.SetDefault(320,240);
  makeDisp.Reset(pair);
  RenderDisp();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Mesh to Disparity");
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

   but1->SetChild(lab1); lab1->Set("Load '.pcc'...");
   but2->SetChild(lab2); lab2->Set("Add Mesh...");
   but3->SetChild(lab3); lab3->Set("Save Disparity...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&ModelDisp::Quit));
  canvas->OnResize(MakeCB(this,&ModelDisp::Resize));
  but1->OnClick(MakeCB(this,&ModelDisp::LoadPair));
  but2->OnClick(MakeCB(this,&ModelDisp::AddModel));
  but3->OnClick(MakeCB(this,&ModelDisp::SaveDisp));
}

ModelDisp::~ModelDisp()
{
 delete win;
 delete var;
}

void ModelDisp::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void ModelDisp::Resize(gui::Base * obj,gui::Event * event)
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

void ModelDisp::LoadPair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Pair Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
  else
  {
   makeDisp.Reset(pair);
   RenderDisp();
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
  }
 }
}

void ModelDisp::AddModel(gui::Base * obj,gui::Event * event)
{
 // First load a 3d model...
  sur::Mesh * mesh;

  str::String fn;
  if (cyclops.App().LoadFileDialog("Load .ply/.obj mesh file","*.ply,*.obj",fn)==false) return;

  time::Progress * prog = cyclops.BeginProg();
  prog->Report(0,3);

  mesh = file::LoadMesh(fn,prog);


  if (mesh==null<sur::Mesh*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load mesh");
   cyclops.EndProg();
   return;
  }


 // Then triangulate...
  prog->Report(1,3);
  mesh->Triangulate();


 // Then iterate all triangles...
  prog->Report(2,3);
  ds::Array<sur::Face> face;
  mesh->GetFaces(face);

  ds::Array<sur::Vertex> tri(3);
  prog->Push();
   for (nat32 i=0;i<face.Size();i++)
   {
    prog->Report(i,face.Size());
    face[i].GetVertices(tri);
    makeDisp.Add(tri[0].Pos(),tri[1].Pos(),tri[2].Pos());
   }
  prog->Pop();
  cyclops.EndProg();


 // Clean up and redraw...
  delete mesh;
  RenderDisp();
  canvas->Redraw();
}

void ModelDisp::SaveDisp(gui::Base * obj,gui::Event * event)
{
 // Create the result svt...
  real32 dispIni = 0.0;
  bit maskIni = false;

  svt::Var * result = new svt::Var(cyclops.Core());
  result->Setup2D(makeDisp.Width(),makeDisp.Height());
  result->Add("disp",dispIni);
  result->Add("mask",maskIni);
  result->Commit();


 // Fill the result svt...
  svt::Field<real32> disp(result,"disp");
  svt::Field<bit> mask(result,"mask");

  makeDisp.GetDisp(disp);
  makeDisp.GetMask(mask);


 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Disparity Image",fn))
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

void ModelDisp::RenderDisp()
{
 if ((image.Size(0)!=makeDisp.Width())||(image.Size(1)!=makeDisp.Height()))
 {
  var->Setup2D(makeDisp.Width(),makeDisp.Height());
  var->Commit();
  var->ByName("rgb",image);
 }

 real32 minDisp = 0.0;
 real32 maxDisp = 0.0;
 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++)
  {
   if (makeDisp.Mask(x,y))
   {
    minDisp = math::Min(minDisp,makeDisp.Disp(x,y));
    maxDisp = math::Max(maxDisp,makeDisp.Disp(x,y));
   }
  }
 }

 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++)
  {
   if (makeDisp.Mask(x,y))
   {
    nat8 val = nat8(math::Clamp<real32>(255.0*(makeDisp.Disp(x,y)-minDisp)/(maxDisp-minDisp),0,255));
    image.Get(x,y) = bs::ColRGB(val,val,val);
   }
   else image.Get(x,y) = bs::ColRGB(0,0,255);
  }
 }
}

//------------------------------------------------------------------------------
