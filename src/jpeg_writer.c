#include <stdlib.h>
#include <stdint.h>
#include "jpeg_writer.h" 
#include <stdbool.h>
#include "bitstream.h"
#include "huffman.h"
struct jpeg *jpeg_create(void){
    //allouer filenames
    struct jpeg *jpg=(struct jpeg *)malloc(sizeof(struct jpeg));
    return jpg;
}

void jpeg_set_ppm_filename(struct jpeg *jpg, const char *ppm_filename){
    jpg->ppm_filename = (char *) ppm_filename;
}
char *jpeg_get_ppm_filename(struct jpeg *jpg){
    char *ppm;
    ppm = jpg->ppm_filename;
    return ppm;
}
void jpeg_set_jpeg_filename(struct jpeg *jpg, const char *jpeg_filename){
    jpg->jpeg_filename =(char *) jpeg_filename;
}
char *jpeg_get_jpeg_filename(struct jpeg *jpg){
    return jpg->jpeg_filename;
}

void jpeg_set_image_height(struct jpeg *jpg, uint32_t image_height){
    jpg->image_height = image_height;
}
uint32_t jpeg_get_image_height(struct jpeg *jpg){
    uint32_t img_height;
    img_height = jpg->image_height;
    return img_height;
}

void jpeg_set_image_width(struct jpeg *jpg, uint32_t image_width){
    jpg->image_width = image_width;
}
uint32_t jpeg_get_image_width(struct jpeg *jpg){
    uint32_t img_width = jpg->image_width;
    return img_width;
}

void jpeg_set_nb_components(struct jpeg *jpg, uint8_t nb_components){
    jpg->nb_components = nb_components;
}
uint8_t jpeg_get_nb_components(struct jpeg *jpg){
    uint8_t nb_components = jpg->nb_components;
    return nb_components;
}
void jpeg_set_sampling_factor(struct jpeg *jpg, enum color_component cc,
                                enum direction dir, uint8_t sampling_factor){
     jpg->sampling_factor[cc][dir] = sampling_factor;
}
uint8_t jpeg_get_sampling_factor(struct jpeg *jpg, enum color_component cc,
                                enum direction dir){

    uint8_t sf= jpg->sampling_factor[cc][dir];
    return sf;
}

void jpeg_set_huffman_table(struct jpeg *jpg, enum sample_type acdc, 
                            enum color_component cc,
                            struct huff_table *htable){
    jpg->huffman_tables[cc][acdc] = htable;
}
struct huff_table *jpeg_get_huffman_table(struct jpeg *jpg, enum sample_type acdc, 
                                          enum color_component cc){
       struct huff_table *ht_cc_acdc = jpg->huffman_tables[cc][acdc];
       return ht_cc_acdc;
}

void jpeg_set_quantization_table(struct jpeg *jpg, enum color_component cc,
                                 uint8_t *qtable){
     jpg->qt[cc]=qtable;
}

uint8_t *jpeg_get_quantization_table(struct jpeg *jpg, enum color_component cc){
    uint8_t *qt_cc = jpg->qt[cc];
    return qt_cc;

}

struct bitstream *jpeg_get_bitstream(struct jpeg *jpg){
    struct bitstream *bs = jpg->bs;
    return bs;
}

void jpeg_write_header(struct jpeg *jpg){
    struct bitstream *bs = bitstream_create(jpeg_get_jpeg_filename(jpg));
    jpg->bs = bs;
    bool is_marker = true;
    bitstream_write_bits(bs, 0xffd8, 16, is_marker);
    // première entete
    bitstream_write_bits(bs, 0xffe0, 16, is_marker);
    bitstream_write_bits(bs, 0x0010, 16, !is_marker);
    bitstream_write_bits(bs, 0x4a, 8, !is_marker);
    bitstream_write_bits(bs, 0x46, 8, !is_marker);
    bitstream_write_bits(bs, 0x49, 8, !is_marker);
    bitstream_write_bits(bs, 0x46, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x01, 8, !is_marker);
    bitstream_write_bits(bs, 0x01, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    // table de quantification Y
    bitstream_write_bits(bs, 0xffdb, 16, is_marker);
    bitstream_write_bits(bs, 0x0043, 16, !is_marker);
    bitstream_write_bits(bs, 0x0, 4, !is_marker);
    bitstream_write_bits(bs, 0x0, 4, !is_marker);
    uint8_t *tab_quant_Y = jpeg_get_quantization_table(jpg, Y);
    for (uint8_t i=0; i<64; i++ ){
      bitstream_write_bits(bs, tab_quant_Y[i], 8, !is_marker);
    }
    uint8_t nb_comp = jpeg_get_nb_components(jpg);
    // table de quantification Cb Cr si besoin
    if (nb_comp == 3){
      bitstream_write_bits(bs, 0xffdb, 16, is_marker);
      bitstream_write_bits(bs, 0x0043, 16, !is_marker);
      bitstream_write_bits(bs, 0x0, 4, !is_marker);
      bitstream_write_bits(bs, 0x1, 4, !is_marker);
      uint8_t *tab_quant_cb = jpeg_get_quantization_table(jpg, Cb);
      for (uint8_t i=0; i<64; i++ ){
        bitstream_write_bits(bs, tab_quant_cb[i], 8, !is_marker);
      }
    }
    // Start of frame
    uint8_t longueur = 8 + 3*nb_comp;
    bitstream_write_bits(bs, 0xffc0, 16, is_marker);
    bitstream_write_bits(bs, longueur, 16, !is_marker);
    bitstream_write_bits(bs, 0x08, 8, !is_marker);
    uint32_t width = jpeg_get_image_width(jpg);
    uint32_t height = jpeg_get_image_height(jpg);
    bitstream_write_bits(bs, height, 16, !is_marker);
    bitstream_write_bits(bs, width, 16, !is_marker);
    bitstream_write_bits(bs, nb_comp, 8, !is_marker);
    uint8_t h1 = jpeg_get_sampling_factor(jpg, Y, H);
    uint8_t v1 = jpeg_get_sampling_factor(jpg, Y, V);
    bitstream_write_bits(bs, 0x01, 8, !is_marker);
    bitstream_write_bits(bs, h1, 4, !is_marker);
    bitstream_write_bits(bs, v1, 4, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);

    if (nb_comp == 3){
      uint8_t h2 = jpeg_get_sampling_factor(jpg, Cb, H);
      uint8_t v2 = jpeg_get_sampling_factor(jpg, Cb, V);
      uint8_t h3 = jpeg_get_sampling_factor(jpg, Cr, H);
      uint8_t v3 = jpeg_get_sampling_factor(jpg, Cr, V);
      bitstream_write_bits(bs, 0x02, 8, !is_marker);
      bitstream_write_bits(bs, h2, 4, !is_marker);
      bitstream_write_bits(bs, v2, 4, !is_marker);
      bitstream_write_bits(bs, 0x01, 8, !is_marker);
      bitstream_write_bits(bs, 0x03, 8, !is_marker);
      bitstream_write_bits(bs, h3, 4, !is_marker);
      bitstream_write_bits(bs, v3, 4, !is_marker);
      bitstream_write_bits(bs, 0x01, 8, !is_marker);
    }
    // DHT Y
    struct huff_table *ht_dc_y = jpeg_get_huffman_table(jpg, DC, Y);
    uint8_t *ht_dc_y_symbol = huffman_table_get_symbols(ht_dc_y);
    uint8_t *ht_dc_y_longueur_symbol = huffman_table_get_length_vector(ht_dc_y);
    bitstream_write_bits(bs, 0xffc4, 16, is_marker);
    longueur = 16 + 3;
    for (uint8_t i = 0; i<16; i++){
      longueur += (ht_dc_y_longueur_symbol[i]);
    }
    bitstream_write_bits(bs, longueur, 16, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    uint8_t nb_symbols_to_write = longueur-19;
    for (uint8_t i = 0; i<16; i++){
      bitstream_write_bits(bs, ht_dc_y_longueur_symbol[i], 8, !is_marker);
    }
    for (uint8_t i = 0; i<nb_symbols_to_write; i++){
      bitstream_write_bits(bs, ht_dc_y_symbol[i], 8, !is_marker);
    }

    struct huff_table *ht_ac_y = jpeg_get_huffman_table(jpg, AC, Y);
    uint8_t *ht_ac_y_symbol = huffman_table_get_symbols(ht_ac_y);
    uint8_t *ht_ac_y_longueur_symbol = huffman_table_get_length_vector(ht_ac_y);
    bitstream_write_bits(bs, 0xffc4, 16, is_marker);
    longueur = 16+3;
    for (uint8_t i = 0; i<16; i++){
      longueur += ht_ac_y_longueur_symbol[i];
    }
    bitstream_write_bits(bs, longueur, 16, !is_marker);
    bitstream_write_bits(bs, 0x10, 8, !is_marker);
    nb_symbols_to_write = longueur-19;
    for (uint8_t i = 0; i<16; i++){
      bitstream_write_bits(bs, ht_ac_y_longueur_symbol[i], 8, !is_marker);
    }
    for (uint8_t i = 0; i<nb_symbols_to_write; i++){
      bitstream_write_bits(bs, ht_ac_y_symbol[i], 8, !is_marker);
    }
    // DHT cb cr si besoin
    if (nb_comp == 3){
      struct huff_table *ht_dc_cb = jpeg_get_huffman_table(jpg, DC, Cb);
      uint8_t *ht_dc_cb_symbol = huffman_table_get_symbols(ht_dc_cb);
      uint8_t *ht_dc_cb_longueur_symbol = huffman_table_get_length_vector(ht_dc_cb);
      bitstream_write_bits(bs, 0xffc4, 16, is_marker);
      longueur = 16 + 3;
      for (uint8_t i = 0; i<16; i++){
        longueur += (ht_dc_cb_longueur_symbol[i]);
      }
      bitstream_write_bits(bs, longueur, 16, !is_marker);
      bitstream_write_bits(bs, 0x01, 8, !is_marker);
      for (uint8_t i = 0; i<16; i++){
        bitstream_write_bits(bs, ht_dc_cb_longueur_symbol[i], 8, !is_marker);
      }
      nb_symbols_to_write = longueur-19;
      for (uint8_t i = 0; i<nb_symbols_to_write; i++){
        bitstream_write_bits(bs, ht_dc_cb_symbol[i], 8, !is_marker);
      }
      
      struct huff_table *ht_ac_cb = jpeg_get_huffman_table(jpg, AC, Cb);
      uint8_t *ht_ac_cb_symbol = huffman_table_get_symbols(ht_ac_cb);
      uint8_t *ht_ac_cb_longueur_symbol = huffman_table_get_length_vector(ht_ac_cb);
      bitstream_write_bits(bs, 0xffc4, 16, is_marker);
      longueur = 16 + 3;
      for (uint8_t i = 0; i<16; i++){
        longueur += (ht_ac_cb_longueur_symbol[i]);
      }
      bitstream_write_bits(bs, longueur, 16, !is_marker);
      bitstream_write_bits(bs, 0x11, 8, !is_marker);
      for (uint8_t i = 0; i<16; i++){
        bitstream_write_bits(bs, ht_ac_cb_longueur_symbol[i], 8, !is_marker);
      }
      nb_symbols_to_write = longueur -19;
      for (uint8_t i = 0; i<nb_symbols_to_write; i++){
        bitstream_write_bits(bs, ht_ac_cb_symbol[i], 8, !is_marker);
      }
    }

    // Start of scan
    bitstream_write_bits(bs, 0xffda, 16, is_marker);
    longueur = 2*nb_comp + 6;
    bitstream_write_bits(bs, longueur, 16, !is_marker);
    bitstream_write_bits(bs, nb_comp, 8, !is_marker);
    bitstream_write_bits(bs, 0x01, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    if(nb_comp ==3){
        bitstream_write_bits(bs, 0x02, 8, !is_marker);
        bitstream_write_bits(bs, 0x11, 8, !is_marker);
        bitstream_write_bits(bs, 0x03, 8, !is_marker);
        bitstream_write_bits(bs, 0x11, 8, !is_marker);
    }
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
    bitstream_write_bits(bs, 0x3f, 8, !is_marker);
    bitstream_write_bits(bs, 0x00, 8, !is_marker);
}

void jpeg_write_footer(struct jpeg *jpg){
    struct bitstream *bs = jpeg_get_bitstream(jpg);
    bool is_marker=true;
    bitstream_write_bits(bs, 0xffd9, 16, is_marker);
}

void jpeg_destroy(struct jpeg *jpg){
    bitstream_destroy(jpg->bs);
    free(jpg->jpeg_filename);
    free(jpg);
}
