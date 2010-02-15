//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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


#include "cyclops/cam_response.h"

//------------------------------------------------------------------------------
CamResponse::CamResponse(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),expRec(false),
irrVar(null<svt::Var*>()),imageVar(null<svt::Var*>()),imgVar(null<svt::Var*>())
{
 // Create default images maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  bs::ColRGB colIni(0,0,0);

  irrVar = new svt::Var(cyclops.Core());
  irrVar->Setup2D(320,240);
  irrVar->Add("rgb",colourIni);
  irrVar->Commit();
  irrVar->ByName("rgb",irr);


  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colourIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);
  
  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);
  
 // Make default camera response function flat...
  crf.SetMult();
  expNorm = -1.0;
  
 // Make default region a suitable size...
  pMin[0] = 80;
  pMin[1] = 60;
  pMax[0] = 240;
  pMax[1] = 180;

 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Camera Response Function Estimator");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   info = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   result = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   info->Set("Samples = 0");
   result->Set("Not yet run");
   vert1->AttachBottom(info,false);
   vert1->AttachBottom(result,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Add");
   but3->SetChild(lab3); lab3->Set("Run");
   but4->SetChild(lab4); lab4->Set("Save Camera Response...");

   
   
   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Image");
   viewSelect->Append("Graph");
   viewSelect->Set(0);

   
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   exposure = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   exposureInv = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   degrees = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab5->Set("  Exposure (seconds)");
   lab6->Set(" Exposure (1/seconds)");
   lab7->Set(" Max Exponent");
   
   exposure->Set("1.0");
   exposureInv->Set("1.0");
   degrees->Set("5");

   exposure->SetSize(96,24);
   exposureInv->SetSize(96,24);
   degrees->SetSize(48,24);

   
   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(viewSelect,false);
   horiz1->AttachRight(lab7,false);
   horiz1->AttachRight(degrees,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);   

   horiz2->AttachRight(lab5,false);
   horiz2->AttachRight(exposure,false);
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(exposureInv,false);
   horiz2->AttachRight(but2,false);   




   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&CamResponse::Quit));
  canvas->OnResize(MakeCB(this,&CamResponse::Resize));
  canvas->OnClick(MakeCB(this,&CamResponse::Click));
  canvas->OnMove(MakeCB(this,&CamResponse::Move));
    
  but1->OnClick(MakeCB(this,&CamResponse::LoadIrr));
  but2->OnClick(MakeCB(this,&CamResponse::Add));
  but3->OnClick(MakeCB(this,&CamResponse::Run));
  but4->OnClick(MakeCB(this,&CamResponse::SaveCRF));
  viewSelect->OnChange(MakeCB(this,&CamResponse::ChangeView));
  
  exposure->OnChange(MakeCB(this,&CamResponse::ExpChange));
  exposureInv->OnChange(MakeCB(this,&CamResponse::InvExpChange));
}

CamResponse::~CamResponse()
{
 delete win;
 delete irrVar;
 delete imageVar;
 delete imgVar;
}

void CamResponse::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void CamResponse::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(img.Size(0),img.Size(1))),bs::Pos(sx,sy),img);
  
 // If needed render the selected box...
  if (viewSelect->Get()==0)
  {
   math::Vect<2,real32> tr;
    tr[0] = pMax[0];
    tr[1] = pMin[1];
   math::Vect<2,real32> bl;
    bl[0] = pMin[0];
    bl[1] = pMax[1];

   RenderLine(pMin,tr,bs::ColourRGB(1.0,0.0,1.0));
   RenderLine(pMin,bl,bs::ColourRGB(1.0,0.0,1.0));
   RenderLine(tr,pMax,bs::ColourRGB(1.0,0.0,1.0));
   RenderLine(bl,pMax,bs::ColourRGB(1.0,0.0,1.0));
  }

 // Update...
  canvas->Update();
}

void CamResponse::Click(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;
  
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2; 

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;
  
  if (mbe->button==gui::MouseButtonEvent::LMB)
  {
   // Move the nearest corner point, including the 2 virtual corners...
    if (mp[0]<(0.5*(pMin[0]+pMax[0]))) pMin[0] = mp[0];
                                  else pMax[0] = mp[0];

    if (mp[1]<(0.5*(pMin[1]+pMax[1]))) pMin[1] = mp[1];
                                  else pMax[1] = mp[1];  
  }
  else
  {
   pMin[0] = mp[0];
   pMax[0] = mp[0]+1;
   pMin[1] = mp[1];
   pMax[1] = mp[1]+1;
  }


 // Redraw the display... 
  canvas->Redraw();
}

void CamResponse::Move(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;
  
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2; 
  
  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;
  
 // Move the nearest end point...
  if (mp[0]<(0.5*(pMin[0]+pMax[0]))) pMin[0] = mp[0];
                                else pMax[0] = mp[0];

  if (mp[1]<(0.5*(pMin[1]+pMax[1]))) pMin[1] = mp[1];
                                else pMax[1] = mp[1];
 
 // Redraw the display... 
  canvas->Redraw();
}

void CamResponse::LoadIrr(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.JPG,*.png,*.tif",fn))
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
   
  // Try and get the exposure time...
   file::Exif exif;
   if (exif.Load(fn))
   {
    if (exif.HasExposureTime())
    {
     str::String s;
     s << exif.GetExposureTime();
     exposure->Set(s);
    }
   }

  // Stick in right variables...
   delete irrVar;
   irrVar = tempVar;
   irrVar->ByName("rgb",irr);

  // Update the display...
   Update();
 }
}

void CamResponse::Add(gui::Base * obj,gui::Event * event)
{
 // Find the average colour in the selected region...
  nat32 minX = nat32(math::Clamp<int32>(int32(math::Round(pMin[0])),0,irr.Size(0)-1));
  nat32 maxX = nat32(math::Clamp<int32>(int32(math::Round(pMax[0])),0,irr.Size(0)-1));
  nat32 minY = nat32(math::Clamp<int32>(int32(math::Round(pMin[1])),0,irr.Size(1)-1));
  nat32 maxY = nat32(math::Clamp<int32>(int32(math::Round(pMax[1])),0,irr.Size(1)-1));
  
  bs::ColourRGB avg(0.0,0.0,0.0);
  nat32 div = 0;
  for (nat32 y=minY;y<=maxY;y++)
  {
   for (nat32 x=minX;x<=maxX;x++)
   {
    div += 1;
    avg.r += (irr.Get(x,y).r-avg.r)/real32(div);
    avg.g += (irr.Get(x,y).g-avg.g)/real32(div);
    avg.b += (irr.Get(x,y).b-avg.b)/real32(div);
   }
  }

 // Add the sample...
  nat32 ind = samples.Size();
  samples.Size(ind+1);
  
  samples[ind].colour = avg;
  samples[ind].expTime = exposure->GetReal(1.0);


 // Update the view...
  if (viewSelect->Get()!=0) Update();


 // Also need to update the number of samples avaliable...
  str::String s;
  s << "Samples = " << samples.Size();
  info->Set(s);
}

void CamResponse::Run(gui::Base * obj,gui::Event * event)
{
 // Count the usable samples...
  nat32 samps = 0;
  for (nat32 i=0;i<samples.Size();i++)
  {
   real32 val = (samples[i].colour.r+samples[i].colour.g+samples[i].colour.b)/3.0;
   if ((val>0.05)&&(val<0.95)) samps += 1;
  }
  if (samps==0) {result->Set("Needs data"); return;}
  
 
 // Work out how many degrees we will be using...
  nat32 maxExp = degrees->GetInt(5);
  maxExp = math::Min(maxExp,samps-1);
  if (maxExp<2) {result->Set("Not enough data"); return;}


 // Construct the matrices...
  math::Matrix<real32> a(samps,maxExp);
  math::Vector<real32> b(samps);
  nat32 s = 0;
  for (nat32 i=0;i<samples.Size();i++)
  {
   real32 val = (samples[i].colour.r+samples[i].colour.g+samples[i].colour.b)/3.0;
   if ((val>0.05)&&(val<0.95))
   {
    real32 x = 1.0;
    for (nat32 j=0;j<maxExp;j++)
    {
     x *= val;
     a[s][j] = x;
    }
    
    b[s] = samples[i].expTime;
   
    s += 1;
   }
  }


 // Solve...
  math::Vector<real32> ans(maxExp);
  math::Matrix<real32> temp(maxExp,maxExp);
  if (math::SolveLeastSquares(a,b,ans,temp)==false) {result->Set("Failed to solve equation - try less degrees"); return;}


 // Scale such that it goes through the point (1,1)...
  real32 at1 = 0.0;
  for (nat32 i=0;i<ans.Size();i++) at1 += ans[i];
  ans /= at1;
  expNorm = 1.0/at1;


 // Store in crf variable...
  crf.SetPolyNC(ans);

 // Update the view...
  if (viewSelect->Get()!=0) Update();
  
 // Update the result with the polynomial...
 {
  str::String s;
  s << "y =";
  for (nat32 i=0;i<ans.Size();i++)
  {
   if ((i==0)||(ans[i]<0.0)) s << " ";
                        else s << " +";
   s << ans[i] << "*x^" << (i+1);
  }
  result->Set(s);
 }
}

void CamResponse::ChangeView(gui::Base * obj,gui::Event * event)
{
 Update();
}

void CamResponse::SaveCRF(gui::Base * obj,gui::Event * event)
{
 str::String fn("");
 if (cyclops.App().SaveFileDialog("Save Camera Response...",fn))
 {
  if (!fn.EndsWith(".crf")) fn += ".crf";
  if (!crf.Save(fn,true))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving back the file.");
   return;	  
  }
 }
}

void CamResponse::ExpChange(gui::Base * obj,gui::Event * event)
{
 if (expRec==true)
 {
  expRec = false;
  return;
 }
 expRec = true;
 
 real32 inv = 1.0/exposure->GetReal(1.0);
 if (math::IsFinite(inv))
 {
  str::String s;
  s << inv;
  exposureInv->Set(s);
 }
}

void CamResponse::InvExpChange(gui::Base * obj,gui::Event * event)
{
 if (expRec==true)
 {
  expRec = false;
  return;
 }
 expRec = true;

 real32 inv = 1.0/exposureInv->GetReal(1.0);
 if (math::IsFinite(inv))
 {
  str::String s;
  s << inv;
  exposure->Set(s);
 }
}

void CamResponse::Update()
{
 if (viewSelect->Get()==0)
 {
  // Image...
  
  // Get the size right...
   nat32 width = irr.Size(0);
   nat32 height = irr.Size(1);
   if ((image.Size(0)!=width)||(image.Size(1)!=height))
   {
    imageVar->Setup2D(width,height);
    imageVar->Commit();
    imageVar->ByName("rgb",image);
   }
   if ((img.Size(0)!=width)||(img.Size(1)!=height))
   {
    imgVar->Setup2D(width,height);
    imgVar->Commit();
    imgVar->ByName("rgb",img);
   }
    
  // Copy in the image...
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     real32 l = (irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0;
     image.Get(x,y).r = l;
     image.Get(x,y).g = l;
     image.Get(x,y).b = l;
    }
   }    
 }
 else
 {
  // Graph...
  
  // Get the size right...
   nat32 width = 400;
   nat32 height = 400;
   if ((image.Size(0)!=width)||(image.Size(1)!=height))
   {
    imageVar->Setup2D(width,height);
    imageVar->Commit();
    imageVar->ByName("rgb",image);
   }
   if ((img.Size(0)!=width)||(img.Size(1)!=height))
   {
    imgVar->Setup2D(width,height);
    imgVar->Commit();
    imgVar->ByName("rgb",img);
   }
   
  // Make everything white...
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = bs::ColourRGB(1.0,1.0,1.0);
   }


  // Get a normalising multiplier for exposure...
   real32 expMult = expNorm;
   if (expMult<0.0)
   {
    expMult = math::Infinity<real32>();
    for (nat32 i=0;i<samples.Size();i++)
    {
     real32 val = 1.0/samples[i].expTime;
     expMult = math::Min(expMult,val);
    }
   }
  
  // Render the samples...
   for (nat32 i=0;i<samples.Size();i++)
   {
    bs::Pnt loc;
    loc[0] = (samples[i].colour.r+samples[i].colour.g+samples[i].colour.b)/3.0;
    loc[1] = samples[i].expTime * expMult;
    
    bs::Pnt locM = loc;
    locM[0] *= real32(width);
    locM[1] *= real32(height);

    bs::Pnt left = locM; left[0] -= 3.0;
    bs::Pnt right = locM; right[0] += 3.0;    
    bs::Pnt up = locM; up[1] += 3.0;
    bs::Pnt down = locM; down[1] -= 3.0;
    
    if ((loc[0]>0.05)&&(loc[0]<0.95))
    {
     rend::Line(image,up,down,bs::ColourRGB(0.0,0.0,1.0));
     rend::Line(image,left,right,bs::ColourRGB(0.0,0.0,1.0));
    }
    else
    {
     rend::Line(image,up,down,bs::ColourRGB(1.0,0.0,0.0));
     rend::Line(image,left,right,bs::ColourRGB(1.0,0.0,0.0));
    }    
   }


  // Render the crf...
   int32 lastHeight = 0;
   for (nat32 x=0;x<width;x++)
   {
    int32 yt = int32(math::Round(real32(height)*crf(x/real32(width))));
    if (lastHeight>yt) lastHeight = yt;
    for (int32 y=lastHeight;y<=yt;y++)
    {
     if ((y>=0)&&(y<int32(height))) image.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
    }
    lastHeight = yt+1;
   }
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

void CamResponse::RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - img.Size(0))/2;
 nat32 sy = (canvas->P().Height() - img.Size(1))/2;
 
 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);
 
 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);
 
 canvas->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col); 
}

//------------------------------------------------------------------------------
