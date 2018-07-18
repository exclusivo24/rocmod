// Copyright (C) 2003 Caleb Gingles
//
#include "g_local.h"

// g_admin.c -- ROCmod admin functions

#define GUID_BUFFER_SIZE	16000

void GetGUID( int clientNum, char *guid, int guidlen, char *name, int namelen )
{
	char	*value;
	char	userinfo[MAX_INFO_STRING];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	value = Info_ValueForKey (userinfo, "cl_guid");
	Com_sprintf ( guid, guidlen, value );

	if ( name ){
		value = Info_ValueForKey (userinfo, "name");
		Com_sprintf ( name, namelen, value );
		Q_CleanStr( name );
	}
}

void LoadAdminFile( char *cvar, char *buffer, int bufflen )
{
	char			filename[256];
	fileHandle_t	f;
	int				len;

	trap_Cvar_VariableStringBuffer( cvar, filename, sizeof( filename ) );
	len = trap_FS_FOpenFile( filename, &f, FS_READ_TEXT );
	if ( !f ) {
		buffer[ 0 ] = 0;
		return;
	}
	if ( len >= bufflen ) {
		len = bufflen;
	}
	trap_FS_Read( buffer, len, f);
	trap_FS_FCloseFile( f );
	buffer[ len ] = 0;
}

/*
===========
CheckAdmin

called just before a player enters the level, to
determine if they are an active admin on this server
============
*/
int CheckAdmin( int clientNum )
{
	char			guid[64];
	char			buffer[GUID_BUFFER_SIZE];

	GetGUID( clientNum, guid, sizeof( guid ), NULL, 0 );

	if ( strlen(guid) < 32 )
	{
		return 0;
	}

	LoadAdminFile( "sv_sysopfile", buffer, sizeof( buffer ) );

	if ( strstr( buffer, guid ) ) {
		return 3;
	}

	LoadAdminFile( "sv_adminfile", buffer, sizeof( buffer ) );

	if ( strstr( buffer, guid ) && g_enableAdmin.integer ) {
		return 2;
	}

	LoadAdminFile( "sv_refereefile", buffer, sizeof( buffer ) );

	if ( strstr( buffer, guid ) && g_enableReferee.integer ) {
		return 1;
	}

	return 0;
}

/*
===========
CheckClan
============
*/
qboolean CheckClan( int clientNum )
{
	char			guid[64];
	char			buffer[GUID_BUFFER_SIZE];

	GetGUID( clientNum, guid, sizeof( guid ), NULL, 0 );

	if ( strlen(guid) < 32 )
	{
		return qfalse;
	}

	LoadAdminFile( "sv_clanfile", buffer, sizeof( buffer ) );

	if ( strstr( buffer, guid ) ) {
		return qtrue;
	}

	return qfalse;
}

/*
===========
AddAdmin

adds a player GUID to the admin file
============
*/
qboolean AddAdmin( int clientNum, char *cvar )
{
	char			guid[65];
	char			name[256];
	char			adminfile[256];
	fileHandle_t	f;
	int				len;

	GetGUID( clientNum, guid, sizeof( guid ), name, sizeof( name ) );
	len = strlen( guid );

	trap_Cvar_VariableStringBuffer( cvar, adminfile, sizeof( adminfile ) );
	trap_FS_FOpenFile( adminfile, &f, FS_APPEND_TEXT );
	if ( !f ) {
		return qfalse;
	}

	len = strlen( guid );
	guid[len++] = ' ';
	guid[len] = 0;
	trap_FS_Write( guid, len, f);
	len = strlen( name );
	name[len++] = '\n';
	name[len] = 0;
	trap_FS_Write( name, len, f);
	trap_FS_FCloseFile( f );

	return qtrue;
}

/*
===========
RemoveAdmin

removes a player GUID from the admin file
============
*/
qboolean RemoveAdmin( int clientNum, char *cvar )
{
	char			guid[64];
	char			buffer[GUID_BUFFER_SIZE];
	char			adminfile[256];
	fileHandle_t	f;
	int				len;
	char			*dest, *src;

	GetGUID( clientNum, guid, sizeof( guid ), NULL, 0 );

	trap_Cvar_VariableStringBuffer( cvar, adminfile, sizeof( adminfile ) );
	len = trap_FS_FOpenFile( adminfile, &f, FS_READ_TEXT );
	if ( !f ) {
		return qfalse;
	}
	if ( len >= sizeof( buffer ) ) {
		len = sizeof( buffer );
	}
	trap_FS_Read( buffer, len, f);
	trap_FS_FCloseFile( f );
	buffer[ len ] = 0;

	if ( ( dest = strstr( buffer, guid ) ) == NULL ) {
		return qtrue;		// not an error if its not found
	}
	src = dest;
	SkipRestOfLine( (const char **) &src );
	strcpy( dest, src );

	len = strlen( buffer );

	trap_FS_FOpenFile( adminfile, &f, FS_WRITE_TEXT );
	if ( !f ) {
		return qfalse;
	}
	trap_FS_Write( buffer, len, f);
	trap_FS_FCloseFile( f );

	return qtrue;
}

qboolean CheckCommand( vmCvar_t *cvar, gentity_t *ent, int type, qboolean mutesafe, qboolean matchsafe, qboolean moderatorsafe )
{
	if ( !ent )
	{
		return qtrue;
	}

	if ( type == 4 && moderatorsafe && ent->client->sess.modData->moderator && level.match ) {
		return qtrue;
	}

	if ( type == 4 ) {
		G_PrintMessage( ent, "^1>>^7 This command is only available to match moderators during a match.\n");
		return qfalse;
	}

	if ( cvar->string[type] == '0' && ent->client->sess.modData->adminref != 3 ) {
		G_PrintMessage( ent, "^1>>^7 This command has been disabled.\n");
		return qfalse;
	}

	if ( ent->client->sess.modData->penalty && (ent->client->sess.modData->adminref < 2 || g_disableAdminInPenaltyBox.integer) ) {
		G_PrintMessage( ent, "^1>>^7 This command is disabled while you are in the penalty box!\n");
		return qfalse;
	}

	if ( ent->client->sess.modData->adminref == -1 && cvar->string[type] != '2' ) {
		G_PrintMessage( ent, "^1>>^7 This command is not available while suspended.\n");
		return qfalse;
	}

	if ( ent->client->sess.modData->adminref < 3 && G_IsSysopPresent() && cvar->string[0] == '1' )
	{
		G_PrintMessage( ent, "^1>>^7 This command has been disabled while a sysop is present.\n");
		return qfalse;
	}

	if ( ent->client->sess.modData->adminref < 2 && G_IsAdminPresent() && cvar->string[0] == '1' )
	{
		G_PrintMessage( ent, "^1>>^7 This command has been disabled while an admin is present.\n");
		return qfalse;
	}

	if ( ent->client->sess.modData->adminref < 3 && !G_IsSysopPresent() && cvar->string[0] == '2' )
	{
		G_PrintMessage( ent, "^1>>^7 This command is disabled while a sysop is not present.\n");
		return qfalse;
	}

	if ( ent->client->sess.modData->adminref < 2 && !G_IsAdminPresent() && cvar->string[0] == '2' )
	{
		G_PrintMessage( ent, "^1>>^7 This command is disabled while an admin is not present.\n");
		return qfalse;
	}

	if ( !mutesafe && ent->client->sess.muted ) {
		G_PrintMessage( ent, "^1>>^7 Sorry, no talking while you are muted!\n");
		return qfalse;
	}

	if ( !matchsafe && level.match ) {
		G_PrintMessage( ent, "^1>>^7 This command is disabled during a match.\n");
		return qfalse;
	}

	return qtrue;
}

void PrintCommand( int type, char *text )
{
	switch (type)
	{
		case 1:
			trap_SendServerCommand( -1, va("cp \"^5** Admin Action **\n%s\n\"", text ) );
			break;
		case 2:
			trap_SendServerCommand( -1, va("cp \"^3* Referee Action *\n%s\n\"", text ) );
			break;
		case 3:
			trap_SendServerCommand( -1, va("cp \"^3* Clan Action *\n%s\n\"", text ) );
			break;
		default:
			return;
	}
	G_BroadcastSound("sound/misc/events/tut_lift02.mp3");
}

int ParseID( gentity_t *ent, const char *buffer )
{
	gentity_t	*nearby;

	if ( buffer[0] == '#' )
	{
		switch ( buffer[1] )
		{
			case 'z':
			case 'Z':
				return level.lastPlayer;
			case 'x':
				return 9999;	// hack id
			case 'X':
				return 9998;	// hack id
		}
		if ( ent )
		{
			switch ( buffer[1] )
			{
				case 'n':
				case 'N':
					return ent->s.number;
				case 'd':
					return ent->client->lasthurt_us;
				case 'D':
					return ent->client->lastkilled_us;
				case 't':
					return ent->client->lasthurt_client;
				case 'T':
					return ent->client->lastkilled_client;
				case 'f':
				case 'F':
					nearby = G_FindNearbyClient( ent->r.currentOrigin, ent->client->sess.team, 1200, ent );
					if ( nearby != NULL )
					{
						return nearby->s.number;
					}
					break;
				case 'c':
					if ( ent->client->sess.team == TEAM_RED )
					{
						return level.blueFlagCarrier;
					}
					else if ( ent->client->sess.team == TEAM_BLUE )
					{
						return level.redFlagCarrier;
					}
					break;
				case 'C':
					if ( ent->client->sess.team == TEAM_RED )
					{
						return level.redFlagCarrier;
					}
					else if ( ent->client->sess.team == TEAM_BLUE )
					{
						return level.blueFlagCarrier;
					}
					break;
				default:
					return atoi( &buffer[1] );
			}
		}
	}

	return atoi( buffer );
}

qboolean CheckID( gentity_t *ent, int id )
{
	if ( id < 0 || id > MAX_CLIENTS || !g_entities[id].inuse || !g_entities[id].client || g_entities[id].client->pers.connected != CON_CONNECTED ) {
		G_PrintMessage( ent, "^1>>^7 Invalid player id.\n");
		return qfalse;
	}

	return qtrue;
}

int CheckTag( gentity_t *ent, int id )
{
	if ( id == 9999 || id == 9998 ) {			// hack ids
		if ( !level.tagcount ) {
			G_PrintMessage( ent, "^1>>^7 No players have been tagged.\n");
			return qfalse;
		}
		G_PrintMessage( ent, "^4>>^7 Using tags\n");
		return (id == 9999)?1:2;				// #x = 1, #X = 2
	}
	return qfalse;
}

qboolean CheckRank( gentity_t *ent, int id, qboolean peersafe )
{
	if ( !ent ) {
		return qtrue;
	}

	if ( g_entities[id].client->sess.modData->adminref == 0 ) {
		return qtrue;
	}

	if ( !peersafe && g_entities[id].client->sess.modData->adminref == ent->client->sess.modData->adminref ) {
		G_PrintMessage( ent, "^1>>^7 You can't use this command on your peers.\n");
		return qfalse;
	}
	if ( g_entities[id].client->sess.modData->adminref > ent->client->sess.modData->adminref && !(g_entities[id].client->sess.modData->adminref == 3 && ent->client->sess.modData->adminref == 2 && !g_sysopImmuneToAdmin.integer) ) {
		G_PrintMessage( ent, "^1>>^7 You can't use this command your superiors.\n");
		return qfalse;
	}

	return qtrue;
}


