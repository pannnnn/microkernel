#ifndef LMCVITTI_Y247PAN_DISPLAY
#define LMCVITTI_Y247PAN_DISPLAY

// color
#define ANSI_RESET "\033[0m"
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_PURPLE "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

// terminal code
#define BACKSPACE "\033[1D "
#define CLEAR_END_OF_LINE "\033[K"
#define CLEAR_START_OF_LINE "\033[1K"
#define CLEAR_LINE "\033[2K"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define CURSOR_COMMAND_LINE "\033[31;5H"
#define PATCH_COMMAND_LINE_PREFIX "\033[31;0H| > "
#define SAVE_CURSOR "\033[s"
#define RESTORE_CURSOR "\033[u"
#define HIDE_CURSOR "\033[?25l"

// data attributes
#define MAX_MOVEMENT_BUFFER 128;
#define SENSOR_TRACKED 10
#define SWITCH_TRACKED 22


#endif