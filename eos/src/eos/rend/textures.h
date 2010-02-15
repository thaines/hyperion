#ifndef EOS_REND_TEXTURES_H
#define EOS_REND_TEXTURES_H
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


/// \file textures.h
/// Provides classes that impliment the texture interface. Funky.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Represents a constant value, can only be one type at any given time but can 
/// represent them all. You call the relevent Set method with the type you want,
/// it will then be that type with that value until you next call a Set method.
class EOS_CLASS ConstantTexture : public Texture
{
 public:
  /// On construction it is a value type with an actual value of 0.0.
   ConstantTexture():type(TextureValue),value(0.0) {}

  /// &nbsp;
  ~ConstantTexture();


  /// &nbsp;
   void Set(real32 v) {value = v; type = TextureValue;}
   
  /// &nbsp;
   void Set(bs::ColourRGB c) {colour = c; type = TextureColour;}
   
  /// &nbsp;
   void Set(bs::Pnt o) {offset = o; type = TextureOffset;}
   
  /// &nbsp;
   void Set(bs::Vert v) {vector = v; type = TextureVector;}


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::ConstantTexture";}


 private:
  TextureType type;

  real32 value;
  bs::ColourRGB colour;
  bs::Pnt offset;
  bs::Vert vector;
};

//------------------------------------------------------------------------------
/// Given two inputs this simply adds them both together.
/// You can choose which type it takes as input.
class EOS_CLASS AddTexture : public Texture
{
 public:
  /// &nbsp;
   AddTexture(TextureType t)
   :type(t),a(null<Texture*>()),b(null<Texture*>())
   {}

  /// &nbsp;
   ~AddTexture();


  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::AddTexture";}


 private:
  TextureType type;
  Texture * a;
  Texture * b;
};

//------------------------------------------------------------------------------
/// Multiplies two inputs, you choose the input type.
/// For the offset type it outputs the dot product.
/// For the vector type it suports outputing the dot product, and the
/// cross product, depending on if you link to value or vector.
class EOS_CLASS MultTexture : public Texture
{
 public:
  /// &nbsp;
   MultTexture(TextureType t)
   :type(t),a(null<Texture*>()),b(null<Texture*>())
   {}

  /// &nbsp;
   ~MultTexture();


  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::MultTexture";}


 private:
  TextureType type;

  Texture * a;
  Texture * b;
};

//------------------------------------------------------------------------------
/// Takes and requires three input textures, a,b and mix. mix is a value texture
/// which linearly mixes between the other two textures, a and b. A value of 0 gets you a
/// and a value of 1 gets you b. a and b must be the same type, which you set
/// by construction.
class EOS_CLASS LinearMixTexture : public Texture
{
 public:
  /// &nbsp;
   LinearMixTexture(TextureType t)
   :type(t),a(null<Texture*>()),b(null<Texture*>()),mix(null<Texture*>())
   {}

  /// &nbsp;
   ~LinearMixTexture();


  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::LinearMixTexture";}


 private:
  TextureType type;

  Texture * a;
  Texture * b;
  Texture * mix; 
};

//------------------------------------------------------------------------------
// This takes 3 value fields as input and combined them to create an RGB colour.
class EOS_CLASS ToColourTexture : public Texture
{
 public:
  /// &nbsp;
   ToColourTexture(TextureType t)
   :r(null<Texture*>()),g(null<Texture*>()),b(null<Texture*>())
   {}

  /// &nbsp;
   ~ToColourTexture();


  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::ToColourTexture";}


 private:
  Texture * r;
  Texture * g;
  Texture * b;  
};

//------------------------------------------------------------------------------
// Given a colour texture this outputs a value, it being either the red component, 
// the green component, the blue component or the luminance value.
class EOS_CLASS FromColourTexture : public Texture
{
 public:
  /// An enum of modes the parent class can be in.
   enum Mode {ModeR, ///< Outputs the red component.
              ModeG, ///< Outputs the green component.
              ModeB, ///< Outputs the blue component.
              ModeL  ///< Outputs the luminance component.
             };
 
  /// &nbsp;
   FromColourTexture(Mode m)
   :mode(m),in(null<Texture*>())
   {}

  /// &nbsp;
   ~FromColourTexture();


  /// &nbsp;
   Mode GetMode() const {return mode;}


  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::FromColourTexture";}


 private:
  Mode mode;
  Texture * in; 
};

//------------------------------------------------------------------------------
/// This applys a 4x4 matrix to the texture coordinates, after extending them 
/// to be homogenous, before passing it on to the child texture and returning 
/// whatever it returns. 
/// Simply copys type from child, so no type fixing is required.
class EOS_CLASS TransformTexture : public Texture
{
 public:
  /// &nbsp;
   TransformTexture();

  /// &nbsp;
   ~TransformTexture();
   
  
  /// &nbsp;
   void SetTransform(const math::Mat<4,4> & t) {matrix = t;}
   
  /// &nbsp;
   const math::Mat<4,4> & Transform() const {return matrix;}



  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::TransformTexture";}


 private:
  math::Mat<4,4> matrix;
  Texture * in; 
};


//------------------------------------------------------------------------------
/// Adjusts the texture coordinates passed through to a child texture, in.
/// The distortion channel can be a value, offset or vector, which will then be
/// added to the texture coordinates that its dimensionality covers.
class EOS_CLASS DistortionTexture : public Texture
{
 public:
  /// You construct with the type of distortion texture to be used.
  /// (As texture coordinates are mostly 2D its ushally the TextureOffset type.)
   DistortionTexture(TextureType dt);

  /// &nbsp;
   ~DistortionTexture();


  /// &nbsp;
   nat32 Parameters() const;

  /// &nbsp;
   TextureType ParaType(nat32 index) const;

  /// &nbsp;
   cstrconst ParaName(nat32 index) const;

  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);


  /// &nbsp;
   bit Suports(TextureType type) const;


  /// &nbsp;
   void GetValue(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,real32 & out) const;

  /// &nbsp;
   void GetColour(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::ColourRGB & out) const;

  /// &nbsp;
   void GetOffset(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Pnt & out) const;
                         
  /// &nbsp;
   void GetVector(const TexCoord & coord,const bs::Normal & inDir,const bs::Normal & norm,bs::Vert & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::DistortionTexture";}


 private:
  TextureType dt; // Type of distortion texture.
  Texture * in;
  Texture * distort;
};

//------------------------------------------------------------------------------
// Takes a value as input and outputs a value, after adjusting it using a 
// standard ramp arrangment.
// Contains a cubic spline for providing this functionality.


//------------------------------------------------------------------------------
// Voronoi

//------------------------------------------------------------------------------
// Perlin noise.

//------------------------------------------------------------------------------
// Raw noise.

//------------------------------------------------------------------------------
// From svt types.

//------------------------------------------------------------------------------
 };
};
#endif
