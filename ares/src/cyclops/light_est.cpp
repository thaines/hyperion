//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/light_est.h"

//------------------------------------------------------------------------------
LightEst::LightEst(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),lightD(0.0,0.0,1.0),
irrVar(null<svt::Var*>()),segVar(null<svt::Var*>()),dispVar(null<svt::Var*>()),albedoVar(null<svt::Var*>()),
imageVar(null<svt::Var*>()),imgVar(null<svt::Var*>())
{
 // Create default images maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  nat32 segIni = 0;
  math::Fisher fishIni;
  real32 albedoIni = 0.0;

  irrVar = new svt::Var(cyclops.Core());
  irrVar->Setup2D(320,240);
  irrVar->Add("rgb",colourIni);
  irrVar->Commit();
  irrVar->ByName("rgb",irr);

  segVar = new svt::Var(cyclops.Core());
  segVar->Setup2D(320,240);
  segVar->Add("seg",segIni);
  segVar->Commit();
  segVar->ByName("seg",seg);
  
  dispVar = new svt::Var(cyclops.Core());
  dispVar->Setup2D(320,240);
  dispVar->Add("fish",fishIni);
  dispVar->Commit();
  dispVar->ByName("fish",fish);

  albedoVar = new svt::Var(cyclops.Core());
  albedoVar->Setup2D(320,240);
  albedoVar->Add("albedo",albedoIni);
  albedoVar->Commit();
  albedoVar->ByName("albedo",albedo);


  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colourIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);
  
  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);
  
 // Make default camera response function flat...
  crf.SetMult();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Light Source & Albedo Estimation");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   lightDir = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lightDir->Set("Not yet run");
   vert1->AttachBottom(lightDir,false);

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

   but1->SetChild(lab1); lab1->Set("Load Irradiance...");
   but2->SetChild(lab2); lab2->Set("Load Camera Response...");
   but3->SetChild(lab3); lab3->Set("Load Segmentation...");
   but4->SetChild(lab4); lab4->Set("Load Disparity...");
   but5->SetChild(lab5); lab5->Set("Run");
   but6->SetChild(lab6); lab6->Set("Save View...");
   
   
   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Irradiance");
   viewSelect->Append("Corrected Irradiance");
   viewSelect->Append("Segmentation");
   viewSelect->Append("Fisher Direction");
   viewSelect->Append("Fisher Concentration (log10)");
   viewSelect->Append("Sphere Rendered with Estimate");
   viewSelect->Append("Cost of Direction Sphere");
   viewSelect->Append("Albedo");
   viewSelect->Set(0);
   
   algSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   algSelect->Append("Haines & Wilson");
   algSelect->Set(0);


   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   horiz2->AttachRight(viewSelect,false);
   horiz2->AttachRight(algSelect,false);
   horiz2->AttachRight(but5,false);
   horiz2->AttachRight(but6,false);



   brutalFish = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(brutalFish,false);
   brutalFish->Visible(true);
   brutalFish->Set("Haines & Wilson Parameters");
   brutalFish->Expand(false);
   
   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));         
   gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz4 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   brutalFish->SetChild(vert2);
   vert2->AttachBottom(horiz3,false);
   vert2->AttachBottom(horiz4,false);
   
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   bfMinAlb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfMaxAlb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfMaxSegCost = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfSampleSubdiv = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfAlbRecursion = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab7->Set("    Minimum Albedo");
   lab8->Set(" Maximum Albedo");
   lab9->Set(" Maximum Segment Cost");
   lab10->Set("   Sampling Subdivisions");
   lab11->Set(" Albedo Recursion Depth");
   
   bfMinAlb->Set("0.001");
   bfMaxAlb->Set("3.0");
   bfMaxSegCost->Set("1.0");
   bfSampleSubdiv->Set("4");
   bfAlbRecursion->Set("8");
   
   bfMinAlb->SetSize(48,24);
   bfMaxAlb->SetSize(48,24);
   bfMaxSegCost->SetSize(48,24);
   bfSampleSubdiv->SetSize(48,24);
   bfAlbRecursion->SetSize(48,24);
   
   horiz3->AttachRight(lab7,false);
   horiz3->AttachRight(bfMinAlb,false);
   horiz3->AttachRight(lab8,false);
   horiz3->AttachRight(bfMaxAlb,false);
   horiz3->AttachRight(lab9,false);
   horiz3->AttachRight(bfMaxSegCost,false);
   
   horiz4->AttachRight(lab10,false);
   horiz4->AttachRight(bfSampleSubdiv,false);
   horiz4->AttachRight(lab11,false);
   horiz4->AttachRight(bfAlbRecursion,false);



   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&LightEst::Quit));
  canvas->OnResize(MakeCB(this,&LightEst::Resize));
  but1->OnClick(MakeCB(this,&LightEst::LoadIrr));
  but2->OnClick(MakeCB(this,&LightEst::LoadCRF));
  but3->OnClick(MakeCB(this,&LightEst::LoadSeg));
  but4->OnClick(MakeCB(this,&LightEst::LoadDisp));
  but5->OnClick(MakeCB(this,&LightEst::Run));
  but6->OnClick(MakeCB(this,&LightEst::SaveView));
  viewSelect->OnChange(MakeCB(this,&LightEst::ChangeView));
  algSelect->OnChange(MakeCB(this,&LightEst::ChangeAlg));
}

LightEst::~LightEst()
{
 delete win;
 delete irrVar;
 delete segVar;
 delete dispVar;
 delete albedoVar;
 delete imageVar;
 delete imgVar;
}

void LightEst::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void LightEst::Resize(gui::Base * obj,gui::Event * event)
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

void LightEst::LoadIrr(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Irradiance Image","*.bmp,*.jpg,*.png,*.tif",fn))
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

  // Stick in right variables...
   delete irrVar;
   irrVar = tempVar;
   irrVar->ByName("rgb",irr);

  // Update the display...
   Update();
 }
}

void LightEst::LoadCRF(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera Response Function","*.crf",fn))
 {
  // Load...
   if (crf.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load function");
   }
   
  // Update the display...
   Update();   
 }
}

void LightEst::LoadSeg(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Segmentation File","*.seg",fn))
 {
  // Load svt, verify it is a segmentation...
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
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a segmentation");
    delete floatNode;
    return;
   }
   svt::Var * floatVar = static_cast<svt::Var*>(floatNode);

   nat32 segInd;
   if ((floatVar->Dims()!=2)||
       (floatVar->GetIndex(cyclops.TT()("seg"),segInd)==false)||
       (floatVar->FieldType(segInd)!=cyclops.TT()("eos::nat32")))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a segmentation");
    delete floatVar;
    return;
   }

  // Store it...
   delete segVar;
   segVar = floatVar;
   segVar->ByName("seg",seg);
   
  // Update the display...
   Update();   
 }
}
void LightEst::LoadDisp(gui::Base * obj,gui::Event * event)
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

   nat32 fishInd;
   if ((floatVar->GetIndex(cyclops.TT()("fish"),fishInd)==false)||
       (floatVar->FieldType(fishInd)!=cyclops.TT()("eos::math::Fisher")))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is not augmented with orientation information.");
    delete floatVar;
    return;
   }


  // Move the floatVar into the asReal array, deleting previous...
   delete dispVar;
   dispVar = floatVar;
   dispVar->ByName("fish",fish);


  // Refresh the display...
   Update();
 }
}

void LightEst::Run(gui::Base * obj,gui::Event * event)
{
 // Get the parameters...
  real32 minAlb = bfMinAlb->GetReal(0.001);
  real32 maxAlb = bfMaxAlb->GetReal(3.0);
  real32 maxCost = bfMaxSegCost->GetReal(1.0);
  nat32 subdiv = bfSampleSubdiv->GetInt(4);
  nat32 recursion = bfAlbRecursion->GetInt(8);


 // Calculate the corrected irradiance...
  svt::Var tempVar(irr);
  real32 irrIni = 0.0;
  tempVar.Add("irr",irrIni);
  tempVar.Commit();
  svt::Field<real32> irradiance(&tempVar,"irr");
  
  for (nat32 y=0;y<irr.Size(1);y++)
  {
   for (nat32 x=0;x<irr.Size(0);x++)
   {
    irradiance.Get(x,y) = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);
   }
  }


 // Setup the algorithm object...
  fit::LightDir ld;
  ld.SetData(seg,irradiance,fish);
  ld.SetAlbRange(minAlb,maxAlb);
  ld.SetSegCap(maxCost);
  ld.SetSampleSubdiv(subdiv);
  ld.SetRecursion(recursion);


 // Run the algorithm...
  ld.Run(cyclops.BeginProg());
  cyclops.EndProg();


 // Get the output...
  lightD = ld.BestLightDir();
  samples.Size(ld.SampleSize());
  for (nat32 i=0;i<samples.Size();i++)
  {
   samples[i].cost = ld.SampleCost(i);
   samples[i].dir = ld.SampleDir(i);
   LogDebug("sample {dir,cost}" << LogDiv() << samples[i].dir << LogDiv() << samples[i].cost);
  }
  
  str::String s;
  s << lightD << " (" << ld.SampleSize() << " samples)";
  lightDir->Set(s);
  
 // Update the view...
  Update();
}

void LightEst::ChangeView(gui::Base * obj,gui::Event * event)
{
 Update();
}

void LightEst::ChangeAlg(gui::Base * obj,gui::Event * event)
{
 // Noop.
}

void LightEst::SaveView(gui::Base * obj,gui::Event * event)
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

void LightEst::Update()
{
 // Find what size we need to be operatinf at...
  nat32 width=320, height=240;
  nat32 mode = viewSelect->Get();
  switch(mode)
  {
   case 0: // Irradiance
   case 1: // Corrected Irradiance
   case 6: // Cost Sphere
   case 5: // Light Source Sphere
    width = irr.Size(0);
    height = irr.Size(1);
   break;
   case 2: // Segmentation
    width = seg.Size(0);
    height = seg.Size(1);
   break;
   case 3: // Fisher Direction
   case 4: // Fisher Concentration
    width = fish.Size(0);
    height = fish.Size(1);
   break;
   case 7: // Albedo
    width = albedo.Size(0);
    height = albedo.Size(1);
   break;
  }

 // Check sizes match, adjust if need be...
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

 
 // Switch on mode and render correct image...  
  switch(mode)
  {
   case 0: // Irradiance
   {
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
   break;
   case 1: // Corrected Irradiance
   {
    real32 max = 0.01;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 l = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);
      max = math::Max(max,l);
      image.Get(x,y).r = l;
      image.Get(x,y).g = l;
      image.Get(x,y).b = l;
     }
    }
    
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) /= max;
    }
   }
   break;
   case 2: // Segmentation
   {
    filter::RenderSegsMean(seg,irr,image);
    filter::RenderSegsLines(seg,image,bs::ColourRGB(1.0,0.0,0.0));
   }
   break;
   case 3: // Fisher Direction
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      math::Fisher f = fish.Get(x,y);
      real32 len = f.Length();
      if (math::IsZero(len))
      {
       image.Get(x,y).r = 0.0;
       image.Get(x,y).g = 0.0;
       image.Get(x,y).b = 0.0;
      }
      else
      {
       f /= len;
       
       image.Get(x,y).r = f[0]*0.5+0.5;
       image.Get(x,y).g = f[1]*0.5+0.5;
       image.Get(x,y).b = f[2];
      }
     }
    }
   }
   break;
   case 4: // Fisher Concentration
   {
    real32 max = 0.001;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 k = math::Log10(1.0 + fish.Get(x,y).Length());
      max = math::Max(max,k);
      
      image.Get(x,y).r = k;
      image.Get(x,y).g = k;
      image.Get(x,y).b = k;
     }
    }
    
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) /= max;
    }
   }
   break;
   case 5: // Light Source Sphere
   {
    real32 centX = irr.Size(0)/2.0;
    real32 centY = irr.Size(1)/2.0;
    real32 radius = math::Min(centX,centY)*0.9;
    
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 dirX = (x-centX)/radius;
      real32 dirY = (y-centY)/radius;
      
      real32 l;
      if (math::Sqr(dirX)+math::Sqr(dirY)<1.0)
      {
       real32 dirZ = math::Sqrt(1.0 - math::Sqr(dirX) - math::Sqr(dirY));
       l = dirX*lightD[0] + dirY*lightD[1] + dirZ*lightD[2];
       l = math::Max<real32>(l,0.0);
      }
      else
      {
       l = 0.0;
      }
      
      image.Get(x,y).r = l;
      image.Get(x,y).g = l;
      image.Get(x,y).b = l;
     }
    }
   }
   break;
   case 6: // Cost Sphere
   {
    if (samples.Size()!=0)
    {
     // First generate a Delauney triangulation with all the samples in, with 
     // costs as the stored data...
      ds::Delaunay2D<real32> doc;
      real32 minCost = math::Infinity<real32>();
      for (nat32 i=0;i<samples.Size();i++)
      {
       doc.Add(samples[i].dir[0],samples[i].dir[1],samples[i].cost);
       minCost = math::Min(minCost,samples[i].cost);
      }
     
     // Now iterate all the pixels, interpolating costs for those inside the 
     // sphere and rendering the Log10 of the costs...
      real32 max = 0.001;
      real32 centX = irr.Size(0)/2.0;
      real32 centY = irr.Size(1)/2.0;
      real32 radius = math::Min(centX,centY)*0.9;   
     
      for (nat32 y=0;y<image.Size(1);y++)
      {
       for (nat32 x=0;x<image.Size(0);x++)
       {
        real32 dirX = (x-centX)/radius;
        real32 dirY = (y-centY)/radius;
       
        real32 l;
        if (math::Sqr(dirX)+math::Sqr(dirY)<1.0)
        {
         ds::Delaunay2D<real32>::Mid tri = doc.Triangle(dirX,dirY);
         nat32 pc = 0;
         real32 cost[3];
         real32 xp[3];
         real32 yp[3];
         for (nat32 i=0;i<3;i++)
         {
          if (!tri.Infinite(i))
          {
           ds::Delaunay2D<real32>::Pos pos = tri.GetPos(i);
           cost[pc] = *pos - minCost;
           xp[pc] = pos.X();
           yp[pc] = pos.Y();
           ++pc;
          }
         }
         
         switch(pc)
         {
          case 0:
           l = -1.0;
          break;
          case 1:
           l = cost[0];
          break;
          case 2:
          {
           real32 length = math::Sqrt(math::Sqr(xp[1]-xp[0]) + math::Sqr(yp[1]-yp[0]));
           real32 t = ((dirX-xp[0])*(xp[1]-xp[0]) + (dirY-yp[0])*(yp[1]-yp[0]))/math::Sqr(length);
           l = (1.0-t)*cost[0] + t*cost[1];
          }
          break;
          case 3:
          {
           // Below is using Barycentric Coordinates...
            real32 xC =  dirX-xp[2];
            real32 yC =  dirY-yp[2];
            real32 x1 = xp[0]-xp[2];
            real32 x2 = xp[1]-xp[2];
            real32 y1 = yp[0]-yp[2];
            real32 y2 = yp[1]-yp[2];
           
            real32 mult = 1.0/(x1*y2 - x2*y1);
           
            real32 lam1 = (y2*xC - x2*yC) * mult;
            real32 lam2 = (x1*yC - y1*xC) * mult;

            l = lam1*cost[0] + lam2*cost[1] + (1.0-lam1-lam2)*cost[2];
          }
          break;
         }
        }
        else
        {
         l = -1.0;
        }
       
        max = math::Max(max,l);
        image.Get(x,y).r = l;
        image.Get(x,y).g = l;
        image.Get(x,y).b = l;
       }
      }
    
     // Normalise...
      for (nat32 y=0;y<image.Size(1);y++)
      {
       for (nat32 x=0;x<image.Size(0);x++)
       {
        if (image.Get(x,y).r<0.0)
        {
         image.Get(x,y).r = 0.0;
         image.Get(x,y).g = 0.0;
         image.Get(x,y).b = 0.5;
        }
        else image.Get(x,y) /= max;
       }
      }
    }
   }
   break;
   case 7: // Albedo
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      image.Get(x,y).r = albedo.Get(x,y);
      image.Get(x,y).g = albedo.Get(x,y);
      image.Get(x,y).b = albedo.Get(x,y);
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
