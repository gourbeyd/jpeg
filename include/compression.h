//1er arg: nb de pixel a 0 avant
// 2e arg: magnitude du pixel
extern int rle_code(int, int);

extern int magnitude(int);

extern int index(int, int);

extern void compression_bloc(struct bitstream*, enum color_component, int16_t *, int*);

extern void compression(struct bitstream*, struct jpeg *, int16_t ****, int16_t ****, int16_t ****);
