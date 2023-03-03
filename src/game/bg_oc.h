/*
===========================================================================
Copyright (C) 2009 Byron Johnson

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef _G_OC_H
#define _G_OC_H

/* Preprocessor abuse begins here. */

/*
 * bg_oc.h
 */

#ifdef _TREMULOUS_H
#error tremulous.h was included before g_oc.h
#endif /* ifndef _TREMULOUS_H */

#define BG_OC_OCMode() ((oc_gameMode) ? (1) : (0))

#define TREMULOUS_VALUE(d, o) ((BG_OC_OCMode()) ? (o) : (d))

extern int oc_gameMode;

// typedef struct gentity_s gentity_t;  // gentity_t might not be typedef'd here yet
// typedef int weapon_t;  // ditto

#define gentity_t struct gentity_s
#define weapon_t int

// these can be post-2.0
// TODO: add 'u'oc
// TODO: restore OC stuff on ptrc (can be post-2.0, but highest priority)
// TODO: Initialize value for scrim weapon feeder
// TODO: help reads !info directly, or have a client-side "info"
// TODO: fix player names not showing (can be post-2.0)
// TODO: 'x' is building 'x', if it's worth adding (post-2.0)
// TODO: fix advanced granger mispredictions (different bbox from server) and let the client know that it can wallwalk

//<+===============================================+><+===============================================+>
// game only stuff
//<+===============================================+><+===============================================+>

#ifdef GAME
	#include "../qcommon/q_shared.h"

	//<+===============================================+>
	// admin
	//<+===============================================+>

	#define G_OC_CanOverride(ent) ((BG_OC_OCMode()) ? ((G_admin_canEditOC((ent))) ? (1) : (0)) : (0))

	#define MAX_ADMIN_HIDES 1024
	#define MAX_ADMIN_HIDE_REASON 50

	#define MAX_ADMIN_SHOWHIDES 10

	typedef struct g_admin_hideTest
	{
		char name[MAX_NAME_LENGTH];
		char guid[33];
		char ip[40];
	}
	g_admin_hideTest_t;

	qboolean G_admin_editoc(void *ent);
	qboolean G_admin_hide(void *ent);
	qboolean G_admin_showhides(void *ent);
	qboolean G_admin_devchangemap(void *ent);
	qboolean G_admin_layoutsave(void *ent);
	qboolean G_admin_adjusthide(void *ent);
	qboolean G_admin_startscrim(void *ent);
	qboolean G_admin_endscrim(void *ent);

	qboolean G_admin_canEditOC(void *ent);

	qboolean G_admin_hide_check(void *ent, const char *userinfo, char *reason, int rlen, int *hidden, int *hiddenTime, void *hideP);

	#define G_OC_ADMINDEFS \
	, \
	{"adjusthide", (qboolean(*)(gentity_t *))(G_admin_adjusthide), "hide", \
      "change the duration or reason of a hide.  duration is specified as " \
      "numbers followed by units 'w' (weeks), 'd' (days), 'h' (hours) or " \
      "'m' (minutes), or seconds if no units are specified.  If the duration is" \
      " preceded by a + or -, the hide duration will be extended or shortened by" \
      " the specified amount.  Hidden can be \"yes\" or \"no\".  Use two backspaces (\"\\\\\")" \
      " if the reason should begin with a backslash.", \
      "[^3hide#^7] (^5/mask^7) (^5duration^7) (^5(\\\\)reason^7) (^5\\hidden^7)" \
    }, \
 \
    {"devchangemap", (qboolean(*)(gentity_t *))(G_admin_devchangemap), "devchangemap", \
      "load a map with cheats (and optionally force layout)", \
      "[^3mapname^7] (^5layout^7)" \
    }, \
 \
	{"editoc", (qboolean(*)(gentity_t *))(G_admin_editoc), "editoc", \
		"editoc", \
		"[^30 - none|1 - admins|2 - all#^7]" \
	}, \
 \
	{"endscrim", (qboolean(*)(gentity_t *))(G_admin_endscrim), "scrim", \
		"Ends the scrim taking place", \
		"" \
	}, \
 \
	{"adminhide", (qboolean(*)(gentity_t *))(G_admin_hide), "adminhide", \
		"hide a player", \
		"[^3name|slot#^7]" \
	}, \
 \
	{"adminhide", (qboolean(*)(gentity_t *))(G_admin_hide), "hide", \
		"hide a player", \
		"[^3name|slot#^7]" \
	}, \
 \
	{"playerhide", (qboolean(*)(gentity_t *))(G_admin_hide), "hide", \
		"hide a player", \
		"[^3name|slot#^7]" \
	}, \
 \
	{"hideplayer", (qboolean(*)(gentity_t *))(G_admin_hide), "hideplayer", \
		"hide a player", \
		"[^3name|slot#^7]" \
	}, \
 \
	{"layoutsave", (qboolean(*)(gentity_t *))(G_admin_layoutsave), "layoutsave", \
		"save a map layout", \
		"[^3layoutname^7]" \
	}, \
 \
	{"layoutsavereview", (qboolean(*)(gentity_t *))(G_admin_layoutsave), "editoc", \
		"save a map layout", \
		"[^3layoutname^7]" \
	}, \
 \
	{"showhides", (qboolean(*)(gentity_t *))(G_admin_showhides), "hide", \
		"display a (partial) list of active hides", \
		"(^5start at hide#^7) (^5name|IP^7)" \
	}, \
 \
	{"startscrim", (qboolean(*)(gentity_t *))(G_admin_startscrim), "scrim", \
		"Starts a medical station scrim or an armoury scrim", \
		"[^3m|a -- medical station or armoury scrim^7]" \
	}, \
 \
	{"unhide", (qboolean(*)(gentity_t *))(G_admin_hide), "hide", \
		"Un-Hide a player", \
		"[^3name|slot#^7]" \
	}

	#define ADMF_CAN_PERM_HIDE "CANPERMHIDE"
	#define MAX_ADMIN_SHOWHIDES 10

    #define G_OC_ADMINBEG \
		typedef struct g_admin_hide \
		{ \
			struct g_admin_hide *next; \
			char name[MAX_NAME_LENGTH]; \
			char guid[33]; \
			addr_t ip; \
			char reason[MAX_ADMIN_HIDE_REASON]; \
			char made[18];  /* big enough for strftime() %c */ \
			int expires; \
			char hider[MAX_NAME_LENGTH]; \
			int hidden; \
		} g_admin_hide_t; \
	    extern g_admin_hide_t *g_admin_hides;

	#define G_OC_ADMINWRITE \
	{ \
		g_admin_hide_t *h; \
 \
		for(h = g_admin_hides; h; h = h->next) \
		{ \
			/* don't write expired hides */ \
			/* if expires is 0, then it's a perm hide */ \
			if(h->expires != 0 && h->expires <= t) \
				continue; \
 \
			trap_FS_Write("[hide]\n", 7, f); \
			trap_FS_Write("name    = ", 10, f); \
			admin_writeconfig_string(h->name, f); \
			trap_FS_Write("guid    = ", 10, f); \
			admin_writeconfig_string(h->guid, f); \
			trap_FS_Write("ip      = ", 10, f); \
			admin_writeconfig_string(h->ip.str, f); \
			trap_FS_Write("reason  = ", 10, f); \
			admin_writeconfig_string(h->reason, f); \
			trap_FS_Write("made    = ", 10, f); \
			admin_writeconfig_string(h->made, f); \
			trap_FS_Write("expires = ", 10, f); \
			admin_writeconfig_int(h->expires, f); \
			trap_FS_Write("hider  = ", 10, f); \
			admin_writeconfig_string(h->hider, f); \
			trap_FS_Write("hidden = ", 10, f); \
			admin_writeconfig_int(h->hidden, f); \
			trap_FS_Write("\n", 1, f); \
		} \
	}

	#define G_OC_Cleanup() \
	do \
	{ \
		g_admin_hide_t *h; \
 \
		for(h = g_admin_hides; h; h = n) \
		{ \
			n = h->next; \
			BG_Free(h); \
		} \
		g_admin_hides = NULL; \
	} while(0)

	#define G_OC_ADMINREADDEC \
	g_admin_hide_t *h = NULL; \
	int hc = 0; \
	qboolean hide_open;

	#define G_OC_ADMININITOPEN \
	hide_open = qfalse;

	#define G_OC_ADMINREADSET \
	else if(hide_open) \
	{ \
		if(!Q_stricmp(t, "name")) \
		{ \
			admin_readconfig_string(&cnf, h->name, sizeof(h->name)); \
		} \
		else if(!Q_stricmp(t, "guid")) \
		{ \
			admin_readconfig_string(&cnf, h->guid, sizeof(h->guid)); \
		} \
		else if(!Q_stricmp(t, "ip")) \
		{ \
			admin_readconfig_string( &cnf, ip, sizeof( ip ) ); \
			G_AddressParse( ip, &h->ip ); \
		} \
		else if(!Q_stricmp(t, "reason")) \
		{ \
			admin_readconfig_string(&cnf, h->reason, sizeof(h->reason)); \
		} \
		else if(!Q_stricmp(t, "made")) \
		{ \
			admin_readconfig_string(&cnf, h->made, sizeof(h->made)); \
		} \
		else if(!Q_stricmp(t, "expires")) \
		{ \
			admin_readconfig_int(&cnf, &h->expires); \
		} \
		else if(!Q_stricmp(t, "hider")) \
		{ \
			admin_readconfig_string(&cnf, h->hider, sizeof(h->hider)); \
		} \
		else if(!Q_stricmp(t, "hidden")) \
		{ \
			admin_readconfig_int(&cnf, &h->hidden); \
		} \
		else \
		{ \
			COM_ParseError("[hide] unrecognized token \"%s\"", t); \
		} \
	}
 
	#define G_OC_ADMINREADOPEN \
    else if(!Q_stricmp(t, "[hide]")) \
    { \
      if(h) \
        h = h->next = BG_Alloc(sizeof(g_admin_hide_t)); \
      else \
        h = g_admin_hides = BG_Alloc(sizeof(g_admin_hide_t)); \
      hide_open = qtrue; \
      level_open = ban_open = admin_open = command_open = qfalse; \
      hc++; \
    }

	#define G_OC_ADMINNUM (va(", %d hides", hc))

	//<+===============================================+>
	// scrims and votes
	//<+===============================================+>

	#define G_OC_DYNAMICALLY_ALLOCATE_REWARDS

	#ifndef G_OC_DYNAMICALLY_ALLOCATE_REWARDS
	#define G_OC_MAXARMS 512
	#define G_OC_MAXMEDIS 1024
	#endif
	typedef struct
	{
	//	struct g_oc_scrimTeam_t *next;
		int       active;
		char      name[50];  // MAX_NAME_LENGTH isn't here
		int       time;
		#ifdef G_OC_DYNAMICALLY_ALLOCATE_REWARDS
		gentity_t **arms;
		gentity_t **medis;
		#else
		gentity_t *arms[G_OC_MAXARMS];
		gentity_t *medis[G_OC_MAXMEDIS];
		#endif
		gentity_t *checkpoint;
		weapon_t  weapon;
		int       flags;
		#define G_OC_SCRIMFLAG_EQUIPMENT     0x0001
		#define G_OC_SCRIMFLAG_NOTSINGLETEAM 0x0002  // set when a player joins a team that already has another player in it and is never unset while the team exists
	} g_oc_scrimTeam_t;

	#define G_OC_MAX_SCRIM_TEAMS ((G_OC_MAX_SCRIM_TEAMS_REAL) + (1))
	#define G_OC_MAX_SCRIM_TEAMS_REAL 63
	#define G_OC_MIN_SCRIM_TEAMS 2

	/*
	#define G_OC_GETTEAM(d, s, n) \
	{ \
		int i; \
		for(i = 0, (d) = (s); (d) && i < (n); (d) = ((d)->next), i++); \
		if(!t) \
			G_LogPrintf("ERROR: non-existant team %d requested\n", n); \
	}
	*/

	// no longer a linked list
	#define G_OC_GETTEAM(d, s, n) \
	{ \
		(d) = (s) + (n); \
	}

	#define G_OC_SCRIMAFTERTIME 20000  // 20 seconds after the first team wins for other teams to win

	#define G_OC_SCRIMTIME (t->time - level.ocStartTime - (G_OC_IsSingleScrim() ? 0 : g_ocWarmup.integer) * 1000)

	#define G_OC_STATE_NONE 0
	#define G_OC_STATE_PREP 1
	#define G_OC_STATE_WARM 2
	#define G_OC_STATE_PLAY 3

	#define G_OC_MODE_NONE 0
	#define G_OC_MODE_MEDI 1
	#define G_OC_MODE_ARM  2

	#define G_OC_NeedOtherVoteCheck() ((BG_OC_OCMode()) ? ((level.ocScrimVote) ? (1) : (0)) : (0))
	#define G_OC_OtherVoteCheck() \
	do \
	{ \
		int i; \
		int scrimClients = 0; \
		gentity_t *client; \
 \
		for(i = 0; i < MAX_CLIENTS; i++) \
		{ \
			client = g_entities + i; \
 \
			if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam) \
			{ \
				scrimClients++; \
			} \
		} \
 \
		if(level.time - level.voteTime >= VOTE_TIME || (voteYes + voteNo == scrimClients)) \
		{ \
			if(voteYesPercent >= votePassThreshold || voteNo == 0) \
			{ \
				/* execute the command, then remove the vote */ \
				trap_SendServerCommand(-1, va("print \"Vote passed (%d - %d)\n\"", voteYes, voteNo)); \
				G_LogPrintf("Vote passed\n"); \
				level.voteExecuteTime = level.time + VOTE_TIME; \
			} \
			else \
			{ \
				/* same behavior as a timeout */ \
				trap_SendServerCommand(-1, va("print \"Vote failed (%d - %d)\n\"", voteYes, voteNo)); \
				G_LogPrintf("Vote failed\n"); \
			} \
		} \
		else if(g_majorityVotes.integer) \
		{ \
			if(voteYes > (int)((double)level.numConnectedClients * ((double)votePassThreshold/100.0))) \
			{ \
				/* execute the command, then remove the vote */ \
				trap_SendServerCommand(-1, va("print \"Vote passed (majority) (%d - %d)\n\"", voteYes, voteNo)); \
				level.voteExecuteTime = level.time + VOTE_TIME; \
			} \
			else if(voteNo > (int)((double)level.numConnectedClients * ((double)(100.0-votePassThreshold)/100.0))) \
			{ \
				/* same behavior as a timeout */ \
				trap_SendServerCommand(-1, va("print \"Vote failed (majority) (%d - %d)\n\"", voteYes, voteNo)); \
			} \
			else \
			{ \
				/* still waiting for a majority */ \
				return; \
			} \
		} \
	} while(0)

	#define G_OC_PostCheckVote() level.ocScrimVote = 0;

	//<+===============================================+>
	// oc stats, ratings and layouts
	//<+===============================================+>

	#define G_OC_STAT_MAXRECORDS 32
	#define G_OC_MAX_LAYOUT_RATINGS 1024

	void G_OC_LoadRatings(void);
	char *G_OC_Rating(char *mapname, char *layoutname);
	void G_OC_LayoutLoad(char *layout);

	typedef struct stat_s stat_t;
	struct stat_s
	{
		int count;
		int time;  // in ms
		char name[MAX_NAME_LENGTH];
		char date[256];
		char guid[33];
		char ip[33];
		char adminName[MAX_NAME_LENGTH];
	};

	//<+===============================================+>
	// editoc
	//<+===============================================+>

	//<+===============================================+>
	// cvars
	//<+===============================================+>

	#define G_OC_CONNECTIONRECORD \
	gentity_t *checkpoint; \
	gentity_t **medisLastCheckpoint; \
	gentity_t **armsLastCheckpoint; \
	int       totalMedistations; \
	int       totalArmouries; \
	int       lastAliveTime; \
	unsigned int aliveTime; \
	int       hasCheated;

	#define G_OC_CLIENTDATA \
	int       hasCheated; \
	int       cheated; \
	int       scrimTeam; \
	int       nextCheckpointTime; \
	int       nextValidEvolveTime;  \
	gentity_t *checkpoint; \
	gentity_t **medis; \
	gentity_t **medisLastCheckpoint; \
	gentity_t **arms; \
	gentity_t **armsLastCheckpoint; \
	int       hidden; \
	int       hiddenTime; \
	char      hiddenReason[MAX_STRING_CHARS]; \
	int       needEvolve; \
	int       evolveTime; \
	int       aliveTime; \
	int       lastAliveTime; \
	int       noAuO;  /* TODO: <------- ? */ \
	int       mediTime;  /* recorded time for medis */ \
	int       winTime;  /* recorded time for win */ \
	/* frame checking to ease CPU usage */ \
	int       nextWeaponCheckTime; \
	int       nextOverrideCheckTime; \
	int       buildableOverride; \
	int       grenadeUsed;

	#define G_OC_FRAMETIMEWEAPON 1000
	#define G_OC_FRAMETIMEOVERRIDE 1500

	#define G_OC_PTRCDATA

	#define G_OC_CVARS \
	vmCvar_t g_alwaysSaveStats; \
	vmCvar_t g_ocOnly; \
	vmCvar_t g_ocWarmup; /* warmup time for scrims */ \
	vmCvar_t g_statsEnabled; \
	vmCvar_t g_statsRecords; \
	vmCvar_t g_ocReview; \
	vmCvar_t g_allowHide; \
	vmCvar_t g_forceHide; \
	vmCvar_t g_allowHideVote; \
	vmCvar_t g_allowUnhideVote; \
	vmCvar_t g_hideTimeCallvoteMinutes; \
	vmCvar_t g_unhideTimeCallvoteMinutes; \
	vmCvar_t g_timelimitDrop; \
	vmCvar_t g_startScrimVotePercent; \
	vmCvar_t g_endScrimVotePercent; \
	vmCvar_t g_adminMaxHide; \
	vmCvar_t g_ocHostName; \
	vmCvar_t g_ocTimelimit; \
	vmCvar_t g_noOCHostName; \
	vmCvar_t g_noOCTimelimit; \
	vmCvar_t g_voteHideDuration; \
	vmCvar_t g_voteUnhideDuration; \
	vmCvar_t g_ocConfig; \
	vmCvar_t g_noOCConfig;

	#define G_OC_EXTERNCVARS \
	extern vmCvar_t g_alwaysSaveStats; \
	extern vmCvar_t g_ocOnly; \
	extern vmCvar_t g_ocWarmup; \
	extern vmCvar_t g_statsEnabled; \
	extern vmCvar_t g_statsRecords; \
	extern vmCvar_t g_ocReview; \
	extern vmCvar_t g_allowHide; \
	extern vmCvar_t g_forceHide; \
	extern vmCvar_t g_allowHideVote; \
	extern vmCvar_t g_allowUnhideVote; \
	extern vmCvar_t g_hideTimeCallvoteMinutes; \
	extern vmCvar_t g_unhideTimeCallvoteMinutes; \
	extern vmCvar_t g_timelimitDrop; \
	extern vmCvar_t g_startScrimVotePercent; \
	extern vmCvar_t g_endScrimVotePercent; \
	extern vmCvar_t g_adminMaxHide; \
	extern vmCvar_t g_ocHostName; \
	extern vmCvar_t g_ocTimelimit; \
	extern vmCvar_t g_noOCHostName; \
	extern vmCvar_t g_noOCTimelimit; \
	extern vmCvar_t g_voteHideDuration; \
	extern vmCvar_t g_voteUnhideDuration; \
	extern vmCvar_t g_ocConfig; \
	extern vmCvar_t g_noOCConfig;

	#define G_OC_CVARTABLE \
	{ &g_alwaysSaveStats, "g_alwaysSaveStats", "0", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_ocOnly, "g_ocOnly", "0", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_ocWarmup, "g_ocWarmup", "20", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_statsEnabled, "g_statsEnabled", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_statsRecords, "g_statsRecords", "10", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_ocReview, "g_ocReview", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_allowHide, "g_allowHide", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_forceHide, "g_forceHide", "0", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_allowHideVote, "g_allowHideVote", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_allowUnhideVote, "g_allowUnhideVote", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_hideTimeCallvoteMinutes, "g_hideTimeCallvoteMinutes", "5", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_unhideTimeCallvoteMinutes, "g_unhideTimeCallvoteMinutes", "5", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_timelimitDrop, "g_timelimitDrop", "17.5", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_startScrimVotePercent, "g_startScrimVotePercent", "85", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_endScrimVotePercent, "g_endScrimVotePercent", "90", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_adminMaxHide, "g_adminMaxHide", "2w", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_ocHostName, "g_ocHostName", "", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_noOCHostName, "g_noOCHostName", "", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_ocTimelimit, "g_ocTimelimit", "", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_noOCTimelimit, "g_noOCTimelimit", "", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_voteHideDuration, "g_voteHideDuration", "5m", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_voteUnhideDuration, "g_voteUnhideDuration", "5m", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_ocConfig, "g_ocConfig", "oc.cfg", CVAR_ARCHIVE, 0, qfalse  }, \
	{ &g_noOCConfig, "g_noOCConfig", "no-oc.cfg", CVAR_ARCHIVE, 0, qfalse  },

	#define G_OC_LEVEL_LOCALS \
	int totalMedistations; \
	int totalArmouries; \
	int numNodes; \
	g_oc_scrimTeam_t scrimTeam[G_OC_MAX_SCRIM_TEAMS];  /* scrimTeam[0] is always NULL */ \
	int ocLoadTime; \
	int ocLoadDelayedByTrigger; \
	int ocScrimMode; \
	int ocStartTime; \
	int ocScrimState; \
	int scrimEndTime; \
	int scrimWinOrder; /* 0 = nobody's won yet, 1 = one team has one, etc */ \
	int ocEditMode; \
	int ocScrimVote;

	void G_OC_RegisterCvars(void);
	void G_OC_PreRegisterCvars(void);

	//<+===============================================+>
	// initilization
	//<+===============================================+>

	#define OC_PREP_TIME 1500  // enough to get a few frames in
	#define OC_PREP_TRIGGER_TIME 6500

	#define G_OC_NeedLoadOC() (((g_ocOnly.integer >= 0) && ((g_ocOnly.integer > 0) || (tolower(level.layout[0]) == 'o' && tolower(level.layout[1]) == 'c'))) ? ((BG_StrToLower(level.layout)), (trap_SetConfigstring(CS_OCMODE, "1")), (BG_OC_SetOCModeOC()), (1)) : ((trap_SetConfigstring(CS_OCMODE, "0")), (BG_OC_SetOCModeNone()), (0)))

	#define G_OC_LoadOC() \
	do \
	{ \
		const char *filename; \
		unsigned int triggers = 0; \
 \
		BG_StrToLower(level.layout); \
		trap_SetConfigstring(CS_LAYOUT, level.layout); \
 \
		trap_Cvar_Set("g_humanBuildPoints", va("%d", INFINITE)); \
		trap_Cvar_Set("g_alienBuildPoints", va("%d", INFINITE)); \
		/*trap_Cvar_Set("g_alienStage", va("%d", S3));*/ \
		/*trap_Cvar_Set("g_humanStage", va("%d", S3));*/ \
		trap_Cvar_Set("g_alienStage", va("%d", S4)); \
		trap_Cvar_Set("g_humanStage", va("%d", S4)); \
		trap_Cvar_Set("g_tag", "oc"); \
		if(g_ocHostName.string[0]) \
		{ \
			trap_Cvar_Set("sv_hostname", g_ocHostName.string); \
		} \
		if(g_ocTimelimit.string[0]) \
		{ \
			trap_Cvar_Set("timelimit", g_ocTimelimit.string); \
		} \
		triggers += G_Checktrigger_stages(TEAM_ALIENS, S2); \
		triggers += G_Checktrigger_stages(TEAM_HUMANS, S2); \
		triggers += G_Checktrigger_stages(TEAM_ALIENS, S3); \
		triggers += G_Checktrigger_stages(TEAM_HUMANS, S3); \
		G_OC_LoadRatings(); \
		level.ocLoadTime = level.time + OC_PREP_TIME; \
		if(triggers) \
		{ \
			level.ocLoadTime += OC_PREP_TRIGGER_TIME - OC_PREP_TIME; \
			level.ocLoadDelayedByTrigger = 1; \
		} \
 \
		/* set some special BG modes */ \
 \
		if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_NOWALLWALK)) \
		{ \
			BG_OC_SetNoWallWalk(qtrue); \
 \
			trap_SetConfigstring(CS_NOWALLWALK, "1"); \
		} \
		else \
		{ \
			BG_OC_SetNoWallWalk(qfalse); \
 \
			trap_SetConfigstring(CS_NOWALLWALK, "0"); \
		} \
 \
		trap_SetConfigstring(CS_OCMODE, "1"); \
 \
		trap_SetConfigstring(CS_SCRIMTEAMS, "");  /* Reset scrim-team list */ \
 \
		if(g_ocConfig.string[0]) \
		{ \
			filename = va("%s", g_ocConfig.string); \
			if( trap_FS_FOpenFile( filename, NULL, FS_READ ) ) \
			{ \
				trap_SendConsoleCommand( EXEC_APPEND, va( "exec \"%s\"\n", filename ) ); \
			} \
		} \
	} while(0)

	#define G_OC_NoLoadOC() \
	{ \
		const char *filename; \
 \
		trap_Cvar_Set("g_tag", "main"); \
		trap_SetConfigstring(CS_NOWALLWALK, "0"); \
		trap_SetConfigstring(CS_OCMODE, "0"); \
 \
		BG_StrToLower(level.layout); \
		trap_SetConfigstring(CS_LAYOUT, level.layout); \
 \
		trap_SetConfigstring(CS_SCRIMTEAMS, "");  /* Reset scrim-team list */ \
 \
		if(g_noOCHostName.string[0]) \
		{ \
			trap_Cvar_Set("sv_hostname", g_noOCHostName.string); \
		} \
		if(g_noOCTimelimit.string[0]) \
		{ \
			trap_Cvar_Set("timelimit", g_noOCTimelimit.string); \
		} \
 \
		if(g_noOCConfig.string[0]) \
		{ \
			filename = va("%s", g_noOCConfig.string); \
			if( trap_FS_FOpenFile( filename, NULL, FS_READ ) ) \
			{ \
				trap_SendConsoleCommand( EXEC_APPEND, va( "exec \"%s\"\n", filename ) ); \
			} \
		} \
	} while(0)

	//<+===============================================+>
	// game and balance stuff
	//<+===============================================+>

  #define G_OC_SUBTRACTFUNDS ((BG_OC_OCMode()) ? ((!BG_InventoryContainsUpgrade(UP_BATTLESUIT_GOLD, ent->client->ps.stats)) || (upgrade == UP_BATTLESUIT_GOLD)) : (1))  // hardcoded hack  // when upgrade isn't set (buying or selling a weapon), upgrade won't be UP_BATTLESUIT_GOLD

  #define G_OC_WEAPON \
	do \
  { \
		if( BG_OC_OCMode() ) \
			break; \
 \
		if( upgrade == UP_BATTLESUIT_CHROME ) \
		{ \
      trap_SendServerCommand( ent-g_entities, "print \"You can't buy obstacle course items\n\"" ); \
			return; \
		} \
 \
		if( upgrade == UP_BATTLESUIT_GOLD ) \
		{ \
      trap_SendServerCommand( ent-g_entities, "print \"You can't buy obstacle course items\n\"" ); \
			return; \
		} \
  } while(0);

  #define G_OC_UPGRADE \
	do \
  { \
		if( BG_OC_OCMode() ) \
			break; \
 \
  } while(0);

	#define G_OC_NoPunishment() ((BG_OC_OCMode() ? (1) : (0)))

	#define G_OC_NoFreeFunds() ((BG_OC_OCMode() ? (1) : (0)))

	#define G_OC_NoCreepThroughWalls() (!(BG_OC_OCMode()))

	#define G_OC_OCHovelNeverOccupied() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_GroupID() ((BG_OC_OCMode()) ? (1) : (0))

	//char *G_OC_MediStats(gclient_t *client, int count, int time);
	//char *G_OC_WinStats(gclient_t *client, int count, int time);
	char *G_OC_MediStats(void *client, int count, int time);
	char *G_OC_WinStats(void *client, int count, int time);

	#define G_OC_NoKnockbackCap() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_NoDamageAlert() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_CanBuildableBeDestoryedOnOtherTeam() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_CanBuildablesMove() (!(BG_OC_OCMode()))

	#define G_OC_Damage() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* test and process checkpoints */ \
		G_OC_Checkpoint(targ, attacker); \
 \
		/* no damage to a buildable and no damage from a client */ \
		if(attacker->client) \
			return; \
		if(targ->s.eType == ET_BUILDABLE) \
			return; \
	} while(0)

	#define G_OC_PLAYERSPAWN(x) G_OC_PlayerSpawn((x))

	#define G_OC_SelectHumanSpawnPoint() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(not)  /* a telenode */ \
			break; \
 \
		if(!ent) \
			break; \
 \
		if(!ent->client) \
			break; \
 \
		if(ent->client->pers.scrimTeam) \
		{ \
			if(level.ocScrimState >= G_OC_STATE_PLAY) \
			{ \
				spot = level.scrimTeam[ent->client->pers.scrimTeam].checkpoint; \
 \
				if(spot && G_CheckSpawnPoint(spot->s.number, spot->s.origin, spot->s.origin2, spot->s.modelindex, NULL) == NULL) \
					return spot; \
			} \
		} \
		else if(ent->client->pers.checkpoint) \
		{ \
			spot = ent->client->pers.checkpoint; \
 \
			if(spot && G_CheckSpawnPoint(spot->s.number, spot->s.origin, spot->s.origin2, spot->s.modelindex, NULL) == NULL) \
				return spot; \
		} \
	} while(0)

	#define G_OC_NoTurretDroop() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_SelectAlienSpawnPoint() G_OC_SelectHumanSpawnPoint()

	#define G_OC_SpotNeverTelefrags() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_NeedOtherSayTeamCheck() ((BG_OC_OCMode()) ? ((ent && ent->client->pers.scrimTeam && mode == SAY_TEAM) ? (1) : (0)) : (0))
	#define G_OC_OtherSayTeamCheck() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent) \
			break; \
 \
		if(mode == SAY_TEAM && other->client->pers.scrimTeam != ent->client->pers.scrimTeam) \
		{ \
			if(other->client->pers.teamSelection != TEAM_NONE) \
				return qfalse; \
 \
			/* if(!G_admin_permission(other, ADMF_SPEC_ALLCHAT)) */  /* even if they are admins, they can't real teamchat from scrim teams */ \
				return qfalse; \
		} \
	} while(0)

	#define G_OC_NeedResetStages() (!(BG_OC_OCMode()))
	#define G_OC_NeedSuddenDeath() (!(BG_OC_OCMode()))
	#define G_OC_NeedSuddenDtMsg() (!(BG_OC_OCMode()))
	#define G_OC_NeedTimelimigMg() (!(BG_OC_OCMode()))

	#define G_OC_MAPROTATIONTOKENS \
	else if( !Q_stricmp( token, "oc" ) ) \
	{ \
		condition->lhs = CV_OC; \
 \
		token = COM_Parse( text_p ); \
 \
		if( !*token ) \
			return qfalse; \
 \
		if( !Q_stricmp( token, "yes" ) ) \
			condition->oc = qtrue; \
		else if( !Q_stricmp( token, "no" ) ) \
			condition->oc = qfalse; \
		else \
		{ \
			/* default to yes*/ \
			condition->oc = qtrue; \
			/*G_Printf( S_COLOR_RED "ERROR: invalid right hand side in expression: %s\n", token );*/ \
			/*return qfalse;*/ \
		} \
	}

	#define G_OC_EVALUATEMAPCONDITION \
    case CV_OC: \
      result = !!BG_OC_OCMode() == !!localCondition->oc; \
      break;

	#define G_OC_PTRCUpdate() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
break;  /* TODO: the current ptrc for oc data causes memory corruption and doesn't work.  Needs to be rewritten */ \
 \
		if(level.totalMedistations && client->pers.medisLastCheckpoint && (!client->pers.connection->totalMedistations || client->pers.connection->totalMedistations == level.totalMedistations)) \
		{ \
			if(!client->pers.connection->totalMedistations) \
				client->pers.connection->totalMedistations = level.totalMedistations; \
 \
			if(!client->pers.connection->medisLastCheckpoint) \
			{ \
				client->pers.connection->medisLastCheckpoint = BG_Alloc((level.totalMedistations) * sizeof(gentity_t *)); \
			} \
 \
			memcpy(client->pers.connection->medisLastCheckpoint, client->pers.medisLastCheckpoint, level.totalMedistations + 1); \
		} \
		else if(client->pers.connection->medisLastCheckpoint) \
		{ \
			BG_Free(client->pers.connection->medisLastCheckpoint); \
			client->pers.connection->medisLastCheckpoint = NULL; \
		} \
 \
		if(level.totalArmouries && client->pers.armsLastCheckpoint && (!client->pers.connection->totalArmouries || client->pers.connection->totalArmouries == level.totalArmouries)) \
		{ \
			if(!client->pers.connection->totalArmouries) \
				client->pers.connection->totalArmouries = level.totalArmouries; \
 \
			if(!client->pers.connection->armsLastCheckpoint) \
			{ \
				client->pers.connection->armsLastCheckpoint = BG_Alloc((level.totalArmouries) * sizeof(gentity_t *)); \
			} \
 \
			memcpy(client->pers.connection->armsLastCheckpoint, client->pers.armsLastCheckpoint, level.totalArmouries + 1); \
		} \
		else if(client->pers.connection->armsLastCheckpoint) \
		{ \
			BG_Free(client->pers.connection->armsLastCheckpoint); \
			client->pers.connection->armsLastCheckpoint = NULL; \
		} \
 \
		client->pers.connection->checkpoint = client->pers.checkpoint; \
		client->pers.connection->aliveTime = client->pers.aliveTime; \
		client->pers.connection->lastAliveTime = client->pers.lastAliveTime;  /* TODO: find out if this is necessary (it probably isn't, but probably is here just in case) */ \
		client->pers.connection->hasCheated = client->pers.hasCheated; \
		if(client->pers.scrimTeam) \
		{ \
			client->pers.connection->checkpoint = NULL; \
			client->pers.connection->hasCheated = 1; \
			client->pers.connection->aliveTime = INFINITE; \
			/* client->pers.connection->lastAliveTime = INFINITE */  /* unnecessary? */ \
			while(client->pers.connection->aliveTime < 1) client->pers.connection->aliveTime--; \
			/* while(client->pers.connection->lastAliveTime < 1) client->pers.connection->lastAliveTime--; */ \
		} \
	} while(0)

	#define G_OC_NeedAlternateHiveSearchAndDestroy() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_AlternateHiveSearchAndDestroy() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		else if(self->timestamp > level.time) \
		{ \
			VectorCopy(self->r.currentOrigin, self->s.pos.trBase); \
			self->s.pos.trType = TR_STATIONARY; \
			self->s.pos.trTime = level.time; \
 \
			self->think = G_ExplodeMissile; \
			self->nextthink = level.time + 50; \
			self->parent->active = qfalse;  /* allow the parent to start again */ \
			return; \
		} \
	} while(0)

	#define G_OC_RADIUSDAMAGE \
	if((BG_OC_OCMode())) \
	{ \
		if(ent != attacker) \
			continue; \
 \
		if(mod == MOD_GRENADE && ent->client) \
			ent->client->pers.grenadeUsed = 1; \
	}

	#define G_OC_CloseRangeWeaponFired(x) \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent->client) \
			break; \
 \
		if(!ent->client->pers.scrimTeam) \
			break; \
 \
		if(level.ocScrimState < G_OC_STATE_PLAY) \
			break; \
 \
		bulletFire(x, RIFLE_SPREAD, RIFLE_DMG, MOD_GRENADE); \
	} while(0)

	#define G_OC_NeedAlternateVenomAttackCheck() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_AlternateVenomAttackCheck() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(traceEnt->s.eType != ET_BUILDABLE) \
			return qfalse; \
 \
		if(traceEnt->s.modelindex != BA_A_BOOSTER) \
			return qfalse; \
	} while(0)

	#define G_OC_FireWeapon() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent || !ent->client) \
			break; \
 \
		if(ent->client->pers.teamSelection != TEAM_ALIENS) \
			break; \
 \
		if(ent->client->pers.scrimTeam) \
		{ \
			g_oc_scrimTeam_t *t; \
			G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam); \
			if(t->active) \
			{ \
				/* throw a grenade if it's the team's weapon because otherwise the grenade won't be thrown */ \
				if(t->weapon == WP_GRENADE) \
				{ \
					throwGrenade(ent); \
				} \
			} \
		} \
 \
		/* aliens can't fire weapons */ \
		/* but sometimes they need weapons to distinguish on teams */ \
		/* so simulate a goon */ \
		if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL0) \
		{ \
			;; \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_BUILDER0) \
		{ \
			if (G_admin_canEditOC(ent)) \
			{ \
				buildFire( ent, MN_A_BUILD ); \
			} \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_BUILDER0_UPG) \
		{ \
			if (G_admin_canEditOC(ent)) \
			{ \
				buildFire( ent, MN_A_BUILD ); \
			} \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL1) \
		{ \
			meleeAttack(ent, LEVEL1_CLAW_RANGE, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_DMG, MOD_LEVEL1_CLAW); \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL1_UPG) \
		{ \
			meleeAttack(ent, LEVEL1_CLAW_RANGE, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_DMG, MOD_LEVEL1_CLAW); \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL2) \
		{ \
			meleeAttack(ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW); \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL2_UPG) \
		{ \
			meleeAttack(ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW); \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL3) \
		{ \
			meleeAttack(ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW); \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL3_UPG) \
		{ \
			meleeAttack(ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW); \
		} \
		else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL4) \
		{ \
			meleeAttack(ent, LEVEL4_CLAW_RANGE, LEVEL4_CLAW_WIDTH, LEVEL4_CLAW_WIDTH, LEVEL4_CLAW_DMG, MOD_LEVEL4_CLAW); \
		} \
		else \
		{ \
			G_ClientPrint(ent, "^1Error: ^3Alien class unknown for scrim - using Dragoon chomp", CLIENT_SPECTATORS); \
			meleeAttack(ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW); \
		} \
 \
		return; \
	} while(0)

	#define G_OC_FireWeapon2() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent->client) \
			break; \
 \
		AngleVectors(ent->client->ps.viewangles, forward, right, up); \
		CalcMuzzlePoint(ent, forward, right, up, muzzle); \
 \
		if(!ent->client->pers.scrimTeam || ent->client->pers.override || G_admin_canEditOC(ent)) \
		{ \
			if(ent->s.weapon == WP_ABUILD || ent->s.weapon == WP_ABUILD2 || ent->s.weapon == WP_HBUILD) \
			{ \
				cancelBuildFire(ent); \
			} \
 \
			return; \
		} \
 \
		if(ent->client->pers.teamSelection == TEAM_ALIENS) \
		{ \
			/* aliens can't fire weapons */ \
			/* but sometimes they need weapons to distinguish on teams */ \
			/* so simulate a goon */ \
			if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL0) \
			{ \
				;; \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_BUILDER0) \
			{ \
				;; \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_BUILDER0_UPG) \
			{ \
				;; \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL1) \
			{ \
				meleeAttack(ent, LEVEL1_CLAW_RANGE, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_DMG, MOD_LEVEL1_CLAW); \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL1_UPG) \
			{ \
				meleeAttack(ent, LEVEL1_CLAW_RANGE, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_DMG, MOD_LEVEL1_CLAW); \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL2) \
			{ \
				meleeAttack(ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW); \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL2_UPG) \
			{ \
				meleeAttack(ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW); \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL3) \
			{ \
				meleeAttack(ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW); \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL3_UPG) \
			{ \
				meleeAttack(ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW); \
			} \
			else if(ent->client->pers.classSelection == PCL_ALIEN_LEVEL4) \
			{ \
				meleeAttack(ent, LEVEL4_CLAW_RANGE, LEVEL4_CLAW_WIDTH, LEVEL4_CLAW_WIDTH, LEVEL4_CLAW_DMG, MOD_LEVEL4_CLAW); \
			} \
			else \
			{ \
				G_ClientPrint(ent, "^1Error: ^3Alien class unknown for scrim - using Dragoon chomp", CLIENT_SPECTATORS); \
				meleeAttack(ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW); \
			} \
 \
			if(ent->s.weapon == WP_ALEVEL1_UPG) \
			{ \
				poisonCloud(ent); \
			} \
			if(ent->s.weapon == WP_ALEVEL2_UPG) \
			{ \
				poisonCloud(ent); \
			} \
			/* if(ent->s.weapon == WP_ABUILD || ent->s.weapon == WP_ABUILD2 || ent->s.weapon == WP_HBUILD || ent->s.weapon == WP_HBUILD2) */ \
			if(ent->s.weapon == WP_ABUILD || ent->s.weapon == WP_ABUILD2 || ent->s.weapon == WP_HBUILD) \
			{ \
				cancelBuildFire(ent); \
			} \
		} \
		else \
		{ \
			if(ent->s.weapon == WP_LUCIFER_CANNON) \
			{ \
				LCChargeFire(ent, qtrue); \
			} \
			if(ent->s.weapon == WP_ALEVEL1_UPG) \
			{ \
				poisonCloud(ent); \
			} \
			if(ent->s.weapon == WP_ALEVEL2_UPG) \
			{ \
				poisonCloud(ent); \
			} \
			/* if(ent->s.weapon == WP_ABUILD || ent->s.weapon == WP_ABUILD2 || ent->s.weapon == WP_HBUILD || ent->s.weapon == WP_HBUILD2) */ \
			if(ent->s.weapon == WP_ABUILD || ent->s.weapon == WP_ABUILD2 || ent->s.weapon == WP_HBUILD) \
			{ \
				cancelBuildFire(ent); \
			} \
		} \
 \
		return; \
	} while(0)

	#define G_OC_FireWeapon3() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* Normal grangers can't fire blobs in OC mode */ \
		if(ent->client->pers.classSelection == PCL_ALIEN_BUILDER0) \
			return; \
 \
		if(ent->client->pers.scrimTeam && !ent->client->pers.override && !G_admin_canEditOC(ent)) \
			return; \
	} while(0)

	#define G_OC_ClientDisconnect() \
	do \
	{ \
		gclient_t *client; \
 \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent || !ent->client) \
			break; \
 \
		client = ent->client; \
 \
		/* cleanly handle scrim teams */ \
		G_OC_RemovePlayerFromScrimTeam(ent); \
 \
		if(level.totalMedistations && client->pers.medis && client->pers.medisLastCheckpoint) \
		{ \
			BG_Free(client->pers.medis); \
			client->pers.medis = NULL; \
			BG_Free(client->pers.medisLastCheckpoint); \
			client->pers.medisLastCheckpoint = NULL; \
		} \
		if(level.totalArmouries && client->pers.arms && client->pers.armsLastCheckpoint) \
		{ \
			BG_Free(client->pers.arms); \
			client->pers.arms = NULL; \
			BG_Free(client->pers.armsLastCheckpoint); \
			client->pers.armsLastCheckpoint = NULL; \
		} \
	} while(0)

	#define G_OC_ClientBegin() \
	do \
	{ \
		char reason[MAX_STRING_CHARS] = {""}; \
		char userinfo[MAX_INFO_STRING]; \
		int hidden, hiddenTime; \
 \
		if(!BG_OC_OCMode()) \
			break; \
 \
		trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); \
 \
		if(G_admin_hide_check(ent, userinfo, reason, sizeof(reason), &hidden, &hiddenTime, NULL)) \
		{ \
			client->pers.hiddenTime = hiddenTime; \
			Q_strncpyz(client->pers.hiddenReason, reason, sizeof(client->pers.hiddenReason)); \
 \
			if(hidden) \
			{ \
				G_StopFromFollowing(ent, 0); \
				ent->r.svFlags |= SVF_SINGLECLIENT; \
				ent->r.singleClient = clientNum; \
				client->pers.hidden = qtrue; \
			} \
			else \
			{ \
				ent->r.svFlags &= ~SVF_SINGLECLIENT; \
				client->pers.hidden = qfalse; \
			} \
 \
			trap_SendServerCommand(clientNum, va("print \"You have been %shidden.  Reason: %s\n\"", client->pers.hidden ? "" : "un", (client->pers.hiddenReason[0]) ? (client->pers.hiddenReason) : ("hidden by admin"))); \
		} \
 \
		if(level.totalMedistations) \
		{ \
			client->pers.medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *)); \
			client->pers.medisLastCheckpoint = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *)); \
		} \
		if(level.totalArmouries) \
		{ \
			client->pers.arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *)); \
			client->pers.armsLastCheckpoint = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *)); \
		} \
 \
		client->pers.nextWeaponCheckTime = client->pers.nextOverrideCheckTime = level.time; \
	} while(0)

	#define BG_OC_NeedPreCreep() ((BG_OC_OCMode()) ? (1) :(0))
	#define BG_OC_PreCreep() \
	do \
	{ \
		return qtrue; \
	} while(0)

	#define G_OC_NeedNoCreep() (BG_OC_OCMode() ? ((BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_NOCREEP)) ? (1) : (0)) : (0))
	#define G_OC_NoCreep() \
	do \
	{ \
		return; \
	} while(0)

	// TODO: better system than ugly hardcoded default powered or unpowered
	#define G_OC_OvermindPowered() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!self->spawned) \
			break; \
 \
		if(self->health <= 0) \
			break; \
 \
		switch(self->groupID) \
		{ \
			default: \
			case 0: \
				self->powered = qtrue; \
				break; \
 \
			case 1: \
				self->powered = 1; \
				break; \
 \
			case 2: \
				self->powered = 0; \
				break; \
		} \
	} while(0)

	#define G_OC_FollowTest() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* test if the player is hidden */ \
		if(level.clients[i].pers.hidden && !G_admin_permission(ent, ADMF_SPEC_ALLCHAT)) \
		{ \
			/* cannot spectate */ \
			return; \
		} \
	} \
	while(0)

	#define G_OC_NewFollowTest() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* test if the player is hidden */ \
		if(level.clients[clientnum].pers.hidden && !G_admin_permission(ent, ADMF_SPEC_ALLCHAT)) \
		{ \
			/* cannot spectate */ \
			continue; \
		} \
	} \
	while(0)

	#define G_OC_NeedAlternateCanBuild() (BG_OC_OCMode() ? (1) : (0))
	#define G_OC_AlternateCanBuild() \
	do \
	{ \
		int override; \
 \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(ent && ent->client && !ent->client->pers.buildableOverride) \
			override = 0; \
		else \
			override = 1; \
 \
		/* Stop all buildables from interacting with traces */ \
		/*if(!override)*/ \
		  /*G_SetBuildableLinkState(qfalse);*/  /* in OC mode, stackables ae allowed if buildable override is on for the client */ \
 \
		BG_BuildableBoundingBox(buildable, mins, maxs); \
 \
		BG_PositionBuildableRelativeToPlayer(ps, mins, maxs, trap_Trace, entity_origin, angles, &tr1); \
		trap_Trace(&tr2, entity_origin, mins, maxs, entity_origin, ent->s.number, MASK_PLAYERSOLID); \
		trap_Trace(&tr3, ps->origin, NULL, NULL, entity_origin, ent->s.number, MASK_PLAYERSOLID); \
 \
		VectorCopy(entity_origin, origin); \
 \
		VectorCopy(tr1.plane.normal, normal); \
		minNormal = BG_Buildable(buildable)->minNormal; \
		invert = BG_Buildable(buildable)->invertNormal; \
 \
		/* can we build at this angle? */ \
		if(!override) \
		{ \
			if(!(normal[2] >= minNormal || (invert && normal[2] <= -minNormal))) \
			reason = IBE_NORMAL; \
			\
			if(tr1.entityNum != ENTITYNUM_WORLD) \
			reason = IBE_NORMAL; \
			\
		} \
		contents = trap_PointContents(entity_origin, -1); \
		buildPoints = BG_Buildable(buildable)->buildPoints; \
 \
		if(!override) \
		{ \
			if(ent->client->ps.stats[STAT_TEAM] == TEAM_ALIENS) \
			{ \
				/*alien criteria */ \
				\
				/* \
				/8 Check there is an Overmind 8/ \
				if(buildable != BA_A_OVERMIND) \
				{ \
					if(!level.overmindPresent) \
					reason = IBE_NOOVERMIND; \
				} \
				*/ \
				\
				/*check there is creep near by for building on */ \
				if(BG_Buildable(buildable)->creepTest) \
				{ \
					if(!G_IsCreepHere(entity_origin)) \
					reason = IBE_NOCREEP; \
				} \
				\
				if(buildable == BA_A_HOVEL) \
				{ \
					vec3_t    builderMins, builderMaxs; \
					\
					/*this assumes the adv builder is the biggest thing that'll use the hovel */ \
					BG_ClassBoundingBox(PCL_ALIEN_BUILDER0_UPG, builderMins, builderMaxs, NULL, NULL, NULL); \
					\
					if(APropHovel_Blocked(origin, angles, normal, ent)) \
					reason = IBE_HOVELEXIT; \
				} \
				\
				/* Check permission to build here */ \
				if(tr1.surfaceFlags & SURF_NOALIENBUILD || contents & CONTENTS_NOALIENBUILD) \
				reason = IBE_PERMISSION; \
			} \
			else if(ent->client->ps.stats[STAT_TEAM] == TEAM_HUMANS) \
			{ \
				/*human criteria */ \
				\
				/* Check for power */ \
				/* \
				if(G_IsPowered(entity_origin) == BA_NONE) \
				{ \
					/8tell player to build a repeater to provide power 8/ \
					if(buildable != BA_H_REACTOR && buildable != BA_H_REPEATER) \
					reason = IBE_NOPOWERHERE; \
				} \
				*/ \
				\
				/*this buildable requires a DCC */ \
				/* \
				if(BG_Buildable(buildable)->dccTest && !G_IsDCCBuilt()) \
				reason = IBE_NODCC; \
				*/ \
				\
				/*check that there is a parent reactor when building a repeater */ \
				/* \
				if(buildable == BA_H_REPEATER) \
				{ \
					tempent = G_FindBuildable(BA_H_REACTOR); \
					\
					if(tempent == NULL) /8 No reactor 8/ \
					reason = IBE_RPTNOREAC; \
					/8      else if(g_markDeconstruct.integer && G_IsPowered(entity_origin) == BA_H_REACTOR && !G_OC_NoMarkDeconstruct()) \
							reason = IBE_RPTPOWERHERE;8/ \
					else if(!g_markDeconstruct.integer && G_RepeaterEntityForPoint(entity_origin) && !G_OC_NoMarkDeconstruct()) \
					reason = IBE_RPTPOWERHERE; \
				} \
				*/ \
				\
				/* Check permission to build here */ \
				if(tr1.surfaceFlags & SURF_NOHUMANBUILD || contents & CONTENTS_NOHUMANBUILD) \
				reason = IBE_PERMISSION; \
			} \
		} \
 \
 /* \
 if(buildable == BA_A_HOVEL) \
				{ \
					vec3_t    builderMins, builderMaxs; \
					\
					/8this assumes the adv builder is the biggest thing that'll use the hovel 8/ \
					BG_ClassBoundingBox(PCL_ALIEN_BUILDER0_UPG, builderMins, builderMaxs, NULL, NULL, NULL); \
					\
					if(APropHovel_Blocked(origin, angles, normal, ent)) \
					reason = IBE_HOVELEXIT; \
				}	*/ \
 \
		/* Check permission to build here */ \
		if(!override)  /* && */ \
		if(tr1.surfaceFlags & SURF_NOBUILD || contents & CONTENTS_NOBUILD) \
			reason = IBE_PERMISSION; \
 \
		/* Can we only have one of these? */ \
		/* in OC mode, multiple buildables of any type should always be available */ \
		/* \
		if(BG_Buildable(buildable)->uniqueTest) \
		{ \
			tempent = G_FindBuildable(buildable); \
			if(tempent && !tempent->deconstruct && !(tempent->s.eFlags & EF_DEAD)) \
			{ \
				switch(buildable) \
				{ \
					case BA_A_OVERMIND: \
						reason = IBE_ONEOVERMIND; \
						break; \
 \
					case BA_A_HOVEL: \
						reason = IBE_ONEHOVEL; \
						break; \
 \
					case BA_H_REACTOR: \
						reason = IBE_ONEREACTOR; \
						break; \
 \
					default: \
						Com_Error(ERR_FATAL, "No reason for denying build of %d\n", buildable); \
						break; \
				} \
			} \
		} \
		*/ \
 \
		/* always enough BP in OC mode */ \
		/* \
		if((tempReason = G_SufficientBPAvailable(buildable, origin)) != IBE_NONE) \
			reason = tempReason; \
		*/ \
 \
		/* Relink buildables */ \
			G_SetBuildableLinkState(qtrue); \
 \
		/*check there is enough room to spawn from (presuming this is a spawn) */ \
		/* even if they do have buildable override, players should always be able to spawn from it */ \
		if(reason == IBE_NONE) \
		{ \
			G_SetBuildableMarkedLinkState(qfalse); \
			if(G_CheckSpawnPoint(ENTITYNUM_NONE, origin, normal, buildable, NULL) != NULL) \
				reason = IBE_NORMAL; \
			G_SetBuildableMarkedLinkState(qtrue); \
		} \
 \
		/*this item does not fit here */ \
		/* */ \
		if(!override) \
		{ \
			if(reason == IBE_NONE && (tr2.fraction < 1.0 || tr3.fraction < 1.0)) \
				reason = IBE_NOROOM; \
 \
		} \
		if(reason != IBE_NONE) \
			level.numBuildablesForRemoval = 0; \
 \
		return reason; \
	} \
	while(0)

	#define G_OC_DefaultAlienPowered() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!self->spawned) \
			break; \
 \
		if(self->health <= 0) \
			break; \
 \
		switch(self->groupID) \
		{ \
			default: \
			case 0: \
				self->powered = G_FindProvider(self, qfalse); \
				break; \
 \
			case 1: \
				self->powered = 1; \
				break; \
 \
			case 2: \
				self->powered = 0; \
				break; \
		} \
	} while(0)

	#define G_OC_ReactorPowered() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!self->spawned) \
			break; \
 \
		if(self->health <= 0) \
			break; \
 \
		switch(self->groupID) \
		{ \
			default: \
			case 0: \
				self->powered = qtrue; \
				break; \
 \
			case 1: \
				self->powered = 1; \
				break; \
 \
			case 2: \
				self->powered = 0; \
				break; \
		} \
	} while(0)

	#define G_OC_DefaultHumanPowered() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!self->spawned) \
			break; \
 \
		if(self->health <= 0) \
			break; \
 \
		switch(self->groupID) \
		{ \
			default: \
			case 0: \
				self->powered = G_FindProvider(self, qfalse); \
				break; \
 \
			case 1: \
				self->powered = 1; \
				break; \
 \
			case 2: \
				self->powered = 0; \
				break; \
		} \
	} while(0)

	#define G_OC_NeedRepeaterBlast() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_Frame() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* process scrim warmup and prep */ \
 \
		/* if a warmup is taking place, let everybody on a scrim team know */ \
		/* and see if it's time to start the scrim */ \
		if(level.ocScrimState == G_OC_STATE_WARM) \
		{ \
			G_ClientCP(NULL, va("OC scrim starts in %d.\n\nUse ^5teamchat^7 to plan your play with scrim teammates!", ((level.ocStartTime + g_ocWarmup.integer * 1000) - level.time) / 1000), "OC scrim starts in ", CLIENT_ONLYTEAM); \
			if(level.time > level.ocStartTime + g_ocWarmup.integer * 1000) \
			{ \
				/* restart every client and begin */ \
 \
				for(i = 0; i < level.maxclients; i++) \
				{ \
					ent = &g_entities[i]; \
					if(ent && ent->client && ent->client->pers.scrimTeam) \
					{ \
						G_OC_RestartClient(ent, 0, 0); \
					} \
				} \
 \
				level.ocScrimState = G_OC_STATE_PLAY; \
			} \
		} \
 \
		/* if the scrim has barely started, see if it needs to be started */ \
		/* immediately or if a warmup needs to start */ \
		if(level.ocScrimState == G_OC_STATE_PREP) \
		{ \
			if(g_ocWarmup.integer && !G_OC_IsSingleScrim()) \
			{ \
				level.ocScrimState = G_OC_STATE_WARM; \
			} \
			else \
			{ \
				/* restart every client and begin */ \
 \
				for(i = 0; i < level.maxclients; i++) \
				{ \
					ent = &g_entities[i]; \
					G_OC_RestartClient(ent, 0, 0); \
				} \
 \
				level.ocScrimState = G_OC_STATE_PLAY; \
			} \
		} \
 \
		/* see if we need to load the layout */ \
		if(level.ocLoadTime && level.time > level.ocLoadTime) \
		{ \
			G_OC_LayoutLoad(NULL); \
			level.ocLoadTime = level.ocLoadDelayedByTrigger = 0; \
			/* removal of the Waiting for stage triggers CP in case it exists */ \
			G_ClientCP(NULL, "^7", "^l^o^a^d^i^n^g", 0); \
		} \
		/* see if it hasn't loaded yet, but it's going to take longer than */ \
		/* usual because doors and everything else need to open */ \
		else if(level.ocLoadDelayedByTrigger && level.ocLoadTime) \
		{ \
			G_ClientCP(NULL, va("Waiting for map's stage triggers to finish..%s^l^o^a^d^i^n^g\n%d seconds remain", ((level.time / 1000 % 2 == 0) ? (".") : ("")), SECS(level.ocLoadTime - level.time)), "^l^o^a^d^i^n^g", 0); \
		} \
 \
		/* if the end times of a scrim are known, either end it or alert */ \
		/* everybody that's it's going to end */ \
		if(level.scrimEndTime) \
		{ \
			if(level.time >= level.scrimEndTime) \
			{ \
				G_OC_EndScrim(); \
			} \
			else \
			{ \
				if(G_OC_HasScrimFinished()) \
				{ \
					/* immediately remove the alert message */ \
					G_ClientCP(NULL, "^7", "crim ends in", 0); \
				} \
				else \
				{ \
					/* alert */ \
					G_ClientCP(NULL, va("OC Scrim ends in ^2%d^7!", (level.scrimEndTime - level.time) / 1000), "crim ends in", 0); \
				} \
			} \
		} \
	} while(0)

	#define G_OC_NeedLevelCheck() (BG_OC_OCMode() ? ((level.time % 6000 == 0) ? (1) : (0)) : (1))  // save some processing power

	#define G_OC_LevelChecks()
	#define G_OC_LevelChecks_() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
	} while(0)

	#define G_OC_CheckpointSpawnCheck() ((BG_OC_OCMode()) ? (spawn == BA_A_BOOSTER ? (1) : (0)) : (0))

	void G_OC_RestartClient(gentity_t *ent, int quick, int resetScrimTeam);

	#define G_OC_BarricadeShrink() \
	do \
	{ \
		if(BG_OC_OCMode()) \
			return; \
	} while(0)

	#define G_OC_NeedUnshrink() ((BG_OC_OCMode()) ? (1) : (0))
//	#define G_OC_NeedUnshrink() 0  // barricades are never shrunk in OC mode
#if 1
	#define G_OC_Unshrink() return
#else
	#define G_OC_Unshrink() \
	do \
	{ \
		int anim; \
 \
		shrink = qfalse; \
		self->shrunkTime = 0; \
 \
		/* unshrink animation, IDLE2 has been hijacked for this */ \
		anim = self->s.legsAnim & ~(ANIM_FORCEBIT | ANIM_TOGGLEBIT); \
		G_SetIdleBuildableAnim(self, BG_Buildable(BA_A_BARRICADE)->idleAnim); \
		G_SetBuildableAnim(self, BANIM_ATTACK2, qtrue); \
 \
		/* a change in size requires a relink */ \
		if(self->spawned) \
			trap_LinkEntity(self); \
		return; \
	} while(0)
#endif

	#define G_OC_BONUS_BUILDABLE_THINK() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!self->powered && !self->verifyUnpowered) \
		{ \
			G_OC_BuildableDestroyed(self); \
			self->verifyUnpowered = qtrue; \
		} \
		if(self->powered && self->verifyUnpowered)  /* buildable has repowered */ \
		{ \
			G_OC_BuildableBuilt(self); \
			self->verifyUnpowered = qfalse; \
		} \
	} while(0)

	#define G_OC_NoMarkDeconstruct() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_Teleport() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_NeedNoDestroyLastSpawn() (!(BG_OC_OCMode()))

	#define G_OC_BUILDABLEBUILD() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(built->s.modelindex == BA_H_SPAWN || built->s.modelindex == BA_A_SPAWN) \
			built->groupID = 0;  /* default - TODO: ... */ \
		else \
			built->groupID = 2;  /* UNPOWERED - TODO: ... */ \
	} while(0)

	#define G_OC_NeedStartSolid() (!(BG_OC_OCMode()))  // OC buildables can be in and on other stuff

	#define G_OC_BUILDABLE_STRUCT_DEFS \
	qboolean verifyUnpowered;

	#define G_OC_RESTARTOC_TIME 9000  // time after a restartoc before a checkpoint can be used
	#define G_OC_NOBONUSMESSAGE "Cannot use bonuses while using\na jetpack, lucifer cannon,\nflamer, battlesuit, or\ngrenade\n(did you use a grenade on yourself?)"
	#define G_OC_NOBONUSJETPACKMESSAGE "Cannot use bonuses while using\na jetpack"
	#define G_OC_NOBONUSLCANNONMESSAGE "Cannot use bonuses while holding\na lucifer cannon"
	#define G_OC_NOBONUSFLAMERMESSAGE  "Cannot use bonuses while holding\na flamer"
	#define G_OC_NOBATTLESUITMESSAGE  "Cannot use bonuses while wearing\na battlesuit"
	#define G_OC_NOBONUSGRENADEMESSAGE "Cannot use bonuses after using\na grenade on yourself\n(restart the course to regain the ability\nto use bonuses)"
	#define G_OC_EVOLVEBLOCK_TIME 2000

	#define G_OC_HumanNameForWeapon(x) (BG_Weapon((x)) ? (BG_Weapon((x))->humanName) : ("NULL"))
	#define G_OC_TeamForWeapon(x) (BG_Weapon((x)) ? (BG_Weapon((x))->team) : (0))

	#define G_OC_NoSuddenDeath() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_NoBuildTimer() ((BG_OC_OCMode()) ? ((G_admin_canEditOC(ent)) ? (1) : (0)) : (0))

	void G_OC_UseMedi(gentity_t *ent, gentity_t *medi);
	void G_OC_SyncMedis(gentity_t **medis, int len);
	void G_OC_MergeMedis(gentity_t **dst, gentity_t **src);
	void G_OC_AppendMedi(gentity_t **medis, gentity_t *medi);
	void G_OC_RemoveMedi(gentity_t **medis, gentity_t *medi);
	int  G_OC_AllMedis(gentity_t **medis);
	int  G_OC_NumberOfMedis(gentity_t **medis);
	int  G_OC_HasMediBeenUsed(gentity_t *medi, gentity_t **medis);
	void G_OC_ClearMedis(gentity_t **medis);
	void G_OC_UseArm(gentity_t *ent, gentity_t *arm);
	void G_OC_SyncArms(gentity_t **arms, int len);
	void G_OC_MergeArms(gentity_t **dst, gentity_t **src);
	void G_OC_AppendArm(gentity_t **arms, gentity_t *arm);
	void G_OC_RemoveArm(gentity_t **arms, gentity_t *arm);
	int  G_OC_AllArms(gentity_t **arms);
	int  G_OC_NumberOfArms(gentity_t **arms);
	int  G_OC_HasArmBeenUsed(gentity_t *arm, gentity_t **arms);
	void G_OC_ClearArms(gentity_t **arms);
	void G_OC_Checkpoint(gentity_t *checkpoint, gentity_t *ent);
	void G_OC_PlayerSpawn(gentity_t *ent);
	void G_OC_PlayerDie(gentity_t *ent);
	int  G_OC_PlayerCredits(gentity_t *ent);
	int  G_OC_PlayerCreditsSingle(gentity_t *ent);
	int  G_OC_CanUseBonus(gentity_t *ent);
//	int  G_OC_WeaponIsReserved(weapon_t weapon);
	int  G_OC_WeaponIsReserved(int weapon);
	int  G_OC_WeaponRemoveReserved(gentity_t *ent);
	int  G_OC_IsSingleScrim(void);
	void G_OC_EndScrim(void);
//  int  G_OC_ValidScrimWeapon(weapon_t weapon);
	int  G_OC_ValidScrimWeapon(int weapon);
	void G_OC_JoinPlayerToScrimTeam(gentity_t *ent, gentity_t *reportEnt, char *teamName, char *weaponName);
	int  G_OC_ValidScrimTeamName(char *name);
	void G_OC_RemovePlayerFromScrimTeam(gentity_t *ent);
	int  G_OC_EmptyScrim(void);
	int  G_OC_NumScrimTeams(void);
	int  G_OC_TooFewScrimTeams(void);
	int  G_OC_HasScrimFinished(void);
	int  G_OC_NumberOfTeams(void);
	#define G_OC_BUILDABLEBUILT(x) G_OC_BuildableBuilt(x)
	void G_OC_BuildableBuilt(gentity_t *ent);
	#define G_OC_BUILDABLEDESTROYED(x) G_OC_BuildableDestroyed(x)
	void G_OC_BuildableDestroyed(gentity_t *ent);

	void G_OC_Lol(gentity_t *ent);

	#define G_OC_NeedSuicide() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeverForceTeamBalance() ((BG_OC_OCMode()) ? (1) : (0))

	//<+===============================================+>
	// buildable optimization
	//<+===============================================+>

	#define G_OC_AlienBuildableOptimizedThinkTime() (BG_OC_OCMode() ? ((BG_OC_Aliens(level.layout) && !BG_OC_Humans(level.layout)) ? (G_OC_OPTIMIZED_BUILDABLE_THINK_OFFSET) : (0)) : (0))  // if it's an alien only OC, certain thinks can have more latency
	#define G_OC_HumanBuildableOptimizedThinkTime() (BG_OC_OCMode() ? ((BG_OC_Humans(level.layout) && !BG_OC_Aliens(level.layout)) ? (G_OC_OPTIMIZED_BUILDABLE_THINK_OFFSET) : (0)) : (0))  // if it's a human only OC, certain thinks can have more latency

	#define G_OC_OPTIMIZED_BUILDABLE_THINK_OFFSET +4000  // save some processing power for tubes, hovels, barricades, trappers, turrets, teslas, DC's, and armouries

	//<+===============================================+>
	// game times
	//<+===============================================+>

	#define G_OC_NeedEndGameTimelimit() ((BG_OC_OCMode()) ? ((level.numConnectedClients) ? (0) : (1)) : (1))
	#define G_OC_NeedEndGameTeamWin() (!(BG_OC_OCMode()))

	//<+===============================================+>
	// cmds
	//<+===============================================+>

	#define G_OC_TeamChange() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(G_admin_canEditOC(ent)) \
			break; \
 \
		if(team == TEAM_HUMANS && !BG_OC_Humans(level.layout)) \
		{ \
			G_ClientPrint(ent, "Humans cannot join this course", CLIENT_NULL); \
			return; \
		} \
 \
		if(team == TEAM_ALIENS && !BG_OC_Aliens(level.layout)) \
		{ \
			G_ClientPrint(ent, "Aliens cannon join this course", CLIENT_NULL); \
			return; \
		} \
	} while(0)

	#define G_OC_CallvotePercentage() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!g_timelimit.integer) \
			break; \
 \
		percentAddition -= (((float) g_timelimitDrop.value) * ((float) (((int) ((int) (level.time - level.startTime) / (int) 60000) / (int) g_timelimit.integer)))); \
	} while(0)

	#define G_OC_NamedVoteMatches() \
	|| !Q_stricmp(vote, "hide") \
	|| !Q_stricmp(vote, "unhide")

	#define G_OC_NamedVoteNegativeMatches()

	#define G_OC_NeedAlternateMapVote() (1)
	#define G_OC_NeedAlternateNextMapVote() (1)
	#define G_OC_NeedAlternateMapRestartVote() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeedAlternateDrawVote() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NoRestartVote() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_AlternateNextMapVote() \
	do \
	{ \
		BG_StrToLower( arg2 ); \
 \
		if( strstr( g_unvotableMaps.string, arg ) && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) ) \
		{ \
		  trap_SendServerCommand ( ent - g_entities, va( "print \"%s: " \
				"server disabled voting for map '%s'\n\"", cmd, arg ) ); \
 \
		  return; \
		} \
 \
		if( level.numNextVotes == 0 && G_MapExists( g_nextMap.string ) ) \
		{ \
			trap_SendServerCommand( ent - g_entities, va( "print \"%s: " \
						"the next map is already set to '%s^7'\n\"", cmd, g_nextMap.string ) ); \
			return; \
		} \
 \
		if( !G_MapExists( arg ) ) \
		{ \
			trap_SendServerCommand( ent - g_entities, va( "print \"%s: " \
						"'maps/%s^7.bsp' could not be found on the server\n\"", arg, cmd ) ); \
			return; \
		} \
 \
		if( arg2[ 0 ] ) \
		{ \
			if( strstr( g_unvotableLayouts.string, arg2 ) && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) ) \
			{ \
				trap_SendServerCommand ( ent - g_entities, va( "print \"%s: " \
							"server disabled voting for layout '%s'\n\"", cmd, arg2 ) ); \
 \
				return; \
			} \
 \
			if( strcmp( arg2, "*BUILTIN*" ) != 0 && !trap_FS_FOpenFile( va( "layouts/%s/%s.dat", arg, arg2 ), NULL, FS_READ ) ) \
			{ \
				trap_SendServerCommand( ent - g_entities, va( "print \"%s: " \
							"'layouts/%s/%s.dat' could not be found on the server\n\"", cmd, arg, arg2 ) ); \
				return; \
			} \
		} \
 \
		if(g_ocOnly.integer > 0) \
		{ \
			if(!arg2[0]) \
				Q_strncpyz(arg2, "oc", sizeof(arg2)); \
 \
			if(arg2[0] != 'o' || arg2[1] != 'c') \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
					"'%s^7' is not an obstacle course\n\"", cmd, ((arg2[0]) ? (arg2) : ("(empty)")))); \
				return; \
			} \
		} \
		else if(g_ocOnly.integer < 0) \
		{ \
			if(arg2[0] == 'o' || arg2[1] == 'c') \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
					"'%s^7' is an obstacle course\n\"", cmd, ((arg2[0]) ? (arg2) : ("(empty)")))); \
				return; \
			} \
		} \
 \
        if( arg2[ 0 ] ) \
        { \
          Com_sprintf( level.voteString[ team ], sizeof( level.voteString ), \
            "set g_nextMap \"%s\"; set g_layouts \"%s\"", arg, arg2 ); \
 \
          Com_sprintf( level.voteDisplayString[ team ], sizeof( level.voteDisplayString[ team ] ), "Set the next map to '%s' with layout '%s'", arg, arg2 ); \
        } \
        else \
        { \
          Com_sprintf( level.voteString[ team ], sizeof( level.voteString ), \
            "set g_nextMap \"%s\"", arg ); \
 \
          Com_sprintf( level.voteDisplayString[ team ], sizeof( level.voteDisplayString[ team ] ), "Set the next map to '%s'", arg ); \
        } \
 \
        --level.numNextVotes; \
		level.voteNextMap = qtrue; \
	} while(0)

	#define G_OC_AlternateMapVote() \
	do \
	{ \
		BG_StrToLower(arg2); \
 \
		if(BG_OC_OCMode() && level.ocScrimState > G_OC_STATE_NONE && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: you cannot call for a map change during a scrim\n\"", cmd)); \
			return; \
		} \
 \
		if(strstr(g_unvotableMaps.string, arg) && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
		  trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
				"server disabled voting for map '%s'\n\"", cmd, arg)); \
 \
		  return; \
		} \
 \
		if(!trap_FS_FOpenFile(va("maps/%s.bsp", arg), NULL, FS_READ)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
				"'maps/%s.bsp' could not be found on the server\n\"", cmd, arg)); \
			return; \
		} \
 \
		if(g_ocOnly.integer > 0) \
		{ \
			if(!arg2[0]) \
				Q_strncpyz(arg2, "oc", sizeof(arg2)); \
 \
			if(arg2[0] != 'o' || arg2[1] != 'c') \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
					"'%s^7' is not an obstacle course\n\"", cmd, ((arg2[0]) ? (arg2) : ("(empty)")))); \
				return; \
			} \
		} \
		else if(g_ocOnly.integer < 0) \
		{ \
			if(arg2[0] == 'o' || arg2[1] == 'c') \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
					"'%s^7' is an obstacle course\n\"", cmd, ((arg2[0]) ? (arg2) : ("(empty)")))); \
				return; \
			} \
		} \
 \
		if(!G_MapExists(arg)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
				"'maps/%s.bsp' could not be found on the server\n\"", cmd, arg)); \
			return; \
		} \
 \
		if(arg2[0]) \
		{ \
			if( strstr( g_unvotableLayouts.string, arg2 ) && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) ) \
			{ \
				trap_SendServerCommand ( ent - g_entities, va( "print \"%s: " \
							"server disabled voting for layout '%s'\n\"", cmd, arg2 ) ); \
 \
				return; \
			} \
 \
			if(strcmp( arg2, "*BUILTIN*" ) != 0 && !trap_FS_FOpenFile(va("layouts/%s/%s.dat", arg, arg2), NULL, FS_READ)) \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"%s: " \
					"'layouts/%s/%s.dat' could not be found on the server\n\"", cmd, arg, arg2)); \
				return; \
			} \
		} \
 \
		if(arg2[0]) \
		{ \
			const char *options = BG_OC_ParseLayoutFlags(arg2); \
			level.voteThreshold[team] = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "changemap %s %s", arg, arg2); \
			Com_sprintf(level.voteDisplayString[team], \
			sizeof(level.voteDisplayString[team]), "Change to map '%s^7' using layout '%s^7'%s%s%s", arg, arg2, (*options) ? (" (layout uses flags '") : (""), options, (*options) ? ("')") : ("")); \
		} \
		else \
		{ \
			level.voteThreshold[team] = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "%s %s", vote, arg); \
			Com_sprintf(level.voteDisplayString[team], \
				sizeof(level.voteDisplayString[team]), "Change to map '%s^7'", arg); \
		} \
 \
		level.voteNextMap = qfalse; \
	} while(0)

	#define G_OC_AlternateMapRestartVote() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		BG_StrToLower(level.layout); \
 \
		if(BG_OC_OCMode() && level.ocScrimState > G_OC_STATE_NONE && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: you cannot call for a mapchange during a scrim\n\"", cmd)); \
			return; \
		} \
 \
		if(*level.layout) \
		{ \
			level.voteThreshold[team] = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "restart %s", level.layout); \
			Com_sprintf(level.voteDisplayString[team], \
				sizeof(level.voteDisplayString[team]), "Restart current map using layout '%s^7'", level.layout); \
		} \
		else \
		{ \
			level.voteThreshold[team] = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "%s", vote); \
			Com_sprintf(level.voteDisplayString[team], \
				sizeof(level.voteDisplayString[team]), "Restart current map"); \
		} \
 \
		level.voteNextMap = qfalse; \
	} while(0)

	#define G_OC_AlternateDrawVote() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		BG_StrToLower(level.layout); \
 \
		if(BG_OC_OCMode() && level.ocScrimState > G_OC_STATE_NONE && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: you cannot call for a mapchange during a scrim\n\"", cmd)); \
			return; \
		} \
 \
		level.voteThreshold[team] = g_mapVotePercent.value + percentAddition; \
		Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "evacuation"); \
		Com_sprintf(level.voteDisplayString[team], sizeof(level.voteDisplayString[team]), \
			"End match in a draw"); \
 \
		level.voteNextMap = qfalse; \
	} while(0)

	#define G_OC_OTHERVOTECOMMANDS \
	else if(!Q_stricmp(vote, "hide") && BG_OC_OCMode()) \
	{ \
		if(!g_allowHideVote.integer) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: hide votes are disabled\n\"", cmd)); \
			return; \
		} \
 \
		if(level.clients[clientNum].pers.hidden) \
		{ \
			trap_SendServerCommand(ent - g_entities, \
				va("print \"%s: player is already hidden\n\"", cmd)); \
			return; \
		} \
 \
		if(level.clients[clientNum].pers.hiddenTime) \
		{ \
			trap_SendServerCommand(ent - g_entities, \
				va("print \"%s: player is force hidden\n\"", cmd)); \
			return; \
		} \
 \
		if(G_admin_permission(&g_entities[clientNum], ADMF_IMMUNITY)) \
		{ \
			trap_SendServerCommand(ent - g_entities, \
				va("print \"%s: admin is immune from vote hide\n\"", cmd)); \
			return; \
		} \
 \
		Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), \
			"adminhide %d %s", clientNum, g_voteHideDuration.string); \
		Com_sprintf(level.voteDisplayString[team], sizeof(level.voteDisplayString[team]), \
			"Hide player '%s'", name); \
 \
		level.voteNextMap = qfalse; \
	} \
	else if(!Q_stricmp(vote, "unhide") && BG_OC_OCMode()) \
	{ \
		if(!g_allowUnhideVote.integer) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"%s: unhide votes are disabled\n\"", cmd)); \
			return; \
		} \
 \
		if(!level.clients[clientNum].pers.hidden) \
		{ \
			trap_SendServerCommand(ent - g_entities, \
				va("print \"%s: player is not currently hidden\n\"", cmd)); \
			return; \
		} \
 \
		if(level.clients[clientNum].pers.hiddenTime) \
		{ \
			trap_SendServerCommand(ent - g_entities, \
				va("print \"%s: player is force unhidden\n\"", cmd)); \
			return; \
		} \
 \
		if(G_admin_permission(&g_entities[clientNum], ADMF_IMMUNITY)) \
		{ \
			trap_SendServerCommand(ent - g_entities, \
				va("print \"%s: admin is immune from vote unhide\n\"", cmd)); \
			return; \
		} \
 \
		Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), \
			"unhide %d %s", clientNum, g_voteUnhideDuration.string); \
		Com_sprintf(level.voteDisplayString[team], sizeof(level.voteDisplayString[team]), \
			"Un-Hide player \'%s\'", name); \
 \
		level.voteNextMap = qfalse; \
	} \
	else if(!Q_stricmp(vote, "startscrim") && BG_OC_OCMode()) \
	{ \
		if(arg[0] != 'a' && arg[0] != 'm') \
		{ \
			G_ClientPrint(ent, va("%s: usage /%s startscrim [a/m]", cmd, cmd), CLIENT_NULL); \
			return; \
		} \
 \
		if(level.ocScrimState > G_OC_STATE_NONE) \
		{ \
			G_ClientPrint(ent, va("%s: a scrim is already taking place", cmd), CLIENT_NULL); \
			return; \
		} \
 \
		if(G_OC_EmptyScrim()) \
		{ \
			G_ClientPrint(ent, va("%s: empty is the scrim", cmd), CLIENT_NULL); \
			return; \
		} \
 \
		if(G_OC_TooFewScrimTeams()) \
		{ \
			G_ClientPrint(ent, va("%s: too few teams exist to start a scrim", cmd), CLIENT_NULL); \
			return; \
		} \
 \
		if(!ent->client->pers.scrimTeam) \
		{ \
			G_ClientPrint(ent, va("%s: you need to be on a scrim team to do this", cmd), CLIENT_NULL); \
			return; \
		} \
 \
		level.voteThreshold[team] = g_startScrimVotePercent.value; \
		Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "startscrim %c", arg[0]); \
		Com_sprintf(level.voteDisplayString[team], sizeof(level.voteDisplayString[team]), \
			"Start a '%s^7' scrim^7", arg[0] == 'm' ? "^1medi^7" : "^2armoury^7"); \
 \
		level.voteNextMap = qfalse; \
	} \
	else if(!Q_stricmp(vote, "endscrim") && BG_OC_OCMode()) \
	{ \
		if(level.ocScrimState <= G_OC_STATE_NONE) \
		{ \
			G_ClientPrint(ent, va("%s: no scrim is currently taking place", cmd), CLIENT_NULL); \
			return; \
		} \
 \
		level.voteThreshold[team] = g_endScrimVotePercent.value; \
		Com_sprintf(level.voteString[team], sizeof(level.voteString[team]), "endscrim"); \
		Com_sprintf(level.voteDisplayString[team], sizeof(level.voteDisplayString[team]), "End the scrim"); \
 \
		level.voteNextMap = qfalse; \
	}

	#define G_OC_OtherCommandDescription() ((BG_OC_OCMode()) ? (", hide, unhide, startscrim, endscrim") : (""))

	#define G_OC_AllowSuddenDeathVote() (!(BG_OC_OCMode()))

	#define G_OC_VoteCheck() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent->client->pers.scrimTeam && strstr(level.voteString[team], "startscrim")) \
		{ \
			G_ClientPrint(ent, "You need to be on a scrim team to do this", CLIENT_NULL); \
			return; \
		} \
	} while(0)

	#define G_OC_CheckTeamVote() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(BG_OC_OCMode()) \
		{ \
			G_ClientPrint(ent, "Team votes are disabled during an obstacle course", CLIENT_NULL); \
			return; \
		} \
	} while(0)

	#define G_OC_NeverHumanNearby() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeverOvermindAbsent() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeverRemoveCredits() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_BuildableStageAlwaysValid() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeedAlternateStageTest() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_AlternateStageTest() ((S3))  // maximum stage

	#define G_OC_Class() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(G_admin_canEditOC(ent)) \
			break; \
 \
		ent->client->pers.needEvolve = 1; \
		ent->client->pers.evolveTime = level.time + G_OC_EVOLVEBLOCK_TIME;  /* warning: funny things _might_ happen if clients do not spawn immediately, but they should */ \
	} while(0)

	#define G_OC_CanActivateItem() ((BG_OC_OCMode()) ? ((G_admin_canEditOC(ent)) ? (1) : (0)) : (0))  // can use or activate or reload weapon

	#define G_OC_ClassChange() \
	(\
		(BG_OC_OCMode() && !G_admin_canEditOC(ent)) \
		? \
		(\
			/* can only evolve in OC mode if the client has full health */ \
			(ent->client->ps.stats[STAT_HEALTH] >= BG_Class(currentClass)->health) \
			? \
			(\
				/* check for evolve spam */ \
				(level.time >= ent->client->pers.nextValidEvolveTime && newClass != currentClass) \
				? \
				(\
					(ent->client->pers.nextValidEvolveTime = level.time + 2500), \
					(\
						/* check if the class is invalid for the OC */ \
						(newClass == PCL_ALIEN_BUILDER0 && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AGRANGER)) || \
						(newClass == PCL_ALIEN_BUILDER0_UPG && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AGRANGERUPG)) || \
						(newClass == PCL_ALIEN_LEVEL0 && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ADRETCH)) || \
						(newClass == PCL_ALIEN_LEVEL1 && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ABASILISK)) || \
						(newClass == PCL_ALIEN_LEVEL1_UPG && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ABASILISKUPG)) || \
						(newClass == PCL_ALIEN_LEVEL2 && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AMARAUDER)) || \
						(newClass == PCL_ALIEN_LEVEL2_UPG && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AMARAUDERUPG)) || \
						(newClass == PCL_ALIEN_LEVEL3 && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ADRAGOON)) || \
						(newClass == PCL_ALIEN_LEVEL3_UPG && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ADRAGOONUPG)) || \
						(newClass == PCL_ALIEN_LEVEL4 && !BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ATYRANT)) \
					) \
					? \
					(\
						/* not a valid class for the OC */ \
						(0) \
					) \
					: \
					(\
						/* can evolve */ \
						(1) \
					) \
				) \
				: \
				(\
					(0) \
				) \
			) \
			: \
			(\
				(0) \
			) \
		) \
		: \
		(\
			/* not OC mode, or client can edit; no OC limits */ \
			(1) \
		) \
	)

	#define G_OC_ArmouryUsed(x, y) \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!(x)) \
			break; \
 \
		if(!(y)) \
			break; \
 \
		G_OC_UseArm((x), (y)); \
	} while(0)

	#define G_OC_CheckWeaponPurchase() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(G_admin_canEditOC(ent)) \
			break; \
 \
		/* some weapons are OK */ \
		if(weapon == WP_NONE || weapon == WP_BLASTER || weapon ==  WP_MACHINEGUN) \
			break; \
 \
		if(!G_BuildableRange(ent->client->ps.origin, 100, BA_H_ARMOURY)) \
			return; \
 \
		if(!G_admin_canEditOC(ent)) \
		{ \
			if(ent->client->pers.scrimTeam) \
			{ \
				/* see if the team can buy equipment yet */ \
				g_oc_scrimTeam_t *t; \
				G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam); \
 \
				if(!(t->flags & G_OC_SCRIMFLAG_EQUIPMENT)) \
				{ \
					G_ClientPrint(ent, "You cannot buy equipment for the OC yet", CLIENT_NULL); \
					return; \
				} \
			} \
			else \
			{ \
				/* make sure the client has won */ \
				if((!G_OC_AllArms(ent->client->pers.arms) || (BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ONEARM) && G_OC_NumberOfArms(ent->client->pers.arms))) && !ent->client->pers.override) \
				{ \
					G_ClientPrint(ent, "You cannot buy equipment for the OC yet", CLIENT_NULL); \
					return; \
				} \
 \
				/* is the weapon reserved for a scrim team? */ \
				if(G_OC_WeaponIsReserved(weapon) && !ent->client->pers.override && !G_admin_canEditOC(ent)) \
				{ \
					G_ClientPrint(ent, "This item is reserved for the oc scrim", CLIENT_SPECTATORS); \
					return; \
				} \
			} \
		} \
	} while(0)

	#define G_OC_MediUsed(x, y) \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!(x)) \
			break; \
 \
		if(!(y)) \
			break; \
 \
		G_OC_UseMedi((x), (y)); \
	} while(0)

	#define G_OC_ArmouryUsed(x, y) \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!(x)) \
			break; \
 \
		if(!(y)) \
			break; \
 \
		G_OC_UseArm((x), (y)); \
	} while(0)

	#define G_OC_CheckUpgradePurchase() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(G_admin_canEditOC(ent)) \
			break; \
 \
		/* some upgrades are OK */ \
		if(upgrade == UP_AMMO) \
			break; \
 \
		if(!G_BuildableRange(ent->client->ps.origin, 100, BA_H_ARMOURY)) \
			return; \
 \
		if(!G_admin_canEditOC(ent)) \
		{ \
			if(ent->client->pers.scrimTeam) \
			{ \
				/* see if the team can buy equipment yet */ \
				g_oc_scrimTeam_t *t; \
				G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam); \
 \
				if(!(t->flags & G_OC_SCRIMFLAG_EQUIPMENT)) \
				{ \
					G_ClientPrint(ent, "You cannot buy equipment for the OC yet", CLIENT_NULL); \
					return; \
				} \
			} \
			else \
			{ \
				/* make sure the client has won */ \
 \
				if(!G_OC_AllArms(ent->client->pers.arms)) \
				{ \
					G_ClientPrint(ent, "You cannot buy equipment for the OC yet", CLIENT_NULL); \
					return; \
				} \
			} \
		} \
	} while(0)

	void G_OC_PlayerMaxAmmo(gentity_t *ent);
	void G_OC_PlayerMaxClips(gentity_t *ent);
	void G_OC_PlayerMaxCash(gentity_t *ent);
	void G_OC_PlayerMaxHealth(gentity_t *ent);

	#define G_OC_NeverUnpowerJetpack() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_CanBuildableBeUsedOnOtherTeam() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_CanUseBuildableInArea() (!(BG_OC_OCMode()))

	#define G_OC_NeedFreeCash() (!(BG_OC_OCMode()))

	#define G_OC_SpectatorThink() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		client->ps.persistant[PERS_OCTIMER] = (unsigned int) (((unsigned int) client->pers.aliveTime) & ((unsigned int) 0x0000FFFF));  /* only 16 bits are transmitted */ \
		client->ps.persistant[PERS_OCTIMER + 1] = client->pers.aliveTime >> 16; \
	} while(0)

	#define G_OC_ClientThink() \
	do \
	{ \
		int i; \
 \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(g_forceHide.integer) \
		{ \
			if(!ent->client->pers.hidden) \
			{ \
				ent->client->pers.hidden = !ent->client->pers.hidden; \
				G_StopFromFollowing(ent, 0); \
				ent->r.svFlags |= SVF_SINGLECLIENT; \
				ent->r.singleClient = ent - g_entities; \
				G_ClientPrint(ent, "You have been hidden (g_forceHide set on server)\n", CLIENT_SPECTATORS); \
			} \
		} \
 \
		if(client->ps.stats[STAT_HEALTH] > 0 && client->sess.spectatorState == SPECTATOR_NOT) /* increment the timer if the player is alive and not spectating */ \
		{ \
			client->pers.aliveTime += trap_Milliseconds() - client->pers.lastAliveTime; \
			client->pers.lastAliveTime = trap_Milliseconds(); \
		} \
 \
		if(client->pers.scrimTeam) \
		{ \
			g_oc_scrimTeam_t *t; \
			G_OC_GETTEAM(t, level.scrimTeam, client->pers.scrimTeam); \
 \
			if(level.ocScrimState >= G_OC_STATE_PLAY) \
			{ \
				client->ps.persistant[PERS_OCTIMER]     = (unsigned int) ((unsigned int) ((unsigned int) G_OC_SCRIMTIME) & 0x0000FFFF); \
				client->ps.persistant[PERS_OCTIMER + 1] = (unsigned int) ((unsigned int) ((unsigned int) G_OC_SCRIMTIME) >> ((unsigned int) 16)); \
			} \
			else \
			{ \
				client->ps.persistant[PERS_OCTIMER]     = 0; \
				client->ps.persistant[PERS_OCTIMER + 1] = 0; \
			} \
		} \
		else \
		{ \
			client->ps.persistant[PERS_OCTIMER]     = (unsigned int) ((unsigned int) ((unsigned int) client->pers.aliveTime) & 0x0000FFFF); \
			client->ps.persistant[PERS_OCTIMER + 1] = (unsigned int) ((unsigned int) ((unsigned int) client->pers.aliveTime) >> ((unsigned int) 16)); \
		} \
 \
		if(client->pers.needEvolve) \
		{ \
			class_t currentClass = client->pers.classSelection; \
			class_t newClass; \
			vec3_t infestOrigin; \
 \
			if(level.time >= client->pers.evolveTime) \
			{ \
				G_ClientCP(ent, "Don't evolve block!", NULL, CLIENT_SPECTATORS); \
				G_Damage(ent, NULL, NULL, NULL, NULL, INFINITE, 0,  MOD_TRIGGER_HURT); \
				client->pers.needEvolve = client->pers.evolveTime = 0; \
			} \
 \
			/* default granger */ \
			newClass =  PCL_ALIEN_BUILDER0; \
 \
			/* stop wallwalking */ \
			pm.ps->persistant[PERS_STATE] &= ~PS_WALLCLIMBINGFOLLOW; \
			pm.ps->eFlags &= ~(EF_WALLCLIMBCEILING | EF_WALLCLIMB); \
			pm.ps->stats[STAT_STATE] &= ~SS_WALLCLIMBING; \
			if(pm.cmd.upmove < 0) \
				pm.cmd.upmove = 0; \
 \
			/* find the first class to which to evolve */ \
			if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AGRANGER)) \
				newClass = PCL_ALIEN_BUILDER0; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AGRANGERUPG)) \
				newClass = PCL_ALIEN_BUILDER0_UPG; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ADRETCH)) \
				newClass = PCL_ALIEN_LEVEL0; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ABASILISK)) \
				newClass = PCL_ALIEN_LEVEL1; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ABASILISKUPG)) \
				newClass = PCL_ALIEN_LEVEL1_UPG; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AMARAUDER)) \
				newClass = PCL_ALIEN_LEVEL2; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_AMARAUDERUPG)) \
				newClass = PCL_ALIEN_LEVEL2_UPG; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ADRAGOON)) \
				newClass = PCL_ALIEN_LEVEL3; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ADRAGOONUPG)) \
				newClass = PCL_ALIEN_LEVEL3_UPG; \
			else if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ATYRANT)) \
				newClass = PCL_ALIEN_LEVEL4; \
 \
			/* further unnecessary checks for wallwaking and some other checks */ \
			if(!(client->ps.stats[STAT_STATE] & SS_WALLCLIMBING) && client->pers.teamSelection == TEAM_ALIENS && !(client->ps.stats[STAT_STATE] & SS_HOVELING) && client->ps.stats[STAT_HEALTH] > 0 && G_RoomForClassChange(ent, newClass, infestOrigin)) \
			{ \
				if(currentClass != newClass) \
				{ \
					client->pers.evolveHealthFraction = (((float) ent->client->ps.stats[STAT_HEALTH]) / ((float) BG_Class(currentClass)->health)); \
 \
					if(client->pers.evolveHealthFraction < 0.0f) \
						client->pers.evolveHealthFraction = 0.0f; \
					if(client->pers.evolveHealthFraction > 1.0f) \
						client->pers.evolveHealthFraction = 1.0f; \
	 \
					client->pers.classSelection = newClass; \
					ClientUserinfoChanged(ent - g_entities, qfalse); \
					VectorCopy(infestOrigin, ent->s.pos.trBase); \
					ClientSpawn(ent, ent, ent->s.pos.trBase, ent->s.apos.trBase); \
				} \
 \
				/* client is already a valid class */ \
				client->pers.needEvolve = client->pers.evolveTime = 0; \
			} \
		} \
 \
		if(client->pers.scrimTeam) \
		{ \
			if(client->pers.classSelection == PCL_ALIEN_LEVEL0) \
			{ \
				pm.autoWeaponHit[client->ps.weapon] = CheckVenomAttack(ent); \
			} \
			else if(client->pers.classSelection == PCL_ALIEN_LEVEL1) \
			{ \
				CheckGrabAttack(ent); \
			} \
			else if(client->pers.classSelection == PCL_ALIEN_LEVEL1_UPG) \
			{ \
				CheckGrabAttack(ent); \
			} \
			else if(client->pers.classSelection == PCL_ALIEN_LEVEL3) \
			{ \
				if(client->ps.weaponTime <= 0) \
					pm.autoWeaponHit[client->ps.weapon] = CheckPounceAttack(ent); \
			} \
			else if(client->pers.classSelection == PCL_ALIEN_LEVEL3_UPG) \
			{ \
				if(client->ps.weaponTime <= 0) \
					pm.autoWeaponHit[client->ps.weapon] = CheckPounceAttack(ent); \
			} \
		} \
 \
		if(client->ps.stats[STAT_HEALTH] <= 0) \
			break; \
 \
		if(level.time >= client->pers.nextWeaponCheckTime && !client->pers.override && !G_admin_canEditOC(ent)) \
		{ \
			g_oc_scrimTeam_t *t; \
 \
			client->pers.nextWeaponCheckTime += G_OC_FRAMETIMEWEAPON; \
			if(client->pers.nextWeaponCheckTime < level.time - G_OC_FRAMETIMEWEAPON * 10)  /* only up to 10 frames behind */ \
				client->pers.nextWeaponCheckTime = level.time + G_OC_FRAMETIMEWEAPON; \
			if(client->pers.scrimTeam && level.ocScrimState >= G_OC_STATE_WARM) \
			{ \
				G_OC_GETTEAM(t, level.scrimTeam, client->pers.scrimTeam); \
 \
				/* always give scrim players max ammo */ \
				G_OC_PlayerMaxAmmo(ent); \
 \
				/* remove all non-team weapons */ \
				for(i = WP_NONE; i < WP_NUM_WEAPONS; i++) \
				{ \
					if(i != t->weapon && client->ps.stats[STAT_WEAPON] == i) \
					{ \
						client->ps.stats[STAT_WEAPON] = WP_NONE; \
						G_ForceWeaponChange(ent, WP_NONE); \
					} \
				} \
 \
				/* make sure player has scrim weapons */ \
				if(client->ps.stats[STAT_WEAPON] != t->weapon) \
				{ \
					client->ps.stats[STAT_WEAPON] = t->weapon; \
					G_ForceWeaponChange(ent, t->weapon); \
				} \
			} \
			else if(!client->pers.scrimTeam) \
			{ \
				G_OC_WeaponRemoveReserved(ent); \
				G_OC_PlayerMaxClips(ent);  /* infinite clips in OC mode */ \
				if(client->pers.teamSelection == TEAM_HUMANS) \
				{ \
					if(!client->pers.arms || !(\
						(client->pers.scrimTeam) \
						? \
						(level.scrimTeam[client->pers.scrimTeam].flags & G_OC_SCRIMFLAG_EQUIPMENT) \
						: \
						(G_OC_AllArms(client->pers.arms) || (BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_ONEARM) && G_OC_NumberOfArms(client->pers.arms))) \
					))  /* messy mess of tests if client can buy equipment */ \
					{ \
						int commonWeapon = WP_MACHINEGUN; \
 \
						if(BG_OC_TestLayoutFlag(level.layout, BG_OC_OCFLAG_LUCIJUMP)) \
						{ \
							commonWeapon = WP_LUCIFER_CANNON; \
						} \
 \
						if(client->ps.stats[STAT_WEAPON] != commonWeapon) \
						{ \
							client->ps.stats[STAT_WEAPON] = commonWeapon; \
							G_ForceWeaponChange(ent, commonWeapon); \
						} \
					} \
				} \
			} \
 \
			if(client->ps.stats[STAT_WEAPON] == WP_HBUILD) \
			{ \
				client->ps.stats[STAT_WEAPON] = WP_NONE; \
				G_ForceWeaponChange(ent, WP_NONE); \
			} \
		} \
 \
		/* hacky method of automatically enabling and disabling override */ \
		if(level.time >= client->pers.nextOverrideCheckTime) \
		{ \
			client->pers.nextOverrideCheckTime += G_OC_FRAMETIMEOVERRIDE; \
			if(client->pers.nextOverrideCheckTime < level.time - G_OC_FRAMETIMEOVERRIDE * 10)  /* only up to 10 frames behind (so that it doesn't continously increment) */ \
			{ \
				client->pers.nextOverrideCheckTime = level.time + G_OC_FRAMETIMEOVERRIDE; \
			} \
 \
			if(!client->pers.noAuO && !g_cheats.integer) \
			{ \
				if(G_admin_canEditOC(ent)) \
				{ \
					client->pers.override = 1; \
				} \
				else \
				{ \
					if(client->noclip) \
					{ \
						G_ClientPrint(ent, "noclip OFF", CLIENT_NULL); \
					} \
					client->noclip = 0; \
 \
					if(client->speed) \
					{ \
						G_ClientPrint(ent, "speedmode OFF", CLIENT_NULL); \
					} \
					client->speed = 0; \
 \
					if(ent->flags & FL_GODMODE) \
					{ \
						G_ClientPrint(ent, "godmode OFF", CLIENT_NULL); \
					} \
					ent->flags &= FL_GODMODE; \
 \
					if(ent->flags & FL_NOTARGET) \
					{ \
						G_ClientPrint(ent, "notarget OFF", CLIENT_NULL); \
					} \
					ent->flags &= FL_NOTARGET; \
 \
					client->pers.override = 0; \
				} \
			} \
		} \
	} while(0)

	#define G_OC_ClientCanBuild(x) ((BG_OC_OCMode()) ? ((G_admin_canEditOC((x))) ? (1) : (0)) : (1))
	#define G_OC_ClientCannotBuild(x) G_ClientPrint((x), "You cannot build in an obstacle course", CLIENT_SPECTATORS)

	void Cmd_BuildableOverride_f(gentity_t *ent);
	void Cmd_Stats_f(gentity_t *ent);
	void Cmd_Mystats_f(gentity_t *ent);
	void Cmd_Spawnup_f(gentity_t *ent);
	void Cmd_Spawndown_f(gentity_t *ent);
	void Cmd_Spawn_f(gentity_t *ent);
	void Cmd_Groupup_f(gentity_t *ent);
	void Cmd_Groupdown_f(gentity_t *ent);
	void Cmd_Group_f(gentity_t *ent);
	void Cmd_RestartOC_f(gentity_t *ent);
	void Cmd_TeleportToCheckpoint_f(gentity_t *ent);
	void Cmd_LeaveScrim_f(gentity_t *ent);
	void Cmd_JoinScrim_f(gentity_t *ent);
	void Cmd_ListScrim_f(gentity_t *ent);
	void Cmd_Hide_f(gentity_t *ent);
	void Cmd_TestHidden_f(gentity_t *ent);
	void Cmd_QuickRestartOC_f(gentity_t *ent);
	void Cmd_AskLayout_f(gentity_t *ent);
	void Cmd_AutoAngle_f(gentity_t *ent);
	void Cmd_AutoUnAngle_f(gentity_t *ent);

	#define G_OC_CMDS \
	{"groupUp", CMD_TEAM | CMD_LIVING, Cmd_Groupup_f}, \
		{"upGroup", CMD_TEAM | CMD_LIVING, Cmd_Groupup_f}, \
	{"groupDown", CMD_TEAM | CMD_LIVING, Cmd_Groupdown_f}, \
		{"downGroup", CMD_TEAM | CMD_LIVING, Cmd_Groupdown_f}, \
	{"group", CMD_TEAM | CMD_LIVING, Cmd_Group_f}, \
		{"groupDisplay", CMD_TEAM | CMD_LIVING, Cmd_Group_f}, \
		{"displayGroup", CMD_TEAM | CMD_LIVING, Cmd_Group_f}, \
		{"groupShow", CMD_TEAM | CMD_LIVING, Cmd_Group_f}, \
		{"showGroup", CMD_TEAM | CMD_LIVING, Cmd_Group_f}, \
	{"myStats", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Mystats_f}, \
	{"stats", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"statistics", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"hiScores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"hi-Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"hi_Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"highScores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"high-Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"high_Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
	{"buildableOverride", 0, Cmd_BuildableOverride_f}, \
		{"buildOverride", 0, Cmd_BuildableOverride_f}, \
		{"overrideBuildable", 0, Cmd_BuildableOverride_f}, \
		{"overrideBuild", 0, Cmd_BuildableOverride_f}, \
		{"canBuild", 0, Cmd_BuildableOverride_f}, \
		{"bo", 0, Cmd_BuildableOverride_f}, \
		{"ob", 0, Cmd_BuildableOverride_f}, \
	{"leaveScrim", CMD_MESSAGE, Cmd_LeaveScrim_f}, \
		{"leaveScrimTeam", CMD_MESSAGE, Cmd_LeaveScrim_f}, \
		{"ScrimTeamLeave", CMD_MESSAGE, Cmd_LeaveScrim_f}, \
	{"listScrim", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"listScrims", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimList", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimsList", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"listScrimTeams", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimTeamList", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimsTeamList", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimTeamsList", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimListTeams", CMD_MESSAGE, Cmd_ListScrim_f}, \
		{"scrimListTeam", CMD_MESSAGE, Cmd_ListScrim_f}, \
	{"joinScrim", CMD_MESSAGE, Cmd_JoinScrim_f}, \
		{"joinScrimTeam", CMD_MESSAGE, Cmd_JoinScrim_f}, \
		{"scrimTeamJoin", CMD_MESSAGE, Cmd_JoinScrim_f}, \
	{"restartOC", CMD_MESSAGE | CMD_TEAM, Cmd_RestartOC_f}, \
		{"oldRestartOC", CMD_MESSAGE | CMD_TEAM, Cmd_RestartOC_f}, \
		{"restartOCOld", CMD_MESSAGE | CMD_TEAM, Cmd_RestartOC_f}, \
		{"restartOldOC", CMD_MESSAGE | CMD_TEAM, Cmd_RestartOC_f}, \
	{"newRestartOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"realRestartOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"restartOCReal", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"restartRealOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"quickRestartOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"restartOCQuick", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"quicklyRestartOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"restartOCQuickly", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"restartQuickOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
		{"restartQuicklyOC", CMD_MESSAGE | CMD_TEAM | CMD_LIVING, Cmd_QuickRestartOC_f}, \
	{"hide", CMD_MESSAGE, Cmd_Hide_f}, \
	{"testHidden", 0, Cmd_TestHidden_f}, \
		{"isHidden", 0, Cmd_TestHidden_f}, \
		{"playerIsHidden", 0, Cmd_TestHidden_f}, \
		{"isPlayerHidden", 0, Cmd_TestHidden_f}, \
		{"hidden", 0, Cmd_TestHidden_f}, \
		{"hiddenTest", 0, Cmd_TestHidden_f}, \
		{"hiddenPlayer", 0, Cmd_TestHidden_f}, \
	{"teleportToCheckpoint", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleboost", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleCheckpoint", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleCheck", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleportCheck", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleportCheckpoint", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleToCheckpoint", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleToCheck", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleportToCheck", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleportToBooster", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
		{"teleportToBoost", CMD_TEAM | CMD_LIVING, Cmd_TeleportToCheckpoint_f }, \
	{"enableAutoAngle", 0, Cmd_AutoAngle_f}, \
		{"autoAngleEnable", 0, Cmd_AutoAngle_f}, \
	{"disableAutoAngle", 0, Cmd_AutoUnAngle_f}, \
		{"autoAngleDisable", 0, Cmd_AutoUnAngle_f}, \
	{"askLayout", 0, Cmd_AskLayout_f}, \
		{"askForLayout", 0, Cmd_AskLayout_f}, \
		{"layoutInfo", 0, Cmd_AskLayout_f}, \
		{"layoutInformation", 0, Cmd_AskLayout_f}, \
		{"getLayoutInfo", 0, Cmd_AskLayout_f}, \
		{"getLayoutInformation", 0, Cmd_AskLayout_f}, \
		{"askForLayoutInfo", 0, Cmd_AskLayout_f}, \
		{"askForLayoutInformation", 0, Cmd_AskLayout_f}, \
		{"askLayoutInfo", 0, Cmd_AskLayout_f}, \
		{"askLayoutInformation", 0, Cmd_AskLayout_f}, \
		{"layoutOptions", 0, Cmd_AskLayout_f}, \
		{"getLayoutOptions", 0, Cmd_AskLayout_f}, \
	{"lol", CMD_MESSAGE, G_OC_Lol},

	#define G_OC_PTRCRestore() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
break;  /* TODO: the current ptrc for oc data causes memory corruption and doesn't work.  Needs to be rewritten */ \
 \
		if(connection->hasCheated) \
			break; \
 \
		if(ent->client->pers.hasCheated) \
			break; \
 \
		if(ent->client->pers.scrimTeam) \
			break; \
 \
		ent->client->pers.checkpoint = connection->checkpoint; \
		ent->client->pers.lastAliveTime = connection->lastAliveTime; \
		ent->client->pers.aliveTime = connection->aliveTime; \
		ent->client->pers.hasCheated = connection->hasCheated; \
		if(level.totalMedistations && connection->totalMedistations && connection->medisLastCheckpoint && ent->client->pers.medisLastCheckpoint && ent->client->pers.medis) \
		{ \
			gentity_t **tmp; \
 \
			if(level.totalMedistations > connection->totalMedistations) \
			{ \
				tmp = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *)); \
				memcpy(tmp, connection->medisLastCheckpoint, connection->totalMedistations + 1); \
				G_OC_SyncMedis(tmp, level.totalMedistations); \
			} \
			else \
			{ \
				tmp = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *)); \
				memcpy(tmp, connection->medisLastCheckpoint, connection->totalMedistations + 1); \
				G_OC_SyncMedis(tmp, connection->totalMedistations); \
			} \
 \
			if(tmp[level.totalMedistations])  /* check to make sure that they've been synced right */ \
			{ \
				BG_Free(tmp); \
				G_ClientPrint(ent, "^1Error restoring PTRC", CLIENT_SPECTATORS); \
				return; \
			} \
 \
			memcpy(ent->client->pers.medisLastCheckpoint, tmp, (level.totalMedistations + 1) * sizeof(gentity_t *)); \
			BG_Free(tmp); \
			memcpy(ent->client->pers.medis, ent->client->pers.medisLastCheckpoint, (level.totalMedistations + 1) * sizeof(gentity_t *));  /* all saved medis for the last checkpoint also belong in medis */ \
		} \
		if(level.totalArmouries && connection->totalArmouries && connection->armsLastCheckpoint && ent->client->pers.armsLastCheckpoint && ent->client->pers.arms) \
		{ \
			gentity_t **tmp; \
 \
			if(level.totalArmouries > connection->totalArmouries) \
			{ \
				tmp = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *)); \
				memcpy(tmp, connection->armsLastCheckpoint, connection->totalArmouries + 1); \
				G_OC_SyncArms(tmp, level.totalArmouries); \
			} \
			else \
			{ \
				tmp = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *)); \
				memcpy(tmp, connection->armsLastCheckpoint, connection->totalArmouries + 1); \
				G_OC_SyncArms(tmp, connection->totalArmouries); \
			} \
 \
			if(tmp[level.totalArmouries])  /* check to make sure that they've been synced right */ \
			{ \
				BG_Free(tmp); \
				G_ClientPrint(ent, "^1Error restoring PTRC", CLIENT_SPECTATORS); \
				return; \
			} \
 \
			memcpy(ent->client->pers.armsLastCheckpoint, tmp, (level.totalArmouries + 1) * sizeof(gentity_t *)); \
			BG_Free(tmp); \
			memcpy(ent->client->pers.arms, ent->client->pers.armsLastCheckpoint, (level.totalArmouries + 1) * sizeof(gentity_t *));  /* all saved arms for the last checkpoint also belong in arms */ \
		} \
	} while(0)
#endif /* ifdef GAME */

//<+===============================================+><+===============================================+>
// cgame only stuff
//<+===============================================+><+===============================================+>

#ifdef CGAME
	/* TODO: move some toggles such as hide to a client-side cvar */

	#define CG_OC_OCTIMER ((unsigned int) ((unsigned int) ((unsigned int) ((unsigned int) cg.snap->ps.persistant[PERS_OCTIMER]) & (unsigned int) 0x0000FFFF) | ((unsigned int) (((unsigned int) cg.snap->ps.persistant[PERS_OCTIMER + 1]) << 16))))  /* only 16 bits are sent */

	#define CG_OC_PLAYERTIMER (va("%dm:%ds:%3dms", MINS(CG_OC_OCTIMER), SECS(CG_OC_OCTIMER), MSEC(CG_OC_OCTIMER)))
	#define CG_OC_PLAYERTIMERPRINT (va("^t^i^m^e%dm:%ds:%3dms", MINS(CG_OC_OCTIMER), SECS(CG_OC_OCTIMER), MSEC(CG_OC_OCTIMER)))

	#define CG_OC_CanSetPlayerTimer() ((BG_OC_OCMode()) ? (1) : (0))

	#define CG_OC_OCNameOtherTeams() ((BG_OC_OCMode()) ? (1) : (0))

	#define CG_OC_Frame() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(cg_printTimer.integer) \
		{ \
			/* timer */ \
 \
			CG_CenterPrint(CG_OC_PLAYERTIMERPRINT, "^t^i^m^e", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH); \
		} \
 \
		if(cg_printSpeedometer.integer) \
		{ \
			/* speedometer */ \
 \
			float tmp = cg.snap->ps.velocity[2]; \
 \
			CG_CenterPrint(va("^2XYZ: %d^7ups", (int) VectorLength(cg.snap->ps.velocity)), "XYZ:", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH); \
 \
			cg.snap->ps.velocity[2] = 0.0f; \
			CG_CenterPrint(va("^2XY: %d^7ups", (int) VectorLength(cg.snap->ps.velocity)), "XY:", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH); \
			cg.snap->ps.velocity[2] = tmp; \
		} \
	} while(0)

	#define CG_OC_STATICANGLE 0.3f
	#define CG_OC_STATICLEN 15.0f

	#define CG_OC_PositionBuildable() \
	do \
	{ \
		if(!(BG_OC_OCMode())) \
			break; \
 \
		if(ghost) \
			break; \
 \
		/*if((BG_Buildable(buildable)->minNormal >= CG_OC_STATICANGLE) || (sqrt(DotProduct(angles, angles)) <= CG_OC_STATICLEN))*/ \
		if(sqrt(DotProduct(angles, angles)) <= CG_OC_STATICLEN) \
			VectorCopy(inOrigin, outOrigin); \
 \
		return; \
	} while(0)

	#define BG_OC_BuildablePositionMissed \
	do \
	{ \
		if(!(BG_OC_OCMode())) \
			break; \
 \
		VectorCopy(inOrigin, outOrigin); \
 \
		return; \
	} while(0)

	#define CG_OC_CVARS \
	vmCvar_t cg_printTimer; \
	vmCvar_t cg_printSpeedometer;

	#define CG_OC_DCVARS \
	{ &cg_printTimer, "cg_printTimer", "0", CVAR_ARCHIVE }, \
	{ &cg_printSpeedometer, "cg_printSpeedometer", "0", CVAR_ARCHIVE },

	#define CG_OC_CONFIGSTRINGMODIFIED \
	else if(num == CS_OCMODE) \
	{ \
		switch(atoi(str)) \
		{ \
			case 0: \
				BG_OC_SetOCModeNone(); \
				break; \
 \
			default: \
				BG_OC_SetOCModeOC(); \
				break; \
		} \
	} \
	else if(num == CS_NOWALLWALK) \
	{ \
		BG_OC_SetNoWallWalk(atoi(str)); \
	}

	#define CG_OC_SetConfigStrings() \
	do \
	{ \
		static char buf[1024], buf2[1024]; \
 \
		switch(atoi(CG_ConfigString(CS_OCMODE))) \
		{ \
			case 0: \
				BG_OC_SetOCModeNone(); \
				break; \
 \
			default: \
				BG_OC_SetOCModeOC(); \
				break; \
		} \
 \
		BG_OC_SetNoWallWalk(atoi(CG_ConfigString(CS_NOWALLWALK))); \
 \
		strncpy(buf, CG_ConfigString(CS_LAYOUT), sizeof(buf)); \
		strncpy(buf2, BG_OC_ParseLayoutFlags(buf), sizeof(buf2)); \
		strncpy(buf, "Current Layout Options: ", sizeof(buf)); \
		if(buf2[0]) \
			Q_strcat(buf, sizeof(buf), buf2); \
		else \
			Q_strcat(buf, sizeof(buf), "(none)"); \
		trap_Cvar_Set("ui_layoutOptions", buf); \
	} while(0)
	#define CG_OC_ECVARS \
	extern vmCvar_t cg_printTimer; \
	extern vmCvar_t cg_printSpeedometer;

	#define CG_OC_SERVERCMDS \
	/*, { "command", function}*/

	/*void function(void);*/
#endif /* ifdef CGAME */

//<+===============================================+><+===============================================+>
// game or cgame
//<+===============================================+><+===============================================+>

#if defined(CGAME) || defined(GAME) || defined(OC_BGAME)
	//<+===============================================+>
	// OC modes
	//<+===============================================+>

	void BG_OC_SetOCModeNone(void);
	void BG_OC_SetOCModeOC(void);
	int  BG_OC_GetOCMode(void);

	const char *BG_OC_ParseLayoutFlags(char *layout);
	/*
	qboolean   BG_OC_TestLayoutFlag(char *layout, const char *flag);
	qboolean   BG_OC_LayoutExtraFlags(char *layout);
	qboolean   BG_OC_Aliens(char *layout);
	qboolean   BG_OC_Humans(char *layout);
	*/
	/* qboolean isn't typedef'd here yet */
	int        BG_OC_TestLayoutFlag(char *layout, const char *flag);
	int        BG_OC_LayoutExtraFlags(char *layout);
	int        BG_OC_Aliens(char *layout);
	int        BG_OC_Humans(char *layout);

	#define BG_OC_CS
	#define CS_SCRIMTEAMS 28
	#define CS_LAYOUT     30
	#define CS_OCMODE     31
	#define CS_NOWALLWALK 32

	//<+===============================================+>
	// OC flags
	//<+===============================================+>

	// these need to be lowercase
	#define BG_OC_OCFLAG_ONEARM            "o"
	#define BG_OC_OCFLAG_NOCREEP           "c"
	#define BG_OC_OCFLAG_HUMANS            "h"
	#define BG_OC_OCFLAG_NOWALLWALK        "w"
	#define BG_OC_OCFLAG_AGRANGER          "g"
	#define BG_OC_OCFLAG_AGRANGERUPG       "^g"
	#define BG_OC_OCFLAG_ADRETCH           "d"
	#define BG_OC_OCFLAG_ABASILISK         "b"
	#define BG_OC_OCFLAG_ABASILISKUPG      "^b"
	#define BG_OC_OCFLAG_AMARAUDER         "m"
	#define BG_OC_OCFLAG_AMARAUDERUPG      "^m"
	#define BG_OC_OCFLAG_ADRAGOON          "r"
	#define BG_OC_OCFLAG_ADRAGOONUPG       "^r"
	#define BG_OC_OCFLAG_ATYRANT           "t"
	#define BG_OC_OCFLAG_LUCIJUMP          "l"

	// oc flag names
	#define BG_OC_OCFLAG_ONEARM_NAME       "Only one armoury needs to be used"
	#define BG_OC_OCFLAG_NOCREEP_NAME      "No creep"
	#define BG_OC_OCFLAG_HUMANS_NAME       "Humans"
	#define BG_OC_OCFLAG_NOWALLWALK_NAME   "No wall walking"
	#define BG_OC_OCFLAG_AGRANGER_NAME     "Grangers"
	#define BG_OC_OCFLAG_AGRANGERUPG_NAME  "Advanced grangers"
	#define BG_OC_OCFLAG_ADRETCH_NAME      "Dretches"
	#define BG_OC_OCFLAG_ABASILISK_NAME    "Basilisks"
	#define BG_OC_OCFLAG_ABASILISKUPG_NAME "Advanced basilisks"
	#define BG_OC_OCFLAG_AMARAUDER_NAME    "Marauders"
	#define BG_OC_OCFLAG_AMARAUDERUPG_NAME "Advanced marauders"
	#define BG_OC_OCFLAG_ADRAGOON_NAME     "Dragoons"
	#define BG_OC_OCFLAG_ADRAGOONUPG_NAME  "Advanced dragoons"
	#define BG_OC_OCFLAG_ATYRANT_NAME      "Tyrants"
	#define BG_OC_OCFLAG_LUCIJUMP_NAME     "Lucifer cannon mode"

	//<+===============================================+>
	// game and balance stuff
	//<+===============================================+>

	#define BG_OC_PERS \
	,PERS_OCTIMER  // netcode now has room for 4 more

	#define S4 3
	#define BG_OC_HOVELSTAGE S4

	#define BG_OC_NeedBuildableAppend() BG_OC_OCMode()
	#define BG_OC_NeedClassAppend() BG_OC_OCMode()
	#define BG_OC_NeedWeaponAppend() BG_OC_OCMode()
	#define BG_OC_BuildableAppend() Q_strcat(buf, sizeof(buf), "/oc");
	#define BG_OC_ClassAppend() Q_strcat(buf, sizeof(buf), "/oc");
	#define BG_OC_WeaponAppend() Q_strcat(buf, sizeof(buf), "/oc");

	#define BG_OC_PLAYERMASK ((BG_OC_OCMode()) ? (MASK_PLAYERSOLID | CONTENTS_CORPSE) : (MASK_PLAYERSOLID))  // use this instead of MASK_PLAYERSOLID when traces should always hit players.  In OC mode, MASK_PLAYERSOLID by itself will not interact with clients since their contents is CONTENTS_CORPSE.
	#define BG_OC_SHOTMASK ((BG_OC_OCMode()) ? (MASK_SHOT | CONTENTS_CORPSE) : (MASK_SHOT))  // same as above, but for shots
	#define BG_OC_CLIENTCONTENTS ((BG_OC_OCMode()) ? (CONTENTS_CORPSE) : (CONTENTS_BODY))

	#define BG_OC_BUILDABLEPOSITIONMASK ((BG_OC_OCMode()) ? (MASK_PLAYERSOLID) : (MASK_DEADSOLID))

	//<+===============================================+>
	// pmove
	//<+===============================================+>

	#define BG_OC_PMNeedCrashLand() ((BG_OC_OCMode()) ? (1) : (0))
	#define BG_OC_PMOCDodge() ((BG_OC_OCMode()) ? (1) : (0))
	#define BG_OC_PMOCWallJump() (((BG_OC_OCMode()) && (BG_Class(pm->ps->stats[STAT_CLASS])->abilities & SCA_OLDWALLJUMPER)) ? (1) : (0))
	#define BG_OC_PMOCGroundTraceWallJump() (((BG_OC_OCMode()) && (BG_Class(pm->ps->stats[STAT_CLASS])->abilities & SCA_OLDWALLJUMPER)) ? (1) : (0))
	#define BG_OC_PMOCPounce() (0)

	#define BG_OC_PMNoDodge() BG_OC_OCMode()

	#define BG_OC_PMCrashLand() \
	{ \
		/* ducking while falling doubles damage */ \
		if(pm->ps->pm_flags & PMF_DUCKED) \
		{ \
			delta *= 2; \
		} \
	}

	#define BG_OC_PMCheckDodge() \
	{ \
		return qfalse; \
	}

	#define BG_OC_PMCheckWallJump() return qfalse;
	#define BG_OC_PMCheckWallJump_() \
	{ \
		if(!(BG_Class(pm->ps->stats[STAT_CLASS])->abilities & SCA_WALLJUMPER)) \
			return qfalse; \
 \
		if(pm->ps->pm_flags & PMF_RESPAWNED) \
			return qfalse;		/* don't allow jump until all buttons are up */ \
 \
		if(pm->cmd.upmove < 10) \
			/* not holding jump */ \
			return qfalse; \
 \
		if(pm->ps->pm_flags & PMF_TIME_WALLJUMP) \
			return qfalse; \
 \
		/* must wait for jump to be released */ \
		if(pm->ps->pm_flags & PMF_JUMP_HELD && \
				pm->ps->grapplePoint[2] == 1.0f) \
		{ \
			/* clear upmove so cmdscale doesn't lower running speed */ \
			pm->cmd.upmove = 0; \
			return qfalse; \
		} \
 \
		pm->ps->pm_flags |= PMF_TIME_WALLJUMP; \
		pm->ps->pm_time = 200; \
 \
		pml.groundPlane = qfalse;	 /* jumping away */ \
		pml.walking = qfalse; \
		pm->ps->pm_flags |= PMF_JUMP_HELD; \
 \
		pm->ps->groundEntityNum = ENTITYNUM_NONE; \
 \
		ProjectPointOnPlane(forward, pml.forward, pm->ps->grapplePoint); \
		ProjectPointOnPlane(right, pml.right, pm->ps->grapplePoint); \
 \
		VectorScale(pm->ps->grapplePoint, normalFraction, dir); \
 \
		if(pm->cmd.forwardmove > 0) \
			VectorMA(dir, cmdFraction, forward, dir); \
		else if(pm->cmd.forwardmove < 0) \
			VectorMA(dir, -cmdFraction, forward, dir); \
 \
		if(pm->cmd.rightmove > 0) \
			VectorMA(dir, cmdFraction, right, dir); \
		else if(pm->cmd.rightmove < 0) \
			VectorMA(dir, -cmdFraction, right, dir); \
 \
		VectorMA(dir, upFraction, refNormal, dir); \
		VectorNormalize(dir); \
 \
		VectorMA(pm->ps->velocity, BG_Class(pm->ps->stats[STAT_CLASS])->jumpMagnitude, \
							dir, pm->ps->velocity); \
 \
		/* for a long run of wall jumps the velocity can get pretty large, this caps it */ \
		if(VectorLength(pm->ps->velocity) > LEVEL2_WALLJUMP_MAXSPEED) \
		{ \
			VectorNormalize(pm->ps->velocity); \
			VectorScale(pm->ps->velocity, LEVEL2_WALLJUMP_MAXSPEED, pm->ps->velocity); \
		} \
 \
		PM_AddEvent(EV_JUMP); \
 \
		if(pm->cmd.forwardmove >= 0) \
		{ \
			if(!(pm->ps->persistant[PERS_STATE] & PS_NONSEGMODEL)) \
				PM_ForceLegsAnim(LEGS_JUMP); \
			else \
				PM_ForceLegsAnim(NSPA_JUMP); \
 \
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; \
		} \
		else \
		{ \
			if(!(pm->ps->persistant[PERS_STATE] & PS_NONSEGMODEL)) \
				PM_ForceLegsAnim(LEGS_JUMPB); \
			else \
				PM_ForceLegsAnim(NSPA_JUMPBACK); \
 \
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP; \
		} \
 \
		return qtrue; \
	}

	#define BG_OC_PMGroundTraceWallJump() \
	{ \
		vec3_t movedir; \
 \
		if(BG_ClassHasAbility(pm->ps->stats[STAT_CLASS], SCA_WALLJUMPER)) \
		{ \
			ProjectPointOnPlane(movedir, pml.forward, refNormal); \
			VectorNormalize(movedir); \
 \
			if(pm->cmd.forwardmove < 0) \
				VectorNegate(movedir, movedir); \
 \
			/* allow strafe transitions */ \
			if(pm->cmd.rightmove) \
			{ \
				VectorCopy(pml.right, movedir); \
 \
				if(pm->cmd.rightmove < 0) \
					VectorNegate(movedir, movedir); \
			} \
 \
			/* trace into direction we are moving */ \
			VectorMA(pm->ps->origin, 0.25f, movedir, point); \
			/*pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);*/ \
			pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask & ~CONTENTS_BODY & ~CONTENTS_CORPSE); \
 \
			/* if(trace.fraction < 1.0f && !(trace.surfaceFlags & (SURF_SKY | SURF_SLICK)) && */ \
			/*     (trace.entityNum == ENTITYNUM_WORLD)) */ \
			if(trace.fraction < 1.0f && \
				!(trace.surfaceFlags & (SURF_SKY | SURF_SLICK))) \
			{ \
				if(!VectorCompare(trace.plane.normal, pm->ps->grapplePoint)) \
				{ \
					VectorCopy(trace.plane.normal, pm->ps->grapplePoint); \
					/* PM_CheckWallJump(); */ \
 \
					/* a return-friendly version of BG_OC_PMCheckWallJump() */ \
					{ \
						vec3_t  dir, forward, right; \
						vec3_t  refNormal = { 0.0f, 0.0f, 1.0f }; \
						float   normalFraction = 1.5f; \
						float   cmdFraction = 1.0f; \
						float   upFraction = 1.5f; \
 \
						if(!(BG_Class(pm->ps->stats[STAT_CLASS])->abilities & SCA_WALLJUMPER)) \
							return; \
 \
						if(pm->ps->pm_flags & PMF_RESPAWNED) \
							return;		/* don't allow jump until all buttons are up */ \
 \
						if(pm->cmd.upmove < 10) \
							/* not holding jump */ \
							return; \
 \
						if(pm->ps->pm_flags & PMF_TIME_WALLJUMP) \
							return; \
 \
						/* must wait for jump to be released */ \
						if(pm->ps->pm_flags & PMF_JUMP_HELD && \
								pm->ps->grapplePoint[2] == 1.0f) \
						{ \
							/* clear upmove so cmdscale doesn't lower running speed */ \
							pm->cmd.upmove = 0; \
							return; \
						} \
 \
						pm->ps->pm_flags |= PMF_TIME_WALLJUMP; \
						pm->ps->pm_time = 200; \
 \
						pml.groundPlane = qfalse;	 /* jumping away */ \
						pml.walking = qfalse; \
						pm->ps->pm_flags |= PMF_JUMP_HELD; \
 \
						pm->ps->groundEntityNum = ENTITYNUM_NONE; \
 \
						ProjectPointOnPlane(forward, pml.forward, pm->ps->grapplePoint); \
						ProjectPointOnPlane(right, pml.right, pm->ps->grapplePoint); \
 \
						VectorScale(pm->ps->grapplePoint, normalFraction, dir); \
 \
						if(pm->cmd.forwardmove > 0) \
							VectorMA(dir, cmdFraction, forward, dir); \
						else if(pm->cmd.forwardmove < 0) \
							VectorMA(dir, -cmdFraction, forward, dir); \
 \
						if(pm->cmd.rightmove > 0) \
							VectorMA(dir, cmdFraction, right, dir); \
						else if(pm->cmd.rightmove < 0) \
							VectorMA(dir, -cmdFraction, right, dir); \
 \
						VectorMA(dir, upFraction, refNormal, dir); \
						VectorNormalize(dir); \
 \
						VectorMA(pm->ps->velocity, BG_Class(pm->ps->stats[STAT_CLASS])->jumpMagnitude, \
											dir, pm->ps->velocity); \
 \
						/* for a long run of wall jumps the velocity can get pretty large, this caps it */ \
						if(VectorLength(pm->ps->velocity) > LEVEL2_WALLJUMP_MAXSPEED) \
						{ \
							VectorNormalize(pm->ps->velocity); \
							VectorScale(pm->ps->velocity, LEVEL2_WALLJUMP_MAXSPEED, pm->ps->velocity); \
						} \
 \
						PM_AddEvent(EV_JUMP); \
 \
						if(pm->cmd.forwardmove >= 0) \
						{ \
							if(!(pm->ps->persistant[PERS_STATE] & PS_NONSEGMODEL)) \
								PM_ForceLegsAnim(LEGS_JUMP); \
							else \
								PM_ForceLegsAnim(NSPA_JUMP); \
 \
							pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; \
						} \
						else \
						{ \
							if(!(pm->ps->persistant[PERS_STATE] & PS_NONSEGMODEL)) \
								PM_ForceLegsAnim(LEGS_JUMPB); \
							else \
								PM_ForceLegsAnim(NSPA_JUMPBACK); \
 \
							pm->ps->pm_flags |= PMF_BACKWARDS_JUMP; \
						} \
 \
						return; \
					} \
				} \
			} \
		} \
	}

	#define BG_OC_PMPounce() \
	{ \
		if(pm->ps->weapon == WP_ALEVEL3) \
			jumpMagnitude = pm->ps->stats[STAT_MISC] * LEVEL3_POUNCE_JUMP_MAG / LEVEL3_POUNCE_TIME; \
		else \
			jumpMagnitude = pm->ps->stats[STAT_MISC] * LEVEL3_POUNCE_JUMP_MAG_UPG / LEVEL3_POUNCE_TIME_UPG; \
	}

	#define BG_OC_PMNeedJumpChange() ((BG_OC_OCMode()) ? (1) : (0))

	#define BG_OC_PMJumpChange() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(pml.groundTrace.plane.normal[2] > 0.99995f && pm->ps->velocity[2] < 0) \
			pm->ps->velocity[2] = 0; \
 \
		VectorMA(pm->ps->velocity, BG_Class(pm->ps->stats[STAT_CLASS])->jumpMagnitude, \
				normal, pm->ps->velocity); \
	} while(0)

	#define BG_OC_PMNeedAlternateStopSprintCheck() ((BG_OC_OCMode()) ? (1) : (0))

	#define BG_OC_PMAlternateStopSprintCheck() \
	(((pm->cmd.forwardmove == 0 && pm->cmd.rightmove == 0) || pm->ps->pm_type != PM_NORMAL || pm->cmd.buttons & BUTTON_WALKING) ? (1) : (0))

	#define BG_OC_PMGROUNDTRACEHITCHECK ((BG_OC_OCMode()) ? ((trace.fraction < 1.0f && !(trace.surfaceFlags & (SURF_SKY | SURF_SLICK)) && !(trace.entityNum != ENTITYNUM_WORLD && i != 4)) ? (1) : (0)) : ((trace.fraction < 1.0f && !(trace.surfaceFlags & (SURF_SKY | SURF_SLICK))) ? (1) : (0)))

	//<+===============================================+>
	// special modes
	//<+===============================================+>

	void BG_OC_SetNoWallWalk(int c);
	int BG_OC_GetNoWallWalk(void);
#endif /* if defined CGAME || defined GAME */

//<+===============================================+><+===============================================+>
// game or cgame or ui
//<+===============================================+><+===============================================+>
#if 1
	#ifndef ISDEFINED_BG_STRTOLOWER__
	#define ISDEFINED_BG_STRTOLOWER__
	void BG_StrToLower(char *s);
	#endif
#endif /* if 1 */

#undef gentity_t
#undef weapon_t

#endif /* ifndef _G_OC_H */
