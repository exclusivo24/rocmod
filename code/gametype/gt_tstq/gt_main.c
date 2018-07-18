// Copyright (C) 2001-2002 Raven Software.
//

#include "gt_local.h"

#define	ITEM_BRIEFCASE			100

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

	memset ( &gametype, 0, sizeof(gametype) );

	// Register all cvars for this gametype
	GT_RegisterCvars ( );

	// Register the global sounds
	gametype.caseTakenSound   = trap_Cmd_RegisterSound ( "sound/ctf_flag.mp3" );
	gametype.caseCarrySound = trap_Cmd_RegisterSound ( "sound/movers/buttons/button02.mp3" );
	gametype.caseReturnSound  = trap_Cmd_RegisterSound ( "sound/ctf_return.mp3" );

	// Register the items
	memset ( &itemDef, 0, sizeof(itemDef) );
	trap_Cmd_RegisterItem ( ITEM_BRIEFCASE,  "briefcase", &itemDef );
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

	if ( gametype.carrierTime && time - gametype.carrierTime > 5000 )
	{
		trap_Cmd_GetClientOrigin ( gametype.carrier, gametype.carrierOrigin );
		trap_Cmd_StartSound ( gametype.caseCarrySound, gametype.carrierOrigin );
		trap_Cmd_AddClientScore ( gametype.carrier, 1 );
		trap_Cmd_AddTeamScore ( gametype.carrierTeam, 1 );
		gametype.carrierTime = time;
	}

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
		case GTEV_ITEM_STUCK:
			trap_Cmd_ResetItem ( ITEM_BRIEFCASE );
			trap_Cmd_TextMessage ( -1, "The Briefcase has returned!" );
			trap_Cmd_StartGlobalSound ( gametype.caseReturnSound );
			return 1;

		case GTEV_ITEM_DROPPED:
		{
			char clientname[MAX_QPATH];
			trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
			trap_Cmd_TextMessage ( -1, va("^%s%s^7 has dropped the briefcase!", blueCode, clientname ) );
			gametype.carrierTime = 0;
			break;
		}

		case GTEV_ITEM_TOUCHED:

			switch ( arg0 )
			{
				case ITEM_BRIEFCASE:
				{
					char clientname[MAX_QPATH];
					trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
					trap_Cmd_TextMessage ( -1, va("^%s%s^7 has taken the briefcase!", blueCode, clientname ) );
					trap_Cmd_StartGlobalSound ( gametype.caseTakenSound );
					trap_Cmd_RadioMessage ( arg1, "got_it" );
					gametype.carrierTime = time;
					gametype.carrierTeam = arg2;
					gametype.carrier = arg1;

					return 1;
				}
			}

			return 0;

		case GTEV_TIME_EXPIRED:
			gametype.carrierTime = 0;
			break;

		case GTEV_TIME_ADJUST:
			if ( gametype.carrierTime )
			{
				gametype.carrierTime += arg0;
			}
			break;

		case GTEV_ENDGAME:
			gametype.carrierTime = 0;
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
