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


#include "cyclops/triangulator.h"

//------------------------------------------------------------------------------
Triangulator::Triangulator(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>()),
selected(null<Stored*>())
{
 // Create default images...
  leftVar = new svt::Var(cyclops.Core());
  leftVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  leftVar->Add("rgb",colIni);
  leftVar->Commit();
  leftVar->ByName("rgb",leftImage);

  rightVar = new svt::Var(cyclops.Core());
  rightVar->Setup2D(320,240);
  rightVar->Add("rgb",colIni);
  rightVar->Commit();
  rightVar->ByName("rgb",rightImage);

  pair.SetDefault(320,240);

  leftP[0] = 160; leftP[1] = 120;
  rightP[0] = 160; rightP[1] = 120;


 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Triangulator");
  cyclops.App().Attach(win);
  win->SetSize(leftImage.Size(0)+rightImage.Size(0)+24,math::Max(leftImage.Size(1),rightImage.Size(1))+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Left Image...");
   but2->SetChild(lab2); lab2->Set("Load Right Image...");

   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert2,false);
   vert2->AttachBottom(but1);
   vert2->AttachBottom(but2);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but3->SetChild(lab3); lab3->Set("Load Configuration...");
   horiz1->AttachRight(but3,false);


   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but4->SetChild(lab4); lab4->Set("Add");
   but5->SetChild(lab5); lab5->Set("Delete");

   gui::Vertical * vert4 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert4,false);
   vert4->AttachBottom(but4);
   vert4->AttachBottom(but5);


   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but6->SetChild(lab6); lab6->Set("Save Model...");
   horiz1->AttachRight(but6,false);


   gui::Button * but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but7->SetChild(lab7); lab7->Set("Reload Model...");
   horiz1->AttachRight(but7,false);


   inst1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   inst2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   inst1->Set("Ready");
   inst2->Set("");

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(inst1);
   vert3->AttachBottom(inst2);


   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2);

   gui::Panel * panel1 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   gui::Panel * panel2 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   horiz2->AttachRight(panel1);
   horiz2->AttachRight(panel2);

   left = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   right = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel1->SetChild(left);
   panel2->SetChild(right);

   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   right->SetSize(rightImage.Size(0),rightImage.Size(1));


 // Handlers...
  win->OnDeath(MakeCB(this,&Triangulator::Quit));

  left->OnResize(MakeCB(this,&Triangulator::ResizeLeft));
  left->OnClick(MakeCB(this,&Triangulator::ClickLeft));
  left->OnMove(MakeCB(this,&Triangulator::MoveLeft));

  right->OnResize(MakeCB(this,&Triangulator::ResizeRight));
  right->OnClick(MakeCB(this,&Triangulator::ClickRight));
  right->OnMove(MakeCB(this,&Triangulator::MoveRight));

  but1->OnClick(MakeCB(this,&Triangulator::LoadLeft));
  but2->OnClick(MakeCB(this,&Triangulator::LoadRight));
  but3->OnClick(MakeCB(this,&Triangulator::LoadPair));
  but4->OnClick(MakeCB(this,&Triangulator::Add));
  but5->OnClick(MakeCB(this,&Triangulator::Delete));
  but6->OnClick(MakeCB(this,&Triangulator::SaveModel));
  but7->OnClick(MakeCB(this,&Triangulator::ReloadModel));
}

Triangulator::~Triangulator()
{
 delete win;
 delete leftVar;
 delete rightVar;
}

void Triangulator::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Triangulator::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));


 // Render the image...
  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(leftImage.Size(0),leftImage.Size(1))),bs::Pos(sx,sy),leftImage);


 // Render the triangulation of the stored points...
  ds::Array<ds::Delaunay2D< math::Vect<2,real64> >::Pos> ps;
  ds::Array<ds::Delaunay2D< math::Vect<2,real64> >::Pos> neSet;
  tri.GetAllPos(ps);
  for (nat32 i=0;i<ps.Size();i++)
  {
   ps[i].GetPos(neSet);
   for (nat32 j=0;j<neSet.Size();j++)
   {
    if (ps[i]<neSet[j])
    {
     math::Vect<2,real64> a; a[0] = ps[i].X(); a[1] = ps[i].Y();
     math::Vect<2,real64> b; b[0] = neSet[j].X(); b[1] = neSet[j].Y();
     RenderLeftLine(a,b,bs::ColourRGB(0.0,1.0,0.0));
    }
   }
  }


 // Render the stored points...
 {
  ds::List<Stored>::Cursor targ = store.FrontPtr();
  while (!targ.Bad())
  {
   if (selected==&*targ)
   {
    RenderLeftPoint(targ->leftP,bs::ColourRGB(0.0,0.75,0.0));
   }
   else
   {
    RenderLeftPoint(targ->leftP,bs::ColourRGB(0.75,0.0,0.0));
   }
   ++targ;
  }
 }


 // Render the point and epipolar line of the other point...
 // (Radial distortion makes this a total bitch, rectification just makes it harder.)
  // Get equation of line...
   math::Vect<2,real64> rightA;
    rightA[0] = rightP[0];
    rightA[1] = rightImage.Size(1)-1.0 - rightP[1];

    rightA[0] *= pair.rightDim[0]/real64(rightImage.Size(0));
    rightA[1] *= pair.rightDim[1]/real64(rightImage.Size(1));

    {
     math::Vect<2,real64> temp;
     math::MultVectEH(pair.unRectRight,rightA,temp);
     rightA = temp;
    }

   math::Vect<2,real64> rightB;
    pair.right.radial.UnDis(rightA,rightB);

   math::Vect<3,real64> rightC;
    rightC[0] = rightB[0];
    rightC[1] = rightB[1];
    rightC[2] = 1.0;

   math::Vect<3,real64> line;
   math::TransMultVect(pair.fun,rightC,line);

  // Find nearest point on line to left point, and suitable travel vector...
   math::Vect<2,real64> leftA;
    leftA[0] = leftP[0];
    leftA[1] = leftImage.Size(1)-1.0 - leftP[1];

    leftA[0] *= pair.leftDim[0]/real64(leftImage.Size(0));
    leftA[1] *= pair.leftDim[1]/real64(leftImage.Size(1));

    {
     math::Vect<2,real64> temp;
     math::MultVectEH(pair.unRectLeft,leftA,temp);
     leftA = temp;
    }

   math::Vect<2,real64> lineDir;
    lineDir[0] = line[1];
    lineDir[1] = -line[0];
    lineDir.Normalise();

   math::Vect<2,real64> linePoint;
    linePoint[0] = line[0];
    linePoint[1] = line[1];
    linePoint *= -line[2]/linePoint.LengthSqr();

   math::Vect<2,real64> offset = leftA; offset -= linePoint;
    real64 d = lineDir * offset;
   math::Vect<2,real64> start = lineDir;
    start *= d;
    start += linePoint;

   const nat32 res = 64;
   lineDir *= math::Sqrt(math::Sqr(pair.leftDim[0])+math::Sqr(pair.leftDim[0]))/real64(res);


  // Stuff for below...
   math::Mat<3,3,real64> invURleft;
   {
    math::Mat<3,3,real64> temp;
    invURleft = pair.unRectLeft;
    math::Inverse(invURleft,temp);
   }
   real64 sc[2];
    sc[0] = leftImage.Size(0)/real64(pair.leftDim[0]);
    sc[1] = leftImage.Size(1)/real64(pair.leftDim[1]);


  // In both directions from the starting point render short line segments,
  // compensating for radial distortion, so a suitably bent line can be
  // drawn...
   math::Vect<2,real64> targ = start;
   for (nat32 i=0;i<res;i++)
   {
    math::Vect<2,real64> nTarg = targ; nTarg += lineDir;

    math::Vect<2,real64> a;
    math::Vect<2,real64> b;
    pair.left.radial.Dis(targ,a);
    pair.left.radial.Dis(nTarg,b);

    math::Vect<2,real64> temp;
    math::MultVectEH(invURleft,a,temp); a = temp;
    math::MultVectEH(invURleft,b,temp); b = temp;

    a[0] *= sc[0]; a[1] *= sc[1];
    b[0] *= sc[0]; b[1] *= sc[1];

    a[1] = leftImage.Size(1)-1.0 - a[1];
    b[1] = leftImage.Size(1)-1.0 - b[1];

    RenderLeftLine(a,b,bs::ColourRGB(0.0,0.0,1.0));

    targ = nTarg;
   }

   targ = start;
   for (nat32 i=0;i<res;i++)
   {
    math::Vect<2,real64> nTarg = targ; nTarg -= lineDir;

    math::Vect<2,real64> a;
    math::Vect<2,real64> b;
    pair.left.radial.Dis(targ,a);
    pair.left.radial.Dis(nTarg,b);

    math::Vect<2,real64> temp;
    math::MultVectEH(invURleft,a,temp); a = temp;
    math::MultVectEH(invURleft,b,temp); b = temp;

    a[0] *= sc[0]; a[1] *= sc[1];
    b[0] *= sc[0]; b[1] *= sc[1];

    a[1] = leftImage.Size(1)-1.0 - a[1];
    b[1] = leftImage.Size(1)-1.0 - b[1];

    RenderLeftLine(a,b,bs::ColourRGB(0.0,0.0,1.0));

    targ = nTarg;
   }


  // Render the point transformed to match the epipolar line, sort of a shadow...
   math::Vect<2,real64> lPS = leftP;
   math::Vect<2,real64> rPS = rightP;

   lPS[1] = leftImage.Size(1)-1.0 - lPS[1];
   rPS[1] = rightImage.Size(1)-1.0 - rPS[1];

   lPS[0] *= pair.leftDim[0]/real64(leftImage.Size(0));
   lPS[1] *= pair.leftDim[1]/real64(leftImage.Size(1));
   rPS[0] *= pair.rightDim[0]/real64(rightImage.Size(0));
   rPS[1] *= pair.rightDim[1]/real64(rightImage.Size(1));

   {
    math::Vect<2,real64> temp;
    math::MultVectEH(pair.unRectLeft,lPS,temp); lPS = temp;
    math::MultVectEH(pair.unRectRight,rPS,temp); rPS = temp;
   }

   math::Vect<2,real64> lPS2; pair.left.radial.UnDis(lPS,lPS2);
   math::Vect<2,real64> rPS2; pair.right.radial.UnDis(rPS,rPS2);

   cam::CorrectMatch(lPS2,rPS2,pair.fun);

   pair.left.radial.Dis(lPS2,lPS);

   {
    math::Vect<2,real64> temp;
    math::MultVectEH(invURleft,lPS,temp); lPS = temp;
   }

   lPS[0] *= leftImage.Size(0)/real64(pair.leftDim[0]);
   lPS[1] *= leftImage.Size(1)/real64(pair.leftDim[1]);;

   lPS[1] = leftImage.Size(1)-1.0 - lPS[1];

   RenderLeftPoint(lPS,bs::ColourRGB(0.6,0.0,0.6));


  // Render the point...
   RenderLeftPoint(leftP,bs::ColourRGB(1.0,0.0,1.0));


 // Update...
  left->Update();
}

void Triangulator::ClickLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;

 // If it is a right mouse button click its selection time...
  if (mbe->button==gui::MouseButtonEvent::RMB)
  {
   real64 bestDist = 0.0;
   selected = null<Stored*>();
   ds::List<Stored>::Cursor targ = store.FrontPtr();
    while (!targ.Bad())
    {
     real64 dist = math::Sqr(targ->leftP[0]-mp[0]) + math::Sqr(targ->leftP[1]-mp[1]);
     if ((selected==null<Stored*>())||(dist<bestDist))
     {
      bestDist = dist;
      selected = &*targ;
     }
     ++targ;
    }
   left->Redraw();
   right->Redraw();
   return;
  }

 // Update the point and displayed coordinates...
  leftP[0] = mp[0];
  leftP[1] = mp[1];

  CalcPos();

 // Redraw the display...
  left->Redraw();
  right->Redraw();
}

void Triangulator::MoveLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;

 // Update the point...
  leftP[0] = mp[0];
  leftP[1] = mp[1];

 // Redraw the display...
  left->Redraw();
}

void Triangulator::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));


 // Render the image...
  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(rightImage.Size(0),rightImage.Size(1))),bs::Pos(sx,sy),rightImage);


 // Render the triangulation of the stored points...
  ds::Array<ds::Delaunay2D< math::Vect<2,real64> >::Pos> ps;
  ds::Array<ds::Delaunay2D< math::Vect<2,real64> >::Pos> neSet;
  tri.GetAllPos(ps);
  for (nat32 i=0;i<ps.Size();i++)
  {
   ps[i].GetPos(neSet);
   for (nat32 j=0;j<neSet.Size();j++)
   {
    if (ps[i]<neSet[j])
    {
     RenderRightLine(*(ps[i]),*(neSet[j]),bs::ColourRGB(0.0,1.0,0.0));
    }
   }
  }


 // Render the stored points...
 {
  ds::List<Stored>::Cursor targ = store.FrontPtr();
  while (!targ.Bad())
  {
   if (selected==&*targ)
   {
    RenderRightPoint(targ->rightP,bs::ColourRGB(0.0,0.75,0.0));
   }
   else
   {
    RenderRightPoint(targ->rightP,bs::ColourRGB(0.75,0.0,0.0));
   }
   ++targ;
  }
 }


 // Render the point and epipolar line of the other point...
 // (Radial distortion makes this a total bitch, rectification just makes it harder.)
  // Get equation of line...
   math::Vect<2,real64> leftA;
    leftA[0] = leftP[0];
    leftA[1] = leftImage.Size(1)-1.0 - leftP[1];

    leftA[0] *= pair.leftDim[0]/real64(leftImage.Size(0));
    leftA[1] *= pair.leftDim[1]/real64(leftImage.Size(1));

    {
     math::Vect<2,real64> temp;
     math::MultVectEH(pair.unRectLeft,leftA,temp);
     leftA = temp;
    }

   math::Vect<2,real64> leftB;
    pair.left.radial.UnDis(leftA,leftB);

   math::Vect<3,real64> leftC;
    leftC[0] = leftB[0];
    leftC[1] = leftB[1];
    leftC[2] = 1.0;

   math::Vect<3,real64> line;
   math::Mat<3,3,real64> funTra = pair.fun;
   math::Transpose(funTra);
   math::TransMultVect(funTra,leftC,line);

  // Find nearest point on line to right point, and suitable travel vector...
   math::Vect<2,real64> rightA;
    rightA[0] = rightP[0];
    rightA[1] = rightImage.Size(1)-1.0 - rightP[1];

    rightA[0] *= pair.rightDim[0]/real64(rightImage.Size(0));
    rightA[1] *= pair.rightDim[1]/real64(rightImage.Size(1));

    {
     math::Vect<2,real64> temp;
     math::MultVectEH(pair.unRectRight,rightA,temp);
     rightA = temp;
    }

   math::Vect<2,real64> lineDir;
    lineDir[0] = line[1];
    lineDir[1] = -line[0];
    lineDir.Normalise();

   math::Vect<2,real64> linePoint;
    linePoint[0] = line[0];
    linePoint[1] = line[1];
    linePoint *= -line[2]/linePoint.LengthSqr();

   math::Vect<2,real64> offset = rightA; offset -= linePoint;
    real64 d = lineDir * offset;
   math::Vect<2,real64> start = lineDir;
    start *= d;
    start += linePoint;

   const nat32 res = 64;
   lineDir *= math::Sqrt(math::Sqr(pair.rightDim[0])+math::Sqr(pair.rightDim[0]))/real64(res);

  // Stuff for below...
   math::Mat<3,3,real64> invURright;
   {
    math::Mat<3,3,real64> temp;
    invURright = pair.unRectRight;
    math::Inverse(invURright,temp);
   }
   real64 sc[2];
    sc[0] = rightImage.Size(0)/real64(pair.rightDim[0]);
    sc[1] = rightImage.Size(1)/real64(pair.rightDim[1]);

  // In both directions from the starting point render short line segments,
  // compensating for radial distortion, so a suitably bent line can be
  // drawn...
   math::Vect<2,real64> targ = start;
   for (nat32 i=0;i<res;i++)
   {
    math::Vect<2,real64> nTarg = targ; nTarg += lineDir;

    math::Vect<2,real64> a;
    math::Vect<2,real64> b;
    pair.right.radial.Dis(targ,a);
    pair.right.radial.Dis(nTarg,b);

    math::Vect<2,real64> temp;
    math::MultVectEH(invURright,a,temp); a = temp;
    math::MultVectEH(invURright,b,temp); b = temp;

    a[0] *= sc[0]; a[1] *= sc[1];
    b[0] *= sc[0]; b[1] *= sc[1];

    a[1] = rightImage.Size(1)-1.0 - a[1];
    b[1] = rightImage.Size(1)-1.0 - b[1];

    RenderRightLine(a,b,bs::ColourRGB(0.0,0.0,1.0));

    targ = nTarg;
   }

   targ = start;
   for (nat32 i=0;i<res;i++)
   {
    math::Vect<2,real64> nTarg = targ; nTarg -= lineDir;

    math::Vect<2,real64> a;
    math::Vect<2,real64> b;
    pair.right.radial.Dis(targ,a);
    pair.right.radial.Dis(nTarg,b);

    math::Vect<2,real64> temp;
    math::MultVectEH(invURright,a,temp); a = temp;
    math::MultVectEH(invURright,b,temp); b = temp;

    a[0] *= sc[0]; a[1] *= sc[1];
    b[0] *= sc[0]; b[1] *= sc[1];

    a[1] = rightImage.Size(1)-1.0 - a[1];
    b[1] = rightImage.Size(1)-1.0 - b[1];

    RenderRightLine(a,b,bs::ColourRGB(0.0,0.0,1.0));

    targ = nTarg;
   }

  // Render the point transformed to match the epipolar line, sort of a shadow...
   math::Vect<2,real64> lPS = leftP;
   math::Vect<2,real64> rPS = rightP;

   lPS[1] = leftImage.Size(1)-1.0 - lPS[1];
   rPS[1] = rightImage.Size(1)-1.0 - rPS[1];

   lPS[0] *= pair.leftDim[0]/real64(leftImage.Size(0));
   lPS[1] *= pair.leftDim[1]/real64(leftImage.Size(1));
   rPS[0] *= pair.rightDim[0]/real64(rightImage.Size(0));
   rPS[1] *= pair.rightDim[1]/real64(rightImage.Size(1));

   {
    math::Vect<2,real64> temp;
    math::MultVectEH(pair.unRectLeft,lPS,temp); lPS = temp;
    math::MultVectEH(pair.unRectRight,rPS,temp); rPS = temp;
   }

   math::Vect<2,real64> lPS2; pair.left.radial.UnDis(lPS,lPS2);
   math::Vect<2,real64> rPS2; pair.right.radial.UnDis(rPS,rPS2);

   cam::CorrectMatch(lPS2,rPS2,pair.fun);

   pair.right.radial.Dis(rPS2,rPS);

   {
    math::Vect<2,real64> temp;
    math::MultVectEH(invURright,rPS,temp); rPS = temp;
   }

   rPS[0] *= rightImage.Size(0)/real64(pair.rightDim[0]);
   rPS[1] *= rightImage.Size(1)/real64(pair.rightDim[1]);

   rPS[1] = rightImage.Size(1)-1.0 - rPS[1];

   RenderRightPoint(rPS,bs::ColourRGB(0.6,0.0,0.6));

  // Render the point...
   RenderRightPoint(rightP,bs::ColourRGB(1.0,0.0,1.0));

 // Update...
  right->Update();
}

void Triangulator::ClickRight(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;

 // If it is a right mouse button click its selection time...
  if (mbe->button==gui::MouseButtonEvent::RMB)
  {
   real64 bestDist = 0.0;
   selected = null<Stored*>();
   ds::List<Stored>::Cursor targ = store.FrontPtr();
    while (!targ.Bad())
    {
     real64 dist = math::Sqr(targ->rightP[0]-mp[0]) + math::Sqr(targ->rightP[1]-mp[1]);
     if ((selected==null<Stored*>())||(dist<bestDist))
     {
      bestDist = dist;
      selected = &*targ;
     }
     ++targ;
    }
   left->Redraw();
   right->Redraw();
   return;
  }

 // Update the point and displayed coordinates...
  rightP[0] = mp[0];
  rightP[1] = mp[1];

  CalcPos();

 // Redraw the display...
  left->Redraw();
  right->Redraw();
}

void Triangulator::MoveRight(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;

 // Update the point...
  rightP[0] = mp[0];
  rightP[1] = mp[1];

 // Redraw the display...
  right->Redraw();
}

void Triangulator::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
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
   leftVar->Setup2D(floatVar->Size(0),floatVar->Size(1));
   leftVar->Commit();
   leftVar->ByName("rgb",leftImage);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     leftImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;

  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();
 }
}

void Triangulator::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
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
   rightVar->Setup2D(floatVar->Size(0),floatVar->Size(1));
   rightVar->Commit();
   rightVar->ByName("rgb",rightImage);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<rightImage.Size(1);y++)
   {
    for (nat32 x=0;x<rightImage.Size(0);x++)
    {
     rightImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;

  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
 }
}

void Triangulator::LoadPair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
  else
  {
   left->Redraw();
   right->Redraw();
   CalcPos();
  }
 }
}

void Triangulator::Add(gui::Base * obj,gui::Event * event)
{
 // Create the structure to store...
  Stored s;
   s.leftP = leftP;
   s.rightP = rightP;

  math::Vect<2,real64> leftIC;
  math::Vect<2,real64> rightIC;

  leftIC[0] = leftP[0];
  leftIC[1] = leftImage.Size(1)-1.0 - leftP[1];
  rightIC[0] = rightP[0];
  rightIC[1] = rightImage.Size(1)-1.0 - rightP[1];

  leftIC[0] *= pair.leftDim[0]/real64(leftImage.Size(0));
  leftIC[1] *= pair.leftDim[1]/real64(leftImage.Size(1));
  rightIC[0] *= pair.rightDim[0]/real64(rightImage.Size(0));
  rightIC[1] *= pair.rightDim[1]/real64(rightImage.Size(1));

  {
   math::Vect<2,real64> temp;
   math::MultVectEH(pair.unRectLeft,leftIC,temp); leftIC = temp;
   math::MultVectEH(pair.unRectRight,rightIC,temp); rightIC = temp;
  }

  pair.left.radial.UnDis(leftIC,leftIC);
  pair.right.radial.UnDis(rightIC,rightIC);


  math::Vect<4,real64> out;
  cam::CorrectMatch(leftIC,rightIC,pair.fun);
  
  cam::Triangulate(leftIC,rightIC,pair.lp,pair.rp,out);
  out.Normalise();
  if (math::IsZero(out[3]))
  {
   inst2->Set(str::String("Can't store infinite points"));
   return;
  }

  out /= out[3];
  s.pos[0] = out[0];
  s.pos[1] = out[1];
  s.pos[2] = out[2];

 // Store it...
  store.AddFront(s);

 // Add to Delaunay triangulation...
  tri.Add(s.leftP,s.rightP);

 // Redraw...
  left->Redraw();
  right->Redraw();
}

void Triangulator::Delete(gui::Base * obj,gui::Event * event)
{
 ds::List<Stored>::Cursor targ = store.FrontPtr();
 bit rebuildTri = false;
 while (!targ.Bad())
 {
  if (&*targ==selected)
  {
   targ.RemKillNext();
   selected = null<Stored*>();
   rebuildTri = true;
   break;
  }
  ++targ;
 }

 if (rebuildTri)
 {
  tri.Reset();

  targ = store.FrontPtr();
  while (!targ.Bad())
  {
   tri.Add(targ->leftP,targ->rightP);
   ++targ;
  }

  left->Redraw();
  right->Redraw();
 }
}

void Triangulator::SaveModel(gui::Base * obj,gui::Event * event)
{
 // First build a deluaney triangulation of the matches, using the left image
 // coordinates to triangulate it...
 // Simultaneously store the relevent vertices into a mesh...
  ds::Delaunay2D<sur::Vertex> tri;
  str::TokenTable tt;
  real32 realIni = 0.0;

  sur::Mesh mesh(&tt);
  mesh.AddVertProp("u",realIni);
  mesh.AddVertProp("v",realIni);
  mesh.Commit();

  data::Property<sur::Vertex,real32> propU = mesh.GetVertProp<real32>("u");
  data::Property<sur::Vertex,real32> propV = mesh.GetVertProp<real32>("v");

  {
   ds::List<Stored>::Cursor targ = store.FrontPtr();
   for (nat32 i=0;i<store.Size();i++)
   {
    // Create relevent vertex and uv coordinate...
     bs::Vert vert;
     bs::Tex2D uv;
      vert[0] = targ->pos[0];
      vert[1] = targ->pos[1];
      vert[2] = targ->pos[2];
      uv[0] = targ->leftP[0]/real32(leftImage.Size(0));
      uv[1] = 1.0 - (targ->leftP[1]/real32(leftImage.Size(1)));

    // Add the vertex to the mesh...
     sur::Vertex mVert = mesh.NewVertex(vert);
     propU.Get(mVert) = uv[0];
     propV.Get(mVert) = uv[1];

    // Add it in...
     tri.Add(targ->leftP[0],targ->leftP[1],mVert);

    ++targ;
   }
  }


 // Create all triangles, direct from the triangulation...
 {
  ds::Array<ds::Delaunay2D<sur::Vertex>::Mid> mid;
  tri.GetAllMid(mid);
  for (nat32 i=0;i<mid.Size();i++)
  {
   if (mid[i].HasCentre())
   {
    mesh.NewFace(*mid[i].GetPos(0),*mid[i].GetPos(1),*mid[i].GetPos(2));
   }
  }
 }


 // Ask for a filename, and save...
  str::String fn(".ply");
  if (cyclops.App().SaveFileDialog("Save mesh...",fn))
  {
   if ((!fn.EndsWith(".obj"))&&(!fn.EndsWith(".ply"))) fn += ".ply";
   // Save the file...
    if (!file::SaveMesh(mesh,fn,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
}

void Triangulator::ReloadModel(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Reload Mesh","*.obj,*.ply",fn))
 {
  str::TokenTable tt;
  sur::Mesh * mesh = file::LoadMesh(fn,null<time::Progress*>(),&tt);
  if (mesh)
  {
   ds::Array<sur::Vertex> verts;
   mesh->GetVertices(verts);

   math::Mat<3,3,real64> tempMat;
   math::Mat<3,3,real64> rectLeft  = pair.unRectLeft;
   math::Mat<3,3,real64> rectRight = pair.unRectRight;
   math::Inverse(rectLeft,tempMat);
   math::Inverse(rectRight,tempMat);

   for (nat32 i=0;i<verts.Size();i++)
   {
    Stored ns;
    for (nat32 j=0;j<3;j++) ns.pos[j] = verts[i].Pos()[j];

    math::MultVectEH(pair.lp,ns.pos,ns.leftP);
    math::MultVectEH(pair.rp,ns.pos,ns.rightP);

    pair.left.radial.Dis(ns.leftP,ns.leftP);
    pair.right.radial.Dis(ns.rightP,ns.rightP);

    math::Vect<2,real64> temp;
    math::MultVectEH(rectLeft,ns.leftP,temp);   ns.leftP  = temp;
    math::MultVectEH(rectRight,ns.rightP,temp); ns.rightP = temp;
    
    ns.leftP[0] *= real64(leftImage.Size(0))/pair.leftDim[0];
    ns.leftP[1] *= real64(leftImage.Size(1))/pair.leftDim[1];
    ns.rightP[0] *= real64(rightImage.Size(0))/pair.rightDim[0];
    ns.rightP[1] *= real64(rightImage.Size(1))/pair.rightDim[1];

    ns.leftP[1]  = leftImage.Size(1)-1.0 - ns.leftP[1];
    ns.rightP[1] = rightImage.Size(1)-1.0 - ns.rightP[1];


    store.AddBack(ns);
    tri.Add(ns.leftP,ns.rightP);
   }
   
   left->Redraw();
   right->Redraw();   

   delete mesh;
  }
  else
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error loading the file.");
  }
 }
}

void Triangulator::RenderLeftPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col)
{
 nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
 nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 left->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 left->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);
}

void Triangulator::RenderRightPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col)
{
 nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
 nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

 nat32 cx = sx + nat32(p[0]);
 nat32 cy = sy + nat32(p[1]);

 right->P().Line(bs::Pos(cx-5,cy),bs::Pos(cx+5,cy),col);
 right->P().Line(bs::Pos(cx,cy-5),bs::Pos(cx,cy+5),col);
}

void Triangulator::RenderLeftLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col)
{
 nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
 nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);

 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);

 left->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col);
}

void Triangulator::RenderRightLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col)
{
 nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
 nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);

 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);

 right->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col);
}

void Triangulator::CalcPos()
{
 // Change the point origins & scale...
  math::Vect<2,real64> leftA;
   leftA[0] = leftP[0];
   leftA[1] = leftImage.Size(1)-1.0 - leftP[1];

   leftA[0] *= pair.leftDim[0]/real64(leftImage.Size(0));
   leftA[1] *= pair.leftDim[1]/real64(leftImage.Size(1));

  math::Vect<2,real64> rightA;
   rightA[0] = rightP[0];
   rightA[1] = rightImage.Size(1)-1.0 - rightP[1];

   rightA[0] *= pair.rightDim[0]/real64(rightImage.Size(0));
   rightA[1] *= pair.rightDim[1]/real64(rightImage.Size(1));

  LogDebug("[triangulator] Input {left,right}" << LogDiv() << leftA[0] << " ," << leftA[1] << LogDiv()
           << rightA[0] << " ," << rightA[1]);
  

 // Factor in rectification...
  {
   math::Vect<2,real64> temp;
   math::MultVectEH(pair.unRectLeft,leftA,temp); leftA = temp;
   math::MultVectEH(pair.unRectRight,rightA,temp); rightA = temp;
  }

  LogDebug("[triangulator] Unrectified {left,right}" << LogDiv() << leftA[0] << " ," << leftA[1] << LogDiv()
           << rightA[0] << " ," << rightA[1]);


 // Remove radial distortion...
   pair.left.radial.UnDis(leftA,leftA);
   pair.right.radial.UnDis(rightA,rightA);

  LogDebug("[triangulator] Undistorted {left,right}" << LogDiv() << leftA[0] << " ," << leftA[1] << LogDiv()
           << rightA[0] << " ," << rightA[1]);


 // Adjust to minimise epipolar error...
  cam::CorrectMatch(leftA,rightA,pair.fun);

  LogDebug("[triangulator] Corrected {left,right}" << LogDiv() << leftA[0] << " ," << leftA[1] << LogDiv()
           << rightA[0] << " ," << rightA[1]);

 // Triangulate...
  math::Vect<4,real64> pos;
  cam::Triangulate(leftA,rightA,pair.lp,pair.rp,pos);
  if (math::IsZero(pos[3]))
  {
   // Stick it in the labels...
    str::String s1;
     s1 << "At infinity, in direction (" << pos[0] << "," << pos[1] << "," << pos[2] << ")";
    inst1->Set(s1);
    str::String s2;
    inst2->Set(s2);
  }
  else
  {
   // Calculate depth and distance as well...
    math::Vect<4,real64> centre;
    cam::Intrinsic intr;
    math::Mat<3,3,real64> rot;
    pair.lp.Centre(centre);
    pair.lp.Decompose(intr,rot);

    pos /= pos[3];
    centre /= centre[3];

    math::Vect<4,real64> vecTo = pos; vecTo -= centre;
    real64 distance = vecTo.Length();
    real64 depth = -pair.lp.Depth(pos);

   // Stick it in the labels...
    str::String s1;
     s1 << "(" << pos[0] << "," << pos[1] << "," << pos[2] << ")";
    inst1->Set(s1);
    str::String s2;
     s2 << "distance = " << distance << "; depth = " << depth;
    inst2->Set(s2);
  }


  LogDebug("[triangulator] Triangulated to {pos}" << LogDiv() << pos);
}

//------------------------------------------------------------------------------
