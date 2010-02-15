#ifndef EOS_OS_CONSOLE_H
#define EOS_OS_CONSOLE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file console.h
/// Provides an interface to the command line of the running program, abstracts
/// all the weird idiosynchracities of the current terminal.

#include "eos/types.h"
#include "eos/io/inout.h"
#include "eos/time/progress.h"
#include "eos/data/buffers.h"

namespace eos
{
 namespace os
 {
//------------------------------------------------------------------------------
/// An enum of console colours, for making things look preity.
enum ConCol {Black = 0,   ///< &nbsp;
             Red = 1,     ///< &nbsp;
             Green = 2,   ///< &nbsp;
             Yellow = 3,  ///< &nbsp;
             Blue = 4,    ///< &nbsp;
             Magenta = 5, ///< &nbsp;
             Cyan = 6,    ///< &nbsp;
             White = 7    ///< &nbsp;
            };

//------------------------------------------------------------------------------
// The progress bar used by the below classes...
class EOS_CLASS ConversationProg : public time::Progress
{
 public:
  ConversationProg(class Conversation & con);

  void Begin();
  void End(bit clear);

  void OnChange();


 private:
  class Conversation & con;

  nat32 pos; // How many characters of the bar are filled in, so we only redraw it when necesary.
  nat32 width; // The width that has been decided on for the bar.

  nat64 lastUpdate; // When the console was last updated, so we don't waste time by doing it too often.
};

//------------------------------------------------------------------------------
/// This represents the most common console usage pattern, as a conversation
/// which alternates between the program writting lines to the console whilst
/// the user waits and the user entering input whilst the program waits.
///
/// Note that updates to the screen only have to happen after a newline -
/// partialy written lines can not be presumed visible.
///
/// Provides an indentation model and distinguishes between new lines that are
/// created due to word wrap and new lines created due to writting newlines to
/// the stream. This effects line deletion, so it deletes back to the most
/// recent newline.
///
/// The IO system uses a buffer into which all input is read, you can wait for
/// various events to happen to the buffer, such as the user hitting enter to
/// indicate they have finished entering input. The buffer understands and
/// handles the delete key automatically, but only if you have not allready
/// read the data out of it.
class EOS_CLASS Conversation : public io::InOutVirt<io::Text>
{
 public:
  /// &nbsp;
   Conversation();

  /// &nbsp;
   ~Conversation();


  /// Sets the colour of the output, foreground and background.
  /// For beautification, not guaranteed to work so do not rely on it.
  /// Defaults to whatever the operating system in question considers to be default.
   virtual void SetColour(bit bright,ConCol fg,ConCol bg) = 0;


  /// This returns the currently set indent. This is an offset for all lines
  /// from the left hand side of the screen. Useful for laying out certain
   virtual nat32 GetIndent() const = 0;

  /// This offsets the indent from the current position.
   virtual void Indent(int32 offset) = 0;


  /// Returns the width of each console line. This does not include space that
  /// has been removed by the indent.
   virtual nat32 Cols() const = 0;

  /// Returns the current write position of the cursor, relative to the start
  /// of the line. The start of the line is relative to the indent position.
   virtual nat32 Col() const = 0;


  /// Clears a given number of characters, will not clear past the start of the current line.
   virtual void Clear(nat32 n = 1) = 0;

  /// Identical to Clear(), except it will clear past the start of the current line and
  /// onto the previous line. Newlines are not counted.
   virtual void Delete(nat32 n = 1) = 0;

  /// Clears the current line, returning the cursor to the (indented) start of
  /// the current line.
   virtual void ClearLine() = 0;

  /// This is identical to ClearLine except it returns the cursor to the end
  /// of the previous line, can be called repeatedly to roll-back output.
  /// Note that implimentations are allowed to provide a fundamental limit on
  /// how many lines can be rolled back before it stops considering word-wrap,
  /// but it must be at least 32 lines.
   virtual void DeleteLine() = 0;


  /// Whenever the user hits a key the text goes straight into the read buffer
  /// and is not displayed to screen, this means that before getting input
  /// calling this is recomended, to remove any accidental keypresses from earlier.
   virtual void Empty() = 0;

  /// This toggles echoing of the read buffer to screen as its typed, ushally
  /// switched on when user input is expected.
  /// When echoing a cursor should be visible, when not echoing the cursor should
  /// not be visible.
  /// Default is false when Conversation mode is entered.
   virtual void Echo(bit echo) = 0;

  /// This waits for a particular key to be pressed by the user, all the while
  /// filling up the read buffer. When that key is pressed it is still put into
  /// the read buffer.
  /// The default code given happens to be the enter key, as that the ushall
  /// delinator.
   virtual void WaitKey(cstrchar c = '\r') = 0;

  /// This waits for a particular number of characters to have been typed in,
  /// i.e. the read buffer will be >= the requested size on return.
   virtual void WaitSize(nat32 size) = 0;


  /// This returns a progress bar object and sets the console into progress bar
  /// mode, you should not do anything else with it until you call the finish
  /// method, unless you want arbitary corruption of output.
  /// It respects the indentation and uses he entire remaining width, and
  /// indicates task numbers.
   time::Progress & BeginProg();

  /// When you have finished with the progress bar object call this method,
  /// if clear is true the progress bar is deleted, leaving it as though it
  /// was never there, assuming you are treating things as a command/response
  /// scroll down mode. Otherwise the progress bar is replaced with stats
  /// about how long it took etc.
   void EndProg(bit clear = false);


  /// &nbsp;
   virtual cstrconst TypeString() const {return "eos::os::Conversation";}


 private:
  ConversationProg prog;
};

//------------------------------------------------------------------------------
/// This represents the extremely flexble but often inappropriate console mode
/// where the program treats the console as an editable buffer of characters,
/// and has full responsibility for updating it. Input is not echoed and capture
/// in a real time fashion is suported. Ushally used for text editors and real
/// time displays of/interfaces to changing data.
class EOS_CLASS LiveText : public io::InOutVirt<io::Text>
{
 public:

};

//------------------------------------------------------------------------------
/// An interface to a text console, actual implimentations whilst ushally for a
/// console on the current computer can be for telnet etc, you just obtain one
/// of these objects somehow and work from there.
/// The console can exist in various modes, each of which provides a different
/// object for its interface. When you enter a mode you must stop using the
/// other mode objects, as they will now be invalid. A program can do multiple
/// mode changes as it sees fit.
class EOS_CLASS Console : public Deletable
{
 public:
  /// &nbsp;
   ~Console();


  /// Enters Conversation mode, this mode must allways work.
   virtual Conversation * StartConversation() = 0;

  /// Enters LiveText mode, this can return null if this mode is not
  /// suported by this object.
   virtual LiveText * StartLiveText();


  /// &nbsp;
   virtual cstrconst TypeString() const {return "eos::os::Console";}
};

//------------------------------------------------------------------------------
/// An implimentation of the console interface for use in basic console
/// applications, simply declare one of these at the start of main() and go.
/// There should never be more than one of these, things get hairy if there are.
class EOS_CLASS Con : public Console, private Conversation
{
 public:
  /// &nbsp;
   Con();

  /// &nbsp;
   ~Con();


  /// &nbsp;
   Conversation * StartConversation();


   void SetColour(bit bright,ConCol fg,ConCol bg);
   nat32 GetIndent() const;
   void Indent(int32 offset);
   nat32 Cols() const;
   nat32 Col() const;
   void Clear(nat32 n);
   void Delete(nat32 n);
   void ClearLine();
   void DeleteLine();
   void Empty();
   void Echo(bit echo);
   void WaitKey(cstrchar c);
   void WaitSize(nat32 size);

   bit EOS() const;
   nat32 Avaliable() const;
   nat32 Read(void * out,nat32 bytes);
   nat32 Peek(void * out,nat32 bytes) const;
   nat32 Skip(nat32 bytes);

   nat32 Write(const void * in,nat32 bytes);
   nat32 Pad(nat32 bytes);


 private:
  bit echo;
  nat32 indent;

  mutable data::Buffer buf;
  void GrabInput() const;

  static const nat32 lcl = 32;
  nat32 lc[lcl]; // Buffer of number of lines consumed by each virtual line.
  nat32 lcp; // Current position in above circular buffer.
};

//------------------------------------------------------------------------------
 };
};
#endif
