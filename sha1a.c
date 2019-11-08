#include<stdio.h>
#include<stdint.h>//uint32_t用

#define SHA1CircularShift(n,x) (((x)<<(n))|((x)>>(32-(n))))

int main(){
  uint32_t a,b,c,d,e,T;
  uint32_t H[5]={0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
  const uint32_t K[4]={0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
  uint32_t W[100];
  W[80]=0;
  int j=0,i=0;
  int size=64;
  char plain[16];
  plain[0]='a';
  plain[1]='a';
  plain[2]='a';
  plain[3]='a';
  plain[4]='a';
  plain[5]='a';
  plain[6]='a';
  plain[7]='a';
  plain[8]='a';
  plain[9]='a';
  plain[10]='a';
  plain[11]='a';
  plain[12]='a';
  plain[13]='a';
  plain[14]='a';
  plain[15]='a';



  //パディング
  for(i=0;i<=64;i++){
    W[j]|=plain[i-1]<<8*((j+1)*4-i);
    if (i%4 == 0) {
      j++;
    }
  }

//ここまでは出力は同じ
  // for(i=0;i<16;i++){
  //   printf("%x\n",W[i]);
  // }
  //初期化不足

  W[size/4]|=0x8<<(4*(7-(size%4)*2));
  W[15]|=size*8;

  W[64]=0;
  for (i = 16 ; i < 80 ; i++){
    W[i]=SHA1CircularShift(1,W[i-3]^W[i-8]^W[i-14]^W[i-16]);
  }

//ここで値が毎回変化
  for(i=16;i<80;i++){
    printf("%u\n",W[i]);
  }

  a=H[0];
  b=H[1];
  c=H[2];
  d=H[3];
  e=H[4];

  for(j=0;j<20;j++){
    T = SHA1CircularShift(5, a) + ((b&c)|((~b)&d)) + e + K[0] + W[j];
    e=d;
    d=c;
    c = SHA1CircularShift(30, b);
    b=a;
    a=T;
  }

  for(j=20;j<40;j++){
    T = SHA1CircularShift(5, a) + (b^c^d) + e + K[1] + W[j];
    e=d;
    d=c;
    c = SHA1CircularShift(30, b);
    b=a;
    a=T;
  }

  for(j=40;j<60;j++){
    T = SHA1CircularShift(5, a) + ((b&c)|(b&d)|(c&d)) + e + K[2] + W[j];
    e=d;
    d=c;
    c = SHA1CircularShift(30, b);
    b=a;
    a=T;
  }

  for(j=60;j<80;j++){
    T = SHA1CircularShift(5, a) + (b^c^d) + e + K[3] + W[j];
    e=d;
    d=c;
    c = SHA1CircularShift(30, b);
    b=a;
    a=T;
  }

  H[0]+=a;
  H[1]+=b;
  H[2]+=c;
  H[3]+=d;
  H[4]+=e;

  // for(i=0;i<5;i++){
  //   printf("%u\n",H[i]);
  // }

  return 0;
}
