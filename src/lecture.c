#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include "jpeg_writer.h"
#include "ppm2jpeg.h"
#include "assert.h"
#include <stdbool.h>
uint8_t *read_rgb(struct jpeg *jpg) //renvoie le format, les dimensions et le rgb du fichier passé en parametres:
{
   // lecture de l'intro du fichier pgm, sur 11 bytes:
   FILE *ptr;
   size_t len = 0;
   char * height_width = NULL;
   char * format = NULL;
   unsigned char c;
   ptr = fopen(jpeg_get_ppm_filename(jpg),"rb");  // r for read, b for binary
   getline(&format, &len, ptr); // premier ligne: format de l'Image (p5 etc)
   getline(&height_width, &len, ptr); // width   height : 2ème ligne
   while(height_width[0]=='#'){
           getline(&height_width, &len, ptr); // width   height : 2ème ligne
   }
   // on split la ligne précédente pour avoir height et width de type int
   uint32_t width = atoi(strtok(height_width, " "));
   uint32_t height= atoi(strtok(NULL, " "));
   // on récupère les echantillons rgb
   jpeg_set_image_height(jpg, height);
   jpeg_set_image_width(jpg, width);
   if(strcmp(format, "P5\n")==0){
     jpeg_set_nb_components(jpg, 1);
   }
   else if(strcmp(format, "P6\n")==0){
     jpeg_set_nb_components(jpg, 3);
   }
   else{
       printf("erreur, ni p6 ni p5 dans l'en tête \n");
        assert(true);

   }
   uint32_t taille = jpeg_get_nb_components(jpg)*height*width;
   uint8_t *rgb = (uint8_t *) malloc(taille*sizeof(uint8_t)); // le tableau rgb est construit selon une dimension, ligne 1
   // puis ligne2 etc
   uint32_t i = 0;
   uint32_t valeur_case;
   while (!feof(ptr)){                        // while not end of file
       c=fgetc(ptr);// get a character/byte from the file
       if ((i >=4) && (i < (taille +4))){
            valeur_case = c-'0';
            rgb[i - 4] = (uint8_t) (valeur_case+48); // +48 a cause de la table ascii
        }
        i++;
    }
    fclose(ptr);
    free(height_width);
    free(format);
    return rgb;
}

uint8_t **rgb_to_ycbcr(struct jpeg *jpg, enum color_component color, uint8_t *rgb){
    uint32_t height = jpeg_get_image_height(jpg);
    uint32_t width = jpeg_get_image_width(jpg);
    uint8_t **pixels = (uint8_t **)malloc(height*sizeof(uint8_t *));
    for(uint32_t i = 0; i<height; i++){
      pixels[i] = (uint8_t *)malloc(width*sizeof(uint8_t));
    }
    if (jpeg_get_nb_components(jpg) == 1){
        uint32_t current;
        for (uint32_t i=0; i<height; i++){
            for (uint32_t j=0; j<width; j++){
                current = width*i+j;
                pixels[i][j] = rgb[current];
            }
        }
    }
    else{
        uint32_t current = 0;
        uint32_t i,j;
        while(current<(width*height)){
         i = current/width;
         j = current%width;
         if (color == Y){
           pixels[i][j] = (uint8_t) (0.299*rgb[current*3] + 0.587*rgb[current*3+1] + 0.114*rgb[3*current +2]);
         }
         else if(color == Cb) {
           pixels[i][j] = (uint8_t) (-0.1687*rgb[current*3]-0.3313*rgb[current*3+1] + 0.5*rgb[3*current +2]+128);
         }
         else{
           pixels[i][j] = (uint8_t) (0.5*rgb[current*3] - 0.4187*rgb[current*3+1] -0.0813*rgb[3*current +2]+128);
         }
         current++;
         }
        }
    return pixels;
}
