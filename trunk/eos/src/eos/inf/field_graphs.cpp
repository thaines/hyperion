//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/inf/field_graphs.h"

#include "eos/typestring.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
FieldGraph::FieldGraph(bit ms)
:doMS(ms),maximumLevel(0xFFFFFFFF),iters(6),extraHigh(0),extraLow(0),countFP(0),countVP(0)
{}

FieldGraph::~FieldGraph()
{
 for (nat32 i=0;i<countFP;i++) delete fp[i];
 
 for (nat32 i=0;i<res.Size();i++)
 {
  delete res[i].vp;
  mem::Free(res[i].res);
 }
}

void FieldGraph::SetIters(nat32 it,nat32 exHigh,nat32 exLow)
{
 iters = it;
 extraHigh = exHigh;
 extraLow = exLow;
}

void FieldGraph::SetMaxLevel(nat32 maxLevel)
{
 maximumLevel = maxLevel;
}

nat32 FieldGraph::NewFP(FactorPattern * gfp)
{
 if (fp.Size()==countFP) fp.Size(fp.Size()+4);
 nat32 ret = countFP;
  fp[countFP] = gfp;
  ++countFP;
 return ret;
}

nat32 FieldGraph::NewVP()
{
 nat32 ret = countVP;
  ++countVP;
 return ret;
}

void FieldGraph::Lay(nat32 fp,nat32 pipe,nat32 vp)
{ 
 WorkOrder wo;
  wo.fp = fp;
  wo.pipe = pipe;
  wo.vp = vp;
 wol.AddBack(wo);
}

bit FieldGraph::Run(time::Progress * prog)
{
 LogBlock("bit FieldGraph::Run(...)","-");
 prog->Push();

  // First we have to infer and verify the typing, i.e. calculate all the 
  // details of the graph and check it all matches...
  // (Work out the maximum level as well.)
   prog->Report(0,2);
    // Compress fp for conveniance...
     fp.Size(countFP);
   
    // Create the shell of the res array...
     res.Size(countVP);
     for (nat32 i=0;i<res.Size();i++)
     {
      res[i].vp = null<VariablePattern*>();
      res[i].res = null<real32*>();
     }


    // Loop whilst change happens...
     LogDebug("[inf.field] Start type propagation");
     bit doMore;
     do
     {
      doMore = false;
      // Iterate the wol array, trying to find new inferences of type,
      // passing them on as possible - this involves passing into the 
      // res array as well as out of the res array...
      // (Stop when no changes are made.)
       ds::List<WorkOrder>::Cursor targ = wol.FrontPtr();       
       while (!targ.Bad())
       {        
        if (res[targ->vp].vp)
        {
         // Variable allready set - passing from variable to factors...        
          nat32 set;
          if (SetVarPat(*fp[targ->fp],targ->pipe,*res[targ->vp].vp,set)==false)
          {
           // We have not been given a consistant graph - exit with an error...
            LogAlways("[inf.field] Failed due to error when setting pipe {factor pattern,pipe,variable}"
                      << LogDiv() << targ->fp << LogDiv() << targ->pipe << LogDiv() << targ->vp);
            prog->Pop();
            return false;
          }
          doMore |= set!=0;
        }
        else
        {
         // Variable not set, check if we can set it from the factor...
          if (fp[targ->fp]->PipeIsSet(targ->pipe))
          {
           res[targ->vp].vp = fp[targ->fp]->PipeGet(targ->pipe).Clone();
           doMore |= true;
          }
        }

        ++targ;       
       }
     } while (doMore);
     LogDebug("[inf.field] End type propagation");


    // Check that all variables and all factors have been set,
    // its an error if they have not...
    // (So some logging whilst we are at it.)
     // Check variables...
      for (nat32 i=0;i<res.Size();i++)
      {
       if (res[i].vp==null<VariablePattern*>())
       {
        LogAlways("[inf.field] Failed due to unset variable {variable pattern}" << LogDiv() << i);
        prog->Pop();
        return false;
       }
       else
       {
        LogDebug("[inf.field] Variable " << i << " {variable type,paras}" <<
                 LogDiv() << typestring(*res[i].vp) << LogDiv() << res[i].vp->Paras());
       }
      }
     
     // Check factors...
      for (nat32 i=0;i<fp.Size();i++)
      {
       for (nat32 j=0;j<fp[i]->PipeCount();j++)
       {
        if (!fp[i]->PipeIsSet(j))
        {
         LogAlways("[inf.field] Failed due to unset pipe {factor pattern,pipe}" << LogDiv() << i << LogDiv() << j);
         prog->Pop();
         return false;
        }
        else
        {
         LogDebug("[inf.field] Factor " << i << ", Pipe " << j << " {factor type,variable type,paras}" <<
                  LogDiv() << typestring(*fp[i]) <<
                  LogDiv() << typestring(fp[i]->PipeGet(j)) <<
                  LogDiv() << fp[i]->PipeGet(j).Paras());
        }
       }
      }
      LogDebug("[inf.field] Validated propagation");


    // Finish off the res structure, so its not a concern latter, also find
    // the maximum level worth doing...
     nat32 maxLevel = 0;
     for (nat32 i=0;i<res.Size();i++)
     {
      // maxLevel...
       maxLevel = math::Max(maxLevel,res[i].vp->MaxLevel());

      // mc...
       Frequency::MakeClass(res[i].vp->Labels(),res[i].mc);
      
      // res...
       res[i].res = mem::Malloc<real32>(res[i].vp->Labels() * res[i].vp->Vars(0));
     }
     maxLevel = math::Min(maxLevel,maximumLevel);



  // Now we have to iterate over each level, creating the factor graph,
  // transfering from the previous as relevant, and solving...
   prog->Report(1,2);
   prog->Push();
    nat32 level = maxLevel+1;
    LevelData * prev = null<LevelData*>();
    LevelData * curr = null<LevelData*>();
    do
    {
     --level;
     prog->Report(maxLevel-level,maxLevel+1);
   
      // Move along one, creating the next FactorGraph...
      delete prev;
      prev = curr;
      curr = new LevelData(doMS);

      // Do the work...
       DoLevel(level,prev,*curr,prog);
        
    } while (level!=0);

    delete prev;
    delete curr;
   prog->Pop();

 prog->Pop();
 return true;
}

//------------------------------------------------------------------------------
bit FieldGraph::SetVarPat(FactorPattern & fp,nat32 pipe,const VariablePattern & vp,nat32 & set)
{
 LogTime("bit FieldGraph::SetVarPat(...)");
 set = 0;

 // If this pipe has allready been set check for equality...
  if (fp.PipeIsSet(pipe))
  {
   bit equal = true;
    const VariablePattern & op = fp.PipeGet(pipe);
    equal &= str::Compare(typestring(vp),typestring(op))==0;
    equal &= vp.Paras()==op.Paras();
   return equal;
  }


 // Cache the state of the fp as it currently stands...
  ds::Array<const VariablePattern*> ps(fp.PipeCount());
  for (nat32 i=0;i<fp.PipeCount();i++)
  {
   if (fp.PipeIsSet(i)) ps[i] = &fp.PipeGet(i);
                   else ps[i] = null<VariablePattern*>();
  }


 // Update, this can fail and make us return false...
  if (fp.PipeSet(pipe,vp)==false) return false;


 // Check how the cached state compares to the actual state now - 
 // update set and check for incompatabilities...
  for (nat32 i=0;i<fp.PipeCount();i++)
  {
   if (fp.PipeIsSet(i))
   {
    if (ps[i]==null<VariablePattern*>()) ++set;
    else
    {
     bit equal = true;
      const VariablePattern & op = fp.PipeGet(i);
      equal &= str::Compare(typestring(*ps[i]),typestring(op))==0;
      equal &= ps[i]->Paras()==op.Paras();
     if (equal==false) return false;
    }
   }
   else
   {
    if (ps[i]!=null<VariablePattern*>()) return false;
   }
  }
  
 return true;
}

void FieldGraph::DoLevel(nat32 level,LevelData * prev,LevelData & curr,time::Progress * prog)
{
 LogBlock("void FieldGraph::DoLevel(...)","{level}" << LogDiv() << level);
 prog->Push();
 nat32 steps = 3 + (level==0?1:0) + (prev?1:0);
 nat32 step = 0;

  // If we have to copy from the previous level generate a copy array, 
  // that indicates from which variable in the previous version each 
  // variable in the new version should be...
  // This is a double array, indexed by variable pattern then variable index,
  // that contains the variable index for the previous level. (Same pattern index.)
  // Variables that have no previous are set to -1.
   ds::Array<ds::Array<int32>,
             mem::MakeNew<ds::Array<int32> >,
             mem::KillOnlyDel<ds::Array<int32> > > toPrev;
   if (prev)
   {
    prog->Report(step++,steps);    
    toPrev.Size(res.Size());
    for (nat32 i=0;i<res.Size();i++)
    {
     nat32 sze = res[i].vp->Vars(level);
     toPrev[i].Size(sze);
     VariableTransfer vt(toPrev[i]);
     res[i].vp->Transfer(level,vt);
    }
   }


  // Create the factors, collecting the linking information...
   prog->Report(step++,steps);
   // First create the FactorConstruct objects...
    curr.vpl.Size(res.Size());
    ds::Array<FactorConstruct*> fca(fp.Size());
    for (nat32 i=0;i<fp.Size();i++)
    {
     fca[i] = new FactorConstruct(curr.fg,i,fp[i]->PipeCount());
    }

   // Now fill 'em up...
   {
    ds::List<WorkOrder>::Cursor targ = wol.FrontPtr();
    while (!targ.Bad())
    {
     fca[targ->fp]->SetPipe(targ->pipe,&curr.vpl[targ->vp]);
     ++targ;
    }
   }
   
   // Send them unto the breach, where they may do their duty for bit & byte...
   // Kill them upon their return.
    for (nat32 i=0;i<fp.Size();i++)
    {
     fp[i]->Construct(level,*fca[i]);
     delete fca[i];
    }


  // Create the variables, using the collected link information, if we have a previous layer
  // we simultaneously copy it in...
   prog->Report(step++,steps);
   Variable var;
   curr.piv.Size(res.Size());
   for (nat32 i=0;i<res.Size();i++)
   {
    curr.piv[i].Size(res[i].vp->Vars(level));
    
    ds::SortList<PatLink>::Cursor targ = curr.vpl[i].FrontPtr();
    for (nat32 j=0;j<curr.piv[i].Size();j++)
    {
     while ((!targ.Bad())&&(targ->var==j))
     {
      // Make the link...
       curr.fg.MakeLink(&var,targ->func,targ->inst,targ->link);
      
      // If we have previous copy it in...
       if (prev)
       {
        int32 preVar = toPrev[i][j];
        if (preVar!=-1)
        {
         // Create the dummy node with which to search, include getting the 
         // relevent variable number from the previous data set...
          PatLink dummy = *targ;
          dummy.var = preVar;
        
         // See if we can find a match...
          PatLink * match = prev->vpl[i].Get(dummy);
        
         // If a match has been found perform the transfer...
          if (match)
          {
           curr.fg.SetMsg(targ->func,targ->inst,targ->link,res[i].mc,
                          prev->fg.GetMsg(match->func,match->inst,match->link));
          }
        }
       }
     
      // To next...
       ++targ;
     }
     if (var.LinkCount()==0) curr.piv[i][j] = nat32(-1);
                        else curr.piv[i][j] = curr.fg.MakeVar(&var);
    }
   }


  // Solve...
   prog->Report(step++,steps);
   curr.fg.SetIters(iters + (prev?0:extraHigh) + (level==0?extraLow:0));
   curr.fg.Run(prog,level!=0);


  // If the lift is now at level 0 then copy the results into the res structure...
   if (level==0)
   {
    prog->Report(step++,steps);
    curr.fg.FromNegLn();
    
    for (nat32 i=0;i<res.Size();i++)
    {
     nat32 vars = res[i].vp->Vars(0);
     nat32 labels = res[i].vp->Labels();
     for (nat32 j=0;j<vars;j++)
     {
      if (curr.piv[i][j]!=nat32(-1))
      {
       mem::Copy<real32>(res[i].res + j*labels,(real32*)curr.fg.GetProb(curr.piv[i][j]),labels);
      }
      else
      {
       real32 * targ = res[i].res + j*labels;
       real32 val = 1.0/real32(labels);
       for (nat32 k=0;k<labels;k++) targ[k] = val;
      }
     }
    }
   }

  
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
