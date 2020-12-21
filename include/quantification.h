#include "jpeg_writer.h"
extern void quantify_luminance(int*, int);
extern void quantify_chrominance(int*, int);

extern void quantification(struct jpeg *jpg, enum color_component, int16_t ****mcu_zz, int quality);
