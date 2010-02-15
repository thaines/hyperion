//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/homography.h"

//------------------------------------------------------------------------------
Homography::Homography(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
imageFullVar(null<svt::Var*>()),imageVar(null<svt::Var*>()),
transFullVar(null<svt::Var*>()),transVar(null<svt::Var*>()),
nextSnap(0)
{
 // Create default images and associated variables...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  bs::ColRGB colIni(0,0,0);

  imageFullVar = new svt::Var(cyclops.Core());
  imageFullVar->Setup2D(320,240);
  imageFullVar->Add("rgb",colourIni);
  imageFullVar->Commit();
  imageFullVar->ByName("rgb",imageFull);

  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);

  transFullVar = new svt::Var(cyclops.Core());
  transFullVar->Setup2D(256,256);
  transFullVar->Add("rgb",colourIni);
  transFullVar->Commit();
  transFullVar->ByName("rgb",transFull);

  transVar = new svt::Var(cyclops.Core());
  transVar->Setup2D(256,256);
  transVar->Add("rgb",colIni);
  transVar->Commit();
  transVar->ByName("rgb",trans);

  width = 256; height = 256;
  corner[0][0] = 110.0; corner[0][1] = 70.0;
  corner[1][0] = 210.0; corner[1][1] = 70.0;
  corner[2][0] = 210.0; corner[2][1] = 170.0;
  corner[3][0] = 110.0; corner[3][1] = 170.0;
  xLow = 0.0; xHigh = 1.0;
  yLow = 0.0; yHigh = 1.0;



 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Homography");
  cyclops.App().Attach(win);
  win->SetSize(650,350);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Calculate");
   but3->SetChild(lab3); lab3->Set("Save Transformed...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);


   widthEdit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   heightEdit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   xLowEdit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   xHighEdit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   yLowEdit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   yHighEdit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   widthEdit->SetSize(64,24);  widthEdit->Set("256");
   heightEdit->SetSize(64,24); heightEdit->Set("256");
   xLowEdit->SetSize(48,24);   xLowEdit->Set("0.0");
   xHighEdit->SetSize(48,24);  xHighEdit->Set("1.0");
   yLowEdit->SetSize(48,24);   yLowEdit->Set("0.0");
   yHighEdit->SetSize(48,24);  yHighEdit->Set("1.0");

   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab4->Set("Width:");
   lab5->Set("Height:");
   lab6->Set("Bounds:");

   horiz2->AttachRight(lab4,false);
   horiz2->AttachRight(widthEdit,false);
   horiz2->AttachRight(lab5,false);
   horiz2->AttachRight(heightEdit,false);
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(xLowEdit,false);
   horiz2->AttachRight(xHighEdit,false);
   horiz2->AttachRight(yLowEdit,false);
   horiz2->AttachRight(yHighEdit,false);


   gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz3);

   gui::Panel * panel1 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   gui::Panel * panel2 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   horiz3->AttachRight(panel1);
   horiz3->AttachRight(panel2);

   left = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   right = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel1->SetChild(left);
   panel2->SetChild(right);

   left->SetSize(image.Size(0),image.Size(1));
   right->SetSize(trans.Size(0),trans.Size(1));



 // Handlers...
  win->OnDeath(MakeCB(this,&Homography::Quit));

  left->OnResize(MakeCB(this,&Homography::ResizeLeft));
  left->OnClick(MakeCB(this,&Homography::ClickLeft));
  left->OnMove(MakeCB(this,&Homography::MoveLeft));
  right->OnResize(MakeCB(this,&Homography::ResizeRight));

  but1->OnClick(MakeCB(this,&Homography::LoadImage));
  but2->OnClick(MakeCB(this,&Homography::Calculate));
  but3->OnClick(MakeCB(this,&Homography::SaveImage));

  widthEdit->OnChange(MakeCB(this,&Homography::OnEditChange));
  heightEdit->OnChange(MakeCB(this,&Homography::OnEditChange));
  xLowEdit->OnChange(MakeCB(this,&Homography::OnEditChange));
  xHighEdit->OnChange(MakeCB(this,&Homography::OnEditChange));
  yLowEdit->OnChange(MakeCB(this,&Homography::OnEditChange));
  yHighEdit->OnChange(MakeCB(this,&Homography::OnEditChange));
}

Homography::~Homography()
{
 delete win;
 delete imageFullVar;
 delete imageVar;
 delete transFullVar;
 delete transVar;
}

void Homography::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Homography::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));


 // Render the image...
  nat32 sx = (left->P().Width() - image.Size(0))/2;
  nat32 sy = (left->P().Height() - image.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);


 // Render the bounds square...
  math::Mat<3,3,real64> hg;
  {
   cam::Homography2D hgCalc;
   math::Vect<2,real64> t,c;

   t[0] = 0.0; t[1] = 1.0; c[0] = corner[0][0]; c[1] = image.Size(1)-1-corner[0][1]; hgCalc.Add(t,c);
   t[0] = 1.0; t[1] = 1.0; c[0] = corner[1][0]; c[1] = image.Size(1)-1-corner[1][1]; hgCalc.Add(t,c);
   t[0] = 1.0; t[1] = 0.0; c[0] = corner[2][0]; c[1] = image.Size(1)-1-corner[2][1]; hgCalc.Add(t,c);
   t[0] = 0.0; t[1] = 0.0; c[0] = corner[3][0]; c[1] = image.Size(1)-1-corner[3][1]; hgCalc.Add(t,c);
   hgCalc.Result(hg);
  }

  math::Vect<2,real64> lims[4];
  {
   math::Vect<2,real64> t;
   t[0] = xLow;  t[1] = yHigh; math::MultVectEH(hg,t,lims[0]); lims[0][1] = image.Size(1)-1-lims[0][1];
   t[0] = xHigh; t[1] = yHigh; math::MultVectEH(hg,t,lims[1]); lims[1][1] = image.Size(1)-1-lims[1][1];
   t[0] = xHigh; t[1] = yLow;  math::MultVectEH(hg,t,lims[2]); lims[2][1] = image.Size(1)-1-lims[2][1];
   t[0] = xLow;  t[1] = yLow;  math::MultVectEH(hg,t,lims[3]); lims[3][1] = image.Size(1)-1-lims[3][1];
  }

  for (nat32 i=0;i<4;i++) RenderLeftLine(lims[i],lims[(i+1)%4],bs::ColourRGB(0.0,0.0,1.0));


 // Render the selection square...
  for (nat32 i=0;i<4;i++) RenderLeftLine(corner[i],corner[(i+1)%4],bs::ColourRGB(1.0,0.0,0.0));
  for (nat32 i=0;i<4;i++) RenderLeftPoint(corner[i],bs::ColourRGB(1.0,0.0,1.0));


 // Update...
  left->Update();
}

void Homography::ClickLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - image.Size(0))/2;
  nat32 sy = (left->P().Height() - image.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;


  if (mbe->button==gui::MouseButtonEvent::LMB)
  {
   // Find closest corner, if its within tolerance snap...
    int32 closest = 0;
    real32 bestDist = math::Sqr(corner[0][0]-mp[0]) + math::Sqr(corner[0][1]-mp[1]);
    for (nat32 i=1;i<4;i++)
    {
     real32 dist = math::Sqr(corner[i][0]-mp[0]) + math::Sqr(corner[i][1]-mp[1]);
     if (dist<bestDist)
     {
      bestDist = dist;
      closest = i;
     }
    }

    if (bestDist<math::Sqr(250.0))
    {
     corner[closest][0] = mp[0];
     corner[closest][1] = mp[1];
    }
  }
  else
  {
   if (mbe->down)
   {
    corner[nextSnap][0] = mp[0];
    corner[nextSnap][1] = mp[1];
    nextSnap = (nextSnap+1)%4;
   }
  }

 // Redraw...
  left->Redraw();
}

void Homography::MoveLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - image.Size(0))/2;
  nat32 sy = (left->P().Height() - image.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;



 // Find closest corner, if its within tolerance snap...
  int32 closest = 0;
  real32 bestDist = math::Sqr(corner[0][0]-mp[0]) + math::Sqr(corner[0][1]-mp[1]);
  for (nat32 i=1;i<4;i++)
  {
   real32 dist = math::Sqr(corner[i][0]-mp[0]) + math::Sqr(corner[i][1]-mp[1]);
   if (dist<bestDist)
   {
    bestDist = dist;
    closest = i;
   }
  }

  if (bestDist<math::Sqr(250.0))
  {
   corner[closest][0] = mp[0];
   corner[closest][1] = mp[1];
  }


 // Redraw the display...
  // left->Redraw(); // Too slow.
}

void Homography::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (right->P().Width() - trans.Size(0))/2;
  nat32 sy = (right->P().Height() - trans.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(trans.Size(0),trans.Size(1))),bs::Pos(sx,sy),trans);

 // Update...
  right->Update();
}

void Homography::OnEditChange(gui::Base * obj,gui::Event * event)
{
 // Update values...
  str::String s = widthEdit->Get();
  str::String::Cursor c = s.GetCursor();
  c >> width;
  if (c.Error()) width = 256;

  s = heightEdit->Get();
  c = s.GetCursor();
  c >> height;
  if (c.Error()) height = 256;

  s = xLowEdit->Get();
  c = s.GetCursor();
  c >> xLow;
  if (c.Error()) xLow = 0.0;

  s = xHighEdit->Get();
  c = s.GetCursor();
  c >> xHigh;
  if (c.Error()) xHigh = 1.0;

  s = yLowEdit->Get();
  c = s.GetCursor();
  c >> yLow;
  if (c.Error()) yLow = 0.0;

  s = yHighEdit->Get();
  c = s.GetCursor();
  c >> yHigh;
  if (c.Error()) yHigh = 1.0;

 // Redraw the left side...
  ResizeLeft(obj,event);
}

void Homography::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * temp = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (temp==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

   delete imageFullVar;
   imageFullVar = temp;
   imageFullVar->ByName("rgb",imageFull);


  // Image loaded, but its not in a format suitable for fast display - convert...
   imageVar->Setup2D(imageFull.Size(0),imageFull.Size(1));
   imageVar->Commit();
   imageVar->ByName("rgb",image);

   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     image.Get(x,y) = imageFull.Get(x,y);
    }
   }


  // Refresh the display...
   left->SetSize(image.Size(0),image.Size(1));
   left->Redraw();
 }
}

void Homography::Calculate(gui::Base * obj,gui::Event * event)
{
 // Calculate the homography...
  math::Mat<3,3,real64> hg;
  {
   cam::Homography2D hgCalc;
   math::Vect<2,real64> t,c;

   t[0] = 0.0; t[1] = 1.0; c[0] = corner[0][0]; c[1] = image.Size(1)-1-corner[0][1]; hgCalc.Add(t,c);
   t[0] = 1.0; t[1] = 1.0; c[0] = corner[1][0]; c[1] = image.Size(1)-1-corner[1][1]; hgCalc.Add(t,c);
   t[0] = 1.0; t[1] = 0.0; c[0] = corner[2][0]; c[1] = image.Size(1)-1-corner[2][1]; hgCalc.Add(t,c);
   t[0] = 0.0; t[1] = 0.0; c[0] = corner[3][0]; c[1] = image.Size(1)-1-corner[3][1]; hgCalc.Add(t,c);
   hgCalc.Result(hg);
  }


 // Scale the output image...
  if ((int32(transFullVar->Size(0))!=width)||(int32(transFullVar->Size(1))!=height))
  {
   transFullVar->Setup2D(width,height);
   transFullVar->Commit();
   transFullVar->ByName("rgb",transFull);

   transVar->Setup2D(width,height);
   transVar->Commit();
   transVar->ByName("rgb",trans);
  }


 // Iterate every pixel and sample from the original...
  time::Progress * prog = cyclops.BeginProg();
  for (nat32 y=0;y<transFull.Size(1);y++)
  {
   prog->Report(y,transFull.Size(1));
   for (nat32 x=0;x<transFull.Size(0);x++)
   {
    math::Vect<2,real64> ip;
    ip[0] = xLow + (real64(x)/real64(transFull.Size(0)-1))*(xHigh-xLow);
    ip[1] = yLow + (real64(y)/real64(transFull.Size(1)-1))*(yHigh-yLow);
    math::Vect<2,real64> op;
    math::MultVectEH(hg,ip,op);

    if (svt::SampleColourRGBLin2D(transFull.Get(x,y),op[0],op[1],imageFull)==false)
    {
     transFull.Get(x,y) = bs::ColourRGB(1.0,0.0,1.0);
    }
   }
  }
  cyclops.EndProg();


 // Copy the new image into the render buffer...
  for (nat32 y=0;y<transFull.Size(1);y++)
  {
   for (nat32 x=0;x<transFull.Size(0);x++)
   {
    trans.Get(x,y) = transFull.Get(x,y);
   }
  }

 // Refresh...
  right->SetSize(trans.Size(0),trans.Size(1));
  right->Redraw();
}

void Homography::SaveImage(gui::Base * obj,gui::Event * event)
{
 str::String fn("");
 if (cyclops.App().SaveFileDialog("Save Reprojected Image",fn))
 {
  // Save the image...
   str::String imFn(fn);
   if (!(imFn.EndsWith(".bmp")||imFn.EndsWith(".jpg")||imFn.EndsWith(".tga")||imFn.EndsWith(".png"))) imFn << ".bmp";
   cstr ts = imFn.ToStr();
   if (!filter::SaveImage(transFull,ts,true))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
   }
   mem::Free(ts);
 }
}

void Homography::RenderLeftPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col)
{
 nat32 sx = (left->P().Width() - image.Size(0))/2;
 nat32 sy = (left->P().Height() - image.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 left->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 left->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);
}

void Homography::RenderLeftLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col)
{
 nat32 sx = (left->P().Width() - image.Size(0))/2;
 nat32 sy = (left->P().Height() - image.Size(1))/2;

 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);

 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);

 left->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col);
}

//------------------------------------------------------------------------------
