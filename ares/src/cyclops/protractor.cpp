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


#include "cyclops/protractor.h"

//------------------------------------------------------------------------------
Protractor::Protractor(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),var(null<svt::Var*>()),nor(false)
{
 // Create default image and calibration...
  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);
  
  math::Identity(calib.intrinsic);
  calib.radial.aspectRatio = 0.75;
  calib.radial.centre[0] = 160.0;
  calib.radial.centre[1] = 120.0;
  for (nat32 i=0;i<calib.radial.k.Size();i++) calib.radial.k[i] = 0.0;
  calib.dim[0] = 320.0;
  calib.dim[1] = 240.0;
  
  pa[0] = 107.0;
  pa[1] = 120.0;
  pb[0] = 214.0;
  pb[1] = 120.0;


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Protractor");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+48);
   
   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);
   
   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   
   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   angle = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Load Calibration...");
   angle->Set("");
   
   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(angle,false);

   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);
   
   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);
   
   canvas->SetSize(image.Size(0),image.Size(1));

 // Event handlers...
  win->OnDeath(MakeCB(this,&Protractor::Quit));
  canvas->OnResize(MakeCB(this,&Protractor::Resize));
  canvas->OnClick(MakeCB(this,&Protractor::Click));
  canvas->OnMove(MakeCB(this,&Protractor::Move));
  but1->OnClick(MakeCB(this,&Protractor::LoadImage));
  but2->OnClick(MakeCB(this,&Protractor::LoadIntrinsic));
}

Protractor::~Protractor()
{
 delete win;
 delete var;
}

void Protractor::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Protractor::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2; 
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // Render the angle measurement line...
  RenderLine(pa,pb,bs::ColourRGB(1.0,0.0,1.0));
 
 // Update...
  canvas->Update();
}

void Protractor::Click(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;
  
  real32 sx = (canvas->P().Width() - image.Size(0))/2;
  real32 sy = (canvas->P().Height() - image.Size(1))/2; 

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;
  
  if (mbe->button==gui::MouseButtonEvent::LMB)
  {
   // Move the nearest end point...
    real32 da = math::Sqr(mp[0]-pa[0]) + math::Sqr(mp[1]-pa[1]);
    real32 db = math::Sqr(mp[0]-pb[0]) + math::Sqr(mp[1]-pb[1]);
  
    if (da<db) pa = mp;
          else pb = mp;
  }
  else
  {
   if (mbe->down)
   {
    // Snap the next end point...
     if (nor) pb = mp;
         else pa = mp;
     nor = !nor;
   }
  }
        
  CalcAngle();
 
 // Redraw the display... 
  canvas->Redraw();
}

void Protractor::Move(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;
  
  real32 sx = (canvas->P().Width() - image.Size(0))/2;
  real32 sy = (canvas->P().Height() - image.Size(1))/2; 
  
  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;
  
 // Move the nearest end point...
  real32 da = math::Sqr(mp[0]-pa[0]) + math::Sqr(mp[1]-pa[1]);
  real32 db = math::Sqr(mp[0]-pb[0]) + math::Sqr(mp[1]-pb[1]);
  
  if (da<db) pa = mp;
        else pb = mp;
        
  //CalcAngle(); // Too expensive - only do it on the actual changes in button state.
 
 // Redraw the display... 
  canvas->Redraw();
}

void Protractor::LoadImage(gui::Base * obj,gui::Event * event)
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
   
   pa[0] = real32(image.Size(0))/3.0;
   pa[1] = real32(image.Size(1))/2.0;
   pb[0] = real32(image.Size(0))*2.0/3.0;
   pb[1] = pa[1];
   
   CalcAngle();
   
  // Refresh the display...
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
 }
}

void Protractor::LoadIntrinsic(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Intrinsic Calibration","*.icd",fn))
 {
  if (calib.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
  }
  else
  {
   CalcAngle();
  }
 }
}

void Protractor::CalcAngle()
{
 // Factor in change of image size, and the origin change...
  math::Vect<2,real64> la;
  math::Vect<2,real64> lb;
   la[0] = calib.dim[0]*pa[0]/real64(image.Size(0));
   la[1] = calib.dim[1]*(image.Size(1)-1.0-pa[1])/real64(image.Size(1));
   lb[0] = calib.dim[0]*pb[0]/real64(image.Size(0));
   lb[1] = calib.dim[1]*(image.Size(1)-1.0-pb[1])/real64(image.Size(1));
 
 // Remove radial distortion...
  math::Vect<2,real64> ma;
  math::Vect<2,real64> mb;  
  calib.radial.UnDis(la,ma);
  calib.radial.UnDis(lb,mb);
 
 // Calculate direction vectors...
  math::Mat<3,3,real64> invInt = calib.intrinsic;
  math::Mat<3,3,real64> temp;
  math::Inverse(invInt,temp);
 
  math::Vect<3,real64> na;
  math::Vect<3,real64> nb;
  math::Vect<3,real64> oa;
  math::Vect<3,real64> ob;
  
  na[0] = ma[0]; na[1] = ma[1]; na[2] = 1.0;
  nb[0] = mb[0]; nb[1] = mb[1]; nb[2] = 1.0;
    
  math::MultVect(invInt,na,oa);
  math::MultVect(invInt,nb,ob);
 
 // Take the dot product and get the angle...
  oa.Normalise();
  ob.Normalise();
  real64 ang = math::ToDeg(math::InvCos(oa*ob));
 
 // Update the angle readout...
  str::String s;
  s << "Angle = " << ang << " degrees";
  angle->Set(s);
}

void Protractor::RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col)
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
