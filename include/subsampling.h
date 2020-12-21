#include <stdint.h>
#include "jpeg_writer.h"

extern uint8_t ***decoupe_blocs(struct jpeg *, uint8_t **);

extern uint8_t ****decoupe_MCU(struct jpeg *, uint8_t **);

extern uint8_t ****subsampling(struct jpeg *, enum color_component, uint8_t **);
