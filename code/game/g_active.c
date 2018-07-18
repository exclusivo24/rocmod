// Copyright (C) 2001-2002 Raven Software
//
// g_active.c --

#include "g_local.h"

void P_SetTwitchInfo(gclient_t	*client)
{
	client->ps.painTime = level.time;
	client->ps.painDirection ^= 1;
}

/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) 
{
	gclient_t	*client;
	float		count;
	vec3_t		angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) 
	{
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) 
	{
		// didn't take any damage
		return;		
	}

	if ( count > 255 ) 
	{
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) 
	{
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} 
	else 
	{
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 255;
		client->ps.damageYaw = angles[YAW]/360.0 * 255;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time)) 
	{
		// don't do more than two pain sounds a second
		if ( level.time - client->ps.painTime < 500 ) 
		{
			return;
		}

		P_SetTwitchInfo(client);
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;
	}


	client->ps.damageCount = count;

	// clear totals
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}

/*
=============
P_WorldEffects

Check for drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) 
{
	int	 waterlevel;

	if ( ent->client->noclip ) 
	{
		// don't need air
		ent->client->airOutTime = level.time + 12000;	
		return;
	}

	waterlevel = ent->waterlevel;

	// check for drowning
	if ( waterlevel == 3 && (ent->watertype & CONTENTS_WATER)) 
	{
		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) 
		{
			// drown!
			ent->client->airOutTime += 1000;
			if ( ent->health > 0 ) 
			{
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
				{
					ent->damage = 15;
				}

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) 
				{
//					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/pain_death/mullins/drown_dead.wav"));
				} 
				else
				{
					G_AddEvent ( ent, EV_PAIN_WATER, 0 );
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, ent->damage, DAMAGE_NO_ARMOR, MOD_WATER, HL_NONE );
			}
		}
	} 
	else 
	{
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) 
{
//	ent->client->ps.loopSound = 0;
}

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_IsClientSiameseTwin

Checks to see if the two clients should never have been separated at birth
============
*/
static qboolean G_IsClientSiameseTwin ( gentity_t* ent, gentity_t* ent2 )
{
	if ( G_IsClientSpectating ( ent->client ) || G_IsClientDead ( ent->client ) )
	{
		return qfalse;
	}

	if ( G_IsClientSpectating ( ent2->client ) || G_IsClientDead ( ent2->client ) )
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[0] + ent2->r.mins[0] > ent->r.currentOrigin[0] + ent->r.maxs[0])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[1] + ent2->r.mins[1] > ent->r.currentOrigin[1] + ent->r.maxs[1])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[2] + ent2->r.mins[2] > ent->r.currentOrigin[2] + ent->r.maxs[2])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[0] + ent2->r.maxs[0] < ent->r.currentOrigin[0] + ent->r.mins[0])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[1] + ent2->r.maxs[1] < ent->r.currentOrigin[1] + ent->r.mins[1])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[2] + ent2->r.maxs[2] < ent->r.currentOrigin[2] + ent->r.mins[2])
	{
		return qfalse;
	}

	return qtrue;
}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_TouchTriggers( gentity_t *ent ) 
{
	int				i;
	int				num;
	int				touch[MAX_GENTITIES];
	gentity_t		*hit;
	trace_t			trace;
	vec3_t			mins;
	vec3_t			maxs;
	static vec3_t	range = { 20, 20, 40 };

	if ( !ent->client ) 
	{
		return;
	}

	// dead clients don't activate triggers!
	if ( G_IsClientDead ( ent->client ) ) 
	{
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->r.absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	// Reset the players can use flag
	ent->client->ps.pm_flags &= ~(PMF_CAN_USE);
	ent->client->useEntity = 0;
	ent->client->ps.loopSound = 0;
	ent->s.modelindex  = 0;

	for ( i=0 ; i<num ; i++ ) 
	{
		hit = &g_entities[touch[i]];

		// pmove would have to have detected siamese twins first
		if ( hit->client && hit != ent && !hit->client->siameseTwin && (ent->client->ps.pm_flags & PMF_SIAMESETWINS) )
		{
			// See if this client has a twin
			if ( !G_IsClientSiameseTwin ( ent, hit ) )
			{
				continue;
			}

			// About time these twins were separated!!
			ent->client->siameseTwin = hit;
			hit->client->siameseTwin = ent;
		}

		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
		{
			continue;
		}

		// Look for usable gametype triggers and you cant use when zoomed
		if ( !(ent->client->ps.pm_flags & PMF_ZOOMED ) )
		{
			switch ( hit->s.eType )
			{
				case ET_GAMETYPE_TRIGGER:
					if ( hit->use && trap_GT_SendEvent ( GTEV_TRIGGER_CANBEUSED, level.time, hit->health, ent->s.number, ent->client->sess.team, 0, 0 ) )
					{
						ent->client->ps.pm_flags |= PMF_CAN_USE;
						ent->client->ps.stats[STAT_USEICON] = hit->delay;
						ent->client->ps.stats[STAT_USETIME_MAX] = hit->soundPos1;

						if ( ent->client->ps.stats[STAT_USETIME] )
						{
							ent->client->ps.loopSound = hit->soundLoop;
						}
						ent->client->useEntity = hit;
						continue;
					}
					break;
				
				case ET_ITEM:
					if ( hit->item->giType == IT_GAMETYPE && trap_GT_SendEvent ( GTEV_ITEM_CANBEUSED, level.time, hit->item->quantity, ent->s.number, ent->client->sess.team, 0, 0 ) )
					{
						ent->client->ps.pm_flags |= PMF_CAN_USE;
						ent->client->ps.stats[STAT_USEICON] = level.gametypeItems[hit->item->giTag].useIcon;
						ent->client->ps.stats[STAT_USETIME_MAX] = level.gametypeItems[hit->item->giTag].useTime;

						if ( ent->client->ps.stats[STAT_USETIME] )
						{
							ent->client->ps.loopSound = level.gametypeItems[hit->item->giTag].useSound;
						}
						ent->client->useEntity = hit;
						continue;
					}
					break;
			}
		}

		if ( !hit->touch && !ent->touch ) 
		{
			continue;
		}

		// ignore most entities if a spectator
		if ( G_IsClientSpectating ( ent->client ) ) 
		{
			if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) 
			{
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) 
		{
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) 
			{
				continue;
			}
		} 
		else 
		{
			if ( !trap_EntityContact( mins, maxs, hit ) ) 
			{
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) 
		{
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) 
		{
			ent->touch( ent, hit, &trace );
		}
	}

	// Dont bother looking for twins again unless pmove says so
	ent->client->ps.pm_flags &= (~PMF_SIAMESETWINS);
}


/*
============
G_MoverTouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_MoverTouchPushTriggers( gentity_t *ent, vec3_t oldOrg ) 
{
	int			i, num;
	float		step, stepSize, dist;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs, dir, size, checkSpot;
	const vec3_t	range = { 40, 40, 52 };

	// non-moving movers don't hit triggers!
	if ( !VectorLengthSquared( ent->s.pos.trDelta ) ) 
	{
		return;
	}

	VectorSubtract( ent->r.mins, ent->r.maxs, size );
	stepSize = VectorLength( size );
	if ( stepSize < 1 )
	{
		stepSize = 1;
	}

	VectorSubtract( ent->r.currentOrigin, oldOrg, dir );
	dist = VectorNormalize( dir );
	for ( step = 0; step <= dist; step += stepSize )
	{
		VectorMA( ent->r.currentOrigin, step, dir, checkSpot );
		VectorSubtract( checkSpot, range, mins );
		VectorAdd( checkSpot, range, maxs );

		num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		// can't use ent->r.absmin, because that has a one unit pad
		VectorAdd( checkSpot, ent->r.mins, mins );
		VectorAdd( checkSpot, ent->r.maxs, maxs );

		for ( i=0 ; i<num ; i++ ) 
		{
			hit = &g_entities[touch[i]];

			if ( hit->s.eType != ET_PUSH_TRIGGER )
			{
				continue;
			}

			if ( hit->touch == NULL ) 
			{
				continue;
			}

			if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
			{
				continue;
			}


			if ( !trap_EntityContact( mins, maxs, hit ) ) 
			{
				continue;
			}

			memset( &trace, 0, sizeof(trace) );

			if ( hit->touch != NULL ) 
			{
				hit->touch(hit, ent, &trace);
			}
		}
	}
}

/*
=================
G_UpdatePlayerStateScores

Update the scores in the playerstate
=================
*/
void G_UpdatePlayerStateScores ( gentity_t* ent )
{
	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( level.gametypeData->teams ) 
	{
		ent->client->ps.persistant[PERS_RED_SCORE] = level.teamScores[TEAM_RED];
		ent->client->ps.persistant[PERS_BLUE_SCORE] = level.teamScores[TEAM_BLUE];

		ent->client->ps.persistant[PERS_BLUE_ALIVE_COUNT] = level.teamAliveCount[TEAM_BLUE];
		ent->client->ps.persistant[PERS_RED_ALIVE_COUNT] = level.teamAliveCount[TEAM_RED];
	} 
	else 
	{
		if ( level.numConnectedClients == 0 ) 
		{
			ent->client->ps.persistant[PERS_RED_SCORE] = 0;
			ent->client->ps.persistant[PERS_BLUE_SCORE] = 0;
		} 
		else if ( level.numConnectedClients == 1 ) 
		{
			ent->client->ps.persistant[PERS_RED_SCORE] = level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE];
			ent->client->ps.persistant[PERS_BLUE_SCORE] = 0;
		} 
		else 
		{
			ent->client->ps.persistant[PERS_RED_SCORE] = level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE];
			ent->client->ps.persistant[PERS_BLUE_SCORE] = level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE];
		}
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) 
{
	pmove_t		pm;
	gclient_t	*client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = g_speed.integer + 120; // faster than normal
		client->ps.loopSound = 0;

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		pm.animations = NULL;

		// perform a pmove
		Pmove (&pm);

		G_UpdatePlayerStateScores ( ent );

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	if ( client->sess.modData->adminref > 1 && g_allowAdminGhost.integer && (client->buttons & BUTTON_WALKING) && !(client->oldbuttons & BUTTON_WALKING) && client->sess.team != TEAM_SPECTATOR && !client->sess.modData->penalty )
	{
		if ( !client->sess.modData->adminFreezeRespawn )
		{
			client->sess.modData->adminFreezeRespawn = qtrue;
			client->ps.respawnTimer = 0;
		}
		else
		{
			client->sess.modData->adminFreezeRespawn = qfalse;
			if ( level.gametypeData->respawnType == RT_INTERVAL )
			{
				G_SetRespawnTimer( ent );
			}
			else
			{
				ent->client->ps.respawnTimer = 0;
				G_StopFollowing( ent );
				G_StopGhosting( ent );
				respawn( ent );
			}
		}
	}

	// attack button cycles through spectators
	if ( client->sess.spectatorState != SPECTATOR_FOLLOW && g_forceFollow.integer && (client->sess.modData->adminref < 2 || (client->sess.modData->adminref > 1 && client->sess.modData->adminFollow)) && (!level.match || !client->sess.modData->moderator) )
	{
		if ( g_forceFollow.integer > 1 )
		{
			client->sess.spectatorFirstPerson = qtrue;
		}

		Cmd_FollowCycle_f( ent, 1 );
	}
	if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) 
	{
		Cmd_FollowCycle_f( ent, 1 );
	}
	else if ( ( client->buttons & BUTTON_ALT_ATTACK ) && ! ( client->oldbuttons & BUTTON_ALT_ATTACK ) ) 
	{
		Cmd_FollowCycle_f( ent, -1 );
	}		
	else if ( (!g_forceFollow.integer || client->sess.modData->adminref > 1 || (level.match && client->sess.modData->moderator)) && ucmd->upmove > 0 && (client->ps.pm_flags & PMF_FOLLOW) )
	{
		ent->client->sess.modData->adminFollow = qfalse;
		G_StopFollowing( ent );
	}
	else if ( (client->buttons & BUTTON_USE) && !( client->oldbuttons & BUTTON_USE ) )
	{
		// If not following then go to either third or first
		if ( client->sess.spectatorState != SPECTATOR_FOLLOW )
		{
			client->sess.spectatorFirstPerson = g_forceFollow.integer < 2 ? qfalse : qtrue;
			Cmd_FollowCycle_f( ent, -1 );
		}
		// If in first person then either go to free float or third person
		else if ( client->sess.spectatorFirstPerson )
		{
			if ( g_forceFollow.integer < 2 )
			{
				client->sess.spectatorFirstPerson = qfalse;
			}	
		}
		// Must be in third person so just go to first
		else
		{
			client->sess.spectatorFirstPerson = qtrue;
		}
	}
}

/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( !g_inactivity.integer || level.paused ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		client->sess.modData->adminref > 1 ||
		(client->sess.modData->clan && g_protectClan.integer) ||
		(client->pers.cmd.buttons & (BUTTON_ATTACK|BUTTON_ALT_ATTACK)) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->idleTime = level.time;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			client->inactivityTime = level.time + g_inactivity.integer * 1000;
			switch ( g_inactivityAction.integer ) {
				case 0:
					trap_DropClient( client - level.clients, "Dropped due to inactivity" );
					return qfalse;
				case 1:
					if ( g_entities[client - level.clients].client->sess.team != TEAM_SPECTATOR ) {
						SetTeam ( &g_entities[client - level.clients], "spectator", NULL );
					}
				case 2:
					if ( g_entities[client - level.clients].client->sess.team == TEAM_SPECTATOR ) {
						trap_DropClient( client - level.clients, "Dropped due to inactivity" );
					} else {
						SetTeam ( &g_entities[client - level.clients], "spectator", NULL );
					}
					return qtrue;
			}
			return qtrue;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
=================
ClientCampTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientCampTimer( gclient_t *client )
{
	gentity_t	*ent;

	if ( !g_camper.integer || level.paused || level.match || !client->camperTime || client->sess.ghost || client->ps.pm_type == PM_DEAD || client->spawn || (g_camperMode.integer == 1 && g_entities[client - level.clients].s.weapon == WP_MSG90A1) || g_camperMode.integer == 2 ) {
		// give everyone some time, so if the operator sets g_camper during
		// gameplay, everyone isn't kicked
		client->camperTime = level.time + 60 * 1000;
		client->camperWarning = qfalse;
	} else if ( client->inactivityWarning ||
		client->sess.modData->adminref > 2 ||
		DistanceSquared( client->ps.origin, client->camperOrigin ) > (g_camperRadius.integer * g_camperRadius.integer) ) {
		VectorCopy( client->ps.origin, client->camperOrigin );
		client->camperTime = level.time + g_camper.integer * 1000;
		client->camperWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->camperTime ) {
			client->camperTime = level.time + g_camper.integer * 1000;
			switch ( g_camperAction.integer ) {
				case 0:
					ent = &g_entities[ client - level.clients ];
					ent->flags &= ~FL_GODMODE;
					client->ps.stats[STAT_HEALTH] = ent->health = -999;
					player_die( ent, ent, ent, 100000, MOD_ADMIN_STRIKE, HL_NONE, vec3_origin );
					trap_SendServerCommand( -1, va("print \"%s%s^7's tent burned down.\n\"", level.teamData.teamcolor[client->sess.team], client->pers.netname));
					G_BroadcastSound("sound/ambience/generic/thunder_loud.mp3");
					return qtrue;
				case 1:
					trap_DropClient( client - level.clients, "Dropped due to camping" );
					return qfalse;
				case 2:
					if ( !g_entities[client - level.clients].client->sess.modData->dummy ) {
						g_entities[client - level.clients].client->sess.modData->dummy = qtrue;
						ClientUserinfoChanged( client - level.clients );
						trap_SendServerCommand( -1, va("print \"%s%s^7 has been labeled a ^3TARGET DUMMY^7 for camping.\n\"", level.teamData.teamcolor[client->sess.team], client->pers.netname));
						G_BroadcastSound("sound/npc/air1/guard02/laughs.mp3");
					}
					return qtrue;
				case 3:
					if ( !g_entities[client - level.clients].client->ps.stats[STAT_PLANTED] ) {
						g_entities[client - level.clients].client->ps.stats[STAT_PLANTED] = 1;
						trap_SendServerCommand( -1, va("print \"%s%s^7 has been glued to his camping spot.\n\"", level.teamData.teamcolor[client->sess.team], client->pers.netname));
						G_BroadcastSound("sound/npc/air1/guard02/laughs.mp3");
					}
					return qtrue;
				case 4:
					if ( g_entities[client - level.clients].client->ps.weapon ) {
						g_entities[client - level.clients].client->ps.weapon = 0;
						g_entities[client - level.clients].client->ps.weaponstate = WEAPON_READY;
						g_entities[client - level.clients].client->ps.stats[STAT_WEAPONS] = 0;
						g_entities[client - level.clients].client->noOutfittingChange = qtrue;
						trap_SendServerCommand( -1, va("print \"%s%s^7 gives up camping and surrenders.\n\"", level.teamData.teamcolor[client->sess.team], client->pers.netname));
						G_BroadcastSound("sound/npc/civ/english/male/unarmed.mp3");
					}
					return qtrue;
			}
			return qtrue;
		}
		if ( level.time > client->camperTime - 10000 && !client->camperWarning ) {
			client->camperWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"No Camping! Ten seconds until punishment!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientSpecList
==================
*/
void ClientSpecList( gentity_t *ent ) 
{
	qboolean	sendlast = qfalse;
	qboolean	sendnext = qfalse;

	if ( ent->s.number == ent->client->sess.spectatorClient )
	{
		return;
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW && g_entities[ent->client->sess.spectatorClient].client->sess.modData->extraFeatures & EXTRA_SPECLIST )
	{
		sendnext = qtrue;
	}
	if ( ent->client->sess.spectatorLastClient != -1 && g_entities[ent->client->sess.spectatorLastClient].client->sess.modData->extraFeatures & EXTRA_SPECLIST )
	{
		sendlast = qtrue;
	}

	if ( ent->client->sess.modData->adminref == 3 )
	{
		return;
	}
	if ( level.match && ent->client->sess.modData->moderator )
	{
		return;
	}
	if ( ent->client->sess.modData->adminref == 2 && sendnext && g_entities[ent->client->sess.spectatorClient].client->sess.modData->adminref < 2 )
	{
		sendnext = qfalse;
	}
	if ( ent->client->sess.modData->adminref == 2 && sendlast && g_entities[ent->client->sess.spectatorLastClient].client->sess.modData->adminref < 2 )
	{
		sendlast = qfalse;
	}

	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW && ent->client->sess.spectatorLastClient == -1 )
	{
		return;
	}
	else if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW && ent->client->sess.spectatorLastClient != -1 )
	{
		if ( sendlast )
		{
			trap_SendServerCommand( ent->client->sess.spectatorLastClient, va("ers %i", ent->s.number) );
		}
		ent->client->sess.spectatorLastClient = -1;
		return;
	}
	else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW && ent->client->sess.spectatorLastClient == -1 )
	{
		if ( sendnext )
		{
			trap_SendServerCommand( ent->client->sess.spectatorClient, va("eas %i", ent->s.number) );
		}
		ent->client->sess.spectatorLastClient = ent->client->sess.spectatorClient;
		return;
	}
	else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW && ent->client->sess.spectatorLastClient != -1 && ent->client->sess.spectatorClient != ent->client->sess.spectatorLastClient )
	{
		if ( sendlast )
		{
			trap_SendServerCommand( ent->client->sess.spectatorLastClient, va("ers %i", ent->s.number) );
		}
		if ( sendnext )
		{
			trap_SendServerCommand( ent->client->sess.spectatorClient, va("eas %i", ent->s.number) );
		}
		ent->client->sess.spectatorLastClient = ent->client->sess.spectatorClient;
		return;
	}
}

/*
==================
ClientFileScanner
==================
*/
void ClientFileScanner( gentity_t *ent ) 
{
	int num;

	num = ent->client->sess.modData->currentScan;
	if ( ent->client->sess.modData->scanRepeat > g_scannerRepeat.integer )
	{
		ent->client->sess.modData->scanRepeat = 0;
		G_ScanLogPrintf("No response from player %i (%s) while requesting scan for file '%s'\n", ent-g_entities, ent->client->pers.netname, scanlist.scanListFile[num] );
		switch ( g_scannerDefaultAction.integer )
		{
			case 0:
			default:
				trap_DropClient( ent-g_entities, "No response during file scan. Try turning on auto-download." );
				break;
			case 1:
				G_Say( ent, NULL, SAY_ALL, va("^1WARNING: ^7No response while requesting scan for file ^5%s\n", scanlist.scanListFile[num] ), qtrue );
				break;
			case 2:
				G_Say( ent, NULL, SAY_ADMIN_ONLY, va("^1WARNING: ^7No response while requesting scan for file ^5%s\n", scanlist.scanListFile[num] ), qtrue );
				break;
			case 3:
				break;
		}
		return;
	}
	trap_SendServerCommand( ent-g_entities, va("scan %i %s", num, scanlist.scanListFile[num]) );
	ent->client->sess.modData->scanRepeat++;
}

/*
==================
ClientPeriodicActions

Actions that happen once every few seconds
==================
*/
void ClientPeriodicActions( gentity_t *ent ) 
{
	// Display the MOTD to new clients
	if ( !ent->client->sess.modData->motdShown )
	{
		if ( g_motdEnabled.integer && level.time < ( ent->client->pers.enterTime + (g_motdDisplayTime.integer * 1000) ) )
		{
			SendMOTD( ent );
//			trap_SendServerCommand( ent-g_entities, va("cp \"@^2Soldier of Fortune II\n^1ROCmod %s - %s\n\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n^7%s\n\n\"", ROCMOD_VERSION, ROCMOD_DATE, g_motd1.string, g_motd2.string, g_motd3.string, g_motd4.string, g_motd5.string, g_motd6.string, g_motd7.string, g_motd8.string, g_motd9.string, g_motd10.string) );
		}
		else
		{
			ent->client->sess.modData->motdShown = qtrue;
		}
	}
}

/*
==================
ClientOneSecActions

Actions that happen once a second
==================
*/
void ClientOneSecActions( gentity_t *ent ) 
{
	// See if they need to be released from the penalty box
	if ( ent->client->sess.modData->penalty && level.time > ent->client->ps.respawnTimer )
	{
		AdminCmd_DoUnPenalty( ent );
	}

	if (  ent->r.svFlags & SVF_BOT ) {
		return;
	}

	// Display the penalty message for clients in the penalty box
	if ( ent->client->sess.modData->penalty )
	{
		char	*text;
		int		time;

		time = ent->client->ps.respawnTimer - level.time;
		time /= 1000;

		switch ( ent->client->sess.modData->penalty )
		{
			case 5:
				text = recondata[ ent-g_entities ].penaltyReason;
				break;
			case 4:
				text = "Spawn killing";
				break;
			case 3:
				text = "Flag stealing";
				break;
			case 2:
				text = "Team killing";
				break;
			case 1:
			default:
				text = "Bad behavior";
				break;
		}
		trap_SendServerCommand( ent-g_entities, va("cp \"@\n^1You have been thrown into the penalty box!\n^7Reason: ^5%s\n^3Time remaining: ^7%i seconds\n\"", text, time) );
	}

	// Verify the client mod and display a warning if necessary
	if ( g_verifyClients.integer && !ent->client->sess.modData->versionVerified && level.time - ent->client->sess.modData->versionWarnTime > 2000 )
	{
		if ( ent->client->sess.modData->versionChecks < 5 )
		{
			ent->client->sess.modData->versionWarnTime = level.time;
			ent->client->sess.modData->versionChecks++;
			trap_SendServerCommand( ent-g_entities, "verifymod" );
		}
		else if ( sv_modClient.integer && level.time - ent->client->sess.modData->versionWarnTime > (g_verifyClientsTime.integer * 1000) )
		{
			if ( ent->client->sess.modData->versionWarnCount < g_verifyClientsMax.integer || g_verifyClientsMax.integer == 0 )
			{
				ent->client->sess.modData->versionWarnCount++;
				ent->client->sess.modData->versionWarnTime = level.time;
				trap_SendServerCommand( ent-g_entities, va("chat -1 \"^1WARNING: ^7This server is running ^1ROCmod %s^7, which you do not appear to have.\n\"", ROCMOD_VERSION ) );
				trap_SendServerCommand( ent-g_entities, "chat -1 \"^7Please go ^3download the mod^7, or just ^3turn on auto-downloading^7, and re-join the game.\n\"" );
				trap_SendServerCommand( ent-g_entities, "verifymod" );
			}
			else
			{
				trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %d This server requires ROCmod %s\n", ent->s.number, ROCMOD_VERSION ) );
			}
		}
	}

	// Check their rate and warn or kick them if necessary
	if ( !ent->client->sess.modData->rateVerified && level.time - ent->client->sess.modData->rateWarnTime > 15000 )
	{
		if ( ent->client->sess.modData->rateChecks < 5 )
		{
			ent->client->sess.modData->rateWarnTime = level.time;
			ent->client->sess.modData->rateChecks++;
			trap_SendServerCommand( ent-g_entities, va("chat -1 \"^1WARNING: ^7Your connection rate is below ^3%i^7. Increase it or you will be ^1kicked^7.\n\"", sv_minRate.integer ) );
			trap_SendServerCommand( ent-g_entities, va("chat -1 \"^7(To change your rate, open the console and type: ^3/rate %i^7)\n\"", sv_minRate.integer ) );
		}
		else
		{
			trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %d Raise your connection rate above %i\n", ent->s.number, sv_minRate.integer ) );
		}
	}
}

/*
==================
ClientTimerActions

Actions that happen once per client frame
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) 
{
	gclient_t	*client;

	client = ent->client;
	// Check so see if the player has moved and if so dont let them change their outfitting
	if ( !client->noOutfittingChange && ((level.time - client->respawnTime) > 3000))
	{
		vec3_t vel;

		// Check the horizontal velocity for movement
		VectorCopy ( client->ps.velocity, vel );
		vel[2] = 0;
		if ( VectorLengthSquared ( vel ) > 100 )
		{
			client->noOutfittingChange = qtrue;
		}
	}

	if ( client->spawn && DistanceSquared(client->ps.origin, client->spawnOrigin) > (200 * 200) )
	{
		client->spawn = qfalse;
	}

	// Forgive voice chats
	if ( g_voiceFloodCount.integer && recondata[ent->s.number].voiceFloodCount )
	{
		int forgiveTime = 60000 / g_voiceFloodCount.integer;

		recondata[ent->s.number].voiceFloodTimer += msec;
		while ( recondata[ent->s.number].voiceFloodTimer >= forgiveTime ) 
		{
			// Forgive one voice chat
			recondata[ent->s.number].voiceFloodCount--;

			recondata[ent->s.number].voiceFloodTimer -= forgiveTime;
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) 
{
	G_UpdatePlayerStateScores ( &g_entities[client->ps.clientNum] );

	client->ps.loopSound = 0;

	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

	if ( (client->buttons & BUTTON_ATTACK) & ( client->oldbuttons ^ client->buttons ) ) 
	{
//		// this used to be an ^1 but once a player says ready, it should stick
//		client->readyToExit = 1;
		client->readyToExit ^= 1;
	}
}

/*
====================
G_Use

use key pressed
====================
*/
void G_Use ( gentity_t* ent )
{
	if ( !ent->client->useEntity )
	{
		return;
	}

	if ( ent->client->useEntity->s.eType == ET_ITEM )
	{
		// Make sure one last time that it can still be used
		if ( !trap_GT_SendEvent ( GTEV_ITEM_CANBEUSED, level.time, ent->client->useEntity->item->quantity, ent->s.number, ent->client->sess.team, 0, 0 ) )
		{
			return;
		}

		gametype_item_use ( ent->client->useEntity, ent );
		return;
	}

	// Make double sure it can still be used
	if ( !trap_GT_SendEvent ( GTEV_TRIGGER_CANBEUSED, level.time, ent->client->useEntity->health, ent->s.number, ent->client->sess.team, 0, 0 ) )
	{
		return;
	}
		
	ent->client->useEntity->use ( ent->client->useEntity, ent, ent );
}

/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) 
{
	int			i;
	int			event;
	gclient_t	*client;
	vec3_t		dir;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) 
	{
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}

	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) 
	{
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) 
		{
			case EV_FALL_MEDIUM:
			case EV_FALL_FAR:
			{
				int damage;
				
				damage  = client->ps.eventParms[ i & (MAX_PS_EVENTS-1) ];
				damage &= 0x000000ff;

				client->ps.eventParms[ i & (MAX_PS_EVENTS-1) ] = damage;
							
				if ( ent->s.eType != ET_PLAYER ) 
				{
					break;		// not in the player model
				}
				
				if ( g_dmflags.integer & DF_NO_FALLING ) 
				{
					break;
				}		
			
				VectorSet (dir, 0, 0, 1);
				ent->pain_debounce_time = level.time + 200;	// no normal pain sound
				G_Damage (ent, NULL, NULL, NULL, NULL, damage, DAMAGE_NO_ARMOR, MOD_FALLING, HL_NONE );
				break;
			}

			case EV_FIRE_WEAPON:
				ent->client->noOutfittingChange = qtrue;
				ent->client->spawn = qfalse;
				ent->client->invulnerableTime = 0;
				G_FireWeapon( ent, ATTACK_NORMAL );
				break;

			case EV_ALT_FIRE:
				ent->client->noOutfittingChange = qtrue;
				ent->client->spawn = qfalse;
				ent->client->invulnerableTime = 0;
				G_FireWeapon( ent, ATTACK_ALTERNATE );
				break;

			case EV_USE:
				G_Use ( ent );
				break;

			default:
				break;
		}
	}

}

/*
==============
StuckInOtherClient
==============
*/
static int StuckInOtherClient(gentity_t *ent) 
{
	int i;
	gentity_t	*ent2;

	ent2 = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent2++ ) 
	{
		if ( ent2 == ent ) 
		{
			continue;
		}

		if ( !ent2->inuse ) 
		{
			continue;
		}
		
		if ( !ent2->client ) 
		{
			continue;
		}
		
		if ( ent2->health <= 0 ) 
		{
			continue;
		}
		
		//
		if (ent2->r.absmin[0] > ent->r.absmax[0])
			continue;	
		if (ent2->r.absmin[1] > ent->r.absmax[1])
			continue;
		if (ent2->r.absmin[2] > ent->r.absmax[2])
			continue;
		if (ent2->r.absmax[0] < ent->r.absmin[0])
			continue;
		if (ent2->r.absmax[1] < ent->r.absmin[1])
			continue;
		if (ent2->r.absmax[2] < ent->r.absmin[2])
			continue;
		return qtrue;
	}
	return qfalse;
}

void BotTestSolid(vec3_t origin);

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) 
{
	gclient_t	*client;
	pmove_t		pm;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) 
	{
		return;
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) 
	{
		ucmd->serverTime = level.time + 200;
	}
	
	if ( ucmd->serverTime < level.time - 1000 ) 
	{
		ucmd->serverTime = level.time - 1000;
	} 
	
	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
		return;
	}

	if ( msec > 200 ) 
	{
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) 
	{
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) 
	{
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) 
	{
//		if ( g_allowPmoveFixed.integer )
//		{
			ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
//		}
//		else
//		{
//			pmove_fixed.integer = 0;
//			client->pers.pmoveFixed = 0;
//		}
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) 
	{
		ClientIntermissionThink( client );
		return;
	}

	// Perform once-per-second actions
	if ( level.time > client->sess.modData->oneSecTimer )
	{
		client->sess.modData->oneSecTimer = level.time + 1000;
		ClientOneSecActions( ent );
	}

	// Perform periodic actions
	if ( level.time > client->sess.modData->periodicTimer )
	{
		client->sess.modData->periodicTimer = level.time + 3000;
		ClientPeriodicActions( ent );
	}

	if ( g_clientFeatures.integer )
	{
		ClientSpecList( ent );
	}

	if ( level.time > client->sess.modData->scanTimer && g_scannerInterval.integer && !client->sess.modData->scanPaused && client->sess.modData->versionVerified && scanlist.scanListCount )
	{
		client->sess.modData->scanTimer = level.time + (g_scannerInterval.integer * 1000);
		ClientFileScanner( ent );
	}

	// spectators don't do much
	if ( G_IsClientSpectating ( client ) ) 
	{
		// check for inactivity timer, but never drop the local client of a non-dedicated server
		if ( g_inactivitySpectator.integer && !ClientInactivityTimer( client ) ) 
		{
			return;
		}

		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
		{
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) 
	{
		return;
	}
	if ( !ClientCampTimer( client ) ) 
	{
		return;
	}

	if ( client->noclip ) 
	{
		client->ps.pm_type = PM_NOCLIP;
	} 
	else if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		client->ps.pm_type = PM_DEAD;
	}
	else 
	{
		client->ps.pm_type = PM_NORMAL;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));

	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	if ( pm.ps->pm_type == PM_DEAD ) 
	{
		pm.ps->loopSound = 0;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( client->siameseTwin ) 
	{
		// Make sure we are still stuck, if so, clip through players.
		if ( G_IsClientSiameseTwin ( ent, client->siameseTwin ) )
		{
			pm.tracemask = MASK_PLAYERSOLID & ~(CONTENTS_BODY);
		}
		else
		{
			// Ok, we arent stuck anymore so we can clear the stuck flag.
			client->siameseTwin->client->siameseTwin = NULL;
			client->siameseTwin = NULL;

			pm.tracemask = MASK_PLAYERSOLID;
		}
	}
	else if ( ent->r.svFlags & SVF_BOT ) 
	{
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else 
	{
		pm.tracemask = MASK_PLAYERSOLID;
	}
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.animations = NULL;

#if _Debug
	pm.isClient=0;
#endif

	VectorCopy( client->ps.origin, client->oldOrigin );

	Pmove (&pm);

	G_UpdatePlayerStateScores ( ent );

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) 
	{
		ent->eventTime = level.time;
	}
	
	// See if the invulnerable flag should be removed for this client
	if ( ent->client->ps.eFlags & EF_INVULNERABLE )
	{
		if ( level.time - ent->client->invulnerableTime >= g_respawnInvulnerability.integer * 1000 )
		{
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
		}
	}

	if (g_smoothClients.integer)
	{
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else 
	{
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}

	SendPendingPredictableEvents( &ent->client->ps );

	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) 
	{
		client->fireHeld = qfalse;		// for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// Need to cache off the firemodes to the persitant data segment so they
	// are maintained across spectating and respawning
	memcpy ( ent->client->pers.firemode, ent->client->ps.firemode, sizeof(ent->client->ps.firemode ) );

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// Update the client animation info
	G_UpdateClientAnimations ( ent );
	
	if ( ent->client->ps.pm_flags & PMF_LEANING )
	{
		ent->r.svFlags |= SVF_LINKHACK;
	}
	else
	{
		ent->r.svFlags &= ~SVF_LINKHACK;
	}

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->client->noclip ) 
	{
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// Update the clients anti-lag history
	G_UpdateClientAntiLag ( ent );

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) 
	{
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{			
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) 
		{
			if ( client->sess.modData->adminref > 1 && g_allowAdminGhost.integer && (client->buttons & BUTTON_WALKING) && !(client->oldbuttons & BUTTON_WALKING) )
			{
				if ( !ent->client->sess.modData->adminFreezeRespawn )
				{
					ent->client->sess.modData->adminFreezeRespawn = qtrue;
					G_StartGhosting( ent );
				}
				else
				{
					ent->client->sess.modData->adminFreezeRespawn = qfalse;
					respawn( ent );
				}
				return;
			}

			if ( g_forcerespawn.integer > 0 && 
				( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 ) 
			{
				respawn( ent );
				return;
			}

			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & BUTTON_ATTACK ) 
			{
				respawn( ent );
			}
		}

		return;
	}

	// perform once-per-frame actions
	ClientTimerActions( ent, msec );
}

/*
==================
G_CheckClientTeamkill

Checks to see whether or not this client should be booted from the server
because they killed too many teammates
==================
*/
void G_CheckClientTeamkill ( gentity_t* ent )
{
	char userinfo[MAX_INFO_STRING];
	char *value;

	if ( ent->client->sess.modData->adminref > 1 )
	{
		return;
	}

	if ( !g_teamkillDamageMax.integer || !level.gametypeData->teams || !ent->client->sess.teamkillDamage ) 
	{
		return;
	}
	// See if they crossed the max team kill damage
	else if ( ent->client->sess.teamkillDamage < g_teamkillDamageMax.integer )
	{
		int i;

		// Does the client need to be vote kicked?
		if ( g_teamkillVoteKick.integer && (100 * ent->client->sess.teamkillDamage / g_teamkillDamageMax.integer) > g_teamkillVoteKick.integer )
		{
			if ( !level.voteTime && !ent->client->sess.modData->kickVoteCalled && (!g_protectClan.integer || (g_protectClan.integer && !ent->client->sess.modData->clan)) && ent->client->sess.modData->adminref < 2 && ent->client->sess.modData->adminref > -1 )
			{
				ent->client->sess.modData->kickVoteCalled = qtrue;

				if ( g_voteKickBanTime.integer )
				{
					Com_sprintf ( level.voteString, sizeof(level.voteString ), "banclient %d %d vote kicked for teamkilling", ent->s.number, g_voteKickBanTime.integer );
				}
				else
				{
					Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", ent->s.number );
				}
				level.voteTime = level.time;
				level.voteYes = 1;
				level.voteNo = 0;

				for ( i = 0 ; i < level.maxclients ; i++ ) 
				{
					level.clients[i].ps.eFlags &= ~EF_VOTED;
				}
				ent->client->ps.eFlags |= EF_VOTED;

				level.numVotesNeeded = (level.numVotingClients / 2) + 1;

				trap_SetConfigstring( CS_VOTE_TIME, va("%i,%i", level.voteTime, g_voteDuration.integer*1000 ) );
				trap_SetConfigstring( CS_VOTE_STRING, va("auto kick %s  ^3[^1tk: %i^3] [^5idle: %i sec^3]", ent->client->pers.netname, g_teamkillDamageMax.integer ? 100 * ent->client->sess.teamkillDamage / g_teamkillDamageMax.integer : 0, (level.time - ent->client->idleTime) / 1000) );	
				trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
				trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
				trap_SetConfigstring( CS_VOTE_NEEDED, va("%i", level.numVotesNeeded ) );
			}
		}

		// Does the client need a reminder?
		if ( g_teamkillAutoPenaltyBox.integer && ent->client->sess.teamkillDamage >= g_teamkillAutoPenaltyBox.integer && !ent->client->sess.modData->teamkillPenaltyDone )
		{
			AdminCmd_DoPenalty( ent, level.time + (g_teamkillAutoPenaltyBoxTime.integer * 1000), 2 );
			ent->client->sess.modData->teamkillPenaltyDone = qtrue;
			G_Sound(ent, CHAN_AUTO, G_SoundIndex( "sound/movers/doors/airlock_door02/airlock02_doorstop.mp3" ) );
			trap_SendServerCommand( -1, va("cp \"@%s^3 has been thrown into\nthe ^1penalty box^3!\n^7Reason: ^5Team killing\n\"", ent->client->pers.netname));
			trap_SendServerCommand( -1, va("print \"@%s^3 has been thrown into the ^1penalty box^3 for ^5Team killing!\n\"", ent->client->pers.netname));
			return;
		}

		// Does the client need forgiving?
		if ( ent->client->sess.teamkillForgiveTime )
		{
			// Are we in a forgiving mood yet?
			if ( level.time > ent->client->sess.teamkillForgiveTime + 60000 )
			{				
				ent->client->sess.teamkillForgiveTime += 60000;
				ent->client->sess.teamkillDamage -= g_teamkillDamageForgive.integer;

				if ( (100 * ent->client->sess.teamkillDamage / g_teamkillDamageMax.integer) < g_teamkillVoteKick.integer )
				{
					ent->client->sess.modData->kickVoteCalled = qfalse;
				}
			}
		}

		// All forgivin now?
		if ( ent->client->sess.teamkillDamage <= 0 )
		{
			ent->client->sess.teamkillDamage = 0;
			ent->client->sess.teamkillForgiveTime = 0;
		}

		return;
	}

	if ( g_protectClan.integer && ent->client->sess.modData->clan )
	{
		return;
	}

	trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
	value = Info_ValueForKey (userinfo, "ip");

	G_LogPrintf( "ClientKick: %i %s - auto kick for teamkilling\n", ent->s.number, value );

	ent->client->sess.teamkillDamage      = 0;
	ent->client->sess.teamkillForgiveTime = 0;

	// Keep track of who was autokicked so we can display a list if need be
	Com_sprintf ( level.autokickedIP[level.autokickedHead], sizeof(level.autokickedIP[0]), value );
	Com_sprintf ( level.autokickedName[level.autokickedHead], sizeof(level.autokickedName[0]), ent->client->pers.netname );
	level.autokickedCount++;
	if ( level.autokickedCount >= MAX_AUTOKICKLIST )
	{
		level.autokickedCount = MAX_AUTOKICKLIST;
	}

	level.autokickedHead++;
	if ( level.autokickedHead >= MAX_AUTOKICKLIST )
	{
		level.autokickedHead = 0;
	}

	// Buh bye
	if ( g_teamkillBanTime.integer )
	{
		trap_SendConsoleCommand( EXEC_INSERT, va("banclient \"%d\" \"%d\" \"team killing\"\n", ent->s.number, g_teamkillBanTime.integer ) );
	}
	else
	{
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"team killing\"\n", ent->s.number ) );
	}
}

/*
==================
G_CheckClientSpawnkill
==================
*/
void G_CheckClientSpawnkill ( gentity_t* ent )
{
	if ( ent->client->sess.modData->adminref > 1 )
	{
		return;
	}

	if ( (!level.gametypeData->teams && !g_spawnkillDeathmatch.integer) || !ent->client->sess.spawnkillDamage )
	{
		return;
	}

	// Does the client need a reminder?
	if ( g_spawnkillAutoPenaltyBox.integer && ent->client->sess.spawnkillDamage >= g_spawnkillAutoPenaltyBox.integer && !ent->client->sess.modData->spawnkillPenaltyDone )
	{
		AdminCmd_DoPenalty( ent, level.time + (g_spawnkillAutoPenaltyBoxTime.integer * 1000), 4 );
		ent->client->sess.modData->spawnkillPenaltyDone = qtrue;
		G_Sound(ent, CHAN_AUTO, G_SoundIndex( "sound/movers/doors/airlock_door02/airlock02_doorstop.mp3" ) );
		trap_SendServerCommand( -1, va("cp \"@%s^3 has been thrown into\nthe ^1penalty box^3!\n^7Reason: ^5Spawn killing\n\"", ent->client->pers.netname));
		trap_SendServerCommand( -1, va("print \"@%s^3 has been thrown into the ^1penalty box^3 for ^5Spawn killing!\n\"", ent->client->pers.netname));
		return;
	}

	// Does the client need forgiving?
	if ( ent->client->sess.spawnkillForgiveTime )
	{
		// Are we in a forgiving mood yet?
		if ( level.time > ent->client->sess.spawnkillForgiveTime + 60000 )
		{				
			ent->client->sess.spawnkillForgiveTime += 60000;
			ent->client->sess.spawnkillDamage -= g_teamkillDamageForgive.integer;
		}
	}

	// All forgivin now?
	if ( ent->client->sess.spawnkillDamage <= 0 )
	{
		ent->client->sess.spawnkillDamage = 0;
		ent->client->sess.spawnkillForgiveTime = 0;
	}
}

/*
==================
G_CheckClientFry
==================
*/
void G_CheckClientFry ( gentity_t* ent )
{
	if ( !ent->client->sess.modData->fryFireTime )
	{
		return;
	}

	if ( G_IsClientDead ( ent->client ) ) {
		ent->client->sess.modData->fryFireTime = 0;
		ent->client->sess.modData->fryPainTime = 0;
		return;
	}

	if ( level.time - ent->client->sess.modData->fryFireTime > 2000 )
	{
		gentity_t	*tent;
		vec3_t		hitdir;

		ent->client->sess.modData->fryFireTime = level.time;

		tent = G_TempEntity( ent->r.currentOrigin, EV_EXPLOSION_HIT_FLESH );
		VectorSubtract(ent->r.currentOrigin, ent->r.currentOrigin, hitdir);
		VectorNormalize(hitdir);
		tent->s.eventParm = DirToByte( hitdir );
		tent->s.otherEntityNum2 = ent->s.number;
		tent->s.time  = WP_ANM14_GRENADE + ((((int)ent->s.apos.trBase[YAW]&0x7FFF) % 360) << 16);		
		VectorCopy ( ent->r.currentOrigin, tent->s.angles );
		SnapVector ( tent->s.angles );
	}

	if ( level.time - ent->client->sess.modData->fryPainTime > 1000 )
	{
		ent->client->sess.modData->fryPainTime = level.time;

		G_Damage( ent, NULL, ent->client->sess.modData->fryInflictor, NULL, NULL, 10, DAMAGE_NO_TEAMKILL|DAMAGE_NO_PROTECTION, MOD_ADMIN_FRY, 0);
	}
}

/*
==================
G_CheckClientTimeouts

Checks whether a client has exceded any timeouts and act accordingly
==================
*/
void G_CheckClientTimeouts ( gentity_t *ent )
{
	// Only timeout supported right now is the timeout to spectator mode
	if ( !g_timeouttospec.integer )
	{
		return;
	}

	// Can only do spect timeouts on dedicated servers
	if ( !g_dedicated.integer )
	{
		return;
	}

	// Already a spectator, no need to boot them to spectator
	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		return;
	}

	// Need to be connected
	if ( ent->client->pers.connected != CON_CONNECTED )
	{
		return;
	}

	// See how long its been since a command was received by the client and if its 
	// longer than the timeout to spectator then force this client into spectator mode
	if ( level.time - ent->client->pers.cmd.serverTime > g_timeouttospec.integer * 1000 )
	{
		SetTeam ( ent, "spectator", NULL );
	}
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) 
{
	gentity_t *ent;

	ent = g_entities + clientNum;
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) 
	{
		ClientThink_real( ent );
	}
}

/*
==================
G_RunClient
==================
*/
void G_RunClient( gentity_t *ent ) 
{
	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) 
	{		
		return;
	}

	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame
==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) 
{
	gclient_t	*cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) 
	{
		int		clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) 
		{
			clientNum = level.follow1;
		} 
		else if ( clientNum == -2 ) 
		{
			clientNum = level.follow2;
		}
		
		if ( clientNum >= 0 ) 
		{
			cl = &level.clients[ clientNum ];
		
			if ( cl->pers.connected == CON_CONNECTED && !G_IsClientSpectating ( cl ) ) 
			{
				int count;
				int ping;
				int score;
				int respawnTimer;

				count = ent->client->ps.persistant[PERS_SPAWN_COUNT];
				ping  = ent->client->ps.ping;
				score = ent->client->ps.persistant[PERS_SCORE];
				flags = (cl->ps.eFlags & ~(EF_VOTED)) | (ent->client->ps.eFlags & (EF_VOTED));
				respawnTimer = ent->client->ps.respawnTimer;

				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				if ( ent->client->sess.spectatorFirstPerson ) 
				{
					ent->client->ps.pm_flags |= PMF_FOLLOWFIRST;
				}
				ent->client->ps.eFlags = flags;
				ent->client->ps.persistant[PERS_SPAWN_COUNT] = count;
				ent->client->ps.persistant[PERS_SCORE] = score;
				ent->client->ps.ping = ping;
				ent->client->ps.respawnTimer = respawnTimer;

				return;
			} 
			else 
			{
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) 
				{
					Cmd_FollowCycle_f (ent, 1);
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
	{
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} 
	else 
	{
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) 
{
	clientPersistant_t	*pers;

	if ( G_IsClientSpectating ( ent->client ) ) 
	{
		SpectatorClientEndFrame( ent );
		return;
	}

	pers = &ent->client->pers;

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) 
	{
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) 
	{
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) 
	{
		ent->s.eFlags |= EF_CONNECTION;
	} 
	else 
	{
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	// FIXME: get rid of ent->health...
	ent->client->ps.stats[STAT_HEALTH] = ent->health;	

	G_SetClientSound (ent);

	// set the latest infor
	if (g_smoothClients.integer) 
	{
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else 
	{
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	
	SendPendingPredictableEvents( &ent->client->ps );

	// set the bit for the reachability area the client is currently in
//	i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
//	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}


