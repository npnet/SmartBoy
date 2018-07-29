//#ifdef __cplusplus
extern "C"
{
//#endif // __cplusplus

#include <ctype.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>

//#include <sys/limits.h>
//#include <cutils/properties.h>
#include <errno.h>
#include "key.h"
#include "pthread.h"
#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#include "sys/unistd.h"
#include "malloc.h"
#include "memory.h"


#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define LONG_PRESS_DEFAULT_DURATION   5000000     //5 seconds
#define LONG_PRESS_PERIOD   1000*100   //100 seconds
#define MONITOR_SLEEP_PERIOD   1000*50   //50 seconds
#define SEC_TO_USEC    1000000 //1s = 1000000us
#define KEY_RECORD_MASK   0xFFFF
#define INPUT_DEVICE_PATH "/dev/input"
#define BUFFER_LENGTH_L  512 //buffer length
#define BUFFER_LENGTH_S  80 //buffer length
#define NAME_ELEMENT(element) [element] = #element
#define FACTORY_RESET 249// add by yuyun0707
#define KEYPAD_DEVICE_TYPE 0x01
#define IR_DEVICE_TYPE     0x02

#ifndef EV_SYN
#define EV_SYN 0
#endif /*ifndef EV_SYN*/
#ifndef SYN_MAX
#define SYN_MAX 3
#define SYN_CNT (SYN_MAX + 1)
#endif /*ifndef SYN_MAX*/
#ifndef SYN_REPORT
#define SYN_REPORT 0
#endif /*ifndef SYN_REPORT*/
#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT 2
#endif /*ifndef SYN_MT_REPORT*/
#ifndef SYN_DROPPED
#define SYN_DROPPED 3
#endif /*ifndef SYN_DROPPED*/

static const char * keypad_device_name[] =
{
    "mtk-kpd",
    "mtk-pmic-keys",
    "gpio-keys"
};

static const char * mtk_IRRX_device_name[] =
{
    "MTK_Remote_Controller",
    "MTK_Remote_Controller_Mouse"
};
#if 1
#define IR_SM_NUM 16
IR_SM_MAP ir_sm_map[IR_SM_NUM]={
        /*
{BTN_IR_POWER,   SM_BODY_IR_POWER,    "[IR_POWER]POWER"},
{BTN_IR_DISPLAY, SM_BODY_IR_SOURCE,   "[IR_DISPLAY]SOURCE"},
{BTN_IR_CLEAR,   SM_BODY_IR_BT,       "[IR_CLEAR]BT"},
{BTN_IR_EJECT,   SM_BODY_IR_REQUEST,  "[IR_EJECT]REQUEST"},
{BTN_IR_ZOOM,    SM_BODY_IR_MICMUTE,  "[IR_ZOOM]MICMUTE"},
{BTN_IR_ENTER,   SM_BODY_IR_PLAYPAUSE,"[IR_ENTER]PLAYPAUSE"},
{BTN_IR_LEFT,    SM_BODY_IR_PRE,      "[IR_LEFT]PRE"},
{BTN_IR_RIGHT,   SM_BODY_IR_NEXT,     "[IR_RIGHT]NEXT"},
{BTN_IR_GOTO,    SM_BODY_IR_SURROUND, "[IR_GOTO]SURROUND"},
{BTN_IR_OSC,     SM_BODY_IR_USB,      "[IR_OSC]USB"},
{BTN_IR_TOPMENU, SM_BODY_IR_LINE,     "[IR_ZOOM]LINE"},
         */
{BTN_IR_RETURN,  0XFFFF,              "[IR_RETURN]MUTE"},
{BTN_IR_UP,      0XFFFF,              "[IR_UP]VOL+"},
{BTN_IR_DOWN,    0XFFFF,              "[IR_DOWN]VOL-"}
};
#endif /*if 1*/
static struct pollfd *gufds;
static char **gdevice_names;
static int gnfds;
static int gwd;
static pthread_mutex_t gmutex;
pthread_cond_t gkeycond;
static int gKeyPressed = False;
static int gKeyLongPressed = True;
static int gKeyValueRecord = 0xFFFF;
static uint32 glongPressDuration = LONG_PRESS_DEFAULT_DURATION;

static uint8 device_type_nfds[16];
#if 0
static const char * const kevents[EV_MAX + 1] = {
 /*
    [0 ... EV_MAX] = NULL,
    NAME_ELEMENT(EV_SYN),           NAME_ELEMENT(EV_KEY),
    NAME_ELEMENT(EV_REL),           NAME_ELEMENT(EV_ABS),
    NAME_ELEMENT(EV_MSC),           NAME_ELEMENT(EV_LED),
    NAME_ELEMENT(EV_SND),           NAME_ELEMENT(EV_REP),
    NAME_ELEMENT(EV_FF),            NAME_ELEMENT(EV_PWR),
    NAME_ELEMENT(EV_FF_STATUS),     NAME_ELEMENT(EV_SW),
    */
};

static const int gmaxval[EV_MAX + 1] = {
        /*
    [0 ... EV_MAX] = -1,
    [EV_SYN] = SYN_MAX,
    [EV_KEY] = KEY_MAX,
    [EV_REL] = REL_MAX,
    [EV_ABS] = ABS_MAX,
    [EV_MSC] = MSC_MAX,
    [EV_SW] = SW_MAX,
    [EV_LED] = LED_MAX,
    [EV_SND] = SND_MAX,
    [EV_REP] = REP_MAX,
    [EV_FF] = FF_MAX,
    [EV_FF_STATUS] = FF_STATUS_MAX,
         */
};

#ifdef sine
#ifdef INPUT_PROP_SEMI_MT
static const char * const kprops[INPUT_PROP_MAX + 1] = {
    [0 ... INPUT_PROP_MAX] = NULL,
    NAME_ELEMENT(INPUT_PROP_POINTER),
    NAME_ELEMENT(INPUT_PROP_DIRECT),
    NAME_ELEMENT(INPUT_PROP_BUTTONPAD),
    NAME_ELEMENT(INPUT_PROP_SEMI_MT),
#ifdef INPUT_PROP_TOPBUTTONPAD
    NAME_ELEMENT(INPUT_PROP_TOPBUTTONPAD),
#endif /*INPUT_PROP_TOPBUTTONPAD*/
#ifdef INPUT_PROP_POINTING_STICK
    NAME_ELEMENT(INPUT_PROP_POINTING_STICK),
#endif /*INPUT_PROP_POINTING_STICK*/
};
#endif /*INPUT_PROP_SEMI_MT*/
#endif

static const char * const kKeys[KEY_MAX + 1] = {
#if 0
    [0 ... KEY_MAX] = NULL,
    NAME_ELEMENT(KEY_RESERVED),     NAME_ELEMENT(KEY_ESC),
    NAME_ELEMENT(KEY_1),            NAME_ELEMENT(KEY_2),
    NAME_ELEMENT(KEY_3),            NAME_ELEMENT(KEY_4),
    NAME_ELEMENT(KEY_5),            NAME_ELEMENT(KEY_6),
    NAME_ELEMENT(KEY_7),            NAME_ELEMENT(KEY_8),
    NAME_ELEMENT(KEY_9),            NAME_ELEMENT(KEY_0),
    NAME_ELEMENT(KEY_MINUS),        NAME_ELEMENT(KEY_EQUAL),
    NAME_ELEMENT(KEY_BACKSPACE),        NAME_ELEMENT(KEY_TAB),
    NAME_ELEMENT(KEY_Q),            NAME_ELEMENT(KEY_W),
    NAME_ELEMENT(KEY_E),            NAME_ELEMENT(KEY_R),
    NAME_ELEMENT(KEY_T),            NAME_ELEMENT(KEY_Y),
    NAME_ELEMENT(KEY_U),            NAME_ELEMENT(KEY_I),
    NAME_ELEMENT(KEY_O),            NAME_ELEMENT(KEY_P),
    NAME_ELEMENT(KEY_LEFTBRACE),        NAME_ELEMENT(KEY_RIGHTBRACE),
    NAME_ELEMENT(KEY_ENTER),        NAME_ELEMENT(KEY_LEFTCTRL),
    NAME_ELEMENT(KEY_A),            NAME_ELEMENT(KEY_S),
    NAME_ELEMENT(KEY_D),            NAME_ELEMENT(KEY_F),
    NAME_ELEMENT(KEY_G),            NAME_ELEMENT(KEY_H),
    NAME_ELEMENT(KEY_J),            NAME_ELEMENT(KEY_K),
    NAME_ELEMENT(KEY_L),            NAME_ELEMENT(KEY_SEMICOLON),
    NAME_ELEMENT(KEY_APOSTROPHE),       NAME_ELEMENT(KEY_GRAVE),
    NAME_ELEMENT(KEY_LEFTSHIFT),        NAME_ELEMENT(KEY_BACKSLASH),
    NAME_ELEMENT(KEY_Z),            NAME_ELEMENT(KEY_X),
    NAME_ELEMENT(KEY_C),            NAME_ELEMENT(KEY_V),
    NAME_ELEMENT(KEY_B),            NAME_ELEMENT(KEY_N),
    NAME_ELEMENT(KEY_M),            NAME_ELEMENT(KEY_COMMA),
    NAME_ELEMENT(KEY_DOT),          NAME_ELEMENT(KEY_SLASH),
    NAME_ELEMENT(KEY_RIGHTSHIFT),       NAME_ELEMENT(KEY_KPASTERISK),
    NAME_ELEMENT(KEY_LEFTALT),      NAME_ELEMENT(KEY_SPACE),
    NAME_ELEMENT(KEY_CAPSLOCK),     NAME_ELEMENT(KEY_F1),
    NAME_ELEMENT(KEY_F2),           NAME_ELEMENT(KEY_F3),
    NAME_ELEMENT(KEY_F4),           NAME_ELEMENT(KEY_F5),
    NAME_ELEMENT(KEY_F6),           NAME_ELEMENT(KEY_F7),
    NAME_ELEMENT(KEY_F8),           NAME_ELEMENT(KEY_F9),
    NAME_ELEMENT(KEY_F10),          NAME_ELEMENT(KEY_NUMLOCK),
    NAME_ELEMENT(KEY_SCROLLLOCK),       NAME_ELEMENT(KEY_KP7),
    NAME_ELEMENT(KEY_KP8),          NAME_ELEMENT(KEY_KP9),
    NAME_ELEMENT(KEY_KPMINUS),      NAME_ELEMENT(KEY_KP4),
    NAME_ELEMENT(KEY_KP5),          NAME_ELEMENT(KEY_KP6),
    NAME_ELEMENT(KEY_KPPLUS),       NAME_ELEMENT(KEY_KP1),
    NAME_ELEMENT(KEY_KP2),          NAME_ELEMENT(KEY_KP3),
    NAME_ELEMENT(KEY_KP0),          NAME_ELEMENT(KEY_KPDOT),
    NAME_ELEMENT(KEY_ZENKAKUHANKAKU),   NAME_ELEMENT(KEY_102ND),
    NAME_ELEMENT(KEY_F11),          NAME_ELEMENT(KEY_F12),
    NAME_ELEMENT(KEY_RO),           NAME_ELEMENT(KEY_KATAKANA),
    NAME_ELEMENT(KEY_HIRAGANA),     NAME_ELEMENT(KEY_HENKAN),
    NAME_ELEMENT(KEY_KATAKANAHIRAGANA), NAME_ELEMENT(KEY_MUHENKAN),
    NAME_ELEMENT(KEY_KPJPCOMMA),        NAME_ELEMENT(KEY_KPENTER),
    NAME_ELEMENT(KEY_RIGHTCTRL),        NAME_ELEMENT(KEY_KPSLASH),
    NAME_ELEMENT(KEY_SYSRQ),        NAME_ELEMENT(KEY_RIGHTALT),
    NAME_ELEMENT(KEY_LINEFEED),     NAME_ELEMENT(KEY_HOME),
    NAME_ELEMENT(KEY_UP),           NAME_ELEMENT(KEY_PAGEUP),
    NAME_ELEMENT(KEY_LEFT),         NAME_ELEMENT(KEY_RIGHT),
    NAME_ELEMENT(KEY_END),          NAME_ELEMENT(KEY_DOWN),
    NAME_ELEMENT(KEY_PAGEDOWN),     NAME_ELEMENT(KEY_INSERT),
    NAME_ELEMENT(KEY_DELETE),       NAME_ELEMENT(KEY_MACRO),
    NAME_ELEMENT(KEY_MUTE),         NAME_ELEMENT(KEY_VOLUMEDOWN),
    NAME_ELEMENT(KEY_VOLUMEUP),     NAME_ELEMENT(KEY_POWER),
    NAME_ELEMENT(KEY_KPEQUAL),      NAME_ELEMENT(KEY_KPPLUSMINUS),
    NAME_ELEMENT(KEY_PAUSE),        NAME_ELEMENT(KEY_KPCOMMA),
    NAME_ELEMENT(KEY_HANGUEL),      NAME_ELEMENT(KEY_HANJA),
    NAME_ELEMENT(KEY_YEN),          NAME_ELEMENT(KEY_LEFTMETA),
    NAME_ELEMENT(KEY_RIGHTMETA),        NAME_ELEMENT(KEY_COMPOSE),
    NAME_ELEMENT(KEY_STOP),         NAME_ELEMENT(KEY_AGAIN),
    NAME_ELEMENT(KEY_PROPS),        NAME_ELEMENT(KEY_UNDO),
    NAME_ELEMENT(KEY_FRONT),        NAME_ELEMENT(KEY_COPY),
    NAME_ELEMENT(KEY_OPEN),         NAME_ELEMENT(KEY_PASTE),
    NAME_ELEMENT(KEY_FIND),         NAME_ELEMENT(KEY_CUT),
    NAME_ELEMENT(KEY_HELP),         NAME_ELEMENT(KEY_MENU),
    NAME_ELEMENT(KEY_CALC),         NAME_ELEMENT(KEY_SETUP),
    NAME_ELEMENT(KEY_SLEEP),        NAME_ELEMENT(KEY_WAKEUP),
    NAME_ELEMENT(KEY_FILE),         NAME_ELEMENT(KEY_SENDFILE),
    NAME_ELEMENT(KEY_DELETEFILE),       NAME_ELEMENT(KEY_XFER),
    NAME_ELEMENT(KEY_PROG1),        NAME_ELEMENT(KEY_PROG2),
    NAME_ELEMENT(KEY_WWW),          NAME_ELEMENT(KEY_MSDOS),
    NAME_ELEMENT(KEY_COFFEE),       NAME_ELEMENT(KEY_DIRECTION),
    NAME_ELEMENT(KEY_CYCLEWINDOWS),     NAME_ELEMENT(KEY_MAIL),
    NAME_ELEMENT(KEY_BOOKMARKS),        NAME_ELEMENT(KEY_COMPUTER),
    NAME_ELEMENT(KEY_BACK),         NAME_ELEMENT(KEY_FORWARD),
    NAME_ELEMENT(KEY_CLOSECD),      NAME_ELEMENT(KEY_EJECTCD),
    NAME_ELEMENT(KEY_EJECTCLOSECD),     NAME_ELEMENT(KEY_NEXTSONG),
    NAME_ELEMENT(KEY_PLAYPAUSE),        NAME_ELEMENT(KEY_PREVIOUSSONG),
    NAME_ELEMENT(KEY_STOPCD),       NAME_ELEMENT(KEY_RECORD),
    NAME_ELEMENT(KEY_REWIND),       NAME_ELEMENT(KEY_PHONE),
    NAME_ELEMENT(KEY_ISO),          NAME_ELEMENT(KEY_CONFIG),
    NAME_ELEMENT(KEY_HOMEPAGE),     NAME_ELEMENT(KEY_REFRESH),
    NAME_ELEMENT(KEY_EXIT),         NAME_ELEMENT(KEY_MOVE),
    NAME_ELEMENT(KEY_EDIT),         NAME_ELEMENT(KEY_SCROLLUP),
    NAME_ELEMENT(KEY_SCROLLDOWN),       NAME_ELEMENT(KEY_KPLEFTPAREN),
    NAME_ELEMENT(KEY_KPRIGHTPAREN),     NAME_ELEMENT(KEY_F13),
    NAME_ELEMENT(KEY_F14),          NAME_ELEMENT(KEY_F15),
    NAME_ELEMENT(KEY_F16),          NAME_ELEMENT(KEY_F17),
    NAME_ELEMENT(KEY_F18),          NAME_ELEMENT(KEY_F19),
    NAME_ELEMENT(KEY_F20),          NAME_ELEMENT(KEY_F21),
    NAME_ELEMENT(KEY_F22),          NAME_ELEMENT(KEY_F23),
    NAME_ELEMENT(KEY_F24),          NAME_ELEMENT(KEY_PLAYCD),
    NAME_ELEMENT(KEY_PAUSECD),      NAME_ELEMENT(KEY_PROG3),
    NAME_ELEMENT(KEY_PROG4),        NAME_ELEMENT(KEY_SUSPEND),
    NAME_ELEMENT(KEY_CLOSE),        NAME_ELEMENT(KEY_PLAY),
    NAME_ELEMENT(KEY_FASTFORWARD),      NAME_ELEMENT(KEY_BASSBOOST),
    NAME_ELEMENT(KEY_PRINT),        NAME_ELEMENT(KEY_HP),
    NAME_ELEMENT(KEY_CAMERA),       NAME_ELEMENT(KEY_SOUND),
    NAME_ELEMENT(KEY_QUESTION),     NAME_ELEMENT(KEY_EMAIL),
    NAME_ELEMENT(KEY_CHAT),         NAME_ELEMENT(KEY_SEARCH),
    NAME_ELEMENT(KEY_CONNECT),      NAME_ELEMENT(KEY_FINANCE),
    NAME_ELEMENT(KEY_SPORT),        NAME_ELEMENT(KEY_SHOP),
    NAME_ELEMENT(KEY_ALTERASE),     NAME_ELEMENT(KEY_CANCEL),
    NAME_ELEMENT(KEY_BRIGHTNESSDOWN),   NAME_ELEMENT(KEY_BRIGHTNESSUP),
    NAME_ELEMENT(KEY_MEDIA),        NAME_ELEMENT(KEY_UNKNOWN),
    NAME_ELEMENT(KEY_OK),
    NAME_ELEMENT(KEY_SELECT),       NAME_ELEMENT(KEY_GOTO),
    NAME_ELEMENT(KEY_CLEAR),        NAME_ELEMENT(KEY_POWER2),
    NAME_ELEMENT(KEY_OPTION),       NAME_ELEMENT(KEY_INFO),
    NAME_ELEMENT(KEY_TIME),         NAME_ELEMENT(KEY_VENDOR),
    NAME_ELEMENT(KEY_ARCHIVE),      NAME_ELEMENT(KEY_PROGRAM),
    NAME_ELEMENT(KEY_CHANNEL),      NAME_ELEMENT(KEY_FAVORITES),
    NAME_ELEMENT(KEY_EPG),          NAME_ELEMENT(KEY_PVR),
    NAME_ELEMENT(KEY_MHP),          NAME_ELEMENT(KEY_LANGUAGE),
    NAME_ELEMENT(KEY_TITLE),        NAME_ELEMENT(KEY_SUBTITLE),
    NAME_ELEMENT(KEY_ANGLE),        NAME_ELEMENT(KEY_ZOOM),
    NAME_ELEMENT(KEY_MODE),         NAME_ELEMENT(KEY_KEYBOARD),
    NAME_ELEMENT(KEY_SCREEN),       NAME_ELEMENT(KEY_PC),
    NAME_ELEMENT(KEY_TV),           NAME_ELEMENT(KEY_TV2),
    NAME_ELEMENT(KEY_VCR),          NAME_ELEMENT(KEY_VCR2),
    NAME_ELEMENT(KEY_SAT),          NAME_ELEMENT(KEY_SAT2),
    NAME_ELEMENT(KEY_CD),           NAME_ELEMENT(KEY_TAPE),
    NAME_ELEMENT(KEY_RADIO),        NAME_ELEMENT(KEY_TUNER),
    NAME_ELEMENT(KEY_PLAYER),       NAME_ELEMENT(KEY_TEXT),
    NAME_ELEMENT(KEY_DVD),          NAME_ELEMENT(KEY_AUX),
    NAME_ELEMENT(KEY_MP3),          NAME_ELEMENT(KEY_AUDIO),
    NAME_ELEMENT(KEY_VIDEO),        NAME_ELEMENT(KEY_DIRECTORY),
    NAME_ELEMENT(KEY_LIST),         NAME_ELEMENT(KEY_MEMO),
    NAME_ELEMENT(KEY_CALENDAR),     NAME_ELEMENT(KEY_RED),
    NAME_ELEMENT(KEY_GREEN),        NAME_ELEMENT(KEY_YELLOW),
    NAME_ELEMENT(KEY_BLUE),         NAME_ELEMENT(KEY_CHANNELUP),
    NAME_ELEMENT(KEY_CHANNELDOWN),      NAME_ELEMENT(KEY_FIRST),
    NAME_ELEMENT(KEY_LAST),         NAME_ELEMENT(KEY_AB),
    NAME_ELEMENT(KEY_NEXT),         NAME_ELEMENT(KEY_RESTART),
    NAME_ELEMENT(KEY_SLOW),         NAME_ELEMENT(KEY_SHUFFLE),
    NAME_ELEMENT(KEY_BREAK),        NAME_ELEMENT(KEY_PREVIOUS),
    NAME_ELEMENT(KEY_DIGITS),       NAME_ELEMENT(KEY_TEEN),
    NAME_ELEMENT(KEY_TWEN),         NAME_ELEMENT(KEY_DEL_EOL),
    NAME_ELEMENT(KEY_DEL_EOS),      NAME_ELEMENT(KEY_INS_LINE),
    NAME_ELEMENT(KEY_DEL_LINE),
    NAME_ELEMENT(KEY_VIDEOPHONE),       NAME_ELEMENT(KEY_GAMES),
    NAME_ELEMENT(KEY_ZOOMIN),       NAME_ELEMENT(KEY_ZOOMOUT),
    NAME_ELEMENT(KEY_ZOOMRESET),        NAME_ELEMENT(KEY_WORDPROCESSOR),
    NAME_ELEMENT(KEY_EDITOR),       NAME_ELEMENT(KEY_SPREADSHEET),
    NAME_ELEMENT(KEY_GRAPHICSEDITOR),   NAME_ELEMENT(KEY_PRESENTATION),
    NAME_ELEMENT(KEY_DATABASE),     NAME_ELEMENT(KEY_NEWS),
    NAME_ELEMENT(KEY_VOICEMAIL),        NAME_ELEMENT(KEY_ADDRESSBOOK),
    NAME_ELEMENT(KEY_MESSENGER),        NAME_ELEMENT(KEY_DISPLAYTOGGLE),
#ifdef KEY_SPELLCHECK
    NAME_ELEMENT(KEY_SPELLCHECK),
#endif
#ifdef KEY_LOGOFF
    NAME_ELEMENT(KEY_LOGOFF),
#endif
#ifdef KEY_DOLLAR
    NAME_ELEMENT(KEY_DOLLAR),
#endif
#ifdef KEY_EURO
    NAME_ELEMENT(KEY_EURO),
#endif
#ifdef KEY_FRAMEBACK
    NAME_ELEMENT(KEY_FRAMEBACK),
#endif
#ifdef KEY_FRAMEFORWARD
    NAME_ELEMENT(KEY_FRAMEFORWARD),
#endif
#ifdef KEY_CONTEXT_MENU
    NAME_ELEMENT(KEY_CONTEXT_MENU),
#endif
#ifdef KEY_MEDIA_REPEAT
    NAME_ELEMENT(KEY_MEDIA_REPEAT),
#endif
#ifdef KEY_10CHANNELSUP
    NAME_ELEMENT(KEY_10CHANNELSUP),
#endif
#ifdef KEY_10CHANNELSDOWN
    NAME_ELEMENT(KEY_10CHANNELSDOWN),
#endif
#ifdef KEY_IMAGES
    NAME_ELEMENT(KEY_IMAGES),
#endif
    NAME_ELEMENT(KEY_DEL_EOL),      NAME_ELEMENT(KEY_DEL_EOS),
    NAME_ELEMENT(KEY_INS_LINE),     NAME_ELEMENT(KEY_DEL_LINE),
    NAME_ELEMENT(KEY_FN),           NAME_ELEMENT(KEY_FN_ESC),
    NAME_ELEMENT(KEY_FN_F1),        NAME_ELEMENT(KEY_FN_F2),
    NAME_ELEMENT(KEY_FN_F3),        NAME_ELEMENT(KEY_FN_F4),
    NAME_ELEMENT(KEY_FN_F5),        NAME_ELEMENT(KEY_FN_F6),
    NAME_ELEMENT(KEY_FN_F7),        NAME_ELEMENT(KEY_FN_F8),
    NAME_ELEMENT(KEY_FN_F9),        NAME_ELEMENT(KEY_FN_F10),
    NAME_ELEMENT(KEY_FN_F11),       NAME_ELEMENT(KEY_FN_F12),
    NAME_ELEMENT(KEY_FN_1),         NAME_ELEMENT(KEY_FN_2),
    NAME_ELEMENT(KEY_FN_D),         NAME_ELEMENT(KEY_FN_E),
    NAME_ELEMENT(KEY_FN_F),         NAME_ELEMENT(KEY_FN_S),
    NAME_ELEMENT(KEY_FN_B),
    NAME_ELEMENT(KEY_BRL_DOT1),     NAME_ELEMENT(KEY_BRL_DOT2),
    NAME_ELEMENT(KEY_BRL_DOT3),     NAME_ELEMENT(KEY_BRL_DOT4),
    NAME_ELEMENT(KEY_BRL_DOT5),     NAME_ELEMENT(KEY_BRL_DOT6),
    NAME_ELEMENT(KEY_BRL_DOT7),     NAME_ELEMENT(KEY_BRL_DOT8),
    NAME_ELEMENT(KEY_BRL_DOT9),     NAME_ELEMENT(KEY_BRL_DOT10),
#ifdef KEY_NUMERIC_0
    NAME_ELEMENT(KEY_NUMERIC_0),        NAME_ELEMENT(KEY_NUMERIC_1),
    NAME_ELEMENT(KEY_NUMERIC_2),        NAME_ELEMENT(KEY_NUMERIC_3),
    NAME_ELEMENT(KEY_NUMERIC_4),        NAME_ELEMENT(KEY_NUMERIC_5),
    NAME_ELEMENT(KEY_NUMERIC_6),        NAME_ELEMENT(KEY_NUMERIC_7),
    NAME_ELEMENT(KEY_NUMERIC_8),        NAME_ELEMENT(KEY_NUMERIC_9),
    NAME_ELEMENT(KEY_NUMERIC_STAR),     NAME_ELEMENT(KEY_NUMERIC_POUND),
#endif
    NAME_ELEMENT(KEY_BATTERY),
    NAME_ELEMENT(KEY_BLUETOOTH),        NAME_ELEMENT(KEY_BRIGHTNESS_CYCLE),
    NAME_ELEMENT(KEY_BRIGHTNESS_ZERO),
#ifdef KEY_DASHBOARD
    NAME_ELEMENT(KEY_DASHBOARD),
#endif
    NAME_ELEMENT(KEY_DISPLAY_OFF),      NAME_ELEMENT(KEY_DOCUMENTS),
    NAME_ELEMENT(KEY_FORWARDMAIL),      NAME_ELEMENT(KEY_NEW),
    NAME_ELEMENT(KEY_KBDILLUMDOWN),     NAME_ELEMENT(KEY_KBDILLUMUP),
    NAME_ELEMENT(KEY_KBDILLUMTOGGLE),   NAME_ELEMENT(KEY_REDO),
    NAME_ELEMENT(KEY_REPLY),        NAME_ELEMENT(KEY_SAVE),
#ifdef KEY_SCALE
    NAME_ELEMENT(KEY_SCALE),
#endif
    NAME_ELEMENT(KEY_SEND),
    NAME_ELEMENT(KEY_SCREENLOCK),       NAME_ELEMENT(KEY_SWITCHVIDEOMODE),
#ifdef KEY_UWB
    NAME_ELEMENT(KEY_UWB),
#endif
#ifdef KEY_VIDEO_NEXT
    NAME_ELEMENT(KEY_VIDEO_NEXT),
#endif
#ifdef KEY_VIDEO_PREV
    NAME_ELEMENT(KEY_VIDEO_PREV),
#endif
#ifdef KEY_WIMAX
    NAME_ELEMENT(KEY_WIMAX),
#endif
#ifdef KEY_WLAN
    NAME_ELEMENT(KEY_WLAN),
#endif
#ifdef KEY_RFKILL
    NAME_ELEMENT(KEY_RFKILL),
#endif
#ifdef KEY_MICMUTE
    NAME_ELEMENT(KEY_MICMUTE),
#endif
#ifdef KEY_CAMERA_FOCUS
    NAME_ELEMENT(KEY_CAMERA_FOCUS),
#endif
#ifdef KEY_WPS_BUTTON
    NAME_ELEMENT(KEY_WPS_BUTTON),
#endif
#ifdef KEY_TOUCHPAD_TOGGLE
    NAME_ELEMENT(KEY_TOUCHPAD_TOGGLE),
    NAME_ELEMENT(KEY_TOUCHPAD_ON),
    NAME_ELEMENT(KEY_TOUCHPAD_OFF),
#endif
#ifdef KEY_CAMERA_ZOOMIN
    NAME_ELEMENT(KEY_CAMERA_ZOOMIN),    NAME_ELEMENT(KEY_CAMERA_ZOOMOUT),
    NAME_ELEMENT(KEY_CAMERA_UP),        NAME_ELEMENT(KEY_CAMERA_DOWN),
    NAME_ELEMENT(KEY_CAMERA_LEFT),      NAME_ELEMENT(KEY_CAMERA_RIGHT),
#endif
#ifdef KEY_ATTENDANT_ON
    NAME_ELEMENT(KEY_ATTENDANT_ON),     NAME_ELEMENT(KEY_ATTENDANT_OFF),
    NAME_ELEMENT(KEY_ATTENDANT_TOGGLE), NAME_ELEMENT(KEY_LIGHTS_TOGGLE),
#endif

    NAME_ELEMENT(BTN_0),            NAME_ELEMENT(BTN_1),
    NAME_ELEMENT(BTN_2),            NAME_ELEMENT(BTN_3),
    NAME_ELEMENT(BTN_4),            NAME_ELEMENT(BTN_5),
    NAME_ELEMENT(BTN_6),            NAME_ELEMENT(BTN_7),
    NAME_ELEMENT(BTN_8),            NAME_ELEMENT(BTN_9),
    NAME_ELEMENT(BTN_LEFT),         NAME_ELEMENT(BTN_RIGHT),
    NAME_ELEMENT(BTN_MIDDLE),       NAME_ELEMENT(BTN_SIDE),
    NAME_ELEMENT(BTN_EXTRA),        NAME_ELEMENT(BTN_FORWARD),
    NAME_ELEMENT(BTN_BACK),         NAME_ELEMENT(BTN_TASK),
    NAME_ELEMENT(BTN_TRIGGER),      NAME_ELEMENT(BTN_THUMB),
    NAME_ELEMENT(BTN_THUMB2),       NAME_ELEMENT(BTN_TOP),
    NAME_ELEMENT(BTN_TOP2),         NAME_ELEMENT(BTN_PINKIE),
    NAME_ELEMENT(BTN_BASE),         NAME_ELEMENT(BTN_BASE2),
    NAME_ELEMENT(BTN_BASE3),        NAME_ELEMENT(BTN_BASE4),
    NAME_ELEMENT(BTN_BASE5),        NAME_ELEMENT(BTN_BASE6),
    NAME_ELEMENT(BTN_DEAD),         NAME_ELEMENT(BTN_C),
#ifdef BTN_SOUTH
    NAME_ELEMENT(BTN_SOUTH),        NAME_ELEMENT(BTN_EAST),
    NAME_ELEMENT(BTN_NORTH),        NAME_ELEMENT(BTN_WEST),
#else
    NAME_ELEMENT(BTN_A),            NAME_ELEMENT(BTN_B),
    NAME_ELEMENT(BTN_X),            NAME_ELEMENT(BTN_Y),
#endif
    NAME_ELEMENT(BTN_Z),            NAME_ELEMENT(BTN_TL),
    NAME_ELEMENT(BTN_TR),           NAME_ELEMENT(BTN_TL2),
    NAME_ELEMENT(BTN_TR2),          NAME_ELEMENT(BTN_SELECT),
    NAME_ELEMENT(BTN_START),        NAME_ELEMENT(BTN_MODE),
    NAME_ELEMENT(BTN_THUMBL),       NAME_ELEMENT(BTN_THUMBR),
    NAME_ELEMENT(BTN_TOOL_PEN),     NAME_ELEMENT(BTN_TOOL_RUBBER),
    NAME_ELEMENT(BTN_TOOL_BRUSH),       NAME_ELEMENT(BTN_TOOL_PENCIL),
    NAME_ELEMENT(BTN_TOOL_AIRBRUSH),    NAME_ELEMENT(BTN_TOOL_FINGER),
    NAME_ELEMENT(BTN_TOOL_MOUSE),       NAME_ELEMENT(BTN_TOOL_LENS),
    NAME_ELEMENT(BTN_TOUCH),        NAME_ELEMENT(BTN_STYLUS),
    NAME_ELEMENT(BTN_STYLUS2),      NAME_ELEMENT(BTN_TOOL_DOUBLETAP),
    NAME_ELEMENT(BTN_TOOL_TRIPLETAP),
#ifdef BTN_TOOL_QUADTAP
    NAME_ELEMENT(BTN_TOOL_QUADTAP),
#endif
    NAME_ELEMENT(BTN_GEAR_DOWN),
    NAME_ELEMENT(BTN_GEAR_UP),

#ifdef BTN_DPAD_UP
    NAME_ELEMENT(BTN_DPAD_UP),      NAME_ELEMENT(BTN_DPAD_DOWN),
    NAME_ELEMENT(BTN_DPAD_LEFT),        NAME_ELEMENT(BTN_DPAD_RIGHT),
#endif
#ifdef KEY_ALS_TOGGLE
    NAME_ELEMENT(KEY_ALS_TOGGLE),
#endif
#ifdef KEY_BUTTONCONFIG
    NAME_ELEMENT(KEY_BUTTONCONFIG),
#endif
#ifdef KEY_TASKMANAGER
    NAME_ELEMENT(KEY_TASKMANAGER),
#endif
#ifdef KEY_JOURNAL
    NAME_ELEMENT(KEY_JOURNAL),
#endif
#ifdef KEY_CONTROLPANEL
    NAME_ELEMENT(KEY_CONTROLPANEL),
#endif
#ifdef KEY_APPSELECT
    NAME_ELEMENT(KEY_APPSELECT),
#endif
#ifdef KEY_SCREENSAVER
    NAME_ELEMENT(KEY_SCREENSAVER),
#endif
#ifdef KEY_VOICECOMMAND
    NAME_ELEMENT(KEY_VOICECOMMAND),
#endif
#ifdef KEY_BRIGHTNESS_MIN
    NAME_ELEMENT(KEY_BRIGHTNESS_MIN),
#endif
#ifdef KEY_BRIGHTNESS_MAX
    NAME_ELEMENT(KEY_BRIGHTNESS_MAX),
#endif
#ifdef KEY_KBDINPUTASSIST_PREV
    NAME_ELEMENT(KEY_KBDINPUTASSIST_PREV),
#endif
#ifdef KEY_KBDINPUTASSIST_NEXT
    NAME_ELEMENT(KEY_KBDINPUTASSIST_NEXT),
#endif
#ifdef KEY_KBDINPUTASSIST_PREVGROUP
    NAME_ELEMENT(KEY_KBDINPUTASSIST_PREVGROUP),
#endif
#ifdef KEY_KBDINPUTASSIST_NEXTGROUP
    NAME_ELEMENT(KEY_KBDINPUTASSIST_NEXTGROUP),
#endif
#ifdef KEY_KBDINPUTASSIST_ACCEPT
    NAME_ELEMENT(KEY_KBDINPUTASSIST_ACCEPT),
#endif
#ifdef KEY_KBDINPUTASSIST_CANCEL
    NAME_ELEMENT(KEY_KBDINPUTASSIST_CANCEL),
#endif
#ifdef BTN_TRIGGER_HAPPY
    NAME_ELEMENT(BTN_TRIGGER_HAPPY1),   NAME_ELEMENT(BTN_TRIGGER_HAPPY11),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY2),   NAME_ELEMENT(BTN_TRIGGER_HAPPY12),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY3),   NAME_ELEMENT(BTN_TRIGGER_HAPPY13),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY4),   NAME_ELEMENT(BTN_TRIGGER_HAPPY14),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY5),   NAME_ELEMENT(BTN_TRIGGER_HAPPY15),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY6),   NAME_ELEMENT(BTN_TRIGGER_HAPPY16),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY7),   NAME_ELEMENT(BTN_TRIGGER_HAPPY17),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY8),   NAME_ELEMENT(BTN_TRIGGER_HAPPY18),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY9),   NAME_ELEMENT(BTN_TRIGGER_HAPPY19),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY10),  NAME_ELEMENT(BTN_TRIGGER_HAPPY20),

    NAME_ELEMENT(BTN_TRIGGER_HAPPY21),  NAME_ELEMENT(BTN_TRIGGER_HAPPY31),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY22),  NAME_ELEMENT(BTN_TRIGGER_HAPPY32),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY23),  NAME_ELEMENT(BTN_TRIGGER_HAPPY33),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY24),  NAME_ELEMENT(BTN_TRIGGER_HAPPY34),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY25),  NAME_ELEMENT(BTN_TRIGGER_HAPPY35),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY26),  NAME_ELEMENT(BTN_TRIGGER_HAPPY36),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY27),  NAME_ELEMENT(BTN_TRIGGER_HAPPY37),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY28),  NAME_ELEMENT(BTN_TRIGGER_HAPPY38),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY29),  NAME_ELEMENT(BTN_TRIGGER_HAPPY39),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY30),  NAME_ELEMENT(BTN_TRIGGER_HAPPY40),
#endif
#ifdef BTN_TOOL_QUINTTAP
    NAME_ELEMENT(BTN_TOOL_QUINTTAP),
#endif
};

static const char * const kAbsval[6] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat ", "Resolution "};

static const char * const kRelatives[REL_MAX + 1] = {
    [0 ... REL_MAX] = NULL,
    NAME_ELEMENT(REL_X),            NAME_ELEMENT(REL_Y),
    NAME_ELEMENT(REL_Z),            NAME_ELEMENT(REL_RX),
    NAME_ELEMENT(REL_RY),           NAME_ELEMENT(REL_RZ),
    NAME_ELEMENT(REL_HWHEEL),
    NAME_ELEMENT(REL_DIAL),         NAME_ELEMENT(REL_WHEEL),
    NAME_ELEMENT(REL_MISC),
};

static const char * const kAbsolutes[ABS_MAX + 1] = {
    [0 ... ABS_MAX] = NULL,
    NAME_ELEMENT(ABS_X),            NAME_ELEMENT(ABS_Y),
    NAME_ELEMENT(ABS_Z),            NAME_ELEMENT(ABS_RX),
    NAME_ELEMENT(ABS_RY),           NAME_ELEMENT(ABS_RZ),
    NAME_ELEMENT(ABS_THROTTLE),     NAME_ELEMENT(ABS_RUDDER),
    NAME_ELEMENT(ABS_WHEEL),        NAME_ELEMENT(ABS_GAS),
    NAME_ELEMENT(ABS_BRAKE),        NAME_ELEMENT(ABS_HAT0X),
    NAME_ELEMENT(ABS_HAT0Y),        NAME_ELEMENT(ABS_HAT1X),
    NAME_ELEMENT(ABS_HAT1Y),        NAME_ELEMENT(ABS_HAT2X),
    NAME_ELEMENT(ABS_HAT2Y),        NAME_ELEMENT(ABS_HAT3X),
    NAME_ELEMENT(ABS_HAT3Y),        NAME_ELEMENT(ABS_PRESSURE),
    NAME_ELEMENT(ABS_DISTANCE),     NAME_ELEMENT(ABS_TILT_X),
    NAME_ELEMENT(ABS_TILT_Y),       NAME_ELEMENT(ABS_TOOL_WIDTH),
    NAME_ELEMENT(ABS_VOLUME),       NAME_ELEMENT(ABS_MISC),
#ifdef ABS_MT_BLOB_ID
    NAME_ELEMENT(ABS_MT_TOUCH_MAJOR),
    NAME_ELEMENT(ABS_MT_TOUCH_MINOR),
    NAME_ELEMENT(ABS_MT_WIDTH_MAJOR),
    NAME_ELEMENT(ABS_MT_WIDTH_MINOR),
    NAME_ELEMENT(ABS_MT_ORIENTATION),
    NAME_ELEMENT(ABS_MT_POSITION_X),
    NAME_ELEMENT(ABS_MT_POSITION_Y),
    NAME_ELEMENT(ABS_MT_TOOL_TYPE),
    NAME_ELEMENT(ABS_MT_BLOB_ID),
#endif
#ifdef ABS_MT_TRACKING_ID
    NAME_ELEMENT(ABS_MT_TRACKING_ID),
#endif
#ifdef ABS_MT_PRESSURE
    NAME_ELEMENT(ABS_MT_PRESSURE),
#endif
#ifdef ABS_MT_SLOT
    NAME_ELEMENT(ABS_MT_SLOT),
#endif
#ifdef ABS_MT_TOOL_X
    NAME_ELEMENT(ABS_MT_TOOL_X),
    NAME_ELEMENT(ABS_MT_TOOL_Y),
    NAME_ELEMENT(ABS_MT_DISTANCE),
#endif
#endif
};

static const char * const kMisc[MSC_MAX + 1] = {
/*
    [ 0 ... MSC_MAX] = NULL,
    NAME_ELEMENT(MSC_SERIAL),       NAME_ELEMENT(MSC_PULSELED),
    NAME_ELEMENT(MSC_GESTURE),      NAME_ELEMENT(MSC_RAW),
    NAME_ELEMENT(MSC_SCAN),
#ifdef MSC_TIMESTAMP
    NAME_ELEMENT(MSC_TIMESTAMP),
#endif
 */
};

static const char * const kLeds[LED_MAX + 1] = {
    [0 ... LED_MAX] = NULL,
    NAME_ELEMENT(LED_NUML),         NAME_ELEMENT(LED_CAPSL),
    NAME_ELEMENT(LED_SCROLLL),      NAME_ELEMENT(LED_COMPOSE),
    NAME_ELEMENT(LED_KANA),         NAME_ELEMENT(LED_SLEEP),
    NAME_ELEMENT(LED_SUSPEND),      NAME_ELEMENT(LED_MUTE),
    NAME_ELEMENT(LED_MISC),
#ifdef LED_MAIL
    NAME_ELEMENT(LED_MAIL),
#endif
#ifdef LED_CHARGING
    NAME_ELEMENT(LED_CHARGING),
#endif
};

static const char * const kRepeats[REP_MAX + 1] = {
        /*
    [0 ... REP_MAX] = NULL,
    NAME_ELEMENT(REP_DELAY),        NAME_ELEMENT(REP_PERIOD)
         */
};

static const char * const kSounds[SND_MAX + 1] = {
        /*
    [0 ... SND_MAX] = NULL,
    NAME_ELEMENT(SND_CLICK),        NAME_ELEMENT(SND_BELL),
    NAME_ELEMENT(SND_TONE)
         */
};

static const char * const kSyns[SYN_MAX + 1] = {
    [0 ... SYN_MAX] = NULL,
    NAME_ELEMENT(SYN_REPORT),
    NAME_ELEMENT(SYN_CONFIG),
    NAME_ELEMENT(SYN_MT_REPORT),
    NAME_ELEMENT(SYN_DROPPED)
};

static const char * const kSwitches[SW_MAX + 1] = {
    [0 ... SW_MAX] = NULL,
    NAME_ELEMENT(SW_LID),
    NAME_ELEMENT(SW_TABLET_MODE),
    NAME_ELEMENT(SW_HEADPHONE_INSERT),
#ifdef SW_RFKILL_ALL
    NAME_ELEMENT(SW_RFKILL_ALL),
#endif
#ifdef SW_MICROPHONE_INSERT
    NAME_ELEMENT(SW_MICROPHONE_INSERT),
#endif
#ifdef SW_DOCK
    NAME_ELEMENT(SW_DOCK),
#endif
#ifdef SW_LINEOUT_INSERT
    NAME_ELEMENT(SW_LINEOUT_INSERT),
#endif
#ifdef SW_JACK_PHYSICAL_INSERT
    NAME_ELEMENT(SW_JACK_PHYSICAL_INSERT),
#endif
#ifdef SW_VIDEOOUT_INSERT
    NAME_ELEMENT(SW_VIDEOOUT_INSERT),
#endif
#ifdef SW_CAMERA_LENS_COVER
    NAME_ELEMENT(SW_CAMERA_LENS_COVER),
    NAME_ELEMENT(SW_KEYPAD_SLIDE),
    NAME_ELEMENT(SW_FRONT_PROXIMITY),
#endif
#ifdef SW_ROTATE_LOCK
    NAME_ELEMENT(SW_ROTATE_LOCK),
#endif
#ifdef SW_LINEIN_INSERT
    NAME_ELEMENT(SW_LINEIN_INSERT),
#endif
#ifdef SW_MUTE_DEVICE
    NAME_ELEMENT(SW_MUTE_DEVICE),
#endif
};

static const char * const kForce[FF_MAX + 1] = {
    [0 ... FF_MAX] = NULL,
    NAME_ELEMENT(FF_RUMBLE),        NAME_ELEMENT(FF_PERIODIC),
    NAME_ELEMENT(FF_CONSTANT),      NAME_ELEMENT(FF_SPRING),
    NAME_ELEMENT(FF_FRICTION),      NAME_ELEMENT(FF_DAMPER),
    NAME_ELEMENT(FF_INERTIA),       NAME_ELEMENT(FF_RAMP),
    NAME_ELEMENT(FF_SQUARE),        NAME_ELEMENT(FF_TRIANGLE),
    NAME_ELEMENT(FF_SINE),          NAME_ELEMENT(FF_SAW_UP),
    NAME_ELEMENT(FF_SAW_DOWN),      NAME_ELEMENT(FF_CUSTOM),
    NAME_ELEMENT(FF_GAIN),          NAME_ELEMENT(FF_AUTOCENTER),
};

static const char * const kForcestatus[FF_STATUS_MAX + 1] = {
    [0 ... FF_STATUS_MAX] = NULL,
    NAME_ELEMENT(FF_STATUS_STOPPED),    NAME_ELEMENT(FF_STATUS_PLAYING),
};

static const char * const * const kNames[EV_MAX + 1] = {
    [0 ... EV_MAX] = NULL,
    [EV_SYN] = kSyns,            [EV_KEY] = kKeys,
    [EV_REL] = kRelatives,           [EV_ABS] = kAbsolutes,
    [EV_MSC] = kMisc,            [EV_LED] = kLeds,
    [EV_SND] = kSounds,          [EV_REP] = kRepeats,
    [EV_SW] = kSwitches,
    [EV_FF] = kForce,            [EV_FF_STATUS] = kForcestatus,
};
#endif
#if 0
static inline const char* typename(unsigned int type)
{
    return (type <= EV_MAX && 
            kevents[type]) ? kevents[type] : "?";
}

static inline const char* codename(unsigned int type, unsigned int code)
{
    return (type <= EV_MAX && 
            code <= gmaxval[type] && 
            kNames[type] && 
            kNames[type][code]) ? kNames[type][code] : "?";
}
#endif
/**
 *
 * @param start
 * @return
 */
int time_exceed(struct timeval start)
{
    struct timeval currentTime;
    uint32 ui8Offset = 0;
    FUNC_START

    gettimeofday(&currentTime, NULL);

    ui8Offset = (currentTime.tv_sec - start.tv_sec)*SEC_TO_USEC + (currentTime.tv_usec - start.tv_usec);
    if (ui8Offset >= LONG_PRESS_DEFAULT_DURATION)
    {
        return True;
    }
    else
    {
        return False;
    }
    FUNC_END
}


/**
 * 按键长按线程处理
 * @param arg
 * @return
 */
static void *key_long_press_thread_routine(void *arg)
{
    struct timeval start;
    //int32 i4_ret = UI_OK;
    int32 i4_ret=0;
    int32 long_key_msg;
    FUNC_START
    while (1)
    {
        pthread_mutex_lock(&gmutex);
        //线程等待
        pthread_cond_wait(&gkeycond, &gmutex);

        gettimeofday(&start, NULL);

        while (gKeyPressed)
        {
            usleep(LONG_PRESS_PERIOD); //100ms

            if (time_exceed(start))
            {
                /* key long press process */
                printf("key long press process,KeyValueRecord = %d\n",gKeyValueRecord);
                gKeyLongPressed = True;
                gKeyPressed = False; //add by lei.xiao

                    switch (gKeyValueRecord)
                    {

                    case KEY_WAKEUP:

                        printf("KEY p long press\n");
                          break;
                    default:
                          printf("key long press isn't MUTE/BLUTOOTH/POWER/VOLUME+/VOLUME-\n");
                          break;
                    }

                printf("long key=%d\n", gKeyValueRecord);

                if (0 != i4_ret)
                {
                    printf("[lei]end long key to sm fail(%d)!!\n", i4_ret);
                }
            }
        }
        pthread_mutex_unlock(&gmutex);
    }
    FUNC_END

    return NULL;
}

void Long_Press_Ctrl()
{
    int ret = 0;
    pthread_attr_t attr;
    pthread_t key_long_press_thread;
    FUNC_START
    pthread_mutex_init(&gmutex, NULL);
    pthread_cond_init(&gkeycond, NULL);
    pthread_attr_init(&attr);

    ret = pthread_create(&key_long_press_thread, &attr, key_long_press_thread_routine, NULL);
    if (ret != 0)
    {
        printf("create key pthread failed.\n");
    }
    FUNC_END
}

static void *Gesture_Press_Thread_Handle(void *arg){

}

void Gesture_Press_Ctrl(){

}

static int is_keypad_device( char *filename)
{
    int i;
    char name[PATH_MAX];
    char *strpos = NULL;

    FUNC_START
    for (i = 0; i < (int) ARRAY_SIZE(keypad_device_name); i++)
    {
        printf("check device name: %s v.s. %s \n", filename, keypad_device_name[i]);
        strpos = strcasestr(filename, keypad_device_name[i]);
        if (strpos != NULL)
        {
            return True;
        }
    }
    FUNC_END
    return False;
}

static int is_irrx_device( char *filename)
{
    int i;
    char name[PATH_MAX];
    char *strpos = NULL;
    FUNC_START
    for (i = 0; i < (int) ARRAY_SIZE(mtk_IRRX_device_name); i++)
    {
        printf("check device name: %s v.s. %s \n", filename, mtk_IRRX_device_name[i]);
        strpos = strcasestr(filename, mtk_IRRX_device_name[i]);
        if (strpos != NULL)
        {
            return True;
        }
    }
    FUNC_END
    return False;
}

/**
 *
 * @param device
 * @return
 */

static int open_device(const char *device)
{
    int version;
    int fd;
    struct pollfd *new_ufds;
    char **new_device_names;
    char name[BUFFER_LENGTH_S];
    char location[BUFFER_LENGTH_S];
    char idstr[BUFFER_LENGTH_S];
    struct input_id id;
    int print_flags = KEY_RECORD_MASK;
    FUNC_START
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        printf("could not open %s, %s\n", device, strerror(errno));
        return -1;
    }
    if (ioctl(fd, EVIOCGVERSION, &version))
    {
        printf("could not get driver version for %s, %s\n", device, strerror(errno));
        return -1;
    }
    if (ioctl(fd, EVIOCGID, &id))
    {
        printf("could not get driver id for %s, %s\n", device, strerror(errno));
        return -1;
    }
    name[sizeof(name) - 1] = '\0';
    location[sizeof(location) - 1] = '\0';
    idstr[sizeof(idstr) - 1] = '\0';
    if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
    {
        name[0] = '\0';
    }
    printf("device=%s,open_device_name=%s \n", device,name);

    if (ioctl(fd, EVIOCGPHYS(sizeof(location) - 1), &location) < 1)
    {
        location[0] = '\0';
    }
    if (ioctl(fd, EVIOCGUNIQ(sizeof(idstr) - 1), &idstr) < 1)
    {
        idstr[0] = '\0';
    }
    new_ufds = (struct pollfd*)realloc(gufds, sizeof(gufds[0]) * (gnfds + 1));
    if (NULL == new_ufds)
    {
        printf("out of memory\n");
        return -1;
    }
    gufds = new_ufds;
    new_device_names = (char**)realloc(gdevice_names, sizeof(gdevice_names[0]) * (gnfds + 1));
    if (NULL == new_device_names)
    {
        printf("out of memory\n");
        return -1;
    }
    gdevice_names = new_device_names;
    gufds[gnfds].fd = fd;
    gufds[gnfds].events = POLLIN;
    gdevice_names[gnfds] = strdup(device);

    /**
     *
     */
    if (is_keypad_device(name))
    {
        device_type_nfds[gnfds] = KEYPAD_DEVICE_TYPE;
    }
    if (is_irrx_device(name))
    {
        device_type_nfds[gnfds] = IR_DEVICE_TYPE;
    }
    gnfds++;
    FUNC_END

    return 0;
}

int close_device(const char *device)
{
    int i;
    FUNC_START
    for (i = 1; i < gnfds; i++) {
        if (!strcmp(gdevice_names[i], device))
        {
            int count = gnfds - i - 1;
            printf(("remove device %d: %s\n", i, device));
            free(gdevice_names[i]);
            memmove(gdevice_names + i, gdevice_names + i + 1, sizeof(gdevice_names[0]) * count);
            memmove(gufds + i, gufds + i + 1, sizeof(gufds[0]) * count);
            gnfds--;
            return 0;
        }
    }
    printf("remote device: %s not found\n", device);
    FUNC_END
    return -1;
}

static int read_notify(const char *dirname, int nfd)
{
    int res;
    char devname[PATH_MAX];
    char *filename;
    char event_buf[BUFFER_LENGTH_L];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;
    FUNC_START
    res = read(nfd, event_buf, sizeof(event_buf));
    if (res < (int)sizeof(*event))
    {
        if (errno == EINTR)
        {
            return 0;
        }
        printf("could not get event, %s\n", strerror(errno));
        return 1;
    }

    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';

    while (res >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        if (event->len) {
            strcpy(filename, event->name);
            if (event->mask & IN_CREATE) {
                open_device(devname);
            }
            else {
                close_device(devname);
            }
        }
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }
    FUNC_END
    return 0;
}

/**
 * 读取/dev/input/下的设备，并打开
 */
static int scan_dir(const char *dirname)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;

    FUNC_START
    dir = opendir(dirname);
    if (NULL == dir)
    {
        return -1;
    }
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while ((de = readdir(dir)))
    {
        if ((de->d_name[0] == '.') &&
           ((de->d_name[1] == '\0') ||
            ((de->d_name[1] == '.') && 
            (de->d_name[2] == '\0'))))
        {
            continue;
        }
        strcpy(filename, de->d_name);
        printf("open_device %s\n",devname);
        open_device(devname);
    }
    closedir(dir);
    FUNC_END
    return 0;
}




void *key_event_monitor_thread(void *arg)
{
    FUNC_START
    int res, i, j;
    int pollres;
    const char *device = NULL;
    struct input_event event;
    int32 irbtnMsg;

    glongPressDuration = 500;
    /**
     * 开启长按键判断线程
     */
    Long_Press_Ctrl();

    /**
     * 手势判断
     */
    Gesture_Press_Ctrl();

    gnfds = 1;
    gufds = (struct pollfd *)calloc(1, sizeof(gufds[0]));
    gufds[0].fd = inotify_init();
    gufds[0].events = POLLIN;

    /**
     *使用inotify机制监控文件或目录
     */
    gwd = inotify_add_watch(gufds[0].fd, INPUT_DEVICE_PATH, IN_DELETE | IN_CREATE);
    if (gwd < 0)
    {
        printf("could not add watch for %s, %s\n", INPUT_DEVICE_PATH, strerror(errno));
    }
    res = scan_dir(INPUT_DEVICE_PATH);
    if (res < 0)
    {
        printf("scan dir failed for %s\n", INPUT_DEVICE_PATH);
    }


    /*
     *
     */
    while (1)
    {
        //pollres > 0,gufds准备好好读、写或出错状态
        pollres = poll(gufds, gnfds, -1);

        //POLLIN:普通或优先级带数据可读
        if (gufds[0].revents & POLLIN)
        {
            read_notify(INPUT_DEVICE_PATH, gufds[0].fd);
        }

        for (i = 1; i < gnfds; i++)
        {
            if (gufds[i].revents)
            {
                if (gufds[i].revents & POLLIN)
                {
                    res = read(gufds[i].fd, &event, sizeof(event));
                    if (res < (int)sizeof(event))
                    {
                        printf("could not get event\n");
                    }

                    /**
                     * 按键处理
                     */
                    if (KEYPAD_DEVICE_TYPE == device_type_nfds[i] && EV_KEY == event.type)
                    {
                       //printf("KeyValueRecord=%d \n",KeyValueRecord);
                        /**
                         * 按键按下处理
                         */
                        if (1 == event.value)   /* key press process */
                        {
                            if ((KEY_WAKEUP == event.code || KEY_VOLUMEUP == event.code ||
                             KEY_POWER == event.code) && KEY_RECORD_MASK == gKeyValueRecord)
                            {
                              mprintf("[%s]==>key code=%d,value%d\n",__FUNCTION__,event.code,event.value);
//                              printf("<user_interface> key code = %d (%s), value = %d \n",
//                                     event.code, codename(event.type, event.code), event.value);
                              gKeyPressed = True;
                              gKeyLongPressed = False;  /* long press flag, handle in the function key_long_press_thread_routine */
                              gKeyValueRecord = event.code;
                              pthread_cond_signal(&gkeycond);/*trigger long key thread to start time record*/
                            } else if(KEY_A==event.code||KEY_B==event.code||
                                    KEY_C==event.code||KEY_D==event.code||
                                    KEY_E==event.code||KEY_F==event.code||
                                    KEY_G==event.code||KEY_H==event.code||
                                    KEY_I==event.code||KEY_J==event.code||
                                    KEY_J==event.code||KEY_K==event.code||
                                    KEY_L==event.code)
                            {

                            }

                        }
                        else    /* 按键释放处理 */
                        {
                            /*so far, only handle the case of one key press, if other key release, don't care it*/
                            if (gKeyValueRecord == event.code)
                            {
 //                             printf("<user_interface> key code = %d (%s), value = %d \n",
//                              event.code, codename(event.type, event.code), event.value);

                                printf(" KEY released \n");
                                gKeyPressed = False;    /* long press flag clear */
                                gKeyValueRecord = KEY_RECORD_MASK;
                                if (!gKeyLongPressed)
                                {

                                }
                            }
                        }
                    }

                    /**
                     * 针对IR设备（猜测是无线键盘鼠标之类，项目中用不到)
                     */
                    if (IR_DEVICE_TYPE == device_type_nfds[i] && EV_KEY == event.type)
                    {
                        if (1 == event.value) //IRRX BTN press down
                        {
                            printf("<user_interface>[IR]pressed keycode=%d.\n", event.code);
                            for (j=0; j<IR_SM_NUM; j++)
                            {
                                if (ir_sm_map[j].IR_Keycode == event.code)
                                {
                                    if (KEY_RECORD_MASK != ir_sm_map[j].SM_Msg_Type)
                                    {
                                        printf("<user_interface>Send IR key to SM:%s.\n",
                                               ir_sm_map[j].SM_MSG_NAME);
                                      //  user_interface_send_key_to_sm(ir_sm_map[j].SM_Msg_Type);
                                    }
                                    else
                                    {
                                        switch (ir_sm_map[j].IR_Keycode)
                                        {
                                        case BTN_IR_RETURN:
                                            irbtnMsg = KEY_MUTE;
                                            break;
                                        case BTN_IR_UP:
                                            irbtnMsg = KEY_VOLUMEUP;
                                            break;
                                        case BTN_IR_DOWN:
                                            irbtnMsg = KEY_VOLUMEDOWN;
                                            break;
                                        default:
                                            break;
                                        }
                                        printf("<user_interface>%s,send IR to user_inter handle.\n",
                                               ir_sm_map[j].SM_MSG_NAME);

                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    FUNC_END
}


int create_KeyThread(){
    pthread_t keythread;
    FUNC_START
    int ret=pthread_create(&keythread,NULL,key_event_monitor_thread,NULL);
    if(ret){
        printf("creat key thread fail\n");
    }
    FUNC_END
    return 0;

}

//#ifdef __cplusplus
} // extern "C"
//#endif // __cplusplus