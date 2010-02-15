//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "svt/main.h"

using namespace eos;


//------------------------------------------------------------------------------
// This function returns a node indexed by "9:5:0" format strings.
// Returns null if anything goes wrong.
svt::Node * GetNode(os::Conversation & con,svt::Node * root,char * ss)
{
 str::String s(ss);
 str::String::Cursor targ = s.GetCursor();
 
 while (true)
 {
  // Get the index...
   nat32 num;
   targ >> num;
   if (targ.Error()==true) return null<svt::Node*>();

  // Do the offset...
   root = root->Child();
   if (root==null<svt::Node*>()) return null<svt::Node*>();
   for (nat32 i=0;i<num;i++)
   {
    root = root->Next();
    if (root==null<svt::Node*>()) return null<svt::Node*>();
   }
  
  // Skip the ':', or break if end of string...
   byte nc;
   if (targ.Peek(&nc,1)!=1) break;
   if (nc!=':') return null<svt::Node*>();
   targ.Skip(1);
 }
 return root;
}

//------------------------------------------------------------------------------
// This function is given a field referencing string, it then returns a the node
// that contains the string and outputs to 'index' the index of the relevant
// field. Returns null if anything goes wrong.
svt::Var * GetField(os::Conversation & con,svt::Node * root,char * ss,nat32 & index)
{
 LogBlock("GetField","-");
 str::String s(ss);
 str::String::Cursor targ = s.GetCursor();
 
 byte nc;
 if (targ.Peek(&nc,1)!=1) return null<svt::Var*>();
 if (nc!='.')
 {
  while (true)
  {
   // Get the index...
    nat32 num;
    targ >> num;
    if (targ.Error()==true) return null<svt::Var*>();

   // Do the offset...
    root = root->Child();
    if (root==null<svt::Node*>()) return null<svt::Var*>();
    for (nat32 i=0;i<num;i++)
    {
     root = root->Next();
     if (root==null<svt::Node*>()) return null<svt::Var*>();
    }
  
   // Skip the ':', or break if end of string...
    byte nc;
    if (targ.Peek(&nc,1)!=1) return null<svt::Var*>();
    if (nc!=':')
    {
     if (nc!='.') return null<svt::Var*>();
             else break;
    }
    targ.Skip(1);
  }
 }


 targ.Skip(1);
 if (str::Compare(root->TypeString(),"eos::svt::Var")!=0) return null<svt::Var*>();

 svt::Var * asVar = static_cast<svt::Var*>(root);
 str::Token name = asVar->GetCore().GetTT()(targ); // Crashes here.
 if (asVar->GetIndex(name,index)==false) return null<svt::Var*>();

 return asVar;
}

//------------------------------------------------------------------------------
// Prints out the help info...
void Help(os::Conversation & con)
{
 con << "Usage: svt [file.svt] <operations...>\n";
 con << "Given a file on its own this tool prints out the basic structure of the file.\n";
 con << "Alternativly operations to be applied to the file can be given.\n";
 con << "The operations form a list, which is done in sequence.\n";
 con << "The possible operations are:\n";
 con << "-h : Prints out this help information.\n";
 con << "-p : Prints out the structure of the file as held in memory at this time.\n";
 con << "-pn [node] : Prints out the structure of the given node.\n";
 con << "-si [field] [filename] : Saves an image, it only suports a limited variety of field types, with custom write out for some types.\n";
 //con << "-sia [field] [filename] [min] [max] : Saves an image, only suports numeric types. You provide the value that==0 and the value that==255, any values outside of the range are clamped. max can be less than min to invert.\n";
 con << "-st [field] [filename] : Saves a text file, it only suports a limited variety of field types. Uses commas then newlines then two newlines etc as the seperators.\n";
 con << "-sb [field] [filename] : Saves a binary blob, just the raw field data.\n";
 //con << "-wn [filename] : Saves back a svt file.\n";
 //con << "-wb : Saves back over the original file.\n";
 //con << "-mn [node] [new_parent] : Moves a node\n";
 //con << "-dd [field] : Deletes a field\n";
 //con << "-dn [node] : Deletes a node, and all its children.\n"; 
 con << "If any error is found in the operation list it stops at that point.\n";
 con << "Indexing of nodes is done by offset for each level of the hierachy, with colons as dividers, i.e. 0:3:2\n";
 con << "Indexing of fields is done by indexing the node, a full stop, then the field name, i.e. 0:3:2.rgb.\n";
 con << "Root-node fields are indexed as .rgb etc.\n";
}

//------------------------------------------------------------------------------
// Recursive function for printing out svt hierachy...
void PrintNode(os::Conversation & con,svt::Node * node)
{
 con << node->TypeString() << ":\n";

 bit doMeta = false;
 if (str::Compare(node->TypeString(),"eos::svt::Var")==0)
 {
  con.Indent(1);
  doMeta = true;
  svt::Var * var = static_cast<svt::Var*>(node);

  con << "dimensions = " << var->Dims() << "\n";
  
  con << "size = ";
   for (nat32 i=0;i<var->Dims();i++)
   {
    con << var->Size(i);
    if (i+1!=var->Dims()) con << "x";
   }
  con << "\n";

  con << "stride = ";
   for (nat32 i=0;i<=var->Dims();i++)
   {
    con << var->Stride(i);
    if (i!=var->Dims()) con << "|";
   }
  con << "\n";
  con.Indent(-1);

  if (var->Fields()!=0)
  {
   con << ">Multiple Instance Fields (" << var->Fields() << "):\n"; 
   con.Indent(1);
   for (nat32 i=0;i<var->Fields();i++)
   {
    con << var->GetCore().GetTT().Str(var->FieldName(i)) << " = ";
    con << var->GetCore().GetTT().Str(var->FieldType(i));
    con << " (size = " << var->FieldSize(i) << ")\n";
   }
   con.Indent(-1);
  }
 }


 if (doMeta||str::Compare(node->TypeString(),"eos::svt::Meta")==0)
 {
  svt::Meta * meta = static_cast<svt::Meta*>(node);
 
  if ((meta->Size()-meta->SizeExports())!=0)
  {
   con << ">Single Instance Fields (" << meta->Size() << "):\n";
   con.Indent(1);
   for (nat32 i=meta->SizeExports();i<meta->Size();i++)
   {
    svt::Meta::Item & targ = meta->Entry(i);
    con << node->GetCore().GetTT().Str(targ.Key());
    switch (targ.Is())
    {
     case svt::Meta::Aint:
      con << "(int) = " << targ.AsInt();
     break;
     case svt::Meta::Areal:
      con << "(real) = " << targ.AsReal();
     break;
     case svt::Meta::Atoken:
      con << "(token) = " << node->GetCore().GetTT().Str(targ.AsToken());
     break;
     case svt::Meta::Astring:
     {
      str::String temp;
      targ.AsString(temp);
      con << "(string) = " << temp;
     }
     break;
    }
    con << "\n";
   }
   con.Indent(-1);
  } 
 }


 if (node->Child())
 {
  con << ">Children (" << node->ChildCount() << "):\n";
  svt::Node * targ = node->Child();
  if (targ)
  {
   con << "\n";
   do
   {
    con.Indent(2);
    PrintNode(con,targ);
    con.Indent(-2);
    
    targ = targ->Next();
   } while (targ!=node->Child());
  }
 }

 con << "\n";
}

//------------------------------------------------------------------------------
// Saves an image, has custom code for each possibility...
bit SaveImage(os::Conversation & con,svt::Var * node,nat32 field,char * filename)
{
 if (node->Dims()!=2)
 {
  con << "Can only save 2D fields as images.\n";
  return false;
 }


 // Create tempory storage...
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
 
  svt::Var temp(node->GetCore());
  temp.Setup2D(node->Size(0),node->Size(1));
  temp.Add("rgb",rgbIni);
  temp.Commit();
  
  svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
  

 // Render the data...
  bit ok = false;
  if (node->FieldType(field)==node->GetCore().GetTT()("eos::real32"))
  {
   ok = true;
   svt::Field<real32> f;
   node->ByInd(field,f);
   
   // Find the range...
    real32 min = f.Get(0,0);
    real32 max = min;
    for (nat32 y=0;y<f.Size(1);y++)
    {
     for (nat32 x=0;x<f.Size(0);x++)
     {
      min = math::Min(min,f.Get(x,y));
      max = math::Max(max,f.Get(x,y));
     }
    }
    
   // Convert it over...
    for (nat32 y=0;y<f.Size(1);y++)
    {
     for (nat32 x=0;x<f.Size(0);x++)
     {
      real32 val = (f.Get(x,y)-min)/(max-min);

      rgb.Get(x,y).r = val;
      rgb.Get(x,y).g = val;
      rgb.Get(x,y).b = val;
     }
    }
  }


  if (ok==false)
  {
   con << "Unsuported field type.\n";
   return false;
  }


 // Save the file... 
  if (filter::SaveImageRGB(&temp,filename,true)==false)
  {
   con << "Failed to save image file.\n";
   return false;
  }


 return true;
}

// Saves a field as a text file...
bit SaveText(os::Conversation & con,svt::Var * node,nat32 field,char * filename)
{
 // Switch on the type of input...
 str::Token fieldType = node->FieldType(field);


 if (fieldType==node->GetCore().GetTT()("eos::real32"))
 {
  // Open file...
   file::File<io::Text> f(filename,file::way_ow,file::mode_write);
   if (!f.Active())
   {
    con << "Error: Could not open file.\n";
    return false;
   }
   
  // Multi-dimensional loop. I hate these...
   file::Cursor<io::Text> fc = f.GetCursor();
  
   svt::Field<real32> d;
   node->ByInd(field,d);
   
   ds::Array<nat32> pos(node->Dims());
   for (nat32 i=0;i<pos.Size();i++) pos[i] = 0;
   
   while (true)
   {
    // The data...
     fc << d.Get(pos);
     
    // To next...
     pos[0] += 1;
     nat32 lev = 0;
     while (pos[lev]>=d.Size(lev))
     {
      pos[lev] = 0;
      lev += 1;
      if (lev==d.Dims())
      {
       fc << "\n"; // Files end with newlines.
       return true;	      
      }
      pos[lev] += 1;
     }
    
    // The seperator...
     if (lev==0) fc << ",";
     else
     {
      for (nat32 i=0;i<lev;i++) fc << "\n";	     
     }
   }
  // This line will never be passed.
 }


 con << "Error: Unsuported field type.\n";
 return false;	
}

// Saves a field as a binary blob...
bit SaveBlob(os::Conversation & con,svt::Var * node,nat32 field,char * filename)
{
 // Get data...
  nat32 size = node->FieldMemory(field);
  byte * data = mem::Malloc<byte>(size);
  node->GetRaw(field,data);

 // Store data...
  file::File<io::Binary> f(filename,file::way_ow,file::mode_write);
  if (!f.Active())
  {
   mem::Free(data);
   con << "Error: Could not open file.\n";
   return false;
  }
  
  file::Cursor<io::Binary> fc = f.GetCursor();
  if (fc.Write(data,size)!=size)
  {
   mem::Free(data);
   con << "Error: Could not save all the data.\n";
   return false;
  }
  
 // Clean up...
  mem::Free(data);  
    
 return true;
}

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 
 // If run without parameters print the help...
  if (argc<2)
  {
   Help(con);
   return 1;
  }
 
 str::TokenTable tt;
 svt::Core core(tt);


 // Load the SVT file...
  bit warning;
  svt::Node * root = svt::Load(core,argv[1],&warning);
  if (root==null<svt::Node*>())
  {
   con << "Error: Could not load given file.\n";
   return 1;
  }
  if (warning)
  {
   con << "Warning: The file contains data that can not be loaded. It will be lost if the file is saved back.\n";
  }
  
 // If run without operations print the contents of the file...
  if (argc==2)
  {
   PrintNode(con,root);
   return 0;
  }


 // Keep doing operations...
  int arg = 2;
  while (arg<argc)
  {
   if (str::Compare(argv[arg],"-h")==0)
   {
    Help(con);
    ++arg;
    continue;
   }
   
   
   if (str::Compare(argv[arg],"-p")==0)
   {
    PrintNode(con,root);
    ++arg;
    continue;
   }
   
   
   if (str::Compare(argv[arg],"-pn")==0)
   {
    ++arg;
    if (arg==argc)
    {
     con << "Error: -pn was not provided with a node index.\n";
     return 1;
    }

    svt::Node * node = GetNode(con,root,argv[arg]);
    if (node==null<svt::Node*>())
    {
     con << "Error: Bad node index for -pn.\n";
     return 1;
    }
    ++arg;
    
    PrintNode(con,node);    
    continue;
   }


   if (str::Compare(argv[arg],"-si")==0)
   {
    ++arg;
    if (arg==argc)
    {
     con << "Error: -si was not provided with a field index.\n";
     return 1;
    }

    nat32 field;
    svt::Var * node = GetField(con,root,argv[arg],field);
    if (node==null<svt::Node*>())
    {
     con << "Error: Bad field index for -si.\n";
     return 1;
    }
    ++arg;

    if (arg==argc)
    {
     con << "Error: -si was not provided with a filename.\n";
     return 1;
    }
     
    if (SaveImage(con,node,field,argv[arg])==false) return 1;
    ++arg;
    continue;
   }
   

   if (str::Compare(argv[arg],"-st")==0)
   {
    ++arg;
    if (arg==argc)
    {
     con << "Error: -st was not provided with a field index.\n";
     return 1;
    }

    nat32 field;
    svt::Var * node = GetField(con,root,argv[arg],field);
    if (node==null<svt::Node*>())
    {
     con << "Error: Bad field index for -st.\n";
     return 1;
    }
    ++arg;

    if (arg==argc)
    {
     con << "Error: -st was not provided with a filename.\n";
     return 1;
    }
     
    if (SaveText(con,node,field,argv[arg])==false) return 1;
    ++arg;
    continue;
   }

   
   if (str::Compare(argv[arg],"-sb")==0)
   {
    ++arg;
    if (arg==argc)
    {
     con << "Error: -sb was not provided with a field index.\n";
     return 1;
    }

    nat32 field;
    svt::Var * node = GetField(con,root,argv[arg],field);
    if (node==null<svt::Node*>())
    {
     con << "Error: Bad field index for -sb.\n";
     return 1;
    }
    ++arg;

    if (arg==argc)
    {
     con << "Error: -sb was not provided with a filename.\n";
     return 1;
    }
     
    if (SaveBlob(con,node,field,argv[arg])==false) return 1;
    ++arg;
    continue;
   }

 
   con << "Error: Unrecognised operation.\n";
   return 1;   
  }


 //con << "-si [field] [filename] : Saves an image, it only suports a limited variety of field types.\n";
 // con << "-sia [field] [filename] [min] [max] : Saves an image, only suports numeric types. You provide the value that==0 and the value that==255, any values outside of the range are clamped. max can be less than min to invert.\n";
 //con << "-wn [filename] : Saves back a svt file.\n";
 //con << "-wb : Saves back over the original file.\n";
 //con << "-mn [node] [new_parent] : Moves a node\n";
 //con << "-dd [field] : Deletes a field\n";
 //con << "-dn [node] : Deletes a node, and all its children.\n";

 return 0;
}

//------------------------------------------------------------------------------
