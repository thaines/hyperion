//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/rectification.h"

//------------------------------------------------------------------------------
Rectification::Rectification(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>()),
leftResult(null<svt::Var*>()),rightResult(null<svt::Var*>())
{
 // Create default images...
  leftVar = new svt::Var(cyclops.Core());
  leftVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  leftVar->Add("rgb",colIni);
  leftVar->Commit();
  leftVar->ByName("rgb",leftImage);

  rightVar = new svt::Var(cyclops.Core());
  rightVar->Setup2D(320,240);
  rightVar->Add("rgb",colIni);
  rightVar->Commit();
  rightVar->ByName("rgb",rightImage);

  pair.SetDefault(320,240);


 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Rectification");
  cyclops.App().Attach(win);
  win->SetSize(leftImage.Size(0)+rightImage.Size(0)+24,math::Max(leftImage.Size(1),rightImage.Size(1))+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Left Image...");
   but2->SetChild(lab2); lab2->Set("Load Right Image...");

   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert2,false);
   vert2->AttachBottom(but1);
   vert2->AttachBottom(but2);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but3->SetChild(lab3); lab3->Set("Load Configuration...");
   horiz1->AttachRight(but3,false);


   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but4->SetChild(lab4); lab4->Set("Rectify");
   horiz1->AttachRight(but4,false);


   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but5->SetChild(lab5); lab5->Set("Save Left Image...");
   but6->SetChild(lab6); lab6->Set("Save Right Image...");

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(but5);
   vert3->AttachBottom(but6);


   gui::Button * but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but7->SetChild(lab7); lab7->Set("Save Configuration...");
   horiz1->AttachRight(but7,false);


   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2);

   gui::Panel * panel1 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   gui::Panel * panel2 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   horiz2->AttachRight(panel1);
   horiz2->AttachRight(panel2);

   left = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   right = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel1->SetChild(left);
   panel2->SetChild(right);

   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   right->SetSize(rightImage.Size(0),rightImage.Size(1));


 // Handlers...
  win->OnDeath(MakeCB(this,&Rectification::Quit));

  left->OnResize(MakeCB(this,&Rectification::ResizeLeft));
  right->OnResize(MakeCB(this,&Rectification::ResizeRight));

  but1->OnClick(MakeCB(this,&Rectification::LoadLeft));
  but2->OnClick(MakeCB(this,&Rectification::LoadRight));
  but3->OnClick(MakeCB(this,&Rectification::LoadConfig));
  but4->OnClick(MakeCB(this,&Rectification::Rectify));
  but5->OnClick(MakeCB(this,&Rectification::SaveLeft));
  but6->OnClick(MakeCB(this,&Rectification::SaveRight));
  but7->OnClick(MakeCB(this,&Rectification::SaveConfig));
}

Rectification::~Rectification()
{
 delete win;
 delete leftVar;
 delete rightVar;
 delete leftResult;
 delete rightResult;
}

void Rectification::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Rectification::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(leftImage.Size(0),leftImage.Size(1))),bs::Pos(sx,sy),leftImage);

 // Update...
  left->Update();
}

void Rectification::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(rightImage.Size(0),rightImage.Size(1))),bs::Pos(sx,sy),rightImage);

 // Update...
  right->Update();
}

void Rectification::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   delete leftResult;
   leftResult = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (leftResult==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Image loaded, but its not in a format suitable for fast display - convert...
   leftVar->Setup2D(leftResult->Size(0),leftResult->Size(1));
   leftVar->Commit();
   leftVar->ByName("rgb",leftImage);

   svt::Field<bs::ColourRGB> floatImage(leftResult,"rgb");
   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     leftImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();
 }
}

void Rectification::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   delete rightResult;
   rightResult = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (rightResult==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Image loaded, but its not in a format suitable for fast display - convert...
   rightVar->Setup2D(rightResult->Size(0),rightResult->Size(1));
   rightVar->Commit();
   rightVar->ByName("rgb",rightImage);

   svt::Field<bs::ColourRGB> floatImage(rightResult,"rgb");
   for (nat32 y=0;y<rightImage.Size(1);y++)
   {
    for (nat32 x=0;x<rightImage.Size(0);x++)
    {
     rightImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
 }
}

void Rectification::LoadConfig(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Fundamental Matrix","*.fun,*.pcc",fn))
 {
  if (fn.EndsWith(".pcc"))
  {
   if (pair.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
   }
  }
  else
  {
   if (pair.fun.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load fundamental matrix file");
   }
  }
 }
}

void Rectification::Rectify(gui::Base * obj,gui::Event * event)
{
 if ((leftResult==null<svt::Var*>())||(rightResult==null<svt::Var*>()))
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to load images to rectify!");
 }
 else
 {
  if ((leftResult->Fields()!=1)||(rightResult->Fields()!=1))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"The images have allready been rectified");
  }
  else
  {
   // Adjust the things given to the rectification function to be the size of the input images...
    cam::Fundamental fun = pair.fun;
    cam::Radial leftRad = pair.left.radial;
    cam::Radial rightRad = pair.right.radial;

    fun.ChangeSize(bs::Pnt(pair.left.dim[0],pair.left.dim[1]),
                   bs::Pnt(pair.right.dim[0],pair.right.dim[1]),
                   bs::Pnt(leftImage.Size(0),leftImage.Size(1)),
                   bs::Pnt(rightImage.Size(0),rightImage.Size(1)));
    leftRad.ChangeSize(bs::Pnt(pair.left.dim[0],pair.left.dim[1]),
                       bs::Pnt(leftImage.Size(0),leftImage.Size(1)));
    rightRad.ChangeSize(bs::Pnt(pair.right.dim[0],pair.right.dim[1]),
                        bs::Pnt(rightImage.Size(0),rightImage.Size(1)));


   // Rectify...
    svt::Var * lResult = new svt::Var(leftResult->GetCore());
    svt::Var * rResult = new svt::Var(rightResult->GetCore());

     cam::PlaneRectify(leftResult,rightResult,leftRad,rightRad,fun,
                       lResult,rResult,&pair.unRectLeft,&pair.unRectRight,
                       1,false,false,cyclops.BeginProg());
    cyclops.EndProg();

    delete leftResult; leftResult = lResult;
    delete rightResult; rightResult = rResult;


   // Calculate fundamental matrix from camera matrices and log...     
    #ifdef EOS_DEBUG
     /*{
      cam::Camera cl;
      cam::Camera cr;
      math::Mat<3,3,real64> temp[2];

      temp[0] = pair.unRectLeft;
      math::Inverse(temp[0],temp[1]);
      math::Mult(temp[0],pair.lp,cl);
      
      temp[0] = pair.unRectRight;
      math::Inverse(temp[0],temp[1]);
      math::Mult(temp[0],pair.rp,cr);
      
      
      math::Vect<4,real64> leftCentre;
      cl.Centre(leftCentre);

      math::Vect<3,real64> rightEPL;
      math::MultVect(cr,leftCentre,rightEPL);

      math::Mat<3,3,real64> epiMat;
      math::SkewSymetric33(rightEPL,epiMat);

      cam::Camera tempCam;
      math::Mult(epiMat,cr,tempCam);

      math::Mat<4,3,real64> invCam;
      math::PseudoInverse(cl,invCam);

      math::Mat<3,3,real64> tempFun;
      math::Mult(tempCam,invCam,tempFun);

      tempFun /= tempFun[1][2];      
      LogDebug("tempFun" << LogDiv() << tempFun);
     }*/
    #endif


   // Remove the radial distortion from the CameraPair -
   // its been compensated for by the rectification...
    for (nat32 i=0;i<4;i++)
    {
     pair.left.radial.k[i] = 0.0;
     pair.right.radial.k[i] = 0.0;
    }


   // Update dimensions...
    pair.leftDim[0] = leftResult->Size(0);
    pair.leftDim[1] = leftResult->Size(1);
    pair.rightDim[0] = rightResult->Size(0);
    pair.rightDim[1] = rightResult->Size(1);


   // Display the rectified images...
    // Left...
    {
     leftVar->Setup2D(leftResult->Size(0),leftResult->Size(1));
     leftVar->Commit();
     leftVar->ByName("rgb",leftImage);

     svt::Field<bs::ColourRGB> floatImage(leftResult,"rgb");
     for (nat32 y=0;y<leftImage.Size(1);y++)
     {
      for (nat32 x=0;x<leftImage.Size(0);x++)
      {
       leftImage.Get(x,y) = floatImage.Get(x,y);
      }
     }

     left->SetSize(leftImage.Size(0),leftImage.Size(1));
     left->Redraw();
    }

    // Right...
    {
     rightVar->Setup2D(rightResult->Size(0),rightResult->Size(1));
     rightVar->Commit();
     rightVar->ByName("rgb",rightImage);

     svt::Field<bs::ColourRGB> floatImage(rightResult,"rgb");
     for (nat32 y=0;y<rightImage.Size(1);y++)
     {
      for (nat32 x=0;x<rightImage.Size(0);x++)
      {
       rightImage.Get(x,y) = floatImage.Get(x,y);
      }
     }

     right->SetSize(rightImage.Size(0),rightImage.Size(1));
     right->Redraw();
    }
  }
 }
}

void Rectification::SaveLeft(gui::Base * obj,gui::Event * event)
{
 if (leftResult)
 {
  // Get the filename...
   str::String fn("");
   if (cyclops.App().SaveFileDialog("Save Left Image",fn))
   {
    // Save the image...
     str::String imFn(fn);
     if (!(imFn.EndsWith(".bmp")||imFn.EndsWith(".jpg")||imFn.EndsWith(".tga")||imFn.EndsWith(".png"))) imFn << ".bmp";
     cstr ts = imFn.ToStr();
     if (!filter::SaveImageRGB(leftResult,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving left image.");
     }
     mem::Free(ts);
   }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to rectify first!");
 }
}

void Rectification::SaveRight(gui::Base * obj,gui::Event * event)
{
 if (rightResult)
 {
  // Get the filename...
   str::String fn("");
   if (cyclops.App().SaveFileDialog("Save Right Image",fn))
   {
    // Save the image...
     str::String imFn(fn);
     if (!(imFn.EndsWith(".bmp")||imFn.EndsWith(".jpg")||imFn.EndsWith(".tga")||imFn.EndsWith(".png"))) imFn << ".bmp";
     cstr ts = imFn.ToStr();
     if (!filter::SaveImageRGB(rightResult,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving right image.");
     }
     mem::Free(ts);
   }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to rectify first!");
 }
}

void Rectification::SaveConfig(gui::Base * obj,gui::Event * event)
{
 if (leftResult&&rightResult)
 {
  str::String fn(".pcc");
  if (cyclops.App().SaveFileDialog("Save Camera Pair Calibration...",fn))
  {
   if (!fn.EndsWith(".pcc")) fn += ".pcc";
   // Save the file...
    if (!pair.Save(fn,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to rectify first!");
 }
}

//------------------------------------------------------------------------------
