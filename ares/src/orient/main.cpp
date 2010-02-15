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


#include "orient/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;

 // Create basic objects...
  os::Con conObj;
  os::Conversation & con = *conObj.StartConversation();

  str::TokenTable tt;
  svt::Core core(tt);



 // Check we have enough parameters.
  if (argc!=5)
  {
   con << "Usage: orient [left.bmp] [right.bmp] [calibration.pair] [config.xml]\n";
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
  stereo::OrientStereo orient;
  bs::Element * paras = file::LoadXML(tt,argv[4]);
  if (paras==null<bs::Element*>())
  {
   delete root;
   con << "Unable to load algortihm configuration.\n";
   return 1;   
  }

  orient.Set(paras->GrabInt(":disparity.min",-30),
             paras->GrabInt(":disparity.max",30),
             paras->GrabInt(":resolution.albedo",100),
             paras->GrabInt(":resolution.angle",90));
 
  orient.Set(paras->GrabReal(":paras:alpha.k",0.5),
             paras->GrabReal(":paras:alpha.err",0.25),
             paras->GrabReal(":paras:beta.value",1),
             paras->GrabReal(":paras:gamma.mult",0.5),
             paras->GrabReal(":paras:gamma.max",1.0));
  
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
  LogDebug("[exe.orient] Light Vectors {left,right}" << LogDiv() << leftLight << LogDiv() << rightLight);
  orient.SetLight(leftLight,rightLight);



 // Add luv, luminence, albedo, disparity, needle, confidence, dx and dy maps to both images...
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  bs::ColourL lIni(0.0);
  bs::Normal normIni(0.0,0.0,1.0);

  left->Add("luv",luvIni);
  left->Add("albedo",lIni);
  left->Add("needle",normIni);
  left->Commit();

  right->Add("luv",luvIni);
  right->Add("albedo",lIni);
  right->Add("needle",normIni);
  right->Commit();


 // Obtain field objects to represent all of the above...
  svt::Field<bs::ColourRGB> leftRGB(left,"rgb");
  svt::Field<bs::ColourLuv> leftLuv(left,"luv");
  svt::Field<bs::ColourL> leftAlbedo(left,"albedo");
  svt::Field<bs::Normal> leftNeedle(left,"needle");

  svt::Field<bs::ColourRGB> rightRGB(right,"rgb");   
  svt::Field<bs::ColourLuv> rightLuv(right,"luv");
  svt::Field<bs::ColourL> rightAlbedo(right,"albedo");
  svt::Field<bs::Normal> rightNeedle(left,"needle");


 // Convert the loaded rgb images into luv...
  filter::RGBtoLuv(left);
  filter::RGBtoLuv(right);


 // Set all the field values for the algorithm...
  orient.SetIr(leftLuv,rightLuv);
  orient.SetAlbedo(leftAlbedo,rightAlbedo);
  orient.SetNeedle(leftNeedle,rightNeedle);


 // Setup the disparity to surface orientation conversion...
  cam::CameraPair swapPair = pair; swapPair.Swap();
  
  cam::DispConvPlane leftDC;
  leftDC.Set(pair,leftLuv.Size(0),leftLuv.Size(1),rightLuv.Size(0),rightLuv.Size(1));
  
  cam::DispConvPlane rightDC;
  rightDC.Set(swapPair,rightLuv.Size(0),rightLuv.Size(1),leftLuv.Size(0),leftLuv.Size(1));
  
  orient.SetDispConv(leftDC,rightDC);



 // Run...
  orient.Run(&con.BeginProg());
  con.EndProg();



 // Output stuff...
  // Albedo...
   filter::LtoRGB(leftAlbedo,leftRGB);  
   if (filter::SaveImageRGB(left,"left_albedo.bmp",true)==false)
   {
    con << "Failed to save left_albedo.bmp\n";
   }

   filter::LtoRGB(rightAlbedo,rightRGB);  
   if (filter::SaveImageRGB(right,"right_albedo.bmp",true)==false)
   {
    con << "Failed to save right_albedo.bmp\n";
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


 // Remove unnecesary fields then save a svt file...
  // Remove...
   left->Rem("rgb");
   left->Commit();
   
   right->Rem("rgb");
   right->Commit();
  
  // Save...
   svt::Save("orient.svt",root,true);



 // Clean up...
  delete paras;
  delete root;

 return 0;
}

//------------------------------------------------------------------------------
