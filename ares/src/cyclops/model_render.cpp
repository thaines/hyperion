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


#include "cyclops/model_render.h"

//------------------------------------------------------------------------------
ModelRender::ModelRender(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),var(null<svt::Var*>())
{
 // Create default image and calibration...
  floatVar = new svt::Var(cyclops.Core());
  floatVar->Setup2D(320,240);
  bs::ColourRGB colourIni(00.0,0.0,0.0);
  floatVar->Add("rgb",colourIni);
  floatVar->Commit();
  floatVar->ByName("rgb",floatImage);
  
  depth.Resize(floatVar->Size(0),floatVar->Size(1));
  for (nat32 y=0;y<depth.Height();y++)
  {
   for (nat32 x=0;x<depth.Width();x++) depth.Get(x,y) = -math::Infinity<real32>();
  }  
 
  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);

  math::Identity(camera.camera);
  camera.radial.aspectRatio = 1.0;
  camera.radial.centre[0] = 160.0;
  camera.radial.centre[1] = 120.0;
  for (nat32 i=0;i<4;i++) camera.radial.k[i] = 0.0;
  camera.dim[0] = 320.0;
  camera.dim[1] = 240.0;
  
  camRays.Resize(floatVar->Size(0),floatVar->Size(1));
  //cam::CamToRays(camera,camRays);
  
  UpdateImage();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Render Mesh");
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

   but1->SetChild(lab1); lab1->Set("Load '.cam'...");
   but2->SetChild(lab2); lab2->Set("Add Mesh Needle...");
   but3->SetChild(lab3); lab3->Set("Save Image...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&ModelRender::Quit));
  canvas->OnResize(MakeCB(this,&ModelRender::Resize));
  but1->OnClick(MakeCB(this,&ModelRender::LoadCam));
  but2->OnClick(MakeCB(this,&ModelRender::AddModelNeedle));
  but3->OnClick(MakeCB(this,&ModelRender::SaveImage));
}

ModelRender::~ModelRender()
{
 delete win;
 delete var;
}

void ModelRender::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void ModelRender::Resize(gui::Base * obj,gui::Event * event)
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

void ModelRender::LoadCam(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera","*.cam",fn))
 {
  if (camera.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load cam file");
  }
  else
  {
   floatVar->Setup2D(nat32(camera.dim[0]),nat32(camera.dim[1]));
   floatVar->Commit();
   floatVar->ByName("rgb",floatImage);
   
   depth.Resize(floatVar->Size(0),floatVar->Size(1));
   for (nat32 y=0;y<depth.Height();y++)
   {
    for (nat32 x=0;x<depth.Width();x++) depth.Get(x,y) = -math::Infinity<real32>();
   }
   
   camRays.Resize(floatVar->Size(0),floatVar->Size(1));
   cam::CamToRays(camera,camRays,cyclops.BeginProg());
   cyclops.EndProg();
   
   
   UpdateImage();
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
  }
 }
}

void ModelRender::AddModelNeedle(gui::Base * obj,gui::Event * event)
{
 // First load a 3d model...
  sur::Mesh * mesh;

  str::String fn;
  if (cyclops.App().LoadFileDialog("Load .ply/.obj mesh file","*.ply,*.obj",fn)==false) return;

  time::Progress * prog = cyclops.BeginProg();
  prog->Report(0,3);

  cstr f = fn.ToStr();
  if (str::Compare(f + str::Length(f)-4,".obj")==0) mesh = file::LoadWavefront(f,prog);
                                               else mesh = file::LoadPly(f,prog);
  mem::Free(f);

  if (mesh==null<sur::Mesh*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load mesh");
   cyclops.EndProg();
   return;
  }


 // Then triangulate...
  prog->Report(1,3);
  mesh->Triangulate();
    
  
  math::Mat<3,3,real64> rot;
  {
   cam::Intrinsic intr;
   camera.camera.Decompose(intr,rot);
   LogDebug("Camera decomposition {intrinsic,rotation}" << LogDiv() << intr << LogDiv() << rot);
   math::Transpose(rot);
  }


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
 
    // Calculate the triangles normal...
     math::Vect<3,real64> dir;
     {
      math::Vect<3,real64> dirA;
      math::Vect<3,real64> dirB;
      for (nat32 i=0;i<3;i++)
      {
       dirA[i] = tri[1].Pos()[i] - tri[0].Pos()[i];
       dirB[i] = tri[2].Pos()[i] - tri[0].Pos()[i];
      }
      math::CrossProduct(dirA,dirB,dir);
     }
    
    // Rotate it into the camera space...
     math::Vect<3,real64> cDir;
     math::MultVect(rot,dir,cDir);
     if (math::IsZero(cDir.LengthSqr())) continue;
     cDir.Normalise();
     cDir[2] = math::Abs(cDir[2]); // *********************************************
     //if (cDir[2]<0.0) continue;
   
    // Render the triangle...
     bs::ColourRGB c(0.5*(1.0+cDir[0]),0.5*(1.0+cDir[1]),cDir[2]);
     cam::RenderTri(camera,camRays,floatImage,depth,tri[0].Pos(),c,tri[1].Pos(),c,tri[2].Pos(),c);
   }
  prog->Pop();
  cyclops.EndProg();


 // Clean up and redraw...
  delete mesh;
  UpdateImage();
  canvas->Redraw();
}

void ModelRender::SaveImage(gui::Base * obj,gui::Event * event)
{
 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Image",fn))
  {  
   // Save the image...
    if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImage(floatImage,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
    }
    mem::Free(ts); 
  }
}

void ModelRender::UpdateImage()
{
 if ((image.Size(0)!=floatImage.Size(0))||(image.Size(1)!=floatImage.Size(1)))
 {
  var->Setup2D(floatImage.Size(0),floatImage.Size(1));
  var->Commit();
  var->ByName("rgb",image);
 }

 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = floatImage.Get(x,y);
 }
}

//------------------------------------------------------------------------------
