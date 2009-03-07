/*
===========================================================================
Copyright (C) 2004-2006 Tony J. White

This file is part of Tremulous.

This shrubbot implementation is the original work of Tony J. White.

Contains contributions from Wesley van Beelen, Chris Bajumpaa, Josh Menke,
and Travis Maurer.

The functionality of this code mimics the behaviour of the currently
inactive project shrubet (http://www.etstats.com/shrubet/index.php?ver=2)
by Ryan Mannion.   However, shrubet was a closed-source project and
none of it's code has been copied, only it's functionality.

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

// TODO: a cleaner g_admin.[ch] not based off of a 1.1-ish g_admin.[ch]

#include "g_local.h"

// big ugly global buffer for use with buffered printing of long outputs
static char g_bfb[ 32000 ];

// note: list ordered alphabetically
g_admin_cmd_t g_admin_cmds[ ] =
  {
    {"adjustban", G_admin_adjustban, "b",
      "change the duration or reason of a ban.  time is specified as numbers "
      "followed by units 'w' (weeks), 'd' (days), 'h' (hours) or 'm' (minutes),"
      " or seconds if no units are specified",
      "[^3ban#^7] (^5time^7) (^5reason^7)"
    },

    {"adjusthide", G_admin_adjusthide, "c",
      "change the duration, hidden or reason of a hide.  time is specified as numbers "
      "followed by units 'w' (weeks), 'd' (days), 'h' (hours) or 'm' (minutes),"
      " or seconds if no units are specified - if hidden is only"
      " arg, add c as first arg."
      " For example, to enable isHidden for slot #4, use"
      " !adjusthide 4 c 1.  The 0 does not represent anything",
      "[^3ban#^7] (^5time^7) (^5chidden hidden^7) (^5reason^7)"
    },

    {"admintest", G_admin_admintest, "^h",
      "display your current admin level",
      ""
    },

    {"adminsay", G_admin_adminsay, "^s",
      "assert your authority using chat",
      "[^5message^7]"
    },

    {"allowbuild", G_admin_denybuild, "^b",
      "restore a player's ability to build",
      "[^3name|slot#^7]"
    },

    {"allready", G_admin_allready, "^r",
      "makes everyone ready in intermission",
      ""
    },

    {"ban", G_admin_ban, "b",
      "ban a player by IP and GUID with an optional expiration time and reason."
      "  time is specified as numbers followed by units 'w' (weeks), 'd' "
      "(days), 'h' (hours) or 'm' (minutes), or seconds if no units are "
      "specified",
      "[^3name|slot#|IP^7] (^5time^7) (^5reason^7)"
    },

    {"bigsay", G_admin_bigsay, "^s",
      "assert your authority using chat",
      "[^5message^7]"
    },

    {"cancelvote", G_admin_cancelvote, "v",
      "cancel a vote taking place",
      ""
    },

    {"endscrim", G_admin_endscrim, "c",
      "end an oc scrim",
      ""
    },

    {"cheat-abp", G_admin_abp, "^c",
      "set alien build points - requires cheats",
      "[^5bp^7]"
    },

    {"cheat-as", G_admin_as, "^c",
      "set alien stage - requires cheats",
      "[^5stage^7]"
    },

    {"cheat-ao", G_admin_override, "^c",
      "override checks for various things",
      "(^5name|slot^7)"
    },

    {"cheat-do", G_admin_override, "^c",
      "deny override",
      "(^5name|slot^7)"
    },

    {"cheat-hbp", G_admin_hbp, "^c",
      "set alien build points - requires cheats"
      "[^5bp^7]"
    },

    {"cheat-hs", G_admin_hs, "^c",
      "set human stage - requires cheats",
      "[^5stage^7]"
    },

    {"cheat-give-all", G_admin_giveall, "^c",
      "give all - requires cheats",
      ""
    },

    {"cheat-giveall", G_admin_giveall, "^c",
      "give all - requires cheats",
      ""
    },

    {"cheat-god", G_admin_god, "^c",
      "god - requires cheats",
      ""
    },

    {"cheat-kill", G_admin_kill, "^c",
      "kill - requires cheats",
      ""
    },

    {"cheat-noclip", G_admin_noclip, "^c",
      "noclip - requires cheats",
      ""
    },

    {"cheat-set", G_admin_setCheat, "^^c",
      "set cheat - requires cheats",
      "[^3name|slot#^7]"
    },

    {"cheat-speed", G_admin_speed, "^c",
      "speed - requires cheats",
      ""
    },

    {"cheat-notarget", G_admin_notarget, "^c",
      "notarget - requires cheats",
      ""
    },

    {"cheat-unset", G_admin_setCheat, "^^c",
      "set cheat - requires cheats",
      "[^3name|slot#^7]"
    },

    {"crash", G_admin_crash, "^^^a",
      "crash a client",
      "[^5stage^7]"
    },

    {"denybuild", G_admin_denybuild, "^b",
      "take away a player's ability to build",
      "[^3name|slot#^7]"
    },

//    {"designate", G_admin_designate, "^b",
//      "give the player designated builder privileges",
//      "[^3name|slot#^7]"
//    },

    {"editoc", G_admin_editoc, ADMF_LAYOUTEDIT,
      "Edit an obstacle course",
      "[mode - 0/1/2/off/allwithcallerlevelplus/all]"
    },

    {"flag", G_admin_flag, "S",
      "Handle a player's flags",
      "[^3name|slot#^7|admin#^7] (^3flag^7)"
    },

    {"giveflag", G_admin_flag, "S",
      "Give a player a flag",
      "[^3name|slot#^7|admin#^7] (^3flag^7)"
    },

    {"help", G_admin_help, "h",
      "display commands available to you or help on a specific command",
      "(^5command^7)"
    },

    {"hide", G_admin_hide, "c",
      "hide a player",
      "[^3name|slot#^7] (^5time^7)"
    },

    {"info", G_admin_info, "^h",
      "display the contents of server info files",
      "(^5subject^7)"
    },

    {"kick", G_admin_kick, "k",
      "kick a player with an optional reason",
      "(^5reason^7)"
    },

    {"layoutsave", G_admin_layoutsave, "^^L",
      "save a map layout",
      "[^3mapname^7]"
    },

    {"layoutsavereview", G_admin_layoutsave, ADMF_LAYOUTEDIT,
      "save a map layout without a queue limit",
      "[^3mapname^7]"
    },

    {"layoutsave-review", G_admin_layoutsave, ADMF_LAYOUTEDIT,
      "save a map layout without a queue limit",
      "[^3mapname^7]"
    },

    {"layoutsave_review", G_admin_layoutsave, ADMF_LAYOUTEDIT,
      "save a map layout without a queue limit",
      "[^3mapname^7]"
    },

    {"listadmins", G_admin_listadmins, "^L",
      "display a list of all server admins and their levels",
      "(^5name|start admin#^7)"
    },

    {"listlayouts", G_admin_listlayouts, "L",
      "display a list of all available layouts for a map",
      "(^5mapname^7)"
    },

    {"listplayers", G_admin_listplayers, "l",
      "display a list of players, their client numbers and their levels",
      ""
    },

    {"lock", G_admin_lock, "K",
      "lock a team to prevent anyone from joining it",
      "[^3a|h^7]"
    },

    {"map", G_admin_map, "m",
      "load a map (and optionally force layout)",
      "[^3mapname^7] (^5layout^7)"
    },

    {"devmap", G_admin_devmap, "m",
      "load a map with cheats (and optionally force layout)",
      "[^3mapname^7] (^5layout^7)"
    },

    {"mute", G_admin_mute, "M",
      "mute a player",
      "[^3name|slot#^7]"
    },

    {"namelog", G_admin_namelog, "^^l",
      "display a list of names used by recently connected players",
      "(^5name^7)"
    },

    {"nextmap", G_admin_nextmap, "m",
      "go to the next map in the cycle",
      ""
    },

    {"passvote", G_admin_passvote, "v",
      "pass a vote currently taking place",
      ""
    },

    {"pause", G_admin_pause, "^p",
      "Pause (or unpause) the game.",
      ""
    },

    {"putteam", G_admin_putteam, "P",
      "move a player to a specified team for an optional duration",
      "[^3name|slot#^7] [^3h|a|s^7] (^3duration^7)"
    },

    {"putscrimteam", G_admin_putscrimteam, "o",
      "move a player to a specified obstacle course team",
      "[^3name|slot#^7] [^3teamname/0fornoteam^7]"
    },

    {"readconfig", G_admin_readconfig, "^r",
      "reloads the admin config file and refreshes  flags",
      ""
    },

    {"register", G_admin_register, "r",
      "Registers your name to protect it from being used by others or updates your admin name to your current name.",
      ""
    },

    {"removeflag", G_admin_flag, "S",
      "Remove a player a flag",
      "[^3name|slot#^7|admin#^7] (^3flag^7)"
    },

    {"rename", G_admin_rename, "R",
      "rename a player",
      "[^3name|slot#^7] [^3new name^7]"
    },

    {"restart", G_admin_restart, "m",
      "restart the current map (optionally using named layout or keeping/switching teams)",
      "(^5layout^7) (^5keepteams|switchteams|keepteamslock|switchteamslock^7)"
    },

    {"say", G_admin_say, "s",
      "assert your authority using chat",
      "[^5message^7]"
    },

    {"setlevel", G_admin_setlevel, "S",
      "sets the admin level of a player",
      "[^3name|slot#|admin#^7] [^3level^7]"
    },

    {"showbans", G_admin_showbans, "b",
      "display a (partial) list of active bans",
      "(^5start at ban#^7)"
    },

    {"showhides", G_admin_showhides, "c",
      "display a (partial) list of active hides",
      "(^5start at ban#^7)"
    },

    {"spec999", G_admin_spec999, "i",
      "move 999 pingers to the spectator team",
      ""},

    {"specme", G_admin_putmespec, "^h",
     "moves you to the spectators",
     ""
    },

    {"startscrim", G_admin_startscrim, "o",
     "starts a scrim all medis or armoury win",
     "[m/a]"
    },

    {"time", G_admin_time, "^h",
      "show the current local server time",
     ""
    },

    {"unban", G_admin_unban, "b",
      "unbans a player specified by the slot as seen in showbans",
      "[^3ban#^7]"
    },

    {"unlock", G_admin_unlock, "K",
      "unlock a locked team",
      "[^3a|h^7]"
    },

    {"unhide", G_admin_hide, "c",
      "unhide a hidden player",
      "[^3name|slot#^7]"
    },

//    {"undesignate", G_admin_designate, "^b",
//      "revoke designated builder privileges",
//      "[^3name|slot#^7]"
//    },

    {"unmute", G_admin_mute, "M",
      "unmute a muted player",
      "[^3name|slot#^7]"
    },

    {
     "warn", G_admin_warn, "w",
      "Warn a player to cease or face admin intervention",
      "[^3name|slot#^7] [reason]"
    }
  };

static int adminNumCmds = sizeof( g_admin_cmds ) / sizeof( g_admin_cmds[ 0 ] );

static int admin_level_maxname = 0;
g_admin_level_t *g_admin_levels[ MAX_ADMIN_LEVELS ];
g_admin_admin_t *g_admin_admins[ MAX_ADMIN_ADMINS ];
g_admin_ban_t *g_admin_bans[ MAX_ADMIN_BANS ];
g_admin_hide_t *g_admin_hides[ MAX_ADMIN_HIDES ];
g_admin_command_t *g_admin_commands[ MAX_ADMIN_COMMANDS ];
g_admin_namelog_t *g_admin_namelog[ MAX_ADMIN_NAMELOGS ];

qboolean G_admin_permission( gentity_t *ent, char *flag )
{
  int i;
  int l = 0;
  char *flags;
  char *flag1;

  // console always wins
  if( !ent )
    return qtrue;

  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( ent->client->pers.guid, g_admin_admins[ i ]->guid ) )
    {
      flags = g_admin_admins[ i ]->flags;
      while( *flags )
      {
        if( *flags == *flag )
        {
          if( *flag != '^' )
            return qtrue;
          flag1 = flag + 1;
          flags++;
          while ( *flags == *flag1 && *flags == '^' )
          {
            flags++;
            flag1++;
          }
          if ( *flags++ == *flag1 )
            return qtrue;
          else
            continue;
        }
        else if( *flags == '-' )
        {
            flags++;
            if( *flags == *flag )
            {
              if ( *flag != '^' )
                return qfalse;
              flag1 = flag + 1;
              flags++;
              while ( *flags == *flag1 && *flags == '^' )
              {
                flags++;
                flag1++;
              }
              if ( *flags++ == *flag1 )
                return qfalse;
              else
                continue;
            }
        }
        else if( *flags == '*' )
        {
          // flags with significance only for individuals (
          // like ADMF_INCOGNITO and ADMF_IMMUTABLE are NOT covered
          // by the '*' wildcard.  They must be specified manually.
          if ( !Q_stricmp( flag, ADMF_INCOGNITO ) || !Q_stricmp( flag, ADMF_IMMUTABLE ) || !Q_stricmp( flag, ADMF_SEESFULLLISTPLAYERS ) || !Q_stricmp( flag, ADMF_DBUILDER ) || !Q_stricmp( flag, ADMF_ADMINCHAT ) || !Q_stricmp( flag, ADMF_MAINTAIN ) || !Q_stricmp( flag, ADMF_EXTENSIVEADMINCONTROL ) || !Q_stricmp( flag, ADMF_PINGOVERRIDE ) )
            return qfalse;
          else
            return qtrue;
        }
        flags++;
      }
      l = g_admin_admins[ i ]->level;
    }
  }
  for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
  {
    if( g_admin_levels[ i ]->level == l )
    {
      flags = g_admin_levels[ i ]->flags;
      while( *flags )
      {
        if( *flags == *flag )
        {
          if ( *flag != '^' )
            return qtrue;
           flag1 = flag + 1;
           flags++;
          while ( *flags == *flag1 && *flags == '^' )
          {
            flags++;
            flag1++;
          }
          if ( *flags == *flag1 )
            return qtrue;
          else
            continue;
        }
        if( *flags == '*' )
        {
          // flags with significance only for individuals (
          // like ADMF_INCOGNITO and ADMF_IMMUTABLE are NOT covered
          // by the '*' wildcard.  They must be specified manually.
          if ( !Q_stricmp( flag, ADMF_INCOGNITO ) || !Q_stricmp( flag, ADMF_IMMUTABLE ) || !Q_stricmp( flag, ADMF_SEESFULLLISTPLAYERS ) || !Q_stricmp( flag, ADMF_DBUILDER ) || !Q_stricmp( flag, ADMF_ADMINCHAT ) || !Q_stricmp( flag, ADMF_MAINTAIN ) || !Q_stricmp( flag, ADMF_EXTENSIVEADMINCONTROL ) || !Q_stricmp( flag, ADMF_PINGOVERRIDE ) )
            return qfalse;
          else
            return qtrue;
        }
        flags++;
      }
    }
  }
  return qfalse;
}

//qboolean G_admin_name_check( gentity_t *ent, char *name, char *err, int len, int testUnnamed )
qboolean G_admin_name_check( gentity_t *ent, char *name, char *err, int len )
{
  int i;
  //gclient_t *client;
  char testName[ MAX_NAME_LENGTH ] = {""};
  char name2[ MAX_NAME_LENGTH ] = {""};
  char namePrefix[ MAX_NAME_LENGTH - 4 ];
  char namePrefix2[ MAX_NAME_LENGTH - 4 ];
  int alphaCount = 0;
  qboolean unnamed = qtrue;

  G_SanitiseString( name, name2, sizeof( name2 ) );

//  if( !Q_stricmp( name2, "UnnamedPlayer" ) && !testUnnamed )
//    return qtrue;

  if( !Q_stricmp( name2, "console" ) )
  {
    Q_strncpyz( err, va( "The name '%s^7' is invalid here", name2 ),
      len );
    return qfalse;
  }

  if( !Q_stricmp( name2, "name" ) )  // name might conflict with stats
  {
    Q_strncpyz( err, va( "The name '%s^7' is invalid here", name2 ),
      len );
    return qfalse;
  }

  if( !Q_stricmp( name2, "noname" ) )  // noname might conflict with stats
  {
    Q_strncpyz( err, va( "The name '%s^7' is invalid here", name2 ),
      len );
    return qfalse;
  }

  //if( !testUnnamed )
  //{
      //for( i = 0; i < level.maxclients; i++ )
      //{
        //client = &level.clients[ i ];
        //if( client->pers.connected != CON_CONNECTING
          //&& client->pers.connected != CON_CONNECTED )
        //{
          //continue;
        //}
//
        //// can rename ones self to the same name using different colors
        //if( i == ( ent - g_entities ) )
          //continue;
//
        //////////////////////////////////////G_SanitiseString( client->pers.netname, testName, sizeof( testName ) );
        //if( !Q_stricmp( name2, testName ) && !testUnnamed )
        //{
          //////////////////////////////////////Q_strncpyz( err, va( "The name '%s^7' is already in use", name ),
            //len );
          //return qfalse;
        //}
      //}
  //}

  if( '0' <= name2[ 0 ]  && name2[ 0 ] <= '9' )
  {
    Q_strncpyz( err, "Names cannot begin with a number. Please choose another.", len );
    return qfalse;
  }

  for( i = 0; name2[ i ] !='\0'; i++)
  {
    if( Q_isalpha( name2[ i ] ) )
     alphaCount++;
    if( name2[ i ] == '/' )
    {
      if( name2[ i + 1 ] == '/' || name2[ i + 1 ] == '*' )
      {
        Q_strncpyz( err, "Names cannot contain '//' or '/*'. Please choose another.", len );
        return qfalse;
      }
    }
    if( name2[ i ] == ';' )
    {
      Q_strncpyz( err, "Names cannot contain 'SEMICOLON'. Please choose another.", len );
      return qfalse;
    }
  }

  if( alphaCount == 0 )
  {
    Q_strncpyz( err, va( "The name '%s^7' does not include at least one letter. Please choose another.", name ), len );
    return qfalse;
  }

//  if( g_newbieNamePrefix.string[ 0 ] )
//    Q_strncpyz( namePrefix, g_newbieNamePrefix.string , sizeof( namePrefix ) );
//  else
//    strcpy( namePrefix, "Newbie#" );

  G_SanitiseString( namePrefix, namePrefix2, sizeof( namePrefix2 ) );

  i = 0;

  while( i < strlen( namePrefix2 ) && i < strlen( name2 ) )
  {
    if( namePrefix2[ i ] != name2[ i ] )
    {
      unnamed = qfalse;
      break;
    }
    i++;
  }

//  if( testUnnamed && unnamed )
//  {
//    Q_strncpyz( err, va( "The name '%s^7' }.", name ), len );
//    return qfalse;
//  }

//  if( !g_adminNameProtect.string[ 0 ] || testUnnamed )
  if( !g_adminNameProtect.string[ 0 ] )
    return qtrue;

  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( g_admin_admins[ i ]->level < 1 )
      continue;
    G_SanitiseString( g_admin_admins[ i ]->name, testName, sizeof( testName ) );
    if( !Q_stricmp( name2, testName ) &&
      Q_stricmp( ent->client->pers.guid, g_admin_admins[ i ]->guid ) )
    {
      Q_strncpyz( err, va( "The name '%s^7' belongs to an admin. "
        "Please choose another.", name ), len );
      return qfalse;
    }
  }
  return qtrue;
}

static qboolean admin_higher_guid( char *admin_guid, char *victim_guid )
{
  int i;
  int alevel = 0;

  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( admin_guid, g_admin_admins[ i ]->guid ) )
    {
      alevel = g_admin_admins[ i ]->level;
      break;
    }
  }
  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( victim_guid, g_admin_admins[ i ]->guid ) )
    {
      if( alevel < g_admin_admins[ i ]->level )
        return qfalse;
      if( strstr( g_admin_admins[ i ]->flags, va( "%s", ADMF_IMMUTABLE ) ) )
        return qfalse;
    }
  }
  return qtrue;
}

static qboolean admin_higher( gentity_t *admin, gentity_t *victim )
{

  // console always wins
  if( !admin )
    return qtrue;
  // just in case
  if( !victim )
    return qtrue;

  return admin_higher_guid( admin->client->pers.guid,
    victim->client->pers.guid );
}

static void admin_writeconfig_string( char *s, fileHandle_t f )
{
  char buf[ MAX_STRING_CHARS ];

  buf[ 0 ] = '\0';
  if( s[ 0 ] )
  {
    //Q_strcat(buf, sizeof(buf), s);
    Q_strncpyz( buf, s, sizeof( buf ) );
    trap_FS_Write( buf, strlen( buf ), f );
  }
  trap_FS_Write( "\n", 1, f );
}

static void admin_writeconfig_int( int v, fileHandle_t f )
{
  char buf[ 64 ];

  Com_sprintf( buf, sizeof(buf), "%d", v );
  if( buf[ 0 ] )
    trap_FS_Write( buf, strlen( buf ), f );
  trap_FS_Write( "\n", 1, f );
}

static void admin_writeconfig( void )
{
  fileHandle_t f;
  int len, i, j;
  int t;
  char levels[ MAX_STRING_CHARS ] = {""};

  if( !g_admin.string[ 0 ] )
  {
    G_Printf( S_COLOR_YELLOW "WARNING: g_admin is not set. "
      " configuration will not be saved to a file.\n" );
    return;
  }
  t = trap_RealTime( NULL );
  len = trap_FS_FOpenFile( g_admin.string, &f, FS_WRITE );
  if( len < 0 )
  {
    G_Printf( "admin_writeconfig: could not open g_admin file \"%s\"\n",
              g_admin.string );
    return;
  }
  for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
  {
    trap_FS_Write( "[level]\n", 8, f );
    trap_FS_Write( "level   = ", 10, f );
    admin_writeconfig_int( g_admin_levels[ i ]->level, f );
    trap_FS_Write( "name    = ", 10, f );
    admin_writeconfig_string( g_admin_levels[ i ]->name, f );
    trap_FS_Write( "flags   = ", 10, f );
    admin_writeconfig_string( g_admin_levels[ i ]->flags, f );
    trap_FS_Write( "\n", 1, f );
  }
  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    // don't write level 0 users
    if( g_admin_admins[ i ]->level == 0 )
      continue;

    trap_FS_Write( "[admin]\n", 8, f );
    trap_FS_Write( "name    = ", 10, f );
    admin_writeconfig_string( g_admin_admins[ i ]->name, f );
    trap_FS_Write( "guid    = ", 10, f );
    admin_writeconfig_string( g_admin_admins[ i ]->guid, f );
    trap_FS_Write( "level   = ", 10, f );
    admin_writeconfig_int( g_admin_admins[ i ]->level, f );
    trap_FS_Write( "flags   = ", 10, f );
    admin_writeconfig_string( g_admin_admins[ i ]->flags, f );
    trap_FS_Write( "\n", 1, f );
  }
  for( i = 0; i < MAX_ADMIN_BANS && g_admin_bans[ i ]; i++ )
  {
    // don't write expired bans
    // if expires is 0, then it's a perm ban
    if( g_admin_bans[ i ]->expires != 0 &&
         ( g_admin_bans[ i ]->expires - t ) < 1 )
      continue;

    trap_FS_Write( "[ban]\n", 6, f );
    trap_FS_Write( "name    = ", 10, f );
    admin_writeconfig_string( g_admin_bans[ i ]->name, f );
    trap_FS_Write( "guid    = ", 10, f );
    admin_writeconfig_string( g_admin_bans[ i ]->guid, f );
    trap_FS_Write( "ip      = ", 10, f );
    admin_writeconfig_string( g_admin_bans[ i ]->ip, f );
    trap_FS_Write( "reason  = ", 10, f );
    admin_writeconfig_string( g_admin_bans[ i ]->reason, f );
    trap_FS_Write( "made    = ", 10, f );
    admin_writeconfig_string( g_admin_bans[ i ]->made, f );
    trap_FS_Write( "expires = ", 10, f );
    admin_writeconfig_int( g_admin_bans[ i ]->expires, f );
    trap_FS_Write( "banner  = ", 10, f );
    admin_writeconfig_string( g_admin_bans[ i ]->banner, f );
    trap_FS_Write( "\n", 1, f );
  }
  for( i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]; i++ )
  {
    // don't write expired hides
    // if expires is 0, then it's a perm hide
    if( g_admin_hides[ i ]->expires != 0 &&
         ( g_admin_hides[ i ]->expires - t ) < 1 )
      continue;

    trap_FS_Write( "[hide]\n", 7, f );
    trap_FS_Write( "name    = ", 10, f );
    admin_writeconfig_string( g_admin_hides[ i ]->name, f );
    trap_FS_Write( "guid    = ", 10, f );
    admin_writeconfig_string( g_admin_hides[ i ]->guid, f );
    trap_FS_Write( "ip      = ", 10, f );
    admin_writeconfig_string( g_admin_hides[ i ]->ip, f );
    trap_FS_Write( "reason  = ", 10, f );
    admin_writeconfig_string( g_admin_hides[ i ]->reason, f );
    trap_FS_Write( "made    = ", 10, f );
    admin_writeconfig_string( g_admin_hides[ i ]->made, f );
    trap_FS_Write( "expires = ", 10, f );
    admin_writeconfig_int( g_admin_hides[ i ]->expires, f );
    trap_FS_Write( "hider   = ", 10, f );
    admin_writeconfig_string( g_admin_hides[ i ]->hider, f );
    trap_FS_Write( "hidden  = ", 10, f );
    admin_writeconfig_int( g_admin_hides[ i ]->hidden, f );
    trap_FS_Write( "\n", 1, f );
  }
  for( i = 0; i < MAX_ADMIN_COMMANDS && g_admin_commands[ i ]; i++ )
  {
    levels[ 0 ] = '\0';
    trap_FS_Write( "[command]\n", 10, f );
    trap_FS_Write( "command = ", 10, f );
    admin_writeconfig_string( g_admin_commands[ i ]->command, f );
    trap_FS_Write( "exec    = ", 10, f );
    admin_writeconfig_string( g_admin_commands[ i ]->exec, f );
    trap_FS_Write( "desc    = ", 10, f );
    admin_writeconfig_string( g_admin_commands[ i ]->desc, f );
    trap_FS_Write( "levels  = ", 10, f );
    for( j = 0; g_admin_commands[ i ]->levels[ j ] != -1; j++ )
    {
      Q_strcat( levels, sizeof( levels ),
                va( "%i ", g_admin_commands[ i ]->levels[ j ] ) );
    }
    admin_writeconfig_string( levels, f );
    trap_FS_Write( "\n", 1, f );
  }
  trap_FS_FCloseFile( f );
}

static void admin_readconfig_string( char **cnf, char *s, int size )
{
  char * t;

  //COM_MatchToken(cnf, "=");
  t = COM_ParseExt( cnf, qfalse );
  if( !strcmp( t, "=" ) )
  {
    t = COM_ParseExt( cnf, qfalse );
  }
  else
  {
    G_Printf( "readconfig: warning missing = before "
              "\"%s\" on line %d\n",
              t,
              COM_GetCurrentParseLine() );
  }
  s[ 0 ] = '\0';
  while( t[ 0 ] )
  {
    if( ( s[ 0 ] == '\0' && strlen( t ) <= size )
      || ( strlen( t ) + strlen( s ) < size ) )
    {

      Q_strcat( s, size, t );
      Q_strcat( s, size, " " );
    }
    t = COM_ParseExt( cnf, qfalse );
  }
  // trim the trailing space
  if( strlen( s ) > 0 && s[ strlen( s ) - 1 ] == ' ' )
    s[ strlen( s ) - 1 ] = '\0';
}

static void admin_readconfig_int( char **cnf, int *v )
{
  char * t;

  //COM_MatchToken(cnf, "=");
  t = COM_ParseExt( cnf, qfalse );
  if( !strcmp( t, "=" ) )
  {
    t = COM_ParseExt( cnf, qfalse );
  }
  else
  {
    G_Printf( "readconfig: warning missing = before "
              "\"%s\" on line %d\n",
              t,
              COM_GetCurrentParseLine() );
  }
  *v = atoi( t );
}

// if we can't parse any levels from readconfig, set up default
// ones to make new installs easier for admins
static void admin_default_levels( void )
{
  g_admin_level_t * l;
  int i;

  for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
  {
    BG_Free( g_admin_levels[ i ] );
    g_admin_levels[ i ] = NULL;
  }
  for( i = 0; i <= 5; i++ )
  {
    l = BG_Alloc( sizeof( g_admin_level_t ) );
    l->level = i;
    *l->name = '\0';
    *l->flags = '\0';
    g_admin_levels[ i ] = l;
  }
  Q_strncpyz( g_admin_levels[ 0 ]->name, "^4Unknown Player",
    sizeof( l->name ) );
  Q_strncpyz( g_admin_levels[ 0 ]->flags, "iahC", sizeof( l->flags ) );

  Q_strncpyz( g_admin_levels[ 1 ]->name, "^5Server Regular",
    sizeof( l->name ) );
  Q_strncpyz( g_admin_levels[ 1 ]->flags, "iahC", sizeof( l->flags ) );

  Q_strncpyz( g_admin_levels[ 2 ]->name, "^6Team Manager",
    sizeof( l->name ) );
  Q_strncpyz( g_admin_levels[ 2 ]->flags, "iahCpP", sizeof( l->flags ) );

  Q_strncpyz( g_admin_levels[ 3 ]->name, "^2Junior Admin",
    sizeof( l->name ) );
  Q_strncpyz( g_admin_levels[ 3 ]->flags, "iahCpPkm$", sizeof( l->flags ) );

  Q_strncpyz( g_admin_levels[ 4 ]->name, "^3Senior Admin",
    sizeof( l->name ) );
  Q_strncpyz( g_admin_levels[ 4 ]->flags, "iahCpPkmBbe$", sizeof( l->flags ) );

  Q_strncpyz( g_admin_levels[ 5 ]->name, "^1Server Operator",
    sizeof( l->name ) );
  Q_strncpyz( g_admin_levels[ 5 ]->flags, "*", sizeof( l->flags ) );
}

//  return a level for a player entity.
int G_admin_level( gentity_t *ent )
{
  int i;
  qboolean found = qfalse;

  if( !ent )
  {
    return MAX_ADMIN_LEVELS;
  }

  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( g_admin_admins[ i ]->guid, ent->client->pers.guid ) )
    {
      found = qtrue;
      break;
    }
  }

  if( found )
  {
    return g_admin_admins[ i ]->level;
  }

  return 0;
}

static qboolean admin_command_permission( gentity_t *ent, char *command )
{
  int i, j;
  int level;

  if( !ent )
    return qtrue;
  level  = ent->client->pers.adminLevel;
  for( i = 0; i < MAX_ADMIN_COMMANDS && g_admin_commands[ i ]; i++ )
  {
    if( !Q_stricmp( command, g_admin_commands[ i ]->command ) )
    {
      for( j = 0; g_admin_commands[ i ]->levels[ j ] != -1; j++ )
      {
        if( g_admin_commands[ i ]->levels[ j ] == level )
        {
          return qtrue;
        }
      }
    }
  }
  return qfalse;
}

static void admin_log( gentity_t *admin, char *cmd, int skiparg )
{
  fileHandle_t f;
  int len, i, j;
  char string[ MAX_STRING_CHARS ];
  int min, tens, sec;
  g_admin_admin_t *a;
  g_admin_level_t *l;
  char flags[ MAX_ADMIN_FLAGS * 2 ];
  gentity_t *victim = NULL;
  int pids[ MAX_CLIENTS ];
  char name[ MAX_NAME_LENGTH ];

  if( !g_adminLog.string[ 0 ] )
    return ;


  len = trap_FS_FOpenFile( g_adminLog.string, &f, FS_APPEND );
  if( len < 0 )
  {
    G_Printf( "admin_log: error could not open %s\n", g_adminLog.string );
    return ;
  }

  sec = level.time / 1000;
  min = sec / 60;
  sec -= min * 60;
  tens = sec / 10;
  sec -= tens * 10;

  *flags = '\0';
  if( admin )
  {
    for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
    {
      if( !Q_stricmp( g_admin_admins[ i ]->guid , admin->client->pers.guid ) )
      {

        a = g_admin_admins[ i ];
        Q_strncpyz( flags, a->flags, sizeof( flags ) );
        for( j = 0; j < MAX_ADMIN_LEVELS && g_admin_levels[ j ]; j++ )
        {
          if( g_admin_levels[ j ]->level == a->level )
          {
            l = g_admin_levels[ j ];
            Q_strcat( flags, sizeof( flags ), l->flags );
            break;
          }
        }
        break;
      }
    }
  }

  if( G_SayArgc() > 1 + skiparg )
  {
    G_SayArgv( 1 + skiparg, name, sizeof( name ) );
    if( G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) == 1 )
    {
      victim = &g_entities[ pids[ 0 ] ];
    }
  }

  if( victim && Q_stricmp( cmd, "attempted" ) )
  {
    Com_sprintf( string, sizeof( string ),
                 "%3i:%i%i: %i: %s: %s: %s: %s: %s: %s: \"%s\"\n",
                 min,
                 tens,
                 sec,
                 ( admin ) ? admin->s.clientNum : -1,
                 ( admin ) ? admin->client->pers.guid
                 : "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                 ( admin ) ? admin->client->pers.netname : "console",
                 flags,
                 cmd,
                 victim->client->pers.guid,
                 victim->client->pers.netname,
                 G_SayConcatArgs( 2 + skiparg ) );
  }
  else
  {
    Com_sprintf( string, sizeof( string ),
                 "%3i:%i%i: %i: %s: %s: %s: %s: \"%s\"\n",
                 min,
                 tens,
                 sec,
                 ( admin ) ? admin->s.clientNum : -1,
                 ( admin ) ? admin->client->pers.guid
                 : "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                 ( admin ) ? admin->client->pers.netname : "console",
                 flags,
                 cmd,
                 G_SayConcatArgs( 1 + skiparg ) );
  }
  trap_FS_Write( string, strlen( string ), f );
  trap_FS_FCloseFile( f );
  if ( !Q_stricmp( cmd, "attempted" ) )
  {
    Com_sprintf( string, sizeof( string ),
                 "%s (%i) %s: %s",
                 ( admin ) ? admin->client->pers.netname : "console",
                 ( admin ) ? admin->s.clientNum : -1,
                 cmd,
                 G_SayConcatArgs( 1 + skiparg ) );
    G_AdminMessage(NULL, "%s\n",string);
  }



  G_LogPrintf("Admin Command: %s^7: %s %s\n",( admin ) ? admin->client->pers.netname : "console", cmd, G_SayConcatArgs( 1 + skiparg ));
}

static int admin_listadmins( gentity_t *ent, int start, char *search )
{
  int drawn = 0;
  char guid_stub[9];
  char name[ MAX_NAME_LENGTH ] = {""};
  char name2[ MAX_NAME_LENGTH ] = {""};
  char lname[ MAX_NAME_LENGTH ] = {""};
  char lname_fmt[ 5 ];
  int i,j;
  gentity_t *vic;
  int l = 0;
  qboolean dup = qfalse;

  ADMBP_begin();

  // print out all connected players regardless of level if name searching
  for( i = 0; i < level.maxclients && search[ 0 ]; i++ )
  {
    vic = &g_entities[ i ];

    if( vic->client && vic->client->pers.connected != CON_CONNECTED )
      continue;

    l = vic->client->pers.adminLevel;

    G_SanitiseString( vic->client->pers.netname, name, sizeof( name ) );
    if( !strstr( name, search ) )
      continue;

    for( j = 0; j < 8; j++ )
      guid_stub[ j ] = vic->client->pers.guid[ j + 24 ];
    guid_stub[ j ] = '\0';

    lname[ 0 ] = '\0';
    Q_strncpyz( lname_fmt, "%s", sizeof( lname_fmt ) );
    for( j = 0; j < MAX_ADMIN_LEVELS && g_admin_levels[ j ]; j++ )
    {
      if( g_admin_levels[ j ]->level == l )
      {
        G_DecolorString( g_admin_levels[ j ]->name, lname, sizeof(lname) );
        Com_sprintf( lname_fmt, sizeof( lname_fmt ), "%%%is",
          ( admin_level_maxname + strlen( g_admin_levels[ j ]->name )
            - strlen( lname ) ) );
        Com_sprintf( lname, sizeof( lname ), lname_fmt,
           g_admin_levels[ j ]->name );
        break;
      }
    }
    ADMBP( va( "%4i %4i %s^7 (*%s) %s^7\n",
      i,
      l,
      lname,
      guid_stub,
      vic->client->pers.netname ) );
    drawn++;
  }

  for( i = start; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]
    && drawn < MAX_ADMIN_LISTITEMS; i++ )
  {
    if( search[ 0 ] )
    {
      G_SanitiseString( g_admin_admins[ i ]->name, name, sizeof( name ) );
      if( !strstr( name, search ) )
        continue;

      // verify we don't have the same guid/name pair in connected players
      // since we don't want to draw the same player twice
      dup = qfalse;
      for( j = 0; j < level.maxclients; j++ )
      {
        vic = &g_entities[ j ];
        if( !vic->client || vic->client->pers.connected != CON_CONNECTED )
          continue;
        G_SanitiseString( vic->client->pers.netname, name2, sizeof( name2 ) );
        if( !Q_stricmp( vic->client->pers.guid, g_admin_admins[ i ]->guid )
          && strstr( name2, search ) )
        {
          dup = qtrue;
          break;
        }
      }
      if( dup )
        continue;
    }
    for( j = 0; j < 8; j++ )
      guid_stub[ j ] = g_admin_admins[ i ]->guid[ j + 24 ];
    guid_stub[ j ] = '\0';

    lname[ 0 ] = '\0';
    Q_strncpyz( lname_fmt, "%s", sizeof( lname_fmt ) );
    for( j = 0; j < MAX_ADMIN_LEVELS && g_admin_levels[ j ]; j++ )
    {
      if( g_admin_levels[ j ]->level == g_admin_admins[ i ]->level )
      {
        G_DecolorString( g_admin_levels[ j ]->name, lname, sizeof(lname) );
        Com_sprintf( lname_fmt, sizeof( lname_fmt ), "%%%is",
          ( admin_level_maxname + strlen( g_admin_levels[ j ]->name )
            - strlen( lname ) ) );
        Com_sprintf( lname, sizeof( lname ), lname_fmt,
           g_admin_levels[ j ]->name );
        break;
      }
    }
    ADMBP( va( "%4i %4i %s^7 (*%s) %s^7\n",
      ( i + MAX_CLIENTS ),
      g_admin_admins[ i ]->level,
      lname,
      guid_stub,
      g_admin_admins[ i ]->name ) );
    drawn++;
  }
  ADMBP_end();
  return drawn;
}

void G_admin_duration( int secs, char *duration, int dursize )
{

  if( secs > ( 60 * 60 * 24 * 365 * 50 ) || secs < 0 )
    Q_strncpyz( duration, "PERMANENT", dursize );
  else if( secs >= ( 60 * 60 * 24 * 365 ) )
    Com_sprintf( duration, dursize, "%1.1f years",
      ( secs / ( 60 * 60 * 24 * 365.0f ) ) );
  else if( secs >= ( 60 * 60 * 24 * 90 ) )
    Com_sprintf( duration, dursize, "%1.1f weeks",
      ( secs / ( 60 * 60 * 24 * 7.0f ) ) );
  else if( secs >= ( 60 * 60 * 24 ) )
    Com_sprintf( duration, dursize, "%1.1f days",
      ( secs / ( 60 * 60 * 24.0f ) ) );
  else if( secs >= ( 60 * 60 ) )
    Com_sprintf( duration, dursize, "%1.1f hours",
      ( secs / ( 60 * 60.0f ) ) );
  else if( secs >= 60 )
    Com_sprintf( duration, dursize, "%1.1f minutes",
      ( secs / 60.0f ) );
  else
    Com_sprintf( duration, dursize, "%i seconds", secs );
}

qboolean G_admin_ban_check( char *userinfo, char *reason, int rlen )
{
  char *guid, *ip;
  int i;
  int t;

  *reason = '\0';
  t = trap_RealTime( NULL );
  if( !*userinfo )
    return qfalse;
  ip = Info_ValueForKey( userinfo, "ip" );
  if( !*ip )
    return qfalse;
  guid = Info_ValueForKey( userinfo, "cl_guid" );
  for( i = 0; i < MAX_ADMIN_BANS && g_admin_bans[ i ]; i++ )
  {
    // 0 is for perm ban
    if( g_admin_bans[ i ]->expires != 0 &&
         ( g_admin_bans[ i ]->expires - t ) < 1 )
      continue;
    if( strstr( ip, g_admin_bans[ i ]->ip ) )
    {
      char duration[ 32 ];
      G_admin_duration( ( g_admin_bans[ i ]->expires - t ),
        duration, sizeof( duration ) );
      Com_sprintf(
        reason,
        rlen,
        "Banned player %s (%s) tried to connnect (ban #%i by %s^7  expires %s reason: %s^7 )",
        Info_ValueForKey( userinfo, "name" ),
    g_admin_bans[ i ]->name,
        i,
        g_admin_bans[ i ]->banner,
        duration,
        g_admin_bans[ i ]->reason
      );
      G_AdminMessage(NULL, "%s\n",reason);
      Com_sprintf(
        reason,
        rlen,
        "You have been banned by %s^7 reason: %s^7 expires: %s",
        g_admin_bans[ i ]->banner,
        g_admin_bans[ i ]->reason,
        duration
      );
      G_LogPrintf("Banned player tried to connect from IP %s\n", ip);
      return qtrue;
    }
    if( *guid && !Q_stricmp( g_admin_bans[ i ]->guid, guid ) )
    {
      char duration[ 32 ];
      G_admin_duration( ( g_admin_bans[ i ]->expires - t ),
        duration, sizeof( duration ) );
      Com_sprintf(
        reason,
        rlen,
        "You have been banned by %s^7 reason: %s^7 expires: %s",
        g_admin_bans[ i ]->banner,
        g_admin_bans[ i ]->reason,
        duration
      );
      G_Printf("Banned player tried to connect with GUID %s\n", guid);
      return qtrue;
    }
  }
  return qfalse;
}

qboolean G_admin_hide_check( char *userinfo, char *reason, int rlen, int *hidden, int *hiddenTime, int *id )
{
  char *guid, *ip;
  int i;
  int t;

  if(reason)
    *reason = '\0';
  if(hidden)
    *hidden = 0;
  if(hiddenTime)
    *hiddenTime = 0;
  if(id)
    *id = 0;
  t = trap_RealTime( NULL );
  if( !*userinfo )
    return qfalse;
  ip = Info_ValueForKey( userinfo, "ip" );
  if( !*ip )
    return qfalse;
  guid = Info_ValueForKey( userinfo, "cl_guid" );
  for( i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]; i++ )
  {
    // 0 is for perm hide
    if( g_admin_hides[ i ]->expires != 0 &&
         ( g_admin_hides[ i ]->expires - t ) < 1 )
      continue;
    if(reason)
        Com_sprintf(reason, rlen, "%s", g_admin_hides[ i ]->reason);
    if(hidden)
        *hidden = g_admin_hides[ i ]->hidden;
    if(hiddenTime)
    {
        if(!g_admin_hides[ i ]->expires)
        {
            *hiddenTime = level.time + INFINITE;
            while(*hiddenTime < level.time - 10)
                (*hiddenTime)--;
        }
        else
        {
            *hiddenTime = level.time + ( g_admin_hides[ i ]->expires - t ) * 1000;
            while(*hiddenTime < level.time - 10)
                (*hiddenTime)--;
        }
    }
    if(id)
        *id = i;
    
    if( strstr( ip, g_admin_hides[ i ]->ip ) )
    {
      return qtrue;
    }
    if( *guid && !Q_stricmp( g_admin_hides[ i ]->guid, guid ) )
    {
      return qtrue;
    }
  }
  return qfalse;
}

qboolean G_admin_cmd_check( gentity_t *ent, qboolean say )
{
  int i;
  char command[ MAX_ADMIN_CMD_LEN ];
  char *cmd;
  int skip = 0;

  command[ 0 ] = '\0';
  G_SayArgv( 0, command, sizeof( command ) );
  if( !Q_stricmp( command, "say" ) ||
       ( G_admin_permission( ent, ADMF_TEAMCHAT_CMD ) &&
         ( !Q_stricmp( command, "say_team" ) ) ) )
  {
    skip = 1;
    G_SayArgv( 1, command, sizeof( command ) );
  }
  if( !command[ 0 ] )
    return qfalse;

  if( command[ 0 ] == '!' )
  {
    cmd = &command[ 1 ];
  }
  else
  {
    return qfalse;
  }

  for( i = 0; i < MAX_ADMIN_COMMANDS && g_admin_commands[ i ]; i++ )
  {
    if( Q_stricmp( cmd, g_admin_commands[ i ]->command ) )
      continue;

    if( admin_command_permission( ent, cmd ) )
    {
      trap_SendConsoleCommand( EXEC_APPEND, g_admin_commands[ i ]->exec );
      admin_log( ent, cmd, skip );
    }
    else
    {
      ADMP( va( "^3!%s: ^7permission denied\n", g_admin_commands[ i ]->command ) );
      admin_log( ent, "attempted", skip - 1 );
    }
    return qtrue;
  }

  for( i = 0; i < adminNumCmds; i++ )
  {
    if( Q_stricmp( cmd, g_admin_cmds[ i ].keyword ) )
      continue;

    if( G_admin_permission( ent, g_admin_cmds[ i ].flag ) )
    {
      g_admin_cmds[ i ].handler( ent, skip );
      admin_log( ent, cmd, skip );
    }
    else
    {
      ADMP( va( "^3!%s: ^7permission denied\n", g_admin_cmds[ i ].keyword ) );
      admin_log( ent, "attempted", skip - 1 );
    }
    return qtrue;
  }
  return qfalse;
}

void G_admin_namelog_cleanup( )
{
  int i;

  for( i = 0; i < MAX_ADMIN_NAMELOGS && g_admin_namelog[ i ]; i++ )
  {
    BG_Free( g_admin_namelog[ i ] );
    g_admin_namelog[ i ] = NULL;
  }
}

void G_admin_namelog_update( gclient_t *client, qboolean disconnect )
{
  int i, j;
  g_admin_namelog_t *namelog;
  char n1[ MAX_NAME_LENGTH ];
  char n2[ MAX_NAME_LENGTH ];
  int clientNum = ( client - level.clients );

  G_SanitiseString( client->pers.netname, n1, sizeof( n1 ) );
  for( i = 0; i < MAX_ADMIN_NAMELOGS && g_admin_namelog[ i ]; i++ )
  {
    if( disconnect && g_admin_namelog[ i ]->slot != clientNum )
      continue;

    if( !disconnect && !( g_admin_namelog[ i ]->slot == clientNum ||
                          g_admin_namelog[ i ]->slot == -1 ) )
    {
      continue;
    }

    if( !Q_stricmp( client->pers.ip, g_admin_namelog[ i ]->ip )
      && !Q_stricmp( client->pers.guid, g_admin_namelog[ i ]->guid ) )
    {
      for( j = 0; j < MAX_ADMIN_NAMELOG_NAMES
        && g_admin_namelog[ i ]->name[ j ][ 0 ]; j++ )
      {
        G_SanitiseString( g_admin_namelog[ i ]->name[ j ], n2, sizeof( n2 ) );
        if( !Q_stricmp( n1, n2 ) )
          break;
      }
      if( j == MAX_ADMIN_NAMELOG_NAMES )
        j = MAX_ADMIN_NAMELOG_NAMES - 1;
      Q_strncpyz( g_admin_namelog[ i ]->name[ j ], client->pers.netname,
        sizeof( g_admin_namelog[ i ]->name[ j ] ) );
      g_admin_namelog[ i ]->slot = ( disconnect ) ? -1 : clientNum;

      // if this player is connecting, they are no longer banned
      if( !disconnect )
        g_admin_namelog[ i ]->banned = qfalse;

      return;
    }
  }
  if( i >= MAX_ADMIN_NAMELOGS )
  {
    G_Printf( "G_admin_namelog_update: warning, g_admin_namelogs overflow\n" );
    return;
  }
  namelog = BG_Alloc( sizeof( g_admin_namelog_t ) );
  memset( namelog, 0, sizeof( namelog ) );
  for( j = 0; j < MAX_ADMIN_NAMELOG_NAMES ; j++ )
    namelog->name[ j ][ 0 ] = '\0';
  Q_strncpyz( namelog->ip, client->pers.ip, sizeof( namelog->ip ) );
  Q_strncpyz( namelog->guid, client->pers.guid, sizeof( namelog->guid ) );
  Q_strncpyz( namelog->name[ 0 ], client->pers.netname,
    sizeof( namelog->name[ 0 ] ) );
  namelog->slot = ( disconnect ) ? -1 : clientNum;
  g_admin_namelog[ i ] = namelog;
}

qboolean G_admin_readconfig( gentity_t *ent, int skiparg )
{
  g_admin_level_t * l = NULL;
  g_admin_admin_t *a = NULL;
  g_admin_ban_t *b = NULL;
  g_admin_hide_t *h = NULL;
  g_admin_command_t *c = NULL;
  int lc = 0, ac = 0, bc = 0, hc = 0, cc = 0;
  fileHandle_t f;
  int len;
  char *cnf, *cnf2;
  char *t;
  qboolean level_open, admin_open, ban_open, hide_open, command_open;
  char levels[ MAX_STRING_CHARS ] = {""};

  G_admin_cleanup();

  if( !g_admin.string[ 0 ] )
  {
    ADMP( "^3!readconfig: g_admin is not set, not loading configuration "
      "from a file\n" );
    admin_default_levels();
    return qfalse;
  }

  len = trap_FS_FOpenFile( g_admin.string, &f, FS_READ ) ;
  if( len < 0 )
  {
    ADMP( va( "^3!readconfig: ^7could not open admin config file %s\n",
            g_admin.string ) );
    admin_default_levels();
    return qfalse;
  }
  cnf = BG_Alloc( len + 1 );
  cnf2 = cnf;
  trap_FS_Read( cnf, len, f );
  *( cnf + len ) = '\0';
  trap_FS_FCloseFile( f );

  t = COM_Parse( &cnf );
  level_open = admin_open = ban_open = hide_open = command_open = qfalse;
  while( *t )
  {
    if( !Q_stricmp( t, "[level]" ) ||
         !Q_stricmp( t, "[admin]" ) ||
         !Q_stricmp( t, "[ban]" ) ||
         !Q_stricmp( t, "[hide]" ) ||
         !Q_stricmp( t, "[command]" ) )
    {

      if( level_open )
        g_admin_levels[ lc++ ] = l;
      else if( admin_open )
        g_admin_admins[ ac++ ] = a;
      else if( ban_open )
        g_admin_bans[ bc++ ] = b;
      else if( hide_open )
        g_admin_hides[ hc++ ] = h;
      else if( command_open )
        g_admin_commands[ cc++ ] = c;
      level_open = admin_open =
                     ban_open = hide_open = command_open = qfalse;
    }

    if( level_open )
    {
      if( !Q_stricmp( t, "level" ) )
      {
        admin_readconfig_int( &cnf, &l->level );
      }
      else if( !Q_stricmp( t, "name" ) )
      {
        admin_readconfig_string( &cnf, l->name, sizeof( l->name ) );
      }
      else if( !Q_stricmp( t, "flags" ) )
      {
        admin_readconfig_string( &cnf, l->flags, sizeof( l->flags ) );
      }
      else
      {
        ADMP( va( "^3!readconfig: ^7[level] parse error near %s on line %d\n",
                t,
                COM_GetCurrentParseLine() ) );
      }
    }
    else if( admin_open )
    {
      if( !Q_stricmp( t, "name" ) )
      {
        admin_readconfig_string( &cnf, a->name, sizeof( a->name ) );
      }
      else if( !Q_stricmp( t, "guid" ) )
      {
        admin_readconfig_string( &cnf, a->guid, sizeof( a->guid ) );
      }
      else if( !Q_stricmp( t, "level" ) )
      {
        admin_readconfig_int( &cnf, &a->level );
      }
      else if( !Q_stricmp( t, "flags" ) )
      {
        admin_readconfig_string( &cnf, a->flags, sizeof( a->flags ) );
      }
      else
      {
        ADMP( va( "^3!readconfig: ^7[admin] parse error near %s on line %d\n",
                t,
                COM_GetCurrentParseLine() ) );
      }

    }
    else if( ban_open )
    {
      if( !Q_stricmp( t, "name" ) )
      {
        admin_readconfig_string( &cnf, b->name, sizeof( b->name ) );
      }
      else if( !Q_stricmp( t, "guid" ) )
      {
        admin_readconfig_string( &cnf, b->guid, sizeof( b->guid ) );
      }
      else if( !Q_stricmp( t, "ip" ) )
      {
        admin_readconfig_string( &cnf, b->ip, sizeof( b->ip ) );
      }
      else if( !Q_stricmp( t, "reason" ) )
      {
        admin_readconfig_string( &cnf, b->reason, sizeof( b->reason ) );
      }
      else if( !Q_stricmp( t, "made" ) )
      {
        admin_readconfig_string( &cnf, b->made, sizeof( b->made ) );
      }
      else if( !Q_stricmp( t, "expires" ) )
      {
        admin_readconfig_int( &cnf, &b->expires );
      }
      else if( !Q_stricmp( t, "banner" ) )
      {
        admin_readconfig_string( &cnf, b->banner, sizeof( b->banner ) );
      }
    }
    else if( hide_open )
    {
      if( !Q_stricmp( t, "name" ) )
      {
        admin_readconfig_string( &cnf, h->name, sizeof( h->name ) );
      }
      else if( !Q_stricmp( t, "guid" ) )
      {
        admin_readconfig_string( &cnf, h->guid, sizeof( h->guid ) );
      }
      else if( !Q_stricmp( t, "ip" ) )
      {
        admin_readconfig_string( &cnf, h->ip, sizeof( h->ip ) );
      }
      else if( !Q_stricmp( t, "reason" ) )
      {
        admin_readconfig_string( &cnf, h->reason, sizeof( h->reason ) );
      }
      else if( !Q_stricmp( t, "made" ) )
      {
        admin_readconfig_string( &cnf, h->made, sizeof( h->made ) );
      }
      else if( !Q_stricmp( t, "expires" ) )
      {
        admin_readconfig_int( &cnf, &h->expires );
      }
      else if( !Q_stricmp( t, "hider" ) )
      {
        admin_readconfig_string( &cnf, h->hider, sizeof( h->hider ) );
      }
      else if( !Q_stricmp( t, "hidden" ) )
      {
        admin_readconfig_int( &cnf, &h->hidden );
      }
      else
      {
        ADMP( va( "^3!readconfig: ^7[hide] parse error near %s on line %d\n",
                t,
                COM_GetCurrentParseLine() ) );
      }
    }
    else if( command_open )
    {
      if( !Q_stricmp( t, "command" ) )
      {
        admin_readconfig_string( &cnf, c->command, sizeof( c->command ) );
      }
      else if( !Q_stricmp( t, "exec" ) )
      {
        admin_readconfig_string( &cnf, c->exec, sizeof( c->exec ) );
      }
      else if( !Q_stricmp( t, "desc" ) )
      {
        admin_readconfig_string( &cnf, c->desc, sizeof( c->desc ) );
      }
      else if( !Q_stricmp( t, "levels" ) )
      {
        char level[ 4 ] = {""};
        char *lp = levels;
        int cmdlevel = 0;

        admin_readconfig_string( &cnf, levels, sizeof( levels ) );
        while( *lp )
        {
          if( *lp == ' ' )
          {
            c->levels[ cmdlevel++ ] = atoi( level );
            level[ 0 ] = '\0';
            lp++;
            continue;
          }
          Q_strcat( level, sizeof( level ), va( "%c", *lp ) );
          lp++;
        }
        if( level[ 0 ] )
          c->levels[ cmdlevel++ ] = atoi( level );
        // ensure the list is -1 terminated
        c->levels[ MAX_ADMIN_LEVELS ] = -1;
      }
      else
      {
        ADMP( va( "^3!readconfig: ^7[command] parse error near %s on line %d\n",
                t,
                COM_GetCurrentParseLine() ) );
      }
    }

    if( !Q_stricmp( t, "[level]" ) )
    {
      if( lc >= MAX_ADMIN_LEVELS )
        return qfalse;
      l = BG_Alloc( sizeof( g_admin_level_t ) );
      l->level = 0;
      *l->name = '\0';
      *l->flags = '\0';
      level_open = qtrue;
    }
    else if( !Q_stricmp( t, "[admin]" ) )
    {
      if( ac >= MAX_ADMIN_ADMINS )
        return qfalse;
      a = BG_Alloc( sizeof( g_admin_admin_t ) );
      *a->name = '\0';
      *a->guid = '\0';
      a->level = 0;
      *a->flags = '\0';
      admin_open = qtrue;
    }
    else if( !Q_stricmp( t, "[ban]" ) )
    {
      if( bc >= MAX_ADMIN_BANS )
        return qfalse;
      b = BG_Alloc( sizeof( g_admin_ban_t ) );
      *b->name = '\0';
      *b->guid = '\0';
      *b->ip = '\0';
      *b->made = '\0';
      b->expires = 0;
      *b->reason = '\0';
      *b->banner = '\0';
      ban_open = qtrue;
    }
    else if( !Q_stricmp( t, "[hide]" ) )
    {
      if( hc >= MAX_ADMIN_HIDES )
        return qfalse;
      h = BG_Alloc( sizeof( g_admin_hide_t ) );
      *h->name = '\0';
      *h->guid = '\0';
      *h->ip = '\0';
      *h->made = '\0';
      h->expires = 0;
      *h->reason = '\0';
      *h->hider = '\0';
      h->hidden = 0;
      hide_open = qtrue;
    }
    else if( !Q_stricmp( t, "[command]" ) )
    {
      if( cc >= MAX_ADMIN_COMMANDS )
        return qfalse;
      c = BG_Alloc( sizeof( g_admin_command_t ) );
      *c->command = '\0';
      *c->exec = '\0';
      *c->desc = '\0';
      memset( c->levels, -1, sizeof( c->levels ) );
      command_open = qtrue;
    }
    t = COM_Parse( &cnf );
  }
  if( level_open )
  {

    g_admin_levels[ lc++ ] = l;
  }
  if( admin_open )
    g_admin_admins[ ac++ ] = a;
  if( ban_open )
    g_admin_bans[ bc++ ] = b;
  if( hide_open )
    g_admin_hides[ hc++ ] = h;
  if( command_open )
    g_admin_commands[ cc++ ] = c;
  BG_Free( cnf2 );
  ADMP( va( "^3!readconfig: ^7loaded %d levels, %d admins, %d bans, %d hides, %d commands\n",
          lc, ac, bc, hc, cc ) );
  if( lc == 0 )
    admin_default_levels();
  else
  {
    char n[ MAX_NAME_LENGTH ] = {""};
    int i = 0;

    // max printable name length for formatting
    for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
    {
      G_DecolorString( l->name, n, sizeof(n) );
      if( strlen( n ) > admin_level_maxname )
        admin_level_maxname = strlen( n );
    }
  }
  return qtrue;
}

qboolean G_admin_time( gentity_t *ent, int skiparg )
{
  qtime_t qt;
  int t;

  t = trap_RealTime( &qt );
  ADMP( va( "^3!time: ^7local time is %02i:%02i:%02i\n",
    qt.tm_hour, qt.tm_min, qt.tm_sec ) );
  return qtrue;
}

static int G_admin_find_slot( gentity_t *ent, char *cmd, char *name, char *nick, int nick_len )
{
char guid[ 33 ];
char *p;
int id;
int i;
qboolean numeric = qtrue;
gentity_t *vic;

if ( nick )
nick[ 0 ] = '\0';

p = name;
while ( *p )
{
if( *p < '0' || *p > '9' )
{
numeric = qfalse;
break;
}
p++;
}
if( !numeric || name[ 0 ] == '\0' )
{
ADMP( va( "^3!%s:^7 invalid slot number\n.", cmd ) );
return -1;
}

guid[ 0 ] = '\0';
id = atoi( name );
if( id >= 0 && id < level.maxclients )
{
vic = &g_entities[ id ];
if( !vic || !(vic->client) || vic->client->pers.connected != CON_CONNECTED )
{
ADMP( va( "^3!%s:^7 no one connected by that slot number\n", cmd ) );
return qfalse;
}
Q_strncpyz( guid, vic->client->pers.guid, sizeof( guid ) );
if( *guid == 'X' )
{
ADMP( va( "^3!%s:^7 player in slot %d has no GUID.\n", cmd, id ) );
return qfalse;
}
for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
{
if( !Q_stricmp( guid, g_admin_admins[ i ]->guid ) )
{
id = i + MAX_CLIENTS;
if( nick )
Q_strncpyz( nick, vic->client->pers.netname, nick_len );
break;
}
}
}
if( id < MAX_CLIENTS || id >= MAX_CLIENTS + MAX_ADMIN_ADMINS
|| g_admin_admins[ id - MAX_CLIENTS ] == NULL )
{
if( *guid )
ADMP( va( "^3!%s:^7 player is not !registered\n", cmd ) );
else
ADMP( va( "^3!%s:^7 no match. use !listplayers or !listadmins to "
"find an appropriate slot # to use.\n", cmd ) );
return -1;
}
id -= MAX_CLIENTS;


if ( nick && !nick[ 0 ] )
{
Q_strncpyz( nick, g_admin_admins[ id ]->name, nick_len );
}

return id;
}

static int G_admin_user_flag_flagEqual( const char *flags, const char *flag )
{
    while( *flags == '-' || *flags == '+' ) flags++;
    while( *flag == '-' || *flag == '+' ) flag++;
    while( *flags && *flag )
    {
        if( *flags != '^' || *flag != '^' )
            return *flags == *flag;
        flags++;
        flag++;
    }
    return 1;
}

static const char *G_admin_user_flag( int id, char *flag, qboolean add, qboolean clear )
{
  char add_flags[ MAX_ADMIN_FLAGS ];
  char sub_flags[ MAX_ADMIN_FLAGS ];
  char flagbuf[ MAX_NAME_LENGTH ];
  char *flags;
  int add_pos = 0;
  int sub_pos = 0;
  int i;
  int wildcard = 0;

  if( id < 0 || id >= MAX_ADMIN_ADMINS
    || g_admin_admins[ id ] == NULL )
  {
    return "admin id out of range";
  }

  if( *flag == '-' || *flag == '+' || *flag == '*')
  {
    return "invalid admin flag";
  }

  flags = g_admin_admins[ id ]->flags;
  while( *flags )
  {
    if( *flags == '+' )
        flags++;
    if( !G_admin_user_flag_flagEqual( flags, flag ) )
    {
      if( *flags != '-' )
      {
        if( add_pos < MAX_ADMIN_FLAGS - 1 )
        {
            flagbuf[0] = 0;
            Q_strncpyz( flagbuf, flags, sizeof( flagbuf ) );
            for( i = 0; i < sizeof( flagbuf ) - 1; i++ )
            {
                if( flagbuf[ i ] != '^' )
                {
                    flagbuf[ i + 1 ] = 0;
                    break;
                }
            }
            Q_strcat( add_flags, MAX_ADMIN_FLAGS, va( "%s", flagbuf ) );
            add_pos += strlen( flagbuf );
        }
      }
      else
      {
        if( sub_pos < MAX_ADMIN_FLAGS - 1 )
        {
            flags++;
            flagbuf[0] = 0;
            Q_strncpyz( flagbuf, flags, sizeof( flagbuf ) );
            for( i = 0; i < sizeof( flagbuf ) - 1; i++ )
            {
                if( flagbuf[ i ] != '^' )
                {
                    flagbuf[ i + 1 ] = 0;
                    break;
                }
            }
            Q_strcat( sub_flags, MAX_ADMIN_FLAGS, va( "-%s", flagbuf ) );
            sub_pos += strlen( va( "-%s", flagbuf ) );
        }
      }
    }

    while(*flags++ == '^');
  }

  if( !clear )
  {
    if( add )
    {
      if( *flag == '*' )
      {
        wildcard = qtrue;
      }
      else if( add_pos < MAX_ADMIN_FLAGS - 1 )
      {
        flagbuf[0] = 0;
        Q_strncpyz( flagbuf, flag, sizeof( flagbuf ) );
        for( i = 0; i < sizeof( flagbuf ) - 1; i++ )
        {
            if( flagbuf[ i ] != '^' )
            {
                flagbuf[ i + 1 ] = 0;
                break;
            }
        }
        Q_strcat( add_flags, MAX_ADMIN_FLAGS, va( "%s", flagbuf ) );
        add_pos += strlen( flagbuf );
      }
    }
    else
    {
      if( *flag == '*' )
      {
        wildcard = qfalse;
      }
      else if( sub_pos < MAX_ADMIN_FLAGS - 1 )
      {
        flagbuf[0] = 0;
        Q_strncpyz( flagbuf, flag, sizeof( flagbuf ) );
        for( i = 0; i < sizeof( flagbuf ) - 1; i++ )
        {
            if( flagbuf[ i ] != '^' )
            {
                flagbuf[ i + 1 ] = 0;
                break;
            }
        }
        Q_strcat( sub_flags, MAX_ADMIN_FLAGS, va( "-%s", flagbuf ) );
        sub_pos += strlen( va( "-%s", flagbuf ) );
G_Printf( "Debug: sub_pos: %d\nsub_flags: %s\nclear: %d\nadd: %d\nflag: %s\nflags: %s\nflagbuf: %s\n", sub_pos, sub_flags, clear, add, flag, flags, flagbuf );
      }
    }
  }

  if( add_pos + sub_pos + ( (wildcard || sub_pos) ? 1 : 0 ) + 1 >= MAX_ADMIN_FLAGS )
  {
    return "maximum admin flags exceeded";
  }

  add_flags[ add_pos ] = sub_flags[ sub_pos ] = 0;

  Com_sprintf( g_admin_admins[ id ]->flags, MAX_ADMIN_FLAGS,
    "%s%s%s",
     add_flags,
     ( wildcard ) ? "*" : "",
//     ( sub_count && !wildcard ) ? "-" : "",
     sub_flags );

  return NULL;
}

qboolean G_admin_flag( gentity_t *ent, int skiparg )
{
  char output[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  char name[ MAX_NAME_LENGTH ];
  char flagbuf[ MAX_ADMIN_FLAGS + 9 ], *flag;
  int id;
  char adminname[ MAX_NAME_LENGTH ] = {""};
  const char *result;
  qboolean add = qtrue;
  qboolean clear = qfalse;

  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( *cmd == '!' )
    cmd++;

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va( "^3!%s: ^7usage: !%s slot# flag\n", cmd, cmd ) );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  id = G_admin_find_slot( ent, cmd, name, adminname, sizeof( adminname ) );
  if( id < 0 )
    return qfalse;

  if( ent && !admin_higher_guid( ent->client->pers.guid, g_admin_admins[ id ]->guid ) )
  {
    ADMP( va( "^3%s:^7 your intended victim has a higher admin level then you\n", cmd ) );
    return qfalse;
  }
  if( G_SayArgc() < 3 + skiparg )
  {
    Q_strncpyz( output, g_admin_admins[ id ]->flags, sizeof( output ) );
    EXCOLOR( output );
    ADMP( va( "^3%s:^7 flags for %s^7 are '^3%s^7'\n",
      cmd, adminname, output ) );
    return qtrue;
  }

  G_SayArgv( 2 + skiparg, flagbuf, sizeof( flagbuf ) );
  flag = flagbuf;
  if( flag[ 0 ] == '-' && flag[ 1 ] != '\0' )
  {
    add = qfalse;
    flag++;
  }
//  if( ent && !Q_stricmp( ent->client->pers.guid, g_admin_admins[ id ]->guid ) )
//  {
//    ADMP( va( "^3%s:^7 you may not change your own flags (use rcon)\n", cmd ) );
//    return qfalse;
//  }
  if( !G_admin_permission( ent, flag ) && !G_admin_permission( ent, ADMF_EXTENSIVEADMINCONTROL ) )
  {
    ADMP( va( "^3%s:^7 you can only change flags that you also have\n", cmd ) );
    return qfalse;
  }

  if( !Q_stricmp( cmd, "removeflag" ) )
  {
    clear = qtrue;
  }

  result = G_admin_user_flag( id, flag, add, clear );
  if( result )
  {
    ADMP( va( "^3!%s: ^7an error occured setting flag '^3%c^7', %s\n",
      cmd, flag[ 0 ], result ) );
    return qfalse;
  }

  if( !Q_stricmp( cmd, "removeflag" ) )
  {
    Q_strncpyz( output, flag, sizeof( output ) );
    EXCOLOR( output );
    AP( va(
      "print \"^3!%s: ^7admin flag '%s' for %s^7 cleared by %s\n\"",
      cmd, output, adminname,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  else
  {
    Q_strncpyz( output, flag, sizeof( output ) );
    EXCOLOR( output );
    AP( va(
      "print \"^3!%s: ^7%s^7 was %s admin flag '%s' by %s\n\"",
      cmd, adminname,
      ( add ) ? "given" : "denied",
      output,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }

  if( !g_admin.string[ 0 ] )
    ADMP( va( "^3!%s: ^7WARNING g_admin not set, not saving admin record "
      "to a file\n", cmd ) );
  else
    admin_writeconfig();

  return qtrue;
}

qboolean G_admin_setlevel( gentity_t *ent, int skiparg )
{
  char name[ MAX_NAME_LENGTH ] = {""};
  char lstr[ 11 ]; // 10 is max strlen() for 32-bit int
  char adminname[ MAX_NAME_LENGTH ] = {""};
  char testname[ MAX_NAME_LENGTH ] = {""};
  char testname2[ MAX_NAME_LENGTH ] = {""};
  char guid[ 33 ];
  int l, i, j;
  gentity_t *vic = NULL;
  qboolean updated = qfalse;
  g_admin_admin_t *a;
  qboolean found = qfalse;
  qboolean numeric = qtrue;
  int matches = 0;
  int id = -1;


  if( G_SayArgc() < 3 + skiparg )
  {
    ADMP( "^3!setlevel: ^7usage: !setlevel [name|slot#] [level]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, testname, sizeof( testname ) );
  G_SayArgv( 2 + skiparg, lstr, sizeof( lstr ) );
  l = atoi( lstr );
  G_SanitiseString( testname, name, sizeof( name ) );
  for( i = 0; i < sizeof( name ) && name[ i ] ; i++ )
  {
    if( name[ i ] < '0' || name[ i ] > '9' )
    {
      numeric = qfalse;
      break;
    }
  }
  if( numeric )
    id = atoi( name );

  if( ent && l > ent->client->pers.adminLevel )
  {
    ADMP( "^3!setlevel: ^7you may not use !setlevel to set a level higher "
      "than your current level\n" );
    return qfalse;
  }

  // if admin is activated for the first time on a running server, we need
  // to ensure at least the default levels get created
  if( !ent && !g_admin_levels[ 0 ] )
    G_admin_readconfig(NULL, 0);

  for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
  {
    if( g_admin_levels[ i ]->level == l )
    {
      found = qtrue;
      break;
    }
  }
  if( !found )
  {
    ADMP( "^3!setlevel: ^7level is not defined\n" );
    return qfalse;
  }

  if( numeric  && id >= 0 && id < level.maxclients )
    vic = &g_entities[ id ];

  if( vic && vic->client && vic->client->pers.connected == CON_CONNECTED )
  {
    vic = &g_entities[ id ];
    Q_strncpyz( adminname, vic->client->pers.netname, sizeof( adminname ) );
    Q_strncpyz( guid, vic->client->pers.guid, sizeof( guid ) );
    matches = 1;
  }
  else if( numeric && id >= MAX_CLIENTS && id < MAX_CLIENTS + MAX_ADMIN_ADMINS
    && g_admin_admins[ id - MAX_CLIENTS ] )
  {
    Q_strncpyz( adminname, g_admin_admins[ id - MAX_CLIENTS ]->name,
      sizeof( adminname ) );
    Q_strncpyz( guid, g_admin_admins[ id - MAX_CLIENTS ]->guid,
      sizeof( guid ) );
    matches = 1;
  }
  else
  {
    for( i = 0; i < level.maxclients && matches < 2; i++ )
    {
      vic = &g_entities[ i ];
      if( !vic->client || vic->client->pers.connected != CON_CONNECTED )
        continue;
      G_SanitiseString( vic->client->pers.netname, testname, sizeof( testname ) );
      if( strstr( testname, name ) )
      {
        matches++;
        Q_strncpyz( adminname, vic->client->pers.netname, sizeof( adminname ) );
        Q_strncpyz( guid, vic->client->pers.guid, sizeof( guid ) );
      }
    }
    for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ] && matches < 2; i++)
    {
      G_SanitiseString( g_admin_admins[ i ]->name, testname, sizeof( testname ) );
      if( strstr( testname, name ) )
      {
        qboolean dup = qfalse;

        // verify we don't have the same guid/name pair in connected players
        for( j = 0; j < level.maxclients; j++ )
        {
          vic = &g_entities[ j ];
          if( !vic->client || vic->client->pers.connected != CON_CONNECTED )
            continue;
          G_SanitiseString(  vic->client->pers.netname, testname2, sizeof( testname2 ) );
          if( !Q_stricmp( vic->client->pers.guid, g_admin_admins[ i ]->guid )
            && strstr( testname2, name ) )
          {
            dup = qtrue;
            break;
          }
        }
        if( dup )
          continue;
        Q_strncpyz( adminname, g_admin_admins[ i ]->name, sizeof( adminname ) );
        Q_strncpyz( guid, g_admin_admins[ i ]->guid, sizeof( guid ) );
        matches++;
      }
    }
  }

  if( matches == 0 )
  {
    ADMP( "^3!setlevel:^7 no match.  use !listplayers or !listadmins to "
      "find an appropriate number to use instead of name.\n" );
    return qfalse;
  }
  else if( matches > 1 )
  {
    ADMP( "^3!setlevel:^7 more than one match.  Use the admin number "
      "instead:\n" );
    admin_listadmins( ent, 0, name );
    return qfalse;
  }

  if( !Q_stricmp( guid, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) )
  {
    ADMP( va( "^3!setlevel: ^7%s does not have a valid GUID\n", adminname ) );
    return qfalse;
  }
  if( ent && !admin_higher_guid( ent->client->pers.guid, guid ) )
  {
    ADMP( "^3!setlevel: ^7sorry, but your intended victim has a higher"
        " admin level than you\n" );
    return qfalse;
  }

  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ];i++ )
  {
    if( !Q_stricmp( g_admin_admins[ i ]->guid, guid ) )
    {
      g_admin_admins[ i ]->level = l;
      Q_strncpyz( g_admin_admins[ i ]->name, adminname,
                  sizeof( g_admin_admins[ i ]->name ) );
      updated = qtrue;
    }
  }
  if( !updated )
  {
    if( i == MAX_ADMIN_ADMINS )
    {
      ADMP( "^3!setlevel: ^7too many admins\n" );
      return qfalse;
    }
    a = BG_Alloc( sizeof( g_admin_admin_t ) );
    a->level = l;
    Q_strncpyz( a->name, adminname, sizeof( a->name ) );
    Q_strncpyz( a->guid, guid, sizeof( a->guid ) );
    *a->flags = '\0';
    g_admin_admins[ i ] = a;
  }

  AP( va(
    "print \"^3!setlevel: ^7%s^7 was given level %d admin rights by %s\n\"",
    adminname, l, ( ent ) ? ent->client->pers.netname : "console" ) );
  if( vic )
    vic->client->pers.adminLevel = l;

  if( !g_admin.string[ 0 ] )
    ADMP( "^3!setlevel: ^7WARNING g_admin not set, not saving admin record "
      "to a file\n" );
  else
    admin_writeconfig();
  return qtrue;
}

static qboolean admin_create_ban( gentity_t *ent,
  char *netname,
  char *guid,
  char *ip,
  int seconds,
  char *reason )
{
  g_admin_ban_t *b = NULL;
  qtime_t qt;
  int t;
  int i;
  int j;
  qboolean foundAdminTrueName=qfalse;

  t = trap_RealTime( &qt );
  b = BG_Alloc( sizeof( g_admin_ban_t ) );

  if( !b )
    return qfalse;

  Q_strncpyz( b->name, netname, sizeof( b->name ) );
  Q_strncpyz( b->guid, guid, sizeof( b->guid ) );
  Q_strncpyz( b->ip, ip, sizeof( b->ip ) );

  //strftime( b->made, sizeof( b->made ), "%m/%d/%y %H:%M:%S", lt );
  Q_strncpyz( b->made, va( "%02i/%02i/%02i %02i:%02i:%02i",
    (qt.tm_mon + 1), qt.tm_mday, (qt.tm_year - 100),
    qt.tm_hour, qt.tm_min, qt.tm_sec ),
    sizeof( b->made ) );

  if( ent ) {
    //Get admin true name
    for(j = 0; j < MAX_ADMIN_ADMINS && g_admin_admins[ j ]; j++ )
    {
      if( !Q_stricmp( g_admin_admins[ j ]->guid, ent->client->pers.guid ) )
      {
          Q_strncpyz( b->banner, g_admin_admins[ j ]->name, sizeof( b->banner  ) );
      foundAdminTrueName=qtrue;
        break;
      }
    }
    if(foundAdminTrueName==qfalse) Q_strncpyz( b->banner, ent->client->pers.netname, sizeof( b->banner ) );
  }
  else
    Q_strncpyz( b->banner, "console", sizeof( b->banner ) );
  if( !seconds )
    b->expires = 0;
  else
    b->expires = t + seconds;
  if( !*reason )
    Q_strncpyz( b->reason, "banned by admin", sizeof( b->reason ) );
  else
    Q_strncpyz( b->reason, reason, sizeof( b->reason ) );
  for( i = 0; i < MAX_ADMIN_BANS && g_admin_bans[ i ]; i++ )
    ;
  if( i == MAX_ADMIN_BANS )
  {
    ADMP( "^3!ban: ^7too many bans\n" );
    BG_Free( b );
    return qfalse;
  }
  g_admin_bans[ i ] = b;
  return qtrue;
}

static qboolean admin_create_hide( gentity_t *ent,
  char *netname,
  char *guid,
  char *ip,
  int seconds,
  char *reason,
  int hidden )
{
  g_admin_hide_t *h = NULL;
  qtime_t qt;
  int t;
  int i;
  int j;
  qboolean foundAdminTrueName=qfalse;

  t = trap_RealTime( &qt );
  h = BG_Alloc( sizeof( g_admin_hide_t ) );

  if( !h )
    return qfalse;

  Q_strncpyz( h->name, netname, sizeof( h->name ) );
  Q_strncpyz( h->guid, guid, sizeof( h->guid ) );
  Q_strncpyz( h->ip, ip, sizeof( h->ip ) );

  //strftime( h->made, sizeof( b->made ), "%m/%d/%y %H:%M:%S", lt );
  Q_strncpyz( h->made, va( "%02i/%02i/%02i %02i:%02i:%02i",
    (qt.tm_mon + 1), qt.tm_mday, (qt.tm_year - 100),
    qt.tm_hour, qt.tm_min, qt.tm_sec ),
    sizeof( h->made ) );

  if( ent ) {
    //Get admin true name
    for(j = 0; j < MAX_ADMIN_ADMINS && g_admin_admins[ j ]; j++ )
    {
      if( !Q_stricmp( g_admin_admins[ j ]->guid, ent->client->pers.guid ) )
      {
          Q_strncpyz( h->hider, g_admin_admins[ j ]->name, sizeof( h->hider  ) );
      foundAdminTrueName=qtrue;
        break;
      }
    }
    if(foundAdminTrueName==qfalse) Q_strncpyz( h->hider, ent->client->pers.netname, sizeof( h->hider ) );
  }
  else
    Q_strncpyz( h->hider, "console", sizeof( h->hider ) );
  if( !seconds )
    h->expires = 0;
  else
    h->expires = t + seconds;
  if( !*reason )
    Q_strncpyz( h->reason, "hidden by admin", sizeof( h->reason ) );
  else
    Q_strncpyz( h->reason, reason, sizeof( h->reason ) );
  if( !hidden )
    h->hidden = 0;
  else
    h->hidden = hidden;
  for( i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]; i++ )
    ;
  if( i == MAX_ADMIN_HIDES )
  {
    ADMP( "^3!hides: ^7too many hides\n" );
    BG_Free( h );
    return qfalse;
  }
  g_admin_hides[ i ] = h;
  return qtrue;
}

int G_admin_parse_time( const char *time )
{
  int seconds = 0, num = 0;
  int i;
  for( i = 0; time[ i ]; i++ )
  {
    if( isdigit( time[ i ] ) )
    {
      num = num * 10 + time[ i ] - '0';
      continue;
    }
    if( i == 0 || !isdigit( time[ i - 1 ] ) )
      return -1;
    switch( time[ i ] )
    {
      case 'w': num *= 7;
      case 'd': num *= 24;
      case 'h': num *= 60;
      case 'm': num *= 60;
      case 's': break;
      default:  return -1;
    }
    seconds += num;
    num = 0;
  }
  if( num )
    seconds += num;
  // overflow
  if( seconds < 0 )
    seconds = 0;
  return seconds;
}

qboolean G_admin_kick( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], *reason, err[ MAX_STRING_CHARS ];
  int minargc;
  gentity_t *vic;

  minargc = 3 + skiparg;
  if( G_admin_permission( ent, ADMF_UNACCOUNTABLE ) )
    minargc = 2 + skiparg;

  if( G_SayArgc() < minargc )
  {
    ADMP( "^3!kick: ^7usage: !kick [name] [reason]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  reason = G_SayConcatArgs( 2 + skiparg );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!kick: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!kick: ^7sorry, but your intended victim has a higher admin"
        " level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];+   admin_create_ban( ent,
     vic->client->pers.netname,
     vic->client->pers.guid,
     vic->client->pers.ip, G_admin_parse_time( g_adminTempBan.string ),
     ( *reason ) ? reason : "kicked by admin" );
   if( g_admin.string[ 0 ] )
     admin_writeconfig();

   trap_DropClient( pids[ 0 ], va( "kicked^7, reason: %s",
    ( *reason ) ? reason : "kicked by admin" ) );

  return qtrue;
}

qboolean G_admin_ban( gentity_t *ent, int skiparg )
{
  int seconds;
  char search[ MAX_NAME_LENGTH ];
  char secs[ 7 ];
  char *reason;
  int minargc;
  char duration[ 32 ];
  int logmatch = -1, logmatches = 0;
  int i, j;
  qboolean exactmatch = qfalse;
  char n2[ MAX_NAME_LENGTH ];
  char s2[ MAX_NAME_LENGTH ];
  char guid_stub[ 9 ];

  if( G_admin_permission( ent, ADMF_CAN_PERM_BAN ) &&
       G_admin_permission( ent, ADMF_UNACCOUNTABLE ) )
  {
    minargc = 2 + skiparg;
  }
  else if( G_admin_permission( ent, ADMF_CAN_PERM_BAN ) ||
            G_admin_permission( ent, ADMF_UNACCOUNTABLE ) )
  {
    minargc = 3 + skiparg;
  }
  else
  {
    minargc = 4 + skiparg;
  }
  if( G_SayArgc() < minargc )
  {
    ADMP( "^3!ban: ^7usage: !ban [name|slot|ip] [time] [reason]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, search, sizeof( search ) );
  G_SanitiseString( search, s2, sizeof( s2 ) );
  G_SayArgv( 2 + skiparg, secs, sizeof( secs ) );

  seconds = G_admin_parse_time( secs );
  if( seconds <= 0 )
  {
    if( G_admin_permission( ent, ADMF_CAN_PERM_BAN ) )
    {
      seconds = 0;
    }
    else
    {
      ADMP( "^3!ban: ^7ban time must be positive\n" );
      return qfalse;
    }
    reason = G_SayConcatArgs( 2 + skiparg );
  }
  else
  {
    reason = G_SayConcatArgs( 3 + skiparg );
  }

  for( i = 0; i < MAX_ADMIN_NAMELOGS && g_admin_namelog[ i ]; i++ )
  {
    // skip players in the namelog who have already been banned
    if( g_admin_namelog[ i ]->banned )
      continue;

    // skip disconnected players when banning on slot number
    if( g_admin_namelog[ i ]->slot == -1 )
      continue;

    if( !Q_stricmp( va( "%d", g_admin_namelog[ i ]->slot ), s2 ) )
    {
      logmatches = 1;
      logmatch = i;
      exactmatch = qtrue;
      break;
    }
  }

  for( i = 0;
       !exactmatch && i < MAX_ADMIN_NAMELOGS && g_admin_namelog[ i ];
       i++ )
  {
    // skip players in the namelog who have already been banned
    if( g_admin_namelog[ i ]->banned )
      continue;

    if( !Q_stricmp( g_admin_namelog[ i ]->ip, s2 ) )
    {
      logmatches = 1;
      logmatch = i;
      exactmatch = qtrue;
      break;
    }
    for( j = 0; j < MAX_ADMIN_NAMELOG_NAMES
      && g_admin_namelog[ i ]->name[ j ][ 0 ]; j++ )
    {
      G_SanitiseString(g_admin_namelog[ i ]->name[ j ], n2, sizeof(n2));
      if( strstr( n2, s2 ) )
      {
        if( logmatch != i )
          logmatches++;
        logmatch = i;
      }
    }
  }

  if( !logmatches )
  {
    ADMP( "^3!ban: ^7no player found by that name, IP, or slot number\n" );
    return qfalse;
  }
  else if( logmatches > 1 )
  {
    ADMBP_begin();
    ADMBP( "^3!ban: ^7multiple recent clients match name, use IP or slot#:\n" );
    for( i = 0; i < MAX_ADMIN_NAMELOGS && g_admin_namelog[ i ]; i++ )
    {
      for( j = 0; j < 8; j++ )
        guid_stub[ j ] = g_admin_namelog[ i ]->guid[ j + 24 ];
      guid_stub[ j ] = '\0';
      for( j = 0; j < MAX_ADMIN_NAMELOG_NAMES
        && g_admin_namelog[ i ]->name[ j ][ 0 ]; j++ )
      {
        G_SanitiseString(g_admin_namelog[ i ]->name[ j ], n2, sizeof(n2));
        if( strstr( n2, s2 ) )
        {
          if( g_admin_namelog[ i ]->slot > -1 )
            ADMBP( "^3" );
          ADMBP( va( "%-2s (*%s) %15s ^7'%s^7'\n",
           (g_admin_namelog[ i ]->slot > -1) ?
             va( "%d", g_admin_namelog[ i ]->slot ) : "-",
           guid_stub,
           g_admin_namelog[ i ]->ip,
           g_admin_namelog[ i ]->name[ j ] ) );
        }
      }
    }
    ADMBP_end();
    return qfalse;
  }

  G_admin_duration( ( seconds ) ? seconds : -1,
    duration, sizeof( duration ) );

  if( ent && !admin_higher_guid( ent->client->pers.guid,
    g_admin_namelog[ logmatch ]->guid ) )
  {

    ADMP( "^3!ban: ^7sorry, but your intended victim has a higher admin"
      " level than you\n" );
    return qfalse;
  }

  admin_create_ban( ent,
    g_admin_namelog[ logmatch ]->name[ 0 ],
    g_admin_namelog[ logmatch ]->guid,
    g_admin_namelog[ logmatch ]->ip,
    seconds, reason );

  g_admin_namelog[ logmatch ]->banned = qtrue;

  if( !g_admin.string[ 0 ] )
    ADMP( "^3!ban: ^7WARNING g_admin not set, not saving ban to a file\n" );
  else
    admin_writeconfig();

  if( g_admin_namelog[ logmatch ]->slot == -1 )
  {
    // client is already disconnected so stop here
    AP( va( "print \"^3!ban:^7 %s^7 has been banned by %s^7 "
      "duration: %s, reason: %s\n\"",
      g_admin_namelog[ logmatch ]->name[ 0 ],
      ( ent ) ? ent->client->pers.netname : "console",
      duration,
      ( *reason ) ? reason : "banned by admin" ) );
    return qtrue;
  }

  trap_DropClient(  g_admin_namelog[ logmatch ]->slot,
    va( "banned by %s^7, duration: %s, reason: %s",
      ( ent ) ? ent->client->pers.netname : "console",
      duration,
      ( *reason ) ? reason : "banned by admin" ) );
  return qtrue;
}

qboolean G_admin_adjustban( gentity_t *ent, int skiparg )
{
  int bnum;
  int length;
  int expires;
  char duration[ 32 ] = {""};
  char *reason;
  char bs[ 5 ];
  char secs[ 7 ];

  if( G_SayArgc() < 3 + skiparg )
  {
    ADMP( "^3!adjustban: ^7usage: !adjustban [ban#] [time] [reason]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs );
  if( bnum < 1 || bnum > MAX_ADMIN_BANS || !g_admin_bans[ bnum - 1] )
  {
    ADMP( "^3!adjustban: ^7invalid ban#\n" );
    return qfalse;
  }

  G_SayArgv( 2 + skiparg, secs, sizeof( secs ) );
  length = G_admin_parse_time( secs );
  if( length < 0 )
    reason = G_SayConcatArgs( 2 + skiparg );
  else
  {
    if( length != 0 )
      expires = trap_RealTime( NULL ) + length;
    else if( G_admin_permission( ent, ADMF_CAN_PERM_BAN ) )
      expires = 0;
    else
    {
      ADMP( "^3!ban: ^7ban time must be positive\n" );
      return qfalse;
    }

    g_admin_bans[ bnum - 1 ]->expires = expires;
    G_admin_duration( ( length ) ? length : -1, duration, sizeof( duration ) );
    reason = G_SayConcatArgs( 3 + skiparg );
  }
  if( *reason )
    Q_strncpyz( g_admin_bans[ bnum - 1 ]->reason, reason,
      sizeof( g_admin_bans[ bnum - 1 ]->reason ) );
  AP( va( "print \"^3!adjustban: ^7ban #%d for %s^7 has been updated by %s^7 "
    "%s%s%s%s%s\n\"",
    bnum,
    g_admin_bans[ bnum - 1 ]->name,
    ( ent ) ? ent->client->pers.netname : "console",
    ( length >= 0 ) ? "duration: " : "",
    duration,
    ( length >= 0 && *reason ) ? ", " : "",
    ( *reason ) ? "reason: " : "",
    reason ) );
  if( ent )
    Q_strncpyz( g_admin_bans[ bnum - 1 ]->banner, ent->client->pers.netname,
      sizeof( g_admin_bans[ bnum - 1 ]->banner ) );
  if( g_admin.string[ 0 ] )
    admin_writeconfig();
  return qtrue;
}

qboolean G_admin_adjusthide( gentity_t *ent, int skiparg )
{
  int hnum;
  int length;
  int hidden = 1;
  int expires;
  char duration[ 32 ] = {""};
  char *reason = NULL;
  char hs[ 5 ];
  char secs[ 7 ];
  char hiddenC[ 7 ];

  if( G_SayArgc() < 3 + skiparg )
  {
    ADMP( "^3!adjusthide: ^7usage: !adjusthide [hide#] [time] [(c) hidden] [reason]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, hs, sizeof( hs ) );
  hnum = atoi( hs );
  if( hnum < 1 || hnum > MAX_ADMIN_HIDES || !g_admin_hides[ hnum - 1] )
  {
    ADMP( "^3!adjusthide: ^7invalid hide#\n" );
    return qfalse;
  }

  G_SayArgv( 2 + skiparg, secs, sizeof( secs ) );
  G_SayArgv( 3 + skiparg, hiddenC, sizeof( hiddenC ) );
  length = G_admin_parse_time( secs );
  if( length < 0 && ((secs[0] == 'c' || secs[0] == 'C' || secs[0] == 'h' || secs[0] == 'H') && ((secs[1] == ' ' || secs[1] == '\t' || secs[1] == '-') || (secs[1] >= '0' && secs[1] <= '9') || (hiddenC[0] && hiddenC[0] >= '0' && hiddenC[0] <= '9'))) && G_SayArgc() > 3 + skiparg )
  {
    G_SayArgv(3 + skiparg, secs, sizeof( secs ) );
    if(secs[1] == ' ')
    {
        char *p = &secs[1];
        while(*p == ' ' || *p == '\t' || *p == '-') p++;
        hidden = atoi(p);
    }
    else
    {
        hidden = atoi(hiddenC);
    }
    g_admin_hides[ hnum - 1 ]->hidden = hidden;
  }
  else if( length < 0 )
    reason = G_SayConcatArgs( 2 + skiparg );
  else
  {
    if( length != 0 )
      expires = trap_RealTime( NULL ) + length;
    else if( length >= 0 )
      expires = 0;
    else
    {
      ADMP( "^3!adjusthide: ^7hide time must be positive\n" );
      return qfalse;
    }

    g_admin_hides[ hnum - 1 ]->expires = expires;
    G_admin_duration( ( length ) ? length : -1, duration, sizeof( duration ) );
    reason = G_SayConcatArgs( 3 + skiparg );
  }
  if( reason && *reason )
    Q_strncpyz( g_admin_hides[ hnum - 1 ]->reason, reason,
      sizeof( g_admin_hides[ hnum - 1 ]->reason ) );
  AP( va( "print \"^3!adjusthide: ^7hide #%d for %s^7 has been updated by %s^7 "
    "%s%s%s%s%s%s\n\"",
    hnum,
    g_admin_hides[ hnum - 1 ]->name,
    ( ent ) ? ent->client->pers.netname : "console",
    ( length < 0 && ((secs[0] == 'c' || secs[0] == 'C' || secs[0] == 'h' || secs[0] == 'H') && ((secs[1] == ' ' || secs[1] == '\t' || secs[1] == '-') || (secs[1] >= '0' && secs[1] <= '9') || (hiddenC[0] && hiddenC[0] >= '0' && hiddenC[0] <= '9'))) && G_SayArgc() > 3 + skiparg ) ? (hidden ? "hidden: hidden" : "hidden: unhidden") : "",
    ( length >= 0 ) ? "duration: " : "",
    duration,
    ( length >= 0 && reason && *reason ) ? ", " : "",
    ( reason && *reason ) ? "reason: " : "",
    reason ? reason : "" ) );
  if( ent )
    Q_strncpyz( g_admin_hides[ hnum - 1 ]->hider, ent->client->pers.netname,
      sizeof( g_admin_hides[ hnum - 1 ]->hider ) );
  if( g_admin.string[ 0 ] )
    admin_writeconfig();
  return qtrue;
}

qboolean G_admin_unban( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 5 ];
  int t;

  t = trap_RealTime( NULL );
  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!unban: ^7usage: !unban [ban#]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs );
  if( bnum < 1 || bnum > MAX_ADMIN_BANS || !g_admin_bans[ bnum - 1 ] )
  {
    ADMP( "^3!unban: ^7invalid ban#\n" );
    return qfalse;
  }
  g_admin_bans[ bnum -1 ]->expires = t;
  AP( va( "print \"^3!unban: ^7ban #%d for %s^7 has been removed by %s\n\"",
          bnum,
          g_admin_bans[ bnum - 1 ]->name,
          ( ent ) ? ent->client->pers.netname : "console" ) );
  if( g_admin.string[ 0 ] )
    admin_writeconfig();
  return qtrue;
}

qboolean G_admin_putscrimteam( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], weaponName[ MAX_STRING_CHARS ], *teamName, err[ MAX_STRING_CHARS ];
  //weapon_t weapon;
  gentity_t *vic;
  //g_oc_scrimTeam_t *t;

  if(!BG_OC_OCMode())return(qfalse);

  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  G_SayArgv( 2 + skiparg, weaponName, sizeof( weaponName ) );
  teamName = G_SayConcatArgs( 3 + skiparg );
  if( G_SayArgc() < 4 + skiparg )
  {
    ADMP( "^3!putscrimteam: ^7usage: !putscrimteam [name] [weapon] [teamname/0fornoteam]\n" );
    return qfalse;
  }
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!putscrimteam: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!putscrimteam: ^7sorry, but your intended victim has a higher "
        " admin level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
/*
  if(!strcmp(teamName, "0"))
  {
    // remove player from team
    G_OCScrimTeamRemovePlayer(vic);
    return qtrue;
  }
  else if(G_OCScrimTeam(teamName))
  {
    // existing team
    if(vic->client->pers.ocTeam)
        G_OCScrimTeamRemovePlayer(vic);
    G_OCScrimTeam(teamName)->notSingleTeam = 1;
    vic->client->pers.ocTeam = G_OCScrimTeam(teamName) - level.scrimTeam;
    G_ClientPrint(NULL, va("%s^7 was put on to scrim team %s^7 (%ss^7) by %s", vic->client->pers.netname, level.scrimTeam[vic->client->pers.ocTeam].name, BG_FindHumanNameForWeapon(level.scrimTeam[vic->client->pers.ocTeam].weapon), ( ent ) ? ent->client->pers.netname : "console"), 0);
    return qtrue;
  }
  else
  {
    // new team
    weapon = BG_FindWeaponNumForName( weaponName );
    if(G_WeaponIsReserved(weapon))
    {
        ADMP( va( "^3!putscrimteam: ^7the %s^7 is already in use by another team\n", BG_FindHumanNameForWeapon(weapon) ) );
        return qfalse;
    }
    if(!(t = G_OCNewScrimTeam(teamName, weapon, err, sizeof(err))))
    {
        ADMP( va( "^3!putscrimteam: ^7couldn't creat scrim team: %s\n", err ) );
        return qfalse;
    }
    if(vic->client->pers.ocTeam)
        G_OCScrimTeamRemovePlayer(vic);
    vic->client->pers.ocTeam = t - level.scrimTeam;
    AP(va("print \"^3!putscrimteam: ^7%s^7 created and put %s^7 on to the new OC scrim team %s (%ss^7)\n\"", ( ent ) ? ent->client->pers.netname : "console", vic->client->pers.netname, t->name, BG_FindHumanNameForWeapon(weapon)));
    return qtrue;
  }
  */
  G_OC_JoinPlayerToScrimTeam(vic, ent, teamName, weaponName);
  AP(va("print \"^3!putscrimteam: ^7%s^7 put %s^7 on to a scrim team\n\"", (ent) ? ent->client->pers.netname : "console", vic->client->pers.netname));
  return qtrue;
}

qboolean G_admin_putteam( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], team[ 7 ], err[ MAX_STRING_CHARS ];//, secs[ 7 ];
  //int seconds = 0;
  gentity_t *vic;
  team_t teamnum = TEAM_NONE;
  char teamdesc[ 32 ] = {"spectators"};

  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  G_SayArgv( 2 + skiparg, team, sizeof( team ) );
  if( G_SayArgc() < 3 + skiparg )
  {
    ADMP( "^3!putteam: ^7usage: !putteam [name] [h|a|s] (duration)\n" );
    return qfalse;
  }

  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!putteam: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!putteam: ^7sorry, but your intended victim has a higher "
        " admin level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  switch( team[ 0 ] )
  {
  case 'a':
    teamnum = TEAM_ALIENS;
    Q_strncpyz( teamdesc, "aliens", sizeof( teamdesc ) );
    break;
  case 'h':
    teamnum = TEAM_HUMANS;
    Q_strncpyz( teamdesc, "humans", sizeof( teamdesc ) );
    break;
  case 's':
    teamnum = TEAM_NONE;
    break;
  default:
    ADMP( va( "^3!putteam: ^7unknown team %c\n", team[ 0 ] ) );
    return qfalse;
  }
  // duration
  /*
  if( G_SayArgc() > 3 + skiparg )
  {
    int modifier = 1;
    // only allow locking into spec
    if ( teamnum != TEAM_NONE )
    {
      ADMP( "^3!putteam: ^7You are can only lock a player into the spectators team\n" );
      return qfalse;
    }
    // only allow setting a time if have permission
    if ( !G_admin_permission( ent, ADMF_PUTTEAMEXT ) )
    {
      ADMP( "^3!putteam: ^7You are not allowed to lock a player into the spectators team\n" );
      return qfalse;
    }
    G_SayArgv( 3 + skiparg, secs, sizeof( secs ) );
    // support "h" (hours), and "m" (minutes) modifiers
    if( secs[ 0 ] )
    {
      int lastchar = strlen( secs ) - 1;
      if( secs[ lastchar ] == 'h' )
        modifier = 60 * 60;
      else if( secs[ lastchar ] == 'm' )
        modifier = 60;
      else if( secs[ lastchar ] < '0' || secs[ lastchar ] > '9' )
        secs[ lastchar ] = '\0';
    }
    seconds = atoi( secs );
    if( seconds > 0 )
      seconds *= modifier;
    else
      seconds = 0;
  }
  */
  if( vic->client->pers.teamSelection == teamnum && teamnum != TEAM_NONE )
  {
    ADMP( va( "^3!putteam: ^7%s ^7is already on the %s team\n", vic->client->pers.netname, teamdesc ) );
    return qfalse;
  }
  //vic->client->pers.specExpires = level.time + seconds * 1000;
  G_ChangeTeam( vic, teamnum );

  AP( va( "print \"^3!putteam: ^7%s^7 put %s^7 on to the %s team%s\n\"",
          ( ent ) ? ent->client->pers.netname : "console",
          vic->client->pers.netname, teamdesc,
          //( seconds ) ? va( " for %i seconds", seconds ) : "" ) );
		  ""));
  return qtrue;
}

qboolean G_admin_map( gentity_t *ent, int skiparg )
{
  char map[ MAX_QPATH ];
  char layout[ MAX_QPATH ] = { "" };

  if( G_SayArgc( ) < 2 + skiparg )
  {
    ADMP( "^3!map: ^7usage: !map [map] (layout)\n" );
    return qfalse;
  }

  G_SayArgv( skiparg + 1, map, sizeof( map ) );

  if( !trap_FS_FOpenFile( va( "maps/%s.bsp", map ), NULL, FS_READ ) )
  {
    ADMP( va( "^3!map: ^7invalid map name '%s'\n", map ) );
    return qfalse;
  }

  if( G_SayArgc( ) > 2 + skiparg )
  {
    G_SayArgv( skiparg + 2, layout, sizeof( layout ) );
    if( !Q_stricmp( layout, "*BUILTIN*" ) ||
      trap_FS_FOpenFile( va( "layouts/%s/%s.dat", map, layout ),
        NULL, FS_READ ) > 0 )
    {
      trap_Cvar_Set( "g_layouts", layout );
    }
    else
    {
      ADMP( va( "^3!map: ^7invalid layout name '%s'\n", layout ) );
      return qfalse;
    }
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "map %s", map ) );
  level.restarted = qtrue;
  AP( va( "print \"^3!map: ^7map '%s' started by %s^7 %s\n\"", map,
          ( ent ) ? ent->client->pers.netname : "console",
          ( layout[ 0 ] ) ? va( "(forcing layout '%s')", layout ) : "" ) );
  return qtrue;
}

qboolean G_admin_devmap( gentity_t *ent, int skiparg )
{
  char map[ MAX_QPATH ];
  char layout[ MAX_QPATH ] = { "" };

  if( G_SayArgc( ) < 2 + skiparg )
  {
    ADMP( "^3!devmap: ^7usage: !devmap [map] (layout)\n" );
    return qfalse;
  }

  G_SayArgv( skiparg + 1, map, sizeof( map ) );

  if( !trap_FS_FOpenFile( va( "maps/%s.bsp", map ), NULL, FS_READ ) )
  {
    ADMP( va( "^3!devmap: ^7invalid map name '%s'\n", map ) );
    return qfalse;
  }

  if( G_SayArgc( ) > 2 + skiparg )
  {
    G_SayArgv( skiparg + 2, layout, sizeof( layout ) );
    if( !Q_stricmp( layout, "*BUILTIN*" ) ||
      trap_FS_FOpenFile( va( "layouts/%s/%s.dat", map, layout ),
        NULL, FS_READ ) > 0 )
    {
      trap_Cvar_Set( "g_layouts", layout );
    }
    else
    {
      ADMP( va( "^3!devmap: ^7invalid layout name '%s'\n", layout ) );
      return qfalse;
    }
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "devmap %s", map ) );
  level.restarted = qtrue;
  AP( va( "print \"^3!devmap: ^7map '%s' started by %s^7 with cheats %s\n\"", map,
          ( ent ) ? ent->client->pers.netname : "console",
          ( layout[ 0 ] ) ? va( "(forcing layout '%s')", layout ) : "" ) );
  return qtrue;
}

qboolean G_admin_layoutsave( gentity_t *ent, int skiparg )
{
  char layout[ MAX_QPATH ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  char output[ MAX_STRING_CHARS ];

  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;

  if( G_SayArgc( ) < 2 + skiparg )
  {
    ADMP( va( "^3!%s: ^7usage: !%s [layout]\n", cmd, cmd ) );
    return qfalse;
  }

  G_SayArgv( skiparg + 1, layout, sizeof( layout ) );
  G_StrToLower(layout);

  if( !( *layout == 'o' && *(layout + 1) == 'c' ) || !( g_ocReview.integer && Q_stricmp( cmd, "layoutsave" ) ) )
  {
    trap_SendConsoleCommand( EXEC_APPEND, va( "layoutsave %s", layout ) );
    Q_strncpyz( output, va( "layout saved as '%s'", layout ), sizeof( output ));
    EXCOLOR( output );
    AP( va( "print \"^3!layoutsave: ^7%s^7 by ^7%s^7\n\"", output,
            ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  else
  {
    trap_SendConsoleCommand( EXEC_APPEND, va( "layoutsave %s_review", layout ) );
    Q_strncpyz( output, va( "layout saved for review as '%s_review'", layout ), sizeof( output ));
    EXCOLOR( output );
    AP( va( "print \"^3!layoutsave: ^7%s^7 by ^7%s^7\n\"", output,
            ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  return qtrue;
}

qboolean G_admin_hbp( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 99 ];
  qtime_t qt;
  int t;

  t = trap_RealTime( &qt );

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!cheat-hbp: ^7usage: !cheat-hbp [bp]\n" );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs );
  if( bnum < 0 )
  {
    ADMP( va("^6Humans Build Points ^4%d\n", trap_Cvar_VariableIntegerValue("g_humanBuildPoints")) );
    ADMP( "^3!cheat-hbp: ^7invalid bp #\n" );
    return qfalse;
  }

  if ( ent && !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-hbp: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "g_humanBuildPoints %d", bnum ) );


  AP( va( "print \"^3!cheat-hbp: ^7Humans' build points changed to '%d' by %s\n\"", bnum,
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_abp( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 99 ];
  qtime_t qt;
  int t;

  t = trap_RealTime( &qt );

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va("^6Aliens Build Points ^4%d\n", trap_Cvar_VariableIntegerValue("g_alienBuildPoints")) );
    ADMP( "^3!cheat-abp: ^7usage: !cheat-abp [bp]\n" );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs );
  if( bnum < 0 )
  {
    ADMP( "^3!cheat-abp: ^7invalid bp #\n" );
    return qfalse;
  }

  if ( ent && !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-abp: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "g_alienBuildPoints %d", bnum ) );


  AP( va( "print \"^3!cheat-abp: ^7Aliens' build points changed to '%d' by %s\n\"", bnum,
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_giveall( gentity_t *ent, int skiparg )
{


  if (!ent || ent->client->pers.teamSelection == TEAM_NONE || ent->client->sess.spectatorState != SPECTATOR_NOT || ent->client->ps.stats[ STAT_HEALTH ] <= 0)
  {
    ADMP( "Join a team first / Must be living to use this command / Cannot be run as console\n" );
    return qfalse;
  }

  if ( !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-give-all: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  ent->client->pers.hasCheated = 1;
  ent->client->pers.cheated = 1;

  ent->health = ent->client->ps.stats[ STAT_MAX_HEALTH ];
  BG_AddUpgradeToInventory( UP_MEDKIT, ent->client->ps.stats );

  G_AddCreditToClient( ent->client, HUMAN_MAX_CREDITS, qtrue );

  ent->client->ps.stats[ STAT_STAMINA ] = MAX_STAMINA;

  ent->client->ps.stats[ STAT_STATE ] |= SS_BOOSTED;
  ent->client->boostedTime = level.time;

  G_OC_PlayerMaxAmmo(ent);

/*  int maxAmmoXXX, maxClipsXXX;
  gclient_t *client = ent->client;

  if( client->ps.weapon != WP_ALEVEL3_UPG &&
    BG_FindInfinteAmmoForWeapon( client->ps.weapon ) )
    return qtrue;

  BG_FindAmmoForWeapon( client->ps.weapon, &maxAmmoXXX, &maxClipsXXX );

  if( BG_FindUsesEnergyForWeapon( client->ps.weapon ) &&
      BG_InventoryContainsUpgrade( UP_BATTPACK, client->ps.stats ) )
      maxAmmoXXX = (int)( (float)maxAmmoXXX * BATTPACK_MODIFIER );

    BG_PackAmmoArray( client->ps.weapon, client->ps.ammo, client->ps.misc, maxAmmoXXX, maxClipsXXX );*/


    AP( va(
      "print \"^3!cheat-give-all: ^7%s^7 cheats.\n\"",
      ( ent ) ? ent->client->pers.netname : "console" ) );

  return qtrue;
}

qboolean G_admin_god( gentity_t *ent, int skiparg )
{
  if (!ent || ent->client->pers.teamSelection == TEAM_NONE || ent->client->sess.spectatorState != SPECTATOR_NOT || ent->client->ps.stats[ STAT_HEALTH ] <= 0)
  {
    ADMP( "Join a team first / Must be living to use this command / Cannot be run as console\n" );
    return qfalse;
  }

  if ( !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-god: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  ent->client->pers.hasCheated = 1;
  ent->client->pers.cheated = ( ( ent->flags & FL_GODMODE ) ? ( 0 ) : ( 1 ) );
  ent->flags ^= FL_GODMODE;

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", ( ent->flags & FL_GODMODE ) ? ("godmode ON\n") : ("godmode OFF\n") ));

    AP( va(
      "print \"^3!cheat-god: ^7%s^7 cheats.\n\"",
      ( ent ) ? ent->client->pers.netname : "console" ) );

  return qtrue;
}

qboolean G_admin_speed( gentity_t *ent, int skiparg )
{
  if (!ent || ent->client->pers.teamSelection == TEAM_NONE || ent->client->sess.spectatorState != SPECTATOR_NOT || ent->client->ps.stats[ STAT_HEALTH ] <= 0)
  {
    ADMP( "Join a team first / Must be living to use this command / Cannot be run as console\n" );
    return qfalse;
  }

  if ( !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-speed: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  ent->client->pers.hasCheated = 1;
  ent->client->speed = !ent->client->speed;

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", ( ent->client->speed ) ? ("speedmode ON\n") : ("speedmode OFF\n") ));

    AP( va(
      "print \"^3!cheat-speed: ^7%s^7 cheats.\n\"",
      ( ent ) ? ent->client->pers.netname : "console" ) );

  return qtrue;
}

qboolean G_admin_kill( gentity_t *ent, int skiparg )
{
  if (!ent || ent->client->pers.teamSelection == TEAM_NONE || ent->client->sess.spectatorState != SPECTATOR_NOT || ent->client->ps.stats[ STAT_HEALTH ] <= 0)
  {
    ADMP( "Join a team first / Must be living to use this command / Cannot be run as console\n" );
    return qfalse;
  }

  if ( !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-kill: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  ent->client->pers.hasCheated = 1;
  ent->client->pers.cheated = 0;
  ent->flags &= ~FL_GODMODE;
  ent->client->ps.stats[ STAT_HEALTH ] = ent->health = 0;
  player_die( ent, ent, ent, 100000, MOD_SUICIDE );


    AP( va(
      "print \"^3!cheat-kill: ^7%s^7 cheats.\n\"",
      ( ent ) ? ent->client->pers.netname : "console" ) );

    return qtrue;
}

qboolean G_admin_noclip( gentity_t *ent, int skiparg )
{
  if (!ent || ent->client->pers.teamSelection == TEAM_NONE || ent->client->sess.spectatorState != SPECTATOR_NOT || ent->client->ps.stats[ STAT_HEALTH ] <= 0)
  {
    ADMP( "Join a team first / Must be living to use this command / Cannot be run as console\n" );
    return qfalse;
  }

  if ( !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-noclip: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  ent->client->pers.hasCheated = 1;
  ent->client->pers.cheated = ent->client->noclip = !ent->client->noclip;

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", (ent->client->noclip) ? ("noclip ON\n") : ("noclip OFF\n") ) );


    AP( va(
      "print \"^3!cheat-noclip: ^7%s^7 cheats.\n\"",
      ( ent ) ? ent->client->pers.netname : "console" ) );

  return qtrue;
}

qboolean G_admin_setCheat( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;
  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va( "^3!%s: ^7usage: !%s [name|slot#]\n", cmd, cmd ) );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!%s: ^7%s\n", cmd, err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( va( "^3!%s: ^7sorry, but your intended victim has a higher admin"
        " level than you\n", cmd ) );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( vic->client->pers.muted == qtrue )
  {
    if( !Q_stricmp( cmd, "cheat-set" ) )
    {
      ADMP( va( "^3!%s: ^7player has already cheated\n", cmd ) );
      return qtrue;
    }
    vic->client->pers.hasCheated = 0;
    CPx( pids[ 0 ], "cp \"^2You haven't cheated\"" );
    AP( va( "print \"^3!unsetCheat: ^7%s^7 forced the game forget that ^7%s^7 had cheated\n\"",
            ( ent ) ? ent->client->pers.netname : "console",
            vic->client->pers.netname ) );
  }
  else
  {
    if( !Q_stricmp( cmd, "cheat-unset" ) )
    {
      ADMP( va( "^3!%s: ^7player has not cheated\n", cmd ) );
      return qtrue;
    }
    vic->client->pers.hasCheated = 1;
    CPx( pids[ 0 ], "cp \"^1You have cheated\"" );
    AP( va( "print \"^3!setCheat: ^7%s^7 reminded the game that ^7%s^7 had cheated\n\"",
            ( ent ) ? ent->client->pers.netname : "console",
            vic->client->pers.netname ) );
  }
  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}

qboolean G_admin_notarget( gentity_t *ent, int skiparg )
{
  if (!ent || ent->client->pers.teamSelection == TEAM_NONE || ent->client->sess.spectatorState != SPECTATOR_NOT || ent->client->ps.stats[ STAT_HEALTH ] <= 0)
  {
    ADMP( "Join a team first / Must be living to use this command / Cannot be run as console\n" );
    return qfalse;
  }

  if ( !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-notarget: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }


  ent->client->pers.hasCheated = 1;
  ent->client->pers.cheated = ( ( ent->flags & FL_NOTARGET ) ? ( 0 ) : ( 1 ) );
  ent->flags ^= FL_NOTARGET;


  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", ( ent->flags & FL_NOTARGET ) ? ("notarget ON\n") : ("notarget OFF\n") ) );


    AP( va(
      "print \"^3!cheat-notarget: ^7%s^7 cheats.\n\"",
      ( ent ) ? ent->client->pers.netname : "console" ) );

  return qtrue;
}

qboolean G_admin_friendlyfire( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 99 ];

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va("^6Friendly Fire ^4%d\n", trap_Cvar_VariableIntegerValue("g_friendlyFire")) );
    ADMP( "^3!friendlyfire: ^7usage: !friendlyfire [1/0]\n" );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs );
  if( bnum < 0 )
  {
    ADMP( "^3!friendlyfire: ^7invalid value\n" );
    return qfalse;
  }
  if( bnum > 1 )
  {
    ADMP( "^3!friendlyfire: ^7invalid value\n" );
    return qfalse;
  }
  if (bnum == 1)
  {
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyFire 1\n" ) );
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyBuildableFire 1\n" ) );
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyFireHumans 1\n" ) );
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyFireAliens 1\n" ) );

  AP( va( "print \"^3!friendlyfire: ^7%s^7 ^5ENABLED^7 friendly fire\n\"",
          ( ent ) ? ent->client->pers.netname : "console" ) );
    return qtrue;
  }
  if (bnum == 0)
  {
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyFire 0\n" ) );
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyBuildableFire 0\n" ) );
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyFireHumans 0\n" ) );
    trap_SendConsoleCommand( EXEC_APPEND, va( "g_friendlyFireAliens 0\n" ) );

  AP( va( "print \"^3!friendlyfire: ^7%s^7 ^5DISABLED^7 friendly fire\n\"",
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
  }
  return qfalse;
}

qboolean G_admin_hs( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 99 ];
  qtime_t qt;
  int t;

  t = trap_RealTime( &qt );

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va("^6Humans Stage ^4%d\n", trap_Cvar_VariableIntegerValue("g_humanStage")) );
    ADMP( "^3!cheat-hs: ^7usage: !cheat-hs [stage]\n" );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs ) - 1;
  if( bnum < 0 )
  {
    ADMP( "^3!cheat-hs: ^7invalid stage #\n" );
    return qfalse;
  }

  if ( ent && !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-hs: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "g_humanStage %d", bnum ) );


  AP( va( "print \"^3!cheat-hs: ^7Humans' stage changed to '%d' by %s\n\"", bnum + 1,
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_as( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 99 ];
  qtime_t qt;
  int t;

  t = trap_RealTime( &qt );

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va("^6Aliens Stage ^4%d\n", trap_Cvar_VariableIntegerValue("g_alienStage")) );
    ADMP( "^3!cheat-as: ^7usage: !cheat-as [stage]\n" );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs ) - 1;
  if( bnum < 0 )
  {
    ADMP( "^3!cheat-as: ^7invalid stage #\n" );
    return qfalse;
  }

  if ( ent && !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( "^3!cheat-as: ^7Cheats are not enabled on this server\n" );
    return qfalse;
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "g_alienStage %d", bnum ) );


  AP( va( "print \"^3!cheat-as: ^7Aliens' stage changed to '%d' by %s\n\"", bnum + 1,
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_customgravity( gentity_t *ent, int skiparg )
{
  int bnum;
  char bs[ 99 ];
  qtime_t qt;
  int t;

  t = trap_RealTime( &qt );

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va("^6Gravity ^4%d\n", trap_Cvar_VariableIntegerValue("g_gravity")) );
    ADMP( "^3!customgravity: ^7usage: !customgravity [gravity]\n" );
    return qfalse;
  }

  G_SayArgv( 1 + skiparg, bs, sizeof( bs ) );
  bnum = atoi( bs );
  if( bnum < 0 )
  {
    ADMP( "^3!customgravity: ^7invalid gravity #\n" );
    return qfalse;
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "g_gravity %d", bnum ) );


  AP( va( "print \"^3!customgravity: ^7Gravity changed to '%d' by %s\n\"", bnum,
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_mute( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!mute: ^7usage: !mute [name|slot#]\n" );
    return qfalse;
  }
  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!mute: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!mute: ^7sorry, but your intended victim has a higher admin"
        " level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( vic->client->pers.muted == qtrue )
  {
    if( !Q_stricmp( cmd, "mute" ) )
    {
      ADMP( "^3!mute: ^7player is already muted\n" );
      return qtrue;
    }
    vic->client->pers.muted = qfalse;
    CPx( pids[ 0 ], "cp \"^1You have been unmuted\"" );
    AP( va( "print \"^3!unmute: ^7%s^7 has been unmuted by %s\n\"",
            vic->client->pers.netname,
            ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  else
  {
    if( !Q_stricmp( cmd, "unmute" ) )
    {
      ADMP( "^3!unmute: ^7player is not currently muted\n" );
      return qtrue;
    }
    vic->client->pers.muted = qtrue;
    CPx( pids[ 0 ], "cp \"^1You've been muted\"" );
    AP( va( "print \"^3!mute: ^7%s^7 has been muted by ^7%s\n\"",
            vic->client->pers.netname,
            ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}

qboolean G_admin_hide( gentity_t *ent, int skiparg )
{
  int seconds;
  char secs[ 7 ];
  char duration[ 32 ];
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], *reason, err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  if( !BG_OC_OCMode() )
  {
    ADMP( "^3!hide: ^7can only be used during an obstacle course\n" );
    return qfalse;
  }
//  // admins can still hide?
//  if( !g_allowHiding.integer )
//  {
//    ADMP( "^3!hide: ^7hiding has been disabled\n" );
//    return qfalse;
//  }

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!hide: ^7usage: !hide [name|slot#] (reason)\n" );
    return qfalse;
  }
  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  reason = G_SayConcatArgs( 2 + skiparg );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!hide: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!hide: ^7sorry, but your intended victim has a higher admin"
        " level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( Q_stricmp( cmd, "hide" ) )
  {
    int id, t;
    char userinfo[ MAX_INFO_STRING ];
    // remove hide
    trap_GetUserinfo( vic - g_entities, userinfo, sizeof( userinfo ) );
    if(G_admin_hide_check( userinfo, NULL, 0, NULL, NULL, &id ) )
    {
        t = trap_RealTime( NULL );
        g_admin_hides[ id ]->expires = t;
        if( g_admin.string[ 0 ] )
            admin_writeconfig();
    }
    vic->r.svFlags &= ~SVF_SINGLECLIENT;
    vic->client->pers.hidden = qfalse;
    if( G_SayArgc() == 2 + skiparg )
    {
        AP( va( "print \"^3!unhide: ^7%s^7 has been unhidden by ^7%s\n\"",
                vic->client->pers.netname,
                ( ent ) ? ent->client->pers.netname : "console" ) );
    }
    else
    {
        G_SayArgv( 2 + skiparg, secs, sizeof( secs ) );
        seconds = G_admin_parse_time( secs );
        G_admin_duration( seconds, duration, sizeof( duration ) );
        admin_create_hide( ent, vic->client->pers.netname, vic->client->pers.guid, vic->client->pers.ip, seconds, ( *reason ) ? reason : "unhidden by admin", 0 );
        admin_writeconfig();

        vic->client->pers.hiddenTime = level.time + seconds * 1000;
        AP( va( "print \"^3!unhide: ^7%s^7 has been unhidden by ^7%s; force duration: ^7%s^7\n\"",
                vic->client->pers.netname,
                ( ent ) ? ent->client->pers.netname : "console", duration ) );
    }
  }
  else
  {
    int id, t;
    char userinfo[ MAX_INFO_STRING ];
    // remove hide
    trap_GetUserinfo( vic - g_entities, userinfo, sizeof( userinfo ) );
    if(G_admin_hide_check( userinfo, NULL, 0, NULL, NULL, &id ) )
    {
        t = trap_RealTime( NULL );
        g_admin_hides[ id ]->expires = t;
        if( g_admin.string[ 0 ] )
            admin_writeconfig();
    }
    vic->client->pers.hidden = qtrue;
    G_StopFromFollowing( vic, 0 );
    vic->r.svFlags |= SVF_SINGLECLIENT;
    vic->r.singleClient = vic-g_entities;
    CPx( pids[ 0 ], "cp \"^1You've been hidden\"" );
    if( G_SayArgc() == 2 + skiparg )
    {
        AP( va( "print \"^3!hide: ^7%s^7 has been hidden by ^7%s\n\"",
                vic->client->pers.netname,
                ( ent ) ? ent->client->pers.netname : "console" ) );
    }
    else
    {
        G_SayArgv( 2 + skiparg, secs, sizeof( secs ) );
        seconds = G_admin_parse_time( secs );
        G_admin_duration( seconds, duration, sizeof( duration ) );
        admin_create_hide( ent, vic->client->pers.netname, vic->client->pers.guid, vic->client->pers.ip, seconds, ( *reason ) ? reason : "hidden by admin", 1 );
        admin_writeconfig();

        vic->client->pers.hiddenTime = level.time + seconds * 1000;
        AP( va( "print \"^3!hide: ^7%s^7 has been hidden by ^7%s; force duration: ^7%s^7\n\"",
                vic->client->pers.netname,
                ( ent ) ? ent->client->pers.netname : "console", duration ) );
    }
  }

  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}

qboolean G_admin_canEditOC( gentity_t *ent )
{
    if ( !ent )  // console can't edit
      return qfalse;

    if (!BG_OC_OCMode())  // oc only
      return qfalse;

    if (level.ocEditMode == 0)
        return qfalse;

    if (level.ocEditMode == 1 && !G_admin_permission( ent, ADMF_LAYOUTEDIT ))
        return qfalse;

    return qtrue;
}

qboolean G_admin_editoc( gentity_t *ent, int skiparg )
{
  gentity_t *client;
  char command[ MAX_ADMIN_CMD_LEN ];
  int i;
  G_SayArgv( skiparg+1, command, sizeof( command ) );
  if( BG_OC_OCMode() )
  {
    if( !Q_stricmp( command, "0" ) || !Q_stricmp( command, "off" ) )
    {
      AP( va( "print \"^3!editoc: ^7Admin cheating and oc editing ^5DISABLED^7 to ^2off^7 by ^7%s^7\n\"",
              ( ent ) ? ent->client->pers.netname : "console" ) );
      level.ocEditMode = 0;
      for( i = 0; i < level.maxclients; i++ )
      {
        client = &g_entities[ i ];
        if( client->client )
          client->client->pers.noAuO = 0;
      }
    }
    else if( !Q_stricmp( command, "1" ) || !Q_stricmp( command, "allwithflag" ) )
    {
      AP( va( "print \"^3!editoc: ^7Admin cheating and oc editing ^5ENABLED^7 to ^2allwithflag^7 by ^7%s^7\n\"",
              ( ent ) ? ent->client->pers.netname : "console" ) );
      level.ocEditMode = 1;
      for( i = 0; i < level.maxclients; i++ )
      {
        client = &g_entities[ i ];
        if( client->client )
          client->client->pers.noAuO = 0;
      }
    }
    else if( !Q_stricmp( command, "2" ) || !Q_stricmp( command, "all" ) )
    {
      AP( va( "print \"^3!editoc: ^7Admin cheating and oc editing ^5ENABLED^7 to ^1all^7 by ^7%s^7\n\"",
              ( ent ) ? ent->client->pers.netname : "console" ) );
      level.ocEditMode = 2;
      for( i = 0; i < level.maxclients; i++ )
      {
        client = &g_entities[ i ];
        if( client->client )
          client->client->pers.noAuO = 0;
      }
    }
    else
    {
      return qfalse;
    }
  }
  else
  {
    return qfalse;
  }
  return qtrue;
}

qboolean G_admin_denybuild( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;
  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va( "^3!%s: ^7usage: !%s [name|slot#]\n", cmd, cmd ) );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!%s: ^7%s\n", cmd, err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( va( "^3!%s: ^7sorry, but your intended victim has a higher admin"
              " level than you\n", cmd ) );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( vic->client->pers.denyBuild )
  {
    if( !Q_stricmp( cmd, "denybuild" ) )
    {
      ADMP( "^3!denybuild: ^7player already has no building rights\n" );
      return qtrue;
    }
    vic->client->pers.denyBuild = qfalse;
    CPx( pids[ 0 ], "cp \"^1You've regained your building rights\"" );
    AP( va(
      "print \"^3!allowbuild: ^7building rights for ^7%s^7 restored by %s\n\"",
      vic->client->pers.netname,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  else
  {
    if( !Q_stricmp( cmd, "allowbuild" ) )
    {
      ADMP( "^3!allowbuild: ^7player already has building rights\n" );
      return qtrue;
    }
    vic->client->pers.denyBuild = qtrue;
    vic->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
    if( vic->client->ps.stats[ STAT_CLASS ]== PCL_ALIEN_BUILDER0 || vic->client->ps.stats[ STAT_CLASS ] == PCL_ALIEN_BUILDER0_UPG )
    {
      vic->suicideTime = level.time + 1000;
    }
    CPx( pids[ 0 ], "cp \"^1You've lost your building rights\"" );
    AP( va(
      "print \"^3!denybuild: ^7building rights for ^7%s^7 revoked by ^7%s\n\"",
      vic->client->pers.netname,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}

qboolean G_admin_listadmins( gentity_t *ent, int skiparg )
{
  int i, found = 0;
  char search[ MAX_NAME_LENGTH ] = {""};
  char s[ MAX_NAME_LENGTH ] = {""};
  int start = 0;
  qboolean numeric = qtrue;
  int drawn = 0;

  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( g_admin_admins[ i ]->level == 0 )
      continue;
    found++;
  }
  if( !found )
  {
    ADMP( "^3!listadmins: ^7no admins defined\n" );
    return qfalse;
  }

  if( G_SayArgc() == 2 + skiparg )
  {
    G_SayArgv( 1 + skiparg, s, sizeof( s ) );
    for( i = 0; i < sizeof( s ) && s[ i ]; i++ )
    {
      if( s[ i ] >= '0' && s[ i ] <= '9' )
        continue;
      numeric = qfalse;
    }
    if( numeric )
    {
      start = atoi( s );
      if( start > 0 )
        start -= 1;
      else if( start < 0 )
        start = found + start;
    }
    else
      G_SanitiseString( s, search, sizeof( search ) );
  }

  if( start >= found || start < 0 )
    start = 0;

  if( start >= found )
  {
    ADMP( va( "^3!listadmins: ^7listing %d admins\n", found ) );
    return qfalse;
  }

  drawn = admin_listadmins( ent, start, search );

  if( search[ 0 ] )
  {
    ADMP( va( "^3!listadmins:^7 found %d admins matching '%s^7'\n",
      drawn, search ) );
  }
  else
  {
    ADMBP_begin();
    ADMBP( va( "^3!listadmins:^7 showing admin %d - %d of %d.  ",
      ( found ) ? ( start + 1 ) : 0,
      ( ( start + MAX_ADMIN_LISTITEMS ) > found ) ?
       found : ( start + MAX_ADMIN_LISTITEMS ),
      found ) );
    if( ( start + MAX_ADMIN_LISTITEMS ) < found )
    {
      ADMBP( va( "run '!listadmins %d' to see more",
        ( start + MAX_ADMIN_LISTITEMS + 1 ) ) );
    }
    ADMBP( "\n" );
    ADMBP_end();
  }
  return qtrue;
}

qboolean G_admin_listlayouts( gentity_t *ent, int skiparg )
{
  char list[ MAX_CVAR_VALUE_STRING ];
  char map[ MAX_QPATH ];
  int count = 0;
  char *s;
  char layout[ MAX_QPATH ] = { "" };
  int i = 0;

  if( G_SayArgc( ) == 2 + skiparg )
    G_SayArgv( 1 +skiparg, map, sizeof( map ) );
  else
    trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );

  count = G_LayoutList( map, list, sizeof( list ) );
  ADMBP_begin( );
  ADMBP( va( "^3!listlayouts:^7 %d layouts found for '%s':\n", count, map ) );
  s = &list[ 0 ];
  while( *s )
  {
    if( *s == ' ' )
    {
      EXCOLOR( layout );
      ADMBP( va ( " %s\n", layout ) );
      layout[ 0 ] = '\0';
      i = 0;
    }
    else if( i < sizeof( layout ) - 2 )
    {
      layout[ i++ ] = *s;
      layout[ i ] = '\0';
    }
    s++;
  }
  if( layout[ 0 ] )
  {
    EXCOLOR( layout );
    ADMBP( va ( " %s\n", layout ) );
  }
  ADMBP_end( );
  return qtrue;
}

qboolean G_admin_listplayers( gentity_t *ent, int skiparg )
{
  int i, j;
  gclient_t *p;
  char c[ 3 ], t[ 2 ]; // color and team letter
  char n[ MAX_NAME_LENGTH ] = {""};
  char n2[ MAX_NAME_LENGTH ] = {""};
  char n3[ MAX_NAME_LENGTH ] = {""};
  char lname[ MAX_NAME_LENGTH ];
  char lname2[ MAX_NAME_LENGTH ];
  char guid_stub[ 9 ];
  char muted[ 2 ], denied[ 2 ];//, dbuilder[ 2 ];
  int l;
  char lname_fmt[ 5 ];

  ADMBP_begin();
  ADMBP( va( "^3!listplayers^7: %d players connected:\n",
    level.numConnectedClients ) );
  for( i = 0; i < level.maxclients; i++ )
  {
    p = &level.clients[ i ];
    Q_strncpyz( t, "S", sizeof( t ) );
    Q_strncpyz( c, S_COLOR_YELLOW, sizeof( c ) );
    if( p->pers.teamSelection == TEAM_HUMANS )
    {
      Q_strncpyz( t, "H", sizeof( t ) );
      Q_strncpyz( c, S_COLOR_BLUE, sizeof( c ) );
    }
    else if( p->pers.teamSelection == TEAM_ALIENS )
    {
      Q_strncpyz( t, "A", sizeof( t ) );
      Q_strncpyz( c, S_COLOR_RED, sizeof( c ) );
    }

    if( p->pers.connected == CON_CONNECTING )
    {
      Q_strncpyz( t, "C", sizeof( t ) );
      Q_strncpyz( c, S_COLOR_CYAN, sizeof( c ) );
    }
    else if( p->pers.connected != CON_CONNECTED )
    {
      continue;
    }

    for( j = 0; j < 8; j++ )
      guid_stub[ j ] = p->pers.guid[ j + 24 ];
    guid_stub[ j ] = '\0';

    muted[ 0 ] = '\0';
    if( p->pers.muted )
    {
      Q_strncpyz( muted, "M", sizeof( muted ) );
    }
    denied[ 0 ] = '\0';
    if( p->pers.denyBuild )
    {
      Q_strncpyz( denied, "B", sizeof( denied ) );
    }

//    dbuilder[ 0 ] = '\0';
//    if( p->pers.designatedBuilder )
//    {
//      if( !G_admin_permission( &g_entities[ i ], ADMF_INCOGNITO ) &&
//          G_admin_permission( &g_entities[ i ], ADMF_DBUILDER ) )
//      {
//        Q_strncpyz( dbuilder, "P", sizeof( dbuilder ) );
//      }
//      else
//      {
//        Q_strncpyz( dbuilder, "D", sizeof( dbuilder ) );
//      }
//    }

    l = 0;
    G_SanitiseString( p->pers.netname, n2, sizeof( n2 ) );
    n[ 0 ] = '\0';
    for( j = 0; j < MAX_ADMIN_ADMINS && g_admin_admins[ j ]; j++ )
    {
      if( !Q_stricmp( g_admin_admins[ j ]->guid, p->pers.guid ) )
      {

        // don't gather aka or level info if the admin is incognito
        if( ent && G_admin_permission( &g_entities[ i ], ADMF_INCOGNITO ) )
        {
          break;
        }
        l = g_admin_admins[ j ]->level;
        G_SanitiseString( g_admin_admins[ j ]->name, n3, sizeof( n3 ) );
        if( Q_stricmp( n2, n3 ) )
        {
          Q_strncpyz( n, g_admin_admins[ j ]->name, sizeof( n ) );
        }
        break;
      }
    }
    lname[ 0 ] = '\0';
    Q_strncpyz( lname_fmt, "%s", sizeof( lname_fmt ) );
    for( j = 0; j < MAX_ADMIN_LEVELS && g_admin_levels[ j ]; j++ )
    {
      if( g_admin_levels[ j ]->level == l )
      {
        Q_strncpyz( lname, g_admin_levels[ j ]->name, sizeof( lname ) );
        if( *lname )
        {
          G_DecolorString( lname, lname2, sizeof(lname2) );
          Com_sprintf( lname_fmt, sizeof( lname_fmt ), "%%%is",
            ( admin_level_maxname + strlen( lname ) - strlen( lname2 ) ) );
          Com_sprintf( lname2, sizeof( lname2 ), lname_fmt, lname );
        }
        break;
      }

    }

    if( G_admin_permission(ent, ADMF_SEESFULLLISTPLAYERS ) )
    {
//      ADMBP( va( "%2i %s%s^7 %-2i %s^7 (*%s) ^1%1s%1s%1s^7 %s^7 %s%s^7%s\n",
      ADMBP( va( "%2i %s%s^7 %-2i %s^7 (*%s) ^1%1s%1s^7 %s^7 %s%s^7%s\n",
                i,
                c,
                t,
                l,
                ( *lname ) ? lname2 : "",
                guid_stub,
  //              dbuilder,
                muted,
                denied,
                p->pers.netname,
                ( *n ) ? "(a.k.a. " : "",
                n,
                ( *n ) ? ")" : "" ) );
    }
    else
    {
      ADMBP( va( "%2i %s%s^7 ^1%1s%1s^7 %s^7\n",
                i,
                c,
                t,
                muted,
                denied,
                p->pers.netname ) );
    }
  }
  ADMBP_end();
  return qtrue;
}

qboolean G_admin_showbans( gentity_t *ent, int skiparg )
{
  int i, found = 0;
  int t;
  char duration[ 32 ];
  char name_fmt[ 32 ] = { "%s" };
  char banner_fmt[ 32 ] = { "%s" };
  int max_name = 1, max_banner = 1;
  int secs;
  int start = 0;
  char skip[ 11 ];
  char date[ 11 ];
  char *made;
  int j;
  char n1[ MAX_NAME_LENGTH ] = {""};
  char n2[ MAX_NAME_LENGTH ] = {""};

  t = trap_RealTime( NULL );

  for( i = 0; i < MAX_ADMIN_BANS && g_admin_bans[ i ]; i++ )
  {
    if( g_admin_bans[ i ]->expires != 0
      && ( g_admin_bans[ i ]->expires - t ) < 1 )
    {
      continue;
    }
    found++;
  }

  if( G_SayArgc() < 3 + skiparg )
  {
    G_SayArgv( 1 + skiparg, skip, sizeof( skip ) );
    start = atoi( skip );
    // showbans 1 means start with ban 0
    if( start > 0 )
      start -= 1;
    else if( start < 0 )
      start = found + start;
  }

  if( start >= MAX_ADMIN_BANS || start < 0 )
    start = 0;

  for( i = start; i < MAX_ADMIN_BANS && g_admin_bans[ i ]
    && ( i - start ) < MAX_ADMIN_SHOWBANS; i++ )
  {
    G_DecolorString( g_admin_bans[ i ]->name, n1, sizeof(n1) );
    G_DecolorString( g_admin_bans[ i ]->banner, n2, sizeof(n2) );
    if( strlen( n1 ) > max_name )
    {
      max_name = strlen( n1 );
    }
    if( strlen( n2 ) > max_banner )
      max_banner = strlen( n2 );
  }

  if( start >= found )
  {
    ADMP( va( "^3!showbans: ^7there are %d active bans\n", found ) );
    return qfalse;
  }
  ADMBP_begin();
  for( i = start; i < MAX_ADMIN_BANS && g_admin_bans[ i ]
    && ( i - start ) < MAX_ADMIN_SHOWBANS; i++ )
  {
    if( g_admin_bans[ i ]->expires != 0
      && ( g_admin_bans[ i ]->expires - t ) < 1 )
      continue;

    // only print out the the date part of made
    date[ 0 ] = '\0';
    made = g_admin_bans[ i ]->made;
    for( j = 0; made && *made; j++ )
    {
      if( ( j + 1 ) >= sizeof( date ) )
        break;
      if( *made == ' ' )
        break;
      date[ j ] = *made;
      date[ j + 1 ] = '\0';
      made++;
    }

    secs = ( g_admin_bans[ i ]->expires - t );
    G_admin_duration( secs, duration, sizeof( duration ) );

    G_DecolorString( g_admin_bans[ i ]->name, n1, sizeof(n1) );
    Com_sprintf( name_fmt, sizeof( name_fmt ), "%%%is",
      ( max_name + strlen( g_admin_bans[ i ]->name ) - strlen( n1 ) ) );
    Com_sprintf( n1, sizeof( n1 ), name_fmt, g_admin_bans[ i ]->name );

    G_DecolorString( g_admin_bans[ i ]->banner, n2, sizeof(n2) );
    Com_sprintf( banner_fmt, sizeof( banner_fmt ), "%%%is",
      ( max_banner + strlen( g_admin_bans[ i ]->banner ) - strlen( n2 ) ) );
    Com_sprintf( n2, sizeof( n2 ), banner_fmt, g_admin_bans[ i ]->banner );

    ADMBP( va( "%4i %s^7 %-15s %-8s %s^7 %-10s\n     \\__ %s\n",
             ( i + 1 ),
             n1,
             g_admin_bans[ i ]->ip,
             date,
             n2,
             duration,
             g_admin_bans[ i ]->reason ) );
  }

  ADMBP( va( "^3!showbans:^7 showing bans %d - %d of %d.  ",
           ( found ) ? ( start + 1 ) : 0,
           ( ( start + MAX_ADMIN_SHOWBANS ) > found ) ?
           found : ( start + MAX_ADMIN_SHOWBANS ),
           found ) );
  if( ( start + MAX_ADMIN_SHOWBANS ) < found )
  {
    ADMBP( va( "run !showbans %d to see more",
             ( start + MAX_ADMIN_SHOWBANS + 1 ) ) );
  }
  ADMBP( "\n" );
  ADMBP_end();
  return qtrue;
}

qboolean G_admin_showhides( gentity_t *ent, int skiparg )
{
  int i, found = 0;
  int t;
  char duration[ 32 ];
  char name_fmt[ 32 ] = { "%s" };
  char hider_fmt[ 32 ] = { "%s" };
  int max_name = 1, max_hider = 1;
  int secs;
  int start = 0;
  char skip[ 11 ];
  char date[ 11 ];
  char *made;
  int j;
  char n1[ MAX_NAME_LENGTH ] = {""};
  char n2[ MAX_NAME_LENGTH ] = {""};

  t = trap_RealTime( NULL );

  for( i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]; i++ )
  {
    if( g_admin_hides[ i ]->expires != 0
      && ( g_admin_hides[ i ]->expires - t ) < 1 )
    {
      continue;
    }
    found++;
  }

  if( G_SayArgc() < 3 + skiparg )
  {
    G_SayArgv( 1 + skiparg, skip, sizeof( skip ) );
    start = atoi( skip );
    // showhides 1 means start with hide 0
    if( start > 0 )
      start -= 1;
    else if( start < 0 )
      start = found + start;
  }

  if( start >= MAX_ADMIN_HIDES || start < 0 )
    start = 0;

  for( i = start; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]
    && ( i - start ) < MAX_ADMIN_SHOWHIDES; i++ )
  {
    G_DecolorString( g_admin_hides[ i ]->name, n1, sizeof(n1) );
    G_DecolorString( g_admin_hides[ i ]->hider, n2, sizeof(n2) );
    if( strlen( n1 ) > max_name )
    {
      max_name = strlen( n1 );
    }
    if( strlen( n2 ) > max_hider )
      max_hider = strlen( n2 );
  }

  if( start >= found )
  {
    ADMP( va( "^3!showhides: ^7there are %d active hides\n", found ) );
    return qfalse;
  }
  ADMBP_begin();
  for( i = start; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]
    && ( i - start ) < MAX_ADMIN_SHOWHIDES; i++ )
  {
    if( g_admin_hides[ i ]->expires != 0
      && ( g_admin_hides[ i ]->expires - t ) < 1 )
      continue;

    // only print out the the date part of made
    date[ 0 ] = '\0';
    made = g_admin_hides[ i ]->made;
    for( j = 0; made && *made; j++ )
    {
      if( ( j + 1 ) >= sizeof( date ) )
        break;
      if( *made == ' ' )
        break;
      date[ j ] = *made;
      date[ j + 1 ] = '\0';
      made++;
    }

    secs = ( g_admin_hides[ i ]->expires - t );
    G_admin_duration( secs, duration, sizeof( duration ) );

    G_DecolorString( g_admin_hides[ i ]->name, n1, sizeof(n1) );
    Com_sprintf( name_fmt, sizeof( name_fmt ), "%%%is",
      ( max_name + strlen( g_admin_hides[ i ]->name ) - strlen( n1 ) ) );
    Com_sprintf( n1, sizeof( n1 ), name_fmt, g_admin_hides[ i ]->name );

    G_DecolorString( g_admin_hides[ i ]->hider, n2, sizeof(n2) );
    Com_sprintf( hider_fmt, sizeof( hider_fmt ), "%%%is",
      ( max_hider + strlen( g_admin_hides[ i ]->hider ) - strlen( n2 ) ) );
    Com_sprintf( n2, sizeof( n2 ), hider_fmt, g_admin_hides[ i ]->hider );

    ADMBP( va( "%4i %s^7 %-15s %-8s %s^7 %-10s %s\n     \\__ %s\n",
             ( i + 1 ),
             n1,
             g_admin_hides[ i ]->ip,
             date,
             n2,
             duration,
             g_admin_hides[ i ]->reason,
             g_admin_hides[ i ]->hidden ? "  hidden" : "unhidden" ) );
  }

  ADMBP( va( "^3!showhides:^7 showing hides %d - %d of %d.  ",
           ( found ) ? ( start + 1 ) : 0,
           ( ( start + MAX_ADMIN_SHOWHIDES ) > found ) ?
           found : ( start + MAX_ADMIN_SHOWHIDES ),
           found ) );
  if( ( start + MAX_ADMIN_SHOWHIDES ) < found )
  {
    ADMBP( va( "run !showhides %d to see more",
             ( start + MAX_ADMIN_SHOWHIDES + 1 ) ) );
  }
  ADMBP( "\n" );
  ADMBP_end();
  return qtrue;
}

qboolean G_admin_help( gentity_t *ent, int skiparg )
{
  int i;

  if( G_SayArgc() < 2 + skiparg )
  {
    int j = 0;
    int count = 0;

    ADMBP_begin();
    for( i = 0; i < adminNumCmds; i++ )
    {
      if( G_admin_permission( ent, g_admin_cmds[ i ].flag ) )
      {
        ADMBP( va( "^3!%-12s", g_admin_cmds[ i ].keyword ) );
        j++;
        count++;
      }
      // show 6 commands per line
      if( j == 6 )
      {
    ADMBP( "\n" );
        j = 0;
      }
    }
    for( i = 0; i < MAX_ADMIN_COMMANDS && g_admin_commands[ i ]; i++ )
    {
      if( ! admin_command_permission( ent, g_admin_commands[ i ]->command ) )
        continue;
      ADMBP( va( "^3!%-12s", g_admin_commands[ i ]->command ) );
      j++;
      count++;
      // show 6 commands per line
      if( j == 6 )
      {
    ADMBP( "\n" );
        j = 0;
      }
    }
    if( count )
    ADMBP( "\n" );
    ADMBP( va( "^3!help: ^7%i available commands\n", count ) );
    ADMBP( "run !help [^3command^7] for help with a specific command.\n" );
    ADMBP_end();

    return qtrue;
  }
  else
  {
    //!help param
    char flagbuf[ MAX_ADMIN_FLAGS ];
    char param[ MAX_ADMIN_CMD_LEN ];
    char *cmd;
    int j;

    G_SayArgv( 1 + skiparg, param, sizeof( param ) );
    cmd = ( param[0] == '!' ) ? &param[1] : &param[0];
    ADMBP_begin();
    for( i = 0; i < adminNumCmds; i++ )
    {
      if( !Q_stricmp( cmd, g_admin_cmds[ i ].keyword ) )
      {
        if( !G_admin_permission( ent, g_admin_cmds[ i ].flag ) )
        {
          ADMBP( va( "^3!help: ^7you have no permission to use '%s'\n",
                   g_admin_cmds[ i ].keyword ) );
          ADMBP_end();
          return qfalse;
        }
        ADMBP( va( "^3!help: ^7help for '!%s':\n",
          g_admin_cmds[ i ].keyword ) );
        ADMBP( va( " ^3Function: ^7%s\n", g_admin_cmds[ i ].function ) );
        ADMBP( va( " ^3Syntax: ^7!%s %s\n", g_admin_cmds[ i ].keyword,
                 g_admin_cmds[ i ].syntax ) );
        Q_strncpyz( flagbuf, g_admin_cmds[ i ].flag, sizeof( flagbuf ) );
        for( j = 0; j < sizeof( flagbuf ); j++ )
        {
            if( flagbuf[ j ] == '^' && j + 3 < sizeof( flagbuf ) )
            {
                memmove( &flagbuf[ j ] + 2, &flagbuf[ j ], strlen( flagbuf ) + 1 );
                flagbuf[ ++j ] = '^';
                flagbuf[ ++j ] = '7';
            }
        }
        ADMBP( va( " ^3Flag: ^7'%s'\n", flagbuf ) );
        ADMBP_end();
        return qtrue;
      }
    }
    for( i = 0; i < MAX_ADMIN_COMMANDS && g_admin_commands[ i ]; i++ )
    {
      if( !Q_stricmp( cmd, g_admin_commands[ i ]->command ) )
      {
        if( !admin_command_permission( ent, g_admin_commands[ i ]->command ) )
        {
          ADMBP( va( "^3!help: ^7you have no permission to use '%s'\n",
                   g_admin_commands[ i ]->command ) );
          ADMBP_end();
          return qfalse;
        }
        ADMBP( va( "^3!help: ^7help for '%s':\n",
          g_admin_commands[ i ]->command ) );
        ADMBP( va( " ^3Description: ^7%s\n", g_admin_commands[ i ]->desc ) );
        ADMBP( va( " ^3Syntax: ^7!%s\n", g_admin_commands[ i ]->command ) );
        ADMBP_end();
        return qtrue;
      }
    }
    ADMBP( va( "^3!help: ^7no help found for '%s'\n", cmd ) );
    ADMBP_end();
    return qfalse;
  }
}

qboolean G_admin_crash( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!crash: ^7usage: !crash [name|slot#]\n" );
    return qfalse;
  }
  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!crash: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!crash: ^7sorry, but your intended victim has a higher admin"
        " level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  vic->client->pers.crashTime = level.time + 3000;
  AP( va( "print \"^3!crash: ^7%s^7 crashed %s^7\n\"", ( ent ) ? ent->client->pers.netname : "console", vic->client->pers.netname ) );
  return qtrue;
}

qboolean G_admin_warn( gentity_t *ent, int skiparg )
{//mostly copy and paste with the proper lines altered from !mute and !kick


  int pids[ MAX_CLIENTS ];
  char name[ MAX_NAME_LENGTH ], *reason, err[ MAX_STRING_CHARS ];
  int minargc;
  int foundvalasdf;
  gentity_t *vic;

  minargc = 3 + skiparg;
  if( G_admin_permission( ent, ADMF_UNACCOUNTABLE ) )
    minargc = 2 + skiparg;

  if( G_SayArgc() < minargc )
  {
    ADMP( "^3!warn: ^7usage: warn [name] [reason]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  reason = G_SayConcatArgs( 2 + skiparg );
  if( (foundvalasdf = G_ClientNumbersFromString( name, pids, MAX_CLIENTS )) != 1 )
  {
    G_MatchOnePlayer( pids, foundvalasdf, err, sizeof( err ) );
    ADMP( va( "^3!warn: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( "^3!warn: ^7sorry, but your intended victim has a higher admin"
        " level than you.\n" );
    return qfalse;
  }

  vic = &g_entities[ pids[ 0 ] ];
  //next line is the onscreen warning
  CPx( pids[ 0 ],va("cp \"^1You have been warned by an administrator.\n ^3Cease immediately or face admin action!\n^1 %s%s\"",(*reason)? "REASON: " : "" ,(*reason)? reason : "") );
  AP( va( "print \"^3!warn: ^7%s^7 has been warned to cease and desist %s by %s \n\"",
            vic->client->pers.netname, (*reason) ? reason : "his current activity",
            ( ent ) ? ent->client->pers.netname : "console" ) );//console announcement
  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}

qboolean G_admin_admintest( gentity_t *ent, int skiparg )
{
  int i, l = 0;
  qboolean found = qfalse;
  qboolean lname = qfalse;

  if( !ent )
  {
    ADMP( "^3!admintest: ^7you are on the console.\n" );
    return qtrue;
  }
  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( g_admin_admins[ i ]->guid, ent->client->pers.guid ) )
    {
      found = qtrue;
      break;
    }
  }

  if( found )
  {
    l = g_admin_admins[ i ]->level;
    for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
    {
      if( g_admin_levels[ i ]->level != l )
        continue;
      if( *g_admin_levels[ i ]->name )
      {
        lname = qtrue;
        break;
      }
    }
  }
  AP( va( "print \"^3!admintest: ^7%s^7 is a level %d admin %s%s^7%s\n\"",
          ent->client->pers.netname,
          l,
          ( lname ) ? "(" : "",
          ( lname ) ? g_admin_levels[ i ]->name : "",
          ( lname ) ? ")" : "" ) );
  return qtrue;
}

qboolean G_admin_allready( gentity_t *ent, int skiparg )
{
  int i = 0;
  gclient_t *cl;

  if( !level.intermissiontime )
  {
    ADMP( "^3!allready: ^7this command is only valid during intermission\n" );
    return qfalse;
  }

  for( i = 0; i < g_maxclients.integer; i++ )
  {
    cl = level.clients + i;
    if( cl->pers.connected != CON_CONNECTED )
      continue;

    if( cl->pers.teamSelection == TEAM_NONE )
      continue;

    cl->readyToExit = 1;
  }
  AP( va( "print \"^3!allready:^7 %s^7 says everyone is READY now\n\"",
     ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_cancelvote( gentity_t *ent, int skiparg )
{

  if( !level.voteExecuteTime && !level.voteTime && !level.teamVoteTime[ 0 ] && !level.teamVoteTime[ 1 ] )
  {
    ADMP( "^3!cancelvote^7: no vote in progress\n" );
    return qfalse;
  }
  if( level.voteExecuteTime )
  {
    level.voteExecuteTime = level.voteTime = 0;
    AP( va( "print \"^3!cancelvote: ^7%s^7 canceled the pending vote\n\"",
            ( ent ) ? ent->client->pers.netname : "console" ) );
    return qtrue;
  }
  level.voteNo = level.numConnectedClients;
  level.voteYes = 0;
  CheckVote( );
  level.teamVoteNo[ 0 ] = level.numConnectedClients;
  level.teamVoteYes[ 0 ] = 0;
  CheckTeamVote( TEAM_HUMANS );
  level.teamVoteNo[ 1 ] = level.numConnectedClients;
  level.teamVoteYes[ 1 ] = 0;
  CheckTeamVote( TEAM_ALIENS );
  AP( va( "print \"^3!cancelvote: ^7%s^7 decided that everyone voted No\n\"",
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_passvote( gentity_t *ent, int skiparg )
{
  if(!level.voteTime && !level.teamVoteTime[ 0 ] && !level.teamVoteTime[ 1 ] )
  {
    ADMP( "^3!passvote^7: no vote in progress\n" );
    return qfalse;
  }
  level.voteYes = level.numConnectedClients;
  level.voteNo = 0;
  CheckVote( );
  level.teamVoteYes[ 0 ] = level.numConnectedClients;
  level.teamVoteNo[ 0 ] = 0;
  CheckTeamVote( TEAM_HUMANS );
  level.teamVoteYes[ 1 ] = level.numConnectedClients;
  level.teamVoteNo[ 1 ] = 0;
  CheckTeamVote( TEAM_ALIENS );
  AP( va( "print \"^3!passvote: ^7%s^7 decided that everyone voted Yes\n\"",
          ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_pause( gentity_t *ent, int skiparg )
{
    if(!level.paused)
    {
        AP( va( "print \"^3!pause: ^7%s^7 paused the game.\n\"", ( ent ) ? ent->client->pers.netname : "console" ) );
        level.paused = qtrue;
        trap_SendServerCommand( -1, "cp \"The game has been paused. Please wait.\"" );
    }
    else {
        AP( va( "print \"^3!pause: ^7%s^7 unpaused the game (Paused for %d msec) \n\"", ( ent ) ? ent->client->pers.netname : "console",level.pausedTime ) );
        trap_SendServerCommand( -1, "cp \"The game has been unpaused!\"" );
        level.paused = qfalse;


        }
    return qtrue;
}

qboolean G_admin_spec999( gentity_t *ent, int skiparg )
{
  int i;
  gentity_t *vic;

  for( i = 0; i < level.maxclients; i++ )
  {
    vic = &g_entities[ i ];
    if( !vic->client )
      continue;
    if( vic->client->pers.connected != CON_CONNECTED )
      continue;
    if( vic->client->pers.teamSelection == TEAM_NONE )
      continue;
    if( vic->client->ps.ping == 999 )
    {
      G_ChangeTeam( vic, TEAM_NONE );
      AP( va( "print \"^3!spec999: ^7%s^7 moved ^7%s^7 to spectators\n\"",
        ( ent ) ? ent->client->pers.netname : "console",
        vic->client->pers.netname ) );
    }
  }
  return qtrue;
}

qboolean G_admin_register(gentity_t *ent, int skiparg )
{
  int level = 0;
  //char nothing[ MAX_STRING_CHARS ];
  //char err[ MAX_STRING_CHARS ];

  if( !ent )
  {
    ADMP( va( "^3!register: ^7Cannot be run as console\n" ) );
    return qfalse;
  }

  level = G_admin_level(ent);

  if( level == 0 )
   level = 1;

  if( !Q_stricmp( ent->client->pers.guid, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) )
  {
    ADMP( va( "^3!register: ^7 You cannot register for name protection until you update your client. Please replace your client executable with the one at http://trem.tjw.org/backport/ and reconnect. Updating your client will also allow you to have faster map downloads.\n" ) );
    return qfalse;
  }

  if( !Q_stricmp( ent->client->pers.netname, "UnnamedPlayer" ) )
  {
	ADMP("^3!register: invalid name\n");
	return qfalse;
  }

/*
  if( !G_admin_name_check( ent, ent->client->pers.netname, err, sizeof( nothing ) ) )
  {
    ADMP( va( "^3!register: ^7%s\n", err ) );
    ADMP( va( "^3!register: ^7Error saving admin\n" ) );
    return qfalse;
  }
*/

  trap_SendConsoleCommand( EXEC_APPEND,va( "!setlevel %d %d;",ent - g_entities, level) );
  ClientUserinfoChanged( ent - g_entities );

  AP( va( "print \"^3!register: ^7%s^7 is now a protected nickname.\n\"", ent->client->pers.netname) );

  return qtrue;
}

qboolean G_admin_rename( gentity_t *ent, int skiparg )
{
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ];
  char newname[ MAX_NAME_LENGTH ];
  char oldname[ MAX_NAME_LENGTH ];
  char err[ MAX_STRING_CHARS ];
  char userinfo[ MAX_INFO_STRING ];
  char *s;
  gentity_t *victim = NULL;

  if( G_SayArgc() < 3 + skiparg )
  {
    ADMP( "^3!rename: ^7usage: !rename [name] [newname]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  s = G_SayConcatArgs( 2 + skiparg );
  Q_strncpyz( newname, s, sizeof( newname ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!rename: ^7%s\n", err ) );
    return qfalse;
  }
  victim = &g_entities[ pids[ 0 ] ] ;
  if( !admin_higher( ent, victim ) )
  {
    ADMP( "^3!rename: ^7sorry, but your intended victim has a higher admin"
        " level than you\n" );
    return qfalse;
  }
  if( !G_admin_name_check( victim, newname, err, sizeof( err ) ) )
  {
    ADMP( va( "^3!rename: ^7%s\n", err ) );
    return qfalse;
  }
  level.clients[ pids[ 0 ] ].pers.nameChanges--;
  level.clients[ pids[ 0 ] ].pers.nameChangeTime = 0;
  trap_GetUserinfo( pids[ 0 ], userinfo, sizeof( userinfo ) );
  s = Info_ValueForKey( userinfo, "name" );
  Q_strncpyz( oldname, s, sizeof( oldname ) );
  Info_SetValueForKey( userinfo, "name", newname );
  trap_SetUserinfo( pids[ 0 ], userinfo );
  ClientUserinfoChanged( pids[ 0 ] );
  if( strcmp( oldname, level.clients[ pids[ 0 ] ].pers.netname ) )
    AP( va( "print \"^3!rename: ^7%s^7 has been renamed to %s^7 by %s\n\"",
        oldname,
        level.clients[ pids[ 0 ] ].pers.netname,
        ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_restart( gentity_t *ent, int skiparg )
{
  char layout[ MAX_CVAR_VALUE_STRING ] = { "" };
  char teampref[ MAX_CVAR_VALUE_STRING ] = { "" };
  int i;
  //gclient_t *cl;

  if( G_SayArgc( ) > 1 + skiparg )
  {
    char map[ MAX_QPATH ];

    trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
    G_SayArgv( skiparg + 1, layout, sizeof( layout ) );

  if( Q_stricmp( layout, "keepteams" ) && Q_stricmp( layout, "keepteamslock" ) && Q_stricmp( layout, "switchteams" ) && Q_stricmp( layout, "switchteamslock" ) )
    {
        if( !Q_stricmp( layout, "*BUILTIN*" ) ||
          trap_FS_FOpenFile( va( "layouts/%s/%s.dat", map, layout ),
            NULL, FS_READ ) > 0 )
        {
          trap_Cvar_Set( "g_layouts", layout );
        }
        else
        {
          ADMP( va( "^3!restart: ^7layout '%s' does not exist\n", layout ) );
          return qfalse;
        }
    }
    else
    {
 strcpy(layout,"");
   G_SayArgv( skiparg + 1, teampref, sizeof( teampref ) );
    }
  }



   if( G_SayArgc( ) > 2 + skiparg )
   {
     G_SayArgv( skiparg + 2, teampref, sizeof( teampref ) );
   }

/*
   if( !Q_stricmp( teampref, "keepteams" ) || !Q_stricmp( teampref, "keepteamslock" ) )
   {
     for( i = 0; i < g_maxclients.integer; i++ )
     {
       cl = level.clients + i;
       if( cl->pers.connected != CON_CONNECTED )
         continue;

       if( cl->pers.teamSelection == TEAM_NONE )
         continue;

       cl->sess.restartTeam = cl->pers.teamSelection;
     }
   }
   else if(!Q_stricmp( teampref, "switchteams" ) ||  !Q_stricmp( teampref, "switchteamslock" ))
   {
     for( i = 0; i < g_maxclients.integer; i++ )
     {
       cl = level.clients + i;
       if( cl->pers.connected != CON_CONNECTED )
         continue;

       if( cl->pers.teamSelection == TEAM_NONE )
         continue;

       if( cl->pers.teamSelection == TEAM_ALIENS )
         cl->sess.restartTeam = TEAM_HUMANS;
       else if(cl->pers.teamSelection == TEAM_HUMANS )
    cl->sess.restartTeam = TEAM_ALIENS;
     }
   }

   if( !Q_stricmp( teampref, "switchteamslock" ) || !Q_stricmp( teampref, "keepteamslock" ) )
   {
     trap_Cvar_Set( "g_lockTeamsAtStart", "1" );
   }
*/
  trap_SendConsoleCommand( EXEC_APPEND, "map_restart" );
   //if(teampref[ 0 ])
     //strcpy(teampref,va( "^7(with teams option: '%s^7')", teampref ));

   AP( va( "print \"^3!restart: ^7map restarted by %s %s %s\n\"",
            ( ent ) ? ent->client->pers.netname : "console",
           ( layout[ 0 ] ) ? va( "^7(forcing layout '%s^7')", layout ) : "",
           teampref ) );
  return qtrue;
}

qboolean G_admin_nextmap( gentity_t *ent, int skiparg )
{
  AP( va( "print \"^3!nextmap: ^7%s^7 decided to load the next map\n\"",
    ( ent ) ? ent->client->pers.netname : "console" ) );
  level.lastWin = TEAM_NONE;
  trap_SetConfigstring( CS_WINNER, "Evacuation" );
  LogExit( va( "nextmap was run by %s",
    ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_namelog( gentity_t *ent, int skiparg )
{
  int i, j;
  char search[ MAX_NAME_LENGTH ] = {""};
  char s2[ MAX_NAME_LENGTH ] = {""};
  char n2[ MAX_NAME_LENGTH ] = {""};
  char guid_stub[ 9 ];
  qboolean found = qfalse;
  int printed = 0;

  if( G_SayArgc() > 1 + skiparg )
  {
    G_SayArgv( 1 + skiparg, search, sizeof( search ) );
    G_SanitiseString( search, s2, sizeof( s2 ) );
  }
  ADMBP_begin();
  for( i = 0; i < MAX_ADMIN_NAMELOGS && g_admin_namelog[ i ]; i++ )
  {
    if( search[0] )
    {
      found = qfalse;
      for( j = 0; j < MAX_ADMIN_NAMELOG_NAMES &&
        g_admin_namelog[ i ]->name[ j ][ 0 ]; j++ )
      {
        G_SanitiseString( g_admin_namelog[ i ]->name[ j ], n2, sizeof( n2 ) );
        if( strstr( n2, s2 ) )
        {
          found = qtrue;
          break;
        }
      }
      if( !found )
        continue;
    }
    printed++;
    for( j = 0; j < 8; j++ )
      guid_stub[ j ] = g_admin_namelog[ i ]->guid[ j + 24 ];
    guid_stub[ j ] = '\0';
    if( g_admin_namelog[ i ]->slot > -1 )
       ADMBP( "^3" );
    ADMBP( va( "%-2s (*%s) %15s^7",
      (g_admin_namelog[ i ]->slot > -1 ) ?
        va( "%d", g_admin_namelog[ i ]->slot ) : "-",
      guid_stub, g_admin_namelog[ i ]->ip ) );
    for( j = 0; j < MAX_ADMIN_NAMELOG_NAMES &&
      g_admin_namelog[ i ]->name[ j ][ 0 ]; j++ )
    {
      ADMBP( va( " '%s^7'", g_admin_namelog[ i ]->name[ j ] ) );
    }
    ADMBP( "\n" );
  }
  ADMBP( va( "^3!namelog:^7 %d recent clients found\n", printed ) );
  ADMBP_end();
  return qtrue;
}

qboolean G_admin_lock( gentity_t *ent, int skiparg )
{
  char teamName[2] = {""};
  team_t team;

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!lock: ^7usage: !lock [a|h]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, teamName, sizeof( teamName ) );
  if( teamName[ 0 ] == 'a' || teamName[ 0 ] == 'A' )
    team = TEAM_ALIENS;
  else if( teamName[ 0 ] == 'h' || teamName[ 0 ] == 'H' )
    team = TEAM_HUMANS;
  else
  {
    ADMP( va( "^3!lock: ^7invalid team\"%c\"\n", teamName[0] ) );
    return qfalse;
  }

  if( team == TEAM_ALIENS )
  {
    if( level.alienTeamLocked )
    {
      ADMP( "^3!lock: ^7Alien team is already locked\n" );
      return qfalse;
    }
    else
      level.alienTeamLocked = qtrue;
  }
  else if( team == TEAM_HUMANS ) {
    if( level.humanTeamLocked )
    {
      ADMP( "^3!lock: ^7Human team is already locked\n" );
      return qfalse;
    }
    else
      level.humanTeamLocked = qtrue;
  }

  AP( va( "print \"^3!lock: ^7%s team has been locked by %s\n\"",
    ( team == TEAM_ALIENS ) ? "Alien" : "Human",
    ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_unlock( gentity_t *ent, int skiparg )
{
  char teamName[2] = {""};
  team_t team;

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!unlock: ^7usage: !unlock [a|h]\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, teamName, sizeof( teamName ) );
  if( teamName[ 0 ] == 'a' || teamName[ 0 ] == 'A' )
    team = TEAM_ALIENS;
  else if( teamName[ 0 ] == 'h' || teamName[ 0 ] == 'H' )
    team = TEAM_HUMANS;
  else
  {
    ADMP( va( "^3!unlock: ^7invalid team\"%c\"\n", teamName[0] ) );
    return qfalse;
  }

  if( team == TEAM_ALIENS )
  {
    if( !level.alienTeamLocked )
    {
      ADMP( "^3!unlock: ^7Alien team is not currently locked\n" );
      return qfalse;
    }
    else
      level.alienTeamLocked = qfalse;
  }
  else if( team == TEAM_HUMANS ) {
    if( !level.humanTeamLocked )
    {
      ADMP( "^3!unlock: ^7Human team is not currently locked\n" );
      return qfalse;
    }
    else
      level.humanTeamLocked = qfalse;
  }

  AP( va( "print \"^3!unlock: ^7%s team has been unlocked by %s\n\"",
    ( team == TEAM_ALIENS ) ? "Alien" : "Human",
    ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}


qboolean G_admin_say( gentity_t *ent, int skiparg )
{
    char *p;

    if( G_SayArgc() < 2 + skiparg )
    {
        return qfalse;
    }

    p = G_SayConcatArgs( 1 + skiparg );

    AP( va( "print \"^7%s^7 said: ^7%s^7\n\"",
            ( ent ) ? ent->client->pers.netname : "console", p ) );

    return qtrue;
}

qboolean G_admin_adminsay( gentity_t *ent, int skiparg )
{
    char *p;

    if( G_SayArgc() < 2 + skiparg )
    {
        return qfalse;
    }

    p = G_SayConcatArgs( 1 + skiparg );

    AP( va( "print \"^7%s^7\n\"",
            p ) );

    return qtrue;
}

qboolean G_admin_bigsay( gentity_t *ent, int skiparg )
{
    char *p;

    if( G_SayArgc() < 2 + skiparg )
    {
        return qfalse;
    }

    p = G_SayConcatArgs( 1 + skiparg );

//    AP( va( "cp \"^7%s^7\n\"",
//            p ) );
    G_ClientCP(NULL, p, NULL, 0);

  AP( va( "print \"^3!bigsay: ^7%s^7 said: %s^7\n\"",
    ( ent ) ? ent->client->pers.netname : "console", p ) );

    return qtrue;
}

void G_Unescape( char *input, char *output, int len );
qboolean G_StringReplaceCvars( char *input, char *output, int len );

qboolean G_admin_info( gentity_t *ent, int skiparg )
{/*
  fileHandle_t infoFile;
  int length, i;
#define MESSAGEBUF_LEN 1000
  char *message, filename[ MAX_OSPATH ], messagebuf[ MESSAGEBUF_LEN ];
  for( i = 0; i < MESSAGEBUF_LEN; i++ )
    messagebuf[ i ] = '\0';
  if( G_SayArgc() == 2 + skiparg )
    G_SayArgv( 1 + skiparg, filename, sizeof( filename ) );
  else if( G_SayArgc() == 1 + skiparg )
    Q_strncpyz( filename, "default", sizeof( filename ) );
  else
  {
    ADMP( "^3!info: ^7usage: ^3!info ^7(^5subject^7)\n" );
    return qfalse;
  }
  Com_sprintf( filename, sizeof( filename ), "info/info-%s.txt", filename );
  length = trap_FS_FOpenFile( filename, &infoFile, FS_READ );
  message = BG_Alloc( length * 2 + 1 );
  if( length <= 0 || !infoFile )
  {
    BG_Free( message );
    trap_FS_FCloseFile( infoFile );
    ADMP( "^3!info: ^7no relevant information is available\n" );
    return qfalse;
  }
  else
  {
    int i;
    char *cr;
    trap_FS_Read( message, length, infoFile );
    *( message + length ) = '\0';
    trap_FS_FCloseFile( infoFile );
    // strip carriage returns for windows platforms
    while( ( cr = strchr( message, '\r' ) ) )
      memmove( cr, cr + 1, strlen( cr + 1 ) + 1 );
#define MAX_INFO_PARSE_LOOPS 100
    for( i = 0; i < MAX_INFO_PARSE_LOOPS &&
        G_StringReplaceCvars( message, message, sizeof( message ) ); i++ );
    G_Unescape( message, message, sizeof( message ) );
    if( i >= MAX_INFO_PARSE_LOOPS )
      G_Printf( S_COLOR_YELLOW "WARNING: %s exceeds MAX_INFO_PARSE_LOOPS\n", filename );
    i = 0;
    while( i < length && *( message + i ) )
    {
      if( i + 1 % ( MESSAGEBUF_LEN ) )
      {
        *( messagebuf + ( i % MESSAGEBUF_LEN ) ) = *( message + i );
      }
      else
      {
        *( messagebuf + ( MESSAGEBUF_LEN - 1 ) ) = '\0';
        ADMP( va( "%s", messagebuf ) );
      }
      i++;
      ADMP( va( "messagebuf: %s\nmessage[ 0 ]: %d\nmessage[ 1 ]: %d\nmessage[ 2 ]: %d\nmessage[ 3 ]: %d\nmessage[ 4 ]: %d\nmessage[ 5 ]: %d\nmessage0: %d\nmessage1: %d\nmessage2: %d\nmessage3: %d\n", messagebuf, message[ 0 ], message[ 1 ], message[ 2 ], message[ 3 ], message[ 4 ], message[ 5 ], *(message+0), *(message+1), *(message+2), *(message+3) ) );
    }
    BG_Free( message );
    *( messagebuf + i ) = '\0';
    ADMP( va( "%s", messagebuf ) );
    return qtrue;
  }
*/




  fileHandle_t f;
  int  len, i=0;
  char *info, *infoPtr, *cr;
  char fileName[ MAX_OSPATH ];
  char line[ MAX_STRING_CHARS ], linebuf[ MAX_STRING_CHARS ];
/*
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return qfalse;
    }
*/
  if( G_SayArgc() == 2 + skiparg )
    G_SayArgv( 1 + skiparg, fileName, MAX_OSPATH );
  else if( G_SayArgc() == 1 + skiparg )
    strcpy( fileName, "default" );
  else
  {
    ADMP( "^3!info: ^7usage: ^3!info ^7(^5subject^7)\n" );
    return qfalse;
  }
  Com_sprintf( fileName, sizeof( fileName ), "info/info-%s.txt", fileName );

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len < 0 )
  {
    trap_FS_FCloseFile( f );
    ADMP( "^3!info: ^7no relevant information is available\n" );
    return qfalse;
  }
  info = BG_Alloc( len + 1 );
  infoPtr = info;
  trap_FS_Read( info, len, f );
  *( info + len ) = '\0';
  trap_FS_FCloseFile( f );

  // strip carriage returns for windows platforms
  while( ( cr = strchr( info, '\r' ) ) )
    memmove( cr, cr + 1, strlen( cr + 1 ) + 1 );
#define MAX_INFO_PARSE_LOOPS 100
  for( i = 0; i < MAX_INFO_PARSE_LOOPS && G_StringReplaceCvars( info, info, len ); i++ );
  G_Unescape( info, info, len );

  strcpy( linebuf, "" );

  while( *info )
  {
    if( i >= sizeof( line ) - 1 )
    {
      BG_Free( info );
      G_Printf( S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
       fileName, line );
      return qfalse;
    }
    line[ i++ ] = *info;
    line[ i ] = '\0';
    if( *info == '\n' )
    {
      i = 0;
//      if( *( info - 1 ) != '\n' )  // nice formatting - implementation somewhat hackish
      if( qtrue )
      {
        if( strlen( linebuf ) + strlen( line ) < MAX_STRING_CHARS - 12 )  // send 24 lines or up to max_string_chars at a time
        {
          strcat( linebuf, line );
        }
        else
        {
          trap_SendServerCommand( ent - g_entities, va( "print \"%s\n\"", linebuf ) );
          strcpy( linebuf, line );
        }
      }
    }
    info++;
  }

  info = infoPtr;

  if( linebuf[0] )
    trap_SendServerCommand( ent - g_entities, va( "print \"%s\n\"", linebuf ) );

  BG_Free( info );
  return qtrue;
}

void G_Unescape( char *input, char *output, int len )
{
  // \n -> newline, \%c -> %c
  // output is terminated at output[len - 1]
  // it's OK for input to equal output, because our position in input is always
  // equal or greater than our position in output
  // however, if output is later in the same string as input, a crash is pretty
  // much inevitable
  int i, j;
  for( i = j = 0; input[i] && j + 1 < len; i++, j++ )
  {
    if( input[i] == '\\' )
    {
      if( !input[++i] )
      {
        output[j] = '\0';
        return;
      }
      else if( input[i] == 'n' )
        output[j] = '\n';
      else
        output[j] = input[i];
    }
    else
      output[j] = input[i];
  }
  output[j] = '\0';
}

qboolean G_StringReplaceCvars( char *input, char *output, int len )
{
  int i, outNum = 0;
  char tmp;
  char cvarName[ 64 ], cvarValue[ MAX_CVAR_VALUE_STRING ];
  char *outputBuffer;
  qboolean doneAnything = qfalse;
  if( len <= 0 )
    return qfalse;
  // use our own internal buffer in case output == input
  outputBuffer = BG_Alloc( len );
  len -= 1; // fit in a terminator
  while( *input && outNum < len )
  {
    if( *input == '\\' && input[1] && outNum < len - 1 )
    {
      outputBuffer[ outNum++ ] = *input++;
      outputBuffer[ outNum++ ] = *input++;
    }
    else if( *input == '$' )
    {
      qboolean brackets = qfalse;
      doneAnything = qtrue;
      input++;
      if( *input == '{' )
      {
        brackets = qtrue;
        input++;
      }
      for( i = 0; *input && ( isalnum( *input ) || *input == '_' || ( brackets && *input != '}' ) ) &&
          i < 63; i++ )
        cvarName[ i ] = *input++;
      cvarName[ i ] = '\0';
      if( *input == '}' )
      {
        input++;
      }

      tmp = cvarName[strlen("oc-rating")];
      cvarName[strlen("oc-rating")] = 0;
      if(strcmp(cvarName, "oc-rating") == 0)
      {
        // ${oc-rating,atcs,oc}
        char *s;
        char map[ MAX_STRING_CHARS ] = {""};
        char layout[ MAX_STRING_CHARS ] = {""};

        cvarValue[0] = 0;
        cvarName[strlen("oc-rating")] = tmp;
        s = cvarName + strlen("oc-rating");
        while(*s == ',') s++;
        i = 0;
        while(*s != ',')
        {
            if(!*s)
            {
                break;
            }

            map[i++] = *s;
            map[i] = 0;

            s++;
        }
        i = 0;
        while(*s == ',') s++;
        while(*s != ',')
        {
            if(!*s)
            {
                break;
            }

            layout[i++] = *s;
            layout[i] = 0;

            s++;
        }
        if(s && map[0] && layout[0] && (s = G_OC_Rating(map, layout)) && s[0])
        {
            Q_strncpyz(cvarValue, s, sizeof(cvarValue));
        }
      }
      else
      {
        cvarName[strlen("oc-rating")] = tmp;
        trap_Cvar_VariableStringBuffer( cvarName, cvarValue, sizeof( cvarValue ) );
      }

      if( cvarValue[ 0 ] )
      {
        for( i = 0; cvarValue[ i ] && outNum < len; i++ )
          outputBuffer[ outNum++ ] = cvarValue[ i ];
      }
    }
    else
      outputBuffer[ outNum++ ] = *input++;
  }
  outputBuffer[ outNum ] = '\0';
  Q_strncpyz( output, outputBuffer, len );
  BG_Free( outputBuffer );
  return doneAnything;
}

qboolean G_admin_endscrim( gentity_t *ent, int skiparg )
{
  if( !BG_OC_OCMode() )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return qfalse;
  }

  if( level.ocScrimState <= G_OC_STATE_NONE )
  {
    ADMP( va( "Can only be used during an OC scrim\n" ) );
    return qfalse;
  }

  G_OC_EndScrim( );

  AP( va( "print \"^3!endscrim: ^7%s^7 ended the scrim^7\n\"", ( ent ) ? ent->client->pers.netname : "console" ) );
  return qtrue;
}

qboolean G_admin_startscrim( gentity_t *ent, int skiparg )
{
  char win[ 7 ];

  if( !BG_OC_OCMode() )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return qfalse;
  }

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!startscrim: ^7usage: !startscrim [m/a] for all medis / armoury\n" );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, win, sizeof( win ) );

  if( !win[0] || !( win[0] == 'm' || win[0] == 'a' ) )
  {
    ADMP( "^3!startscrim: ^7usage: !startscrim [m/a] for all medis / armoury\n" );
    return qfalse;
  }

  if( win[0] == 'a' )
    level.ocScrimMode = G_OC_MODE_ARM;
  else
    level.ocScrimMode = G_OC_MODE_MEDI;

  if( level.ocScrimMode == G_OC_MODE_ARM && level.totalArmouries <= 0 )
  {
    ADMP( "^3!startscrim: ^7there are no armouries for an arm scrim\n" );
    return qfalse;
  }
  if( level.ocScrimMode == G_OC_MODE_MEDI && level.totalMedistations <= 0 )
  {
    ADMP( "^3!startscrim: ^7there are no medis for a medi scrim\n" );
    return qfalse;
  }

  level.ocStartTime = level.time;
  level.ocScrimState = G_OC_STATE_PREP;

  AP( va( "print \"^3!startscrim: ^7%s^7 started the oc scrim - first team to use\n%s^7.^7\n\"", ( ent ) ? ent->client->pers.netname : "console", ( ( level.ocScrimMode == G_OC_MODE_ARM ) ? ( level.totalArmouries == 1 || G_OC_TestLayoutFlag( level.layout, G_OC_OCFLAG_ONEARM ) ? "the ^3armoury^7" : "every ^3armoury^7" ) : ( level.totalMedistations == 1 ? "the ^3medical station^7" : "every ^3medical station^7" ) ) ) );
  return qtrue;
}
/*
qboolean G_admin_designate( gentity_t *ent, int skiparg )
{
  int asdfval=0;
  int pids[ MAX_CLIENTS ];
  char name[ MAX_NAME_LENGTH ], err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( "^3!designate: ^7usage: designate [name|slot#]\n" );
    return qfalse;
  }
  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  if( (asdfval=G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) )!= 1 )
  {
    G_MatchOnePlayer( pids, asdfval, err, sizeof( err ) );
    ADMP( va( "^3!designate: ^7%s\n", err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) &&
    !Q_stricmp( cmd, "undesignate" ) )
  {
    ADMP( "^3!mute: ^7sorry, but your intended victim has a higher admin"
        " level than you\n" );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( vic->client->pers.designatedBuilder == qtrue )
  {
    if( !Q_stricmp( cmd, "designate" ) )
    {
      ADMP( "^3!designate: ^7player is already designated builder\n" );
      return qtrue;
    }
    vic->client->pers.designatedBuilder = qfalse;
    CPx( pids[ 0 ], "cp \"^1Your designation has been revoked\"" );
    AP( va(
      "print \"^3!designate: ^7%s^7's designation has been revoked by %s\n\"",
       vic->client->pers.netname,
       ( ent ) ? ent->client->pers.netname : "console" ) );
    G_CheckDBProtection( );
  }
  else
  {
    if( !Q_stricmp( cmd, "undesignate" ) )
    {
      ADMP( "^3!undesignate: ^7player is not currently designated builder\n" );
      return qtrue;
    }
    vic->client->pers.designatedBuilder = qtrue;
    CPx( pids[ 0 ], "cp \"^1You've been designated\"" );
    AP( va( "print \"^3!designate: ^7%s^7 has been designated by ^7%s\n\"",
      vic->client->pers.netname,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}
*/
qboolean G_admin_putmespec( gentity_t *ent, int skiparg )
{
  if( !ent )
  {
    ADMP( "!specme: sorry, but console isn't allowed on the spectators team\n");
    return qfalse;
  }

  if(ent->client->pers.teamSelection == TEAM_NONE)
    return qfalse;

    //guard against build timer exploit
  if( ent->client->pers.teamSelection != TEAM_NONE &&
     ( ent->client->ps.stats[ STAT_CLASS ] == PCL_ALIEN_BUILDER0 ||
       ent->client->ps.stats[ STAT_CLASS ] == PCL_ALIEN_BUILDER0_UPG ||
       BG_InventoryContainsWeapon( WP_HBUILD, ent->client->ps.stats ) ||
       BG_InventoryContainsWeapon( WP_HBUILD, ent->client->ps.stats ) ) &&
      ent->client->ps.stats[ STAT_MISC ] > 0 )
  {
    ADMP("!specme: You cannot leave your team until the build timer expires");
    return qfalse;
  }

  G_ChangeTeam( ent, TEAM_NONE );
  AP( va("print \"^3!specme: ^7%s^7 decided to join the spectators\n\"", ent->client->pers.netname ) );
  return qtrue;
}

qboolean G_admin_override( gentity_t *ent, int skiparg )
{ //this is all very similar to denybuild,
  //it performs an essentially identical function
  int pids[ MAX_CLIENTS ], found;
  char name[ MAX_NAME_LENGTH ], err[ MAX_STRING_CHARS ];
  char command[ MAX_ADMIN_CMD_LEN ], *cmd;
  gentity_t *vic;

  G_SayArgv( skiparg, command, sizeof( command ) );
  cmd = command;
  if( cmd && *cmd == '!' )
    cmd++;

  if ( ent && !g_cheats.integer && !G_admin_canEditOC( ent ) )
  {
    ADMP( va( "^3!%s: ^7Cheats are not enabled on this server\n", cmd ) );
    return qfalse;
  }

  if( G_SayArgc() < 2 + skiparg )
  {
    ADMP( va( "^3!%s: ^7usage: !%s [name|slot#]\n", cmd, cmd ) );
    return qfalse;
  }
  G_SayArgv( 1 + skiparg, name, sizeof( name ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "^3!%s: ^7%s\n", cmd, err ) );
    return qfalse;
  }
  if( !admin_higher( ent, &g_entities[ pids[ 0 ] ] ) )
  {
    ADMP( va( "^3!%s: ^7sorry, but your intended victim has a higher admin"
              " level than you\n", cmd ) );
    return qfalse;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( !vic->client->pers.override )
  {
    if( !Q_stricmp( cmd, "cheat-do" ) )
    {
      ADMP( "^3!cheat-do: ^7player already has no overriding rights\n" );
      return qtrue;
    }
    vic->client->pers.hasCheated = 1;
    vic->client->pers.override = qtrue;
    vic->client->pers.noAuO = 1;
    CPx( pids[ 0 ], "cp \"^1You've regained your overriding rights\"" );
    if (ent)
    AP( va(
      "print \"^3!cheat-ao: ^7overriding rights for ^7%s^7 restored by %s\n\"",
      vic->client->pers.netname,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  else
  {
    if( !Q_stricmp( cmd, "cheat-ao" ) )
    {
      ADMP( "^3!cheat-ao: ^7player already has overriding rights\n" );
      return qtrue;
    }
    vic->client->pers.hasCheated = 1;
    vic->client->pers.override = qfalse;
    vic->client->pers.noAuO = 1;
    if( vic->client->pers.denyBuild )
    {
      vic->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
      if( vic->client->ps.stats[ STAT_CLASS ]== PCL_ALIEN_BUILDER0 || vic->client->ps.stats[ STAT_CLASS ] == PCL_ALIEN_BUILDER0_UPG )
      {
        vic->suicideTime = level.time + 1000;
      }
    }
    CPx( pids[ 0 ], "cp \"^1You've lost your overriding rights\"" );
    if( ent )
    AP( va(
      "print \"^3!cheat-do: ^7overriding rights for ^7%s^7 revoked by ^7%s\n\"",
      vic->client->pers.netname,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }
  ClientUserinfoChanged( pids[ 0 ] );
  return qtrue;
}

/*
================
 G_admin_print

 This function facilitates the ADMP define.  ADMP() is similar to CP except
 that it prints the message to the server console if ent is not defined.
================
*/
void G_admin_print( gentity_t *ent, char *m )
{

  if( ent )
    trap_SendServerCommand( ent - level.gentities, va( "print \"%s\"", m ) );
  else
  {
    char m2[ MAX_STRING_CHARS ];
    G_DecolorString( m, m2, sizeof(m2) );
    G_Printf( m2 );
  }
}

void G_admin_buffer_begin()
{
  g_bfb[ 0 ] = '\0';
}

void G_admin_buffer_end( gentity_t *ent )
{
  ADMP( g_bfb );
}

void G_admin_buffer_print( gentity_t *ent, char *m )
{
  // 1022 - strlen("print 64 \"\"") - 1
  if( strlen( m ) + strlen( g_bfb ) >= 1009 )
  {
    ADMP( g_bfb );
    g_bfb[ 0 ] = '\0';
  }
  Q_strcat( g_bfb, sizeof( g_bfb ), m );
}


void G_admin_cleanup()
{
  int i = 0;

  for( i = 0; i < MAX_ADMIN_LEVELS && g_admin_levels[ i ]; i++ )
  {
    BG_Free( g_admin_levels[ i ] );
    g_admin_levels[ i ] = NULL;
  }
  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    BG_Free( g_admin_admins[ i ] );
    g_admin_admins[ i ] = NULL;
  }
  for( i = 0; i < MAX_ADMIN_BANS && g_admin_bans[ i ]; i++ )
  {
    BG_Free( g_admin_bans[ i ] );
    g_admin_bans[ i ] = NULL;
  }
  for( i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[ i ]; i++ )
  {
    BG_Free( g_admin_hides[ i ] );
    g_admin_hides[ i ] = NULL;
  }
  for( i = 0; i < MAX_ADMIN_COMMANDS && g_admin_commands[ i ]; i++ )
  {
    BG_Free( g_admin_commands[ i ] );
    g_admin_commands[ i ] = NULL;
  }
}
