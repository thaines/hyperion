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

#include "eos/os/console.h"

#include "eos/io/counter.h"
#include "eos/time/format.h"

#include "eos/file/csv.h"

#ifdef EOS_WIN32
 #include <windows.h>
#else
 #include <ncurses.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <termio.h>
#endif

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
ConversationProg::ConversationProg(class Conversation & c)
:con(c)
{}

void ConversationProg::Begin()
{
 width = con.Cols() - 2;
 pos = 0;

 // Render the progress line...
  con.ClearLine();
  con << "[";
  for (nat32 i=0;i<width;i++) con << " ";
  con << "]\n";

 // Render an empty percentage/step of step line...
  con << "0% 0ns/?ns |\n";

 lastUpdate = eos::time::MilliTime();
 Reset();
}

void ConversationProg::End(bit clear)
{
 con.DeleteLine();
 con.DeleteLine();
 con.ClearLine();

 if (!clear)
 {
  real64 done;
  real64 remaining;
  Time(done,remaining);
  con << "# Finished in " << time::FormatSeconds(done) << "\n";
 }
}

void ConversationProg::OnChange()
{
 nat64 now = eos::time::MilliTime();
 if (now-100<lastUpdate) return;
 lastUpdate = now;
 con.DeleteLine();

 // Discover if we need to re-render the progress bar, if so do it...
  real32 prog = Prog();
  nat32 expPos = nat32(math::RoundUp(real32(width)*prog));
  if (expPos!=pos)
  {
   pos = expPos;
   con.DeleteLine();
   con.ClearLine();

   con << "[";
   for (nat32 i=0;i<pos;i++) con << "#";
   for (nat32 i=pos;i<width;i++) con << " ";
   con << "]\n";
  }
  else
  {
   con.ClearLine();
  }

 // Now render out the percent complete, the time taken/remaining and
 // the highest step of steps indicators for the progress stack...
  io::Counter<io::Text> counter;

  // Render out the percentage complete, time done of total time...
   nat32 percentage = nat32(math::RoundDown(100.0*prog));
   real64 done;
   real64 remaining;
   Time(done,remaining);
   con << percentage << "% " << time::FormatSeconds(done) << "/" << time::FormatSeconds(done+remaining);
   counter << percentage << "% " << time::FormatSeconds(done) << "/" << time::FormatSeconds(done+remaining);

  // Calculate how many step indicators we can fit into the avaliable space...
   nat32 cols = con.Cols();
   counter << " | ";
   nat32 stepsToDo = 0;
   for (int32 i=Depth()-1;i>=0;i--)
   {
    nat32 x,y;
    Part(i,x,y);
    counter << x << "/" << y << ";";
    if (counter.Sum()>cols) break;
    ++stepsToDo;
   }

  // Output the step indicators...
   if (stepsToDo!=0)
   {
    if (stepsToDo==Depth()) con << " | ";
                       else con << " : ";
    for (nat32 i=Depth()-stepsToDo;i<Depth();i++)
    {
     nat32 x,y;
     Part(i,x,y);
     con << x << "/" << y << ";";
    }
   }
   con << "\n";
}

//------------------------------------------------------------------------------
Conversation::Conversation()
:prog(*this)
{}

Conversation::~Conversation()
{}

time::Progress & Conversation::BeginProg()
{
 prog.Begin();
 return prog;
}

void Conversation::EndProg(bit clear)
{
 prog.End(clear);
}

//------------------------------------------------------------------------------
Console::~Console()
{}

LiveText * Console::StartLiveText()
{
 return null<LiveText*>();
}

//------------------------------------------------------------------------------
Con::Con()
:echo(false),indent(0),lcp(0)
{
 for (nat32 i=0;i<lcl;i++) lc[i] = 0;
}

Con::~Con()
{}

Conversation * Con::StartConversation()
{
 return static_cast<Conversation*>(this);
}

void Con::SetColour(bit bright,ConCol fg,ConCol bg)
{
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
  WORD val = 0;

  if (fg|1) val |= FOREGROUND_RED;
  if (fg|2) val |= FOREGROUND_GREEN;
  if (fg|4) val |= FOREGROUND_BLUE;

  if (bg|1) val |= BACKGROUND_RED;
  if (bg|2) val |= BACKGROUND_GREEN;
  if (bg|4) val |= BACKGROUND_BLUE;

  if (bright) val |= FOREGROUND_INTENSITY;

  SetConsoleTextAttribute(hand,val);
 #else
  printf("\033[%i;%i;%im",bright?1:0,30+int(fg),40+int(bg));
 #endif
}

nat32 Con::GetIndent() const
{
 return indent;
}

void Con::Indent(int32 offset)
{
 indent += offset;
}

nat32 Con::Cols() const
{
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(hand,&info);
  return info.dwSize.X-indent-1; // -1 to make behaviour match linux at the loss of a column.
 #else
  struct winsize ws;
  if (ioctl(fileno(stdout),TIOCGWINSZ,&ws)==-1) return 80-indent;
  return ws.ws_col-indent;
 #endif
}

nat32 Con::Col() const
{
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(hand,&info);
  return (info.dwSize.X-indent)*lc[lcp] + info.dwCursorPosition.X - indent;
 #else
  GrabInput();
  printf("\033[6n");

  log::Assert(getchar()=='\033');
  log::Assert(getchar()=='[');

  int col,row;
  scanf("%i",&col);
  log::Assert(getchar()==';');
  scanf("%i",&row);
  log::Assert(getchar()=='R');

  return Cols()*lc[lcp] + col - 1 - indent;
 #endif
}

void Con::Clear(nat32 n)
{
 // *******************************
}

void Con::Delete(nat32 n)
{
 // *******************************
}

void Con::ClearLine()
{
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(hand,&info);
  DWORD temp;
  info.dwCursorPosition.X = 0;
  FillConsoleOutputCharacter(hand,' ',info.dwSize.X,info.dwCursorPosition,&temp);
  info.dwCursorPosition.X = indent;
  SetConsoleCursorPosition(hand,info.dwCursorPosition);
 #else
  printf("\033[2K");
  printf("\033[1G");
 #endif
}

void Con::DeleteLine()
{
 // ******************************************
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(hand,&info);
  DWORD temp;
  info.dwCursorPosition.X = 0;
  FillConsoleOutputCharacter(hand,' ',info.dwSize.X,info.dwCursorPosition,&temp);
  info.dwCursorPosition.X = indent;
  info.dwCursorPosition.Y -= 1;
  SetConsoleCursorPosition(hand,info.dwCursorPosition);
 #else
  printf("\033[2K");
  printf("\033[1A");
  printf("\033[1G");
 #endif
}

void Con::Empty()
{
 GrabInput();
 buf.SetSize(0);
}

void Con::Echo(bit ec)
{
 echo = ec;
}

void Con::WaitKey(cstrchar c)
{

}

void Con::WaitSize(nat32 size)
{

}

bit Con::EOS() const
{
 GrabInput();
 return buf.EOS();
}

nat32 Con::Avaliable() const
{
 GrabInput();
 return buf.Avaliable();
}

nat32 Con::Read(void * out,nat32 bytes)
{
 GrabInput();
 return buf.Read(out,bytes);
}

nat32 Con::Peek(void * out,nat32 bytes) const
{
 GrabInput();
 return buf.Peek(out,bytes);
}

nat32 Con::Skip(nat32 bytes)
{
 GrabInput();
 return buf.Skip(bytes);
}

nat32 Con::Write(const void * in,nat32 bytes)
{
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
  WriteConsole(hand,in,bytes,&bytes,0);
 #else
  fwrite(in,bytes,1,stdout);
 #endif
 return bytes;
}

nat32 Con::Pad(nat32 bytes)
{
 #ifdef EOS_WIN32
  HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
 #endif

 nat32 ret = bytes;
  while (bytes)
  {
   nat32 toDo = math::Min(bytes,nat32(16));
   bytes -= toDo;
   #ifdef EOS_WIN32
    WriteConsole(hand,"                ",toDo,&toDo,0);
   #else
    fwrite("                ",toDo,1,stdout);
   #endif
  }
 return ret;
}

void Con::GrabInput() const
{
 #ifdef EOS_WIN32
  // ***************************************
 #else
  while (true)
  {
   int c = getchar();
   if (c==EOF) break;
   if (echo) putchar(c);
   cstrchar cc = cstrchar(c);
   buf.Write(&cc,1);
  }
 #endif
}

//------------------------------------------------------------------------------
/*ConOS::ConOS()
:indent(0)
{
 initscr();
 start_color();
 noecho();
 keypad(stdscr,TRUE);
 cbreak();
 nodelay(stdscr,TRUE);
 scrollok(stdscr,TRUE);
 idlok(stdscr,TRUE);

 Cursor(false);
 SetColour(false,Black,White);
}

ConOS::~ConOS()
{
 endwin();
}

nat32 ConOS::Write(const void * in,nat32 bytes)
{
 for (nat32 i=0;i<bytes;i++)
 {
  cstrchar c = ((cstrchar*)in)[i];
  switch (c)
  {
   case '\r':
   break;
   case '\n':
   {
    addch('\n');
    for (nat32 i=0;i<indent;i++) addch(' ');
   }
   break;
   default:
    addch(c);
   break;
  }
 }
 refresh();
 return bytes;
}

nat32 ConOS::Pad(nat32 bytes)
{
 for (nat32 i=0;i<bytes;i++) addch(' ');
 refresh();
 return bytes;
}

bit ConOS::EOS() const
{
 GrabInput();
 return buf.EOS();
}

nat32 ConOS::Avaliable() const
{
 GrabInput();
 return buf.Avaliable();
}

nat32 ConOS::Read(void * out,nat32 bytes)
{
 GrabInput();
 return buf.Read(out,bytes);
}

nat32 ConOS::Peek(void * out,nat32 bytes) const
{
 GrabInput();
 return buf.Peek(out,bytes);
}

nat32 ConOS::Skip(nat32 bytes)
{
 GrabInput();
 return buf.Skip(bytes);
}

nat32 ConOS::Width() const
{
 int row,col;
 getmaxyx(stdscr,row,col);
 return col;
}

nat32 ConOS::Height() const
{
 int row,col;
 getmaxyx(stdscr,row,col);
 return row;
}

nat32 ConOS::GetX() const
{
 int x, y;
 getyx(stdscr,x,y);
 return x;
}

nat32 ConOS::GetY() const
{
 int x, y;
 getyx(stdscr,x,y);
 return y;
}

void ConOS::SetPos(nat32 x,nat32 y)
{
 move(y,x);
}

void ConOS::OffsetPos(int32 xo,int32 yo)
{
 int x, y;
 getyx(stdscr,x,y);
 x += xo;
 y += yo;
 move(y,x);
}

void ConOS::SetColour(bit bright,ConCol fg,ConCol bg)
{
 if (bright) attron(A_BOLD);
        else attroff(A_BOLD);
 init_pair(1,fg,bg);
 attron(COLOR_PAIR(1));
}

void ConOS::ClearScreen()
{
 clear();
 refresh();
}

void ConOS::ClearRect(nat32 sx,nat32 sy,nat32 ex,nat32 ey)
{
 int x, y;
 getyx(stdscr,x,y);

 for (nat32 v=sy;v<=ey;v++)
 {
  for (nat32 u=sx;u<=ex;u++)
  {
   mvaddch(v,u,' ');
  }
 }

 move(x,y);
 refresh();
}

void ConOS::ClearLine()
{
 int x, y;
 getyx(stdscr,x,y);
 move(y,0);
 clrtoeol();
 move(y,indent);
 refresh();
}

void ConOS::Clear()
{
 int x, y;
 getyx(stdscr,x,y);
 mvaddch(y,x,' ');
 move(y,x);
 refresh();
}

nat32 ConOS::GetIndent() const
{
 return indent;
}

void ConOS::SetIdent(nat32 ind)
{
 indent = ind;
}

void ConOS::EmptyRB()
{
 GrabInput();
 buf.SetSize(0);
}

void ConOS::Cursor(bit flash)
{
 if (flash) curs_set(1);
       else curs_set(0);
}

void ConOS::Echo(bit ec)
{
 if (ec) echo();
    else noecho();
}

void ConOS::WaitKey(cstrchar c)
{
 // Check the buffer so far...
  GrabInput();
  data::Buffer::Cursor targ = buf.GetCursor();
  while (!targ.EOS())
  {
   cstrchar rc;
   targ.Read(&rc,1);
   if (rc==c) return;
  }

 // If the buffer doesn't allready contain that which we require start delaying...
  nodelay(stdscr,FALSE);
   while (true)
   {
    int val = getch();
    if (val==KEY_BACKSPACE)
    {
     if (buf.Size()!=0) buf.SetSize(buf.Size()-1);
     continue;
    }
    if ((val>0)&&(val<255))
    {
     cstrchar rc = cstrchar(val);
     buf.Write(&rc,1);
     if (rc==c) break;
    }
   }
  nodelay(stdscr,TRUE);
}

void ConOS::WaitSize(nat32 size)
{
 nodelay(stdscr,FALSE);
  while (buf.Size()<size)
  {
   int val = getch();
   if (val==KEY_BACKSPACE)
   {
    if (buf.Size()!=0) buf.SetSize(buf.Size()-1);
    continue;
   }
   if ((val>0)&&(val<255))
   {
    cstrchar c = cstrchar(val);
    buf.Write(&c,1);
   }
  }
 nodelay(stdscr,TRUE);
}

void ConOS::GrabInput() const
{
 while (true)
 {
  int val = getch();
  if (val==ERR) break;
  if (val==KEY_BACKSPACE)
  {
   if (buf.Size()!=0) buf.SetSize(buf.Size()-1);
   continue;
  }
  if ((val>0)&&(val<255))
  {
   cstrchar c = cstrchar(val);
   buf.Write(&c,1);
  }
 }
}*/

//------------------------------------------------------------------------------
 };
};
