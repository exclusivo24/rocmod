// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"

/*
=======================================================================
  SESSION DATA
Session data is the only data that stays persistant across level loads
and map restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) 
{
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i", client->sess.team, client->sess.modData->adminref, client->sess.modData->moderator, client->sess.modData->motdShown, client->sess.modData->versionVerified, client->sess.modData->currentScan );

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) 
{
	char		s[MAX_STRING_CHARS];
	const char	*var;
	int			sessionTeam;
	int			adminref;
	qboolean	moderator;
	qboolean	motdShown;
	qboolean	versionVerified;
	int			currentScan;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i", &sessionTeam, &adminref, &moderator, &motdShown, &versionVerified, &currentScan );

	if ( level.gametypeData->teams && sessionTeam != TEAM_FREE )
	{
		// bk001205 - format issues
		client->sess.team = (team_t)sessionTeam;
	}
	else if ( !level.gametypeData->teams && sessionTeam != TEAM_RED && sessionTeam != TEAM_BLUE )
	{
		// bk001205 - format issues
		client->sess.team = (team_t)sessionTeam;
	}
	client->sess.modData->adminref = adminref;
	client->sess.modData->moderator = moderator;
	client->sess.modData->motdShown = motdShown;
	client->sess.modData->versionVerified = versionVerified;
	client->sess.modData->currentScan = currentScan;
	client->sess.modData->startingScan = currentScan;

	if ( client->sess.modData->adminref > 1 && (g_suspendedAdmin.integer || level.match) )
	{
		client->sess.modData->adminref = -1;
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo, qboolean firstTime ) 
{
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	if ( level.gametypeData->teams ) 
	{
		if ( !level.paused && g_teamAutoJoin.integer && firstTime ) 
		{
 			sess->team = PickTeam( -1 );
		} 
		else 
		{
			// always spawn as spectator in team games
			sess->team = TEAM_SPECTATOR;	
		}
	} 
	else 
	{
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) 
		{
			// a willing spectator, not a waiting-in-line
			sess->team = TEAM_SPECTATOR;
		} 
		else 
		{
			if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer ) 
			{
				sess->team = TEAM_SPECTATOR;
			} 
			else 
			{
				sess->team = TEAM_FREE;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	sess->modData->adminref = 0;
	sess->modData->motdShown = qfalse;
	sess->modData->teamkillPenaltyDone = qfalse;
	sess->modData->spawnkillPenaltyDone = qfalse;
	sess->modData->versionVerified = qtrue;
	sess->modData->versionWarnTime = 0;
	sess->modData->versionChecks = 0;
	sess->modData->versionWarnCount = 0;
	sess->modData->extraFeatures = 0;
	sess->modData->currentScan = 0;
	sess->modData->scanRepeat = 0;

	if ( firstTime )
	{
		G_WriteClientSessionData( client );
	}
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) 
{
	char	s[MAX_STRING_CHARS];
	int		gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	
	gt = BG_FindGametype ( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( level.gametype != gt ) 
	{
		level.newSession = qtrue;
		Com_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData
==================
*/
void G_WriteSessionData( void ) 
{
	int		i;

	trap_Cvar_Set( "session", level.gametypeData->name );

	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		if ( level.clients[i].pers.connected == CON_CONNECTED ) 
		{
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
