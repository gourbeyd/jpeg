#include "lecture.h"
#include "ppm2jpeg.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "jpeg_writer.h"

uint8_t ***decoupe_blocs(struct jpeg *jpg, uint8_t **matrice){
  uint32_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
  uint32_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
  uint32_t height = jpeg_get_image_height(jpg);
  uint32_t width = jpeg_get_image_width(jpg);

  uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
  uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
  //allocation
  uint8_t ***matrice_blocs =(uint8_t ***) malloc(nb_bloc_height*sizeof(uint8_t **));
  for(uint32_t i = 0; i<nb_bloc_height;i++){
    matrice_blocs[i] = (uint8_t **)malloc(nb_bloc_width*sizeof(uint8_t *));
    for(uint32_t j =0; j<nb_bloc_width; j++){
        matrice_blocs[i][j] = (uint8_t *)malloc(64*sizeof(uint8_t));
        }
  }
  for(uint32_t  ib=0; ib<nb_bloc_height;ib++){
     for(uint32_t jb=0; jb<nb_bloc_width; jb++){
     uint32_t i = 8*ib;
     uint32_t k= 0;
     while (i<8*ib+8){
            uint32_t j = 8*jb;
            while(j<8*jb+8){
               if (i<height && j<width){
                    matrice_blocs[ib][jb][k] = matrice[i][j];
                    j++;
                    k++;
                   }
               else if(i>=height && j<width){
                   matrice_blocs[ib][jb][k] = matrice[height-1][j];
                    j++;
                    k++;
                   }
               else if(i<height && j>=width){
                    matrice_blocs[ib][jb][k] =matrice[i][width-1];
                    j++;
                    k++;
                   }
               else{
                    matrice_blocs[ib][jb][k] = matrice[height-1][width-1];
                    j++;
                    k++;
                   }
            }
            i++;
         }
    }
  }
  return matrice_blocs;
}

uint8_t ****decoupe_MCU(struct jpeg *jpg, uint8_t **pixels){
    uint32_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
    uint32_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
    uint32_t height = jpeg_get_image_height(jpg);
    uint32_t width = jpeg_get_image_width(jpg);


    uint32_t nb_blocs_mcu = h1*v1;
    uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
    uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
    uint32_t nb_mcu_width = nb_bloc_width/h1;
    uint32_t nb_mcu = (nb_bloc_width*nb_bloc_height)/(nb_blocs_mcu);


    uint8_t ***blocs = decoupe_blocs(jpg, pixels);
    //allocation
    uint8_t ****mcu = (uint8_t ****) malloc(nb_mcu*sizeof(uint8_t ***));
    for (uint32_t k = 0; k<nb_mcu; k++){
      mcu[k] = (uint8_t ***)malloc(v1*sizeof(uint8_t **));
      for(uint32_t i = 0; i<v1; i++){
        mcu[k][i] = (uint8_t **)malloc(64*h1*sizeof(uint8_t *));
        for(uint32_t j = 0; j<h1; j++){
//          mcu[k][i][j] = (uint8_t *)malloc(64*sizeof(uint8_t));
        }
      }
    }
    //selection des blocs pour chaque MCU
    for(uint32_t k =0; k<nb_mcu;  k++){
        uint32_t i = 0;
        for(uint32_t ib=v1*(k/nb_mcu_width);ib<v1*(k/nb_mcu_width)+v1; ib++){
            uint32_t j = 0;
            for(uint32_t jb=h1*(k%nb_mcu_width); jb<h1*(k%nb_mcu_width)+h1;jb++){
                mcu[k][i][j]= blocs[ib][jb];
                j++;
                }
            i++;
            }
        }
        for(uint32_t k=0; k<nb_bloc_height; k++){
            free(blocs[k]);
        }
        free(blocs);
    return mcu;

}


int moyenne_pixels(uint8_t *pixels, uint32_t taille)
{
    uint32_t s= 0;
    for(uint32_t k =0; k<taille;k++){
        s += pixels[k];
    }
    return (s/taille);
   }


uint8_t ****subsampling(struct jpeg *jpg, enum color_component color, uint8_t **pixels){
    uint32_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
    uint32_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
    uint32_t height = jpeg_get_image_height(jpg);
    uint32_t width = jpeg_get_image_width(jpg);


    uint32_t nb_blocs_mcu = h1*v1;
    uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
    uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
    uint32_t nb_mcu = (nb_bloc_width*nb_bloc_height)/(nb_blocs_mcu);
    uint8_t ****mcu_color = decoupe_MCU(jpg, pixels);
    uint8_t h = jpeg_get_sampling_factor(jpg, color, H);
    uint8_t v = jpeg_get_sampling_factor(jpg, color, V);
    if((color == Cb) | (color == Cr)) {
      //allocation
      uint8_t ****mcu_color_ds = (uint8_t ****) malloc(nb_mcu*sizeof(uint8_t ***));
      for (uint32_t k = 0; k<nb_mcu; k++){
        mcu_color_ds[k] = (uint8_t ***)malloc(v*sizeof(uint8_t **));
        for(uint32_t i = 0; i<v; i++){
          mcu_color_ds[k][i] = (uint8_t **)malloc(h*sizeof(uint8_t *));
          for(uint32_t j = 0; j<h; j++){
            mcu_color_ds[k][i][j] = (uint8_t *)malloc(64*sizeof(uint8_t));
          }
        }
      }
	uint8_t liste_pixels[(h1/h)*(v1/v)];
    uint32_t i, j, i_bloc, j_bloc, compteur;
    for(uint32_t k =0; k<nb_mcu; k++){
        for(uint32_t i_mcu=0; i_mcu<v; i_mcu++){
            for(uint32_t j_mcu=0; j_mcu<h; j_mcu++){
                for(uint32_t s =0; s<64; s++){
                            i = s/8 * v1/v + 8 * i_mcu;
					          j = s%8 * h1/h + 8 * j_mcu;
					          i_bloc = i/8;
					          j_bloc = j/8;
					          compteur = 0;
					          for(uint32_t i_dans_bloc = i%8; i_dans_bloc < i%8 + v1/v; i_dans_bloc++){
						            for(uint32_t j_dans_bloc = j%8; j_dans_bloc < j%8 + h1/h; j_dans_bloc++){
							              liste_pixels[compteur] = mcu_color[k][i_bloc][j_bloc][i_dans_bloc*8 + j_dans_bloc];
                            compteur++;
						            }
					          }
                mcu_color_ds[k][i_mcu][j_mcu][s] = moyenne_pixels(liste_pixels, (h1/h)*(v1/v));
                }


           }
        }
    }
    //free le mcu_color qui ne sert plus
    for(uint32_t k=0; k<nb_mcu;k++){
        for(uint32_t i=0; i<v1; i++){
            for(uint32_t j=0; j<h1;j++){
                free(mcu_color[k][i][j]);
            }
            free(mcu_color[k][i]);
        }
        free(mcu_color[k]);
    }
    free(mcu_color);
    return mcu_color_ds;
    }
    else {
      return mcu_color;
    }
    }
