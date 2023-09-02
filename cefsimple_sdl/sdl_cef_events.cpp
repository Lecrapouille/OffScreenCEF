//
// Original version comes from:
// https://github.com/jamethy/sdl2-cef
// Created by James Burns
//

#include "sdl_cef_events.hpp"

#define _SDLK_KP0          1073741912
#define _SDLK_KP1          1073741913
#define _SDLK_KP2          1073741914
#define _SDLK_KP3          1073741915
#define _SDLK_KP4          1073741916
#define _SDLK_KP5          1073741917
#define _SDLK_KP6          1073741918
#define _SDLK_KP7          1073741919
#define _SDLK_KP8          1073741920
#define _SDLK_KP9          1073741921
#define _SDLK_SCROLL_LOCK  1073741895
#define _SDLK_INSERT       1073741897
#define _SDLK_HOME         1073741898
#define _SDLK_PAGEUP       1073741899
#define _SDLK_END          1073741901
#define _SDLK_PAGEDOWN     1073741902
#define _SDLK_NUM          1073741907
#define _SDLK_NUM_DIVIDE   1073741908
#define _SDLK_NUM_MULTIPLY 1073741909
#define _SDLK_NUM_SUBTRACT 1073741910
#define _SDLK_NUM_ADD      1073741911
#define _SDLK_NUM_DECIMAL  1073741923
#define _SDLK_SELECT       1073741925

const int DEFAULT_KEY_CODE = 0;
const int DEFAULT_CHAR_CODE = -1;

typedef struct Modifiers
{
    bool shift;
    bool ctrl;
    bool alt;
    bool num_lock;
    bool caps_lock;
    bool uppercase;

    explicit Modifiers(SDL_Event &event)
    {
        auto mod = event.key.keysym.mod;
        shift = (mod & KMOD_LSHIFT) || (mod & KMOD_RSHIFT);
        ctrl = (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL);
        alt = (mod & KMOD_LALT) || (mod & KMOD_RALT);
        num_lock = !(mod & KMOD_NUM);
        caps_lock = static_cast<bool>(mod & KMOD_CAPS);
        uppercase = caps_lock == !shift;

        /** Set the modifiers **/
        if (event.key.state == SDL_PRESSED)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                shift = true;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                ctrl = true;
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                alt = true;
                break;
            default:
                break;
            }
        }
    }

    uint32_t getCode()
    {
        uint32_t modifiersCode = 0;
        if (shift)
        {
            modifiersCode += EVENTFLAG_SHIFT_DOWN;
        }
        if (ctrl)
        {
            modifiersCode += EVENTFLAG_CONTROL_DOWN;
        }
        if (alt)
        {
            modifiersCode += EVENTFLAG_ALT_DOWN;
        }
        if (num_lock)
        {
            modifiersCode += EVENTFLAG_NUM_LOCK_ON;
        }
        if (caps_lock)
        {
            modifiersCode += EVENTFLAG_CAPS_LOCK_ON;
        }
        return modifiersCode;
    }
} Modifiers;

typedef struct KeyCodes
{
    int key_code;
    int char_code;

    KeyCodes(int keyCode, int charCode)
        : key_code(keyCode), char_code(charCode)
    {
    }

    char16_t getChar16()
    {
        return static_cast<char16_t>(char_code);
    }
} KeyCodes;

static KeyCodes getAlphabetCodes(SDL_Keycode code, Modifiers &modifiers)
{
    int diff = code - SDLK_a;
    return {
        65 + diff,
        (modifiers.uppercase ? 'A' : 'a') + diff
    };
}

static KeyCodes getNumberRowCodes(SDL_Keycode code, Modifiers &modifiers)
{
    int diff = code - SDLK_0;
    int char_code = '0' + diff;
    int key_code = 48 + diff;

    if (modifiers.shift)
    {
        switch (code)
        {
        case SDLK_1:
            char_code = '!';
            break;
        case SDLK_2:
            char_code = '@';
            break;
        case SDLK_3:
            char_code = '#';
            break;
        case SDLK_4:
            char_code = '$';
            break;
        case SDLK_5:
            char_code = '%';
            break;
        case SDLK_6:
            char_code = '^';
            break;
        case SDLK_7:
            char_code = '&';
            break;
        case SDLK_8:
            char_code = '*';
            break;
        case SDLK_9:
            char_code = '(';
            break;
        case SDLK_0:
            char_code = ')';
            break;
        default:
            break;
        }
    }
    return {key_code, char_code};
}

static KeyCodes getFunctionKeyCodes(SDL_Keycode code)
{
    return {
        112 + (code - 1073741882),
        DEFAULT_CHAR_CODE
    };
}

static KeyCodes getSpecialKeyCodes(SDL_Keycode code, Modifiers &modifiers)
{
    int key_code = DEFAULT_KEY_CODE;
    int char_code = DEFAULT_CHAR_CODE;

    switch (code)
    {
    case SDLK_SPACE:
        key_code = 32;
        char_code = ' ';
        break;
    case SDLK_BACKSPACE:
        key_code = 8;
        break;
    case SDLK_TAB:
        key_code = 9;
        break;
    case SDLK_RETURN:
        key_code = 13;
        char_code = 13;
        break;
    case SDLK_PAUSE:
        key_code = 19;
        break;
    case _SDLK_SCROLL_LOCK:
        key_code = 145;
        break;
    case SDLK_ESCAPE:
        key_code = 27;
        break;
    case SDLK_LEFT:
        key_code = 37;
        break;
    case SDLK_UP:
        key_code = 38;
        break;
    case SDLK_RIGHT:
        key_code = 39;
        break;
    case SDLK_DOWN:
        key_code = 40;
        break;
    case _SDLK_HOME:
        key_code = 36;
        break;
    case _SDLK_END:
        key_code = 35;
        break;
    case _SDLK_PAGEUP:
        key_code = 33;
        break;
    case _SDLK_PAGEDOWN:
        key_code = 34;
        break;
    case _SDLK_INSERT:
        key_code = 45;
        break;
    case SDLK_DELETE:
        key_code = 46;
        char_code = 127;
        break;
    case _SDLK_NUM_DIVIDE:
        key_code = 111;
        char_code = 47;
        break;
    case _SDLK_NUM_MULTIPLY:
        key_code = 106;
        char_code = 42;
        break;
    case _SDLK_NUM_SUBTRACT:
        key_code = 109;
        char_code = 45;
        break;
    case _SDLK_NUM_ADD:
        key_code = 107;
        char_code = 43;
        break;
    case _SDLK_NUM_DECIMAL:
        if (modifiers.num_lock)
        {
            key_code = 110; // keyboard layout dependent!
            char_code = 46;
        } else
        {
            key_code = 46;
        }
        break;
    case _SDLK_KP0:
        key_code = 45;
        break;
    case _SDLK_KP1:
        key_code = 35;
        break;
    case _SDLK_KP2:
        key_code = 40;
        break;
    case _SDLK_KP3:
        key_code = 34;
        break;
    case _SDLK_KP4:
        key_code = 37;
        break;
    case _SDLK_KP5:
        key_code = 12;
        break;
    case _SDLK_KP6:
        key_code = 39;
        break;
    case _SDLK_KP7:
        key_code = 36;
        break;
    case _SDLK_KP8:
        key_code = 38;
        break;
    case _SDLK_KP9:
        key_code = 33;
        break;
    case SDLK_CAPSLOCK:
        key_code = 20;
        break;
    case _SDLK_NUM:
        key_code = 144;
        break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
        key_code = 17;
        break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        key_code = 16;
        break;
    case SDLK_LALT:
    case SDLK_RALT:
        key_code = 18;
        break;
    case SDLK_LGUI:
        key_code = 91;
        break;
    case SDLK_RGUI:
        key_code = 92;
        break;
    case _SDLK_SELECT:
        key_code = 93;
        break;
    case SDLK_SEMICOLON:
        key_code = 186;
        char_code = modifiers.shift ? ':' : ';';
        break;
    case SDLK_QUOTE:
        key_code = 222;
        char_code = modifiers.shift ? '"' : '\'';
        break;
    case SDLK_EQUALS:
        key_code = 187;
        char_code = modifiers.shift ? '+' : '=';
        break;
    case SDLK_COMMA:
        key_code = 188;
        char_code = modifiers.shift ? '<' : ',';
        break;
    case SDLK_MINUS:
        key_code = 189;
        char_code = modifiers.shift ? '_' : '-';
        break;
    case SDLK_PERIOD:
        key_code = 190;
        char_code = modifiers.shift ? '>' : '.';
        break;
    case SDLK_SLASH:
        key_code = 191;
        char_code = modifiers.shift ? '?' : '/';
        break;
    case SDLK_BACKQUOTE:
        key_code = 192;
        char_code = modifiers.shift ? '~' : '`';
        break;
    case SDLK_LEFTBRACKET:
        key_code = 219;
        char_code = modifiers.shift ? '{' : '[';
        break;
    case SDLK_BACKSLASH:
        key_code = 220;
        char_code = modifiers.shift ? '|' : '\\';
        break;
    case SDLK_RIGHTBRACKET:
        key_code = 221;
        char_code = modifiers.shift ? '}' : ']';
        break;
    default:
        break;
    }

    return {
        key_code,
        char_code
    };
}

void handleKeyEvent(SDL_Event& event, CefRefPtr<CefBrowser> browser)
{
    /** Modifiers **/
    Modifiers modifiers(event);

    /** Output codes **/
    KeyCodes keyCodes(0, -1);

    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        SDL_Keycode code = event.key.keysym.sym;
        if (code >= SDLK_a && code <= SDLK_z)
        {
            keyCodes = getAlphabetCodes(code, modifiers);
        }
        else if (code >= SDLK_0 && code <= SDLK_9)
        {
            keyCodes = getNumberRowCodes(code, modifiers);

            /** Function Keys **/
        }
        else if (code >= SDLK_F1 && code <= SDLK_F12)
        {
            keyCodes = getFunctionKeyCodes(code);
        }
        else
        {
            /** Special Keys **/
            keyCodes = getSpecialKeyCodes(code, modifiers);
        }
    }

    /** Still not mapped? **/
    if (keyCodes.key_code == 0)
    {
        keyCodes.key_code = event.key.keysym.sym;
    }

    /** Fire key events to CEF **/
    if (event.key.state == SDL_PRESSED || event.type == SDL_TEXTINPUT)
    {
        // onkeydown
        CefKeyEvent key_event_key_down;
        key_event_key_down.type = KEYEVENT_KEYDOWN;
        key_event_key_down.modifiers = modifiers.getCode();
        key_event_key_down.windows_key_code = keyCodes.key_code;
        key_event_key_down.native_key_code = keyCodes.key_code;
        key_event_key_down.character = keyCodes.getChar16();
        key_event_key_down.unmodified_character = keyCodes.getChar16();
        browser->GetHost()->SendKeyEvent(key_event_key_down);
        // Fire a second key event for characters only
        if (keyCodes.char_code >= 0)
        {
            // onkeypress
            CefKeyEvent key_event_char;
            key_event_char.type = KEYEVENT_CHAR;
            key_event_char.modifiers = modifiers.getCode();
            key_event_char.windows_key_code = keyCodes.key_code;
            key_event_char.native_key_code = keyCodes.key_code;
            key_event_char.character = keyCodes.getChar16();
            key_event_char.unmodified_character = keyCodes.getChar16();
            browser->GetHost()->SendKeyEvent(key_event_char);
        }
    }
    else if (event.key.state == SDL_RELEASED)
    {
        // onkeyup
        CefKeyEvent key_event_key_up;
        key_event_key_up.type = KEYEVENT_KEYUP;
        key_event_key_up.modifiers = modifiers.getCode();
        key_event_key_up.windows_key_code = keyCodes.key_code;
        key_event_key_up.native_key_code = keyCodes.key_code;
        key_event_key_up.character = keyCodes.getChar16();
        key_event_key_up.unmodified_character = keyCodes.getChar16();
        browser->GetHost()->SendKeyEvent(key_event_key_up);
    }
}
