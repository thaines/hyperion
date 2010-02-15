//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/lighting.h"

//------------------------------------------------------------------------------
Lighting::Lighting(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
needleVar(null<svt::Var*>()),origVar(null<svt::Var*>()),imageVar(null<svt::Var*>())
{
 // Create default images maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);

  needleVar = new svt::Var(cyclops.Core());
  needleVar->Setup2D(320,240);
  needleVar->Add("rgb",colourIni);
  needleVar->Commit();
  needleVar->ByName("rgb",needleImage);

  origVar = new svt::Var(cyclops.Core());
  origVar->Setup2D(320,240);
  origVar->Add("rgb",colourIni);
  origVar->Commit();
  origVar->ByName("rgb",origImage);


 // Create default image...
  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colourIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);
  
  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);
  
  
 // Make default camera response function flat...
  crf.SetMult();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Image Relighter");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   vert1->AttachBottom(horiz2,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Needle...");
   but2->SetChild(lab2); lab2->Set("Load Image...");
   but3->SetChild(lab3); lab3->Set("Render");
   but4->SetChild(lab4); lab4->Set("Save Rendered...");
   but5->SetChild(lab5); lab5->Set("Load crf...");
   
   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Needle Map");
   viewSelect->Append("Original Image");
   viewSelect->Append("Corrected Image");
   viewSelect->Append("Albedo Map");
   viewSelect->Append("Relight Image");
   viewSelect->Set(4);

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but5,false);
   horiz1->AttachRight(viewSelect,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
    
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   origLight = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   newLight = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   albLab = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   lab6->Set(" Original Light");
   origLight->Set("(0.0,0.0,1.0)");
   lab7->Set(" New Light");
   newLight->Set("(0.0,0.0,1.0)");
   albLab->Set("-");
  
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(origLight,false);
   horiz2->AttachRight(lab7,false);
   horiz2->AttachRight(newLight,false);
   horiz2->AttachRight(albLab,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&Lighting::Quit));
  canvas->OnResize(MakeCB(this,&Lighting::Resize));
  but1->OnClick(MakeCB(this,&Lighting::LoadNeedle));
  but2->OnClick(MakeCB(this,&Lighting::LoadOrig));
  but3->OnClick(MakeCB(this,&Lighting::Render));
  but4->OnClick(MakeCB(this,&Lighting::SaveRender));
  but5->OnClick(MakeCB(this,&Lighting::LoadCRF));
}

Lighting::~Lighting()
{
 delete win;
 delete needleVar;
 delete origVar;
 delete imageVar;
 delete imgVar;
}

void Lighting::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Lighting::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(img.Size(0),img.Size(1))),bs::Pos(sx,sy),img);

 // Update...
  canvas->Update();
}

void Lighting::LoadNeedle(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Needle Map","*.bmp,*.jpg,*.png,*.tif",fn))
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
   delete needleVar;
   needleVar = tempVar;
   needleVar->ByName("rgb",needleImage);

  // Update the display...
   Update();
 }
}

void Lighting::LoadOrig(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Original Image","*.bmp,*.jpg,*.png,*.tif",fn))
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
   delete origVar;
   origVar = tempVar;
   origVar->ByName("rgb",origImage);

  // Update the display...
   Update();
 }
}

void Lighting::LoadCRF(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera Response Function","*.crf",fn))
 {
  // Load...
   if (crf.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load function");
   }
 }
}

void Lighting::Render(gui::Base * obj,gui::Event * event)
{
 Update();
}

void Lighting::SaveRender(gui::Base * obj,gui::Event * event)
{
 str::String fn("");
 if (cyclops.App().SaveFileDialog("Save Image",fn))
 {
  if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
  cstr ts = fn.ToStr();
  if (!filter::SaveImage(image,ts,true))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
  }
  mem::Free(ts); 
 }
}

void Lighting::Update()
{
 // Check sizes match, adjust/fail as scenario dictates...
  if ((needleImage.Size(0)!=origImage.Size(0))||(needleImage.Size(1)!=origImage.Size(1))) return;
  if ((image.Size(0)!=origImage.Size(0))||(image.Size(1)!=origImage.Size(1)))
  {
   imageVar->Setup2D(origImage.Size(0),origImage.Size(1));
   imageVar->Commit();
   imageVar->ByName("rgb",image);
  }
  if ((img.Size(0)!=origImage.Size(0))||(img.Size(1)!=origImage.Size(1)))
  {
   imgVar->Setup2D(origImage.Size(0),origImage.Size(1));
   imgVar->Commit();
   imgVar->ByName("rgb",img);
  }  


 // Get stuff the user provides...
  bs::Normal origL;
  {
   str::String s = origLight->Get();
   str::String::Cursor cur = s.GetCursor();
   cur.ClearError();
   cur >> origL;
   if (cur.Error()) origL = bs::Normal(0.0,0.0,1.0);
   origL.Normalise();
  }
   
  bs::Normal newL;
  {
   str::String s = newLight->Get();
   str::String::Cursor cur = s.GetCursor();
   cur.ClearError();
   cur >> newL;
   if (cur.Error()) newL = bs::Normal(0.0,0.0,1.0);
   newL.Normalise();
  }

 
 // Switch on mode and render correct image...
  switch(viewSelect->Get())
  {
   case 0: // Needle map
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = needleImage.Get(x,y);
    }
   }
   break;
   case 1: // Original image
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = origImage.Get(x,y);
    }
   }
   break;
   case 2: // Corrected image
   {
    real32 max = 0.01;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) 
     {
      real32 val = crf((origImage.Get(x,y).r+origImage.Get(x,y).g+origImage.Get(x,y).b)/3.0);
      max = math::Max(max,val);
      image.Get(x,y) = bs::ColourRGB(val,val,val);
     }
    }
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) /= max;
    }
   }
   break;
   case 3: // Albedo map
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      bs::ColourRGB & nc = needleImage.Get(x,y);
      bs::Normal norm(nc.r*2.0-1.0,nc.g*2.0-1.0,nc.b);
      real32 dot = norm*origL;
      if (dot>0.0)
      {
       real32 l = crf((origImage.Get(x,y).r + origImage.Get(x,y).g + origImage.Get(x,y).b)/3.0);
       real32 albedo = l/dot;
       if (albedo<=1.0) image.Get(x,y) = bs::ColourRGB(albedo,albedo,albedo);
                   else image.Get(x,y) = bs::ColourRGB(0.25,0.0,0.0);
      }
      else
      {
       image.Get(x,y) = bs::ColourRGB(0.0,0.0,0.25);
      }
     }
    }
   }
   break;
   case 4: // Relight image
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      bs::ColourRGB & nc = needleImage.Get(x,y);
      bs::Normal norm(nc.r*2.0-1.0,nc.g*2.0-1.0,nc.b);
      real32 dot = norm*origL;
      if (dot>0.0)
      {
       real32 mult = (norm*newL)/dot;
       image.Get(x,y) = bs::ColourRGB(math::Clamp<real32>(origImage.Get(x,y).r*mult,0.0,1.0),
                                      math::Clamp<real32>(origImage.Get(x,y).g*mult,0.0,1.0),
                                      math::Clamp<real32>(origImage.Get(x,y).b*mult,0.0,1.0));
      }
      else
      {
       image.Get(x,y) = bs::ColourRGB(0.0,0.0,0.25);
      }
     }
    }
   }
   break;
  }
  
 
 // Calculate and output the albedo of the image assuming constant albedo...
 {
  // Need actual normal map and corrected luminence map...
   bs::Normal normIni(0.0,0.0,1.0);
   real32 realIni = 0.0;
   
   svt::Var temp(needleImage);
   temp.Add("needle",normIni);
   temp.Add("l",realIni);
   temp.Commit();
   svt::Field<bs::Normal> needle(&temp,"needle");
   svt::Field<real32> l(&temp,"l");
   
   for (nat32 y=0;y<needle.Size(1);y++)
   {
    for (nat32 x=0;x<needle.Size(0);x++)
    {
     l.Get(x,y) = crf((origImage.Get(x,y).r + origImage.Get(x,y).g + origImage.Get(x,y).b)/3.0);
     needle.Get(x,y)[0] = needleImage.Get(x,y).r*2.0-1.0;
     needle.Get(x,y)[1] = needleImage.Get(x,y).g*2.0-1.0;
     needle.Get(x,y)[2] = needleImage.Get(x,y).b;
    }
   }
   
  // Calculate...
   real32 a = crf.Inverse(sfs::AlbedoEstimate(l,needle,origL));
   
  
  // Output...
   str::String s;
   s << a;
   albLab->Set(s);
 }


 // Copy from image to img...
  for (nat32 y=0;y<img.Size(1);y++)
  {
   for (nat32 x=0;x<img.Size(0);x++) img.Get(x,y) = image.Get(x,y);
  }


 // Redraw...
  canvas->SetSize(img.Size(0),img.Size(1));
  canvas->Redraw();
}

//------------------------------------------------------------------------------
