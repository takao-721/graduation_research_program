/*
 CUDA版eschalot
卒業研究用
eschalot
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <time.h>

#include"cuda.h"

//OpenSSL---BN_new();,BN_set_word();
#include <openssl/bn.h>
//OpenSSL---RSA_generate_key();
#include <openssl/rsa.h>
//OpenSSL---i2d_RSAPublicKey();
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/engine.h>

#define __THREADS_X 128
#define __THREADS_Y 1
#define __THREADS_Z 1
#define __BLOCKS_Y 1

#define BN_PUT(bn) { printf(#bn "=%s (0x%s)\n", BN_bn2dec(bn), BN_bn2hex(bn)); }

#define BASE32_ALPHABET	"abcdefghijklmnopqrstuvwxyz234567"

//#define SHA1CircularShift(n,x) (((x)<<(n))|((x)>>(32-(n))))

__global__ void __calc_kernel(uint8_t *a1, int *b1,char *tank);

int main(){
  clock_t start,end;
  int counter=0;
  start = clock();
  uint8_t *tmp;
  signed int derlen;
  RSA *rsa = NULL;
  int loop=0;
  int cou_f=0;
  int n=10;

  //ファイル読み込みと比較用変数
  FILE *fp;
  int i=0,z=0;
  char ch;//tank[37122][17];
  //char *p;
  char *__host_tank;
  char *__dev_tank;

  BIGNUM *rsa_d;
  BIGNUM *rsa_e;

  uint8_t *__host_a1;
  uint8_t *__dev_a1;

  int *__host_b1;
  int *__dev_b1;

  __host_a1 = (uint8_t *) malloc(sizeof(uint8_t) * n * 16 + 1);
  __host_b1 = (int *) malloc(sizeof(int) * 1024);

  rsa_d=(BIGNUM *) malloc(sizeof(BIGNUM) * n);
  rsa_e=(BIGNUM *) malloc(sizeof(BIGNUM) * n);
  //printf("%d\n",sizeof(BIGNUM));
  //printf("test64\n");
  __host_tank = (char *)malloc(sizeof(char) * 37122 * 16 + 1);

  //printf("test67\n");
  if((fp=fopen("190413.txt","r"))==NULL){
    printf("Not open file!\n");
  }else{
    while((ch = fgetc(fp)) != EOF){
      __host_tank[i]=ch;
      if(__host_tank[i]=='.'){
        __host_tank[i]=' ';
        i--;
        //.onion\n分飛ばす
        while(z!=6){
          ch = fgetc(fp);
          z++;
        }
        z=0;
      }
      i++;
      if(i==37122*16+1) break;
    }
  }

  while(loop!=1){
    //    printf("test90\n");
    for(cou_f=0;cou_f<n;cou_f++){
      //RSAの公開鍵生成--------------------------------
      //RSA_KEYS_BITLEN---1024  RSA_E_START---0xFFFFFFu + 2
      rsa = RSA_generate_key(1024, 0xFFFFFFu + 2,NULL, NULL);
      rsa_d[cou_f]=*(rsa->d);
      rsa_e[cou_f]=*(rsa->e);

      //DERエンコード----------------------------------
      if((derlen = i2d_RSAPublicKey(rsa, NULL)) < 0)
			   printf("DER encoding failed!\n");
		  if ((tmp = (uint8_t *)malloc(derlen)) == NULL)
			   printf("malloc(derlen) failed!\n");
		  if (i2d_RSAPublicKey(rsa, &tmp) != derlen)
			   printf("DER encoding failed!\n");
      //printf("通っていますよ\n");
      __host_a1[cou_f]=*tmp;
    }


      //printf("test110\n");
    //n 個のグリッド生成
    dim3 __block(__THREADS_X, __THREADS_Y, __THREADS_Z);
    dim3 __grid(8, __BLOCKS_Y,__BLOCKS_Y);
    //printf("test114\n");

    //GPU 側のメモリ確保
    cudaMalloc((void **) &__dev_a1, (sizeof(uint8_t) * n * 16 + 1));
    //printf("%d\n",&__dev_a1);
    cudaMalloc((void **) &__dev_b1, (sizeof(int) * 1024));
    //printf("%d\n",&__dev_b1);
    cudaMalloc((void **) &__dev_tank, (sizeof(char) * n *37122 * 16 + 1));
    //printf("%d\n",&__dev_tank);printf("test120\n");

    //データ転送
    cudaMemcpy(__dev_a1, __host_a1, (sizeof(uint8_t) * n * 16 + 1),cudaMemcpyHostToDevice);
    //printf("%d\n",&__dev_a1);
    cudaMemcpy(__dev_b1, __host_b1, (sizeof(int) * 1024),cudaMemcpyHostToDevice);
    //printf("%d\n",&__dev_b1);
    cudaMemcpy(__dev_tank, __host_tank, (sizeof(char) * n * 37122 * 16 * 2 + 1),cudaMemcpyHostToDevice);
    //printf("%d\n",&__dev_tank);
    //printf("test 127\n");

    //カーネル関数呼出し
    __calc_kernel <<< __grid, __block >>> (__dev_a1, __dev_b1, __dev_tank);
    //printf("test131\n");
    //CPU 側に値を返してくる
    cudaMemcpy(__host_a1, __dev_a1, (sizeof(uint8_t) * n * 16 + 1),cudaMemcpyDeviceToHost);
    cudaMemcpy(__host_b1, __dev_b1, (sizeof(int) * 1024),cudaMemcpyDeviceToHost);

    for(i=0;i<n;i++){
      if(__host_b1[i]==1){
         printf("秘密鍵d\n");
         // BN_PUT(*(rsa_d+i));
         // printf("秘密鍵e\n");
         // BN_PUT(*(rsa_e+i));
         return 0;
      }
    }

    counter=counter+i;
    //printf("test136\n");
    if(counter>=100000){
      end=clock();
      printf("%.2f秒かかりました\n",(double)(end-start)/CLOCKS_PER_SEC );
    }

   }
   free(__host_a1);
   cudaFree(__dev_a1);
   free(__host_b1);
   cudaFree(__dev_b1);
   free(__host_tank);
   cudaFree(__dev_tank);

   return 0;
}


__global__ void __calc_kernel(uint8_t *a1, int *b1,char *tank){
  // unsigned int __tmp_idx_x = (threadIdx.x + (blockDim.x * blockIdx.x));
  // unsigned int __tmp_idx_y = (threadIdx.y + (blockDim.y * blockIdx.y));
  // unsigned int __tmp_size_x = (blockDim.x * gridDim.x);
  int idx = blockDim.x * blockIdx.x + threadIdx.x;
  // unsigned int id = (__tmp_idx_x + (__tmp_idx_y * __tmp_size_x));
  //SHA1ハッシュ-----------------------------------
  //SHA1変数----------------------------------
  uint8_t buf[20],onion[17];
  uint32_t W[80]={0};
  uint32_t a,b,c,d,e;
  uint32_t H[5]={0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
  const uint32_t K[4]={0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
  uint32_t temp;
  uint32_t result_H[5];

  int i=0,j=0,hyouzi=0;
  char con[16];

  //printf("test\n");

 //printf("通っていますね\n");

 //ポインタの配列とただの配列[i-1]は同じ意味か？
 //uint8_t plain[] ==uint8_t *tmp
 // tmp[i-1]
  j=0;  //これがないと値が同じになってしまう
  W[64]=0;  //これがないと値が同じになってしまう
  for(i=1;i<=16;i++){
    W[j]|=a1[i-1]<<8*((j+1)*4-i);
    if(i%4==0){
      j++;
    }
  }

  W[16/4] |= 0x8<<(4+(7-(16%4)*2));
  W[15]|=16*8;

  W[64]=0;

  // n,x --- (n,x) --- (((x)<<(n))|((x)>>(32-(n))))

  for(i=16;i<80;i++){
    W[i]=(((W[i-3]^W[i-8]^W[i-14]^W[i-16])<<(1))|((W[i-3]^W[i-8]^W[i-14]^W[i-16])>>(32-(1))));//SHA1CircularShift(1,W[i-3]^W[i-8]^W[i-14]^W[i-16]);
  }

  a=H[0];
  b=H[1];
  c=H[2];
  d=H[3];
  e=H[4];

  // c=が抜けると正しく計算できない

  //&---and演算 ~---各ビットの反転
  for(j=0;j<20;j++){
    temp=(((a)<<(5))|((a)>>(32-(5))))+((b&c)|((~b)&d))+e+K[0]+W[j];//SHA1CircularShift(5,a)+((b&c)|((~b)&d))+e+K[0]+W[j];
    e=d;
    d=c;
    c=(((b)<<(30))|((b)>>(32-(30))));//SHA1CircularShift(30,b);
    b=a;
    a=temp;
  }

  for(j=20;j<40;j++){
    temp=(((a)<<(5))|((a)>>(32-(5))))+(b^c^d)+e+K[1]+W[j];//SHA1CircularShift(5,a)+(b^c^d)+e+K[1]+W[j];
    e=d;
    d=c;
    c=(((b)<<(30))|((b)>>(32-(30))));//SHA1CircularShift(30,b);
    b=a;
    a=temp;
  }

  for(j=40;j<60;j++){
    temp=(((a)<<(5))|((a)>>(32-(5))))+((b&c)|(b&d)|(c&d))+e+K[2]+W[j];//SHA1CircularShift(5,a)+((b&c)|(b&d)|(c&d))+e+K[2]+W[j];
    e=d;
    d=c;
    c=(((b)<<(30))|((b)>>(32-(30))));//SHA1CircularShift(30,b);
    b=a;
    a=temp;
  }

  for(j=60;j<80;j++){
    temp=(((a)<<(5))|((a)>>(32-(5))))+(b^c^d)+e+K[3]+W[j];//SHA1CircularShift(5,a)+(b^c^d)+e+K[3]+W[j];
    e=d;
    d=c;
    c=(((b)<<(30))|((b)>>(32-(30))));//SHA1CircularShift(30,b);
    b=a;
    a=temp;
  }
  //result_H---32bit
  result_H[0]=a+H[0];
  result_H[1]=b+H[1];
  result_H[2]=c+H[2];
  result_H[3]=d+H[3];
  result_H[4]=e+H[4];

 //ビッグエンディアン
  buf[0] = result_H[0]>>24;
  buf[1] = result_H[0]>>16;
  buf[2] = result_H[0]>>8;
  buf[3] = result_H[0];
  buf[4] = result_H[1]>>24;
  buf[5] = result_H[1]>>16;
  buf[6] = result_H[1]>>8;
  buf[7] = result_H[1];
  buf[8] = result_H[2]>>24;
  buf[9] = result_H[2]>>16;

  // Base32
  onion[ 0] = BASE32_ALPHABET[ (buf[0] >> 3)	];
  onion[ 1] = BASE32_ALPHABET[((buf[0] << 2) | (buf[1] >> 6))	& 31];
  onion[ 2] = BASE32_ALPHABET[ (buf[1] >> 1) & 31];
  onion[ 3] = BASE32_ALPHABET[((buf[1] << 4) | (buf[2] >> 4))	& 31];
  onion[ 4] = BASE32_ALPHABET[((buf[2] << 1) | (buf[3] >> 7))	& 31];
  onion[ 5] = BASE32_ALPHABET[ (buf[3] >> 2)	& 31];
  onion[ 6] = BASE32_ALPHABET[((buf[3] << 3) | (buf[4] >> 5))	& 31];
  onion[ 7] = BASE32_ALPHABET[  buf[4]& 31];

  onion[ 8] = BASE32_ALPHABET[ (buf[5] >> 3)	];
  onion[ 9] = BASE32_ALPHABET[((buf[5] << 2) | (buf[6] >> 6))	& 31];
  onion[10] = BASE32_ALPHABET[ (buf[6] >> 1)& 31];
  onion[11] = BASE32_ALPHABET[((buf[6] << 4) | (buf[7] >> 4))	& 31];
  onion[12] = BASE32_ALPHABET[((buf[7] << 1) | (buf[8] >> 7))	& 31];
  onion[13] = BASE32_ALPHABET[ (buf[8] >> 2)& 31];
  onion[14] = BASE32_ALPHABET[((buf[8] << 3) | (buf[9] >> 5))	& 31];
  onion[15] = BASE32_ALPHABET[  buf[9]& 31];

  onion[16] = '\0';

  //printf("通っています2\n");

  // for(i=0;i<16;i++){
  //   printf("%c\n",onion[i]);
  // }
  // printf("\n");

  for(i=0;i<16;i++){
    con[i]=(char)onion[i];
  }

    //printf("test296\n");
  //比較
  for(i=0;i<37122*16+1;i++){
    if(*(tank+i)==con[j]){
      j++;
      if(j==15){
        printf("発見\n");
        j=j-15;
        while(hyouzi<16){
          printf("%c\n",con[j]);
          b1[idx]=1;
          hyouzi++;
          j++;
        }
        printf("\n");
        j=0;
      }
    }else{
     j=0;
    }
  }
  //printf("test");
}
