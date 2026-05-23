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

#        define BLUE_SPLASH_NUM_LOCK_LED 37
#        define BLUE_SPLASH_CAPS_LOCK_LED 62

#        define BLUE_SPLASH_LOCK_R 0x00
#        define BLUE_SPLASH_LOCK_G 0xFF
#        define BLUE_SPLASH_LOCK_B 0xFF
#        define BLUE_SPLASH_LOCK_FADE_MS 200
#        define BLUE_SPLASH_LOCK_GLOW_RADIUS 32

#        define BLUE_SPLASH_MOD_SOURCE_COUNT 9
#        define BLUE_SPLASH_MOD_RELEASE_DONE 512

extern bool     blue_splash_mod_active[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern bool     blue_splash_mod_releasing[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern uint16_t blue_splash_mod_timer[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern uint8_t  blue_splash_mod_led[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern uint8_t  blue_splash_mod_r[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern uint8_t  blue_splash_mod_g[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern uint8_t  blue_splash_mod_b[BLUE_SPLASH_MOD_SOURCE_COUNT];
extern uint8_t  blue_splash_lock_replay_amount[RGB_MATRIX_LED_COUNT];
extern uint8_t  blue_splash_lock_replay_r[RGB_MATRIX_LED_COUNT];
extern uint8_t  blue_splash_lock_replay_g[RGB_MATRIX_LED_COUNT];
extern uint8_t  blue_splash_lock_replay_b[RGB_MATRIX_LED_COUNT];
extern uint8_t  blue_splash_num_lock_amount;
extern uint8_t  blue_splash_caps_lock_amount;

static uint8_t blue_splash_blend_u8(uint8_t from, uint8_t to, uint8_t amount) {
    return (uint8_t)(((uint16_t)from * (255 - amount) + (uint16_t)to * amount + 127) / 255);
}

static uint8_t blue_splash_scale_u8(uint8_t channel, uint8_t scale) {
    return (uint8_t)(((uint16_t)channel * scale + 127) / 255);
}

static uint8_t blue_splash_lock_scale(uint8_t value, uint8_t amount) {
    if (amount == 255) {
        return value;
    }

    return ((uint16_t)value * amount) / 255;
}

static uint8_t blue_base_solid_splash_amount(uint8_t dist, uint16_t tick) {
    uint16_t effect = tick - dist;

    if (effect > 255) {
        effect = 255;
    }

    return 255 - effect;
}

static uint8_t blue_splash_latching_wave_amount(uint8_t dist, uint16_t tick) {
    if (tick <= dist) {
        return 0;
    }

    uint16_t effect = tick - dist;

    if (effect > 255) {
        effect = 255;
    }

    return effect;
}

static bool blue_splash_is_modifier_source_led(uint8_t led_index) {
    for (uint8_t j = 0; j < BLUE_SPLASH_MOD_SOURCE_COUNT; j++) {
        if (led_index == blue_splash_mod_led[j]) {
            return true;
        }
    }

    return false;
}

static bool blue_splash_is_lock_source_led(uint8_t led_index) {
    return led_index == BLUE_SPLASH_NUM_LOCK_LED || led_index == BLUE_SPLASH_CAPS_LOCK_LED;
}

static bool blue_splash_is_suppressed_source_led(uint8_t led_index) {
    return blue_splash_is_modifier_source_led(led_index) || blue_splash_is_lock_source_led(led_index);
}

static uint8_t blue_splash_lock_falloff(uint8_t dist) {
    uint8_t falloff = 255 - ((uint16_t)dist * 255) / BLUE_SPLASH_LOCK_GLOW_RADIUS;
    return blue_splash_lock_scale(falloff, falloff);
}

static uint8_t blue_splash_lock_glow_amount(uint8_t led_index, uint8_t lock_led, uint8_t fade) {
    int16_t dx   = g_led_config.point[led_index].x - g_led_config.point[lock_led].x;
    int16_t dy   = g_led_config.point[led_index].y - g_led_config.point[lock_led].y;
    uint8_t dist = sqrt16(dx * dx + dy * dy);

    if (dist > BLUE_SPLASH_LOCK_GLOW_RADIUS) {
        return 0;
    }

    uint8_t falloff = blue_splash_lock_falloff(dist);
    return blue_splash_lock_scale(fade, falloff);
}

static void blue_splash_apply_lock_blend(uint8_t led_index, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t amount = 0;

    if (blue_splash_num_lock_amount > 0) {
        amount = blue_splash_lock_glow_amount(led_index, BLUE_SPLASH_NUM_LOCK_LED, blue_splash_num_lock_amount);
    }

    if (blue_splash_caps_lock_amount > 0) {
        uint8_t caps_glow = blue_splash_lock_glow_amount(led_index, BLUE_SPLASH_CAPS_LOCK_LED, blue_splash_caps_lock_amount);
        if (caps_glow > amount) {
            amount = caps_glow;
        }
    }

    blue_splash_lock_replay_amount[led_index] = amount;

    if (amount == 0) {
        return;
    }

    *r = blue_splash_blend_u8(*r, BLUE_SPLASH_LOCK_R, amount);
    *g = blue_splash_blend_u8(*g, BLUE_SPLASH_LOCK_G, amount);
    *b = blue_splash_blend_u8(*b, BLUE_SPLASH_LOCK_B, amount);

    blue_splash_lock_replay_r[led_index] = *r;
    blue_splash_lock_replay_g[led_index] = *g;
    blue_splash_lock_replay_b[led_index] = *b;
}

static void blue_splash_apply_modifier_base(uint8_t led_index, uint8_t *r, uint8_t *g, uint8_t *b) {
    for (uint8_t j = 0; j < BLUE_SPLASH_MOD_SOURCE_COUNT; j++) {
        if (!blue_splash_mod_active[j] && !blue_splash_mod_releasing[j]) {
            continue;
        }

        int16_t  dx   = g_led_config.point[led_index].x - g_led_config.point[blue_splash_mod_led[j]].x;
        int16_t  dy   = g_led_config.point[led_index].y - g_led_config.point[blue_splash_mod_led[j]].y;
        uint8_t  dist = sqrt16(dx * dx + dy * dy);
        uint16_t tick = scale16by8(timer_elapsed(blue_splash_mod_timer[j]), qadd8(rgb_matrix_config.speed, 1));
        uint8_t  wave = blue_splash_latching_wave_amount(dist, tick);
        uint8_t  amount;

        if (blue_splash_mod_active[j]) {
            amount = wave;
        } else {
            amount = 255 - wave;

            if (tick > BLUE_SPLASH_MOD_RELEASE_DONE) {
                blue_splash_mod_releasing[j] = false;
            }
        }

        *r = blue_splash_blend_u8(*r, blue_splash_mod_r[j], amount);
        *g = blue_splash_blend_u8(*g, blue_splash_mod_g[j], amount);
        *b = blue_splash_blend_u8(*b, blue_splash_mod_b[j], amount);
    }
}

static bool BLUE_BASE_SOLID_SPLASH(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t start = qsub8(g_last_hit_tracker.count, 1);
    uint8_t count = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t amount = 0;

        for (uint8_t j = start; j < count; j++) {
            if (blue_splash_is_suppressed_source_led(g_last_hit_tracker.index[j])) {
                continue;
            }

            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));

            amount = qadd8(amount, blue_base_solid_splash_amount(dist, tick));
        }

        uint8_t base_r = BLUE_SPLASH_BASE_R;
        uint8_t base_g = BLUE_SPLASH_BASE_G;
        uint8_t base_b = BLUE_SPLASH_BASE_B;

        blue_splash_apply_modifier_base(i, &base_r, &base_g, &base_b);

        uint8_t r = blue_splash_blend_u8(base_r, BLUE_SPLASH_PEAK_R, amount);
        uint8_t g = blue_splash_blend_u8(base_g, BLUE_SPLASH_PEAK_G, amount);
        uint8_t b = blue_splash_blend_u8(base_b, BLUE_SPLASH_PEAK_B, amount);

        uint8_t brightness = rgb_matrix_config.hsv.v;

        r = blue_splash_scale_u8(r, brightness);
        g = blue_splash_scale_u8(g, brightness);
        b = blue_splash_scale_u8(b, brightness);

        blue_splash_apply_lock_blend(i, &r, &g, &b);

        rgb_matrix_region_set_color(params->region, i, r, g, b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // RGB_MATRIX_KEYREACTIVE_ENABLED
