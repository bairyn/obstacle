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

/*
 * bg_oc.h
 *
 * The main header for the OC mod.  Putting everything in bg_oc makes the mod
 * easier to update and makes it more compatible with Tremulous.  There are,
 * though, several non-OC things that this mod nedes: floating point votes,
 * CP mix and print system, teleporters, trigger count return on at least
 * G_Checktrigger_stages(), extended !info, G_MinorFormatNumber(), override,
 * G_StrToLower(), bg_misc overrides (in the current mod, bg_misc overrides are
 * tied into OC's to know which overrides to use), a small fix to allow a
 * dynamic vec3_t initializer for PCLOUD in g_weapon.c, G_BuildableRange returns
 * buildable in range, extended votes, pause, buildlog, revert, several small
 * variable changes, extended layout format and version build and version; and
 * several which are recommended: client-side speedometer, cmd stealth and poor
 * aimbot detection, crash, connectMessage, CPMode, restart CP, no auto-vote, LayoutLoad memory
 * leak fix, speed, g_connectMessage and 'x is building' message.  Note that
 * this mod uses an entirely different system of flags and a much different
 * g_admin than a typical version.
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

//<+===============================================+><+===============================================+>
// game only stuff
//<+===============================================+><+===============================================+>

#ifdef GAME
	#include "../qcommon/q_shared.h"

	//<+===============================================+>
	// scrims and votes
	//<+===============================================+>

	typedef struct
	{
		int       active;
		char      name[50];  // MAX_NAME_LENGTH isn't here
		int       time;
		gentity_t **arms;
		gentity_t **medis;
		gentity_t *checkpoint;
		weapon_t  weapon;
		int       flags;
		#define G_OC_SCRIMFLAG_EQUIPMENT     0x0001
		#define G_OC_SCRIMFLAG_NOTSINGLETEAM 0x0002  // set when a player joins a team that already has another player in it (and never unset while the team exists)
	//	struct g_oc_scrimTeam_t *next;
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
	} while(0);

	#define G_OC_PostCheckVote() level.ocScrimVote = 0;

	//<+===============================================+>
	// oc stats, ratings and layouts
	//<+===============================================+>

	#define G_OC_STAT_MAXRECORDS 16
	#define G_OC_MAX_LAYOUT_RATINGS 1024
	/*
	 * We can either load a long while by reading the layout file MAX_SPAWNGROUP times,
	 * or read it a few times and put it in fragmented tables.  The problem
	 * with fragmented tables is that spawn groups may not spawn in the right order.
	 * Each table is loaded in order, but each table is loaded in order too.
	 * This rare issue is only ever a problem when a structure is built over
	 * (over, so it relies on another structure's spawnGroup) and the two (or
	 * multiple) structures are in different tables, and the wrong table comes
	 * first.  Fortunately, layout editors usually build in groups so any structures
	 * which depend on each other are usually in the same table.  It is also even
	 * rarer because the layout developer usually builds the second one second, and
	 * the third one third, and since the layout is usually saved in order
	 * the order of the tables is usually correct (this isn't always
	 * the case, especially if the developer deconstructs a lot of structures).
	*/
	#define G_OC_MAX_LAYOUT_BUILDABLES 4096
	// TODO: keep BG_Alloc from crashing server when (and only when) trying different memory sizes it can't handle.
	// until then, both these are somewhat low
	#define G_OC_MIN_LAYOUT_BUILDABLES 512

	void G_OC_LoadRatings(void);
	char *G_OC_Rating(char *mapname, char *layoutname);
	void G_OC_LayoutLoad(char *layout);

	//<+===============================================+>
	// editoc
	//<+===============================================+>

	#define G_OC_MAX_SPAWNGROUP 2048

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
	int       aliveTime; \
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
	int       buildableOverride;

	#define G_OC_FRAMETIMEWEAPON 1000
	#define G_OC_FRAMETIMEOVERRIDE 1500

	#define G_OC_PTRCDATA

	#define G_OC_CVARS \
	vmCvar_t g_ocOnly; \
	vmCvar_t g_ocWarmup; /* warmup time for scrims */ \
	vmCvar_t g_statsEnabled; \
	vmCvar_t g_statsRecords; \
	vmCvar_t g_ocReview; \
	vmCvar_t g_allowHiding; \
	vmCvar_t g_allowHideVote; \
	vmCvar_t g_allowUnhideVote; \
	vmCvar_t g_hideTimeCallvoteMinutes; \
	vmCvar_t g_unhideTimeCallvoteMinutes; \
	vmCvar_t g_timelimitDrop; \
	vmCvar_t g_startScrimVotePercent; \
	vmCvar_t g_endScrimVotePercent;

	#define G_OC_EXTERNCVARS \
	extern vmCvar_t g_ocOnly; \
	extern vmCvar_t g_ocWarmup; \
	extern vmCvar_t g_statsEnabled; \
	extern vmCvar_t g_statsRecords; \
	extern vmCvar_t g_ocReview; \
	extern vmCvar_t g_allowHiding; \
	extern vmCvar_t g_allowHideVote; \
	extern vmCvar_t g_allowUnhideVote; \
	extern vmCvar_t g_hideTimeCallvoteMinutes; \
	extern vmCvar_t g_unhideTimeCallvoteMinutes; \
	extern vmCvar_t g_timelimitDrop; \
	extern vmCvar_t g_startScrimVotePercent; \
	extern vmCvar_t g_endScrimVotePercent;

	#define G_OC_CVARTABLE \
	{ &g_ocOnly, "g_ocOnly", "0", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_ocWarmup, "g_ocWarmup", "20", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_statsEnabled, "g_statsEnabled", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_statsRecords, "g_statsRecords", "10", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_ocReview, "g_ocReview", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_allowHiding, "g_allowHiding", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_allowHideVote, "g_allowHideVote", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_allowUnhideVote, "g_allowUnhideVote", "1", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_hideTimeCallvoteMinutes, "g_hideTimeCallvoteMinutes", "5", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_unhideTimeCallvoteMinutes, "g_unhideTimeCallvoteMinutes", "5", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_timelimitDrop, "g_timelimitDrop", "17.5", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_startScrimVotePercent, "g_startScrimVotePercent", "85", CVAR_ARCHIVE, 0, qtrue  }, \
	{ &g_endScrimVotePercent, "g_endScrimVotePercent", "90", CVAR_ARCHIVE, 0, qtrue  },

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

	#define G_OC_NeedLoadOC() (((g_ocOnly.integer) || (tolower(level.layout[0]) == 'o' && tolower(level.layout[0]) == 'c')) ? ((G_StrToLower(level.layout)), (trap_SetConfigstring(CS_OCMODE, "1")), (BG_OC_SetOCModeOC()), (1)) : ((trap_SetConfigstring(CS_OCMODE, "0")), (BG_OC_SetOCModeNone()), (0)))

	#define G_OC_LoadOC() \
	do \
	{ \
		unsigned int triggers = 0; \
 \
		G_StrToLower(level.layout); \
		trap_Cvar_Set("g_humanBuildPoints", va("%d", INFINITE)); \
		trap_Cvar_Set("g_alienBuildPoints", va("%d", INFINITE)); \
		trap_Cvar_Set("g_alienStage", va("%d", S3)); \
		trap_Cvar_Set("g_humanStage", va("%d", S3)); \
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
		if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_NOWALLWALK)) \
			BG_OC_SetNoWallWalk(1); \
		else \
			BG_OC_SetNoWallWalk(0); \
 \
		if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_NOHEIGHTLOST)) \
			BG_OC_SetHeightNeverLost(1); \
		else \
			BG_OC_SetHeightNeverLost(0); \
	} while(0)

	//<+===============================================+>
	// game and balance stuff
	//<+===============================================+>

	char *G_OC_MediStats(gentity_t *ent, int count, int time);
	char *G_OC_WinStats(gentity_t *ent, int count, int time);

	#define G_OC_NoDamageAlert() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_CanBuildableBeDestoryedOnOtherTeam() ((BG_OC_OCMode()) ? (1) : (0))

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

	#define G_OC_SelectAlienSpawnPoint() G_OC_SelectHumanSpawnPoint()

	#define G_OC_SpotNeverTelefrags() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_NeedOtherSayTeamCheck() ((BG_OC_OCMode()) ? ((ent->client->pers.scrimTeam) ? (1) : (0)) : (0))
	#define G_OC_OtherSayTeamCheck() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(mode == SAY_TEAM && other->client->pers.scrimTeam != ent->client->pers.scrimTeam) \
		{ \
			if(other->client->pers.teamSelection != TEAM_NONE) \
				return; \
 \
			/* if(!G_admin_permission(other, ADMF_SPEC_ALLCHAT)) */  /* even if they are admins, they can't steal teamchat from scrim teams */ \
				return; \
		} \
	} while(0)

	#define G_OC_NeedResetStages() (!(BG_OC_OCMode()))
	#define G_OC_NeedSuddenDeath() (!(BG_OC_OCMode()))
	#define G_OC_NeedSuddenDtMsg() (!(BG_OC_OCMode()))
	#define G_OC_NeedTimelimigMg() (!(BG_OC_OCMode()))

	#define G_OC_PTRCUpdate() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
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

	#define G_OC_FireWeapon2() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent->client->pers.scrimTeam || ent->client->pers.override || G_admin_canEditOC(ent)) \
			return; \
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
				meleeAttack(ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW); \
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
		client = ent->client; \
 \
		if(level.totalMedistations && client->pers.medis && client->pers.medisLastCheckpoint) \
		{ \
			BG_Free(client->pers.medis); \
			BG_Free(client->pers.medisLastCheckpoint); \
		} \
		if(level.totalArmouries && client->pers.arms && client->pers.armsLastCheckpoint) \
		{ \
			BG_Free(client->pers.arms); \
			BG_Free(client->pers.armsLastCheckpoint); \
		} \
 \
		/* cleanly handle scrim teams */ \
		G_OC_RemovePlayerFromScrimTeam(ent); \
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
		if(G_admin_hide_check(userinfo, reason, sizeof(reason), &hidden, &hiddenTime, NULL)) \
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

	#define G_OC_NeedNoCreep() (BG_OC_OCMode() ? (G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_NOCREEP)) : (0))
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

	#define G_OC_NeedAlternateCanBuild() (BG_OC_OCMode() ? (1): (1))
	#define G_OC_AlternateCanBuild() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* Stop all buildables from interacting with traces */ \
		if(!ent->client->pers.buildableOverride) \
		  G_SetBuildableLinkState(qfalse); /* in OC mode, stackables ae allowed if buildable override is on for the client */ \
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
		if(!ent->client->pers.buildableOverride) \
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
		if(!ent->client->pers.buildableOverride) \
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
		if(!ent->client->pers.buildableOverride)  /* && */ \
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
		if(!ent->client->pers.buildableOverride) \
			G_SetBuildableLinkState(qtrue); \
 \
		/*check there is enough room to spawn from (presuming this is a spawn) */ \
		/* no matter if they do have buildable override, players should always be able to spawn from it */ \
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
		if(!ent->client->pers.buildableOverride) \
		{ \
			if(reason == IBE_NONE && (tr2.fraction < 1.0 || tr3.fraction < 1.0)) \
				reason = IBE_NOROOM; \
 \
		} \
		if(reason != IBE_NONE) \
			level.numBuildablesForRemoval = 0; \
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
				self->powered = G_FindCreep(self); \
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
				self->powered = G_FindPower(self); \
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
 \
		if(built->s.modelindex == BA_H_SPAWN) \
			level.numNodes++; \
	} while(0)

	#define G_OC_NeedStartSolid() (!(BG_OC_OCMode()))  // OC buildables can be in and on other stuff

	#define G_OC_BUILDABLE_STRUCT_DEFS \
	qboolean verifyUnpowered;

	#define G_OC_RESTARTOC_TIME 3000  // time after a restartoc before a checkpoint can be used
	#define G_OC_NOBONUSMESSAGE "Cannot collect bonuses using\na jetpack"
	#define G_OC_EVOLVEBLOCK_TIME 2000

	#define G_OC_HumanNameForWeapon(x) (BG_Weapon((x)) ? (BG_Weapon((x))->humanName) : ("NULL"))
	#define G_OC_TeamForWeapon(x) (BG_Weapon((x)) ? (BG_Weapon((x))->team) : (0))

	#define G_OC_NoSuddenDeath() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_NoBuildTimer() ((BG_OC_OCMode()) ? ((G_admin_canEditOC(ent)) ? (1) : (0)) : (0))

	int G_OC_UseMedi(gentity_t *ent, gentity_t *medi);
	int G_OC_SyncMedis(gentity_t **medis, int len);
	int G_OC_MergeMedis(gentity_t **dst, gentity_t **src);
	int G_OC_AppendMedi(gentity_t **medis, gentity_t *medi);
	int G_OC_RemoveMedi(gentity_t **medis, gentity_t *medi);
	int G_OC_AllMedis(gentity_t **medis);
	int G_OC_NumberOfMedis(gentity_t **medis);
	int G_OC_HasMediBeenUsed(gentity_t *medi, gentity_t **medis);
	int G_OC_ClearMedis(gentity_t **medis);
	int G_OC_UseArm(gentity_t *ent, gentity_t *arm);
	int G_OC_SyncArms(gentity_t **arms, int len);
	int G_OC_MergeArms(gentity_t **dst, gentity_t **src);
	int G_OC_AppendArm(gentity_t **arms, gentity_t *arm);
	int G_OC_RemoveArm(gentity_t **arms, gentity_t *arm);
	int G_OC_AllArms(gentity_t **arms);
	int G_OC_NumberOfArms(gentity_t **arms);
	int G_OC_HasArmBeenUsed(gentity_t *arm, gentity_t **arms);
	int G_OC_ClearArms(gentity_t **arms);
	int G_OC_Checkpoint(gentity_t *checkpoint, gentity_t *ent);
	int G_OC_PlayerSpawn(gentity_t *ent);
	int G_OC_PlayerDie(gentity_t *ent);
	int G_OC_CanUseBonus(gentity_t *ent);
//	int G_OC_WeaponIsReserved(weapon_t weapon);
	int G_OC_WeaponIsReserved(int weapon);
	int G_OC_WeaponRemoveReserved(gentity_t *ent);
	int G_OC_IsSingleScrim(void);
	int G_OC_EndScrim(void);
	//int G_OC_ValidScrimWeapon(weapon_t weapon);
	int G_OC_ValidScrimWeapon(int weapon);
	int G_OC_JoinPlayerToScrimTeam(gentity_t *ent, gentity_t *reportEnt, char *teamName, char *weaponName);
	int G_OC_ValidScrimTeamName(char *name);
	int G_OC_RemovePlayerFromScrimTeam(gentity_t *ent);
	int G_OC_EmptyScrim(void);
	int G_OC_NumScrimTeams(void);
	int G_OC_TooFewScrimTeams(void);
	int G_OC_HasScrimFinished(void);
	int G_OC_NumberOfTeams(void);
	#define G_OC_BUILDABLEBUILT(x) G_OC_BuildableBuilt(x)
	int G_OC_BuildableBuilt(gentity_t *ent);
	#define G_OC_BUILDABLEDESTROYED(x) G_OC_BuildableDestroyed(x)
	int G_OC_BuildableDestroyed(gentity_t *ent);

	void G_OC_Lol(gentity_t *ent);

	#define G_OC_NeedSuicide() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeverForceTeamBalance() ((BG_OC_OCMode()) ? (1) : (0))

	//<+===============================================+>
	// buildable optimization
	//<+===============================================+>

	#define G_OC_AlienBuildableOptimizedThinkTime() (BG_OC_OCMode() ? ((G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ALIENONLY) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_BOTHTEAMS)) ? (G_OC_OPTIMIZED_BUILDABLE_THINK_OFFSET) : (0)) : (0))  // if it's an alien only OC, certain thinks can have more latency
	#define G_OC_HumanBuildableOptimizedThinkTime() (BG_OC_OCMode() ? ((!G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ALIENONLY) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_BOTHTEAMS)) ? (G_OC_OPTIMIZED_BUILDABLE_THINK_OFFSET) : (0)) : (0))  // if it's a human only OC, certain thinks can have more latency

	#define G_OC_OPTIMIZED_BUILDABLE_THINK_OFFSET +4000  // save some processing power for tubes, hovels, barricades, trappers, turrets, teslas, DC's, and armouries

	//<+===============================================+>
	// game times
	//<+===============================================+>

	#define G_OC_NeedEndGameTimelimit() ((BG_OC_OCMode()) ? ((level.numConnectedClients) ? (0) : (1)) : (1))
	#define G_OC_NeedEndGameTeamWin() (!(BG_OC_OCMode()))

	//<+===============================================+>
	// OC flags
	//<+===============================================+>

	// these need to be lowercase
	#define G_OC_OCFLAG_ONEARM            "n"
	#define G_OC_OCFLAG_NOCREEP           "p"
	#define G_OC_OCFLAG_ALIENONLY         "a"
	#define G_OC_OCFLAG_BOTHTEAMS         "b"
	#define G_OC_OCFLAG_NOWALLWALK        "w"
	#define G_OC_OCFLAG_AGRANGER          "g"
	#define G_OC_OCFLAG_AGRANGERUPG       "^g"
	#define G_OC_OCFLAG_ADRETCH           "d"
	#define G_OC_OCFLAG_ABASILISK         "l"
	#define G_OC_OCFLAG_ABASILISKUPG      "^l"
	#define G_OC_OCFLAG_AMARAUDER         "m"
	#define G_OC_OCFLAG_AMARAUDERUPG      "^m"
	#define G_OC_OCFLAG_ADRAGOON          "h"
	#define G_OC_OCFLAG_ADRAGOONUPG       "^h"
	#define G_OC_OCFLAG_ATYRANT           "t"
	#define G_OC_OCFLAG_NOHEIGHTLOST      "j"

	// oc flag names
	#define G_OC_OCFLAG_ONEARM_NAME       "UseOneArm"
	#define G_OC_OCFLAG_NOCREEP_NAME      "NoCreep"
	#define G_OC_OCFLAG_ALIENONLY_NAME    "Aliens"
	#define G_OC_OCFLAG_BOTHTEAMS_NAME    "BothTeams"
	#define G_OC_OCFLAG_NOWALLWALK_NAME   "NoWallWalk"
	#define G_OC_OCFLAG_AGRANGER_NAME     "Grangers"
	#define G_OC_OCFLAG_AGRANGERUPG_NAME  "AdvGrangers"
	#define G_OC_OCFLAG_ADRETCH_NAME      "Dretches"
	#define G_OC_OCFLAG_ABASILISK_NAME    "Basilisks"
	#define G_OC_OCFLAG_ABASILISKUPG_NAME "AdvBasilisks"
	#define G_OC_OCFLAG_AMARAUDER_NAME    "Maradauders"
	#define G_OC_OCFLAG_AMARAUDERUPG_NAME "AdvMaradauders"
	#define G_OC_OCFLAG_ADRAGOON_NAME     "Dragoons"
	#define G_OC_OCFLAG_ADRAGOONUPG_NAME  "AdvDragoons"
	#define G_OC_OCFLAG_ATYRANT_NAME      "Tyrants"
	#define G_OC_OCFLAG_NOHEIGHTLOST_NAME "OptimizeJumps"

	void     G_OC_ParseLayoutFlags(char *layout, char *out);
	qboolean G_OC_TestLayoutFlag(char *layout, char *flag);
	qboolean G_OC_LayoutExtraFlags(char *layout);

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
		if(team == TEAM_HUMANS && G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ALIENONLY) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_BOTHTEAMS)) \
		{ \
			G_ClientPrint(ent, va("You cannot join humans with layout option '%s'", G_OC_OCFLAG_ALIENONLY_NAME), CLIENT_NULL); \
			return; \
		} \
 \
		if(team == TEAM_ALIENS && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ALIENONLY) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_BOTHTEAMS)) \
		{ \
			G_ClientPrint(ent, va("You cannot join aliens without layout option '%s' or '%s'", G_OC_OCFLAG_ALIENONLY_NAME, G_OC_OCFLAG_BOTHTEAMS_NAME), CLIENT_NULL); \
			return; \
		} \
 \
		/* long series of tests for alien classes */ \
		if(team == TEAM_ALIENS && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AGRANGER) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AGRANGERUPG) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRETCH) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ABASILISK) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ABASILISKUPG) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AMARAUDER) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AMARAUDERUPG) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRAGOON) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRAGOONUPG) && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ATYRANT)) \
		{ \
			G_ClientPrint(ent, "There are no alien class flags in the layout", CLIENT_NULL); \
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
	|| !Q_stricmp(arg1, "hide") \
	|| !Q_stricmp(arg1, "unhide")

	#define G_OC_NeedAlternateMapVote() (1)
	#define G_OC_NeedAlternateMapRestartVote() ((BG_OC_OCMode()) ? (1) : (0))
	#define G_OC_NeedAlternateDrawVote() ((BG_OC_OCMode()) ? (1) : (0))

	#define G_OC_AlternateMapVote() \
	do \
	{ \
		G_StrToLower(arg3); \
 \
		if(BG_OC_OCMode() && level.ocScrimState > G_OC_STATE_NONE && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
			trap_SendServerCommand(ent-g_entities, va("print \"callvote: you cannot call for a mapchange during a scrim\n\"")); \
			return; \
		} \
 \
		if(!trap_FS_FOpenFile(va("maps/%s.bsp", arg2), NULL, FS_READ)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"callvote: " \
				"'maps/%s.bsp' could not be found on the server\n\"", arg2)); \
			return; \
		} \
 \
		if(g_ocOnly.integer) \
		{ \
			if(!arg3[0]) \
				Q_strncpyz(arg3, "oc", sizeof(arg3)); \
 \
			if(arg3[0] != 'o' || arg3[0] != 'c') \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"callvote: " \
					"'%s^7' is not an obstacle course\n\"", ((arg3[0]) ? (arg3) : ("(empty)")))); \
				return; \
			} \
		} \
 \
		if(!trap_FS_FOpenFile(va("maps/%s.bsp", arg2), NULL, FS_READ)) \
		{ \
			trap_SendServerCommand(ent - g_entities, va("print \"callvote: " \
				"'maps/%s.bsp' could not be found on the server\n\"", arg2)); \
			return; \
		} \
 \
		if(arg3[0]) \
		{ \
			if(!trap_FS_FOpenFile(va("layouts/%s/%s.dat", arg2, arg3), NULL, FS_READ)) \
			{ \
				trap_SendServerCommand(ent - g_entities, va("print \"callvote: " \
					"'layouts/%s/%s.dat' could not be found on the server\n\"", arg2, arg3)); \
				return; \
			} \
		} \
 \
		if(arg3[0]) \
		{ \
			level.votePassThreshold = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString, sizeof(level.voteString), "!map %s %s", arg2, arg3); \
			Com_sprintf(level.voteDisplayString, \
			sizeof(level.voteDisplayString), "Change to map '%s^7' using layout '%s^7'", arg2, arg3); \
		} \
		else \
		{ \
			level.votePassThreshold = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2); \
			Com_sprintf(level.voteDisplayString, \
				sizeof(level.voteDisplayString), "Change to map '%s^7'", arg2); \
		} \
	} while(0)

	#define G_OC_AlternateMapRestartVote() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		G_StrToLower(level.layout); \
 \
		if(BG_OC_OCMode() && level.ocScrimState > G_OC_STATE_NONE && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
			trap_SendServerCommand(ent-g_entities, va("print \"callvote: you cannot call for a mapchange during a scrim\n\"")); \
			return; \
		} \
 \
		if(*level.layout) \
		{ \
			level.votePassThreshold = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString, sizeof(level.voteString), "!restart %s", level.layout); \
			Com_sprintf(level.voteDisplayString, \
				sizeof(level.voteDisplayString), "Restart current map using layout '%s^7'", level.layout); \
		} \
		else \
		{ \
			level.votePassThreshold = g_mapVotePercent.value + percentAddition; \
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s", arg1); \
			Com_sprintf(level.voteDisplayString, \
				sizeof(level.voteDisplayString), "Restart current map"); \
		} \
	} while(0)

	#define G_OC_AlternateDrawVote() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		G_StrToLower(level.layout); \
 \
		if(BG_OC_OCMode() && level.ocScrimState > G_OC_STATE_NONE && !G_admin_permission(ent, ADMF_NO_VOTE_LIMIT)) \
		{ \
			trap_SendServerCommand(ent-g_entities, va("print \"callvote: you cannot call for a mapchange during a scrim\n\"")); \
			return; \
		} \
 \
		level.votePassThreshold = g_mapVotePercent.value + percentAddition; \
		Com_sprintf(level.voteString, sizeof(level.voteString), "evacuation"); \
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), \
			"End match in a draw"); \
	} while(0)

	#define G_OC_OTHERVOTECOMMANDS \
	else if(!Q_stricmp(arg1, "hide") && BG_OC_OCMode()) \
	{ \
		if(!g_allowHideVote.integer) \
		{ \
			trap_SendServerCommand(ent-g_entities, "print \"callvote: hide votes are disabled\n\""); \
			return; \
		} \
 \
		if(level.clients[clientNum].pers.hidden) \
		{ \
			trap_SendServerCommand(ent-g_entities, \
				"print \"callvote: player is already hidden\n\""); \
			return; \
		} \
 \
		if(level.clients[clientNum].pers.hiddenTime) \
		{ \
			trap_SendServerCommand(ent-g_entities, \
				"print \"callvote: player is force hidden\n\""); \
			return; \
		} \
 \
		if(G_admin_permission(&g_entities[clientNum], ADMF_IMMUNITY)) \
		{ \
			trap_SendServerCommand(ent-g_entities, \
				"print \"callvote: admin is immune from vote hide\n\""); \
			return; \
		} \
 \
		Com_sprintf(level.voteString, sizeof(level.voteString), \
			"!hide %i", clientNum); \
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), \
			"Hide player \'%s\'", name); \
	} \
	else if(!Q_stricmp(arg1, "unhide") && BG_OC_OCMode()) \
	{ \
		if(!g_allowUnhideVote.integer) \
		{ \
			trap_SendServerCommand(ent-g_entities, "print \"callvote: unhide votes are disabled\n\""); \
			return; \
		} \
 \
		if(!level.clients[clientNum].pers.hidden) \
		{ \
			trap_SendServerCommand(ent-g_entities, \
				"print \"callvote: player is not currently hidden\n\""); \
			return; \
		} \
 \
		if(level.clients[clientNum].pers.hiddenTime) \
		{ \
			trap_SendServerCommand(ent-g_entities, \
				"print \"callvote: player is force unhidden\n\""); \
			return; \
		} \
 \
		if(G_admin_permission(&g_entities[clientNum], ADMF_IMMUNITY)) \
		{ \
			trap_SendServerCommand(ent-g_entities, \
				"print \"callvote: admin is immune from vote unhide\n\""); \
			return; \
		} \
 \
		Com_sprintf(level.voteString, sizeof(level.voteString), \
			"!unhide %i", clientNum); \
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), \
			"Un-Hide player \'%s\'", name); \
	} \
	else if(!Q_stricmp(arg1, "startscrim") && BG_OC_OCMode()) \
	{ \
		if(arg2[0] != 'a' || arg2[0] != 'm') \
		{ \
			G_ClientPrint(ent, "callvote: usage /callvote startscrim [a/m]", CLIENT_NULL); \
			return; \
		} \
 \
		if(level.ocScrimState > G_OC_STATE_NONE) \
		{ \
			G_ClientPrint(ent, "callvote: a scrim is already taking place", CLIENT_NULL); \
			return; \
		} \
 \
		if(G_OC_EmptyScrim()) \
		{ \
			G_ClientPrint(ent, "callvote: empty is the scrim", CLIENT_NULL); \
			return; \
		} \
 \
		if(G_OC_TooFewScrimTeams()) \
		{ \
			G_ClientPrint(ent, "callvote: too few teams exist for a scrim", CLIENT_NULL); \
			return; \
		} \
 \
		if(!ent->client->pers.scrimTeam) \
		{ \
			G_ClientPrint(ent, "callvote: you need to be on a scrim team to do this", CLIENT_NULL); \
			return; \
		} \
 \
		level.votePassThreshold = g_startScrimVotePercent.value; \
		Com_sprintf(level.voteString, sizeof(level.voteString), "!startscrim %c", arg2[0]); \
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), \
			"Start a '%s^7' scrim^7", arg2[0] == 'm' ? "^1medi^7" : "^2armoury^7"); \
	} \
	else if(!Q_stricmp(arg1, "endscrim") && BG_OC_OCMode()) \
	{ \
		if(level.ocScrimState <= G_OC_STATE_NONE) \
		{ \
			G_ClientPrint(ent, "callvote: no scrim is currently taking place", CLIENT_NULL); \
			return; \
		} \
 \
		level.votePassThreshold = g_endScrimVotePercent.value; \
		Com_sprintf(level.voteString, sizeof(level.voteString), "!endscrim"); \
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "End the scrim"); \
	}

	#define G_OC_OtherCommandDescription() ((BG_OC_OCMode()) ? (", hide, unhide, startscrim, endscrim") : (""))

	#define G_OC_AllowSuddenDeathVote() (!(BG_OC_OCMode()))

	#define G_OC_VoteCheck() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		if(!ent->client->pers.scrimTeam && strstr(level.voteString, "!startscrim")) \
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
						(newClass == PCL_ALIEN_BUILDER0 && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AGRANGER)) || \
						(newClass == PCL_ALIEN_BUILDER0_UPG && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AGRANGERUPG)) || \
						(newClass == PCL_ALIEN_LEVEL0 && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRETCH)) || \
						(newClass == PCL_ALIEN_LEVEL1 && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ABASILISK)) || \
						(newClass == PCL_ALIEN_LEVEL1_UPG && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ABASILISKUPG)) || \
						(newClass == PCL_ALIEN_LEVEL2 && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AMARAUDER)) || \
						(newClass == PCL_ALIEN_LEVEL2_UPG && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AMARAUDERUPG)) || \
						(newClass == PCL_ALIEN_LEVEL3 && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRAGOON)) || \
						(newClass == PCL_ALIEN_LEVEL3_UPG && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRAGOONUPG)) || \
						(newClass == PCL_ALIEN_LEVEL4 && !G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ATYRANT)) \
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
				if((!G_OC_AllArms(ent->client->pers.arms) || (G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM) && G_OC_NumberOfArms(ent->client->pers.arms))) && !ent->client->pers.override) \
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
		client->ps.persistant[PERS_OCTIMER] = client->pers.aliveTime; \
	} while(0)

	#define G_OC_ClientThink() \
	do \
	{ \
		int i; \
 \
		if(!BG_OC_OCMode()) \
			break; \
 \
		client->ps.persistant[PERS_OCTIMER] = client->pers.aliveTime; \
 \
		if((client->pers.teamSelection == TEAM_HUMANS || client->pers.teamSelection == TEAM_ALIENS) && ent->health > 0 && client->sess.spectatorState != SPECTATOR_NOT) \
		{ \
			client->pers.aliveTime += trap_Milliseconds() - client->pers.lastAliveTime; \
			client->pers.lastAliveTime = trap_Milliseconds(); \
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
			/* find the first class to evolve to */ \
			if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AGRANGER)) \
				newClass = PCL_ALIEN_BUILDER0; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AGRANGERUPG)) \
				newClass = PCL_ALIEN_BUILDER0_UPG; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRETCH)) \
				newClass = PCL_ALIEN_LEVEL0; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ABASILISK)) \
				newClass = PCL_ALIEN_LEVEL1; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ABASILISKUPG)) \
				newClass = PCL_ALIEN_LEVEL1_UPG; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AMARAUDER)) \
				newClass = PCL_ALIEN_LEVEL2; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_AMARAUDERUPG)) \
				newClass = PCL_ALIEN_LEVEL2_UPG; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRAGOON)) \
				newClass = PCL_ALIEN_LEVEL3; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ADRAGOONUPG)) \
				newClass = PCL_ALIEN_LEVEL3_UPG; \
			else if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ATYRANT)) \
				newClass = PCL_ALIEN_LEVEL4; \
 \
			/* furthur unnecessary checks for wallwaking and some other checks */ \
			if(!(client->ps.stats[STAT_STATE] & SS_WALLCLIMBING) && client->pers.teamSelection == TEAM_ALIENS && !(client->ps.stats[STAT_STATE] & SS_HOVELING) && ent->health > 0 && G_RoomForClassChange(ent, newClass, infestOrigin)) \
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
					ClientUserinfoChanged(ent - g_entities); \
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
				if(client->pers.teamSelection == TEAM_HUMANS) \
				{ \
					if(!client->pers.arms || !(\
						(client->pers.scrimTeam) \
						? \
						(level.scrimTeam[client->pers.scrimTeam].flags & G_OC_SCRIMFLAG_EQUIPMENT) \
						: \
						(G_OC_AllArms(client->pers.arms) || (G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM) && G_OC_NumberOfArms(client->pers.arms))) \
					))  /* messy mess of tests if client can buy equipment */ \
					{ \
						if(client->ps.stats[STAT_WEAPON] != WP_MACHINEGUN) \
						{ \
							client->ps.stats[STAT_WEAPON] = WP_MACHINEGUN; \
							G_ForceWeaponChange(ent, WP_MACHINEGUN); \
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
			if(!client->pers.noAuO) \
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
	{"spawnUp", CMD_TEAM | CMD_LIVING, Cmd_Spawnup_f}, \
		{"spawnGroupUp", CMD_TEAM | CMD_LIVING, Cmd_Spawnup_f}, \
		{"upSpawn", CMD_TEAM | CMD_LIVING, Cmd_Spawnup_f}, \
		{"upSpawnGroup", CMD_TEAM | CMD_LIVING, Cmd_Spawnup_f}, \
	{"spawnDown", CMD_TEAM | CMD_LIVING, Cmd_Spawndown_f}, \
		{"spawnGroupDown", CMD_TEAM | CMD_LIVING, Cmd_Spawndown_f}, \
		{"downSpawn", CMD_TEAM | CMD_LIVING, Cmd_Spawndown_f}, \
		{"downSpawnGroup", CMD_TEAM | CMD_LIVING, Cmd_Spawndown_f}, \
	{"mystats", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Mystats_f}, \
	{"stats", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Mystats_f}, \
		{"statistics", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"hiScores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"hi-Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"hi_Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"highScores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"high-Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
		{"high_Scores", CMD_MESSAGE | CMD_INTERMISSION, Cmd_Stats_f}, \
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
	{"hide", 0, Cmd_Hide_f}, \
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
	{"lol", CMD_MESSAGE, G_OC_Lol},

	#define G_OC_PTRCRestore() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
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
	#define CG_OC_DRAWCP() \
	do \
	{ \
		if(!BG_OC_OCMode()) \
			break; \
 \
		/* TODO: either find a better spot for this or make it more flexible */ \
 \
		if(cg_printTimer.integer) \
		{ \
			/* timer */ \
 \
			/* cg_printTimer is set, so cg.time is always set */ \
			cg.centerPrintTime = cg.time; \
 \
			Q_strcat(buf, sizeof(buf), va("^2%dm:%ds:%dms\n^7", MINS(cg.snap->ps.persistant[PERS_OCTIMER]), SECS(cg.snap->ps.persistant[PERS_OCTIMER]), MSEC(cg.snap->ps.persistant[PERS_OCTIMER]))); \
		} \
 \
		if(cg_printSpeedometer.integer) \
		{ \
			/* speedometer */ \
 \
			float tmp = cg.snap->ps.velocity[2]; \
 \
			/* cg_printSpeedometer is set, so cg.time is always set */ \
			cg.centerPrintTime = cg.time; \
 \
			Q_strcat(buf, sizeof(buf), va("^2XYZ: %d^7ups\n^7", (int) VectorLength(cg.snap->ps.velocity))); \
			cg.snap->ps.velocity[2] = 0.0f; \
			Q_strcat(buf, sizeof(buf), va("^2XY: %d^7ups\n^7", (int) VectorLength(cg.snap->ps.velocity))); \
			cg.snap->ps.velocity[2] = tmp; \
		} \
	} while(0)

	#define CG_OC_CVARS \
	vmCvar_t cg_printTimer; \
	vmCvar_t cg_printSpeedometer;

	#define CG_OC_DCVARS \
	{ &cg_printTimer, "cg_printTimer", "0", CVAR_ARCHIVE }, \
	{ &cg_printSpeedometer, "cg_printSpeedometer", "0", CVAR_ARCHIVE },

	#define CG_OC_SERVERCMDS \
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
	}

	#define CG_OC_ECVARS \
	extern vmCvar_t cg_printTimer; \
	extern vmCvar_t cg_printSpeedometer;

	#define CG_OC_SetConfigStrings() \
	do \
	{ \
		switch(atoi((const char *) CG_ConfigString(CS_OCMODE))) \
		{ \
			case 0: \
				BG_OC_SetOCModeNone(); \
				break; \
 \
			default: \
				BG_OC_SetOCModeOC(); \
				break; \
		} \
	} while(0)
#endif /* ifdef CGAME */

//<+===============================================+><+===============================================+>
// game or cgame
//<+===============================================+><+===============================================+>

#if defined(CGAME) || defined(GAME) || defined(OC_BGAME)
	//<+===============================================+>
	// OC mode itself
	//<+===============================================+>

	void BG_OC_SetOCModeNone(void);
	void BG_OC_SetOCModeOC(void);
	int  BG_OC_GetOCMode(void);

	#define BG_OC_CS
	#define CS_OCMODE 31

	//<+===============================================+>
	// game and balance stuff
	//<+===============================================+>

	#define BG_OC_PERS \
	,PERS_OCTIMER

	#define BG_OC_NeedBuildableAppend() BG_OC_OCMode()
	#define BG_OC_NeedClassAppend() BG_OC_OCMode()
	#define BG_OC_NeedWeaponAppend() BG_OC_OCMode()
	#define BG_OC_BuildableAppend() Q_strcat(buf, sizeof(buf), "/oc");
	#define BG_OC_ClassAppend() Q_strcat(buf, sizeof(buf), "/oc");
	#define BG_OC_WeaponAppend() Q_strcat(buf, sizeof(buf), "/oc");

	#define BG_OC_PLAYERMASK ((BG_OC_OCMode()) ? (MASK_DEADSOLID) : (MASK_PLAYERSOLID))  // don't clip against players if OC mode (not for everything)
	#define BG_OC_SHOTMASK ((BG_OC_OCMode()) ? (MASK_SOLID) : (MASK_SHOT))  // if OC mode don't clip against players (don't use this for everything; should be used for projectiles from player weapons or spawn checking)

	//<+===============================================+>
	// pmove
	//<+===============================================+>

	#define BG_OC_PMNeedCrashLand() ((BG_OC_OCMode()) ? (1) : (0))
	#define BG_OC_PMOCDodge() (0)
	#define BG_OC_PMOCWallJump() ((BG_OC_OCMode()) ? (1) : (0))
	#define BG_OC_PMOCGroundTraceWallJump() ((BG_OC_OCMode()) ? (1) : (0))
	#define BG_OC_PMOCPounce() (0)

	#define BG_OC_ZERO_HEIGHT_MODIFIER 0.9f
	#define BG_OC_PMZeroJump() ((BG_OC_OCMode()) ? ((pm->ps->velocity[2] <= BG_Class(pm->ps->stats[STAT_CLASS])->jumpMagnitude * BG_OC_ZERO_HEIGHT_MODIFIER) ? (0) : (1)): (1))
//	#define BG_OC_PMZeroJump() ((BG_OC_OCMode()) ? ((oc_heightNeverLost) ? (1) : (0)) : (1))

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
			pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask); \
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

	//<+===============================================+>
	// special modes
	//<+===============================================+>

	void BG_OC_SetHeightNeverLost(int c);
	int BG_OC_GetHeightNeverLost(void);
	void BG_OC_SetNoWallWalk(int c);
	int BG_OC_GetNoWallWalk(void);
#endif /* if defined CGAME || defined GAME */

#undef gentity_t
#undef weapon_t

#endif /* ifndef _G_OC_H */
