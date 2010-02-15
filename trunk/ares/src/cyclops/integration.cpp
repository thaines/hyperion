//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/integration.h"

//------------------------------------------------------------------------------
Integration::Integration(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
dataVar(null<svt::Var*>()),visVar(null<svt::Var*>())
{
 // Create default images...
  bs::ColRGB colIni(0,0,0);
  bs::Normal needleIni(0.0,0.0,0.0);

  dataVar = new svt::Var(cyclops.Core());
  dataVar->Setup2D(320,240);
  dataVar->Add("needle",needleIni);
  dataVar->Commit();
  dataVar->ByName("needle",needle);

  visVar = new svt::Var(cyclops.Core());
  visVar->Setup2D(320,240);
  visVar->Add("rgb",colIni);
  visVar->Commit();
  visVar->ByName("rgb",visible);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Needle Map Integration");
  cyclops.App().Attach(win);
  win->SetSize(needle.Size(0)+8,needle.Size(1)+128);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Needle Map...");
   but2->SetChild(lab2); lab2->Set("Save Mesh...");
   
   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   

   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);
   
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   scale = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   zMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   limit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   
   lab3->Set("  Mesh Scaler:");
   lab4->Set("  Z Additional Scaler:");
   lab5->Set("  Limit:");
   
   scale->Set("0.01");
   zMult->Set("1.0");
   limit->Set("10.0");
   
   horiz2->AttachRight(lab3,false);
   horiz2->AttachRight(scale,false);
   horiz2->AttachRight(lab4,false);
   horiz2->AttachRight(zMult,false);
   horiz2->AttachRight(lab5,false);
   horiz2->AttachRight(limit,false);

                           
   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(needle.Size(0),needle.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&Integration::Quit));
  canvas->OnResize(MakeCB(this,&Integration::Resize));

  but1->OnClick(MakeCB(this,&Integration::LoadNeedle));
  but2->OnClick(MakeCB(this,&Integration::SaveMesh));
}

Integration::~Integration()
{
 delete win;
 delete dataVar;
 delete visVar; 
}

void Integration::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Integration::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - visible.Size(0))/2;
  nat32 sy = (canvas->P().Height() - visible.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(visible.Size(0),visible.Size(1))),bs::Pos(sx,sy),visible);

 // Update...
  canvas->Update();
}

void Integration::LoadNeedle(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Needle Map","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * newVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (newVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

   dataVar->Setup2D(newVar->Size(0),newVar->Size(1));
   dataVar->Commit();
   dataVar->ByName("needle",needle);
   
   svt::Field<bs::ColourRGB> in(newVar,"rgb");
   for (nat32 y=0;y<needle.Size(1);y++)
   {
    for (nat32 x=0;x<needle.Size(0);x++)
    {
     needle.Get(x,y)[0] = 2.0*(in.Get(x,y).r-0.5);
     needle.Get(x,y)[1] = 2.0*(in.Get(x,y).g-0.5);
     needle.Get(x,y)[2] = in.Get(x,y).b;

     if (!math::IsZero(needle.Get(x,y).LengthSqr()))
     {
      needle.Get(x,y).Normalise();
     }
    }
   }

   delete newVar;


  // Redraw...
   UpdateView();
 }
}

void Integration::SaveMesh(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("*.ply");
  if (cyclops.App().SaveFileDialog("Save Mesh",fn))
  {
   // Get parameters...
    real32 sca = scale->GetReal(0.001);
    real32 zM  = zMult->GetReal(1.0);
    real32 lim = limit->GetReal(10.0);
    real32 cap = lim;
   
   // Setup the integration object...  
    inf::IntegrateBP ibp(needle.Size(0),needle.Size(1));
    ibp.SetVal(needle.Size(0)/2,needle.Size(1)/2,0.0,100.0);
    ibp.SetIters(needle.Size(0)+needle.Size(1));
   
   // Fill in the data from the needle map...
    for (nat32 y=0;y<needle.Size(1);y++)
    {
     for (nat32 x=0;x<needle.Size(0);x++)
     {
      bs::Normal n = needle.Get(x,y);
      if ((!math::IsZero(n[2]))&&(n[2]>0.0))
      {
       n /= n[2];
       ibp.SetRel(x,y,0,1.0,math::Clamp( n[0]*zM,real32(-lim),real32(lim)),1.0);
       ibp.SetRel(x,y,1,1.0,math::Clamp( n[1]*zM,real32(-lim),real32(lim)),1.0);
       ibp.SetRel(x,y,2,1.0,math::Clamp(-n[0]*zM,real32(-lim),real32(lim)),1.0);
       ibp.SetRel(x,y,3,1.0,math::Clamp(-n[1]*zM,real32(-lim),real32(lim)),1.0);
      }
     }
    }


   // Run the algorithm...
    ibp.Run(cyclops.BeginProg());
    cyclops.EndProg();


   // Extract the results into a ply file...
    file::Ply ply;
    // Vertices...
     ds::Array2D<nat32> ind(needle.Size(0),needle.Size(1));
     for (nat32 y=0;y<needle.Size(1);y++)
     {
      for (nat32 x=0;x<needle.Size(0);x++)
      {
       bs::Vert v;
       v[0] = x*sca;
       v[1] = y*sca;
       real32 z = ibp.Expectation(x,y);
       if (math::IsFinite(z)) v[2] = math::Clamp(z*sca,real32(-cap),real32(cap));
                         else v[2] = -cap;

       bs::Tex2D uv;
       uv[0] = x / real32(needle.Size(0));
       uv[1] = y / real32(needle.Size(1));
       ind.Get(x,y) = ply.Add(v,uv);
      }
     }
  
    // Faces...
     for (nat32 y=0;y<needle.Size(1)-1;y++)
     {
      for (nat32 x=0;x<needle.Size(0)-1;x++)
      {
       ply.Add(ind.Get(x,y));
       ply.Add(ind.Get(x+1,y));
       ply.Add(ind.Get(x+1,y+1));
       ply.Add(ind.Get(x,y+1));
       ply.Face();
      }
     }


   // Save the file...
    if (!fn.EndsWith(".ply")) fn += ".ply";
    ply.Save(fn,true,cyclops.BeginProg());
    cyclops.EndProg();
  }
}

void Integration::UpdateView()
{
 // Check size...
  if ((visible.Size(0)!=needle.Size(0))||(visible.Size(1)!=needle.Size(1)))
  {
   visVar->Setup2D(needle.Size(0),needle.Size(1));
   visVar->Commit();
   visVar->ByName("rgb",visible);
   
   canvas->SetSize(needle.Size(0),needle.Size(1));
  }


 // Iterate the pixels, and fill them in...
  for (nat32 y=0;y<visible.Size(1);y++)
  {
   for (nat32 x=0;x<visible.Size(0);x++)
   {
    if (needle.Get(x,y).Length()>0.1)
    {
     visible.Get(x,y).r = (byte)math::Clamp(((needle.Get(x,y)[0]+1.0)*0.5)*255.0,0.0,255.0);
     visible.Get(x,y).g = (byte)math::Clamp(((needle.Get(x,y)[1]+1.0)*0.5)*255.0,0.0,255.0);
     visible.Get(x,y).b = (byte)math::Clamp(needle.Get(x,y)[2]*255.0,0.0,255.0);
    }
    else
    {
     visible.Get(x,y).r = 0;
     visible.Get(x,y).g = 0;
     visible.Get(x,y).b = 0;
    }
   }
  }
  
 
 // Redraw...
  canvas->Redraw();
}

//------------------------------------------------------------------------------
