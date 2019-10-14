#include<stdio.h>

int main(){
  FILE *fp;
  int i=0,j=0,z=0,k=0,l=0,m=0;
  char ch,tank[37122][17];
  char *p[]={"zwapwthn2pmul4v3","mda3nxsigriahnxq","zu65yc2xgr7szuiu"};

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

  for(i=0;i<37122;i++){
    for(j=0;j<16;j++){
      printf("%c",tank[i][j]);
      if(j==15){
        printf("\n");
      }
    }
  }

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

  return 0;
}
