#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H
#include <stdint.h>
#define IMAGE_WIDTH 162U
#define IMAGE_HEIGHT 204U
#define IMAGE_TOTAL_FRAMES 12U
#define ANIM_IDLE 0U
#define ANIM_WALK_LEFT 1U
#define ANIM_WALK_RIGHT 2U
#define ANIM_HOT 3U
#define ANIM_HAPPY 4U
extern const uint16_t image_data[IMAGE_TOTAL_FRAMES][IMAGE_WIDTH * IMAGE_HEIGHT];
extern const uint16_t anim_frame_ms[IMAGE_TOTAL_FRAMES];
extern const uint8_t anim_start[5];
extern const uint8_t anim_count[5];
#endif
