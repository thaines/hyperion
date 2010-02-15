//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/sfs.h"

//------------------------------------------------------------------------------
SfS::SfS(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
dataVar(null<svt::Var*>()),visVar(null<svt::Var*>()),hasRun(false)
{
 // Create default images...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  bs::ColRGB colIni(0,0,0);
  bs::Normal needleIni(0.0,0.0,0.0);

  dataVar = new svt::Var(cyclops.Core());
  dataVar->Setup2D(320,240);
  dataVar->Add("rgb",colourIni);
  dataVar->Add("needle",needleIni);
  dataVar->Commit();
  dataVar->ByName("rgb",image);
  dataVar->ByName("needle",needle);

  visVar = new svt::Var(cyclops.Core());
  visVar->Setup2D(320,240);
  visVar->Add("rgb",colIni);
  visVar->Commit();
  visVar->ByName("rgb",visible);

 // Make camera response function flat...
  crf.SetMult();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Shape from Shading");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+128);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Load Camera Response...");
   but3->SetChild(lab3); lab3->Set("Run");
   but4->SetChild(lab4); lab4->Set("Save Needle...");
   
   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   

   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);
   
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   whichAlg = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   albedo = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lightDir = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab5->Set("Algorithm:");
   //whichAlg->Append("Zheng & Chellappa");
   whichAlg->Append("Lee & Kuo");
   whichAlg->Append("Worthington & Hancock");
   whichAlg->Append("Haines & Wilson 1");
   whichAlg->Append("Haines & Wilson 2");
   whichAlg->Append("Haines & Wilson 3");
   whichAlg->Set(4);
   lab6->Set("Albedo:");
   albedo->Set("1.0");
   albedo->SetSize(24,24);
   lab7->Set("To Light:");
   lightDir->Set("(0.0,0.0,1.0)");
   albedo->SetSize(48,24);

   horiz2->AttachRight(lab5,false);
   horiz2->AttachRight(whichAlg,false);
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(albedo,false);
   horiz2->AttachRight(lab7,false);
   horiz2->AttachRight(lightDir,false);
   
   
   alg1 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg1,false);
   alg1->Visible(false);
   alg1->Set("Zheng & Chellappa Parameters");
   alg1->Expand(false);
   
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   zacMu = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); zacMu->SetSize(48,24);
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   zacIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); zacIters->SetSize(48,24);
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   zacDelta = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); zacDelta->SetSize(48,24);
   
   gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   alg1->SetChild(horiz3);
   
   lab8->Set("   Mu:");
   zacMu->Set("1.0");
   lab9->Set("   Iters:");
   zacIters->Set("512");
   lab10->Set("   Delta:");
   zacDelta->Set("0.0001");
   
   horiz3->AttachRight(lab8,false);
   horiz3->AttachRight(zacMu,false);
   horiz3->AttachRight(lab9,false);
   horiz3->AttachRight(zacIters,false);
   horiz3->AttachRight(lab10,false);
   horiz3->AttachRight(zacDelta,false);


   alg2 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg2,false);
   alg2->Visible(false);
   alg2->Set("Lee & Kuo Parameters");
   alg2->Expand(false);
   
   gui::Label * lab24 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lakOuterIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab25 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lakInnerIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab27 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lakTolerance = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab28 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lakStartLambda = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   lakEndLambda = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab29 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lakSpeed = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab30 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lakInitRad = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));         
   gui::Horizontal * horiz8 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz9 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   alg2->SetChild(vert3);
   vert3->AttachBottom(horiz8,false);
   vert3->AttachBottom(horiz9,false);
   
   lab24->Set("   Outer Iterations:");
   lakOuterIters->Set("512"); lakOuterIters->SetSize(64,24);
   lab28->Set("   Start/End Lambda:");
   lakStartLambda->Set("32.0"); lakStartLambda->SetSize(48,24);
   lakEndLambda->Set("32.0"); lakEndLambda->SetSize(48,24);
   lab25->Set("   Inner Iterations:");
   lakInnerIters->Set("512"); lakInnerIters->SetSize(48,24);
   lab27->Set("   Tolerance:");
   lakTolerance->Set("0.001"); lakTolerance->SetSize(48,24);
   lab29->Set("   Speed:");
   lakSpeed->Set("1.0"); lakSpeed->SetSize(48,24);
   lab30->Set("   Init Radius:");
   lakInitRad->Set("32.0"); lakInitRad->SetSize(48,24);
   
   horiz8->AttachRight(lab24,false);
   horiz8->AttachRight(lakOuterIters,false);
   horiz8->AttachRight(lab25,false);
   horiz8->AttachRight(lakInnerIters,false);
   horiz8->AttachRight(lab29,false);
   horiz8->AttachRight(lakSpeed,false);
   horiz9->AttachRight(lab28,false);
   horiz9->AttachRight(lakStartLambda,false);
   horiz9->AttachRight(lakEndLambda,false);
   horiz9->AttachRight(lab30,false);
   horiz9->AttachRight(lakInitRad,false);
   horiz9->AttachRight(lab27,false);
   horiz9->AttachRight(lakTolerance,false);    


   alg3 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg3,false);
   alg3->Visible(false);
   alg3->Set("Worthington & Hancock Parameters");
   alg3->Expand(false);
   
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   wahIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   
   gui::Horizontal * horiz4 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   alg3->SetChild(horiz4);
   
   lab11->Set("   Iters:");
   wahIters->Set("256");
   
   horiz4->AttachRight(lab11,false);
   horiz4->AttachRight(wahIters,false);
   
   
   alg4 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg4,false);
   alg4->Visible(false);
   alg4->Set("Haines & Wilson 1 Parameters");
   alg4->Expand(false);
   
   gui::Label * lab12 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab13 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab14 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab15 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab16 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab17 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab18 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab19 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab20 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab21 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab22 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab23 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   hawBlur = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawBlur->SetSize(48,24);
   hawLength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawLength->SetSize(48,24);
   hawExp = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawExp->SetSize(48,24);
   hawStopChance = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawStopChance->SetSize(48,24);
   hawSimK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawSimK->SetSize(48,24);
   hawConeK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawConeK->SetSize(48,24);
   hawFadeTo = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawFadeTo->SetSize(48,24);
   hawIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawIters->SetSize(48,24);
   hawGradDisc = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawGradDisc->SetSize(48,24);
   hawGradBias = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawGradBias->SetSize(48,24);
   hawBorderK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); hawBorderK->SetSize(48,24);
   hawProject = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   
   lab12->Set("   Blur:");
   lab13->Set(" Walk Length:");
   lab14->Set(" Exponent:");
   lab15->Set(" Stop Chance:");
   lab16->Set("   Similarity K:");
   lab17->Set(" Cone K:");
   lab18->Set(" Fade To:");
   lab19->Set(" Iters:");
   lab20->Set("   Grad Disc:");
   lab21->Set(" Grad Bias:");
   lab22->Set(" Border K:");
   lab23->Set("Project");
   
   hawBlur->Set("1.4");
   hawLength->Set("8");
   hawExp->Set("6");
   hawStopChance->Set("0.0");
   hawSimK->Set("4.5");
   hawConeK->Set("32.0");
   hawFadeTo->Set("1.0");
   hawIters->Set("10");
   hawGradDisc->Set("0.0");
   hawGradBias->Set("4.0");
   hawBorderK->Set("0.0");
   hawProject->SetState(false);
   
   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   gui::Horizontal * horiz5 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz6 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz7 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   alg4->SetChild(vert2);
   vert2->AttachBottom(horiz5,false);
   vert2->AttachBottom(horiz6,false);
   vert2->AttachBottom(horiz7,false);
   
   horiz5->AttachRight(lab12,false);
   horiz5->AttachRight(hawBlur,false);
   horiz5->AttachRight(lab13,false);
   horiz5->AttachRight(hawLength,false);
   horiz5->AttachRight(lab14,false);
   horiz5->AttachRight(hawExp,false);
   horiz5->AttachRight(lab15,false);
   horiz5->AttachRight(hawStopChance,false);
   
   horiz6->AttachRight(lab16,false);
   horiz6->AttachRight(hawSimK,false);
   horiz6->AttachRight(lab17,false);
   horiz6->AttachRight(hawConeK,false);
   horiz6->AttachRight(lab18,false);
   horiz6->AttachRight(hawFadeTo,false);
   horiz6->AttachRight(lab19,false);
   horiz6->AttachRight(hawIters,false);
   
   horiz7->AttachRight(lab20,false);
   horiz7->AttachRight(hawGradDisc,false);
   horiz7->AttachRight(lab21,false);
   horiz7->AttachRight(hawGradBias,false);
   horiz7->AttachRight(lab22,false);
   horiz7->AttachRight(hawBorderK,false);
   hawProject->SetChild(lab23);
   horiz7->AttachRight(hawProject,false);


   alg5 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg5,false);
   alg5->Visible(false);
   alg5->Set("Haines & Wilson 2 Parameters");
   alg5->Expand(false);
   
   gui::Vertical * vert4 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));         
   gui::Horizontal * horiz10 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz11 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz12 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz13 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   alg5->SetChild(vert4);
   vert4->AttachBottom(horiz10,false);
   vert4->AttachBottom(horiz11,false);
   vert4->AttachBottom(horiz12,false);
   vert4->AttachBottom(horiz13,false);
   
   gui::Label * lab31 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab32 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab33 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab34 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab35 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab36 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab37 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab38 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab39 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab40 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab41 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab42 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   haw2SimK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2SimK->SetSize(48,24);
   haw2ConeK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2ConeK->SetSize(48,24);
   haw2FadeTo = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2FadeTo->SetSize(48,24);
   haw2Iters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2Iters->SetSize(48,24);
   haw2AngMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2AngMult->SetSize(48,24);
   haw2Momentum = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2Momentum->SetSize(48,24);
   haw2BoundK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2BoundK->SetSize(48,24);
   haw2BoundLength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2BoundLength->SetSize(48,24);
   haw2BoundExp = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2BoundExp->SetSize(48,24);
   haw2GradK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2GradK->SetSize(48,24);
   haw2GradLength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2GradLength->SetSize(48,24);
   haw2GradExp = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw2GradExp->SetSize(48,24);
   
   lab31->Set("   Similarity K:");
   lab32->Set(" Cone K:");
   lab33->Set(" Fade To:");
   lab34->Set("   Iters:");
   lab35->Set(" Angle Multiplier:");
   lab36->Set(" Momentum:");
   lab37->Set("   Boundary K:");
   lab38->Set(" Boundary Walk Length:");
   lab39->Set(" Boundary Exponent:");
   lab40->Set("   Gradient K:");
   lab41->Set(" Gradient Walk Length:");
   lab42->Set(" Gradient Exponent:");
   
   haw2SimK->Set("4.0");
   haw2ConeK->Set("32.0");
   haw2FadeTo->Set("1.0");
   haw2Iters->Set("10");
   haw2AngMult->Set("1.0");
   haw2Momentum->Set("0.01");
   haw2BoundK->Set("16.0");
   haw2BoundLength->Set("8");
   haw2BoundExp->Set("6.0");
   haw2GradK->Set("2.0");
   haw2GradLength->Set("8");
   haw2GradExp->Set("6.0");
   
   horiz10->AttachRight(lab31,false);
   horiz10->AttachRight(haw2SimK,false);
   horiz10->AttachRight(lab32,false);
   horiz10->AttachRight(haw2ConeK,false);
   horiz10->AttachRight(lab33,false);
   horiz10->AttachRight(haw2FadeTo,false);
   
   horiz11->AttachRight(lab34,false);
   horiz11->AttachRight(haw2Iters,false);
   horiz11->AttachRight(lab35,false);
   horiz11->AttachRight(haw2AngMult,false);
   horiz11->AttachRight(lab36,false);
   horiz11->AttachRight(haw2Momentum,false);
   
   horiz12->AttachRight(lab37,false);
   horiz12->AttachRight(haw2BoundK,false);
   horiz12->AttachRight(lab38,false);
   horiz12->AttachRight(haw2BoundLength,false);
   horiz12->AttachRight(lab39,false);
   horiz12->AttachRight(haw2BoundExp,false);

   horiz13->AttachRight(lab40,false);
   horiz13->AttachRight(haw2GradK,false);
   horiz13->AttachRight(lab41,false);
   horiz13->AttachRight(haw2GradLength,false);
   horiz13->AttachRight(lab42,false);
   horiz13->AttachRight(haw2GradExp,false);
   
   
   alg6 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg6,false);
   alg6->Visible(true);
   alg6->Set("Haines & Wilson 3 Parameters");
   alg6->Expand(false);
   
   gui::Vertical * vert5 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));         
   gui::Horizontal * horiz14 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz15 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz16 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz17 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   alg6->SetChild(vert5);
   vert5->AttachBottom(horiz14,false);
   vert5->AttachBottom(horiz15,false);
   vert5->AttachBottom(horiz16,false);
   vert5->AttachBottom(horiz17,false);
   
   gui::Label * lab43 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab44 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab45 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab46 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab47 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab48 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab49 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab50 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab51 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab52 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab53 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab54 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab55 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab56 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab57 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab58 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab59 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   haw3SmoothChance = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3SmoothChance->SetSize(48,24);
   haw3SmoothBase = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3SmoothBase->SetSize(48,24);
   haw3SmoothMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3SmoothMult->SetSize(48,24);
   haw3SmoothMinK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3SmoothMinK->SetSize(48,24);
   haw3SmoothMaxK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3SmoothMaxK->SetSize(48,24);
   haw3Cone0 = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3Cone0->SetSize(48,24);
   haw3Cone45 = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3Cone45->SetSize(48,24);
   haw3Cone90 = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3Cone90->SetSize(48,24);
   haw3Iters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3Iters->SetSize(48,24);
   haw3BoundK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3BoundK->SetSize(48,24);
   haw3BoundLength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3BoundLength->SetSize(48,24);
   haw3BoundExp = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3BoundExp->SetSize(48,24);
   haw3GradK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3GradK->SetSize(48,24);
   haw3GradLength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3GradLength->SetSize(48,24);
   haw3GradExp = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3GradExp->SetSize(48,24);
   haw3AngMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3AngMult->SetSize(48,24);
   haw3Momentum = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox")); haw3Momentum->SetSize(48,24);
   
   lab43->Set("   Smooth Chance:");
   lab44->Set(" Smooth Base:");
   lab45->Set(" Smooth Mult:");
   lab46->Set(" Smooth Min K:");
   lab47->Set(" Smooth Max K:");
   lab48->Set("   Cone 0 K:");
   lab49->Set(" Cone 45 K:");
   lab50->Set(" Cone 90 K:");
   lab51->Set(" Iters:");
   lab52->Set("   Boundary K:");
   lab53->Set(" Boundary Length:");
   lab54->Set(" Boundary Exp:");
   lab55->Set("   Gradient K:");
   lab56->Set(" Gradient Length:");
   lab57->Set(" Gradient Exp:");
   lab58->Set(" Angle Multiplier:");
   lab59->Set(" Momentum:");
   
   haw3SmoothChance->Set("0.05");
   haw3SmoothBase->Set("15.0");
   haw3SmoothMult->Set("0.0");
   haw3SmoothMinK->Set("1.0");
   haw3SmoothMaxK->Set("6.0");
   haw3Cone0->Set("24.0");
   haw3Cone45->Set("32.0");
   haw3Cone90->Set("24.0");
   haw3Iters->Set("10");
   haw3BoundK->Set("16.0");
   haw3BoundLength->Set("8");
   haw3BoundExp->Set("6.0");
   haw3GradK->Set("0.1");
   haw3GradLength->Set("8");
   haw3GradExp->Set("6.0");
   haw3AngMult->Set("1.0");
   haw3Momentum->Set("0.01");
   
   horiz14->AttachRight(lab43,false);
   horiz14->AttachRight(haw3SmoothChance,false);
   horiz14->AttachRight(lab44,false);
   horiz14->AttachRight(haw3SmoothBase,false);
   horiz14->AttachRight(lab45,false);
   horiz14->AttachRight(haw3SmoothMult,false);
   horiz14->AttachRight(lab46,false);
   horiz14->AttachRight(haw3SmoothMinK,false);
   horiz14->AttachRight(lab47,false);
   horiz14->AttachRight(haw3SmoothMaxK,false);
   
   horiz15->AttachRight(lab48,false);
   horiz15->AttachRight(haw3Cone0,false);
   horiz15->AttachRight(lab49,false);
   horiz15->AttachRight(haw3Cone45,false);
   horiz15->AttachRight(lab50,false);
   horiz15->AttachRight(haw3Cone90,false);
   
   horiz16->AttachRight(lab52,false);
   horiz16->AttachRight(haw3BoundK,false);
   horiz16->AttachRight(lab53,false);
   horiz16->AttachRight(haw3BoundLength,false);
   horiz16->AttachRight(lab54,false);
   horiz16->AttachRight(haw3BoundExp,false);
   horiz16->AttachRight(lab51,false);
   horiz16->AttachRight(haw3Iters,false);
   
   horiz17->AttachRight(lab55,false);
   horiz17->AttachRight(haw3GradK,false);
   horiz17->AttachRight(lab56,false);
   horiz17->AttachRight(haw3GradLength,false);
   horiz17->AttachRight(lab57,false);
   horiz17->AttachRight(haw3GradExp,false);
   horiz17->AttachRight(lab58,false);
   horiz17->AttachRight(haw3AngMult,false);
   horiz17->AttachRight(lab59,false);
   horiz17->AttachRight(haw3Momentum,false);
   

   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&SfS::Quit));
  canvas->OnResize(MakeCB(this,&SfS::Resize));

  but1->OnClick(MakeCB(this,&SfS::LoadImage));
  but2->OnClick(MakeCB(this,&SfS::LoadCalib));
  but3->OnClick(MakeCB(this,&SfS::Run));
  but4->OnClick(MakeCB(this,&SfS::SaveNeedle));
  whichAlg->OnChange(MakeCB(this,&SfS::ChangeAlg));
}

SfS::~SfS()
{
 delete win;
 delete dataVar;
 delete visVar; 
}

void SfS::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void SfS::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - visible.Size(0))/2;
  nat32 sy = (canvas->P().Height() - visible.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(visible.Size(0),visible.Size(1))),bs::Pos(sx,sy),visible);

 // Update...
  canvas->Update();
}

void SfS::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * newVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (newVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

   delete dataVar;
   dataVar = newVar;
   bs::Normal needleIni(0.0,0.0,0.0);
   dataVar->Add("needle",needleIni);
   dataVar->Commit();
   dataVar->ByName("rgb",image);
   dataVar->ByName("needle",needle);
   
   hasRun = false;

  // Redraw...
   UpdateView();
 }
}

void SfS::LoadCalib(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera Response Function","*.crf",fn))
 {
  // Load...
   if (crf.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load function");
   }
 }
}

void SfS::Run(gui::Base * obj,gui::Event * event)
{
 // Read in basic paramterers, remembering to compensate for the response 
 // function for albedo...
  real32 alb = crf(albedo->GetReal(1.0));
  bs::Normal toLight;
  {
   str::String s = lightDir->Get();
   str::String::Cursor cur = s.GetCursor();
   cur.ClearError();
   cur >> toLight;
   if (cur.Error()) toLight = bs::Normal(0.0,0.0,1.0);
  }
  
 
 // Create a copy of the image that has had the camera response function applied...
 // (Knock up an albedo map whilst at it.)
  real32 realIni = 0.0;
  svt::Var tempVar(image);
  tempVar.Add("irr",realIni);
  tempVar.Add("albedo",realIni);
  tempVar.Commit();
  
  svt::Field<real32> l(&tempVar,"irr");
  svt::Field<real32> a(&tempVar,"albedo");
  
  for (nat32 y=0;y<tempVar.Size(1);y++)
  {
   for (nat32 x=0;x<tempVar.Size(0);x++)
   {
    l.Get(x,y) = crf((image.Get(x,y).r+image.Get(x,y).g+image.Get(x,y).b)/3.0);
    a.Get(x,y) = alb;
   }
  }

 
 // Run the selected algorithm...
  switch (whichAlg->Get())
  {
   /*case 0: // Zheng & Chellappa...
   {
    // Get algorithm specific parameters...
     real32 mu = zacMu->GetReal(1.0);
     nat32 iters = zacIters->GetInt(512);
     real32 delta = zacDelta->GetReal(0.0001);
     
    // Make and setup the algorithm object...
      sfs::Zheng alg;
      alg.SetImage(l);
      alg.SetAlbedo(a);
      alg.SetLight(toLight);
      alg.SetParas(mu,iters,delta);
      
    // Run...
     alg.Run(cyclops.BeginProg());
     cyclops.EndProg();
    
    // Extract the result...
     alg.GetNeedle(needle);
   }
   break;*/
   case 0: // Lee & Kuo...
   {
    // Get algorithm specific parameters...
     nat32 outerIters = lakOuterIters->GetInt(16);
     nat32 innerIters = lakInnerIters->GetInt(64);
     real32 tolerance = lakTolerance->GetReal(0.001);
     real32 initRad = lakInitRad->GetReal(32.0);
     real32 startLambda = lakStartLambda->GetReal(0.1);
     real32 endLambda = lakEndLambda->GetReal(0.1);
     real32 speed = lakSpeed->GetReal(0.5);
     
    // Make and setup the algorithm object...
      sfs::Lee alg;
      alg.SetImage(l);
      alg.SetAlbedo(a);
      alg.SetLight(toLight);
      alg.SetParas(outerIters,initRad,startLambda,endLambda,innerIters,tolerance,speed);
      
    // Run...
     alg.Run(cyclops.BeginProg());
     cyclops.EndProg();
    
    // Extract the result...
     alg.GetNeedle(needle);
   }
   break;
   case 1: // Worthington & Hancock...
   {
    // Get algorithm specific parameters...
     nat32 iters = wahIters->GetInt(200);
     
    // Make and setup the algorithm object...
      sfs::Worthington alg;
      alg.SetImage(l);
      alg.SetAlbedo(a);
      alg.SetLight(toLight);
      alg.SetIters(iters);
      
    // Run...
     alg.Run(cyclops.BeginProg());
     cyclops.EndProg();
    
    // Extract the result...
     alg.GetNeedle(needle);
   }
   break;
   case 2: // Haines & Wilson #1...
   {
    // Get algorithm specific parameters...
     real32 blur = hawBlur->GetReal(math::Sqrt(2.0));
     nat32 length = hawLength->GetInt(8);
     real32 exp = hawExp->GetReal(12.0);
     real32 stopChance = hawStopChance->GetReal(0.0);
     real32 simK = hawSimK->GetReal(4.5);
     real32 coneK = hawConeK->GetReal(16.0);
     real32 fadeTo = hawFadeTo->GetReal(0.0);
     nat32 iters = hawIters->GetInt(8);
     real32 gradDisc = hawGradDisc->GetReal(4.0);
     real32 gradBias = hawGradBias->GetReal(4.0);
     real32 borderK = hawBorderK->GetReal(0.0);
     bit project = hawProject->Ticked();
     
    // Make and setup the algorithm object...
      sfs::SfS_BP_Nice alg;
      alg.SetImage(l);
      alg.SetAlbedo(a);
      alg.SetLight(toLight);
      alg.SetParasGrad(blur,length,exp,stopChance);
      alg.SetParasCore(simK,coneK,fadeTo,iters);
      alg.SetParasExtra(gradDisc,gradBias,borderK,project);
      
    // Run...
     alg.Run(cyclops.BeginProg());
     cyclops.EndProg();
    
    // Extract the result...
     alg.GetNeedle(needle);
   }
   break;
   case 3: // Haines & Wilson #2...
   {
    // Get algorithm specific parameters...
     real32 simK = haw2SimK->GetReal(4.5);
     real32 coneK = haw2ConeK->GetReal(16.0);
     real32 fadeTo = haw2FadeTo->GetReal(1.0);
     nat32 iters = haw2Iters->GetInt(8);
     real32 angMult = haw2AngMult->GetReal(1.0);
     real32 momentum = haw2Momentum->GetReal(0.01);
     real32 boundK = haw2BoundK->GetReal(0.0);
     nat32 boundLength = haw2BoundLength->GetInt(8);
     real32 boundExp = haw2BoundExp->GetReal(6.0);
     real32 gradK = haw2GradK->GetReal(2.0);
     nat32 gradLength = haw2GradLength->GetInt(8);
     real32 gradExp = haw2GradExp->GetReal(6.0);
     
    // Make and setup the algorithm object...
     sfs::SfS_BP_Nice2 alg;
      alg.SetImage(l);
      alg.SetAlbedo(a);
      alg.SetLight(toLight);
      alg.SetParasCore(simK,coneK,fadeTo,iters);
      alg.SetParasBound(boundK,boundLength,boundExp);
      alg.SetParasGrad(gradK,gradLength,gradExp);
      alg.SetParasExtract(angMult,momentum);
      
    // Run...
     alg.Run(cyclops.BeginProg());
     cyclops.EndProg();
    
    // Extract the result...
     alg.GetNeedle(needle);
   }
   break;
   case 4: // Haines & Wilson #3...
   {
    // Get algorithm specific parameters...
     real32 smoothChance = haw3SmoothChance->GetReal(0.02);
     real32 smoothBase = haw3SmoothBase->GetReal(2.0);
     real32 smoothMult = haw3SmoothMult->GetReal(0.5);
     real32 smoothMinK = haw3SmoothMinK->GetReal(0.1);
     real32 smoothMaxK = haw3SmoothMaxK->GetReal(8.0);
     real32 cone0 = haw3Cone0->GetReal(24.0);
     real32 cone45 = haw3Cone45->GetReal(32.0);
     real32 cone90 = haw3Cone90->GetReal(24.0);
     nat32 iters = haw3Iters->GetInt(10);
     real32 boundK = haw3BoundK->GetReal(16.0);
     nat32 boundLength = haw3BoundLength->GetInt(8);
     real32 boundExp = haw3BoundExp->GetReal(6.0);
     real32 gradK = haw3GradK->GetReal(2.0);
     nat32 gradLength = haw3GradLength->GetInt(8);
     real32 gradExp = haw3GradExp->GetReal(6.0);
     real32 angMult = haw3AngMult->GetReal(1.0);
     real32 momentum = haw3Momentum->GetReal(0.01);
    
    // Make and setup the algorithm object...
     sfs::SfS_BP_Nice3 alg;
      alg.SetImage(l);
      alg.SetAlbedo(a);
      alg.SetLight(toLight);
      alg.SetSmooth(smoothChance,smoothBase,smoothMult,smoothMinK,smoothMaxK);
      alg.SetCone(cone0,cone45,cone90);
      alg.SetIters(iters);
      alg.SetBound(boundK,boundLength,boundExp);
      alg.SetGrad(gradK,gradLength,gradExp);
      alg.SetExtract(angMult,momentum);
      
    // Run...
     alg.Run(cyclops.BeginProg());
     cyclops.EndProg();
    
    // Extract the result...
     alg.GetNeedle(needle);
   }
   break;  
  }      


 // Update the visualisation...
  hasRun = true;
  UpdateView();
}

void SfS::SaveNeedle(gui::Base * obj,gui::Event * event)
{
 if (hasRun==false)
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"Algorithm has not been run!");
 }
 else
 {
  // Ask the user for a filename...
   str::String fn("*.bmp");
   if (cyclops.App().SaveFileDialog("Save Needle Map",fn))
   {
    // Create image...
     svt::Var civ(cyclops.Core());
     civ.Setup2D(image.Size(0),image.Size(1));
     bs::ColourRGB iniRGB(0.0,0.0,0.0);
     civ.Add("rgb",iniRGB);
     civ.Commit();
     svt::Field<bs::ColourRGB> ci(&civ,"rgb");
     
    // Fill image...
     for (nat32 y=0;y<ci.Size(1);y++)
     {
      for (nat32 x=0;x<ci.Size(0);x++)
      {
       if (needle.Get(x,y).Length()>0.1)
       {
        ci.Get(x,y).r = (needle.Get(x,y)[0]+1.0)*0.5;
        ci.Get(x,y).g = (needle.Get(x,y)[1]+1.0)*0.5;
        ci.Get(x,y).b = math::Max(needle.Get(x,y)[2],real32(0.0));
       }
       else
       {
        ci.Get(x,y).r = 0.0;
        ci.Get(x,y).g = 0.0;
        ci.Get(x,y).b = 0.0;
       }
      }
     }
    
    // Save image...
     if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
     cstr ts = fn.ToStr();
     if (!filter::SaveImage(ci,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving needle map image.");
     }
     mem::Free(ts);
   }
 }
}

void SfS::ChangeAlg(gui::Base * obj,gui::Event * event)
{
 switch (whichAlg->Get())
 {
  /*case 0:
   alg1->Visible(true);
   alg2->Visible(false);
   alg3->Visible(false);
   alg4->Visible(false);
   alg5->Visible(false);
   alg6->Visible(false);
  break;*/
  case 0:
   alg1->Visible(false);
   alg2->Visible(true);
   alg3->Visible(false);
   alg4->Visible(false);
   alg5->Visible(false);
   alg6->Visible(false);
  break;
  case 1:
   alg1->Visible(false);
   alg2->Visible(false);
   alg3->Visible(true);
   alg4->Visible(false);
   alg5->Visible(false);
   alg6->Visible(false);
  break;
  case 2:
   alg1->Visible(false);
   alg2->Visible(false);
   alg3->Visible(false);
   alg4->Visible(true);
   alg5->Visible(false);
   alg6->Visible(false);
  break;
  case 3:
   alg1->Visible(false);
   alg2->Visible(false);
   alg3->Visible(false);
   alg4->Visible(false);
   alg5->Visible(true);
   alg6->Visible(false);
  break;
  case 4:
   alg1->Visible(false);
   alg2->Visible(false);
   alg3->Visible(false);
   alg4->Visible(false);
   alg5->Visible(false);
   alg6->Visible(true);
  break;   
 }
}

void SfS::UpdateView()
{
 // Check size...
  if ((visible.Size(0)!=image.Size(0))||(visible.Size(1)!=image.Size(1)))
  {
   visVar->Setup2D(image.Size(0),image.Size(1));
   visVar->Commit();
   visVar->ByName("rgb",visible);
   
   canvas->SetSize(image.Size(0),image.Size(1));
  }


 // Iterate the pixels, and fill them in...
 // Either rendering a needle map or an image.
  if (hasRun)
  {
   for (nat32 y=0;y<visible.Size(1);y++)
   {
    for (nat32 x=0;x<visible.Size(0);x++)
    {
     if (needle.Get(x,y).Length()>0.1)
     {
      visible.Get(x,y).r = (byte)math::Clamp(((needle.Get(x,y)[0]+1.0)*0.5)*255.0,0.0,255.0);
      visible.Get(x,y).g = (byte)math::Clamp(((needle.Get(x,y)[1]+1.0)*0.5)*255.0,0.0,255.0);
      visible.Get(x,y).b = (byte)math::Clamp(needle.Get(x,y)[2]*255.0,0.0,255.0);
     }
     else
     {
      visible.Get(x,y).r = 0;
      visible.Get(x,y).g = 0;
      visible.Get(x,y).b = 0;
     }
    }
   }
  }
  else
  {
   for (nat32 y=0;y<visible.Size(1);y++)
   {
    for (nat32 x=0;x<visible.Size(0);x++)
    {
     real32 v = (image.Get(x,y).r + image.Get(x,y).g + image.Get(x,y).b)/3.0;
     byte vb = (byte)math::Clamp(v*255.0,0.0,255.0);
     visible.Get(x,y).r = vb;
     visible.Get(x,y).g = vb;
     visible.Get(x,y).b = vb;
    }
   }
  }
  
 
 // Redraw...
  canvas->Redraw();
}

//------------------------------------------------------------------------------
