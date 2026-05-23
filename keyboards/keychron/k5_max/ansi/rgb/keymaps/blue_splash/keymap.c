/* Copyright 2024 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"

enum layers {
    MAC_BASE,
    MAC_FN,
    WIN_BASE,
    WIN_FN,
};

#define BLUE_SPLASH_NUM_LOCK_LED 37
#define BLUE_SPLASH_CAPS_LOCK_LED 62

#define BLUE_SPLASH_LOCK_R 0x00
#define BLUE_SPLASH_LOCK_G 0xFF
#define BLUE_SPLASH_LOCK_B 0xFF
#define BLUE_SPLASH_LOCK_FADE_MS 200
#define BLUE_SPLASH_LOCK_GLOW_RADIUS 32

#define BLUE_SPLASH_LCTRL_LED 95
#define BLUE_SPLASH_RCTRL_LED 102
#define BLUE_SPLASH_LWIN_LED 96
#define BLUE_SPLASH_RWIN_LED 100
#define BLUE_SPLASH_LALT_LED 97
#define BLUE_SPLASH_RALT_LED 99
#define BLUE_SPLASH_LSHIFT_LED 78
#define BLUE_SPLASH_RSHIFT_LED 89
#define BLUE_SPLASH_FN_LED 101

#define BLUE_SPLASH_MOD_SOURCE_COUNT 9

#define BLUE_SPLASH_CTRL_R 0xFF
#define BLUE_SPLASH_CTRL_G 0x44
#define BLUE_SPLASH_CTRL_B 0x00

#define BLUE_SPLASH_WIN_R 0xFF
#define BLUE_SPLASH_WIN_G 0xFF
#define BLUE_SPLASH_WIN_B 0x00

#define BLUE_SPLASH_ALT_R 0x88
#define BLUE_SPLASH_ALT_G 0xFF
#define BLUE_SPLASH_ALT_B 0x00

#define BLUE_SPLASH_SHIFT_R 0x00
#define BLUE_SPLASH_SHIFT_G 0xFF
#define BLUE_SPLASH_SHIFT_B 0x00

#define BLUE_SPLASH_FN_R 0x88
#define BLUE_SPLASH_FN_G 0x00
#define BLUE_SPLASH_FN_B 0xFF

enum blue_splash_mod_sources {
    BLUE_SPLASH_MOD_LCTRL,
    BLUE_SPLASH_MOD_RCTRL,
    BLUE_SPLASH_MOD_LWIN,
    BLUE_SPLASH_MOD_RWIN,
    BLUE_SPLASH_MOD_LALT,
    BLUE_SPLASH_MOD_RALT,
    BLUE_SPLASH_MOD_LSHIFT,
    BLUE_SPLASH_MOD_RSHIFT,
    BLUE_SPLASH_MOD_FN,
};

bool    blue_splash_mod_active[BLUE_SPLASH_MOD_SOURCE_COUNT]    = {false};
bool    blue_splash_mod_releasing[BLUE_SPLASH_MOD_SOURCE_COUNT] = {false};
uint16_t blue_splash_mod_timer[BLUE_SPLASH_MOD_SOURCE_COUNT]    = {0};
uint8_t blue_splash_mod_led[BLUE_SPLASH_MOD_SOURCE_COUNT]       = {BLUE_SPLASH_LCTRL_LED, BLUE_SPLASH_RCTRL_LED, BLUE_SPLASH_LWIN_LED, BLUE_SPLASH_RWIN_LED, BLUE_SPLASH_LALT_LED, BLUE_SPLASH_RALT_LED, BLUE_SPLASH_LSHIFT_LED, BLUE_SPLASH_RSHIFT_LED, BLUE_SPLASH_FN_LED};
uint8_t blue_splash_mod_r[BLUE_SPLASH_MOD_SOURCE_COUNT]         = {BLUE_SPLASH_CTRL_R, BLUE_SPLASH_CTRL_R, BLUE_SPLASH_WIN_R, BLUE_SPLASH_WIN_R, BLUE_SPLASH_ALT_R, BLUE_SPLASH_ALT_R, BLUE_SPLASH_SHIFT_R, BLUE_SPLASH_SHIFT_R, BLUE_SPLASH_FN_R};
uint8_t blue_splash_mod_g[BLUE_SPLASH_MOD_SOURCE_COUNT]         = {BLUE_SPLASH_CTRL_G, BLUE_SPLASH_CTRL_G, BLUE_SPLASH_WIN_G, BLUE_SPLASH_WIN_G, BLUE_SPLASH_ALT_G, BLUE_SPLASH_ALT_G, BLUE_SPLASH_SHIFT_G, BLUE_SPLASH_SHIFT_G, BLUE_SPLASH_FN_G};
uint8_t blue_splash_mod_b[BLUE_SPLASH_MOD_SOURCE_COUNT]         = {BLUE_SPLASH_CTRL_B, BLUE_SPLASH_CTRL_B, BLUE_SPLASH_WIN_B, BLUE_SPLASH_WIN_B, BLUE_SPLASH_ALT_B, BLUE_SPLASH_ALT_B, BLUE_SPLASH_SHIFT_B, BLUE_SPLASH_SHIFT_B, BLUE_SPLASH_FN_B};

uint8_t blue_splash_lock_replay_amount[RGB_MATRIX_LED_COUNT] = {0};
uint8_t blue_splash_lock_replay_r[RGB_MATRIX_LED_COUNT]      = {0};
uint8_t blue_splash_lock_replay_g[RGB_MATRIX_LED_COUNT]      = {0};
uint8_t blue_splash_lock_replay_b[RGB_MATRIX_LED_COUNT]      = {0};
uint8_t blue_splash_num_lock_amount                          = 0;
uint8_t blue_splash_caps_lock_amount                         = 0;

static bool     blue_splash_num_lock_on     = false;
static bool     blue_splash_caps_lock_on    = false;
static uint16_t blue_splash_num_lock_timer  = 0;
static uint16_t blue_splash_caps_lock_timer = 0;
static uint8_t  blue_splash_num_lock_start_amount  = 0;
static uint8_t  blue_splash_caps_lock_start_amount = 0;

static uint8_t blue_splash_record_led(keyrecord_t *record) {
    return g_led_config.matrix_co[record->event.key.row][record->event.key.col];
}

static void blue_splash_set_mod_state(uint8_t index, keyrecord_t *record) {
    uint8_t led = blue_splash_record_led(record);
    bool    pressed = record->event.pressed;

    blue_splash_mod_active[index]    = pressed;
    blue_splash_mod_releasing[index] = !pressed;
    blue_splash_mod_timer[index]     = timer_read();

    if (led != NO_LED) {
        blue_splash_mod_led[index] = led;
    }
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_108_ansi(
        KC_ESC,             KC_BRID,  KC_BRIU,  KC_MCTRL, KC_LNPAD, RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU, KC_SNAP,  KC_SIRI,  RGB_MOD, KC_F13,  KC_F14,  KC_F15,  KC_F16,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC, KC_INS,   KC_HOME,  KC_PGUP, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS, KC_DEL,   KC_END,   KC_PGDN, KC_P7,   KC_P8,   KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,                               KC_P4,   KC_P5,   KC_P6,   KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,           KC_UP,             KC_P1,   KC_P2,   KC_P3,
        KC_LCTL,  KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, KC_ROPTN,MO(MAC_FN),KC_RCTL, KC_LEFT,  KC_DOWN,  KC_RGHT, KC_P0,            KC_PDOT, KC_PENT),

    [MAC_FN] = LAYOUT_108_ansi(
        _______,            KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,  _______,  _______,  RGB_TOG, _______, _______, _______, _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______,  _______,  _______, _______, _______, _______, _______,
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______,  _______,  _______, _______, _______, _______,
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,  _______,  _______,            _______,                              _______, _______, _______, _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,            _______,           _______,           _______, _______, _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______, _______,  _______,  _______, _______,          _______, _______),

    [WIN_BASE] = LAYOUT_108_ansi(
        KC_ESC,             KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,  KC_PSCR,  KC_CTANA, RGB_MOD, _______, _______, _______, _______,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC, KC_INS,   KC_HOME,  KC_PGUP, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS, KC_DEL,   KC_END,   KC_PGDN, KC_P7,   KC_P8,   KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,                               KC_P4,   KC_P5,   KC_P6,   KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,           KC_UP,             KC_P1,   KC_P2,   KC_P3,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN, MO(WIN_FN),KC_RCTL, KC_LEFT,  KC_DOWN,  KC_RGHT, KC_P0,            KC_PDOT, KC_PENT),

    [WIN_FN] = LAYOUT_108_ansi(
        _______,            KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU, _______,  _______,  RGB_TOG, _______, _______, _______, _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______,  _______,  _______, _______, _______, _______, _______,
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______,  _______,  _______, _______, _______, _______,
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,  _______,  _______,            _______,                              _______, _______, _______, _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,            _______,           _______,           _______, _______, _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______, _______,  _______,  _______, _______,          _______, _______),
};

// clang-format on
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case KC_LCTL:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_LCTRL, record);
            break;
        case KC_RCTL:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_RCTRL, record);
            break;
        case KC_LGUI:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_LWIN, record);
            break;
        case KC_RGUI:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_RWIN, record);
            break;
        case KC_LALT:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_LALT, record);
            break;
        case KC_RALT:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_RALT, record);
            break;
        case KC_LSFT:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_LSHIFT, record);
            break;
        case KC_RSFT:
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_RSHIFT, record);
            break;
        case MO(MAC_FN):
        case MO(WIN_FN):
            blue_splash_set_mod_state(BLUE_SPLASH_MOD_FN, record);
            break;
    }

    return true;
}

void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_BLUE_BASE_SOLID_SPLASH);
}

static uint8_t blue_splash_lock_fade_amount(bool active, bool *was_active, uint16_t *timer, uint8_t *start_amount, uint8_t current_amount) {
    if (active != *was_active) {
        *start_amount = current_amount;
        *timer = timer_read();
        *was_active = active;
    }

    uint16_t elapsed = timer_elapsed(*timer);

    if (elapsed >= BLUE_SPLASH_LOCK_FADE_MS) {
        return active ? 255 : 0;
    }

    if (active) {
        return *start_amount + (((uint16_t)(255 - *start_amount) * elapsed) / BLUE_SPLASH_LOCK_FADE_MS);
    }

    return *start_amount - (((uint16_t)*start_amount * elapsed) / BLUE_SPLASH_LOCK_FADE_MS);
}

static void blue_splash_update_lock_amounts(void) {
    led_t led_state = host_keyboard_led_state();

    blue_splash_num_lock_amount  = blue_splash_lock_fade_amount(led_state.num_lock, &blue_splash_num_lock_on, &blue_splash_num_lock_timer, &blue_splash_num_lock_start_amount, blue_splash_num_lock_amount);
    blue_splash_caps_lock_amount = blue_splash_lock_fade_amount(led_state.caps_lock, &blue_splash_caps_lock_on, &blue_splash_caps_lock_timer, &blue_splash_caps_lock_start_amount, blue_splash_caps_lock_amount);
}

void housekeeping_task_user(void) {
    blue_splash_update_lock_amounts();
}

static void blue_splash_replay_lock_glow(void) {
    led_t led_state = host_keyboard_led_state();

    if (!led_state.num_lock && !led_state.caps_lock) {
        return;
    }

    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        if (blue_splash_lock_replay_amount[i] > 0) {
            rgb_matrix_set_color(i, blue_splash_lock_replay_r[i], blue_splash_lock_replay_g[i], blue_splash_lock_replay_b[i]);
        }
    }
}

bool rgb_matrix_indicators_user(void) {
    blue_splash_update_lock_amounts();
    blue_splash_replay_lock_glow();

    return true;
}
