//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/anaglyph.h"

//------------------------------------------------------------------------------
Anaglyph::Anaglyph(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),var(null<svt::Var*>())
{
 // Create default images...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  bs::ColRGB colIni(0,0,0);

  leftVar = new svt::Var(cyclops.Core());
  leftVar->Setup2D(320,240);
  leftVar->Add("rgb",colourIni);
  leftVar->Commit();
  leftVar->ByName("rgb",leftImage);

  rightVar = new svt::Var(cyclops.Core());
  rightVar->Setup2D(320,240);
  rightVar->Add("rgb",colourIni);
  rightVar->Commit();
  rightVar->ByName("rgb",rightImage);

  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Anaglyph");
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
   offset = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   but1->SetChild(lab1); lab1->Set("Load Left...");
   but2->SetChild(lab2); lab2->Set("Load Right...");
   but3->SetChild(lab3); lab3->Set("Save Anaglyph...");
   lab4->Set("Tune Depth:");
   offset->Set("0");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(lab4,false);
   horiz1->AttachRight(offset,false);
   horiz1->AttachRight(but3,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&Anaglyph::Quit));
  canvas->OnResize(MakeCB(this,&Anaglyph::Resize));

  but1->OnClick(MakeCB(this,&Anaglyph::LoadLeft));
  but2->OnClick(MakeCB(this,&Anaglyph::LoadRight));
  offset->OnChange(MakeCB(this,&Anaglyph::OffsetChange));
  but3->OnClick(MakeCB(this,&Anaglyph::SaveAnaglyph));
}

Anaglyph::~Anaglyph()
{
 delete win;
 delete var;
}

void Anaglyph::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Anaglyph::Resize(gui::Base * obj,gui::Event * event)
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

void Anaglyph::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
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

   delete leftVar;
   leftVar = newVar;
   leftVar->ByName("rgb",leftImage);

  // Redraw...
   Update();
   canvas->Redraw();
 }
}

void Anaglyph::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
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

   delete rightVar;
   rightVar = newVar;
   rightVar->ByName("rgb",rightImage);

  // Redraw...
   Update();
   canvas->Redraw();
 }
}

void Anaglyph::OffsetChange(gui::Base * obj,gui::Event * event)
{
 Update();
 canvas->Redraw();
}

void Anaglyph::SaveAnaglyph(gui::Base * obj,gui::Event * event)
{
 // Only continue if image sizes match...
  if (leftImage.Size(1)!=rightImage.Size(1))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Image heights do not match");
   return;
  }


 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Image",fn))
  {
   // Create a tempory image...
    int32 shift = offset->Get().ToInt32();
    int32 width = math::Max(int32(leftImage.Size(0)),int32(rightImage.Size(0))+shift) - math::Min(shift,int32(0));
    int32 height = leftImage.Size(1);    
   
    svt::Var tempVar(image);
    tempVar.Setup2D(width,height);
    bs::ColourRGB colIni(0.0,0.0,0.0);
    tempVar.Add("rgb",colIni);
    tempVar.Commit();
    svt::Field<bs::ColourRGB> temp(&tempVar,"rgb");

   // Add the left image...
    int32 leftShift = -math::Min(shift,int32(0));
    for (nat32 y=0;y<leftImage.Size(1);y++)
    {
     for (nat32 x=0;x<leftImage.Size(0);x++)
     {
      bs::ColourRGB c = leftImage.Get(x,y);
      if (math::Equal(c.r,real32(1.0))&&
          math::Equal(c.g,real32(0.0))&&
          math::Equal(c.b,real32(1.0))) continue;
 
      temp.Get(x+leftShift,y).r = c.r;
     }
    }

   // Add the right image...
    int32 rightShift = math::Max(shift,int32(0));
    for (nat32 y=0;y<rightImage.Size(1);y++)
    {
     for (nat32 x=0;x<rightImage.Size(0);x++)
     {
      bs::ColourRGB c = rightImage.Get(x,y);
      if (math::Equal(c.r,real32(1.0))&&
          math::Equal(c.g,real32(0.0))&&
          math::Equal(c.b,real32(1.0))) continue;

      temp.Get(x+rightShift,y).g = c.g;
      temp.Get(x+rightShift,y).b = c.b;
     }
    }    


   // Save the image...
    if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImage(temp,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
    }
    mem::Free(ts); 
  }
}

void Anaglyph::Update()
{
 // Basic checks, get offset...
  if (leftImage.Size(1)!=rightImage.Size(1)) return;
  int32 shift = offset->Get().ToInt32();

 // Make sure the output image is large enough (We have a system with allowances on the horizontal to save on allocations.)...
  int32 width = math::Max(int32(leftImage.Size(0)),int32(rightImage.Size(0))+shift) - math::Min(shift,int32(0));
  int32 height = leftImage.Size(1);

  if ((int32(image.Size(1))!=height)||(int32(image.Size(0))<width)||(int32(image.Size(0))>width+200))
  {
   var->Setup2D(width+50,height);
   var->Commit();
   var->ByName("rgb",image);

   canvas->SetSize(image.Size(0),image.Size(1));
  }
  else
  {
   // Set it all to black - if resized then thats auto hence the else...
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = bs::ColRGB(0,0,0);
    }
  }

 // Add the left image...
  int32 leftShift = -math::Min(shift,int32(0));
  for (nat32 y=0;y<leftImage.Size(1);y++)
  {
   for (nat32 x=0;x<leftImage.Size(0);x++)
   {
    bs::ColourRGB c = leftImage.Get(x,y);
    if (math::Equal(c.r,real32(1.0))&&
        math::Equal(c.g,real32(0.0))&&
        math::Equal(c.b,real32(1.0))) continue;

    image.Get(x+leftShift,y).r = nat8(math::Clamp<real32>(255.0*c.r,0,255));
   }
  }

 // Add the right image...
  int32 rightShift = math::Max(shift,int32(0));
  for (nat32 y=0;y<rightImage.Size(1);y++)
  {
   for (nat32 x=0;x<rightImage.Size(0);x++)
   {
    bs::ColourRGB c = rightImage.Get(x,y);
    if (math::Equal(c.r,real32(1.0))&&
        math::Equal(c.g,real32(0.0))&&
        math::Equal(c.b,real32(1.0))) continue;

    image.Get(x+rightShift,y).g = nat8(math::Clamp<real32>(255.0*c.g,0,255));
    image.Get(x+rightShift,y).b = nat8(math::Clamp<real32>(255.0*c.b,0,255));
   }
  }
}

//------------------------------------------------------------------------------
