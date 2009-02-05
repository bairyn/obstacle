/*
===========================================================================
Copyright (C) 2004-2006 Tony J. White

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

#ifndef _G_ADMIN_H
#define _G_ADMIN_H

#define AP(x) trap_SendServerCommand(-1, x)
#define CP(x) trap_SendServerCommand(ent-g_entities, x)
#define CPx(x, y) trap_SendServerCommand(x, y)
#define ADMP(x) G_admin_print(ent, x)
#define ADMBP(x) G_admin_buffer_print(ent, x)
#define ADMBP_begin() G_admin_buffer_begin()
#define ADMBP_end() G_admin_buffer_end(ent)

#define MAX_ADMIN_LEVELS 1024
#define MAX_ADMIN_ADMINS 2048
#define MAX_ADMIN_BANS 2048
#define MAX_ADMIN_HIDES 2048
#define MAX_ADMIN_NAMELOGS 128
#define MAX_ADMIN_NAMELOG_NAMES 5
#define MAX_ADMIN_FLAGS 256
#define MAX_ADMIN_COMMANDS 128
#define MAX_ADMIN_CMD_LEN 400
#define MAX_ADMIN_BAN_REASON 80
#define MAX_ADMIN_HIDE_REASON 80

/*
 * 1   - cannot be vote kicked, vote muted
 * 2   - cannot be censored or flood protected
 * 3   - never loses credits for changing teams
 * 4   - can see team chat as a spectator
 * 5   - can switch teams any time, regardless of balance
 * 6   - does not need to specify a reason for a kick/ban
 * 7   - can call a vote at any time (regardless of a vote being disabled or
 *        voting limitations)
 * 8   - does not need to specify a duration for a ban
 * 9   - can run commands from team chat
 * 0   - inactivity rules do not apply to them
 *
 * !   - admin commands cannot be used on them *
 * @   - does not show up as an admin in !listplayers *
 * $   - sees all information in !listplayers *
 * #   - permanent designated builder *
 * )   - overrides ping limitiations *
 * ?   - can see admin chat *
 * &   - will not be booted in maintenance mode *
 * (   - has extensive admin control *
 *
 * b   - bans
 * c   - client control, primarily hide (hide, unhide, etc)
 * d   - can fix deconstructions (buildlog & revert)
 * h   - help
 * i   - inactive commands (spec999, etc)
 * k   - can kick clients
 * l   - listplayers
 * m   - map commands (map, restart, devmap, nextmap, etc)
 * o   - oc scrim management
 * r   - register
 * s   - chat commands (say, etc)
 * v   - vote commands (passvote, cancelvote, etc)
 * w   - warning / notification commands (warn, etc)
 * K   - lock commands (lock, unlock, etc)
 * L   - listlayouts
 * M   - mute commands (mute, unmute, etc)
 * P   - putteam
 * R   - name commands (rename, etc)
 * S   - administration commands (setlevel, giveflag, removeflag, etc)
 * ^b  - control building rights
 * ^c  - can cheat in cheat mdoe (devmap / editoc mode)
 * ^h  - basic commands (specme, info, time, etc)
 * ^l  - can edit oc's
 * ^p  - can pause
 * ^r  - configuration commands (readconfig, etc)
 * ^s  - extended chat commands (bigsay, adminsay, etc)
 * ^L  - listadmins
 * ^P  - putteam extension
 * ^^c - has extensive cheat control (cheat-set, etc)
 * ^^l - namelog
 * ^^L - can layoutsave without review
 * ^^^a - can use potentially abusive commands (crash, etc)
 *
 * *: wildcard '*' does not enable this
 */

#define ADMF_IMMUNITY "1"
#define ADMF_NOCENSORFLOOD "2"
#define ADMF_TEAMCHANGEFREE "3"
#define ADMF_SPEC_ALLCHAT "4"
#define ADMF_FORCETEAMCHANGE "5"
#define ADMF_UNACCOUNTABLE "6"
#define ADMF_NO_VOTE_LIMIT "7"
#define ADMF_CAN_PERM_BAN "8"
#define ADMF_TEAMCHAT_CMD "9"
#define ADMF_ACTIVITY "0"

#define ADMF_IMMUTABLE "!"
#define ADMF_INCOGNITO "@"
#define ADMF_SEESFULLLISTPLAYERS "$"
#define ADMF_DBUILDER "#"
#define ADMF_PINGOVERRIDE ")"
#define ADMF_ADMINCHAT "?"
#define ADMF_MAINTAIN "&"
#define ADMF_EXTENSIVEADMINCONTROL "("

#define ADMF_LAYOUTEDIT "^l"
#define ADMF_PUTTEAMEXT "^P"

#define MAX_ADMIN_LISTITEMS 20
#define MAX_ADMIN_SHOWBANS 10
#define MAX_ADMIN_SHOWHIDES 10

// important note: QVM does not seem to allow a single char to be a
// member of a struct at init time.  flag has been converted to char*
typedef struct
{
  char *keyword;
  qboolean ( * handler ) ( gentity_t *ent, int skiparg );
  char *flag;
  char *function;  // used for !help
  char *syntax;  // used for !help
}
g_admin_cmd_t;

typedef struct g_admin_level
{
  int level;
  char name[ MAX_NAME_LENGTH ];
  char flags[ MAX_ADMIN_FLAGS ];
}
g_admin_level_t;

typedef struct g_admin_admin
{
  char guid[ 33 ];
  char name[ MAX_NAME_LENGTH ];
  int level;
  char flags[ MAX_ADMIN_FLAGS ];
}
g_admin_admin_t;

typedef struct g_admin_ban
{
  char name[ MAX_NAME_LENGTH ];
  char guid[ 33 ];
  char ip[ 18 ];
  char reason[ MAX_ADMIN_BAN_REASON ];
  char made[ 18 ]; // big enough for strftime() %c
  int expires;
  char banner[ MAX_NAME_LENGTH ];
}
g_admin_ban_t;

typedef struct g_admin_hide
{
  char name[ MAX_NAME_LENGTH ];
  char guid[ 33 ];
  char ip[ 18 ];
  char reason[ MAX_ADMIN_HIDE_REASON ];
  char made[ 18 ]; // big enough for strftime() %c
  int expires;
  char hider[ MAX_NAME_LENGTH ];
  int hidden;
}
g_admin_hide_t;

typedef struct g_admin_command
{
  char command[ MAX_ADMIN_CMD_LEN ];
  char exec[ MAX_QPATH ];
  char desc[ 50 ];
  int levels[ MAX_ADMIN_LEVELS + 1 ];
}
g_admin_command_t;

typedef struct g_admin_namelog
{
  char      name[ MAX_ADMIN_NAMELOG_NAMES ][MAX_NAME_LENGTH ];
  char      ip[ 16 ];
  char      guid[ 33 ];
  int       slot;
  qboolean  banned;
}
g_admin_namelog_t;

qboolean G_admin_hide_check( char *userinfo, char *reason, int rlen, int *hidden, int *hiddenTime, int *id );
qboolean G_admin_ban_check( char *userinfo, char *reason, int rlen );
qboolean G_admin_cmd_check( gentity_t *ent, qboolean say );
qboolean G_admin_readconfig( gentity_t *ent, int skiparg );
qboolean G_admin_permission( gentity_t *ent, char *flag );
qboolean G_admin_name_check( gentity_t *ent, char *name, char *err, int len, int testUnnamed );
void G_admin_namelog_update( gclient_t *ent, qboolean disconnect );
int G_admin_level( gentity_t *ent );

// ! command functions
qboolean G_admin_time( gentity_t *ent, int skiparg );
qboolean G_admin_setlevel( gentity_t *ent, int skiparg );
qboolean G_admin_kick( gentity_t *ent, int skiparg );
qboolean G_admin_adjustban( gentity_t *ent, int skiparg );
qboolean G_admin_adjusthide( gentity_t *ent, int skiparg );
qboolean G_admin_ban( gentity_t *ent, int skiparg );
qboolean G_admin_unban( gentity_t *ent, int skiparg );
qboolean G_admin_putteam( gentity_t *ent, int skiparg );
qboolean G_admin_listadmins( gentity_t *ent, int skiparg );
qboolean G_admin_listlayouts( gentity_t *ent, int skiparg );
qboolean G_admin_listplayers( gentity_t *ent, int skiparg );
qboolean G_admin_map( gentity_t *ent, int skiparg );
qboolean G_admin_devmap( gentity_t *ent, int skiparg );
qboolean G_admin_layoutsave( gentity_t *ent, int skiparg );
qboolean G_admin_mute( gentity_t *ent, int skiparg );
qboolean G_admin_hide( gentity_t *ent, int skiparg );
qboolean G_admin_denybuild( gentity_t *ent, int skiparg );
qboolean G_admin_showbans( gentity_t *ent, int skiparg );
qboolean G_admin_showhides( gentity_t *ent, int skiparg );
qboolean G_admin_help( gentity_t *ent, int skiparg );
qboolean G_admin_admintest( gentity_t *ent, int skiparg );
qboolean G_admin_allready( gentity_t *ent, int skiparg );
qboolean G_admin_cancelvote( gentity_t *ent, int skiparg );
qboolean G_admin_passvote( gentity_t *ent, int skiparg );
qboolean G_admin_spec999( gentity_t *ent, int skiparg );
qboolean G_admin_register( gentity_t *ent, int skiparg );
qboolean G_admin_rename( gentity_t *ent, int skiparg );
qboolean G_admin_restart( gentity_t *ent, int skiparg );
qboolean G_admin_nextmap( gentity_t *ent, int skiparg );
qboolean G_admin_namelog( gentity_t *ent, int skiparg );
qboolean G_admin_lock( gentity_t *ent, int skiparg );
qboolean G_admin_unlock( gentity_t *ent, int skiparg );
qboolean G_admin_pause( gentity_t *ent, int skiparg );
qboolean G_admin_putmespec( gentity_t *ent, int skiparg );
qboolean G_admin_designate( gentity_t *ent, int skiparg );
qboolean G_admin_buildlog( gentity_t *ent, int skiparg );
qboolean G_admin_revert( gentity_t *ent, int skiparg );
qboolean G_admin_hbp( gentity_t *ent, int skiparg );
qboolean G_admin_abp( gentity_t *ent, int skiparg );
qboolean G_admin_hs( gentity_t *ent, int skiparg );
qboolean G_admin_as( gentity_t *ent, int skiparg );
qboolean G_admin_giveall( gentity_t *ent, int skiparg );
qboolean G_admin_god( gentity_t *ent, int skiparg );
qboolean G_admin_kill( gentity_t *ent, int skiparg );
qboolean G_admin_noclip( gentity_t *ent, int skiparg );
qboolean G_admin_notarget( gentity_t *ent, int skiparg );
qboolean G_admin_say( gentity_t *ent, int skiparg );
qboolean G_admin_adminsay( gentity_t *ent, int skiparg );
qboolean G_admin_bigsay( gentity_t *ent, int skiparg );
qboolean G_admin_info( gentity_t *ent, int skiparg );
qboolean G_admin_warn( gentity_t *ent, int skiparg );
qboolean G_admin_editoc( gentity_t *ent, int skiparg );
qboolean G_admin_override( gentity_t *ent, int skiparg );
qboolean G_admin_crash( gentity_t *ent, int skiparg );
qboolean G_admin_putscrimteam( gentity_t *ent, int skiparg );
qboolean G_admin_endscrim( gentity_t *ent, int skiparg );
qboolean G_admin_startscrim( gentity_t *ent, int skiparg );
qboolean G_admin_flag( gentity_t *ent, int skiparg );
qboolean G_admin_setCheat( gentity_t *ent, int skiparg );
qboolean G_admin_speed( gentity_t *ent, int skiparg );

qboolean G_admin_canEditOC( gentity_t *ent );

void G_admin_print( gentity_t *ent, char *m );
void G_admin_buffer_print( gentity_t *ent, char *m );
void G_admin_buffer_begin( void );
void G_admin_buffer_end( gentity_t *ent );

void G_admin_duration( int secs, char *duration, int dursize );
void G_admin_cleanup( void );
void G_admin_namelog_cleanup( void );

extern g_admin_level_t *g_admin_levels[ MAX_ADMIN_LEVELS ];
extern g_admin_admin_t *g_admin_admins[ MAX_ADMIN_ADMINS ];
extern g_admin_ban_t *g_admin_bans[ MAX_ADMIN_BANS ];
extern g_admin_hide_t *g_admin_hides[ MAX_ADMIN_HIDES ];
extern g_admin_command_t *g_admin_commands[ MAX_ADMIN_COMMANDS ];
extern g_admin_namelog_t *g_admin_namelog[ MAX_ADMIN_NAMELOGS ];

#endif /* ifndef _G_ADMIN_H */
