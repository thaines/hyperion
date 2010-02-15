//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/calibration.h"

//------------------------------------------------------------------------------
Calibration::Calibration(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),resValid(false),var(null<svt::Var*>())
{
 // Create default image...
  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);

  pos[0] = 160.0;
  pos[1] = 120.0;


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Camera Calibration");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+64);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   vert1->AttachBottom(horiz2,false);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but10 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   worldX = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   worldY = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   worldZ = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   quality = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Store");
   but10->SetChild(lab10); lab10->Set("Un-store");
   lab6->Set("    X:");
   lab7->Set("Y:");
   lab8->Set("Z:");
   worldX->SetSize(64,24);
   worldY->SetSize(64,24);
   worldZ->SetSize(64,24);
   worldX->Set("0.0");
   worldY->Set("0.0");
   worldZ->Set("0.0");

   lab9->Set("    Quality:");
   quality->Append("low");
   quality->Append("normal");
   quality->Append("high");
   quality->Set(1);

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(lab6,false);
   horiz1->AttachRight(worldX,false);
   horiz1->AttachRight(lab7,false);
   horiz1->AttachRight(worldY,false);
   horiz1->AttachRight(lab8,false);
   horiz1->AttachRight(worldZ,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but10,false);
   horiz1->AttachRight(lab9,false);
   horiz1->AttachRight(quality,false);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but11 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but12 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but13 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab12 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab13 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   detail = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but3->SetChild(lab3); lab3->Set("Calculate");
   but4->SetChild(lab4); lab4->Set("Save '.icd'...");
   but5->SetChild(lab5); lab5->Set("Save '.cam'...");
   but11->SetChild(lab11); lab11->Set("Save State...");
   but12->SetChild(lab12); lab12->Set("Append State...");
   but13->SetChild(lab13); lab13->Set("Clear State");
   detail->Set("    ");

   horiz2->AttachRight(but3,false);
   horiz2->AttachRight(but4,false);
   horiz2->AttachRight(but5,false);
   horiz2->AttachRight(detail,false);
   horiz2->AttachRight(but11,false);
   horiz2->AttachRight(but12,false);
   horiz2->AttachRight(but13,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&Calibration::Quit));
  canvas->OnResize(MakeCB(this,&Calibration::Resize));
  canvas->OnClick(MakeCB(this,&Calibration::Click));
  canvas->OnMove(MakeCB(this,&Calibration::Move));
  but1->OnClick(MakeCB(this,&Calibration::LoadImage));
  but2->OnClick(MakeCB(this,&Calibration::Add));
  but3->OnClick(MakeCB(this,&Calibration::Calculate));
  but4->OnClick(MakeCB(this,&Calibration::SaveIntrinsic));
  but5->OnClick(MakeCB(this,&Calibration::SaveCamera));
  but10->OnClick(MakeCB(this,&Calibration::Rem));
  but11->OnClick(MakeCB(this,&Calibration::SaveState));
  but12->OnClick(MakeCB(this,&Calibration::AppendState));
  but13->OnClick(MakeCB(this,&Calibration::ClearState));
}

Calibration::~Calibration()
{
 delete win;
 delete var;
}

void Calibration::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Calibration::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // Render the stored positions...
  ds::List<Match>::Cursor targ = data.FrontPtr();
  while (!targ.Bad())
  {
   RenderPoint(targ->image,bs::ColourRGB(1.0,0.0,0.0));
   ++targ;
  }

 // Render the user selected position...
  RenderPoint(pos,bs::ColourRGB(1.0,0.0,1.0));

 // Update...
  canvas->Update();
}

void Calibration::Click(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);

  real32 sx = (canvas->P().Width() - image.Size(0))/2;
  real32 sy = (canvas->P().Height() - image.Size(1))/2;

 // Snap the point to the new position...
  pos[0] = mbe->x - sx;
  pos[1] = mbe->y - sy;

 // Redraw the display...
  canvas->Redraw();
}

void Calibration::Move(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;

  real32 sx = (canvas->P().Width() - image.Size(0))/2;
  real32 sy = (canvas->P().Height() - image.Size(1))/2;

 // Snap the point to the new position...
  pos[0] = mme->x - sx;
  pos[1] = mme->y - sy;

 // Redraw the display...
  canvas->Redraw();
}

void Calibration::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Image loaded, but its not in a format suitable for fast display - convert...
   var->Setup2D(floatVar->Size(0),floatVar->Size(1));
   var->Commit();
   var->ByName("rgb",image);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     image.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;

   pos[0] = real32(image.Size(0))/2.0;
   pos[1] = real32(image.Size(1))/2.0;


  // Refresh the display...
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
 }
}

void Calibration::Add(gui::Base * obj,gui::Event * event)
{
 // Get the world coordinates...
  real32 x,y,z;

  str::String s = worldX->Get();
  str::String::Cursor c = s.GetCursor();
  c >> x;
  if (c.Error()) x = 0.0;

  s = worldY->Get();
  c = s.GetCursor();
  c >> y;
  if (c.Error()) y = 0.0;

  s = worldZ->Get();
  c = s.GetCursor();
  c >> z;
  if (c.Error()) z = 0.0;


 // Create and store the match...
  Match match;
   match.world[0] = x;
   match.world[1] = y;
   match.world[2] = z;
   match.image[0] = pos[0];
   match.image[1] = pos[1];
  data.AddBack(match);
}

void Calibration::Rem(gui::Base * obj,gui::Event * event)
{
 if (data.Size()!=0) data.RemBack();
 canvas->Redraw();
}

void Calibration::Calculate(gui::Base * obj,gui::Event * event)
{
 // Create calculation class, fill it with details...
  cam::CalculateCamera cc;

  ds::List<Match>::Cursor targ = data.FrontPtr();
  while (!targ.Bad())
  {
   bs::Pnt img = targ->image;
   img[1] = image.Size(1)-1.0 - img[1];
   cc.AddMatch(targ->world,img);
   ++targ;
  }

  switch (quality->Get())
  {
   case 0: cc.SetQuality(cam::CalculateCamera::low); break;
   case 1: cc.SetQuality(cam::CalculateCamera::normal); break;
   case 2: cc.SetQuality(cam::CalculateCamera::high); break;
  }


 // Do calculation...
  cc.Calculate(cyclops.BeginProg());
  cyclops.EndProg();


 // Extract results into local data structures, fill in the detail data
 // structure, etc...
  {
   real64 res = cc.GetResidual();

   str::String s;
   switch (cc.GetQuality())
   {
    case cam::CalculateCamera::failure: s << "Failed."; resValid = false; break;
    case cam::CalculateCamera::low: s << "Low Quality, residual = " << res; resValid = true; break;
    case cam::CalculateCamera::normal: s << "Normal Quality, residual = " << res; resValid = true; break;
    case cam::CalculateCamera::high: s << "High Quality, residual = " << res; resValid = true; break;
   }
   detail->Set(s);
  }

  radial = cc.GetRadial();
  camera = cc.GetCamera();
}

void Calibration::SaveCamera(gui::Base * obj,gui::Event * event)
{
 if (resValid==false)
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"Calibration has not been successfully done.");
  return;
 }

 // Get the filename...
  str::String fn(".cam");
  if (cyclops.App().SaveFileDialog("Save Camera Projection...",fn))
  {
   if (!fn.EndsWith(".cam")) fn += ".cam";
   // Save the file...
    cam::CameraFull cc;
     cc.camera = camera;
     cc.radial = radial;
     cc.dim[0] = image.Size(0);
     cc.dim[1] = image.Size(1);

    if (!cc.Save(fn,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
}

void Calibration::SaveIntrinsic(gui::Base * obj,gui::Event * event)
{
 if (resValid==false)
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"Calibration has not been successfully done.");
  return;
 }

 // Get the filename...
  str::String fn(".icd");
  if (cyclops.App().SaveFileDialog("Save Intrinsic Calibration...",fn))
  {
   if (!fn.EndsWith(".icd")) fn += ".icd";
   // Save the file...
    cam::CameraCalibration cc;
     math::Mat<3,3,real64> rot;
     camera.Decompose(cc.intrinsic,rot);
     cc.radial = radial;
     cc.dim[0] = image.Size(0);
     cc.dim[1] = image.Size(1);

    if (!cc.Save(fn,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
}

void Calibration::SaveState(gui::Base * obj,gui::Event * event)
{
 str::String fn(".xml");
 if (cyclops.App().SaveFileDialog("Save State...",fn))
 {
  if (!fn.EndsWith(".xml")) fn += ".xml";
 
  // Create an xml file containning all the stored matches. Additionally store 
  // a residual with each match, so the dodgy once are obvious...
   // Create...
    str::TokenTable tt;
    bs::Element root(tt,"calibration_data");
    
   // Fill...
    ds::List<Match>::Cursor targ = data.FrontPtr();
    while (!targ.Bad())
    {
     bs::Element * match = root.NewChild("match");
     
     match->SetAttribute("ix",targ->image[0]);
     match->SetAttribute("iy",image.Size(1)-1.0 - targ->image[1]);
     match->SetAttribute("wx",targ->world[0]);
     match->SetAttribute("wy",targ->world[1]);
     match->SetAttribute("wz",targ->world[2]);
     
     {
      // Project the world coordinate to undistorted image space...
       math::Vect<4,real64> worldExt;
       for (nat32 i=0;i<3;i++) worldExt[i] = targ->world[i];
       worldExt[3] = 1.0;
       
       math::Vect<3,real64> imageExt;
       math::MultVect(camera,worldExt,imageExt);
       imageExt /= imageExt[2];
     
      // Apply the radial distortion...
       radial.Dis(imageExt,imageExt);
      
      // The reisudal is then the distance between the image coordiante and the calculated image coordinate...
       real32 dx = imageExt[0] - targ->image[0];
       real32 dy = imageExt[1] - (image.Size(1)-1.0 - targ->image[1]);
       real32 residual = math::Sqrt(math::Sqr(dx)+math::Sqr(dy));
       match->SetAttribute("residual",residual);
     }
   
     ++targ;
    }
 
   // Save...
    if (!file::SaveXML(&root,fn,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
    }
 }
}

void Calibration::AppendState(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select State.","*.xml",fn))
 {
  str::TokenTable tt;
  bs::Element * root = file::LoadXML(tt,fn);
  if (root)
  {
   for (nat32 i=0;i<root->ElementCount("match");i++)
   {
    bs::Element * match = root->GetElement("match",i);
    
    Match m;
    m.world[0] = match->GetReal("wx",0.0);
    m.world[1] = match->GetReal("wy",0.0); 
    m.world[2] = match->GetReal("wz",0.0); 
    m.image[0] = match->GetReal("ix",0.0);
    m.image[1] = image.Size(1)-1.0 - match->GetReal("iy",0.0);
    
    data.AddBack(m);
   }
   
   canvas->Redraw();
   delete root;
  }
  else
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error loading the xml file.");
  }
 }
}

void Calibration::ClearState(gui::Base * obj,gui::Event * event)
{
 if (cyclops.App().ChoiceDialog(gui::App::QuestYesNo,"Clear all current matches?"))
 {
  data.Reset();
  canvas->Redraw();
 }
}

void Calibration::RenderPoint(const bs::Pnt & p,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - image.Size(0))/2;
 nat32 sy = (canvas->P().Height() - image.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 canvas->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 canvas->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);
}

//------------------------------------------------------------------------------
