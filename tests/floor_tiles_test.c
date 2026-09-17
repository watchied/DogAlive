#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "src/core/floor_tiles.h"
int main(void)
{
    uint8_t a[FLOOR_ROWS][FLOOR_COLUMNS], b[FLOOR_ROWS][FLOOR_COLUMNS];
    uint32_t seed = 12345, same = 12345;
    Floor_Generate(a, &seed); Floor_Generate(b, &same);
    assert(memcmp(a, b, sizeof(a)) == 0);
    Floor_Generate(b, &seed);
    assert(memcmp(a, b, sizeof(a)) != 0);
    int counts[MAP_OBJECT_FLOOR_COUNT] = {0};
    for (int i = 0; i < 1000; ++i) {
        Floor_Generate(a, &seed);
        for (int y = 0; y < FLOOR_ROWS; ++y)
            for (int x = 0; x < FLOOR_COLUMNS; ++x) {
                assert(a[y][x] < MAP_OBJECT_FLOOR_COUNT);
                ++counts[a[y][x]];
            }
    }
    float total = 1000.0f * FLOOR_ROWS * FLOOR_COLUMNS;
    assert(counts[0] / total > 0.59f && counts[0] / total < 0.61f);
    for (int i = 1; i < MAP_OBJECT_FLOOR_COUNT; ++i)
        assert(fabsf(counts[i] / total - 0.4f / (MAP_OBJECT_FLOOR_COUNT - 1)) < 0.005f);
    printf("Floor distribution passed: plain %.2f%%\n", counts[0] / total * 100);
    return 0;
}
