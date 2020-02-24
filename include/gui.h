#ifndef LMCVITTI_Y247PAN_GUI_CONSTANTS
#define LMCVITTI_Y247PAN_GUI_CONSTANTS

#define ESC					(char) 27
#define DEL					(char) 8
#define CLEAR_SCREEN		"[2J"
#define CLEAR_TO_END		"[K"
#define SAVE_CURSOR			"[s"
#define RESTORE_CURSOR		"[u"
#define SCROLL_UP			"M"
#define SCROLL_DOWN			"\033D\033[35;2H" // note: \033 is considered 2 characters, not 4. don't ask me why
#define MOVE_CURSOR_HOME	"[H"
#define MOVE_CURSOR			"[  ;  H"
#define CURSOR_CMD_HOME		"[35;2H"
	#define MOVE_CURSOR_ROW_IDX	1
	#define MOVE_CURSOR_COL_IDX 4
#define SCROLL_SECTION	"[26;35r"
#define SECTION_SEPARATOR	"|-------------------------------------------------------------------------------------------------|\n\r"
#define SWITCH_COL			90
#define SWITCH_STATE_COL	96
#define SWITCH_ROW			"|                                                                                      |     |    |\n\r"
#define CONSOLE_ROW			"|                                                                                                 |\n\r"


#endif