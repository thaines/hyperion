//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/sphere_fitter.h"

//------------------------------------------------------------------------------
SphereFitter::SphereFitter(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),valid(false),var(null<svt::Var*>())
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
  win->SetTitle("Sphere Fitter");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   sphereLab = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   pointLab = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   pointLab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   vert1->AttachBottom(horiz1,false);
   vert1->AttachBottom(sphereLab,false);
   vert1->AttachBottom(pointLab,false);
   vert1->AttachBottom(pointLab2,false);
   
   sphereLab->Set("Not yet fitted");
   pointLab->Set("");
   pointLab2->Set("");
   

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   fitRad = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));


   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Load Calibration...");
   but3->SetChild(lab3); lab3->Set("Add");
   but4->SetChild(lab4); lab4->Set("Delete");
   but5->SetChild(lab5); lab5->Set("Fit");
   
   lab6->Set(" Radius:");
   fitRad->SetSize(64,24);
   fitRad->Set("1.0");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(lab6,false);
   horiz1->AttachRight(fitRad,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   horiz1->AttachRight(but5,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&SphereFitter::Quit));
  canvas->OnResize(MakeCB(this,&SphereFitter::Resize));
  canvas->OnClick(MakeCB(this,&SphereFitter::Click));
  canvas->OnMove(MakeCB(this,&SphereFitter::Move));
  but1->OnClick(MakeCB(this,&SphereFitter::LoadImage));
  but2->OnClick(MakeCB(this,&SphereFitter::LoadCam));
  but3->OnClick(MakeCB(this,&SphereFitter::Add));
  but4->OnClick(MakeCB(this,&SphereFitter::Del));
  but5->OnClick(MakeCB(this,&SphereFitter::Fit));
}

SphereFitter::~SphereFitter()
{
 delete win;
 delete var;
}

void SphereFitter::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void SphereFitter::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // Render the sphere using the camera calibration...
  // First find a suitable coordinate system to render the boundary...
   math::Vect<3> toCam = camCentre;
   toCam -= centre;
   toCam.Normalise();
   
   math::Vect<3> perpA;
   math::Vect<3> perpB;
   math::FinishCordSys(toCam,perpA,perpB);
   perpA.Normalise();
   perpB.Normalise();


  // Now render the boundary and cross slices...
   if (valid)
   {
    static const nat32 edgeSize = 32;
    bs::Pnt edge[edgeSize];
    
    // Boundary...
     for (nat32 i=0;i<edgeSize;i++)
     {
      real32 ang = real32(i)*math::pi*2.0/real32(edgeSize);
      real32 ca = radius*math::Cos(ang);
      real32 sa = radius*math::Sin(ang);
   
      math::Vect<4,real64> pos;
      for (nat32 j=0;j<3;j++)
      {
       pos[j] = centre[j] + ca*perpA[j] + sa*perpB[j];
      }
      pos[3] = 1.0;
    
      math::Vect<3,real64> proj;
      math::MultVect(camera.camera,pos,proj);
      camera.radial.Dis(proj,proj);
   
      edge[i][0] = proj[0];
      edge[i][1] = image.Size(1)-1.0 - proj[1];
     }
   
     for (nat32 i=0;i<edgeSize;i++) RenderLine(edge[i],edge[(i+1)%edgeSize],bs::ColourRGB(0.0,1.0,0.0));


    // 45 degree contour...
     for (nat32 i=0;i<edgeSize;i++)
     {
      real32 ang = real32(i)*math::pi*2.0/real32(edgeSize);
      real32 ca = radius*math::Cos(ang)*math::Cos(math::pi*0.25);
      real32 sa = radius*math::Sin(ang)*math::Cos(math::pi*0.25);
      real32 bw = radius*math::Sin(math::pi*0.25);
   
      math::Vect<4,real64> pos;
      for (nat32 j=0;j<3;j++)
      {
       pos[j] = centre[j] + ca*perpA[j] + sa*perpB[j] + bw*toCam[j];
      }
      pos[3] = 1.0;
    
      math::Vect<3,real64> proj;
      math::MultVect(camera.camera,pos,proj);
      camera.radial.Dis(proj,proj);
   
      edge[i][0] = proj[0];
      edge[i][1] = image.Size(1)-1.0 - proj[1];
     }
   
     for (nat32 i=0;i<edgeSize;i++) RenderLine(edge[i],edge[(i+1)%edgeSize],bs::ColourRGB(0.0,1.0,0.0));


    // Cross A...
     for (nat32 i=0;i<(edgeSize/2)+1;i++)
     {
      real32 ang = real32(i)*math::pi/real32(edgeSize/2);
      real32 ca = radius*math::Cos(ang);
      real32 sa = radius*math::Sin(ang);
   
      math::Vect<4,real64> pos;
      for (nat32 j=0;j<3;j++)
      {
       pos[j] = centre[j] + ca*perpA[j] + sa*toCam[j];
      }
      pos[3] = 1.0;
    
      math::Vect<3,real64> proj;
      math::MultVect(camera.camera,pos,proj);
      camera.radial.Dis(proj,proj);
   
      edge[i][0] = proj[0];
      edge[i][1] = image.Size(1)-1.0 - proj[1];
     }
   
     for (nat32 i=0;i<edgeSize/2;i++) RenderLine(edge[i],edge[(i+1)%edgeSize],bs::ColourRGB(0.0,1.0,0.0));
    
    
    // Cross B...
     for (nat32 i=0;i<(edgeSize/2)+1;i++)
     {
      real32 ang = real32(i)*math::pi/real32(edgeSize/2);
      real32 ca = radius*math::Cos(ang);
      real32 sa = radius*math::Sin(ang);
   
      math::Vect<4,real64> pos;
      for (nat32 j=0;j<3;j++)
      {
       pos[j] = centre[j] + ca*perpB[j] + sa*toCam[j];
      }
      pos[3] = 1.0;
    
      math::Vect<3,real64> proj;
      math::MultVect(camera.camera,pos,proj);
      camera.radial.Dis(proj,proj);
   
      edge[i][0] = proj[0];
      edge[i][1] = image.Size(1)-1.0 - proj[1];
     }
   
     for (nat32 i=0;i<edgeSize/2;i++) RenderLine(edge[i],edge[(i+1)%edgeSize],bs::ColourRGB(0.0,1.0,0.0));
   }

 // Render the stored positions...
  ds::List<bs::Pnt>::Cursor targ = boundary.FrontPtr();
  while (!targ.Bad())
  {
   RenderPoint(*targ,bs::ColourRGB(1.0,0.0,0.0));
   ++targ;
  }

 // Render the user selected position...
  RenderPoint(pos,bs::ColourRGB(1.0,0.0,1.0));

 // Update...
  canvas->Update();
}

void SphereFitter::Click(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);

  real32 sx = (canvas->P().Width() - image.Size(0))/2;
  real32 sy = (canvas->P().Height() - image.Size(1))/2;

 // Snap the point to the new position...
  pos[0] = mbe->x - sx;
  pos[1] = mbe->y - sy;

 // Redraw the display...
  Update();
  canvas->Redraw();
}

void SphereFitter::Move(gui::Base * obj,gui::Event * event)
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

void SphereFitter::LoadImage(gui::Base * obj,gui::Event * event)
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

void SphereFitter::LoadCam(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera/Intrinsic","*.cam,*.icd",fn)==false) return;
 
 if (fn.EndsWith(".icd"))
 {
  // Its an intrinsic matrix - presume default position...
   cam::CameraCalibration intrinsic;
   if (intrinsic.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load intrinsic calibration file");
   }
   else
   {
    camera.FromCC(intrinsic);
   }
 }
 else
 {
  // Its a full camera - use direct...
   if (camera.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load camera calibration file");
   }
 }
 
 math::Vect<4,real64> cc;
 camera.camera.Centre(cc);
 cc /= cc[3];
 for (nat32 i=0;i<3;i++) camCentre[i] = cc[i];
 
 Update();
 canvas->Redraw();
}

void SphereFitter::Add(gui::Base * obj,gui::Event * event)
{
 boundary.AddBack(pos);
}

void SphereFitter::Del(gui::Base * obj,gui::Event * event)
{
 if (boundary.Size()!=0) boundary.RemBack();
 canvas->Redraw();
}

void SphereFitter::Fit(gui::Base * obj,gui::Event * event)
{
 // Check solving is sane...
  if (boundary.Size()<3)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"At least 3 boundary coordinates required");
   return;
  }
   

 // Prep the solver...
  fit::ImageOfSphere ios;
  ios.SetCamera(camera);
  radius = fitRad->GetReal(1.0);
  ios.SetRadius(radius);
  
  ds::List<bs::Pnt>::Cursor targ = boundary.FrontPtr();
  while (!targ.Bad())
  {
   bs::Pnt p = *targ;
   p[1] = image.Size(1)-1.0 - p[1];
   
   ios.AddPixel(p[0],p[1]);
   
   ++targ;
  }
  
 
 // Solve...
  valid = ios.Run(cyclops.BeginProg());
  cyclops.EndProg();
  
 
 // Extract results...
  if (valid)
  {
   centre = ios.Centre();

   // Update label with details of sphere...
    str::String s;
    s << "Position = " << centre << "; radius = " << radius;
    sphereLab->Set(s);
  }
 
 
 // Update view to reflect changes...
  Update();
  canvas->Redraw();
}

void SphereFitter::Update()
{
 // Calculate the ray cast by the current position...
  bs::Ray ray;
  camera.GetRay(pos[0],image.Size(1)-1.0 - pos[1],ray);


 // Find the point of interception, plus distance...
  bs::Sphere s;
  s.c = centre;
  s.r = radius;
  
  real32 dist;
  bs::Vert where;
  if (s.Intercept(ray,dist,where))
  {
   bs::Normal norm = where; norm -= centre; norm.Normalise();
   bs::Normal in = where; in -= camCentre; in.Normalise();
   bs::Normal out = in; out.Reflect(norm); out.Normalise();
  
   str::String s;
   s << "Intercept at " << where << ", " << dist << " from camera.";
   s << " normal = " << norm; 
   pointLab->Set(s);
   
   str::String s2;
   s2 << "reflection: in = " << in << ", out = " << out;
   pointLab2->Set(s2);
  }
  else
  {
   pointLab->Set("No intercept");
   pointLab2->Set("");
  }
}

void SphereFitter::RenderPoint(const bs::Pnt & p,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - image.Size(0))/2;
 nat32 sy = (canvas->P().Height() - image.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 canvas->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 canvas->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);
}

void SphereFitter::RenderLine(const bs::Pnt & a,const bs::Pnt & b,const bs::ColourRGB & col)
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
