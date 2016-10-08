/*
 * AppMain.c
 */
 
#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <PalmTypes.h>
#include <Form.h>
#include <AlarmMgr.h>

#include "Global.h"
#include "AppResources.h"

// PhoneUtils.c
extern void 				DelayTask(UInt32 DelaySeconds);
extern void 				Alert(Char* MsgType, Char* Msg, Err err);
extern void 				beep(UInt8 numBeeps);
extern Boolean 				IsPhoneGSM(void);
extern Err 					RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);

// Panl.c
extern void 				PanelFormClose(FormPtr pForm);

// Prototypes
static void 				RegisterForNotifications(AppPreferences_t* prefsP);
static void 				writePrefs(AppPreferences_t* prefsP);
static void 				readPrefs(AppPreferences_t* prefsP);
static void 				writeUSPrefs(USPrefs_t* usPrefsP);
static void 				readUSPrefs(USPrefs_t* usPrefsP);
static Err 					AppStart(void);
static void 				AppStop(void);
static void 				InitializeMainForm(FormType* pForm, AppPreferences_t* prefsP);
static Boolean 				MainFormHandleEvent(EventType* pEvent);
static Boolean 				AppHandleEvent(EventType* pEvent);
static void 				AppEventLoop(void);
static Err	 				SaveRect(RectangleType* rectP, WinHandle* winHP, RectangleType* rectObscuredP);
static void 				RestoreRect(WinHandle* winHP, RectangleType* rectObscuredP);
static void 				DisplayState(Char* str1, Char* str2, UInt16 rectWidth, UInt16 rectHeight);
static void 				SetOffState(USPrefs_t* usPrefsP, OffTimeExtendAction_e state, OffType_e offType);
static void 				getStrDelayTime(UInt32 delayTime, Char* str);
static void 				getStrDateTime(UInt32 TimeSecs, Char* dtStr);
static void 				popupFormCallback(UInt16 almProcCmd, SysAlarmTriggeredParamType *paramP);
static Boolean 				popupFormHandleEvent(EventType* pEvent);
static Boolean 				popupForm(UInt16* initIdxP, Boolean* OffOnAppSwitchP);
UInt32 						PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

/*
 * writePrefs
 */
static void writePrefs(AppPreferences_t* prefsP)
{
	PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, prefsP, sizeof(AppPreferences_t), true);

} // writePrefs

/*
 * readPrefs
 */
static void readPrefs(AppPreferences_t* prefsP)
{	
	UInt16 prefSize = sizeof(AppPreferences_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, prefsP, &prefSize, true) == noPreferenceFound)
	{	
		// default application preference values
		prefsP->bEnabled = PREFS_ENABLED;
		prefsP->timeOffIdx = PREFS_TIME_OFF_IDX;
		prefsP->optionMaskIdx = PREFS_OPTION_MASK_IDX;
		prefsP->keyPressIdx = PREFS_KEYPRESS_IDX;
		prefsP->bDisplayTimeSelection = PREFS_DISPLAY_TIME_SELECTION;
		prefsP->bOffOnAppSwitch = PREFS_OFF_ON_APP_SWITCH;
		prefsP->bBeepConfirmation = PREFS_BEEP_CONFIRMATION;
		prefsP->bKeepOnInCradle = PREFS_KEEP_ON_IN_CRADLE;
		
		writePrefs(prefsP);
	}

} // readPrefs

/*
 * writeUSPrefs
 */
static void writeUSPrefs(USPrefs_t* usPrefsP)
{
	PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, usPrefsP, sizeof(USPrefs_t), false);

} // writeUSPrefs

/*
 * readUSPrefs
 */
static void readUSPrefs(USPrefs_t* usPrefsP)
{	
	UInt16 usPrefSize = sizeof(USPrefs_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, usPrefsP, &usPrefSize, false) == noPreferenceFound)
	{	
		// default application preference values
		usPrefsP->bOffTimeExtended = US_PREFS_OFF_TIME_EXTENDED;
		usPrefsP->offExtendTime = US_PREFS_OFF_EXTEND_TIME;
		usPrefsP->bOffOnAppSwitch = US_PREFS_OFF_ON_APPSWITCH;
		usPrefsP->bInCradle = US_PREFS_IN_CRADLE;
		
		writeUSPrefs(usPrefsP);
	}
	
} // readUSPrefs

/*
 * AppStart
 */
static Err AppStart(void)
{
	FrmGotoForm(MAIN_FORM);
	return errNone;
	
} // AppStart

/*
 * AppStop
 */
static void AppStop(void)
{
	FrmCloseAllForms();	

} // AppStop

/*
 * RegisterForNotifications
 */
static void RegisterForNotifications(AppPreferences_t* prefsP)
{
	UInt16 				cardNo; 
	LocalID  			dbID;
	
	SysCurAppDatabase(&cardNo, &dbID);
		
	if (prefsP->bEnabled)
	{
		SysNotifyRegister(cardNo, dbID, sysNotifyVirtualCharHandlingEvent, NULL, sysNotifyNormalPriority, NULL);
		SysNotifyRegister(cardNo, dbID, sysNotifySleepRequestEvent, NULL, sysNotifyNormalPriority, NULL);
		SysNotifyRegister(cardNo, dbID, sysNotifyAppQuittingEvent, NULL, sysNotifyNormalPriority, NULL);
		
		if (prefsP->bKeepOnInCradle)
		{
			SysNotifyRegister(cardNo, dbID, sysExternalConnectorAttachEvent, NULL, sysNotifyNormalPriority, NULL);
			SysNotifyRegister(cardNo, dbID, sysExternalConnectorDetachEvent, NULL, sysNotifyNormalPriority, NULL);
		}
		else
		{
			SysNotifyUnregister(cardNo, dbID, sysExternalConnectorAttachEvent, sysNotifyNormalPriority);
			SysNotifyUnregister(cardNo, dbID, sysExternalConnectorDetachEvent, sysNotifyNormalPriority); 
		}
	}
	else
	{
		SysNotifyUnregister(cardNo, dbID, sysNotifyVirtualCharHandlingEvent, sysNotifyNormalPriority);
		SysNotifyUnregister(cardNo, dbID, sysNotifySleepRequestEvent, sysNotifyNormalPriority);
		SysNotifyUnregister(cardNo, dbID, sysNotifyAppQuittingEvent, sysNotifyNormalPriority);
		SysNotifyUnregister(cardNo, dbID, sysExternalConnectorAttachEvent, sysNotifyNormalPriority);
		SysNotifyUnregister(cardNo, dbID, sysExternalConnectorDetachEvent, sysNotifyNormalPriority);
	}
				
} // RegisterForNotifications

/*
 * InitializeMainForm
 */
static void InitializeMainForm(FormType* pForm, AppPreferences_t* prefsP)
{	
	ControlType*    pCtl;
	ListType*		pList;
		
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ENABLE_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bEnabled);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_TIME_LIST));
	LstSetSelection(pList, prefsP->timeOffIdx);
	LstSetHeight(pList, 10);
	LstSetTopItem(pList, prefsP->timeOffIdx);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_TIME_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OPTION_MASK_LIST));
	LstSetSelection(pList, prefsP->optionMaskIdx);
	LstSetHeight(pList, 4);
	LstSetTopItem(pList, prefsP->optionMaskIdx);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OPTION_MASK_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEY_PRESS_LIST));
	LstSetSelection(pList, prefsP->keyPressIdx);
	LstSetHeight(pList, 10);
	LstSetTopItem(pList, prefsP->keyPressIdx);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEY_PRESS_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_DISPLAY_TIME_SELECTION_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bDisplayTimeSelection);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OFF_ON_APP_SWITCH_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bOffOnAppSwitch);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_BEEP_CONFIRMATION_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bBeepConfirmation);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEEP_ON_IN_CRADLE_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bKeepOnInCradle);
	
	FrmDrawForm(pForm);
	
} // InitializeMainForm

/*
 * MainFormHandleEvent
 */
static Boolean MainFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType* 				pForm = NULL;
	ControlType*    		pCtl = NULL;
	ListType*				pList = NULL;

	AppPreferences_t*		prefsP = NULL;
	
	EventType 				newEvent;
	
	prefsP = MemPtrNew(sizeof(AppPreferences_t));
	if (!prefsP)
	{
		Alert(ALERT_MEMORY_ERROR, "(AppPreferences_t)", 0);
		return handled;
	}
	MemSet(prefsP, sizeof(AppPreferences_t), 0);

	pForm = FrmGetActiveForm(); // THE CAUSE OF SO MANY CRASHES!!!
	
	switch (pEvent->eType)
	{
		case frmOpenEvent:
		
			readPrefs(prefsP); // have to do this first so that prefsP are read before initialization...
			InitializeMainForm(pForm, prefsP);			
			FrmDrawForm(pForm);
			
			handled = true;
			break;
			
		case frmCloseEvent:
		
			PanelFormClose(pForm);
			
			handled = true;
			
			break;
			
		case ctlSelectEvent:
			switch (pEvent->data.ctlSelect.controlID)
			{	
					
				case MAIN_DONE_BUTTON:
					{									
						readPrefs(prefsP); // What a waste, but not so...
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ENABLE_CHECKBOX));
						prefsP->bEnabled = (CtlGetValue(pCtl) == 1);
						
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_TIME_LIST));
						prefsP->timeOffIdx = LstGetSelection(pList);
						
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OPTION_MASK_LIST));
						prefsP->optionMaskIdx = LstGetSelection(pList);
						
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEY_PRESS_LIST));
						prefsP->keyPressIdx = LstGetSelection(pList);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_DISPLAY_TIME_SELECTION_CHECKBOX));
						prefsP->bDisplayTimeSelection = (CtlGetValue(pCtl) == 1);
		
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OFF_ON_APP_SWITCH_CHECKBOX));
						prefsP->bOffOnAppSwitch = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_BEEP_CONFIRMATION_CHECKBOX));
						prefsP->bBeepConfirmation = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEEP_ON_IN_CRADLE_CHECKBOX));
						prefsP->bKeepOnInCradle = (CtlGetValue(pCtl) == 1);
	
						writePrefs(prefsP);
						
						RegisterForNotifications(prefsP);
						
						newEvent.eType = appStopEvent;
						EvtAddEventToQueue(&newEvent);
						
						handled = true;
					}
					break;
	
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	if (prefsP)
		MemPtrFree(prefsP);
	
	return handled;
	
} // MainFormHandleEvent

/*
 * AppHandleEvent
 */
static Boolean AppHandleEvent(EventType* pEvent)
{
	UInt16 		formID;
	FormType* 	pForm;
	Boolean		handled = false;

	if (pEvent->eType == frmLoadEvent)
	{
		// Load the form resource.
		formID = pEvent->data.frmLoad.formID;
		
		pForm = FrmInitForm(formID);
		FrmSetActiveForm(pForm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		if (formID == MAIN_FORM)
			FrmSetEventHandler(pForm, MainFormHandleEvent);
			
		handled = true;
	}
	
	return handled;
	
} // AppHandleEvent

/*
 * AppEventLoop
 */
static void AppEventLoop(void)
{
	// Err			error;
	EventType	event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (SysHandleEvent(&event))
			continue;
			
		// if (MenuHandleEvent(0, &event, &error))
		//	continue;
			
		if (AppHandleEvent(&event))
			continue; 

		FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);

} // AppEventLoop

/*
 * SaveRect
 */
static Err SaveRect(RectangleType* rectP, WinHandle* winHP, RectangleType* rectObscuredP)
{
	Err						error = errNone;
	
	if ((rectP) && (winHP)) 
	{
		*winHP = NULL; // preset

		WinGetFramesRectangle(dialogFrame, rectP, rectObscuredP);			
		*winHP = WinSaveBits(rectObscuredP, &error);
		
		if (error)
			*winHP = NULL;		
	}
	
	return (error);

} // SaveRect

/*
 * RestoreRect
 */
static void RestoreRect(WinHandle* winHP, RectangleType* rectObscuredP)
{
	if ((*winHP) && (rectObscuredP))
	{	
		WinRestoreBits(*winHP, rectObscuredP->topLeft.x, rectObscuredP->topLeft.y);
		*winHP = NULL;		
	}
	
	return;
	
} // RestoreRect

/*
 * DisplayState
 */
static void DisplayState(Char* str1, Char* str2, UInt16 rectWidth, UInt16 rectHeight)
{
	RectangleType			rectObscured;
	WinHandle				winH = NULL;
	FormActiveStateType		activeState;
//	WinHandle				oldWin;
//	WinHandle				newWin;
	
	UInt16					rectLeft_X = (DISP_WIDTH - rectWidth) / 2;
	UInt16					rectTop_Y = ((DISP_HEIGHT - rectHeight) / 2); //  - 4;
	UInt16					lineWidth = rectWidth - (2 * LINE_GAP);
	
    RectangleType 			rect = {{rectLeft_X, rectTop_Y}, {rectWidth, rectHeight}};
    RGBColorType 			rgb;
    IndexedColorType		colorWhite;
    IndexedColorType		colorBlack;
    IndexedColorType		colorGrey;
	IndexedColorType 		colorRed;
	IndexedColorType		colorGreen;
	
	UInt16					str1Width = 0;
	UInt16					str2Width = 0;
	UInt16					strHeight = 0;
	
	Boolean					onFlag = (StrCompare(str1, APP_NAME) != 0);
	Boolean					cancelledFlag = (StrCompare(str2, STR_CANCELLED) == 0);

	//oldWin = WinGetActiveWindow();
	//newWin = WinGetDisplayWindow();
	
	//WinSetActiveWindow(newWin);
	//WinSetDrawWindow(newWin);

	WinDisplayToWindowPt(&rectLeft_X, &rectTop_Y);

	rect.topLeft.x = rectLeft_X;
	rect.topLeft.y = rectTop_Y;
	
	FrmSaveActiveState(&activeState); 
	
	WinPushDrawState();
		
	SaveRect(&rect, &winH, &rectObscured);
		
	rgb.r=255; rgb.g=255; rgb.b=255; // white
	colorWhite = WinRGBToIndex(&rgb);	
	
	rgb.r=0; rgb.g=0; rgb.b=0; // black
	colorBlack = WinRGBToIndex(&rgb);
	
    rgb.r=150; rgb.g=150; rgb.b=150; // grey?
	colorGrey = WinRGBToIndex(&rgb);
	
	rgb.r=255; rgb.g=0; rgb.b=51; // red?
	colorRed = WinRGBToIndex(&rgb);
		
	rgb.r=0; rgb.g=153; rgb.b=51; // green?
	colorGreen = WinRGBToIndex(&rgb);	
		
	WinSetForeColor(colorGrey);
	WinSetBackColor(colorWhite);
		
	WinEraseRectangleFrame(dialogFrame, &rect); 
    WinEraseRectangle(&rect, 2); // 2nd arg is corner "diameter"!

	WinEraseRectangleFrame(dialogFrame, &rect);
	WinPaintRectangleFrame(dialogFrame, &rect);
	
	FntSetFont(stdFont);
	    
	str1Width = FntLineWidth(str1, StrLen(str1));
	
	if (onFlag)
	{	
		WinSetTextColor(colorRed);
	}
	else
	{
		WinSetTextColor(colorBlack);
	}
	
    WinPaintChars(str1, StrLen(str1),
				rectLeft_X + (rectWidth - str1Width)/2,
				rectTop_Y + TXT_OFF_Y);
			
	if (str2)
	{
		if (onFlag)
		{
			WinSetTextColor(colorBlack);
		}
		else if (cancelledFlag)
		{
			WinSetTextColor(colorRed);
			FntSetFont(boldFont);
		}
		else
		{
			WinSetTextColor(colorGreen);
		}
		
		strHeight = FntLineHeight();
		str2Width = FntLineWidth(str2, StrLen(str2));
		
		WinPaintLine(rectLeft_X + LINE_GAP, rectTop_Y + (rectHeight / 2), rectLeft_X + lineWidth, rectTop_Y + (rectHeight / 2));
		
	    WinPaintChars(str2, StrLen(str2),
				rectLeft_X + (rectWidth - str2Width)/2,
				rectTop_Y + TXT_OFF_Y + strHeight + 4);	
	}
      
    DelayTask(2);
	RestoreRect(&winH, &rectObscured);
	
	WinPopDrawState();
    
	//WinSetActiveWindow(oldWin);
	//WinSetDrawWindow(oldWin);
	
	FrmRestoreActiveState(&activeState);
   
} // DisplayState

/*
 * popupFormHandleEvent
 */
static Boolean popupFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType* 				pForm = FrmGetActiveForm();
	
	switch (pEvent->eType)
	{
		case penDownEvent:
		
			AlmSetProcAlarm(*popupFormCallback, 0, TimGetSeconds() + POPUP_FORM_WAIT_TIME);
			
			break;
			
		case keyDownEvent:
			{
				ListType*	pList;
				UInt16		idx;
				
				AlmSetProcAlarm(*popupFormCallback, 0, TimGetSeconds() + POPUP_FORM_WAIT_TIME);
				
				pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_MUTE_TIME_LIST));
				idx = LstGetSelection(pList);
				
				if (pEvent->data.keyDown.chr == vchrRockerDown)
				{
					if (idx < (LstGetNumberOfItems(pList) - 1))
						++idx;		
					
					handled = true;
				}
				else if (pEvent->data.keyDown.chr == vchrRockerUp)
				{
					if(idx > 0)
						--idx;
					
					handled = true;
				}
				else
				{
					EventType 				newEvent;

					CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_EXIT_STATE_CHECKBOX)),
							((pEvent->data.keyDown.keyCode == vchrRockerCenter)
							|| (pEvent->data.keyDown.keyCode == chrSpace)
							|| (pEvent->data.keyDown.keyCode == chrLineFeed)
							|| (pEvent->data.keyDown.keyCode == chrCarriageReturn)));
							
					newEvent.eType = frmCloseEvent;
					newEvent.data.frmClose.formID = POP_FORM;
					EvtAddEventToQueue(&newEvent);
					
					handled = true;
				}
					
				LstSetSelection(pList, idx);
			}
			
			break;
					
		case frmOpenEvent:
		
			break;
			
		case frmCloseEvent:
			
			AlmSetProcAlarm(*popupFormCallback, 0, 0);
			
			break;
			
		case ctlSelectEvent:
		
			break;
			
		case lstSelectEvent:
			{		
				EventType 				newEvent;
				
				newEvent.eType = frmCloseEvent;
				newEvent.data.frmClose.formID = POP_FORM;
				EvtAddEventToQueue(&newEvent);
			}
			handled = true;
			break;
			
		case frmObjectFocusTakeEvent:
		case frmObjectFocusLostEvent:	
			// handled = (pEvent->data.frmObjectFocusTake.objectID == POP_MUTE_TIME_LIST);
			handled = true;	
			break;
			
		default:
		
			break;
	}			
	return handled;
	
} // popupFormHandleEvent

/*
 * popupFormCallback
 */
static void popupFormCallback(UInt16 almProcCmd, SysAlarmTriggeredParamType *paramP)
{
	if (almProcCmd == almProcCmdTriggered)
	{				
		EventType 				newEvent;
		
		newEvent.eType = frmCloseEvent;
		EvtAddEventToQueue(&newEvent);
	}
		
	EvtWakeup();		
	
	return;
	
} // popupFormCallback

/*
 * popupForm
 */
static Boolean popupForm(UInt16* initIdxP, Boolean* OffOnAppSwitchP)
{
	Boolean				retVal = true;
	
	FormType*			pOldForm = NULL;
	FormType*			pForm = NULL;
	ListType*			pList = NULL;
	ControlType*		pCtl = NULL;
	
	pOldForm = FrmGetActiveForm();
	
	pForm = FrmInitForm(POP_FORM);
	FrmSetEventHandler(pForm, *popupFormHandleEvent);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_MUTE_TIME_LIST));
	LstSetSelection(pList, *initIdxP);
	LstSetTopItem(pList, *initIdxP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_OFF_ON_APP_SWITCH_CHECKBOX));
	CtlSetValue(pCtl, *OffOnAppSwitchP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, 	POP_EXIT_STATE_CHECKBOX));
	CtlSetValue(pCtl, true);
							
	AlmSetProcAlarm(*popupFormCallback, 0, TimGetSeconds() + POPUP_FORM_WAIT_TIME);

	FrmDoDialog(pForm);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_MUTE_TIME_LIST));
	*initIdxP = LstGetSelection(pList);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_OFF_ON_APP_SWITCH_CHECKBOX));
	*OffOnAppSwitchP = (CtlGetValue(pCtl) == 1);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, 	POP_EXIT_STATE_CHECKBOX));
	retVal = (CtlGetValue(pCtl) == 1);

	FrmEraseForm(pForm);
	FrmDeleteForm(pForm);
	
	FrmSetActiveForm(pOldForm);
	
	return (retVal);
	
} // popupForm

/*
 * SetMuteState
 */
static void SetOffState(USPrefs_t* usPrefsP, OffTimeExtendAction_e state, OffType_e offType)
{
	if ((state == offExtend) && (!usPrefsP->bOffTimeExtended))
	{	
		
		usPrefsP->offExtendTime = SysSetAutoOffTime(usPrefsP->offExtendTime);
		
		EvtResetAutoOffTimer();
		
		usPrefsP->bOffTimeExtended = true;
		
		usPrefsP->bInCradle = (offType == typeCradle);	
		
	}
	else if ((state == offNormal) && (usPrefsP->bOffTimeExtended))
	{

		usPrefsP->offExtendTime = SysSetAutoOffTime(
			(usPrefsP->offExtendTime < US_PREFS_OFF_EXTEND_TIME) ? usPrefsP->offExtendTime : US_PREFS_OFF_EXTEND_TIME);
		
		EvtResetAutoOffTimer();
		
		usPrefsP->bOffTimeExtended = false;
		usPrefsP->bInCradle = false;		
	}
			
	writeUSPrefs(usPrefsP);
	
} // SetMuteState

/*
 * getStrDelayTime
 */
static void getStrDelayTime(UInt32 delayTime, Char* str)
{
	DateTimeType		dt;
	
	TimSecondsToDateTime(delayTime, &dt);
	StrPrintF(str, "Awake for [%02uh %02um]", dt.hour, dt.minute);	

} // getStrDelayTime

/*
 * getStrDateTime
 */
static void getStrDateTime(UInt32 TimeSecs, Char* dtStr)
{
	DateTimeType 		dtNow;
	Char				dateStr[dateStringLength];
	Char				timeStr[timeStringLength];
	
	TimSecondsToDateTime(TimeSecs, &dtNow);
	
	DateToAscii(dtNow.month, dtNow.day, dtNow.year, PrefGetPreference(prefDateFormat), dateStr);
	TimeToAscii(dtNow.hour, dtNow.minute, PrefGetPreference(prefTimeFormat), timeStr);
	
	MemMove(dtStr, STR_ON_AT, StrLen(STR_ON_AT));
	MemMove(dtStr + StrLen(STR_ON_AT), timeStr, StrLen(timeStr));
	dtStr[StrLen(STR_ON_AT) + StrLen(timeStr)] = ' ';
	dtStr[StrLen(STR_ON_AT) + StrLen(timeStr) + 1] = ' ';
	MemMove(dtStr + StrLen(STR_ON_AT) +  StrLen(timeStr) + 2, dateStr, StrLen(dateStr));
	dtStr[StrLen(STR_ON_AT) + StrLen(dateStr) + StrLen(timeStr) + 2] = chrNull;
		
} // getStrDateTime

/*
 * PilotMain
 */
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err 						error = errNone;

	UInt16		offTime[11] = { 300, 600, 900, 1800, 2700, 3600, 5400, 7200, 10800, 14400, 21600 }; 
	UInt16		keyPressOptionMask[4] = { 0, shiftKeyMask, optionKeyMask, doubleTapKeyMask };
	UInt16 		keyPress[10] = { 0, vchrRockerUp, vchrRockerDown, vchrRockerLeft, vchrRockerRight, vchrRockerCenter,
								vchrLaunch, vchrMenu, vchrHard2, vchrHard3 };

	AppPreferences_t* 			prefsP = NULL;
	USPrefs_t*					usPrefsP = NULL;
	
	prefsP = MemPtrNew(sizeof(AppPreferences_t));
	if (!prefsP)
	{
		goto EXIT_MAIN;
	}
	
	usPrefsP = MemPtrNew(sizeof(USPrefs_t));
	if (!usPrefsP)
	{
		goto EXIT_MAIN;
	}
	
	MemSet(prefsP, sizeof(AppPreferences_t), 0);
  	readPrefs(prefsP);
  	
  	MemSet(usPrefsP, sizeof(USPrefs_t), 0);
	readUSPrefs(usPrefsP);

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
	    case sysAppLaunchCmdPanelCalledFromApp:
	    case sysAppLaunchCmdReturnFromPanel:

			// Check device
			if ((error = RomVersionCompatible(MIN_VERSION, launchFlags)))
			{
				Alert(ALERT_PHONE_ERROR, "Incompatible Device", error);
				
				goto EXIT_MAIN;
			} 
	
			if ((error = AppStart()) == 0)
			{
				AppEventLoop();
				AppStop();
			}
			break;
			
		// Register for notifications on reset
		case sysAppLaunchCmdSystemReset:
			// Check device
			if (!(error = RomVersionCompatible(MIN_VERSION, launchFlags)))
			{
				RegisterForNotifications(prefsP);
				
				if (prefsP->bEnabled)
					SetOffState(usPrefsP, offNormal, typeManual);
			}		
			break;
			
		case sysAppLaunchCmdNotify:
			if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyVirtualCharHandlingEvent)
			{
				if (prefsP->keyPressIdx)
				{
					UInt16 						cardNo; 
					LocalID  					dbID;
					
					SysNotifyParamType* 		notifyParam = (SysNotifyParamType *)cmdPBP;
				
					UInt16						keyMask = ((SysNotifyVirtualCharHandlingType *)notifyParam->notifyDetailsP)->keyDown.modifiers;
					UInt16						keyCode = ((SysNotifyVirtualCharHandlingType *)notifyParam->notifyDetailsP)->keyDown.keyCode;
					
					if ((keyMask & keyPressOptionMask[prefsP->optionMaskIdx]) //  That's "&" NOT "&&"
							|| (prefsP->optionMaskIdx == 0))
					{
						if (keyCode == keyPress[prefsP->keyPressIdx])
						{
							if (!(usPrefsP->bOffTimeExtended))
							{
								UInt32			onTime = 0;
								Boolean			bDoWink = true;
								
								usPrefsP->bOffOnAppSwitch = prefsP->bOffOnAppSwitch;
								
								if (prefsP->bDisplayTimeSelection)
								{
									if (prefsP->bBeepConfirmation)
										beep(1);
										
									bDoWink = popupForm(&(prefsP->timeOffIdx), (Boolean*)(&(usPrefsP->bOffOnAppSwitch)));
								}
										
								if (prefsP->bBeepConfirmation)
									beep(1);
							
								if (bDoWink)
								{	
									usPrefsP->offExtendTime = offTime[prefsP->timeOffIdx];
									writeUSPrefs(usPrefsP);
									
									onTime = TimGetSeconds() + usPrefsP->offExtendTime;								
									
									{
										Char		timeStr1[36];
										Char		timeStr2[StrLen(STR_ON_AT) + dateStringLength + 1 + timeStringLength];
										
										getStrDelayTime(usPrefsP->offExtendTime, timeStr1);
										getStrDateTime(onTime, timeStr2);
										DisplayState(timeStr1, timeStr2, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
									}
										
									SetOffState(usPrefsP, offExtend, typeManual);
									
				 					if (!SysCurAppDatabase(&cardNo, &dbID))
									{						
					 					AlmSetAlarm(cardNo, dbID, 0, onTime, true);
					 				}
								}
								else
								{
									DisplayState(APP_NAME, STR_CANCELLED, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
								}
							}
							else if (usPrefsP->bOffTimeExtended)
							{
								if (!SysCurAppDatabase(&cardNo, &dbID))						
				 					AlmSetAlarm(cardNo, dbID, 0, 0, true);
				 					
								SetOffState(usPrefsP, offNormal, typeManual);
								
								if (prefsP->bBeepConfirmation)
									beep(2);

								DisplayState(APP_NAME, STR_OFF, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
			 				}
			 				((SysNotifyParamType*)cmdPBP)->handled = true;
						}
					}
				}
			}
			else if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyAppQuittingEvent)
			{
				if ((usPrefsP->bOffOnAppSwitch) 
						&& (usPrefsP->bOffTimeExtended)
						&& !(usPrefsP->bInCradle))
				{
					UInt16 						cardNo; 
					LocalID  					dbID;
				
					if (!SysCurAppDatabase(&cardNo, &dbID))						
	 					AlmSetAlarm(cardNo, dbID, 0, 0, true);
	 					
					SetOffState(usPrefsP, offNormal, typeManual);
					
					if (prefsP->bBeepConfirmation)
						beep(2);
						
					DisplayState(APP_NAME, STR_OFF, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
					
				}
				((SysNotifyParamType*)cmdPBP)->handled = true;
			}
			else if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifySleepRequestEvent)
			{
				if (usPrefsP->bOffTimeExtended)
				{
					UInt16 						cardNo; 
					LocalID  					dbID;
				
					if (!SysCurAppDatabase(&cardNo, &dbID))						
	 					AlmSetAlarm(cardNo, dbID, 0, 0, true);
	 					
					SetOffState(usPrefsP, offNormal, typeManual);
					DisplayState(APP_NAME, STR_OFF, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
				}
				((SysNotifyParamType*)cmdPBP)->handled = true;
			}
			else if (((SysNotifyParamType*)cmdPBP)->notifyType == sysExternalConnectorAttachEvent)
			{
				if (!(usPrefsP->bOffTimeExtended))
				{
					SetOffState(usPrefsP, offExtend, typeCradle);
					DisplayState(STR_IN_CRADLE, APP_NAME, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
				}
				
				((SysNotifyParamType*)cmdPBP)->handled = true;
			}
			else if (((SysNotifyParamType*)cmdPBP)->notifyType == sysExternalConnectorDetachEvent)
			{
				if ((usPrefsP->bOffTimeExtended) && (usPrefsP->bInCradle))
				{
					SetOffState(usPrefsP, offNormal, typeManual);
					DisplayState(APP_NAME, STR_OFF, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
				}
				((SysNotifyParamType*)cmdPBP)->handled = true;				
			}
			break;
		
		case sysAppLaunchCmdAlarmTriggered:
		
			((SysAlarmTriggeredParamType*)cmdPBP)->purgeAlarm = false;
			
			break;
			
		case sysAppLaunchCmdDisplayAlarm:
		
			if (!((SysDisplayAlarmParamType*)cmdPBP)->ref)
			{
				((SysDisplayAlarmParamType*)cmdPBP)->ref += 1;
		
				if (usPrefsP->bOffTimeExtended)
				{
					SetOffState(usPrefsP, offNormal, typeManual);
					
					if (prefsP->bBeepConfirmation)
						beep(2);
						
					DisplayState(APP_NAME, STR_OFF, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);			
				}
			}	
			break;
			
		default:
			break;
	}

EXIT_MAIN:

	if (usPrefsP)
		MemPtrFree(usPrefsP);
		
	if (prefsP)
		MemPtrFree(prefsP);

	return error;
	
} // PilotMain

/*
 * AppMain.c
 */

