//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "sfgs/main.h"

using namespace eos;

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 // Create basic objects...
  os::Con conObj;
  os::Conversation & con = *conObj.StartConversation();

  str::TokenTable tt;
  svt::Core core(tt);



 // Check we have enough parameters.
  if (argc!=5)
  {
   con << "Usage: sfgs [left.bmp] [right.bmp] [calibration.pair] [config.xml]\n";
   con << "[left.bmp] and [right.bmp] are two rectified images.\n";
   con << "[calibration.pair] is the calibration for the rectified pair.\n";
   con << "[config.xml] is an xml file that contains all the parameters.\n";
   return 1;
  }


 // Load the left and right images...
  svt::Var * left = filter::LoadImageRGB(core,argv[1]);
  if (left==null<svt::Var*>())
  {
   con << "Unable to load left image.\n";
   return 1;
  }

  svt::Var * right = filter::LoadImageRGB(core,argv[2]);
  if (right==null<svt::Var*>())
  {
   delete left;
   con << "Unable to load right image.\n";
   return 1;
  }
 
  svt::Node * root = new svt::Node(core);
  left->AttachParent(root);
  right->AttachParent(root);


 // Load the camera calibration, this gives us full capability to
 // triangulate etc coordinates from the rectified input...
  cam::CameraPair pair;
  if (pair.Load(argv[3])==false)
  {
   delete root;
   con << "Unable to load camera configuration.\n";
   return 1;
  }


 // Create the algorithm object, load the parameters file and fill them all in,
 // with defaults as needed...
  stereo::Sfgs1 sfgs;
  bs::Element * paras = file::LoadXML(tt,argv[4]);
  if (paras==null<bs::Element*>())
  {
   delete root;
   con << "Unable to load algortihm configuration.\n";
   return 1;   
  }

  int32 minDisp = paras->GrabInt(":disparity.min",-30);
  int32 maxDisp = paras->GrabInt(":disparity.max",30);
  nat32 scale = paras->GrabInt(":disparity.scale",255/math::Max(math::Abs(minDisp),math::Abs(maxDisp)));
  sfgs.SetDispRange(minDisp,maxDisp);
 
  sfgs.SetIterCount(paras->GrabInt(":iterations.value",3));
  
  bs::Normal leftLight;
  bs::Normal rightLight;
  if (paras->Grab(":light.to")!=null<bs::Attribute*>())
  {
   // Get the light postion and centre of scene position, relative to the left camera...
    str::String defTo("(0,0,0,1)");
    str::String defCentre("(0,0,-1,1)");
   
    math::Vect<4,real64> to;
    math::Vect<4,real64> centre;
   
    str::String s = paras->GrabString(":light.to",defTo);
    str::String::Cursor targ = s.GetCursor();
    targ >> to;

    s = paras->GrabString(":light.centre",defCentre);
    targ = s.GetCursor();
    targ >> centre;

  
   // Convert to normals, this involves converting to the right cameras coordinate 
   // space from the left...
    // First do the left...
     to /= to[3];
     centre /= centre[3];
     leftLight[0] = to[0] - centre[0];
     leftLight[1] = to[1] - centre[1];
     leftLight[2] = to[2] - centre[2];

    // Go from the left cameras local coordinates to the right cameras local coordinates via
    // world  coordinates...
     math::Mat<4,4,real64> fromLeft;
     pair.lp.ToWorld(fromLeft);
     math::Mat<4,4,real64> toRight;
     pair.rp.ToLocal(toRight);
     math::Mat<4,4,real64> tra;
     math::Mult(toRight,fromLeft,tra);

     math::Vect<4,real64> temp;
     math::MultVect(tra,to,temp); to = temp;
     math::MultVect(tra,centre,temp); centre = temp;

    // Do the right...
     to /= to[3];
     centre /= centre[3];
     rightLight[0] = to[0] - centre[0];
     rightLight[1] = to[1] - centre[1];
     rightLight[2] = to[2] - centre[2];
  }
  else
  {
   str::String def("(0,0,1)");

   str::String s = paras->GrabString(":light.left",def);
   str::String::Cursor targ = s.GetCursor();
   targ >> leftLight;

   s = paras->GrabString(":light.right",def);
   targ = s.GetCursor();
   targ >> rightLight;
  }
  leftLight.Normalise();
  rightLight.Normalise();
  LogDebug("[exe.sfgs] Light Vectors {left,right}" << LogDiv() << leftLight << LogDiv() << rightLight);
  sfgs.SetLight(leftLight,rightLight);


  sfgs.SetMatchParas(paras->GrabReal(":shape:match:alpha.mult",0.1),
                     paras->GrabReal(":shape:match:alpha.max",1.0),
                     paras->GrabReal(":shape:match:beta.mult",0.1),
                     paras->GrabReal(":shape:match:beta.max",1.0),
                     paras->GrabReal(":shape:match:gamma.mult",0.05),
                     paras->GrabReal(":shape:match:gamma.max",1.0),
                     paras->GrabReal(":shape:match:delta.mult",0.0),
                     paras->GrabReal(":shape:match:delta.max",0.0),
                     paras->GrabReal(":shape:match.out-of-bound",0.5),
                     paras->GrabReal(":shape:match.max",2.0));
                    
  sfgs.SetDiffParas(paras->GrabReal(":shape:diff.error",0.1));
  
  sfgs.SetSmoothParas(paras->GrabReal(":shape:smooth.mult",1.0),
                      paras->GrabReal(":shape:smooth.max",2.0));
  if (paras->GrabBit(":shape:smooth.override",false)) sfgs.EnableSmoothOverride();
  if (paras->GrabBit(":shape:a-smooth.active",false))
  {
   sfgs.EnableAlbedoSmoothOverride(paras->GrabReal(":shape:a-smooth.cost",1.0),
                                   paras->GrabInt(":shape:a-smooth.angRes",45),
                                   paras->GrabBit(":shape:a-smooth.brute",false),
                                   paras->GrabInt(":shape:a-smooth.irrRes",255));
  }
  
  if (paras->GrabBit(":shape:o-smooth.active",false))
  {
   sfgs.EnableSmoothOrient(paras->GrabInt(":shape:o-smooth.subDivs",2),
                           paras->GrabReal(":shape:o-smooth.maxCost",1.0),
                           paras->GrabReal(":shape:o-smooth.angMult",1.0),
                           paras->GrabReal(":shape:o-smooth.eqCost",0.5));
  }
  
  if (paras->GrabBit(":shape:o-smooth.sfs",false))
  {
   sfgs.EnableSmoothOrientEx(paras->GrabReal(":shape:o-smooth.angAlias",0.25),
                             paras->GrabReal(":shape:o-smooth.angMaxCost",1.0),
                             paras->GrabInt(":shape:o-smooth.angRes",90));
  }
  

               
  sfgs.SetAlbedoLabelCount(paras->GrabInt(":albedo.labels",100));

  sfgs.SetAlbedoParas(paras->GrabReal(":albedo:sfs.sd",3.0),
                      paras->GrabReal(":albedo:sfs.min",0.2),
                      paras->GrabReal(":albedo:sfs.angle",math::pi*0.1));

  sfgs.SetNeighbourParas(paras->GrabReal(":albedo:neighbour.min",1.1),
                         paras->GrabReal(":albedo:neighbour.max",2.0),
                         paras->GrabReal(":albedo:neighbour.alpha",0.5),
                         paras->GrabReal(":albedo:neighbour.beta",0.5));

  sfgs.SetNeedleSmooth(paras->GrabReal(":albedo:smooth.sd",0.0));

 // Add luv, luminence, albedo, disparity, needle, confidence, dx and dy maps to both images...
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  bs::ColourL lIni(0.0);
  real32 realIni = 0.0;
  bs::Normal normIni(0.0,0.0,1.0);

  left->Add("luv",luvIni);
  left->Add("l",lIni);
  left->Add("albedo",realIni);
  left->Add("needle",normIni);
  left->Add("disp",realIni);
  left->Add("dispDx",realIni);
  left->Add("dispDy",realIni);
  left->Add("confidence",realIni);
  left->Commit();

  right->Add("luv",luvIni);
  right->Add("l",lIni);  
  right->Add("albedo",realIni);
  right->Add("needle",normIni);
  right->Add("disp",realIni);
  right->Add("dispDx",realIni);
  right->Add("dispDy",realIni);
  right->Add("confidence",realIni);
  right->Commit();


 // Obtain field objects to represent all of the above...
  svt::Field<bs::ColourRGB> leftRGB(left,"rgb");
  svt::Field<bs::ColourL> leftL(left,"l");  
  svt::Field<bs::ColourLuv> leftLuv(left,"luv");
  svt::Field<real32> leftAlbedo(left,"albedo");
  svt::Field<bs::Normal> leftNeedle(left,"needle");
  svt::Field<real32> leftDisp(left,"disp");
  svt::Field<real32> leftDispDx(left,"dispDx");
  svt::Field<real32> leftDispDy(left,"dispDy");
  svt::Field<real32> leftConfidence(left,"confidence");

  svt::Field<bs::ColourRGB> rightRGB(right,"rgb");
  svt::Field<bs::ColourL> rightL(right,"l");    
  svt::Field<bs::ColourLuv> rightLuv(right,"luv");
  svt::Field<real32> rightAlbedo(right,"albedo");
  svt::Field<bs::Normal> rightNeedle(left,"needle");
  svt::Field<real32> rightDisp(right,"disp");
  svt::Field<real32> rightDispDx(right,"dispDx");
  svt::Field<real32> rightDispDy(right,"dispDy");
  svt::Field<real32> rightConfidence(right,"confidence");


 // Convert the loaded rgb images into luv...
  filter::RGBtoLuv(left);
  filter::RGBtoLuv(right);


 // Set all the field values for the algorithm...
  sfgs.SetIr(leftLuv,rightLuv);
  sfgs.SetAlbedo(leftAlbedo,rightAlbedo);
  sfgs.SetDisp(leftDisp,rightDisp);
  sfgs.SetNeedle(leftNeedle,rightNeedle);
  sfgs.SetConfidence(leftConfidence,rightConfidence);
  sfgs.SetDispDx(leftDispDx,rightDispDx);
  sfgs.SetDispDy(leftDispDy,rightDispDy);


 // Setup the disparity to surface orientation conversion...
  cam::CameraPair swapPair = pair; swapPair.Swap();
  
  cam::DispConvPlane * leftDC = new cam::DispConvPlane();
  leftDC->Set(pair,leftLuv.Size(0),leftLuv.Size(1),rightLuv.Size(0),rightLuv.Size(1));
  
  cam::DispConvPlane * rightDC = new cam::DispConvPlane();
  rightDC->Set(swapPair,rightLuv.Size(0),rightLuv.Size(1),leftLuv.Size(0),leftLuv.Size(1));
  
  sfgs.SetNeedleCreator(leftDC,rightDC);



 // Run...
  sfgs.Run(&con.BeginProg());
  con.EndProg();



 // Output albedo, disparity, needle, disparity dy and dx as well as confidence maps
 // for both images...
  // Albedo...
   for (nat32 y=0;y<leftLuv.Size(1);y++)
   {
    for (nat32 x=0;x<leftLuv.Size(0);x++) leftL.Get(x,y) = leftAlbedo.Get(x,y) / 100.0;
   }
   filter::LtoRGB(left);  
   if (filter::SaveImageRGB(left,"left_albedo.bmp",true)==false)
   {
    con << "Failed to save left_albedo.bmp\n";
   }

   for (nat32 y=0;y<rightLuv.Size(1);y++)
   {
    for (nat32 x=0;x<rightLuv.Size(0);x++) rightL.Get(x,y) = rightAlbedo.Get(x,y) / 100.0;
   }
   filter::LtoRGB(right);  
   if (filter::SaveImageRGB(right,"right_albedo.bmp",true)==false)
   {
    con << "Failed to save right_albedo.bmp\n";
   }


  // Disparity...
   stereo::RenderDisp(leftDisp,leftL,scale/255.0);
   filter::LtoRGB(left);
   if (filter::SaveImageRGB(left,"left_disp.bmp",true)==false)
   {
    con << "Failed to save left_disp.bmp\n";
   }

   stereo::RenderDisp(rightDisp,rightL,scale/255.0);
   filter::LtoRGB(right);
   if (filter::SaveImageRGB(right,"right_disp.bmp",true)==false)
   {
    con << "Failed to save right_disp.bmp\n";
   }
   
  // Disparity as SVT...
   // Left...
   {
    svt::Var leftD(leftDisp);
     real32 iniDisp = 0.0;
     leftD.Add("disp",iniDisp);
    leftD.Commit();
    
    svt::Field<real32> d(&leftD,"disp");
    for (nat32 y=0;y<d.Size(1);y++)
    {
     for (nat32 x=0;x<d.Size(0);x++)
     {
      d.Get(x,y) = leftDisp.Get(x,y);
     }
    }
    
    if (svt::Save("left_disp.obf",&leftD,true)==false)
    {
     con << "Failed to save left_disp.obf\n";
    }
   }

   // Right...
   {
    svt::Var rightD(rightDisp);
     real32 iniDisp = 0.0;
     rightD.Add("disp",iniDisp);
    rightD.Commit();
    
    svt::Field<real32> d(&rightD,"disp");
    for (nat32 y=0;y<d.Size(1);y++)
    {
     for (nat32 x=0;x<d.Size(0);x++)
     {
      d.Get(x,y) = rightDisp.Get(x,y);
     }
    }
    
    if (svt::Save("right_disp.obf",&rightD,true)==false)
    {
     con << "Failed to save right_disp.obf\n";
    }
   }



  // Needle...
   rend::VisNeedleMap(leftNeedle,leftRGB);
   if (filter::SaveImageRGB(left,"left_needle.bmp",true)==false)
   {
    con << "Failed to save left_needle.bmp\n";
   }

   {
    file::Wavefront mod;
    rend::NeedleMapToModel(leftNeedle,mod);
    if (mod.Save("left_needle.obj",true)==false)
    {
     con << "Failed to save left_needle.obj\n";
    }
   }

   rend::VisNeedleMap(rightNeedle,rightRGB);
   if (filter::SaveImageRGB(right,"right_needle.bmp",true)==false)
   {
    con << "Failed to save right_needle.bmp\n";
   }
   
   {
    file::Wavefront mod;
    rend::NeedleMapToModel(rightNeedle,mod);
    if (mod.Save("right_needle.obj",true)==false)
    {
     con << "Failed to save right_needle.obj\n";
    }
   }


  // Find disparity differential range...
   real32 minDiff = 0.0;
   real32 maxDiff = 0.0;

   for (nat32 y=0;y<leftDispDx.Size(1);y++)
   {
    for (nat32 x=0;x<leftDispDx.Size(0);x++)
    {
     real32 val = leftDispDx.Get(x,y);
     minDiff = math::Min(minDiff,val);
     maxDiff = math::Max(maxDiff,val);
    }
   }

   for (nat32 y=0;y<leftDispDy.Size(1);y++)
   {
    for (nat32 x=0;x<leftDispDy.Size(0);x++)
    {
     real32 val = leftDispDy.Get(x,y);
     minDiff = math::Min(minDiff,val);
     maxDiff = math::Max(maxDiff,val);
    }
   }

   for (nat32 y=0;y<rightDispDx.Size(1);y++)
   {
    for (nat32 x=0;x<rightDispDx.Size(0);x++)
    {
     real32 val = rightDispDx.Get(x,y);
     minDiff = math::Min(minDiff,val);
     maxDiff = math::Max(maxDiff,val);
    }
   }

   for (nat32 y=0;y<rightDispDy.Size(1);y++)
   {
    for (nat32 x=0;x<rightDispDy.Size(0);x++)
    {
     real32 val = rightDispDy.Get(x,y);
     minDiff = math::Min(minDiff,val);
     maxDiff = math::Max(maxDiff,val);
    }
   }
   
   con << "Minimum disparity differential value = " << minDiff << "\n";
   con << "Maximum disparity differential value = " << maxDiff << "\n";

   real32 mult = maxDiff-minDiff;
   if (!math::Equal(real32(0.0),mult)) mult = 1.0/mult;

   
  // Disparity dx & dy...
   for (nat32 y=0;y<leftDispDx.Size(1);y++)
   {
    for (nat32 x=0;x<leftDispDx.Size(0);x++) leftL.Get(x,y) = (leftDispDx.Get(x,y) - minDiff)*mult;
   }
   filter::LtoRGB(left);
   if (filter::SaveImageRGB(left,"left_disp_dx.bmp",true)==false)
   {
    con << "Failed to save left_disp_dx.bmp\n";
   }
  
   for (nat32 y=0;y<leftDispDy.Size(1);y++)
   {
    for (nat32 x=0;x<leftDispDy.Size(0);x++) leftL.Get(x,y) = (leftDispDy.Get(x,y) - minDiff)*mult;
   }
   filter::LtoRGB(left);
   if (filter::SaveImageRGB(left,"left_disp_dy.bmp",true)==false)
   { 
    con << "Failed to save left_disp_dy.bmp\n";
   }
  
   for (nat32 y=0;y<rightDispDx.Size(1);y++)
   {
    for (nat32 x=0;x<rightDispDx.Size(0);x++) rightL.Get(x,y) = (rightDispDx.Get(x,y) - minDiff)*mult;
   }
   filter::LtoRGB(right);
   if (filter::SaveImageRGB(right,"right_disp_dx.bmp",true)==false)
   {
    con << "Failed to save right_disp_dx.bmp\n";
   }
  
   for (nat32 y=0;y<rightDispDy.Size(1);y++)
   {
    for (nat32 x=0;x<rightDispDy.Size(0);x++) rightL.Get(x,y) = (rightDispDy.Get(x,y) - minDiff)*mult;
   }
   filter::LtoRGB(right);
   if (filter::SaveImageRGB(right,"right_disp_dy.bmp",true)==false)
   {
    con << "Failed to save right_disp_dy.bmp\n";
   }  
 
 
  // Confidence...
   // Find maximum, average and average of ln confidence...
    real32 maxConf = 0.0001;
    real64 mean = 0.0;
    real64 meanLn = 0.0;
    for (nat32 y=0;y<leftConfidence.Size(1);y++)
    {
     for (nat32 x=0;x<leftConfidence.Size(0);x++)
     {
      maxConf = math::Max(leftConfidence.Get(x,y),maxConf);
      mean += leftConfidence.Get(x,y);
      meanLn += math::Ln(1.0 + leftConfidence.Get(x,y));
     }
    }

    for (nat32 y=0;y<rightConfidence.Size(1);y++)
    {
     for (nat32 x=0;x<rightConfidence.Size(0);x++)
     {
      maxConf = math::Max(rightConfidence.Get(x,y),maxConf);
      mean += rightConfidence.Get(x,y);
      meanLn += math::Ln(1.0 + rightConfidence.Get(x,y));      
     }
    }
   
    real64 n = leftConfidence.Size(0)*leftConfidence.Size(1) +
               rightConfidence.Size(0)*rightConfidence.Size(1);
    mean /= n;
    meanLn /= n;
    con << "Mean Confidence Value = " << mean << "\n";
    con << "Mean Ln of 1+Confidence value = " << meanLn << "\n";

    con << "Maximum Confidence Value = " << maxConf << "\n";
    maxConf = 1.0/maxConf;
    
   // Save left...
    for (nat32 y=0;y<leftConfidence.Size(1)-1;y++)
    {
     for (nat32 x=0;x<leftConfidence.Size(0)-1;x++) leftL.Get(x,y) = leftConfidence.Get(x,y) * maxConf;
    }
    filter::LtoRGB(left);
    if (filter::SaveImageRGB(left,"left_confidence.bmp",true)==false)
    {
     con << "Failed to save left_confidence.bmp\n";
    }    
   
   // Save right...
    for (nat32 y=0;y<rightConfidence.Size(1)-1;y++)
    {
     for (nat32 x=0;x<rightConfidence.Size(0)-1;x++) rightL.Get(x,y) = rightConfidence.Get(x,y) * maxConf;
    }
    filter::LtoRGB(right);
    if (filter::SaveImageRGB(right,"right_confidence.bmp",true)==false)
    {
     con << "Failed to save right_confidence.bmp\n";
    }



 // Save each image warped to the other...
  filter::LuvtoRGB(left);
  stereo::ForwardWarp(leftRGB,leftDisp,rightRGB);
  if (filter::SaveImageRGB(right,"left_warp.bmp",true)==false)
  {
   con << "Failed to save left_warp.bmp\n";
  }


  filter::LuvtoRGB(right);
  stereo::ForwardWarp(rightRGB,rightDisp,leftRGB);
  if (filter::SaveImageRGB(left,"right_warp.bmp",true)==false)
  {
   con << "Failed to save right_warp.bmp\n";
  }



 // Remove unnecesary fields then save a svt file...
  // Remove...
   left->Rem("rgb");
   left->Rem("l");
   left->Commit();
   
   right->Rem("rgb");
   right->Rem("l");      
   right->Commit();
  
  // Save...
   svt::Save("sfgs.svt",root,true);



 // Clean up...
  delete paras;
  delete root;

 return 0;
}

//------------------------------------------------------------------------------
