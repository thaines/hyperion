//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/to_mesh.h"

//------------------------------------------------------------------------------
ToMesh::ToMesh(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
asReal(null<svt::Var*>()),asImg(null<svt::Var*>())
{
 // Create default disparity array...
  asReal = new svt::Var(cyclops.Core());
  asReal->Setup2D(320,240);
  real32 dispIni = 0.0;
  bit maskIni = true;
  asReal->Add("disp",dispIni);
  asReal->Add("mask",maskIni);
  asReal->Commit();
  asReal->ByName("disp",disp);
  asReal->ByName("mask",dispMask);

 // Create default image...
  asImg = new svt::Var(cyclops.Core());
  asImg->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  asImg->Add("rgb",colIni);
  asImg->Commit();
  asImg->ByName("rgb",image);

 // Create default calibration...
  pair.SetDefault(320,240);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Disparity To Mesh");
  cyclops.App().Attach(win);
  win->SetSize(480,308);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but2->SetChild(lab2); lab2->Set("Load Disparity...");
   but3->SetChild(lab3); lab3->Set("Load Calibration...");
   but4->SetChild(lab4); lab4->Set("Save Mesh...");

   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);


   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   conLimit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   sample = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab9->Set("   Connect Cap:");
   conLimit->Set("2");
   lab6->Set("Sample Rate:");
   sample->Set("1");

   horiz2->AttachRight(lab9,false);
   horiz2->AttachRight(conLimit,false);
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(sample,false);


   gui::Label * space = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   space->Set(" ");
   horiz1->AttachRight(space,false);

   gui::Button * but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but7->SetChild(lab7); lab7->Set("Blur");
   horiz1->AttachRight(but7,false);

   gui::Button * but8 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but8->SetChild(lab8); lab8->Set("Quant");
   horiz1->AttachRight(but8,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));

 // Event handlers...
  win->OnDeath(MakeCB(this,&ToMesh::Quit));
  canvas->OnResize(MakeCB(this,&ToMesh::Resize));
  but2->OnClick(MakeCB(this,&ToMesh::LoadSvt));
  but3->OnClick(MakeCB(this,&ToMesh::LoadPair));
  but4->OnClick(MakeCB(this,&ToMesh::SaveModel));
  but7->OnClick(MakeCB(this,&ToMesh::Blur));
  but8->OnClick(MakeCB(this,&ToMesh::Quant));
}

ToMesh::~ToMesh()
{
 delete win;
 delete asImg;
 delete asReal;
}

void ToMesh::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void ToMesh::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // Update...
  canvas->Update();
}

void ToMesh::LoadSvt(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity File","*.obf,*.dis",fn))
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


  // Move the floatVar into the asReal array, deleting previous...
   delete asReal;
   asReal = floatVar;
   if (!asReal->Exists("mask"))
   {
    bit maskIni = true;
    asReal->Add("mask",maskIni);
    asReal->Commit();
   }
   asReal->ByName("disp",disp);
   asReal->ByName("mask",dispMask);
   real32 maxDisp = 0.1;
   real32 minDisp = 0.0;
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (dispMask.Get(x,y))
     {
      minDisp = math::Min(minDisp,disp.Get(x,y));
      maxDisp = math::Max(maxDisp,disp.Get(x,y));
     }
    }
   }


  // Now convert asReal into asImg, for display...
   asImg->Setup2D(floatVar->Size(0),floatVar->Size(1));
   asImg->Commit();
   asImg->ByName("rgb",image);

   maxDisp = 255.0/(maxDisp-minDisp);
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     if (dispMask.Get(x,y))
     {
      byte v = byte((disp.Get(x,y)-minDisp)*maxDisp);
      image.Get(x,y) = bs::ColRGB(v,v,v);
     }
     else image.Get(x,y) = bs::ColRGB(0,0,255);
    }
   }


  // Refresh the display...
   canvas->SetSize(image.Size(0),image.Size(1));
   canvas->Redraw();
 }
}

void ToMesh::LoadPair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
 }
}

void ToMesh::Blur(gui::Base * obj,gui::Event * event)
{
 LogBlock("void ToMesh::Blur(...)","-");

 // Do the blur...
  filter::KernelVect kernel(2);
  kernel.MakeGaussian(math::Sqrt(2.0));
  kernel.Apply(disp,disp);


 // Update the image...
  real32 maxDisp = 0.1;
  real32 minDisp = 0.0;
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    if (dispMask.Get(x,y))
    {
     minDisp = math::Min(minDisp,disp.Get(x,y));
     maxDisp = math::Max(maxDisp,disp.Get(x,y));
    }
   }
  }

  maxDisp = 255.0/(maxDisp-minDisp);
  for (nat32 y=0;y<image.Size(1);y++)
  {
   for (nat32 x=0;x<image.Size(0);x++)
   {
    if (dispMask.Get(x,y))
    {
     byte v = byte((disp.Get(x,y)-minDisp)*maxDisp);
     image.Get(x,y) = bs::ColRGB(v,v,v);
    }
    else image.Get(x,y) = bs::ColRGB(0,0,255);
   }
  }


 // Redraw...
  canvas->Redraw();
}

void ToMesh::Quant(gui::Base * obj,gui::Event * event)
{
 LogBlock("void ToMesh::Quant(...)","-");

 // Round everything...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    disp.Get(x,y) = math::Round(disp.Get(x,y));
   }
  }

 // Update the image...
  real32 maxDisp = 0.1;
  real32 minDisp = 0.0;
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    if (dispMask.Get(x,y))
    {
     minDisp = math::Min(minDisp,disp.Get(x,y));
     maxDisp = math::Max(maxDisp,disp.Get(x,y));
    }
   }
  }

  maxDisp = 255.0/(maxDisp-minDisp);
  for (nat32 y=0;y<image.Size(1);y++)
  {
   for (nat32 x=0;x<image.Size(0);x++)
   {
    if (dispMask.Get(x,y))
    {
     byte v = byte((disp.Get(x,y)-minDisp)*maxDisp);
     image.Get(x,y) = bs::ColRGB(v,v,v);
    }
    else image.Get(x,y) = bs::ColRGB(0,0,255);
   }
  }


 // Redraw...
  canvas->Redraw();
}

void ToMesh::SaveModel(gui::Base * obj,gui::Event * event)
{
 LogBlock("void ToMesh::SaveModel(...)","-");

 // Ask for a filename...
  str::String fn;
  if (cyclops.App().SaveFileDialog("Save .ply/.obj file...",fn))
  {
   time::Progress * prog = cyclops.BeginProg();

   // Obtain the parameters...
    real32 conCap = conLimit->GetReal(8.0);
    nat32 stride = sample->GetInt(1);


   
   // Create a mesh, with uv coordinates...
    str::TokenTable tokTab;
    sur::Mesh mesh(&tokTab);
    
    real32 realIni = 0.0;
    mesh.AddVertProp("u",realIni);
    mesh.AddVertProp("v",realIni);
    mesh.Commit();
    
    data::Property<sur::Vertex,real32> propU = mesh.GetVertProp<real32>("u");
    data::Property<sur::Vertex,real32> propV = mesh.GetVertProp<real32>("v");
    
    
   // Create the vertices...
    prog->Report(0,3);
    
    nat32 width = disp.Size(0)/stride;
    nat32 height = disp.Size(1)/stride;

    if ((width==0)||(height==0))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Stride is too high");
     return;
    }    
    
    ds::Array2D<sur::Vertex> hand(width,height); // Won't all be valid.

    prog->Push();
    for (nat32 y=0;y<height;y++)
    {
     prog->Report(y,height);
     for (nat32 x=0;x<width;x++)
     {
      if (dispMask.Get(x,y))
      {
       // First get the points...
        math::Vect<3,real64> li;
        li[0] = x*stride; li[1] = y*stride; li[2] = 1.0;

        math::Vect<3,real64> ri;
        ri = li;
        ri[0] += disp.Get(x*stride,y*stride);

       // Scale for image size...
        real64 scaleX = pair.leftDim[0]/real64(disp.Size(0));
        real64 scaleY = pair.leftDim[1]/real64(disp.Size(1));
        li[0] *= scaleX; li[1] *= scaleY;
        ri[0] *= scaleX; ri[1] *= scaleY;

       // Unrectify them...
        math::Vect<3,real64> lp;
        math::Vect<3,real64> rp;
        math::MultVect(pair.unRectLeft,li,lp);
        math::MultVect(pair.unRectRight,ri,rp);

       // Convert from homogenous...
        math::Vect<2,real64> lf;
         lf[0] = lp[0]/lp[2];
         lf[1] = lp[1]/lp[2];
        math::Vect<2,real64> rf;
         rf[0] = rp[0]/rp[2];
         rf[1] = rp[1]/rp[2];

       // Triangulate...
        math::Vect<4,real64> loc;
        if (cam::Triangulate(lf,rf,pair.lp,pair.rp,loc)==false)
        {
         hand.Get(x,y) = sur::Vertex();
         continue;
        }
        loc.Normalise();
        if (math::IsZero(loc[3]))
        {
         hand.Get(x,y) = sur::Vertex();
         continue;
        }

       // Check its in front of the camera...
        if (pair.lp.Depth(loc)>0.0)
        {
	 hand.Get(x,y) = sur::Vertex();
         continue;
        }

       // Store...
        bs::Vert vert;
         vert[0] = loc[0]/loc[3];
         vert[1] = loc[1]/loc[3];
         vert[2] = loc[2]/loc[3];
        if (vert.Length()>10000.0) // Simple distance cap.
        {
         hand.Get(x,y) = sur::Vertex();
         continue;
        }
        hand.Get(x,y) = mesh.NewVertex(vert);

       // Tack on the uv...
        propU.Get(hand.Get(x,y)) = real64(x)/real64(width);
        propV.Get(hand.Get(x,y)) = 1.0 - (real64(y)/real64(height));
      }
      else
      {
       hand.Get(x,y) = sur::Vertex();
      }
     }
    }
    prog->Pop();


   // Wire them up...
    prog->Report(1,3);
    prog->Push();
    for (nat32 y=0;y<height-1;y++)
    {
     prog->Report(y,height-1);
     for (nat32 x=0;x<width-1;x++)
     {
      if ((hand.Get(x,y).Valid())&&
          (hand.Get(x+1,y).Valid())&&
          (hand.Get(x,y+1).Valid())&&
          (hand.Get(x+1,y+1).Valid()))
      {
       real32 minDisp = math::Min(disp.Get(x*stride,y*stride),disp.Get(x*stride,(y+1)*stride),
                                    disp.Get((x+1)*stride,y*stride),disp.Get((x+1)*stride,(y+1)*stride));
       real32 maxDisp = math::Max(disp.Get(x*stride,y*stride),disp.Get(x*stride,(y+1)*stride),
                                    disp.Get((x+1)*stride,y*stride),disp.Get((x+1)*stride,(y+1)*stride));
       if ((maxDisp-minDisp)<=conCap)
       {
        mesh.NewFace(hand.Get(x,y),hand.Get(x+1,y),hand.Get(x+1,y+1),hand.Get(x,y+1));
       }
      }
     }
    }
    prog->Pop();


   // Save the file...
    prog->Report(2,3);
    if ((!fn.EndsWith(".obj"))&&(!fn.EndsWith(".ply"))) fn << ".ply";
    if (!file::SaveMesh(mesh,fn,true,prog))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     cyclops.EndProg();
     return;
    }



   cyclops.EndProg();
  }
}

//------------------------------------------------------------------------------
