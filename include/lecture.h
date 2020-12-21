#include "jpeg_writer.h"

/* Return struct Image with format, height, width, rgb */
extern uint8_t *read_rgb(struct jpeg *);



/* Fill Y, Cb, Cr of Image */
extern uint8_t **rgb_to_ycbcr(struct jpeg *, enum color_component, uint8_t *);
