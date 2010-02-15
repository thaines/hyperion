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


#include "cyclops/mesh_ops.h"

//------------------------------------------------------------------------------
MeshOps::MeshOps(Cyclops & cyc)
:cyclops(cyc),
win(null<gui::Window*>()),mesh(null<sur::Mesh*>()),
stats(null<gui::Label*>())
{
 // Create empty mesh...
  mesh = new sur::Mesh(&cyclops.TT());


 // Create window...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Mesh Wibbalizer");
  cyclops.App().Attach(win);
  win->SetSize(1,1);

  gui::Vertical * vert = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
  win->SetChild(vert);


 // Create load/save row...
  gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

  but1->SetChild(lab1); lab1->Set("Load Mesh...");
  but2->SetChild(lab2); lab2->Set("Save Mesh...");

  gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
  vert->AttachBottom(horiz1,false);
  horiz1->AttachRight(but1,false);
  horiz1->AttachRight(but2,false);


 // Create mesh stats row...
  stats = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  vert->AttachBottom(stats,false);
  
 // Create spacer...
  vert->AttachBottom(static_cast<gui::Label*>(cyclops.Fact().Make("Label")),false);


 // Create scaling row...
  gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  but5->SetChild(lab9); lab9->Set("Scale");
  
  scaleMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
  scaleMult->Set("100.0");
  scaleMult->SetSize(64,24);
  
  gui::Horizontal * horiz4 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
  vert->AttachBottom(horiz4,false);
  horiz4->AttachRight(but5,false);
  horiz4->AttachRight(scaleMult,false);

  
 // Create triangulate row...
  gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  but6->SetChild(lab10); lab10->Set("Triangulate");
  
  gui::Horizontal * horiz5 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
  vert->AttachBottom(horiz5,false);
  horiz5->AttachRight(but6,false);


 // Create flat subdivide row...


 // Create catmull-clark subdivide row...


 // Create remove duplicates row...
  gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  but4->SetChild(lab7); lab7->Set("Remove Duplicates");
  lab8->Set("  Range:");
  
  remDupRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
  remDupRange->Set("0.001");
  remDupRange->SetSize(64,24);

  gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
  vert->AttachBottom(horiz3,false);
  horiz3->AttachRight(but4,false);
  horiz3->AttachRight(lab8,false);
  horiz3->AttachRight(remDupRange,false);


 // Create simplify row...
  gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  but3->SetChild(lab3); lab3->Set("Simplify");
  lab4->Set("  Factor/Vertices:");
  lab5->Set("  Merge Dist:");
  lab6->Set("  Boundary Cost:");
  
  simplifyAmount = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
  simplifyAmount->Set("0.25");
  simplifyAmount->SetSize(48,24);
  simplifyRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
  simplifyRange->Set("0.0");
  simplifyRange->SetSize(48,24);
  simplifyEdge = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
  simplifyEdge->Set("0.5");
  simplifyEdge->SetSize(48,24);

  gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
  vert->AttachBottom(horiz2,false);
  horiz2->AttachRight(but3,false);
  horiz2->AttachRight(lab4,false);
  horiz2->AttachRight(simplifyAmount,false);
  horiz2->AttachRight(lab5,false);
  horiz2->AttachRight(simplifyRange,false);
  horiz2->AttachRight(lab6,false);
  horiz2->AttachRight(simplifyEdge,false);


 // Create discontinuity removing row...


 // Create smoothing row...



 // Attach handlers...
  win->OnDeath(MakeCB(this,&MeshOps::Quit));

  but1->OnClick(MakeCB(this,&MeshOps::LoadMesh));
  but2->OnClick(MakeCB(this,&MeshOps::SaveMesh));
  but3->OnClick(MakeCB(this,&MeshOps::Simplify));
  but4->OnClick(MakeCB(this,&MeshOps::RemDup));
  but5->OnClick(MakeCB(this,&MeshOps::Scale));
  but6->OnClick(MakeCB(this,&MeshOps::Triangulate));


 // Update the stats...
  UpdateStats();
}

MeshOps::~MeshOps()
{
 delete win;
 delete mesh;
}

void MeshOps::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void MeshOps::LoadMesh(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Reload Mesh","*.obj,*.ply",fn))
 {
  sur::Mesh * newMesh = file::LoadMesh(fn,cyclops.BeginProg(),&cyclops.TT());
  cyclops.EndProg();
  
  if (newMesh)
  {
   delete mesh;
   mesh = newMesh;
   UpdateStats();
  }
  else
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error loading a mesh.");
  }
 }
}

void MeshOps::SaveMesh(gui::Base * obj,gui::Event * event)
{
 str::String fn(".ply");
 if (cyclops.App().SaveFileDialog("Save mesh...",fn))
 {
  if ((!fn.EndsWith(".obj"))&&(!fn.EndsWith(".ply"))) fn += ".ply";
  
  bit res = file::SaveMesh(*mesh,fn,true,cyclops.BeginProg());
  cyclops.EndProg();
  
  if (!res)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the mesh.");
   return;
  }
 }
}

void MeshOps::RemDup(gui::Base * obj,gui::Event * event)
{
 // Get the range...
  real32 range = remDupRange->Get().ToReal32();
  
 // Do the simplification...
  if (mesh)
  {
   sur::RemDups(mesh,range,cyclops.BeginProg());
   cyclops.EndProg();
  }
  
 // And finally update the stats...
  UpdateStats();  
}

void MeshOps::Simplify(gui::Base * obj,gui::Event * event)
{
 // Get the amount of simplification to do...
  real32 fact = simplifyAmount->Get().ToReal32();
  real32 range = simplifyRange->Get().ToReal32();
  real32 edgeCost = simplifyEdge->Get().ToReal32();
 
 // Do the simplification...
  if (mesh)
  {
   sur::Simplify simp;
   simp.Set(mesh);
   simp.SetMerge(range);
   simp.SetEdge(edgeCost);
   
   if (fact<1.0) simp.RunP(fact,cyclops.BeginProg());
            else simp.Run(nat32(fact),cyclops.BeginProg());
   cyclops.EndProg();
  }

 // And finally update the stats...
  UpdateStats();
}

void MeshOps::Scale(gui::Base * obj,gui::Event * event)
{
 // Get the multipler for the scaling...
  real32 mult = scaleMult->Get().ToReal32();
 
 // Scale every vertex by the scaler...
  if (mesh)
  {
   time::Progress * prog = cyclops.BeginProg();

   ds::Array<sur::Vertex> verts;
   mesh->GetVertices(verts);

   for (nat32 i=0;i<verts.Size();i++)
   {
    prog->Report(i,verts.Size());
    
    verts[i].Pos() *= mult;
   }
   
   cyclops.EndProg();
  }

 // And finally update the stats...
  UpdateStats();
}

void MeshOps::Triangulate(gui::Base * obj,gui::Event * event)
{
 cyclops.BeginProg();
 if (mesh) mesh->Triangulate();
 cyclops.EndProg();
 UpdateStats();
}

void MeshOps::UpdateStats()
{
 str::String s;

 s << "Verts = " << mesh->VertexCount() << "; Edges = " << mesh->EdgeCount() << "; Faces = " << mesh->FaceCount();

 s << "; V Props = {";
 for (nat32 i=0;i<mesh->CountVertProp();i++)
 {
  if (i!=0) s << ",";
  s << cyclops.TT().Str(mesh->NameVertProp(i));
 }
 s << "};";

 stats->Set(s);
}

//------------------------------------------------------------------------------
