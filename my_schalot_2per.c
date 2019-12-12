/*
問題---一次元配列変換後、文字ではなく数字が表示される
　　　　tmp[i-1]修正
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

#include <openssl/engine.h>

#include <time.h>

#define BN_PUT(bn){printf(#bn "=%s (0x%s)\n", BN_bn2dec(bn), BN_bn2hex(bn)); }

#define BASE32_ALPHABET	"abcdefghijklmnopqrstuvwxyz234567"

#define SHA1CircularShift(n,x) (((x)<<(n))|((x)>>(32-(n))))

int main(){
  uint8_t buf[SHA_DIGEST_LENGTH],onion[17], *tmp, *der;
  signed int derlen;
  RSA *rsa = NULL;
  int loop=0;
  int cou=0;

  //ファイル読み込みと比較用変数
  FILE *fp;
  int i=0,j=0,z=0;
  char ch,con[16];
  // char *tank;
  char tank[37122][17];
  int k=0,l=0,m=0;
  char *p[]={"zwapwthn2pmul4v3","mda3nxsigriahnxq","zu65yc2xgr7szuiu"};

  clock_t start, end;

  // tank = (char *)malloc(sizeof(char) * 37122 *16);
  //
  // if((fp=fopen("190413.txt","r"))==NULL){
	//   printf("Not open file!");
  // }else{
  //   while((ch = fgetc(fp)) != EOF){
  //     tank[i]=ch;
  //     if(tank[i]=='.'){
  //       tank[i]=' ';
  //       i--;
  //       //.onion\n分飛ばす
  //       while(z!=6){
  //         ch = fgetc(fp);
  //         z++;
  //       }
  //       z=0;
  //     }
  //     i++;
  //     if(i==37122*16+1) break;
  //   }
  // }

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

  start = clock();

  while(loop!=1){

//RSAの公開鍵生成--------------------------------
    //RSA_KEYS_BITLEN---1024  RSA_E_START---0xFFFFFFu + 2
    rsa = RSA_generate_key(1024, 0xFFFFFFu + 2,NULL, NULL);

    // printf("P\n");
    // BN_PUT(rsa->p);
    // printf("Q\n");
    // BN_PUT(rsa->q);
    // printf("N\n");
    // BN_PUT(rsa->n);
    // printf("E\n");
    // BN_PUT(rsa->e);
    // printf("D\n");
    // BN_PUT(rsa->d);

//DERエンコード----------------------------------
    if((derlen = i2d_RSAPublicKey(rsa, NULL)) < 0)
			printf("DER encoding failed!\n");
		if ((der = tmp = (uint8_t *)malloc(derlen)) == NULL)
			printf("malloc(derlen) failed!\n");
		if (i2d_RSAPublicKey(rsa, &tmp) != derlen)//可変長[¥nが必要に]
			printf("DER encoding failed!\n");

    //printf("DER%d\n",derlen);//ずっと141
    //printf("TMP%x\n",*tmp);

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
    //printf("%d\n",*(tmp+i-1));

    j=0;
    W[64]=0;
    for(i=1;i<=16;i++){
      W[j]|=tmp[i-1]<<8*((j+1)*4-i);//tmp[i-1];
      //printf("TMP%x\n",*(tmp+i-1));
      if(i%4==0){
        j++;
      }
    }
//------ここまでは違う----------

    W[16/4] |= 0x8<<(4+(7-(16%4)*2));
    W[15]|=16*8;

    W[64]=0;

    for(i=16;i<80;i++){
      W[i]=SHA1CircularShift(1,W[i-3]^W[i-8]^W[i-14]^W[i-16]);
    }

//-----ここからは一緒-----------

    a=H[0];
    b=H[1];
    c=H[2];
    d=H[3];
    e=H[4];

    // printf("H\n");
    // for(i=0;i<5;i++){
    //   printf("%x",result_H[i]);
    // }
    // printf("\n");


    // SHA1に問題ありそう
    //暗号の本あった...



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

//H[0~5]は全て値は同じ

    // printf("%u\n",H[0]);
    // printf("%u\n",H[1]);
    // printf("%u\n",H[2]);
    // printf("%u\n",H[3]);
    // printf("%u\n",H[4]);

//c,d,eが2562383102で固定

    // printf("%u\n",a);
    // printf("%u\n",b);
    // printf("%u\n",c);
    // printf("%u\n",d);
    // printf("%u\n",e);

//result_H---32bit
    result_H[0]=a+H[0];
    result_H[1]=b+H[1];
    result_H[2]=c+H[2];
    result_H[3]=d+H[3];
    result_H[4]=e+H[4];

    //原因はc.d.eが毎回ループしているから

    // printf("%u\n",result_H[1]);
    // 毎回異なる値を出力
    // printf("%u\n",result_H[3]);
    //2834116980
    // printf("%u\n",result_H[4]);
    // 1552793326

    // printf("result_H\n");
    // for(i=0;i<5;i++){
    //   printf("%x",result_H[i]);
    // }
    // printf("\n");
    // 3175b9fca8ed31745c8dbeeeが全て同じ　40桁

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

    // printf("%u\n",buf[7]);
    //毎回異なる値を出力
    // printf("%u\n",buf[8]);
    //ずっと49
    // printf("%u\n",buf[9]);
    //ずっと117

// ２桁目以降の値同じ
    // printf("buf\n");
    // for(i=0;i<10;i++){
    //   printf("%x",buf[i]);
    // }
    // printf("\n");
    //最終桁の3175がずっと同じ


// // Base32

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


    cou = cou + 1;
    // printf("%c\n",(char)onion[15]);
    //ずっと118

    //1000個で20.03秒---printf有り-比較あり
    //1000個で19.30秒---printf無し-比較あり
    //1000個で18.49秒---printf無し-比較無し

    if(cou%1000==0){
      end = clock();
      printf("%.2f秒かかりました\n",(double)(end-start)/CLOCKS_PER_SEC);
      printf("%d\n",cou);
    }

    // printf("14~16桁\n");
    // printf("%x",onion[12]);
    // printf("%x",onion[13]);
    // printf("%x",onion[14]);
    // printf("%x",onion[15]);
    // printf("\n");
    //14~16桁目ずっと同じ 13桁目は違う

    // printf("テスト\n");
    // for(i=0;i<16;i++){
    //   printf("%c",onion[i]);
    // }
    // printf("\n");

    //6d6c76

    //printf("本番\n");
    for(i=0;i<16;i++){
      printf("%c",onion[i]);
    }
    printf("\n");

    //秘密鍵を表示
    //BN_PUT(rsa->q);

    //同じ鍵ができてる時がある
    //最後の3文字がmlv

    for(i=0;i<16;i++){
      con[i]=(char)onion[i];
    }

  //   //比較
  //   for(i=0;i<37122*16+1;i++){
  //     printf("%d\n",i);
  //     if(*(tank+i)==con[j]){
  //       j++;
  //       if(j==15){
  //         printf("発見\n");
  //         for(j=0;j<16;j++){
  //           printf("%c",con[j]);
  //         }
  //         printf("\n");
  //         j=0;
  //       }
  //     }else{
  //       j=0;
  //     }
  //   }
  // }


    while(k<3){
      for(i=0;i<37122;i++){
        for(j=0;j<16;j++){
          if(p[k][j]==tank[i][j]){
            l++;
            if(l==16){
              printf("発見\n");
              for(m=0;m<16;m++){
                printf("%c",tank[i][m]);
                if(m==15){
                  printf("\n");
                }
              }
            }
          }else{
            l=0;
          }
        }
      }
      k++;
    }
  }
  free(tank);

  return 0;
}
