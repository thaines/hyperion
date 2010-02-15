//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/intrinsic_est.h"

//------------------------------------------------------------------------------
IntrinsicEst::IntrinsicEst(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),xChange(false),yChange(false)
{
 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Intrinsic Matrix Approximation");
  cyclops.App().Attach(win);
  win->SetSize(1,1);

  gui::Grid * grid = static_cast<gui::Grid*>(cyclops.Fact().Make("Grid"));
  win->SetChild(grid);
  grid->SetDims(5,4);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("from Y");
   but2->SetChild(lab2); lab2->Set("from X");
   but3->SetChild(lab3); lab3->Set("Save .icd...");
   
   grid->Attach(4,1,but1);
   grid->Attach(4,2,but2);
   grid->Attach(3,3,but3,2);

      
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab4->Set("X: ");
   lab5->Set("Y: ");
   lab6->Set("Size ");
   lab7->Set("35mm Equiv. ");
   lab8->Set("Angle (Deg) ");
   lab9->Set("Square Aspect");

   grid->Attach(0,1,lab4);
   grid->Attach(0,2,lab5);
   grid->Attach(1,0,lab6);
   grid->Attach(2,0,lab7);
   grid->Attach(3,0,lab8);
   grid->Attach(4,0,lab9);


   sizeX = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   mmX = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   angX = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   sizeX->Set("1200");
   mmX->Set("50.0");
   angX->Set("39.597753");
   
   grid->Attach(1,1,sizeX);
   grid->Attach(2,1,mmX);
   grid->Attach(3,1,angX);


   sizeY = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   mmY = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   angY = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   sizeY->Set("800");
   mmY->Set("50.0");
   angY->Set("26.991467");

   grid->Attach(1,2,sizeY);
   grid->Attach(2,2,mmY);
   grid->Attach(3,2,angY);   



 // Event handlers...
  win->OnDeath(MakeCB(this,&IntrinsicEst::Quit));
    
  but1->OnClick(MakeCB(this,&IntrinsicEst::fromY));
  but2->OnClick(MakeCB(this,&IntrinsicEst::fromX));
  but3->OnClick(MakeCB(this,&IntrinsicEst::SaveICD));

  mmX->OnChange(MakeCB(this,&IntrinsicEst::mmXChange));
  angX->OnChange(MakeCB(this,&IntrinsicEst::angXChange));

  mmY->OnChange(MakeCB(this,&IntrinsicEst::mmYChange));
  angY->OnChange(MakeCB(this,&IntrinsicEst::angYChange));
}

IntrinsicEst::~IntrinsicEst()
{
 delete win;
}

void IntrinsicEst::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void IntrinsicEst::SaveICD(gui::Base * obj,gui::Event * event)
{
 // Create it...
  cam::CameraCalibration intrinsic;
  
  math::Identity(intrinsic.intrinsic);
  intrinsic.intrinsic[0][0] = sizeX->GetReal(1200.0)*0.5/math::Tan(0.5*math::ToRad(angX->GetReal(20.0)));
  intrinsic.intrinsic[1][1] = sizeY->GetReal(800.0)*0.5/math::Tan(0.5*math::ToRad(angY->GetReal(20.0)));
  intrinsic.intrinsic[0][2] = sizeX->GetReal(1200.0)*0.5;
  intrinsic.intrinsic[1][2] = sizeY->GetReal(800.0)*0.5;
  
  
  intrinsic.radial.aspectRatio = 1.0;
  intrinsic.radial.centre[0] = sizeX->GetReal(1200.0)*0.5;
  intrinsic.radial.centre[1] = sizeY->GetReal(800.0)*0.5;
  for (nat32 i=0;i<4;i++) intrinsic.radial.k[i] = 0.0;
  
  
  intrinsic.dim[0] = sizeX->GetReal(1200.0);
  intrinsic.dim[1] = sizeY->GetReal(800.0);


 // Save it...
  str::String fn(".icd");
  if (cyclops.App().SaveFileDialog("Save .icd...",fn))
  {
   if (!fn.EndsWith(".icd")) fn += ".icd";
   if (!intrinsic.Save(fn,true))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving back the file.");
    return;	  
   }
  }
}

void IntrinsicEst::mmXChange(gui::Base * obj,gui::Event * event)
{
 if (xChange==true)
 {
  xChange = false;
  return;
 }
 xChange = true;
 
 real32 ang = math::ToDeg(cam::FocalLengthToAngleHoriz(mmX->GetReal(50.0)));

 str::String s;
 s << ang;
 angX->Set(s);
}

void IntrinsicEst::angXChange(gui::Base * obj,gui::Event * event)
{
 if (xChange==true)
 {
  xChange = false;
  return;
 }
 xChange = true;
 
 real32 length = cam::AngleToFocalLengthHoriz(math::ToRad(angX->GetReal(20.0)));

 str::String s;
 s << length;
 mmX->Set(s);
}
  
void IntrinsicEst::mmYChange(gui::Base * obj,gui::Event * event)
{
 if (yChange==true)
 {
  yChange = false;
  return;
 }
 yChange = true;
 
 real32 ang = math::ToDeg(cam::FocalLengthToAngleVert(mmY->GetReal(50.0)));

 str::String s;
 s << ang;
 angY->Set(s);
}

void IntrinsicEst::angYChange(gui::Base * obj,gui::Event * event)
{
 if (yChange==true)
 {
  yChange = false;
  return;
 }
 yChange = true;
 
 real32 length = cam::AngleToFocalLengthVert(math::ToRad(angY->GetReal(20.0)));

 str::String s;
 s << length;
 mmY->Set(s);
}

void IntrinsicEst::fromX(gui::Base * obj,gui::Event * event)
{
 real32 width = sizeX->GetReal(1200.0);
 real32 height = sizeY->GetReal(800.0);
 
 real32 ang = math::ToRad(0.5*angX->GetReal(20.0));
 
 real32 length = width*0.5/math::Tan(ang);
 
 real32 nAng = 2.0*math::InvTan(height*0.5/length);
 
 str::String s;
 s << math::ToDeg(nAng);
 angY->Set(s);
}

void IntrinsicEst::fromY(gui::Base * obj,gui::Event * event)
{
 real32 width = sizeX->GetReal(1200.0);
 real32 height = sizeY->GetReal(800.0);
 
 real32 ang = math::ToRad(0.5*angY->GetReal(20.0));
 
 real32 length = height*0.5/math::Tan(ang);
 
 real32 nAng = 2.0*math::InvTan(width*0.5/length);
 
 str::String s;
 s << math::ToDeg(nAng);
 angX->Set(s);
}

//------------------------------------------------------------------------------
