#ifndef EOS_FILE_IMAGES_H
#define EOS_FILE_IMAGES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::file
/// Provides all code for interfacing with the file system, including 
/// representations of advanced file types such as xml and images.

/// \file images.h
/// Provides the image classes, including the ability to load and save images 
/// via DevIL. This system includes the ability to dynamically load DevIL when
/// an image is first loaded.

#include "eos/types.h"
#include "eos/mem/alloc.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
// List of classes provided by this file, to make specification of the image 
// class easy...
class ImageL;
class ImageRGB;

//------------------------------------------------------------------------------
/// The root Image class, from which all image classes are created. Provides the
/// interface required of them. Whilst you canot create this class directly it 
/// does provide a hardcoded object factory for generating its children types as
/// a Load static member. This loads the given file into the most appropriate 
/// Image type.
class EOS_CLASS Image : public Deletable
{
 public:
  /// &nbsp;
   Image(nat32 w,nat32 h):width(w),height(h) {}

  /// &nbsp;
   ~Image() {}
  
  /// &nbsp;
   nat32 Width() const {return width;}

  /// &nbsp;
   nat32 Height() const {return height;}

  /// This must resize the image to the given size, it has no responsability 
  /// as to what the image contains afterwards, it is the callers to then 
  /// fill the image appropriatly.
   virtual void SetSize(nat32 width,nat32 height) = 0;

  /// This returns how many bytes of memory the class is using, useful as images
  /// can quickly consume a hell of a lot.
   virtual nat32 Memory() const = 0;

  /// You must call delete on the return value of this class.
   virtual ImageL * AsL() const = 0;

  /// You must call delete on the return value of this class.
   virtual ImageRGB * AsRGB() const = 0;

  /// &nbsp;
   virtual cstrconst TypeString() const = 0;

 protected:
  // All images need this, so we store it here for speed of access.
   nat32 width;
   nat32 height;
};

//------------------------------------------------------------------------------
/// The greyscale image class, allows access to individual pixels as ColourL.
class EOS_CLASS ImageL : public Image
{
 public:
  /// Initialises the image to contain nothing..
   ImageL():Image(0,0),data(null<bs::ColourL*>()) {}

  /// Initialises the image to be a given size and of constant colour.
   ImageL(nat32 width,nat32 height,const bs::ColourL & colour = bs::ColourL(0.0));

  /// &nbsp;
   ~ImageL() {delete[] data;}


  /// &nbsp;
   template <typename T>
   ImageL & operator = (const T & rhs)
   {
    SetSize(rhs.Width(),rhs.Height());
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++) 
     {Get(x,y) = rhs.Get(x,y);}
    }
    return *this;
   }


  /// This loads an Image into the object in question, returning true on success.
  /// On failure nothing changes.
   bit Load(cstrconst filename); 

  /// This saves the Image into the given filename, returns true on success.
  /// Will not overwrite any existing file unless overwrite is set to true.
   bit Save(cstrconst filename,bit overwrite = false);


  /// This returns a reference to any given pixel in the image,
  /// so it can be both read and written.
   bs::ColourL & Get(nat32 x,nat32 y) {return data[y*width + x];}

  /// &nbsp;
   const bs::ColourL & Get(nat32 x,nat32 y) const {return data[y*width + x];}


  /// &nbsp;
   void SetSize(nat32 width,nat32 height);

  /// &nbsp;
   nat32 Memory() const {return width*height*sizeof(bs::ColourL) + sizeof(ImageL);}


  /// &nbsp;
   ImageL * AsL() const;

  /// &nbsp;
   ImageRGB * AsRGB() const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::file::ImageL";}


 private:
  bs::ColourL * data;
};

//------------------------------------------------------------------------------
/// The RGB image class, allows access to individual pixels as ColourRGB.
class EOS_CLASS ImageRGB : public Image
{
 public:
  /// Initialises the image to contain nothing..
   ImageRGB():Image(0,0),data(null<bs::ColourRGB*>()) {}

  /// Initialises the image to be a given size and of constant colour.
   ImageRGB(nat32 width,nat32 height,const bs::ColourRGB & colour = bs::ColourRGB(0.0,0.0,0.0));

  /// &nbsp;
   ~ImageRGB() {delete[] data;}


  /// &nbsp;
   template <typename T>
   ImageRGB & operator = (const T & rhs)
   {
    SetSize(rhs.Width(),rhs.Height());
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++) 
     {Get(x,y) = rhs.Get(x,y);}
    }
    return *this;
   }


  /// This loads an Image into the object in question, returning true on success.
  /// On failure nothing changes.
   bit Load(cstrconst filename); 
 
  /// This saves the Image into the given filename, returns true on success.
  /// Will not overwrite any existing file unless overwrite is set to true.
   bit Save(cstrconst filename,bit overwrite = false);


  /// This returns a reference to any given pixel in the image,
  /// so it can be both read and written.
   bs::ColourRGB & Get(nat32 x,nat32 y) {return data[y*width + x];}

  /// &nbsp;
   const bs::ColourRGB & Get(nat32 x,nat32 y) const {return data[y*width + x];}


  /// &nbsp;
   void SetSize(nat32 width,nat32 height);

  /// &nbsp;
   nat32 Memory() const {return width*height*sizeof(bs::ColourRGB) + sizeof(ImageRGB);}


  /// &nbsp;
   ImageL * AsL() const;

  /// &nbsp;
   ImageRGB * AsRGB() const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::file::ImageRGB";}


 private:
  bs::ColourRGB * data;
};

//------------------------------------------------------------------------------
 };
};
#endif
