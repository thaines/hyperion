//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/fundamental.h"

//------------------------------------------------------------------------------
Fundamental::Fundamental(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>()),
mode(Edit),selected(null<Match*>())
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


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Fundamental Calibration");
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


   gui::Button * but1b = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2b = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1b->SetChild(lab1b); lab1b->Set("Load Left Calibration...");
   but2b->SetChild(lab2b); lab2b->Set("Load Right Calibration...");

   gui::Vertical * vert2b = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert2b,false);
   vert2b->AttachBottom(but1b);
   vert2b->AttachBottom(but2b);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but3->SetChild(lab3); lab3->Set("Add Match");
   but4->SetChild(lab4); lab4->Set("Delete Match");

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(but3);
   vert3->AttachBottom(but4);


   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   autoAlg = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));

   but5->SetChild(lab5); lab5->Set("Auto Match");
   but6->SetChild(lab6); lab6->Set("Calculate");
   but7->SetChild(lab7); lab7->Set("Trust No Match");
   autoAlg->Append("Harris & NCC");
   //autoAlg->Append("MSER");

   gui::Vertical * vert4 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert4,false);
   vert4->AttachBottom(but6);
   vert4->AttachBottom(but7);

   gui::Vertical * vert5 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert5,false);
   vert5->AttachBottom(but5);
   vert5->AttachBottom(autoAlg);
   autoAlg->Set(0);
   
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Button * but9 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   but9->SetChild(lab10); lab10->Set("AR");
   horiz1->AttachRight(but9,false);
   

   ml = static_cast<gui::Multiline*>(cyclops.Fact().Make("Multiline"));
   horiz1->AttachRight(ml,false);
   ml->SetSize(384,64);
   ml->Edit(false);


   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gap = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab9->Set("Gap Size:");
   gap->SetSize(24,24);
   gap->Set("1");

   gui::Vertical * vert6 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert6,false);
   vert6->AttachBottom(lab9);
   vert6->AttachBottom(gap);


   gui::Button * but8 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but8->SetChild(lab8); lab8->Set("Save '.pcc'...");
   horiz1->AttachRight(but8,false);


   inst = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   inst->Set("Ready.");
   vert1->AttachBottom(inst,false);


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


 // Event handlers...
  win->OnDeath(MakeCB(this,&Fundamental::Quit));

  left->OnResize(MakeCB(this,&Fundamental::ResizeLeft));
  left->OnClick(MakeCB(this,&Fundamental::ClickLeft));
  left->OnMove(MakeCB(this,&Fundamental::MoveLeft));

  right->OnResize(MakeCB(this,&Fundamental::ResizeRight));
  right->OnClick(MakeCB(this,&Fundamental::ClickRight));
  right->OnMove(MakeCB(this,&Fundamental::MoveRight));

  but1->OnClick(MakeCB(this,&Fundamental::LoadLeft));
  but2->OnClick(MakeCB(this,&Fundamental::LoadRight));
  but1b->OnClick(MakeCB(this,&Fundamental::LoadLeftC));
  but2b->OnClick(MakeCB(this,&Fundamental::LoadRightC));
  but3->OnClick(MakeCB(this,&Fundamental::AddMatch));
  but4->OnClick(MakeCB(this,&Fundamental::DelMatch));
  but5->OnClick(MakeCB(this,&Fundamental::AutoMatch));
  but6->OnClick(MakeCB(this,&Fundamental::Calculate));
  but7->OnClick(MakeCB(this,&Fundamental::TrustNoMatch));
  but8->OnClick(MakeCB(this,&Fundamental::SavePair));
  but9->OnClick(MakeCB(this,&Fundamental::AlreadyRectified));
}

Fundamental::~Fundamental()
{
 delete win;
 delete leftVar;
 delete rightVar;
}

void Fundamental::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 if (cyclops.App().ChoiceDialog(gui::App::QuestYesNo,"Are you sure you want to finish calibration?")) delete this;
}

void Fundamental::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(leftImage.Size(0),leftImage.Size(1))),bs::Pos(sx,sy),leftImage);

 // Render the matches, different colours for reliable/non-reliable matches and
 // a square arround the selected match, if it exists...
  ds::List<Match>::Cursor targ = matches.FrontPtr();
  while (!targ.Bad())
  {
   if (targ->reliable) RenderLeftPoint(targ->leftP,bs::ColourRGB(1.0,0.0,1.0),&*targ==selected);
                  else RenderLeftPoint(targ->leftP,bs::ColourRGB(0.0,1.0,1.0),&*targ==selected);
   ++targ;
  }

 // Update...
  left->Update();
}

void Fundamental::ClickLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;

 // Respond depending on mode...
  switch (mode)
  {
   case Edit:
   {
    selected = null<Match*>();
    real64 bestDist = 0.0;
    ds::List<Match>::Cursor targ = matches.FrontPtr();
    while (!targ.Bad())
    {
     real64 dist = math::Sqr(targ->leftP[0]-mp[0]) + math::Sqr(targ->leftP[1]-mp[1]);
     if ((selected==null<Match*>())||(dist<bestDist))
     {
      bestDist = dist;
      selected = &*targ;
     }
     ++targ;
    }

    if (selected)
    {
     if (mbe->button==gui::MouseButtonEvent::LMB)
     {
      selected->leftP[0] = mp[0];
      selected->leftP[1] = mp[1];
      selected->reliable = true;
     }
     right->Redraw();
    }
   }
   break;
   case AddFirst:
   {
    Match m;
     m.leftP[0] = mp[0];
     m.leftP[1] = mp[1];
     m.rightP = m.leftP;
     m.reliable = true;
    matches.AddBack(m);
    selected = &matches.Back();

    mode = AddRight;
    inst->Set("Now select the matching point in the right image.");
   }
   break;
   case AddLeft:
    selected->leftP[0] = mp[0];
    selected->leftP[1] = mp[1];

    mode = Edit;
    inst->Set("Done.");
   break;
   case AddRight:
    if (mbe->down) inst->Set("The RIGHT image, dummy.");
   break;
  }

  left->Redraw();
}

void Fundamental::MoveLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;

 // Snap the selected match...
  if (selected)
  {
   selected->leftP[0] = mp[0];
   selected->leftP[1] = mp[1];
   left->Redraw();
  }
}

void Fundamental::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(rightImage.Size(0),rightImage.Size(1))),bs::Pos(sx,sy),rightImage);

 // Render the matches, different colours for reliable/non-reliable matches and
 // a square arround the selected match, if it exists...
  ds::List<Match>::Cursor targ = matches.FrontPtr();
  while (!targ.Bad())
  {
   if (targ->reliable) RenderRightPoint(targ->rightP,bs::ColourRGB(1.0,0.0,1.0),&*targ==selected);
                  else RenderRightPoint(targ->rightP,bs::ColourRGB(0.0,1.0,1.0),&*targ==selected);
   ++targ;
  }

 // Update...
  right->Update();
}

void Fundamental::ClickRight(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;

 // Respond depending on mode...
  switch (mode)
  {
   case Edit:
   {
    selected = null<Match*>();
    real64 bestDist = 0.0;
    ds::List<Match>::Cursor targ = matches.FrontPtr();
    while (!targ.Bad())
    {
     real64 dist = math::Sqr(targ->rightP[0]-mp[0]) + math::Sqr(targ->rightP[1]-mp[1]);
     if ((selected==null<Match*>())||(dist<bestDist))
     {
      bestDist = dist;
      selected = &*targ;
     }
     ++targ;
    }

    if (selected)
    {
     if (mbe->button==gui::MouseButtonEvent::LMB)
     {
      selected->rightP[0] = mp[0];
      selected->rightP[1] = mp[1];
      selected->reliable = true;
     }
     left->Redraw();
    }
   }
   break;
   case AddFirst:
   {
    Match m;
     m.rightP[0] = mp[0];
     m.rightP[1] = mp[1];
     m.leftP = m.rightP;
     m.reliable = true;
    matches.AddBack(m);
    selected = &matches.Back();

    mode = AddLeft;
    inst->Set("Now select the matching point in the left image.");
   }
   break;
   case AddLeft:
    if (mbe->down) inst->Set("The LEFT image, maggot.");
   break;
   case AddRight:
    selected->rightP[0] = mp[0];
    selected->rightP[1] = mp[1];

    mode = Edit;
    inst->Set("Done.");
   break;
  }

  right->Redraw();
}

void Fundamental::MoveRight(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;

 // Snap the selected match...
  if (selected)
  {
   selected->rightP[0] = mp[0];
   selected->rightP[1] = mp[1];
   right->Redraw();
  }
}

void Fundamental::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
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
   leftVar->Setup2D(floatVar->Size(0),floatVar->Size(1));
   leftVar->Commit();
   leftVar->ByName("rgb",leftImage);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     leftImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;
   
  // If the left calibration details are a different size re-initialise to some that are the correct size...
   if ((!math::Equal(pair.left.dim[0],real64(leftImage.Size(0))))||
       (!math::Equal(pair.left.dim[1],real64(leftImage.Size(1))))) pair.left.SetDefault(leftImage.Size(0),leftImage.Size(1));

  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();
 }
}

void Fundamental::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
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
   rightVar->Setup2D(floatVar->Size(0),floatVar->Size(1));
   rightVar->Commit();
   rightVar->ByName("rgb",rightImage);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<rightImage.Size(1);y++)
   {
    for (nat32 x=0;x<rightImage.Size(0);x++)
    {
     rightImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;

  // If the right calibration details are a different size re-initialise to some that are the correct size...
   if ((!math::Equal(pair.right.dim[0],real64(rightImage.Size(0))))||
       (!math::Equal(pair.right.dim[1],real64(rightImage.Size(1))))) pair.right.SetDefault(rightImage.Size(0),rightImage.Size(1));

  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
 }
}

void Fundamental::LoadLeftC(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Intrinsic Calibration","*.icd",fn))
 {
  if (pair.left.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
  }
 }
}

void Fundamental::LoadRightC(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Intrinsic Calibration","*.icd",fn))
 {
  if (pair.right.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
  }
 }
}

void Fundamental::AddMatch(gui::Base * obj,gui::Event * event)
{
 if (mode==Edit)
 {
  mode = AddFirst;
  inst->Set("Select a matchable point in either image.");
 }
 else
 {
  inst->Set("Finish the current addition first!");
 }
}

void Fundamental::AutoMatch(gui::Base * obj,gui::Event * event)
{
 if (mode==Edit)
 {
  inst->Set("Working...");
  
  // Delete all unreliable matches before we start on the algorithm...
  {
   ds::List<Match>::Cursor targ = matches.FrontPtr();
   while (!targ.Bad())
   {
    if (targ->reliable)
    {
     ++targ;
    }
    else
    {
     targ.RemKillNext();
    }
   }
  }  
  
  if (autoAlg->Get()==0)
  {
   // Do the Haris & NCC approach...
   // Convert both images to normal colour mode...
    bs::ColourRGB colIni(0.0,0.0,0.0);

    svt::Var leftNor(leftImage);
    leftNor.Add("rgb",colIni);
    leftNor.Commit(false);

    svt::Field<bs::ColourRGB> leftRGB(&leftNor,"rgb");
    for (nat32 y=0;y<leftRGB.Size(1);y++)
    {
     for (nat32 x=0;x<leftRGB.Size(0);x++) leftRGB.Get(x,y) = leftImage.Get(x,y);
    }

    svt::Var rightNor(rightImage);
    rightNor.Add("rgb",colIni);
    rightNor.Commit(false);

    svt::Field<bs::ColourRGB> rightRGB(&rightNor,"rgb");
    for (nat32 y=0;y<rightRGB.Size(1);y++)
    {
     for (nat32 x=0;x<rightRGB.Size(0);x++) rightRGB.Get(x,y) = rightImage.Get(x,y);
    }


   // Do the matching - a single function call...
    ds::Array<Pair<bs::Pnt,bs::Pnt> > matFnd;
    filter::MatchImages(leftRGB,rightRGB,matFnd,1500,5,0.7,cyclops.BeginProg());
    cyclops.EndProg();


   // Extract from the returned data structure into our
   // presentation structure all the matches found...
    for (nat32 i=0;i<matFnd.Size();i++)
    {
     Match m;
      m.leftP[0] = matFnd[i].first[0];
      m.leftP[1] = leftImage.Size(1)-1.0 - matFnd[i].first[1];
      m.rightP[0] = matFnd[i].second[0];
      m.rightP[1] = rightImage.Size(1)-1.0 - matFnd[i].second[1];
      m.reliable = false;
     matches.AddBack(m);
    }


   // Let the user see the mess we have created...
    str::String msg;
    msg << "Done - " << matFnd.Size() << " matches made.";
    inst->Set(msg);
    left->Redraw();
    right->Redraw();
  }
  else
  {
   // Do the MSER approach...
   // Convert both images to normal colour mode and luminence colour mode...
    bs::ColourRGB colIni(0.0,0.0,0.0);
    bs::ColourL lIni(0.0);

    svt::Var leftNor(leftImage);
    leftNor.Add("rgb",colIni);
    leftNor.Add("l",lIni);
    leftNor.Commit(false);

    svt::Field<bs::ColourRGB> leftRGB(&leftNor,"rgb");
    svt::Field<bs::ColourL> leftL(&leftNor,"l");
    for (nat32 y=0;y<leftRGB.Size(1);y++)
    {
     for (nat32 x=0;x<leftRGB.Size(0);x++)
     {
      leftRGB.Get(x,y) = leftImage.Get(x,y);
      leftL.Get(x,y) = leftRGB.Get(x,y);
     }
    }

    svt::Var rightNor(rightImage);
    rightNor.Add("rgb",colIni);
    rightNor.Add("l",lIni);
    rightNor.Commit(false);

    svt::Field<bs::ColourRGB> rightRGB(&rightNor,"rgb");
    svt::Field<bs::ColourL> rightL(&rightNor,"l");
    for (nat32 y=0;y<rightRGB.Size(1);y++)
    {
     for (nat32 x=0;x<rightRGB.Size(0);x++)
     {
      rightRGB.Get(x,y) = rightImage.Get(x,y);
      rightL.Get(x,y) = rightRGB.Get(x,y);
     }
    }


   // Do the matching - painful...
    time::Progress * prog = cyclops.BeginProg();
    // Calculate MSERs for the left image...
     prog->Report(0,3);
     filter::MserKeys leftMatch;
     leftMatch.Set(leftL);
     leftMatch.Set(leftRGB);
     leftMatch.Run(prog);

    // Calculate MSERs for the right image...
     prog->Report(1,3);
     filter::MserKeys rightMatch;
     rightMatch.Set(rightL);
     rightMatch.Set(rightRGB);
     rightMatch.Run(prog);

    // Symetrically match keys from both images using an approximate KD tree,
    // store the good matches...
     prog->Report(2,3);
     prog->Push();
      // Create a kd-tree of the left...
       prog->Report(0,3);
       ds::KdTree<filter::MserKey,1323> leftTree;
       for (nat32 i=0;i<leftMatch.Size();i++) leftTree.Add(leftMatch.Key(i));

      // Create a kd-tree of the right...
       prog->Report(1,3);
       ds::KdTree<filter::MserKey,1323> rightTree;
       for (nat32 i=0;i<rightMatch.Size();i++) rightTree.Add(rightMatch.Key(i));

      // Iterate the left, find the approximate best match of each and check it
      // matches back again, store all such pairs...
       prog->Report(2,3);
       nat32 matchCount = 0;
       prog->Push();
       for (nat32 i=0;i<leftMatch.Size();i++)
       {
        prog->Report(i,leftMatch.Size());
        // Find the index of the right mser that best matches this left mser...
         nat32 ri = rightTree.Nearest(leftMatch.Key(i)).index;

        // Check the best match for the right mser is the left mser we have...
         if (leftTree.Nearest(rightMatch.Key(ri)).index!=i) continue;
         matchCount += 1;

        // Add the pair in...
         Match m;
          m.leftP[0] = leftMatch.Pixel(i)[0];
          m.leftP[1] = leftImage.Size(1)-1.0 - leftMatch.Pixel(i)[1];
          m.rightP[0] = rightMatch.Pixel(ri)[0];
          m.rightP[1] = rightImage.Size(1)-1.0 - rightMatch.Pixel(ri)[1];
          m.reliable = false;
         matches.AddBack(m);
       }
       prog->Pop();

     prog->Pop();
    cyclops.EndProg();


   // Let the user see the mess we have created...
    str::String msg;
    msg << "Done - " << matchCount << " matches made. {" << leftMatch.Size() << ":" << rightMatch.Size() << "}";
    inst->Set(msg);
    left->Redraw();
    right->Redraw();
  }
 }
 else
 {
  inst->Set("Finish the current addition first!");
 }
}

void Fundamental::DelMatch(gui::Base * obj,gui::Event * event)
{
 if ((selected))
 {
  ds::List<Match>::Cursor targ = matches.FrontPtr();
  while (!targ.Bad())
  {
   if ((&*targ)==selected)
   {
    targ.RemNext();
    break;
   }
   ++targ;
  }

  inst->Set("Deleted.");
  selected = null<Match*>();
  mode = Edit;

  left->Redraw();
  right->Redraw();
 }
}

void Fundamental::Calculate(gui::Base * obj,gui::Event * event)
{
 if (mode==Edit)
 {
  inst->Set("Working...");
  ml->Empty();

  cam::FunCalc fc;
  ds::List<Match>::Cursor targ = matches.FrontPtr();
  while (!targ.Bad())
  {
   math::Vect<2,real64> leftT = targ->leftP;
   math::Vect<2,real64> rightT = targ->rightP;
    // Adjust for origin...
     leftT[1] = leftImage.Size(1)-1.0 - leftT[1];
     rightT[1] = rightImage.Size(1)-1.0 - rightT[1];

    // Adjust for radial distortion...
     math::Vect<2,real64> leftU;
     math::Vect<2,real64> rightU;
      pair.left.radial.UnDis(leftT,leftU);
      pair.right.radial.UnDis(rightT,rightU);

   fc.AddMatch(leftU,rightU,targ->reliable);
   ++targ;
  }


  if (fc.Run(cyclops.BeginProg()))
  {
   pair.fun = fc.Fun();
   real64 residual = fc.Residual();
   real64 meanError = fc.MeanError();
   nat32 usedCount = fc.UsedCount();

   str::String s;
    s << "[" << (real32)pair.fun[0][0] << "," << (real32)pair.fun[0][1] << "," << (real32)pair.fun[0][2] << "]\n";
    s << "[" << (real32)pair.fun[1][0] << "," << (real32)pair.fun[1][1] << "," << (real32)pair.fun[1][2] << "]\n";
    s << "[" << (real32)pair.fun[2][0] << "," << (real32)pair.fun[2][1] << "," << (real32)pair.fun[2][2] << "]";
   ml->Append(s);

   // Update the display to mark reliable matches as reliable and unreliable matches as not...
    ds::Array<bit> relArr(matches.Size());
    fc.Used(relArr);
    ds::List<Match>::Cursor targ = matches.FrontPtr();
    for (nat32 i=0;i<matches.Size();i++)
    {
     targ->reliable = relArr[i];
     ++targ;
    }

   // Refresh, message to indicate completion...
    left->Redraw();
    right->Redraw();

    str::String msg;
    msg << "Success! " << usedCount << " of " << matches.Size()
        << " matches used. (Residual = " << residual << "; mean error = " << meanError << ".)";
    inst->Set(msg);
  }
  else
  {
   inst->Set("Failure!");
  }
  cyclops.EndProg();
 }
 else
 {
  inst->Set("Finish the current addition first!");
 }
}

void Fundamental::TrustNoMatch(gui::Base * obj,gui::Event * event)
{
 ds::List<Match>::Cursor targ = matches.FrontPtr();
 while (!targ.Bad())
 {
  targ->reliable = false;
  ++targ;
 }
 left->Redraw();
 right->Redraw();
}

void Fundamental::AlreadyRectified(gui::Base * obj,gui::Event * event)
{
 math::Zero(pair.fun);
 pair.fun[1][2] = -0.5*math::Sqrt(2.0);
 pair.fun[2][1] = 0.5*math::Sqrt(2.0);
 
 str::String s;
  s << "[" << (real32)pair.fun[0][0] << "," << (real32)pair.fun[0][1] << "," << (real32)pair.fun[0][2] << "]\n";
  s << "[" << (real32)pair.fun[1][0] << "," << (real32)pair.fun[1][1] << "," << (real32)pair.fun[1][2] << "]\n";
  s << "[" << (real32)pair.fun[2][0] << "," << (real32)pair.fun[2][1] << "," << (real32)pair.fun[2][2] << "]";
 ml->Empty();
 ml->Append(s);
 
 inst->Set("Assumed to already be rectified.");
}

void Fundamental::SavePair(gui::Base * obj,gui::Event * event)
{
 if (mode==Edit)
 {
  // Copy over image sizes...
   pair.leftDim = pair.left.dim;
   pair.rightDim = pair.right.dim;

  // Finish construction of the pair...
   // Grab the gap from the text field...
    str::String gapStr = gap->Get();
    str::String::Cursor gapStrCur = gapStr.GetCursor();
    gapStrCur >> pair.gap;
    if (gapStrCur.Error()) pair.gap = 1.0;

   // Decide which projection matrices are correct...
    // Count how many reliable matches exist...
     nat32 relCount = 0;
     ds::List<Match>::Cursor targ = matches.FrontPtr();
     while (!targ.Bad())
     {
      if (targ->reliable) ++relCount;
      ++targ;
     }

    // Create an array of reliable matches...
     ds::Array<Pair<bs::Point,bs::Point> > relArr(relCount);
     relCount = 0;
     targ = matches.FrontPtr();
     while (!targ.Bad())
     {
      if (targ->reliable)
      {
       // Get...
        relArr[relCount].first[0] = targ->leftP[0];
        relArr[relCount].first[1] = targ->leftP[1];
        relArr[relCount].second[0] = targ->rightP[0];
        relArr[relCount].second[1] = targ->rightP[1];

       // Adjust for origin...
        relArr[relCount].first[1] = leftImage.Size(1)-1.0 - relArr[relCount].first[1];
        relArr[relCount].second[1] = rightImage.Size(1)-1.0 - relArr[relCount].second[1];

       // Scale for changes in image size...
        relArr[relCount].first[0] *= pair.left.dim[0]/leftImage.Size(0);
        relArr[relCount].first[1] *= pair.left.dim[1]/leftImage.Size(1);
        relArr[relCount].second[0] *= pair.right.dim[0]/rightImage.Size(0);
        relArr[relCount].second[1] *= pair.right.dim[1]/rightImage.Size(1);

       // (We ignore radial distortion, its just not worth it considering the nature of use.)

       ++relCount;
      }
      ++targ;
     }

    // Use the reliable matches to make the projection matrices...
     pair.MakeProjection(&relArr);


  // Get the filename...
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
  inst->Set("Finish the current addition first!");
 }
}

void Fundamental::RenderLeftPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col,bit special)
{
 nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
 nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 left->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 left->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);

 if (special)
 {
  left->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx,cy+5),col);
  left->P().Line(bs::Pos(cx+5,cy),bs::Pos(cx,cy-5),col);
 }
}

void Fundamental::RenderRightPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col,bit special)
{
 nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
 nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 right->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 right->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);

 if (special)
 {
  right->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx,cy+5),col);
  right->P().Line(bs::Pos(cx+5,cy),bs::Pos(cx,cy-5),col);
 }
}

//------------------------------------------------------------------------------
