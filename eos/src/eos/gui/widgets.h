#ifndef EOS_GUI_WIDGETS_H
#define EOS_GUI_WIDGETS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file widgets.h
/// Defines all the abstract widget objects the system provides.

#include "eos/types.h"
#include "eos/rend/pixels.h"
#include "eos/time/progress.h"
#include "eos/gui/base.h"
#include "eos/gui/callbacks.h"
#include "eos/str/strings.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
/// The Widget object, everything visible that exists with a Window as its 
/// parent impliments this class.
class EOS_CLASS Widget : public Base
{
 public:
  /// &nbsp;
   Widget();
  
  /// &nbsp;
   ~Widget();


  /// Sets the minimum size the widget will request, should be mostly avoided but
  /// often necesary none-the-less.
   virtual void SetSize(nat32 width,nat32 height) = 0;
   
  /// Returns the border size. (Only relevent if it contains other widgets/content.)
   virtual nat32 Border() const = 0;
   
  /// Sets the border size. (Only relevent if it contains other widgets/content.)
   virtual void SetBorder(nat32 bs) = 0;   
   
  /// Sets the visibility of the widget.
   virtual void Visible(nat32 vis) = 0;
};

//------------------------------------------------------------------------------
/// A Wrapper object, the parent type of widgets that contain just one child 
/// widget.
class EOS_CLASS Wrapper : public Widget
{
 public:
  /// &nbsp;
   Wrapper();
   
  /// &nbsp;
   ~Wrapper();


  /// Returns the child Widget.
   virtual Widget * Child() const = 0;

  /// Sets the child Widget, will ushally be a Layout of some kind.
   virtual void SetChild(Widget * w) = 0;
};

//------------------------------------------------------------------------------
/// A Layout object, the parent of objects which can contain multiple widgets
/// and hence arrange them on the screen somehow.
class EOS_CLASS Layout : public Widget
{
 public:
  /// &nbsp;
   Layout();
  
  /// &nbsp;
   ~Layout();
};

//------------------------------------------------------------------------------
/// Lays out widgets horizontally.
class EOS_CLASS Horizontal : public Layout
{
 public:
  /// &nbsp;
   Horizontal();
    
  /// &nbsp;
   ~Horizontal();

  
  /// Returns the size of the gap between widgets.
  /// Defaults to 2.
   virtual nat32 GapSize() const = 0;
  
  /// Sets the gap between widgets, returns the new gap.
   virtual void GapSize(nat32 ngs) = 0;

  
  /// Adds a widget to the left hand side.
   virtual void AttachLeft(Widget * w,bit stretch = true) = 0;
  
  /// Adds a widget to the right hand side.  
   virtual void AttachRight(Widget * w,bit stretch = true) = 0;

  
  /// Returns how many children are contained.
   virtual nat32 Children() const = 0;
   
   
  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Lays out widgets vertically.
class EOS_CLASS Vertical : public Layout
{
 public:
  /// &nbsp;
   Vertical();
    
  /// &nbsp;
   ~Vertical();

  
  /// Returns the size of the gap between widgets.
   virtual nat32 GapSize() const = 0;
  
  /// Sets the gap between widgets, returns the new gap.
   virtual void GapSize(nat32 ngs) = 0;

  
  /// Adds a widget to the top of the layout.
   virtual void AttachTop(Widget * w,bit stretch = true) = 0;
  
  /// Adds a widget to the bottom of the layout.
   virtual void AttachBottom(Widget * w,bit stretch = true) = 0;


  /// Returns how many children are contained.
   virtual nat32 Children() const = 0;


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Lays out objects in a grid.
class EOS_CLASS Grid : public Layout
{
 public:
  /// &nbsp;
   Grid();
   
  /// &nbsp;
   ~Grid();
   
  /// Returns the number of columns for the grid.
   virtual nat32 Cols() const = 0;
   
  /// Returns the number of rows for the grid.
   virtual nat32 Rows() const = 0;
   
  /// Sets the size of the grid, if you shrink it any widgets outside the new
  /// size will be deleted. Starts out as a 1x1 grid by default, which you will
  /// invariably have to change.
   virtual void SetDims(nat32 cols,nat32 rows) = 0;
   
  /// Outputs the gap size between widgets in the grid.
   virtual void GetGapSize(nat32 & xOut,nat32 & yOut) const = 0;
   
  /// Sets the gap size between widgets in the grid.
   virtual void SetGapSize(nat32 x,nat32 y) = 0;
  
  /// Sets the widget at a particular location in the grid.
  /// If a Widget allready exist there it will be deleted.
  /// \param col The column for the widget to start in.
  /// \param row The row for the widget to start in.
  /// \param w The widget to attach.
  /// \param width The number of colums for the widget to span.
  /// \param height The number of rows for the widget to span.
   virtual void Attach(nat32 col,nat32 row,Widget * w,nat32 width = 1,nat32 height = 1) = 0;
 

  /// Returns a pointer to the widget at a particular location, null if there is
  /// no Widget at that location.
   virtual Widget * Get(nat32 x,nat32 y) const = 0;
   
   
  /// &nbsp;
   cstrconst TypeString() const;  
};

//------------------------------------------------------------------------------
/// An invisible container, if the contents of this container are larger than 
/// the container itself however it adds scroll bars. Any resizing Window should
/// ushally have one of these as there child.
class EOS_CLASS Panel : public Wrapper
{
 public:
  /// &nbsp;
   Panel();
   
  /// &nbsp;
   ~Panel();


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// A frame, for segmenting off widgets into suitable clusters, optionally draws
/// a border and optionally has a label positioned across the top of the frame.
class EOS_CLASS Frame : public Wrapper
{
 public:
  /// &nbsp;
   Frame();
   
  /// &nbsp;
   ~Frame();


  /// Sets if it has a border or not. Defaults to having a border.
   virtual void DrawBorder(bit bv) = 0;

  /// Sets the border label and its position. Position goes from 0.0 (left) to 1.0 (right).
  /// The label defaults to none.
   virtual void Set(cstrconst label,real32 pos) = 0;

  /// Sets the border label and its position. Position goes from 0.0 (left) to 1.0 (right).
  /// The label defaults to none.
   virtual void Set(const str::String & label,real32 pos);


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Allows a contained set of widgets to be hidden/shown at will by the user.
class EOS_CLASS Expander : public Wrapper
{
 public:
  /// &nbsp;
   Expander();
   
  /// &nbsp;
   ~Expander();
   
   
  /// Sets the expanders label, default to an empty string.
   virtual void Set(cstrconst label) = 0;

  /// Sets the expanders label, default to an empty string.
   virtual void Set(const str::String & label);
   
  /// Sets if its expanded or not, false to expand, true to hide.
   virtual void Expand(bit expand) = 0;


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Provides an area where arbitary drawing operations can be called, to draw
/// any arbitary shape of choice. Provides access to the eos::rend::Pixels interface.
class EOS_CLASS Canvas : public Widget
{
 public:
  /// &nbsp;
   Canvas();

  /// &nbsp;
   ~Canvas();


  /// Sets the size of the canvas, if this is never called then it simply adjusts
  /// its size to its container as it changes. Note this is actually a minimum size,
  /// if it has space to be bigger it will be - its your choice on how to manage such
  /// scenarios.
   virtual void SetSize(nat32 width,nat32 height) = 0;
   
  /// Provides a reference to a rend::Pixels interface, so you can draw to this.
  /// Note that there is no clearing of this buffer, that is your responsibility.
   virtual rend::Pixels & P() = 0;
   
  /// Operations performed on the above are not actually visible till the update
  /// method is called, to avoid the horrible effects that would occur otherwise.
  /// (Note there is an exception to this in that if the window is going from 
  /// hidden to unhidden it is allowed to automatically grab whatever is in the 
  /// buffer at the time.)
   virtual void Update() = 0;
   
  /// This forces a Resize event to happen, even though no resize has actually
  /// happened - useful as means of making the canvas redraw after changing its
  /// backend data structure.
   virtual void Redraw() = 0;


  /// This is called whenever the canvas is resized, including the first time the
  /// canvas is shown, allows updating things accordingly. Note that if you don't
  /// provide this handler to draw some stuff then call update expect a mess.
   void OnResize(Callback * cb);
  
  /// Sets the callback handler for when a mouse is clicked on the surface.
   void OnClick(Callback * cb);

  /// Sets the callback handler for when the mouse wheel is scrolled when its over the surface.
   void OnWheel(Callback * cb);
   
  /// Sets the callback handler for when the mouse is moved over the surface.
   void OnMove(Callback * cb);


  /// &nbsp;
   cstrconst TypeString() const;


 protected:
  Callback * onResize;
  Callback * onClick;
  Callback * onWheel;
  Callback * onMove;
};

//------------------------------------------------------------------------------
/// Provides an area of text the user can't edit.
class EOS_CLASS Label : public Widget
{
 public:
  /// &nbsp;
   Label();
   
  /// &nbsp;
   ~Label();


  /// Allows you to set the contents of the label.
   virtual void Set(cstrconst str) = 0;

  /// Allows you to set the contents of the label.
   virtual void Set(const str::String & str);


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Provides a drop down box from which several options can be selected.
class EOS_CLASS ComboBox : public Widget
{
 public:
  /// &nbsp;
   ComboBox();
   
  /// &nbsp;
   ~ComboBox();


  /// Returns the index of the selected item.
   virtual nat32 Get() const = 0;

  /// Sets the selected item by index.
   virtual void Set(nat32 sel) = 0;


  /// Appends an item.
   virtual void Append(cstrconst s) = 0;
   
  /// Prepends an item.
   virtual void Prepend(cstrconst s) = 0;
   
  /// Inserts an item, such that its index is pos, assuming its not past the 
  /// size of the selection set.
   virtual void Insert(cstrconst s,nat32 pos) = 0;
   
  /// Removes an item, indexing via text string.
   virtual void Delete(nat32 pos) = 0;


  /// This is called every time the ComboBox is changed. Takes a NullEvent.
   void OnChange(Callback * cb);


  /// &nbsp;
   cstrconst TypeString() const;


 protected:
  Callback * onChange;   
};

//------------------------------------------------------------------------------
/// Provides the standard edit box, includes a validation result indication 
/// arrangment...
class EOS_CLASS EditBox : public Widget
{
 public:
  /// &nbsp;
   EditBox();
   
  /// &nbsp;
   ~EditBox();


  /// Sets the edit box.
   virtual void Set(cstrconst s) = 0;

  /// Sets the edit box.
   virtual void Set(const str::String & s);
  
  /// Gets the edit boxes contents.
   virtual const str::String & Get() const = 0;

  /// Gets the edit boxes contents as an int. Default provided for if the edit
  /// box is not parsable as such.
   int32 GetInt(int32 def) const;
   
  /// Gets the edit boxes contents as a real. Default provided for if the edit
  /// box is not parsable as such.
   real32 GetReal(real32 def) const;
  
  /// Sets if the box is correct or not, true to indicate the contents is valid,
  /// false to indicate its invalid and the user needs to do something.
  /// This is ushally with a shade of red.
   virtual void Valid(bit state) = 0;


  /// This is called every time the EditBox is changed, its ushally used to validate input
  /// and then to update the validation state as appropriate. Takes a NullEvent.
   void OnChange(Callback * cb);


  /// &nbsp;
   cstrconst TypeString() const; 


 protected:
  Callback * onChange;
};

//------------------------------------------------------------------------------
/// Provides an area of editable text. We use \n to indicate new lines.
class EOS_CLASS Multiline : public Widget
{
 public:
  /// &nbsp;
   Multiline();
   
  /// &nbsp;
   ~Multiline();


  /// Sets if the user can edit it or not. 
   virtual void Edit(bit can) = 0;

   
  /// Emptys the text buffer of all text.
   virtual void Empty() = 0;
   
  /// Appends to the end of the buffer.
   virtual void Append(cstrconst str) = 0;

  /// Appends to the end of the buffer.
   virtual void Append(const str::String & str) = 0;


  /// Returns how many lines are avaliable.
   virtual nat32 Lines() const = 0;
   
  /// Outputs a specific line.
   virtual void GetLine(nat32 iter,str::String & out) const = 0;

  /// Gets all the text.
   virtual void GetAll(str::String & out) = 0;


  /// &nbsp;
   cstrconst TypeString() const;         
};

//------------------------------------------------------------------------------
/// Provides a tick box. Contains a widget to one side of the text box,
/// you will ushally add a label.
class EOS_CLASS TickBox : public Wrapper
{
 public:
  /// &nbsp;
   TickBox();
   
  /// &nbsp;
   ~TickBox();


  /// Call with true to tick it, false ot untick it.
   virtual void SetState(bit flag) = 0;

  /// Returns true if ticked, false if not ticked.
   virtual bit Ticked() const = 0;


  /// Sets the callback handler for when the state changes
  /// Uses a NullEvent.
   void OnChange(Callback * cb);


  /// &nbsp;
   cstrconst TypeString() const;


 protected:
  Callback * onChange;
};

//------------------------------------------------------------------------------
/// Provides a button. Note that it contains a widget, most button instances will
/// contain a Label.
class EOS_CLASS Button : public Wrapper
{
 public:
  /// &nbsp;
   Button();
   
  /// &nbsp;
   ~Button();


  /// Sets the callback handler for when the mouse is clicked on the button.
  /// Uses a NullEvent.
   void OnClick(Callback * cb);


  /// &nbsp;
   cstrconst TypeString() const;

 
 protected:
  Callback * onClick;
};

//------------------------------------------------------------------------------
class EOS_CLASS ProgressBar : public Widget, protected time::Progress
{
 public:
  /// &nbsp;
   ProgressBar();
   
  /// &nbsp;
   ~ProgressBar();


  /// The 4 compass directions the progress bar can be in.
   enum Dir {North, ///< &nbsp;
             East, ///< &nbsp;
             South, ///< &nbsp;
             West, ///< &nbsp;
            };

  /// This sets the direction of the progress bar - any of the 4 compass points 
  /// are acceptable. This is the direction the progress bar heads as it gets 
  /// nearer to completion.
  /// Defaults to East as you would expect.
   virtual void SetDir(Dir dir) = 0;


  /// This starts the progress bar, returns the relevant pointer. When not 
  /// actually running the progress bar remains greyed out etc.
  /// Progress bar updates result in the event queue being proccessed,
  /// so the application remains drawn. Note that further buttons can be pressed
  /// during this, so you need to be careful. Do not call Begin again until End 
  /// is called.
   virtual time::Progress * Begin() = 0;

  /// Once you have finished using the progress bar call this method.
  /// You can then run it again if you so desire.
   virtual void End() = 0;
   
  /// This returns true if Begin() has been called and End has not, false otherwise.
   virtual bit Running() const = 0;


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// The Window object, where widgets live out there blissful lives right upto 
/// the point they are vicously deleted.
class EOS_CLASS Window : public Base
{
 public:
  /// &nbsp;
   Window();
   
  /// &nbsp;
   ~Window();


  /// Returns the width.
   virtual nat32 Width() const = 0;
   
  /// Returns the height.
   virtual nat32 Height() const = 0;
   
  /// Sets the size.
   virtual void SetSize(nat32 width,nat32 height) = 0;


  /// Returns the border size.
   virtual nat32 Border() const = 0;
   
  /// Sets the border size.
   virtual void SetBorder(nat32 bs) = 0;
   
  /// Returns true if it can be resized, false if it can't.
   virtual bit CanResize() const = 0;
   
  /// Sets if it can resize or not.
   virtual void Resizable(bit s) = 0;


  /// Sets if its minimised or not.
   virtual void Minimised(bit mm) = 0;
   
  /// Sets if its maximised or not.
   virtual void Maximised(bit mm) = 0;
   
  /// Sets if its fullscreen or not.
   virtual void Fullscreen(bit fs) = 0;
   
  /// Sets if its is visible or not, note that it can not be visible
  /// unless it has be Attached to an App.
   virtual void Visible(bit vs) = 0;
  
   
  /// Sets the title.
   virtual void SetTitle(cstrconst title) = 0;

  /// Sets the title.
   virtual void SetTitle(const str::String & s);


  /// Returns the child Widget.
   virtual Widget * Child() const = 0;

  /// Sets the child Widget, will ushally be a Layout of some kind.
   virtual void SetChild(Widget * w) = 0;


  /// Sets the callback handler for when the window is closed, allows
  /// a decision on if the window dies or not. (Gets a DeathEvent)
  /// Defaults to kill.
   void OnDeath(Callback * cb);


  /// &nbsp;
   cstrconst TypeString() const;
   
   
 protected:
  Callback * onDeath;
};

//------------------------------------------------------------------------------
/// The root of any gui based application, and the third object a gui should
/// make. This acts as a container for a set of windows, and manages particular
/// events, application global stuff etc. It also provides various dialog boxes,
/// for selecting colour, files and general purpose messages/choices.
class EOS_CLASS App : public Base
{
 public:
  /// &nbsp;
   App();
   
  /// &nbsp;
   ~App();


  /// The the last step of setup for a gui app is to call this, enters
  /// the message pump and dosn't exit until the App has Die() called
  /// on it.
   virtual void Go() = 0;
   
  /// A version of Go that runs code each time through the message pump
  /// a callback, useful for doing work such as rendering an animation etc.
  /// Unlike other callback objects which get copied this callback is 
  /// used directly, as there is no oportunity to delete it until the
  /// message pump ends.
   virtual void Go(Callback * cb) = 0;

  /// Makes it exit the message loop, the first step of program termination ushally.
   virtual void Die() = 0;


  /// Adds a window to the application. If show is set to true it will make the
  /// window visible at the same time.
   virtual void Attach(Window * win,bit show = true) = 0;


  /// Sets the callback handler for when the last window is closed, given a
  /// NullEvent, allows a call to Die() to be posted if wanted. The default
  /// behaviour if no callback is defined is to call Die(), so when the last
  /// window goes the Go() method returns and teh application resumes normal
  /// execution.
   void OnLastDeath(Callback * cb);

   
  /// Possible types for a single button dialog.
   enum MsgType {MsgInfo, ///< &nbsp;
                 MsgWarn, ///< &nbsp;
                 MsgErr   ///< &nbsp;
                };

  /// Possible types for a two button dialog.
   enum QuestType {QuestYesNo,    ///< Yes/No choice, yes is true, no is false.
                   QuestOkCancel, ///< Ok/Cancel choice, Ok is true, Cancel is false.
                  };


  /// This creates a single button dialog, to inform the user of something, its modal.
   virtual void MessageDialog(MsgType type,cstrconst msg) = 0;
  
  /// This creates a two button dialog which allows the user to make a choice, returns
  /// true for one for the positive option, false for the other.
   virtual bit ChoiceDialog(QuestType type,cstrconst msg) = 0;
  
  /// Allows a user to select a file to be loaded.
  /// \param title The title of the dialog.
  /// \param exts A comma seperated list of accepted extensions, i.e. "*.bmp,*.png,*.jpg".
  /// \param fn If a file is choosen this is set to contain the filename.
  /// \returns true if a file was choosen, false if it was canceled.
   virtual bit LoadFileDialog(cstrconst title,cstrconst exts,str::String & fn) = 0;

  /// Allows a user to select a file to be saved.
  /// \param title The title of the dialog.
  /// \param fn If a file is choosen this is set to contain the filename, on call this should contain a filename to be the default to use.
  /// \returns true if a file was choosen, false if it was canceled.
   virtual bit SaveFileDialog(cstrconst title,str::String & fn) = 0;


  /// &nbsp;
   cstrconst TypeString() const;


 protected:
  Callback * onLastDeath;
};

//------------------------------------------------------------------------------
 };
};
#endif
