#include <stdio.h>
#include "lecture.h"
#include "subsampling.h"
#include "jpeg_writer.h"
#include <math.h>
#include <stdlib.h>
#include "ppm2jpeg.h"


int16_t ****dct(struct jpeg *jpg, enum color_component color, uint8_t ****mcu){
  uint32_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
  uint32_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
  uint32_t height = jpeg_get_image_height(jpg);
  uint32_t width = jpeg_get_image_width(jpg);


  uint32_t nb_blocs_mcu = h1*v1;
  uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
  uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
  uint32_t nb_mcu = (nb_bloc_width*nb_bloc_height)/(nb_blocs_mcu);

  uint32_t h = jpeg_get_sampling_factor(jpg, color, H);
  uint32_t v = jpeg_get_sampling_factor(jpg, color, V);
  //allocation
  int16_t ****mcu_dct = (int16_t ****)(malloc(nb_mcu*sizeof(int16_t ***)));
  for (uint32_t s =0; s<nb_mcu; s++){
      mcu_dct[s] = (int16_t ***)malloc(v*sizeof(int16_t**));
      for(uint32_t i=0; i<v;i++){
          mcu_dct[s][i] = (int16_t **)malloc(h*sizeof(int16_t*));
          for(uint32_t j=0; j<h;j++){
              mcu_dct[s][i][j] = (int16_t *)malloc(64*sizeof(int16_t));
          }
      }
  }


  float pi = 3.14159265359;

  double mat_c[8][8];
  double c_fois_pix[8][8];
   for(int i=0; i<8; i++){
    for(int j=0; j<8;j++){
        if(i==0){
            mat_c[i][j]=0.353553;  //1/sqrt(8)
        }
        else{
            mat_c[i][j]=0.5*cos((2*j+1)*i*pi/16); //0.5 = sqrt(2/8)
        }
    }
  }
  double somme;
  for (uint32_t s=0; s<nb_mcu; s++){
    for(uint32_t i=0; i<v; i++){
      for(uint32_t j=0; j<h; j++){
        //mcu[s][i][j] : le bloc qui correspond a pixels danns formule lijk image
           //construction c*pix
           for(int icp=0; icp<8; icp++){
                for(int jcp=0; jcp<8; jcp++){
                    somme=0;
                    for(int k=0; k<8;k++){
                        somme+=mat_c[icp][k]*(float) (mcu[s][i][j][k*8+jcp]-128);
                        }
                    c_fois_pix[icp][jcp]=somme;
                }
           }
           //construction de dct = c*pix*ct
           for(int i_dct=0; i_dct<8; i_dct++){
                for(int j_dct=0;j_dct<8;j_dct++){
                    somme=0;
                    for(int k=0; k<8;k++){
                        somme+=c_fois_pix[i_dct][k]*mat_c[j_dct][k];
                        }
                    mcu_dct[s][i][j][8*i_dct+j_dct]= somme;
                }
            }
          }
        }
      }
  return mcu_dct;
}
