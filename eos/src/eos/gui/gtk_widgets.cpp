//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/gui/gtk_widgets.h"

#include "eos/time/format.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace gui
 {
//------------------------------------------------------------------------------
HorizontalGtk::HorizontalGtk()
{
 box = (GtkHBox*)gtk_hbox_new(false,2);
 log::Assert(box);

 gtk_widget_ref(box);
 gtk_widget_show(box);
}

HorizontalGtk::~HorizontalGtk()
{
 if (Parent()) parent->Detach(this);

 while (children.Size()!=0)
 {
  Base * targ = children.Front();
  Detach(targ);
  targ->Release();
 }

 gtk_widget_unref(box);
}

nat32 HorizontalGtk::GapSize() const
{
 return gtk_box_get_spacing(box);
}

void HorizontalGtk::GapSize(nat32 ngs)
{
 gtk_box_set_spacing(box,ngs);
}

void HorizontalGtk::AttachLeft(Widget * w,bit stretch)
{
 children.AddFront(w);
 w->Acquire();
 w->SetParent(this);
 gtk_box_pack_end(box,(GtkWidget*)w->Special(),stretch,true,0);
}

void HorizontalGtk::AttachRight(Widget * w,bit stretch)
{
 children.AddBack(w);
 w->Acquire();
 w->SetParent(this);
 gtk_box_pack_start(box,(GtkWidget*)w->Special(),stretch,true,0);
}

nat32 HorizontalGtk::Children() const
{
 return children.Size();
}

void HorizontalGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(box,width,height);
}

nat32 HorizontalGtk::Border() const
{
 return gtk_container_get_border_width(box);
}

void HorizontalGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(box,bs);
}

void HorizontalGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(box);
     else gtk_widget_hide(box);
}

void * HorizontalGtk::Special()
{
 return box;
}

void HorizontalGtk::Detach(Base * child)
{
 ds::List<Widget*>::Cursor targ = children.FrontPtr();
 while (!targ.Bad())
 {
  if (*targ==child)
  {
   gtk_container_remove(box,(GtkWidget*)child->Special());
   child->SetParent(null<Base*>());
   targ.RemKillNext();
   break;
  }
  ++targ;
 }
}

//------------------------------------------------------------------------------
VerticalGtk::VerticalGtk()
{
 box = (GtkVBox*)gtk_vbox_new(false,2);
 log::Assert(box);

 gtk_widget_ref(box);
 gtk_widget_show(box);
}

VerticalGtk::~VerticalGtk()
{
 if (Parent()) parent->Detach(this);

 while (children.Size()!=0)
 {
  Base * targ = children.Front();
  Detach(targ);
  targ->Release();
 }

 gtk_widget_unref(box);
}

nat32 VerticalGtk::GapSize() const
{
 return gtk_box_get_spacing(box);
}

void VerticalGtk::GapSize(nat32 ngs)
{
 gtk_box_set_spacing(box,ngs);
}

void VerticalGtk::AttachTop(Widget * w,bit stretch)
{
 children.AddFront(w);
 w->Acquire();
 w->SetParent(this);
 gtk_box_pack_end(box,(GtkWidget*)w->Special(),stretch,true,0);
}

void VerticalGtk::AttachBottom(Widget * w,bit stretch)
{
 children.AddBack(w);
 w->Acquire();
 w->SetParent(this);
 gtk_box_pack_start(box,(GtkWidget*)w->Special(),stretch,true,0);
}

nat32 VerticalGtk::Children() const
{
 return children.Size();
}

void VerticalGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(box,width,height);
}

nat32 VerticalGtk::Border() const
{
 return gtk_container_get_border_width(box);
}

void VerticalGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(box,bs);
}

void VerticalGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(box);
     else gtk_widget_hide(box);
}

void * VerticalGtk::Special()
{
 return box;
}

void VerticalGtk::Detach(Base * child)
{
 ds::List<Widget*>::Cursor targ = children.FrontPtr();
 while (!targ.Bad())
 {
  if (*targ==child)
  {
   gtk_container_remove(box,(GtkWidget*)child->Special());
   child->SetParent(null<Base*>());
   targ.RemKillNext();
   break;
  }
  ++targ;
 }
}

//------------------------------------------------------------------------------
GridGtk::GridGtk()
:children(1,1)
{
 table = (GtkTable*)gtk_table_new(1,1,false);
 log::Assert(table);

 gtk_widget_ref(table);
 gtk_widget_show(table);
}

GridGtk::~GridGtk()
{
 if (Parent()) parent->Detach(this);

 for (nat32 y=0;y<children.Height();y++)
 {
  for (nat32 x=0;x<children.Width();x++)
  {
   if (children.Get(x,y)) children.Get(x,y)->SetParent(null<Base*>());
  }
 }

 gtk_widget_unref(table);
}

nat32 GridGtk::Cols() const
{
 return children.Width();
}

nat32 GridGtk::Rows() const
{
 return children.Height();
}

void GridGtk::SetDims(nat32 cols,nat32 rows)
{
 for (nat32 y=0;y<rows;y++)
 {
  for (nat32 x=cols;x<children.Width();x++) if (children.Get(x,y)) children.Get(x,y)->SetParent(null<Base*>());
 }
 for (nat32 y=rows;y<children.Height();y++)
 {
  for (nat32 x=0;x<children.Width();x++) if (children.Get(x,y)) children.Get(x,y)->SetParent(null<Base*>());
 }

 children.Resize(cols,rows);
 gtk_table_resize(table,rows,cols);
}

void GridGtk::GetGapSize(nat32 & xOut,nat32 & yOut) const
{
 xOut = gtk_table_get_col_spacing(table,0);
 yOut = gtk_table_get_row_spacing(table,0);
}

void GridGtk::SetGapSize(nat32 x,nat32 y)
{
 gtk_table_set_col_spacings(table,x);
 gtk_table_set_row_spacings(table,y);
}

void GridGtk::Attach(nat32 col,nat32 row,Widget * w,nat32 width,nat32 height)
{
 if (children.Get(col,row)) children.Get(col,row)->SetParent(null<Base*>());
 children.Get(col,row)->Release();
 children.Get(col,row) = w;
 w->Acquire();
 w->SetParent(this);

 gtk_table_attach(table,(GtkWidget*)w->Special(),col,col+width,row,row+height,GTK_FILL,GTK_FILL,0,0);
}

Widget * GridGtk::Get(nat32 x,nat32 y) const
{
 return children.Get(x,y);
}

void GridGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(table,width,height);
}

nat32 GridGtk::Border() const
{
 return gtk_container_get_border_width(table);
}

void GridGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(table,bs);
}

void GridGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(table);
     else gtk_widget_hide(table);
}

void * GridGtk::Special()
{
 return table;
}

void GridGtk::Detach(Base * child)
{
 for (nat32 y=0;y<children.Height();y++)
 {
  for (nat32 x=0;x<children.Width();x++)
  {
   if (children.Get(x,y)==child)
   {
    gtk_container_remove(table,(GtkWidget*)child->Special());
    children.Get(x,y)->SetParent(null<Base*>());
    children.Get(x,y) = null<Widget*>();
    return;
   }
  }
 }
}

//------------------------------------------------------------------------------
PanelGtk::PanelGtk()
:child(null<Widget*>())
{
 panel = (GtkScrolledWindow*)gtk_scrolled_window_new(0,0);
 log::Assert(panel);

 gtk_widget_ref(panel);
 gtk_widget_show(panel);
 gtk_scrolled_window_set_policy(panel,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
}

PanelGtk::~PanelGtk()
{
 if (Parent()) parent->Detach(this);

 if (child) child->SetParent(null<Base*>());
 child->Release();

 gtk_widget_unref(panel);
}

Widget * PanelGtk::Child() const
{
 return child;
}

void PanelGtk::SetChild(Widget * w)
{
 if (child) child->SetParent(null<Base*>());
 child->Release();
 child = w;
 w->Acquire();
 w->SetParent(this);

 gtk_scrolled_window_add_with_viewport(panel,(GtkWidget*)w->Special());
}

void PanelGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(panel,width,height);
}

nat32 PanelGtk::Border() const
{
 return gtk_container_get_border_width(panel);
}

void PanelGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(panel,bs);
}

void PanelGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(panel);
     else gtk_widget_hide(panel);
}

void * PanelGtk::Special()
{
 return panel;
}

void PanelGtk::Detach(Base * ch)
{
 if (child==ch)
 {
  gtk_container_remove(panel,(GtkWidget*)ch->Special());
  child->SetParent(null<Base*>());
  child = null<Widget*>();
 }
}

//------------------------------------------------------------------------------
FrameGtk::FrameGtk()
:child(null<Widget*>())
{
 frame = (GtkFrame*)gtk_frame_new(null<char*>());
 log::Assert(frame);

 gtk_widget_ref(frame);
 gtk_widget_show(frame);
}

FrameGtk::~FrameGtk()
{
 if (Parent()) parent->Detach(this);

 if (child) child->SetParent(null<Base*>());
 child->Release();

 gtk_widget_unref(frame);
}

void FrameGtk::DrawBorder(bit bv)
{
 if (bv) gtk_frame_set_shadow_type(frame,GTK_SHADOW_ETCHED_IN);
    else gtk_frame_set_shadow_type(frame,GTK_SHADOW_NONE);
}

void FrameGtk::Set(cstrconst label,real32 pos)
{
 gtk_frame_set_label(frame,label);
 gtk_frame_set_label_align(frame,pos,0.75);
}

Widget * FrameGtk::Child() const
{
 return child;
}

void FrameGtk::SetChild(Widget * w)
{
 if (child) child->SetParent(null<Base*>());
 child->Release();
 child = w;
 w->Acquire();
 w->SetParent(this);

 gtk_container_add(frame,(GtkWidget*)w->Special());
}

void FrameGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(frame,width,height);
}

nat32 FrameGtk::Border() const
{
 return gtk_container_get_border_width(frame);
}

void FrameGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(frame,bs);
}

void FrameGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(frame);
     else gtk_widget_hide(frame);
}

void * FrameGtk::Special()
{
 return frame;
}

void FrameGtk::Detach(Base * ch)
{
 if (child==ch)
 {
  gtk_container_remove(frame,(GtkWidget*)ch->Special());
  child->SetParent(null<Base*>());
  child = null<Widget*>();
 }
}

//------------------------------------------------------------------------------
ExpanderGtk::ExpanderGtk()
:child(null<Widget*>())
{
 exp = (GtkExpander*)gtk_expander_new(null<char*>());
 log::Assert(exp);

 gtk_widget_ref(exp);
 gtk_widget_show(exp);
}

ExpanderGtk::~ExpanderGtk()
{
 if (Parent()) parent->Detach(this);

 if (child) child->SetParent(null<Base*>());
 child->Release();

 gtk_widget_unref(exp);
}

void ExpanderGtk::Set(cstrconst label)
{
 gtk_expander_set_label(exp,label);
}

void ExpanderGtk::Expand(bit expand)
{
 gtk_expander_set_expanded(exp,expand);
}

Widget * ExpanderGtk::Child() const
{
 return child;
}

void ExpanderGtk::SetChild(Widget * w)
{
 if (child) child->SetParent(null<Base*>());
 child->Release();
 child = w;
 w->Acquire();
 w->SetParent(this);

 gtk_container_add(exp,(GtkWidget*)w->Special());
}

void ExpanderGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(exp,width,height);
}

nat32 ExpanderGtk::Border() const
{
 return gtk_container_get_border_width(exp);
}

void ExpanderGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(exp,bs);
}

void ExpanderGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(exp);
     else gtk_widget_hide(exp);
}

void * ExpanderGtk::Special()
{
 return exp;
}

void ExpanderGtk::Detach(Base * ch)
{
 if (child==ch)
 {
  gtk_container_remove(exp,(GtkWidget*)ch->Special());
  child->SetParent(null<Base*>());
  child = null<Widget*>();
 }
}

//------------------------------------------------------------------------------
CanvasGtk::CanvasGtk()
{
 data.can = (GtkDrawingArea*)gtk_drawing_area_new();
 log::Assert(data.can);
 data.img = null<GdkPixmap*>();
 data.gc = null<GdkGC*>();
 data.cursor = null<GdkCursor*>();

 gtk_widget_ref(data.can);
 gtk_widget_add_events(data.can,GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
 gtk_signal_connect(data.can,"expose_event",&ExposeEvent,this);
 gtk_signal_connect(data.can,"configure_event",&ResizeEvent,this);
 gtk_signal_connect(data.can,"button_press_event",&ClickEvent,this);
 gtk_signal_connect(data.can,"button_release_event",&ClickEvent,this);
 gtk_signal_connect(data.can,"scroll_event",&WheelEvent,this);
 gtk_signal_connect(data.can,"motion_notify_event",&MoveEvent,this);
 gtk_widget_show(data.can);
}

CanvasGtk::~CanvasGtk()
{
 if (Parent()) parent->Detach(this);
 gtk_widget_unref(data.can);
 if (data.gc) gdk_gc_unref(data.gc);
 if (data.img) gdk_drawable_unref(data.img);
 if (data.cursor) gdk_cursor_unref(data.cursor);
}

rend::Pixels & CanvasGtk::P()
{
 return data;
}

void CanvasGtk::Update()
{
 if (data.cursor==null<GdkCursor*>())
 {
  // Arrange for it to have a crosshair cursor, as accurate selection is probably wanted...
   data.cursor = gdk_cursor_new(34); //34 == Crosshair.
   gdk_cursor_ref(data.cursor);
   if (data.cursor!=null<GdkCursor*>()) gdk_window_set_cursor(data.can->window,data.cursor);
 }

 gtk_widget_queue_draw_area(data.can,0,0,data.width,data.height);
}

void CanvasGtk::Redraw()
{
 if (this->onResize)
 {
  NullEvent e;
  this->onResize->Call(this,&e);
 }
}

void CanvasGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(data.can,width,height);
}

nat32 CanvasGtk::Border() const
{
 return 0;
}

void CanvasGtk::SetBorder(nat32 bs)
{}

void CanvasGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(data.can);
     else gtk_widget_hide(data.can);
}

void * CanvasGtk::Special()
{
 return data.can;
}

nat32 CanvasGtk::II::Width()
{
 return width;
}

nat32 CanvasGtk::II::Height()
{
 return height;
}

void CanvasGtk::II::Point(const bs::Pos & pos,const bs::ColourRGB & col)
{
 GdkColor c;
  c.red   = nat16(col.r*65535.0);
  c.green = nat16(col.g*65535.0);
  c.blue  = nat16(col.b*65535.0);
 gdk_gc_set_rgb_fg_color(gc,&c);

 gdk_draw_point(img,gc,pos[0],pos[1]);
}

void CanvasGtk::II::Line(const bs::Pos & start,const bs::Pos & end,const bs::ColourRGB & col)
{
 GdkColor c;
  c.red   = nat16(col.r*65535.0);
  c.green = nat16(col.g*65535.0);
  c.blue  = nat16(col.b*65535.0);
 gdk_gc_set_rgb_fg_color(gc,&c);

 gdk_draw_line(img,gc,start[0],start[1],end[0],end[1]);
}

void CanvasGtk::II::Rectangle(const bs::Rect & rect,const bs::ColourRGB & col)
{
 GdkColor c;
  c.red   = nat16(col.r*65535.0);
  c.green = nat16(col.g*65535.0);
  c.blue  = nat16(col.b*65535.0);
 gdk_gc_set_rgb_fg_color(gc,&c);

 gdk_draw_rectangle(img,gc,true,rect.low[0],rect.low[1],rect.high[0]-rect.low[0],rect.high[1]-rect.low[1]);
}

void CanvasGtk::II::Border(const bs::Rect & rect,const bs::ColourRGB & col)
{
 GdkColor c;
  c.red   = nat16(col.r*65535.0);
  c.green = nat16(col.g*65535.0);
  c.blue  = nat16(col.b*65535.0);
 gdk_gc_set_rgb_fg_color(gc,&c);

 gdk_draw_rectangle(img,gc,false,rect.low[0],rect.low[1],rect.high[0]-rect.low[0],rect.high[1]-rect.low[1]);
}

void CanvasGtk::II::Image (const bs::Rect & rect,const bs::Pos & pos,const svt::Field<bs::ColRGB> & image)
{
 int width = math::Min<int>(rect.high[0],image.Size(0)) - rect.low[0];
 int height = math::Min<int>(rect.high[1],image.Size(1)) - rect.low[1];
 if ((width<=0)||(height<=0)) return;

 // Two approaches, depending on the field alignment
 // (gdk does not support half the features it should.)...
  if (image.Stride(0)==3)
  {
   gdk_draw_rgb_image(img,gc,pos[0],pos[1],width,height,GDK_RGB_DITHER_NONE,
                      (byte*)&image.Get(rect.low[0],rect.low[1]+height-1),-image.Stride(1));
  }
  else
  {
   byte * mb = mem::Malloc<byte>(3*width*height);
   for (int y=0;y<height;y++)
   {
    for (int x=0;x<width;x++)
    {
     const bs::ColRGB & targ = image.Get(rect.low[0]+x,rect.low[1]+y);
     mb[3*(y*width + x) + 0] = targ.r;
     mb[3*(y*width + x) + 1] = targ.g;
     mb[3*(y*width + x) + 2] = targ.b;
    }
   }
   gdk_draw_rgb_image(img,gc,pos[0],pos[1],width,height,GDK_RGB_DITHER_NONE,mb+width*3*(height-1),-width*3);
   mem::Free(mb);
  }
}


int CanvasGtk::ExposeEvent(GtkWidget * widget,GdkEventExpose * event,CanvasGtk * self)
{
 if (self->data.gc==null<GdkGC*>()) return false;
 gdk_draw_drawable(self->data.can->window,self->data.gc,self->data.img,
		   event->area.x,event->area.y,event->area.x,event->area.y,event->area.width, event->area.height);
 return false;
}

int CanvasGtk::ResizeEvent(GtkWidget * widget,GdkEventConfigure * event,CanvasGtk * self)
{
 if (self->data.img) gdk_drawable_unref(self->data.img);
 self->data.img = (GdkPixmap*)gdk_pixmap_new(widget->window,event->width,event->height,-1);
 log::Assert(self->data.img);
 gdk_drawable_ref(self->data.img);

 if (self->data.gc)  gdk_gc_unref(self->data.gc);
 self->data.gc = gdk_gc_new(self->data.img);
 log::Assert(self->data.gc);
 gdk_gc_ref(self->data.gc);

 self->data.width  = event->width;
 self->data.height = event->height;

 if (self->onResize)
 {
  NullEvent e;
  self->onResize->Call(self,&e);
 }
 return true;
}

int CanvasGtk::ClickEvent(GtkWidget * widget,GdkEventButton * event,CanvasGtk * self)
{
 if (self->onClick)
 {
  MouseButtonEvent e;
   e.button = (event->button==1)?MouseButtonEvent::LMB:((event->button==2)?MouseButtonEvent::MMB:MouseButtonEvent::RMB);
   e.x = int32(event->x);
   e.y = int32(event->y);
   e.down = (event->type!=GDK_BUTTON_RELEASE);
  self->onClick->Call(self,&e);
  return true;
 } else return false;
}

int CanvasGtk::WheelEvent(GtkWidget * widget,GdkEventScroll * event,CanvasGtk * self)
{
 if (self->onWheel)
 {
  MouseWheelEvent e;
   e.x = int32(event->x);
   e.y = int32(event->y);
   switch (event->direction)
   {
    case GDK_SCROLL_UP:    e.deltaX = 0;  e.deltaY = 1;  break;
    case GDK_SCROLL_DOWN:  e.deltaX = 0;  e.deltaY = -1; break;
    case GDK_SCROLL_LEFT:  e.deltaX = -1; e.deltaY = 0;  break;
    case GDK_SCROLL_RIGHT: e.deltaX = 1;  e.deltaY = 0;  break;
   }
  self->onWheel->Call(self,&e);
  return true;
 } else return false;
}

int CanvasGtk::MoveEvent(GtkWidget * widget,GdkEventMotion * event,CanvasGtk * self)
{
 if (self->onMove)
 {
  MouseMoveEvent e;
   e.x = int32(event->x);
   e.y = int32(event->y);
   e.lmb = event->state & GDK_BUTTON1_MASK;
   e.rmb = event->state & GDK_BUTTON2_MASK;
   e.mmb = event->state & GDK_BUTTON3_MASK;
  self->onMove->Call(self,&e);
  return true;
 } else return false;
}

//------------------------------------------------------------------------------
LabelGtk::LabelGtk()
{
 lab = (GtkLabel*)gtk_label_new("");
 log::Assert(lab);
 gtk_widget_ref(lab);
 gtk_widget_show(lab);
}

LabelGtk::~LabelGtk()
{
 if (Parent()) parent->Detach(this);
 gtk_widget_unref(lab);
}

void LabelGtk::Set(cstrconst str)
{
 gtk_label_set_text(lab,str);
}

void LabelGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(lab,width,height);
}

nat32 LabelGtk::Border() const
{
 return 0;
}

void LabelGtk::SetBorder(nat32)
{}

void LabelGtk::Visible(nat32 vi)
{
 if (vi) gtk_widget_show(lab);
    else gtk_widget_hide(lab);
}

void * LabelGtk::Special()
{
 return lab;
}

//------------------------------------------------------------------------------
ComboBoxGtk::ComboBoxGtk()
{
 box = (GtkComboBox*)gtk_combo_box_new_text();
 log::Assert(box);
 gtk_widget_ref(box);
 gtk_widget_show(box);
 
 gtk_signal_connect(box,"changed",&ChangedEvent,this);
}

ComboBoxGtk::~ComboBoxGtk()
{
 if (Parent()) parent->Detach(this);
 gtk_widget_unref(box);
}

nat32 ComboBoxGtk::Get() const
{
 return gtk_combo_box_get_active(box);
}

void ComboBoxGtk::Set(nat32 sel)
{
 gtk_combo_box_set_active(box,sel);
}

void ComboBoxGtk::Append(cstrconst s)
{
 gtk_combo_box_append_text(box,s);
}

void ComboBoxGtk::Prepend(cstrconst s)
{
 gtk_combo_box_prepend_text(box,s);
}

void ComboBoxGtk::Insert(cstrconst s,nat32 pos)
{
 gtk_combo_box_insert_text(box,pos,s);
}

void ComboBoxGtk::Delete(nat32 pos)
{
 gtk_combo_box_remove_text(box,pos);
}

void ComboBoxGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(box,width,height);
}

nat32 ComboBoxGtk::Border() const
{
 return 0;
}

void ComboBoxGtk::SetBorder(nat32 bs)
{}

void ComboBoxGtk::Visible(nat32 vi)
{
 if (vi) gtk_widget_show(box);
    else gtk_widget_hide(box);
}

void * ComboBoxGtk::Special()
{
 return box;
}

int ComboBoxGtk::ChangedEvent(GtkWidget * widget,ComboBoxGtk * self)
{
 if (self->onChange)
 {
  NullEvent e;
  self->onChange->Call(self,&e);
  return true;
 } else return false;
}

//------------------------------------------------------------------------------
EditBoxGtk::EditBoxGtk()
{
 entry = (GtkEntry*)gtk_entry_new();
 log::Assert(entry);
 gtk_widget_ref(entry);
 gtk_widget_show(entry);
 Valid(true);

 gtk_signal_connect(entry,"changed",&ChangedEvent,this);
}

EditBoxGtk::~EditBoxGtk()
{
 if (Parent()) parent->Detach(this);
 gtk_widget_unref(entry);
}

void EditBoxGtk::Set(cstrconst s)
{
 gtk_entry_set_text(entry,s);
}

const str::String & EditBoxGtk::Get() const
{
 s = gtk_entry_get_text(entry);
 return s;
}

void EditBoxGtk::Valid(bit state)
{
 GdkColor color;
 if (state)
 {
  color.red = 65535;
  color.green = 65535;
  color.blue = 65535;
 }
 else
 {
  color.red = 65535;
  color.green = 17768;
  color.blue = 17768;
 }
 gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&color);
 gtk_widget_modify_base(entry,GTK_STATE_ACTIVE,&color);
 gtk_widget_modify_base(entry,GTK_STATE_PRELIGHT,&color);
 gtk_widget_modify_base(entry,GTK_STATE_SELECTED,&color);
}

void EditBoxGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(entry,width,height);
}

nat32 EditBoxGtk::Border() const
{
 return 0;
}

void EditBoxGtk::SetBorder(nat32 bs)
{}

void EditBoxGtk::Visible(nat32 vi)
{
 if (vi) gtk_widget_show(entry);
    else gtk_widget_hide(entry);
}

void * EditBoxGtk::Special()
{
 return entry;
}

int EditBoxGtk::ChangedEvent(GtkWidget * widget,EditBoxGtk * self)
{
 if (self->onChange)
 {
  NullEvent e;
  self->onChange->Call(self,&e);
  return true;
 } else return false;
}

//------------------------------------------------------------------------------
MultilineGtk::MultilineGtk()
{
 buf = gtk_text_buffer_new(0);
 log::Assert(buf);
 g_object_ref(buf);

 view = (GtkTextView*)gtk_text_view_new_with_buffer(buf);
 log::Assert(view);
 gtk_widget_ref(view);
 gtk_widget_show(view);
}

MultilineGtk::~MultilineGtk()
{
 if (Parent()) parent->Detach(this);
 gtk_widget_unref(view);
 g_object_unref(buf);
}

void MultilineGtk::Edit(bit can)
{
 gtk_text_view_set_editable(view,can);
}

void MultilineGtk::Empty()
{
 GtkTextIter start;
 GtkTextIter end;

 gtk_text_buffer_get_start_iter(buf,&start);
 gtk_text_buffer_get_end_iter(buf,&end);

 gtk_text_buffer_delete(buf,&start,&end);
}

void MultilineGtk::Append(cstrconst str)
{
 GtkTextIter end;
 gtk_text_buffer_get_end_iter(buf,&end);

 gtk_text_buffer_insert(buf,&end,str,-1);
}

void MultilineGtk::Append(const str::String & str)
{
 cstr s = str.ToStr();
 Append(s);
 mem::Free(s);
}

nat32 MultilineGtk::Lines() const
{
 return gtk_text_buffer_get_line_count(buf);
}

void MultilineGtk::GetLine(nat32 iter,str::String & out) const
{
 GtkTextIter start;
 GtkTextIter end;

 gtk_text_buffer_get_iter_at_line(buf,&start,iter);
 if ((iter+1)==Lines()) gtk_text_buffer_get_end_iter(buf,&end);
                   else gtk_text_buffer_get_iter_at_line(buf,&start,iter+1);

 cstr str = gtk_text_buffer_get_text(buf,&start,&end,false);
 out = str;
 g_free(str);
}

void MultilineGtk::GetAll(str::String & out)
{
 GtkTextIter start;
 GtkTextIter end;

 gtk_text_buffer_get_start_iter(buf,&start);
 gtk_text_buffer_get_end_iter(buf,&end);

 cstr str = gtk_text_buffer_get_text(buf,&start,&end,false);
 out = str;
 g_free(str);
}

void MultilineGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(view,width,height);
}

nat32 MultilineGtk::Border() const
{return 0;}

void MultilineGtk::SetBorder(nat32 bs)
{}

void MultilineGtk::Visible(nat32 vi)
{
 if (vi) gtk_widget_show(view);
    else gtk_widget_hide(view);
}

void * MultilineGtk::Special()
{
 return view;
}

//------------------------------------------------------------------------------
TickBoxGtk::TickBoxGtk()
:child(null<Widget*>())
{
 but = (GtkToggleButton*)gtk_check_button_new();
 log::Assert(but);
 gtk_widget_ref(but);
 gtk_widget_show(but);
 gtk_signal_connect(but,"toggled",ChangeEvent,this);
}

TickBoxGtk::~TickBoxGtk()
{
 if (Parent()) parent->Detach(this);

 if (child)
 {
  child->SetParent(null<Base*>());
  child->Release();
 }

 gtk_widget_unref(but);
}

Widget * TickBoxGtk::Child() const
{
 return child;
}

void TickBoxGtk::SetChild(Widget * w)
{
 if (child) child->SetParent(null<Base*>());
 child->Release();
 child = w;
 w->Acquire();
 w->SetParent(this);

 gtk_container_add(but,(GtkWidget*)w->Special());
}

void * TickBoxGtk::Special()
{
 return but;
}

void TickBoxGtk::Detach(Base * ch)
{
 if (child==ch)
 {
  gtk_container_remove(but,(GtkWidget*)ch->Special());
  child->SetParent(null<Base*>());
  child = null<Widget*>();
 }
}

void TickBoxGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(but,width,height);
}

nat32 TickBoxGtk::Border() const
{
 return gtk_container_get_border_width(but);
}

void TickBoxGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(but,bs);
}

void TickBoxGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(but);
     else gtk_widget_hide(but);
}

void TickBoxGtk::SetState(bit flag)
{
 gtk_toggle_button_set_active(but,flag);
}

bit TickBoxGtk::Ticked() const
{
 return gtk_toggle_button_get_active(but);
}

void TickBoxGtk::ChangeEvent(GtkToggleButton * check_button,void * ptr)
{
 TickBoxGtk * self = (TickBoxGtk*)ptr;
 NullEvent e;
 if (self->onChange) self->onChange->Call(self,&e);
}

//------------------------------------------------------------------------------
ButtonGtk::ButtonGtk()
:child(null<Widget*>())
{
 but = (GtkButton*)gtk_button_new();
 log::Assert(but);
 gtk_widget_ref(but);
 gtk_widget_show(but);
 gtk_signal_connect(but,"clicked",ClickEvent,this);
}

ButtonGtk::~ButtonGtk()
{
 if (Parent()) parent->Detach(this);

 if (child)
 {
  child->SetParent(null<Base*>());
  child->Release();
 }

 gtk_widget_unref(but);
}

Widget * ButtonGtk::Child() const
{
 return child;
}

void ButtonGtk::SetChild(Widget * w)
{
 if (child) child->SetParent(null<Base*>());
 child->Release();
 child = w;
 w->Acquire();
 w->SetParent(this);

 gtk_container_add(but,(GtkWidget*)w->Special());
}

void * ButtonGtk::Special()
{
 return but;
}

void ButtonGtk::Detach(Base * ch)
{
 if (child==ch)
 {
  gtk_container_remove(but,(GtkWidget*)ch->Special());
  child->SetParent(null<Base*>());
  child = null<Widget*>();
 }
}

void ButtonGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(but,width,height);
}

nat32 ButtonGtk::Border() const
{
 return gtk_container_get_border_width(but);
}

void ButtonGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(but,bs);
}

void ButtonGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(but);
     else gtk_widget_hide(but);
}

void ButtonGtk::ClickEvent(GtkButton * button,void * ptr)
{
 ButtonGtk * self = (ButtonGtk*)ptr;
 NullEvent e;
 if (self->onClick) self->onClick->Call(self,&e);
}

//------------------------------------------------------------------------------
ProgressBarGtk::ProgressBarGtk()
:running(false)
{
 bar = (GtkProgressBar*)gtk_progress_bar_new();
 log::Assert(bar);
 gtk_widget_ref(bar);
 gtk_widget_show(bar);

 // This fixes a really strange bug where the progress bar doesn't work the first time.
  Begin();
  End();

 gtk_progress_bar_set_text(bar,"");
 gtk_progress_bar_set_fraction(bar,0.0);
}

ProgressBarGtk::~ProgressBarGtk()
{
 if (Parent()) parent->Detach(this);
 gtk_widget_unref(bar);
}

void ProgressBarGtk::SetDir(Dir dir)
{
 switch (dir)
 {
  case North: gtk_progress_bar_set_orientation(bar,GTK_PROGRESS_BOTTOM_TO_TOP);
  case East:  gtk_progress_bar_set_orientation(bar,GTK_PROGRESS_LEFT_TO_RIGHT);
  case South: gtk_progress_bar_set_orientation(bar,GTK_PROGRESS_TOP_TO_BOTTOM);
  case West:  gtk_progress_bar_set_orientation(bar,GTK_PROGRESS_RIGHT_TO_LEFT);
 }
}

time::Progress * ProgressBarGtk::Begin()
{
 running = true;
 Reset();
 gtk_progress_bar_set_text(bar,"Starting...");
 gtk_progress_bar_set_fraction(bar,0.0);
 return static_cast<time::Progress*>(this);
}

void ProgressBarGtk::End()
{
 running = false;
 lastUpdate = 0;

 real64 done;
 real64 remaining;
 Time(done,remaining);

 str::String s;
 s << time::FormatSeconds(done);

 cstr cs = s.ToStr();
 gtk_progress_bar_set_text(bar,cs);
 mem::Free(cs);

 gtk_progress_bar_set_fraction(bar,0.0);
}

bit ProgressBarGtk::Running() const
{
 return running;
}

void ProgressBarGtk::SetSize(nat32 width,nat32 height)
{
 gtk_widget_set_size_request(bar,width,height);
}

nat32 ProgressBarGtk::Border() const
{return 1;}

void ProgressBarGtk::SetBorder(nat32 bs)
{}

void ProgressBarGtk::Visible(nat32 vis)
{
 if (vis) gtk_widget_show(bar);
     else gtk_widget_hide(bar);
}

void * ProgressBarGtk::Special()
{
 return bar;
}

void ProgressBarGtk::OnChange()
{
 LogTime("eos::gui::ProgressBarGtk::OnChange");
 // Cap updates to 5 times a second...
  nat64 now = eos::time::MilliTime();
  if (now-200<lastUpdate) return;
  lastUpdate = now;


 // Update the percent complete...
  real32 prog = Prog();
  gtk_progress_bar_set_fraction(bar,prog);


 // Create a new text string...
  str::String s;
  // The percentage complete, time done of total time...
   nat32 percentage = nat32(math::RoundDown(100.0*prog));
   real64 done;
   real64 remaining;
   Time(done,remaining);
   s << percentage << "% " << time::FormatSeconds(done) << "/" << time::FormatSeconds(done+remaining) << " | ";

  // Step indicators...
   for (nat32 i=0;i<Depth();i++)
   {
    nat32 x,y;
    Part(i,x,y);
    s << x << "/" << y << ";";
   }
  // Set...
   cstr cs = s.ToStr();
   gtk_progress_bar_set_text(bar,cs);
   mem::Free(cs);


 // Make sure the screen gets refreshed...
  while (gtk_events_pending()) gtk_main_iteration();
}

//------------------------------------------------------------------------------
WindowGtk::WindowGtk()
:child(null<Widget*>())
{
 win = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
 log::Assert(win);
 gtk_widget_ref(win);
 gtk_signal_connect(win,"delete_event",DeleteEvent,this);
}

WindowGtk::~WindowGtk()
{
 if (Parent()) Parent()->Detach(this);

 if (child) child->SetParent(null<Base*>());
 child->Release();

 gtk_widget_unref(win);
}

nat32 WindowGtk::Width() const
{
 int w,h;
 gtk_window_get_size(win,&w,&h);
 return w;
}

nat32 WindowGtk::Height() const
{
 int w,h;
 gtk_window_get_size(win,&w,&h);
 return h;
}

void WindowGtk::SetSize(nat32 width,nat32 height)
{
 gtk_window_resize(win,width,height);
}

nat32 WindowGtk::Border() const
{
 return gtk_container_get_border_width(win);
}

void WindowGtk::SetBorder(nat32 bs)
{
 gtk_container_set_border_width(win,bs);
}

bit WindowGtk::CanResize() const
{
 return gtk_window_get_resizable(win);
}

void WindowGtk::Resizable(bit s)
{
 gtk_window_set_resizable(win,s);
}

void WindowGtk::Minimised(bit mm)
{
 if (mm) gtk_window_iconify(win);
    else gtk_window_deiconify(win);
}

void WindowGtk::Maximised(bit mm)
{
 if (mm) gtk_window_maximize(win);
    else gtk_window_unmaximize(win);
}

void WindowGtk::Fullscreen(bit fs)
{
 if (fs) gtk_window_fullscreen(win);
    else gtk_window_unfullscreen(win);
}

void WindowGtk::Visible(bit vs)
{
 if (vs)
 {
  if (Parent()) gtk_widget_show(win);
 }
 else gtk_widget_hide(win);
}

void WindowGtk::SetTitle(cstrconst title)
{
 gtk_window_set_title(win,title);
}

Widget * WindowGtk::Child() const
{
 return child;
}

void WindowGtk::SetChild(Widget * w)
{
 if (child) child->SetParent(null<Base*>());
 child->Release();
 child = w;
 w->Acquire();
 w->SetParent(this);

 gtk_container_add(win,(GtkWidget*)w->Special());
}

void * WindowGtk::Special()
{
 return win;
}

void WindowGtk::Detach(Base * ch)
{
 if (child==ch)
 {
  gtk_container_remove(win,(GtkWidget*)ch->Special());
  child->SetParent(null<Base*>());
  child = null<Widget*>();
 }
}

int EOS_STDCALL WindowGtk::DeleteEvent(GtkWidget * widget,GdkEvent * event,void * ptr)
{
 WindowGtk * self = (WindowGtk*)ptr;

 if (self->onDeath)
 {
  DeathEvent e;
  self->onDeath->Call(self,&e);
  if (e.doDeath) delete self;
 }
 else delete self;

 return true;
}

//------------------------------------------------------------------------------
AppGtk::AppGtk()
{}

AppGtk::~AppGtk()
{
 ds::List<Window*>::Cursor targ = mw.FrontPtr();
 while (!targ.Bad())
 {
  (*targ)->Visible(false);
  (*targ)->SetParent(null<Base*>());
  (*targ)->Release();
  ++targ;
 }
}

void AppGtk::Go()
{
 gtk_main();
}

void AppGtk::Go(Callback * cb)
{
 gtk_idle_add(&IdleEvent,cb);
 gtk_main();
}

void AppGtk::Die()
{
 gtk_main_quit();
}

void AppGtk::Attach(Window * win,bit show)
{
 mw.AddBack(win);
 win->Acquire();
 win->SetParent(this);
 if (show) win->Visible(true);
}

void AppGtk::Detach(Base * ch)
{
 WindowGtk * win = (WindowGtk*)ch;
 win->Visible(false);

 ds::List<Window*>::Cursor targ = mw.FrontPtr();
 while (!targ.Bad())
 {
  if (*targ==win)
  {
   targ.RemKillNext();
   win->SetParent(null<Base*>());
   break;
  }
  ++targ;
 }

 if (mw.Size()==0)
 {
  if (onLastDeath)
  {
   NullEvent e;
   onLastDeath->Call(this,&e);
  }
  else Die();
 }
}

void * AppGtk::Special()
{
 return null<void*>();
}

void AppGtk::MessageDialog(MsgType type,cstrconst msg)
{
 GtkMessageType t;
 switch (type)
 {
  case MsgInfo: t = GTK_MESSAGE_INFO; break;
  case MsgWarn: t = GTK_MESSAGE_WARNING; break;
  default:	t = GTK_MESSAGE_ERROR; break;
 }

 GtkMessageDialog * dialog = (GtkMessageDialog*)gtk_message_dialog_new(0,GTK_DIALOG_MODAL ,t,GTK_BUTTONS_CLOSE,msg);
 gtk_dialog_run(dialog);
 gtk_widget_destroy(dialog);
}

bit AppGtk::ChoiceDialog(QuestType type,cstrconst msg)
{
 GtkButtonsType b;
 if (type==QuestYesNo) b = GTK_BUTTONS_YES_NO;
                  else b = GTK_BUTTONS_OK_CANCEL;

 GtkMessageDialog * dialog = (GtkMessageDialog*)gtk_message_dialog_new(0,GTK_DIALOG_MODAL ,GTK_MESSAGE_QUESTION,b,msg);
 int res = gtk_dialog_run(dialog);
 gtk_widget_destroy(dialog);

 return (res==GTK_RESPONSE_OK) || (res==GTK_RESPONSE_YES);
}

bit AppGtk::LoadFileDialog(cstrconst title,cstrconst exts,str::String & fn)
{
 GtkFileChooser * dialog;
 dialog = (GtkFileChooser*)gtk_file_chooser_dialog_new(title,0,GTK_FILE_CHOOSER_ACTION_OPEN,
			                               "Cancel",GTK_RESPONSE_CANCEL,"Load",GTK_RESPONSE_ACCEPT,NULL);	
 if (exts)
 {
  GtkFileFilter * filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter,exts);
  cstrchar buf[32];
  cstrconst targ = exts;
  while (*targ!=0)
  {
   nat32 i;
   for (i=0;i<31;i++) {if ((*targ==0)||(*targ==',')) break; buf[i] = *targ; ++targ;}
   if (*targ!=0) ++targ;
   buf[i] = 0;

   gtk_file_filter_add_pattern(filter,buf);
  }
  gtk_file_chooser_add_filter(dialog,filter);
 }

 bit ret = gtk_dialog_run(dialog)==GTK_RESPONSE_ACCEPT;
 if (ret)
 {
  cstr filename = gtk_file_chooser_get_filename(dialog);
   fn = filename;
  g_free(filename);
 }

 gtk_widget_destroy (dialog);
 return ret;
}

bit AppGtk::SaveFileDialog(cstrconst title,str::String & fn)
{
 GtkFileChooser * dialog;
 dialog = (GtkFileChooser*)gtk_file_chooser_dialog_new(title,0,GTK_FILE_CHOOSER_ACTION_SAVE,
			                               "Cancel",GTK_RESPONSE_CANCEL,"Save",GTK_RESPONSE_ACCEPT,NULL);

 cstr fnIn = fn.ToStr();
  gtk_file_chooser_set_current_name(dialog,fnIn);
 mem::Free(fnIn);

 bit ret = gtk_dialog_run(dialog)==GTK_RESPONSE_ACCEPT;
 if (ret)
 {
  cstr filename = gtk_file_chooser_get_filename(dialog);
   fn = filename;
  g_free(filename);
 }

 gtk_widget_destroy(dialog);
 return ret;
}

int AppGtk::IdleEvent(void * data)
{
 ((Callback*)data)->Call(null<Base*>(),null<Event*>());
 return 0;
}

//------------------------------------------------------------------------------
GtkFactory::GtkFactory(str::TokenTable & tokTab)
:Factory(tokTab)
{
 active = LoadGtk();
 if (Active())
 {
  Register(tokTab("eos::gui::App"),&NewApp,this);
  Register(tokTab("App"),&NewApp,this);
  Register(tokTab("eos::gui::Window"),&NewWindow,this);
  Register(tokTab("Window"),&NewWindow,this);
  Register(tokTab("eos::gui::ProgressBar"),&NewProgressBar,this);
  Register(tokTab("ProgressBar"),&NewProgressBar,this);
  Register(tokTab("eos::gui::TickBox"),&NewTickBox,this);
  Register(tokTab("TickBox"),&NewTickBox,this);
  Register(tokTab("eos::gui::Button"),&NewButton,this);
  Register(tokTab("Button"),&NewButton,this);
  Register(tokTab("eos::gui::Multiline"),&NewMultiline,this);
  Register(tokTab("Multiline"),&NewMultiline,this);
  Register(tokTab("eos::gui::EditBox"),&NewEditBox,this);
  Register(tokTab("EditBox"),&NewEditBox,this);
  Register(tokTab("eos::gui::ComboBox"),&NewComboBox,this);
  Register(tokTab("ComboBox"),&NewComboBox,this);
  Register(tokTab("eos::gui::Label"),&NewLabel,this);
  Register(tokTab("Label"),&NewLabel,this);
  Register(tokTab("eos::gui::Canvas"),&NewCanvas,this);
  Register(tokTab("Canvas"),&NewCanvas,this);
  Register(tokTab("eos::gui::Expander"),&NewExpander,this);
  Register(tokTab("Expander"),&NewExpander,this);
  Register(tokTab("eos::gui::Frame"),&NewFrame,this);
  Register(tokTab("Frame"),&NewFrame,this);
  Register(tokTab("eos::gui::Panel"),&NewPanel,this);
  Register(tokTab("Panel"),&NewPanel,this);
  Register(tokTab("eos::gui::Grid"),&NewGrid,this);
  Register(tokTab("Grid"),&NewGrid,this);
  Register(tokTab("eos::gui::Vertical"),&NewVertical,this);
  Register(tokTab("Vertical"),&NewVertical,this);
  Register(tokTab("eos::gui::Horizontal"),&NewHorizontal,this);
  Register(tokTab("Horizontal"),&NewHorizontal,this);
 }
}

GtkFactory::~GtkFactory()
{}

bit GtkFactory::Active() const
{
 return active;
}

cstrconst GtkFactory::TypeString() const
{
 return "eos::gui::GtkFactory";
}

Base * GtkFactory::NewApp(str::Token,Factory *)
{return new AppGtk();}

Base * GtkFactory::NewWindow(str::Token,Factory *)
{return new WindowGtk();}

Base * GtkFactory::NewProgressBar(str::Token,Factory *)
{return new ProgressBarGtk();}

Base * GtkFactory::NewTickBox(str::Token,Factory *)
{return new TickBoxGtk();}

Base * GtkFactory::NewButton(str::Token,Factory *)
{return new ButtonGtk();}

Base * GtkFactory::NewMultiline(str::Token,Factory *)
{return new MultilineGtk();}

Base * GtkFactory::NewEditBox(str::Token,Factory *)
{return new EditBoxGtk();}

Base * GtkFactory::NewComboBox(str::Token,Factory *)
{return new ComboBoxGtk();}

Base * GtkFactory::NewLabel(str::Token,Factory *)
{return new LabelGtk();}

Base * GtkFactory::NewCanvas(str::Token,Factory *)
{return new CanvasGtk();}

Base * GtkFactory::NewExpander(str::Token,Factory *)
{return new ExpanderGtk();}

Base * GtkFactory::NewFrame(str::Token,Factory *)
{return new FrameGtk();}

Base * GtkFactory::NewPanel(str::Token,Factory *)
{return new PanelGtk();}

Base * GtkFactory::NewGrid(str::Token,Factory *)
{return new GridGtk();}

Base * GtkFactory::NewVertical(str::Token,Factory *)
{return new VerticalGtk();}

Base * GtkFactory::NewHorizontal(str::Token,Factory *)
{return new HorizontalGtk();}

//------------------------------------------------------------------------------
 };
};
