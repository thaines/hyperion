//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/inf/model_seg.h"

#include "eos/math/functions.h"
#include "eos/ds/arrays.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
ModelSeg::ModelSeg()
:width(0),height(0),models(0),
costCap(1.0),diffCost(1.0),iters(10)
{}

ModelSeg::~ModelSeg()
{}

void ModelSeg::SetSize(nat32 w,nat32 h,nat32 m)
{
 width = w;
 height = h;
 models = m;

 diffCostMult.Resize(width,height);
 for (nat32 y=0;y<diffCostMult.Height();y++)
 {
  for (nat32 x=0;x<diffCostMult.Width();x++)
  {
   diffCostMult.Get(x,y).mx = 1.0;
   diffCostMult.Get(x,y).my = 1.0;
  }
 }
}

void ModelSeg::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void ModelSeg::SetParas(real32 dc,real32 cc,nat32 i)
{
 diffCost = dc;
 costCap = cc;
 iters = i;
}

void ModelSeg::AddCost(nat32 x,nat32 y,nat32 model,real32 cost)
{
 if (cost>costCap) return;

 NodeModCost nmc;
 nmc.x = x;
 nmc.y = y;
 nmc.model = model;
 nmc.cost = cost;
 
 userCosts.Add(nmc);
}

void ModelSeg::SetMult(nat32 x,nat32 y,real32 mx,real32 my)
{
 diffCostMult.Get(x,y).mx = mx;
 diffCostMult.Get(x,y).my = my;
}

void ModelSeg::Run(time::Progress * prog)
{
 LogBlock("eos::inf::ModelSeg::Run","");
 prog->Push();


 // Construct the index hierachy...
  prog->Report(0,3);
  prog->Push();
  prog->Report(0,5);
  nat32 levels = math::Max(math::TopBit(width),math::TopBit(height));
  ds::ArrayDel< ds::Array2D<Node> > ind(levels);

  ind[0].Resize(width,height);
  for (nat32 l=1;l<levels;l++)
  {
   nat32 halfWidth  = (ind[l-1].Width()/2)  + ((ind[l-1].Width()&1)?1:0);
   nat32 halfHeight = (ind[l-1].Height()/2) + ((ind[l-1].Height()&1)?1:0);
   ind[l].Resize(halfWidth,halfHeight);
  }

  for (nat32 l=0;l<levels;l++)
  {
   for (nat32 y=0;y<ind[l].Height();y++)
   {
    for (nat32 x=0;x<ind[l].Width();x++)
    {
     for (nat32 j=0;j<4;j++)
     {
      ind[l].Get(x,y).send[j] = true;
      ind[l].Get(x,y).cap[j] = 0.0;
      ind[l].Get(x,y).in[j] = null<Msg*>();      
     }
     ind[l].Get(x,y).user = null<Msg*>();

     if (l==0)
     {
      ind[l].Get(x,y).cap[0] = diffCost * diffCostMult.Get(x,y).mx;
      ind[l].Get(x,y).cap[1] = diffCost * diffCostMult.Get(x,y).my;
      if (x!=0) ind[l].Get(x,y).cap[2] = diffCost * diffCostMult.Get(x-1,y).mx;
      if (y!=0) ind[l].Get(x,y).cap[3] = diffCost * diffCostMult.Get(x,y-1).my;
     }
    }
   }
  }



 // Fill in the user costs for the bottom and all higher levels...
 // (We use a single memory allocator for this set.)
  prog->Report(1,5);
  prog->Push();
  prog->Report(0,ind.Size());
  mem::Packer userAlloc(blockSize); 
  // Bottom level...
   nat32 targ = 0;
   ds::Array<NodeModCost> uca;
   userCosts.Fill(uca);
   uca.SortNorm();
   
   prog->Push();
   for (nat32 y=0;y<ind[0].Height();y++)
   {
    prog->Report(y,ind[0].Height());
    for (nat32 x=0;x<ind[0].Width();x++)
    {
     if ((!mask.Valid())||(mask.Get(x,y)))
     {
      // Skip any entrys procedding the once we want...
       NodeModCost dummy;
       dummy.x = x;
       dummy.y = y;
       dummy.model = 0;
       while ((targ<uca.Size())&&(uca[targ]<dummy)) ++targ;

      // First pass to count how many we shall be eatting, and find the minimum...
       dummy.model = models;
       nat32 msgSize = 0;
       nat32 targ2 = targ;
       real32 minCost = math::Infinity<real32>();
       while ((targ2<uca.Size())&&(uca[targ2]<dummy))
       {
        minCost = math::Min(minCost,uca[targ2].cost);
        ++msgSize;
        ++targ2;
       }


      // Second pass to eat, and create the user message...
       if (msgSize!=0)
       {
        Msg *& out = ind[0].Get(x,y).user;
        out = (Msg*)(void*)userAlloc.Malloc<byte>(sizeof(Msg)+sizeof(ModCost)*msgSize);
        
        for (nat32 i=0;i<msgSize;i++)
        {
         out->Data()[i].model = uca[targ].model;
         out->Data()[i].cost = uca[targ].cost - minCost;
         ++targ;
        }

        out->size = msgSize;
        out->baseCost = costCap - minCost;
       }
     }
    }
   }
   prog->Pop();


  // Higher levels...
   for (nat32 l=1;l<ind.Size();l++)
   {
    prog->Report(l,ind.Size());
    TransferUserUp(ind[l-1],ind[l],userAlloc);
   }
   prog->Pop();


 // Apply boundary conditions to all levels...
  prog->Report(2,5);
  prog->Push();
  for (nat32 l=0;l<ind.Size();l++)
  {
   prog->Report(l,ind.Size());
   for (nat32 x=0;x<ind[l].Width();x++)
   {
    ind[l].Get(x,0).send[3] = false;
    ind[l].Get(x,ind[l].Height()-1).send[1] = false;
   }

   for (nat32 y=0;y<ind[l].Height();y++)
   {
    ind[l].Get(0,y).send[2] = false;
    ind[l].Get(ind[l].Width()-1,y).send[0] = false;
   }
  }
  prog->Pop();



 // If a mask has been provided apply mask conditions to all levels...  
  if (mask.Valid())
  {
   // First build the mask hierachy...
    prog->Report(3,5);
    ds::ArrayDel< ds::Array2D<bit> > mh(ind.Size());
    
    prog->Push();
    prog->Report(0,mh.Size());
    mh[0].Resize(ind[0].Width(),ind[0].Height());
    for (nat32 y=0;y<mh[0].Height();y++)
    {
     for (nat32 x=0;x<mh[0].Width();x++) mh[0].Get(x,y) = mask.Get(x,y);
    }
    
    for (nat32 l=1;l<mh.Size();l++)
    {
     prog->Report(l,mh.Size());
     mh[l].Resize(ind[l].Width(),ind[l].Height());
     for (nat32 y=0;y<mh[l].Height();y++)
     {
      for (nat32 x=0;x<mh[l].Width();x++)
      {
       nat32 px = x*2;
       nat32 py = y*2;
       
       bit okX = (px+1)!=mh[l].Width();
       bit okY = (py+1)!=mh[l].Height();
       
       mh[l].Get(x,y) = mh[l-1].Get(px,py);
       if (okX) mh[l].Get(x,y) |= mh[l-1].Get(px+1,py);
       if (okY) mh[l].Get(x,y) |= mh[l-1].Get(px,py+1);
       if (okX&&okY) mh[l].Get(x,y) |= mh[l-1].Get(px+1,py+1);
      }
     }
    }
    prog->Pop();


   // Now apply it to the index by blocking messages to masked nodes...
    prog->Report(4,5);
    prog->Push();
    for (nat32 l=0;l<ind.Size();l++)
    {
     prog->Report(l,ind.Size());
     for (nat32 y=0;y<ind[l].Height();y++)
     {
      for (nat32 x=0;x<ind[l].Width();x++)
      {
       if (mh[l].Get(x,y))
       {
        for (nat32 d=0;d<4;d++)
        {
         if (ind[l].Get(x,y).send[d])
         {
          nat32 ax = x;
          nat32 ay = y;
          switch (d)
          {
           case 0: ++ax; break;
           case 1: ++ay; break;
           case 2: --ax; break;
           case 3: --ay; break;
          }
          
          ind[l].Get(x,y).send[d] = mh[l].Get(ax,ay);
         }
        }
       }
       else
       {
        for (nat32 d=0;d<4;d++) ind[l].Get(x,y).send[d] = false;
       }
      }
     }
    }
    prog->Pop();
  }
  prog->Pop();



 // Iterate through the levels passing messages, and transfering from each layer to the one below...
  prog->Report(1,3);
  ds::ArrayDel< mem::StackPtr< mem::Packer > > msgA(ind.Size());
  ds::ArrayDel< mem::StackPtr< mem::Packer > > msgB(ind.Size());
  for (nat32 l=0;l<ind.Size();l++)
  {
   msgA[l] = new mem::Packer(blockSize);
   msgB[l] = new mem::Packer(blockSize);
  }

  prog->Push();
  for (int32 l=int32(ind.Size()-1);l>=0;l--)
  {
   prog->Report(ind.Size()-1-l,ind.Size());
   prog->Push();
   
   // Do the iterations...
    for (nat32 i=0;i<iters;i++)
    {
     prog->Report(i,iters+1);
     if (i&1) msgB[l]->Reset();
         else msgA[l]->Reset();
     PassMessages(ind[l],i,(i&1)?*msgB[l]:*msgA[l]);
    }


   // If level 0 we are done...
    if (l==0) break;


   // Transfer from this level to the level below...
    prog->Report(iters,iters+1);
    TransferMsgDown(ind[l],ind[l-1],*msgB[l-1],*msgA[l-1]);
    msgA[l]->Reset();
    msgB[l]->Reset();
    
   prog->Pop();
  }
  prog->Pop();



 // Extract the final result...
  prog->Report(2,3);
  out.Resize(ind[0].Width(),ind[0].Height());
  prog->Push();
  for (nat32 y=0;y<out.Height();y++)
  {
   prog->Report(y,out.Height());
   for (nat32 x=0;x<out.Width();x++)
   {
    real32 bestCost = math::Infinity<real32>();
    real32 secondBestCost = math::Infinity<real32>();
    nat32 model = 0;
    
    Msg * msg[5];
    for (nat32 d=0;d<4;d++) msg[d] = ind[0].Get(x,y).in[d];
    msg[4] = ind[0].Get(x,y).user;
    
    nat32 remainder[5];
    real32 baseCost[5];
    for (nat32 d=0;d<5;d++)
    {
     if (msg[d])
     {
      remainder[d] = msg[d]->size;
      baseCost[d] = msg[d]->baseCost;
     }
     else
     {
      remainder[d] = 0;
      baseCost[d] = 0.0;
     }
    }

    
    while (true)
    {
     // Find the minimum model index...
      bit first = true;
      nat32 minModel = 0;
      for (nat32 d=0;d<5;d++)
      {
       if (remainder[d]!=0)
       {
        if (first)
        {
         minModel = msg[d]->Data()[msg[d]->size - remainder[d]].model;
         first = false;
        }
        else minModel = math::Min(minModel,msg[d]->Data()[msg[d]->size - remainder[d]].model);
       }
      }
     
     // Break if no minimum, i.e. we have iterated them all...
      if (first) break;
     
     // Calculate the models cost...
      real32 cost = 0.0;
      for (nat32 d=0;d<5;d++)
      {
       if (remainder[d]!=0)
       {
        if (msg[d]->Data()[msg[d]->size - remainder[d]].model==minModel)
        {
         cost += msg[d]->Data()[msg[d]->size - remainder[d]].cost;
         remainder[d] -= 1;
        }
        else cost += baseCost[d];
       }
       else cost += baseCost[d];
      }

     // Factor in the model into the finding of the best and second best...
      if (cost<bestCost)
      {
       secondBestCost = bestCost;
       bestCost = cost;
       model = minModel;
      }
      else secondBestCost = math::Min(secondBestCost,cost);
    }
    
    if (math::IsFinite(bestCost))
    {
     out.Get(x,y).model = model;
     if (math::IsFinite(secondBestCost))
     {
      out.Get(x,y).cost = secondBestCost - bestCost;
     }
     else
     {
      out.Get(x,y).cost = -bestCost;
      for (nat32 d=0;d<5;d++) out.Get(x,y).cost += baseCost[d];
     }
    }
    else
    {
     out.Get(x,y).model = 0xFFFFFFFF;
     out.Get(x,y).cost  = 0.0;
    }
   }
  }
  prog->Pop();


 prog->Pop();
}

bit ModelSeg::Masked(nat32 x,nat32 y) const
{
 return out.Get(x,y).model!=0xFFFFFFFF;
}

nat32 ModelSeg::Model(nat32 x,nat32 y) const
{
 return out.Get(x,y).model;
}

real32 ModelSeg::Confidence(nat32 x,nat32 y) const
{
 return out.Get(x,y).cost;
}

void ModelSeg::TransferUserUp(ds::Array2D<Node> & from,ds::Array2D<Node> & to,mem::Packer & alloc)
{
 LogTime("eos::inf::ModelSeg::TransferUserUp");
 
 for (nat32 y=0;y<to.Height();y++)
 {
  for (nat32 x=0;x<to.Width();x++)
  {
   // Grab pointers to the messages to be merged...
    nat32 sx = x*2;
    nat32 sy = y*2;
   
    bit okX = sx+1!=from.Width();
    bit okY = sy+1!=from.Height();
   
    Msg * source[4];
    source[0] = from.Get(sx,sy).user;
    if (okX) source[1] = from.Get(sx+1,sy).user;
        else source[1] = null<Msg*>();
    if (okY) source[2] = from.Get(sx,sy+1).user;
        else source[2] = null<Msg*>();
    if (okX&&okY) source[3] = from.Get(sx+1,sy+1).user;
             else source[3] = null<Msg*>();


   // First pass to determine the number of entrys and the minimum cost...
    nat32 remainder[4];
    real32 baseCost[4];
    for (nat32 i=0;i<4;i++)
    {
     remainder[i] = source[i]?source[i]->size:0;
     baseCost[i] = source[i]?source[i]->baseCost:0.0;
    }
    
    // Init - find the minimum model...
     bit done = true;
     nat32 minModel = 0;
     for (nat32 i=0;i<4;i++)
     {
      if (remainder[i]!=0)
      {
       if (done)
       {
        minModel = source[i]->Data()[0].model;
        done = false;
       }
       else minModel = math::Min(minModel,source[i]->Data()[0].model);
      }
     }
     
    // Actual pass...
     nat32 modelCount = 0;
     real32 minCost = math::Infinity<real32>();
     while (!done)
     {
      ++modelCount;
      done = true;
      nat32 nextModel = 0;
      real32 cost = 0.0;

      for (nat32 i=0;i<4;i++)
      {
       if (remainder[i]!=0)
       {
        ModCost * mct = &source[i]->Data()[source[i]->size-remainder[i]];
        if (mct->model==minModel)
        {
         cost += mct->cost;
         remainder[i] -= 1;
         ++mct;
        }
        else cost += baseCost[i];
        
        if (remainder[i]!=0)
        {
         if (done)
         {
          nextModel = mct->model;
          done = false;
         }
         else nextModel = math::Min(nextModel,mct->model);
        }
       }
       else cost += baseCost[i];
      }
      
      minCost = math::Min(minCost,cost);
      minModel = nextModel;
     }
     
    // Skip next pass if nothing to transfer...
     if (modelCount==0) continue;


   // Second pass to create and fill in the message...
    for (nat32 i=0;i<4;i++) remainder[i] = source[i]?source[i]->size:0;
    
    // Init - find the minimum model (Again)...
     done = true;
     minModel = 0;
     for (nat32 i=0;i<4;i++)
     {
      if (remainder[i]!=0)
      {
       if (done)
       {
        minModel = source[i]->Data()[0].model;
        done = false;
       }
       else minModel = math::Min(minModel,source[i]->Data()[0].model);
      }
     }
     
    // Actual pass...
     to.Get(x,y).user = (Msg*)(void*)alloc.Mal<byte>(sizeof(Msg) + sizeof(ModCost)*modelCount);     
     nat32 ind = 0;
     while (!done)
     {
      done = true;
      nat32 nextModel = 0;
      
      real32 cost = -minCost;
      for (nat32 j=0;j<4;j++)
      {
       if (remainder[j]!=0)
       {
        ModCost * mct = &source[j]->Data()[source[j]->size-remainder[j]];
        if (mct->model==minModel)
        {
         cost += mct->cost;
         remainder[j] -= 1;
         ++mct;
        }
        else cost += baseCost[j];
        
        if (remainder[j]!=0)
        {
         if (done)
         {
          nextModel = mct->model;
          done = false;
         }
         else nextModel = math::Min(nextModel,mct->model);
        }
       }
       else cost += baseCost[j];
      }

      to.Get(x,y).user->Data()[ind].model = minModel;
      to.Get(x,y).user->Data()[ind].cost  = cost;
      ++ind;

      minModel = nextModel;
     }
     
     to.Get(x,y).user->size = ind;
     to.Get(x,y).user->baseCost = baseCost[0]+baseCost[1]+baseCost[2]+baseCost[3]-minCost;
     alloc.Loc<byte>(sizeof(Msg) + sizeof(ModCost)*ind);


    // Sum into the cost cap (Nulled earlier)...
     nat32 div = 0;
     if (source[0])
     {
      ++div;
      for (nat32 i=0;i<4;i++) to.Get(x,y).cap[i] += from.Get(sx,sy).cap[i];
     }
     if (source[1])
     {
      ++div;
      for (nat32 i=0;i<4;i++) to.Get(x,y).cap[i] += from.Get(sx+1,sy).cap[i];
     }
     if (source[2])
     {
      ++div;
      for (nat32 i=0;i<4;i++) to.Get(x,y).cap[i] += from.Get(sx,sy+1).cap[i];
     }
     if (source[3])
     {
      ++div;
      for (nat32 i=0;i<4;i++) to.Get(x,y).cap[i] += from.Get(sx+1,sy+1).cap[i];
     }
     if (div!=0)
     {
      for (nat32 i=0;i<4;i++) to.Get(x,y).cap[i] /= real32(div);
     }             
  }
 }
}

void ModelSeg::TransferMsgDown(ds::Array2D<Node> & from,ds::Array2D<Node> & to,
                               mem::Packer & allocA,mem::Packer & allocB)
{
 LogTime("eos::inf::ModelSeg::TransferMsgDown");
 
 for (nat32 y=0;y<to.Height();y++)
 {
  for (nat32 x=0;x<to.Width();x++)
  {
   Node & out = to.Get(x,y);
   Node & in = from.Get(x/2,y/2);
   mem::Packer & alloc = ((x+y)&1)?allocB:allocA;
    
   for (nat32 d=0;d<4;d++)
   {
    if ((in.in[d])&&(out.send[d]))
    {
     nat32 size = sizeof(Msg) + sizeof(ModCost) * in.in[d]->size;
     out.in[d] = (Msg*)(void*)alloc.Malloc<byte>(size);
     mem::Copy<byte>((byte*)(void*)out.in[d],(byte*)(void*)in.in[d],size);
    }
   }
  }
 }
}

void ModelSeg::PassMessages(ds::Array2D<Node> & index,nat32 iter,mem::Packer & alloc)
{
 LogTime("eos::inf::ModelSeg::PassMessages");
 // Iterate the checkboard and each direction on it where a message should be sent...
  for (nat32 y=0;y<index.Height();y++)
  {
   for (nat32 x=(y+iter)%2;x<index.Width();x+=2)
   {
    Node & targ = index.Get(x,y);
    for (nat32 d=0;d<4;d++)
    {
     if (targ.send[d])
     {
      // Get a reference to the pointer in the target node we are trying to 
      // recreate...
       nat32 tx = x;
       nat32 ty = y;
       switch (d)
       {
        case 0: ++tx; break;
        case 1: ++ty; break;
        case 2: --tx; break;
        case 3: --ty; break;
       }
       
       Msg *& out = index.Get(tx,ty).in[(d+2)%4];


      // First pass over the user message and 3 relevant messages to find the
      // minimum cost and maximum number of output models...
        Msg * msg[4];
        for (nat32 i=0;i<4;i++) msg[i] = targ.in[i];
        msg[d] = targ.user;

        nat32 remainder[4];
        real32 baseCost[4];
        for (nat32 i=0;i<4;i++)
        {
         remainder[i] = msg[i]?msg[i]->size:0;
         baseCost[i] = msg[i]?msg[i]->baseCost:0.0;
        }

        bit done = true;
        nat32 minModel = 0;
        for (nat32 i=0;i<4;i++)
        {
         if (remainder[i]!=0)
         {
          if (done)
          {
           minModel = msg[i]->Data()[0].model;
           done = false;
          }
          else minModel = math::Min(minModel,msg[i]->Data()[0].model);
         }
        }

       // Keep iterating, each iteration process a particular model index and 
       // work out the next model index that will need work on...
        real32 minCost = math::Infinity<real32>();
        nat32 modelCount = 0;
        while (!done)
        {
         ++modelCount;
         done = true;
         nat32 nextModel = 0;
         real32 cost = 0.0;
         
         for (nat32 i=0;i<4;i++)
         {
          if (remainder[i]!=0)
          {
           ModCost * mct = &msg[i]->Data()[msg[i]->size-remainder[i]];
           if (mct->model==minModel)
           {
            cost += mct->cost;
            remainder[i] -= 1;
            ++mct;
           }
           else cost += baseCost[i];

           if (remainder[i]!=0)
           {
            if (done)
            {
             nextModel = mct->model;
             done = false;
            }
            else nextModel = math::Min(nextModel,mct->model);
           }
          }
          else cost += baseCost[i];
         }

         minCost = math::Min(minCost,cost);
         minModel = nextModel;
        }
        
        if (modelCount==0)
        {
         out = null<Msg*>();
         continue;
        }


      // Second pass, create the message and fill in the relevant incomming
      // for the target node...
       // Same as before, setup the variables... (Little less work though)
        for (nat32 i=0;i<4;i++) remainder[i] = msg[i]?msg[i]->size:0;

        done = true;
        for (nat32 i=0;i<4;i++)
        {
         if (remainder[i]!=0)
         {
          if (done)
          {
           minModel = msg[i]->Data()[0].model;
           done = false;
          }
          else minModel = math::Min(minModel,msg[i]->Data()[0].model);
         }
        }
       
       // Keep iterating, each iteration process a particular model index and 
       // work out the next model index that will need work on...
        out = (Msg*)(void*)alloc.Mal<byte>(sizeof(Msg) + modelCount*sizeof(ModCost));
        modelCount = 0;
        while (!done)
        {
         done = true;
         nat32 nextModel = 0;
         real32 cost = -minCost;
         
         for (nat32 i=0;i<4;i++)
         {
          if (remainder[i]!=0)
          {
           ModCost * mct = &msg[i]->Data()[msg[i]->size-remainder[i]];
           if (mct->model==minModel)
           {
            cost += mct->cost;
            remainder[i] -= 1;
            ++mct;
           }
           else cost += baseCost[i];

           if (remainder[i]!=0)
           {
            if (done)
            {
             nextModel = mct->model;
             done = false;
            }
            else nextModel = math::Min(nextModel,mct->model);
           }
          }
          else cost += baseCost[i];
         }
         
         if (cost<(targ.cap[d]-minCost))
         {
          out->Data()[modelCount].model = minModel;
          out->Data()[modelCount].cost = cost;
          ++modelCount;
         }
         
         minModel = nextModel;
        }

        out->size = modelCount;
        out->baseCost = math::Min(targ.cap[d],baseCost[0]+baseCost[1]+baseCost[2]+baseCost[3]) - minCost;
        alloc.Loc<byte>(sizeof(Msg) + modelCount*sizeof(ModCost));
     }
    }
   }
  }
}

//------------------------------------------------------------------------------
 };
};
