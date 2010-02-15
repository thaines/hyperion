#ifndef EOS_OS_CAMERAS_H
#define EOS_OS_CAMERAS_H
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


/// \namespace eos::os
/// Provides abstractions to specific hardware capabilities. This covers all 
/// non-standard hardware - usb, printers, digital cameras etc.
/// It also provides other highly specific os stuff that is not covered in on 
/// of the other libaries.

/// \file os/cameras.h
/// This uses the libgphoto2 library to provide a link to digital cameras.
/// This unfortunatly makes this a linux only system, though libgphoto is
/// presumably going to be ported to windows at some point - I've guessed
/// how it will be ported so it might just work. But then again, goldfish
/// might be planing to overthrow the human race as revenge for beign kept
/// in spherical rather than dodecahedral bowls.
/// The current system is aimed almost entirely at remote capture, for driving
/// stereo rigs and the like.

#include "eos/types.h"
#include "eos/ds/arrays.h"
#include "eos/bs/dom.h"
#include "eos/file/images.h"

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
/// This represents a camera, allows you to query/set its status and take
/// photos, which are returned straight to the user as svt::Var objects.
/// It uses devIL to decode the file sent back by the camera, so devIL must be 
/// avaliable and the format in use by the camera suported by devIL.
/// Note that this object type is tightly coupled to the camera in question - 
/// you can not have a plurality of these for a single camera.
class EOS_CLASS Camera
{
 public:
  /// &nbsp;
   Camera();
   
  /// &nbsp;
   ~Camera();
   
   
  /// Returns true if the camera is active, i.e. its been linked
  /// and nothing has gone wrong.
   bit Active() const;
   
  /// Disconnects from the camera, leaving this object not active.
   void Deactivate();


  /// This returns the configuration as a dom - this is simply a recoding of 
  /// the gphoto widget structure. Remember to delete the result. Can be saved
  /// to a file and latter reloaded.
   bs::Element * GetConf(str::TokenTable & tt) const;
  
  /// This sets the configuration - you will ushaly use GetConf() get the dom,
  /// present it to the user somehow and then put the returned result back in 
  /// using this, before deleting the dom. 
  /// Is reasonably robust - will revert to defaults with extra information 
  /// and ignore extra information.
  /// All it requires is the same shape and the same id's.
   void SetConf(const bs::Element * root);
   
   
  /// Captures an image and returns it, takes some time so you should probably
  /// send this off into its own thread.
  /// Remember to delete the image.
   file::ImageRGB * Capture() const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::os::Camera";}


 private:
  friend class CameraDetector;
  
  void * cam; // A pointer to a gPhoto Camera object.
};

//------------------------------------------------------------------------------
/// This class does exactly what it says - it detects cameras, it then allows 
/// the user to create Camera objects with which to play.
/// A self populating array with a construction method basically.
class EOS_CLASS CameraDetector
{
 public:
  /// Constructed with an empty list, call Refresh before use.
   CameraDetector();
   
  /// &nbsp;
   ~CameraDetector();
   
   
  /// Rebuilds the list from scratch, scaning the system for cameras.
  /// Returns true on succes, false on failure.
  /// Success just means the scan worked, not that any cameras were 
  /// actually found.
  /// Falure can happen for a lot of reasons - the Log will be 
  /// informed of the actual reason.
   bit Refresh();


  /// Returns the size of the list.
   nat32 Size() const;
   
  /// Returns an identifying string for a particular index.
   cstrconst Name(nat32 i) const;

  /// Returns the port a particular
   cstrconst Port(nat32 i) const;

  /// Searches the list for a particular camera name/port, useful when
  /// loading camera configuration files.
  /// Returns true on success, false on failure.
  /// Either of the strings can be null to not check that string,
  /// if both are null you will just get the first camera on the list.
   bit Search(cstrconst name,cstrconst port,nat32 & out) const;


  /// Given the index of a camera in the list and a Camera object this
  /// arranges for the Camera object to 'be' the camera.
  /// Returns true on success, i.e. the cam i now active.
  /// If the camera is allready active it will be deactivated prior to
  /// reactivation for the new camera.
  /// If you set ptp to true you override the camera's default driver
  /// and use the ptp driver.
   bit Link(nat32 index,Camera * cam,bit ptp = false) const;
   
  /// A conveniance method, combines Search and Link.
   bit Link(cstrconst name,cstrconst port,Camera * cam,bit ptp = false) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::os::CameraDetector";} 


 private:
 
  struct Node
  {
   void * abilities; // Pointer to a gphoto CameraAbilities object.
   void * port; // Pointer to port object, i.e. which port the camera is on.
  };
 
  ds::Array<Node> data;
  void * ptpAbilities; // For ptp override - the gphoto CameraAbilities for this particular object.
  
  void EmptyData(); // Deletes all the data.
};

//------------------------------------------------------------------------------
 };
};
#endif
