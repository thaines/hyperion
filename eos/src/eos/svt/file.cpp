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

#include "eos/svt/file.h"

#include "eos/version.h"
#include "eos/file/files.h"
#include "eos/io/to_virt.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
TaV::TaV()
{
 magic[0] = 'J';
 magic[1] = 'S';
 magic[2] = 'V';
 magic[3] = 'T';
 
 revision = 1;
 extension = 0;
 prog = str::Duplicate(GetBuild());
 
 root = null<Node*>();
}

TaV::~TaV()
{
 mem::Free(prog);
}

bit TaV::Check() const
{
 return (magic[0]=='J')&&(magic[1]=='S')&&(magic[2]=='V')&&(magic[3]=='T')&&(revision==1)&&(extension<=0);
}

bit TaV::Write(io::OutVirt<io::Binary> & out)
{
 LogBlock("bit eos::svt::TaV::Write(out)","-");
 
 if (root==null<Node*>()) {LogAlways("[svt.save] Failed: null pointer"); return false;}
 if (out.Write("TAV",3)!=3) return false;
 if (out.Write("TAV",3)!=3) return false;
 
 nat16 progLength = str::Length(prog);
 nat32 rootLength = root->TotalWriteSize();
  
 nat32 size = 16;
  size += 4+4+4;
  size += 2+progLength;
  size += rootLength;
 if (out.Write(&size,4)!=4) return false;
 if (out.Write(&size,4)!=4) return false;
 
 if (out.Write(magic,4)!=4) return false;
 if (out.Write(&revision,4)!=4) return false;
 if (out.Write(&extension,4)!=4) return false;
 if (out.Write(&progLength,2)!=2) return false;
 if (out.Write(prog,progLength)!=progLength) {LogAlways("[svt.save] Failed: mismatched prog size"); return false;}
  log::Assert(!out.Error(),"A");
 if (root->Write(out)!=rootLength) {LogAlways("[svt.save] Failed: mismatched root size"); return false;}
 log::Assert(!out.Error(),"A-");
 return !out.Error();
}

bit TaV::Read(Core & core,io::InVirt<io::Binary> & in)
{
 LogBlock("bit eos::svt::TaV::Read(core,in)","-");
 if (in.Read(magic,3)!=3) return false;
 if ((magic[0]!='T')||(magic[1]!='A')||(magic[2]!='V')) return false;
 if (in.Read(magic,3)!=3) return false;
 if ((magic[0]!='T')||(magic[1]!='A')||(magic[2]!='V')) return false;

 nat32 size;
 nat32 size2;
 if (in.Read(&size,4)!=4) return false;
 if (in.Read(&size2,4)!=4) return false;
 if (size!=size2) return false;
 
 if (in.Read(magic,4)!=4) return false; 
 if (in.Read(&revision,4)!=4) return false; 
 if (in.Read(&extension,4)!=4) return false;
 
 nat16 progSize;
 if (in.Read(&progSize,2)!=2) return false; 
 mem::Free(prog);
 prog = mem::Malloc<cstrchar>(progSize+1);
 if (in.Read(prog,progSize)!=progSize) return false; 
 prog[progSize] = 0;
 
 root = core.LoadObject(in);

 return (root!=null<Node*>())&&Check();
}

//------------------------------------------------------------------------------
EOS_FUNC Node * CreateNode(Core & core,Node * parent)
{
 LogBlock("Node * eos::svt::CreateNode()","-");
 if (parent) return parent;
        else return new Node(core);
}

EOS_FUNC Node * LoadNode(Core & core,io::InVirt<io::Binary> & in,Node * self)
{
 LogBlock("Node * eos::svt::LoadNode()","-");
 if (self)
 {
  static_cast<Node*>(self)->ReadBlock(in);
  return self;
 }
 else
 {
  Node * ret = new Node(core);
   static_cast<Node*>(ret)->ReadBlock(in);
  return ret;
 }
}

EOS_FUNC Node * CreateMeta(Core & core,Node * parent)
{
 LogBlock("Node * eos::svt::CreateMeta()","-");
 if (parent) return new Meta(static_cast<Node*>(parent));
        else return new Meta(core);
}

EOS_FUNC Node * LoadMeta(Core & core,io::InVirt<io::Binary> & in,Node * self)
{
 LogBlock("Node * eos::svt::LoadMeta()","-");
 if (self)
 {
  static_cast<Meta*>(self)->ReadBlock(in);
  return self;
 }
 else
 {
  Meta * ret = new Meta(core);
   static_cast<Node*>(ret)->ReadBlock(in);
   static_cast<Meta*>(ret)->ReadBlock(in);
  return ret;
 }
}

EOS_FUNC Node * CreateVar(Core & core,Node * parent)
{
 LogBlock("Node * eos::svt::CreateVar()","-");
 if (parent) return new Var(static_cast<Meta*>(parent));
        else return new Var(core);
}

EOS_FUNC Node * LoadVar(Core & core,io::InVirt<io::Binary> & in,Node * self)
{
 LogBlock("Node * eos::svt::LoadVar()","-");
 if (self)
 {
  static_cast<Var*>(self)->ReadBlock(in);
  return self;
 }
 else
 {
  Var * ret = new Var(core);
   static_cast<Node*>(ret)->ReadBlock(in);
   static_cast<Meta*>(ret)->ReadBlock(in);
   static_cast<Var*>(ret)->ReadBlock(in);   
  return ret;
 }
}

//------------------------------------------------------------------------------
EOS_FUNC Node * Load(Core & core,cstrconst fn, bit * warning)
{
 LogBlock("Node * eos::svt::Load(...)","-");
 file::File<io::Binary> file(fn,file::way_edit,file::mode_read);
 if (file.Active()==false) return null<Node*>();
 
 file::Cursor<io::Binary> cursor = file.GetCursor();
 io::VirtIn<file::Cursor<io::Binary> > vFile(cursor);
 
 TaV tav;
 bit success = tav.Read(core,vFile);
 if (warning) *warning = !success;
 
 return tav.root;
}

EOS_FUNC bit Save(cstrconst fn,Node * root,bit overwrite)
{
 LogBlock("bit eos::svt::Save(...)","-");
 file::File<io::Binary> file(fn,overwrite?file::way_ow:file::way_new,file::mode_write);
 if (file.Active()==false) return false; 

 file::Cursor<io::Binary> cursor = file.GetCursor();
 io::VirtOut<file::Cursor<io::Binary> > vFile(cursor);

 TaV tav;
 tav.root = root;
 
 return tav.Write(vFile);
}

//------------------------------------------------------------------------------
 };
};
