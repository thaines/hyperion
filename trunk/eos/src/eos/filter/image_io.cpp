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

#include "eos/filter/image_io.h"

#include "eos/svt/field.h"
#include "eos/file/images.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC svt::Var * LoadImageRGB(svt::Core & core,cstrconst filename,cstrconst fieldname)
{
 file::ImageRGB image;
 if (image.Load(filename))
 {
  svt::Var * ret = new svt::Var(core);
   ret->Setup2D(image.Width(),image.Height());
   bs::ColourRGB ini(0.0,0.0,0.0);
   ret->Add(core.GetTT()(fieldname),ini);
   ret->Commit(false);

   svt::Field<bs::ColourRGB> field;
   ret->ByName(core.GetTT()(fieldname),field);
   //ret->ByInd(0,field);

   for (nat32 y=0;y<image.Height();y++)
   {
    for (nat32 x=0;x<image.Width();x++)
    {
     field.Get(x,y) = image.Get(x,y);
    }
   }

  return ret;
 } else return null<svt::Var*>();
}

EOS_FUNC svt::Var * LoadImageL(svt::Core & core,cstrconst filename,cstrconst fieldname)
{
 file::ImageL image;
 if (image.Load(filename))
 {
  svt::Var * ret = new svt::Var(core);
   ret->Setup2D(image.Width(),image.Height());
   bs::ColourL ini(0.0);
   ret->Add(core.GetTT()(fieldname),ini);
   ret->Commit(false);

   svt::Field<bs::ColourL> field;
   ret->ByName(core.GetTT()(fieldname),field);

   for (nat32 y=0;y<image.Height();y++)
   {
    for (nat32 x=0;x<image.Width();x++)
    {
     field.Get(x,y) = image.Get(x,y);
    }
   }

  return ret;
 } else return null<svt::Var*>();
}

EOS_FUNC bit SaveImageRGB(svt::Var * img,cstrconst filename,bit overwrite,cstrconst fieldname)
{
 file::ImageRGB image;
 image.SetSize(img->Size(0),img->Size(1));

 svt::Field<bs::ColourRGB> field;
 img->ByName(img->GetCore().GetTT()(fieldname),field);

 for (nat32 y=0;y<image.Height();y++)
 {
  for (nat32 x=0;x<image.Width();x++)
  {
   image.Get(x,y) = field.Get(x,y);
  }
 }

 return image.Save(filename,overwrite);
}

EOS_FUNC bit SaveImageL(svt::Var * img,cstrconst filename,bit overwrite,cstrconst fieldname)
{
 file::ImageL image;
 image.SetSize(img->Size(0),img->Size(1));

 svt::Field<bs::ColourL> field;
 img->ByName(img->GetCore().GetTT()(fieldname),field);

 for (nat32 y=0;y<image.Height();y++)
 {
  for (nat32 x=0;x<image.Width();x++)
  {
   image.Get(x,y) = field.Get(x,y);
  }
 }

 return image.Save(filename,overwrite);
}

EOS_FUNC bit SaveImage(const svt::Field<bs::ColourRGB> & in,cstrconst filename,bit overwrite)
{
 file::ImageRGB image;
 image.SetSize(in.Size(0),in.Size(1));

 for (nat32 y=0;y<image.Height();y++)
 {
  for (nat32 x=0;x<image.Width();x++)
  {
   image.Get(x,y) = in.Get(x,y);
  }
 }

 return image.Save(filename,overwrite);
}

EOS_FUNC bit SaveImage(const svt::Field<bs::ColourL> & in,cstrconst filename,bit overwrite)
{
 file::ImageL image;
 image.SetSize(in.Size(0),in.Size(1));

 for (nat32 y=0;y<image.Height();y++)
 {
  for (nat32 x=0;x<image.Width();x++)
  {
   image.Get(x,y) = in.Get(x,y);
  }
 }

 return image.Save(filename,overwrite);
}

//------------------------------------------------------------------------------
 };
};
