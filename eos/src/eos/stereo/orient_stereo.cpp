//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/orient_stereo.h"

#include "eos/inf/fig_factors.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
OrientStereo::OrientStereo()
:minDisp(-30),maxDisp(30),albRes(100),orientRes(180),
alphaK(0.5),alphaErr(0.25),beta(1.0),gammaMult(0.5),gammaMax(1.0)
{
 dispConv[0] = null<cam::DispConv*>();
 dispConv[1] = null<cam::DispConv*>();
 toLight[0] = bs::Normal(0.0,0.0,1.0);
 toLight[1] = bs::Normal(0.0,0.0,1.0);
}

OrientStereo::~OrientStereo()
{
 delete dispConv[0];
 delete dispConv[1];
}

void OrientStereo::Set(int32 minD,int32 maxD,nat32 albR,nat32 orientR)
{
 minDisp = minD;
 maxDisp = maxD;
 albRes = albR;
 orientRes = orientR;
}

void OrientStereo::Set(real32 aK,real32 aErr,real32 b,real32 gMult,real32 gMax)
{
 alphaK = aK;
 alphaErr = aErr;
 beta = b;
 gammaMult = gMult;
 gammaMax = gMax;
}

void OrientStereo::SetIr(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right)
{
 log::Assert(left.Size(1)==right.Size(1),"OrientStereo::SetIr");
 ir[0] = left;
 ir[1] = right;
}

void OrientStereo::SetLight(const bs::Normal & left,const bs::Normal & right)
{
 toLight[0] = left;
 toLight[1] = right;
 
 toLight[0].Normalise();
 toLight[1].Normalise(); 
}

void OrientStereo::SetNeedle(svt::Field<bs::Normal> & left,svt::Field<bs::Normal> & right)
{
 needle[0] = left;
 needle[1] = right;
}

void OrientStereo::SetAlbedo(svt::Field<bs::ColourL> & left,svt::Field<bs::ColourL> & right)
{
 albedo[0] = left;
 albedo[1] = right;
}

void OrientStereo::SetDispConv(const cam::DispConv & left,const cam::DispConv & right)
{
 dispConv[0] = left.Clone();
 dispConv[1] = right.Clone();
}

void OrientStereo::Run(time::Progress * prog)
{
 prog->Push();
  prog->Report(0,2);
  RunHalf(prog,0);
  prog->Report(1,2);
  //RunHalf(prog,1); // Takes too long.
 prog->Pop();
}

cstrconst OrientStereo::TypeString() const
{
 return "eos::stereo::OrientStereo";
}

void OrientStereo::RunHalf(time::Progress * prog,nat32 leftInd)
{
 LogBlock("OrientStereo::RunHalf","{leftInd}" << LogDiv() << leftInd);
 prog->Push();
 
 // Some basic stuff...
  nat32 rightInd = (leftInd+1)%2;
  int32 locMinDisp = (leftInd==0)?minDisp:-maxDisp;
  int32 locMaxDisp = (leftInd==0)?maxDisp:-minDisp;


 // Create the field graph and the smoothing fields...
  prog->Report(0,8);

  inf::FieldGraph fg(true);

  inf::GridSmoothPotts2D * smoAlbX = new inf::GridSmoothPotts2D(false);
  inf::GridSmoothPotts2D * smoAlbY = new inf::GridSmoothPotts2D(true);
  smoAlbX->Set(beta);
  smoAlbY->Set(beta);
  nat32 handSmoAlbX = fg.NewFP(smoAlbX);
  nat32 handSmoAlbY = fg.NewFP(smoAlbY);

  inf::GridSmoothVonMises2D * smoOrient = new inf::GridSmoothVonMises2D();
  smoOrient->Set(alphaK,alphaErr);
  nat32 handSmoOrient = fg.NewFP(smoOrient);


 // Create DSI, ready for creating the albedo-orientation joint distribution...
  prog->Report(1,8);
  
  int32 dispRange = locMaxDisp-locMinDisp+1;
  real32 * dsi = new real32[ir[leftInd].Size(0)*ir[leftInd].Size(1)*dispRange];
  for (nat32 y=0;y<ir[leftInd].Size(1);y++)
  {
   for (nat32 x=0;x<ir[leftInd].Size(0);x++)
   {
    for (int32 d=0;d<dispRange;d++)
    {
     real32 & out = dsi[dispRange*(ir[leftInd].Size(0)*y + x) + d];

     int32 x2 = int32(x) + d - locMinDisp;
     if ((x2<0)||(x2>=int32(ir[rightInd].Size(0)))) out = gammaMax;
     else
     {
      out = math::Min(gammaMax,gammaMult*math::Abs(ir[leftInd].Get(x,y).l-ir[rightInd].Get(x2,y).l));
     }
    }
   }
  }


 // Create irradiance/albedo/orient lookup table to convert to surface orientation...
 // (We use albRes as the iradiance resolution as well.)
  prog->Report(2,8);
  bs::Normal * angToDir = new bs::Normal[albRes*albRes*orientRes];
  
  prog->Push();
  for (nat32 i=0;i<albRes;i++)
  {
   prog->Report(i,albRes);
   for (nat32 a=0;a<albRes;a++)
   {
    for (nat32 o=0;o<orientRes;o++)
    {
     bs::Normal & out = angToDir[orientRes*(albRes*i + a) + o];

     real32 irVal = 100.0*real32(i+1)/real32(albRes);
     real32 albVal = 100.0*real32(a+1)/real32(albRes);
     real32 orientVal = 2.0*math::pi*real32(o)/real32(orientRes);
     real32 cosAng = math::Min<real32>(irVal/albVal,1.0);

     bs::Normal axis;
     math::CrossProduct(toLight[leftInd],bs::Normal(1.0,0.0,0.0),axis);
     axis.Normalise();
     math::Mat<3> rot1;
     math::AngAxisToRotMat(axis,math::InvCos(cosAng),rot1);
     
     math::Mat<3> rot2;
     math::AngAxisToRotMat(toLight[leftInd],orientVal,rot2);
     
     math::Mat<3> rot3;
     math::Mult(rot2,rot1,rot3);
     math::MultVect(rot3,toLight[leftInd],out);
    }
   }
  }
  prog->Pop();

  
 // Create location/disparity to location lookup table...
  prog->Report(3,8);
  prog->Push();
  bs::Vertex * dispToLoc = new bs::Vertex[ir[leftInd].Size(0)*ir[leftInd].Size(1)*dispRange];
  for (nat32 y=0;y<ir[leftInd].Size(1);y++)
  {
   prog->Report(y,ir[leftInd].Size(1));
   for (nat32 x=0;x<ir[leftInd].Size(0);x++)
   {
    for (int32 d=0;d<dispRange;d++)
    {
     bs::Vertex & out = dispToLoc[dispRange*(ir[leftInd].Size(0)*y + x) + d];
     dispConv[leftInd]->DispToPos(x,y,d+locMinDisp,out);
    }
   }
  }
  prog->Pop();


 // Create the albedo-orientation joint distributions (Painful step)...
  prog->Report(4,8);
  
  inf::GridJointPair2D * albOrient = new inf::GridJointPair2D(albRes,orientRes,
                                                              ir[leftInd].Size(0),ir[leftInd].Size(1),
                                                              true);

  LogDebug("[OrientStereo.cache] Starting generation");
  #ifdef EOS_DEBUG
  nat32 caughtInRange = 0;
  #endif
  prog->Push();
  for (nat32 y=0;y<ir[leftInd].Size(1);y++)
  {
   for (nat32 x=0;x<ir[leftInd].Size(0);x++)
   {
    prog->Report(y*ir[leftInd].Size(0)+x,ir[leftInd].Size(0)*ir[leftInd].Size(1));
    for (nat32 a=0;a<albRes;a++)
    {
     for (nat32 o=0;o<orientRes;o++)
     {
      // First convert the alb/orient into a surface orientation...
       nat32 irInd = math::Clamp<nat32>(nat32(albRes*ir[leftInd].Get(x,y).l/100.0),0,albRes-1);
       bs::Normal norm = angToDir[orientRes*(albRes*irInd + a) + o];

      // Convert the adjacent pixel into a ray, ready for the below...
       bs::Vertex start;
       bs::Vertex offset;
       dispConv[leftInd]->Ray(x+1.0,y,start,offset);

      // Now iterate the current pixels dsi, for each calculate the change in 
      // disparity due to the surface orientation and get the resulting dsi value 
      // from the adjuacent pixel. The sum of these two dsi's is the relevant cost.
       real32 minCost = gammaMax*2.0;
       for (int32 d=locMinDisp;d<=locMaxDisp;d++)
       {
        // Convert the disparity to a 3D location...
         bs::Vertex loc = dispToLoc[dispRange*(ir[leftInd].Size(0)*y + x) + locMinDisp+d];

        // Intercept the plane created by the 3D location/orientation with the 
        // ray to get another location...
         bs::Vert lo(loc);
         bs::Vert st(start);
         bs::Vert of(offset);
         
         lo -= st;
         real32 lambda = (lo*norm)/(of*norm);
         loc[0] = st[0] + lambda*of[0];
         loc[1] = st[1] + lambda*of[1];
         loc[2] = st[2] + lambda*of[2];
         loc[3] = 1.0;

        // Convert the location into a disparity...
         real32 lx;
         real32 ly;
         real32 disp;
         dispConv[leftInd]->PosToDisp(loc,lx,ly,disp);
        
        // Grab the relevent DSI scores, add them, set minCost if the new cost 
        // is lower...
         real32 * locDSI = dsi + dispRange*(ir[leftInd].Size(0)*y + x);
         real32 score = locDSI[locMinDisp+d];
         
         disp += locMinDisp;
         nat32 base = nat32(math::Clamp<int32>(int32(disp+locMinDisp),0,dispRange-1));
         nat32 baseP = math::Min<nat32>(dispRange-1,base+1);
         real32 t = math::Mod1(disp);

         score += (1.0-t)*locDSI[base] + t*locDSI[baseP];

         minCost = math::Min(minCost,score);
         #ifdef EOS_DEBUG
         if (base!=baseP) caughtInRange += 1;
         #endif
       }
       albOrient->Val(x,y,a,o) = minCost;
     }
    }
   }
  }
  prog->Pop();
  LogDebug("[OrientStereo.cache] {in-range,in-total}" << LogDiv()
           << caughtInRange << LogDiv() 
           << ir[leftInd].Size(0)*ir[leftInd].Size(1)*albRes*orientRes*dispRange);

  nat32 handAlbOrient = fg.NewFP(albOrient);
  delete[] angToDir;
  delete[] dsi;


 // Declare variables, link it all up...
  prog->Report(5,8);

  nat32 alb = fg.NewVP();
  nat32 orient = fg.NewVP();

  fg.Lay(handSmoAlbX,0,alb);
  fg.Lay(handSmoAlbY,0,alb);

  fg.Lay(handAlbOrient,0,alb);
  fg.Lay(handAlbOrient,1,orient);  

  fg.Lay(handSmoOrient,0,orient);


 // Run...
  prog->Report(6,8);
  fg.Run(prog);


 // Extract the results...
  prog->Report(7,8);

  // Needle...
   for (nat32 y=0;y<needle[leftInd].Size(1);y++)
   {
    for (nat32 x=0;x<needle[leftInd].Size(0);x++)
    {
     real32 albVal = 100.0*real32(fg.Get(alb,y*albedo[leftInd].Size(0) + x).Highest()+1)/real32(albRes);
     real32 orientVal = 2.0*math::pi*fg.Get(alb,y*needle[leftInd].Size(0) + x).Maximum()/real32(orientRes);
     
     real32 cosAng = math::Min<real32>(ir[leftInd].Get(x,y).l/albVal,1.0);
     
     bs::Normal axis;
     math::CrossProduct(toLight[leftInd],bs::Normal(1.0,0.0,0.0),axis);
     axis.Normalise();
     math::Mat<3> rot1;
     math::AngAxisToRotMat(axis,math::InvCos(cosAng),rot1);
     
     math::Mat<3> rot2;
     math::AngAxisToRotMat(toLight[leftInd],orientVal,rot2);
     
     math::Mat<3> rot3;
     math::Mult(rot2,rot1,rot3);
     math::MultVect(rot3,toLight[leftInd],needle[leftInd].Get(x,y));
    }
   }
  
  // Albedo alone, if requested...
   if (albedo[leftInd].Valid())
   {
    for (nat32 y=0;y<albedo[leftInd].Size(1);y++)
    {
     for (nat32 x=0;x<albedo[leftInd].Size(0);x++)
     {
      albedo[leftInd].Get(x,y).l = real32(fg.Get(alb,y*albedo[leftInd].Size(0) + x).Highest()+1)/
                                   real32(albRes);
     }
    }
   }


 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
