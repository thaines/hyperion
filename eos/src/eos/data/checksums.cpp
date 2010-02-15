//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines
#include "eos/data/checksums.h"

#include "eos/mem/functions.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
// A table, used by the below methods...
const nat16 crc16Table[256] = {
                               0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	                       0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	                       0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	                       0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	                       0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	                       0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	                       0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	                       0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	                       0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	                       0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	                       0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 
	                       0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
	                       0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 
	                       0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
	                       0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	                       0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
	                       0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	                       0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
	                       0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	                       0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	                       0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	                       0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	                       0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	                       0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	                       0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	                       0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	                       0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	                       0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	                       0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	                       0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	                       0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	                       0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	                      };

nat32 Crc16::Write(void * in,nat32 bytes)
{
 byte * targ = static_cast<byte*>(in);
 byte * end = targ + bytes;
 while (targ!=end)
 {
  crc = crc16Table[targ[0]^(crc>>8)] ^ (crc<<8);
  ++targ;
 }
 return bytes;       
}

nat32 Crc16::Pad(nat32 bytes)
{
 for (nat32 i=0;i<bytes;i++)
 {
  crc = crc16Table[byte(0)^(crc>>8)] ^ (crc<<8);       
 }
 return bytes;       
}

//------------------------------------------------------------------------------
Md5::Md5()
{
 Reset();
}

Md5::Md5(const Md5 & rhs)
:amount(rhs.amount)
{
 for (nat32 i=0;i<4;i++) md5[i] = rhs.md5[i];
 for (nat32 i=0;i<amount%64;i++) data[i] = rhs.data[i];
}

void Md5::Reset()
{
 amount = 0;
 md5[0] = 0x67452301;
 md5[1] = 0xEFCDAB89;
 md5[2] = 0x98BADCFE;
 md5[3] = 0x10325476;        
}

Md5 & Md5::operator = (const Md5 & rhs)
{
 for (nat32 i=0;i<4;i++) md5[i] = rhs.md5[i];
 amount = rhs.amount;  
 for (nat32 i=0;i<amount%64;i++) data[i] = rhs.data[i];      
 return *this;
}

bit Md5::operator == (const Md5 & rhs) const
{
 nat32 a[4]; Get(a);
 nat32 b[4]; rhs.Get(b);

 return (a[0]==b[0]) && (a[1]==b[1]) && (a[2]==b[2]) && (a[3]==b[3]);
}

bit Md5::operator != (const Md5 & rhs) const
{
 nat32 a[4]; Get(a);
 nat32 b[4]; rhs.Get(b);

 return (a[0]!=b[0]) || (a[1]!=b[1]) || (a[2]!=b[2]) || (a[3]!=b[3]);
}

void Md5::Get(nat32 out[4]) const
{
 for (nat32 i=0;i<4;i++) out[i] = md5[i];
 
 // Add in the last bit of the calculation, either 2 chunks or one chunk depending on remaider.
 nat32 rem = amount%64;
 if (rem>56)
 {
  // 2 blocks - pad with zeros the last block then add in, before creating a final 
  // block with size and adding that in...       
   for (nat32 i=rem;i<64;i++) const_cast<byte*>(data)[i] = 0;
   AddBlock((nat32*)data,out);
  
   nat32 temp[16];
    for (nat32 i=0;i<14;i++) temp[i] = 0;  
    temp[14] = amount<<3;
    temp[15] = amount>>29;
   AddBlock(static_cast<nat32*>(temp),out);
 }
 else
 {
  // 1 block, add the size onto the end, null the bits inbetween and add...
   for (nat32 i=rem;i<56;i++) const_cast<byte*>(data)[i] = 0;
   ((nat32*)data)[14] = amount<<3;
   ((nat32*)data)[15] = amount>>29;
   AddBlock((nat32*)data,out);  
 }
}

nat32 Md5::Write(void * in,nat32 bytes)
{
 nat32 ret = bytes;
  while (true)
  {
   nat32 dsf = amount%64;
   if (dsf+bytes>=64)
   {
    nat32 atd = 64-dsf;
    
    mem::Copy(data+dsf,(byte*)in,atd);
    AddBlock((nat32*)data,md5);
    
    bytes -= atd;
    in = (byte*)in + atd;
    amount += atd;       
   }
   else
   {
    mem::Copy(data+dsf,(byte*)in,bytes);
    amount += bytes;
    break;       
   }
  }
 return ret;       
}
    
nat32 Md5::Pad(nat32 bytes)
{
 nat32 ret = bytes;
  while (true)
  {
   nat32 dsf = amount%64;
   if (dsf+bytes>=64)
   {
    nat32 atd = 64-dsf;
    
    mem::Null(data+dsf,atd);
    AddBlock((nat32*)data,md5);
    
    bytes -= atd;
    amount += atd;       
   }
   else
   {
    mem::Null(data+dsf,bytes);
    amount += bytes;
    break;       
   }
  }
 return ret;       
}
    
void Md5::AddBlock(nat32 block[16],nat32 md5[4])
{
 nat32 a = md5[0];
 nat32 b = md5[1];
 nat32 c = md5[2];
 nat32 d = md5[3];


 #define FUNC_1(a,b,c,d,k,s,i) a = b + math::RotLeft(a + ((b&c)|((~b)&d)) + block[k] + i,s)
 #define FUNC_2(a,b,c,d,k,s,i) a = b + math::RotLeft(a + ((b&d)|(c&(~d))) + block[k] + i,s)
 #define FUNC_3(a,b,c,d,k,s,i) a = b + math::RotLeft(a + (b^c^d)          + block[k] + i,s)
 #define FUNC_4(a,b,c,d,k,s,i) a = b + math::RotLeft(a + (c^(b|(~d)))     + block[k] + i,s)

  FUNC_1(a,b,c,d, 0, 7,0xd76aa478); FUNC_1(d,a,b,c, 1,12,0xe8c7b756); FUNC_1(c,d,a,b, 2,17,0x242070db); FUNC_1(b,c,d,a, 3,22,0xc1bdceee);
  FUNC_1(a,b,c,d, 4, 7,0xf57c0faf); FUNC_1(d,a,b,c, 5,12,0x4787c62a); FUNC_1(c,d,a,b, 6,17,0xa8304613); FUNC_1(b,c,d,a, 7,22,0xfd469501);
  FUNC_1(a,b,c,d, 8, 7,0x698098d8); FUNC_1(d,a,b,c, 9,12,0x8b44f7af); FUNC_1(c,d,a,b,10,17,0xffff5bb1); FUNC_1(b,c,d,a,11,22,0x895cd7be);
  FUNC_1(a,b,c,d,12, 7,0x6b901122); FUNC_1(d,a,b,c,13,12,0xfd987193); FUNC_1(c,d,a,b,14,17,0xa679438e); FUNC_1(b,c,d,a,15,22,0x49b40821);

  FUNC_2(a,b,c,d, 1, 5,0xf61e2562); FUNC_2(d,a,b,c, 6, 9,0xc040b340); FUNC_2(c,d,a,b,11,14,0x265e5a51); FUNC_2(b,c,d,a, 0,20,0xe9b6c7aa);
  FUNC_2(a,b,c,d, 5, 5,0xd62f105d); FUNC_2(d,a,b,c,10, 9,0x02441453); FUNC_2(c,d,a,b,15,14,0xd8a1e681); FUNC_2(b,c,d,a, 4,20,0xe7d3fbc8);
  FUNC_2(a,b,c,d, 9, 5,0x21e1cde6); FUNC_2(d,a,b,c,14, 9,0xc33707d6); FUNC_2(c,d,a,b, 3,14,0xf4d50d87); FUNC_2(b,c,d,a, 8,20,0x455a14ed);
  FUNC_2(a,b,c,d,13, 5,0xa9e3e905); FUNC_2(d,a,b,c, 2, 9,0xfcefa3f8); FUNC_2(c,d,a,b, 7,14,0x676f02d9); FUNC_2(b,c,d,a,12,20,0x8d2a4c8a);

  FUNC_3(a,b,c,d, 5, 4,0xfffa3942); FUNC_3(d,a,b,c, 8,11,0x8771f681); FUNC_3(c,d,a,b,11,16,0x6d9d6122); FUNC_3(b,c,d,a,14,23,0xfde5380c);
  FUNC_3(a,b,c,d, 1, 4,0xa4beea44); FUNC_3(d,a,b,c, 4,11,0x4bdecfa9); FUNC_3(c,d,a,b, 7,16,0xf6bb4b60); FUNC_3(b,c,d,a,10,23,0xbebfbc70);
  FUNC_3(a,b,c,d,13, 4,0x289b7ec6); FUNC_3(d,a,b,c, 0,11,0xeaa127fa); FUNC_3(c,d,a,b, 3,16,0xd4ef3085); FUNC_3(b,c,d,a, 6,23,0x04881d05);
  FUNC_3(a,b,c,d, 9, 4,0xd9d4d039); FUNC_3(d,a,b,c,12,11,0xe6db99e5); FUNC_3(c,d,a,b,15,16,0x1fa27cf8); FUNC_3(b,c,d,a, 2,23,0xc4ac5665);

  FUNC_4(a,b,c,d, 0, 6,0xf4292244); FUNC_4(d,a,b,c, 7,10,0x432aff97); FUNC_4(c,d,a,b,14,15,0xab9423a7); FUNC_4(b,c,d,a, 5,21,0xfc93a039);
  FUNC_4(a,b,c,d,12, 6,0x655b59c3); FUNC_4(d,a,b,c, 3,10,0x8f0ccc92); FUNC_4(c,d,a,b,10,15,0xffeff47d); FUNC_4(b,c,d,a, 1,21,0x85845dd1);
  FUNC_4(a,b,c,d, 8, 6,0x6fa87e4f); FUNC_4(d,a,b,c,15,10,0xfe2ce6e0); FUNC_4(c,d,a,b, 6,15,0xa3014314); FUNC_4(b,c,d,a,13,21,0x4e0811a1);
  FUNC_4(a,b,c,d, 4, 6,0xf7537e82); FUNC_4(d,a,b,c,11,10,0xbd3af235); FUNC_4(c,d,a,b, 2,15,0x2ad7d2bb); FUNC_4(b,c,d,a, 9,21,0xeb86d391);

 #undef FUNC_4
 #undef FUNC_3
 #undef FUNC_2
 #undef FUNC_1


 md5[0] += a;
 md5[1] += b;
 md5[2] += c;
 md5[3] += d;
}

//------------------------------------------------------------------------------
 };
};
