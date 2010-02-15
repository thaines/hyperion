#ifndef EOS_GUI_GTK_WIDGETS_H
#define EOS_GUI_GTK_WIDGETS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines.

/// \file gtk_widgets.h
/// Provides a full set of gtk interfacing widgets that impliment the gui widget
/// set, and the relevent factory for creating them.

#include "eos/types.h"
#include "eos/str/strings.h"
#include "eos/ds/lists.h"
#include "eos/ds/arrays2d.h"
#include "eos/gui/gtk_funcs.h"
#include "eos/gui/widgets.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
class EOS_CLASS HorizontalGtk : public Horizontal
{
 public:
   HorizontalGtk();
  ~HorizontalGtk();

  nat32 GapSize() const;
  void GapSize(nat32 ngs);
  void AttachLeft(Widget * w,bit stretch);
  void AttachRight(Widget * w,bit stretch);
  nat32 Children() const;

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();
  void Detach(Base * child);


 private:
  GtkHBox * box;
  ds::List<Widget*> children;
};

//------------------------------------------------------------------------------
class EOS_CLASS VerticalGtk : public Vertical
{
 public:
   VerticalGtk();
  ~VerticalGtk();

  nat32 GapSize() const;
  void GapSize(nat32 ngs);
  void AttachTop(Widget * w,bit stretch);
  void AttachBottom(Widget * w,bit stretch);
  nat32 Children() const;

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();
  void Detach(Base * child);


 private:
  GtkVBox * box;
  ds::List<Widget*> children;
};

//------------------------------------------------------------------------------
class EOS_CLASS GridGtk : public Grid
{
 public:
   GridGtk();
  ~GridGtk();

  nat32 Cols() const;
  nat32 Rows() const;
  void SetDims(nat32 cols,nat32 rows);
  void GetGapSize(nat32 & xOut,nat32 & yOut) const;
  void SetGapSize(nat32 x,nat32 y);
  void Attach(nat32 col,nat32 row,Widget * w,nat32 width,nat32 height);
  Widget * Get(nat32 x,nat32 y) const;

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();
  void Detach(Base * child);


 private:
  GtkTable * table;
  ds::Array2D<Widget*,mem::MakeNull<Widget*>,mem::KillRelease<Widget> > children;
};

//------------------------------------------------------------------------------
class EOS_CLASS PanelGtk : public Panel
{
 public:
   PanelGtk();
  ~PanelGtk();

  Widget * Child() const;
  void SetChild(Widget * w);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();
  void Detach(Base * child);


 private:
  GtkScrolledWindow * panel;
  Widget * child;
};

//------------------------------------------------------------------------------
class EOS_CLASS FrameGtk : public Frame
{
 public:
   FrameGtk();
  ~FrameGtk();

  void DrawBorder(bit bv);
  void Set(cstrconst label,real32 pos);

  Widget * Child() const;
  void SetChild(Widget * w);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();
  void Detach(Base * child);


 private:
  GtkFrame * frame;
  Widget * child;
};

//------------------------------------------------------------------------------
class EOS_CLASS ExpanderGtk : public Expander
{
 public:
   ExpanderGtk();
  ~ExpanderGtk();

  void Set(cstrconst label);
  void Expand(bit expand);

  Widget * Child() const;
  void SetChild(Widget * w);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();
  void Detach(Base * child);


 private:
  GtkExpander * exp;
  Widget * child;
};

//------------------------------------------------------------------------------
class EOS_CLASS CanvasGtk : public Canvas
{
 public:
   CanvasGtk();
  ~CanvasGtk();

  rend::Pixels & P();
  void Update();
  void Redraw();

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void * Special();


 private:
  struct II : public rend::Pixels
  {
   nat32 Width();
   nat32 Height();
   void Point(const bs::Pos & pos,const bs::ColourRGB & col);
   void Line(const bs::Pos & start,const bs::Pos & end,const bs::ColourRGB & col);
   void Rectangle(const bs::Rect & rect,const bs::ColourRGB & col);
   void Border(const bs::Rect & rect,const bs::ColourRGB & col);
   void Image (const bs::Rect & rect,const bs::Pos & pos,const svt::Field<bs::ColRGB> & image);

   nat32 width;
   nat32 height;
   GtkDrawingArea * can;
   GdkPixmap * img;
   GdkGC * gc;
   GdkCursor * cursor;
  };

  II data;

  // Callbacks...
   static int ExposeEvent(GtkWidget * widget,GdkEventExpose * event,CanvasGtk * self);
   static int ResizeEvent(GtkWidget * widget,GdkEventConfigure * event,CanvasGtk * self);
   static int ClickEvent(GtkWidget * widget,GdkEventButton * event,CanvasGtk * self);
   static int WheelEvent(GtkWidget * widget,GdkEventScroll * event,CanvasGtk * self);
   static int MoveEvent(GtkWidget * widget,GdkEventMotion * event,CanvasGtk * self);
};

//------------------------------------------------------------------------------
class EOS_CLASS LabelGtk : public Label
{
 public:
   LabelGtk();
  ~LabelGtk();

  void Set(cstrconst str);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vi);

  void * Special();


 private:
  GtkLabel * lab;
};

//------------------------------------------------------------------------------
class EOS_CLASS ComboBoxGtk : public ComboBox
{
 public:
   ComboBoxGtk();
  ~ComboBoxGtk();

  nat32 Get() const;
  void Set(nat32 sel);

  void Append(cstrconst s);
  void Prepend(cstrconst s);
  void Insert(cstrconst s,nat32 pos);
  void Delete(nat32 pos);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vi);

  void * Special();


 private:
  // Callbacks...
   static int ChangedEvent(GtkWidget * widget,ComboBoxGtk * self);
 
  GtkComboBox * box;
};

//------------------------------------------------------------------------------
class EOS_CLASS EditBoxGtk : public EditBox
{
 public:
   EditBoxGtk();
  ~EditBoxGtk();

  void Set(cstrconst s);
  const str::String & Get() const;
  void Valid(bit state);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vi);

  void * Special();


 private:
  // Callbacks...
   static int ChangedEvent(GtkWidget * widget,EditBoxGtk * self);

  mutable str::String s;
  GtkEntry * entry;
};

//------------------------------------------------------------------------------
class EOS_CLASS MultilineGtk : public Multiline
{
 public:
   MultilineGtk();
  ~MultilineGtk();

  void Edit(bit can);
  void Empty();
  void Append(cstrconst str);
  void Append(const str::String & str);
  nat32 Lines() const;
  void GetLine(nat32 iter,str::String & out) const;
  void GetAll(str::String & out);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vi);

  void * Special();


 private:
  GtkTextView * view;
  GtkTextBuffer * buf;
};

//------------------------------------------------------------------------------
class EOS_CLASS TickBoxGtk : public TickBox
{
 public:
   TickBoxGtk();
  ~TickBoxGtk();

  Widget * Child() const;
  void SetChild(Widget * w);

  void * Special(); // Returns the gtk object pointer.
  void Detach(Base * child);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);

  void SetState(bit flag);
  bit Ticked() const;


 private:
  GtkToggleButton * but;
  Widget * child;

  static void ChangeEvent(GtkToggleButton * button,void * ptr);
};

//------------------------------------------------------------------------------
class EOS_CLASS ButtonGtk : public Button
{
 public:
   ButtonGtk();
  ~ButtonGtk();

  Widget * Child() const;
  void SetChild(Widget * w);

  void * Special(); // Returns the gtk object pointer.
  void Detach(Base * child);

  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vis);


 private:
  GtkButton * but;
  Widget * child;

  static void ClickEvent(GtkButton * button,void * ptr);
};

//------------------------------------------------------------------------------
class EOS_CLASS ProgressBarGtk : public ProgressBar
{
 public:
   ProgressBarGtk();
  ~ProgressBarGtk();

  void SetDir(Dir dir);

  time::Progress * Begin();
  void End();
  bit Running() const;


  void SetSize(nat32 width,nat32 height);
  nat32 Border() const;
  void SetBorder(nat32 bs);
  void Visible(nat32 vi);

  void * Special();


 private:
  GtkProgressBar * bar;

  bit running;
  nat64 lastUpdate; // To avoid updating it too often.
  void OnChange();
};

//------------------------------------------------------------------------------
class EOS_CLASS WindowGtk : public Window
{
 public:
   WindowGtk();
  ~WindowGtk();

  nat32 Width() const;
  nat32 Height() const;
  void SetSize(nat32 width,nat32 height);

  nat32 Border() const;
  void SetBorder(nat32 bs);

  bit CanResize() const;
  void Resizable(bit s);

  void Minimised(bit mm);
  void Maximised(bit mm);
  void Fullscreen(bit fs);
  void Visible(bit vs);

  void SetTitle(cstrconst title);

  Widget * Child() const;
  void SetChild(Widget * w);

  void * Special(); // Returns the gtk object pointer.
  void Detach(Base * child);


 private:
  GtkWindow * win;
  Widget * child;

  static int  EOS_STDCALL DeleteEvent(GtkWidget * widget,GdkEvent * event,void * ptr);
};

//------------------------------------------------------------------------------
class EOS_CLASS AppGtk : public App
{
 public:
   AppGtk();
  ~AppGtk();

  void Go();
  void Go(Callback * cb);
  void Die();

  void Attach(Window * win,bit show);
  void Detach(Base * win);

  void * Special(); // Returns null.

  void MessageDialog(MsgType type,cstrconst msg);
  bit ChoiceDialog(QuestType type,cstrconst msg);
  bit LoadFileDialog(cstrconst title,cstrconst exts,str::String & fn);
  bit SaveFileDialog(cstrconst title,str::String & fn);


 private:
  ds::List<Window*> mw;

  static int IdleEvent(void * data);
};

//------------------------------------------------------------------------------
/// Gtk object factory, create one of these to make a gtk based gui application.
class EOS_CLASS GtkFactory : public Factory
{
 public:
  /// &nbsp;
   GtkFactory(str::TokenTable & tokTab);

  /// &nbsp;
   ~GtkFactory();

  /// Returns true if the interface type initialised correctly.
   bit Active() const;


  /// &nbsp;
   cstrconst TypeString() const;

 private:
  bit active;

  static Base * NewApp(str::Token,Factory *);
  static Base * NewWindow(str::Token,Factory *);
  static Base * NewProgressBar(str::Token,Factory *);
  static Base * NewTickBox(str::Token,Factory *);
  static Base * NewButton(str::Token,Factory *);
  static Base * NewMultiline(str::Token,Factory *);
  static Base * NewEditBox(str::Token,Factory *);
  static Base * NewComboBox(str::Token,Factory *);
  static Base * NewLabel(str::Token,Factory *);
  static Base * NewCanvas(str::Token,Factory *);
  static Base * NewExpander(str::Token,Factory *);
  static Base * NewFrame(str::Token,Factory *);
  static Base * NewPanel(str::Token,Factory *);
  static Base * NewGrid(str::Token,Factory *);
  static Base * NewVertical(str::Token,Factory *);
  static Base * NewHorizontal(str::Token,Factory *);
};

//------------------------------------------------------------------------------
 };
};
#endif
