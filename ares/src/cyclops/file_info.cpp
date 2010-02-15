//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/file_info.h"

//------------------------------------------------------------------------------
FileInfo::FileInfo(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>())
{
 // Build the gui control panel...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));

  win->SetTitle("File Info");
  win->SetSize(200,300);

  gui::Vertical * vert = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
  win->SetChild(vert);


  gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
  gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

  but1->SetChild(lab1); lab1->Set("Load...");

  vert->AttachBottom(but1,false);

  gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
  vert->AttachBottom(panel);

  ml = static_cast<gui::Multiline*>(cyclops.Fact().Make("Multiline"));
  panel->SetChild(ml);
  ml->Edit(false);

  cyclops.App().Attach(win);


 // Add handlers to it all, and were done - the rest just takes care of itself...
  win->OnDeath(MakeCB(this,&FileInfo::Done));
  but1->OnClick(MakeCB(this,&FileInfo::Load));
}

FileInfo::~FileInfo()
{
 delete win;
}

void FileInfo::Done(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void FileInfo::Load(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select File","*.icd,*.pcc,*.cam",fn))
 {
  if (fn.EndsWith(".icd"))
  {
   cam::CameraCalibration camera;
   if (camera.Load(fn))
   {
    str::String s;

    s << "Image Dimensions:\n";
    s << " Width: " << camera.dim[0] << "\n";
    s << " Height: " << camera.dim[1] << "\n\n";

    s << "Focal Length Estimates:\n";
    real32 focalH = FocalLength35mmHoriz(camera.dim[0],camera.dim[1],camera.intrinsic,&camera.radial);
    real32 focalV = FocalLength35mmVert(camera.dim[0],camera.dim[1],camera.intrinsic,&camera.radial);
    real32 focalD = FocalLength35mmDiag(camera.dim[0],camera.dim[1],camera.intrinsic,&camera.radial);
    s << " Horizontal: " << focalH << "mm\n";
    s << " Vertical: " << focalV << "mm\n";
    s << " Diagonal: " << focalD << "mm\n\n";

    s << "Intrinsic:\n";
    s << " Focal X = " << camera.intrinsic.FocalX() << "\n";
    s << " Focal Y = " << camera.intrinsic.FocalY() << "\n";
    s << " Principal X = " << camera.intrinsic.PrincipalX() << "\n";
    s << " Principal Y = " << camera.intrinsic.PrincipalY() << "\n";
    s << " Skew = " << camera.intrinsic.Skew() << "\n";
    s << " Aspect Ratio = " << camera.intrinsic.AspectRatio() << "\n\n";

    s << "Radial:\n";
    s << " Aspect Ratio = " << camera.radial.aspectRatio << "\n";
    s << " Centre X = " << camera.radial.centre[0] << "\n";
    s << " Centre Y = " << camera.radial.centre[1] << "\n";
    s << " d factor = " << camera.radial.k[0] << "\n";
    s << " d*d factor = " << camera.radial.k[1] << "\n";
    s << " d*d*d factor = " << camera.radial.k[2] << "\n";
    s << " d*d*d*d factor = " << camera.radial.k[3] << "\n";

    ml->Empty();
    ml->Append(s);
   }
   else
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load intrinsic parameters file");
   }
  }
  else
  {
   if (fn.EndsWith(".pcc"))
   {
    cam::CameraPair pair;
    if (pair.Load(fn))
    {
     str::String s;

     s << "Left Camera Intrinsic:\n";
     s << " Image Dimensions:\n";
     s << "  Width: " << pair.left.dim[0] << "\n";
     s << "  Height: " << pair.left.dim[1] << "\n\n";

     s << " Focal Length Estimates:\n";
     real32 focalH = FocalLength35mmHoriz(pair.left.dim[0],pair.left.dim[1],
                                          pair.left.intrinsic,&pair.left.radial);
     real32 focalV = FocalLength35mmVert(pair.left.dim[0],pair.left.dim[1],
                                         pair.left.intrinsic,&pair.left.radial);
     real32 focalD = FocalLength35mmDiag(pair.left.dim[0],pair.left.dim[1],
                                         pair.left.intrinsic,&pair.left.radial);
     s << "  Horizontal: " << focalH << "mm\n";
     s << "  Vertical: " << focalV << "mm\n";
     s << "  Diagonal: " << focalD << "mm\n\n";

     s << " Intrinsic:\n";
     s << "  Focal X = " << pair.left.intrinsic.FocalX() << "\n";
     s << "  Focal Y = " << pair.left.intrinsic.FocalY() << "\n";
     s << "  Principal X = " << pair.left.intrinsic.PrincipalX() << "\n";
     s << "  Principal Y = " << pair.left.intrinsic.PrincipalY() << "\n";
     s << "  Skew = " << pair.left.intrinsic.Skew() << "\n";
     s << "  Aspect Ratio = " << pair.left.intrinsic.AspectRatio() << "\n\n";

     s << " Radial:\n";
     s << "  Aspect Ratio = " << pair.left.radial.aspectRatio << "\n";
     s << "  Centre X = " << pair.left.radial.centre[0] << "\n";
     s << "  Centre Y = " << pair.left.radial.centre[1] << "\n";
     s << "  d factor = " << pair.left.radial.k[0] << "\n";
     s << "  d*d factor = " << pair.left.radial.k[1] << "\n";
     s << "  d*d*d factor = " << pair.left.radial.k[2] << "\n";
     s << "  d*d*d*d factor = " << pair.left.radial.k[3] << "\n\n\n";


     s << "Right Camera Intrinsic:\n";
     s << " Image Dimensions:\n";
     s << "  Width: " << pair.right.dim[0] << "\n";
     s << "  Height: " << pair.right.dim[1] << "\n\n";

     s << " Focal Length Estimates:\n";
     focalH = FocalLength35mmHoriz(pair.right.dim[0],pair.right.dim[1],
                                   pair.right.intrinsic,&pair.right.radial);
     focalV = FocalLength35mmVert(pair.right.dim[0],pair.right.dim[1],
                                  pair.right.intrinsic,&pair.right.radial);
     focalD = FocalLength35mmDiag(pair.right.dim[0],pair.right.dim[1],
                                  pair.right.intrinsic,&pair.right.radial);
     s << "  Horizontal: " << focalH << "mm\n";
     s << "  Vertical: " << focalV << "mm\n";
     s << "  Diagonal: " << focalD << "mm\n\n";

     s << " Intrinsic:\n";
     s << "  Focal X = " << pair.right.intrinsic.FocalX() << "\n";
     s << "  Focal Y = " << pair.right.intrinsic.FocalY() << "\n";
     s << "  Principal X = " << pair.right.intrinsic.PrincipalX() << "\n";
     s << "  Principal Y = " << pair.right.intrinsic.PrincipalY() << "\n";
     s << "  Skew = " << pair.right.intrinsic.Skew() << "\n";
     s << "  Aspect Ratio = " << pair.right.intrinsic.AspectRatio() << "\n\n";

     s << " Radial:\n";
     s << "  Aspect Ratio = " << pair.right.radial.aspectRatio << "\n";
     s << "  Centre X = " << pair.right.radial.centre[0] << "\n";
     s << "  Centre Y = " << pair.right.radial.centre[1] << "\n";
     s << "  d factor = " << pair.right.radial.k[0] << "\n";
     s << "  d*d factor = " << pair.right.radial.k[1] << "\n";
     s << "  d*d*d factor = " << pair.right.radial.k[2] << "\n";
     s << "  d*d*d*d factor = " << pair.right.radial.k[3] << "\n\n\n";


     s << "Left Camera Extrinsic:\n";
     cam::Intrinsic leftIntrinsic;
     math::Mat<3,3,real64> leftRot;
     math::Vect<4,real64> leftCentre;
     pair.lp.Decompose(leftIntrinsic,leftRot);
     pair.lp.Centre(leftCentre);

     leftCentre.Normalise();
     if (!math::IsZero(leftCentre[3])) leftCentre /= leftCentre[3];

     s << " Intrinsic Matrix:\n";
     s << "  [" << leftIntrinsic[0][0] << "," << leftIntrinsic[0][1] << "," << leftIntrinsic[0][2] << "]\n";
     s << "  [" << leftIntrinsic[1][0] << "," << leftIntrinsic[1][1] << "," << leftIntrinsic[1][2] << "]\n";
     s << "  [" << leftIntrinsic[2][0] << "," << leftIntrinsic[2][1] << "," << leftIntrinsic[2][2] << "]\n\n";

     s << " Rotation Matrix:\n";
     s << "  [" << leftRot[0][0] << "," << leftRot[0][1] << "," << leftRot[0][2] << "]\n";
     s << "  [" << leftRot[1][0] << "," << leftRot[1][1] << "," << leftRot[1][2] << "]\n";
     s << "  [" << leftRot[2][0] << "," << leftRot[2][1] << "," << leftRot[2][2] << "]\n\n";

     s << " Position = (" << leftCentre[0] << "," << leftCentre[1] << "," << leftCentre[2] << "," << leftCentre[3] << ")\n\n\n";


     s << "Right Camera Extrinsic:\n";
     cam::Intrinsic rightIntrinsic;
     math::Mat<3,3,real64> rightRot;
     math::Vect<4,real64> rightCentre;
     pair.rp.Decompose(rightIntrinsic,rightRot);
     pair.rp.Centre(rightCentre);

     rightCentre.Normalise();
     if (!math::IsZero(rightCentre[3])) rightCentre /= rightCentre[3];

     s << " Intrinsic Matrix (Extracted):\n";
     s << "  [" << rightIntrinsic[0][0] << "," << rightIntrinsic[0][1] << "," << rightIntrinsic[0][2] << "]\n";
     s << "  [" << rightIntrinsic[1][0] << "," << rightIntrinsic[1][1] << "," << rightIntrinsic[1][2] << "]\n";
     s << "  [" << rightIntrinsic[2][0] << "," << rightIntrinsic[2][1] << "," << rightIntrinsic[2][2] << "]\n\n";

     s << " Rotation Matrix:\n";
     s << "  [" << rightRot[0][0] << "," << rightRot[0][1] << "," << rightRot[0][2] << "]\n";
     s << "  [" << rightRot[1][0] << "," << rightRot[1][1] << "," << rightRot[1][2] << "]\n";
     s << "  [" << rightRot[2][0] << "," << rightRot[2][1] << "," << rightRot[2][2] << "]\n\n";

     s << " Position = (" << rightCentre[0] << "," << rightCentre[1] << "," << rightCentre[2] << "," << rightCentre[3] << ")\n\n\n";


     s << "Homography of Input:\n";
     s << " Left Inverse Rectification Matrix:\n";
     s << "  [" << pair.unRectLeft[0][0] << "," << pair.unRectLeft[0][1] << "," << pair.unRectLeft[0][2] << "]\n";
     s << "  [" << pair.unRectLeft[1][0] << "," << pair.unRectLeft[1][1] << "," << pair.unRectLeft[1][2] << "]\n";
     s << "  [" << pair.unRectLeft[2][0] << "," << pair.unRectLeft[2][1] << "," << pair.unRectLeft[2][2] << "]\n";
     s << " Left Dimensions = (" << pair.leftDim[0] << "," << pair.leftDim[1] << ")\n\n";
     s << " Right Inverse Rectification Matrix:\n";
     s << "  [" << pair.unRectRight[0][0] << "," << pair.unRectRight[0][1] << "," << pair.unRectRight[0][2] << "]\n";
     s << "  [" << pair.unRectRight[1][0] << "," << pair.unRectRight[1][1] << "," << pair.unRectRight[1][2] << "]\n";
     s << "  [" << pair.unRectRight[2][0] << "," << pair.unRectRight[2][1] << "," << pair.unRectRight[2][2] << "]\n";
     s << " Right Dimensions = (" << pair.rightDim[0] << "," << pair.rightDim[1] << ")\n\n\n";


     s << "Fundamental matrix:\n";
     s << " [" << pair.fun[0][0] << "," << pair.fun[0][1] << "," << pair.fun[0][2] << "]\n";
     s << " [" << pair.fun[1][0] << "," << pair.fun[1][1] << "," << pair.fun[1][2] << "]\n";
     s << " [" << pair.fun[2][0] << "," << pair.fun[2][1] << "," << pair.fun[2][2] << "]\n\n";
     if ((!math::IsZero(rightCentre[3]))&&(!math::IsZero(rightCentre[3])))
     {
      real64 gapSqr = math::Sqr(leftCentre[0]-rightCentre[0]) +
                      math::Sqr(leftCentre[1]-rightCentre[1]) +
                      math::Sqr(leftCentre[2]-rightCentre[2]);
      s << " Gap between cameras = " << math::Sqrt(gapSqr) << "\n\n";
     }

     {
      real64 a,b;
      pair.GetDispToDepth(a,b);
      s << "If rectified then: depth = " << a << "/(disparity + " << b << ")\n\n";
     }


     ml->Empty();
     ml->Append(s);
    }
    else
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pair file");
    }
   }
   else
   {
    if (fn.EndsWith(".cam"))
    {
     cam::CameraFull camera;
     if (camera.Load(fn))
     {
      str::String s;

      cam::Intrinsic intrinsic;
      math::Mat<3,3,real64> rot;
      math::Vect<4,real64> centre;
      camera.camera.Decompose(intrinsic,rot);
      camera.camera.Centre(centre);

      centre.Normalise();
      if (!math::IsZero(centre[3])) centre /= centre[3];


      s << "Image Dimensions:\n";
      s << " Width: " << camera.dim[0] << "\n";
      s << " Height: " << camera.dim[1] << "\n\n\n";


      s << "Camera Intrinsic:\n";
      s << " Focal Length Estimates:\n";
      real32 focalH = FocalLength35mmHoriz(camera.dim[0],camera.dim[1],
                                           intrinsic,&camera.radial);
      real32 focalV = FocalLength35mmVert(camera.dim[0],camera.dim[1],
                                          intrinsic,&camera.radial);
      real32 focalD = FocalLength35mmDiag(camera.dim[0],camera.dim[1],
                                          intrinsic,&camera.radial);
      s << "  Horizontal: " << focalH << "mm\n";
      s << "  Vertical: " << focalV << "mm\n";
      s << "  Diagonal: " << focalD << "mm\n\n";

      s << " Intrinsic:\n";
      s << "  Focal X = " << intrinsic.FocalX() << "\n";
      s << "  Focal Y = " << intrinsic.FocalY() << "\n";
      s << "  Principal X = " << intrinsic.PrincipalX() << "\n";
      s << "  Principal Y = " << intrinsic.PrincipalY() << "\n";
      s << "  Skew = " << intrinsic.Skew() << "\n";
      s << "  Aspect Ratio = " << intrinsic.AspectRatio() << "\n\n";

      s << " Radial:\n";
      s << "  Aspect Ratio = " << camera.radial.aspectRatio << "\n";
      s << "  Centre X = " << camera.radial.centre[0] << "\n";
      s << "  Centre Y = " << camera.radial.centre[1] << "\n";
      s << "  d factor = " << camera.radial.k[0] << "\n";
      s << "  d*d factor = " << camera.radial.k[1] << "\n";
      s << "  d*d*d factor = " << camera.radial.k[2] << "\n";
      s << "  d*d*d*d factor = " << camera.radial.k[3] << "\n\n\n";


      s << "Camera Extrinsic:\n";
      s << " Rotation Matrix:\n";
      s << "  [" << rot[0][0] << "," << rot[0][1] << "," << rot[0][2] << "]\n";
      s << "  [" << rot[1][0] << "," << rot[1][1] << "," << rot[1][2] << "]\n";
      s << "  [" << rot[2][0] << "," << rot[2][1] << "," << rot[2][2] << "]\n\n";

      s << " Position = (" << centre[0] << "," << centre[1] << "," << centre[2] << "," << centre[3] << ")";

      ml->Empty();
      ml->Append(s);
     }
     else
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load camera file");
     }
    }
    else
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Unrecognised file extension");
    }
   }
  }
 }
}

//------------------------------------------------------------------------------
