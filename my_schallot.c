/*
 2回目以降に生成される数字が全て同じ
*/

#include <stdio.h>
#include <stdint.h>

//OpenSSL---BN_new();,BN_set_word();
#include <openssl/bn.h>
//OpenSSL---RSA_generate_key();
#include <openssl/rsa.h>
//OpenSSL---i2d_RSAPublicKey();
#include <openssl/rsa.h>
#include <openssl/x509.h>

#define BASE32_ALPHABET	"abcdefghijklmnopqrstuvwxyz234567"

#define SHA1CircularShift(n,x) (((x)<<(n))|((x)>>(32-(n))))

int main(){
  uint8_t buf[20],onion[17], *tmp, *der;
  //ビッグエンディアン
  // unsigned int e_be;//unsigned int e,
  signed int derlen;
  RSA *rsa = NULL;
  // BIGNUM *big_e = BN_new();
  int loop=0;

  //ファイル読み込みと比較用変数
  FILE *fp;
  int i=0,j=0,z=0;
  char ch,con[16];//tank[37122][17];
  //char *p;
  char *tank;
  tank = (char *)malloc(sizeof(char) * 37122 *16+1);

  if((fp=fopen("190413.txt","r"))==NULL){
    printf("Not open file!");
  }else{
    while((ch = fgetc(fp)) != EOF){
      tank[i]=ch;
      if(tank[i]=='.'){
        tank[i]=' ';
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

  // if((fp=fopen("190413.txt","r"))==NULL){
	//   printf("Not open file!");
  // }else{
	//   while((ch = fgetc(fp)) != EOF){
  //     tank[i][j]=ch;
  //     j++;
  //     if(ch=='.'){
  //       tank[i][16]='\0';
  //       //.onion分飛ばす
  //       while(z!=6){
  //         ch = fgetc(fp);
  //         z++;
  //       }
  //       z=0;
  //       i++;
  //       j=0;
  //     }
  //     if(i==37122) break;
  //   }
  // }

  while(loop!=1){
//RSAの公開鍵生成--------------------------------
    //RSA_KEYS_BITLEN---1024  RSA_E_START---0xFFFFFFu + 2
    rsa = RSA_generate_key(1024, 0xFFFFFFu + 2,NULL, NULL);

//DERエンコード----------------------------------
    if((derlen = i2d_RSAPublicKey(rsa, NULL)) < 0)
			printf("DER encoding failed!\n");
		if ((der = tmp = (uint8_t *)malloc(derlen)) == NULL)
			printf("malloc(derlen) failed!\n");
		if (i2d_RSAPublicKey(rsa, &tmp) != derlen)
			printf("DER encoding failed!\n");

//SHA1ハッシュ-----------------------------------
    //SHA1変数----------------------------------
    uint32_t W[80]={0};
    uint32_t a,b,c,d,e;
    uint32_t H[5]={0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    const uint32_t K[4]={0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
    uint32_t temp;
    uint32_t result_H[5];

//ポインタの配列とただの配列[i-1]は同じ意味か？
//uint8_t plain[] ==uint8_t *tmp
// tmp[i-1]
    j=0;  //これがないと値が同じになってしまう
    W[64]=0;  //これがないと値が同じになってしまう
    for(i=1;i<=16;i++){
      W[j]|=tmp[i-1]<<8*((j+1)*4-i);
      if(i%4==0){
        j++;
      }
    }

    W[16/4] |= 0x8<<(4+(7-(16%4)*2));
    W[15]|=16*8;

    W[64]=0;

    for(i=16;i<80;i++){
      W[i]=SHA1CircularShift(1,W[i-3]^W[i-8]^W[i-14]^W[i-16]);
    }

    a=H[0];
    b=H[1];
    c=H[2];
    d=H[3];
    e=H[4];

    // c=が抜けると正しく計算できない

  //&---and演算 ~---各ビットの反転
    for(j=0;j<20;j++){
      temp=SHA1CircularShift(5,a)+((b&c)|((~b)&d))+e+K[0]+W[j];
      e=d;
      d=c;
      c=SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }

    for(j=20;j<40;j++){
      temp=SHA1CircularShift(5,a)+(b^c^d)+e+K[1]+W[j];
      e=d;
      d=c;
      c=SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }

    for(j=40;j<60;j++){
      temp=SHA1CircularShift(5,a)+((b&c)|(b&d)|(c&d))+e+K[2]+W[j];
      e=d;
      d=c;
      c=SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }

    for(j=60;j<80;j++){
      temp=SHA1CircularShift(5,a)+(b^c^d)+e+K[3]+W[j];
      e=d;
      d=c;
      c=SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }
//result_H---32bit
    result_H[0]=a+H[0];
    result_H[1]=b+H[1];
    result_H[2]=c+H[2];
    result_H[3]=d+H[3];
    result_H[4]=e+H[4];

    // printf("%u\n",a);
    // printf("%u\n",b);
    // printf("%u\n",c);
    // printf("%u\n",d);
    // printf("%u\n",e);

    // printf("%u\n",H[0]);
    // printf("%u\n",H[1]);
    // printf("%u\n",H[2]);
    // printf("%u\n",H[3]);
    // printf("%u\n",H[4]);

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

    for(i=0;i<16;i++){
      printf("%c",onion[i]);
    }
    printf("\n");

    for(i=0;i<16;i++){
      con[i]=(char)onion[i];
    }

//比較
    for(i=0;i<37122*16+1;i++){
      //printf("%d\n",i);
      if(*(tank+i)==con[j]){
        j++;
        if(j==15){
          printf("発見\n");
          for(j=0;j<16;j++){
            //printf("%c",con[j]);
            loop=1;
          }
          printf("\n");
          j=0;
        }
      }else{
        j=0;
      }
    }

    // for(i=0;i<37122;i++){
    //   for(j=0;j<16;j++){
    //     if(onion[j]==tank[i][j]){
    //       l++;
    //       if(l==16){
    //         printf("発見\n");
    //         for(m=0;m<16;m++){
    //           printf("%c",tank[i][m]);
    //           if(m==15){
    //             //秘密鍵d
    //             //printf("秘密鍵:%d",rsa->d);
    //             printf("\n");
    //           }
    //         }
    //       }
    //     }else{
    //       l=0;
    //     }
    //   }
    // }
  }

  return 0;
}
