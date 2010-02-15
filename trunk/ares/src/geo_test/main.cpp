//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "geo_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 data::Random rand;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();



 con.WaitSize(1);
 return 0;
}

//------------------------------------------------------------------------------

