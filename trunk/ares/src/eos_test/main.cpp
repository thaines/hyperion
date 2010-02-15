//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include <stdio.h>

#include "eos_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 str::String s("Cabbage");
 
 
 file::ImageRGB image;
 if (image.Load("left.bmp"))
 {
  printf("\nLoaded left.bmp");
  if (image.Save("test-out.bmp")) printf("\nSaved test-out.bmp\n\n");
                             else printf("\nFailed to save test-out.bmp\n\n");
 }
 else printf("\nFailed to load left.bmp\n\n");
 
 
 
 str::String str("::");
 str << "Wibble = " << int32(86) << "\n";
 str += " fishies\n";
 str << 67.8 << "\n";
 
 str::String str2("Hello");
 str2 << " moon\n";
 
 str << str2;
 
 {
  cstr cs = str.ToStr();
  printf(cs);
  printf("|\n\n");
  mem::Free(cs);
 }
 
 
 
 str = "Oh no, a cabbage";
 str::String::Cursor targ = str.GetCursor(3);
 while (!targ.EOS())
 {
  char buf[3]; buf[1] = '>'; buf[2] = 0;
  printf("<");
  targ.Read(buf,1);
  
  printf(buf);
 }
 printf("\n\n");
 
 
 
 str2 = str;
 targ = str.GetCursor(7);
 targ << "era";
 str << "\n" << str2;
 
 {
  cstr cs = str.ToStr();
  printf(cs);
  printf("|\n\n");
  mem::Free(cs);
 } 
 
 
 return 0;
}

//------------------------------------------------------------------------------
