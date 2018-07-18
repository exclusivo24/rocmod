// Copyright (C) 2001-2002 Raven Software.
//
#include "g_local.h"

#include "../../ui/menudef.h"

int AcceptBotCommand(char *cmd, gentity_t *pl);


/*
==================
DeathmatchScoreboardMessage
==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) 
{
	char		entry[1024];
	char		entry2[1024];
	char		string[1400];
	char		string2[1400];
	int			stringlength;
	int			stringlength2;
	int			i, j;
	gclient_t	*cl;
	int			numSorted;
	qboolean	dead;

	// send the latest information on all clients
	string[0]     = 0;
	string2[0]    = 0;
	stringlength  = 0;
	stringlength2 = 0;

	numSorted = level.numConnectedClients;
	
	for (i=0 ; i < numSorted ; i++) 
	{
		int	ping;
		int adminref;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.modData->adminref > 0 && cl->sess.modData->adminref > ent->client->sess.modData->adminref && g_anonymousAdmin.integer == 2 )
		{
			continue;
		}

		if ( cl->pers.connected == CON_CONNECTING ) 
		{
			ping = -1;
		} 
		else 
		{
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if ( g_alternateSort.integer )
		{
			float s;
			int t;

			if ( cl->sess.modData->recondata->kills )
			{
				t = cl->sess.modData->recondata->kills + cl->sess.modData->recondata->deaths;
				s = cl->sess.modData->recondata->kills;
				s /= t;
				s *= 100;
				cl->sess.modData->recondata->score = s;
			}
			else
			{
				cl->sess.modData->recondata->score = 0;
			}
		}

		if ( (cl->sess.ghost || cl->ps.pm_type == PM_DEAD) && (cl->sess.modData->adminref < 2 || !cl->sess.modData->adminFreezeRespawn) )
		{
			dead = qtrue;
		}
		else
		{
			dead = qfalse;
		}

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i", 
			level.sortedClients[i],
			cl->sess.modData->recondata->score,
			cl->sess.modData->recondata->kills, 
			cl->sess.modData->recondata->deaths, 
			ping, 
			(level.time - cl->pers.enterTime)/60000,
			dead,
			g_entities[level.sortedClients[i]].s.gametypeitems,
			g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0
			);

		j = strlen(entry);
		if (stringlength + j > 1022 )
		{
			break;
		}

		strcpy (string + stringlength, entry);
		stringlength += j;

		if ( cl->sess.modData->currkillspree > cl->sess.modData->bestkillspree )
		{
			cl->sess.modData->bestkillspree = cl->sess.modData->currkillspree;
		}

		adminref = cl->sess.modData->adminref;
		if ( g_anonymousAdmin.integer == 2 && cl->sess.modData->clan || (g_anonymousAdmin.integer && (adminref > 1 || adminref < 0) && ent->client->sess.modData->adminref < 2) )
		{
			adminref = 0;
		}
//		if ( g_anonymousReferee.integer && adminref == 1 )
//		{
//			adminref = 0;
//		}

		Com_sprintf (entry2, sizeof(entry2),
			" %i %i %i %i %i %i %i %i %i %i",
			adminref,
			cl->sess.modData->hits,
			cl->sess.modData->misses,
			cl->sess.modData->headshots,
			cl->sess.modData->flagcaps,
			cl->sess.modData->bestkillspree,
			cl->sess.modData->meleekills,
			cl->sess.modData->nadekills,
			cl->sess.modData->clan,
			cl->sess.modData->flagdefends
			);

		j = strlen(entry2);
		if (stringlength2 + j > 1022 )
		{
			break;
		}

		strcpy (string2 + stringlength2, entry2);
		stringlength2 += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE], string ) );
	if ( sv_modClient.integer || ent->client->sess.modData->versionVerified )
	{
		trap_SendServerCommand( ent-g_entities, va("scores4 %i 10 %i%s", g_timelimit.integer?(g_timelimit.integer + level.timeExtension):0, i, string2 ) );
	}
}

/*
==================
G_SendExtraTeamInfo
==================
*/
void G_SendExtraTeamInfo( gentity_t *ent )
{
	char		entry[64];
	char		red[1024];
	char		blue[1024];
	int			redlength;
	int			bluelength;
	int			entrylength;
	int			i, j;
	int			redcount;
	int			bluecount;
	int			changed;
	gclient_t	*cl;
	gentity_t	*loc;

	// Don't update them if they don't want it
	if ( ent && !(ent->client->sess.modData->extraFeatures & EXTRA_TEAMINFO) )
	{
		return;
	}

	// If we're going to fully update this client anyway, make sure
	// their own info gets updated (like after a team switch)
	if ( ent )
	{
		ent->client->sess.modData->tiHealth = 0;
		ent->client->sess.modData->tiArmor = 0;
		ent->client->sess.modData->tiWeapon = 0;
		ent->client->sess.modData->tiLocation = 0;
		ent->client->sess.modData->tiGhost = 0;
	}

	// send the latest information on all clients
	red[0]     = 0;
	redlength  = 0;
	redcount   = 0;
	blue[0]    = 0;
	bluelength = 0;
	bluecount  = 0;

	for (i=0 ; i < level.numConnectedClients ; i++) 
	{
		changed = 0;
		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected != CON_CONNECTED || cl->ps.stats[STAT_HEALTH] <= 0 )
		{
			continue;
		} 

		Com_sprintf( entry, sizeof(entry), " %i", level.sortedClients[i] );
		entrylength = strlen( entry );
		if ( ent || cl->sess.modData->tiHealth != cl->ps.stats[STAT_HEALTH] )
		{
			Com_sprintf( entry + entrylength, sizeof(entry) - entrylength, "h%i", cl->ps.stats[STAT_HEALTH] );
			entrylength = strlen( entry );
			if ( !ent )
			{
				cl->sess.modData->tiHealth = cl->ps.stats[STAT_HEALTH];
			}
			changed = 1;
		}
		if ( ent || cl->sess.modData->tiArmor != cl->ps.stats[STAT_ARMOR] )
		{
			Com_sprintf( entry + entrylength, sizeof(entry) - entrylength, "a%i", cl->ps.stats[STAT_ARMOR] );
			entrylength = strlen( entry );
			if ( !ent )
			{
				cl->sess.modData->tiArmor = cl->ps.stats[STAT_ARMOR];
			}
			changed = 1;
		}
		if ( ent || cl->sess.modData->tiWeapon != g_entities[level.sortedClients[i]].s.weapon )
		{
			Com_sprintf( entry + entrylength, sizeof(entry) - entrylength, "w%i", g_entities[level.sortedClients[i]].s.weapon );
			entrylength = strlen( entry );
			if ( !ent )
			{
				cl->sess.modData->tiWeapon = g_entities[level.sortedClients[i]].s.weapon;
			}
			changed = 1;
		}
		loc = Team_GetLocation( &g_entities[level.sortedClients[i]], qfalse );
		if ( ent || cl->sess.modData->tiLocation != loc->health )
		{
			Com_sprintf( entry + entrylength, sizeof(entry) - entrylength, "l%i", loc->health );
			entrylength = strlen( entry );
			if ( !ent )
			{
				cl->sess.modData->location = loc;
				cl->sess.modData->tiLocation = loc->health;
			}
			changed = 1;
		}
		if ( ent || cl->sess.modData->tiGhost != cl->sess.ghost )
		{
			Com_sprintf( entry + entrylength, sizeof(entry) - entrylength, "g%i", cl->sess.ghost );
			entrylength = strlen( entry );
			if ( !ent )
			{
				cl->sess.modData->tiGhost = cl->sess.ghost;
			}
			changed = 1;
		}

		j = strlen(entry);
		if ( changed && cl->sess.team == TEAM_RED )
		{
			if (redlength + j > 1022 )
			{
				break;
			}

			strcpy (red + redlength, entry);
			redlength += j;
			redcount++;
		}
		else if ( changed && cl->sess.team == TEAM_BLUE )
		{
			if (bluelength + j > 1022 )
			{
				break;
			}

			strcpy (blue + bluelength, entry);
			bluelength += j;
			bluecount++;
		}
	}

	if ( ent )
	{
		if ( ent->client->sess.team == TEAM_RED )
		{
			trap_SendServerCommand( ent-g_entities, va("eti2 %i%s", redcount, red ) );
		}
		else if ( ent->client->sess.team == TEAM_BLUE )
		{
			trap_SendServerCommand( ent-g_entities, va("eti2 %i%s", bluecount, blue ) );
		}
//		else
//		{
//			trap_SendServerCommand( ent-g_entities, va("eti2 %i%s%s", redcount + bluecount, red, blue ) );
//		}
	}
	else
	{
		for (i=0 ; i < level.numConnectedClients ; i++) 
		{
			cl = &level.clients[level.sortedClients[i]];

			if ( cl->pers.connected != CON_CONNECTED )
			{
				continue;
			}

			if ( cl->sess.modData->extraFeatures & EXTRA_TEAMINFO ) {
				if ( cl->sess.team == TEAM_RED && redcount )
				{
					trap_SendServerCommand( level.sortedClients[i], va("eti2 %i%s", redcount, red ) );
				}
				else if ( cl->sess.team == TEAM_BLUE && bluecount )
				{
					trap_SendServerCommand( level.sortedClients[i], va("eti2 %i%s", bluecount, blue ) );
				}
//				else
//				{
//					trap_SendServerCommand( level.sortedClients[i], va("eti2 %i%s%s", redcount + bluecount, red, blue ) );
//				}
			}
		}
	}
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) 
{
	DeathmatchScoreboardMessage( ent );
}

/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, "print \"You must be alive to use this command.\n\"");
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}

	*out = 0;
}

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ ) 
	{
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return i;
		}
	}

	return -1;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Drop_f

Drops the currenty selected weapon
==================
*/
void Cmd_Drop_f ( gentity_t* ent )
{
	gentity_t* dropped;

	// spectators cant drop anything since they dont have anything
	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		return;
	}

	// Ghosts and followers cant drop stuff
	if ( ent->client->ps.pm_flags & (PMF_GHOST|PMF_FOLLOW) )
	{
		return;
	}

	// Drop the weapon the client wanted to drop
	dropped = G_DropWeapon ( ent, atoi(ConcatArgs( 1 )), 3000 );
	if ( !dropped )
	{
		return;
	}
}

/*
==================
Cmd_DropItem_f

Drops the gametype items the player is carrying
==================
*/
void Cmd_DropItem_f ( gentity_t* ent )
{
	// spectators cant drop anything since they dont have anything
	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		return;
	}

	// Ghosts and followers cant drop stuff
	if ( ent->client->ps.pm_flags & (PMF_GHOST|PMF_FOLLOW) )
	{
		return;
	}

	// Nothing to drop
	if ( !ent->client->ps.stats[STAT_GAMETYPE_ITEMS] )
	{
		return;
	}

	// Just plain can't drop it
	if ( !g_allowItemDrop.integer )
	{
		return;
	}

	G_DropGametypeItems ( ent, 3000 );
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_QPATH];

	int start;
	int end;
	int l;

	trap_Argv( 1, arg, sizeof( arg ) );

	if ( !Q_stricmp ( arg, "me" ) )
	{
		start = ent->s.number;
		end = start + 1;
	}
	else if ( !Q_stricmp ( arg, "all" ) )
	{
		start = 0;
		end   = MAX_CLIENTS;
	}
	else
	{
		start = atoi ( arg );
		end = start + 1;
	}

	for ( l = start; l < end; l ++ )
	{
		ent = &g_entities[l];

		if ( !ent->inuse )
		{
			continue;
		}

		if ( G_IsClientDead ( ent->client ) )
		{
			continue;
		}

		if ( !CheatsOk( ent ) ) {
			return;
		}

	name = ConcatArgs( 2 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		ent->health = MAX_HEALTH;
		if (!give_all)
			continue;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 - ( 1 << WP_NONE );
		if (!give_all)
			continue;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for ( i = WP_NONE + 1 ; i < WP_NUM_WEAPONS ; i++ ) 
		{
			attackType_t a;
	
			for ( a = ATTACK_NORMAL; a < ATTACK_MAX; a ++ )
			{
				ent->client->ps.clip[a][i] = weaponData[i].attack[a].clipSize;
				ent->client->ps.ammo[weaponData[i].attack[a].ammoIndex] = ammoData[weaponData[i].attack[a].ammoIndex].max;
			}
		}

		if (!give_all)
			continue;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = MAX_ARMOR;

		if (!give_all)
			continue;
	}

	// spawn a specific item right on the player
	if ( !give_all ) 
	{
		it = BG_FindItem (name);
		if (!it) 
		{
			continue;
		}

		if ( it->giType == IT_GAMETYPE )
		{
			continue;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) 
		{
			G_FreeEntity( it_ent );
		}
	}

	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) 
{
	if ( !CheatsOk( ent ) ) 
	{
		return;
	}

	BeginIntermission();

	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) 
{
	// No killing yourself if your a spectator
	if ( G_IsClientSpectating ( ent->client ) )
	{
		return;
	}

	// No killing yourself if your dead
	if ( G_IsClientDead ( ent->client ) ) 
	{
		return;
	}

	// No killing yourself if being punished
	if ( recondata[ent->s.number].attributes )
	{
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE, HL_NONE, vec3_origin );
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	switch ( client->sess.team )
	{
		case TEAM_RED:
			trap_SendServerCommand( -1, va("cp \"%s ^7joined the %s%s ^7team.\n\"", client->pers.netname, level.teamData.teamcolor[1], level.teamData.redName) );
			break;

		case TEAM_BLUE:
			trap_SendServerCommand( -1, va("cp \"%s ^7joined the %s%s ^7team.\n\"", client->pers.netname, level.teamData.teamcolor[2], level.teamData.blueName));
			break;

		case TEAM_SPECTATOR:
			if ( oldTeam != TEAM_SPECTATOR )
			{
				trap_SendServerCommand( -1, va("cp \"%s ^7joined the spectators.\n\"", client->pers.netname));
			}
			break;

		case TEAM_FREE:
			trap_SendServerCommand( -1, va("cp \"%s ^7joined the battle.\n\"", client->pers.netname));
			break;
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s, const char* identity ) 
{
	int					team;
	int					oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	qboolean			ghost;
   	qboolean			noOutfittingChange = qfalse;

	if ( level.paused && ent->client->sess.team == TEAM_SPECTATOR )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Can't join a team while the game is paused.\n\"");
		return;
	}

	if ( ent->client->sess.modData->penalty )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Can't set teams while in the penalty box.\n\"");
		return;
	}

	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;

	// If an identity was specified then inject it into
	// the clients userinfo
	if ( identity )
	{
		char userinfo[MAX_INFO_STRING];
		trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

		if ( Q_stricmp ( identity, Info_ValueForKey ( userinfo, "identity" ) ) )
		{
			Info_SetValueForKey ( userinfo, "identity", identity );
			Info_SetValueForKey ( userinfo, "team_identity", identity );
			trap_SetUserinfo ( clientNum, userinfo );
		}
		else
		{
			identity = NULL;
		}
	}

	if ( !Q_stricmp( s, "follow1" ) ) 
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} 
	else if ( !Q_stricmp( s, "follow2" ) ) 
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} 
	else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) || ent->client->sess.modData->moderator ) 
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} 
	else if ( level.gametypeData->teams ) 
	{
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, level.teamData.redName ) || !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) 
		{
			team = TEAM_RED;
		} 
		else if ( !Q_stricmp( s, level.teamData.blueName ) || !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) 
		{
			team = TEAM_BLUE;
		} 
		else 
		{
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}

		if ( !level.override && g_teamForceBalance.integer && ent->client->sess.modData->adminref < 2 ) 
		{
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE, NULL );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED, NULL );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) 
			{
				trap_SendServerCommand( ent->client->ps.clientNum, 
										va("cp \"%s%s^7 team has too many players.\n\"", level.teamData.teamcolor[1], level.teamData.redName) );

				// ignore the request
				return; 
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 )
			{
				trap_SendServerCommand( ent->client->ps.clientNum, 
										va("cp \"%s%s^7 team has too many players.\n\"", level.teamData.teamcolor[2], level.teamData.blueName) );

				// ignore the request
				return; 
			}
		}

		// Make sure the team isn't locked
		if ( !level.override && team == TEAM_RED && (level.teamRedLocked || level.match) && !ent->client->sess.modData->invitedRed && ent->client->sess.modData->adminref < 2 )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, 
									va("cp \"%s%s^7 team has been locked.\n\"", level.teamData.teamcolor[1], level.teamData.redName) );

			// ignore the request
			return; 
		}
		if ( !level.override && team == TEAM_BLUE && (level.teamBlueLocked || level.match) && !ent->client->sess.modData->invitedBlue && ent->client->sess.modData->adminref < 2 )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, 
									va("cp \"%s%s^7 team has been locked.\n\"", level.teamData.teamcolor[2], level.teamData.blueName) );

			// ignore the request
			return; 
		}

		// It's ok, the team we are switching to has less or same number of players
	} 
	else 
	{
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer ) 
	{
		team = TEAM_SPECTATOR;
	}

	// decide if we will allow the change
	oldTeam = client->sess.team;
	ghost   = client->sess.ghost;

	if ( team == oldTeam && team != TEAM_SPECTATOR )
	{
		if ( identity )
		{
			// get and distribute relevent paramters
			client->pers.identity = NULL;
			ClientUserinfoChanged( clientNum );
		}
		
		return;
	}

   	noOutfittingChange = ent->client->noOutfittingChange;

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	
	if ( oldTeam != TEAM_SPECTATOR ) 
	{
		if ( ghost )
		{
			G_StopGhosting ( ent );
		}
		else if ( !G_IsClientDead ( client ) )
		{
			// Kill him (makes sure he loses flags, etc)
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die (ent, ent, ent, 100000, MOD_TEAMCHANGE, HL_NONE, vec3_origin );

			ent->client->sess.ghost = qfalse;
		}
	}

	if ( ent->s.number == level.redFlagCarrier )
	{
		level.redFlagCarrier = -1;
	}
	if ( ent->s.number == level.blueFlagCarrier )
	{
		level.blueFlagCarrier = -1;
	}
	if ( ent->s.number == level.redFlagKiller )
	{
		level.redFlagKiller = -1;
	}
	if ( ent->s.number == level.blueFlagKiller )
	{
		level.blueFlagKiller = -1;
	}

	// If respawn interval start as a ghost
	if ( level.gametypeRespawnTime[ team ] )
	{
		ghost = qtrue;
	}

	// they go to the end of the line
	if ( team == TEAM_SPECTATOR ) 
	{
		client->sess.spectatorTime = level.time;
	}

	client->sess.team = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	// Kill any child entities of this client to protect against grenade team changers
	G_FreeEnitityChildren ( ent );

	// Always spawn into a ctf game using a respawn timer.
	if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_INTERVAL )
	{
		G_SetRespawnTimer ( ent );
		ghost = qtrue;
	}

	BroadcastTeamChange( client, oldTeam );

	G_SendExtraTeamInfo( ent );

	// See if we should spawn as a ghost
	if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_NONE )
	{
		// If there are ghosts already then spawn as a ghost because
		// the game is already in progress.
		if ( !level.warmupTime && (level.gametypeJoinTime && (level.time - level.gametypeJoinTime) > (g_roundjointime.integer * 1000)) || noOutfittingChange || client->sess.noTeamChange )
		{
			ghost = qtrue;
		}

		// Spectator to a team doesnt count
		if ( oldTeam != TEAM_SPECTATOR )
		{
			client->sess.noTeamChange = qtrue;
		}
	}

	// If a ghost, enforce it
	if ( ghost )
	{
		// Make them a ghost again
		if ( team != TEAM_SPECTATOR )
		{
			G_StartGhosting ( ent );

			// get and distribute relevent paramters
			client->pers.identity = NULL;
			ClientUserinfoChanged( clientNum );

			CalculateRanks();

			return;
		}
	}	

	// get and distribute relevent paramters
	client->pers.identity = NULL;
	ClientUserinfoChanged( clientNum );

	CalculateRanks();

	// Begin the clients new life on the their new team
	ClientBegin( clientNum );

	if ( team == TEAM_SPECTATOR ) 
	{
		if ( level.match )
		{
			if ( !ent->client->sess.modData->moderator )
			{
				Cmd_FollowCycle_f (ent, 1);
			}
		}
		else
		{
			if ( g_lockSpec.integer )
			{
				Cmd_FollowCycle_f (ent, 1);
			}
		}
	}
}

/*
=================
SetTeamInstant
=================
*/
void SetTeamInstant( gentity_t *ent, int team, qboolean broadcast ) 
{
	int oldteam;

	if ( ent->client->sess.modData->moderator )
	{
		return;
	}

	oldteam = ent->client->sess.team;
	ent->client->sess.team = team;

	ent->client->ps.stats[STAT_WEAPONS] = 0;
	TossClientItems( ent );
	G_StartGhosting( ent );
	ent->client->pers.identity = NULL;
	ClientUserinfoChanged( ent - g_entities );

	CalculateRanks();

	if ( !ent->client->sess.modData->penalty )
	{
		G_StopFollowing( ent );
		G_StopGhosting( ent );
		trap_UnlinkEntity ( ent );
		ClientSpawn( ent );
	}

	if ( broadcast )
	{
		BroadcastTeamChange( ent->client, oldteam );
	}

	G_SendExtraTeamInfo( ent );
}


/*
=================
G_StartGhosting

Starts a client ghosting.  This essentially will kill a player which is alive
=================
*/
void G_StartGhosting ( gentity_t* ent )
{
	int i;

	// Dont start ghosting if already ghosting
	if ( ent->client->sess.ghost )
	{
		return;
	}

	ent->client->sess.ghost = qtrue;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->sess.spectatorClient = -1;
	ent->client->ps.pm_flags |= PMF_GHOST;
	ent->client->ps.stats[STAT_HEALTH] = 100;
	ent->client->ps.pm_type = PM_SPECTATOR;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;

	trap_UnlinkEntity (ent);

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		if ( G_IsClientSpectating ( &level.clients[i] )
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == ent->s.number ) 
		{
			G_StopFollowing( &g_entities[i] );
		}
	}
}

/*
=================
G_StopGhosting

Stops a client from ghosting.  The client will be dead after this
call
=================
*/
void G_StopGhosting ( gentity_t* ent )
{
	// Dont stop someone who isnt ghosting in the first place
	if ( !ent->client->sess.ghost )
	{
		return;
	}

	ent->client->sess.ghost = qfalse;
	ent->client->ps.pm_flags &= ~PMF_GHOST;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;

	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		ent->client->ps.pm_type = PM_SPECTATOR;
	}
	else
	{
		ent->client->ps.pm_type = PM_DEAD;
		ent->health = ent->client->ps.stats[STAT_HEALTH] = 0;
	}
}

/*
=================
G_StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void G_StopFollowing( gentity_t *ent ) 
{
	// Cant stop following if not following in the first place
	if ( !(ent->client->ps.pm_flags&PMF_FOLLOW) )
	{
		return;
	}

	// Clear the following variables
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.zoomFov = 0;
	ent->client->ps.loopSound = 0;
	ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
	ent->client->ps.persistant[PERS_TEAM] = ent->client->sess.team;
//	ent->r.svFlags &= ~SVF_BOT;		// wtf?

	// If we were in fact following someone, then make the angles and origin nice for
	// when we stop
   	if ( ent->client->sess.spectatorClient != -1 )
   	{
   		gclient_t* cl = &level.clients[ent->client->sess.spectatorClient];
    
   		int i;
   		for ( i = 0; i < 3; i ++ )
		{
   			ent->client->ps.delta_angles[i] = ANGLE2SHORT(cl->ps.viewangles[i] - SHORT2ANGLE(ent->client->pers.cmd.angles[i]));
		}
    
  		VectorCopy ( cl->ps.viewangles, ent->client->ps.viewangles );
   		VectorCopy ( cl->ps.origin, ent->client->ps.origin );
   		VectorClear ( ent->client->ps.velocity );
   		ent->client->ps.movementDir = 0;
    
   		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
   	}		

	// Ghots dont really become spectators, just psuedo spectators
	if ( ent->client->sess.ghost )
	{
		// Do a start and stop to ensure the variables are all set properly
		G_StopGhosting ( ent );
		G_StartGhosting ( ent );
	}
	else
	{
		ent->client->sess.team = TEAM_SPECTATOR;	
		ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	}
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) 
{
	char team[MAX_TOKEN_CHARS];
	char identity[MAX_TOKEN_CHARS];

	// Need at least the team specified in the arguments
	if ( trap_Argc() < 2 ) 
	{
		int oldTeam = ent->client->sess.team;
		switch ( oldTeam ) 
		{
			case TEAM_BLUE:
				trap_SendServerCommand( ent-g_entities, va("print \"%s^2 team\n\"", level.teamData.blueName) );
				break;
			
			case TEAM_RED:
				trap_SendServerCommand( ent-g_entities, va("print \"%s^2 team\n\"", level.teamData.redName) );
				break;
			
			case TEAM_FREE:
				trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
				break;
		
			case TEAM_SPECTATOR:
				trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
				break;
		}
		
		return;
	}

	// Limit how often one can switch team
	if ( ent->client->switchTeamTime > level.time ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\"" );
		return;
	}

	trap_Argv( 1, team, sizeof( team ) );
	trap_Argv( 2, identity, sizeof( identity ) );

	SetTeam( ent, team, identity[0]?identity:NULL );

	// Remember the team switch time so they cant do it again really quick
	ent->client->switchTeamTime = level.time + 5000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) 
{
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) 
	{
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) 
		{
			G_StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) 
	{
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) 
	{
		return;
	}

	// cant cycle to dead people
	if ( level.clients[i].ps.pm_type == PM_DEAD )
	{
		return;
	}

	// can't follow another spectator
	if ( G_IsClientSpectating ( &level.clients[ i ] ) )
	{
		return;
	}

	// Dissallow following of the enemy if the cvar is set
	if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR )
	{
		// Are they on the same team?
		if ( level.clients[ i ].sess.team != ent->client->sess.team && ent->client->sess.modData->adminref < 2 )
		{
			return;
		}
	}

	// See if they're even allowed to spec that player's team
	if ( (level.match || g_lockSpec.integer) && level.clients[ i ].sess.team != ent->client->sess.team && (!level.match || !ent->client->sess.modData->moderator) )
	{
		if ( level.clients[ i ].sess.team == TEAM_RED && level.teamRedLocked && !ent->client->sess.modData->invitedRed )
		{
			return;
		}
		if ( level.clients[ i ].sess.team == TEAM_BLUE && level.teamBlueLocked && !ent->client->sess.modData->invitedBlue )
		{
			return;
		}
	}

	// first set them to spectator as long as they arent a ghost
	if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR ) 
	{
		SetTeam( ent, "spectator", NULL );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) 
{
	int		clientnum;
	int		deadclient;
	int		original;

	ent->client->ps.stats[STAT_FROZEN] = 0;

	// first set them to spectator
	if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR ) 
	{
		SetTeam( ent, "spectator", NULL );
	}

	if ( dir != 1 && dir != -1 ) 
	{
		Com_Error( ERR_FATAL, "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	if ( ent->client->sess.spectatorClient == -1 )
	{
		clientnum = original = ent->s.number;
	}
	else
	{
		clientnum = original = ent->client->sess.spectatorClient;
	}

	if ( ent->client->sess.modData->adminref > 1 )
	{
		ent->client->sess.modData->adminFollow = qtrue;
	}

	deadclient = -1;
	do 
	{
		clientnum += dir;
		if ( clientnum >= level.maxclients ) 
		{
			clientnum = 0;
		}
		if ( clientnum < 0 ) 
		{
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		// can't follow another spectator
		if ( G_IsClientSpectating ( &level.clients[ clientnum ] ) ) 
		{
			continue;
		}

		// Cant switch to dead people unless there is nobody else to switch to
		if ( G_IsClientDead ( &level.clients[clientnum] ) )
		{		
			deadclient = clientnum;
			continue;
		}

		if ( ent->client->sess.team == TEAM_SPECTATOR )
		{
			if ( level.match && !ent->client->sess.modData->moderator )
			{
				continue;
			}

			if ( g_lockSpec.integer && (!level.match || !ent->client->sess.modData->moderator) )
			{
				if ( level.teamRedLocked && level.clients[ clientnum ].sess.team == TEAM_RED && !ent->client->sess.modData->invitedRed )
				{
					continue;
				}
				if ( level.teamBlueLocked && level.clients[ clientnum ].sess.team == TEAM_BLUE && !ent->client->sess.modData->invitedBlue )
				{
					continue;
				}
			}
		}
		else
		{
			// Dissallow following of the enemy if the cvar is set
			if ( level.gametypeData->teams && !g_followEnemy.integer )
			{
				// Are they on the same team?
				if ( level.clients[ clientnum ].sess.team != ent->client->sess.team && ent->client->sess.modData->adminref < 2 )
				{
					continue;
				}
			}
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;

	} while ( clientnum != original );

	if ( level.match && ent->client->sess.modData->moderator )
	{
	   	G_StopFollowing( ent );
	}
	else
	{
		// If being forced to follow and there is a dead client to jump to, then jump to them now
		if ( deadclient != -1 && g_forceFollow.integer && ent->client->sess.modData->adminref < 2 )
		{
			// this is good, we can use it
			ent->client->sess.spectatorClient = deadclient;
			ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
			return;
		}

	   	G_StopFollowing( ent );
		if ( level.match || (level.teamRedLocked && level.teamBlueLocked && ent->client->sess.modData->adminref < 2) )
		{
			FindIntermissionPoint ( );
			level.intermission_origin[2] += 3000;		// high enough?
			VectorCopy( level.intermission_origin, ent->s.origin );
			VectorCopy( level.intermission_origin, ent->r.currentOrigin );
			VectorCopy( level.intermission_origin, ent->client->ps.pvsOrigin );
			VectorCopy( level.intermission_origin, ent->client->ps.origin );
			VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
		}

		if ( !ent->client->sess.modData->moderator )
		{
			// no one to follow, so freeze them in place
			ent->client->ps.stats[STAT_PLANTED] = 1;
		}
	}
}

/*
==================
G_SayTo
==================
*/
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, const char *name, const char *message, char *sound, qboolean override ) 
{
	qboolean	ghost = qfalse;
	qboolean	spec = qfalse;
	qboolean	adminchat = qfalse;
	qboolean	srcadmin = qfalse;
	qboolean	dstadmin = qfalse;
	const char*	type;
	const char*	chat;
	const char*	color;

	if (!other) 
	{
		return;
	}

	if (!other->inuse) 
	{
		return;
	}

	if (!other->client) 
	{
		return;
	}

	if ( other->client->pers.connected != CON_CONNECTED ) 
	{
		return;
	}

	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) 
	{
		return;
	}

	if ( !override )
	{
		if ( ent->client->sess.muted )
		{
			return;
		}

		if ( ent->client->sess.modData->penalty )
		{
			return;
		}
	}

	if ( ent->client->sess.modData->adminref == -1 )
	{
		srcadmin = 2;
	}
	else
	{
		srcadmin = ent->client->sess.modData->adminref;
	}
	if ( other->client->sess.modData->adminref == -1 )
	{
		dstadmin = 2;
	}
	else
	{
		dstadmin = other->client->sess.modData->adminref;
	}

	if ( G_IsClientChatIgnored ( other->s.number, ent->s.number ) && mode != SAY_ADMIN_ALL && mode != SAY_ADMIN_ONLY )
	{
		return;
	}

 	if ( (mode == SAY_ADMIN_ALL || mode == SAY_ADMIN_ONLY) && srcadmin < 2 && !override )
	{
		return;
	}

 	if ( mode == SAY_ADMIN_ONLY && dstadmin < 2 )
	{
		return;
	}

 	if ( (mode == SAY_REF_ALL || mode == SAY_REF_ONLY) && srcadmin == 0 && !override )
	{
		return;
	}

 	if ( mode == SAY_REF_ONLY && dstadmin == 0 )
	{
		return;
	}

 	if ( mode == SAY_CLAN_ONLY && !ent->client->sess.modData->clan && !override )
	{
		return;
	}

 	if ( mode == SAY_CLAN_ONLY && (!other->client->sess.modData->clan && other->client->sess.modData->adminref < 2 && other->client->sess.modData->adminref > -1) )
	{
		return;
	}

	if ( !level.intermissiontime && !level.intermissionQueued )
	{
		// Spectators cant talk to alive people
		if ( ent->client->sess.team == TEAM_SPECTATOR )
		{
			spec = qtrue;
		}

		if ( level.gametypeData->respawnType == RT_NONE )
		{
			// Dead people cant talk to alive people
			if ( !spec && G_IsClientDead ( ent->client ) )
			{
				ghost = qtrue;
			}

			// If the client we are talking to is alive then a check
			// must be made to see if this talker is alowed to speak to this person
			if ( ent->s.number != other->s.number && !G_IsClientDead ( other->client ) && !G_IsClientSpectating( other->client) && (ghost || spec) && mode != SAY_ADMIN_ALL && mode != SAY_ADMIN_ONLY && !g_voiceTalkingGhosts.integer)
			{
				return;
			}
		}
	}

	chat = "";
	color = "";
	if ( mode == SAY_TEAM )
	{
		chat = "tchat";
	}
	else
	{
		chat = "chat";
	}

	type = "";
	if ( mode == SAY_ADMIN_TELL )
	{
		color = "^7";
		adminchat = qtrue;
		if ( g_anonymousAdmin.integer && dstadmin < 2 )
		{
			name = "";
		}
	}
	else if ( mode == SAY_ADMIN_ALL )
	{
		color = "^7";
		type = "^5*admin* ^7";
		if ( sv_modClient.integer || other->client->sess.modData->versionVerified )
		{
			chat = "achat";
		}
		else
		{
			chat = "chat";
			adminchat = qtrue;
		}
		if ( g_anonymousAdmin.integer && dstadmin < 2 )
		{
			name = "";
		}
	}
	else if ( mode == SAY_ADMIN_ONLY )
	{
		type = "^6*admins only* ^7";
		if ( sv_modClient.integer || other->client->sess.modData->versionVerified )
		{
			chat = "achat";
		}
		else
		{
			chat = "chat";
			adminchat = qtrue;
		}
	}
	else if ( mode == SAY_REF_ALL )
	{
		color = "^7";
		type = "^3*referee* ^7";
		if ( sv_modClient.integer || other->client->sess.modData->versionVerified )
		{
			chat = "achat";
		}
		else
		{
			chat = "chat";
			adminchat = qtrue;
		}
	}
	else if ( mode == SAY_REF_ONLY )
	{
		type = "^6*referees only* ^7";
		if ( sv_modClient.integer || other->client->sess.modData->versionVerified )
		{
			chat = "achat";
		}
		else
		{
			chat = "chat";
			adminchat = qtrue;
		}
	}
	else if ( mode == SAY_CLAN_ALL )
	{
		color = "^7";
		type = "^1*clan* ^7";
	}
	else if ( mode == SAY_CLAN_ONLY )
	{
		type = "^1*clan only* ^7";
	}
	else if ( ghost )
	{
		type = "*ghost* ";
	}
	else if ( spec )
	{
		type = "*spec* ";
	}

	trap_SendServerCommand( other-g_entities, va("%s %d \"%s%s%s%s\"", chat, ent->s.number, type, name, color, message));
	if ( adminchat )
	{
		G_LocalSound( other->s.number, "sound/misc/menus/quit_request.wav" );
	}
	if ( g_voiceCommandsEnabled.integer && sound[0] )
	{
		if ( g_voiceCommandsEnabled.integer == 1 && sv_modClient.integer || other->client->sess.modData->versionVerified )
		{
			trap_SendServerCommand( other-g_entities, va("sound %i \"%s\"", ent->client->sess.team, sound));
		}
		else
		{
			G_LocalSound( other->s.number, sound );
		}
	}
}

/*
==================
G_GetChatPrefix
==================
*/
void G_GetChatPrefix ( gentity_t* ent, gentity_t* target, int mode, char* name, int nameSize )
{
	const char* namecolor;
	char		location[64];
	char		test[2000];
	qboolean	locationOk = qtrue;

	// Spectators and ghosts dont show locations
	if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
	{
		locationOk = qfalse;
	}

	if ( !level.gametypeData->teams && mode == SAY_TEAM ) 
	{
		mode = SAY_ALL;
	}

	if ( level.gametypeData->teams )
	{
		switch ( ent->client->sess.team )
		{
			case TEAM_BLUE:
				namecolor = level.teamData.teamcolor[2];
				break;

			case TEAM_RED:
				namecolor = level.teamData.teamcolor[1];
				break;

			default:
				namecolor = S_COLOR_WHITE;
				break;
		}
	}
	else
	{
		namecolor = S_COLOR_WHITE;
	}

	switch ( mode ) 
	{
		default:
		case SAY_ALL:

			Com_sprintf (name, nameSize, "%s%s%s: ", namecolor, ent->client->pers.netname, S_COLOR_WHITE );

			break;

		case SAY_TEAM:

			if ( locationOk && Team_GetLocationMsg(ent, location, sizeof(location)))
			{
				Com_sprintf ( name, nameSize, "%s(%s%s) %s(%s): ", 
							  namecolor, 
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE, location );
			}
			else
			{
				Com_sprintf ( name, nameSize, "%s(%s%s)%s: ", 
							  namecolor, 
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE );
			}
			break;

		case SAY_TELL:

			if ( locationOk && target && level.gametypeData->teams   && 
				 target->client->sess.team == ent->client->sess.team  &&
				 Team_GetLocationMsg(ent, location, sizeof(location))    )
			{
				Com_sprintf ( name, nameSize, "%s[private](%s%s) %s(%s): ", 
							  namecolor,
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE, location );
			}
			else
			{
				Com_sprintf ( name, nameSize, "%s[private](%s%s)%s: ", 
							  namecolor,
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE );
			}
			break;

		case SAY_ADMIN_TELL:

			Com_sprintf ( name, nameSize, "%s[private admin](%s%s)%s: ", 
						  S_COLOR_MAGENTA,
						  ent->client->pers.netname, 
						  S_COLOR_MAGENTA,
						  S_COLOR_WHITE );
			break;
	}

	strcat ( name, S_COLOR_GREEN );
}

/*
==================
G_GetHitLocationText
==================
*/
char *G_GetHitLocationText( int location, qboolean detail )
{
	if ( detail )
	{
		switch ( location )
		{
			case HL_FOOT_RT:
				return "right foot";
			case HL_FOOT_LT:
				return "left foot";
			case HL_FOOT_RT+HL_FOOT_LT:
				return "feet";
			case HL_LEG_UPPER_RT:
				return "right thigh";
			case HL_LEG_UPPER_LT:
				return "left thigh";
			case HL_LEG_LOWER_RT:
				return "right calf";
			case HL_LEG_LOWER_LT:
				return "left calf";
			case HL_HAND_RT:
				return "right hand";
			case HL_HAND_LT:
				return "left hand";
			case HL_ARM_RT:
				return "right arm";
			case HL_ARM_LT:
				return "left arm";
			case HL_HEAD:
				return "head";
			case HL_WAIST:
				return "waist";
			case HL_BACK_RT:
				return "right back";
			case HL_BACK_LT:
				return "left back";
			case HL_BACK:
				return "back";
			case HL_CHEST_RT:
				return "right chest";
			case HL_CHEST_LT:
				return "left chest";
			case HL_CHEST:
				return "chest";
			case HL_NECK:
				return "neck";
			default:
				return "guts";
		}
	}
	else
	{
		switch ( location )
		{
			case HL_FOOT_RT:
			case HL_FOOT_LT:
				return "foot";
			case HL_FOOT_RT+HL_FOOT_LT:
				return "feet";
			case HL_LEG_UPPER_RT:
			case HL_LEG_UPPER_LT:
			case HL_LEG_LOWER_RT:
			case HL_LEG_LOWER_LT:
				return "leg";
			case HL_HAND_RT:
			case HL_HAND_LT:
				return "hand";
			case HL_ARM_RT:
			case HL_ARM_LT:
				return "arm";
			case HL_HEAD:
				return "head";
			case HL_WAIST:
				return "waist";
			case HL_BACK_RT:
			case HL_BACK_LT:
			case HL_BACK:
				return "back";
			case HL_CHEST_RT:
			case HL_CHEST_LT:
			case HL_CHEST:
				return "chest";
			case HL_NECK:
				return "neck";
			default:
				return "guts";
		}
	}
}

/*
==================
G_ParseTextModifier
==================
*/
int G_ParseTextModifier ( gentity_t *ent, const char *src )
{
	if ( ent->client->sess.modData->adminref > 1 || ent->client->sess.modData->adminref < 0 )
	{
		if ( Q_stricmpn( src, "!ac ", 4) == 0 && g_allowChat.string[1] != '0' )
		{
			return SAY_ADMIN_ONLY;
		}
		else if ( Q_stricmpn( src, "!at ", 4) == 0 && g_allowTalk.string[1] != '0' )
		{
			return SAY_ADMIN_ALL;
		}
	}
	if ( ent->client->sess.modData->adminref != 0 )
	{
		if ( Q_stricmpn( src, "!rc ", 4) == 0 && g_allowChat.string[2] == '1' )
		{
			return SAY_REF_ONLY;
		}
		else if ( Q_stricmpn( src, "!rt ", 4) == 0 && g_allowTalk.string[2] == '1' )
		{
			return SAY_REF_ALL;
		}
	}
	if ( Q_stricmpn( src, "!cc ", 4) == 0 && g_allowChat.string[3] == '1' )
	{
		return SAY_CLAN_ONLY;
	}
	if ( Q_stricmpn( src, "!ct ", 4) == 0 && g_allowTalk.string[3] == '1' )
	{
		return SAY_CLAN_ALL;
	}

	return 0;
}

// String compare that ignores text coloring
int stricmpc (const char *s1, const char *s2) {
	int		c1, c2;

	// bk001129 - moved in 1.17 fix not in id codebase
        if ( s1 == NULL ) {
           if ( s2 == NULL )
             return 0;
           else
             return -1;
        }
        else if ( s2==NULL )
          return 1;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 == '^') {
			s1++;
			c1 = *s1++;
		}
		if (c2 == '^') {
			s2++;
			c2 = *s2++;
		}

		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (*s1);
	
	return 0;		// strings are equal
}

/*
==================
G_FilterChatText
==================
*/
void G_FilterChatText ( char *text )
{
	int i;
	int c;

	while ( *text != 0 )
	{
		for ( i = 0; i < chatfilter.chatFilterCount; i++ )
		{
			if ( !stricmpc( chatfilter.chatFilterText[i], text ) ) {
				c = chatfilter.chatFilterLength[i];
				while ( c )
				{
					if ( *text == '^' )
					{
						text += 2;
					}
					else
					{
						*text++ = '*';
						c--;
					}
				}
				break;
			}
		}
		while ( *text != 0 && *text != ' ' )
		{
			text++;
		}
		while ( *text != 0 && *text == ' ' )
		{
			text++;
		}
	}
}

/*
==================
G_ParseTextTokens
==================
*/
void G_ParseTextTokens ( gentity_t *ent, const char *src, char *dest, int textlen )
{
	int			len;
	int			weaponmask;
	int			i;
	char		*text;
	char		location[64];
	char		bar[18];
	gentity_t	*nearby;
	const char	*pos;

	static weaponStats_t weaponlist[21] = {
		{ "Knife", "Knife" },
		{ "M1911A1", "M1911A1 Pistol" },
		{ "USSOCOM", "USSOCOM Pistol" },
		{ "Silver Talon", "Silver Talon Pistol" },
		{ "M590", "M590 Shotgun" },
		{ "Micro Uzi", "Micro Uzi Sub-Machinegun" },
		{ "M3A1", "M3A1 Sub-Machinegun" },
		{ "MP5", "MP5 Sub-Machinegun" },
		{ "USAS12", "USAS 12 Automatic Shotgun" },
		{ "M4", "M4 Assault Rifle" },
		{ "AK74", "AK74 Assault Rifle" },
		{ "SIG551", "SIG551 Assault Rifle" },
#ifdef SMKWEAPONS
		{ "OICW", "OICW" },
#endif
		{ "MSG90A1", "MSG90A1 Sniper Rifle" },
		{ "M60", "M60 Machinegun" },
		{ "MM1", "MM1 Grenade Launcher" },
		{ "RPG7", "RPG7 Rocket Launcher" },
		{ "M84", "M84 Flash Grenade" },
		{ "SMOHG92", "SMOHG92 Frag Grenade" },
		{ "ANM14", "ANM14 Incendiary Grenade" },
		{ "M15", "M15 Smoke Grenade" },
	};

	len = textlen;
	text = "";

	while ( *src != '\0' && len )
	{
		if ( *src != '#' )
		{
			*dest++ = *src++;
			len--;
			continue;
		}

		src++;

		if ( *src >= '0' && *src <= '9' )
		{
			pos = src;
			i = 0;
			while ( *src >= '0' && *src <= '9' )
			{
				i = i * 10 + (*src++ - '0');
			}
			if ( i < 0 || i > MAX_CLIENTS || !g_entities[i].inuse || !g_entities[i].client || g_entities[i].client->pers.connected != CON_CONNECTED )
			{
				text = "";
				*dest++ = '#';
				src = pos;
			}
			else
			{
				text = va("%s%s^2", level.teamData.teamcolor[g_entities[i].client->sess.team], g_entities[i].client->pers.netname);
			}
		}

		else
		{
			switch ( *src )
			{
				case 'n':
				case 'N':
					text = va("%s%s^2", level.teamData.teamcolor[ent->client->sess.team], ent->client->pers.netname);
					break;
				case 'h':
				case 'H':
					text = va("%d", ent->client->ps.stats[STAT_HEALTH]);
					break;
				case 'a':
				case 'A':
					text = va("%d", ent->client->ps.stats[STAT_ARMOR]);
					break;
				case 'd':
					if ( ent->client->lasthurt_us >= 0 &&
						ent->client->lasthurt_us <= MAX_CLIENTS &&
						g_entities[ent->client->lasthurt_us].inuse &&
						g_entities[ent->client->lasthurt_us].client &&
						g_entities[ent->client->lasthurt_us].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[ent->client->lasthurt_us].client->sess.team], g_entities[ent->client->lasthurt_us].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 'D':
					if ( ent->client->lastkilled_us >= 0 &&
						ent->client->lastkilled_us <= MAX_CLIENTS &&
						g_entities[ent->client->lastkilled_us].inuse &&
						g_entities[ent->client->lastkilled_us].client &&
						g_entities[ent->client->lastkilled_us].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[ent->client->lastkilled_us].client->sess.team], g_entities[ent->client->lastkilled_us].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 't':
					if ( ent->client->lasthurt_client >= 0 &&
						ent->client->lasthurt_client <= MAX_CLIENTS &&
						g_entities[ent->client->lasthurt_client].inuse &&
						g_entities[ent->client->lasthurt_client].client &&
						g_entities[ent->client->lasthurt_client].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[ent->client->lasthurt_client].client->sess.team], g_entities[ent->client->lasthurt_client].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 'T':
					if ( ent->client->lastkilled_client >= 0 &&
						ent->client->lastkilled_client <= MAX_CLIENTS &&
						g_entities[ent->client->lastkilled_client].inuse &&
						g_entities[ent->client->lastkilled_client].client &&
						g_entities[ent->client->lastkilled_client].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[ent->client->lastkilled_client].client->sess.team], g_entities[ent->client->lastkilled_client].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 'l':
				case 'L':
					location[0] = 0;
					Team_GetLocationMsg(ent, location, sizeof(location));
					text = location;
					break;
				case 'f':
				case 'F':
					nearby = G_FindNearbyClient( ent->r.currentOrigin, ent->client->sess.team, 1200, ent );
					if ( nearby != NULL )
					{
						text = va("%s%s^2", level.teamData.teamcolor[nearby->client->sess.team], nearby->client->pers.netname);
					}
					else
					{
						text = "somebody";
					}
					break;
				case 'p':
					i = 8;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 9) & 0x000000ff;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].shortname;
					}
					else {
						text = "gun";
					}
					break;
				case 'P':
					i = 8;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 9) & 0x000000ff;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].longname;
					}
					else {
						text = "gun";
					}
					break;
				case 's':
					i = 4;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 5) & 0x0000000f;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].shortname;
					}
					else {
						text = "gun";
					}
					break;
				case 'S':
					i = 4;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 5) & 0x0000000f;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].longname;
					}
					else {
						text = "gun";
					}
					break;
				case 'i':
					i = 1;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 2) & 0x00000007;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].shortname;
					}
					else {
						text = "gun";
					}
					break;
				case 'I':
					i = 1;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 2) & 0x00000007;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].longname;
					}
					else {
						text = "gun";
					}
					break;
				case 'g':
					i = 16;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 17) & 0x0000000f;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].shortname;
					}
					else {
						text = "gun";
					}
					break;
				case 'G':
					i = 16;
					weaponmask = (ent->client->ps.stats[STAT_WEAPONS] >> 17) & 0x0000000f;
					if ( weaponmask ) {
						while ( !(weaponmask & 1) ) {
							weaponmask >>= 1;
							i++;
						}
						text = weaponlist[i].longname;
					}
					else {
						text = "gun";
					}
					break;
				case 'e':
				case 'E':
					if ( ent->client->ps.stats[STAT_GOGGLES] == 1) {
						text = "NV Goggles";
					}
					else if ( ent->client->ps.stats[STAT_GOGGLES] == 2) {
						text = "Thermal Goggles";
					}
					else {
						text = "Armor";
					}
					break;
				case 'b':
					i = (ent->client->ps.stats[STAT_ARMOR] + 5) / 10;
					if ( i < 0 )
					{
						i = 0;
					}
					bar[0] = '^';
					bar[1] = '4';
					memset( &bar[2], '-', i );
					bar[i + 2] = '^';
					bar[i + 3] = '0';
					memset( &bar[i + 4], '-', 10 - i );
					bar[14] = '^';
					bar[15] = '2';
					bar[16] = 0;
					text = bar;
					break;
				case 'B':
					i = (ent->client->ps.stats[STAT_HEALTH] + 5) / 10;
					if ( i < 0 )
					{
						i = 0;
					}
					bar[0] = '^';
					bar[1] = '1';
					memset( &bar[2], '-', i );
					bar[i + 2] = '^';
					bar[i + 3] = '0';
					memset( &bar[i + 4], '-', 10 - i );
					bar[14] = '^';
					bar[15] = '2';
					bar[16] = 0;
					text = bar;
					break;
				case 'w':
					if ( ent->s.weapon > 0 && ent->s.weapon < 20 ) {
						text = weaponlist[ent->s.weapon - 1].shortname;
					}
					else {
						text = "gun";
					}
					break;
				case 'W':
					if ( ent->s.weapon > 0 && ent->s.weapon < 20 ) {
						text = weaponlist[ent->s.weapon - 1].longname;
					}
					else {
						text = "gun";
					}
					break;
				case 'r':
					if ( ent->client->lasthurt_mod > MOD_UNKNOWN && ent->client->lasthurt_mod < MOD_WATER ) {
						text = weaponlist[ent->client->lasthurt_mod - 1].shortname;
					}
					else {
						text = "gun";
					}
					break;
				case 'R':
					if ( ent->client->lasthurt_mod > MOD_UNKNOWN && ent->client->lasthurt_mod < MOD_WATER ) {
						text = weaponlist[ent->client->lasthurt_mod - 1].longname;
					}
					else {
						text = "gun";
					}
					break;
				case 'u':
					text = G_GetHitLocationText( ent->client->lasthurt_body, qfalse );
					break;
				case 'U':
					text = G_GetHitLocationText( ent->client->lasthurt_body, qtrue );
					break;
				case 'v':
					text = G_GetHitLocationText( ent->client->lastkilled_body, qfalse );
					break;
				case 'V':
					text = G_GetHitLocationText( ent->client->lastkilled_body, qtrue );
					break;
				case 'z':
				case 'Z':
					if ( level.lastPlayer >= 0 &&
						level.lastPlayer <= MAX_CLIENTS &&
						g_entities[level.lastPlayer].inuse &&
						g_entities[level.lastPlayer].client &&
						g_entities[level.lastPlayer].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[level.lastPlayer].client->sess.team], g_entities[level.lastPlayer].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 'c':
					if ( ent->client->sess.team == TEAM_RED )
					{
						i = level.blueFlagCarrier;
					}
					else if ( ent->client->sess.team == TEAM_BLUE )
					{
						i = level.redFlagCarrier;
					}
					else
					{
						i = -1;
					}
					if ( i >= 0 &&
						i <= MAX_CLIENTS &&
						g_entities[i].inuse &&
						g_entities[i].client &&
						g_entities[i].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[i].client->sess.team], g_entities[i].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 'C':
					if ( ent->client->sess.team == TEAM_RED )
					{
						i = level.redFlagCarrier;
					}
					else if ( ent->client->sess.team == TEAM_BLUE )
					{
						i = level.blueFlagCarrier;
					}
					else
					{
						i = -1;
					}
					if ( i >= 0 &&
						i <= MAX_CLIENTS &&
						g_entities[i].inuse &&
						g_entities[i].client &&
						g_entities[i].client->pers.connected == CON_CONNECTED )
					{
						text = va("%s%s^2", level.teamData.teamcolor[g_entities[i].client->sess.team], g_entities[i].client->pers.netname);
					}
					else
					{
						text = "";
					}
					break;
				case 'x':
					if ( ent->client->lasthurtloc_us )
					{
						text = ent->client->lasthurtloc_us->message;
					}
					else
					{
						text = "";
					}
					break;
				case 'X':
					if ( ent->client->lastkilledloc_us )
					{
						text = ent->client->lastkilledloc_us->message;
					}
					else
					{
						text = "";
					}
					break;
				case 'y':
					if ( ent->client->lasthurtloc_client )
					{
						text = ent->client->lasthurtloc_client->message;
					}
					else
					{
						text = "";
					}
					break;
				case 'Y':
					if ( ent->client->lastkilledloc_client )
					{
						text = ent->client->lastkilledloc_client->message;
					}
					else
					{
						text = "";
					}
					break;
				case 'o':
					if ( level.gametypeData->teams )
					{
						if ( ent->client->sess.team == TEAM_RED )
						{
							text = va("%i", level.teamScores[TEAM_RED]);
						}
						else if ( ent->client->sess.team == TEAM_BLUE )
						{
							text = va("%i", level.teamScores[TEAM_BLUE]);
						}
						else
						{
							text = "";
						}
					}
					else
					{
						text = va("%i", level.teamScores[TEAM_FREE]);
					}
					break;
				case 'O':
					if ( level.gametypeData->teams )
					{
						if ( ent->client->sess.team == TEAM_RED )
						{
							text = va("%i", level.teamScores[TEAM_BLUE]);
						}
						else if ( ent->client->sess.team == TEAM_BLUE )
						{
							text = va("%i", level.teamScores[TEAM_RED]);
						}
						else
						{
							text = "";
						}
					}
					else
					{
						text = va("%i", level.teamScores[TEAM_FREE]);
					}
					break;
				case 'k':
					if ( level.gametypeData->teams )
					{
						text = va("%i", level.teamScores[TEAM_RED]);
					}
					else
					{
						text = va("%i", level.teamScores[TEAM_FREE]);
					}
					break;
				case 'K':
					if ( level.gametypeData->teams )
					{
						text = va("%i", level.teamScores[TEAM_BLUE]);
					}
					else
					{
						text = va("%i", level.teamScores[TEAM_FREE]);
					}
					break;
				case '#':
					text = "#";
					break;
				default:
					text = "";
					*dest++ = '#';
					src--;
					break;
			}
			src++;
		}
		while (*text != '\0' && len )
		{
			*dest++ = *text++;
			len--;
		}
	}
	*dest = '\0';
}

/*
==================
G_ParseVoiceCommands
==================
*/
void G_ParseVoiceCommands ( gentity_t *ent, const char *chatText, char *text, int textlen, char *sound )
{
	int len;
	int num;
	char *message;
	const char *src;
	const char *pos;
	char c;
	qboolean dotext;

	dotext = qtrue;
	len = textlen;
	src = chatText;

	while ( *src != '\0' && len )
	{
		if ( *src != '@' && *src != '&' )
		{
			*text++ = *src++;
			len--;
			continue;
		}

		if ( *src == '&' )
		{
			dotext = qfalse;
		}

		src++;

		if ( (*src == '@' && dotext) || (*src == '&' && !dotext) )
		{
			*text++ = *src++;
			len--;
			continue;
		}

		pos = src;
		num = 0;
		c = *src;
		while ( c >= '0' && c <= '9' && len )
		{
			c -= '0';
			num = num * 10 + c;
			src++;
			len--;
			c = *src;
		}

		if (num <= 0 || num > voicecmds.voiceCommandCount || !voicecmds.voiceCommandSound[num][0] )
		{
			*text++ = dotext?'@':'&';
			src = pos;
			continue;
		}

		if ( ent != NULL && ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref != -1 && voicecmds.voiceCommandFlag[num] == VOICE_ADMIN_ONLY )
		{
			*text++ = dotext?'@':'&';
			src = pos;
			continue;
		}
		if ( ent != NULL && !ent->client->sess.modData->clan && voicecmds.voiceCommandFlag[num] == VOICE_CLAN_ONLY )
		{
			*text++ = dotext?'@':'&';
			src = pos;
			continue;
		}
		if ( ent != NULL && ent->client->sess.modData->adminref < 3 && ent->client->sess.modData->adminref > -2 && voicecmds.voiceCommandFlag[num] == VOICE_SYSOP_ONLY )
		{
			*text++ = dotext?'@':'&';
			src = pos;
			continue;
		}
		if ( ent != NULL && ent->client->sess.modData->adminref < 1 && ent->client->sess.modData->adminref > -1 && voicecmds.voiceCommandFlag[num] == VOICE_REFEREE_ONLY )
		{
			*text++ = dotext?'@':'&';
			src = pos;
			continue;
		}

		strcpy( sound, voicecmds.voiceCommandSound[num] );
		message = voicecmds.voiceCommandText[num];

		if ( dotext )
		{
			while (*message != '\0' && len )
			{
				*text++ = *message++;
				len--;
			}
		}
	}
	*text = '\0';
}

/*
==================
G_CheckVoiceFlood
==================
*/
qboolean G_CheckVoiceFlood ( gentity_t *ent ) 
{
	// Voice flooding protection on?
	if ( g_voiceFloodCount.integer )
	{
		// If this client has been penalized for voice chatting to much then dont allow the voice chat
		if ( recondata[ent->s.number].voiceFloodPenalty )
		{
			if ( recondata[ent->s.number].voiceFloodPenalty > level.time )
			{
				return qtrue;
			}

			// No longer penalized
			recondata[ent->s.number].voiceFloodPenalty = 0;
		}

		// See if this client flooded with voice chats
		recondata[ent->s.number].voiceFloodCount++;
		if ( recondata[ent->s.number].voiceFloodCount >= g_voiceFloodCount.integer )
		{
			recondata[ent->s.number].voiceFloodCount = 0;
			recondata[ent->s.number].voiceFloodTimer = 0;
			recondata[ent->s.number].voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;

			trap_SendServerCommand( ent-g_entities, va("print \"Voice chat flooded, you will be able use voice chats again in (%d) seconds\n\"", g_voiceFloodPenalty.integer ) );

			return qtrue;
		}
	}

	return qfalse;
}

/*
==================
G_Say
==================
*/
void G_Say ( gentity_t *ent, gentity_t *target, int mode, const char *chatText, qboolean override ) 
{
	int			j;
	gentity_t	*other;
	char		text[MAX_SAY_TEXT];
	char		text2[MAX_SAY_TEXT];
	char		name[64];
	char		sound[128];

	j = G_ParseTextModifier( ent, chatText );
	if ( j )
	{
		mode = j;
		chatText += 4;
	}

	if ( g_globalMute.integer && !level.warmupTime && !level.intermissiontime && !level.paused && !level.matchPending && !ent->client->sess.modData->moderator )
	{
		return;
	}

	// Logging stuff
	switch ( mode )
	{
		case SAY_ALL:
			G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_TEAM:
			G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_ADMIN_ALL:
			G_LogPrintf( "sayadminall: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_ADMIN_ONLY:
			G_LogPrintf( "sayadminonly: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_REF_ALL:
			G_LogPrintf( "sayrefall: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_REF_ONLY:
			G_LogPrintf( "sayrefonly: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_CLAN_ALL:
			G_LogPrintf( "sayclanall: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_CLAN_ONLY:
			G_LogPrintf( "sayclanonly: %s: %s\n", ent->client->pers.netname, chatText );
			break;
	}

	// Generate the chat prefix
	G_GetChatPrefix ( ent, target, mode, name, sizeof(name)-1 );

	// Parse any voice commands
	sound[0] = 0;
	G_ParseVoiceCommands( ent, chatText, text2, sizeof(text2)-1, sound );

	// Filter out any banned words
	if ( g_filterChat.integer )
	{
		G_FilterChatText( text2 );
	}

	if ( sound[0] && G_CheckVoiceFlood( ent ) )
	{
		sound[0] = 0;
	}

	// Parse any text tokens
	G_ParseTextTokens( ent, text2, text, sizeof(text)-1 );

//	// Save off the chat text
//	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) 
	{
		G_SayTo( ent, target, mode, name, text, sound, override );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) 
	{
		Com_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.numConnectedClients; j++) 
	{
		other = &g_entities[level.sortedClients[j]];
		G_SayTo( ent, other, mode, name, text, sound, override );
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p, qfalse );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p, qfalse );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p, qfalse );
	}
}

/*
==================
Cmd_AdmTell_f
==================
*/
static void Cmd_AdmTell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_ADMIN_TELL, p, qfalse );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_ADMIN_TELL, p, qfalse );
	}
}

static void G_VoiceTo ( gentity_t *ent, gentity_t *other, int mode, const char* name, const char *id, qboolean voiceonly ) 
{
	// Only team say is supported right now for voice chatting
	if (mode != SAY_TEAM) 
	{
		return;
	}

	if (!other || !other->inuse || !other->client) 
	{
		return;
	}

	if ( !OnSameTeam(ent, other) ) 
	{
		return;
	}

	trap_SendServerCommand( other-g_entities, va("%s %d %d \"%s\" \"%s\"", "vtchat", voiceonly, ent->s.number, name, id));
}

/*
==================
G_CanVoiceGlobal

Can we globaly speak right now
==================
*/
qboolean G_CanVoiceGlobal ( void )
{
	if ( level.gametypeData->teams && level.time - level.globalVoiceTime > 5000 )
	{
		return qtrue;
	}

	return qfalse;
}

/*
==================
G_VoiceGlobal

says something out loud that everyone in the radius can hear
==================
*/
void G_VoiceGlobal ( gentity_t* ent, const char* id, qboolean force )
{
	if ( !ent )
	{
		return;
	}

	if ( !level.gametypeData->teams )
	{
		return;
	}

	if ( !force && level.time - level.globalVoiceTime < 5000 )
	{
		return;
	}
		
	level.globalVoiceTime = level.time;

	trap_SendServerCommand( -1, va("vglobal %d \"%s\"", ent->s.number, id));
}

/*
==================
G_Voice
==================
*/
void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) 
{
	int			j;
	gentity_t	*other;
	char		name[MAX_SAY_TEXT];

	// Spectators and ghosts dont talk
	if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
	{
		return;
	}

	if ( G_CheckVoiceFlood( ent ) )
	{
		return;
	}

	G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

	if ( target ) 
	{
		G_VoiceTo( ent, target, mode, name, id, voiceonly );
		return;
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) 
	{
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, name, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) 
{
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) 
{
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

/*
==================
G_IsSysopPresent
==================
*/
qboolean G_IsSysopPresent( void ) 
{
	int			i;
	gentity_t	*ent;

	for (i = 0; i < level.numConnectedClients; i++) 
	{
		ent = &g_entities[level.sortedClients[i]];
		if ( ent->client->sess.modData->adminref == 3 )
		{
			return qtrue;
		}
	}
	return qfalse;
}

/*
==================
G_IsAdminPresent
==================
*/
qboolean G_IsAdminPresent( void ) 
{
	int			i;
	gentity_t	*ent;

	for (i = 0; i < level.numConnectedClients; i++) 
	{
		ent = &g_entities[level.sortedClients[i]];
		if ( ent->client->sess.modData->adminref == 2 || ent->client->sess.modData->adminref == -1 )
		{
			return qtrue;
		}
	}
	return qfalse;
}

/*
============
G_VoteDisabled

determines if the given vote is disabled
============
*/
int G_VoteDisabled ( const char* callvote ) 
{
	return trap_Cvar_VariableIntegerValue( va("novote_%s", callvote) );
}

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) 
{
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	arg3[MAX_STRING_TOKENS];

	if ( (ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref > -1) || !g_adminVote.integer )
	{
		if ( !g_allowVote.integer ) 
		{
			trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
			return;
		}

		if ( level.intermissiontime || level.intermissionQueued )
		{
			trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed during intermission.\n\"" );
			return;
		}

		// No voting within the minute of a map change
		if ( level.time - level.startTime < 1000 * 60 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"Cannot vote within the first minute of a map change.\n\"" );
			return;
		}

		if ( level.numConnectedClients > 1 && level.numVotingClients == 1 ) 
		{
			trap_SendServerCommand( ent-g_entities, "print \"You need at least 2 clients to call a vote.\n\"" );
			return;
		}

		if ( recondata[ent->s.number].voteCount >= g_voteLimit.integer ) 
		{
			trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
			return;
		}
	
		if ( ent->client->sess.team == TEAM_SPECTATOR ) 
		{
			trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
			return;
		}

		if ( recondata[ent->s.number].voteDelayTime > level.time )
		{
			if ( recondata[ent->s.number].attributes & RECON_NOVOTE )
			{
				trap_SendServerCommand( ent-g_entities, "print \"Your voting rights have been temporarily suspended.\n\"" );
				return;
			}
			else
			{
				trap_SendServerCommand( ent-g_entities, va("print \"You are not allowed to vote within %d minute of a failed vote.\n\"", g_failedVoteDelay.integer ) );
				return;
			}
		}
		recondata[ent->s.number].attributes &= (~RECON_NOVOTE);
	}

	if ( level.voteTime ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
		return;
	}

	// Save the voting client id
	level.voteClient = ent->s.number;

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	trap_Argv( 3, arg3, sizeof( arg3 ) );

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "mapcycle" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "rmgmap" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "g_friendlyfire" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "timeextension" ) ) {
	} else if ( !Q_stricmp( arg1, "scorelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "endmap" ) ) {
	} else if ( !Q_stricmp( arg1, "shuffleteams" ) ) {
	} else if ( !Q_stricmp( arg1, "swapteams" ) ) {
	} else if ( !Q_stricmp( arg1, "referee" ) ) {
	} else if ( !Q_stricmp( arg1, "poll" ) ) {
	} else if ( !Q_stricmp( arg1, "mute" ) ) {
	} else if ( !Q_stricmp( arg1, "callvote" ) ) {
	} else if ( !Q_stricmp( arg1, "clanvsall" ) ) {
	} else
	{
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, scorelimit <score>, endmap, shuffleteams, swapteams, referee <client id>, poll <text>, mute, clanvsall <team>.\n\"" );
		return;
	}

	// see if this particular vote is disabled
	if ( G_VoteDisabled ( arg1 ) && ((ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref > -1) || !g_adminVote.integer) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"The '%s' vote has been disabled on this server.\n\"", arg1) );
		return;
	}	

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) 
	{
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) ) 
	{
		// Verify the gametype
		i = BG_FindGametype ( arg2 );
		if ( i < 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}	

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, bg_gametypeData[i].name );
	} 
	else if ( !Q_stricmp( arg1, "map" ) ) 
	{
		if ( !G_DoesMapExist ( arg2 ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"Unknown mapname.\n\"" );
			return;
		}				

		if ( strlen( arg3 ) > 0 )
		{
			Com_sprintf( level.voteString, sizeof( level.voteString ), "gmap %s %s", arg1, arg2, arg3 );
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s (%s)", arg2, arg3 );
		}
		else
		{
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		}
	} 
	else if ( !Q_stricmp( arg1, "rmgmap" ) ) 
	{
		char	arg3[MAX_STRING_TOKENS];
		char	arg4[MAX_STRING_TOKENS];

		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_Argv( 4, arg4, sizeof( arg4 ) );

		Com_sprintf( level.voteString, sizeof( level.voteString ), "rmgmap 1 %s 2 %s 3 %s 4 \"%s\" 0", arg2, arg3, arg4, ConcatArgs ( 5 ) );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Randomly Generated Map" );
	}
	else if ( !Q_stricmp( arg1, "mapcycle" ) ) 
	{
		if (!*g_mapcycle.string || !Q_stricmp ( g_mapcycle.string, "none" ) ) 
		{
			trap_SendServerCommand( ent-g_entities, "print \"there is no map cycle currently set up.\n\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "mapcycle");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "nextmap" );
	} 
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		int n = atoi ( arg2 );

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->sess.modData->adminref > 1 || g_entities[n].client->sess.modData->adminref < 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"You can't vote kick an admin.\n\"" );
			return;
		}

		if ( g_protectClan.integer && g_entities[n].client->sess.modData->clan )
		{
			trap_SendServerCommand( ent-g_entities, "print \"You can't vote kick a clan member.\n\"" );
			return;
		}

		if ( g_voteKickBanTime.integer )
		{
			Com_sprintf ( level.voteString, sizeof(level.voteString ), "banclient %s %d voted off server", arg2, g_voteKickBanTime.integer );
		}
		else
		{
			Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %s", arg2 );
		}

		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s  ^3[^1tk: %i^3] [^5idle: %i sec^3]", g_entities[n].client->pers.netname, g_teamkillDamageMax.integer ? 100 * g_entities[n].client->sess.teamkillDamage / g_teamkillDamageMax.integer : 0, (level.time - g_entities[n].client->idleTime) / 1000 );
	}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		int clientid = G_ClientNumberFromName ( arg2 );

		if ( clientid == -1 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
			return;
		}

		if ( g_entities[clientid].client->sess.modData->adminref > 1 || g_entities[clientid].client->sess.modData->adminref < 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"You can't vote kick an admin.\n\"" );
			return;
		}

		if ( g_protectClan.integer && g_entities[clientid].client->sess.modData->clan )
		{
			trap_SendServerCommand( ent-g_entities, "print \"You can't vote kick a clan member.\n\"" );
			return;
		}

		if ( g_voteKickBanTime.integer )
		{
			Com_sprintf ( level.voteString, sizeof(level.voteString ), "banclient %d %d voted off server", clientid, g_voteKickBanTime.integer );
		}
		else
		{
			Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		}

		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s  ^3[^1tk: %i^3] [^5idle: %i sec^3]", g_entities[clientid].client->pers.netname, g_teamkillDamageMax.integer ? 100 * g_entities[clientid].client->sess.teamkillDamage / g_teamkillDamageMax.integer : 0, (level.time - g_entities[clientid].client->idleTime) / 1000 );
	}
	else if ( !Q_stricmp ( arg1, "timeextension" ) )
	{
		if ( !g_timelimit.integer )
		{
			trap_SendServerCommand( ent-g_entities, "print \"There is no timelimit to extend.\n\"" );
			return;
		}

		if ( !g_timeextension.integer )
		{
			trap_SendServerCommand( ent-g_entities, "print \"This server does not allow time extensions.\n\"" );
			return;
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "extendtime %d", g_timeextension.integer );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "extend timelimit by %d minutes", g_timeextension.integer );
	}
	else if ( !Q_stricmp ( arg1, "referee" ) )
	{
		if ( g_entities[atoi(arg2)].client->sess.modData->adminref != 0 ) {
			trap_SendServerCommand( ent-g_entities, "print \"Player is already priviledged.\n\"");
			return;
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "referee %d", atoi(arg2) );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "give referee status to %s", g_entities[atoi(arg2)].client->pers.netname );
	}
	else if ( !Q_stricmp ( arg1, "endmap" ) )
	{
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s", arg1 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", arg1 );		
	}
	else if ( !Q_stricmp ( arg1, "shuffleteams" )    ||
			  !Q_stricmp ( arg1, "swapteams" )  )
	{
		if ( level.teamRedLocked || level.teamBlueLocked )
		{
			trap_SendServerCommand( ent-g_entities, "print \"Can't adjust while teams are locked.\n\"");
			return;
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s", arg1 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", arg1 );
	}
	else if ( !Q_stricmp ( arg1, "poll" ) )
	{
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s", arg1 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "%s", ConcatArgs(2) );
	}
	else if ( !Q_stricmp ( arg1, "mute" ) )
	{
		int id;

		id = atoi(arg2);
		if ( g_entities[id].client->sess.modData->adminref > 1 || g_entities[id].client->sess.modData->adminref < 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"You can't vote mute an admin.\n\"" );
			return;
		}

		if ( g_entities[id].client->sess.muted ) {
			Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "unmute %s", g_entities[id].client->pers.netname );
		} else {
			Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "mute %s", g_entities[id].client->pers.netname );
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "mute %d", atoi(arg2) );
	}
	else if ( !Q_stricmp ( arg1, "clanvsall" ) )
	{
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "clanvsall %s", arg2 );
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clanvsall %s", arg2 );
	}
	else if ( !Q_stricmp ( arg1, "timelimit" )     ||
			  !Q_stricmp ( arg1, "scorelimit" )    ||
			  !Q_stricmp ( arg1, "map_restart" )   ||  
			  !Q_stricmp ( arg1, "g_doWarmup" )    ||  
			  !Q_stricmp ( arg1, "g_friendlyfire" )  )
	{
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %d", arg1, atoi(arg2) );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );		
	}
	else 
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client->pers.netname ) );
	recondata[level.voteClient].voteCount++;

	if ( voicecmds.voicePromptSound[16][0] )
	{
		G_BroadcastSound( voicecmds.voicePromptSound[16] );
	}

	level.numVotesNeeded = (level.numVotingClients / 2) + 1;

	// start the voting, the caller autoamtically votes yes
	if ( ent->client->sess.modData->adminref > 1 && g_adminVote.integer == 2 )
	{
		level.voteYes = level.numVotesNeeded * 2;
	}
	else
	{
		level.voteYes = 1;
	}
	level.voteTime = level.time;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i,%i", level.voteTime, g_voteDuration.integer*1000 ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	trap_SetConfigstring( CS_VOTE_NEEDED, va("%i", level.numVotesNeeded ) );

	G_LogPrintf("Vote: %s\n", level.voteDisplayString );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) 
{
	char msg[64];

	if ( !level.voteTime ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		return;
	}

	if ( ent->client->ps.eFlags & EF_VOTED ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		return;
	}

	if ( ent->client->sess.team == TEAM_SPECTATOR ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) 
	{
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} 
	else 
	{
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
=================
Cmd_Ignore_f
=================
*/
void Cmd_Ignore_f( gentity_t *ent ) 
{
	char buffer[MAX_TOKEN_CHARS];
	int  ignoree;
	qboolean ignore;

	trap_Argv( 1, buffer, sizeof( buffer ) );

	ignoree = atoi( buffer );

	if ( !CheckID( ent, ignoree ) ) {
//	if ( ignoree < 0 || ignoree > MAX_CLIENTS || !g_entities[ignoree].inuse || !g_entities[ignoree].client ) {
		G_PrintMessage( ent, "^1>>^7 Invalid player id.\n");
		return;
	}

	ignore = G_IsClientChatIgnored ( ent->s.number, ignoree ) ? qfalse : qtrue;

	if ( ignoree == ent->s.number )
	{
		trap_SendServerCommand( ent-g_entities, "print \"cant ignore yourself.\n\"");
		return;
	}	

	G_IgnoreClientChat ( ent->s.number, ignoree, ignore);

	if ( ignore )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s ignored.\n\"", g_entities[ignoree].client->pers.netname));
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s unignored.\n\"", g_entities[ignoree].client->pers.netname));
	}		
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) 
{
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return;
	}
	
	if ( trap_Argc() != 5 ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"usage: setviewpos x y z yaw\n\"");
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) 
	{
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

/*
==================
Cmd_ClientVerified_f
==================
*/
void Cmd_ClientVerified_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	int				value;

	if ( trap_Argc() == 1 ) {
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	value = atoi( buffer );

	if ( value < ROCMOD_VER ) {
		return;
	}

	G_LogPrintf( "Client Verified: %i\n", clientNum );
	ent->client->sess.modData->versionVerified = qtrue;
}

/*
==================
Cmd_ClientUpdate_f
==================
*/
void Cmd_ClientUpdate_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];

	if ( trap_Argc() == 1 ) {
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	ent->client->sess.modData->extraFeatures = atoi( buffer );

	trap_SendServerCommand( ent-g_entities, va("efack %i", ent->client->sess.modData->extraFeatures) );
	if ( ent->client->sess.modData->extraFeatures & EXTRA_TEAMINFO )
	{
		G_SendExtraTeamInfo( ent );
	}
}

/*
==================
Cmd_ClientCvarVal_f
==================
*/
void Cmd_ClientCvarVal_f( gentity_t *ent, int clientNum )
{
	char		buffer[MAX_TOKEN_CHARS];
	int			value;
	gentity_t	*id_ent;
	int			id;
	int i;
	
	char	*text;

	if ( trap_Argc() < 4 ) {
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( id != 999 ) {
		id_ent = g_entities + id;
	} else {
		id_ent = 0;
	}

	trap_Argv( 2, buffer, sizeof( buffer ) );
	text = ConcatArgs( 3 );
	G_PrintMessage( id_ent, va("^4>>^7 Cvar response from %s^7: ^4[^7%s^4]^2=^3[^7%s^3]\n", g_entities[clientNum].client->pers.netname, buffer, text) );

//	i = trap_Cvar_VariableIntegerValue( "sv_maxrate" );
//	G_PrintMessage( ent, va("Current max rate: %i\n", i) );
}

/*
==================
Cmd_Motd_f
==================
*/
void Cmd_Motd_f( gentity_t *ent, int clientNum )
{
	SendMOTD( ent );
//	trap_SendServerCommand( ent-g_entities, va("cp \"^2Soldier of Fortune II\n^1ROCmod %s - %s\n\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n\n\"", ROCMOD_VERSION, ROCMOD_DATE, g_motd1.string, g_motd2.string, g_motd3.string, g_motd4.string, g_motd5.string, g_motd6.string, g_motd7.string, g_motd8.string, g_motd9.string, g_motd10.string) );
}

/*
==================
Cmd_AdminCommand_f
==================
*/
void Cmd_AdminCommand_f( gentity_t *ent, int clientNum, int type )
{
	char	cmd[MAX_TOKEN_CHARS];

	if ( trap_Argc() == 1 ) {
		G_PrintMessage( ent, "^3Available commands:\n");
		if ( !ent || ent->client->sess.modData->adminref > 1 )
		{
			G_PrintMessage( ent, " ^7suspend      ^6- Toggle your admin status on or off\n");
			if ( g_allowSuspendAll.string[type] != '0' )	G_PrintMessage( ent, " ^7suspendall   ^6- Force all admins to suspend their admin status\n");
		}

		if ( g_allowTalk.string[type] != '0' )			G_PrintMessage( ent, " ^7talk         ^2- Send a message to all players\n");
		if ( g_allowChat.string[type] != '0' )			G_PrintMessage( ent, " ^7chat         ^2- Send a message to your peers only\n");

		if ( g_allowKick.string[type] != '0' )			G_PrintMessage( ent, " ^7kick         ^3- Kick a player off the server\n");
		if ( g_allowBan.string[type] != '0' )			G_PrintMessage( ent, " ^7ban          ^3- Kick a player off the server for a length of time\n");
		if ( g_allowCancelVote.string[type] != '0' )	G_PrintMessage( ent, " ^7cancelvote   ^3- Cancel the current vote\n");
		if ( g_allowPassVote.string[type] != '0' )		G_PrintMessage( ent, " ^7passvote     ^3- Passes the current vote\n");
		if ( g_allowGametype.string[type] != '0' )		G_PrintMessage( ent, " ^7gametype     ^3- Sets the game type\n");
		if ( g_allowMapRestart.string[type] != '0' )	G_PrintMessage( ent, " ^7restart      ^3- Restarts the current map\n");
		if ( g_allowMapcycle.string[type] != '0' )		G_PrintMessage( ent, " ^7mapcycle     ^3- Switches to the next map\n");
		if ( g_allowSetMap.string[type] != '0' )		G_PrintMessage( ent, " ^7map          ^3- Switches to a specific map\n");
		if ( g_allowWarmup.string[type] != '0' )		G_PrintMessage( ent, " ^7warmup       ^3- Turns warmup on or off\n");
		if ( g_allowFriendlyFire.string[type] != '0' )	G_PrintMessage( ent, " ^7friendlyfire ^3- Turns friendly fire on or off\n");
		if ( g_allowExtendTime.string[type] != '0' )	G_PrintMessage( ent, " ^7extendtime   ^3- Extends the timelimit by a certain amount\n");
		if ( g_allowTimelimit.string[type] != '0' )		G_PrintMessage( ent, " ^7timelimit    ^3- Sets the timelimit to a specific amount\n");
		if ( g_allowScorelimit.string[type] != '0' )	G_PrintMessage( ent, " ^7scorelimit   ^3- Sets the scorelimit to a specific amount\n");
		if ( g_allowEndMap.string[type] != '0' )		G_PrintMessage( ent, " ^7endmap       ^3- Ends the current map and moves everyone to intermission\n");
		if ( g_allowPause.string[type] != '0' )			G_PrintMessage( ent, " ^7pause        ^3- Pauses the game\n");
		if ( g_allowUnPause.string[type] != '0' )		G_PrintMessage( ent, " ^7unpause      ^3- Unpauses the game\n");
		if ( g_allowFreezeMap.string[type] != '0' )		G_PrintMessage( ent, " ^7freezemap    ^3- Freezes or unfreezes the current map in the mapcycle\n");
		if ( g_allowMatch.string[type] != '0' )			G_PrintMessage( ent, " ^7startmatch   ^3- Begins a match, with special server settings\n");
		if ( g_allowMatch.string[type] != '0' )			G_PrintMessage( ent, " ^7endmatch     ^3- Ends the current match, returning to normal settings\n");
		if ( g_allowMatch.string[type] != '0' )			G_PrintMessage( ent, " ^7forcematch   ^3- Forces a match to start, even if all players aren't ready\n");
		if ( g_allowCfg.string[type] != '0' )			G_PrintMessage( ent, " ^7cfg          ^3- Forces the server to apply a specific server cfg\n");
		if ( g_allowPassword.string[type] != '0' )		G_PrintMessage( ent, " ^7password     ^3- Changes the server password\n");
		if ( g_allowReset.string[type] != '0' )			G_PrintMessage( ent, " ^7reset        ^3- Resets all gametype items\n");

		if ( g_allowEvenTeams.string[type] != '0' )		G_PrintMessage( ent, " ^7eventeams    ^5- Evens the teams\n");
		if ( g_allowSwapTeams.string[type] != '0' )		G_PrintMessage( ent, " ^7swapteams    ^5- Swaps the teams\n");
		if ( g_allowShuffleTeams.string[type] != '0' )	G_PrintMessage( ent, " ^7shuffleteams ^5- Randomly mixes up the teams\n");
		if ( g_allowSwitch.string[type] != '0' )		G_PrintMessage( ent, " ^7forceteam    ^5- Force a player onto a particular team\n");
		if ( g_allowSwitch.string[type] != '0' )		G_PrintMessage( ent, " ^7switch       ^5- Move a player onto the opposite team\n");
		if ( g_allowSwap.string[type] != '0' )			G_PrintMessage( ent, " ^7swap         ^5- Swaps two players on opposite teams\n");
		if ( g_allowLockTeam.string[type] != '0' )		G_PrintMessage( ent, " ^7lock         ^5- Locks or unlocks a team\n");
		if ( g_allowInvite.string[type] != '0' )		G_PrintMessage( ent, " ^7invite       ^5- Invites a player to join or spectate a locked team\n");
		if ( g_allowClanVsAll.string[type] != '0' )		G_PrintMessage( ent, " ^7clanvsall    ^5- Rearranges teams to put all clan members on one side\n");
		if ( g_allowTag.string[type] != '0' )			G_PrintMessage( ent, " ^7tag          ^5- Tags specific players for subsequent commands\n");
		if ( g_allowTag.string[type] != '0' )			G_PrintMessage( ent, " ^7swaptags     ^5- Swaps the tags on all players\n");
		if ( g_allowTag.string[type] != '0' )			G_PrintMessage( ent, " ^7cleartags    ^5- Clears all player tags\n");
		if ( g_allowTagVsAll.string[type] != '0' )		G_PrintMessage( ent, " ^7tagvsall     ^5- Rearranges teams to put all tagged players on one side\n");

		if ( g_allowMute.string[type] != '0' )			G_PrintMessage( ent, " ^7mute         ^1- Mute or unmute a player\n");
		if ( g_allowPenalty.string[type] != '0' )		G_PrintMessage( ent, " ^7penalty      ^1- Throw a player in the penalty box\n");
		if ( g_allowUnPenalty.string[type] != '0' )		G_PrintMessage( ent, " ^7unpenalty    ^1- Pull a player out of the penalty box prematurely\n");
		if ( g_allowStrike.string[type] != '0' )		G_PrintMessage( ent, " ^7strike       ^1- Kill a player\n");
		if ( g_allowSlap.string[type] != '0' )			G_PrintMessage( ent, " ^7slap         ^1- Knock a player back with a taunt\n");
		if ( g_allowStrip.string[type] != '0' )			G_PrintMessage( ent, " ^7strip        ^1- Strip a player of all weapons\n");
		if ( g_allowFry.string[type] != '0' )			G_PrintMessage( ent, " ^7fry          ^1- Set a player on fire\n");
		if ( g_allowLaunch.string[type] != '0' )		G_PrintMessage( ent, " ^7launch       ^1- Tosses a player up into the air\n");
		if ( g_allowExplode.string[type] != '0' )		G_PrintMessage( ent, " ^7explode      ^1- Detonates a player from within\n");
		if ( g_allowPlant.string[type] != '0' )			G_PrintMessage( ent, " ^7plant        ^1- Immobilizes a player\n");
		if ( g_allowTelefrag.string[type] != '0' )		G_PrintMessage( ent, " ^7telefrag     ^1- Kills a player by telefragging them\n");
		if ( g_allowSurrender.string[type] != '0' )		G_PrintMessage( ent, " ^7surrender    ^1- Strips a player of all weapons and puts their hands up\n");
		if ( g_allowRespawn.string[type] != '0' )		G_PrintMessage( ent, " ^7respawn      ^1- Forces a player back to the respawn area\n");
		if ( g_allowBait.string[type] != '0' )			G_PrintMessage( ent, " ^7bait         ^1- Plants a player in the enemy spawn, defenseless\n");
		if ( g_allowForceSay.string[type] != '0' )		G_PrintMessage( ent, " ^7forcesay     ^1- Forces a player to say something\n");
		if ( g_allowDummy.string[type] != '0' )			G_PrintMessage( ent, " ^7dummy        ^1- Marks or unmarks a player as a free-kill target dummy\n");
		if ( g_allowNoVote.string[type] != '0' )		G_PrintMessage( ent, " ^7novote       ^1- Prevents a player from calling votes for a period of time\n");
		if ( g_allowRename.string[type] != '0' )		G_PrintMessage( ent, " ^7rename       ^1- Changes a player's name\n");
		if ( g_allowCripple.string[type] != '0' )		G_PrintMessage( ent, " ^7cripple      ^1- Weakens a player's health and damage\n");
		if ( g_allowBackfire.string[type] != '0' )		G_PrintMessage( ent, " ^7backfire     ^1- Causes a player's own bullets to turn against him\n");

		if ( g_allowPbKick.string[type] != '0' )		G_PrintMessage( ent, " ^7pbkick       ^6- Kick a player for a length of time (using PunkBuster)\n");
		if ( g_allowPbBan.string[type] != '0' )			G_PrintMessage( ent, " ^7pbban        ^6- Permanently ban a player (using PunkBuster)\n");
		if ( g_allowPbGetSS.string[type] != '0' )		G_PrintMessage( ent, " ^7pbgetss      ^6- Take a screenshot of a player (using PunkBuster)\n");
		if ( g_allowCvarCheck.string[type] != '0' )		G_PrintMessage( ent, " ^7cvarcheck    ^6- Displays the value of a player's cvar\n");
//		if ( g_allowCvarSet.string[type] != '0' )		G_PrintMessage( ent, " ^7cvarset      ^6- Changes the value of a player's cvar\n");

		return;
	}
	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( ent )
	{
		// Type 1 = Admin
		if ( type == 1 && ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref != -1 ) {
			G_PrintMessage( ent, "You are not an admin.\n");
			return;
		}
		// Type 2 = Referee
		if ( type == 2 && ent->client->sess.modData->adminref < 1 ) {
			G_PrintMessage( ent, "You are not a referee.\n");
			return;
		}
		// Type 3 = Clan
		if ( type == 3 && !ent->client->sess.modData->clan ) {
			G_PrintMessage( ent, "You are not a clan member.\n");
			return;
		}
	}

	if ( Q_stricmp ( cmd, "talk" ) == 0 )
		AdminCmd_Talk( ent, type );
	else if ( Q_stricmp ( cmd, "chat" ) == 0 )
		AdminCmd_Chat( ent, type );
	else if ( Q_stricmp ( cmd, "kick" ) == 0 )
		AdminCmd_Kick( ent, type );
	else if ( Q_stricmp ( cmd, "ban" ) == 0 )
		AdminCmd_Ban( ent, type );
	else if ( Q_stricmp ( cmd, "cancelvote" ) == 0 )
		AdminCmd_CancelVote( ent, type );
	else if ( Q_stricmp ( cmd, "passvote" ) == 0 )
		AdminCmd_PassVote( ent, type );
	else if ( Q_stricmp ( cmd, "mute" ) == 0 )
		AdminCmd_Mute( ent, type );
	else if ( Q_stricmp ( cmd, "suspend" ) == 0 )
		AdminCmd_Suspend( ent, type );
	else if ( Q_stricmp ( cmd, "pause" ) == 0 )
		AdminCmd_Pause( ent, type );
	else if ( Q_stricmp ( cmd, "unpause" ) == 0 )
		AdminCmd_UnPause( ent, type );
	else if ( Q_stricmp ( cmd, "strike" ) == 0 )
		AdminCmd_Strike( ent, type );
	else if ( Q_stricmp ( cmd, "slap" ) == 0 )
		AdminCmd_Slap( ent, type );
	else if ( Q_stricmp ( cmd, "strip" ) == 0 )
		AdminCmd_Strip( ent, type );
	else if ( Q_stricmp ( cmd, "forceteam" ) == 0 )
		AdminCmd_ForceTeam( ent, type );
	else if ( Q_stricmp ( cmd, "switch" ) == 0 )
		AdminCmd_Switch( ent, type );
	else if ( Q_stricmp ( cmd, "penalty" ) == 0 )
		AdminCmd_Penalty( ent, type );
	else if ( Q_stricmp ( cmd, "unpenalty" ) == 0 )
		AdminCmd_UnPenalty( ent, type );
	else if ( Q_stricmp ( cmd, "gametype" ) == 0 )
		AdminCmd_Gametype( ent, type );
	else if ( Q_stricmp ( cmd, "restart" ) == 0 )
		AdminCmd_MapRestart( ent, type );
	else if ( Q_stricmp ( cmd, "mapcycle" ) == 0 )
		AdminCmd_Mapcycle( ent, type );
	else if ( Q_stricmp ( cmd, "map" ) == 0 )
		AdminCmd_SetMap( ent, type );
	else if ( Q_stricmp ( cmd, "warmup" ) == 0 )
		AdminCmd_Warmup( ent, type );
	else if ( Q_stricmp ( cmd, "friendlyfire" ) == 0 )
		AdminCmd_FriendlyFire( ent, type );
	else if ( Q_stricmp ( cmd, "extendtime" ) == 0 )
		AdminCmd_ExtendTime( ent, type );
	else if ( Q_stricmp ( cmd, "timelimit" ) == 0 )
		AdminCmd_Timelimit( ent, type );
	else if ( Q_stricmp ( cmd, "scorelimit" ) == 0 )
		AdminCmd_Scorelimit( ent, type );
	else if ( Q_stricmp ( cmd, "swapteams" ) == 0 )
		AdminCmd_SwapTeams( ent, type );
	else if ( Q_stricmp ( cmd, "shuffleteams" ) == 0 )
		AdminCmd_ShuffleTeams( ent, type );
	else if ( Q_stricmp ( cmd, "endmap" ) == 0 )
		AdminCmd_EndMap( ent, type );
	else if ( Q_stricmp ( cmd, "lock" ) == 0 )
		AdminCmd_LockTeam( ent, type );
	else if ( Q_stricmp ( cmd, "suspendall" ) == 0 )
		AdminCmd_SuspendAll( ent, type );
	else if ( Q_stricmp ( cmd, "fry" ) == 0 )
		AdminCmd_Fry( ent, type );
	else if ( Q_stricmp ( cmd, "launch" ) == 0 )
		AdminCmd_Launch( ent, type );
	else if ( Q_stricmp ( cmd, "explode" ) == 0 )
		AdminCmd_Explode( ent, type );
	else if ( Q_stricmp ( cmd, "plant" ) == 0 )
		AdminCmd_Plant( ent, type );
	else if ( Q_stricmp ( cmd, "telefrag" ) == 0 )
		AdminCmd_Telefrag( ent, type );
	else if ( Q_stricmp ( cmd, "surrender" ) == 0 )
		AdminCmd_Surrender( ent, type );
	else if ( Q_stricmp ( cmd, "respawn" ) == 0 )
		AdminCmd_Respawn( ent, type );
	else if ( Q_stricmp ( cmd, "bait" ) == 0 )
		AdminCmd_Bait( ent, type );
	else if ( Q_stricmp ( cmd, "eventeams" ) == 0 )
		AdminCmd_EvenTeams( ent, type );
	else if ( Q_stricmp ( cmd, "invite" ) == 0 )
		AdminCmd_Invite( ent, type );
	else if ( Q_stricmp ( cmd, "forcesay" ) == 0 )
		AdminCmd_ForceSay( ent, type );
	else if ( Q_stricmp ( cmd, "swap" ) == 0 )
		AdminCmd_Swap( ent, type );
	else if ( Q_stricmp ( cmd, "clanvsall" ) == 0 )
		AdminCmd_ClanVsAll( ent, type );
	else if ( Q_stricmp ( cmd, "pbkick" ) == 0 )
		AdminCmd_PbKick( ent, type );
	else if ( Q_stricmp ( cmd, "pbban" ) == 0 )
		AdminCmd_PbBan( ent, type );
	else if ( Q_stricmp ( cmd, "pbgetss" ) == 0 )
		AdminCmd_PbGetSS( ent, type );
	else if ( Q_stricmp ( cmd, "freezemap" ) == 0 )
		AdminCmd_FreezeMap( ent, type );
	else if ( Q_stricmp ( cmd, "startmatch" ) == 0 )
		AdminCmd_StartMatch( ent, type );
	else if ( Q_stricmp ( cmd, "endmatch" ) == 0 )
		AdminCmd_EndMatch( ent, type );
	else if ( Q_stricmp ( cmd, "forcematch" ) == 0 )
		AdminCmd_ForceMatch( ent, type );
	else if ( Q_stricmp ( cmd, "dummy" ) == 0 )
		AdminCmd_Dummy( ent, type );
	else if ( Q_stricmp ( cmd, "novote" ) == 0 )
		AdminCmd_NoVote( ent, type );
	else if ( Q_stricmp ( cmd, "rename" ) == 0 )
		AdminCmd_Rename( ent, type );
	else if ( Q_stricmp ( cmd, "cvarcheck" ) == 0 )
		AdminCmd_CvarCheck( ent, type );
//	else if ( Q_stricmp ( cmd, "cvarset" ) == 0 )
//		AdminCmd_CvarSet( ent, type );
	else if ( Q_stricmp ( cmd, "cfg" ) == 0 )
		AdminCmd_Cfg( ent, type );
	else if ( Q_stricmp ( cmd, "tag" ) == 0 )
		AdminCmd_Tag( ent, type );
	else if ( Q_stricmp ( cmd, "swaptags" ) == 0 )
		AdminCmd_SwapTags( ent, type );
	else if ( Q_stricmp ( cmd, "cleartags" ) == 0 )
		AdminCmd_ClearTags( ent, type );
	else if ( Q_stricmp ( cmd, "tagvsall" ) == 0 )
		AdminCmd_TagVsAll( ent, type );
	else if ( Q_stricmp ( cmd, "cripple" ) == 0 )
		AdminCmd_Cripple( ent, type );
	else if ( Q_stricmp ( cmd, "backfire" ) == 0 )
		AdminCmd_Backfire( ent, type );
	else if ( Q_stricmp ( cmd, "password" ) == 0 )
		AdminCmd_Password( ent, type );
	else
		G_PrintMessage( ent, va("Unknown command: %s\n", cmd ) );
}

/*
==================
Cmd_SysopAdd_f
==================
*/
void Cmd_SysopAdd_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	char			adminfile[256];
	int				id;
	int				len;
	gentity_t		*id_ent;
	fileHandle_t	f;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: addsysop #id\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;
	trap_Cvar_VariableStringBuffer( "sv_sysopfile", adminfile, sizeof( adminfile ) );
	len = trap_FS_FOpenFile( adminfile, &f, FS_READ_TEXT );
	if ( !f ) {
		trap_FS_FOpenFile( adminfile, &f, FS_WRITE_TEXT );
		trap_FS_FCloseFile( f );
		AddAdmin( id, "sv_sysopfile" );
		id_ent->client->sess.modData->adminref = 3;
		G_AdminLogPrintf( "Admin action by %s: addsysop \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
		G_PrintMessage( ent, "^6Sysop added.\n");
		ClientUserinfoChanged( id );
		return;
	}
	trap_FS_FCloseFile( f );
	if ( len == 0 ){
		AddAdmin( id, "sv_sysopfile" );
		id_ent->client->sess.modData->adminref = 3;
		G_AdminLogPrintf( "Admin action by %s: addsysop \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
		G_PrintMessage( ent, "^6Sysop added.\n");
		ClientUserinfoChanged( id );
		return;
	}

	if ( ent )
	{
		if ( ent->client->sess.modData->adminref < 3 ) {
			G_PrintMessage( ent, "You are not an active sysop.\n");
			return;
		}

		if ( clientNum == id) {
			G_PrintMessage( ent, "You are already a sysop.\n");
			return;
		}
	}

	if ( id_ent->client->sess.modData->adminref > 2 ) {
		G_PrintMessage( ent, "Player is already a sysop.\n");
		return;
	}

	AddAdmin( id, "sv_sysopfile" );
	id_ent->client->sess.modData->adminref = 3;
	G_AdminLogPrintf( "Admin action by %s: addsysop \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^6Sysop added.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_SysopRemove_f
==================
*/
void Cmd_SysopRemove_f( gentity_t *ent, int clientNum )
{
	char		buffer[MAX_TOKEN_CHARS];
	int			id;
	gentity_t	*id_ent;

	if ( ent && ent->client->sess.modData->adminref < 3 ) {
		G_PrintMessage( ent, "You are not an active sysop.\n");
		return;
	}

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: removesysop #id\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( CheckAdmin( id ) < 3 ) {
		G_PrintMessage( ent, "Player is not a sysop.\n");
		return;
	}

	if ( !RemoveAdmin( id, "sv_sysopfile" ) ) {
		G_PrintMessage( ent, "Error removing sysop.\n");
		return;
	}

	id_ent->client->sess.modData->adminref = CheckAdmin( id );
	G_AdminLogPrintf( "Admin action by %s: removesysop \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^6Sysop removed.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_AdminAdd_f
==================
*/
void Cmd_AdminAdd_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	char			adminfile[256];
	int				id;
	int				len;
	gentity_t		*id_ent;
	fileHandle_t	f;
	qboolean		sysop;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: addadmin #id\n");
		return;
	}

	if ( !g_enableAdmin.integer )
	{
		G_PrintMessage( ent, "Admins are not enabled on this server.\n");
		return;
	}

	sysop = qtrue;
	trap_Cvar_VariableStringBuffer( "sv_sysopfile", adminfile, sizeof( adminfile ) );
	len = trap_FS_FOpenFile( adminfile, &f, FS_READ_TEXT );
	if ( !f ) {
		sysop = qfalse;
	}
	trap_FS_FCloseFile( f );
	if ( len == 0 ){
		sysop = qfalse;
	}
	if ( sysop && ent && ent->client->sess.modData->adminref < 3 ) {
		G_PrintMessage( ent, "You are not an active sysop.\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;
	trap_Cvar_VariableStringBuffer( "sv_adminfile", adminfile, sizeof( adminfile ) );
	len = trap_FS_FOpenFile( adminfile, &f, FS_READ_TEXT );
	if ( !f ) {
		trap_FS_FOpenFile( adminfile, &f, FS_WRITE_TEXT );
		trap_FS_FCloseFile( f );
		AddAdmin( id, "sv_adminfile" );
		id_ent->client->sess.modData->adminref = 2;
		G_AdminLogPrintf( "Admin action by %s: addadmin \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
		G_PrintMessage( ent, "^6Admin added.\n");
		ClientUserinfoChanged( id );
		return;
	}
	trap_FS_FCloseFile( f );
	if ( len == 0 ){
		AddAdmin( id, "sv_adminfile" );
		id_ent->client->sess.modData->adminref = 2;
		G_AdminLogPrintf( "Admin action by %s: addadmin \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
		G_PrintMessage( ent, "^6Admin added.\n");
		ClientUserinfoChanged( id );
		return;
	}

	if ( ent )
	{
		if ( ent->client->sess.modData->adminref < 3 ) {
			G_PrintMessage( ent, "You are not an active sysop.\n");
			return;
		}

		if ( clientNum == id) {
			G_PrintMessage( ent, "You already have admin.\n");
			return;
		}
	}

	if ( id_ent->client->sess.modData->adminref > 1 ) {
		G_PrintMessage( ent, "Player is already an admin.\n");
		return;
	}

	AddAdmin( id, "sv_adminfile" );
	id_ent->client->sess.modData->adminref = 2;
	G_AdminLogPrintf( "Admin action by %s: addadmin \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^6Admin added.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_AdminRemove_f
==================
*/
void Cmd_AdminRemove_f( gentity_t *ent, int clientNum )
{
	char		buffer[MAX_TOKEN_CHARS];
	int			id;
	gentity_t	*id_ent;

	if ( ent && ent->client->sess.modData->adminref < 3 ) {
		G_PrintMessage( ent, "You are not an active sysop.\n");
		return;
	}

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: removeadmin #id\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( CheckAdmin( id ) < 2 ) {
		G_PrintMessage( ent, "Player is not an admin.\n");
		return;
	}

	if ( id_ent->client->sess.modData->adminref == 3 ) {
		G_PrintMessage( ent, "You can't touch the sysop!\n");
		return;
	}

	if ( !RemoveAdmin( id, "sv_adminfile" ) ) {
		G_PrintMessage( ent, "Error removing admin.\n");
		return;
	}

	id_ent->client->sess.modData->adminref = CheckAdmin( id );
	G_AdminLogPrintf( "Admin action by %s: removeadmin \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^6Admin removed.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_RefereeMe_f
==================
*/
void Cmd_RefereeMe_f( gentity_t *ent, int clientNum )
{
	char			*password;

	if ( !Q_stricmp( g_refereePass.string, "none" ) )
	{
		G_PrintMessage( ent, "This command is unavailable.\n");
		return;
	}

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: refme <password>\n");
		return;
	}

	if ( !g_enableReferee.integer )
	{
		G_PrintMessage( ent, "Referees are not enabled on this server.\n");
		return;
	}

	if ( ent->client->sess.modData->adminref != 0 ) {
		G_PrintMessage( ent, "You are already priviledged.\n");
		return;
	}

	password = ConcatArgs( 1 );

	Com_Printf("Correct password: \"%s\"\n", g_refereePass.string);
	Com_Printf("Entered text: \"%s\"\n", password);

	if ( Q_stricmp( password, g_refereePass.string ) )
	{
		G_PrintMessage( ent, "Incorrect password.\n");
		return;
	}
	
	ent->client->sess.modData->adminref = 1;
	G_PrintMessage( ent, "^6Referee status granted.\n");
	ClientUserinfoChanged( ent-g_entities );
}

/*
==================
Cmd_RefereeSet_f
==================
*/
void Cmd_RefereeSet_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	int				id;
	gentity_t		*id_ent;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: setref #id\n");
		return;
	}

	if ( !g_enableReferee.integer )
	{
		G_PrintMessage( ent, "Referees are not enabled on this server.\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	if ( id_ent->client->sess.modData->adminref != 0 ) {
		G_PrintMessage( ent, "Player is already priviledged.\n");
		return;
	}

	id_ent->client->sess.modData->adminref = 1;
	G_PrintMessage( ent, "^6Referee added.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_RefereeAdd_f
==================
*/
void Cmd_RefereeAdd_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	char			refereefile[256];
	int				id;
	int				len;
	gentity_t		*id_ent;
	fileHandle_t	f;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: addref #id\n");
		return;
	}

	if ( !g_enableReferee.integer )
	{
		G_PrintMessage( ent, "Referees are not enabled on this server.\n");
		return;
	}

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( id_ent->client->sess.modData->adminref != 0 ) {
		G_PrintMessage( ent, "Player is already priviledged.\n");
		return;
	}

	trap_Cvar_VariableStringBuffer( "sv_refereefile", refereefile, sizeof( refereefile ) );
	len = trap_FS_FOpenFile( refereefile, &f, FS_READ_TEXT );
	if ( !f ) {
		trap_FS_FOpenFile( refereefile, &f, FS_WRITE_TEXT );
		trap_FS_FCloseFile( f );
	}
	trap_FS_FCloseFile( f );

	AddAdmin( id, "sv_refereefile" );
	id_ent->client->sess.modData->adminref = 1;
	G_AdminLogPrintf( "Admin action by %s: addref \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^6Referee added.\n");
}

/*
==================
Cmd_RefereeRemove_f
==================
*/
void Cmd_RefereeRemove_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	int				id;
	gentity_t		*id_ent;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: removeref #id\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	if ( id_ent->client->sess.modData->adminref != 1 ) {
		G_PrintMessage( ent, "Player is not a referee.\n");
		return;
	}

	RemoveAdmin( id, "sv_refereefile" );

	id_ent->client->sess.modData->adminref = 0;
	G_PrintMessage( ent, "^6Referee removed.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_ClanAdd_f
==================
*/
void Cmd_ClanAdd_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	char			clanfile[256];
	int				id;
	int				len;
	gentity_t		*id_ent;
	fileHandle_t	f;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: addclan #id\n");
		return;
	}

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( id_ent->client->sess.modData->clan ) {
		G_PrintMessage( ent, "Player is already a clan member.\n");
		return;
	}

	trap_Cvar_VariableStringBuffer( "sv_clanfile", clanfile, sizeof( clanfile ) );
	len = trap_FS_FOpenFile( clanfile, &f, FS_READ_TEXT );
	if ( !f ) {
		trap_FS_FOpenFile( clanfile, &f, FS_WRITE_TEXT );
		trap_FS_FCloseFile( f );
	}
	trap_FS_FCloseFile( f );

	AddAdmin( id, "sv_clanfile" );
	id_ent->client->sess.modData->clan = qtrue;
	G_AdminLogPrintf( "Admin action by %s: addclan \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^3Clan member added.\n");
}

/*
==================
Cmd_ClanRemove_f
==================
*/
void Cmd_ClanRemove_f( gentity_t *ent, int clientNum )
{
	char		buffer[MAX_TOKEN_CHARS];
	int			id;
	gentity_t	*id_ent;

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: removeclan #id\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( !CheckClan( id ) ) {
		G_PrintMessage( ent, "Player is not a clan member.\n");
		return;
	}

	if ( !RemoveAdmin( id, "sv_clanfile" ) ) {
		G_PrintMessage( ent, "Error removing clan member.\n");
		return;
	}

	id_ent->client->sess.modData->clan = qfalse;
	G_AdminLogPrintf( "Admin action by %s: removeclan \"%s\"\n", ent?ent->client->pers.netname:"rcon", id_ent->client->pers.netname );
	G_PrintMessage( ent, "^3Clan member removed.\n");
}

/*
==================
Cmd_ModeratorAdd_f
==================
*/
void Cmd_ModeratorAdd_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	int				id;
	gentity_t		*id_ent;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: addmod #id\n");
		return;
	}

	if ( !g_enableModerator.integer )
	{
		G_PrintMessage( ent, "Match moderators are not enabled on this server.\n");
		return;
	}

	if ( ent && level.match )
	{
		G_PrintMessage( ent, "Match moderator status cannot be changed by a player during a match.\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	if ( id_ent->client->sess.modData->moderator != 0 ) {
		G_PrintMessage( ent, "Player is already a match moderator.\n");
		return;
	}

	SetTeam( id_ent, "s", NULL );
	id_ent->client->sess.modData->moderator = qtrue;
	G_PrintMessage( ent, "^1Match moderator added.\n");
	ClientUserinfoChanged( id );
	trap_SendServerCommand( id_ent-g_entities, "cp \"^1You are now a match moderator\n\"");
}

/*
==================
Cmd_ModeratorRemove_f
==================
*/
void Cmd_ModeratorRemove_f( gentity_t *ent, int clientNum )
{
	char			buffer[MAX_TOKEN_CHARS];
	int				id;
	gentity_t		*id_ent;

	if ( trap_Argc() != 2 ) {
		G_PrintMessage( ent, "usage: removemod #id\n");
		return;
	}

	if ( ent && level.match )
	{
		G_PrintMessage( ent, "Match moderator status cannot be changed by a player during a match.\n");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( !CheckID( ent, id ) ) {
		return;
	}

	id_ent = g_entities + id;

	if ( ent && ent->client->sess.modData->adminref < 2 ) {
		G_PrintMessage( ent, "You are not an active admin.\n");
		return;
	}

	if ( !id_ent->client->sess.modData->moderator ) {
		G_PrintMessage( ent, "Player is not a match moderator.\n");
		return;
	}

	id_ent->client->sess.modData->moderator = qfalse;
	G_PrintMessage( ent, "^1Match moderator removed.\n");
	ClientUserinfoChanged( id );
}

/*
==================
Cmd_ListCmds_f
==================
*/
void Cmd_ListCmds_f( gentity_t *ent, int clientNum )
{
	G_PrintMessage( ent, va("^7This server is running ROCmod v%s\n^3Available commands:\n", ROCMOD_VERSION));
	G_PrintMessage( ent, " ^7/sounds      ^3- List all available voice commands\n");
	G_PrintMessage( ent, " ^7/tokens      ^3- List all available text tokens\n");
	G_PrintMessage( ent, " ^7/shortcuts   ^3- Lists all available text shortcuts\n");
	G_PrintMessage( ent, " ^7/adm         ^3- Admin commands\n");
	G_PrintMessage( ent, " ^7/ref         ^3- Referee commands\n");
	G_PrintMessage( ent, " ^7/clan        ^3- Clan commands\n");
	G_PrintMessage( ent, " ^7/match       ^3- Match moderator commands\n");
	G_PrintMessage( ent, " ^7/addadmin    ^3- Adds a new admin to the server\n");
	G_PrintMessage( ent, " ^7/removeadmin ^3- Removes an existing admin from the server\n");
	G_PrintMessage( ent, " ^7/addsysop    ^3- Adds a new sysop to the server\n");
	G_PrintMessage( ent, " ^7/removesysop ^3- Removes an existing sysop from the server\n");
	G_PrintMessage( ent, " ^7/addref      ^3- Sets a particular player as permanent referee\n");
	G_PrintMessage( ent, " ^7/removeref   ^3- Removes a player's referee status\n");
	G_PrintMessage( ent, " ^7/refme       ^3- Gives yourself temporary referee status\n");
	G_PrintMessage( ent, " ^7/setref      ^3- Sets a particular player as a temporary referee\n");
	G_PrintMessage( ent, " ^7/addclan     ^3- Adds a player as a clan member\n");
	G_PrintMessage( ent, " ^7/removeclan  ^3- Removes an existing clan member from the server\n");
	G_PrintMessage( ent, " ^7/addmod      ^3- Adds a player as a temporary match moderator\n");
	G_PrintMessage( ent, " ^7/removemod   ^3- Removes an existing match moderator from the server\n");
	G_PrintMessage( ent, " ^7/motd        ^3- Display the server MOTD again\n");
	G_PrintMessage( ent, " ^7/admrcon     ^3- Admin access to selected rcon commands\n");
	G_PrintMessage( ent, " ^7/players     ^3- Lists all players with their id numbers\n");
	G_PrintMessage( ent, " ^7/admlist     ^3- Lists all admins currently in the game\n");
	G_PrintMessage( ent, " ^7/reflist     ^3- Lists all referees currently in the game\n");
	G_PrintMessage( ent, " ^7/clanlist    ^3- Lists all clan members currently in the game\n");
	G_PrintMessage( ent, " ^7/modlist     ^3- Lists all moderators currently in the game\n");
}

/*
==================
Cmd_ListSounds_f
==================
*/
void Cmd_ListSounds_f( gentity_t *ent, int clientNum )
{
	int		i;
	char	buffer[1024];
	int		pos;
	char	*text;

	G_PrintMessage( ent, va("^7There are %i available sounds:\n", voicecmds.voiceCommandCount));

	pos = 0;
	for ( i = 1; i <= voicecmds.voiceCommandCount; i++ )
	{
		if ( !voicecmds.voiceCommandSound[i][0] )
		{
			continue;
		}
		if ( ent )
		{
			if ( ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref > -1 && voicecmds.voiceCommandFlag[i] == VOICE_ADMIN_ONLY )
			{
				continue;
			}
			if ( !ent->client->sess.modData->clan && voicecmds.voiceCommandFlag[i] == VOICE_CLAN_ONLY )
			{
				continue;
			}
			if ( ent->client->sess.modData->adminref < 3 && ent->client->sess.modData->adminref > -2 && voicecmds.voiceCommandFlag[i] == VOICE_SYSOP_ONLY )
			{
				continue;
			}
			if ( ent->client->sess.modData->adminref < 1 && ent->client->sess.modData->adminref > -1 && voicecmds.voiceCommandFlag[i] == VOICE_REFEREE_ONLY )
			{
				continue;
			}
		}
		if ( voicecmds.voiceCommandText[i][0] )
		{
			text = va(" ^7@%i ^2%s\n", i, voicecmds.voiceCommandText[i]);
		}
		else
		{
			text = va(" ^7@%i ^2%s\n", i, &voicecmds.voiceCommandText[i][1]);
		}
		strncpy( &buffer[pos], text, 128 );
		pos += strlen( text );
		if ( pos > 750 )
		{
			buffer[pos++] = 0;
			G_PrintMessage( ent, buffer);
			pos = 0;
		}
	}
	if ( pos > 0 )
	{
		buffer[pos++] = 0;
		G_PrintMessage( ent, buffer);
	}
	G_PrintMessage( ent, "^7Use PgUp / PgDn to scroll through the list.\n");
}

/*
==================
Cmd_Tokens_f
==================
*/
void Cmd_Tokens_f( gentity_t *ent, int clientNum )
{
	G_PrintMessage( ent, "^7Available Text tokens:\n^7#n^3,^7#N  ^3- Your name\n^7#h^3,^7#H  ^3- Your health, numeric value\n^7#a^3,^7#A  ^3- Your armor, numeric value\n^7#b     ^3- Your armor, in bar format\n^7#B     ^3- Your health, in bar format\n^7#d     - The name of the person that last hurt you\n^7#D     - The name of the person that last killed you\n^7#t     - The name of the person you last hurt\n^7#T     - The name of the person you last killed\n^7#l^3,^7#L  ^3- Your current location\n^7#f^3,^7#F  ^3- The name of your nearest teammate\n");
	G_PrintMessage( ent, "^7#p     ^3- Name of your primary weapon (short)\n^7#P     ^3- Name of your primary weapon (long)\n^7#s     ^3- Name of your secondary weapon (short)\n^7#S     ^3- Name of your secondary weapon (long)\n^7#i     ^3- Name of your pistol (short)\n^7#I     ^3- Name of your pistol (long)\n^7#g     ^3- Name of your grenade (short)\n^7#G     ^3- Name of your grenade (long)\n^7#e^3,^7#E  ^3- Name of your equipment (Armor, NV goggles, Thermal goggles)\n^7#w     ^3- Name of the weapon you are currently holding (short)\n");
	G_PrintMessage( ent, "^7#W     ^3- Name of the weapon you are currently holding (long)\n^7#r     ^3- Name of the last weapon used to hurt you (short)\n^7#R     ^3- Name of the last weapon used to hurt you (long)\n^7#u,#U  ^3- Body location where the last person hit you\n^7#v,#V  ^3- Body location on the person you last killed\n^7#x     ^3- Your location when you were last hurt\n^7#X     ^3- Your location when you were last killed\n^7#y     ^3- Your location when you last hurt someone\n^7#Y     ^3- Your location when you last killed someone\n");
	G_PrintMessage( ent, "^7#z,#Z  ^3- The name of the last player to join the game\n^7#nn    ^3- The name of the person with the specified client id (i.e. #13)\n^7#c     ^3- The name of your teammate currently carrying the flag/briefcase\n^7#C     ^3- The name of your enemy currently carrying the flag/briefcase\n^7#o     ^3- Your team's score (or total number for non-team game)\n^7#O     ^3- The enemy team's score (or total number for non-team game)\n^7#k     ^3- Red team's score (or total number for non-team game)\n^7#K     ^3- Blue team's score (or total number for non-team game)\n");
}

/*
==================
Cmd_Shortcuts_f
==================
*/
void Cmd_Shortcuts_f( gentity_t *ent, int clientNum )
{
	G_PrintMessage( ent, "^3Available Shortcuts:\n ^7!ac  ^3- Enter at start of line to perform a /adm chat\n ^7!at  ^3- Enter at start of line to perform a /adm talk\n ^7!rc  ^3- Enter at start of line to perform a /ref chat\n ^7!rt  ^3- Enter at start of line to perform a /ref talk\n ^7!cc  ^3- Enter at start of line to perform a /clan chat\n");
}

/*
==================
Cmd_ServerInfo_f
==================
*/
void Cmd_ServerInfo_f( gentity_t *ent, int clientNum )
{
	G_PrintMessage( ent, va("^7This server is runnind ROCmod v%s\n^3Soldier of Fortune II v%s\n", ROCMOD_VERSION, GAME_VERSION));
}

/*
==================
Cmd_AdmRconList_f
==================
*/
void Cmd_AdmRconList_f( gentity_t *ent, int clientNum )
{
	static char		buffer[2048];
	char			cfgfile[256];
	char			cmd[MAX_TOKEN_CHARS];
	fileHandle_t	f;
	int				len;
	char			*fullcmd;

	if ( !ent ) {
		G_PrintMessage( ent, "Using admrcon from within rcon is redundant.\n");
		return;
	}

	if ( ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref > -1 ) {
		G_PrintMessage( ent, "You are not an admin.\n");
		return;
	}

	trap_Argv( 1, cmd, sizeof( cmd ) );

	buffer[0] = 0;
	trap_Cvar_VariableStringBuffer( "sv_extracmdsfile", cfgfile, sizeof( cfgfile ) );
	len = trap_FS_FOpenFile( cfgfile, &f, FS_READ_TEXT );
	if ( f )
	{
		if ( len >= sizeof( buffer ) )
			len = sizeof( buffer );
		trap_FS_Read( buffer, len, f);
	}
	trap_FS_FCloseFile( f );
	buffer[ len ] = 0;

	if ( strstr( buffer, va("\n%s", cmd) ) != NULL )
	{
		fullcmd = ConcatArgs( 1 );
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", fullcmd) );
	}
	else
	{
		G_PrintMessage( ent, "Unavailable command or cvar.\n");
	}
}

/*
==================
Cmd_AdminRcon_f
==================
*/
void Cmd_AdminRcon_f( gentity_t *ent, int clientNum )
{
	static char		buffer[2048];
	char			cfgfile[256];
	char			cmd[MAX_TOKEN_CHARS];
	fileHandle_t	f;
	int				len;
	char			*fullcmd;

	if ( !ent ) {
		G_PrintMessage( ent, "Using admrcon from within rcon is redundant.\n");
		return;
	}

	if ( ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref != -1 ) {
		G_PrintMessage( ent, "You are not an admin.\n");
		return;
	}

	trap_Argv( 1, cmd, sizeof( cmd ) );

	buffer[0] = 0;
	trap_Cvar_VariableStringBuffer( "sv_extracmdsfile", cfgfile, sizeof( cfgfile ) );
	len = trap_FS_FOpenFile( cfgfile, &f, FS_READ_TEXT );
	if ( f )
	{
		if ( len >= sizeof( buffer ) )
			len = sizeof( buffer );
		trap_FS_Read( buffer, len, f);
	}
	trap_FS_FCloseFile( f );
	buffer[ len ] = 0;

	if ( strstr( buffer, va("\n%s", cmd) ) != NULL )
	{
		fullcmd = ConcatArgs( 1 );
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", fullcmd) );
	}
	else
	{
		G_PrintMessage( ent, "Unavailable command or cvar.\n");
	}
}

/*
==================
Cmd_Players_f
==================
*/
void Cmd_Players_f( gentity_t *ent, int clientNum )
{
	int			i;

	G_PrintMessage( ent, va("^3There are ^7%i^3 players currently in the game.\n^2id - name\n", level.numConnectedClients));

	for (i = 0; i < level.maxclients; i++) 
	{
		if ( level.clients[i].pers.connected != CON_CONNECTED )
		{
			continue;
		}

		G_PrintMessage( ent, va("^%i%s%i^3 - %s%s   ^7(ping: %i, rate: %i)%s\n", level.clients[i].sess.modData->versionVerified?7:1, (i < 10)?" ":"", i, level.teamData.teamcolor[level.clients[i].sess.team], level.clients[i].pers.netname, (level.clients[i].ps.ping < 999)?level.clients[i].ps.ping:999, level.clients[i].sess.modData->rate, (level.clients[i].sess.team == TEAM_SPECTATOR)?" (spectator)":""));
	}
}

/*
==================
Cmd_AdmList_f
==================
*/
void Cmd_AdmList_f( gentity_t *ent, int clientNum )
{
	int			i;
	char		buffer[1024];
	char		*string;
	int			size;
	int			count;

	if ( ent && g_anonymousAdmin.integer && ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref > -1)
	{
		G_PrintMessage( ent, "The admins on this server are anonymous.\n");
		return;
	}

	strcpy(buffer, "^2id - name\n");
	size = strlen(buffer);

	count = 0;
	for (i = 0; i < level.maxclients; i++)
	{
		if ( level.clients[i].pers.connected != CON_CONNECTED )
		{
			continue;
		}
		if ( level.clients[i].sess.modData->adminref < 2 && level.clients[i].sess.modData->adminref > -1 )
		{
			continue;
		}

		count++;

		string = va("^7%s%i^3 - %s%s %s\n", (i < 10)?" ":"", i, level.teamData.teamcolor[level.clients[i].sess.team], level.clients[i].pers.netname, (level.clients[i].sess.team == TEAM_SPECTATOR)?"^7(spectator)":"");
		if ( size + strlen(string) >= 1024 )
		{
			break;
		}
		strcpy(buffer + size, string);
		size += strlen(string);
	}

	G_PrintMessage( ent, va("^3There are ^7%i^3 admins currently in the game.\n", count));
	if ( count )
	{
		G_PrintMessage( ent, buffer);
	}
}

/*
==================
Cmd_RefList_f
==================
*/
void Cmd_RefList_f( gentity_t *ent, int clientNum )
{
	int			i;
	char		buffer[1024];
	char		*string;
	int			size;
	int			count;

	strcpy(buffer, "^2id - name\n");
	size = strlen(buffer);

	count = 0;
	for (i = 0; i < level.maxclients; i++)
	{
		if ( level.clients[i].pers.connected != CON_CONNECTED )
		{
			continue;
		}
		if ( level.clients[i].sess.modData->adminref != 1 )
		{
			continue;
		}

		count++;

		string = va("^7%s%i^3 - %s%s %s\n", (i < 10)?" ":"", i, level.teamData.teamcolor[level.clients[i].sess.team], level.clients[i].pers.netname, (level.clients[i].sess.team == TEAM_SPECTATOR)?"^7(spectator)":"");
		if ( size + strlen(string) >= 1024 )
		{
			break;
		}
		strcpy(buffer + size, string);
		size += strlen(string);
	}

	G_PrintMessage( ent, va("^3There are ^7%i^3 referees currently in the game.\n", count));
	if ( count )
	{
		G_PrintMessage( ent, buffer);
	}
}

/*
==================
Cmd_ClanList_f
==================
*/
void Cmd_ClanList_f( gentity_t *ent, int clientNum )
{
	int			i;
	char		buffer[1024];
	char		*string;
	int			size;
	int			count;

	strcpy(buffer, "^2id - name\n");
	size = strlen(buffer);

	count = 0;
	for (i = 0; i < level.maxclients; i++)
	{
		if ( level.clients[i].pers.connected != CON_CONNECTED )
		{
			continue;
		}
		if ( !level.clients[i].sess.modData->clan )
		{
			continue;
		}

		count++;

		string = va("^7%s%i^3 - %s%s %s\n", (i < 10)?" ":"", i, level.teamData.teamcolor[level.clients[i].sess.team], level.clients[i].pers.netname, (level.clients[i].sess.team == TEAM_SPECTATOR)?"^7(spectator)":"");
		if ( size + strlen(string) >= 1024 )
		{
			break;
		}
		strcpy(buffer + size, string);
		size += strlen(string);
	}

	G_PrintMessage( ent, va("^3There are ^7%i^3 clan members currently in the game.\n", count));
	if ( count )
	{
		G_PrintMessage( ent, buffer);
	}
}

/*
==================
Cmd_ModList_f
==================
*/
void Cmd_ModList_f( gentity_t *ent, int clientNum )
{
	int			i;
	char		buffer[1024];
	char		*string;
	int			size;
	int			count;

	strcpy(buffer, "^2id - name\n");
	size = strlen(buffer);

	count = 0;
	for (i = 0; i < level.maxclients; i++)
	{
		if ( level.clients[i].pers.connected != CON_CONNECTED )
		{
			continue;
		}
		if ( !level.clients[i].sess.modData->moderator )
		{
			continue;
		}

		count++;

		string = va("^7%s%i^3 - %s%s\n", (i < 10)?" ":"", i, level.teamData.teamcolor[level.clients[i].sess.team], level.clients[i].pers.netname);
		if ( size + strlen(string) >= 1024 )
		{
			break;
		}
		strcpy(buffer + size, string);
		size += strlen(string);
	}

	G_PrintMessage( ent, va("^3There are ^7%i^3 moderators currently in the game.\n", count));
	if ( count )
	{
		G_PrintMessage( ent, buffer);
	}
}

/*
==================
Cmd_ModVersion_f
==================
*/
void Cmd_ModVersion_f( gentity_t *ent, int clientNum )
{
	G_PrintMessage( ent, va("\n^1RoC mod\n^7Version ^3%s^7 (build %i)\nBuild date: ^5%s\n\n^3Written by -]RoC[- Arrnor\nwww.rocclan.com\n", ROCMOD_VERSION, ROCMOD_BUILD, __DATE__));

}

/*
==================
Cmd_ScanResult_f
==================
*/
void Cmd_ScanResult_f( gentity_t *ent, int clientNum )
{
	char	buffer[MAX_TOKEN_CHARS];
	int		value;
	int		id;
	int		result;
	
	char	*text;

	if ( trap_Argc() < 3 ) {
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	id = atoi( buffer );

	if ( id != ent->client->sess.modData->currentScan ) {
		return;
	}

	text = ConcatArgs( 2 );
	result = Q_stricmpn( text, scanlist.scanListValue[id], 32 );
	if ( Q_stricmp( text, "0" ) == 0 )
	{
		if ( ent->client->sess.modData->scanRepeat == 1 )
		{
			G_ScanLogPrintf("File not found response from player %i (%s) while requesting scan for file '%s'\n", ent-g_entities, ent->client->pers.netname, scanlist.scanListFile[id] );
		}
	}
	else if ( result != 0 )
	{
		if ( ent->client->sess.modData->scanRepeat == 1 )
		{
			G_ScanLogPrintf("Invalid file modification detected from player %i (%s) for file '%s'\n", ent-g_entities, ent->client->pers.netname, scanlist.scanListFile[id] );
		}
		switch ( scanlist.scanListFlag[id] )
		{
			case 1:
			default:
				trap_DropClient( ent-g_entities, va("Invalid file modification: %s", scanlist.scanListFile[id] ) );
				break;
			case 2:
				G_Say( ent, NULL, SAY_ALL, va("^1WARNING: ^7Invalid file modification detected: ^5%s\n", scanlist.scanListFile[id] ), qtrue );
				break;
			case 3:
				G_Say( ent, NULL, SAY_ADMIN_ONLY, va("^1WARNING: ^7Invalid file modification detected: ^5%s\n", scanlist.scanListFile[id] ), qtrue );
				break;
			case 4:
				break;
		}
	}
	if ( result == 0 || ent->client->sess.modData->scanRepeat == g_scannerRepeat.integer )
	{
		ent->client->sess.modData->currentScan++;
		if ( ent->client->sess.modData->currentScan >= scanlist.scanListCount )
		{
			ent->client->sess.modData->currentScan = 0;
		}
		ent->client->sess.modData->scanRepeat = 0;
		if ( ent->client->sess.modData->currentScan == ent->client->sess.modData->startingScan )
		{
			ent->client->sess.modData->scanPaused = qtrue;
		}
	}
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t		*ent;
	char			cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	if (Q_stricmp (cmd, "say") == 0)
		Cmd_Say_f (ent, SAY_ALL, qfalse);
	else if (Q_stricmp (cmd, "say_team") == 0)
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
	else if (Q_stricmp (cmd, "tell") == 0) 
		Cmd_Tell_f ( ent );
	else if (Q_stricmp (cmd, "admtell") == 0) 
		Cmd_AdmTell_f ( ent );
	else if (Q_stricmp (cmd, "vsay_team") == 0) 
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
	else if (Q_stricmp (cmd, "score") == 0)
		Cmd_Score_f (ent);
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);

	else if (Q_stricmp (cmd, "adm") == 0)
		Cmd_AdminCommand_f( ent, clientNum, 1 );
	else if (Q_stricmp (cmd, "ref") == 0)
		Cmd_AdminCommand_f( ent, clientNum, 2 );
	else if (Q_stricmp (cmd, "clan") == 0)
		Cmd_AdminCommand_f( ent, clientNum, 3 );
	else if (Q_stricmp (cmd, "match") == 0)
		Cmd_AdminCommand_f( ent, clientNum, 4 );
	else if (Q_stricmp (cmd, "addadmin") == 0 && sv_adminControl.integer)
		Cmd_AdminAdd_f( ent, clientNum );
	else if (Q_stricmp (cmd, "removeadmin") == 0 && sv_adminControl.integer)
		Cmd_AdminRemove_f( ent, clientNum );
	else if (Q_stricmp (cmd, "addsysop") == 0 && sv_adminControl.integer)
		Cmd_SysopAdd_f( ent, clientNum );
	else if (Q_stricmp (cmd, "removesysop") == 0 && sv_adminControl.integer)
		Cmd_SysopRemove_f( ent, clientNum );
	else if (Q_stricmp (cmd, "refme") == 0)
		Cmd_RefereeMe_f( ent, clientNum );
	else if (Q_stricmp (cmd, "setref") == 0)
		Cmd_RefereeSet_f( ent, clientNum );
	else if (Q_stricmp (cmd, "addref") == 0)
		Cmd_RefereeAdd_f( ent, clientNum );
	else if (Q_stricmp (cmd, "removeref") == 0)
		Cmd_RefereeRemove_f( ent, clientNum );
	else if (Q_stricmp (cmd, "addclan") == 0)
		Cmd_ClanAdd_f( ent, clientNum );
	else if (Q_stricmp (cmd, "removeclan") == 0)
		Cmd_ClanRemove_f( ent, clientNum );
	else if (Q_stricmp (cmd, "addmod") == 0)
		Cmd_ModeratorAdd_f( ent, clientNum );
	else if (Q_stricmp (cmd, "removemod") == 0)
		Cmd_ModeratorRemove_f( ent, clientNum );
	else if (Q_stricmp (cmd, "?") == 0)
		Cmd_ListCmds_f( ent, clientNum );
	else if (Q_stricmp (cmd, "sounds") == 0)
		Cmd_ListSounds_f( ent, clientNum );
	else if (Q_stricmp (cmd, "motd") == 0)
		Cmd_Motd_f( ent, clientNum );
	else if (Q_stricmp (cmd, "tokens") == 0)
		Cmd_Tokens_f( ent, clientNum );
	else if (Q_stricmp (cmd, "serverinfo") == 0)
		Cmd_ServerInfo_f( ent, clientNum );
	else if (Q_stricmp (cmd, "admrcon") == 0)
		Cmd_AdminRcon_f( ent, clientNum );
	else if (Q_stricmp (cmd, "players") == 0)
		Cmd_Players_f( ent, clientNum );
	else if (Q_stricmp (cmd, "shortcuts") == 0)
		Cmd_Shortcuts_f( ent, clientNum );
	else if (Q_stricmp (cmd, "admlist") == 0)
		Cmd_AdmList_f( ent, clientNum );
	else if (Q_stricmp (cmd, "reflist") == 0)
		Cmd_RefList_f( ent, clientNum );
	else if (Q_stricmp (cmd, "clanlist") == 0)
		Cmd_ClanList_f( ent, clientNum );
	else if (Q_stricmp (cmd, "modlist") == 0)
		Cmd_ModList_f( ent, clientNum );
	else if (Q_stricmp (cmd, "about") == 0)
		Cmd_ModVersion_f( ent, clientNum );

	else if (Q_stricmp (cmd, "verified") == 0)
		Cmd_ClientVerified_f( ent, clientNum );
	else if (Q_stricmp (cmd, "uef") == 0)
		Cmd_ClientUpdate_f( ent, clientNum );
	else if (Q_stricmp (cmd, "cvarval") == 0)
		Cmd_ClientCvarVal_f( ent, clientNum );
	else if (Q_stricmp (cmd, "result") == 0)
		Cmd_ScanResult_f( ent, clientNum );

	else if (Q_stricmp (cmd, "dotest") == 0)
	{
		trap_SendServerCommand( ent-g_entities, va("scan %i %s", 1, scanlist.scanListFile[1]) );
	}

	else if ( !level.intermissiontime )
	{
		if ( Q_stricmp ( cmd, "drop" ) == 0 )
			Cmd_Drop_f ( ent );
		else if ( Q_stricmp (cmd, "dropitem" ) == 0 )
			Cmd_DropItem_f ( ent );
		else if ( Q_stricmp (cmd, "give") == 0 )
			Cmd_Give_f (ent);
		else if ( Q_stricmp (cmd, "god") == 0 )
			Cmd_God_f (ent);
		else if ( Q_stricmp (cmd, "notarget") == 0 )
			Cmd_Notarget_f (ent);
		else if ( Q_stricmp (cmd, "noclip") == 0 )
			Cmd_Noclip_f (ent);
		else if ( Q_stricmp (cmd, "kill") == 0 )
			Cmd_Kill_f (ent);
		else if ( Q_stricmp (cmd, "levelshot") == 0 )
			Cmd_LevelShot_f (ent);
		else if ( Q_stricmp (cmd, "follow") == 0 )
			Cmd_Follow_f (ent);
		else if ( Q_stricmp (cmd, "follownext") == 0 )
			Cmd_FollowCycle_f (ent, 1);
		else if ( Q_stricmp (cmd, "followprev") == 0 )
			Cmd_FollowCycle_f (ent, -1);
		else if ( Q_stricmp (cmd, "where") == 0 )
			Cmd_Where_f (ent);
		else if ( Q_stricmp (cmd, "callvote") == 0 )
			Cmd_CallVote_f (ent);
		else if ( Q_stricmp (cmd, "vote") == 0 )
			Cmd_Vote_f (ent);
		else if ( Q_stricmp (cmd, "setviewpos") == 0 )
			Cmd_SetViewpos_f( ent );
		else if ( Q_stricmp ( cmd, "ignore" ) == 0 )
			Cmd_Ignore_f ( ent );

#ifdef _SOF2_BOTS
		else if ( Q_stricmp (cmd, "addbot") == 0 )
			trap_SendServerCommand( clientNum, "print \"ADDBOT command can only be used via RCON\n\"" );
#endif
	}
	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
