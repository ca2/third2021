#include "framework.h"
#include "aqua.h"
#include <math.h>

CLASS_DECL_AQUA void pre_multiply_alpha(unsigned int * pui,int cx,int cy,int stride)
{
   unsigned char * point = (unsigned char *) pui;
   unsigned char a1;
   unsigned char a2;
   unsigned char a3;
   unsigned char a4;
   int s =  stride  * cy / 16;
#pragma loop(hint_parallel(0))
   for(int i = 0; i < s; i++)
   {
      a1 = point[3];
      a2 = point[7];
      a3 = point[11];
      a4 = point[15];

      point[0] = (point[0] * a1) >> 8;
      point[1] = (point[1] * a1) >> 8;
      point[2] = (point[2] * a1) >> 8;

      point[4] = (point[4] * a2) >> 8;
      point[5] = (point[5] * a2) >> 8;
      point[6] = (point[6] * a2) >> 8;

      point[8] = (point[8] * a3) >> 8;
      point[9] = (point[9] * a3) >> 8;
      point[10] = (point[10] * a3) >> 8;

      point[12] = (point[12] * a4) >> 8;
      point[13] = (point[13] * a4) >> 8;
      point[14] = (point[14] * a4) >> 8;
      //pui[i]=((pui[i] & 0xff000000) >> 24) << 24
      //   | (((((pui[i] & 0xff0000) >> 16) * ((pui[i] & 0xff000000) >> 24)) << 8) & 0xff0000)
      //   | ((((pui[i] & 0xff00) >> 8) * ((pui[i] & 0xff000000) >> 24)) & 0xff00)
      //   | (((pui[i] & 0xff) * ((pui[i] & 0xff000000) >> 24)) >> 8);
      //pui++;
      point+=16;
   }

}

/*
extern "C"
double hypot(double x,double y)
{
   return sqrt(x*x + y*y);
}
*/
