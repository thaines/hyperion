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


#include "cyclops/intrinsic.h"

//------------------------------------------------------------------------------
// Code for IntrinsicCalib class...
IntrinsicCalib::IntrinsicCalib(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),resQ(cam::Zhang98::failure)
{
 // Build the gui control panel...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
    
  win->SetTitle("Intrinsic Calibration");
  win->SetSize(1,1);
  
  gui::Horizontal * horiz = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
  gui::Vertical * vert = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
  win->SetChild(horiz);
  horiz->AttachRight(vert,false);
  
  
  gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
  but1->SetChild(lab1); lab1->Set("Add Shot...");
  but2->SetChild(lab2); lab2->Set("Calculate");
  but3->SetChild(lab3); lab3->Set("Save '.icd'...");
  but4->SetChild(lab4); lab4->Set("Store All");  

  vert->AttachBottom(but1,false);
  vert->AttachBottom(but2,false);
  vert->AttachBottom(but3,false);
  vert->AttachBottom(but4,false);

  
  gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  quality = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
  
  lab5->Set("Quality:");
  quality->Append("low");
  quality->Append("normal");
  quality->Append("high");
  quality->Set(1);
  
  vert->AttachBottom(lab5,false);
  vert->AttachBottom(quality,false);
  
  
  ml = static_cast<gui::Multiline*>(cyclops.Fact().Make("Multiline"));
  horiz->AttachRight(ml,false);
  ml->SetSize(235,320);
  ml->Edit(false);
  
  win->Resizable(false);
  cyclops.App().Attach(win);


 // Add handlers to it all, and were done - the rest just takes care of itself...
  win->OnDeath(MakeCB(this,&IntrinsicCalib::Done));
  but1->OnClick(MakeCB(this,&IntrinsicCalib::Load));
  but2->OnClick(MakeCB(this,&IntrinsicCalib::Calc));
  but3->OnClick(MakeCB(this,&IntrinsicCalib::Save));
  but4->OnClick(MakeCB(this,&IntrinsicCalib::SaveState));
}

IntrinsicCalib::~IntrinsicCalib()
{
 while (shots.Size()!=0) delete shots.Front();
 delete win;	
}

void IntrinsicCalib::Register(Shot * shot)
{
 shots.AddBack(shot);
}

void IntrinsicCalib::Unregister(Shot * shot)
{
 ds::List<Shot*>::Cursor targ = shots.FrontPtr();
 while (!targ.Bad())
 {
  if (*targ==shot)
  {
   targ.RemKillNext();
   break;	  
  }
  ++targ;	 
 }
}

void IntrinsicCalib::Done(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 if (cyclops.App().ChoiceDialog(gui::App::QuestYesNo,"Are you sure you want to finish calibration?")) delete this;
}

void IntrinsicCalib::Load(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  new Shot(cyclops,*this,fn);
 }	
}

void IntrinsicCalib::Calc(gui::Base * obj,gui::Event * event)
{
 // Check we have enough data...
  if (shots.Size()<3)
  {
   cyclops.App().MessageDialog(gui::App::MsgWarn,"Not enough data. At least 3 shots are required.");
   return;
  }
  
 // Check none of the images are in the initialisation stage...
  ds::List<Shot*>::Cursor targ = shots.FrontPtr();
  while (!targ.Bad())
  {
   if ((*targ)->DoingIni())
   {
    cyclops.App().MessageDialog(gui::App::MsgWarn,"At least one shot has not been initialised from its corners. Please complete marking calibration grid(s).");
    return;	   
   }
   ++targ;	  
  }
 
 // Clear the output, to indicate we are working... 
  ml->Empty();

 // Create the calibration object and fill it with the data collected...
  cam::Zhang98 cc;
  
  switch(quality->Get())
  {
   case 0: cc.SetQuality(cam::Zhang98::low); break;
   case 1: cc.SetQuality(cam::Zhang98::normal); break;
   case 2: cc.SetQuality(cam::Zhang98::high); break;
  }
  
  nat32 sn = 0;
  targ = shots.FrontPtr();
  while (!targ.Bad())
  {
   for (nat32 x=0;x<Shot::gridSize;x++)
   {
    for (nat32 y=0;y<Shot::gridSize;y++)
    {
     math::Vect<2> model; model[0] = (*targ)->ToModel(x); model[1] = 23 - (*targ)->ToModel(y);
     math::Vect<2> seen = (*targ)->pos[x][y];
     // Adjust the Y axis, as we work with bottom-left coordinates rather than the gui top-left...
      seen[1] = (*targ)->Height()-1.0 - seen[1];
     cc.Add(sn,model,seen);
    }
   }
  
   ++sn;
   ++targ;   
  }
  
 // Do the work and stick the final result into the text area...
  cc.Calculate(cyclops.BeginProg());
  cyclops.EndProg();
  
  resQ = cc.GetQuality();
  imgWidth = shots.Front()->Width();
  imgHeight = shots.Front()->Height();

  residual = cc.GetResidual();
  intrinsic = cc.GetIntrinsic();
  radial = cc.GetRadial();

  
  str::String str;
   
   switch (resQ)
   {
    case cam::Zhang98::failure: str << "Quality: failure\n"; break;
    case cam::Zhang98::low:     str << "Quality: low\n";     break;
    case cam::Zhang98::normal:  str << "Quality: normal\n";  break;
    case cam::Zhang98::high:    str << "Quality: high\n";    break;
   }
   
   if (resQ!=cam::Zhang98::failure)
   {
    str << "Residual = " << residual << "\n";
    real32 focalH = FocalLength35mmHoriz(imgWidth,imgHeight,intrinsic,&radial);
    real32 focalV = FocalLength35mmVert(imgWidth,imgHeight,intrinsic,&radial);
    real32 focalD = FocalLength35mmDiag(imgWidth,imgHeight,intrinsic,&radial);
    str << "Horizontal: " << focalH << "mm\n";
    str << "Vertical: " << focalV << "mm\n";
    str << "Diagonal: " << focalD << "mm\n\n";

    str << "Intrinsic:\n";
    str << " Focal X = " << intrinsic.FocalX() << "\n";
    str << " Focal Y = " << intrinsic.FocalY() << "\n";
    str << " Principal X = " << intrinsic.PrincipalX() << "\n";
    str << " Principal Y = " << intrinsic.PrincipalY() << "\n";  
    str << " Skew = " << intrinsic.Skew() << "\n\n";
  
    str << "Radial:\n";
    str << " Aspect Ratio = " << radial.aspectRatio << "\n";
    str << " Centre X = " << radial.centre[0] << "\n";
    str << " Centre Y = " << radial.centre[1] << "\n";   
    str << " d factor = " << radial.k[0] << "\n";
    str << " d*d factor = " << radial.k[1] << "\n";
    str << " d*d*d factor = " << radial.k[2] << "\n";
    str << " d*d*d*d factor = " << radial.k[3] << "\n";
    
    str << "\nExtrinsic parameters:\n";
    for (nat32 i=0;i<cc.GetShots();i++)
    {
     str << "Shot " << i << ":\n" << cc.GetExtrinsic(i) << "\n";
    }
   }
  
  ml->Append(str);
}

void IntrinsicCalib::Save(gui::Base * obj,gui::Event * event)
{
 if (resQ==cam::Zhang98::failure)
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
     cc.intrinsic = intrinsic;
     cc.radial = radial;
     cc.dim[0] = imgWidth;
     cc.dim[1] = imgHeight;

    if (!cc.Save(fn,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;	  
    }
  }
}

void IntrinsicCalib::SaveState(gui::Base * obj,gui::Event * event)
{
 // Iterate the shots, for each one save an xml file with the same 
 // name storing the coordinates of the grid selections, so they can be
 // reloaded when an image is loaded...
 ds::List<Shot*>::Cursor targ = shots.FrontPtr();
 while (!targ.Bad())
 {
  if (!(*targ)->DoingIni())
  {
   // Create the relevent (and rather simplistic) dom...
    bs::Element * root = new bs::Element(cyclops.TT(),"c-grid");
    str::String data;
    data << (*targ)->pos;
    root->SetAttribute("points",data);
   
   // Generate the filename...
    str::String filename = (*targ)->Filename();
    filename.SetSize(filename.Size()-3);
    filename << "cgr";
  
   // Save, spitting out a warning on failure...
    if (file::SaveXML(root,filename,true)==false)
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving a file.");
    }
   
   // Clean up...
    delete root;
  }
  ++targ;
 }

}

//------------------------------------------------------------------------------
Shot::Shot(Cyclops & cyc,IntrinsicCalib & ica,const str::String & fn)
:cyclops(cyc),intCalib(ica),win(null<gui::Window*>()),
filename(fn),var(null<svt::Var*>()),harrisSnap(null<filter::HarrisSnap*>())
{
 // Attempt to open the image - if we can't then we warn the user and commit
 // suicide...	
  cstr lfn = fn.ToStr();
  svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),lfn);
  mem::Free(lfn);
  if (floatVar==null<svt::Var*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
   delete this;
   return;	  
  }
  
 // Check it matches the size of any previous images...
  Shot * prev = intCalib.FirstShot();
  if ((prev)&&((prev->Width()!=floatVar->Size(0))||(prev->Height()!=floatVar->Size(1))))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Image size does not match that of allready loaded images");
   delete this;
   return;  
  }
  
 // Image loaded, but its not in a format suitable for fast display - convert...
  var = filter::MakeByteRGB(floatVar);
  var->ByName("rgb",image);
  delete floatVar;
 
 // We have loaded the image - build a window to display it, add handlers and 
 // register ourself...
  // Window...
   win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
   win->SetTitle(fn);
   cyclops.App().Attach(win);
   win->SetSize(math::Min<nat32>(image.Size(0)+8,800),math::Min<nat32>(image.Size(1)+48,600));


  // Toolbar...
   gui::Vertical * vert = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert);
   
   gui::Horizontal * horiz = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert->AttachBottom(horiz,false);
   
   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   but1->SetChild(lab1); lab1->Set("Restart");
   but2->SetChild(lab2); lab2->Set("Restore");
   but3->SetChild(lab3); lab3->Set("Store");
   but4->SetChild(lab4); lab4->Set("Snap");
   
   horiz->AttachRight(but1,false);
   horiz->AttachRight(but2,false);
   horiz->AttachRight(but3,false);
   horiz->AttachRight(but4,false);
   
   
  // Image... 
   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert->AttachBottom(panel);
   
   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);
   
   canvas->SetSize(image.Size(0),image.Size(1));

   
  // Handlers...
   win->OnDeath(MakeCB(this,&Shot::Quit));
   canvas->OnResize(MakeCB(this,&Shot::Resize));
   canvas->OnClick(MakeCB(this,&Shot::Click));
   canvas->OnMove(MakeCB(this,&Shot::Move));
   
   but1->OnClick(MakeCB(this,&Shot::Reset));
   but2->OnClick(MakeCB(this,&Shot::Restore));
   but3->OnClick(MakeCB(this,&Shot::Store));
   but4->OnClick(MakeCB(this,&Shot::Snap));
 
    
  // Self register...
   intCalib.Register(this);

    
  // Starting state...
   doingIni = true;
   iniStep = 0;
   
  // Now try and load a cgr file, to resume any previous calibration work...
   // Generate a filename...
    cgFn = filename;
    cgFn.SetSize(cgFn.Size()-3);
    cgFn << "cgr";
    
   // Try to open the filename...
    bs::Element * dom = file::LoadXML(cyclops.TT(),cgFn);
    if (dom)
    {
     LogDebug("Found cgr file to match image");
     // Try to read the matrix in...
      str::String def;
      str::String mat = dom->GrabString(".points",def);
      str::String::Cursor mTarg = mat.GetCursor();
      mTarg.ClearError();
      mTarg >> pos;
      if (!mTarg.Error())
      {
       // On success use the loaded data...
        doingIni = false;
        iniStep = 4;
      }

     delete dom;
    }
}

Shot::~Shot()
{
 intCalib.Unregister(this);
 delete win;
 delete var;
 delete harrisSnap;
}

void Shot::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 cstrconst msg = "Are you sure you want to remove this shot from the calibration set?";
 if (cyclops.App().ChoiceDialog(gui::App::QuestYesNo,msg)) delete this;
}

void Shot::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2; 
 
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);
  
 // Depending on the mode either render the pre-calibration square or the full calibration grid...
  if (doingIni)
  {
   // Render the 4 corners...
    if (iniStep>0) RenderPoint(pos[0][0],bs::ColourRGB(1.0,0.0,1.0));
    if (iniStep>1) RenderPoint(pos[gridSize-1][0],bs::ColourRGB(1.0,0.0,1.0));
    if (iniStep>2) RenderPoint(pos[gridSize-1][gridSize-1],bs::ColourRGB(1.0,0.0,1.0));
    if (iniStep>3) RenderPoint(pos[0][gridSize-1],bs::ColourRGB(1.0,0.0,1.0));
  }
  else
  {
   // Squares... 
    // Horizontal...
     for (nat32 y=0;y<gridSize;y++)
     {
      for (nat32 x=0;x<gridSize;x+=2) RenderLine(pos[x][y],pos[x+1][y],bs::ColourRGB(1.0,0.0,0.0));
     }
    
    // Vertical...
     for (nat32 x=0;x<gridSize;x++)
     {
      for (nat32 y=0;y<gridSize;y+=2) RenderLine(pos[x][y],pos[x][y+1],bs::ColourRGB(1.0,0.0,0.0));
     }    
  
   // Points...
    for (nat32 x=0;x<gridSize;x++)
    {
     for (nat32 y=0;y<gridSize;y++) RenderPoint(pos[x][y],bs::ColourRGB(1.0,0.0,1.0));
    }
  }
  
 canvas->Update();
}

void Shot::Click(gui::Base * obj,gui::Event * event)
{
 // Determine the mouse position in image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  if (mbe->down==false) return;
  math::Vect<2,real32> mp;
  
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2; 
  
  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;
 
 // If in iniMode we put the next corner point, if not we move the closest point to
 // the cursor to where we currently are...
 if (doingIni)
 {
  if (iniStep==0) pos[0][0] = mp;
  if (iniStep==1) pos[gridSize-1][0] = mp;
  if (iniStep==2) pos[gridSize-1][gridSize-1] = mp;
  if (iniStep==3) pos[0][gridSize-1] = mp;      
  ++iniStep;
  if (iniStep==4)
  {
   doingIni = false;
   // We are moving to full adjustment mode - create a homography from the 4 points
   // we have and position the rest based on it...
    cam::Homography2D hg;
    math::Vect<2,real32> mp;
     mp[0] = ToModel(0);          mp[1] = ToModel(0);          hg.Add(mp,pos[0][0]);
     mp[0] = ToModel(gridSize-1); mp[1] = ToModel(0);          hg.Add(mp,pos[gridSize-1][0]);
     mp[0] = ToModel(gridSize-1); mp[1] = ToModel(gridSize-1); hg.Add(mp,pos[gridSize-1][gridSize-1]);
     mp[0] = ToModel(0);          mp[1] = ToModel(gridSize-1); hg.Add(mp,pos[0][gridSize-1]);            
     
    math::Mat<3,3> res;
    hg.Result(res);
    
    for (nat32 x=0;x<gridSize;x++)
    {
     for (nat32 y=0;y<gridSize;y++)
     {
      math::Vect<3> in; in[0] = ToModel(x); in[1] = ToModel(y); in[2] = 1.0;
      math::Vect<3> out;
      MultVect(res,in,out);
      pos[x][y][0] = out[0]/out[2];
      pos[x][y][1] = out[1]/out[2];      
     }
    }
  }
 }
 else
 {
  // Find closest point to click position...
   nat32 cx = 0;
   nat32 cy = 0;
   real32 cDist = 1e100;
   
   for (nat32 x=0;x<gridSize;x++)
   {
    for (nat32 y=0;y<gridSize;y++)
    {
     real32 dist = math::Sqrt(math::Sqr(pos[x][y][0] - mp[0]) + math::Sqr(pos[x][y][1] - mp[1]));  
     if (dist<cDist)
     {
      cx = x;
      cy = y;
      cDist = dist;
     }
    } 
   }
  
  // Set closest point to be click position, if its within a tolerance...
   if (cDist<=25.0)
   {
    pos[cx][cy] = mp;
   }
 }
 
 canvas->Redraw();
}

void Shot::Move(gui::Base * obj,gui::Event * event)
{
 gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
 if (!mme->lmb) return;
 gui::MouseButtonEvent mbe;
  mbe.x = mme->x;
  mbe.y = mme->y;
  mbe.down = true;

 Click(obj,&mbe);
}

void Shot::Reset(gui::Base * obj,gui::Event * event)
{
 if (cyclops.App().ChoiceDialog(gui::App::QuestYesNo,"You will lose all edits for this image. Are you sure?"))
 {
  doingIni = true;
  iniStep = 0;
  canvas->Redraw();
 }
}

void Shot::Restore(gui::Base * obj,gui::Event * event)
{
 if (cyclops.App().ChoiceDialog(gui::App::QuestYesNo,"You will lose all edits for this image. Are you sure?"))
 {
  bs::Element * dom = file::LoadXML(cyclops.TT(),cgFn);
  if (dom)
  {
   // Try to read the matrix in...
    str::String def;
    str::String mat = dom->GrabString(".points",def);
    str::String::Cursor mTarg = mat.GetCursor();
    mTarg.ClearError();
    mTarg >> pos;
    if (!mTarg.Error())
    {
     doingIni = false;
     iniStep = 4;
    }
    else
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load stored details - corrupted file.");
     doingIni = false;
     iniStep = 4;    
    }
   delete dom;
  }
  else
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Could not find stored details for image.");	  
  }
  canvas->Redraw();
 }	
}

void Shot::Store(gui::Base * obj,gui::Event * event)
{
 // Create the relevent (and rather simplistic) dom...
  bs::Element * root = new bs::Element(cyclops.TT(),"c-grid");
  str::String data;
  data << pos;
  root->SetAttribute("points",data);
  
 // Save, spitting out a warning on failure...
  if (file::SaveXML(root,cgFn,true)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving a file.");
  }
   
 // Clean up...
  delete root;
}

void Shot::Snap(gui::Base * obj,gui::Event * event)
{
 // If we havn't got a grid we can't snap...
  if (doingIni)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"You can only snap once the initialalisation corners have been done.");
   return;	  
  }
 
 
 // If not snap image exists create it...
  if (harrisSnap==null<filter::HarrisSnap*>())
  {
   harrisSnap = new filter::HarrisSnap();
   
   // Create floating point greyscale image...
    svt::Var varFG(image);
    real32 iniReal = 0.0;
    varFG.Add("l",iniReal);
    varFG.Commit();
    
    svt::Field<real32> grey(&varFG,"l");
    for (nat32 y=0;y<grey.Size(1);y++)
    {
     for (nat32 x=0;x<grey.Size(0);x++)
     {
      bs::ColourL l = image.Get(x,y);
      grey.Get(x,y) = l.l;
     }	    
    }
   
   // Run algortihm, present it on the cyclops progress bar...
    harrisSnap->Set(grey);
    time::Progress * prog = cyclops.BeginProg();
    harrisSnap->Run(prog);
    cyclops.EndProg();
  }
 
 
 // Apply the snap image to all points...
  for (nat32 r=0;r<gridSize;r++)
  {
   for (nat32 c=0;c<gridSize;c++)
   {
    math::Vect<2,real32> & targ = pos[r][c];
    
    bs::Pnt snap;
    int32 bx = int32(math::Round(targ[0]));
    int32 by = int32(math::Round(image.Size(1)-1.0 - targ[1]));
    harrisSnap->Get(bx,by,snap);
    
    targ[0] = snap[0];
    targ[1] = image.Size(1)-1.0 - snap[1];
   }
  }
  canvas->Redraw();
}

void Shot::RenderPoint(const math::Vect<2,real32> & p,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - image.Size(0))/2;
 nat32 sy = (canvas->P().Height() - image.Size(1))/2; 
 
 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);
  
 canvas->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 canvas->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col); 
}

void Shot::RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - image.Size(0))/2;
 nat32 sy = (canvas->P().Height() - image.Size(1))/2;
 
 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);
 
 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);
 
 canvas->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col); 
}

//------------------------------------------------------------------------------
