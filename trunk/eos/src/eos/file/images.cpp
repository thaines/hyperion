//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "eos/file/images.h"
#include "eos/file/devil_funcs.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
// Helper code for DevIL...
  
// First part of a 2 stage file loader...
bit DevilLoadStart(cstrconst filename,nat32 & handle,nat32 & outWidth,nat32 & outHeight)
{
 ilGenImages(1,(unsigned int *)&handle);
 ilBindImage(handle);
 ilEnable(0x0600);
 ilOriginFunc(0x0601);
 
 ilLoadImage(filename);
 if (ilGetError())
 {
  ilDeleteImages(1,(unsigned int *)&handle);
  return false;
 } 
 else
 {
  outWidth = ilGetInteger(0x0DE4);
  outHeight = ilGetInteger(0x0DE5);
  return true;
 }
}
  
// Second part of 2 stage image loader...
void DevilLoad(nat32 handle,nat32 width,nat32 height,nat32 format,nat32 type,void * data)
{
 ilBindImage(handle);
 ilCopyPixels(0,0,0,width,height,1,format,type,data);
 ilDeleteImages(1,(unsigned int *)&handle);
}


// Image saver...
bit DevilSave(cstrconst filename,nat32 width,nat32 height,nat32 format,nat32 type,void * data,bit overwrite)
{
 unsigned int handle;
 ilGenImages(1,(unsigned int *)&handle); 
 ilBindImage(handle);
 ilEnable(0x0600);
 ilOriginFunc(0x0601);

 if (overwrite) ilEnable(DEVIL_FILE_SQUISH);
           else ilDisable(DEVIL_FILE_SQUISH);
  
 int num;
 switch (format)
 {
  case DEVIL_FORM_RGB: num = 3; break;
  case DEVIL_FORM_RGBA: num = 4; break;
  default: num =  1; break;
 }
 ilTexImage(width,height,1,num,format,type,data);

 ilSaveImage(filename);
 if (ilGetError())
 {
  ilDeleteImages(1,(unsigned int *)&handle);
  return false;
 }
 else
 { 
  ilDeleteImages(1,(unsigned int *)&handle);
  return true;
 }
}

//------------------------------------------------------------------------------
ImageL::ImageL(nat32 w,nat32 h,const bs::ColourL & colour)
:Image(w,h)
{
 data = new bs::ColourL[width*height];

 for (nat32 i=0;i<width*height;i++)
 {
  data[i] = colour;
 }
}

bit ImageL::Load(cstrconst filename)
{
 nat32 handle,width,height;
 if (DevilActive()==false) return false;
 if (DevilLoadStart(filename,handle,width,height))
 {
  SetSize(width,height);
  DevilLoad(handle,width,height,DEVIL_FORM_L,DEVIL_TYPE_FLOAT,data);
  return true;
 } else return false;
}

bit ImageL::Save(cstrconst filename,bit overwrite)
{
 if (DevilActive()==false) return false;
 return DevilSave(filename,width,height,DEVIL_FORM_L,DEVIL_TYPE_FLOAT,data,overwrite);
}

void ImageL::SetSize(nat32 w,nat32 h)
{
 if ((w!=width)||(h!=height))
 {
  width = w;
  height = h;
  delete[] data;
  data = new bs::ColourL[width*height];
 } 
}

ImageL * ImageL::AsL() const
{
 ImageL * ret = new ImageL();
 *ret = *this;
 return ret;
}

ImageRGB * ImageL::AsRGB() const
{
 ImageRGB * ret = new ImageRGB();
 *ret = *this;
 return ret;
}

//------------------------------------------------------------------------------
ImageRGB::ImageRGB(nat32 w,nat32 h,const bs::ColourRGB & colour)
:Image(w,h)
{
 data = new bs::ColourRGB[width*height];

 for (nat32 i=0;i<width*height;i++)
 {
  data[i] = colour;
 }
}

bit ImageRGB::Load(cstrconst filename)
{
 if (DevilActive()==false) return false;
 nat32 handle,width,height;
 if (DevilLoadStart(filename,handle,width,height))
 {
  SetSize(width,height);
  DevilLoad(handle,width,height,DEVIL_FORM_RGB,DEVIL_TYPE_FLOAT,data);
  return true;
 } else return false;
}

bit ImageRGB::Save(cstrconst filename,bit overwrite)
{
 if (DevilActive()==false) return false;
 return DevilSave(filename,width,height,DEVIL_FORM_RGB,DEVIL_TYPE_FLOAT,data,overwrite);
}

void ImageRGB::SetSize(nat32 w,nat32 h)
{
 if ((w!=width)||(h!=height))
 {
  width = w;
  height = h;
  delete[] data;
  data = new bs::ColourRGB[width*height];
 } 
}

ImageL * ImageRGB::AsL() const
{
 ImageL * ret = new ImageL();
 *ret = *this;
 return ret;
}

ImageRGB * ImageRGB::AsRGB() const
{
 ImageRGB * ret = new ImageRGB();
 *ret = *this;
 return ret;
}

//------------------------------------------------------------------------------
 };
};
