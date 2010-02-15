//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


#include "cyclops/capture.h"

//------------------------------------------------------------------------------
Capture::Capture(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>()),
leftName(null<cstr>()),leftPort(null<cstr>()),
rightName(null<cstr>()),rightPort(null<cstr>()),
leftConfig(null<bs::Element*>()),rightConfig(null<bs::Element*>())
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


 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Capture");
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
  
   but1->SetChild(lab1); lab1->Set("Select Left...");
   but2->SetChild(lab2); lab2->Set("Select Right...");
   
   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert2,false);
   vert2->AttachBottom(but1);
   vert2->AttachBottom(but2);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   but3->SetChild(lab3); lab3->Set("Config Left...");
   but4->SetChild(lab4); lab4->Set("Config Right...");
   
   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(but3);
   vert3->AttachBottom(but4);


   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   but5->SetChild(lab5); lab5->Set("Load Config...");
   but6->SetChild(lab6); lab6->Set("Save Config...");
   
   gui::Vertical * vert4 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert4,false);
   vert4->AttachBottom(but5);
   vert4->AttachBottom(but6);


   gui::Button * but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));   
   but7->SetChild(lab7); lab7->Set("Capture");
   horiz1->AttachRight(but7,false);
   

   gui::Button * but7b = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but7c = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab7b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7c = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   but7b->SetChild(lab7b); lab7b->Set("Capture Left");
   but7c->SetChild(lab7c); lab7c->Set("Capture Right");
   
   gui::Vertical * vert6 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert6,false);
   vert6->AttachBottom(but7b);
   vert6->AttachBottom(but7c);

   
   gui::Button * but8 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but9 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   but8->SetChild(lab8); lab8->Set("Save Left...");
   but9->SetChild(lab9); lab9->Set("Save Right...");
   
   gui::Vertical * vert5 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert5,false);
   vert5->AttachBottom(but8);
   vert5->AttachBottom(but9);
   
   
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
  win->OnDeath(MakeCB(this,&Capture::Quit));
  
  left->OnResize(MakeCB(this,&Capture::ResizeLeft));
  right->OnResize(MakeCB(this,&Capture::ResizeRight));
  
  but1->OnClick(MakeCB(this,&Capture::SelectLeft));
  but2->OnClick(MakeCB(this,&Capture::SelectRight));
  but3->OnClick(MakeCB(this,&Capture::ConfigLeft));
  but4->OnClick(MakeCB(this,&Capture::ConfigRight));
  but5->OnClick(MakeCB(this,&Capture::LoadConfig));
  but6->OnClick(MakeCB(this,&Capture::SaveConfig));
  but7->OnClick(MakeCB(this,&Capture::CaptureNow));
  but7b->OnClick(MakeCB(this,&Capture::CaptureLeft));
  but7c->OnClick(MakeCB(this,&Capture::CaptureRight));  
  but8->OnClick(MakeCB(this,&Capture::SaveLeft));
  but9->OnClick(MakeCB(this,&Capture::SaveRight));
}

Capture::~Capture()
{
 delete win;
 delete leftVar;
 delete rightVar;
 
 mem::Free(leftName);
 mem::Free(leftPort);
 mem::Free(rightName);
 mem::Free(rightPort);
 
 delete leftConfig;
 delete rightConfig;
}

void Capture::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Capture::ResizeLeft(gui::Base * obj,gui::Event * event)
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

void Capture::ResizeRight(gui::Base * obj,gui::Event * event)
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

void Capture::SelectLeft(gui::Base * obj,gui::Event * event)
{
 new SelectCamera(cyclops,cd,leftName,leftPort,leftPtp,leftConfig);
}

void Capture::SelectRight(gui::Base * obj,gui::Event * event)
{
 new SelectCamera(cyclops,cd,rightName,rightPort,rightPtp,rightConfig);
}

void Capture::ConfigLeft(gui::Base * obj,gui::Event * event)
{
 if (leftConfig)
 {
  new EditXML(cyclops,leftConfig);
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgWarn,"No configuration to edit");
 }
}

void Capture::ConfigRight(gui::Base * obj,gui::Event * event)
{
 if (rightConfig)
 {
  new EditXML(cyclops,rightConfig);
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgWarn,"No configuration to edit");
 }
}

void Capture::LoadConfig(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Capture Configuration","*.capture",fn))
 {
  bs::Element * root = file::LoadXML(cyclops.TT(),fn);
  if (root)
  {      
   mem::Free(leftName); leftName = root->GrabString(":left.model",str::String("")).ToStr();
   mem::Free(leftPort); leftPort = root->GrabString(":left.port",str::String("")).ToStr();
   leftPtp = root->GrabBit(":left.ptp",false);

   mem::Free(rightName); rightName = root->GrabString(":right.model",str::String("")).ToStr();
   mem::Free(rightPort); rightPort = root->GrabString(":right.port",str::String("")).ToStr();
   rightPtp = root->GrabBit(":right.ptp",false);
   
   delete leftConfig; leftConfig = null<bs::Element*>();
   bs::Element * left = root->GetElement("left");
   if (left)
   {
    bs::Element * config = left->GetElement("config");
    if ((config)&&(config->Front()!=config->Bad()))
    {
     leftConfig = new bs::Element(*config->Front());
    }
   }

   delete rightConfig; rightConfig = null<bs::Element*>();
   bs::Element * right = root->GetElement("right");
   if (right)
   {
    bs::Element * config = right->GetElement("config");
    if ((config)&&(config->Front()!=config->Bad()))
    {
     rightConfig = new bs::Element(*config->Front());
    }
   }
   
   delete root;
  }
  else
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error loading file");
  }
 }
}

void Capture::SaveConfig(gui::Base * obj,gui::Event * event)
{
 if (leftConfig&&rightConfig)
 {
  // Get the filename...
   str::String fn("");
   if (cyclops.App().SaveFileDialog("Save Capture Configuration",fn))
   {
    if (!fn.EndsWith(".capture")) fn += ".capture";

    bs::Element * root = new bs::Element(cyclops.TT(),"capture");
    bs::Element * left = root->NewChild("left");
    bs::Element * right = root->NewChild("right");
    bs::Element * leftC = left->NewChild("config");
    bs::Element * rightC = right->NewChild("config");
   
    left->SetAttribute("model",leftName);
    left->SetAttribute("port",leftPort);
    left->SetAttribute("ptp",leftPtp);
    
    right->SetAttribute("model",rightName);
    right->SetAttribute("port",rightPort);
    right->SetAttribute("ptp",rightPtp);    
   
    leftC->AppendChild(new bs::Element(*leftConfig));
    rightC->AppendChild(new bs::Element(*rightConfig));
   
    cstr fs = fn.ToStr();
     if (file::SaveXML(root,fs,true)==false)
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving file");
     }
    mem::Free(fs);
    
    delete root;
   }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgWarn,"Incomplete/invalid configuration - can not save");
 }
}

void Capture::CaptureNow(gui::Base * obj,gui::Event * event)
{
 // Try and create the two cameras...
  os::Camera left;
  os::Camera right;
  
  cd.Refresh();

  nat32 leftIndex;
  if (cd.Search(leftName,leftPort,leftIndex)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not find left camera - is it connected and switched on?");
   return;
  }
  if (cd.Link(leftIndex,&left,leftPtp)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not connect to left camera.");
   return;
  }

  nat32 rightIndex;
  if (cd.Search(rightName,rightPort,rightIndex)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not find right camera - is it connected and switched on?");
   return;
  }
  if (cd.Link(rightIndex,&right,rightPtp)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not connect to right camera.");
   return;
  }


 // Apply the configurations...
  if (leftConfig) left.SetConf(leftConfig);
  if (rightConfig) right.SetConf(rightConfig);


 // Create a thread for each camera into which the final capture and return shall go...
  CaptureThread leftCT(left,leftVar,leftImage);
  CaptureThread rightCT(right,rightVar,rightImage);
  
  leftCT.Run();
  leftCT.Wait(); // ****************************
  rightCT.Run();

 // Wait for the threads to finish...
  //leftCT.Wait();
  rightCT.Wait();

 // Disconnect and cleanup...
  left.Deactivate();
  right.Deactivate();
  
 // Scale the canvases and redraw...
  this->left->SetSize(leftImage.Size(0),leftImage.Size(1));
  this->left->Redraw();
  this->right->SetSize(rightImage.Size(0),rightImage.Size(1));
  this->right->Redraw();  
}

void Capture::CaptureLeft(gui::Base * obj,gui::Event * event)
{
 // As CaptureNow, but just the one image...
  os::Camera left; 
  cd.Refresh();
 
  nat32 leftIndex;
  if (cd.Search(leftName,leftPort,leftIndex)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not find left camera - is it connected and switched on?");
   return;
  }
  if (cd.Link(leftIndex,&left,leftPtp)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not connect to left camera.");
   return;
  }
  
  if (leftConfig) left.SetConf(leftConfig);
  
  CaptureThread leftCT(left,leftVar,leftImage);
  leftCT.Run();
  leftCT.Wait();
  
  left.Deactivate();
  
  this->left->SetSize(leftImage.Size(0),leftImage.Size(1));
  this->left->Redraw();
}

void Capture::CaptureRight(gui::Base * obj,gui::Event * event)
{
 // As CaptureNow, but just the one image...
  os::Camera right; 
  cd.Refresh();
 
  nat32 rightIndex;
  if (cd.Search(rightName,rightPort,rightIndex)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not find right camera - is it connected and switched on?");
   return;
  }
  if (cd.Link(rightIndex,&right,rightPtp)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not connect to right camera.");
   return;
  }
  
  if (rightConfig) right.SetConf(rightConfig);
  
  CaptureThread rightCT(right,rightVar,rightImage);
  rightCT.Run();
  rightCT.Wait();
  
  right.Deactivate();
  
  this->right->SetSize(rightImage.Size(0),rightImage.Size(1));
  this->right->Redraw();    
}

void Capture::SaveLeft(gui::Base * obj,gui::Event * event)
{
 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Left Image",fn))
  {
   // Convert to something we can save...
    svt::Var leftImg(leftImage);
     bs::ColourRGB rgbIni(0.0,0.0,0.0);
     leftImg.Add("rgb",rgbIni);
    leftImg.Commit();
    
    svt::Field<bs::ColourRGB> out(&leftImg,"rgb");
    for (nat32 y=0;y<out.Size(1);y++)
    {
     for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = leftImage.Get(x,y);
    }
   
   // Save the image...
    if (!fn.EndsWith(".bmp")) fn += ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImageRGB(&leftImg,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
    }
    mem::Free(ts); 
  }
}

void Capture::SaveRight(gui::Base * obj,gui::Event * event)
{
 // Get the filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Right Image",fn))
  {
   // Convert to something we can save...
    svt::Var rightImg(rightImage);
     bs::ColourRGB rgbIni(0.0,0.0,0.0);
     rightImg.Add("rgb",rgbIni);
    rightImg.Commit();
    
    svt::Field<bs::ColourRGB> out(&rightImg,"rgb");
    for (nat32 y=0;y<out.Size(1);y++)
    {
     for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = rightImage.Get(x,y);
    }
   
   // Save the image...
    if (!fn.EndsWith(".bmp")) fn += ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImageRGB(&rightImg,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
    }
    mem::Free(ts); 
  }
}

//------------------------------------------------------------------------------
SelectCamera::SelectCamera(Cyclops & cyc,os::CameraDetector & cd,cstr & n,cstr & p,bit & pb,bs::Element *& c)
:cyclops(cyc),camD(cd),name(n),port(p),ptp(pb),config(c),
win(null<gui::Window*>())
{
 // Create the gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Select Camera");
  win->SetSize(256,1);
  cyclops.App().Attach(win);
  
  gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
  win->SetChild(vert1);  

  camSel = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
  vert1->AttachBottom(camSel);
  
  ptpTick = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
  gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  ptpTick->SetChild(lab2); lab2->Set("PTP Mode");
  vert1->AttachBottom(ptpTick);  

  gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  but1->SetChild(lab1); lab1->Set("Store");
  vert1->AttachBottom(but1);


 // Attach handlers...
  win->OnDeath(MakeCB(this,&SelectCamera::Quit));
  but1->OnClick(MakeCB(this,&SelectCamera::Store));

 // Populate the camera list...
  if (camD.Refresh()==false)
  {
   camSel->Append("Error detecting cameras");
   camSel->Set(0);
  }
  else
  {
   if (camD.Size()==0)
   {
    camSel->Append("No cameras detected");
    camSel->Set(0);
   }
   else
   {
    for (nat32 i=0;i<camD.Size();i++)
    {
     str::String s;
      s += camD.Name(i);
      s += " - ";
      s += camD.Port(i);
      
     cstr st = s.ToStr();
      camSel->Append(st);
     mem::Free(st);
    }
    camSel->Set(0);
    if (name&&port)
    {
     for (nat32 i=0;i<camD.Size();i++)
     {
      if ((str::Compare(camD.Name(i),name)==0)&&
          (str::Compare(camD.Port(i),port)==0))
      {
       camSel->Set(i);
       break;
      }
     }
    }
   }
  }
}

SelectCamera::~SelectCamera()
{
 delete win;
}

void SelectCamera::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void SelectCamera::Store(gui::Base * obj,gui::Event * event)
{
 // Extract the user selection back to the main window...
  if (camSel->Get()<camD.Size())
  {
   // The basics...
    mem::Free(name); name = str::Duplicate(camD.Name(camSel->Get()));
    mem::Free(port); port = str::Duplicate(camD.Port(camSel->Get()));
    ptp = ptpTick->Ticked();
    
   // Now connect, for only a moment, and grab the cameras configuration...
    delete config; config = null<bs::Element*>();
   
    os::Camera c;
    if (camD.Link(camSel->Get(),&c,ptp))
    {
     config = c.GetConf(cyclops.TT());
     c.Deactivate();
    }
  }
 
 // Exit selection dialog...
  delete this;
}

//------------------------------------------------------------------------------
EditXML::EditXML(Cyclops & cyc,bs::Element *& d)
:cyclops(cyc),data(d),win(null<gui::Window*>())
{
 // Create the gui - quite simple really...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Select Camera");
  win->SetSize(512,384);
  cyclops.App().Attach(win);
  
  gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
  win->SetChild(vert1);  

  gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
  vert1->AttachBottom(panel);
  ml = static_cast<gui::Multiline*>(cyclops.Fact().Make("Multiline"));
  ml->Edit(true);
  panel->SetChild(ml);
  
  gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  but1->SetChild(lab1); lab1->Set("Store");
  vert1->AttachBottom(but1,false);


 // Add handlers...
  win->OnDeath(MakeCB(this,&EditXML::Quit));
  but1->OnClick(MakeCB(this,&EditXML::Store));

 // Populate the multipline edit with the (humanised) XML...
  data->MakeHuman();
  str::String s;
  s << *data;
  ml->Append(s);
}

EditXML::~EditXML()
{
 delete win;
}

void EditXML::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void EditXML::Store(gui::Base * obj,gui::Event * event)
{
 // Update the XML, assuming its possible, i.e. the user hasn't broken it...
  str::String s;
  ml->GetAll(s);
  
  bs::Element * newData = new bs::Element(data->TokTab(),"");
  str::String::Cursor c = s.GetCursor();
  c >> *newData;
  if (c.Error())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"There is an error in your XML");
   delete newData;
   return;
  }
  else
  {
   delete data;
   data = newData;
  }
  

 // Exit dialog...
  delete this;
}

//------------------------------------------------------------------------------
CaptureThread::CaptureThread(os::Camera & c,svt::Var * o,svt::Field<bs::ColRGB> & f)
:cam(c),out(o),field(f)
{}

CaptureThread::~CaptureThread()
{}

void CaptureThread::Execute()
{
 LogBlock("void CaptureThread::Execute()","-");

 file::ImageRGB * img = cam.Capture();
 if (img)
 {
  LogDebug("[cyclops.capture] Captured an image {width,height}" << LogDiv() << img->Width() << LogDiv() << img->Height());
  out->Setup2D(img->Width(),img->Height());
  out->Commit();
  
  out->ByName("rgb",field);
  for (nat32 y=0;y<img->Height();y++)
  {
   for (nat32 x=0;x<img->Width();x++)
   {
    field.Get(x,y) = img->Get(x,y);
   }
  }
  
  delete img;
 }
 else
 {
  LogAlways("[cyclops.capture] Capture failed to return an image");
 }
}

//------------------------------------------------------------------------------
