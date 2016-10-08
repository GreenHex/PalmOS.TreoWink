/*
 * Common.h
 */
 
#define MIN_VERSION  						sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)
#define LAUNCH_FLAGS 						(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)

/* Compiler options **************************************************/
// #define ERROR_CHECK_FULL 				// something...

#define BUILD_FOR_TREO_600 					// Treo 600 specific build
#define BUILD_FOR_TREO_680
#define BUILD_FOR_TREO_700p
/***********************************************************************/

#define ALERT_MEMORY_ERROR					"Memory Error"
#define ALERT_PHONE_ERROR					"Phone Error"	

/*
 * Miscellaneous stuff
 */
typedef enum {
	offNormal = 0,
	offExtend = 1
} OffTimeExtendAction_e;

typedef enum {
	typeManual = 0,
	typeCradle = 1	
} OffType_e;

typedef struct { 
	Boolean									bEnabled; // 1
	UInt16									timeOffIdx; // 2
	UInt16									optionMaskIdx; // 2
	UInt16									keyPressIdx; // 2
	Boolean									bDisplayTimeSelection; // 1
	Boolean									bOffOnAppSwitch; // 1
	Boolean									bBeepConfirmation; // 1
	Boolean									bKeepOnInCradle; // 1
} AppPreferences_t;

#define PREFS_ENABLED						true
#define PREFS_TIME_OFF_IDX					0
#define PREFS_OPTION_MASK_IDX				2
#define PREFS_KEYPRESS_IDX					2
#define PREFS_DISPLAY_TIME_SELECTION		true
#define PREFS_OFF_ON_APP_SWITCH				true
#define PREFS_BEEP_CONFIRMATION				true
#define PREFS_KEEP_ON_IN_CRADLE				false

typedef struct {
	Boolean									bOffTimeExtended;
	UInt16									offExtendTime;
	Boolean									bOffOnAppSwitch;
	Boolean									bInCradle;
} USPrefs_t;

#define US_PREFS_OFF_TIME_EXTENDED			false
#define US_PREFS_OFF_EXTEND_TIME			180
#define US_PREFS_OFF_ON_APPSWITCH			false
#define US_PREFS_IN_CRADLE					false

#define DISP_WIDTH							160
#define DISP_HEIGHT							DISP_WIDTH

#define LINE_GAP							2

#define RECT_WIDTH_SMALL					58
#define RECT_HEIGHT_SMALL					16

#define RECT_WIDTH_BIG						110
#define RECT_HEIGHT_BIG						30

#define TXT_OFF_X							5
#define TXT_OFF_Y							2

#define STR_ON_AT							"Off at ~"
#define STR_OFF								"AutoOff Default"
#define STR_CANCELLED						"CANCELLED"
#define STR_IN_CRADLE						"On Cradle"

#define POPUP_FORM_WAIT_TIME				3

/*
 * Common.h
 */
