//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


#include "smes/main.h"

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
  if (argc!=6)
  {
   con << "Usage: smes [left.bmp] [right.bmp] [calibration.pair] [camera.crf] [config.xml]\n";
   con << "[left.bmp] and [right.bmp] are two rectified images.\n";
   con << "[calibration.pair] is the calibration for the rectified pair.\n";
   con << "[camera.crf] is the camera response function of the left image, applied prior to SfS.\n";
   con << "[config.xml] is an xml file that contains all the parameters.\n";
   return 1;
  }



 // Load the left and right images...
  // Left...
   svt::Var * left = filter::LoadImageRGB(core,argv[1]);
   if (left==null<svt::Var*>())
   {
    con << "Unable to load left image.\n";
    return 1;
   }

  // Right...
   svt::Var * right = filter::LoadImageRGB(core,argv[2]);
   if (right==null<svt::Var*>())
   {
    delete left;
    con << "Unable to load right image.\n";
    return 1;
   }

  // Create stack-based root node for automatic deletion...
   svt::Node root(core);
   left->AttachParent(&root);
   right->AttachParent(&root);



 // Load the camera calibration, this gives us full capability to
 // triangulate etc coordinates from the rectified input...
  cam::CameraPair pair;
  if (pair.Load(argv[3])==false)
  {
   con << "Unable to load camera configuration.\n";
   return 1;
  }



 // Transform pair so left camera at origin, changes nothing but allows certain
 // assumptions to be made...
  math::Mat<4,4,real64> toCS;
  pair.LeftToDefault(&toCS);



 // Load the camera response function...
  math::Func crf;
  if (crf.Load(argv[4])==false)
  {
   con << "Unable to load camera response function.\n";
   return 1;
  }



 // Load the configuration file...
  mem::StackPtr<bs::Element> paras = file::LoadXML(tt,argv[5]);
  if (paras.IsNull())
  {
   con << "Unable to load algortihm configuration.\n";
   return 1;
  }



 // Load in all the parameters...
  nat32 superIters = paras->GrabInt(":main.iters",1);


  struct
  {
   real32 highOccCost;
   real32 lowOccCost;
   real32 occLimMult;
   real32 matchCap;
   real32 occCap;
   nat32 iters;
   nat32 outCap;
  } stereo;

  stereo.highOccCost = paras->GrabReal(":stereo:hbp.highOccCost",16.0);
  stereo.lowOccCost = paras->GrabReal(":stereo:hbp.lowOccCost",8.0);
  stereo.occLimMult = paras->GrabReal(":stereo:hbp.occLimMult",2.0);
  stereo.matchCap = paras->GrabReal(":stereo:hbp.matchCap",36.0);
  stereo.occCap = paras->GrabReal(":stereo:hbp.occCap",6.0);
  stereo.iters = paras->GrabInt(":stereo:hbp.iters",6);
  stereo.outCap = paras->GrabInt(":stereo:hbp.outCap",1);


  struct
  {
   nat32 radius;
   real32 mix;
   real32 spatial;
   real32 range;
   real32 cutoff;
   nat32 minSeg;
   nat32 avgSteps;
   real32 maxMerge;
  } segment;

  segment.radius = paras->GrabInt(":segment:synergism.radius",2);
  segment.mix = paras->GrabReal(":segment:synergism.mix",0.3);
  segment.spatial = paras->GrabReal(":segment:synergism.spatial",7.0);
  segment.range = paras->GrabReal(":segment:synergism.range",4.5);
  segment.cutoff = paras->GrabReal(":segment:synergism.cutoff",2.25);
  segment.minSeg = paras->GrabInt(":segment:synergism.min_seg",20);
  segment.avgSteps = paras->GrabInt(":segment:synergism.avg_steps",2);
  segment.maxMerge = paras->GrabReal(":segment:synergism.max_merge",0.9);


  struct
  {
   nat32 iters;

   bit best_mean;

   real32 normInvSd;
   bit doImageMod;
   real32 imageModThreshold;
   real32 imageModHalflife;
  } integration;

  integration.iters = paras->GrabInt(":integration:iters.value",1000);

  integration.best_mean = paras->GrabBit(":integration:disp.best_mean",false);
  integration.normInvSd = paras->GrabReal(":integration:disp_diff.invSd",1.0);
  integration.doImageMod = paras->GrabBit(":integration:disp_diff.imageMod",false);
  integration.imageModThreshold = paras->GrabReal(":integration:disp_diff.threshold",32.0);
  integration.imageModHalflife = paras->GrabReal(":integration:disp_diff.halflife",2.0);


  struct
  {
   bit mode;
   bit refine;

   nat32 population;
   nat32 keptBest;
   nat32 immigrants;
   real32 mutationRate;
   nat32 generations;
   real32 albSd;
   real32 lightSd;

   real32 chance;
   real32 limit;
   nat32 cap;

   real32 cutoff;
   real32 halflife;
  } fitter;

  fitter.mode = paras->GrabString(":fitter.mode","ransac")=="ransac";
  fitter.refine = paras->GrabBit(":fitter.refine",true);

  fitter.population = paras->GrabInt(":fitter:size.population",100);
  fitter.generations = paras->GrabInt(":fitter:size.generations",250);
  fitter.keptBest = paras->GrabInt(":fitter:lifelines.keptBest",5);
  fitter.immigrants = paras->GrabInt(":fitter:lifelines.immigrants",5);
  fitter.mutationRate = paras->GrabReal(":fitter:mutation.rate",0.1);
  fitter.albSd = paras->GrabReal(":fitter:mutation.alb_sd",1.0);
  fitter.lightSd = paras->GrabReal(":fitter:mutation.light_sd",0.1);

  fitter.chance = paras->GrabReal(":fitter:lims.chance",0.99);
  fitter.limit = paras->GrabReal(":fitter:lims.limit",250.0);
  fitter.cap = paras->GrabInt(":fitter:lims.cap",10000);

  fitter.cutoff = paras->GrabReal(":fitter:fitness.cutoff",math::pi/6.0);
  fitter.halflife = paras->GrabReal(":fitter:fitness.halflife",math::pi/36.0);


  struct
  {
   real32 irrSd;
   real32 irrCap;
   real32 freedomCost;
  } mc;

  mc.irrSd = paras->GrabReal(":mc.irrSd",2.0);
  mc.irrCap = paras->GrabReal(":mc.irrCap",4.0);
  mc.freedomCost = paras->GrabReal(":mc.freedomCost",10.0);


  struct
  {
   bit enabled;
   nat32 radius;
   real32 radSD;
   real32 prob;
   real32 cutoff;
   nat32 maxSamples;
  } ppl;

  ppl.enabled = paras->GrabBit(":ppl.enabled",false);
  ppl.radius = paras->GrabInt(":ppl:window.radius",5);
  ppl.radSD = paras->GrabReal(":ppl:window.radSD",2.0);
  ppl.prob = paras->GrabReal(":ppl:ransac.prob",0.99);
  ppl.cutoff = paras->GrabReal(":ppl:ransac.cutoff",2.0);
  ppl.maxSamples = paras->GrabInt(":ppl:ransac.maxSamples",1000);


  struct
  {
   bit enabled;
   nat32 subDivs;
   real32 minAlb;
   real32 maxAlb;
   nat32 albDivs;
   real32 albSd;
   real32 cutoff;
   nat32 bestCap;
   real32 maxErr;
  } hough;

  hough.enabled = paras->GrabBit(":hough.enabled",false);
  hough.subDivs = paras->GrabInt(":hough:size.subDivs",7);
  hough.minAlb = paras->GrabReal(":hough:size.minAlb",0.0);
  hough.maxAlb = paras->GrabReal(":hough:size.maxAlb",200.0);
  hough.albDivs = paras->GrabInt(":hough:size.albDivs",200);
  hough.albSd = paras->GrabReal(":hough:blur.albSd",2.0);
  hough.cutoff = paras->GrabReal(":hough:blur.cutoff",2.0);
  hough.bestCap = paras->GrabInt(":hough:prune.bestCap",3);
  hough.maxErr = paras->GrabReal(":hough:prune.maxErr",2.0);
  
  
  struct
  {
   bit enabled;
   real32 noise;
   real32 adjProbMin;
   real32 adjProbMax;
   real32 adjProbThreshold;
   real32 adjProbHalflife;
   real32 minProb;
   nat32 iters;
  } modelSeg;

  modelSeg.enabled = paras->GrabBit(":modelSeg.enabled",false);
  modelSeg.noise = paras->GrabReal(":modelSeg:noise.base",1.0);
  modelSeg.adjProbMin       = paras->GrabReal(":modelSeg:neighbour.min",0.05);
  modelSeg.adjProbMax       = paras->GrabReal(":modelSeg:neighbour.max",0.5);
  modelSeg.adjProbThreshold = paras->GrabReal(":modelSeg:neighbour.threshold",5.0);
  modelSeg.adjProbHalflife  = paras->GrabReal(":modelSeg:neighbour.halflife",0.5);
  modelSeg.minProb = paras->GrabReal(":modelSeg:model.prob",0.2);
  modelSeg.iters = paras->GrabInt(":modelSeg.iters",10);



 // Augment the images with lots and lots of fields...
  bs::ColourL lIni(0.0);
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
  real32 realIni = 0.0;
  nat32 natIni = 0;
  bs::Normal normalIni(0.0,0.0,1.0);

  left->Add("l",lIni);
  left->Add("luv",luvIni);
  left->Add("model_image",rgbIni);
  left->Add("seg",natIni);
  left->Add("irr",realIni);
  left->Add("disp",realIni);
  left->Add("disp_smooth",realIni);
  left->Add("needle",normalIni);
  left->Add("needle_smooth",normalIni);
  left->Add("albedo",realIni);
  left->Add("light",normalIni);
  left->Commit();


  right->Add("luv",luvIni);
  right->Commit();



 // Obtain field objects to represent all of the above...
  svt::Field<bs::ColourRGB> leftRGB(left,"rgb");
  svt::Field<bs::ColourL> leftL(left,"l");
  svt::Field<bs::ColourLuv> leftLuv(left,"luv");

  svt::Field<bs::ColourRGB> leftModelImage(left,"model_image");

  svt::Field<nat32> leftSeg(left,"seg");

  svt::Field<real32> irr(left,"irr");

  svt::Field<real32> leftDisp(left,"disp");
  svt::Field<real32> leftDispSmooth(left,"disp_smooth");

  svt::Field<bs::Normal> leftNeedle(left,"needle");
  svt::Field<bs::Normal> leftNeedleSmooth(left,"needle_smooth");

  svt::Field<real32> leftAlbedo(left,"albedo");
  svt::Field<bs::Normal> leftLight(left,"light");


  svt::Field<bs::ColourRGB> rightRGB(right,"rgb");
  svt::Field<bs::ColourLuv> rightLuv(right,"luv");



 // Convert the loaded rgb images into l and luv...
  filter::RGBtoL(left);
  filter::RGBtoLuv(left);
  filter::RGBtoLuv(right);



 // Run...
  // Initialisation of data structures...
   // Disparity...
    stereo::HEBP sdsi;
    sdsi.Set(stereo.highOccCost,(stereo.lowOccCost-stereo.highOccCost)/stereo.occCap,
             stereo.occLimMult,stereo.iters,stereo.outCap);
    stereo::SqrBoundLuvDSC dsc(leftLuv,rightLuv,1.0,stereo.matchCap);
    sdsi.Set(&dsc);
    stereo::LuvDSC dscOcc(leftLuv,rightLuv,1.0,stereo.occCap);
    sdsi.SetOcc(&dscOcc);

    con << "Pre: Dsi generation...\n";
    sdsi.Run(&con.BeginProg());
    con.EndProg();

    sdsi.GetDisp(leftDisp);


   // Segmentation...
    filter::Synergism segy;
    segy.SetImage(leftL,leftLuv);
    segy.DiffWindow(segment.radius);
    segy.SetMix(segment.mix);
    segy.SetSpatial(segment.spatial);
    segy.SetRange(segment.range);
    segy.SetCutoff(segment.cutoff);
    segy.SetSegmentMin(segment.minSeg);
    segy.SetAverageSteps(segment.avgSteps);
    segy.SetMergeMax(segment.maxMerge);

    con << "Pre: Segmentation...\n";
    segy.Run(&con.BeginProg());
    con.EndProg();

    nat32 segments = segy.Segments();
    segy.GetSegments(leftSeg);
    con << "Obtained " << segments << " segments.\n";


   // Needle...
    for (nat32 y=0;y<leftNeedle.Size(1);y++)
    {
     for (nat32 x=0;x<leftNeedle.Size(0);x++) leftNeedle.Get(x,y) = bs::Normal(0.0,0.0,1.0);
    }


   // Irradiance map...
    for (nat32 y=0;y<irr.Size(1);y++)
    {
     for (nat32 x=0;x<irr.Size(0);x++)
     {
      bs::ColourRGB & col = leftRGB.Get(x,y);
      real64 val = (col.r + col.g + col.b)/3.0;
      irr.Get(x,y) = crf(val) * 100.0;
     }
    }


  // Save initialisation details as necesary...
   file::SaveReal(irr,"left_irradiance");
   file::SaveDisparity(leftDisp,"left_disp");
   file::SaveSeg(leftSeg,"left_seg");


  // Super iterations...
   for (nat32 i=0;i<superIters;i++)
   {
    // Create a disparity map smoothed by the current needle map...
    {
     stereo::GaussianDispSimple gds;
     gds.SetDSI(sdsi);
     gds.SetMode(integration.best_mean);

     stereo::GaussianDispDiffSimple gdds;
     gdds.SetPair(pair);
     gdds.SetNeedle(leftNeedle);
     gdds.SetInvSd(integration.normInvSd);
     if (integration.doImageMod)
     {
      gdds.SetImageMod(leftLuv,integration.imageModThreshold,integration.imageModHalflife);
     }

     stereo::GaussRefine gr;
     gr.Set(gds);
     gr.Set(gdds);
     gr.Set(integration.iters);

     con << "Pass " << i << ": Integration...\n";
     gr.Run(&con.BeginProg());
     con.EndProg();

     gr.DispMap(leftDispSmooth);
    }


    // Extract the smoothed needle map from the disparity map...
     pair.Convert(leftDispSmooth,leftNeedleSmooth);


    if (hough.enabled)
    {
     // Run the hougn transform to find candidate light models...
      sfs::LamHough lh;
      lh.Set(hough.subDivs,hough.minAlb,hough.maxAlb,hough.albDivs);
      lh.Set(hough.albSd,hough.cutoff);
      lh.Set(hough.bestCap,hough.maxErr);
      lh.Add(irr,leftNeedleSmooth,svt::Field<real32>(),leftSeg);

      con << "Pass " << i << ": Model Finding...\n";
      lh.Run(&con.BeginProg());
      con.EndProg();
      con << "Maximas accepted: " << lh.Maximas() << "\n";


     if (modelSeg.enabled)
     {
      // Use belief propagation to select the light model for each pixel as a segmentation process...
       sfs::LamSeg ls;
       ls.SetMaps(irr,leftNeedleSmooth);
       for (nat32 y=0;y<irr.Size(1);y++)
       {
        for (nat32 x=0;x<irr.Size(0);x++) ls.SetNoise(x,y,modelSeg.noise);
       }
       ls.Set(1.0,modelSeg.minProb,modelSeg.iters);
       ls.Set(leftLuv,modelSeg.adjProbMin,modelSeg.adjProbMax,
                      modelSeg.adjProbThreshold,modelSeg.adjProbHalflife);

       ls.SetModels(lh.Maximas());
       for (nat32 j=0;j<lh.Maximas();j++) ls.SetModel(j,lh.Maxima(j));
       
       
       con << "Pass " << i << ": Model Segmentation...\n";
       ls.Run(&con.BeginProg());
       con.EndProg();
     
     
       ls.GetSplit(leftAlbedo,leftLight);
     }
     else
     {
      // Fill in the light and albedo maps...
       con << "Pass " << i << ": Model Extraction...\n";
       time::Progress * prog = &con.BeginProg();
       for (nat32 y=0;y<leftLight.Size(1);y++)
       {
        prog->Report(y,leftLight.Size(1));
        for (nat32 x=0;x<leftLight.Size(0);x++)
        {
         // Find best fit model for pixel...
          leftLight.Get(x,y) = lh.Maxima(0);
          real32 bestScore = math::Abs(irr.Get(x,y) - (lh.Maxima(0) * leftNeedleSmooth.Get(x,y)))/lh.Weight(0);
          for (nat32 i=1;i<lh.Maximas();i++)
          {
           real32 score = math::Abs(irr.Get(x,y) - (lh.Maxima(i) * leftNeedleSmooth.Get(x,y)))/lh.Weight(i);
           if (score<bestScore)
           {
            bestScore = score;
            leftLight.Get(x,y) = lh.Maxima(i);
           }
          }

         // Split to light and albedo...
          leftAlbedo.Get(x,y) = leftLight.Get(x,y).Length();
          leftLight.Get(x,y) /= leftAlbedo.Get(x,y);
        }
       }
       con.EndProg();
      }
    }
    else
    {
     // Infer the shading model from the smoothed disparity map...
      if (!ppl.enabled)
      {
       sfs::LambertianSeg ls;
       ls.Set(irr);
       ls.Set(leftSeg);
       ls.Set(leftNeedleSmooth);
       if (fitter.mode)
       {
        ls.SetRansac(fitter.chance,fitter.limit,fitter.cap,fitter.cutoff,fitter.halflife);
       }
       else
       {
        ls.SetGA(fitter.population,fitter.keptBest,fitter.immigrants,
                 fitter.mutationRate,fitter.generations,fitter.albSd,fitter.lightSd,
                 fitter.cutoff,fitter.halflife);
       }
       ls.Refine(fitter.refine);
       ls.SetMC(mc.irrSd,mc.irrCap,mc.freedomCost);

       con << "Pass " << i << ": Model Fitting...\n";
       ls.Run(&con.BeginProg());
       con.EndProg();

       ls.GetLight(leftLight);
       ls.GetAlbedo(leftAlbedo);
      }
      else
      {
       sfs::LambertianPP lpp;
       lpp.Set(irr);
       lpp.Set(leftNeedleSmooth);
       lpp.SetWin(ppl.radius,ppl.radSD);
       lpp.SetSel(ppl.prob,ppl.cutoff,ppl.maxSamples);

       con << "Pass " << i << ": Model Fitting...\n";
       lpp.Run(&con.BeginProg());
       con.EndProg();

       for (nat32 y=0;y<irr.Size(1);y++)
       {
        for (nat32 x=0;x<irr.Size(0);x++)
        {
         lpp.Get(x,y,leftLight.Get(x,y));
         leftAlbedo.Get(x,y) = leftLight.Get(x,y).Length();
         leftLight.Get(x,y) /= leftAlbedo.Get(x,y);
        }
       }
      }

      for (nat32 y=0;y<irr.Size(1);y++)
      {
       for (nat32 x=0;x<irr.Size(0);x++)
       {
        leftAlbedo.Get(x,y) = math::Min(leftAlbedo.Get(x,y),real32(150.0));
       }
      }
    }


    // Calculate the irradiance from the fitted shading model, copying over
    // u and v. Shows how good (or not) the fitting is...
     for (nat32 y=0;y<leftModelImage.Size(1);y++)
     {
      for (nat32 x=0;x<leftModelImage.Size(0);x++)
      {
       real32 ir = leftAlbedo.Get(x,y) * (leftNeedleSmooth.Get(x,y) * leftLight.Get(x,y));
       ir = math::Clamp(ir/100.0,0.0,1.0);
       bs::ColourRGB rgb;
       rgb.r = ir; rgb.g = ir; rgb.b = ir;

       bs::ColourLuv temp = rgb;
       temp.u = leftLuv.Get(x,y).u;
       temp.v = leftLuv.Get(x,y).v;

       leftModelImage.Get(x,y) = temp;
      }
     }


    // Run SfS with the shading model, get the new normal map...


    // Save result of super iteration...
     cstrchar buf[512];

     str::Copy(buf,"left_disp_pass_"); str::ToStr(i,buf+str::Length(buf));
     file::SaveDisparity(leftDispSmooth,buf);

     str::Copy(buf,"left_needle_pass_"); str::ToStr(i,buf+str::Length(buf));
     file::SaveNeedle(leftNeedleSmooth,buf);

     str::Copy(buf,"left_light_pass_"); str::ToStr(i,buf+str::Length(buf));
     file::SaveNeedle(leftLight,buf);

     str::Copy(buf,"left_albedo_pass_"); str::ToStr(i,buf+str::Length(buf));
     file::SaveReal(leftAlbedo,buf);

     str::Copy(buf,"left_model_image_"); str::ToStr(i,buf+str::Length(buf)); str::Append(buf,".bmp");
     filter::SaveImage(leftModelImage,buf,true);
   }


  // Use last generated needle map to generate and then save one final smoothed
  // disparity map...
  {
   stereo::GaussianDispSimple gds;
   gds.SetDSI(sdsi);
   gds.SetMode(integration.best_mean);

   stereo::GaussianDispDiffSimple gdds;
   gdds.SetPair(pair);
   gdds.SetNeedle(leftNeedle);
   gdds.SetInvSd(integration.normInvSd);
   if (integration.doImageMod)
   {
    gdds.SetImageMod(leftLuv,integration.imageModThreshold,integration.imageModHalflife);
   }

   stereo::GaussRefine gr;
   gr.Set(gds);
   gr.Set(gdds);
   gr.Set(integration.iters);

   con << "Post: Integration...\n";
   gr.Run(&con.BeginProg());
   con.EndProg();

   gr.DispMap(leftDispSmooth);
   file::SaveDisparity(leftDispSmooth,"left_disp_final");
  }


  /*
  // Some last minute configuration...
   bit eps = paras->GrabBit(":refine-ex.eps",false);
   stereo::LuvDSC luvDSC(leftLuv,rightLuv);
   if (eps)
   {
    bpRefine.Set(luvDSC,
                 paras->GrabReal(":refine-ex.threshold",1.0),
                 paras->GrabReal(":refine-ex.width",0.25));
   }


  // Run refine bp with plane normals to get initial surface...
   bpRefine.Set(dsi);
   bpRefine.Set(leftNeedleIni);
   if (!eps) bpRefine.Set(leftSeg);
   bpRefine.Set(pair);

   con << "Initial Surface Fit...\n";
   bpRefine.Run(&con.BeginProg());
   con.EndProg();

   bpRefine.Disp(leftDispSmooth);
   pair.Convert(leftDispSmooth,leftNeedleSmooth);


  // Estimate albedo...
   albEst.SetImage(leftLuv);
   stereo::SurSegBasic ssb;
   ssb.Set(leftDispSmooth,leftSeg,pair);
   albEst.SetSurSeg(ssb);

   con << "Albedo Estimation...\n";
   albEst.Run(&con.BeginProg());
   con.EndProg();

   albEst.GetAlbedo(leftAlbedo);
   for (nat32 y=0;y<leftAlbedo.Size(1);y++)
   {
    for (nat32 x=0;x<leftAlbedo.Size(0);x++)
    {
     if (leftAlbedo.Get(x,y)>400.0) leftAlbedo.Get(x,y) = 0.0;
    }
   }


  // 'Find' the background...
  // For each segment count its size and how many border pixels it has, if
  // border pixels multiplied by constant is greater than segment size its
  // background.
   nat32 background = 0;//ssb.GetBackground();
   {
    static const nat32 mult = 65;
    ds::Array<nat32> pixels(segy.Segments());
    ds::Array<nat32> bPixels(segy.Segments());
    for (nat32 i=0;i<pixels.Size();i++)
    {
     pixels[i] = 0;
     bPixels[i] = 0;
    }

    for (nat32 y=0;y<leftSeg.Size(1);y++)
    {
     for (nat32 x=0;x<leftSeg.Size(0);x++)
     {
      pixels[leftSeg.Get(x,y)] += 1;
      if ((x==0)||(y==0)||(x==leftSeg.Size(0)-1)||(y==leftSeg.Size(1)-1)) bPixels[leftSeg.Get(x,y)] += 1;
     }
    }

    ds::Array<bit> bg(segy.Segments());
    bg[0] = false;
    for (nat32 i=1;i<bg.Size();i++) bg[i] = pixels[i]<(bPixels[i]*mult);

    for (nat32 y=0;y<leftSeg.Size(1);y++)
    {
     for (nat32 x=0;x<leftSeg.Size(0);x++)
     {
      if (bg[leftSeg.Get(x,y)]) leftSeg.Get(x,y) = 0;
     }
    }
   }


  // Create the background segmentation if needed...
   if (eps)
   {
    for (nat32 y=0;y<leftSegBG.Size(1);y++)
    {
     for (nat32 x=0;x<leftSegBG.Size(0);x++)
     {
      leftSegBG.Get(x,y) = (leftSeg.Get(x,y)==background)?background:(background+1);
     }
    }
   }


  // Run sfs, initialise normals from initial surface...
   sfs.Set(leftLuv);
   sfs.Set(ssb);
   sfs.SetBG(background);
   sfs.Set(albEst.Light());
   sfs.Set(leftAlbedo);

   con << "SfS...\n";
   sfs.Run(&con.BeginProg());
   con.EndProg();

   for (nat32 y=0;y<leftNeedleSfs.Size(1);y++)
   {
    for (nat32 x=0;x<leftNeedleSfs.Size(0);x++) leftNeedleSfs.Get(x,y) = sfs.Orient(x,y);
   }


  // Run refine bp with sfs normals...
   bpRefine.Set(dsi);
   bpRefine.Set(leftNeedleSfs);
   if (eps) bpRefine.Set(leftSegBG);
       else bpRefine.Set(leftSeg);
   bpRefine.Set(pair);

   con << "Final Surface Fit...\n";
   bpRefine.Run(&con.BeginProg());
   con.EndProg();

   bpRefine.Disp(leftDisp);
   pair.Convert(leftDisp,leftNeedle);



  // Quickly create a validity mask...
   for (nat32 y=0;y<leftMask.Size(1);y++)
   {
    for (nat32 x=0;x<leftMask.Size(0);x++) leftMask.Get(x,y) = background!=leftSeg.Get(x,y);
   }

  // Write out all stuff to file...
   con << "Finished. Saving output...\n";
   {
    time::Progress & prog = con.BeginProg();

    prog.Report(0,12);  file::SaveReal(leftAlbedo,"left_albedo");
    prog.Report(1,12);  file::SaveDisparity(leftDispIni,"left_disp_ini",&leftMask);
    prog.Report(2,12);  file::SaveDisparity(leftDispSmooth,"left_disp_smooth",&leftMask);
    prog.Report(3,12);  file::SaveDisparity(leftDisp,"left_disp",&leftMask);
    prog.Report(4,12);  file::SaveSeg(leftSeg,"left_seg");
    prog.Report(5,12);  file::SaveNeedle(leftNeedleIni,"left_needle_ini");
    prog.Report(6,12);  file::SaveNeedle(leftNeedleSmooth,"left_needle_smooth");
    prog.Report(7,12);  file::SaveNeedle(leftNeedleSfs,"left_needle_sfs");
    prog.Report(8,12);  file::SaveNeedle(leftNeedle,"left_needle");
    prog.Report(9,12);  file::SaveWarp(leftRGB,rightRGB,leftDispIni,"left_warp_ini",&leftMask);
    prog.Report(10,11); file::SaveWarp(leftRGB,rightRGB,leftDispSmooth,"left_warp_smooth",&leftMask);
    prog.Report(11,12); file::SaveWarp(leftRGB,rightRGB,leftDisp,"left_warp",&leftMask);

    con.EndProg();
   }*/



 return 0;
}

//------------------------------------------------------------------------------
