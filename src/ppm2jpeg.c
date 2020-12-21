#include "qtables.h"
#include "lecture.h"
#include "dct.h"
#include "quantification.h"
#include "zigzag.h"
#include "subsampling.h"
#include "htables.h"
#include "jpeg_writer.h"
#include "huffman.h"
#include "bitstream.h"
#include "compression.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#define  _GNU_SOURCE

void afficher_options() {
  printf("Usage: ./ppm2jpeg image.{pgm, ppm} [options]\nOptions list:\n         --outfile filename: sets the output file name to filename ;\n         --sample h1xv1,h2xv2,h3xv3: sets the horizontal and vertical\n           sampling factors for components 1..3\n           Ex:--sample 2x2,1x1,1x1 ; \n         --quality: choose compression quality (from 1 to 9);\n         --help: prints this help.\n");
}

enum options{
  FILENAME,
  OUTFILE,
  SAMPLE,
  QUALITY
};


bool verify_sample(uint32_t h1, uint32_t v1,
                   uint32_t h2, uint32_t v2,
                   uint32_t h3, uint32_t v3){
    if ( h1*v1 + h2*v2 + h3*v3 > 10){
        printf("Error: h1v1 + h2v2 + h3v3 > 10");
        return false;
    }
    if ((h1 > 4) + (h1 < 1)){
        printf("Error: h1 should be between 1 and 4");
        return false;
    }
    if ((h2 > 4) + (h2 < 1)){
        printf("Error: h2 should be between 1 and 4");
        return false;
    }
    if ((h3 > 4) + (h3 < 1)){
        printf("Error: h3 should be between 1 and 4");
        return false;
    }
    if ((v1 > 4) + (v1 < 1)){
        printf("Error: v1 should be between 1 and 4");
        return false;
    }
    if ((v2 > 4) + (v2 < 1)){
        printf("Error: v2 should be between 1 and 4");
        return false;
    }
    if ((v3 > 4) + (v3 < 1)){
        printf("Error: v3 should be between 1 and 4");
        return false;
    }
    if (v1 % v2 != 0){
        printf("Error: v2 must divide v1");
        return false;
    }
    if (v1 % v3 != 0){
        printf("Error: v3 must divide v1");
        return false;
    }
   if (h1 % h2 != 0){
        printf("Error: h2 must divide h1");
        return false;
    }
    if (h1 % h3 != 0){
        printf("Error: h3 must divide h1");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{ bool options_passed[4] = {false, false, false, false};
  struct jpeg *jpg = jpeg_create();
  char *jpeg_name;
  uint32_t h1 = 1;
  uint32_t v1 = 1;
  uint32_t h2 = 1;
  uint32_t v2 = 1;
  uint32_t h3 = 1;
  uint32_t v3 = 1;
  int quality = 0;
  //Pas d'option
  if(argc == 1) {
    fprintf(stderr, "Missing one required argument\n");
    free(jpg);
    return -1;
  }
  //Une seule option donc soit help soit un fichier
  if (strcmp(argv[1], "--help") == 0) {
    afficher_options();
    free(jpg);
    return 0;
  }
  else if(strstr(argv[1], "-") == NULL){
    FILE* f_test = fopen(argv[1], "r");
    if (f_test == NULL){
  	   fprintf(stderr, "Cannot open file '%s'!\n", argv[1]);
       free(jpg);
  	    return -1;
    }
    fclose(f_test);
  }
  else{
    afficher_options();
    free(jpg);
    return 0;
  }
  jpeg_set_ppm_filename(jpg, argv[1]);
  options_passed[FILENAME] = true;

  jpeg_name=malloc(sizeof(char)*(strlen(jpeg_get_ppm_filename(jpg))+1));
  strcpy(jpeg_name, jpeg_get_ppm_filename(jpg));
  jpeg_name[strlen(jpeg_name)-3]='\0';
  strcat(jpeg_name, "jpg");
  jpeg_set_jpeg_filename(jpg,jpeg_name);
  //test si on a trop d'arguments
  if( argc > 5 ) {
    printf("Too many arguments supplied.\n");
    free(jpg);
    return -1;
  }
  //sinon on parcours les arguments et on évite les doublons
  else {
    for(int i = 2; i<argc; i++) {
      if (strstr(argv[i], "--sample=")!= NULL && !options_passed[SAMPLE]) {
        if (strlen(argv[i]) != 20) {
          printf("Sample invalid.\n");
          free(jpg);
          return -1;
        }
        h1 = argv[i][9] - '0';
        v1 = argv[i][11] - '0';
        h2 = argv[i][13] - '0';
        v2 = argv[i][15] - '0';
        h3 = argv[i][17] - '0';
        v3 = argv[i][19] - '0';
      }
      else if (strstr(argv[i], "--outfile=") != NULL && !options_passed[OUTFILE]) {
        jpeg_set_jpeg_filename(jpg, argv[i] + 10);
        options_passed[OUTFILE] = true;
      }
      else if (strstr(argv[i], "--quality=")!=NULL && !options_passed[QUALITY]){
        if ((strlen(argv[i])!=11) | (argv[i][10]-'0'>9) | (argv[i][10] - '0'<0)) {
          printf("invalid quality, quality should be between 0 and 9.\n");
          free(jpg);
          return -1;
        }
        quality=argv[i][10]-'0';
        options_passed[QUALITY] = true;
      }
      else {
        afficher_options();
        return -1;
      }
    }
  }
  //verification of sample format
  if(!verify_sample(h1, v1, h2, v2, h3, v3)){
    free(jpg);
    return -1;
  }
jpeg_set_sampling_factor(jpg, Y, H, h1);
jpeg_set_sampling_factor(jpg, Y, V, v1);
jpeg_set_sampling_factor(jpg, Cb, H, h2);
jpeg_set_sampling_factor(jpg, Cb, V, v2);
jpeg_set_sampling_factor(jpg, Cr, H, h3);
jpeg_set_sampling_factor(jpg, Cr, V, v3);

/*-----------Fin lecture arguments-------------*/


/* -------------------Lecture ppm/pgm ---------------------------*/
    uint8_t *rgb = read_rgb(jpg);
    uint32_t height = jpeg_get_image_height(jpg);
    uint32_t width = jpeg_get_image_width(jpg);
    if((jpeg_get_nb_components(jpg)==1) &&( (h1!=1) | (v1!=1))){
        printf("invalid sample for grey images \n");
        free(jpg);
        return -1;
        }
    //variables utiles
    uint32_t nb_blocs_mcu = h1*v1;
    uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
    uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
    uint32_t nb_mcu = (nb_bloc_width*nb_bloc_height)/(nb_blocs_mcu);
/* ----------------conversion RGB vers YCbCr ---------------------*/
    uint8_t **y = rgb_to_ycbcr(jpg, Y, rgb);
    uint8_t **cb = rgb_to_ycbcr(jpg, Cb, rgb);
    uint8_t **cr = rgb_to_ycbcr(jpg, Cr, rgb);
    free(rgb);
/* ----------------downsampling ---------------------*/
    uint8_t ****mcu_y = subsampling(jpg, Y, y);
    uint8_t ****mcu_cb = subsampling(jpg, Cb, cb);
    uint8_t ****mcu_cr = subsampling(jpg, Cr, cr);

    //libération de la mémoire
    for(uint32_t i=0; i<height;i++){
        free(y[i]);
        free(cb[i]);
        free(cr[i]);
    }
    free(y);
    free(cb);
    free(cr);

    //dct
    int16_t ****mcu_y_dct = dct(jpg, Y, mcu_y);
    int16_t ****mcu_cb_dct = dct(jpg, Cb, mcu_cb);
    int16_t ****mcu_cr_dct = dct(jpg, Cr, mcu_cr);

    //libération de la mémoire
    for(uint32_t k=0; k<nb_mcu; k++){
        for(uint32_t i=0; i<v1; i++){
            for(uint32_t j=0; j<h1; j++){
               free(mcu_y[k][i][j]);}
            free(mcu_y[k][i]);
            }
        for(uint32_t i=0; i<v2; i++){
            for(uint32_t j=0; j<h2; j++){
               free(mcu_cb[k][i][j]);}
            free(mcu_cb[k][i]);
            }
        for(uint32_t i=0; i<v3; i++){
            for(uint32_t j=0; j<h3; j++){
               free(mcu_cr[k][i][j]);}
            free(mcu_cr[k][i]);
            }
        free(mcu_y[k]);
        free(mcu_cb[k]);
        free(mcu_cr[k]);
    }
    free(mcu_y);
    free(mcu_cb);
    free(mcu_cr);

/* ----------------------ZIG ZAG-------------------------*/
    int16_t ****mcu_y_zz = zigzag(jpg, Y, mcu_y_dct);
    int16_t ****mcu_cb_zz = zigzag(jpg, Cb, mcu_cb_dct);
    int16_t ****mcu_cr_zz = zigzag(jpg, Cr, mcu_cr_dct);

    //libération de la mémoire
    for(uint32_t k=0; k<nb_mcu; k++){
        for(uint32_t i=0; i<v1; i++){
            for(uint32_t j=0; j<h1; j++){
               free(mcu_y_dct[k][i][j]);}
               free(mcu_y_dct[k][i]);
            }
        for(uint32_t i=0; i<v2; i++){
            for(uint32_t j=0; j<h2; j++){
               free(mcu_cb_dct[k][i][j]);}
               free(mcu_cb_dct[k][i]);
            }
        for(uint32_t i=0; i<v3; i++){
            for(uint32_t j=0; j<h3; j++){
               free(mcu_cr_dct[k][i][j]);}
               free(mcu_cr_dct[k][i]);
            }
        free(mcu_y_dct[k]);
        free(mcu_cb_dct[k]);
        free(mcu_cr_dct[k]);
    }
    free(mcu_y_dct);
    free(mcu_cb_dct);
    free(mcu_cr_dct);


    quantification(jpg, Y, mcu_y_zz, quality);
    quantification(jpg, Cb, mcu_cb_zz, quality);
    quantification(jpg, Cr, mcu_cr_zz, quality);


    //construction des tables ac/dc y
    uint8_t *dc_y_symbols = htables_symbols[DC][Y];
    uint8_t *nb_symb_per_length_dc_y = htables_nb_symb_per_lengths[DC][Y];
    uint8_t nb_symbols_dc_y = htables_nb_symbols[DC][Y];
    struct huff_table *ht_dc_y;
    ht_dc_y = huffman_table_build(nb_symb_per_length_dc_y, dc_y_symbols, nb_symbols_dc_y);

    uint8_t *ac_y_symbols = htables_symbols[AC][Y];
    uint8_t *nb_symb_per_length_ac_y = htables_nb_symb_per_lengths[AC][Y];
    uint8_t nb_symbols_ac_y = htables_nb_symbols[AC][Y];
    struct huff_table *ht_ac_y;
    ht_ac_y = huffman_table_build(nb_symb_per_length_ac_y, ac_y_symbols, nb_symbols_ac_y);

    jpeg_set_huffman_table(jpg, AC, Y, ht_ac_y);
    jpeg_set_huffman_table(jpg, DC, Y, ht_dc_y);
    jpeg_set_quantization_table(jpg, Y, qtables_Y[quality]);
    //construction des tables ac/dc cb
    uint8_t *dc_cb_symbols = htables_symbols[DC][Cb];
    uint8_t *nb_symb_per_length_dc_cb = htables_nb_symb_per_lengths[DC][Cb];
    uint8_t nb_symbols_dc_cb = htables_nb_symbols[DC][Cb];
    struct huff_table *ht_dc_cb;
    ht_dc_cb = huffman_table_build(nb_symb_per_length_dc_cb, dc_cb_symbols, nb_symbols_dc_cb);
    uint8_t *ac_cb_symbols = htables_symbols[AC][Cb];
    uint8_t *nb_symb_per_length_ac_cb = htables_nb_symb_per_lengths[AC][Cb];
    uint8_t nb_symbols_ac_cb = htables_nb_symbols[AC][Cb];
    struct huff_table *ht_ac_cb;
    ht_ac_cb = huffman_table_build(nb_symb_per_length_ac_cb, ac_cb_symbols, nb_symbols_ac_cb);
    jpeg_set_huffman_table(jpg, AC, Cb, ht_ac_cb);
    jpeg_set_huffman_table(jpg, DC, Cb, ht_dc_cb);
    jpeg_set_quantization_table(jpg, Cb, qtables_CbCr[quality]);
    jpeg_set_quantization_table(jpg, Cr, qtables_CbCr[quality]);


    //construction des tables ac/dc cr
    uint8_t *dc_cr_symbols = htables_symbols[DC][Cr];
    uint8_t *nb_symb_per_length_dc_cr = htables_nb_symb_per_lengths[DC][Cr];
    uint8_t nb_symbols_dc_cr = htables_nb_symbols[DC][Cr];
    struct huff_table *ht_dc_cr;
    ht_dc_cr = huffman_table_build(nb_symb_per_length_dc_cr, dc_cr_symbols, nb_symbols_dc_cr);
    uint8_t *ac_cr_symbols = htables_symbols[AC][Cr];
    uint8_t *nb_symb_per_length_ac_cr = htables_nb_symb_per_lengths[AC][Cr];
    uint8_t nb_symbols_ac_cr = htables_nb_symbols[AC][Cr];
    struct huff_table *ht_ac_cr;
    ht_ac_cr = huffman_table_build(nb_symb_per_length_ac_cr, ac_cr_symbols, nb_symbols_ac_cr);


    jpeg_set_huffman_table(jpg, AC, Cr, ht_ac_cr);
    jpeg_set_huffman_table(jpg, DC, Cr, ht_dc_cr);


    jpeg_write_header(jpg);
    struct bitstream *bs = jpeg_get_bitstream(jpg);
    compression(bs, jpg, mcu_y_zz, mcu_cb_zz, mcu_cr_zz);
    bitstream_flush(bs);
    huffman_table_destroy(ht_ac_y);
    huffman_table_destroy(ht_dc_y);
    huffman_table_destroy(ht_ac_cb);
    huffman_table_destroy(ht_dc_cb);
    huffman_table_destroy(ht_ac_cr);
    huffman_table_destroy(ht_dc_cr);
    jpeg_write_footer(jpg);
    jpeg_destroy(jpg);
   }
