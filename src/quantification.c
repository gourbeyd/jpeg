#include "lecture.h"
#include "zigzag.h"
#include "ppm2jpeg.h"
#include <stdint.h>
#include "qtables.h"
#include <math.h>
#include <stdio.h>

void quantify_luminance(int16_t *zigzag, int quality){
    int taille = 64;
    for (int i = 0; i < taille; i++) {
        zigzag[i] = zigzag[i] / qtables_Y[quality][i];
    }
}

void quantify_chrominance(int16_t *zigzag,int quality){
    int taille = 64;
    for (int i = 0; i < taille; i++) {
        zigzag[i] = zigzag[i] / qtables_CbCr[quality][i];
    }
}

void quantification(struct jpeg *jpg, enum color_component color, int16_t ****mcu_zz, int quality){
  uint32_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
  uint32_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
  uint32_t height = jpeg_get_image_height(jpg);
  uint32_t width = jpeg_get_image_width(jpg);


  uint32_t nb_blocs_mcu = h1*v1;
  uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
  uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
  uint32_t nb_mcu = (nb_bloc_width*nb_bloc_height)/(nb_blocs_mcu);
  uint8_t h = jpeg_get_sampling_factor(jpg, color, H);
  uint8_t v = jpeg_get_sampling_factor(jpg, color, V);
  if(color == Y){
    for(uint32_t k=0; k<nb_mcu; k++){
        for(uint32_t i=0; i<v; i++){
            for(uint32_t j=0; j<h; j++){
                 quantify_luminance(mcu_zz[k][i][j], quality);
            }
        }
    }
  }
  else{
    for(uint32_t k=0; k<nb_mcu; k++){
      for(uint32_t i=0; i<v; i++){
          for(uint32_t j=0; j<h; j++){
               quantify_chrominance(mcu_zz[k][i][j], quality);
          }
      }
    }
  }
}
