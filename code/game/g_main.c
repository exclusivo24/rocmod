// Copyright (C) 2001-2002 Raven Software.
//

#include "g_local.h"

level_locals_t	level;
voiceCommands_t	voicecmds;
scanList_t		scanlist;
clientRecon_t	recondata[MAX_CLIENTS];
chatFilter_t	chatfilter;

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

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];
sessionMod_t	sessionData[MAX_CLIENTS];

vmCvar_t	g_gametype;
vmCvar_t	g_dmflags;
vmCvar_t	g_scorelimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_friendlyFire;
vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_backpackRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;
vmCvar_t	g_warmup;
vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t	g_log;
vmCvar_t	g_adminlog;
vmCvar_t	g_logSync;
vmCvar_t	g_logHits;
vmCvar_t	g_allowVote;
vmCvar_t	g_voteDuration;
vmCvar_t	g_voteKickBanTime;
vmCvar_t	g_failedVoteDelay;
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	g_rankings;
vmCvar_t	g_listEntity;
vmCvar_t	g_fps;
vmCvar_t	g_respawnInterval;
vmCvar_t	g_respawnInvulnerability;
vmCvar_t	g_roundtimelimit;
vmCvar_t	g_roundjointime;
vmCvar_t	g_timeextension;
vmCvar_t	g_timeouttospec;
vmCvar_t	g_roundstartdelay;
vmCvar_t	g_availableWeapons;
vmCvar_t	g_forceFollow;
vmCvar_t	g_followEnemy;
vmCvar_t	g_mapcycle;
vmCvar_t	g_pickupsDisabled;				// Whether or not pickups are available in a map (uses outfitting if not)
vmCvar_t	g_suicidePenalty;				// Amount of score added for killing yourself (typically negative)
vmCvar_t	g_teamkillPenalty;				// Amount of score added for killing a teammates (typically negative)
vmCvar_t	g_teamkillDamageMax;			// max damage one can do to teammates before being kicked
vmCvar_t	g_teamkillDamageForgive;		// amount of teamkill damage forgiven each minute
vmCvar_t	g_teamkillBanTime;				// number of minutes to ban someone for after being kicked 
vmCvar_t	g_teamkillNoExcuseTime;			// number of seconds into a round where tk damage is inexcusable
vmCvar_t	g_teamkillNoExcuseMultiplier;	// multipier for tk damage that is inexcusable
vmCvar_t	g_voiceFloodCount;				// Number of voice messages in one minute to be concidered flooding
vmCvar_t	g_voiceFloodPenalty;			// Amount of time a void flooder must wait before they can use voice again
vmCvar_t	g_suddenDeath;
vmCvar_t	g_voiceTalkingGhosts;			// Allow ghosts to talk to alive players

vmCvar_t	RMG;
vmCvar_t	g_debugRMG;

vmCvar_t	sv_adminfile;
vmCvar_t	sv_sysopfile;
vmCvar_t	sv_clanfile;
vmCvar_t	sv_refereefile;
vmCvar_t	sv_extracmdsfile;
vmCvar_t	sv_voicecmdsfile;
vmCvar_t	sv_modClient;
vmCvar_t	sv_logo;
vmCvar_t	sv_modVersion;
vmCvar_t	sv_minRate;
vmCvar_t	sv_filterchatfile;
vmCvar_t	g_motd1;
vmCvar_t	g_motd2;
vmCvar_t	g_motd3;
vmCvar_t	g_motd4;
vmCvar_t	g_motd5;
vmCvar_t	g_motd6;
vmCvar_t	g_motd7;
vmCvar_t	g_motd8;
vmCvar_t	g_motd9;
vmCvar_t	g_motd10;
vmCvar_t	g_motdDisplayTime;
vmCvar_t	g_motdEnabled;
vmCvar_t	g_teamkillNoDouble;
vmCvar_t	g_teamkillAutoPenaltyBox;
vmCvar_t	g_teamkillAutoPenaltyBoxTime;
vmCvar_t	g_teamkillWarning;
vmCvar_t	g_teamkillBackfire;
vmCvar_t	g_flagstealAutoPenaltyBox;
vmCvar_t	g_flagstealAutoPenaltyBoxTime;
vmCvar_t	g_flagstealWarning;
vmCvar_t	g_spawnkillAutoPenaltyBox;
vmCvar_t	g_spawnkillAutoPenaltyBoxTime;
vmCvar_t	g_spawnkillWarning;
vmCvar_t	g_spawnkillBackfire;
vmCvar_t	g_voiceCommandsEnabled;
vmCvar_t	g_sysopImmuneToAdmin;
vmCvar_t	g_mapcycleSwapTeams;
vmCvar_t	g_mapcycleShuffleTeams;
vmCvar_t	g_verifyClients;
vmCvar_t	g_verifyClientsTime;
vmCvar_t	g_verifyClientsMax;
vmCvar_t	g_disableAdminInPenaltyBox;
vmCvar_t	g_bestStatsMinKills;
vmCvar_t	g_allowColorNames;
vmCvar_t	g_allowWarmupFriendlyFire;
vmCvar_t	g_realisticSniper;
vmCvar_t	g_realisticShotgun;
vmCvar_t	g_intermissionDelay;
vmCvar_t	g_allowTalk;
vmCvar_t	g_allowChat;
vmCvar_t	g_allowKick;
vmCvar_t	g_allowBan;
vmCvar_t	g_allowCancelVote;
vmCvar_t	g_allowPassVote;
vmCvar_t	g_allowWarmup;
vmCvar_t	g_allowFriendlyFire;
vmCvar_t	g_allowMute;
vmCvar_t	g_allowStrike;
vmCvar_t	g_allowSlap;
vmCvar_t	g_allowStrip;
vmCvar_t	g_allowSwitch;
vmCvar_t	g_allowPenalty;
vmCvar_t	g_allowUnPenalty;
vmCvar_t	g_allowGametype;
vmCvar_t	g_allowMapRestart;
vmCvar_t	g_allowMapcycle;
vmCvar_t	g_allowSetMap;
vmCvar_t	g_allowRMG;
vmCvar_t	g_allowExtendTime;
vmCvar_t	g_allowTimelimit;
vmCvar_t	g_allowScorelimit;
vmCvar_t	g_allowSwapTeams;
vmCvar_t	g_allowShuffleTeams;
vmCvar_t	g_allowEndMap;
vmCvar_t	g_allowLockTeam;
vmCvar_t	g_allowSuspendAll;
vmCvar_t	g_allowFry;
vmCvar_t	g_allowLaunch;
vmCvar_t	g_allowExplode;
vmCvar_t	g_allowPlant;
vmCvar_t	g_allowTelefrag;
vmCvar_t	g_allowSurrender;
vmCvar_t	g_allowRespawn;
vmCvar_t	g_allowBait;
vmCvar_t	g_allowEvenTeams;
vmCvar_t	g_allowInvite;
vmCvar_t	g_allowForceSay;
vmCvar_t	g_allowSwap;
vmCvar_t	g_allowClanVsAll;
vmCvar_t	g_allowPause;
vmCvar_t	g_allowUnPause;
vmCvar_t	g_allowFreezeMap;
vmCvar_t	g_allowPbKick;
vmCvar_t	g_allowPbBan;
vmCvar_t	g_allowPbGetSS;
vmCvar_t	g_defaultPenaltyBoxTime;
vmCvar_t	g_teamChangeDeath;
vmCvar_t	g_bestStatsTime;
vmCvar_t	g_timeRemaining;
vmCvar_t	g_suspendedAdmin;
vmCvar_t	g_inactivitySpectator;
vmCvar_t	g_explosionGore;
vmCvar_t	g_message1;
vmCvar_t	g_message2;
vmCvar_t	g_message3;
vmCvar_t	g_message4;
vmCvar_t	g_message5;
vmCvar_t	g_message6;
vmCvar_t	g_message7;
vmCvar_t	g_message8;
vmCvar_t	g_message9;
vmCvar_t	g_message10;
vmCvar_t	g_messageStart;
vmCvar_t	g_messageInterval;
vmCvar_t	g_messageEnabled;
vmCvar_t	g_anonymousAdmin;
vmCvar_t	g_killSpree;
vmCvar_t	g_adminBackfire;
vmCvar_t	g_emptyMapcycle;
vmCvar_t	g_enableAdmin;
vmCvar_t	g_enableReferee;
vmCvar_t	g_alternateSort;
vmCvar_t	g_clientFeatures;
vmCvar_t	g_weaponMod;
vmCvar_t	g_protectClan;
vmCvar_t	g_spawnkillDeathmatch;
vmCvar_t	g_suicideTeamPenalty;
vmCvar_t	g_dropWeapon;
vmCvar_t	g_lastPlayer;
vmCvar_t	g_teamkillVoteKick;
vmCvar_t	g_adminVote;
vmCvar_t	g_defaultBanTime;
vmCvar_t	g_lockSpec;
vmCvar_t	g_refereePass;
vmCvar_t	g_realisticAmmo;
vmCvar_t	g_punishRecon;
vmCvar_t	g_allowAdminGhost;
vmCvar_t	g_camper;
vmCvar_t	g_allowMatch;
//vmCvar_t	g_anonymousReferee;
vmCvar_t	g_messageNew;
vmCvar_t	g_allowDummy;
vmCvar_t	g_voteLimit;
vmCvar_t	g_allowNoVote;
vmCvar_t	g_customBlueName;
vmCvar_t	g_customBlueCode;
vmCvar_t	g_customBlueShade;
vmCvar_t	g_customRedName;
vmCvar_t	g_customRedCode;
vmCvar_t	g_customRedShade;
vmCvar_t	g_customClanShade;
vmCvar_t	g_allowCustomTeams;
vmCvar_t	g_teamSkins; // 01/26/05 pimpy
vmCvar_t	g_allowItemDrop;
vmCvar_t	g_allowRename;
vmCvar_t	g_filterChat;
vmCvar_t	g_globalMute;
vmCvar_t	g_camperRadius;
vmCvar_t	g_camperMode;
vmCvar_t	g_camperAction;
//vmCvar_t	g_allowPmoveFixed;
vmCvar_t	g_allowCvarCheck;
vmCvar_t	g_allowCvarSet;
vmCvar_t	g_allowCfg;
vmCvar_t	g_allowTag;
vmCvar_t	g_allowTagVsAll;
vmCvar_t	g_enableModerator;
vmCvar_t	g_autoMatchDemo;
vmCvar_t	g_inactivityAction;
vmCvar_t	g_allowCripple;
vmCvar_t	g_allowBackfire;
vmCvar_t	g_allowPassword;
vmCvar_t	g_duplicatePlayerMax;
vmCvar_t	g_allowReset;
vmCvar_t	g_autoEvenTeams;
vmCvar_t	g_autoEvenTeamsInterval;
vmCvar_t	sv_adminControl;
vmCvar_t	g_dummyReward;
vmCvar_t	sv_scannerfile;
vmCvar_t	g_scannerInterval;
vmCvar_t	g_scannerRepeat;
vmCvar_t	g_scannerDefaultAction;
vmCvar_t	g_scannerlog;

vmCvar_t	inMatch;
vmCvar_t	exitReady;
vmCvar_t	disable_armor;
vmCvar_t	disable_thermal;
vmCvar_t	disable_nightvision;

static cvarTable_t gameCvarTable[] = 
{
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0.0, 0.0, 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "gameversion", ROCMOD_STRING , CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },

	{ &g_fps, "sv_fps", "", CVAR_ROM, 0.0, 0.0, 0, qfalse },

	// latched vars
	{ &g_gametype, "g_gametype", "dm", CVAR_SERVERINFO | CVAR_LATCH, 0.0, 0.0, 0, qfalse  },

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_scorelimit, "scorelimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse  },

	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_SERVERINFO|CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE, 0.0, 0.0,   },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE, 0.0, 0.0,   },

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_log, "g_log", "games.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_adminlog, "g_adminlog", "admin.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_logHits, "g_logHits", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0.0, 0.0, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0.0, 0.0, 0, qfalse  },

	{ &g_speed, "g_speed", "280", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_knockback, "g_knockback", "700", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponrespawn", "15", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_backpackRespawn, "g_backpackrespawn", "40", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_forcerespawn, "g_forcerespawn", "5", 0, 0.0, 0.0, 0, qtrue },
	{ &g_inactivity, "g_inactivity", "180", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0.0, 0.0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0.0, 0.0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0.0, 0.0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0.0, 0.0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_voteDuration, "g_voteDuration", "60", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_voteKickBanTime, "g_voteKickBanTime", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_failedVoteDelay, "g_failedVoteDelay", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0.0, 0.0, 0, qfalse },

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0.0, 0.0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse},

	{ &g_rankings, "g_rankings", "0", 0, 0.0, 0.0, 0, qfalse},

	{ &g_respawnInterval, "g_respawnInterval", "15", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_respawnInvulnerability, "g_respawnInvulnerability", "5", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &RMG, "RMG", "0", 0, 0.0, 0.0, },
	{ &g_debugRMG, "g_debugRMG", "0", 0, 0.0f, 0.0f },

	{ &g_timeouttospec,		"g_timeouttospec",	"15",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_roundtimelimit,	"g_roundtimelimit",	"5",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_roundjointime,		"g_roundjointime",	"5",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_timeextension,		"g_timeextension",	"15",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_roundstartdelay,	"g_roundstartdelay", "5",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	// 1.03 CHANGE - Rename availableWeapons Cvar which might confuse old map cycles
	{ &g_availableWeapons,	"g_available", "0", CVAR_SERVERINFO|CVAR_ROM|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

	{ NULL,					"disable_weapon_knife",					"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_US_SOCOM",		"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M19",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_silvertalon",	"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_microuzi",		"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M3A1",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_MP5",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_USAS_12",		"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M590",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_MSG90A1",		"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M4",				"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_AK_74",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_SIG551",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M60",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_RPG_7",			"2", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_MM_1",			"2", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M84",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_SMOHG92",		"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_AN_M14",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M15",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
#ifdef SMKWEAPONS
	{ NULL,					"disable_pickup_weapon_OICW",			"0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse }, 
#endif
	{ &disable_armor,		"disable_armor",						"0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse }, 
	{ &disable_thermal,		"disable_thermal",						"0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse }, 
	{ &disable_nightvision,	"disable_nightvision",					"0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse }, 

	{ &g_forceFollow,		"g_forceFollow",	 "0",			  CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_followEnemy,		"g_followEnemy",	 "1",			  CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_mapcycle,			"sv_mapcycle",		 "none",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_pickupsDisabled,	"g_pickupsDisabled", "0",					CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

	{ &g_suicidePenalty,	"g_suicidePenalty",  "-1",					CVAR_ARCHIVE,	0.0f,	0.0f,	0,	qfalse },

	{ &g_voiceFloodCount,	"g_voiceFloodCount",	"6",				CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },
	{ &g_voiceFloodPenalty,	"g_voiceFloodPenalty",	"60",				CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },

	{ &g_teamkillPenalty,			 "g_teamkillPenalty",			 "-1",	CVAR_ARCHIVE,	0.0f,	0.0f,	0,	qfalse },
	{ &g_teamkillDamageMax,			 "g_teamkillDamageMax",			 "300",	CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },
	{ &g_teamkillDamageForgive,		 "g_teamkillDamageForgive",		 "50",	CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },
	{ &g_teamkillBanTime,			 "g_teamkillBanTime",			 "5",	CVAR_ARCHIVE,   0.0f,	0.0f,	0,  qfalse },
	{ &g_teamkillNoExcuseTime,		 "g_teamkillNoExcuseTime",		 "8",	CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },
	{ &g_teamkillNoExcuseMultiplier, "g_teamkillNoExcuseMultiplier", "3",	CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },

	{ &g_suddenDeath,			"g_suddenDeath",			"1",		CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },

	{ &g_voiceTalkingGhosts,	"g_voiceTalkingGhosts",		"1",		CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },


	{ &sv_adminfile, "sv_adminfile", "admin.cfg", CVAR_ROM, 0.0f, 0.0f, 0, qfalse },
	{ &sv_sysopfile, "sv_sysopfile", "sysop.cfg", CVAR_ROM, 0.0f, 0.0f, 0, qfalse },
	{ &sv_clanfile, "sv_clanfile", "clan.cfg", CVAR_ROM, 0.0f, 0.0f, 0, qfalse },
	{ &sv_refereefile, "sv_refereefile", "referee.cfg", CVAR_ROM, 0.0f, 0.0f, 0, qfalse },
	{ &sv_extracmdsfile, "sv_extracmdsfile", "extracmds.cfg", CVAR_ROM, 0.0f, 0.0f, 0, qfalse },
	{ &sv_voicecmdsfile, "sv_voicecmdsfile", "voicecmds.cfg", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &sv_modClient, "sv_modClient", "1", CVAR_SERVERINFO, 0.0f, 0.0f, 0, qfalse },
	{ &sv_logo, "sv_logo", "gfx/blank.png", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &sv_minRate, "sv_minRate", "2500", CVAR_SERVERINFO, 0.0f, 0.0f, 0, qfalse },
	{ &sv_modVersion, "sv_modVersion", ROCMOD_VERSION, CVAR_SYSTEMINFO|CVAR_ROM|CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ &sv_filterchatfile, "sv_filterchatfile", "chatfilter.cfg", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &g_motdEnabled, "g_motdEnabled", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_motdDisplayTime, "g_motdDisplayTime", "10", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd1, "g_motd1", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd2, "g_motd2", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd3, "g_motd3", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd4, "g_motd4", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd5, "g_motd5", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd6, "g_motd6", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd7, "g_motd7", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd8, "g_motd8", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd9, "g_motd9", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_motd10, "g_motd10", "", 0, 0.0f, 0.0f, 0, qfalse },

	{ &g_teamkillNoDouble, "g_teamkillNoDouble", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamkillAutoPenaltyBox, "g_teamkillAutoPenaltyBox", "200", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamkillAutoPenaltyBoxTime, "g_teamkillAutoPenaltyBoxTime", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamkillWarning, "g_teamkillWarning", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamkillBackfire, "g_teamkillBackfire", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_flagstealAutoPenaltyBox, "g_flagstealAutoPenaltyBox", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_flagstealAutoPenaltyBoxTime, "g_flagstealAutoPenaltyBoxTime", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_flagstealWarning, "g_flagstealWarning", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_spawnkillAutoPenaltyBox, "g_spawnkillAutoPenaltyBox", "200", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_spawnkillAutoPenaltyBoxTime, "g_spawnkillAutoPenaltyBoxTime", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_spawnkillWarning, "g_spawnkillWarning", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_spawnkillBackfire, "g_spawnkillBackfire", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_voiceCommandsEnabled, "g_voiceCommandsEnabled", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &g_sysopImmuneToAdmin, "g_sysopImmuneToAdmin", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_mapcycleSwapTeams, "g_mapcycleSwapTeams", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_mapcycleShuffleTeams, "g_mapcycleShuffleTeams", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_verifyClients, "g_verifyClients", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_verifyClientsTime, "g_verifyClientsTime", "30", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_verifyClientsMax, "g_verifyClientsMax", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &g_disableAdminInPenaltyBox, "g_disableAdminInPenaltyBox", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_bestStatsMinKills, "g_bestStatsMinKills", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowColorNames, "g_allowColorNames", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowWarmupFriendlyFire, "g_allowWarmupFriendlyFire", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_realisticSniper, "g_realisticSniper", "1", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse  },
	{ &g_realisticShotgun, "g_realisticShotgun", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_intermissionDelay, "g_intermissionDelay", "10", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &g_allowTalk, "g_allowTalk", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowChat, "g_allowChat", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowKick, "g_allowKick", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowBan, "g_allowBan", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowCancelVote, "g_allowCancelVote", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPassVote, "g_allowPassVote", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowWarmup, "g_allowWarmup", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowFriendlyFire, "g_allowFriendlyFire", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowMute, "g_allowMute", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowStrike, "g_allowStrike", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSlap, "g_allowSlap", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowStrip, "g_allowStrip", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSwitch, "g_allowSwitch", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPenalty, "g_allowPenalty", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowUnPenalty, "g_allowUnPenalty", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowGametype, "g_allowGametype", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowMapRestart, "g_allowMapRestart", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowMapcycle, "g_allowMapcycle", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSetMap, "g_allowSetMap", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowRMG, "g_allowRMG", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowExtendTime, "g_allowExtendTime", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowTimelimit, "g_allowTimelimit", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowScorelimit, "g_allowScorelimit", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSwapTeams, "g_allowSwapTeams", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowShuffleTeams, "g_allowShuffleTeams", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowEndMap, "g_allowEndMap", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowLockTeam, "g_allowLockTeam", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSuspendAll, "g_allowSuspendAll", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowFry, "g_allowFry", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowLaunch, "g_allowLaunch", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowExplode, "g_allowExplode", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPlant, "g_allowPlant", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowTelefrag, "g_allowTelefrag", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSurrender, "g_allowSurrender", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowRespawn, "g_allowRespawn", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowBait, "g_allowBait", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowEvenTeams, "g_allowEvenTeams", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowInvite, "g_allowInvite", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowForceSay, "g_allowForceSay", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowSwap, "g_allowSwap", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowClanVsAll, "g_allowClanVsAll", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPause, "g_allowPause", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowUnPause, "g_allowUnPause", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPbKick, "g_allowPbKick", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPbBan, "g_allowPbBan", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPbGetSS, "g_allowPbGetSS", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowFreezeMap, "g_allowFreezeMap", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowMatch, "g_allowMatch", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowDummy, "g_allowDummy", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowNoVote, "g_allowNoVote", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowRename, "g_allowRename", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowCvarCheck, "g_allowCvarCheck", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowCvarSet, "g_allowCvarSet", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowCfg, "g_allowCfg", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowTag, "g_allowTag", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowTagVsAll, "g_allowTagVsAll", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowCripple, "g_allowCripple", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowBackfire, "g_allowBackfire", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowPassword, "g_allowPassword", "0200", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowReset, "g_allowReset", "0200", 0, 0.0f, 0.0f, 0, qfalse },

	{ &g_defaultPenaltyBoxTime, "g_defaultPenaltyBoxTime", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamChangeDeath, "g_teamChangeDeath", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_bestStatsTime, "g_bestStatsTime", "10", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_timeRemaining, "g_timeRemaining", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_suspendedAdmin, "g_suspendedAdmin", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_inactivitySpectator, "g_inactivitySpectator", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_explosionGore, "g_explosionGore", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_anonymousAdmin, "g_anonymousAdmin", "0", CVAR_SYSTEMINFO|CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_killSpree, "g_killSpree", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &g_message1, "g_message1", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message2, "g_message2", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message3, "g_message3", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message4, "g_message4", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message5, "g_message5", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message6, "g_message6", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message7, "g_message7", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message8, "g_message8", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message9, "g_message9", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_message10, "g_message10", "", 0, 0.0f, 0.0f, 0, qfalse },
	{ &g_messageStart, "g_messageStart", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_messageInterval, "g_messageInterval", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_messageEnabled, "g_messageEnabled", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &g_adminBackfire, "g_adminBackfire", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_emptyMapcycle, "g_emptyMapcycle", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_enableAdmin, "g_enableAdmin", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_enableReferee, "g_enableReferee", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_alternateSort, "g_alternateSort", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_clientFeatures, "g_clientFeatures", "3", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_weaponMod, "g_weaponMod", "ext_data/sof2.wpn", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_protectClan, "g_protectClan", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_spawnkillDeathmatch, "g_spawnkillDeathmatch", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_suicideTeamPenalty, "g_suicideTeamPenalty", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_dropWeapon, "g_dropWeapon", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_lastPlayer, "g_lastPlayer", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamkillVoteKick, "g_teamkillVoteKick", "75", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_adminVote, "g_adminVote", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_defaultBanTime, "g_defaultBanTime", "15", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_lockSpec, "g_lockSpec", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_refereePass, "g_refereePass", "none", CVAR_INTERNAL|CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_realisticAmmo, "g_realisticAmmo", "0", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_punishRecon, "g_punishRecon", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowAdminGhost, "g_allowAdminGhost", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_camper, "g_camper", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
//	{ &g_anonymousReferee, "g_anonymousReferee", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_messageNew, "g_messageNew", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_voteLimit, "g_voteLimit", "5", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_customBlueName, "g_customBlueName", "Blue", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_customBlueCode, "g_customBlueCode", "4", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_customBlueShade, "g_customBlueShade", "30,30,100", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_customRedName, "g_customRedName", "Red", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_customRedCode, "g_customRedCode", "1", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_customRedShade, "g_customRedShade", "100,10,10", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_customClanShade, "g_customClanShade", "100,50,0", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_allowCustomTeams, "g_allowCustomTeams", "0", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse },
	{ &g_teamSkins, "g_teamSkins", "1", CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qfalse }, //01/26/05 pimpy
	{ &g_allowItemDrop, "g_allowItemDrop", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_filterChat, "g_filterChat", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_globalMute, "g_globalMute", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_camperRadius, "g_camperRadius", "200", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_camperMode, "g_camperMode", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_camperAction, "g_camperAction", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
//	{ &g_allowPmoveFixed, "g_allowPmoveFixed", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_enableModerator, "g_enableModerator", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_autoMatchDemo, "g_autoMatchDemo", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_inactivityAction, "g_inactivityAction", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_duplicatePlayerMax, "g_duplicatePlayerMax", "3", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_autoEvenTeams, "g_autoEvenTeams", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_autoEvenTeamsInterval, "g_autoEvenTeamsInterval", "10", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &sv_adminControl, "sv_adminControl", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_dummyReward, "g_dummyReward", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &sv_scannerfile, "sv_scannerfile", "scanlist.cfg", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_scannerInterval, "g_scannerInterval", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_scannerRepeat, "g_scannerRepeat", "3", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_scannerDefaultAction, "g_scannerDefaultAction", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &g_scannerlog, "g_scannerlog", "scanner.log", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ &inMatch, "inMatch", "0", CVAR_SYSTEMINFO|CVAR_ROM|CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
	{ &exitReady, "exitReady", "0", CVAR_SYSTEMINFO|CVAR_ROM|CVAR_TEMP, 0.0f, 0.0f, 0, qfalse },
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );


void G_InitGame					( int levelTime, int randomSeed, int restart );
void G_RunFrame					( int levelTime );
void G_ShutdownGame				( int restart );
void CheckExitRules				( void );
void G_InitGhoul				( void );
void G_ShutdownGhoul			( void );

int trap_RealTime( qtime_t *qtime );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) 
{
	switch ( command ) 
	{
		case GAME_GHOUL_INIT:
			G_InitGhoul ( );
			return 0;
		case GAME_GHOUL_SHUTDOWN:
			G_ShutdownGhoul ( );
			return 0;
		case GAME_INIT:
			G_InitGame( arg0, arg1, arg2 );
			return 0;
		case GAME_SHUTDOWN:
			G_ShutdownGame( arg0 );
			return 0;
		case GAME_CLIENT_CONNECT:
			return (int)ClientConnect( arg0, arg1, arg2 );
		case GAME_CLIENT_THINK:
			ClientThink( arg0 );
			return 0;
		case GAME_CLIENT_USERINFO_CHANGED:
			ClientUserinfoChanged( arg0 );
			return 0;
		case GAME_CLIENT_DISCONNECT:
			ClientDisconnect( arg0 );
			return 0;
		case GAME_CLIENT_BEGIN:
			ClientBegin( arg0 );
			return 0;
		case GAME_CLIENT_COMMAND:
			ClientCommand( arg0 );
			return 0;
		case GAME_GAMETYPE_COMMAND:
			return G_GametypeCommand ( arg0, arg1, arg2, arg3, arg4, arg5 );
		case GAME_RUN_FRAME:
			G_RunFrame( arg0 );
			return 0;
		case GAME_CONSOLE_COMMAND:
			return ConsoleCommand();
		case BOTAI_START_FRAME:
			return BotAIStartFrame( arg0 );
		case GAME_SPAWN_RMG_ENTITY:
			if (G_ParseSpawnVars(qfalse))
			{
				G_SpawnGEntityFromSpawnVars(qfalse);
			}
			return 0;
	}

	return -1;
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	Com_Printf ("%i teams with %i entities\n", c, c2);
}

/*
=================
G_RemapTeamShaders
=================
*/
void G_RemapTeamShaders(void) 
{
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
}

/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;
	qboolean	remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) 
	{
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue );
		
		if ( cv->vmCvar )
		{
			cv->modificationCount = cv->vmCvar->modificationCount;
		}

		if (cv->teamShader) 
		{
			remapped = qtrue;
		}
	}

	if (remapped) 
	{
		G_RemapTeamShaders();
	}

	level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;
	qboolean	remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) 
	{
		if ( cv->vmCvar ) 
		{
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) 
			{
				// Handle any modified cvar checks
				if ( !Q_stricmp ( cv->cvarName, "sv_fps" ) )
				{
					if ( cv->vmCvar->integer > 50 )
					{
						trap_Cvar_Set ( "sv_fps", "50" );
						trap_Cvar_Update ( cv->vmCvar );
					}
					else if ( cv->vmCvar->integer < 10 )
					{
						trap_Cvar_Set ( "sv_fps", "10" );
						trap_Cvar_Update ( cv->vmCvar );
					}
				}						

				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) 
				{
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", cv->cvarName, cv->vmCvar->string ) );
				}

				if (cv->teamShader) 
				{
					remapped = qtrue;
				}
			}
		}
	}

	if (remapped) 
	{
		G_RemapTeamShaders();
	}
}

/*
===============
G_UpdateAvailableWeapons

Updates the g_availableWeapons cvar using the disable cvars.
===============
*/
void G_UpdateAvailableWeapons ( void )
{
	weapon_t weapon;
	char	 available[WP_NUM_WEAPONS+1];

	memset ( available, 0, sizeof(available) );

	for ( weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon ++ )
	{
		gitem_t* item = BG_FindWeaponItem ( weapon );
		if ( !item )
		{
			continue;
		}
	
		switch ( (int)trap_Cvar_VariableValue ( va("disable_%s", item->classname ) ) )
		{
			case 0:
				available[weapon-1] = '2';
				break;

			case 1:
				available[weapon-1] = '0';
				break;

			case 2:
				available[weapon-1] = '1';
				break;
		}
	}

	// 1.03 CHANGE - Rename availableWeapons Cvar which might confuse old map cycles
	trap_Cvar_Set ( "g_available", available );
    trap_Cvar_Update ( &g_availableWeapons );
}

/*
===============
G_SetGametype

Sets the current gametype to the given value, if the map doesnt support it then it will
use the first gametype that it does support
===============
*/
void G_SetGametype ( const char* gametype )
{	
	// 1.03 CHANGE - (unofficial) Fix for base gametype issue
	// Make sure the gametype is valid, if not default to deathmatch
	level.gametype = BG_FindGametype ( gametype );

	// First make sure its supported on this map
	if ((level.gametype == -1) || (!G_DoesMapSupportGametype (gametype)
	 && (!bg_gametypeData[level.gametype].basegametype || !G_DoesMapSupportGametype (bg_gametypeData[level.gametype].basegametype))))
	{
		int i = 0;
		
		// Find a gametype it does support
		for ( i = 0; i < bg_gametypeCount; i ++ )
		{
			if ( G_DoesMapSupportGametype ( bg_gametypeData[i].name ) )
			{
				break;
			}
		}

		// This is bad, this means the map doesnt support any gametypes
		if ( i >= bg_gametypeCount )
		{
			Com_Error ( ERR_FATAL, "map does not support any of the available gametypes" );
		}

		G_LogPrintf ( "gametype '%s' is not supported on this map and was defaulted to '%s'\n", 
					 gametype,
					 bg_gametypeData[i].name );

		gametype = bg_gametypeData[i].name;
		trap_Cvar_Set( "g_gametype", gametype );
		trap_Cvar_Set( "RMG_mission", gametype );
		level.gametype = BG_FindGametype ( gametype );

		trap_Cvar_Update( &g_gametype );
	}

	level.gametypeData = &bg_gametypeData[level.gametype];	

	// Copy the backpack percentage over
	bg_itemlist[MODELINDEX_BACKPACK].quantity = level.gametypeData->backpack;

	// Set the pickup state
	if ( RMG.integer || g_pickupsDisabled.integer || level.gametypeData->pickupsDisabled )
	{
		level.pickupsDisabled = qtrue;
		trap_SetConfigstring ( CS_PICKUPSDISABLED, "1" );
	}
	else
	{
		level.pickupsDisabled = qfalse;
		trap_SetConfigstring ( CS_PICKUPSDISABLED, "0" );
	}
}

/*
===============
G_InitChatFilter
===============
*/
void G_InitChatFilter ( void )
{
	char			buffer[1025];
	char			filename[256];
	fileHandle_t	f;
	int				len;
	int				maxlen;
	const char		*buffptr;
	char			*dest;
	char			c;
	int				i;
	int				count;

	chatfilter.chatFilterCount = 0;
	memset( buffer, 0, 1024 );

	trap_Cvar_VariableStringBuffer( "sv_filterchatfile", filename, sizeof( filename ) );
	len = trap_FS_FOpenFile( filename, &f, FS_READ_TEXT );
	if ( !f ) {
		Com_Printf (" File not found: %s\n", filename);
		return;
	}
	maxlen = len;

	dest = chatfilter.chatFilterBuffer;

	if ( len > 1024 ) {
		len = 1024;
	}
	maxlen -= len;
	trap_FS_Read( buffer, len, f);
	buffer[ sizeof(buffer) - 1 ] = 0;

	for ( i = 0; i < len; i++ )
	{
		if ( buffer[i] == '\t' )
		{
			buffer[i] = ' ';
		}
	}

	chatfilter.chatFilterCount = 0;
	buffptr = buffer;
	while ( *buffptr != 0 && dest < chatfilter.chatFilterBuffer + MAX_FILTER_BUFFER )
	{
		if ( buffptr > (buffer + 512) && maxlen > 0 )
		{
			memmove( buffer, buffer + 512, 512 );
			buffptr -= 512;

			len = maxlen;
			if ( len > 512 ) {
				len = 512;
			}
			maxlen -= len;
			memset( buffer + 512, 0, 512 );
			trap_FS_Read( buffer + 512, len, f);
		}

		count = 0;
		chatfilter.chatFilterText[ chatfilter.chatFilterCount ] = dest;
		do
		{
			c = *buffptr++;
		}
		while ( c == '\n' );
		if ( *buffptr == '/' )
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		i = 0;
		while ( c >= ' ' && dest < chatfilter.chatFilterBuffer + MAX_FILTER_BUFFER )
		{
			*dest++ = c;
			c = *buffptr++;
			count++;
		}
		*dest++ = 0;
		chatfilter.chatFilterLength[ chatfilter.chatFilterCount ] = count;
		if ( count ) {
			chatfilter.chatFilterCount++;
		}
		buffptr--;
		SkipRestOfLine( &buffptr );
	}
	trap_FS_FCloseFile( f );
}

/*
===============
G_InitVoiceCommands
===============
*/
void G_InitVoiceCommands ( void )
{
	char			buffer[1025];
	char			filename[256];
	fileHandle_t	f;
	int				len;
	int				maxlen;
	const char		*buffptr;
	char			*dest;
	char			c;
	int				i;
	int				num;
	qboolean		alpha;

	voicecmds.voiceCommandCount = 0;
	memset( buffer, 0, 1024 );

	trap_Cvar_VariableStringBuffer( "sv_voicecmdsfile", filename, sizeof( filename ) );
	len = trap_FS_FOpenFile( filename, &f, FS_READ_TEXT );
	if ( !f ) {

		Com_Printf (" File not found: %s\n", filename);
		return;
	}
	maxlen = len;

	dest = voicecmds.voiceCommandBuffer;

	if ( len > 1024 ) {
		len = 1024;
	}
	maxlen -= len;
	trap_FS_Read( buffer, len, f);
	buffer[ sizeof(buffer) - 1 ] = 0;

	for ( i = 0; i < len; i++ )
	{
		if ( buffer[i] == '\t' )
		{
			buffer[i] = ' ';
		}
	}

	buffptr = buffer;
	while ( *buffptr != 0 && dest < voicecmds.voiceCommandBuffer + MAX_VOICE_BUFFER )
	{
		if ( buffptr > (buffer + 512) && maxlen > 0 )
		{
			memmove( buffer, buffer + 512, 512 );
			buffptr -= 512;

			len = maxlen;
			if ( len > 512 ) {
				len = 512;
			}
			maxlen -= len;
			memset( buffer + 512, 0, 512 );
			trap_FS_Read( buffer + 512, len, f);

			for ( i = 0; i < len; i++ )
			{
				if ( buffer[i] == '\t' )
				{
					buffer[i] = ' ';
				}
			}
		}

		num = 0;
		alpha = qfalse;
		do
		{
			c = *buffptr++;
		}
		while ( c == '\n' );
		if ( tolower(c) >= 'a' && tolower(c) <= 'z' )
		{
			alpha = qtrue;
			num = tolower(c) - 'a';
			c = *buffptr++;
		}
		else if ( c < '0' || c > '9' )
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		while ( c >= '0' && c <= '9' )
		{
			c -= '0';
			num = num * 10 + c;
			c = *buffptr++;
		}
		if ( c != ' ' )
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		while ( c == ' ' )
		{
			c = *buffptr++;
			if ( c == '\n' )
			{
				SkipRestOfLine( &buffptr );
				continue;
			}
		}
		buffptr--;
		if ( Q_stricmpn( buffptr, "text ", 5) == 0 )
		{
			if ( alpha )
			{
				voicecmds.voicePromptText[num] = dest;
			}
			else
			{
				voicecmds.voiceCommandText[num] = dest;
			}
			buffptr += 5;
		}
		else if ( Q_stricmpn( buffptr, "notext ", 7) == 0 )
		{
			if ( alpha )
			{
				voicecmds.voicePromptText[num] = dest;
			}
			else
			{
				voicecmds.voiceCommandText[num] = dest;
			}
			*dest++ = 0;
			buffptr += 7;
		}
		else if ( Q_stricmpn( buffptr, "sound ", 6) == 0 )
		{
			if ( alpha )
			{
				voicecmds.voicePromptSound[num] = dest;
			}
			else
			{
				voicecmds.voiceCommandSound[num] = dest;
			}
			buffptr += 6;
		}
		else if ( !alpha && Q_stricmpn( buffptr, "adminonly", 9) == 0 )
		{
			voicecmds.voiceCommandFlag[num] = VOICE_ADMIN_ONLY;
			SkipRestOfLine( &buffptr );
			continue;
		}
		else if ( !alpha && Q_stricmpn( buffptr, "clanonly", 9) == 0 )
		{
			voicecmds.voiceCommandFlag[num] = VOICE_CLAN_ONLY;
			SkipRestOfLine( &buffptr );
			continue;
		}
		else if ( !alpha && Q_stricmpn( buffptr, "sysoponly", 9) == 0 )
		{
			voicecmds.voiceCommandFlag[num] = VOICE_SYSOP_ONLY;
			SkipRestOfLine( &buffptr );
			continue;
		}
		else if ( !alpha && Q_stricmpn( buffptr, "refereeonly", 9) == 0 )
		{
			voicecmds.voiceCommandFlag[num] = VOICE_REFEREE_ONLY;
			SkipRestOfLine( &buffptr );
			continue;
		}
		else
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		c = *buffptr++;
		while ( c == ' ' )
		{
			c = *buffptr++;
			if ( c == '\n' )
			{
				SkipRestOfLine( &buffptr );
				continue;
			}
		}
		if ( c != '\"' )
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		c = *buffptr++;
		i = 0;
		while ( c != '\"' && c >= ' ' && i < 126 && dest < voicecmds.voiceCommandBuffer + MAX_VOICE_BUFFER )
		{
			*dest++ = c;
			c = *buffptr++;
			i++;
		}
		*dest++ = 0;
		if ( !alpha && num > voicecmds.voiceCommandCount )
		{
			voicecmds.voiceCommandCount = num;
		}
		SkipRestOfLine( &buffptr );
	}
	trap_FS_FCloseFile( f );
}

/*
===============
G_CheckModFolder
Make sure the WP version of the mod isn't running out of the "rocmod" folder
===============
*/
void G_CheckModFolder ( void )
{
	char paks[1024];

	trap_Cvar_VariableStringBuffer( "sv_referencedPakNames", paks, sizeof( paks ) );
	if ( paks[0] == 0 )
	{
		return;
	}
	paks[1023] = 0;
	if ( strstr( paks, " rocmod/" ) )
	{
		// The fuckers!
		level.badFolder = 1;
	}
	else
	{
		level.badFolder = 2;
	}
}

/*
===============
G_InitScanList
===============
*/
void G_InitScanList ( void )
{
	char			buffer[1025];
	char			filename[256];
	fileHandle_t	f;
	int				len;
	int				maxlen;
	const char		*buffptr;
	char			*dest;
	char			c;
	int				i;
	int				num;

	scanlist.scanListCount = 0;
	memset( buffer, 0, 1024 );

	trap_Cvar_VariableStringBuffer( "sv_scannerfile", filename, sizeof( filename ) );
	len = trap_FS_FOpenFile( filename, &f, FS_READ_TEXT );
	if ( !f ) {

		Com_Printf (" File not found: %s\n", filename);
		return;
	}
	maxlen = len;

	dest = scanlist.scanListBuffer;

	if ( len > 1024 ) {
		len = 1024;
	}
	maxlen -= len;
	trap_FS_Read( buffer, len, f);
	buffer[ sizeof(buffer) - 1 ] = 0;

	for ( i = 0; i < len; i++ )
	{
		if ( buffer[i] == '\t' )
		{
			buffer[i] = ' ';
		}
	}

	num = 0;
	buffptr = buffer;
	while ( *buffptr != 0 && dest < scanlist.scanListBuffer + MAX_SCAN_BUFFER )
	{
		if ( buffptr > (buffer + 512) && maxlen > 0 )
		{
			memmove( buffer, buffer + 512, 512 );
			buffptr -= 512;

			len = maxlen;
			if ( len > 512 ) {
				len = 512;
			}
			maxlen -= len;
			memset( buffer + 512, 0, 512 );
			trap_FS_Read( buffer + 512, len, f);

			for ( i = 0; i < len; i++ )
			{
				if ( buffer[i] == '\t' )
				{
					buffer[i] = ' ';
				}
			}
		}

		do
		{
			c = *buffptr++;
		}
		while ( c == '\n' );
		switch ( tolower(c) )
		{
			case 'k':
				scanlist.scanListFlag[num] = SCANFILE_KICK;
				break;
			case 'm':
				scanlist.scanListFlag[num] = SCANFILE_MESSAGE;
				break;
			case 'a':
				scanlist.scanListFlag[num] = SCANFILE_ADMIN;
				break;
			case 'l':
				scanlist.scanListFlag[num] = SCANFILE_LOG;
				break;
			default:
				SkipRestOfLine( &buffptr );
				continue;
		}
		c = *buffptr++;
		if ( c != ' ' )
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		while ( c == ' ' )
		{
			c = *buffptr++;
			if ( c == '\n' )
			{
				SkipRestOfLine( &buffptr );
				continue;
			}
		}
		scanlist.scanListValue[num] = dest;
		i = 0;
		while ( i < 32 && c != ' ' && c != '\n' && dest < scanlist.scanListBuffer + MAX_SCAN_BUFFER )
		{
			*dest++ = c;
			c = *buffptr++;
			i++;
		}
		*dest++ = 0;
		if ( c != ' ' )
		{
			SkipRestOfLine( &buffptr );
			continue;
		}
		while ( c == ' ' )
		{
			c = *buffptr++;
			if ( c == '\n' )
			{
				SkipRestOfLine( &buffptr );
				continue;
			}
		}
		scanlist.scanListFile[num] = dest;
		while ( c != '\n' && c != ' ' && dest < scanlist.scanListBuffer + MAX_SCAN_BUFFER )
		{
			*dest++ = c;
			c = *buffptr++;
		}
		*dest++ = 0;
//		SkipRestOfLine( &buffptr );
		num++;
		if ( num >= MAX_SCAN_ITEMS )
		{
			break;
		}
	}
	scanlist.scanListCount = num;
	trap_FS_FCloseFile( f );
}

/*
============
G_InitGame
============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) 
{
	int	i;
	char	s[32];

	Com_Printf ("------- Game Initialization -------\n");
	Com_Printf ("gamename:    %s\n", GAMEVERSION);
	Com_Printf ("gameversion: %s\n", ROCMOD_STRING);
	Com_Printf ("gamedate:    %s\n", __DATE__);

	srand( randomSeed );

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	memset( &voicecmds, 0, sizeof( voicecmds ) );
	memset( &recondata, 0, sizeof( recondata ) );
	memset( &chatfilter, 0, sizeof( chatfilter ) );
	level.time = levelTime;
	level.startTime = levelTime;
	level.redFlagCarrier = -1;
	level.redFlagKiller = -1;
	level.redFlagMartyr = -1;
	level.blueFlagCarrier = -1;
	level.blueFlagKiller = -1;
	level.blueFlagMartyr = -1;

	trap_Cvar_VariableStringBuffer( "inMatch", s, sizeof( s ) );
	if ( s[0] == '1' || s[0] == '2' )
	{
		level.match = qtrue;
	}
	trap_Cvar_Set( "exitReady", "0" );

	G_RegisterCvars();

//	if ( inMatch.integer )
//	{
//		level.match = qtrue;
//	}

	// Load the list of arenas
	G_LoadArenas ( );

	// Build the gametype list so we can verify the given gametype
	BG_BuildGametypeList ( );

	// Set the current gametype
	G_SetGametype(g_gametype.string);

	// Sets the available weapons cvar from the disable_ cvars.
	G_UpdateAvailableWeapons ( );

	// Set the available outfitting
	BG_SetAvailableOutfitting ( g_availableWeapons.string );

	// Give the game a uniqe id
	trap_SetConfigstring ( CS_GAME_ID, va("%d", randomSeed ) );

	if ( g_log.string[0] ) 
	{
		if ( g_logSync.integer ) 
		{
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_SYNC_TEXT );
		} 
		else 
		{
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_TEXT );
		}
		
		if ( !level.logFile ) 
		{
			Com_Printf( "WARNING: Couldn't open logfile: %s\n", g_log.string );
		} 
		else 
		{
			char	serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", serverinfo );
		}
	}
	else
	{
		Com_Printf( "Not logging to disk.\n" );
	}
	if ( g_adminlog.string[0] )
	{
		if ( g_logSync.integer )
		{
			trap_FS_FOpenFile( g_adminlog.string, &level.logAdminFile, FS_APPEND_SYNC_TEXT );
		}
		else
		{
			trap_FS_FOpenFile( g_adminlog.string, &level.logAdminFile, FS_APPEND_TEXT );
		}
		
		if ( !level.logAdminFile )
		{
			Com_Printf( "WARNING: Couldn't open logfile: %s\n", g_adminlog.string );
		}
		else
		{
			G_AdminLogPrintf("------------------------------------------------------------\n" );
		}
	}
	if ( g_scannerlog.string[0] )
	{
		if ( g_logSync.integer )
		{
			trap_FS_FOpenFile( g_scannerlog.string, &level.logScannerFile, FS_APPEND_SYNC_TEXT );
		}
		else
		{
			trap_FS_FOpenFile( g_scannerlog.string, &level.logScannerFile, FS_APPEND_TEXT );
		}
		
		if ( !level.logScannerFile )
		{
			Com_Printf( "WARNING: Couldn't open logfile: %s\n", g_scannerlog.string );
		}
	}

	G_InitWorldSession();

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i=0 ; i<level.maxclients ; i++ ) 
	{
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, 
						 level.num_entities, 
						 sizeof( gentity_t ), 
						 &level.clients[0].ps, 
						 sizeof( level.clients[0] ) );

	// Get the boundaries of the world
	trap_GetWorldBounds ( level.worldMins, level.worldMaxs );

	// reserve some spots for dead player bodies
	G_InitBodyQueue();

	ClearRegisteredItems();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString(qfalse);

	// Now parse the gametype information that we need.  This needs to be
	// done after the entity spawn so that the items and triggers can be 
	// linked up properly
	G_ParseGametypeFile ( );

	BG_ParseInviewFile( level.pickupsDisabled );

	// Load in the identities
	BG_ParseNPCFiles ( );

	// general initialization
	G_FindTeams();

	SaveRegisteredItems();

	G_InitVoiceCommands();
	G_InitChatFilter();
	G_InitScanList();

	Com_Printf ("-----------------------------------\n");

	if( trap_Cvar_VariableIntegerValue( "com_buildScript" ) ) 
	{
		G_SoundIndex( "sound/player/gurp1.wav" );
		G_SoundIndex( "sound/player/gurp2.wav" );
	}

#ifdef _SOF2_BOTS
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) 
	{
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}
#endif

	G_RemapTeamShaders();

	// Initialize the gametype 
	trap_GT_Init ( g_gametype.string, restart );

	// Music
	if ( RMG.integer )
	{
		char temp[MAX_INFO_STRING];

		// start the music
		trap_Cvar_VariableStringBuffer("RMG_music", temp, MAX_QPATH);
		trap_SetConfigstring( CS_MUSIC, temp );
	}

	trap_SetConfigstring( CS_VOTE_TIME, "" );
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) 
{
	Com_Printf ("==== ShutdownGame ====\n");

	if ( level.logScannerFile ) 
	{
		G_ScanLogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logScannerFile );
	}
	if ( level.logAdminFile ) 
	{
		G_AdminLogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logAdminFile );
	}
	if ( level.logFile ) 
	{
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
	}
	
	// write all the client session data so we can get it back
	G_WriteSessionData();

#ifdef _SOF2_BOTS
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) 
	{
		BotAIShutdown( restart );
	}
#endif
}


#ifndef GAME_HARD_LINKED

void QDECL Com_Error ( int level, const char *fmt, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vnsprintf (text, sizeof(text), fmt, argptr);
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

	trap_Printf( text );
}

#endif

/*
=============
SortRanks
=============
*/
int QDECL SortRanks( const void *a, const void *b ) 
{
	gclient_t	*ca;
	gclient_t	*cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
	{
		return 1;
	}
	
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
	{
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) 
	{
		return 1;
	}
	
	if ( cb->pers.connected == CON_CONNECTING ) 
	{
		return -1;
	}

	// then spectators
	if ( ca->sess.team == TEAM_SPECTATOR && cb->sess.team == TEAM_SPECTATOR ) 
	{
		if ( ca->sess.spectatorTime < cb->sess.spectatorTime ) 
		{
			return -1;
		}
		if ( ca->sess.spectatorTime > cb->sess.spectatorTime ) 
		{
			return 1;
		}
		return 0;
	}
	
	if ( ca->sess.team == TEAM_SPECTATOR ) 
	{
		return 1;
	}
	
	if ( cb->sess.team == TEAM_SPECTATOR ) 
	{
		return -1;
	}

	if ( g_alternateSort.integer )
	{
		float sa;
		float sb;
		int t;

		if ( ca->sess.modData->recondata->kills )
		{
			t = ca->sess.modData->recondata->kills + ca->sess.modData->recondata->deaths;
			sa = ca->sess.modData->recondata->kills;
			sa /= t;
			sa *= 100;
		}
		else
		{
			sa = 0;
		}
		if ( cb->sess.modData->recondata->kills )
		{
			t = cb->sess.modData->recondata->kills + cb->sess.modData->recondata->deaths;
			sb = cb->sess.modData->recondata->kills;
			sb /= t;
			sb *= 100;
		}
		else
		{
			sb = 0;
		}

		if ( sa > sb ) 
		{
			return -1;
		}

		if ( sa < sb ) 
		{
			return 1;
		}
	}

	// then sort by score
	if ( ca->sess.modData->recondata->score > cb->sess.modData->recondata->score ) 
	{
		return -1;
	}
	
	if ( ca->sess.modData->recondata->score < cb->sess.modData->recondata->score ) 
	{
		return 1;
	}

	// then sort by kills
	if ( ca->sess.modData->recondata->kills > cb->sess.modData->recondata->kills ) 
	{
		return -1;
	}
	
	if ( ca->sess.modData->recondata->kills < cb->sess.modData->recondata->kills ) 
	{
		return 1;
	}

	// then sort by deaths
	if ( ca->sess.modData->recondata->deaths > cb->sess.modData->recondata->deaths ) 
	{
		return -1;
	}
	
	if ( ca->sess.modData->recondata->deaths < cb->sess.modData->recondata->deaths ) 
	{
		return 1;
	}
	
	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) 
{
	int			i;
	int			rank;
	int			score;
	int			newScore;
	gclient_t	*cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots

	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) 
		{
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.team != TEAM_SPECTATOR ) 
			{
				level.numNonSpectatorClients++;
			
				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) 
				{
					level.numPlayingClients++;
					if ( !(g_entities[i].r.svFlags & SVF_BOT) ) 
					{
						level.numVotingClients++;
					}
					if ( level.follow1 == -1 ) 
					{
						level.follow1 = i;
					} 
					else if ( level.follow2 == -1 ) 
					{
						level.follow2 = i;
					}
				}
			}
		}
	}

	qsort( level.sortedClients, level.numConnectedClients, 
		   sizeof(level.sortedClients[0]), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( level.gametypeData->teams ) 
	{
		int rank;
		if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) 
		{
			rank = 2;
		} 
		else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) 
		{
			rank = 0;
		} 
		else 
		{
			rank = 1;
		}

		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numConnectedClients; i++ ) 
		{
			cl = &level.clients[ level.sortedClients[i] ];
			cl->ps.persistant[PERS_RANK] = rank;
		}
	} 
	else 
	{	
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) 
		{
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->sess.modData->recondata->score;
			if ( i == 0 || newScore != score ) 
			{
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} 
			else 
			{
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
		}
	}


	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) 
	{
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
CalculateBestPlayerStats
========================
*/
void CalculateBestPlayerStats( void )
{
	int			i;
	int			v;
	gclient_t	*cl;
	int			headshots;
	int			kills;
	int			flagcaps;
	int			flagdefends;
	int			value;
	int			killrate;
	int			killspree;
	int			nadekills;
	int			meleekills;
	float		accuracy;
	float		bestaccuracy;
	float		ratio;
	float		bestratio;

	level.bestHeadshots = -1;
	level.bestKills = -1;
	level.bestAccuracy = -1;
	level.bestRatio = -1;
	level.bestFlagCaps = -1;
	level.bestFlagDefends = -1;
	level.bestValue = -1;
	level.bestKillRate = -1;
	level.bestKillSpree = -1;
	level.bestNadeKills = -1;
	level.bestMeleeKills = -1;
	headshots = 0;
	kills = 0;
	flagcaps = 0;
	flagdefends = 0;
	value = 0;
	killrate = 0;
	killspree = 0;
	nadekills = 0;
	meleekills = 0;
	bestaccuracy = 0.0f;
	bestratio = 0.0f;

	for (i = 0; i < g_maxclients.integer; i++) 
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		if ( cl->sess.modData->recondata->kills < g_bestStatsMinKills.integer )
		{
			continue;
		}

		if ( cl->sess.modData->headshots > headshots )
		{
			headshots = cl->sess.modData->headshots;
			level.bestHeadshots = i;
		}
		if ( cl->sess.modData->recondata->kills > kills )
		{
			kills = cl->sess.modData->recondata->kills;
			level.bestKills = i;
		}
		if ( cl->sess.modData->flagcaps > flagcaps )
		{
			flagcaps = cl->sess.modData->flagcaps;
			level.bestFlagCaps = i;
		}
		if ( cl->sess.modData->flagdefends > flagdefends )
		{
			flagdefends = cl->sess.modData->flagdefends;
			level.bestFlagDefends = i;
		}
		if ( cl->sess.modData->hits )
		{
			if ( cl->sess.modData->misses )
			{
				accuracy = cl->sess.modData->hits;
				accuracy /= ( cl->sess.modData->hits + cl->sess.modData->misses );
			}
			else
			{
				accuracy = 1.0f;
			}
			if ( accuracy > bestaccuracy )
			{
				bestaccuracy = accuracy;
				level.bestAccuracyValue = accuracy;
				level.bestAccuracy = i;
			}
		}
		if ( cl->sess.modData->recondata->kills )
		{
			ratio = cl->sess.modData->recondata->kills;
			if ( cl->sess.modData->recondata->deaths )
			{
				ratio /= cl->sess.modData->recondata->deaths;
			}
			if ( ratio > bestratio )
			{
				bestratio = ratio;
				level.bestRatioValue = ratio;
				level.bestRatio = i;
			}
		}
		v = (cl->sess.modData->recondata->score / 2) + cl->sess.modData->recondata->kills - cl->sess.modData->recondata->deaths + cl->sess.modData->headshots;
		if ( cl->sess.modData->misses )
		{
			v *= cl->sess.modData->hits;
			v /= ( cl->sess.modData->hits + cl->sess.modData->misses );
		}
		if ( v > value )
		{
			value = v;
			level.bestValue = i;
		}
		v = (level.time - cl->pers.enterTime) / 60000;
		if ( v && (cl->sess.modData->recondata->kills * (60 / v)) > killrate )
		{
			killrate = cl->sess.modData->recondata->kills * (60 / v);
			level.bestKillRate = i;
		}
		if ( cl->sess.modData->currkillspree > cl->sess.modData->bestkillspree )
		{
			cl->sess.modData->bestkillspree = cl->sess.modData->currkillspree;
		}
		if ( cl->sess.modData->bestkillspree > killspree )
		{
			killspree = cl->sess.modData->bestkillspree;
			level.bestKillSpree = i;
		}
		if ( cl->sess.modData->meleekills > meleekills )
		{
			meleekills = cl->sess.modData->meleekills;
			level.bestMeleeKills = i;
		}
		if ( cl->sess.modData->nadekills > nadekills )
		{
			nadekills = cl->sess.modData->nadekills;
			level.bestNadeKills = i;
		}
	}
}

/*
========================
SendBestPlayerStatsToAllClients
========================
*/
void SendBestPlayerStatsToAllClients( void )
{
	int			i;
	gentity_t	*ent;

	CalculateBestPlayerStats();

	for ( i = 0; i < level.numConnectedClients; i++ ) 
	{
		ent = &g_entities[level.sortedClients[i]];
		if ( sv_modClient.integer || ent->client->sess.modData->versionVerified )
		{
			trap_SendServerCommand( ent-g_entities, va("playerstats %i %i %i %i %i %i %i %i %i %i %i", level.bestKills, level.bestAccuracy, level.bestHeadshots, level.bestFlagCaps, level.bestRatio, level.bestValue, level.bestKillRate, level.bestKillSpree, level.bestNadeKills, level.bestMeleeKills, level.bestFlagDefends ) );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) 
{
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) 
	{
		G_StopFollowing( ent );
	}

	FindIntermissionPoint ( );

	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->r.currentOrigin );
	VectorCopy( level.intermission_origin, ent->client->ps.pvsOrigin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);

	// Reset some client variables
	ent->client->ps.pm_type		  = PM_INTERMISSION;
	ent->client->ps.stats[STAT_GAMETYPE_ITEMS] = 0;
	ent->client->ps.eFlags		  = 0;
	ent->s.eFlags				  = 0;
	ent->s.eType				  = ET_GENERAL;
	ent->s.modelindex			  = 0;
	ent->s.loopSound			  = 0;
	ent->s.event				  = 0;
	ent->r.contents				  = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) 
{
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if ( !ent ) 		
	{	
		gspawn_t* spawn = G_SelectRandomSpawnPoint ( -1 );
		if ( spawn )
		{
			VectorCopy (spawn->origin, level.intermission_origin);
			VectorCopy (spawn->angles, level.intermission_angle);
		}

		return;
	} 

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// if it has a target, look towards it
	if ( ent->target ) 
	{
		target = G_PickTarget( ent->target );
		if ( target ) 
		{
			VectorSubtract( target->s.origin, level.intermission_origin, dir );
			vectoangles( dir, level.intermission_angle );
		}
	}
}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) 
{
	int			i;
	gentity_t	*ent;

	if ( level.intermissiontime ) 
	{
		// already active
		return;		
	}

	// End any recurring gametype events
	G_EndGametype();

	// Kill any votes
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
	
	level.intermissiontime  = level.time;
	level.gametypeResetTime = level.time;
	FindIntermissionPoint();

	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) 
	{
		ent = g_entities + i;
		if (!ent->inuse)
		{
			continue;
		}

		// take out of follow mode
		G_StopFollowing( ent );

		// Get rid of ghost state
		G_StopGhosting ( ent );

		// respawn if dead
		if ( G_IsClientDead ( ent->client ) ) 
		{
			respawn ( ent );
		}

		MoveClientToIntermission( ent );
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

	// send the "best player" stats to all clients
	SendBestPlayerStatsToAllClients();
}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar 

=============
*/
void ExitLevel (void) 
{
	int			i;
	int			rnd;
	int			red;
	int			blue;
	gclient_t	*cl;

	G_LogPrintf("Match ends\n");

	if ( !level.match )
	{
		// See if we need to swap the teams
		if ( g_mapcycleSwapTeams.integer )
		{
			for (i = 0; i < g_maxclients.integer; i++) 
			{
				cl = level.clients + i;
				if ( cl->pers.connected != CON_CONNECTED ) 
				{
					continue;
				}
				if ( cl->sess.team == TEAM_FREE || cl->sess.team == TEAM_SPECTATOR ) 
				{
					continue;
				}

				if ( cl->sess.team == TEAM_RED ) {
					cl->sess.team = TEAM_BLUE;
				}
				else {
					cl->sess.team = TEAM_RED;
				}
			}
		}

		// See if we need to shuffle the teams
		if ( g_mapcycleShuffleTeams.integer )
		{
			red = 0;
			blue = 0;
			for (i = 0; i < g_maxclients.integer; i++) 
			{
				cl = level.clients + i;
				if ( cl->pers.connected != CON_CONNECTED ) 
				{
					continue;
				}
				if ( cl->sess.team == TEAM_FREE || cl->sess.team == TEAM_SPECTATOR ) 
				{
					continue;
				}

				if ( (blue - red) > 2 ) {
					cl->sess.team = TEAM_RED;
					red++;
					continue;
				}
				if ( (red - blue) > 2 ) {
					cl->sess.team = TEAM_BLUE;
					blue++;
					continue;
				}
				rnd = rand() % 10;
				if ( rnd < 5 ) {
					cl->sess.team = TEAM_BLUE;
					blue++;
					continue;
				}
				else {
					cl->sess.team = TEAM_RED;
					red++;
					continue;
				}
			}
		}
	}

	// Next map
	if ( level.match || level.freezeMap )
	{
		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
	}
	else
	{
		trap_SendConsoleCommand( EXEC_APPEND, "mapcycle\n" );
	}
	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) 
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		cl->sess.modData->recondata->score = 0;
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) 
	{
		if ( level.clients[i].pers.connected == CON_CONNECTED ) 
		{
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%4i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	vnsprintf( string +8, sizeof(string)-8, fmt,argptr );
	va_end( argptr );

#ifndef _DEBUG
	if ( g_dedicated.integer ) {
#endif
		Com_Printf( "%s", string + 8 );
#ifndef _DEBUG
	}
#endif

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
=================
G_AdminLogPrintf

Print to the admin logfile with a time stamp if it is open
=================
*/
void QDECL G_AdminLogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;
	qtime_t		currtime;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%4i:%i%i ", min, tens, sec );

	trap_RealTime (&currtime);

	Com_sprintf( string+8, sizeof(string), "[%02i/%02i/%02i %02i:%02i:%02i] ", currtime.tm_mon, currtime.tm_mday, currtime.tm_year % 100, currtime.tm_hour, currtime.tm_min, currtime.tm_sec );
	va_start( argptr, fmt );
	vnsprintf( string+28, sizeof(string)-28, fmt,argptr );
	va_end( argptr );

#ifndef _DEBUG
	if ( g_dedicated.integer ) {
#endif
		Com_Printf( "%s", string + 28 );
#ifndef _DEBUG
	}
#endif

	if ( !level.logAdminFile ) {
		return;
	}

//	Q_CleanStr( string );
	trap_FS_Write( string, strlen( string ), level.logAdminFile );
	G_LogPrintf( &string[28] );
}

/*
=================
G_ScanLogPrintf

Print to the scanner logfile with a time stamp if it is open
=================
*/
void QDECL G_ScanLogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	qtime_t		currtime;

	trap_RealTime (&currtime);

	Com_sprintf( string, sizeof(string), "[%04i-%02i-%02i %02i:%02i:%02i] ", currtime.tm_year, currtime.tm_mon, currtime.tm_mday, currtime.tm_hour, currtime.tm_min, currtime.tm_sec );
	va_start( argptr, fmt );
	vnsprintf( string+22, sizeof(string)-22, fmt,argptr );
	va_end( argptr );

#ifndef _DEBUG
	if ( g_dedicated.integer ) {
#endif
		Com_Printf( "%s", string + 22 );
#ifndef _DEBUG
	}
#endif

	if ( !level.logScannerFile ) {
		return;
	}

//	Q_CleanStr( string );
	trap_FS_Write( string, strlen( string ), level.logScannerFile );
	G_LogPrintf( &string[22] );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) 
{
	int				i;
	int				numSorted;
	gclient_t		*cl;

	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) 
	{
		numSorted = 32;
	}

	if ( level.gametypeData->teams ) 
	{
		G_LogPrintf( "red:%i  blue:%i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	for (i=0 ; i < numSorted ; i++) 
	{
		int	ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.team == TEAM_SPECTATOR ) 
		{
			continue;
		}

		if ( cl->pers.connected == CON_CONNECTING ) 
		{
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->sess.modData->recondata->score, ping, level.sortedClients[i],	cl->pers.netname );
	}
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) 
{
	int			ready, notReady;
	int			i;
	gclient_t	*cl;
	int			readyMask;
	char		readylist[65];

	if ( level.paused && !level.matchPending )
	{
		return;
	}

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	memset( readylist, '0', 64 );
	readylist[64] = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) {
			continue;
		}

		if ( cl->readyToExit ) {
			ready++;
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
			readylist[ i ] = '1';
		} else {
			notReady++;
		}
	}
	if ( Q_stricmp ( exitReady.string, readylist ) != 0 )
	{
		trap_Cvar_Set( "exitReady", readylist );
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	if ( level.matchPending == 1 && ready >= level.numNonSpectatorClients )
	{
		level.matchPending = 2;
		level.unpauseTime = level.time;
//		AdminCmd_DoStartMatch();
		return;
	}

//	if ( notReady > ready && (level.match || level.matchPending) )
	if ( level.match || level.matchPending )
	{
		return;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) {
		return;
	}

	// if nobody wants to go, clear timer
	if ( !ready ) {
		level.readyToExit = qfalse;
		return;
	}

	// if everyone wants to go, go now
	if ( !notReady ) {
		ExitLevel();
		return;
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + (g_intermissionDelay.integer * 1000) ) {
		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) 
{
	int	a;
	int b;

	if ( level.numPlayingClients < 2 ) 
	{
		return qfalse;
	}
	
	if ( level.gametypeData->teams ) 
	{
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].sess.modData->recondata->score;
	b = level.clients[level.sortedClients[1]].sess.modData->recondata->score;

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) 
{
 	int			i;
	gclient_t	*cl;
	int			accm;
	int			accn;
	int			ratiom;
	int			ration;

	int			headshotsvalue;
	int			killsvalue;
	int			flagcapsvalue;
	int			flagdefendsvalue;
	int			overallvalue;
	int			killratevalue;
	int			killspreevalue;
	int			nadekillsvalue;
	int			meleekillsvalue;
	float		accuracyvalue;
	float		ratiovalue;
	char		*headshotsname;
	char		*killsname;
	char		*flagcapsname;
	char		*flagdefendsname;
	char		*overallname;
	char		*killratename;
	char		*killspreename;
	char		*nadekillsname;
	char		*meleekillsname;
	char		*accuracyname;
	char		*rationame;
	int			headshotsteam;
	int			killsteam;
	int			flagcapsteam;
	int			flagdefendsteam;
	int			overallteam;
	int			killrateteam;
	int			killspreeteam;
	int			nadekillsteam;
	int			meleekillsteam;
	int			accuracyteam;
	int			ratioteam;

	char		*stat1;
	char		*stat2;

	if ( level.intermissionQueued && !level.numVotingClients && g_emptyMapcycle.integer )
	{
		ExitLevel();
		return;
	}

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) 
	{
		CheckIntermissionExit ();
		return;
	}

	if ( level.warmupTime && (level.numVotingClients || !g_emptyMapcycle.integer) )
	{
		return;
	}

	if ( level.intermissionQueued ) 
	{
		int time = INTERMISSION_DELAY_TIME;

		if ( !sv_modClient.integer )
		{
			time += g_bestStatsTime.integer * 1000;
		}

		if ( level.time - level.intermissionQueued >= time ) 
		{
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		else if ( !sv_modClient.integer && level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME )
		{
			if ( level.time > level.lastStatTime + 1000 )
			{
				headshotsvalue = 0;
				headshotsname = "none";
				headshotsteam = 0;
				killsvalue = 0;
				killsname = "none";
				killsteam = 0;
				flagcapsvalue = 0;
				flagcapsname = "none";
				flagcapsteam = 0;
				flagdefendsvalue = 0;
				flagdefendsname = "none";
				flagdefendsteam = 0;
				overallvalue = 0;
				overallname = "none";
				overallteam = 0;
				killratevalue = 0;
				killratename = "none";
				killrateteam = 0;
				killspreevalue = 0;
				killspreename = "none";
				killspreeteam = 0;
				nadekillsvalue = 0;
				nadekillsname = "none";
				nadekillsteam = 0;
				meleekillsvalue = 0;
				meleekillsname = "none";
				meleekillsteam = 0;
				accuracyvalue = 0.0f;
				accuracyname = "none";
				accuracyteam = 0;
				ratiovalue = 0.0f;
				rationame = "none";
				ratioteam = 0;

				level.lastStatTime = level.time;
				CalculateBestPlayerStats();

				if ( level.bestHeadshots != -1 )
				{
					headshotsvalue = g_entities[level.bestHeadshots].client->sess.modData->headshots;
					headshotsname = g_entities[level.bestHeadshots].client->pers.netname;
					headshotsteam = g_entities[level.bestHeadshots].client->sess.team;
				}
				if ( level.bestKills != -1 )
				{
					killsvalue = g_entities[level.bestKills].client->sess.modData->recondata->kills;
					killsname = g_entities[level.bestKills].client->pers.netname;
					killsteam = g_entities[level.bestKills].client->sess.team;
				}
				if ( level.bestFlagCaps != -1 )
				{
					flagcapsvalue = g_entities[level.bestFlagCaps].client->sess.modData->flagcaps;
					flagcapsname = g_entities[level.bestFlagCaps].client->pers.netname;
					flagcapsteam = g_entities[level.bestFlagCaps].client->sess.team;
				}
				if ( level.bestFlagDefends != -1 )
				{
					flagdefendsvalue = g_entities[level.bestFlagDefends].client->sess.modData->flagdefends;
					flagdefendsname = g_entities[level.bestFlagDefends].client->pers.netname;
					flagdefendsteam = g_entities[level.bestFlagDefends].client->sess.team;
				}
				if ( level.bestValue != -1 )
				{
					overallvalue = (g_entities[level.bestValue].client->sess.modData->recondata->score / 2) + g_entities[level.bestValue].client->sess.modData->recondata->kills - g_entities[level.bestValue].client->sess.modData->recondata->deaths + g_entities[level.bestValue].client->sess.modData->headshots;
					if ( g_entities[level.bestValue].client->sess.modData->misses )
					{
						overallvalue *= g_entities[level.bestValue].client->sess.modData->hits;
						overallvalue /= ( g_entities[level.bestValue].client->sess.modData->hits + g_entities[level.bestValue].client->sess.modData->misses );
					}
					overallname = g_entities[level.bestValue].client->pers.netname;
					overallteam = g_entities[level.bestValue].client->sess.team;
				}
				if ( level.bestKillRate != -1 )
				{
					killratevalue = g_entities[level.bestKillRate].client->sess.modData->recondata->kills * (60 / ((level.time - g_entities[level.bestKillRate].client->pers.enterTime) / 60000));
					killratename = g_entities[level.bestKillRate].client->pers.netname;
					killrateteam = g_entities[level.bestKillRate].client->sess.team;
				}
				if ( level.bestKillSpree != -1 )
				{
					killspreevalue = g_entities[level.bestKillSpree].client->sess.modData->bestkillspree;
					killspreename = g_entities[level.bestKillSpree].client->pers.netname;
					killspreeteam = g_entities[level.bestKillSpree].client->sess.team;
				}
				if ( level.bestNadeKills != -1 )
				{
					nadekillsvalue = g_entities[level.bestNadeKills].client->sess.modData->nadekills;
					nadekillsname = g_entities[level.bestNadeKills].client->pers.netname;
					nadekillsteam = g_entities[level.bestNadeKills].client->sess.team;
				}
				if ( level.bestMeleeKills != -1 )
				{
					meleekillsvalue = g_entities[level.bestMeleeKills].client->sess.modData->meleekills;
					meleekillsname = g_entities[level.bestMeleeKills].client->pers.netname;
					meleekillsteam = g_entities[level.bestMeleeKills].client->sess.team;
				}
				if ( level.bestAccuracy != -1 )
				{
					accuracyvalue = level.bestAccuracyValue;
					accuracyname = g_entities[level.bestAccuracy].client->pers.netname;
					accuracyteam = g_entities[level.bestAccuracy].client->sess.team;
				}
				if ( level.bestRatio != -1 )
				{
					ratiovalue = level.bestRatioValue;
					rationame = g_entities[level.bestRatio].client->pers.netname;
					ratioteam = g_entities[level.bestRatio].client->sess.team;
				}

				accuracyvalue *= 100;
				accm = (int) accuracyvalue;
				accuracyvalue -= accm;
				accuracyvalue *= 10;
				accn = (int) accuracyvalue;
				ratiom = (int) ratiovalue;
				ratiovalue -= ratiom;
				ratiovalue *= 10;
				ration = (int) ratiovalue;

				stat1 = va("cp \"@^7- Best Player Stats -\n^3Overall Skill - ^7%i ^3- %s%s\n^3Kills - ^7%i ^3- %s%s\n^3Headshots - ^7%i ^3- %s%s\n^3Ratio - ^7%i.%i ^3- %s%s\n^3Accuracy - ^7%i.%ip ^3- %s%s\n", overallvalue, level.teamData.teamcolor[overallteam], overallname, killsvalue, level.teamData.teamcolor[killsteam], killsname, headshotsvalue, level.teamData.teamcolor[headshotsteam], headshotsname, ratiom, ration, level.teamData.teamcolor[ratioteam], rationame, accm, accn, level.teamData.teamcolor[accuracyteam], accuracyname);
				if ( BG_RelatedGametype( level.gametype, "ctf" ) )
				{
					stat2 = va("%s^3Flag Caps - ^7%i ^3- %s%s\n^3Flag Defends - ^7%i ^3- %s%s\n", stat1, flagcapsvalue, level.teamData.teamcolor[flagcapsteam], flagcapsname, flagdefendsvalue, level.teamData.teamcolor[flagdefendsteam], flagdefendsname);
				}
				else if ( BG_RelatedGametype( level.gametype, "inf" ) )
				{
					stat2 = va("%s^3Case Caps - ^7%i ^3- %s%s\n^3Case Defends - ^7%i ^3- %s%s\n", stat1, flagcapsvalue, level.teamData.teamcolor[flagcapsteam], flagcapsname, flagdefendsvalue, level.teamData.teamcolor[flagdefendsteam], flagdefendsname);
				}
				else
				{
					stat2 = va("%s", stat1);
				}
				stat1 = va("%s^3Kill Rate - ^7%i ^3- %s%s\n^3Killing Spree - ^7%i ^3- %s%s\n^3Nade Kills - ^7%i ^3- %s%s\n^3Melee Kills - ^7%i ^3- %s%s\n\"", stat2, killratevalue, level.teamData.teamcolor[killrateteam], killratename, killspreevalue, level.teamData.teamcolor[killspreeteam], killspreename, nadekillsvalue, level.teamData.teamcolor[nadekillsteam], nadekillsname, meleekillsvalue, level.teamData.teamcolor[meleekillsteam], meleekillsname);

				for ( i = 0 ; i < level.numConnectedClients ; i++ ) 
				{
					gentity_t	*ent;
					ent = &g_entities[level.sortedClients[i]];

					if ( ent->client->pers.connected != CON_CONNECTED )
					{
						continue;
					}

					G_StartGhosting( ent );
					ent->client->ps.stats[STAT_PLANTED] = 1;	// freeze forever until intermission

					trap_SendServerCommand( ent-g_entities, stat1 );
				}
			}
		}

		return;
	}

	// check for sudden death
	if ( !level.endMap && g_suddenDeath.integer && ScoreIsTied() && (level.numVotingClients || !g_emptyMapcycle.integer) ) 
	{
		if ( !level.shownSuddenDeath && g_timelimit.integer && !level.warmupTime )
		{
			if ( level.gametypeData->respawnType != RT_NONE || level.gametypeResetTime )
			{
				if ( level.time - level.startTime >= (g_timelimit.integer + level.timeExtension)*60000 ) 
				{
					level.shownSuddenDeath = qtrue;
					if ( voicecmds.voicePromptText[14][0] )
					{
						trap_SendServerCommand( -1, va("cp \"\n%s\n\"", voicecmds.voicePromptText[14]));
					}
					if ( voicecmds.voicePromptSound[14][0] )
					{
						G_BroadcastSound( voicecmds.voicePromptSound[14] );
					}
				}
			}
		}

		// always wait for sudden death
		return;
	}

	// Check to see if the timelimit was hit
	if ( g_timelimit.integer && (!level.warmupTime || g_emptyMapcycle.integer) ) 
	{
		if ( level.gametypeData->respawnType != RT_NONE || level.gametypeResetTime )
		{
			if ( level.time - level.startTime >= (g_timelimit.integer + level.timeExtension)*60000 ) 
			{
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				tent->s.eventParm = GAME_OVER_TIMELIMIT;
				tent->r.svFlags = SVF_BROADCAST;	

				LogExit( "Timelimit hit." );
				return;
			}
		}
	}

	// Check to see if the score was hit
	if ( g_scorelimit.integer ) 
	{
		if ( level.gametypeData->teams )
		{
			if ( level.teamScores[TEAM_RED] >= g_scorelimit.integer ) 
			{
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				tent->s.eventParm = GAME_OVER_SCORELIMIT;
				tent->r.svFlags = SVF_BROADCAST;	
				tent->s.otherEntityNum = TEAM_RED;

				LogExit( va("%s team hit the score limit.", level.teamData.redName) );
				return;
			}

			if ( level.teamScores[TEAM_BLUE] >= g_scorelimit.integer ) 
			{
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				tent->s.eventParm = GAME_OVER_SCORELIMIT;
				tent->r.svFlags = SVF_BROADCAST;	
				tent->s.otherEntityNum = TEAM_BLUE;

				LogExit( va("%s team hit the score limit.", level.teamData.blueName) );
				return;
			}
		}
		else
		{
			// Check to see if any of the clients scores have crossed the scorelimit
			for ( i = 0 ; i < level.numConnectedClients ; i++ ) 
			{
				cl = g_entities[level.sortedClients[i]].client;

				if ( cl->pers.connected != CON_CONNECTED )
				{
					continue;
				}

				if ( cl->sess.team != TEAM_FREE ) 
				{
					continue;
				}

				if ( cl->sess.modData->recondata->score >= g_scorelimit.integer ) 
				{
					gentity_t* tent;
					tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
					tent->s.eventParm = GAME_OVER_SCORELIMIT;
					tent->r.svFlags = SVF_BROADCAST;	
					tent->s.otherEntityNum = level.sortedClients[i];

					LogExit( "Scorelimit hit." );
					return;
				}
			}
		}
	}

	// Check to see if the map was ended by admin
	if ( level.endMap ) 
	{
		LogExit( "Map ended by admin." );
		return;
	}
}

/*
=============
CheckWarmupStats
=============
*/
void CheckWarmupStats ( void ) 
{
	gclient_t	*red;
	gclient_t	*blue;
	gclient_t	*cl;
	int			redval;
	int			blueval;
	int			i;

	blueval = 0;
	redval = 0;

	if ( !level.gametypeData->teams || !g_allowWarmupFriendlyFire.integer )
	{
		return;
	}

	for (i=0 ; i < level.numConnectedClients ; i++) 
	{
		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		if ( cl->sess.team == TEAM_RED && cl->sess.modData->warmupTKs > redval )
		{
			redval = cl->sess.modData->warmupTKs;
			red = cl;
		}
		else if ( cl->sess.team == TEAM_BLUE && cl->sess.modData->warmupTKs > blueval )
		{
			blueval = cl->sess.modData->warmupTKs;
			blue = cl;
		}
	}

	for (i=0 ; i < level.numConnectedClients ; i++) 
	{
		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		if ( cl->sess.team == TEAM_RED && redval )
		{
			trap_SendServerCommand( level.sortedClients[i], va("cp \"\n%sMost Warmup TKs\n%s ^,- ^7%i\n\"", level.teamData.teamcolor[1], red->pers.netname, redval ) );
		}
		else if ( cl->sess.team == TEAM_BLUE && blueval )
		{
			trap_SendServerCommand( level.sortedClients[i], va("cp \"\n%sMost Warmup TKs\n%s ^,- ^7%i\n\"", level.teamData.teamcolor[2], blue->pers.netname, blueval ) );
		}
	}
}

/*
=============
CheckWarmup
=============
*/
void CheckWarmup ( void ) 
{
	int			counts[TEAM_NUM_TEAMS];
	qboolean	notEnough = qfalse;

	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( level.numPlayingClients == 0 ) 
	{
		return;
	}

	if ( !level.warmupTime  )
	{
		return;
	}

	if ( level.gametypeData->teams ) 
	{
		counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE, NULL );
		counts[TEAM_RED] = TeamCount( -1, TEAM_RED, NULL );

		if (counts[TEAM_RED] + counts[TEAM_BLUE] < 2) 
		{
			notEnough = qtrue;
		}
	} 
	else if ( level.numPlayingClients < 2 ) 
	{
		notEnough = qtrue;
	}

	if ( notEnough ) 
	{
		if ( level.warmupTime != -1 ) 
		{
			level.warmupTime = -1;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			G_LogPrintf( "Warmup:\n" );
		}
		
		return; // still waiting for team members
	}

	if ( level.warmupTime == 0 ) 
	{
		return;
	}

	// if the warmup is changed at the console, restart it
	if ( g_warmup.modificationCount != level.warmupModificationCount ) 
	{
		level.warmupModificationCount = g_warmup.modificationCount;
		level.warmupTime = -1;
	}

	// if all players have arrived, start the countdown
	if ( level.warmupTime < 0 ) 
	{
		// fudge by -1 to account for extra delays
		if ( level.match )
		{
			level.warmupTime = level.time + 29000;
		}
		else
		{
			level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
		}
		trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		return;
	}

	// if the warmup time has counted down, restart
	if ( level.time > level.warmupTime ) 
	{
//		level.warmupTime += 10000;
//		trap_Cvar_Set( "g_restarted", "1" );

		if ( !level.match )
		{
			CheckWarmupStats();
		}
		G_ResetGametype ( qtrue );
//		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
//		level.restarted = qtrue;

		if (!level.matchStarted)
		{
			level.matchStarted = qtrue;
			G_LogPrintf("Match starts\n");

//			if ( voicecmds.voicePromptText[14][0] )
//			{
//				trap_SendServerCommand( -1, va("cp \"\n%s\n\"", voicecmds.voicePromptText[14]));
//			}
//			if ( voicecmds.voicePromptSound[14][0] )
//			{
//				G_BroadcastSound( voicecmds.voicePromptSound[14] );
//			}
		}

		return;
	}
}

/*
==================
CheckVote
==================
*/
void CheckVote( void ) 
{
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) 
	{
		level.voteExecuteTime = 0;

		if ( strstr( level.voteString, "referee" ) )
		{
			int id;

			id = atoi(&level.voteString[8]);
			g_entities[id].client->sess.modData->adminref = 1;
			trap_SendServerCommand( -1, "print \"^6Referee added.\n\"");
			ClientUserinfoChanged( id );
		}
		else if ( strstr( level.voteString, "endmap" ) )
		{
			level.endMap = qtrue;
		}
		else if ( strstr( level.voteString, "shuffleteams" ) )
		{
			AdminCmd_DoShuffleTeams( NULL );
		}
		else if ( strstr( level.voteString, "swapteams" ) )
		{
			AdminCmd_DoSwapTeams( NULL );
		}
		else if ( strstr( level.voteString, "poll" ) )
		{
			trap_SendServerCommand( -1, va("print \"Vote results: ^3Yes: ^7%i  ^3No: ^7%i\n\"", level.voteYes, level.voteNo) );
		}
		else if ( strstr( level.voteString, "mute" ) )
		{
			int id;

			id = atoi(&level.voteString[5]);
			if ( g_entities[id].client->sess.muted )
			{
				g_entities[id].client->sess.muted = qfalse;
				trap_SendServerCommand( -1, va("print \"%s%s^7 has been ^3UNMUTED^7 by vote\n\"", level.teamData.teamcolor[g_entities[id].client->sess.team], g_entities[id].client->pers.netname));
			}
			else
			{
				g_entities[id].client->sess.muted = qtrue;
				trap_SendServerCommand( -1, va("print \"%s%s^7 has been ^1MUTED^7 by vote\n\"", level.teamData.teamcolor[g_entities[id].client->sess.team], g_entities[id].client->pers.netname));
			}
		}
		else if ( strstr( level.voteString, "clanvsall" ) )
		{
			AdminCmd_DoClanVsAll( NULL, &level.voteString[10] );
		}
		else if ( strstr( level.voteString, "gmap" ) )
		{
			char	*mapname;
			char	*gametype;

			mapname = strchr( level.voteString, ' ' );
			mapname++;
			gametype = strchr( mapname, ' ' );
			*gametype = 0;
			gametype++;
			trap_SendConsoleCommand( EXEC_APPEND, va(" g_gametype %s\n", gametype ) );
			trap_SendConsoleCommand( EXEC_APPEND, va(" map %s\n", mapname ) );
		}
		else
		{
			trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
		}

		if ( strstr( level.voteString, "g_gametype" ) )
		{
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		}
		else if ( strstr( level.voteString, "timeextension" ) )
		{
			level.shown5min = qfalse;
			level.shown3min = qfalse;
			level.shown1min = qfalse;
			level.shownSuddenDeath = qfalse;
		}
	}

	if ( !level.voteTime )
	{
		return;
	}

	// Update the needed clients
	trap_SetConfigstring ( CS_VOTE_NEEDED, va("%i", level.numVotesNeeded ) );

	if ( level.time - level.voteTime >= g_voteDuration.integer*1000 ) 
	{
		trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		recondata[level.voteClient].voteDelayTime = level.time + g_failedVoteDelay.integer * 60000;
		G_LogPrintf("Vote failed.\n");

		if ( voicecmds.voicePromptSound[18][0] )
		{
			G_BroadcastSound( voicecmds.voicePromptSound[18] );
		}
	} 
	else 
	{
		if ( level.voteYes >= level.numVotesNeeded ) 
		{
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			level.voteExecuteTime = level.time + 3000;
			G_LogPrintf("Vote passed.\n");

			if ( voicecmds.voicePromptSound[17][0] )
			{
				G_BroadcastSound( voicecmds.voicePromptSound[17] );
			}
		} 
		else if ( level.voteNo >= level.numVotesNeeded ) 
		{
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
			recondata[level.voteClient].voteDelayTime = level.time + g_failedVoteDelay.integer * 60000;
			G_LogPrintf("Vote failed.\n");

			if ( voicecmds.voicePromptSound[18][0] )
			{
				G_BroadcastSound( voicecmds.voicePromptSound[18] );
			}
		} 
		else 
		{
			// still waiting for a majority
			return;
		}
	}

	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message) 
{
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		if (level.clients[i].sess.team != team)
		{
			continue;
		}

		trap_SendServerCommand( i, message );
	}
}

/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) 
{
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) 
	{
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) 
		{
			trap_Cvar_Set( "g_needpass", "1" );
		} else 
		{
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
}

/*
==================
CheckPause
==================
*/
qboolean CheckPause( void ) 
{
	if ( !level.paused )
	{
		return qfalse;
	}

	if ( level.time > level.lastPauseTime + 500 )
	{
		level.lastPauseTime += 500;
		trap_GT_SendEvent ( GTEV_TIME_ADJUST, level.time, 500, 0, 0, 0, 0 );
	}

	if ( level.unpauseTime > 0 )
	{
		if ( level.unpauseCount > 3 )
		{
			level.unpauseTime = 0;
			level.unpauseCount = 0;
			if ( level.matchPending )
			{
				AdminCmd_DoStartMatch();
			}
			else
			{
				AdminCmd_DoUnPause();
			}
		}
		else if ( level.time >= level.unpauseTime )
		{
			if ( level.matchPending )
			{
				trap_SetConfigstring( CS_GAMETYPE_MESSAGE, va("%i,Match will begin in %i", level.time + 1000, 3 - level.unpauseCount ) );
			}
			else
			{
				trap_SetConfigstring( CS_GAMETYPE_MESSAGE, va("%i,Game will resume in %i", level.time + 1000, 3 - level.unpauseCount ) );
			}
			level.unpauseTime += 1000;
			level.unpauseCount += 1;
			G_BroadcastSound("sound/misc/events/buzz02.wav");
		}
	}
	return qtrue;
}

void G_UpdateCustomTeams( void )
{
	if ( g_allowCustomTeams.integer )
	{
		level.teamData.teamcolor[1][0] = '^';
		level.teamData.teamcolor[1][1] = g_customRedCode.string[0];
		strncpy( level.teamData.redName, g_customRedName.string, sizeof(level.teamData.redName) - 1 );
		level.teamData.redName[ sizeof(level.teamData.redName) - 1 ] = 0;
		level.teamData.teamcolor[2][0] = '^';
		level.teamData.teamcolor[2][1] = g_customBlueCode.string[0];
		strncpy( level.teamData.blueName, g_customBlueName.string, sizeof(level.teamData.blueName) - 1 );
		level.teamData.blueName[ sizeof(level.teamData.blueName) - 1 ] = 0;
	}
	else
	{
		level.teamData.teamcolor[1][0] = '^';
		level.teamData.teamcolor[1][1] = '1';
		strncpy( level.teamData.redName, "Red", sizeof(level.teamData.redName) - 1 );
		level.teamData.redName[ sizeof(level.teamData.redName) - 1 ] = 0;
		level.teamData.teamcolor[2][0] = '^';
		level.teamData.teamcolor[2][1] = '4';
		strncpy( level.teamData.blueName, "Blue", sizeof(level.teamData.blueName) - 1 );
		level.teamData.blueName[ sizeof(level.teamData.blueName) - 1 ] = 0;
	}
}

/*
==================
ServerPeriodicActions

Server tasks that happen once every few seconds
==================
*/
void ServerPeriodicActions( void ) 
{
	G_UpdateCustomTeams();

	if ( g_timeRemaining.integer && g_timelimit.integer && !level.warmupTime ) 
	{
		if ( level.gametypeData->respawnType != RT_NONE || level.gametypeResetTime )
		{
			if ( !level.shown1min && level.time - level.startTime >= (g_timelimit.integer + level.timeExtension - 1)*60000 ) 
			{
				level.shown1min = qtrue;
				if ( voicecmds.voicePromptText[13][0] )
				{
					trap_SendServerCommand( -1, va("cp \"\n%s\n\"", voicecmds.voicePromptText[13]));
				}
				if ( voicecmds.voicePromptSound[13][0] )
				{
					G_BroadcastSound( voicecmds.voicePromptSound[13] );
				}
			}
			else if ( !level.shown3min && level.time - level.startTime >= (g_timelimit.integer + level.timeExtension - 3)*60000 ) 
			{
				level.shown3min = qtrue;
				if ( voicecmds.voicePromptText[12][0] )
				{
					trap_SendServerCommand( -1, va("cp \"\n%s\n\"", voicecmds.voicePromptText[12]));
				}
				if ( voicecmds.voicePromptSound[12][0] )
				{
					G_BroadcastSound( voicecmds.voicePromptSound[12] );
				}
			}
			else if ( !level.shown5min && level.time - level.startTime >= (g_timelimit.integer + level.timeExtension - 5)*60000 ) 
			{
				level.shown5min = qtrue;
				if ( voicecmds.voicePromptText[11][0] )
				{
					trap_SendServerCommand( -1, va("cp \"\n%s\n\"", voicecmds.voicePromptText[11]));
				}
				if ( voicecmds.voicePromptSound[11][0] )
				{
					G_BroadcastSound( voicecmds.voicePromptSound[11] );
				}
			}
		}
	}
}

/*
==================
AutoEvenTeams

Check to see if teams need to be evened
==================
*/
void AutoEvenTeams( void ) 
{
	int team;
	int count;
	int teamred;
	int teamblue;

	if ( level.teamRedLocked || level.teamBlueLocked )
	{
		return;
	}

	teamred = TeamCount( -1, TEAM_RED, NULL );
	teamblue = TeamCount( -1, TEAM_BLUE, NULL );
	if ( teamred - teamblue >= g_autoEvenTeams.integer )
	{
		count = (teamred - teamblue) / 2;
		team = TEAM_RED;
	}
	else if ( teamblue - teamred >= g_autoEvenTeams.integer )
	{
		count = (teamblue - teamred) / 2;
		team = TEAM_BLUE;
	}
	else
	{
		return;
	}

	trap_SendServerCommand( -1, "cp \"^7* Auto Even Teams *\n\"" );
	AdminCmd_DoEvenTeams( NULL, team, count );
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) 
{
	float	thinktime;

	thinktime = ent->nextthink;

	if (thinktime <= 0) 
	{
		return;
	}
	if (thinktime > level.time) 
	{
		return;
	}
	
	ent->nextthink = 0;
	if (!ent->think) 
	{
		Com_Error ( ERR_FATAL, "NULL ent->think");
	}
	ent->think (ent);
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) 
{
	int			i;
	gentity_t	*ent;
	int			msec;

	//NT - store the time the frame started
	level.frameStartTime = trap_Milliseconds();

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) 
	{
		return;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	msec = level.time - level.previousTime;

	// get any cvar changes
	G_UpdateCvars();

	// go through all allocated objects
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) 
	{
		if ( !ent->inuse ) 
		{
			continue;
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) 
		{
			if ( ent->s.event ) 
			{
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if ( ent->client ) 
				{
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			
			if ( ent->freeAfterEvent ) 
			{
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} 
			else if ( ent->unlinkAfterEvent ) 
			{
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) 
		{
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) 
		{
			continue;
		}

		if ( ent->s.eType == ET_MISSILE ) 
		{
			G_RunMissile( ent );
			continue;
		}

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) 
		{
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER ) 
		{
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) 
		{
			G_CheckClientTimeouts ( ent );
			G_RunClient( ent );
			if ( !level.match )
			{
				G_CheckClientTeamkill ( ent );
				G_CheckClientSpawnkill ( ent );
				G_CheckClientFry( ent );
			}
			continue;
		}

		G_RunThink( ent );
	}

	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) 
	{
		if ( ent->inuse ) 
		{
			ClientEndFrame( ent );
		}
	}

	if ( !CheckPause() )
	{
		// Check warmup rules
		CheckWarmup();

		// see if it is time to end the level
		CheckExitRules();

		// Update gametype stuff
		CheckGametype ();		
	}
	// Duplicate this for simplicity
	else if ( level.matchPending && level.intermissiontime ) 
	{
		CheckIntermissionExit ();
	}

	// Send a server message if necessary
	if ( !level.match )
	{
		ServerMessage();
	}

#ifdef SMKWEAPONS
	if ( level.time > level.badTimer && level.badFolder != 2 )
	{
		level.badTimer = level.time + 30000;
		if ( level.badFolder == 0 )
		{
			G_CheckModFolder();
		}
		if ( level.badFolder == 1 )
		{
			trap_SendServerCommand( -1, "chat -1 \"^1WARNING: ^7This server is running ^3rocmod-wp^7 in the regular ^3rocmod^7 folder! ^1This will create bad PK3 conflicts in all players!\n\"" );
		}

	}
#endif
	if ( level.time > level.periodicTimer )
	{
		level.periodicTimer = level.time + 3000;
		ServerPeriodicActions();
	}
	if ( level.time > level.autoEvenTeamsTimer && g_autoEvenTeams.integer > 1 )
	{
		level.autoEvenTeamsTimer = level.time + (g_autoEvenTeamsInterval.integer * 1000);
		AutoEvenTeams();
	}
	if ( g_clientFeatures.integer && level.time > level.updateTimer )
	{
		level.updateTimer = level.time + (g_clientFeatures.integer * 1000);
		G_SendExtraTeamInfo( NULL );
	}

	trap_GT_RunFrame ( level.time );

	// cancel vote if timed out
	CheckVote();

	// for tracking changes
	CheckCvars();

	if (g_listEntity.integer) 
	{
		for (i = 0; i < MAX_GENTITIES; i++) 
		{
			Com_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}
}

void G_InitGhoul ( void )
{
	G_InitHitModels ( );
}

void G_ShutdownGhoul ( void )
{
	if ( !level.serverGhoul2 )
	{
		return;
	}

	trap_G2API_CleanGhoul2Models ( &level.serverGhoul2 );
	level.serverGhoul2 = NULL;
}

