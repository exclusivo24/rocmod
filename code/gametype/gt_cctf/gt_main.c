// Copyright (C) 2001-2002 Raven Software.
//

#include "gt_local.h"

#define	ITEM_REDFLAG			100
#define ITEM_BLUEFLAG			101

#define TRIGGER_REDCAPTURE		200
#define TRIGGER_BLUECAPTURE		201

void	GT_Init		( void );
void	GT_RunFrame	( int time );
int		GT_Event	( int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4 );

gametypeLocals_t	gametype;

typedef struct 
{
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	float		mMinValue, mMaxValue;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
	qboolean	teamShader;			// track and if changed, update shader state

} cvarTable_t;

vmCvar_t	gt_simpleScoring;
vmCvar_t	gt_customBlueName;
vmCvar_t	gt_customBlueCode;
vmCvar_t	gt_customRedName;
vmCvar_t	gt_customRedCode;
vmCvar_t	gt_allowCustomTeams;

static cvarTable_t gametypeCvarTable[] = 
{
	// don't override the cheat state set by the system
	{ &gt_simpleScoring,	"gt_simpleScoring",		"0",  CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &gt_customBlueName,	"g_customBlueName",		"Blue",  CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ &gt_customBlueCode,	"g_customBlueCode",		"4",  CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ &gt_customRedName,	"g_customRedName",		"Red",  CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ &gt_customRedCode,	"g_customRedCode",		"1",  CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ &gt_allowCustomTeams,	"g_allowCustomTeams",	"0",  CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ NULL, NULL, NULL, 0, 0.0f, 0.0f, 0, qfalse },
};

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 ) 
{
	switch ( command ) 
	{
		case GAMETYPE_INIT:
			GT_Init ( );
			return 0;

		case GAMETYPE_START:
			trap_Cmd_SetHUDIcon ( 0, gametype.iconRedFlag );
			trap_Cmd_SetHUDIcon ( 1, gametype.iconBlueFlag );
			return 0;

		case GAMETYPE_RUN_FRAME:
			GT_RunFrame ( arg0 );
			return 0;

		case GAMETYPE_EVENT:
			return GT_Event ( arg0, arg1, arg2, arg3, arg4, arg5, arg6 );
	}

	return -1;
}

/*
=================
GT_RegisterCvars
=================
*/
void GT_RegisterCvars( void ) 
{
	cvarTable_t	*cv;

	for ( cv = gametypeCvarTable ; cv->cvarName != NULL; cv++ ) 
	{
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue );
		
		if ( cv->vmCvar )
		{
			cv->modificationCount = cv->vmCvar->modificationCount;
		}
	}
}

/*
=================
GT_UpdateCvars
=================
*/
void GT_UpdateCvars( void ) 
{
	cvarTable_t	*cv;

	for ( cv = gametypeCvarTable ; cv->cvarName != NULL; cv++ ) 
	{
		if ( cv->vmCvar ) 
		{
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) 
			{
				cv->modificationCount = cv->vmCvar->modificationCount;
			}
		}
	}
}

/*
================
GT_Init

initializes the gametype by spawning the gametype items and 
preparing them
================
*/
void GT_Init ( void )
{
	gtItemDef_t		itemDef;
	gtTriggerDef_t	triggerDef;

	memset ( &gametype, 0, sizeof(gametype) );

	// Register the global sounds
	gametype.flagReturnSound  = trap_Cmd_RegisterSound ( "sound/ctf_return.mp3" );
	gametype.flagTakenSound   = trap_Cmd_RegisterSound ( "sound/ctf_flag.mp3" );
	gametype.flagCaptureSound = trap_Cmd_RegisterSound ( "sound/ctf_win.mp3" );

	gametype.iconRedFlag = trap_Cmd_RegisterIcon ( "gfx/menus/hud/ctf_red" );
	gametype.iconBlueFlag = trap_Cmd_RegisterIcon ( "gfx/menus/hud/ctf_blue" );

	gametype.iconRedFlagDropped = trap_Cmd_RegisterIcon ( "gfx/menus/hud/ctf_red_dropped" );
	gametype.iconBlueFlagDropped = trap_Cmd_RegisterIcon ( "gfx/menus/hud/ctf_blue_dropped" );

	gametype.iconRedFlagCarried = trap_Cmd_RegisterIcon ( "gfx/menus/hud/ctf_red_carried" );
	gametype.iconBlueFlagCarried = trap_Cmd_RegisterIcon ( "gfx/menus/hud/ctf_blue_carried" );

	// Register all cvars for this gametype
	GT_RegisterCvars ( );

	// Register the items
	memset ( &itemDef, 0, sizeof(itemDef) );
	trap_Cmd_RegisterItem ( ITEM_REDFLAG,  "red_flag", &itemDef );
	trap_Cmd_RegisterItem ( ITEM_BLUEFLAG, "blue_flag", &itemDef );

	// Register the triggers
	memset ( &triggerDef, 0, sizeof(triggerDef) );
	trap_Cmd_RegisterTrigger ( TRIGGER_REDCAPTURE, "red_capture_point", &triggerDef );
	trap_Cmd_RegisterTrigger ( TRIGGER_BLUECAPTURE, "blue_capture_point", &triggerDef );
}

/*
================
GT_RunFrame

Runs all thinking code for gametype
================
*/
void GT_RunFrame ( int time )
{
	gametype.time = time;

	GT_UpdateCvars ( );
}

/*
================
GT_Event

Handles all events sent to the gametype
================
*/
int GT_Event ( int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4 )
{
	char *redCode;
	char *redName;
	char *blueCode;
	char *blueName;

	if ( gt_allowCustomTeams.integer )
	{
		redCode = gt_customRedCode.string;
		redName = gt_customRedName.string;
		blueCode = gt_customBlueCode.string;
		blueName = gt_customBlueName.string;
	}
	else
	{
		redCode = "1";
		redName = "Red";
		blueCode = "4";
		blueName = "Blue";
	}

	switch ( cmd )
	{
		case GTEV_ITEM_DEFEND:
			if ( !gt_simpleScoring.integer )
			{
				trap_Cmd_AddClientScore ( arg1, 5 );
			}
			return 0;

		case GTEV_ITEM_STUCK:
			switch ( arg0 )
			{
				case ITEM_REDFLAG:
					trap_Cmd_ResetItem ( ITEM_REDFLAG );
					trap_Cmd_TextMessage ( -1, va("The ^%s%s^7 Flag has returned!", redCode, redName) );
					trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
					gametype.redFlagDropTime = 0;
					gametype.redFlagTaken = 0;
					trap_Cmd_SetHUDIcon ( 0, gametype.iconRedFlag );
					return 1;

				case ITEM_BLUEFLAG:
					trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
					trap_Cmd_TextMessage ( -1, va("The ^%s%s^7 Flag has returned!", blueCode, blueName) );
					trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
					gametype.blueFlagDropTime = 0;
					gametype.blueFlagTaken = 0;
					trap_Cmd_SetHUDIcon ( 1, gametype.iconBlueFlag );
					return 1;
			}
			
			break;

		case GTEV_ITEM_DROPPED:
		{
			char clientname[MAX_QPATH];

			trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );

			switch ( arg0 )
			{
				case ITEM_BLUEFLAG:
					trap_Cmd_TextMessage ( -1, va("^%s%s^7 has dropped the ^%s%s^7 Flag!", redCode, clientname, blueCode, blueName ) );
					trap_Cmd_SetHUDIcon ( 1, gametype.iconBlueFlagDropped );
					gametype.blueFlagDropTime = time;
					gametype.blueFlagTaken = 1;
					break;
				
				case ITEM_REDFLAG:
					trap_Cmd_TextMessage ( -1, va("^%s%s^7 has dropped the ^%s%s^7 Flag!", blueCode, clientname, redCode, redName ) );
					trap_Cmd_SetHUDIcon ( 0, gametype.iconRedFlagDropped );
					gametype.redFlagDropTime = time;
					gametype.redFlagTaken = 1;
					break;
			}
			break;
		}

		case GTEV_ITEM_TOUCHED:

			switch ( arg0 )
			{
				case ITEM_BLUEFLAG:
					if ( arg2 == TEAM_RED )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cmd_TextMessage ( -1, va("^%s%s^7 has taken the ^%s%s^7 Flag!", redCode, clientname, blueCode, blueName ) );
						trap_Cmd_StartGlobalSound ( gametype.flagTakenSound );
						trap_Cmd_RadioMessage ( arg1, "got_it" );
						trap_Cmd_SetHUDIcon ( 1, gametype.iconBlueFlagCarried );
						gametype.blueFlagDropTime = 0;
						gametype.blueFlagTaken = 1;

						return 1;
					}
					else if ( arg2 == TEAM_BLUE && gametype.blueFlagTaken )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
						trap_Cmd_TextMessage ( -1, va("^%s%s^7 has returned the ^%s%s^7 Flag!", blueCode, clientname, blueCode, blueName ) );
						trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
						gametype.blueFlagDropTime = 0;
						gametype.blueFlagTaken = 0;
						trap_Cmd_SetHUDIcon ( 1, gametype.iconBlueFlag );
					}
					break;

				case ITEM_REDFLAG:
					if ( arg2 == TEAM_BLUE )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cmd_TextMessage ( -1, va("^%s%s^7 has taken the ^%s%s^7 Flag!", blueCode, clientname, redCode, redName ) );
						trap_Cmd_StartGlobalSound ( gametype.flagTakenSound );
						trap_Cmd_RadioMessage ( arg1, "got_it" );
						trap_Cmd_SetHUDIcon ( 0, gametype.iconRedFlagCarried );
						gametype.redFlagDropTime = 0;
						gametype.redFlagTaken = 1;

						return 1;
					}
					else if ( arg2 == TEAM_RED && gametype.redFlagTaken )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cmd_ResetItem ( ITEM_REDFLAG );
						trap_Cmd_TextMessage ( -1, va("^%s%s^7 has returned the ^%s%s^7 Flag!", redCode, clientname, redCode, redName ) );
						trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
						gametype.redFlagDropTime = 0;
						gametype.redFlagTaken = 0;
						trap_Cmd_SetHUDIcon ( 0, gametype.iconRedFlag );
					}
					break;
			}

			return 0;

		case GTEV_TRIGGER_TOUCHED:
			switch ( arg0 )
			{
				case TRIGGER_BLUECAPTURE:
					if ( trap_Cmd_DoesClientHaveItem ( arg1, ITEM_REDFLAG ) )
					{
						if ( gametype.blueFlagTaken )
						{
//							trap_Cmd_TextMessage ( arg1, "^7Your own flag must be returned first!" );
						}
						else
						{
							char clientname[MAX_QPATH];
							trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
							trap_Cmd_TextMessage ( -1, va("^%s%s^7 has captured the ^%s%s^7 Flag!", blueCode, clientname, redCode, redName ) );
							trap_Cmd_ResetItem ( ITEM_REDFLAG );
							trap_Cmd_StartGlobalSound ( gametype.flagCaptureSound );
							trap_Cmd_AddTeamScore ( arg2, 1 );
							trap_Cmd_SetHUDIcon ( 0, gametype.iconRedFlag );

							if ( !gt_simpleScoring.integer )
							{
								trap_Cmd_AddClientScore ( arg1, 10 );
							}
							else
							{
								// gotta send something with their name for recording
								trap_Cmd_AddClientScore ( arg1, 0 );
							}
							gametype.redFlagDropTime = 0;
							gametype.redFlagTaken = 0;
							return 1;
						}
					}
					break;

				case TRIGGER_REDCAPTURE:
					if ( trap_Cmd_DoesClientHaveItem ( arg1, ITEM_BLUEFLAG ) )
					{
						if ( gametype.redFlagTaken )
						{
//							trap_Cmd_TextMessage ( arg1, "^7Your own flag must be returned first!" );
						}
						else
						{
							char clientname[MAX_QPATH];
							trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
							trap_Cmd_TextMessage ( -1, va("^%s%s^7 has captured the ^%s%s^7 Flag!", redCode, clientname, blueCode, blueName ) );
							trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
							trap_Cmd_StartGlobalSound ( gametype.flagCaptureSound );
							trap_Cmd_AddTeamScore ( arg2, 1 );
							trap_Cmd_SetHUDIcon ( 1, gametype.iconBlueFlag );

							if ( !gt_simpleScoring.integer )
							{
								trap_Cmd_AddClientScore ( arg1, 10 );
							}
							else
							{
								// gotta send something with their name for recording
								trap_Cmd_AddClientScore ( arg1, 0 );
							}
							gametype.blueFlagDropTime = 0;
							gametype.blueFlagTaken = 0;
							return 1;
						}
					}
					break;
			}

			return 0;

		case GTEV_TIME_ADJUST:
			if ( gametype.blueFlagDropTime )
			{
				gametype.blueFlagDropTime += arg0;
			}
			if ( gametype.redFlagDropTime )
			{
				gametype.redFlagDropTime += arg0;
			}
			break;
	}

	return 0;
}

#ifndef GAMETYPE_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vnsprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vnsprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

#endif
