#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "lecture.h"
#include "jpeg_writer.h"

int16_t ****zigzag(struct jpeg *jpg, enum color_component color, int16_t ****mcu){
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
  //allocation
  int16_t ****mcu_zz = (int16_t ****)(malloc(nb_mcu*sizeof(int16_t ***)));
  for (uint32_t s =0; s<nb_mcu; s++){
      mcu_zz[s] = (int16_t ***)malloc(v*sizeof(int16_t**));
      for(uint32_t i=0; i<v;i++){
          mcu_zz[s][i] = (int16_t **)malloc(h*sizeof(int16_t*));
          for(uint32_t j=0; j<h;j++){
              mcu_zz[s][i][j] = (int16_t *)malloc(64*sizeof(int16_t));
          }
      }
  }

  for(uint32_t s=0; s<nb_mcu; s++){
      for(uint32_t i=0; i<v;i++){
          for(uint32_t j=0; j<h; j++){
             uint32_t element = 0;
             mcu_zz[s][i][j][0] = mcu[s][i][j][0];
             for (uint32_t r = 1; r < 64; r++) {
               /* A droite */
               if ((((element / 8) == 0) & ((element % 2) == 0)) |( ((element / 8) == 7) & ((element % 2) == 0))){
                 element = element + 1;
               }
               /* A gauche */
               else if ((((element % 8) == 0) & ((element % 16) != 0)) |( ((element % 8) == 7) & ((element % 16) == 15))) {
                 element = element + 8;
               }
               /* Diagonale */
               else{
                 /* Monte */
                 if ((element / 8 + element % 8) % 2 == 0){
                   element = element - 7;
                 }
                 /* Descend */
                 else{
                   element = element + 7;
                 }
               }
               mcu_zz[s][i][j][r] = mcu[s][i][j][element];
               }
          }
        }
  }
  return mcu_zz;
}
