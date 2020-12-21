#include "jpeg_writer.h"
#include "htables.h"
#include "lecture.h"
#include "huffman.h"
#include "bitstream.h"
#include "ppm2jpeg.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
int rle_code(int compteur, int magnitude){
    int code = 16*compteur + magnitude;
    return code;
}


int magnitude(int valeur){
    int puissance = 0;
    int divise = 1;
    while ( valeur / divise != 0) {
      puissance = puissance + 1;
      divise = divise * 2;
    }
    return puissance;
}

int to_index(int valeur, int magnitude){
    int index;
    if( valeur>=0){
        index = valeur;
    }
    else{
        index = pow(2,magnitude)+valeur-1;
        }
    return index;
}


void compression_bloc(struct bitstream *bs, enum color_component color, int16_t *bloc, int *ptr_predicateur){

    //construction de la table dc color
    int dc_value = bloc[0]- *ptr_predicateur;
    *ptr_predicateur = bloc[0];
    uint8_t *dc_color_symbols = htables_symbols[DC][color];
    uint8_t *nb_symb_per_length_dc_color = htables_nb_symb_per_lengths[DC][color];
    uint8_t nb_symbols_dc_color = htables_nb_symbols[DC][color];
    struct huff_table *ht_dc_color;
    ht_dc_color = huffman_table_build(nb_symb_per_length_dc_color, dc_color_symbols, nb_symbols_dc_color);

    //construction de la table ac color
    uint8_t *ac_color_symbols = htables_symbols[AC][color];
    uint8_t *nb_symb_per_length_ac_color = htables_nb_symb_per_lengths[AC][color];
    uint8_t nb_symbols_ac_color = htables_nb_symbols[AC][color];
    struct huff_table *ht_ac_color;
    ht_ac_color = huffman_table_build(nb_symb_per_length_ac_color, ac_color_symbols, nb_symbols_ac_color);

    //dc
    uint8_t *nb_bits= malloc(sizeof(uint8_t));
    uint8_t dc_magnitude = (uint8_t) magnitude(dc_value);
    uint32_t dc_index=(uint32_t)to_index(dc_value, dc_magnitude);
    uint32_t path = huffman_table_get_path(ht_dc_color, dc_magnitude, nb_bits);
    bool is_marker = false;
    bitstream_write_bits(bs, path, *nb_bits, is_marker);
    bitstream_write_bits(bs, dc_index, dc_magnitude, is_marker);
    //ac
    int compteur_restants = 63;
    int compteur_zero = 0;
    int i_non_nul = 0;
    for (int s=0; s<64; s++){
        if(bloc[s]!=0){
            i_non_nul= s;
        }
    }
    for(int i=1; i<i_non_nul +1; i++){
        int ac_value = bloc[i];
        if (ac_value == 0){
            compteur_zero++;
            }
        else{
            while(compteur_zero >= 16){
                uint32_t f0_index = to_index(0xf0,8);
                path = huffman_table_get_path(ht_ac_color, f0_index, nb_bits);
                bitstream_write_bits(bs, path, *nb_bits, is_marker);
                compteur_zero -= 16;
            }
            int ac_magnitude = magnitude(ac_value);
            uint32_t ac_index =(uint32_t)to_index(ac_value, ac_magnitude);
                uint8_t rlecode = (uint8_t) rle_code(compteur_zero, ac_magnitude);
                path = huffman_table_get_path(ht_ac_color, rlecode, nb_bits);
                bitstream_write_bits(bs, path, *nb_bits, is_marker);
                 bitstream_write_bits(bs, ac_index, ac_magnitude, is_marker);
                compteur_zero = 0;
        }
        compteur_restants--;


    }
    if(i_non_nul<63){
    //endof block en fct de color component
        uint32_t zero_index = 0;
        path = huffman_table_get_path(ht_ac_color, zero_index, nb_bits);
    if (color == Y) {
      bitstream_write_bits(bs, 10, 4, is_marker);
    }
    else{
      bitstream_write_bits(bs, 0, 2, is_marker);
    }
    }
    free(nb_bits);
    huffman_table_destroy(ht_dc_color);
    huffman_table_destroy(ht_ac_color);

}

void compression(struct bitstream *bs, struct jpeg *jpg, int16_t ****mcu_y, int16_t ****mcu_cb, int16_t ****mcu_cr){
    uint32_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
    uint32_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
    uint32_t h2 = jpeg_get_sampling_factor(jpg, Cb, H);
    uint32_t v2 = jpeg_get_sampling_factor(jpg, Cb, V);
    uint32_t h3 = jpeg_get_sampling_factor(jpg, Cr, H);
    uint32_t v3 = jpeg_get_sampling_factor(jpg, Cr, V);
    uint32_t height = jpeg_get_image_height(jpg);
    uint32_t width = jpeg_get_image_width(jpg);


    uint32_t nb_blocs_mcu = h1*v1;
    uint32_t nb_bloc_width = (uint32_t)(ceil((float) width/8.0)) + (h1 - (uint32_t)(ceil((float) width/8.0))%h1)*(((uint32_t)(ceil((float) width/8.0))%h1)!=0);
    uint32_t nb_bloc_height = (uint32_t)(ceil((float) height/8.0)) + (v1 - (uint32_t)(ceil((float) height/8.0))%v1)*(((uint32_t)(ceil((float) height/8.0))%v1)!=0);
    uint32_t nb_mcu = (nb_bloc_width*nb_bloc_height)/(nb_blocs_mcu);
    int predicateur_y = 0;
    int predicateur_cb =0;
    int predicateur_cr =0;
    for(uint32_t k =0; k<nb_mcu; k++){
        //compression des blocs y
        for(uint32_t i=0; i<v1; i++){
            for(uint32_t j=0; j<h1; j++){
                compression_bloc(bs, Y, mcu_y[k][i][j], &predicateur_y);
                free(mcu_y[k][i][j]);
            }
            free(mcu_y[k][i]);
       }
       free(mcu_y[k]);
          // compression des blocs cb
        for(uint32_t i=0; i<v2; i++)
        {
            for(uint32_t j=0; j<h2; j++){
                if(jpeg_get_nb_components(jpg)==3){
                compression_bloc(bs, Cb, mcu_cb[k][i][j], &predicateur_cb);}
                free(mcu_cb[k][i][j]);
            }
                free(mcu_cb[k][i]);
        }
        free(mcu_cb[k]);
        // comrpession des blocs cr
        for(uint32_t i=0; i<v3; i++)
        {
            for(uint32_t j=0; j<h3; j++){
                if(jpeg_get_nb_components(jpg)==3){
                compression_bloc(bs, Cr, mcu_cr[k][i][j], &predicateur_cr);}
                free(mcu_cr[k][i][j]);
            }
                free(mcu_cr[k][i]);
        }
        free(mcu_cr[k]);
    }
        free(mcu_cr);
        free(mcu_y);
        free(mcu_cb);

}
