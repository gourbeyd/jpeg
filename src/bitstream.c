#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#define  _GNU_SOURCE

#define BUFFER_SIZE (8) //taille du buffer en nb d'octets


/* Renvois une chaine de caractères de longueur nombre bit qui est la
   représentation binaire de value (poids faible à gauche) */
char *binary_string(uint32_t value, uint8_t nb_bits){
  char *c = (char *)malloc((nb_bits+1)*sizeof(char));
  c[nb_bits] = '\0';
  for(uint8_t i = 0; i<nb_bits; i++){
    c[nb_bits - i - 1] = (value%2 && value!=0) ? '1' : '0';
    value/=2;
  }
  return c;
}

/*
    Type opaque représentant le flux d'octets à écrire dans le fichier JPEG de
    sortie (appelé bitstream dans le sujet).
*/
struct bitstream {
  FILE *outfile;
	size_t length;
	size_t data_size;
	uint8_t buffer[BUFFER_SIZE];
};

/* Retourne un nouveau bitstream prêt à écrire dans le fichier filename. */
struct bitstream *bitstream_create(const char *filename){
  struct bitstream *stream = (struct bitstream *)malloc(sizeof(struct bitstream));
  stream->outfile = fopen(filename, "wb");
  stream->data_size = BUFFER_SIZE;
  stream->length = 0;
  for(size_t i = 0; i<BUFFER_SIZE; i++){
    stream->buffer[i] = 0;
  }
  return stream;
}
/*
    Force l'exécution des écritures en attente sur le bitstream, s'il en
    existe.
*/
void bitstream_flush(struct bitstream *stream){
    for(uint32_t i = 0; i<stream->length/8;i++){
      fputc(stream->buffer[i], stream->outfile);
    }
    if(stream->length%8){
      fputc(stream->buffer[stream->length/8], stream->outfile);
      }
    // remise à zéro du buffer
    stream->length = 0;
    for(size_t i = 0; i<BUFFER_SIZE; i++){
        stream->buffer[i] = 0;
    }
    fflush(stream->outfile);
}

/*
    Ecrit nb_bits bits dans le bitstream. La valeur portée par cet ensemble de
    bits est value. Le paramètre is_marker permet d'indiquer qu'on est en train
    d'écrire un marqueur de section dans l'entête JPEG ou non (voir section
    2.10.4 du sujet).
*/
void bitstream_write_bits(struct bitstream *stream, uint32_t value, uint8_t nb_bits, bool is_marker){
  if(is_marker){
    bitstream_flush(stream);
  }
  char *to_write = binary_string(value,nb_bits);
  for(size_t i = 0; i<nb_bits;i++){
    if(stream->length == 8*BUFFER_SIZE){
      bitstream_flush(stream);
    }
    stream->buffer[stream->length/8] += (to_write[i] - '0') * pow(2, ((8 - (stream->length+1)%8)*((stream->length + 1)%8 !=0)));
    stream->length += 1;
    if(stream->buffer[(stream->length-1)/8]==255 && !is_marker){
        bitstream_write_bits(stream, 0, 8, false);
    }
  }
  free(to_write);
}



/*
    Détruit le bitstream passé en paramètre, en libérant la mémoire qui lui est
    associée.
*/
void bitstream_destroy(struct bitstream *stream){
    bitstream_flush(stream);
    fclose(stream->outfile);
    free(stream);
}


// int main(){
//   struct bitstream *bs = bitstream_create("test");
//   bitstream_write_bits(bs, 0x43, 16, false);
//   bitstream_write_bits(bs, 0, 4, false);
//   bitstream_write_bits(bs, 0, 4, false);
//   bitstream_write_bits(bs, 5, 8, false);
//   bitstream_flush(bs);
// }
