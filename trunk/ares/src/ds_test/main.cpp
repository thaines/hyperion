//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "ds_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;
 
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 con << "Begin.\n";

 data::Random rand;


 // Test the sort list - do 1000 random operations on one, checking that all 
 // answers are right by comparison with a linked list that mirrors all
 // operations...
 /*{
  ds::SortList<nat32> list;
  ds::List<nat32> mirror;
  
  for (nat32 i=0;i<1000;i++)
  {
   // Randomly choose an operation - insert or remove...
    if ((rand.Bool())||(mirror.Size()==0))
    {
     // Insert...
      nat32 val = nat32(rand.Int(0,100000));
      con << "Adding " << val << "\n";
      
      nat32 preSize = list.Size();
      list.Add(val);
      if (preSize!=list.Size())
      {
       mirror.AddBack(val);
      }
      else con << "Added a duplicate\n";
    }
    else
    {
     // Remove...
      // Choose victim...
       nat32 index = nat32(rand.Int(0,mirror.Size()-1));
       ds::List<nat32>::Cursor targ = mirror.FrontPtr();
       for (nat32 i=0;i<index;i++) ++targ;
      
      con << "Removing " << *targ << "\n";

      // Do it...
       nat32 preSize = list.Size();
       list.Rem(*targ);
       if (preSize==list.Size()) con << "Remove appears to have failed.\n";

      // Mirror it...
       targ.RemNext();
    }
   
   // Check the contents of the list and mirror match up...
    if (list.Size()!=mirror.Size())
    {
     con << "Sizes mismatch: list.size = " << list.Size() << "; mirror.size = " << mirror.Size() << "\n";
    }
    else
    {
     con << "Size = " << list.Size() << "\n";
    }

    if (list.Size()!=0)
    {
     // Move list into array...    
      ds::Array<nat32> items(mirror.Size());
      {
       ds::List<nat32>::Cursor targ = mirror.FrontPtr();
       for (nat32 i=0;i<items.Size();i++)
       {
        items[i] = *targ;
        ++targ;
       }
      }

     // Sort array...
      items.SortNorm();

     // Iterate both, they should remain synchronised...
     {
      ds::SortList<nat32>::Cursor targ = list.FrontPtr();
      for (nat32 i=0;i<math::Min(items.Size(),list.Size());i++)
      {
       if (targ.Bad())
       {
        con << "(" << i << ") targ went bad early.\n";
        break;
       }
       if (*targ!=items[i])
       {
        con << "(" << i << ") Mismatch: " << *targ << " != " << items[i] << "\n";
       }
       ++targ;
      }
      if (!targ.Bad())
      {
       con << "targ went bad late.\n";
       while (!targ.Bad())
       {
        con << "Overflow: " << *targ << "\n";
        ++targ;
       }
      }
     }
    }


   // Check that iterating the sort list produces a sorted set...
    if (list.Size()!=0)
    {
     ds::SortList<nat32>::Cursor targ = list.FrontPtr();
     nat32 prev = *targ;
     ++targ;
     while (!targ.Bad())
     {
      nat32 curr = *targ;
      if (prev>=curr)
      {
       con << "bad sort order: " << prev << " before " << curr << "\n";
      }
      prev = curr;
      ++targ;
     }
    }

  }
  con << "\n\n\n";
 }*/
 
 
 
 // Test the sparse bit array - fill it up with a random set, whilst mirroring
 // in a sort list. Test that they match at the end.
 /*{
  ds::SortList<nat32> list;
  ds::SparseBitArray sba;
  
  // Fill both with random stuff...
   for (nat32 i=0;i<50;i++)
   {
    nat32 val = nat32(rand.Int(0,1000));
    list.Add(val);
    sba.Set(val);
    con << "add " << val << "\n";
   }
   con << "\n\n";
   
  // Iterate both in sorted order and check they match...
   nat32 val = 0;
   ds::SortList<nat32>::Cursor targ = list.FrontPtr();
   while (true)
   {
    bit sbaOK = sba.NextInc(val);
    bit listOK = !targ.Bad();
    
    if (sbaOK!=listOK) {con << "End mismatch\n"; break;}
    else
    {
     if ((sbaOK==false)||(listOK==false)) break;
     
     if (*targ!=val) con << "Value mismatch: " << *targ << "/" << val << "\n";
     else
     {
      con << "match " << val << "\n";
     }
    }    
    
    ++val;
    ++targ;
   }
   
   con << "Finished bit array test.\n";
 }*/



 // Test the inplace kd-tree...
 {
  // Fill a kd tree with some random positions...
   ds::InplaceKdTree<nat32,2> kdt(100);
   for (nat32 i=0;i<100;i++)
   {
    math::Vect<2,real32> pos;
    pos[0] =  rand.Real(-10.0,10.0);
    pos[1] =  rand.Real(-10.0,10.0);
    kdt[i] = i;
    kdt.SetPos(i,pos);
    con << "i = " << pos << "\n";
   }


  // Build...
   kdt.Build();
  
  
  // Generate random positions and find the nearest node via both the kd tree 
  // and brute force - check they match...
   for (nat32 i=0;i<100;i++)
   {
    math::Vect<2,real32> pos;
    pos[0] =  rand.Real(-10.0,10.0);
    pos[1] =  rand.Real(-10.0,10.0);
    
    nat32 byTree = kdt.NearestMan(pos);
    
    math::Vect<2,real32> targPos;
    kdt.GetPos(0,targPos);
    nat32 byBrute = 0;
    real32 bestBrute = math::Abs(pos[0]-targPos[0]) + math::Abs(pos[1]-targPos[1]);
    for (nat32 i=1;i<100;i++)
    {
     kdt.GetPos(i,targPos);
     real32 brute = math::Abs(pos[0]-targPos[0]) + math::Abs(pos[1]-targPos[1]);
     if (brute<bestBrute)
     {
      bestBrute = brute;
      byBrute = i;
     }
    }
    
    if (byBrute==byTree)
    {
     con << "Match " << pos << "\n";
    }
    else
    {
     con << "Mis-match " << pos << ", (" << byBrute << "!=" << byTree << ")\n";
     math::Vect<2,real32> posBrute;
     kdt.GetPos(byBrute,posBrute);
     con << "byBrute = " << posBrute << "\n";
     math::Vect<2,real32> posTree;
     kdt.GetPos(byTree,posTree);
     con << "byTree = " << posTree << "\n";
    }
   }
 }


 con << "End.\n";
 return 0;
}

//------------------------------------------------------------------------------
