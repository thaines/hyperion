//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/inf/factor_graphs.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
FactorGraph::FactorGraph(bit ms,bit l)
:doMS(ms),loopy(l),iters(1),
funcCount(0),funcs(funcInc),
varCount(0),vars(varInc),
outIndex(null<nat32*>()),output(null<byte*>())
{}

FactorGraph::~FactorGraph()
{
 // Function data...
  for (nat32 i=0;i<funcCount;i++) delete funcs[i];

 // Variable data...
  for (nat32 i=0;i<varCount;i++) mem::Free(vars[i]);
 
 // Output data...
  delete[] outIndex;
  delete[] output;
}

nat32 FactorGraph::MakeFuncs(Function * func,nat32 instances)
{
 if (funcCount==funcs.Size()) funcs.Size(funcs.Size()+funcInc);
 
 if (doMS) func->ToMS();
      else func->ToSP();
 
 nat32 ret = funcCount;
 funcs[ret] = new FunctionSet(func,instances);
 if (doMS) funcs[ret]->FlatlineLn();
      else funcs[ret]->Flatline();
 
 ++funcCount;
 return ret;
}

bit FactorGraph::MakeLink(Variable * variable,nat32 function,nat32 instance,nat32 link)
{
 Variable::Link ln;
  ln.fsInd = function;
  ln.fs = funcs[function];
  ln.instance = instance;
  ln.link = link;
 
 return variable->AddLink(ln);
}

nat32 FactorGraph::MakeVar(Variable * var)
{
 if (varCount==vars.Size()) vars.Size(vars.Size()+varInc);
 
 nat32 ret = varCount;
 vars[ret] = var->GetPacked();
 
 if (!loopy)
 {
  // Record the links for scheduling the message passes...
   for (nat32 i=0;i<var->LinkCount();i++)
   {
    Variable::Link vl;
    var->LinkDetail(i,vl);
   
    Link link;
     link.var = ret;
     link.varLink = i;
     link.func = vl.fsInd;
     link.funcInst = vl.instance;
     link.funcLink = vl.link;
    links.AddBack(link);
   }
 }
 
 var->Reset();
 ++varCount;
 return ret;
}

bit FactorGraph::SetMsg(nat32 function,nat32 instance,nat32 link,const MessageClass & type,const void * message)
{
 if (type!=funcs[function]->GetClass(link)) return false;
 mem::Copy<byte>((byte*)funcs[function]->ToFunc(instance,link),(byte*)message,type.Size(type));
 return true;
}

bit FactorGraph::Run(time::Progress * prog,bit multipass)
{
 LogBlock("bit FactorGraph::Run(...)","{multipass}" << LogDiv() << multipass);
 if (doMS)
 {
  if (loopy)
  {
   RunLoopyMS(prog,multipass);
   return true; 
  }
  else
  {
   return RunNonLoopyMS(prog);
  } 
 }
 else
 {
  if (loopy)
  {
   RunLoopySP(prog,multipass);
   return true; 
  }
  else
  {
   return RunNonLoopySP(prog);
  }
 }
}

void FactorGraph::Reset()
{
 for (nat32 i=0;i<funcCount;i++) funcs[i]->Flatline();
}

const void * FactorGraph::GetMsg(nat32 function,nat32 instance,nat32 link) const
{
 return funcs[function]->ToFunc(instance,link);
}

const MessageClass & FactorGraph::GetType(nat32 function,nat32 link) const
{
 return funcs[function]->GetClass(link);
}

void FactorGraph::FromNegLn()
{
 LogBlock("void FactorGraph::FromNegLn()","-");
 if (!doMS) return;
 for (nat32 i=0;i<varCount;i++)
 {
  const MessageClass & mc = Variable::Class(vars[i]);
  mc.FromNegLn(mc,&output[outIndex[i]]);
 }
}

bit FactorGraph::RunNonLoopySP(time::Progress * prog)
{
 LogBlock("bit FactorGraph::RunNonLoopySP(...)","-");
 prog->Push();   
 // Create a schedule of jobs that we will do...
  ds::Scheduler<MsgJob> work;
   
  // Create a data structure to contain all jobs such that we can get to them given
  // the function indexing info...
   JobStore store(funcs,funcCount);
   
  // Create the dual structure to the above, this one indexed by variable indexing info...
   ds::Array<ds::List<ds::Scheduler<MsgJob>::Job>, 
            mem::MakeNew< ds::List<ds::Scheduler<MsgJob>::Job> >,
            mem::KillOnlyDel< ds::List<ds::Scheduler<MsgJob>::Job> > > varList(varCount);

  // Add in all the jobs, registering them with the above structures...
  {
   ds::List<Link>::Cursor targ = links.FrontPtr();
   while (!targ.Bad())
   {
    MsgJob mj;
     mj.var = targ->var;
     mj.varLink = targ->varLink;
     mj.func = targ->func;
     mj.funcInst = targ->funcInst;
     mj.funcLink = targ->funcLink;

    mj.dir = MsgJob::ToVar;
    ds::Scheduler<MsgJob>::Job toVar = work.AddJob(mj);
    store.GetToVar(mj.func,mj.funcInst,mj.funcLink) = toVar;
    varList[targ->var].AddBack(toVar);
    
    mj.dir = MsgJob::ToFunc;
    store.GetToFunc(mj.func,mj.funcInst,mj.funcLink) = work.AddJob(mj);

    ++targ;
   }
  }

  // Add in all the constraints...
  {
   ds::List<Link>::Cursor targ = links.FrontPtr();
   while (!targ.Bad())
   {
    // Constraints for the function to variable...
     ds::Scheduler<MsgJob>::Job toVar = store.GetToVar(targ->func,targ->funcInst,targ->funcLink);
     for (nat32 i=0;i<funcs[targ->func]->GetFunc()->Links();i++)
     {
      if (i==targ->funcLink) continue;
      ds::Scheduler<MsgJob>::Job pj = store.GetToFunc(targ->func,targ->funcInst,i);
      if (work.Good(pj)) work.AddCon(pj,toVar);
     }
     
    // Constraints for the variable to function...
     ds::Scheduler<MsgJob>::Job toFunc = store.GetToFunc(targ->func,targ->funcInst,targ->funcLink);
     ds::List<ds::Scheduler<MsgJob>::Job>::Cursor ob = varList[targ->var].FrontPtr();
     while (!ob.Bad())
     {
      if ((*ob!=toVar)&&(work.Good(*ob))) work.AddCon(*ob,toFunc);
      ++ob;
     }
     
    ++targ;
   }
  }


 // Create a set of flags indicating the status of each function/variable,
 // this saves on effort as we can skip most jobs this way...
  ds::Array<nat32> funcOff(funcCount);
  funcOff[0] = 0;
  for (nat32 i=1;i<funcCount;i++)
  {
   funcOff[i] = funcOff[i-1] + funcs[i-1]->Instances();
  }
  nat32 funcTotal = funcOff[funcCount-1] + funcs[funcCount-1]->Instances();
  
  ds::Array<State> funcState(funcTotal);  
  ds::Array<State> varState(varCount);

  for (nat32 i=0;i<funcState.Size();i++) funcState[i].done = State::None;
  for (nat32 i=0;i<varState.Size();i++) varState[i].done = State::None;

 
 // Do the jobs until none are left, at which point its all done...
  ds::Scheduler<MsgJob>::Job targ = work.Schedule();
  if (work.Bad(targ)) {prog->Pop(); return false;}
  data::Block temp;
  while (work.Good(targ))
  {
   MsgJob & job = work.Item(targ);
   
   if (job.dir==MsgJob::ToVar)
   {
    // ToVar...
     nat32 funcInd = funcOff[job.func] + job.funcInst;
     if (funcState[funcInd].done!=State::All)
     {
      if (funcState[funcInd].done==State::None)
      {
       // Send a single message...
        funcState[funcInd].link = job.funcLink;
        funcs[job.func]->SendOneSP(job.funcInst,job.funcLink);
      }
      else
      {
       // Send all but one messages...
        funcs[job.func]->SendAllButOneSP(job.funcInst,funcState[funcInd].link);
      }
     }
   }
   else
   {
    // ToFunc...
     if (varState[job.var].done!=State::All)
     {
      if (varState[job.var].done==State::None)
      {
       // Send a single message...
        varState[job.var].link = job.varLink;
        Variable::SendOneSP(vars[job.var],job.varLink);
      }
      else
      {
       // Send all but one messages...
        Variable::SendAllButOneSP(vars[job.var],varState[job.var].link,temp);
      }
     }
   }
   
   targ = work.Next(targ);
  }


 // Finally calculate the output distributions...
  CalcOutputSP();
 
 prog->Pop();
 return true;
}

void FactorGraph::RunLoopySP(time::Progress * prog,bit multipass)
{
 LogBlock("void FactorGraph::RunLoopySP(...)","{multipass}" << LogDiv() << multipass);
 prog->Push();

 // Pass arround the messages...
  data::Block temp;
  for (nat32 i=0;i<iters;i++)
  {
   // Calculate all the function to variable messages...
    prog->Report(i*2,iters*2);
    prog->Push();
     for (nat32 j=0;j<funcCount;j++)
     {
      prog->Report(j,funcCount);
      funcs[j]->SendAllSP(prog);
     }
    prog->Pop();
  
   // If its not a multipass we bug out here...
    if ((!multipass)&&(i+1==iters)) break;

   // Calculate all the variable to function messages...
    prog->Report(i*2+1,iters*2);
    prog->Push();
     for (nat32 j=0;j<varCount;j++)
     {
      prog->Report(j,varCount);
      Variable::SendAllSP(vars[j],temp);
     }
    prog->Pop();
  }
  
 // Calcualte the output from the final rack of messages, if applicable...
  if (!multipass)
  {
   prog->Report(2*iters-1,2*iters);
   CalcOutputSP();
  }
  
 prog->Pop();
}

void FactorGraph::CalcOutputSP()
{
 LogBlock("void FactorGraph::CalcOutputSP()","-");
 // Delete up any previous data...
  delete[] outIndex;
  delete[] output;

 // Calculate the size of the structure and offsets into it...
  outIndex = new nat32[varCount];
  outIndex[0] = 0;
  for (nat32 i=1;i<varCount;i++)
  {
   outIndex[i] = outIndex[i-1] + Variable::Class(vars[i-1]).Size(Variable::Class(vars[i-1]));
  }
  nat32 totalSize = outIndex[varCount-1] + Variable::Class(vars[varCount-1]).Size(Variable::Class(vars[varCount-1]));

 // Calculate every output probability functions...
  output = new byte[totalSize];
  for (nat32 i=0;i<varCount;i++)
  {
   Variable::CalcOutputSP(vars[i],&output[outIndex[i]]);
  }
}

bit FactorGraph::RunNonLoopyMS(time::Progress * prog)
{
 LogBlock("bit FactorGraph::RunNonLoopyMS(...)","-");
 prog->Push();
 // Create a schedule of jobs that we will do...
  ds::Scheduler<MsgJob> work;
   
  // Create a data structure to contain all jobs such that we can get to them given
  // the function indexing info...
   JobStore store(funcs,funcCount);
   
  // Create the dual structure to the above, this one indexed by variable indexing info...
   ds::Array<ds::List<ds::Scheduler<MsgJob>::Job>, 
            mem::MakeNew< ds::List<ds::Scheduler<MsgJob>::Job> >,
            mem::KillOnlyDel< ds::List<ds::Scheduler<MsgJob>::Job> > > varList(varCount);

  // Add in all the jobs, registering them with the above structures...
  {
   ds::List<Link>::Cursor targ = links.FrontPtr();
   while (!targ.Bad())
   {
    MsgJob mj;
     mj.var = targ->var;
     mj.varLink = targ->varLink;
     mj.func = targ->func;
     mj.funcInst = targ->funcInst;
     mj.funcLink = targ->funcLink;

    mj.dir = MsgJob::ToVar;
    ds::Scheduler<MsgJob>::Job toVar = work.AddJob(mj);
    store.GetToVar(mj.func,mj.funcInst,mj.funcLink) = toVar;
    varList[targ->var].AddBack(toVar);
    
    mj.dir = MsgJob::ToFunc;
    store.GetToFunc(mj.func,mj.funcInst,mj.funcLink) = work.AddJob(mj);

    ++targ;
   }
  }

  // Add in all the constraints...
  {
   ds::List<Link>::Cursor targ = links.FrontPtr();
   while (!targ.Bad())
   {
    // Constraints for the function to variable...
     ds::Scheduler<MsgJob>::Job toVar = store.GetToVar(targ->func,targ->funcInst,targ->funcLink);
     for (nat32 i=0;i<funcs[targ->func]->GetFunc()->Links();i++)
     {
      if (i==targ->funcLink) continue;
      ds::Scheduler<MsgJob>::Job pj = store.GetToFunc(targ->func,targ->funcInst,i);
      if (work.Good(pj)) work.AddCon(pj,toVar);
     }
     
    // Constraints for the variable to function...
     ds::Scheduler<MsgJob>::Job toFunc = store.GetToFunc(targ->func,targ->funcInst,targ->funcLink);
     ds::List<ds::Scheduler<MsgJob>::Job>::Cursor ob = varList[targ->var].FrontPtr();
     while (!ob.Bad())
     {
      if ((*ob!=toVar)&&(work.Good(*ob))) work.AddCon(*ob,toFunc);
      ++ob;
     }
     
    ++targ;
   }
  }


 // Create a set of flags indicating the status of each function/variable,
 // this saves on effort as we can skip most jobs this way...
  ds::Array<nat32> funcOff(funcCount);
  funcOff[0] = 0;
  for (nat32 i=1;i<funcCount;i++)
  {
   funcOff[i] = funcOff[i-1] + funcs[i-1]->Instances();
  }
  nat32 funcTotal = funcOff[funcCount-1] + funcs[funcCount-1]->Instances();
  
  ds::Array<State> funcState(funcTotal);  
  ds::Array<State> varState(varCount);

  for (nat32 i=0;i<funcState.Size();i++) funcState[i].done = State::None;
  for (nat32 i=0;i<varState.Size();i++) varState[i].done = State::None;

 
 // Do the jobs until none are left, at which point its all done...
  ds::Scheduler<MsgJob>::Job targ = work.Schedule();
  if (work.Bad(targ)) {prog->Pop(); return false;}
  while (work.Good(targ))
  {
   MsgJob & job = work.Item(targ);
   
   if (job.dir==MsgJob::ToVar)
   {
    // ToVar...
     nat32 funcInd = funcOff[job.func] + job.funcInst;
     if (funcState[funcInd].done!=State::All)
     {
      if (funcState[funcInd].done==State::None)
      {
       // Send a single message...
        funcState[funcInd].link = job.funcLink;
        funcs[job.func]->SendOneMS(job.funcInst,job.funcLink);
      }
      else
      {
       // Send all but one messages...
        funcs[job.func]->SendAllButOneMS(job.funcInst,funcState[funcInd].link);
      }
     }
   }
   else
   {
    // ToFunc...
     if (varState[job.var].done!=State::All)
     {
      if (varState[job.var].done==State::None)
      {
       // Send a single message...
        varState[job.var].link = job.varLink;
        Variable::SendOneMS(vars[job.var],job.varLink);
      }
      else
      {
       // Send all but one messages...
        Variable::SendAllButOneMS(vars[job.var],varState[job.var].link);
      }
     }
   }
   
   targ = work.Next(targ);
  }


 // Finally calculate the output distributions...
  CalcOutputMS();
 
 prog->Pop();
 return true;
}

void FactorGraph::RunLoopyMS(time::Progress * prog,bit multipass)
{
 LogBlock("void FactorGraph::RunLoopyMS(...)","{multipass}" << LogDiv() << multipass);
 prog->Push();

 // Pass arround the messages...
  for (nat32 i=0;i<iters;i++)
  {
   // Calculate all the function to variable messages...
    prog->Report(i*2,iters*2);
    prog->Push();
     for (nat32 j=0;j<funcCount;j++)
     {
      prog->Report(j,funcCount);
      funcs[j]->SendAllMS(prog);
     }
    prog->Pop();
  
   // If its not a multipass we bug out here...
    if ((!multipass)&&(i+1==iters)) break;

   // Calculate all the variable to function messages...
    prog->Report(i*2+1,iters*2);
    prog->Push();
     //LogDebug("varCount = " << varCount);
     for (nat32 j=0;j<varCount;j++)
     {
      prog->Report(j,varCount);
      Variable::SendAllMS(vars[j]);
     }
    prog->Pop();
  }
  
 // Calcualte the output from the final rack of messages, if applicable...
  if (!multipass)
  {
   prog->Report(2*iters-1,2*iters);
   CalcOutputMS();
  }
  
 prog->Pop();
}

void FactorGraph::CalcOutputMS()
{
 LogBlock("void FactorGraph::CalcOutputMS()","-");
 // Delete up any previous data...
  delete[] outIndex;
  delete[] output;

 // Calculate the size of the structure and offsets into it...
  outIndex = new nat32[varCount];
  outIndex[0] = 0;
  for (nat32 i=1;i<varCount;i++)
  {
   outIndex[i] = outIndex[i-1] + Variable::Class(vars[i-1]).Size(Variable::Class(vars[i-1]));
  }
  nat32 totalSize = outIndex[varCount-1] + Variable::Class(vars[varCount-1]).Size(Variable::Class(vars[varCount-1]));

 // Calculate every output probability functions...
  output = new byte[totalSize];
  for (nat32 i=0;i<varCount;i++)
  {
   Variable::CalcOutputMS(vars[i],&output[outIndex[i]]);
  }
}

//------------------------------------------------------------------------------
 };
};
