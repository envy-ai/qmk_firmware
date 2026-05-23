// Custom Solid Splash variant for Keychron K5 Max RGB.
// Base color: #0000c0
// Splash color: #0077ff
//
// This keeps the same timing behavior as QMK/Keychron SOLID_SPLASH:
// effect = tick - dist
// pulse = 255 - clamp(effect, 0..255)
//
// Difference from stock Solid Splash:
// stock runner fades to black because it starts hsv.v at 0;
// this effect blends from a non-black base RGB color to a splash RGB color.

#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED

RGB_MATRIX_EFFECT(BLUE_BASE_SOLID_SPLASH)

#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#        define BLUE_SPLASH_BASE_R 0x00
#        define BLUE_SPLASH_BASE_G 0x00
#        define BLUE_SPLASH_BASE_B 0xC0

#        define BLUE_SPLASH_PEAK_R 0x00
#        define BLUE_SPLASH_PEAK_G 0x77
#        define BLUE_SPLASH_PEAK_B 0xFF

static uint8_t blue_splash_blend_u8(uint8_t from, uint8_t to, uint8_t amount) {
    return (uint8_t)(((uint16_t)from * (255 - amount) + (uint16_t)to * amount + 127) / 255);
}

static uint8_t blue_splash_scale_u8(uint8_t channel, uint8_t scale) {
    return (uint8_t)(((uint16_t)channel * scale + 127) / 255);
}

static uint8_t blue_base_solid_splash_amount(uint8_t dist, uint16_t tick) {
    uint16_t effect = tick - dist;

    if (effect > 255) {
        effect = 255;
    }

    return 255 - effect;
}

static bool BLUE_BASE_SOLID_SPLASH(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t start = qsub8(g_last_hit_tracker.count, 1);
    uint8_t count = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t amount = 0;

        for (uint8_t j = start; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));

            amount = qadd8(amount, blue_base_solid_splash_amount(dist, tick));
        }

        uint8_t r = blue_splash_blend_u8(BLUE_SPLASH_BASE_R, BLUE_SPLASH_PEAK_R, amount);
        uint8_t g = blue_splash_blend_u8(BLUE_SPLASH_BASE_G, BLUE_SPLASH_PEAK_G, amount);
        uint8_t b = blue_splash_blend_u8(BLUE_SPLASH_BASE_B, BLUE_SPLASH_PEAK_B, amount);

        uint8_t brightness = rgb_matrix_config.hsv.v;

        r = blue_splash_scale_u8(r, brightness);
        g = blue_splash_scale_u8(g, brightness);
        b = blue_splash_scale_u8(b, brightness);

        rgb_matrix_region_set_color(params->region, i, r, g, b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // RGB_MATRIX_KEYREACTIVE_ENABLED
