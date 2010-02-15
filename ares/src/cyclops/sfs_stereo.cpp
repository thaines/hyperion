//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

#include "cyclops/sfs_stereo.h"

//------------------------------------------------------------------------------
StereoSfS::StereoSfS(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
dispVar(null<svt::Var*>()),needleVar(null<svt::Var*>()),
imageVar(null<svt::Var*>()),imgVar(null<svt::Var*>())
{
 // Create default data maps...
  real32 dispIni = 0.0;
  math::Vect<3,real32> zDir(0.0); zDir[2] = 1.0;
  math::Fisher fishIni(zDir);
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  bs::ColRGB colIni(0,0,0);

  dispVar = new svt::Var(cyclops.Core());
  dispVar->Setup2D(320,240);
  dispVar->Add("disp",dispIni);
  dispVar->Add("sd",dispIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);
  dispVar->ByName("sd",dispSd);

  needleVar = new svt::Var(cyclops.Core());
  needleVar->Setup2D(320,240);
  needleVar->Add("fish",fishIni);
  needleVar->Commit();
  needleVar->ByName("fish",needle);
  noNeedleFish = true;

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


 // Make default camera geometry boring...
  pair.SetDefault(320,240);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Stereo & SfS Combiner");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   but1->SetChild(lab1); lab1->Set("Load Disparity...");
   but2->SetChild(lab2); lab2->Set("Load Needle...");
   but3->SetChild(lab3); lab3->Set("Load Pair...");
   but4->SetChild(lab4); lab4->Set("Run");
   but5->SetChild(lab5); lab5->Set("Save View...");
   but6->SetChild(lab6); lab6->Set("Save Disparity...");
   
   needleWeight = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   iterations = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   deltaCap = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   needleWeight->Set("1.0");
   iterations->Set("1120");
   deltaCap->Set("4.0");
   
   needleWeight->SetSize(48,24);
   iterations->SetSize(48,24);
   deltaCap->SetSize(32,24);
   
   lab7->Set(" Needle Sd Mult:");
   lab8->Set(" Iterations:");
   lab9->Set(" Delta Half Life:");


   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Input Needle Map");
   viewSelect->Append("Input Disparity");
   viewSelect->Append("Input Disparity Dirs");
   viewSelect->Append("Output Disparity");
   viewSelect->Append("Output Disparity Dirs");
   viewSelect->Set(1);


   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(viewSelect,false);
   horiz1->AttachRight(but4,false);
   
   horiz2->AttachRight(lab7,false);
   horiz2->AttachRight(needleWeight,false);
   horiz2->AttachRight(lab8,false);
   horiz2->AttachRight(iterations,false);
   horiz2->AttachRight(lab9,false);
   horiz2->AttachRight(deltaCap,false);
   horiz2->AttachRight(but5,false);
   horiz2->AttachRight(but6,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&StereoSfS::Quit));
  canvas->OnResize(MakeCB(this,&StereoSfS::Resize));
  but1->OnClick(MakeCB(this,&StereoSfS::LoadDisp));
  but2->OnClick(MakeCB(this,&StereoSfS::LoadNeedle));
  but3->OnClick(MakeCB(this,&StereoSfS::LoadCalib));
  but4->OnClick(MakeCB(this,&StereoSfS::Run));
  but5->OnClick(MakeCB(this,&StereoSfS::SaveView));
  but6->OnClick(MakeCB(this,&StereoSfS::SaveDisp));
  viewSelect->OnChange(MakeCB(this,&StereoSfS::ChangeView));
}

StereoSfS::~StereoSfS()
{
 delete win;
 delete dispVar;
 delete needleVar;
 delete imageVar;
 delete imgVar;
}

void StereoSfS::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void StereoSfS::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(img.Size(0),img.Size(1))),bs::Pos(sx,sy),img);

 // Update...
  canvas->Update();
}

void StereoSfS::LoadDisp(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity File","*.dis",fn))
 {
  // Load svt, verify it is a disparity map...
   cstr filename = fn.ToStr();
   svt::Node * floatNode = svt::Load(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatNode==null<svt::Node*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load file.");
    delete floatNode;
    return;
   }

   if (str::Compare(typestring(*floatNode),"eos::svt::Var")!=0)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a disparity map");
    delete floatNode;
    return;
   }
   svt::Var * floatVar = static_cast<svt::Var*>(floatNode);

   nat32 dispInd;
   if ((floatVar->Dims()!=2)||
       (floatVar->GetIndex(cyclops.TT()("disp"),dispInd)==false)||
       (floatVar->FieldType(dispInd)!=cyclops.TT()("eos::real32")))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a disparity map");
    delete floatVar;
    return;
   }

   nat32 sdInd;
   if ((floatVar->GetIndex(cyclops.TT()("sd"),sdInd)==false)||
       (floatVar->FieldType(sdInd)!=cyclops.TT()("eos::real32")))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is not augmented with Gaussain distribution");
    delete floatVar;
    return;
   }


  // Move the floatVar into the asReal array, deleting previous...
   delete dispVar;
   dispVar = floatVar;
   dispVar->ByName("disp",disp);
   dispVar->ByName("mask",mask);
   dispVar->ByName("sd",dispSd);

  
  // If the needle map doesn't match size create a new one with all the normals 
  // pointing at the viewer - this allows this tool to double as a smoothing tool...
   math::Vect<3,real32> zDir(0.0); zDir[2] = 1.0;
   math::Fisher fishIni(zDir);

   needleVar = new svt::Var(disp);
   needleVar->Add("fish",fishIni);
   needleVar->Commit();
   needleVar->ByName("fish",needle);
   noNeedleFish = true;

   
  // Update iteration count...
   nat32 iters = 2 * (disp.Size(0) + disp.Size(1));
   str::String s;
   s << iters;
   iterations->Set(s);
   

  // Refresh the display...
   Update();
 }
}

void StereoSfS::LoadNeedle(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Needle Map Image","*.bmp,*.jpg,*.png,*.tif",fn))
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

  // Convert into internal representation...
   delete needleVar;
   math::Fisher fishIni;
   
   needleVar = new svt::Var(cyclops.Core());
   needleVar->Setup2D(tempVar->Size(0),tempVar->Size(1));
   needleVar->Add("fish",fishIni);
   needleVar->Commit();
   needleVar->ByName("fish",needle);
   noNeedleFish = true;
   
   svt::Field<bs::ColourRGB> rgb(tempVar,"rgb");
   for (nat32 y=0;y<needle.Size(1);y++)
   {
    for (nat32 x=0;x<needle.Size(0);x++)
    {
     math::Fisher & f = needle.Get(x,y);
     bs::ColourRGB & c = rgb.Get(x,y);
     
     f[0] = 2.0*(c.r-0.5);
     f[1] = 2.0*(c.g-0.5);
     f[2] = c.b;
     if (!math::IsZero(f.Length())) f.Normalise();
    }
   }
   
   delete tempVar;

  // Update the display...
   Update();
 }
}

void StereoSfS::LoadCalib(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
 }
 pair.LeftToDefault();
}

void StereoSfS::Run(gui::Base * obj,gui::Event * event)
{
 // Get the parameters...
  real32 nWeight = needleWeight->GetReal(1.0);
  nat32 iters = iterations->GetInt(100);
  real32 dCap = deltaCap->GetReal(1.0);
 
 
 // Create the solver...
  inf::IntegrateBP ibp(disp.Size(0),disp.Size(1));
  ibp.SetIters(iters);

 
 // The below code needs the disparity positions as locations in 3D space...
  ds::Array2D< math::Vect<3,real32> > loc(disp.Size(0),disp.Size(1));
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    pair.Triangulate(real32(x),real32(y),disp.Get(x,y),loc.Get(x,y));
   }
  }


 // Fill it in...
  math::Vect<3,real32> origin(0.0);
  math::Mat<3,3,real64> rectLeft = pair.unRectLeft;
  math::Mat<3,3,real64> rectRight = pair.unRectRight;
  {
   math::Mat<3,3,real64> temp;
   math::Inverse(rectLeft,temp);
   math::Inverse(rectRight,temp);
  }

  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    // Stereo information - the disparity value and its confidence...
     if ((!mask.Valid())||(mask.Get(x,y)))
     {
      ibp.SetVal(x,y,disp.Get(x,y),1.0/dispSd.Get(x,y));
     }
    
    // SfS information - the expected differences between disparity values.
    // Have to make extensive use of the camera geometry for this...
     if (!math::IsZero(needle.Get(x,y).Length()))
     {
      if (noNeedleFish)
      {
       // Calculate the plane implied by the surface normal, going through the vertex...
        bs::Plane p;
        for (nat32 i=0;i<3;i++) p.n[i] = needle.Get(x,y)[i];
        p.d = - (p.n * loc.Get(x,y));
        
       // Intercept with the rays of the pixels and project back to get expected disparity values...
        real32 exp[4]; // Expected disparity for each direction.
        
        if (x+1<disp.Size(0))
        {
         math::Vect<4,real32> ip;
         p.LineIntercept(origin,loc.Get(x+1,y),ip);
         real32 outX,outY;
         pair.Project(ip,outX,outY,exp[0],&rectLeft,&rectRight);
        }
        else exp[0] = disp.Get(x,y);
        
        if (y+1<disp.Size(1))
        {
         math::Vect<4,real32> ip;
         p.LineIntercept(origin,loc.Get(x,y+1),ip);
         real32 outX,outY;
         pair.Project(ip,outX,outY,exp[1],&rectLeft,&rectRight);
        }
        else exp[1] = disp.Get(x,y);
        
        if (x>0)
        {
         math::Vect<4,real32> ip;
         p.LineIntercept(origin,loc.Get(x-1,y),ip);
         real32 outX,outY;
         pair.Project(ip,outX,outY,exp[2],&rectLeft,&rectRight);
        }
        else exp[2] = disp.Get(x,y);

        if (y>0)
        {
         math::Vect<4,real32> ip;
         p.LineIntercept(origin,loc.Get(x,y-1),ip);
         real32 outX,outY;
         pair.Project(ip,outX,outY,exp[3],&rectLeft,&rectRight);
        }
        else exp[3] = disp.Get(x,y);


       // Create relations between this pixel and all neighbours...
        for (nat32 i=0;i<4;i++)
        {
         real32 d = exp[i]-disp.Get(x,y);
         real32 w = (1.0/nWeight) / math::Pow(real32(2.0),math::Abs(d)/dCap);
         ibp.SetRel(x,y,i,1.0,d,w);
        }
      }
      else
      {
       // Code me ***************************
      }
     }
   }
  }


 // Run the algorithm...
  ibp.Run(cyclops.BeginProg());
  cyclops.EndProg();


 // Extract the answer...
  real32 dispIni = 0.0;
  dispVar->Add("disp_new",dispIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);
  dispVar->ByName("disp_new",dispNew);
  dispVar->ByName("mask",mask);
  dispVar->ByName("sd",dispSd);
  
  for (nat32 y=0;y<dispNew.Size(1);y++)
  {
   for (nat32 x=0;x<dispNew.Size(0);x++)
   {
    if (ibp.Defined(x,y))
    {
     dispNew.Get(x,y) = ibp.Expectation(x,y);
    }
    else
    {
     dispNew.Get(x,y) = disp.Get(x,y);
    }
   }
  }


 // Update the view...
  Update();
}

void StereoSfS::ChangeView(gui::Base * obj,gui::Event * event)
{
 Update();
}

void StereoSfS::SaveView(gui::Base * obj,gui::Event * event)
{
 str::String fn("");
 if (cyclops.App().SaveFileDialog("Save Image",fn))
 {
  if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
  cstr ts = fn.ToStr();
  if (!filter::SaveImage(image,ts,true))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
  }
  mem::Free(ts);
 }
}

void StereoSfS::SaveDisp(gui::Base * obj,gui::Event * event)
{
 if (dispNew.Valid())
 {
  // Construct the disparity map object to save...
   real32 dispIni = 0.0;
   bit maskIni = true;

   svt::Var output(dispNew);
   output.Add("disp",dispIni);
   if (mask.Valid()) output.Add("mask",maskIni);
   output.Commit();
   
   svt::Field<real32> outDisp(&output,"disp");
   svt::Field<bit> outMask(&output,"mask");
   
   for (nat32 y=0;y<outDisp.Size(1);y++)
   {
    for (nat32 x=0;x<outDisp.Size(0);x++)
    {
     outDisp.Get(x,y) = dispNew.Get(x,y);
     if (mask.Valid()) outMask.Get(x,y) = mask.Get(x,y);
    }
   }


  // Get the filename...
   str::String fn("");
   if (cyclops.App().SaveFileDialog("Save Disparity Map",fn))
   {
    if (!fn.EndsWith(".dis")) fn << ".dis";
    cstr ts = fn.ToStr();
    if (!svt::Save(ts,&output,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving .dis file.");
    }
    mem::Free(ts);
   }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to run the algorithm first!");
 }
}

void StereoSfS::Update()
{
 // Check sizes match, adjust if need be...
  nat32 width = disp.Size(0);
  nat32 height = disp.Size(1);
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


 // Switch on viewing mode and render correct image...
  switch(viewSelect->Get())
  {
   case 0: // Input needle map
   {
    for (nat32 y=0;y<math::Min(height,needle.Size(1));y++)
    {
     for (nat32 x=0;x<math::Min(width,needle.Size(0));x++)
     {
      image.Get(x,y).r = math::Clamp<real32>((needle.Get(x,y)[0]+1.0)/2.0,0.0,1.0);
      image.Get(x,y).g = math::Clamp<real32>((needle.Get(x,y)[1]+1.0)/2.0,0.0,1.0);
      image.Get(x,y).b = math::Clamp<real32>(needle.Get(x,y)[2],0.0,1.0);
     }
    }
   }
   break;
   case 1: // Input disparity
   {
    // Find range...
     real32 minDisp =  math::Infinity<real32>();
     real32 maxDisp = -math::Infinity<real32>();
     for (nat32 y=0;y<disp.Size(1);y++)
     {
      for (nat32 x=0;x<disp.Size(0);x++)
      {
       if ((!mask.Valid())||(mask.Get(x,y)))
       {
        minDisp = math::Min(minDisp,disp.Get(x,y));
        maxDisp = math::Max(maxDisp,disp.Get(x,y));
       }
      }
     }
    
    // Render...
     for (nat32 y=0;y<disp.Size(1);y++)
     {
      for (nat32 x=0;x<disp.Size(0);x++)
      {
       if ((!mask.Valid())||(mask.Get(x,y)))
       {
        real32 v = (disp.Get(x,y)-minDisp)/(maxDisp-minDisp);
        image.Get(x,y) = bs::ColourRGB(v,v,v);
       }
       else
       {
        image.Get(x,y) = bs::ColourRGB(0.0,0.0,1.0);
       }
      }
     }
   }
   break;
   case 2: // Input disparity dirs
   {
    // First triangulate every point in the disparity map...
     ds::Array2D< math::Vect<3,real32> > loc(disp.Size(0),disp.Size(1));
     for (nat32 y=0;y<disp.Size(1);y++)
     {
      for (nat32 x=0;x<disp.Size(0);x++)
      {
       pair.Triangulate(real32(x),real32(y),disp.Get(x,y),loc.Get(x,y));
      }
     }
    
    
    // Now calculate forward difference surface normals and render them...
     for (nat32 y=0;y<disp.Size(1);y++)
     {
      for (nat32 x=0;x<disp.Size(0);x++)
      {
       math::Vect<3,real32> dirX;
       if (x+1<disp.Size(0))
       {
        dirX = loc.Get(x+1,y);
        dirX -= loc.Get(x,y);
       }
       else
       {
        dirX = loc.Get(x,y);
        dirX -= loc.Get(x-1,y);
       }
       
       math::Vect<3,real32> dirY;
       if (y+1<disp.Size(1))
       {
        dirY = loc.Get(x,y+1);
        dirY -= loc.Get(x,y);
       }
       else
       {
        dirY = loc.Get(x,y);
        dirY -= loc.Get(x,y-1);
       }
       
       math::Vect<3,real32> norm;
       math::CrossProduct(dirX,dirY,norm);
       norm.Normalise();
       
       image.Get(x,y).r = math::Clamp<real32>((norm[0]+1.0)/2.0,0.0,1.0);
       image.Get(x,y).g = math::Clamp<real32>((norm[1]+1.0)/2.0,0.0,1.0);
       image.Get(x,y).b = math::Clamp<real32>(norm[2],0.0,1.0);
      }
     }
   }
   break;
   case 3: // Output disparity
   {
    if (dispNew.Valid())
    {
     // Find range...
      real32 minDisp =  math::Infinity<real32>();
      real32 maxDisp = -math::Infinity<real32>();
      for (nat32 y=0;y<dispNew.Size(1);y++)
      {
       for (nat32 x=0;x<dispNew.Size(0);x++)
       {
        if ((!mask.Valid())||(mask.Get(x,y)))
        {
         minDisp = math::Min(minDisp,dispNew.Get(x,y));
         maxDisp = math::Max(maxDisp,dispNew.Get(x,y));
        }
       }
      }
    
     // Render...
      for (nat32 y=0;y<dispNew.Size(1);y++)
      {
       for (nat32 x=0;x<dispNew.Size(0);x++)
       {
        if ((!mask.Valid())||(mask.Get(x,y)))
        {
         real32 v = (dispNew.Get(x,y)-minDisp)/(maxDisp-minDisp);
         image.Get(x,y) = bs::ColourRGB(v,v,v);
        }
        else
        {
         image.Get(x,y) = bs::ColourRGB(0.0,0.0,1.0);
        }
       }
      }
    }
   }
   break;
   case 4: // Output disparity dirs
   {
    if (dispNew.Valid())
    {
     // First triangulate every point in the disparity map...
      ds::Array2D< math::Vect<3,real32> > loc(dispNew.Size(0),dispNew.Size(1));
      for (nat32 y=0;y<dispNew.Size(1);y++)
      {
       for (nat32 x=0;x<dispNew.Size(0);x++)
       {
        pair.Triangulate(real32(x),real32(y),dispNew.Get(x,y),loc.Get(x,y));
       }
      }
    
    
     // Now calculate forward difference surface normals and render them...
      for (nat32 y=0;y<dispNew.Size(1);y++)
      {
       for (nat32 x=0;x<dispNew.Size(0);x++)
       {
        math::Vect<3,real32> dirX;
        if (x+1<dispNew.Size(0))
        {
         dirX = loc.Get(x+1,y);
         dirX -= loc.Get(x,y);
        }
        else
        {
         dirX = loc.Get(x,y);
         dirX -= loc.Get(x-1,y);
        }
       
        math::Vect<3,real32> dirY;
        if (y+1<dispNew.Size(1))
        {
         dirY = loc.Get(x,y+1);
         dirY -= loc.Get(x,y);
        }
        else
        {
         dirY = loc.Get(x,y);
         dirY -= loc.Get(x,y-1);
        }
       
        math::Vect<3,real32> norm;
        math::CrossProduct(dirX,dirY,norm);
        norm.Normalise();
       
        image.Get(x,y).r = math::Clamp<real32>((norm[0]+1.0)/2.0,0.0,1.0);
        image.Get(x,y).g = math::Clamp<real32>((norm[1]+1.0)/2.0,0.0,1.0);
        image.Get(x,y).b = math::Clamp<real32>(norm[2],0.0,1.0);
       }
      }
    }
   }
   break;
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

//------------------------------------------------------------------------------
