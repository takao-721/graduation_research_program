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

int main(){
  uint8_t buf[20],onion[17], *tmp, *der;
  //ビッグエンディアン
  unsigned int e, e_be;
  signed int derlen;
  RSA *rsa = NULL;
  BIGNUM *big_e = BN_new();
  int loop=0;

  //ファイル読み込みと比較用変数
  FILE *fp;
  int i=0,j=0,z=0,k=0,l=0,m=0;
  char ch,tank[37122][17];
  //char *p;

  if((fp=fopen("190413.txt","r"))==NULL){
	  printf("Not open file!");
  }else{
	  while((ch = fgetc(fp)) != EOF){
      tank[i][j]=ch;
      j++;
      if(ch=='.'){
        tank[i][16]='\0';
        //.onion分飛ばす
        while(z!=6){
          ch = fgetc(fp);
          z++;
        }
        z=0;
        i++;
        j=0;
      }
      if(i==37122) break;
    }
  }

  while(loop!){
//RSAの公開鍵生成--------------------------------
    //RSA_KEYS_BITLEN---1024  RSA_E_START---0xFFFFFFu + 2
    rsa = RSA_generate_key(1024, 0xFFFFFFu + 2,NULL, NULL);

//DERエンコード----------------------------------
    if((derlen = i2d_RSAPublicKey(rsa, NULL)) < 0)
			error("DER encoding failed!\n");
		if ((der = tmp = (uint8_t *)malloc(derlen)) == NULL)
			error("malloc(derlen) failed!\n");
		if (i2d_RSAPublicKey(rsa, &tmp) != derlen)
			error("DER encoding failed!\n");

//SHA1ハッシュ-----------------------------------
    //SHA1変数----------------------------------
    int i=0,j=0;
    uint32_t W[80]={0};
    uint32_t a,b,c,d,e;
    uint32_t H[5]={0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    const uint32_t K[4]={0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
    uint32_t temp;
    uint32_t result_H[5];

//ポインタの配列とただの配列[i-1]は同じ意味か？
//uint8_t plain[] == tmp[]
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

  //&---and演算 ~---各ビットの反転
    for(j=0;j<20;j++){
      temp=SHA1CircularShift(5,a)+((b&c)|((~b)&d))+e+K[0]+W[j];
      e=d;
      d=c;
      SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }

    for(j=20;j<40;j++){
      temp=SHA1CircularShift(5,a)+(b^c^d)+e+K[1]+W[j];
      e=d;
      d=c;
      SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }

    for(j=40;j<60;j++){
      temp=SHA1CircularShift(5,a)+((b&c)|(b&d)|(c&d))+e+K[2]+W[j];
      e=d;
      d=c;
      SHA1CircularShift(30,b);
      b=a;
      a=temp;
    }

    for(j=60;j<80;j++){
      temp=SHA1CircularShift(5,a)+(b^c^d)+e+K[3]+W[j];
      e=d;
      d=c;
      SHA1CircularShift(30,b);
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

//比較
    for(i=0;i<37122;i++){
      for(j=0;j<16;j++){
        if(onion[j]==tank[i][j]){
          l++;
          if(l==16){
            printf("発見\n");
            for(m=0;m<16;m++){
              printf("%c",tank[i][m]);
              if(m==15){
                //秘密鍵d
                printf("秘密鍵:%d",rsa->d);
                printf("\n");
              }
            }
          }
        }else{
          l=0;
        }
      }
    }
  }

  return 0;
}
