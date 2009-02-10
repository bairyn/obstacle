/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

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

#include "g_local.h"

static char buf[MAX_STRING_CHARS];  // big buffer used by cp functions

/*
==================
G_SanitiseName

Remove case and control characters from a player name
==================
*/
void G_SanitiseName( char *in, char *out )
{
  qboolean skip = qtrue;
  int spaces = 0;
  int out_len = 0;

  while( *in && out_len < MAX_NAME_LENGTH - 1 )
  {
    // strip leading white space
    if( *in == ' ' )
    {
      if( skip )
      {
        in++;
        continue;
      }
      spaces++;
    }
    else
    {
      spaces = 0;
      skip = qfalse;
    }

    if( *in == 27 || *in == '^' )
    {
      in += 2;    // skip color code
      continue;
    }

    if( *in < 32 )
    {
      in++;
      continue;
    }

    *out++ = tolower( *in++ );
    out_len++;
  }
  out -= spaces;
  *out = 0;
}


/*
==================
G_ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int G_ClientNumberFromString( gentity_t *to, char *s )
{
  gclient_t *cl;
  int       i;
  char      s2[ MAX_STRING_CHARS ];
  char      n2[ MAX_STRING_CHARS ];

  // numeric values are just slot numbers
  for( i = 0; s[ i ] && isdigit( s[ i ] ); i++ );
  if( !s[ i ] )
  {
    i = atoi( s );

    if( i < 0 || i >= level.maxclients )
      return -1;

    cl = &level.clients[ i ];

    if( cl->pers.connected == CON_DISCONNECTED )
      return -1;

    return i;
  }

  // check for a name match
  G_SanitiseName( s, s2 );

  for( i = 0, cl = level.clients; i < level.maxclients; i++, cl++ )
  {
    if( cl->pers.connected == CON_DISCONNECTED )
      continue;

    G_SanitiseName( cl->pers.netname, n2 );

    if( !strcmp( n2, s2 ) )
      return i;
  }

  return -1;
}


/*
==================
G_MatchOnePlayer

This is a companion function to G_ClientNumbersFromString()

err will be populated with an error message.
==================
*/
void G_MatchOnePlayer( int *plist, int num, char *err, int len )
{
  gclient_t *cl;
  int i;
  char line[ MAX_NAME_LENGTH + 10 ] = {""};

  err[ 0 ] = '\0';
  if( num == 0 )
  {
    Q_strcat( err, len, "no connected player by that name or slot #" );
  }
  else if( num > 1 )
  {
    Q_strcat( err, len, "more than one player name matches. "
            "be more specific or use the slot #:\n" );
    for( i = 0; i < num; i++ )
    {
      cl = &level.clients[ plist[ i ] ];
      if( cl->pers.connected == CON_DISCONNECTED )
        continue;
      Com_sprintf( line, sizeof( line ), "%2i - %s^7\n",
        plist[ i ], cl->pers.netname );
      if( strlen( err ) + strlen( line ) > len )
        break;
      Q_strcat( err, len, line );
    }
  }
}

/*
==================
G_ClientNumbersFromString

Sets plist to an array of integers that represent client numbers that have
names that are a partial match for s.

Returns number of matching clientids up to max.
==================
*/
int G_ClientNumbersFromString( char *s, int *plist, int max )
{
  gclient_t *p;
  int i, found = 0;
  char n2[ MAX_NAME_LENGTH ] = {""};
  char s2[ MAX_NAME_LENGTH ] = {""};

  // if a number is provided, it might be a slot #
  for( i = 0; s[ i ] && isdigit( s[ i ] ); i++ );
  if( !s[ i ] )
  {
    i = atoi( s );
    if( i >= 0 && i < level.maxclients )
    {
      p = &level.clients[ i ];
      if( p->pers.connected != CON_DISCONNECTED )
      {
        *plist = i;
        return 1;
      }
    }
    // we must assume that if only a number is provided, it is a clientNum
    return 0;
  }

  // now look for name matches
  G_SanitiseName( s, s2 );
  if( strlen( s2 ) < 1 )
    return 0;
  for( i = 0; i < level.maxclients && found <= max; i++ )
  {
    p = &level.clients[ i ];
    if( p->pers.connected == CON_DISCONNECTED )
    {
      continue;
    }
    G_SanitiseName( p->pers.netname, n2 );
    if( strstr( n2, s2 ) )
    {
      *plist++ = i;
      found++;
    }
  }
  return found;
}

/*
==================
ScoreboardMessage

==================
*/
void ScoreboardMessage( gentity_t *ent )
{
  char      entry[ 1024 ];
  char      string[ 1400 ];
  int       stringlength;
  int       i, j;
  gclient_t *cl;
  int       numSorted;
  weapon_t  weapon = WP_NONE;
  upgrade_t upgrade = UP_NONE;

  // send the latest information on all clients
  string[ 0 ] = 0;
  stringlength = 0;

  numSorted = level.numConnectedClients;

  for( i = 0; i < numSorted; i++ )
  {
    int   ping;

    cl = &level.clients[ level.sortedClients[ i ] ];

    if( cl->pers.connected == CON_CONNECTING )
      ping = -1;
    else if( cl->sess.spectatorState == SPECTATOR_FOLLOW )
      ping = cl->pers.ping < 999 ? cl->pers.ping : 999;
    else
      ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

    //If (loop) client is a spectator, they have nothing, so indicate such.
    //Only send the client requesting the scoreboard the weapon/upgrades information for members of their team. If they are not on a team, send it all.
    if( cl->sess.sessionTeam != TEAM_SPECTATOR &&
      (ent->client->pers.teamSelection == PTE_NONE || cl->pers.teamSelection == ent->client->pers.teamSelection ) )
    {
      weapon = cl->ps.weapon;

      if( BG_InventoryContainsUpgrade( UP_BATTLESUIT, cl->ps.stats ) )
        upgrade = UP_BATTLESUIT;
      else if( BG_InventoryContainsUpgrade( UP_JETPACK, cl->ps.stats ) )
        upgrade = UP_JETPACK;
      else if( BG_InventoryContainsUpgrade( UP_BATTPACK, cl->ps.stats ) )
        upgrade = UP_BATTPACK;
      else if( BG_InventoryContainsUpgrade( UP_HELMET, cl->ps.stats ) )
        upgrade = UP_HELMET;
      else if( BG_InventoryContainsUpgrade( UP_LIGHTARMOUR, cl->ps.stats ) )
        upgrade = UP_LIGHTARMOUR;
      else
        upgrade = UP_NONE;
    }
    else
    {
      weapon = WP_NONE;
      upgrade = UP_NONE;
    }

    Com_sprintf( entry, sizeof( entry ),
      " %d %d %d %d %d %d", level.sortedClients[ i ], cl->pers.score, ping,
      ( level.time - cl->pers.enterTime ) / 60000, weapon, upgrade );

    j = strlen( entry );

    if( stringlength + j > 1024 )
      break;

    strcpy( string + stringlength, entry );
    stringlength += j;
  }

  trap_SendServerCommand( ent-g_entities, va( "scores %i %i %i%s", i,
    level.alienKills, level.humanKills, string ) );
}


/*
==================
ConcatArgs
==================
*/
char *ConcatArgs( int start )
{
  int         i, c, tlen;
  static char line[ MAX_STRING_CHARS ];
  int         len;
  char        arg[ MAX_STRING_CHARS ];

  len = 0;
  c = trap_Argc( );

  for( i = start; i < c; i++ )
  {
    trap_Argv( i, arg, sizeof( arg ) );
    tlen = strlen( arg );

    if( len + tlen >= MAX_STRING_CHARS - 1 )
      break;

    memcpy( line + len, arg, tlen );
    len += tlen;

    if( i != c - 1 )
    {
      line[ len ] = ' ';
      len++;
    }
  }

  line[ len ] = 0;

  return line;
}

/*
==================
G_ParseLayoutFlags

Used to cat onto vote strings
==================
*/

void G_ParseLayoutFlags( char *layout, char *out )
{
  int  num = 0;
  char ret[ MAX_STRING_CHARS ];

  strcpy( out, "" );

  if( !layout || !layout[0] || *(layout) != 'o' || *((layout) + 1) != 'c' )  // must be an oc
    return;

  if( !G_LayoutExtraFlags( layout ) )  // no extra flags
    return;

  strcpy( ret, " (layout uses options '" );

  if( G_TestLayoutFlag( layout, OCFL_ONEARM ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ONEARM_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_NOCREEP ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_NOCREEP_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ALIENONLY ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ALIENONLY_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_BOTHTEAMS ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_BOTHTEAMS_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_NOWALLWALK ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_NOWALLWALK_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_NOALIENTURRETFIRE ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_NOALIENTURRETFIRE_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_NOALIENTESLAFIRE ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_NOALIENTESLAFIRE_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_NOALIENREACTORFIRE ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_NOALIENREACTORFIRE_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_AGRANGER ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_AGRANGER_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_AGRANGERUPG ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_AGRANGERUPG_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ADRETCH ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ADRETCH_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ABASILISK ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ABASILISK_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ABASILISKUPG ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ABASILISKUPG_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_AMARAUDER ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_AMARAUDER_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_AMARAUDERUPG ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_AMARAUDERUPG_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ADRAGOON ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ADRAGOON_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ADRAGOONUPG ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ADRAGOONUPG_NAME );
  }

  if( G_TestLayoutFlag( layout, OCFL_ATYRANT ) )
  {
    if( num++ )
      strcat( ret, ", " );
    strcat( ret, OCFL_ATYRANT_NAME );
  }

  strcat( ret, "')" );
  strcpy( out, ret );
}

/*
==================
G_TestLayoutFlag

Similar to G_admin_permission
==================
*/

qboolean G_TestLayoutFlag( char *layout, char *flag )
{
  int i;
  char *flagPtr = flag;    // the flag to test
  char *flags   = layout;  // the layout to test

  G_ToLowerCase(flags);
  G_ToLowerCase(layout);

  if( !layout || !layout[0] || *(layout) != 'o' || *((layout) + 1) != 'c' )  // must be an oc
    return qfalse;

  for( i = 0; i < strlen( "oc" ); i++ )
    flags++;

  while( *flags )
  {
    if( *flags == '_' )
    {
      break;
    }
    else if( *flags == *flagPtr )
    {
      if( *flagPtr != '^' )
        return qtrue;
      while( *flags == '^' && *(flags++) == *(flagPtr++) )
        if( *flags == '_' )
          break;
      if( *flags == '_' )
        break;
      if( *flags == *flagPtr && *flags != '_' && *flagPtr != '_' && *flags != '^' )
        return qtrue;
      else
        break;
    }
    else if( *flags == '^' )
    {
      while( *(flags++) == '^' )
        if( *flags == '_' )
          break;
      continue;
    }

    flags++;
  }

  return qfalse;
}

/*
==================
G_LayoutExtraFlags

Test for any defined flags
==================
*/

qboolean G_LayoutExtraFlags( char *layout )
{
  if( G_TestLayoutFlag( layout, OCFL_ALIENONLY ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ONEARM ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_NOCREEP ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_BOTHTEAMS ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_NOWALLWALK ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_NOALIENTURRETFIRE ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_NOALIENTESLAFIRE ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_NOALIENREACTORFIRE ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_AGRANGER ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_AGRANGERUPG ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ADRETCH ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ABASILISK ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ABASILISKUPG ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_AMARAUDER ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_AMARAUDERUPG ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ADRAGOON ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ADRAGOONUPG ) )
    return qtrue;

  if( G_TestLayoutFlag( layout, OCFL_ATYRANT ) )
    return qtrue;

  return qfalse;
}

/*
==================
G_Flood_Limited

Determine whether a user is flood limited, and adjust their flood demerits
==================
*/

qboolean G_Flood_Limited( gentity_t *ent )
{
  int millisSinceLastCommand;
  int maximumDemerits;

  // This shouldn't be called if g_floodMinTime isn't set, but handle it anyway.
  if( !g_floodMinTime.integer )
    return qfalse;

  // Do not limit admins with no censor/flood flag
  if( G_admin_permission( ent, ADMF_NOCENSORFLOOD ) )
   return qfalse;

  millisSinceLastCommand = level.time - ent->client->pers.lastFloodTime;
  if( millisSinceLastCommand < g_floodMinTime.integer )
    ent->client->pers.floodDemerits += ( g_floodMinTime.integer - millisSinceLastCommand );
  else
  {
    ent->client->pers.floodDemerits -= ( millisSinceLastCommand - g_floodMinTime.integer );
    if( ent->client->pers.floodDemerits < 0 )
      ent->client->pers.floodDemerits = 0;
  }

  ent->client->pers.lastFloodTime = level.time;

  // If g_floodMaxDemerits == 0, then we go against g_floodMinTime^2.

  if( !g_floodMaxDemerits.integer )
     maximumDemerits = g_floodMinTime.integer * g_floodMinTime.integer / 1000;
  else
     maximumDemerits = g_floodMaxDemerits.integer;

  if( ent->client->pers.floodDemerits > maximumDemerits )
     return qtrue;

  return qfalse;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f( gentity_t *ent )
{
  char      *name;
  qboolean  give_all = qfalse;

  name = ConcatArgs( 1 );
  if( Q_stricmp( name, "all" ) == 0 )
    give_all = qtrue;

  if( give_all || Q_stricmp( name, "health" ) == 0 )
  {
    if(!g_devmapNoGod.integer)
    {
     ent->health = ent->client->ps.stats[ STAT_MAX_HEALTH ];
     BG_AddUpgradeToInventory( UP_MEDKIT, ent->client->ps.stats );
    }
  }

  if( give_all || Q_stricmpn( name, "funds", 5 ) == 0 )
  {
    int credits = give_all ? HUMAN_MAX_CREDITS : atoi( name + 6 );
    G_AddCreditToClient( ent->client, credits, qtrue );
  }

  if( give_all || Q_stricmp( name, "stamina" ) == 0 )
    ent->client->ps.stats[ STAT_STAMINA ] = MAX_STAMINA;

  if( Q_stricmp( name, "poison" ) == 0 )
  {
    ent->client->ps.stats[ STAT_STATE ] |= SS_BOOSTED;
    ent->client->lastBoostedTime = level.time;
  }

  if( give_all || Q_stricmp( name, "ammo" ) == 0 )
  {
    int maxAmmo, maxClips;
    gclient_t *client = ent->client;

    if( client->ps.weapon != WP_ALEVEL3_UPG &&
        BG_FindInfinteAmmoForWeapon( client->ps.weapon ) )
      return;

    BG_FindAmmoForWeapon( client->ps.weapon, &maxAmmo, &maxClips );

    if( BG_FindUsesEnergyForWeapon( client->ps.weapon ) &&
        BG_InventoryContainsUpgrade( UP_BATTPACK, client->ps.stats ) )
      maxAmmo = (int)( (float)maxAmmo * BATTPACK_MODIFIER );

    BG_PackAmmoArray( client->ps.weapon, client->ps.ammo, client->ps.misc, maxAmmo, maxClips );
  }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f( gentity_t *ent )
{
  char  *msg;

 if( !g_devmapNoGod.integer )
 {
  ent->flags ^= FL_GODMODE;

  if( !( ent->flags & FL_GODMODE ) )
    msg = "godmode OFF\n";
  else
    msg = "godmode ON\n";
 }
 else
 {
  msg = "Godmode has been disabled.\n";
 }

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}


/*
==================
Cmd_Speed_f

Sets client to speedmode

argv(0) speed
==================
*/
void Cmd_Speed_f( gentity_t *ent )
{
  char  *msg;

  ent->client->pers.speed = !ent->client->pers.speed;

  if( !ent->client->pers.speed )
    msg = "speedmode OFF\n";
  else
    msg = "speedmode ON\n";

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent )
{
  char  *msg;

  ent->flags ^= FL_NOTARGET;

  if( !( ent->flags & FL_NOTARGET ) )
    msg = "notarget OFF\n";
  else
    msg = "notarget ON\n";

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent )
{
  char  *msg;

 if( !g_devmapNoGod.integer )
 {
  if( ent->client->noclip )
    msg = "noclip OFF\n";
  else
    msg = "noclip ON\n";

  ent->client->noclip = !ent->client->noclip;
 }
 else
 {
  msg = "Godmode has been disabled.\n";
 }

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
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
  BeginIntermission( );
  trap_SendServerCommand( ent - g_entities, "clientLevelShot" );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent )
{
  if( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING )
    return;

  if( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Leave the hovel first (use your destroy key)\n\"" );
    return;
  }

  if( g_cheats.integer || level.oc )
  {
    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[ STAT_HEALTH ] = ent->health = 0;
    player_die( ent, ent, ent, 100000, MOD_SUICIDE );
  }
  else
  {
    if( ent->suicideTime == 0 )
    {
      trap_SendServerCommand( ent-g_entities, "print \"You will suicide in 20 seconds\n\"" );
      ent->suicideTime = level.time + 20000;
    }
    else if( ent->suicideTime > level.time )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Suicide canceled\n\"" );
      ent->suicideTime = 0;
    }
  }
}

/*
==================
G_LeaveTeam
==================
*/
void G_LeaveTeam( gentity_t *self )
{
  pTeam_t   team = self->client->pers.teamSelection;
  gentity_t *ent;
  int       i;

//  if( team == PTE_ALIENS && !level.oc )
  if( team == PTE_ALIENS )
    G_RemoveFromSpawnQueue( &level.alienSpawnQueue, self->client->ps.clientNum );
//  else if( team == PTE_HUMANS && !level.oc )
  else if( team == PTE_HUMANS )
    G_RemoveFromSpawnQueue( &level.humanSpawnQueue, self->client->ps.clientNum );
  else
    return;

  G_TeamVote( self, qfalse );

  for( i = 0; i < level.num_entities; i++ )
  {
    ent = &g_entities[ i ];
    if( !ent->inuse )
      continue;

    // clean up projectiles
    if( ent->s.eType == ET_MISSILE && ent->r.ownerNum == self->s.number )
      G_FreeEntity( ent );
    if( ent->client && ent->client->pers.connected == CON_CONNECTED )
    {
      // cure poison
      if( ent->client->ps.stats[ STAT_STATE ] & SS_POISONCLOUDED &&
          ent->client->lastPoisonCloudedClient == self )
        ent->client->ps.stats[ STAT_STATE ] &= ~SS_POISONCLOUDED;
      if( ent->client->ps.stats[ STAT_STATE ] & SS_POISONED &&
          ent->client->lastPoisonClient == self )
        ent->client->ps.stats[ STAT_STATE ] &= ~SS_POISONED;
    }
  }
}

/*
=================
G_ChangeTeam
=================
*/
void G_ChangeTeam( gentity_t *ent, pTeam_t newTeam )
{
  pTeam_t oldTeam = ent->client->pers.teamSelection;
  qboolean isFixingImbalance=qfalse;

  if( oldTeam == newTeam )
    return;

  // stop any following clients
  if( !level.oc )
      G_StopFromFollowing( ent );
  G_LeaveTeam( ent );
  ent->client->pers.teamSelection = newTeam;

   if ( ( level.numAlienClients - level.numHumanClients > 2 && oldTeam==PTE_ALIENS && newTeam == PTE_HUMANS && level.numHumanSpawns>0 ) ||
        ( level.numHumanClients - level.numAlienClients > 2 && oldTeam==PTE_HUMANS && newTeam == PTE_ALIENS  && level.numAlienSpawns>0 ) )
   {
     isFixingImbalance=qtrue;
   }

  // under certain circumstances, clients can keep their kills and credits
  // when switching teams
  if( G_admin_permission( ent, ADMF_TEAMCHANGEFREE ) ||
    ( g_teamImbalanceWarnings.integer && isFixingImbalance ) ||
    ( ( oldTeam == PTE_HUMANS || oldTeam == PTE_ALIENS )
    && ( level.time - ent->client->pers.teamChangeTime ) > 60000 ) )
  {
    if( oldTeam == PTE_ALIENS )
      ent->client->pers.credit *= (float)FREEKILL_HUMAN / FREEKILL_ALIEN;
    else if( newTeam == PTE_ALIENS )
      ent->client->pers.credit *= (float)FREEKILL_ALIEN / FREEKILL_HUMAN;
  }
  else
  {
    ent->client->pers.credit = 0;
    ent->client->pers.score = 0;
  }

  if( G_admin_permission( ent, ADMF_DBUILDER ) )
  {
    if( !ent->client->pers.designatedBuilder )
    {
      ent->client->pers.designatedBuilder = qtrue;
      trap_SendServerCommand( ent-g_entities,
        "print \"Your designation has been restored\n\"" );
    }
  }
  else if( ent->client->pers.designatedBuilder )
  {
    ent->client->pers.designatedBuilder = qfalse;
    trap_SendServerCommand( ent-g_entities,
     "print \"You have lost designation due to teamchange\n\"" );
  }

  ent->client->pers.classSelection = PCL_NONE;
  ClientSpawn( ent, NULL, NULL, NULL );

  ent->client->pers.joinedATeam = qtrue;
  ent->client->pers.teamChangeTime = level.time;

  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
  G_CheckDBProtection( );
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent )
{
  pTeam_t team;
  pTeam_t oldteam = ent->client->pers.teamSelection;
  char    s[ MAX_TOKEN_CHARS ];
  qboolean force = G_admin_permission(ent, ADMF_FORCETEAMCHANGE);
  int     aliens = level.numAlienClients;
  int     humans = level.numHumanClients;

  // stop team join spam
  if( level.time - ent->client->pers.teamChangeTime < 1000 )
    return;

  if( oldteam == PTE_ALIENS )
    aliens--;
  else if( oldteam == PTE_HUMANS )
    humans--;

  // do warm up
  if( g_doWarmup.integer &&
      level.time - level.startTime < g_warmup.integer * 1000 )
  {
    trap_SendServerCommand( ent - g_entities, va( "print \"team: you can't join"
      " a team during warm up (%d seconds remaining)\n\"",
      g_warmup.integer - ( level.time - level.startTime ) / 1000 ) );
    return;
  }

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"team: %i\n\"",
      oldteam ) );
    return;
  }

  if( !Q_stricmp( s, "spectate" ) )
    team = PTE_NONE;
  else if( !force && ent->client->pers.teamSelection == PTE_NONE &&
           g_maxGameClients.integer && level.numPlayingClients >=
           g_maxGameClients.integer )
  {
    trap_SendServerCommand( ent - g_entities, va( "print \"The maximum number "
      "of playing clients has been reached (g_maxGameClients = %i)\n\"",
      g_maxGameClients.integer ) );
    return;
  }
  else if( !force && oldteam == PTE_NONE && g_maxGameClients.integer &&
           level.numPlayingClients >= g_maxGameClients.integer )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"The maximum number of "
      "playing clients has been reached (g_maxGameClients = %d)\n\"",
      g_maxGameClients.integer ) );
    return;
  }
  else if ( ent->client->pers.specExpires > level.time )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You can't join a team yet. Expires in %d seconds.\n\"",
      ( ent->client->pers.specExpires - level.time ) / 1000 ) );
    return;
  }
  else if( !Q_stricmp( s, "aliens" ) )
  {
    if( !force && level.alienTeamLocked )
    {
      trap_SendServerCommand( ent-g_entities,
        va( "print \"Alien team has been ^1LOCKED\n\"" ) );
      return;
    }
    else if( level.humanTeamLocked )
    {
      // if only one team has been locked, let people join the other
      // regardless of balance
      force = qtrue;
    }

    if( !force && g_teamForceBalance.integer && aliens > humans && !level.oc )
    {
      G_TriggerMenu( ent - g_entities, MN_A_TEAMFULL );
      return;
    }

    team = PTE_ALIENS;
  }
  else if( !Q_stricmp( s, "humans" ) )
  {
    if( level.humanTeamLocked && !force )
    {
      trap_SendServerCommand( ent-g_entities,
        va( "print \"Human team has been ^1LOCKED\n\"" ) );
      return;
    }
    else if( level.alienTeamLocked )
    {
      // if only one team has been locked, let people join the other
      // regardless of balance
      force = qtrue;
    }

    if( !force && g_teamForceBalance.integer && humans > aliens && !level.oc )
    {
      G_TriggerMenu( ent - g_entities, MN_H_TEAMFULL );
      return;
    }

    team = PTE_HUMANS;
  }
  else if( !Q_stricmp( s, "auto" ) )
  {
    if( level.humanTeamLocked && level.alienTeamLocked )
      team = PTE_NONE;
    else if( humans > aliens )
      team = PTE_ALIENS;
    else if( humans < aliens )
      team = PTE_HUMANS;
    else
      team = PTE_ALIENS + ( rand( ) % 2 );

    if( team == PTE_ALIENS && level.alienTeamLocked )
      team = PTE_HUMANS;
    else if( team == PTE_HUMANS && level.humanTeamLocked )
      team = PTE_ALIENS;
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown team: %s\n\"", s ) );
    return;
  }

  // stop team join spam
  if( oldteam == team )
    return;

  //guard against build timer exploit
  if( oldteam != PTE_NONE && ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
     ( ent->client->ps.stats[ STAT_PCLASS ] == PCL_ALIEN_BUILDER0 ||
       ent->client->ps.stats[ STAT_PCLASS ] == PCL_ALIEN_BUILDER0_UPG ||
       BG_InventoryContainsWeapon( WP_HBUILD, ent->client->ps.stats ) ||
       BG_InventoryContainsWeapon( WP_HBUILD2, ent->client->ps.stats ) ) &&
      ent->client->ps.stats[ STAT_MISC ] > 0 )
  {
    trap_SendServerCommand( ent-g_entities,
        va( "print \"You cannot change teams until build timer expires\n\"" ) );
    return;
  }

  if( level.oc && !G_admin_canEditOC( ent ) )
  {
    if( G_TestLayoutFlag( level.layout, OCFL_ALIENONLY ) && !G_TestLayoutFlag( level.layout, OCFL_BOTHTEAMS ) && team == PTE_HUMANS )
    {
    trap_SendServerCommand( ent-g_entities,
        va( "print \"You cannot join humans with option '%s'\n\"", OCFL_ALIENONLY_NAME ) );
    return;
    }

    if( !G_TestLayoutFlag( level.layout, OCFL_ALIENONLY ) && !G_TestLayoutFlag( level.layout, OCFL_BOTHTEAMS ) && team == PTE_ALIENS )
    {
    trap_SendServerCommand( ent-g_entities,
        va( "print \"You cannot join aliens without option '%s' or '%s'\n\"", OCFL_ALIENONLY_NAME, OCFL_BOTHTEAMS_NAME ) );
    return;
    }

    if( !G_TestLayoutFlag( level.layout, OCFL_AGRANGER ) && !G_TestLayoutFlag( level.layout, OCFL_AGRANGERUPG ) && !G_TestLayoutFlag( level.layout, OCFL_ADRETCH ) && !G_TestLayoutFlag( level.layout, OCFL_ABASILISK ) && !G_TestLayoutFlag( level.layout, OCFL_ABASILISKUPG ) && !G_TestLayoutFlag( level.layout, OCFL_AMARAUDER ) && !G_TestLayoutFlag( level.layout, OCFL_AMARAUDERUPG ) && !G_TestLayoutFlag( level.layout, OCFL_ADRAGOON ) && !G_TestLayoutFlag( level.layout, OCFL_ADRAGOONUPG ) && !G_TestLayoutFlag( level.layout, OCFL_ATYRANT ) && team == PTE_ALIENS )
    {
    trap_SendServerCommand( ent-g_entities,
        va( "print \"There were no alien classes found in the flag list.\nPlease report this problem to an admin\n\"" ) );
    return;
    }
  }

  oldteam = ent->client->pers.teamSelection;
  G_ChangeTeam( ent, team );

  if( team == PTE_ALIENS ) {
    if ( oldteam == PTE_HUMANS )
      trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " abandoned humans and joined the aliens\n\"",
                                      ent->client->pers.netname ) );
    else
      trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " joined the aliens\n\"",
                                      ent->client->pers.netname ) );
  }
  else if( team == PTE_HUMANS ) {
    if ( oldteam == PTE_ALIENS )
      trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " abandoned aliens and joined the humans\n\"",
                                      ent->client->pers.netname ) );
    else
      trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " joined the humans\n\"",
                                      ent->client->pers.netname ) );
  }
  else if( team == PTE_NONE ) {
    if ( oldteam == PTE_HUMANS )
      trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " left the humans\n\"",
                                      ent->client->pers.netname ) );
    else
      trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " left the aliens\n\"",
                                      ent->client->pers.netname ) );
  }
}


/*
==================
G_Say
==================
*/
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message )
{
  qboolean ignore = qfalse, ocTeam = qfalse;

  if( !other )
    return;

  if( !other->inuse )
    return;

  if( !other->client )
    return;

  if( other->client->pers.connected != CON_CONNECTED )
    return;

  if( ent->client->pers.ocTeam && other->client->pers.ocTeam && ent->client->pers.ocTeam == other->client->pers.ocTeam )
    ocTeam = qtrue;  // both clients are on the same scrim team

  if( ( mode == SAY_TEAM || mode == SAY_ACTION_T ) && !OnSameTeam( ent, other ) )
  {
    if( other->client->pers.teamSelection != PTE_NONE && !ocTeam )
      return;

    if( !G_admin_permission( other, ADMF_SPEC_ALLCHAT ) )
      return;

    // specs with ADMF_SPEC_ALLCHAT flag can see team chat
  }

  if( ( mode == SAY_TEAM || mode == SAY_ACTION_T ) && !ocTeam && ent->client->pers.ocTeam )
  {
    // if( !G_admin_permission( other, ADMF_SPEC_ALLCHAT ) )
    return;
  }

  if( mode == SAY_ADMINS && !G_admin_permission( other, ADMF_ADMINCHAT) )
     return;

  if( BG_ClientListTest( &other->client->sess.ignoreList, ent-g_entities ) )
    ignore = qtrue;

  trap_SendServerCommand( other-g_entities, va( "%s \"%s%s%c%c%s\"",
    ( mode == SAY_TEAM || mode == SAY_ACTION_T ) ? "tchat" : "chat",
    ( ignore ) ? "[skipnotify]" : "",
    name, Q_COLOR_ESCAPE, color, message ) );
}

#define EC    "\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText )
{
  int         j;
  gentity_t   *other;
  int         color;
  const char  *prefix;
  char        name[ 64 ];
  // don't let text be too long for malicious reasons
  char        text[ MAX_SAY_TEXT ];
  char        location[ 64 ];

  // Bail if the text is blank.
  if( ! chatText[0] )
     return;

  // Flood limit.  If they're talking too fast, determine that and return.
  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }

  if (g_chatTeamPrefix.integer)
  {
    switch( ent->client->pers.teamSelection)
    {
      default:
      case PTE_NONE:
        prefix = "[^5S^7] ";
        break;

      case PTE_ALIENS:
        prefix = "[^1A^7] ";
        break;

      case PTE_HUMANS:
        prefix = "[^4H^7] ";
    }
  }
  else
    prefix = "";

  switch( mode )
  {
    default:
    case SAY_ALL:
      G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
      Com_sprintf( name, sizeof( name ), "%s%s%c%c"EC": ", prefix,
                   ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_GREEN;
      break;

    case SAY_TEAM:
      G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
      if( Team_GetLocationMsg( ent, location, sizeof( location ) ) )
        Com_sprintf( name, sizeof( name ), EC"(%s%c%c"EC") (%s)"EC": ",
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
      else
        Com_sprintf( name, sizeof( name ), EC"(%s%c%c"EC")"EC": ",
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_CYAN;
      break;

    case SAY_TELL:
      if( target && OnSameTeam( target, ent ) &&
          Team_GetLocationMsg( ent, location, sizeof( location ) ) )
        Com_sprintf( name, sizeof( name ), EC"[%s%c%c"EC"] (%s)"EC": ",
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
      else
        Com_sprintf( name, sizeof( name ), EC"[%s%c%c"EC"]"EC": ",
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_MAGENTA;
      break;

    case SAY_ACTION:
      G_LogPrintf( "action: %s: %s\n", ent->client->pers.netname, chatText );
      Com_sprintf( name, sizeof( name ), "^2%s^7%s%s%c%c"EC" ", g_actionPrefix.string, prefix,
                   ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_WHITE;
      break;

    case SAY_ACTION_T:
      G_LogPrintf( "actionteam: %s: %s\n", ent->client->pers.netname, chatText );
      if( Team_GetLocationMsg( ent, location, sizeof( location ) ) )
        Com_sprintf( name, sizeof( name ), EC"^5%s^7%s%c%c"EC"(%s)"EC" ", g_actionPrefix.string,
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
      else
        Com_sprintf( name, sizeof( name ), EC"^5%s^7%s%c%c"EC""EC" ", g_actionPrefix.string,
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_WHITE;
      break;

      case SAY_ADMINS:
        if( G_admin_permission( ent, ADMF_ADMINCHAT ) ) //Differentiate between inter-admin chatter and user-admin alerts
        {
         G_LogPrintf( "say_admins: [ADMIN]%s: %s\n", ent->client->pers.netname, chatText );
         Com_sprintf( name, sizeof( name ), "%s[ADMIN]%s%c%c"EC": ", prefix,
                    ( ent ) ? ent->client->pers.netname : "console", Q_COLOR_ESCAPE, COLOR_WHITE );
    color = COLOR_MAGENTA;
    }
    else
    {
         G_LogPrintf( "say_admins: [PLAYER]%s: %s\n", ent->client->pers.netname, chatText );
         Com_sprintf( name, sizeof( name ), "%s[PLAYER]%s%c%c"EC": ", prefix,
                    ( ent ) ? ent->client->pers.netname : "console", Q_COLOR_ESCAPE, COLOR_WHITE );
    color = COLOR_MAGENTA;
    }

    break;
  }

  Q_strncpyz( text, chatText, sizeof( text ) );

//  //R1's simple flood protection (lowered in g_client timer actions)
//   if ( g_floodProtection.integer && !G_admin_permission( ent, ADMF_NOCENSORFLOOD ) )
//   {
//     if ( ent->client->pers.floodTimer >= 100 )  return;
//     ent->client->pers.floodTimer += g_floodProtection.integer;
//   }


  if( target )
  {
    G_SayTo( ent, target, mode, color, name, text );
    return;
  }

  // send it to all the apropriate clients
  for( j = 0; j < level.maxclients; j++ )
  {
    other = &g_entities[ j ];
    G_SayTo( ent, other, mode, color, name, text );
  }

  if( g_adminParseSay.integer )
  {
    if( g_adminParseSay.integer )
    {
      G_admin_cmd_check ( ent, qtrue );
    }
  }
}

static void Cmd_SayArea_f( gentity_t *ent )
{
  int    entityList[ MAX_GENTITIES ];
  int    num, i;
  int    color = COLOR_BLUE;
  vec3_t range = { HELMET_RANGE, HELMET_RANGE, HELMET_RANGE };
  vec3_t mins, maxs;
  char   *msg = ConcatArgs( 1 );
  char   name[ 64 ];

  G_LogPrintf( "sayarea: %s: %s\n", ent->client->pers.netname, msg );
  Com_sprintf( name, sizeof( name ), EC"<%s%c%c"EC"> ",
    ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );

  VectorAdd( ent->s.origin, range, maxs );
  VectorSubtract( ent->s.origin, range, mins );

  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
    G_SayTo( ent, &g_entities[ entityList[ i ] ], SAY_TEAM, color, name, msg );
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent )
{
  char    *p;
  char    *args;
  int     offset = 0;
  int     mode = SAY_ALL;

  args = G_SayConcatArgs( 0 );
  if( Q_stricmpn( args, "say_team ", 9 ) == 0 )
    mode = SAY_TEAM;
  if( Q_stricmpn( args, "say_admins ", 11 ) == 0 || Q_stricmpn( args, "a ", 2 ) == 0)
    mode = SAY_ADMINS;

  // support parsing /m out of say text since some people have a hard
  // time figuring out what the console is.
  if( !Q_stricmpn( args, "say /m ", 7 ) ||
      !Q_stricmpn( args, "say_team /m ", 12 ) ||
      !Q_stricmpn( args, "say /mt ", 8 ) ||
      !Q_stricmpn( args, "say_team /mt ", 13 ) )
  {
    G_PrivateMessage( ent );
    return;
  }




   if( !Q_stricmpn( args, "say /a ", 7) ||
       !Q_stricmpn( args, "say_team /a ", 12) ||
       !Q_stricmpn( args, "say /say_admins ", 16) ||
       !Q_stricmpn( args, "say_team /say_admins ", 21) )
   {
       mode = SAY_ADMINS;
       offset =3;
   }

   if( mode == SAY_ADMINS)
   if(!G_admin_permission( ent, ADMF_ADMINCHAT ) )
   {
     if( !g_publicSayadmins.integer )
     {
      ADMP( "Sorry, but public use of say_admins has been disabled.\n" );
      return;
     }
     else
     {
       ADMP( "Your message has been sent to any available admins and to the server logs.\n" );
    }
   }


   if( g_allowShare.integer )
   {
     args = G_SayConcatArgs(0);
     if( !Q_stricmpn( args, "say /share", 10 ) ||
       !Q_stricmpn( args, "say_team /share", 15 ) )
     {
       Cmd_Share_f( ent );
       return;
     }
   }

   if( g_allowDonate.integer )
   {
     args = G_SayConcatArgs(0);
     if( !Q_stricmpn( args, "say /donate", 11 ) ||
       !Q_stricmpn( args, "say_team /donate", 16 ) )
     {
       Cmd_Donate_f( ent );
       return;
     }
   }


  if(!Q_stricmpn( args, "say /me ", 8 ) )
  {
   if( g_allowActions.integer )
   {
    mode = SAY_ACTION;
    offset = 4;
   } else return;
  }
  else if(!Q_stricmpn( args, "say_team /me ", 13 ) )
  {
   if( g_allowActions.integer )
   {
    mode = SAY_ACTION_T;
    offset = 4;
   } else return;
  }
  else if( !Q_stricmpn( args, "me ", 3 ) )
  {
   if( g_allowActions.integer )
   {
    mode = SAY_ACTION;
   } else return;
  }
  else if( !Q_stricmpn( args, "me_team ", 8 ) )
  {
   if( g_allowActions.integer )
   {
    mode = SAY_ACTION_T;
   } else return;
  }


  if( trap_Argc( ) < 2 )
    return;

  p = ConcatArgs( 1 );

  p += offset;

  G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent )
{
  int     targetNum;
  gentity_t *target;
  char    *p;
  char    arg[MAX_TOKEN_CHARS];

  if( trap_Argc( ) < 2 )
    return;

  trap_Argv( 1, arg, sizeof( arg ) );
  targetNum = atoi( arg );

  if( targetNum < 0 || targetNum >= level.maxclients )
    return;

  target = &g_entities[ targetNum ];
  if( !target || !target->inuse || !target->client )
    return;

  p = ConcatArgs( 2 );

  G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
  G_Say( ent, target, SAY_TELL, p );
  // don't tell to the player self if it was already directed to this player
  // also don't send the chat back to a bot
  if( ent != target )
    G_Say( ent, ent, SAY_TELL, p );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent )
{
  trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", vtos( ent->s.origin ) ) );
}

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent )
{
  int   i;
  char  arg1[ MAX_STRING_TOKENS ];
  char  arg2[ MAX_STRING_TOKENS ];
  char  arg3[ MAX_STRING_TOKENS ];
  char  layoutt[ MAX_STRING_TOKENS ];
  int   clientNum = -1;
  int   percentModifier = 0;
  char  name[ MAX_NETNAME ];
  char  message[ MAX_STRING_CHARS ];
  char  *arg2plus;
  char  layoutAddition[ MAX_STRING_CHARS ];
  char  nullstring[] = "";

  arg2plus = G_SayConcatArgs( 2 );

  if( !g_allowVote.integer )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here\n\"" );
    return;
  }

  if( level.oc && g_timelimit.integer && level.time - level.startTime >= g_timelimit.integer * 60000 )
  {
    percentModifier -= g_ocTimeMapDropPercent.value * (level.time - level.startTime / g_timelimit.integer * 60000);
  }

  if( g_voteMinTime.integer
    && ent->client->pers.firstConnect
    && !level.oc
    && level.numPlayingClients > 2
    && level.time - ent->client->pers.enterTime < g_voteMinTime.integer * 1000
    && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
  {
    trap_SendServerCommand( ent-g_entities, va(
      "print \"You must wait %d seconds after connecting before calling a vote\n\"",
      g_voteMinTime.integer ) );
    return;
  }

  if( level.voteTime )
  {
    trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress\n\"" );
    return;
  }

  if( g_voteLimit.integer > 0
    && ent->client->pers.voteCount >= g_voteLimit.integer
    && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
  {
    trap_SendServerCommand( ent-g_entities, va(
      "print \"You have already called the maximum number of votes (%d)\n\"",
      g_voteLimit.integer ) );
    return;
  }

  if( ent->client->pers.muted )
  {
    trap_SendServerCommand( ent - g_entities,
      "print \"You are muted and cannot call votes\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );
  trap_Argv( 3, arg3, sizeof( arg3 ) );

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) || strchr( arg3, ';' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string\n\"" );
    return;
  }

  // if there is still a vote to be executed
  if( level.voteExecuteTime )
  {
    level.voteExecuteTime = 0;
    trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
  }

  level.votePercentToPass=51;

  // detect clientNum for partial name match votes
  if( !Q_stricmp( arg1, "kick" ) ||
    !Q_stricmp( arg1, "spec" ) ||
    !Q_stricmp( arg1, "mute" ) ||
    !Q_stricmp( arg1, "unmute" ) ||
    !Q_stricmp( arg1, "hide" ) ||
    !Q_stricmp( arg1, "unhide" ) )
  {
    int clientNums[ MAX_CLIENTS ] = { -1 };
    int numMatches=0;
    char err[ MAX_STRING_CHARS ];

    if( !arg2[ 0 ] )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: no target\n\"" );
      return;
    }

    if( (numMatches = G_ClientNumbersFromString( arg2plus, clientNums, MAX_CLIENTS )) == 1 )
    {
      // there was only one partial name match
      clientNum = clientNums[ 0 ];
    }
    else
    {
      // look for an exact name match (sets clientNum to -1 if it fails)
      clientNum = G_ClientNumberFromString( ent, arg2plus );
    }

    if( clientNum==-1  && numMatches > 1 )
    {
      G_MatchOnePlayer( clientNums, numMatches, err, sizeof( err ) );
      ADMP( va( "^3callvote: ^7%s\n", err ) );
      return;
    }

    if( clientNum != -1 &&
      level.clients[ clientNum ].pers.connected == CON_DISCONNECTED )
    {
      clientNum = -1;
    }

    if( clientNum != -1 )
    {
      Q_strncpyz( name, level.clients[ clientNum ].pers.netname,
        sizeof( name ) );
      Q_CleanStr( name );
      if ( G_admin_permission ( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
        Com_sprintf( message, sizeof( message ), "%s^7 attempted /callvote %s %s on immune admin %s^7",
          ent->client->pers.netname, arg1, arg2, g_entities[ clientNum ].client->pers.netname );
    }
    else
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: invalid player\n\"" );
      return;
    }
  }

  if( !Q_stricmp( arg1, "kick" ) )
  {
    if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: admin is immune from vote kick\n\"" );
      G_AdminsPrintf("%s\n",message);
      return;
    }

    // use ip in case this player disconnects before the vote ends
    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!ban %s \"%s\" vote kick", level.clients[ clientNum ].pers.ip,
      g_adminTempBan.string );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Kick player \'%s\'", name );
  }
  else if( !Q_stricmp( arg1, "spec" ) )
  {
    if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: admin is immune from vote spec\n\"" );
      return;
    }

    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!putteam %i s %d", clientNum, g_adminTempSpec.integer + 1 );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Spec player \'%s\'", name );
  }
  else if( !Q_stricmp( arg1, "startscrim" ) )
  {
    if(!level.oc)
    {
        trap_SendServerCommand( ent - g_entities, "print \"callvote: can only be called during an obstacle course\n\"" );
        return;
    }

    if( !arg2[ 0 ] || ( arg2[ 0 ] != 'a' && arg2[ 0 ] != 'm' ) )
    {
        trap_SendServerCommand( ent - g_entities, "print \"callvote: usage /callvote startscrim [a/m]\n\"" );
        return;
    }

    if( G_OCScrimTeamEmpty( ) || level.ocScrimState > OC_STATE_NONE )
    {
        trap_SendServerCommand( ent - g_entities, "print \"callvote: cannot start scrim\n\"" );
        return;
    }

    if( !ent->client->pers.ocTeam )
    {
        G_ClientPrint( ent, "Cannot do this when not on a scrim team", 0 );
        return;
    }

    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!startscrim %c", arg2[ 0 ] );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Start \'%s\' scrim", arg2[ 0 ] == 'm' ? "medi" : "armoury" );
    level.votePercentToPass = g_startScrimVotePercent.integer;
  }
  else if( !Q_stricmp( arg1, "endscrim" ) )
  {
    if(!level.oc)
    {
        trap_SendServerCommand( ent - g_entities, "print \"callvote: can only be used during an obstacle course\n\"" );
        return;
    }
    if(level.ocScrimState <= OC_STATE_PREP)
    {
        trap_SendServerCommand( ent - g_entities, "print \"callvote: no scrim is being played\n\"" );
        return;
    }

    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!endscrim" );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "End the scrim" );
    level.votePercentToPass = g_endScrimVotePercent.integer;
  }
  else if( !Q_stricmp( arg1, "mute" ) )
  {
    if( level.clients[ clientNum ].pers.muted )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: player is already muted\n\"" );
      return;
    }

    if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: admin is immune from vote mute\n\"" );
      G_AdminsPrintf("%s\n",message);
      return;
    }
    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!mute %i", clientNum );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Mute player \'%s\'", name );
  }
  else if( !Q_stricmp( arg1, "unmute" ) )
  {
    if( !level.clients[ clientNum ].pers.muted )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: player is not currently muted\n\"" );
      return;
    }
    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!unmute %i", clientNum );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Un-Mute player \'%s\'", name );
  }
  else if( !Q_stricmp( arg1, "hide" ) )
  {
    if( !level.oc )
    {
        trap_SendServerCommand( ent-g_entities, "print \"callvote: can only be used during an obstacle course\"");
        return;
    }
    if( !g_allowHiding.integer )
    {
        trap_SendServerCommand( ent-g_entities, "print \"callvote: server disabled non-admin hiding\n\"" );
        return;
    }
    if( level.clients[ clientNum ].pers.hidden )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: player is already hidden\n\"" );
      return;
    }
    if( level.clients[ clientNum ].pers.hiddenTime )
    {
      trap_SendServerCommand( ent-g_entities, "print \"callvote: player is force hidden\n\"" );
      return;
    }
    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!hide %i %im", clientNum, g_hideTimeCallvoteMinutes.integer );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Hide player \'%s\'", name );
  }
  else if( !Q_stricmp( arg1, "unhide" ) )
  {
    if( !level.oc )
    {
        trap_SendServerCommand( ent-g_entities, "print \"callvote: can only be used during an obstacle course\"");
        return;
    }
    if( !g_allowHiding.integer )
    {
        trap_SendServerCommand( ent-g_entities, "print \"callvote: server disabled non-admin hiding\n\"" );
        return;
    }
    if( !level.clients[ clientNum ].pers.hidden )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: player is not current hidden\n\"" );
      return;
    }
    if( level.clients[ clientNum ].pers.hiddenTime )
    {
      trap_SendServerCommand( ent-g_entities, "print \"callvote: player is force unhidden\n\"" );
      return;
    }
    Com_sprintf( level.voteString, sizeof( level.voteString ),
      "!unhide %i %im", clientNum, g_hideNotTimeCallvoteMinutes.integer );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
      "Un-Hide player \'%s\'", name );
  }
  else if( !Q_stricmp( arg1, "map_restart" ) )
  {
    if( level.ocScrimState > OC_STATE_NONE && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
    {
        trap_SendServerCommand( ent-g_entities, va( "print \"You cannot call for a mapchange during a scrim\n\"" ) );
        return;
    }
      if( g_mapvoteMaxTime.integer
        && level.numPlayingClients > 2
        && !level.oc
        && level.time >= g_mapvoteMaxTime.integer * 1000
        && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
        {
          trap_SendServerCommand( ent-g_entities, va(
          "print \"You cannot call for a restart after %d seconds\n\"",
          g_mapvoteMaxTime.integer ) );
          return;
        }
        if( !g_ocAutoVotes.integer )
        {
          Com_sprintf( level.voteString, sizeof( level.voteString ), "!restart" );

        Com_sprintf( level.voteDisplayString,
            sizeof( level.voteDisplayString ), "Restart current map" );
        }
        else if( level.layout && level.layout[ 0 ] )
        {
          Com_sprintf( level.voteString, sizeof( level.voteString ), va( "!restart %s", level.layout ) );

        Com_sprintf( level.voteDisplayString,
            sizeof( level.voteDisplayString ), va( "Restart current map (layout '%s^7')", level.layout ) );
        level.votePercentToPass = g_ocVotesPercent.integer + percentModifier;
        }
        level.votePercentToPass = g_mapVotesPercent.integer + percentModifier;
  }
  else if( !Q_stricmp( arg1, "map" ) )
  {
    if( g_mapvoteMaxTime.integer
      && !level.oc
      && level.numPlayingClients > 2
      && level.time >= g_mapvoteMaxTime.integer * 1000
      && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
    {
       trap_SendServerCommand( ent-g_entities, va(
         "print \"You cannot call for a mapchange after %d seconds\n\"",
         g_mapvoteMaxTime.integer ) );
       return;
    }

    if( level.ocScrimState > OC_STATE_NONE && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
    {
        trap_SendServerCommand( ent-g_entities, va( "print \"You cannot call for a mapchange during a scrim\n\"" ) );
        return;
    }

    if (arg3[ 0 ])
    {
        G_ToLowerCase(arg3);

        if( !trap_FS_FOpenFile( va( "maps/%s.bsp", arg2 ), NULL, FS_READ ) )
        {
          trap_SendServerCommand( ent - g_entities, va( "print \"callvote: "
            "'maps/%s.bsp' could not be found on the server\n\"", arg2 ) );
          return;
        }
        if( !trap_FS_FOpenFile( va( "layouts/%s/%s.dat", arg2, arg3 ), NULL, FS_READ ) )
        {
          trap_SendServerCommand( ent - g_entities, va( "print \"callvote: "
            "'layouts/%s/%s.dat' could not be found on the server\n\"", arg2, arg3 ) );
          return;
        }
        G_ParseLayoutFlags( arg3, layoutAddition );
        Com_sprintf( level.voteString, sizeof( level.voteString ), "!map %s %s", arg2, arg3 );
        Com_sprintf( level.voteDisplayString,
            sizeof( level.voteDisplayString ), "Change to map '%s' using layout '%s'%s", arg2, arg3, layoutAddition );
        EXCOLOR( level.voteDisplayString );
        level.votePercentToPass = g_ocVotesPercent.integer + percentModifier;
    }

    else
    {
        if( !trap_FS_FOpenFile( va( "maps/%s.bsp", arg2 ), NULL, FS_READ ) )
        {
          trap_SendServerCommand( ent - g_entities, va( "print \"callvote: "
            "'maps/%s.bsp' could not be found on the server\n\"", arg2 ) );
          return;
        }

        Com_sprintf( level.voteString, sizeof( level.voteString ), "!map %s", arg2 );
        Com_sprintf( level.voteDisplayString,
            sizeof( level.voteDisplayString ), "Change to map '%s'", arg2 );
        level.votePercentToPass = g_mapVotesPercent.integer + percentModifier;
    }
        if( g_ocAutoVotes.integer && !(arg3[ 0 ] && arg3[ 0 ] == 'o' && arg3[ 1 ] == 'c') )
        {
        Q_strncpyz( layoutt, (arg3[ 0 ] && arg3[ 0 ] == 'o' && arg3[ 1 ] == 'c') ? (arg3) : ("oc"), MAX_STRING_TOKENS);
        if( !trap_FS_FOpenFile( va( "maps/%s.bsp", arg2 ), NULL, FS_READ ) )
        {
          trap_SendServerCommand( ent - g_entities, va( "print \"callvote: "
            "'maps/%s.bsp' could not be found on the server\n\"", arg2 ) );
          return;
        }
        if( !trap_FS_FOpenFile( va( "layouts/%s/%s.dat", arg2, layoutt ), NULL, FS_READ ) )
        {
          trap_SendServerCommand( ent - g_entities, va( "print \"callvote: "
            "'layouts/%s/%s.dat' could not be found on the server\n\"", arg2, layoutt ) );
          return;
        }
        level.votePercentToPass = g_ocVotesPercent.integer + percentModifier;
          Com_sprintf( level.voteString, sizeof( level.voteString ), va("!map %s %s", arg2, layoutt) );
        Com_sprintf( level.voteDisplayString,
            sizeof( level.voteDisplayString ), "Change to map '%s' using layout '%s'", arg2, layoutt );
        }
  }
  else if( !Q_stricmp( arg1, "draw" ) )
  {

    if( level.ocScrimState > OC_STATE_NONE && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
    {
        trap_SendServerCommand( ent-g_entities, va( "print \"You cannot call for a mapchange during a scrim\"" ) );
        return;
    }
    Com_sprintf( level.voteString, sizeof( level.voteString ), "evacuation" );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
        "End match in a draw" );
    level.votePercentToPass = g_mapVotesPercent.integer + percentModifier;
  }

    else if( !Q_stricmp( arg1, "poll" ) )
     {
     Com_sprintf( level.voteString, sizeof( level.voteString ), nullstring);
     Com_sprintf( level.voteDisplayString,
         sizeof( level.voteDisplayString ), "[Poll] \'^7%s^7\'", arg2plus );
    }
    else if(( !Q_stricmp( arg1, "sudden_death" ) ||
      !Q_stricmp( arg1, "suddendeath" )) && !level.oc )
    {
      if(!g_suddenDeathVotePercent.integer)
      {
        trap_SendServerCommand( ent-g_entities, "print \"Sudden Death votes have been disabled\n\"" );
        return;
      }
      else if( g_suddenDeath.integer )
      {
       trap_SendServerCommand( ent - g_entities, va( "print \"callvote: ""Sudden Death has already begun\n\"") );
       return;
      }
     else
      {
      level.votePercentToPass = g_suddenDeathVotePercent.integer;
      Com_sprintf( level.voteString, sizeof( level.voteString ), "g_suddenDeath 1" );
      Com_sprintf( level.voteDisplayString,
          sizeof( level.voteDisplayString ), "Begin sudden death" );
      }
    }

  else
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Valid vote commands are: "
      "map, map_restart, draw, kick, mute, unmute, poll, sudden_death, spec, hide, and unhide\n" );
    return;
  }


   if (Q_stricmp( arg1, "poll" ))
   {
     Q_strcat( level.voteDisplayString, sizeof( level.voteDisplayString ), va( " (Needs %d percent)", level.votePercentToPass ) );
   }

  trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE
        " called a vote: %s\n\"", ent->client->pers.netname, level.voteDisplayString ) );

    G_LogPrintf("%s^7 called a vote: %s^7\n", ent->client->pers.netname, level.voteDisplayString );

  Q_strcat( level.voteDisplayString, sizeof( level.voteDisplayString ), va( " Called by: %s^7", ent->client->pers.netname ) );

  // start the voting
  level.voteTime = level.time;
  level.voteYes = 0;
  level.voteNo = 0;
  ent->client->pers.voteCount++;
  ent->client->pers.vote = qtrue;

  for( i = 0 ; i < level.maxclients ; i++ )
    level.clients[i].ps.eFlags &= ~EF_VOTED;

  /*ent->client->ps.eFlags |= EF_VOTED;*/

  trap_SetConfigstring( CS_VOTE_TIME, va( "%i", level.voteTime ) );
  trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
  trap_SetConfigstring( CS_VOTE_YES, "0" );
  trap_SetConfigstring( CS_VOTE_NO, "0" );
}


/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent )
{
  char msg[ 64 ];

  if( !level.voteTime )
  {
    if( ent->client->pers.teamSelection != PTE_NONE )
    {
      // If there is a teamvote going on but no global vote, forward this vote on as a teamvote
      // (ugly hack for 1.1 cgames + noobs who can't figure out how to use any command that isn't bound by default)
      int     cs_offset = 0;
      if( ent->client->pers.teamSelection == PTE_ALIENS )
        cs_offset = 1;

      if( level.teamVoteTime[ cs_offset ] )
      {
         if( !(ent->client->ps.eFlags & EF_TEAMVOTED ) )
        {
          Cmd_TeamVote_f(ent);
      return;
        }
      }
    }
    trap_SendServerCommand( ent-g_entities, "print \"No vote in progress\n\"" );
    return;
  }

  if( ent->client->ps.eFlags & EF_VOTED )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Vote already cast\n\"" );
    return;
  }

  if( !ent->client->pers.ocTeam && G_StrFind( level.voteString, "!startscrim" ) )
  {
    G_ClientPrint( ent, "Cannot do this when not on a scrim team", 0 );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Vote cast\n\"" );

  trap_Argv( 1, msg, sizeof( msg ) );
  ent->client->pers.vote = ( tolower( msg[ 0 ] ) == 'y' || msg[ 0 ] == '1' );
  G_Vote( ent, qtrue );

  // a majority will be determined in CheckVote, which will also account
  // for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent )
{
  int   i, team, cs_offset = 0;
  char  arg1[ MAX_STRING_TOKENS ];
  char  arg2[ MAX_STRING_TOKENS ];
  int   clientNum = -1;
  char  name[ MAX_NETNAME ];
  char  message[ MAX_STRING_CHARS ];
  char nullstring[] = "";
  char *arg2plus;
  arg2plus = G_SayConcatArgs( 2 );

  team = ent->client->pers.teamSelection;

  if( team == PTE_ALIENS )
    cs_offset = 1;

  if( !g_allowVote.integer )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here\n\"" );
    return;
  }

  if( level.teamVoteTime[ cs_offset ] )
  {
    trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress\n\"" );
    return;
  }

  if( g_voteLimit.integer > 0
    && ent->client->pers.voteCount >= g_voteLimit.integer
    && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
  {
    trap_SendServerCommand( ent-g_entities, va(
      "print \"You have already called the maximum number of votes (%d)\n\"",
      g_voteLimit.integer ) );
    return;
  }

  if( ent->client->pers.muted )
  {
    trap_SendServerCommand( ent - g_entities,
      "print \"You are muted and cannot call teamvotes\n\"" );
    return;
  }

  if( g_voteMinTime.integer
    && ent->client->pers.firstConnect
    && level.time - ent->client->pers.enterTime < g_voteMinTime.integer * 1000
    && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
  {
    trap_SendServerCommand( ent-g_entities, va(
      "print \"You must wait %d seconds after connecting before calling a vote\n\"",
      g_voteMinTime.integer ) );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid team vote string\n\"" );
    return;
  }

  // detect clientNum for partial name match votes
  if( !Q_stricmp( arg1, "kick" ) ||
    !Q_stricmp( arg1, "denybuild" ) ||
    !Q_stricmp( arg1, "allowbuild" ) ||
    !Q_stricmp( arg1, "designate" ) ||
    !Q_stricmp( arg1, "undesignate" ) )
  {
    int clientNums[ MAX_CLIENTS ] = { -1 };
    int numMatches=0;
    char err[ MAX_STRING_CHARS ];

    if( !arg2[ 0 ] )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: no target\n\"" );
      return;
    }

    if( ( numMatches = G_ClientNumbersFromString( arg2plus, clientNums, MAX_CLIENTS )) == 1 )
    {
      // there was only one partial name match
      clientNum = clientNums[ 0 ];
    }
    else
    {
      // look for an exact name match (sets clientNum to -1 if it fails)
      clientNum = G_ClientNumberFromString( ent, arg2plus );
    }

    if( clientNum==-1  && numMatches > 1 )
    {
      G_MatchOnePlayer( clientNums, numMatches, err, sizeof( err ) );
      ADMP( va( "^3callteamvote: ^7%s\n", err ) );
      return;
    }

    // make sure this player is on the same team
    if( clientNum != -1 && level.clients[ clientNum ].pers.teamSelection !=
      team )
    {
      clientNum = -1;
    }

    if( clientNum != -1 &&
      level.clients[ clientNum ].pers.connected == CON_DISCONNECTED )
    {
      clientNum = -1;
    }

    if( clientNum != -1 )
    {
      Q_strncpyz( name, level.clients[ clientNum ].pers.netname,
        sizeof( name ) );
      Q_CleanStr( name );
      if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
      {
        Com_sprintf( message, sizeof( message ), "%s^7 attempted /callteamvote %s %s on immune admin %s^7",
          ent->client->pers.netname, arg1, arg2, g_entities[ clientNum ].client->pers.netname );
      }
    }
    else
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: invalid player\n\"" );
      return;
    }
  }

  if( !Q_stricmp( arg1, "kick" ) )
  {
    if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: admin is immune from vote kick\n\"" );
      G_AdminsPrintf("%s\n",message);
      return;
    }


    // use ip in case this player disconnects before the vote ends
    Com_sprintf( level.teamVoteString[ cs_offset ],
      sizeof( level.teamVoteString[ cs_offset ] ),
      "!ban %s \"%s\" team vote kick", level.clients[ clientNum ].pers.ip,
      g_adminTempBan.string );
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Kick player '%s'", name );
  }
  else if( !Q_stricmp( arg1, "denybuild" ) )
  {
    if( level.clients[ clientNum ].pers.denyBuild )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: player already lost building rights\n\"" );
      return;
    }

    if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: admin is immune from denybuild\n\"" );
      G_AdminsPrintf("%s\n",message);
      return;
    }

    Com_sprintf( level.teamVoteString[ cs_offset ],
      sizeof( level.teamVoteString[ cs_offset ] ), "!denybuild %i", clientNum );
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Take away building rights from '%s'", name );
  }
  else if( !Q_stricmp( arg1, "allowbuild" ) )
  {
    if( !level.clients[ clientNum ].pers.denyBuild )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: player already has building rights\n\"" );
      return;
    }

    Com_sprintf( level.teamVoteString[ cs_offset ],
      sizeof( level.teamVoteString[ cs_offset ] ), "!allowbuild %i", clientNum );
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Allow '%s' to build", name );
  }
  else if( !Q_stricmp( arg1, "designate" ) )
  {

    if( !g_designateVotes.integer )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: Designate votes have been disabled.\n\"" );
      return;
    }

    if( level.clients[ clientNum ].pers.designatedBuilder )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: player is already a designated builder\n\"" );
      return;
    }
    Com_sprintf( level.teamVoteString[ cs_offset ],
      sizeof( level.teamVoteString[ cs_offset ] ), "!designate %i", clientNum );
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Make '%s' a designated builder", name );
  }
  else if( !Q_stricmp( arg1, "undesignate" ) )
  {

    if( !g_designateVotes.integer )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: Designate votes have been disabled.\n\"" );
      return;
    }

    if( !level.clients[ clientNum ].pers.designatedBuilder )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callvote: player is not currently a designated builder\n\"" );
      return;
    }
    Com_sprintf( level.teamVoteString[ cs_offset ],
      sizeof( level.teamVoteString[ cs_offset ] ), "!undesignate %i", clientNum );
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Remove designated builder status from '%s'", name );
  }
  else if( !Q_stricmp( arg1, "admitdefeat" ) && !level.oc )
  {
    Com_sprintf( level.teamVoteString[ cs_offset ],
      sizeof( level.teamVoteString[ cs_offset ] ), "admitdefeat %i", team );
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Admit Defeat" );
  }
   else if( !Q_stricmp( arg1, "poll" ) )
   {
     Com_sprintf( level.teamVoteString[ cs_offset ], sizeof( level.teamVoteString[ cs_offset ] ), nullstring );
     Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
         sizeof( level.voteDisplayString ), "[Poll] \'%s\'", arg2plus );
   }
  else
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string\n\"" );
    trap_SendServerCommand( ent-g_entities,
       "print \"Valid team vote commands are: "
       "kick, denybuild, allowbuild, poll, designate, undesignate, and admitdefeat\n\"" );
    return;
  }

  G_TeamCommand( team, va( "print \"%s " S_COLOR_WHITE "called a team vote: %s \n\"", ent->client->pers.netname, level.teamVoteDisplayString[ cs_offset ] ) );

   if(team==PTE_ALIENS)
     G_LogPrintf("%s^7 called a teamvote (aliens): %s^7\n", ent->client->pers.netname, level.teamVoteDisplayString[ cs_offset ] );
   else if(team==PTE_HUMANS)
     G_LogPrintf("%s^7 called a teamvote (humans): %s^7\n", ent->client->pers.netname, level.teamVoteDisplayString[ cs_offset ] );

    Q_strcat( level.teamVoteDisplayString[cs_offset], sizeof( level.teamVoteDisplayString ), va( " Called by: %s^7", ent->client->pers.netname ) );

  // start the voting
  level.teamVoteTime[ cs_offset ] = level.time;
  level.teamVoteYes[ cs_offset ] = 0;
  level.teamVoteNo[ cs_offset ] = 0;
  ent->client->pers.voteCount++;
  ent->client->pers.teamVote = qtrue;

  for( i = 0 ; i < level.maxclients ; i++ )
  {
    if( level.clients[ i ].ps.stats[ STAT_PTEAM ] == team )
      level.clients[ i ].ps.eFlags &= ~EF_TEAMVOTED;
  }


  trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset,
    va( "%i", level.teamVoteTime[ cs_offset ] ) );
  trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset,
    level.teamVoteDisplayString[ cs_offset ] );
  trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, "0" );
  trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, "0" );
}


/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent )
{
  int     cs_offset = 0;
  char    msg[ 64 ];

  if( ent->client->pers.teamSelection == PTE_ALIENS )
    cs_offset = 1;

  if( !level.teamVoteTime[ cs_offset ] )
  {
    trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress\n\"" );
    return;
  }

  if( ent->client->ps.eFlags & EF_TEAMVOTED )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Team vote cast\n\"" );

  trap_Argv( 1, msg, sizeof( msg ) );
  ent->client->pers.teamVote = ( tolower( msg[ 0 ] ) == 'y' || msg[ 0 ] == '1' );
  G_TeamVote( ent, qtrue );

  // a majority will be determined in CheckTeamVote, which will also account
  // for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent )
{
  vec3_t  origin, angles;
  char    buffer[ MAX_TOKEN_CHARS ];
  int     i;

  if( trap_Argc( ) != 5 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"usage: setviewpos x y z yaw\n\"" ) );
    return;
  }

  VectorClear( angles );

  for( i = 0 ; i < 3 ; i++ )
  {
    trap_Argv( i + 1, buffer, sizeof( buffer ) );
    origin[ i ] = atof( buffer );
  }

  trap_Argv( 4, buffer, sizeof( buffer ) );
  angles[ YAW ] = atof( buffer );

  TeleportPlayer( ent, origin, angles );
}

#define AS_OVER_RT3         ((ALIENSENSE_RANGE*0.5f)/M_ROOT3)

qboolean G_RoomForClassChange( gentity_t *ent, pClass_t class,
  vec3_t newOrigin )
{
  vec3_t    fromMins, fromMaxs;
  vec3_t    toMins, toMaxs;
  vec3_t    temp;
  trace_t   tr;
  float     nudgeHeight;
  float     maxHorizGrowth;
  pClass_t  oldClass = ent->client->ps.stats[ STAT_PCLASS ];

  BG_FindBBoxForClass( oldClass, fromMins, fromMaxs, NULL, NULL, NULL );
  BG_FindBBoxForClass( class, toMins, toMaxs, NULL, NULL, NULL );

  VectorCopy( ent->s.origin, newOrigin );

  // find max x/y diff
  maxHorizGrowth = toMaxs[ 0 ] - fromMaxs[ 0 ];
  if( toMaxs[ 1 ] - fromMaxs[ 1 ] > maxHorizGrowth )
    maxHorizGrowth = toMaxs[ 1 ] - fromMaxs[ 1 ];
  if( toMins[ 0 ] - fromMins[ 0 ] > -maxHorizGrowth )
    maxHorizGrowth = -( toMins[ 0 ] - fromMins[ 0 ] );
  if( toMins[ 1 ] - fromMins[ 1 ] > -maxHorizGrowth )
    maxHorizGrowth = -( toMins[ 1 ] - fromMins[ 1 ] );

  if( maxHorizGrowth > 0.0f )
  {
    // test by moving the player up the max required on a 60 degree slope
    nudgeHeight = maxHorizGrowth * 2.0f;
  }
  else
  {
    // player is shrinking, so there's no need to nudge them upwards
    nudgeHeight = 0.0f;
  }

  // find what the new origin would be on a level surface
  newOrigin[ 2 ] += fabs( toMins[ 2 ] ) - fabs( fromMins[ 2 ] );

  //compute a place up in the air to start the real trace
  VectorCopy( newOrigin, temp );
  temp[ 2 ] += nudgeHeight;
  trap_Trace( &tr, newOrigin, toMins, toMaxs, temp, ent->s.number, MASK_OCSOLID );

  //trace down to the ground so that we can evolve on slopes
  VectorCopy( newOrigin, temp );
  temp[ 2 ] += ( nudgeHeight * tr.fraction );
  trap_Trace( &tr, temp, toMins, toMaxs, newOrigin, ent->s.number, MASK_OCSOLID );
  VectorCopy( tr.endpos, newOrigin );

  //make REALLY sure
  trap_Trace( &tr, newOrigin, toMins, toMaxs, newOrigin,
    ent->s.number, MASK_OCSOLID );

  //check there is room to evolve
  if( !tr.startsolid && tr.fraction == 1.0f )
    return qtrue;
  else
    return qfalse;
}

/*
=================
Cmd_Class_f
=================
*/
void Cmd_Class_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       clientNum;
  int       i;
  vec3_t    infestOrigin;
  pClass_t  currentClass = ent->client->pers.classSelection;
  pClass_t  newClass;
  int       numLevels;
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { AS_OVER_RT3, AS_OVER_RT3, AS_OVER_RT3 };
  vec3_t    mins, maxs;
  int       num;
//  vec3_t    spawn_origin, spawn_angles;
//  gentity_t *spawn;
  gentity_t *other;


  clientNum = ent->client - level.clients;
  trap_Argv( 1, s, sizeof( s ) );
  newClass = BG_FindClassNumForName( s );

  if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
  {
    if( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
      G_StopFollowing( ent );

    if( ent->client->pers.teamSelection == PTE_ALIENS )
    {
      if( newClass != PCL_ALIEN_BUILDER0 &&
          newClass != PCL_ALIEN_BUILDER0_UPG &&
          newClass != PCL_ALIEN_LEVEL0 &&
         !ent->client->pers.override )
      {
        trap_SendServerCommand( ent-g_entities,
          va( "print \"You cannot spawn with class %s\n\"", s ) );
        return;
      }

      if( !BG_ClassIsAllowed( newClass ) && !ent->client->pers.override )
      {
        trap_SendServerCommand( ent-g_entities,
          va( "print \"Class %s is not allowed\n\"", s ) );
        return;
      }

      if( !BG_FindStagesForClass( newClass, g_alienStage.integer ) && !ent->client->pers.override )
      {
        trap_SendServerCommand( ent-g_entities,
          va( "print \"Class %s not allowed at stage %d\n\"",
              s, g_alienStage.integer ) );
        return;
      }

      if( level.oc && !G_admin_canEditOC( ent ) )
      {
        ent->client->pers.ocNeedSpawn = 1;
        ent->client->pers.ocNeedSpawnTime = level.time + OC_TIMELATENCY_EVOLVEBLOCK;
      }

      // spawn from an egg
//      if( !level.oc && G_PushSpawnQueue( &level.alienSpawnQueue, clientNum ) )
      if( G_PushSpawnQueue( &level.alienSpawnQueue, clientNum ) )
      {
        ent->client->pers.classSelection = newClass;
        ent->client->ps.stats[ STAT_PCLASS ] = newClass;
      }
//      else if( level.oc )
//      {
//        if( ( spawn = G_SelectTremulousSpawnPoint( ent->client->pers.teamSelection,
//            ent->client->pers.lastDeathLocation,
//            spawn_origin, spawn_angles, ent ) ) )
//        {
//          ent->client->sess.sessionTeam = TEAM_FREE;
//          ent->client->pers.lastAliveTime = level.time;
//          ClientUserinfoChanged( clientNum );
//          ClientSpawn( ent, spawn, spawn_origin, spawn_angles );
//          if( level.totalMedistations && ent->client->pers.medis && ent->client->pers.medisLastCheckpoint )
//          {
//            memcpy( ent->client->pers.medis, ent->client->pers.medisLastCheckpoint, level.totalMedistations * sizeof( int ) );
//          }
//          ent->client->pers.lastAliveTime = level.time;
//          ent->client->pers.classSelection = newClass;
//          ent->client->ps.stats[ STAT_PCLASS ] = newClass;
//        }
//      }
    }
    else if( ent->client->pers.teamSelection == PTE_HUMANS )
    {
      //set the item to spawn with
      if( !Q_stricmp( s, BG_FindNameForWeapon( WP_MACHINEGUN ) ) &&
          BG_WeaponIsAllowed( WP_MACHINEGUN ) )
      {
        ent->client->pers.humanItemSelection = WP_MACHINEGUN;
      }
      else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD ) ) &&
               BG_WeaponIsAllowed( WP_HBUILD ) )
      {
        ent->client->pers.humanItemSelection = WP_HBUILD;
      }
      else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD2 ) ) &&
               BG_WeaponIsAllowed( WP_HBUILD2 ) &&
               BG_FindStagesForWeapon( WP_HBUILD2, g_humanStage.integer ) )
      {
        ent->client->pers.humanItemSelection = WP_HBUILD2;
      }
      else
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"Unknown starting item\n\"" );
        return;
      }
      // spawn from a telenode
//      if( !level.oc && G_PushSpawnQueue( &level.humanSpawnQueue, clientNum ) )
      if( G_PushSpawnQueue( &level.humanSpawnQueue, clientNum ) );
      {
        ent->client->pers.classSelection = PCL_HUMAN;
        ent->client->ps.stats[ STAT_PCLASS ] = PCL_HUMAN;
      }
//      else if( level.oc )
//      {
//        if( ( spawn = G_SelectTremulousSpawnPoint( ent->client->pers.teamSelection,
//            ent->client->pers.lastDeathLocation,
//            spawn_origin, spawn_angles, ent ) ) )
//        {
//          ent->client->sess.sessionTeam = TEAM_FREE;
//          ent->client->pers.lastAliveTime = level.time;
//          ClientUserinfoChanged( clientNum );
//          ClientSpawn( ent, spawn, spawn_origin, spawn_angles );
//          if( level.totalMedistations && ent->client->pers.medis && ent->client->pers.medisLastCheckpoint )
//          {
//            memcpy( ent->client->pers.medis, ent->client->pers.medisLastCheckpoint, level.totalMedistations * sizeof( int ) );
//          }
//          ent->client->pers.lastAliveTime = level.time;
//          ent->client->pers.classSelection = PCL_HUMAN;
//          ent->client->ps.stats[ STAT_PCLASS ] = PCL_HUMAN;
//        }
//      }
    }
    return;
  }

  if( ent->health <= 0 )
    return;

  if( ent->client->pers.teamSelection == PTE_ALIENS &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
  {
    if( newClass == PCL_NONE )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Unknown class\n\"" );
      return;
    }

    //if we are not currently spectating, we are attempting evolution
    if( ent->client->pers.classSelection != PCL_NONE )
    {
      if( ( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING ) ||
          ( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING ) )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"You cannot evolve while wallwalking\n\"" );
        return;
      }

      //check there are no humans nearby
      VectorAdd( ent->client->ps.origin, range, maxs );
      VectorSubtract( ent->client->ps.origin, range, mins );

      num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
      for( i = 0; i < num && !level.oc; i++ )
      {
        other = &g_entities[ entityList[ i ] ];

        if( (( other->client && other->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) ||
            ( other->s.eType == ET_BUILDABLE && other->biteam == BIT_HUMANS ))  &&
        !ent->client->pers.override )
        {
          G_TriggerMenu( clientNum, MN_A_TOOCLOSE );
          return;
        }
      }

      if( !level.overmindPresent  &&
        !ent->client->pers.override && !level.oc )
      {
        G_TriggerMenu( clientNum, MN_A_NOOVMND_EVOLVE );
        return;
      }

      //guard against selling the HBUILD weapons exploit
      if( ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
          ( currentClass == PCL_ALIEN_BUILDER0 ||
            currentClass == PCL_ALIEN_BUILDER0_UPG ) &&
          ent->client->ps.stats[ STAT_MISC ] > 0  &&
        !ent->client->pers.override && !level.oc )
      {
        trap_SendServerCommand( ent-g_entities,
            va( "print \"You cannot evolve until build timer expires\n\"" ) );
        return;
      }

      numLevels = BG_ClassCanEvolveFromTo( currentClass,
                                           newClass,
                                           (short)ent->client->ps.persistant[ PERS_CREDIT ], 0 );

      if( G_RoomForClassChange( ent, newClass, infestOrigin ) )
      {
        //...check we can evolve to that class
        if( (numLevels >= 0 &&
            BG_FindStagesForClass( newClass, g_alienStage.integer ) &&
            BG_ClassIsAllowed( newClass ) ) || ent->client->pers.override || level.oc )
        {
          G_LogOnlyPrintf("ClientTeamClass: %i alien %s\n", clientNum, s);


           if( !level.oc && ent->client->pers.denyBuild && ( newClass==PCL_ALIEN_BUILDER0 || newClass==PCL_ALIEN_BUILDER0_UPG ) )
           {
             trap_SendServerCommand( ent-g_entities, "print \"Your building rights have been revoked\n\"" );
             return;
           }

           if( level.oc && !G_admin_canEditOC( ent ) )
           {
              if( newClass == PCL_ALIEN_BUILDER0 && !G_TestLayoutFlag( level.layout, OCFL_AGRANGER ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_AGRANGER_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_BUILDER0_UPG && !G_TestLayoutFlag( level.layout, OCFL_AGRANGERUPG ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_AGRANGERUPG_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL0 && !G_TestLayoutFlag( level.layout, OCFL_ADRETCH ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ADRETCH_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL1 && !G_TestLayoutFlag( level.layout, OCFL_ABASILISK ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ABASILISK_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL1_UPG && !G_TestLayoutFlag( level.layout, OCFL_ABASILISKUPG ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ABASILISKUPG_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL1_UPG && !G_TestLayoutFlag( level.layout, OCFL_ABASILISKUPG ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ABASILISKUPG_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL2 && !G_TestLayoutFlag( level.layout, OCFL_AMARAUDER ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_AMARAUDER_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL2_UPG && !G_TestLayoutFlag( level.layout, OCFL_AMARAUDERUPG ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_AMARAUDERUPG_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL3 && !G_TestLayoutFlag( level.layout, OCFL_ADRAGOON ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ADRAGOON_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL3_UPG && !G_TestLayoutFlag( level.layout, OCFL_ADRAGOONUPG ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ADRAGOONUPG_NAME ) );
                return;
              }
              if( newClass == PCL_ALIEN_LEVEL4 && !G_TestLayoutFlag( level.layout, OCFL_ATYRANT ) )
              {
                trap_SendServerCommand( ent-g_entities, va( "print \"No option: '%s'\n\"", OCFL_ATYRANT_NAME ) );
                return;
              }
              if( newClass == currentClass )
              {
                return;
              }
           }

          ent->client->pers.evolveHealthFraction = (float)ent->client->ps.stats[ STAT_HEALTH ] /
            (float)BG_FindHealthForClass( currentClass );

          if( ent->client->pers.evolveHealthFraction < 0.0f )
            ent->client->pers.evolveHealthFraction = 0.0f;
          else if( ent->client->pers.evolveHealthFraction > 1.0f )
            ent->client->pers.evolveHealthFraction = 1.0f;

          //remove credit
          if(!level.oc)
            G_AddCreditToClient( ent->client, -(short)numLevels, qtrue );
          ent->client->pers.classSelection = newClass;
          ClientUserinfoChanged( clientNum );
          VectorCopy( infestOrigin, ent->s.pos.trBase );
          ClientSpawn( ent, ent, ent->s.pos.trBase, ent->s.apos.trBase );
          return;
        }
        else
        {
          trap_SendServerCommand( ent-g_entities,
               "print \"You cannot evolve from your current class\n\"" );
          return;
        }
      }
      else
      {
        G_TriggerMenu( clientNum, MN_A_NOEROOM );
        return;
      }
    }
    else
    {/*
      //spawning from an egg
      for( i = 0; i < numClasses; i++ )
      {
        if( allowedClasses[ i ] == newClass &&
            BG_FindStagesForClass( newClass, g_alienStage.integer ) &&
            BG_ClassIsAllowed( newClass ) )
        {
          G_LogOnlyPrintf("ClientTeamClass: %i alien %s\n", clientNum, s);


           if( ent->client->pers.denyBuild && ( newClass==PCL_ALIEN_BUILDER0 || newClass==PCL_ALIEN_BUILDER0_UPG ) )
           {
             trap_SendServerCommand( ent-g_entities, "print \"Your building rights have been revoked\n\"" );
             return;
           }

          ent->client->pers.classSelection =
            ent->client->ps.stats[ STAT_PCLASS ] = newClass;
          G_PushSpawnQueue( &level.alienSpawnQueue, clientNum );
          return;
        }
      }
      trap_SendServerCommand( ent-g_entities, va( "print \"You cannot spawn as this class\n\"" ) );
      return;
    */}
  }
  else if( ent->client->pers.teamSelection == PTE_HUMANS )
  {/*
    //humans cannot use this command whilst alive
    if( ent->client->pers.classSelection != PCL_NONE )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You must be dead to use the class command\n\"" ) );
      return;
    }

    ent->client->pers.classSelection =
      ent->client->ps.stats[ STAT_PCLASS ] = PCL_HUMAN;

    //set the item to spawn with
    if( !Q_stricmp( s, BG_FindNameForWeapon( WP_MACHINEGUN ) ) && BG_WeaponIsAllowed( WP_MACHINEGUN ) )
      ent->client->pers.humanItemSelection = WP_MACHINEGUN;
    else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD ) ) && BG_WeaponIsAllowed( WP_HBUILD ) )
      ent->client->pers.humanItemSelection = WP_HBUILD;
    else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD2 ) ) && BG_WeaponIsAllowed( WP_HBUILD2 ) &&
        BG_FindStagesForWeapon( WP_HBUILD2, g_humanStage.integer ) )
      ent->client->pers.humanItemSelection = WP_HBUILD2;
    else
    {
      ent->client->pers.classSelection = PCL_NONE;
      trap_SendServerCommand( ent-g_entities, va( "print \"Unknown starting item\n\"" ) );
      return;
    }

    G_LogOnlyPrintf("ClientTeamClass: %i human %s\n", clientNum, s);

    G_PushSpawnQueue( &level.humanSpawnQueue, clientNum );*/
  }
}

/*
=================
DBCommand

Send command to all designated builders of selected team
=================
*/
void DBCommand( pTeam_t team, const char *text )
{
  int i;
  gentity_t *ent;

  for( i = 0, ent = g_entities + i; i < level.maxclients; i++, ent++ )
  {
    if( !ent->client || ( ent->client->pers.connected != CON_CONNECTED ) ||
        ( ent->client->pers.teamSelection != team ) ||
    !ent->client->pers.designatedBuilder )
      continue;

    trap_SendServerCommand( i, text );
  }
}

/*
=================
Cmd_Destroy_f
=================
*/
void Cmd_Destroy_f( gentity_t *ent )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;
  char        cmd[ 12 ];
  qboolean    deconstruct = qtrue;

  if( ent->client->pers.denyBuild )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Your building rights have been revoked\n\"" );
    return;
  }

  if( level.oc && !G_admin_canEditOC( ent ) )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"You cannot build in an obstacle course\n\"" );
    return;
  }

  trap_Argv( 0, cmd, sizeof( cmd ) );
  if( Q_stricmp( cmd, "destroy" ) == 0 )
    deconstruct = qfalse;

  if( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
  {
    if( ( ent->client->hovel->s.eFlags & EF_DBUILDER ) &&
      !ent->client->pers.designatedBuilder )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"This structure is protected by designated builder\n\"" );
      DBCommand( ent->client->pers.teamSelection,
        va( "print \"%s^3 has attempted to decon a protected structure!\n\"",
    ent->client->pers.netname ) );
      return;
    }
    G_Damage( ent->client->hovel, ent, ent, forward, ent->s.origin,
      10000, 0, MOD_SUICIDE );
  }

  if( !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) )
  {
    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
    VectorMA( ent->client->ps.origin, 100, forward, end );

    trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID );
    traceEnt = &g_entities[ tr.entityNum ];

    if( tr.fraction < 1.0f &&
        ( traceEnt->s.eType == ET_BUILDABLE ) &&
        ( traceEnt->biteam == ent->client->pers.teamSelection || level.oc || g_cheats.integer ) &&
        ( ( ent->client->ps.weapon >= WP_ABUILD ) &&
          ( ent->client->ps.weapon <= WP_HBUILD ) ) )
    {
      // Cancel deconstruction
      if( g_markDeconstruct.integer && traceEnt->deconstruct && !level.oc )
      {
        traceEnt->deconstruct = qfalse;
        return;
      }
      if( ( traceEnt->s.eFlags & EF_DBUILDER ) &&
        !ent->client->pers.designatedBuilder )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"This structure is protected by designated builder\n\"" );
        DBCommand( ent->client->pers.teamSelection,
          va( "print \"%s^3 has attempted to decon a protected structure!\n\"",
      ent->client->pers.netname ) );
        return;
      }


      // Prevent destruction of the last spawn
      if( !g_markDeconstruct.integer && !level.oc )
      {
        if( ent->client->pers.teamSelection == PTE_ALIENS &&
            traceEnt->s.modelindex == BA_A_SPAWN )
        {
          if( level.numAlienSpawns <= 1 )
            return;
        }
        else if( ent->client->pers.teamSelection == PTE_HUMANS &&
                 traceEnt->s.modelindex == BA_H_SPAWN )
        {
          if( level.numHumanSpawns <= 1 )
            return;
        }
      }

      // Don't allow destruction of hovel with granger inside
      if( traceEnt->s.modelindex == BA_A_HOVEL && traceEnt->active )
        return;

      // Don't allow destruction of buildables that cannot be rebuilt
      if( !level.oc && g_suddenDeath.integer && traceEnt->health > 0 &&
          ( ( g_suddenDeathMode.integer == SDMODE_SELECTIVE &&
              !BG_FindReplaceableTestForBuildable( traceEnt->s.modelindex ) ) ||
            ( g_suddenDeathMode.integer == SDMODE_BP &&
              BG_FindBuildPointsForBuildable( traceEnt->s.modelindex ) ) ||
            g_suddenDeathMode.integer == SDMODE_NO_BUILD ) )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"During Sudden Death you can only decon buildings that "
          "can be rebuilt\n\"" );
        return;
      }

      if( ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
        return;
      }

      if( traceEnt->health > 0 || g_deconDead.integer )
      {
        if( g_markDeconstruct.integer && !level.oc )
        {
          traceEnt->deconstruct     = qtrue; // Mark buildable for deconstruction
          traceEnt->deconstructTime = level.time;
        }
        else
        {
          buildHistory_t *new;

          new = G_Alloc( sizeof( buildHistory_t ) );
          new->ID = ( ++level.lastBuildID > 1000 )
              ? ( level.lastBuildID = 1 ) : level.lastBuildID;
          new->ent = ent;
          new->name[ 0 ] = 0;
          new->buildable = traceEnt->s.modelindex;
          VectorCopy( traceEnt->s.pos.trBase, new->origin );
          VectorCopy( traceEnt->s.angles, new->angles );
          VectorCopy( traceEnt->s.origin2, new->origin2 );
          VectorCopy( traceEnt->s.angles2, new->angles2 );
          new->fate = BF_DECONNED;
          new->next = NULL;
          new->marked = NULL;
          G_LogBuild( new );

           if( traceEnt->health > 0 )
           {
            G_TeamCommand( ent->client->pers.teamSelection,
              va( "print \"%s ^3DECONSTRUCTED^7 by %s^7\n\"",
                BG_FindHumanNameForBuildable( traceEnt->s.modelindex ),
                ent->client->pers.netname ) );

            G_LogPrintf( "Decon: %i %i 0: %s deconstructed %s\n",
              ent->client->ps.clientNum,
              traceEnt->s.modelindex,
              ent->client->pers.netname,
              BG_FindNameForBuildable( traceEnt->s.modelindex ) );
          }

          if( level.oc && traceEnt->s.modelindex == BA_H_SPAWN )
            level.numNodes--;

          if( !deconstruct )
            G_Damage( traceEnt, ent, ent, forward, tr.endpos, 10000, 0, MOD_SUICIDE );
          else
            G_FreeEntity( traceEnt );

          if( !g_cheats.integer && !level.oc )
            ent->client->ps.stats[ STAT_MISC ] +=
              BG_FindBuildDelayForWeapon( ent->s.weapon ) >> 2;
        }
      }
    }
  }
}


/*
=================
Cmd_ActivateItem_f

Activate an item
=================
*/
void Cmd_ActivateItem_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  int   upgrade, weapon;

  if( ent->client->pers.teamSelection != PTE_HUMANS && !G_admin_canEditOC( ent ) && !ent->client->pers.override )
  {
    return;
  }

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );
  weapon = BG_FindWeaponNumForName( s );

  if( upgrade != UP_NONE && BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ))
  {
    BG_ActivateUpgrade( upgrade, ent->client->ps.stats );
  }
  else if( weapon != WP_NONE && BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ) )
    G_ForceWeaponChange( ent, weapon );
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"You don't have the %s\n\"", s ) );
}


/*
=================
Cmd_DeActivateItem_f

Deactivate an item
=================
*/
void Cmd_DeActivateItem_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  int   upgrade;

  if( ent->client->pers.teamSelection != PTE_HUMANS && !G_admin_canEditOC( ent ) && !ent->client->pers.override )
  {
    return;
  }

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );

  if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
    BG_DeactivateUpgrade( upgrade, ent->client->ps.stats );
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"You don't have the %s\n\"", s ) );
}


/*
=================
Cmd_ToggleItem_f
=================
*/
void Cmd_ToggleItem_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  int   upgrade, weapon, i;

  if( ent->client->pers.teamSelection != PTE_HUMANS && !G_admin_canEditOC( ent ) && !ent->client->pers.override )
  {
    return;
  }

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );
  weapon = BG_FindWeaponNumForName( s );

  if( weapon != WP_NONE )
  {
    //special case to allow switching between
    //the blaster and the primary weapon

    if( ent->client->ps.weapon != WP_BLASTER )
      weapon = WP_BLASTER;
    else
    {
      //find a held weapon which isn't the blaster
      for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
      {
        if( i == WP_BLASTER )
          continue;

        if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) )
        {
          weapon = i;
          break;
        }
      }

      if( i == WP_NUM_WEAPONS )
        weapon = WP_BLASTER;
    }

    G_ForceWeaponChange( ent, weapon );
  }
  else if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
  {
    if( BG_UpgradeIsActive( upgrade, ent->client->ps.stats ) )
      BG_DeactivateUpgrade( upgrade, ent->client->ps.stats );
    else
      BG_ActivateUpgrade( upgrade, ent->client->ps.stats );
  }
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"You don't have the %s\n\"", s ) );
}

/*
=================
Cmd_Buy_f
=================
*/
void Cmd_Buy_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       i;
  int       weapon, upgrade, numItems = 0;
  int       maxAmmo, maxClips;
  qboolean  buyingEnergyAmmo = qfalse;
  qboolean  hasEnergyWeapon = qfalse;

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS && !level.oc && !ent->client->pers.override )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Must be human to use this command\n\"" );
    return;
  }

  for( i = UP_NONE; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_InventoryContainsUpgrade( i, ent->client->ps.stats ) )
      numItems++;
  }

  for( i = WP_NONE; i < WP_NUM_WEAPONS; i++ )
  {
    if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) )
    {
      if( BG_FindUsesEnergyForWeapon( i ) )
        hasEnergyWeapon = qtrue;
      numItems++;
    }
  }

  trap_Argv( 1, s, sizeof( s ) );

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );

  //special case to keep norf happy
  if( weapon == WP_NONE && upgrade == UP_AMMO )
  {
    buyingEnergyAmmo = hasEnergyWeapon;
  }

  if( buyingEnergyAmmo && !( level.oc && ent->client->pers.ocTeam ) )
  {
    //no armoury nearby
    if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_REACTOR ) &&
        !G_BuildableRange( ent->client->ps.origin, 100, BA_H_REPEATER ) &&
        !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) &&
        !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va(
        "print \"You must be near a reactor, repeater or armoury\n\"" ) );
      return;
    }
  }
  else
  {
    //no armoury nearby
    if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY )  &&
        !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You must be near a powered armoury\n\"" ) );
      return;
    }
  }

  if( weapon != WP_NONE )
  {
    //already got this?
    if( BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ) )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ITEMHELD );
      return;
    }

    //can afford this?
    if( BG_FindPriceForWeapon( weapon ) > (short)ent->client->ps.persistant[ PERS_CREDIT ]  &&
        !ent->client->pers.override )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOFUNDS );
      return;
    }

    //have space to carry this?
    if( BG_FindSlotsForWeapon( weapon ) & ent->client->ps.stats[ STAT_SLOTS ] && !ent->client->pers.override )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOSLOTS );
      return;
    }

    if( BG_FindTeamForWeapon( weapon ) != WUT_HUMANS && !ent->client->pers.override )
    {
      //shouldn't need a fancy dialog
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy alien items\n\"" ) );
      return;
    }

    //are we /allowed/ to buy this?
    if( (!BG_FindStagesForWeapon( weapon, g_humanStage.integer ) || !BG_WeaponIsAllowed( weapon ))  &&
        !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }

    //is the weapon reserved for a scrim team?
    if( G_WeaponIsReserved( weapon && !ent->client->pers.override ) )
    {
        G_ClientPrint( ent, "This item is reserved for the oc scrim", CLIENT_SPECTATORS );
        return;
    }

    //add to inventory
    if( weapon == WP_MACHINEGUN )
    {
      //add to inventory
      BG_AddWeaponToInventory( weapon, ent->client->ps.stats );
      BG_FindAmmoForWeapon( weapon, &maxAmmo, &maxClips );

      if( BG_FindUsesEnergyForWeapon( weapon ) &&
          BG_InventoryContainsUpgrade( UP_BATTPACK, ent->client->ps.stats ) )
        maxAmmo = (int)( (float)maxAmmo * BATTPACK_MODIFIER );

      BG_PackAmmoArray( weapon, ent->client->ps.ammo, ent->client->ps.misc,
                        maxAmmo, maxClips );

      G_ForceWeaponChange( ent, weapon );

      //set build delay/pounce etc to 0
      ent->client->ps.stats[ STAT_MISC ] = 0;

      //subtract from funds
      G_AddCreditToClient( ent->client, -(short)BG_FindPriceForWeapon( weapon ), qfalse );
    }
    else if( !level.oc || G_AllArms(ent->client->pers.arms) || ent->client->pers.override || G_admin_canEditOC( ent ) )
    {
      //add to inventory
      BG_AddWeaponToInventory( weapon, ent->client->ps.stats );
      BG_FindAmmoForWeapon( weapon, &maxAmmo, &maxClips );

      if( BG_FindUsesEnergyForWeapon( weapon ) &&
          BG_InventoryContainsUpgrade( UP_BATTPACK, ent->client->ps.stats ) )
        maxAmmo = (int)( (float)maxAmmo * BATTPACK_MODIFIER );

      BG_PackAmmoArray( weapon, ent->client->ps.ammo, ent->client->ps.misc,
                        maxAmmo, maxClips );

      G_ForceWeaponChange( ent, weapon );

      //set build delay/pounce etc to 0
      ent->client->ps.stats[ STAT_MISC ] = 0;

      //subtract from funds
      G_AddCreditToClient( ent->client, -(short)BG_FindPriceForWeapon( weapon ), qfalse );
    }
    else
    {
      ADMP( "Cannot buy upgrades or weapons until you win\n" );
    }

    if(G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ))
        G_UseArm( ent, G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) );
  }
  else if( upgrade != UP_NONE )
  {
    //already got this?
    if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) && !ent->client->pers.override )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ITEMHELD );
      return;
    }

    //can afford this?
    if( BG_FindPriceForUpgrade( upgrade ) > (short)ent->client->ps.persistant[ PERS_CREDIT ]  &&
        !ent->client->pers.override )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOFUNDS );
      return;
    }

    //have space to carry this?
    if( BG_FindSlotsForUpgrade( upgrade ) & ent->client->ps.stats[ STAT_SLOTS ] && !ent->client->pers.override )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOSLOTS );
      return;
    }

    if( BG_FindTeamForUpgrade( upgrade ) != WUT_HUMANS && !ent->client->pers.override )
    {
      //shouldn't need a fancy dialog
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy alien items\n\"" ) );
      return;
    }

    //are we /allowed/ to buy this?
    if( !BG_FindPurchasableForUpgrade( upgrade ) && !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }

    //are we /allowed/ to buy this?
    if( (!BG_FindStagesForUpgrade( upgrade, g_humanStage.integer ) || !BG_UpgradeIsAllowed( upgrade ) )  &&
        !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }

    if( upgrade == UP_AMMO )
      G_GiveClientMaxAmmo( ent, buyingEnergyAmmo );
    else if( !level.oc || G_AllArms(ent->client->pers.arms) || ent->client->pers.override || G_admin_canEditOC( ent ) )
    {
      //add to inventory
      BG_AddUpgradeToInventory( upgrade, ent->client->ps.stats );

      if( upgrade == UP_BATTPACK )
        G_GiveClientMaxAmmo( ent, qtrue );

      //subtract from funds
      G_AddCreditToClient( ent->client, -(short)BG_FindPriceForUpgrade( upgrade ), qfalse );
    }
    else
    {
      ADMP( "Cannot buy upgrades or weapons until you win\n" );
    }

    if(G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ))
        G_UseArm( ent, G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) );
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown item\n\"" ) );
  }

  if( trap_Argc( ) >= 2 )
  {
    trap_Argv( 2, s, sizeof( s ) );

    //retrigger the armoury menu
    if( !Q_stricmp( s, "retrigger" ) )
      ent->client->retriggerArmouryMenu = level.framenum + RAM_FRAMES;
  }

  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
}


/*
=================
Cmd_Sell_f
=================
*/
void Cmd_Sell_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       i;
  int       weapon, upgrade;

  trap_Argv( 1, s, sizeof( s ) );

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS && !level.oc && !ent->client->pers.override )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Must be human to use this command\n\"" );
    return;
  }

  //no armoury nearby
  if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY )  &&
        !ent->client->pers.override )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You must be near a powered armoury\n\"" ) );
    return;
  }

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );

  if( weapon != WP_NONE )
  {
    //are we /allowed/ to sell this?
    if( !BG_FindPurchasableForWeapon( weapon ) && !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell this weapon\n\"" ) );
      return;
    }

    if( level.ocScrimState && ent->client->pers.ocTeam == 1 && weapon == WP_LAS_GUN && !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell an oc scrim weapon\n\"" ) );
      return;
    }

    if( level.ocScrimState == OC_STATE_PLAY && ent->client->pers.ocTeam == 2 && weapon == WP_MASS_DRIVER && !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell an oc scrim weapon\n\"" ) );
      return;
    }

    //remove weapon if carried
    if( BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ))
    {
      //guard against selling the HBUILD weapons exploit
      if( ( weapon == WP_HBUILD || weapon == WP_HBUILD2 ) &&
          ent->client->ps.stats[ STAT_MISC ] > 0 &&
        !ent->client->pers.override)
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"Cannot sell until build timer expires\n\"" ) );
        return;
      }

      BG_RemoveWeaponFromInventory( weapon, ent->client->ps.stats );

      //add to funds
      G_AddCreditToClient( ent->client, (short)BG_FindPriceForWeapon( weapon ), qfalse );
    }

    //if we have this weapon selected, force a new selection
    if( weapon == ent->client->ps.weapon )
      G_ForceWeaponChange( ent, WP_NONE );
  }
  else if( upgrade != UP_NONE )
  {
    //are we /allowed/ to sell this?
    if( !BG_FindPurchasableForUpgrade( upgrade ) && !ent->client->pers.override )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell this item\n\"" ) );
      return;
    }
    //remove upgrade if carried
    if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
    {
      //add to inventory
      BG_RemoveUpgradeFromInventory( upgrade, ent->client->ps.stats );

      if( upgrade == UP_BATTPACK )
        G_GiveClientMaxAmmo( ent, qtrue );

      //add to funds
      G_AddCreditToClient( ent->client, (short)BG_FindPriceForUpgrade( upgrade ), qfalse );
    }
  }
  else if( !Q_stricmp( s, "weapons" ) )
  {
    for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
    {
      //guard against selling the HBUILD weapons exploit
      if( ( i == WP_HBUILD || i == WP_HBUILD2 ) &&
          ent->client->ps.stats[ STAT_MISC ] > 0  &&
        !ent->client->pers.override )
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"Cannot sell until build timer expires\n\"" ) );
        continue;
      }

      if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) &&
          BG_FindPurchasableForWeapon( i ) )
      {
        BG_RemoveWeaponFromInventory( i, ent->client->ps.stats );

        //add to funds
        G_AddCreditToClient( ent->client, (short)BG_FindPriceForWeapon( i ), qfalse );
      }

      //if we have this weapon selected, force a new selection
      if( i == ent->client->ps.weapon )
        G_ForceWeaponChange( ent, WP_NONE );
    }
  }
  else if( !Q_stricmp( s, "upgrades" ) )
  {
    for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
    {
      //remove upgrade if carried
      if( BG_InventoryContainsUpgrade( i, ent->client->ps.stats ) &&
          BG_FindPurchasableForUpgrade( i ) )
      {
        BG_RemoveUpgradeFromInventory( i, ent->client->ps.stats );

        if( i == UP_BATTPACK )
        {
          int j;

          //remove energy
          for( j = WP_NONE; j < WP_NUM_WEAPONS; j++ )
          {
            if( BG_InventoryContainsWeapon( j, ent->client->ps.stats ) &&
                BG_FindUsesEnergyForWeapon( j ) &&
                !BG_FindInfinteAmmoForWeapon( j ) )
            {
              BG_PackAmmoArray( j, ent->client->ps.ammo, ent->client->ps.misc, 0, 0 );
            }
          }
        }

        //add to funds
        G_AddCreditToClient( ent->client, (short)BG_FindPriceForUpgrade( i ), qfalse );
      }
    }
  }
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown item\n\"" ) );

  if( trap_Argc( ) >= 2 )
  {
    trap_Argv( 2, s, sizeof( s ) );

    //retrigger the armoury menu
    if( !Q_stricmp( s, "retrigger" ) )
      ent->client->retriggerArmouryMenu = level.framenum + RAM_FRAMES;
  }

  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
}


/*
=================
Cmd_Build_f
=================
*/
void Cmd_Build_f( gentity_t *ent )
{
  char          s[ MAX_TOKEN_CHARS ];
  buildable_t   buildable;
  float         dist;
  vec3_t        origin;
  pTeam_t       team;

  if( ent->client->pers.denyBuild || ( level.oc && !G_admin_canEditOC( ent ) ) )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Your building rights have been revoked\n\"" );
    return;
  }

  trap_Argv( 1, s, sizeof( s ) );

  buildable = BG_FindBuildNumForName( s );


  if( g_suddenDeath.integer && !level.oc )
  {
    if( g_suddenDeathMode.integer == SDMODE_SELECTIVE )
    {
      if( !BG_FindReplaceableTestForBuildable( buildable ) )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"This building type cannot be rebuilt during Sudden Death\n\"" );
        return;
      }
      if( G_BuildingExists( buildable ) )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"You can only rebuild one of each type of rebuildable building during Sudden Death.\n\"" );
        return;
      }
    }
    else if( g_suddenDeathMode.integer == SDMODE_NO_BUILD )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"Building is not allowed during Sudden Death\n\"" );
      return;
    }
  }

  team = ent->client->ps.stats[ STAT_PTEAM ];

  if(( buildable != BA_NONE &&
      ( ( 1 << ent->client->ps.weapon ) & BG_FindBuildWeaponForBuildable( buildable ) ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) &&
      BG_BuildableIsAllowed( buildable ) &&
      ( ( team == PTE_ALIENS && BG_FindStagesForBuildable( buildable, g_alienStage.integer ) ) ||
        ( team == PTE_HUMANS && BG_FindStagesForBuildable( buildable, g_humanStage.integer ) ) ) ))
  {
    dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );

    //these are the errors displayed when the builder first selects something to use
    switch( G_CanBuild( ent, buildable, dist, origin ) )
    {
      case IBE_NONE:
      case IBE_TNODEWARN:
      case IBE_RPTWARN:
      case IBE_RPTWARN2:
      case IBE_SPWNWARN:
      case IBE_NOROOM:
      case IBE_NORMAL:
      case IBE_HOVELEXIT:
        ent->client->ps.stats[ STAT_BUILDABLE ] = ( buildable | SB_VALID_TOGGLEBIT );
        break;

      case IBE_NOASSERT:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOASSERT );
        break;

      case IBE_NOOVERMIND:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOOVMND );
        break;

      case IBE_OVERMIND:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_OVERMIND );
        break;

      case IBE_REACTOR:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_REACTOR );
        break;

      case IBE_REPEATER:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_REPEATER );
        break;

      case IBE_NOPOWER:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOPOWER );
        break;

      case IBE_NOCREEP:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOCREEP );
        break;

      case IBE_NODCC:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NODCC );
        break;

      default:
        break;
    }
  }
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"Cannot build this item\n\"" ) );
}

/*
=================
Cmd_Share_f
=================
*/
void Cmd_Share_f( gentity_t *ent )
{
  int   i, clientNum = 0, creds = 0, skipargs = 0;
  int   clientNums[ MAX_CLIENTS ] = { -1 };
  char  cmd[ 12 ];
  char  arg1[ MAX_STRING_TOKENS ];
  char  arg2[ MAX_STRING_TOKENS ];
  pTeam_t team;

  if( !ent || !ent->client || ( ent->client->pers.teamSelection == PTE_NONE ) )
  {
    return;
  }

  if( !g_allowShare.integer )
  {
    return;
  }

  team = ent->client->pers.teamSelection;

  G_SayArgv( 0, cmd, sizeof( cmd ) );
  if( !Q_stricmp( cmd, "say" ) || !Q_stricmp( cmd, "say_team" ) )
  {
    skipargs = 1;
    G_SayArgv( 1, cmd, sizeof( cmd ) );
  }

  // target player name is in arg1
  G_SayArgv( 1+skipargs, arg1, sizeof( arg1 ) );
  // amount to be shared is in arg2
  G_SayArgv( 2+skipargs, arg2, sizeof( arg2 ) );

  if( arg1[0] && !strchr( arg1, ';' ) && Q_stricmp( arg1, "target_in_aim" ) )
  {
    //check arg1 is a number
    for( i = 0; arg1[ i ]; i++ )
    {
      if( arg1[ i ] < '0' || arg1[ i ] > '9' )
      {
        clientNum = -1;
        break;
      }
    }

    if( clientNum >= 0 )
    {
      clientNum = atoi( arg1 );
    }
    else if( G_ClientNumbersFromString( arg1, clientNums, MAX_CLIENTS ) == 1 )
    {
      // there was one partial name match
      clientNum = clientNums[ 0 ];
    }
    else
    {
      // look for an exact name match before bailing out
      clientNum = G_ClientNumberFromString( ent, arg1 );
      if( clientNum == -1 )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"share: invalid player name specified.\n\"" );
        return;
      }
    }
  }
  else // arg1 not set
  {
    vec3_t      forward, end;
    trace_t     tr;
    gentity_t   *traceEnt;


    // trace a teammate
    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
    VectorMA( ent->client->ps.origin, 8192 * 16, forward, end );

    trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_OCSOLID );
    traceEnt = &g_entities[ tr.entityNum ];

    if( tr.fraction < 1.0f && traceEnt->client &&
      ( traceEnt->client->pers.teamSelection == team ) )
    {
      clientNum = traceEnt - g_entities;
    }
    else
    {
      trap_SendServerCommand( ent-g_entities,
        va( "print \"share: aim at a teammate to share %s.\n\"",
        ( team == PTE_HUMANS ) ? "credits" : "evolvepoints" ) );
      return;
    }
  }

  // verify target player team
  if( ( clientNum < 0 ) || ( clientNum >= level.maxclients ) ||
      ( level.clients[ clientNum ].pers.teamSelection != team ) )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"share: not a valid player of your team.\n\"" );
    return;
  }

  if( !arg2[0] || strchr( arg2, ';' ) )
  {
    // default credit count
    if( team == PTE_HUMANS )
    {
      creds = FREEKILL_HUMAN;
    }
    else if( team == PTE_ALIENS )
    {
      creds = FREEKILL_ALIEN;
    }
  }
  else
  {
    //check arg2 is a number
    for( i = 0; arg2[ i ]; i++ )
    {
      if( arg2[ i ] < '0' || arg2[ i ] > '9' )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"usage: share [name|slot#] [amount]\n\"" );
        break;
      }
    }

    // credit count from parameter
    creds = atoi( arg2 );
  }

  // player specified "0" to transfer
  if( creds <= 0 )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Ooh, you are a generous one, indeed!\n\"" );
    return;
  }

  // transfer only credits the player really has
  if( creds > ent->client->ps.persistant[ PERS_CREDIT ] )
  {
    creds = ent->client->ps.persistant[ PERS_CREDIT ];
  }

  // player has no credits
  if( creds <= 0 )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Earn some first, lazy gal!\n\"" );
    return;
  }

  // allow transfers only up to the credit/evo limit
  if( ( team == PTE_HUMANS ) &&
      ( creds > HUMAN_MAX_CREDITS - level.clients[ clientNum ].ps.persistant[ PERS_CREDIT ] ) )
  {
    creds = HUMAN_MAX_CREDITS - level.clients[ clientNum ].ps.persistant[ PERS_CREDIT ];
  }
  else if( ( team == PTE_ALIENS ) &&
      ( creds > ALIEN_MAX_KILLS - level.clients[ clientNum ].ps.persistant[ PERS_CREDIT ] ) )
  {
    creds = ALIEN_MAX_KILLS - level.clients[ clientNum ].ps.persistant[ PERS_CREDIT ];
  }

  // target cannot take any more credits
  if( creds <= 0 )
  {
    trap_SendServerCommand( ent-g_entities,
      va( "print \"share: player cannot receive any more %s.\n\"",
        ( team == PTE_HUMANS ) ? "credits" : "evolvepoints" ) );
    return;
  }

  // transfer credits
  ent->client->ps.persistant[ PERS_CREDIT ] -= creds;
  trap_SendServerCommand( ent-g_entities,
    va( "print \"share: transferred %d %s to %s^7.\n\"", creds,
      ( team == PTE_HUMANS ) ? "credits" : "evolvepoints",
      level.clients[ clientNum ].pers.netname ) );
  level.clients[ clientNum ].ps.persistant[ PERS_CREDIT ] += creds;
  trap_SendServerCommand( clientNum,
    va( "print \"You have received %d %s from %s^7.\n\"", creds,
      ( team == PTE_HUMANS ) ? "credits" : "evolvepoints",
      ent->client->pers.netname ) );

  G_LogPrintf( "Share: %i %i %i %d: %s^7 transferred %d%s to %s^7\n",
    ent->client->ps.clientNum,
    clientNum,
    team,
    creds,
    ent->client->pers.netname,
    creds,
    ( team == PTE_HUMANS ) ? "c" : "e",
    level.clients[ clientNum ].pers.netname );
}


/*
=================
Cmd_Boost_f
=================
*/
void Cmd_Boost_f( gentity_t *ent )
{
  if( BG_InventoryContainsUpgrade( UP_JETPACK, ent->client->ps.stats ) &&
      BG_UpgradeIsActive( UP_JETPACK, ent->client->ps.stats ) )
    return;

  if( ent->client->pers.cmd.buttons & BUTTON_WALKING )
    return;

  if( ( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) &&
      ( ent->client->ps.stats[ STAT_STAMINA ] > 0 ) )
    ent->client->ps.stats[ STAT_STATE ] |= SS_SPEEDBOOST;
}

/*
=================
strlcmp
=================
*/
int strlcmp( const char *a, const char *b, int len )
{
  const char *string1 = a;
  const char *string2 = b;

  while( string1 - a < len && string2 - b < len && *string1 && *string2 && *string1 == *string2 && *string1 != '\n' && *string2 != '\n' )
  {
    string1++;
    string2++;
  }

  return *string1 - *string2;
}

/*
=================
G_SameGuy

Test for another instance (test for guid, ip and admin name)
=================
*/
int G_SameGuy( gentity_t *ent, char *stats )  // there's a segfault somewhere in here
{
    char *statsPtr, *ip;
    char linebuf[ MAX_STRING_CHARS ];
    char toTest[ MAX_STRING_CHARS ];
    char realName[ MAX_NAME_LENGTH ] = {""};
    char pureName[ MAX_NAME_LENGTH ] = {""};
    char cleanName[ MAX_NAME_LENGTH ] = {""};
    char userinfo[ MAX_INFO_STRING ];
    int i = 0, j = 0, l = 0;

    statsPtr = stats;

    trap_GetUserinfo( ent-g_entities, userinfo, sizeof( userinfo ) );

    if( !( ip = Info_ValueForKey( userinfo, "ip" ) ) )
      return 1;  // if for some reason ip cannot be parsed, return same guy

    G_SanitiseName( ent->client->pers.netname, cleanName );
    realName[ 0 ] = '\0';
    pureName[ 0 ] = '\0';
    cleanName[ 0 ] = '\0';
    for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
    {
        if( !Q_stricmp( g_admin_admins[ i ]->guid, ent->client->pers.guid ) )
        {
            l = g_admin_admins[ i ]->level;
            G_SanitiseName( g_admin_admins[ i ]->name, pureName );
            if( Q_stricmp( cleanName, pureName ) )
            {
                Q_strncpyz( realName, g_admin_admins[ i ]->name, sizeof( realName ) );
            }
            break;
        }
    }
    i = 0;
    if( realName[0] && pureName[0] )
        strcpy( realName, pureName );
    else if( cleanName[0] )
        strcpy( realName, cleanName );
    else
        strcpy( realName, "UnnamedPlayer" );  // How can this happen?

    while( *statsPtr )
    {
      if( i >= sizeof( linebuf ) - 1 )
      {
        G_Printf( S_COLOR_RED "ERROR: line overflow" );
        return 0;
      }
      linebuf[ i++ ] = *statsPtr;
      linebuf[ i ]   = '\0';
      if( *statsPtr == '\n' )
      {
        i = 0;
        while( linebuf[ i++ ] > 1 );  // skip the count
        while( linebuf[ i++ ] > 1 );  // skip the time
        while( linebuf[ i++ ] > 1 );  // skip the (aliased) name
        while( linebuf[ i++ ] > 1 );  // skip the date

        while( linebuf[ i ] > 1 )     // parse and test for the guid
        {
            toTest[j++] = linebuf[i++];
            toTest[j]   = '\0';
        }
        if( !Q_stricmp( ent->client->pers.guid, toTest ) )
          return 1;
        toTest[ 0 ] = j = 0;
        while( linebuf[ i++ ] > 1 );  // skip the ip
//        while( linebuf[ i ] > 1 )     // parse and test for the ip
//        {
//            toTest[j++] = linebuf[i++];
//            toTest[j]   = '\0';
//        }
        if( !Q_stricmp( ip, toTest ) )
          return 1;
        toTest[ 0 ] = j = 0;
        while( linebuf[ i ] > 1 )     // parse and test for admin name
        {
            toTest[j++] = linebuf[i++];
            toTest[j]   = '\0';
        }
        if( !Q_stricmp( realName, toTest ) )
          return 1;
        toTest[ 0 ] = i = j = 0;
      }
      statsPtr++;
    }
    return 0;
}

/*
=================
G_CompareStats
=================
*/
int G_CompareStats( const char *a, const char *b )
{
    int countA;
    int countB;
    int scoreA;
    int scoreB;
    sscanf( a, "%d %d", &countA, &scoreA );
    sscanf( b, "%d %d", &countB, &scoreB );
    if( countA != countB )
        return countB - countA;
    else if( scoreA != scoreB )
        return scoreA - scoreB;
    else
        return strcmp( a, b );  // alpha-order: count, time, name, date, guid, ip, adminname
//    return 0;
}

/*
=================
G_MediStats

Stats format 1 - Incompatible with formats < 1! Stat files need manual update
'type: variable': a variable of type type
*seperator*: the seperator char, 0x01

First line is:

'totalArmouries'*seperator*'totalMedistations'

The following lines represent a player record:

'num: count'*seperator*'num: timeInMS'*seperator*'str: date'*seperator*'str: guid'*seperator*'str: ip'*seperator*'str: adminName'

Note: new records must be GREATER than the last record
=================
*/
char *G_MediStats( gentity_t *ent, int count, int time )
{
  int   worstCount = level.totalMedistations;   //  ///  ////*************UGLY+
  int worstTime = 0;
  char map[ MAX_QPATH ];
  char fileName[ MAX_OSPATH ];
  char stat[ MAX_STRING_CHARS ];
  char stats[ MAX_STRING_CHARS * 8];
  fileHandle_t f;
  int len, line = 0, record = 0, i = 0, j = 0, k = 0, ssss;
  char *statsh, *stats2, *statsh2;
  char buf[ OC_STATMAXRECORDS ][ MAX_STRING_CHARS ];
  char data[ OC_STATMAXRECORDS ];
  char name[ MAX_NAME_LENGTH ] = {""};
  char realName[ MAX_NAME_LENGTH ] = {""};
  char pureName[ MAX_NAME_LENGTH ] = {""};
  char cleanName[ MAX_NAME_LENGTH ] = {""};
  char date[ MAX_CVAR_VALUE_STRING ] = {""};
  qtime_t qt;
  int t;
  char *ip, *statsPos;
  char userinfo[ MAX_INFO_STRING ];
  int l;
  int records;

  // stats disabled?
  if( !g_statsEnabled.integer || g_statsRecords.integer <= 0 || g_statsRecords.integer >= OC_STATMAXRECORDS )
    return "";

  // other checks
  if( !level.oc )
    return "";

  if( g_cheats.integer )
  {
    G_ClientPrint( ent, "Cannot store record with cheats enabled", 0 );
    return "";
  }

  // initialize values

  l = 0;
  records = ( ( g_statsRecords.integer > 0 ) ? ( g_statsRecords.integer ) : ( 1 ) );
  G_SanitiseName( ent->client->pers.netname, cleanName );
  realName[ 0 ] = '\0';
  pureName[ 0 ] = '\0';
  cleanName[ 0 ] = '\0';
  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( g_admin_admins[ i ]->guid, ent->client->pers.guid ) )
    {
      l = g_admin_admins[ i ]->level;
      G_SanitiseName( g_admin_admins[ i ]->name, pureName );
      if( Q_stricmp( cleanName, pureName ) )
      {
        Q_strncpyz( realName, g_admin_admins[ i ]->name, sizeof( realName ) );
      }
      break;
    }
  }
  i = 0;
  if( realName[0] && pureName[0] )
    strcpy( realName, pureName );
  else if( cleanName[0] )
    strcpy( realName, cleanName );
  else
    strcpy( realName, "noname");

  trap_GetUserinfo( ent - g_entities, userinfo, sizeof( userinfo ) );
  ip = Info_ValueForKey( userinfo, "ip" );
  if( !ip )
    return " - ip detection error";

  t = trap_RealTime( &qt );
  trap_Cvar_VariableStringBuffer( "gamedate", date, sizeof( date ) );
  strcat( date, va( " %d:%02i", qt.tm_hour, qt.tm_min ) );

  strcpy(name, ent->client->pers.netname);
  G_SanitiseName( ent->client->pers.netname, name );

  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  if( !map[ 0 ] )
  {
    G_Printf( "MediStats( ): no map is loaded\n" );
    return "";
  }
  G_ToLowerCase(level.layout);
  Com_sprintf( fileName, sizeof( fileName ), "stats/%s/%s/med.dat", map, level.layout );

  if( !ip || !Q_stricmp( ip, "noip" ) || !Q_stricmp( ent->client->pers.guid, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"^3Your client is out of date.  You will ^1NOT^3 be able to set records.  Please replace your client executable with the one at ^2http://trem.tjw.org/backport/^3 and reconnect.\n\"" );
    return "";
  }

  // input into stats

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len < 0 )
  {
    trap_FS_FCloseFile( f );
    if( trap_FS_FOpenFile( fileName, &f, FS_APPEND ) < 0 )
    {
      trap_FS_FCloseFile( f );
      G_Printf( "medistats: could not open %s\n", fileName );
      return "";
    }
    else
    {
      trap_FS_FCloseFile( f );
      len = trap_FS_FOpenFile( fileName, &f, FS_READ );
    }
  }

  strcpy( stats, va( "%d %d\n", level.totalArmouries, level.totalMedistations ) );
  statsh    = G_Alloc( len + 1 );
  trap_FS_Read( statsh, len, f );
  *( statsh + len ) = '\0';
  if( len >= MAX_STRING_CHARS * 7 )
  {
    G_Free(statsh);
    return " - overflow caught: file too big";
  }
  statsh2 = statsh;
  while( *statsh2 && *statsh2++ != '\n' );
  strcat( stats, ( len ) ? ( statsh2 ) : ( "" ) );
  G_Free( statsh );
  stats2 = statsPos = stats + strlen( va( "%d %d\n", level.totalArmouries, level.totalMedistations ) );

  trap_FS_FCloseFile( f );

  // update stats in buffer
    // add the new stat
    Com_sprintf( stat, sizeof( stat ), "%d%c%d%c%s%c%s%c%s%c%s%c%s\n", count, '\1', time, '\1', name, '\1', date, '\1', ent->client->pers.guid, '\1', ( ( ip ) ? ( ip ) : ( "noip" ) ), '\1', realName );

    // explode into buf
    while( *stats2 )
    {
        buf[line][i++] = *stats2;
        buf[line][i] = '\0';
        if( *stats2 == '\n' )
        {
            i = 0;
            if( ++line >= records )
            {
                break;
            }
        }
        stats2++;
    }
    stats2 = statsPos;
    qsort( buf, line, MAX_STRING_CHARS, (int(*)())G_CompareStats );
    // if the same guy has another stat..
    if( G_SameGuy( ent, stats2 ) )
    {
        // iterate through each stat, and return if there is a better time
        for( i = 0; i < line; i++ )
        {
            if( G_SameGuy( ent, buf[ i ] ) )
            {
                k = 0;
                data[ 0 ] = j = 0;
                while( buf[ i ][ k ] > 1 )  // parse count
                {
                    data[j++] = buf[i][k++];
                    data[j]   = '\0';
                } k++;
                ssss = atoi( data );
                if( ssss > count )
                    return "";

                data[ 0 ] = j = 0;
                while( buf[ i ][ k ] > 1 )  // parse time
                {
                    data[j++] = buf[i][k++];
                    data[j]   = '\0';
                }  k++;
                if( atoi( data ) < time && ssss == count )
                    return "";
            }
        } i = k = 0;
        // guy broke his own record, so remove all old records
        test:  // JMP for duplicates
        for( i = 0; i < line; i++ )
        {
            if( G_SameGuy( ent, buf[ i ] ) )
            {
//                while( *stats2 && strlcmp( buf[ i ], *stats2++, MAX_STRING_CHARS ) );
//                if( !*stats2-- )
//                    return " - ^1Error removing duplicate stats";  // marker
//                memmove( stats2, stats2 + strlen( buf[ i ] ) + 1, strlen( buf[ i ] ) + 1 );  // this line is no longer needed
                memmove( buf[ i ], buf[ i + 1 ], ( line - i ) * sizeof( buf[ 0 ] ) );
                memset( buf[ line-- ], 0, sizeof( buf[ 0 ] ) );
                goto test;
            }
        }
    }
    stats2 = statsPos;
    // if the guy isn't on the top x, return -- NOTE: removing / beating old records will bypass the list not full check, but if the guy beat a record that's already on the list, the new record is also going to be on the list; changing g_statsRecords will also not affect it, because the old stats have either been truncated before parsing, or more room for records is open anyway; and, if the order is wrong, which should only be possible by manually editing the files, and g_statsRecords is lowered, this will still not be affected just as the previous, but loss of some records data is possible: the records, regardless of order, up to g_statsRecords are going to be parsed and handled. If the order is correct, this would not be a problem, because the worse times will be truncated. Thus manual resorting is hazardous, and not advised.
    if( line >= records )
    {
        for( i = 0; i < line; i++ )
        {
            k = 0;
            data[ 0 ] = j = 0;
            while( buf[ i ][ k ] > 1 )  // parse count
            {
                data[j++] = buf[i][k++];
                data[j]   = '\0';
            } k++;
            if( atoi( data ) <= worstCount )
                worstCount = atoi( data );
        } i = k = 0;

        for( i = 0; i < line; i++ )
        {
            k = 0;
            data[ 0 ] = j = 0;
            while( buf[ i ][ k ] > 1 )  // parse count
            {
                data[j++] = buf[i][k++];
                data[j]   = '\0';
            } k++;
            data[ 0 ] = j = 0;
            if( atoi( data ) <= worstCount )
            {
              while( buf[ i ][ k ] > 1 )  // parse time
              {
                  data[j++] = buf[i][k++];
                  data[j]   = '\0';
              } k++;
              if( atoi( data ) >= worstTime )
                  worstTime = atoi( data );
            }
        } i = k = 0;

        if( count < worstCount )
            return "";
        if( count == worstCount && time > worstTime )
            return "";
    }
    // add the record - if there's no space, remove the last records until there is (since the list was sorted, old scores can be truncated.  See previous comment for data loss details)
    while( line >= records )
    {
        memset( buf[ --line ], 0, sizeof( buf[ 0 ] ) );
    }
    strcpy( buf[ line++ ], stat );
    // truncate -- Not needed
    // sort
    qsort( buf, line, MAX_STRING_CHARS, (int(*)())G_CompareStats );
    // parse for the record # - if not found, return
    record = 0;
    for( i = 0; i < line; i++ )
    {
        record++;
        if( !strlcmp( buf[ i ], stat, sizeof( buf[ 0 ] ) ) )
        {
            break;
        }
    }
    if( record <= 0 )
        return " - ^1Error retrieving record id, not saving stat";
    // implode buf into stats
    stats2 = statsPos;
    memset( stats2, 0, sizeof( stats ) - ( stats2 - stats ) );
    for( i = 0; i < line; i++ )
    {
        strcat( stats2, buf[ i ] );
    }
    //
  // output updated stats
  len = trap_FS_FOpenFile( fileName, &f, FS_WRITE );
  if( len < 0 )
  {
    G_Printf( "medistats: could not open %s\n", fileName );
    return "";
  }
  stats2 = statsPos;

  G_Printf("medistats: saving stats to %s\n", fileName );

  trap_FS_Write( stats, strlen( stats ), f );

  trap_FS_FCloseFile( f );

  if( record )
  {
    char *s = G_Alloc(MAX_STRING_CHARS);
    Com_sprintf(s, MAX_STRING_CHARS, " ^s^f^r^e^e^2New Record!: #%d^7", record);
    return s;
  }
  return "";
}

/*
=================
G_WinStats
=================
*/
char *G_WinStats( gentity_t *ent, int count, int time )
{
  int   worstCount = level.totalArmouries;   //  ///  ////*************UGLY+
  int worstTime = 0;
  char map[ MAX_QPATH ];
  char fileName[ MAX_OSPATH ];
  char stat[ MAX_STRING_CHARS ];
  char stats[ MAX_STRING_CHARS * 8];
  fileHandle_t f;
  int len, line = 0, record = 0, i = 0, j = 0, k = 0, ssss;
  char *statsh, *stats2, *statsh2;
  char buf[ OC_STATMAXRECORDS ][ MAX_STRING_CHARS ];
  char data[ OC_STATMAXRECORDS ];
  char name[ MAX_NAME_LENGTH ] = {""};
  char realName[ MAX_NAME_LENGTH ] = {""};
  char pureName[ MAX_NAME_LENGTH ] = {""};
  char cleanName[ MAX_NAME_LENGTH ] = {""};
  char date[ MAX_CVAR_VALUE_STRING ] = {""};
  qtime_t qt;
  int t;
  char *ip, *statsPos;
  char userinfo[ MAX_INFO_STRING ];
  int l;
  int records;

  // stats disabled?
  if( !g_statsEnabled.integer || g_statsRecords.integer <= 0 || g_statsRecords.integer > OC_STATMAXRECORDS )
    return "";

  // other checks
  if( !level.oc )
    return "";

  if( g_cheats.integer )
  {
    G_ClientPrint( ent, "Cannot store record with cheats enabled", 0 );
    return "";
  }

  // initialize values

  l = 0;
  records = ( ( g_statsRecords.integer > 0 ) ? ( g_statsRecords.integer ) : ( 1 ) );
  G_SanitiseName( ent->client->pers.netname, cleanName );
  realName[ 0 ] = '\0';
  pureName[ 0 ] = '\0';
  cleanName[ 0 ] = '\0';
  for( i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[ i ]; i++ )
  {
    if( !Q_stricmp( g_admin_admins[ i ]->guid, ent->client->pers.guid ) )
    {
      l = g_admin_admins[ i ]->level;
      G_SanitiseName( g_admin_admins[ i ]->name, pureName );
      if( Q_stricmp( cleanName, pureName ) )
      {
        Q_strncpyz( realName, g_admin_admins[ i ]->name, sizeof( realName ) );
      }
      break;
    }
  }
  i = 0;
  if( realName[0] && pureName[0] )
    strcpy( realName, pureName );
  else if( cleanName[0] )
    strcpy( realName, cleanName );
  else
    strcpy( realName, "noname");

  trap_GetUserinfo( ent - g_entities, userinfo, sizeof( userinfo ) );
  ip = Info_ValueForKey( userinfo, "ip" );
  if( !ip )
    return " - ip detection error";

  t = trap_RealTime( &qt );
  trap_Cvar_VariableStringBuffer( "gamedate", date, sizeof( date ) );
  strcat( date, va( " %d:%02i", qt.tm_hour, qt.tm_min ) );

  strcpy(name, ent->client->pers.netname);
  G_SanitiseName( ent->client->pers.netname, name );

  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  if( !map[ 0 ] )
  {
    G_Printf( "WinStats( ): no map is loaded\n" );
    return "";
  }
  G_ToLowerCase(level.layout);
  Com_sprintf( fileName, sizeof( fileName ), "stats/%s/%s/win.dat", map, level.layout );

  if( !ip || !Q_stricmp( ip, "noip" ) || !Q_stricmp( ent->client->pers.guid, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"^3Your client is out of date.  You will ^1NOT^3 be able to set records.  Please replace your client executable with the one at ^2http://trem.tjw.org/backport/^3 and reconnect.\n\"" );
    return "";
  }

  // input into stats

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len < 0 )
  {
    trap_FS_FCloseFile( f );
    if( trap_FS_FOpenFile( fileName, &f, FS_APPEND ) < 0 )
    {
      trap_FS_FCloseFile( f );
      G_Printf( "winstats: could not open %s\n", fileName );
      return "";
    }
    else
    {
      trap_FS_FCloseFile( f );
      len = trap_FS_FOpenFile( fileName, &f, FS_READ );
    }
  }

  strcpy( stats, va( "%d %d\n", level.totalArmouries, level.totalMedistations ) );
  statsh    = G_Alloc( len + 1 );
  trap_FS_Read( statsh, len, f );
  *( statsh + len ) = '\0';
  if( len >= MAX_STRING_CHARS * 7 )
  {
    G_Free(statsh);
    return " - overflow caught: file too big";
  }
  statsh2 = statsh;
  while( *statsh2 && *statsh2++ != '\n' );
  strcat( stats, ( len ) ? ( statsh2 ) : ( "" ) );
  G_Free( statsh );
  stats2 = statsPos = stats + strlen( va( "%d %d\n", level.totalArmouries, level.totalMedistations ) );

  trap_FS_FCloseFile( f );

  // update stats in buffer
    // add the new stat
    Com_sprintf( stat, sizeof( stat ), "%d%c%d%c%s%c%s%c%s%c%s%c%s\n", count, '\1', time, '\1', name, '\1', date, '\1', ent->client->pers.guid, '\1', ( ( ip ) ? ( ip ) : ( "noip" ) ), '\1', realName );

    // explode into buf
    while( *stats2 )
    {
        buf[line][i++] = *stats2;
        buf[line][i] = '\0';
        if( *stats2 == '\n' )
        {
            i = 0;
            if( ++line >= records )
            {
                break;
            }
        }
        stats2++;
    }
    stats2 = statsPos;
    qsort( buf, line, MAX_STRING_CHARS, (int(*)())G_CompareStats );
    // if the same guy has another stat..
    if( G_SameGuy( ent, stats2 ) )
    {
        // iterate through each stat, and return if there is a better time
        for( i = 0; i < line; i++ )
        {
            if( G_SameGuy( ent, buf[ i ] ) )
            {
                k = 0;
                data[ 0 ] = j = 0;
                while( buf[ i ][ k ] > 1 )  // parse count
                {
                    data[j++] = buf[i][k++];
                    data[j]   = '\0';
                } k++;
                ssss = atoi( data );
                if( ssss > count )
                    return "";

                data[ 0 ] = j = 0;
                while( buf[ i ][ k ] > 1 )  // parse time
                {
                    data[j++] = buf[i][k++];
                    data[j]   = '\0';
                }  k++;
                if( atoi( data ) < time && ssss == count )
                    return "";
            }
        } i = k = 0;
        // guy broke his own record, so remove all old records
        test:  // JMP for duplicates
        for( i = 0; i < line; i++ )
        {
            if( G_SameGuy( ent, buf[ i ] ) )
            {
//                while( *stats2 && strlcmp( buf[ i ], *stats2++, MAX_STRING_CHARS ) );
//                if( !*stats2-- )
//                    return " - ^1Error removing duplicate stats";  // marker
//                memmove( stats2, stats2 + strlen( buf[ i ] ) + 1, strlen( buf[ i ] ) + 1 );  // this line is no longer needed
                memmove( buf[ i ], buf[ i + 1 ], ( line - i ) * sizeof( buf[ 0 ] ) );
                memset( buf[ line-- ], 0, sizeof( buf[ 0 ] ) );
                goto test;
            }
        }
    }
    stats2 = statsPos;
    // if the guy isn't on the top x, return -- NOTE: removing / beating old records will bypass the list not full check, but if the guy beat a record that's already on the list, the new record is also going to be on the list; changing g_statsRecords will also not affect it, because the old stats have either been truncated before parsing, or more room for records is open anyway; and, if the order is wrong, which should only be possible by manually editing the files, and g_statsRecords is lowered, this will still not be affected just as the previous, but loss of some records data is possible: the records, regardless of order, up to g_statsRecords are going to be parsed and handled. If the order is correct, this would not be a problem, because the worse times will be truncated. Thus manual resorting is hazardous, and not advised.
    if( line >= records )
    {
        for( i = 0; i < line; i++ )
        {
            k = 0;
            data[ 0 ] = j = 0;
            while( buf[ i ][ k ] > 1 )  // parse count
            {
                data[j++] = buf[i][k++];
                data[j]   = '\0';
            } k++;
            if( atoi( data ) <= worstCount )
                worstCount = atoi( data );
        } i = k = 0;

        for( i = 0; i < line; i++ )
        {
            k = 0;
            data[ 0 ] = j = 0;
            while( buf[ i ][ k ] > 1 )  // parse count
            {
                data[j++] = buf[i][k++];
                data[j]   = '\0';
            } k++;
            data[ 0 ] = j = 0;
            if( atoi( data ) <= worstCount )
            {
              while( buf[ i ][ k ] > 1 )  // parse time
              {
                  data[j++] = buf[i][k++];
                  data[j]   = '\0';
              } k++;
              if( atoi( data ) >= worstTime )
                  worstTime = atoi( data );
            }
        } i = k = 0;

        if( count < worstCount )
            return "";
        if( count == worstCount && time > worstTime )
            return "";
    }
    // add the record - if there's no space, remove the last records until there is (since the list was sorted, old scores can be truncated.  See previous comment for data loss details)
    while( line >= records )
    {
        memset( buf[ --line ], 0, sizeof( buf[ 0 ] ) );
    }
    strcpy( buf[ line++ ], stat );
    // truncate -- Not needed
    // sort
    qsort( buf, line, MAX_STRING_CHARS, (int(*)())G_CompareStats );
    // parse for the record # - if not found, return
    record = 0;
    for( i = 0; i < line; i++ )
    {
        record++;
        if( !strlcmp( buf[ i ], stat, sizeof( buf[ 0 ] ) ) )
        {
            break;
        }
    }
    if( record <= 0 )
        return " - ^1Error retrieving record id, not saving stat";
    // implode buf into stats
    stats2 = statsPos;
    memset( stats2, 0, sizeof( stats ) - ( stats2 - stats ) );
    for( i = 0; i < line; i++ )
    {
        strcat( stats2, buf[ i ] );
    }
    //
  // output updated stats

  len = trap_FS_FOpenFile( fileName, &f, FS_WRITE );
  if( len < 0 )
  {
    G_Printf( "winstats: could not open %s\n", fileName );
    return "";
  }
  stats2 = statsPos;

  G_Printf("winstats: saving stats to %s\n", fileName );

  trap_FS_Write( stats, strlen( stats ), f );

  trap_FS_FCloseFile( f );

  if( record )
  {
    char *s = G_Alloc(MAX_STRING_CHARS);
    Com_sprintf(s, MAX_STRING_CHARS, " ^s^f^r^e^e^2New Record!: #%d^7", record);
    return s;
  }
  return "";
}

/*
=================
Cmd_Mystats_f
=================
*/
void Cmd_Mystats_f( gentity_t *ent )
{
  gentity_t *client;
  char color[ MAX_STRING_CHARS ];
  int percent, i;
  if( level.ocLoadTime || !ent || !ent->client )
    return;
  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }
  G_ClientPrint(ent, "Your Obstacle Course Information--", 0);
  if( ent->client->pers.ocTeam )
  {
    if( level.ocScrimState > OC_STATE_PREP)
    {
        if(level.ocScrimMode == OC_MODE_ARM && level.totalArmouries && level.scrimTeam[ent->client->pers.ocTeam].arms)
        {
            gentity_t **tmp = G_Alloc( level.totalArmouries * sizeof( gentity_t * ) );
            memcpy(tmp, level.scrimTeam[ent->client->pers.ocTeam].arms, level.totalArmouries * sizeof( gentity_t * ));  // memcpy should be faster than merge itself
            for(i = 0; i < MAX_CLIENTS; i++)
            {
                client = g_entities + i;

                if(client->client && client->client->pers.ocTeam == ent->client->pers.ocTeam)
                    G_MergeArms(tmp, client->client->pers.arms);
            }

            percent = (int)( 100 * (G_NumberOfArms(tmp)) / (level.totalArmouries) );
            if(percent < 50)
            {
                Q_strncpyz(color, "^1", sizeof(color));
            }
            else if(percent < 100)
            {
                Q_strncpyz(color, "^3", sizeof(color));
            }
            else
            {
                Q_strncpyz(color, "^2", sizeof(color));
            }

            if(level.totalArmouries == 1 || G_TestLayoutFlag(level.layout, OCFL_ONEARM))
            {
                if(G_NumberOfArms(tmp))
                {
                    G_ClientPrint(ent, va("Armouries: ^2Win^7 - %dm%ds%dms)", MINS(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer)), SECS(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer)), MSEC(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer))), 0);
                }
                else
                {
                    G_ClientPrint(ent, "Armouries: ^1None", 0);
                }
            }
            else if(G_AllArms(ent->client->pers.arms))
            {
                G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent) - ^2%dm%ds%dms", G_NumberOfArms(tmp), level.totalArmouries, color, percent, MINS(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer)), SECS(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer)), MSEC(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer))), 0);
            }
            else
            {
                G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent)", G_NumberOfArms(tmp), level.totalArmouries, color, percent), 0);
            }

            G_Free(tmp);
        }
        else if(level.ocScrimMode == OC_MODE_MEDI && level.totalMedistations && level.scrimTeam[ent->client->pers.ocTeam].medis)
        {
            gentity_t **tmp = G_Alloc( level.totalMedistations * sizeof( gentity_t * ) );
            memcpy(tmp, level.scrimTeam[ent->client->pers.ocTeam].medis, level.totalMedistations * sizeof( gentity_t * ));  // memcpy should be faster than merge itself
            for(i = 0; i < MAX_CLIENTS; i++)
            {
                client = g_entities + i;

                if(client->client && client->client->pers.ocTeam == ent->client->pers.ocTeam)
                    G_MergeMedis(tmp, client->client->pers.medis);
            }

            percent = (int)( 100 * (G_NumberOfMedis(tmp)) / (level.totalMedistations) );
            if(percent < 50)
            {
                Q_strncpyz(color, "^1", sizeof(color));
            }
            else if(percent < 100)
            {
                Q_strncpyz(color, "^3", sizeof(color));
            }
            else
            {
                Q_strncpyz(color, "^2", sizeof(color));
            }

            G_ClientPrint(ent, va("Medical Stations: %d/%d (%s%d^7 percent)%s", G_NumberOfMedis(tmp), level.totalMedistations, color, percent, G_AllMedis(tmp) ? va(" - ^2%dm%ds%dms", MINS(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer)), SECS(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer)), MSEC(level.scrimTeam[ent->client->pers.ocTeam].time - (level.ocStartTime + g_ocWarmup.integer))) : ("")), 0);

            G_Free(tmp);
        }
    }
    return;
  }
  else
  {
    if(level.totalArmouries && ent->client->pers.arms)
    {
        percent = (int)( 100 * (G_NumberOfArms(ent->client->pers.arms)) / (level.totalArmouries) );
        if(percent < 50)
        {
            Q_strncpyz(color, "^1", sizeof(color));
        }
        else if(percent < 100)
        {
            Q_strncpyz(color, "^3", sizeof(color));
        }
        else
        {
            Q_strncpyz(color, "^2", sizeof(color));
        }

        if(level.totalArmouries == 1 || G_TestLayoutFlag(level.layout, OCFL_ONEARM))
        {
            if(G_NumberOfArms(ent->client->pers.arms))
            {
                G_ClientPrint(ent, va("Armouries: ^2Win^7 - %dm%ds%dms)", MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime)), 0);
            }
            else
            {
                G_ClientPrint(ent, "Armouries: ^1None", 0);
            }
        }
        else if(G_AllArms(ent->client->pers.arms))
        {
            G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent) - ^2%dm%ds%dms", G_NumberOfArms(ent->client->pers.arms), level.totalArmouries, color, percent, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime)), 0);
        }
        else
        {
            G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent)", G_NumberOfArms(ent->client->pers.arms), level.totalArmouries, color, percent), 0);
        }
    }
    if(level.totalMedistations && ent->client->pers.medis)
    {
        percent = (int)( 100 * (G_NumberOfMedis(ent->client->pers.medis)) / (level.totalMedistations) );
        if(percent < 50)
        {
            Q_strncpyz(color, "^1", sizeof(color));
        }
        else if(percent < 100)
        {
            Q_strncpyz(color, "^3", sizeof(color));
        }
        else
        {
            Q_strncpyz(color, "^2", sizeof(color));
        }

        G_ClientPrint(ent, va("Medical Stations: %d/%d (%s%d^7 percent)%s", G_NumberOfMedis(ent->client->pers.medis), level.totalMedistations, color, percent, G_AllMedis(ent->client->pers.medis) ? va(" - ^2%dm%ds%dms", MINS(ent->client->pers.mediTime), SECS(ent->client->pers.mediTime), MSEC(ent->client->pers.mediTime)) : ("")), 0);
    }
  }
}

/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent )
{
  fileHandle_t f;
  int arms = 0;
  int medis = 0;
  int  len, i=0, j=0;
  char arg1[ MAX_STRING_TOKENS ];
  char arg2[ MAX_STRING_TOKENS ];
  char *statsWin, *statsMedi, *statsWinPtr, *statsMediPtr;
  char *linePtr;
//  char *linePtr2;
  char map[ MAX_QPATH ];
  char layout[ MAX_QPATH ];
  char fileName[ MAX_OSPATH ];
  char line[ MAX_STRING_CHARS ];
  char name[ MAX_STRING_CHARS ];
  char dateTime[ MAX_STRING_CHARS ];
  char data[ MAX_STRING_CHARS ];
  int  score, record, count;

  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );

  if( level.ocLoadTime || !ent || !ent->client )
    return;
  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) || strchr( arg1, '/' ) || strchr( arg2, '/' ) || strchr( arg1, '\\' ) || strchr( arg2, '\\' ) || strchr( arg1, '\"' ) || strchr( arg2, '\"' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid stat string\n\"" );
    return;
  }

  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }

  if( trap_Argc( ) > 4 || trap_Argc( ) < 1 )
  {
    ADMP( "stats: Usage:\n/stats: see current oc stats\n/stats mapname: see mapname with layout oc's stats\n/stats mapname layoutname: see mapname with layoutname's stats\n/stats mapname layoutname more: a third argument will give more details of records" );
    return;
  }

  if( trap_Argc( ) < 2 )
    trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  else if( trap_Argc( ) <= 4 )
    Q_strncpyz( map, arg1, sizeof( map ) );
  else
    return;
  if( trap_Argc( ) == 3 || trap_Argc( ) == 4 )
    Q_strncpyz( layout, arg2, sizeof( layout ) );
  else if( trap_Argc( ) == 2 )
    strcpy( layout, "oc" );
  else if( trap_Argc( ) < 2 )
    Q_strncpyz( layout, level.layout, sizeof( layout ) );
  else
    return;
  Com_sprintf( fileName, sizeof( fileName ), "stats/%s/%s/win.dat", map, layout );

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len < 0 )
  {
    trap_FS_FCloseFile( f );
    if( level.totalArmouries <= 0 && trap_Argc( ) <= 2 )
    {
        if( trap_FS_FOpenFile( fileName, &f, FS_APPEND ) < 0 )
        {
            trap_FS_FCloseFile( f );
            ADMP( "stat: No records (needs at least 1 medi record)\n" );
            return;
        }
        else
        {
          trap_FS_Write( va( "%d %d\n", level.totalArmouries, level.totalMedistations ), strlen( va( "%d %d\n", level.totalArmouries, level.totalMedistations ) ), f );
          trap_FS_FCloseFile( f );
          len = trap_FS_FOpenFile( fileName, &f, FS_READ );
        }
    }
    else
    {
      ADMP( "stat: No records (needs at least 1 medi and arm record)\n" );
      return;
    }
  }
  statsWin = G_Alloc( len + 1 );
  statsWinPtr = statsWin;
  trap_FS_Read( statsWin, len, f );
  *( statsWin + len ) = '\0';
  trap_FS_FCloseFile( f );

  Com_sprintf( fileName, sizeof( fileName ), "stats/%s/%s/med.dat", map, layout );

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len < 0 )
  {
    trap_FS_FCloseFile( f );
    if( level.totalMedistations <= 0 && !level.ocLoadTime && trap_Argc( ) <= 2 )
    {
        if( trap_FS_FOpenFile( fileName, &f, FS_APPEND ) < 0 )
        {
            trap_FS_FCloseFile( f );
            ADMP( "stat: No records (needs at least 1 arm record)\n" );
            return;
        }
        else
        {
          trap_FS_Write( va( "%d %d\n", level.totalArmouries, level.totalMedistations ), strlen( va( "%d %d\n", level.totalArmouries, level.totalMedistations ) ), f );
          trap_FS_FCloseFile( f );
          len = trap_FS_FOpenFile( fileName, &f, FS_READ );
        }
    }
    else
    {
      ADMP( "stat: No records (needs at least 1 medi and arm record)\n" );
      return;
    }
  }
  statsMedi = G_Alloc( len + 1 );
  statsMediPtr = statsMedi;
  trap_FS_Read( statsMedi, len, f );
  *( statsMedi + len ) = '\0';
  trap_FS_FCloseFile( f );

  data[ 0 ] = i = j = 0;
  while( *statsWinPtr > 1 && *statsWinPtr != ' ' && *statsWinPtr != '\n' && i < MAX_STRING_CHARS )  // parse total arms
  {
    data[ i++ ] = *statsWinPtr++;
    data[ i ] = 0;
  } statsWinPtr++; i = j = 0;
  while( *statsWinPtr > 1 && *statsWinPtr != ' ' && *statsWinPtr != '\n' && i < MAX_STRING_CHARS ) statsWinPtr++;  // skip total medis
  ;;statsWinPtr++; i = j = 0;
  arms = atoi( data );

  data[ 0 ] = i = j = 0;
  while( *statsMediPtr > 1 && *statsMediPtr != ' ' && *statsMediPtr != '\n' && i < MAX_STRING_CHARS ) statsMediPtr++;  // skip total arms
  ;;statsMediPtr++; i = j = 0;
  while( *statsMediPtr > 1 && *statsMediPtr != ' ' && *statsMediPtr != '\n' && i < MAX_STRING_CHARS )  // parse total medis
  {
    data[ i++ ] = *statsMediPtr++;
    data[ i ] = 0;
  } statsMediPtr++; i = j = 0;

  medis = atoi( data );

  if( medis > 0 )
  {
    if( trap_Argc( ) < 4 )
      trap_SendServerCommand( ent - g_entities, va( "print \"%s %s Most Medical Stations Used\n\n---\nName - Count - Time\n\n\"", map, layout ) );
    else
      trap_SendServerCommand( ent - g_entities, va( "print \"%s %s Most Medical Stations Used\n\n---\nName - Count - Time - Date\n\n\"", map, layout ) );
    i = 0;
    record = 0;
    while( *statsMediPtr )
    {
      if( i >= sizeof( line ) - 1 )
      {
        G_Free( statsWin );
        G_Free( statsMedi );
        G_Printf( S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
         va( "stats/%s/%s/med.dat", map, layout ), line );
        return;
      }
      line[ i++ ] = *statsMediPtr;
      line[ i ] = '\0';
      if( *statsMediPtr == '\n' )
      {
        i = j = 0;
//        sscanf( line, "%d %d", &count, &score );
        linePtr = line;
        count = atoi(linePtr++);
        while( *linePtr && *linePtr > 1) linePtr++;
        if(*linePtr) linePtr++;
        score = atoi(linePtr);
        linePtr = line;
//        linePtr2 = line;  // originally used to put a terminator after name
        for( i = 0; i < 2; i++ ) // twice to skip first two numbers
        {
          while( *linePtr && *linePtr >  1 )
          {
            linePtr++;
          }
          while( *linePtr && *linePtr <= 1 )
          {
            linePtr++;
          }
//          while( *linePtr2 && *linePtr2 != '\n' )
//          {
//            linePtr2++;
//          }
//          while( *linePtr2 && *linePtr2 == '\n' )
//          {
//            *(linePtr2++) = '\0';
//          }
        } i = 0;
        name[0] = '\0';
        dateTime[0] = '\0';
        while( i + 1 < MAX_STRING_CHARS && linePtr[i] > 1 )
        {
          name[i] = linePtr[i];
          i++;
          name[i] = '\0';
        }
        if( linePtr[i++] )
        while( j + 1 < MAX_STRING_CHARS && i + 1 < MAX_STRING_CHARS && linePtr[i] > 1 )
        {
          dateTime[j++] = linePtr[i++];
          dateTime[j] = '\0';
        } i = j = 0;
        record++;
        if( trap_Argc( ) < 4 )
//, score / 60000, ( score - ( ( score / 60000 ) * 60000 ) ) / 1000, score - ( ( score / 1000 ) * 1000 ) )
          trap_SendServerCommand( ent - g_entities, va( "print \"^7#^7%d^7: ^7%s^7 - %d/%d ^7%dm:%ds:%dms^7\n\"", record, name, count, medis, MINS( score ), SECS( score ), MSEC( score ) ) );
        else
          trap_SendServerCommand( ent - g_entities, va( "print \"^7#^7%d^7: ^7%s^7 - %d/%d ^7%dm:%ds:%dms^7 - %s^7\n\"", record, name, count, medis, MINS( score ), SECS( score ), MSEC( score ), dateTime ) );
      }
      statsMediPtr++;
    }
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"No Medical Stations\n\"" ) );
  }

  if( arms > 0 )
  {
    if( trap_Argc( ) < 4 )
      trap_SendServerCommand( ent - g_entities, va( "print \"\n\n\n%s %s Best Winning Times\n\n---\nName - Time\n\n\"", map, layout ) );
    else
      trap_SendServerCommand( ent - g_entities, va( "print \"\n\n\n%s %s Best Winning Times\n\n---\nName - Time - Date\n\n\"", map, layout ) );

    i = 0;
    record = 0;

    while( *statsWinPtr )
    {
      if( i >= sizeof( line ) - 1 )
      {
        G_Free( statsWin );
        G_Free( statsMedi );
        G_Printf( S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
         va( "stats/%s/%s/win.dat", map, layout ), line );
        return;
      }
      line[ i++ ] = *statsWinPtr;
      line[ i ] = '\0';
      if( *statsWinPtr == '\n' )
      {
        i = j = 0;
//        sscanf( line, "%d %d", &count, &score );
        linePtr = line;
        count = atoi(linePtr++);
        while( *linePtr && *linePtr > 1) linePtr++;
        if(*linePtr) linePtr++;
        score = atoi(linePtr);
        linePtr = line;
//        linePtr2 = line;  // originally used to put a terminator after name
        for( i = 0; i < 2; i++ ) // twice to skip first two numbers
        {
          while( *linePtr && *linePtr >  1 )
          {
            linePtr++;
          }
          while( *linePtr && *linePtr <= 1 )
          {
            linePtr++;
          }
//          while( *linePtr2 && *linePtr2 != '\n' )
//          {
//            linePtr2++;
//          }
//          while( *linePtr2 && *linePtr2 == '\n' )
//          {
//            *(linePtr2++) = '\0';
//          }
        } i = 0;
        name[0] = '\0';
        dateTime[0] = '\0';
        while( i + 1 < MAX_STRING_CHARS && linePtr[i] > 1 )
        {
          name[i] = linePtr[i];
          i++;
          name[i] = '\0';
        }
        if( linePtr[i++] )
        while( j + 1 < MAX_STRING_CHARS && i + 1 < MAX_STRING_CHARS && linePtr[i] > 1 )
        {
          dateTime[j++] = linePtr[i++];
          dateTime[j] = '\0';
        } i = j = 0;
        record++;
        if( trap_Argc( ) < 4 )
          trap_SendServerCommand( ent - g_entities, va( "print \"^7#^7%d^7: ^7%s^7 - ^7%dm:%ds:%dms^7\n\"", record, name, MINS( score ), SECS( score ), MSEC( score ) ) );
        else
          trap_SendServerCommand( ent - g_entities, va( "print \"^7#^7%d^7: ^7%s^7 - ^7%dm:%ds:%dms^7 - %s^7\n\"", record, name, MINS( score ), SECS( score ), MSEC( score ), dateTime ) );
      }
      statsWinPtr++;
    }
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"No Armouries\n\"" ) );
  }

  G_Free( statsMedi );
  G_Free( statsWin );
}

/*
=================
Cmd_Spawnup_f
=================
*/
void Cmd_Spawnup_f( gentity_t *ent )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) && level.oc && G_admin_canEditOC( ent ) )
  {
    if( ++traceEnt->spawnGroup >= MAX_SPAWNGROUP )
      traceEnt->spawnGroup = 0;
    trap_SendServerCommand( ent-g_entities, va("print \"Structure ordered to spawn %d%s\n\"", traceEnt->spawnGroup, SUFN(traceEnt->spawnGroup) ) );
  }
}

/*
=================
Cmd_Spawndown_f
=================
*/
void Cmd_Spawndown_f( gentity_t *ent )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) && level.oc && G_admin_canEditOC( ent ) )
  {
    if( --traceEnt->spawnGroup <= 0 )
      traceEnt->spawnGroup = 0;
    trap_SendServerCommand( ent-g_entities, va("print \"Structure ordered to spawn %d%s\n\"", traceEnt->spawnGroup, SUFN(traceEnt->spawnGroup) ) );
  }
}

/*
=================
Cmd_Spawn_f
=================
*/
void Cmd_Spawn_f( gentity_t *ent )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) && level.oc && G_admin_canEditOC( ent ) )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"Structure ordered to spawn %d%s\n\"", traceEnt->spawnGroup, SUFN(traceEnt->spawnGroup) ) );
  }
}

/*
=================
Cmd_Groupup_f
=================
*/
void Cmd_Groupup_f( gentity_t *ent )
{
  int         count = 0, i;
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt, *countEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) && level.oc && G_admin_canEditOC( ent ) )
  {
    traceEnt->groupID++;
    if( traceEnt->s.modelindex == BA_H_SPAWN && traceEnt->groupID >= level.numNodes )
      traceEnt->groupID = 0;
    for( i = 1, countEnt = g_entities + i; i < level.num_entities; countEnt++, i++ )
    {
      if( countEnt->s.modelindex == BA_H_SPAWN && countEnt->groupID == traceEnt->groupID && countEnt->powered && !( countEnt->health <= 0 ) && countEnt->spawned )
      {
          count++;
      }
    }
    if( traceEnt->s.modelindex == BA_H_SPAWN || traceEnt->s.modelindex == BA_A_SPAWN )
    trap_SendServerCommand( ent-g_entities, va("print \"Structure grouped as %d (%d total telenodes in group)%s\n\"", traceEnt->groupID, count, ( ( traceEnt->groupID ) ? ( "" ) : ( " (spawning)" ) ) ) );
    else
    trap_SendServerCommand( ent-g_entities, va("print \"Structure grouped as %d%s\n\"", traceEnt->groupID, traceEnt->groupID == 2 ? " (unpowered)" : (traceEnt->groupID == 1 ? " (powered)" : (traceEnt->groupID == 0 ? " (default behaviour)" : " (undefined behaviour)")) ) );
  }
}

/*
=================
Cmd_Groupdown_f
=================
*/
void Cmd_Groupdown_f( gentity_t *ent )
{
  int         count = 0, i;
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt, *countEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) && level.oc && G_admin_canEditOC( ent ) )
  {
    traceEnt->groupID--;
    if( traceEnt->groupID < 0 )
      traceEnt->groupID = ( ( traceEnt->s.modelindex == BA_H_SPAWN ) ? ( level.numNodes - 1 ) : ( 0 ) );
    for( i = 1, countEnt = g_entities + i; i < level.num_entities; countEnt++, i++ )
    {
      if( countEnt->s.modelindex == BA_H_SPAWN && countEnt->groupID == traceEnt->groupID && countEnt->powered && !( countEnt->health <= 0 ) && countEnt->spawned )
      {
        count++;
      }
    }
    if( traceEnt->s.modelindex == BA_H_SPAWN || traceEnt->s.modelindex == BA_A_SPAWN )
    trap_SendServerCommand( ent-g_entities, va("print \"Structure grouped as %d (%d total telenodes in group)%s\n\"", traceEnt->groupID, count, ( ( traceEnt->groupID ) ? ( "" ) : ( " (spawning)" ) ) ) );
    else
    trap_SendServerCommand( ent-g_entities, va("print \"Structure grouped as %d%s\n\"", traceEnt->groupID, traceEnt->groupID == 2 ? " (unpowered)" : (traceEnt->groupID == 1 ? " (powered)" : (traceEnt->groupID == 0 ? " (default behaviour)" : " (undefined behaviour)")) ) );
  }
}

/*
=================
Cmd_Group_f
=================
*/
void Cmd_Group_f( gentity_t *ent )
{
  int         count = 0, i;
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt, *countEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) && level.oc && G_admin_canEditOC( ent ) )
  {
      for( i = 1, countEnt = g_entities + i; i < level.num_entities; countEnt++, i++ )
      {
        if( countEnt->s.modelindex == BA_H_SPAWN && countEnt->groupID == traceEnt->groupID && countEnt->powered && !( countEnt->health <= 0 ) && countEnt->spawned )
        {
            count++;
        }
      }
    if( traceEnt->s.modelindex == BA_H_SPAWN || traceEnt->s.modelindex == BA_A_SPAWN )
    trap_SendServerCommand( ent-g_entities, va("print \"Structure grouped as %d (%d total telenodes in group)%s\n\"", traceEnt->groupID, count, ( ( traceEnt->groupID ) ? ( "" ) : ( " (spawning)" ) ) ) );
    else
    trap_SendServerCommand( ent-g_entities, va("print \"Structure grouped as %d%s\n\"", traceEnt->groupID, traceEnt->groupID == 2 ? " (unpowered)" : (traceEnt->groupID == 1 ? " (powered)" : (traceEnt->groupID == 0 ? " (default behaviour)" : " (undefined behaviour)")) ) );
  }
}

/*
=================
Cmd_Cheat_f
=================
*/
void Cmd_Cheat_f( gentity_t *ent )
{
  G_LogPrintf(va("Possible aimbotter: %s\n",ent->client->pers.netname));
  G_AdminsPrintf(va("Possible aimbotter: %s\n",ent->client->pers.netname));
}

/*
=================
Cmd_Protect_f
=================
*/
void Cmd_Protect_f( gentity_t *ent )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;

  AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
  VectorMA( ent->client->ps.origin, 100, forward, end );

  trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number,
    MASK_PLAYERSOLID );
  traceEnt = &g_entities[ tr.entityNum ];

  if( !ent->client->pers.designatedBuilder )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Only designated"
        " builders can toggle structure protection.\n\"" );
    return;
  }

  if( tr.fraction < 1.0f && ( traceEnt->s.eType == ET_BUILDABLE ) &&
      ( traceEnt->biteam == ent->client->pers.teamSelection ) )
  {
    if( traceEnt->s.eFlags & EF_DBUILDER )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"Structure protection removed\n\"" );
      traceEnt->s.eFlags &= ~EF_DBUILDER;
    }
    else
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"Structure protection applied\n\"" );
      traceEnt->s.eFlags |= EF_DBUILDER;
    }
  }
}

 /*
 =================
 Cmd_Resign_f
 =================
 */
 void Cmd_Resign_f( gentity_t *ent )
 {
   if( !ent->client->pers.designatedBuilder )
   {
     trap_SendServerCommand( ent-g_entities,
       "print \"You are not a designated builder\n\"" );
     return;
   }

   ent->client->pers.designatedBuilder = qfalse;
   trap_SendServerCommand( -1, va(
     "print \"%s" S_COLOR_WHITE " has resigned\n\"",
     ent->client->pers.netname ) );
   G_CheckDBProtection( );
 }



/*
=================
Cmd_Reload_f
=================
*/
void Cmd_Reload_f( gentity_t *ent )
{
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS && !level.oc && !ent->client->pers.override )
  {
    trap_SendServerCommand( ent-g_entities,
      "print \"Must be human to use this command\n\"" );
    return;
  }

  if( ( ent->client->ps.weapon >= WP_ABUILD ) &&
    ( ent->client->ps.weapon <= WP_HBUILD ) )
  {
    Cmd_Protect_f( ent );
  }
  else if( ent->client->ps.weaponstate != WEAPON_RELOADING )
    ent->client->ps.pm_flags |= PMF_WEAPON_RELOAD;
}





/*
=================
G_StopFromFollowing

stops any other clients from following this one
called when a player leaves a team or dies
=================
*/
void G_StopFromFollowing( gentity_t *ent )
{
  int i;

  for( i = 0; i < level.maxclients; i++ )
  {
    if( level.clients[ i ].sess.spectatorState == SPECTATOR_FOLLOW &&
        level.clients[ i ].sess.spectatorClient == ent-g_entities && !G_admin_permission( &g_entities[ i ], ADMF_SPEC_ALLCHAT ) )
    {
      if( !G_FollowNewClient( &g_entities[ i ], 1 ) && !(level.oc && G_admin_permission( &g_entities[ i ], ADMF_SPEC_ALLCHAT ) ))
        G_StopFollowing( &g_entities[ i ] );
    }
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
  ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
  ent->client->sess.sessionTeam = TEAM_SPECTATOR;
  ent->client->ps.stats[ STAT_PTEAM ] = ent->client->pers.teamSelection;

  if( ent->client->pers.teamSelection == PTE_NONE )
  {
    ent->client->sess.spectatorState = SPECTATOR_FREE;
  }
  else
  {
    vec3_t   spawn_origin, spawn_angles;

    ent->client->sess.spectatorState = SPECTATOR_LOCKED;
    if( ent->client->pers.teamSelection == PTE_ALIENS )
      G_SelectAlienLockSpawnPoint( spawn_origin, spawn_angles );
    else if( ent->client->pers.teamSelection == PTE_HUMANS )
      G_SelectHumanLockSpawnPoint( spawn_origin, spawn_angles );
    G_SetOrigin( ent, spawn_origin );
    VectorCopy( spawn_origin, ent->client->ps.origin );
    G_SetClientViewAngle( ent, spawn_angles );
  }
  ent->client->sess.spectatorClient = -1;
  ent->client->ps.pm_flags &= ~PMF_FOLLOW;

  ent->client->ps.stats[ STAT_STATE ] &= ~SS_WALLCLIMBING;
  ent->client->ps.stats[ STAT_STATE ] &= ~SS_WALLCLIMBINGCEILING;
  ent->client->ps.eFlags &= ~EF_WALLCLIMB;
  ent->client->ps.viewangles[ PITCH ] = 0.0f;

  ent->client->ps.clientNum = ent - g_entities;

  CalculateRanks( );
}

/*
=================
G_FollowNewClient

This was a really nice, elegant function. Then I fucked it up.
=================
*/
qboolean G_FollowNewClient( gentity_t *ent, int dir )
{
  int       clientnum = ent->client->sess.spectatorClient;
  int       original = clientnum;
  qboolean  selectAny = qfalse;

  if( dir > 1 )
    dir = 1;
  else if( dir < -1 )
    dir = -1;
  else if( dir == 0 )
    return qtrue;

  if( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
    return qfalse;

  // select any if no target exists
  if( clientnum < 0 || clientnum >= level.maxclients )
  {
    clientnum = original = 0;
    selectAny = qtrue;
  }

  do
  {
    clientnum += dir;

    if( clientnum >= level.maxclients )
      clientnum = 0;

    if( clientnum < 0 )
      clientnum = level.maxclients - 1;

    // can't follow a hidden client
    if( level.clients[ clientnum ].pers.hidden && !G_admin_permission( ent, ADMF_SPEC_ALLCHAT ) )
        continue;

    // avoid selecting existing follow target
    if( clientnum == original && !selectAny )
      continue; //effectively break;

    // can't follow self
    if( &level.clients[ clientnum ] == ent->client )
      continue;

    // can only follow connected clients
    if( level.clients[ clientnum ].pers.connected != CON_CONNECTED )
      continue;

    // can't follow another spectator
    if( level.clients[ clientnum ].pers.teamSelection == PTE_NONE )
        continue;

      // can only follow teammates when dead and on a team
     if( ent->client->pers.teamSelection != PTE_NONE &&
         ( level.clients[ clientnum ].pers.teamSelection !=
           ent->client->pers.teamSelection ) )
       continue;

     // cannot follow a teammate who is following you
     if( level.clients[ clientnum ].sess.spectatorState == SPECTATOR_FOLLOW &&
         ( level.clients[ clientnum ].sess.spectatorClient == ent->s.number ) )
       continue;

    // this is good, we can use it
    ent->client->sess.spectatorClient = clientnum;
    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    return qtrue;

  } while( clientnum != original );

  return qfalse;
}

/*
=================
G_ToggleFollow
=================
*/
void G_ToggleFollow( gentity_t *ent )
{
  if( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
    G_StopFollowing( ent );
  else
    G_FollowNewClient( ent, 1 );
}

 /*
 =================
Cmd_Donate_f

Alms for the poor
=================
*/
void Cmd_Donate_f( gentity_t *ent ) {
  char s[ 20 ] = "", *type = "evo(s)";
  int i, value, divisor, portion, new_credits, total=0,
    max = ALIEN_MAX_KILLS, *amounts, skipargs=0;
  qboolean donated = qtrue;

  if( !ent->client || !g_allowDonate.integer ) return;

  if( ent->client->pers.teamSelection == PTE_ALIENS )
    divisor = level.numAlienClients-1;
  else if( ent->client->pers.teamSelection == PTE_HUMANS ) {
    divisor = level.numHumanClients-1;
    max = HUMAN_MAX_CREDITS;
    type = "credit(s)";
  } else {
    trap_SendServerCommand( ent-g_entities,
      va( "print \"donate: spectators cannot be so gracious\n\"" ) );
    return;
  }

  if( divisor < 1 ) {
    trap_SendServerCommand( ent-g_entities,
      "print \"donate: get yourself some teammates first\n\"" );
    return;
  }

  G_SayArgv ( 0, s, sizeof( s ) );
  if ( !Q_stricmp( s, "say" ) || !Q_stricmp( s, "say_team" ) )
  {
     skipargs = 1;
     G_SayArgv ( 1, s, sizeof( s )); //usefull ?
  }

  G_SayArgv( 1 + skipargs, s, sizeof( s ) );
  value = atoi(s);
  if( value <= 0 ) {
    trap_SendServerCommand( ent-g_entities,
      "print \"donate: very funny\n\"" );
    return;
  }
  if( value > ent->client->ps.persistant[ PERS_CREDIT ] )
    value = ent->client->ps.persistant[ PERS_CREDIT ];

  // allocate memory for distribution amounts
  amounts = G_Alloc( level.maxclients * sizeof( int ) );
  for( i = 0; i < level.maxclients; i++ ) amounts[ i ] = 0;

  // determine donation amounts for each client
  total = value;
  while( donated && value ) {
    donated = qfalse;
    portion = value / divisor;
    if( portion < 1 ) portion = 1;
    for( i = 0; i < level.maxclients; i++ )
      if( level.clients[ i ].pers.connected == CON_CONNECTED &&
           ent->client != level.clients + i &&
           level.clients[ i ].pers.teamSelection ==
           ent->client->pers.teamSelection ) {
        new_credits = level.clients[ i ].ps.persistant[ PERS_CREDIT ] + portion;
        amounts[ i ] = portion;
        if( new_credits > max ) {
          amounts[ i ] -= new_credits - max;
          new_credits = max;
        }
        if( amounts[ i ] ) {
          level.clients[ i ].ps.persistant[ PERS_CREDIT ] = new_credits;
          donated = qtrue;
          value -= amounts[ i ];
          if( value < portion ) break;
        }
      }
  }

  // transfer funds
  G_AddCreditToClient( ent->client, value - total, qtrue );
  for( i = 0; i < level.maxclients; i++ )
    if( amounts[ i ] ) {
      trap_SendServerCommand( i,
        va( "print \"%s^7 donated %d %s to you, don't forget to say 'thank you'!\n\"",
        ent->client->pers.netname, amounts[ i ], type ) );
    }

  G_Free( amounts );

  trap_SendServerCommand( ent-g_entities,
    va( "print \"Donated %d %s to the cause.\n\"",
    total-value, type ) );
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent )
{
  int   i;
  int   pids[ MAX_CLIENTS ];
  char  arg[ MAX_TOKEN_CHARS ];

  if( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
  {
    trap_SendServerCommand( ent - g_entities, "print \"follow: You cannot follow unless you are dead or on the spectators.\n\"" );
    return;
  }

  if( trap_Argc( ) != 2 )
  {
    G_ToggleFollow( ent );
  }
  else
  {
    trap_Argv( 1, arg, sizeof( arg ) );
    if( G_ClientNumbersFromString( arg, pids, MAX_CLIENTS ) == 1 )
    {
      i = pids[ 0 ];
    }
    else
    {
      i = G_ClientNumberFromString( ent, arg );

      if( i == -1 )
      {
        trap_SendServerCommand( ent - g_entities,
          "print \"follow: invalid player\n\"" );
        return;
      }
    }

    // can't follow a hidden client
    if( level.clients[ i ].pers.hidden && !G_admin_permission( ent, ADMF_SPEC_ALLCHAT ) )
    {
        trap_SendServerCommand( ent - g_entities, "print \"follow: You cannot follow a hidden client.\n\"" );
        return;
    }

    // can't follow self
    if( &level.clients[ i ] == ent->client )
    {
      trap_SendServerCommand( ent - g_entities, "print \"follow: You cannot follow yourself.\n\"" );
      return;
    }

    // can't follow another spectator
    if( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR && !G_admin_permission( ent, ADMF_SPEC_ALLCHAT ) )
    {
      trap_SendServerCommand( ent - g_entities, "print \"follow: You cannot follow another spectator.\n\"" );
      return;
    }

    // can only follow teammates when dead and on a team
    if( ent->client->pers.teamSelection != PTE_NONE &&
        ( level.clients[ i ].pers.teamSelection !=
          ent->client->pers.teamSelection ) )
    {
      trap_SendServerCommand( ent - g_entities, "print \"follow: You can only follow teammates, and only when you are dead.\n\"" );
      return;
    }

    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    ent->client->sess.spectatorClient = i;
  }
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent )
{
  char args[ 11 ];
  int  dir = 1;

  trap_Argv( 0, args, sizeof( args ) );
  if( Q_stricmp( args, "followprev" ) == 0 )
    dir = -1;

  // won't work unless spectating
   if( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
     return;
   if( ent->client->sess.spectatorState == SPECTATOR_NOT )
     return;
  G_FollowNewClient( ent, dir );
}

/*
=================
Cmd_PTRCVerify_f

Check a PTR code is valid
=================
*/
void Cmd_PTRCVerify_f( gentity_t *ent )
{
  connectionRecord_t  *connection;
  char                s[ MAX_TOKEN_CHARS ] = { 0 };
  int                 code;

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
    return;

  code = atoi( s );

  if( G_VerifyPTRC( code ) )
  {
    connection = G_FindConnectionForCode( code );

    // valid code
    if( connection->clientTeam != PTE_NONE )
      trap_SendServerCommand( ent->client->ps.clientNum, "ptrcconfirm" );

    // restore mapping
    ent->client->pers.connection = connection;
  }
  else
  {
    // invalid code -- generate a new one
    connection = G_GenerateNewConnection( ent->client );

    if( connection )
    {
      trap_SendServerCommand( ent->client->ps.clientNum,
        va( "ptrcissue %d", connection->ptrCode ) );
    }
  }
}

/*
=================
Cmd_PTRCRestore_f

Restore against a PTR code
=================
*/
void Cmd_PTRCRestore_f( gentity_t *ent )
{
  char                s[ MAX_TOKEN_CHARS ] = { 0 };
  int                 code;
  connectionRecord_t  *connection;

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
    return;

  code = atoi( s );

  if( G_VerifyPTRC( code ) )
  {
    if( ent->client->pers.joinedATeam )
    {
      trap_SendServerCommand( ent - g_entities,
        "print \"You cannot use a PTR code after joining a team\n\"" );
    }
    else
    {
      // valid code
      connection = G_FindConnectionForCode( code );

      if( connection )
      {
        // set the correct team
        G_ChangeTeam( ent, connection->clientTeam );

        // set the correct credit
        ent->client->ps.persistant[ PERS_CREDIT ] = 0;
        G_AddCreditToClient( ent->client, connection->clientCredit, qtrue );

        // set oc data
        if(level.oc && !connection->hasCheated && !ent->client->pers.hasCheated)
        {
            ent->client->pers.lastOCCheckpoint = connection->lastOCCheckpoint;
            if( level.totalMedistations && connection->totalMedistations && ent->client->pers.medisLastCheckpoint && ent->client->pers.medis )
            {
                gentity_t **tmp;
                if(level.totalMedistations > connection->totalMedistations)
                {
                    tmp = G_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
                    memcpy(tmp, connection->medisLastCheckpoint, connection->totalMedistations + 1);
                    G_SyncMedis(tmp, level.totalMedistations);
                }
                else
                {
                    tmp = G_Alloc((connection->totalMedistations + 1) * sizeof(gentity_t *));
                    memcpy(tmp, connection->medisLastCheckpoint, level.totalMedistations + 1);
                    G_SyncMedis(tmp, connection->totalMedistations);
                }
                if(tmp[level.totalMedistations])
                {
                    G_ClientPrint(ent, "^1Error restoring ptrc", CLIENT_SPECTATORS);
                    return;
                }
                memcpy(ent->client->pers.medisLastCheckpoint, tmp, (level.totalMedistations + 1) * sizeof(gentity_t *));
                G_Free(tmp);
                memcpy(ent->client->pers.medis, ent->client->pers.medisLastCheckpoint, (level.totalMedistations + 1) * sizeof(gentity_t *));
            }
            if( level.totalArmouries && connection->totalArmouries && ent->client->pers.armsLastCheckpoint && ent->client->pers.arms )
            {
                gentity_t **tmp;
                if(level.totalArmouries > connection->totalArmouries)
                {
                    tmp = G_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
                    memcpy(tmp, connection->armsLastCheckpoint, connection->totalArmouries + 1);
                    G_SyncArms(tmp, level.totalArmouries);
                }
                else
                {
                    tmp = G_Alloc((connection->totalArmouries + 1) * sizeof(gentity_t *));
                    memcpy(tmp, connection->armsLastCheckpoint, level.totalArmouries + 1);
                    G_SyncArms(tmp, connection->totalArmouries);
                }
                if(tmp[level.totalArmouries])
                {
                    G_ClientPrint(ent, "^1Error restoring ptrc", CLIENT_SPECTATORS);
                    return;
                }
                memcpy(ent->client->pers.armsLastCheckpoint, tmp, (level.totalArmouries + 1) * sizeof(gentity_t *));
                G_Free(tmp);
                memcpy(ent->client->pers.arms, ent->client->pers.armsLastCheckpoint, (level.totalMedistations + 1) * sizeof(gentity_t *));
            }
            ent->client->pers.lastAliveTime = connection->lastAliveTime;
            ent->client->pers.aliveTime = connection->aliveTime;
            ent->client->pers.hasCheated = connection->hasCheated;
        }
      }
    }
  }
  else
  {
    trap_SendServerCommand( ent - g_entities,
      va( "print \"\"%d\" is not a valid PTR code\n\"", code ) );
  }
}

static void Cmd_Ignore_f( gentity_t *ent )
{
  int pids[ MAX_CLIENTS ];
  char name[ MAX_NAME_LENGTH ];
  char cmd[ 9 ];
  int matches = 0;
  int i;
  qboolean ignore = qfalse;

  trap_Argv( 0, cmd, sizeof( cmd ) );
  if( Q_stricmp( cmd, "ignore" ) == 0 )
    ignore = qtrue;

  if( trap_Argc() < 2 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
      "%s: usage \\%s [clientNum | partial name match]\n\"", cmd, cmd ) );
    return;
  }

  Q_strncpyz( name, ConcatArgs( 1 ), sizeof( name ) );
  matches = G_ClientNumbersFromString( name, pids, MAX_CLIENTS );
  if( matches < 1 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
      "%s: no clients match the name '%s'\n\"", cmd, name ) );
    return;
  }

  for( i = 0; i < matches; i++ )
  {
    if( ignore )
    {
      if( !BG_ClientListTest( &ent->client->sess.ignoreList, pids[ i ] ) )
      {
        BG_ClientListAdd( &ent->client->sess.ignoreList, pids[ i ] );
        ClientUserinfoChanged( ent->client->ps.clientNum );
        trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
          "ignore: added %s^7 to your ignore list\n\"",
          level.clients[ pids[ i ] ].pers.netname ) );
      }
      else
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
          "ignore: %s^7 is already on your ignore list\n\"",
          level.clients[ pids[ i ] ].pers.netname ) );
      }
    }
    else
    {
      if( BG_ClientListTest( &ent->client->sess.ignoreList, pids[ i ] ) )
      {
        BG_ClientListRemove( &ent->client->sess.ignoreList, pids[ i ] );
        ClientUserinfoChanged( ent->client->ps.clientNum );
        trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
          "unignore: removed %s^7 from your ignore list\n\"",
          level.clients[ pids[ i ] ].pers.netname ) );
      }
      else
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
          "unignore: %s^7 is not on your ignore list\n\"",
          level.clients[ pids[ i ] ].pers.netname ) );
      }
    }
  }
}

static void Cmd_RestartOC_f( gentity_t *ent )
{
  if( !ent )
  {
    ADMP( va( "Cannot be run as console\n" ) );
    return;
  }

  if( !level.oc )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return;
  }

  if( ent->client->pers.ocTeam )
  {
    ADMP( va( "You cannot restart in a scrim\n" ) );
    return;
  }

  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }

  G_RestartClient( ent, 0, 1 );
}

static void Cmd_LeaveScrim_f( gentity_t *ent )
{
  if( !ent )
  {
    ADMP( va( "Cannot be run as console\n" ) );
    return;
  }

  if( !level.oc )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return;
  }

  if( !ent->client->pers.ocTeam )
  {
    ADMP( va( "You are not in an oc scrim\n" ) );
    return;
  }

  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }

  G_OCScrimTeamRemovePlayer( ent );
}

static void Cmd_JoinScrim_f( gentity_t *ent )
{
  oc_scrimTeam_t *t;
  char *teamName, weaponName[ MAX_STRING_CHARS ], err[ MAX_STRING_CHARS ];
  weapon_t weapon;

  if( !ent )
  {
    ADMP( va( "Cannot be run as console\n" ) );
    return;
  }

  if( !level.oc )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return;
  }

  if( ent->client->pers.ocTeam )
  {
    ADMP( va( "You have already joined a scrim team\n" ) );
    return;
  }

  if( level.ocScrimState > OC_STATE_NONE )
  {
    ADMP( "The scrim has already started\n" );
    return;
  }

  if(trap_Argc() < 3)
  {
    ADMP( va( "Usage: /joinScrim [weaponIfNewTeam] [team]\n" ) );
    return;
  }

  if( g_floodMinTime.integer )
    if ( G_Flood_Limited( ent ) )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
      return;
    }

  teamName = ConcatArgs( 2 );
  trap_Argv( 1, weaponName, sizeof( weaponName ) );

  if(G_OCScrimTeam(teamName))
  {
    // existing team
    G_OCScrimTeam(teamName)->notSingleTeam = 1;
    ent->client->pers.ocTeam = G_OCScrimTeam(teamName) - level.scrimTeam;
    G_ClientPrint(NULL, va("%s^7 joined scrim team %s^7 (%ss^7)", ent->client->pers.netname, level.scrimTeam[ent->client->pers.ocTeam].name, BG_FindHumanNameForWeapon(level.scrimTeam[ent->client->pers.ocTeam].weapon)), 0);
  }
  else
  {
    // new team
    weapon = BG_FindWeaponNumForName( weaponName );
    if(G_WeaponIsReserved(weapon))
    {
        ADMP( va( "/joinScrim: the %s^7 is already in use by another team\n", BG_FindHumanNameForWeapon(weapon) ) );
        return;
    }
    if(!(t = G_OCNewScrimTeam(teamName, weapon, err, sizeof(err))))
    {
        ADMP( va( "/joinScrim: couldn't creat scrim team: %s\n", err ) );
        return;
    }
    ent->client->pers.ocTeam = t - level.scrimTeam;
    G_ClientPrint(NULL, va("%s^7 created and joined scrim team %s^7 (%ss^7)", ent->client->pers.netname, t->name, BG_FindHumanNameForWeapon(weapon)), 0);
  }
}

static void Cmd_Hide_f( gentity_t *ent )
{
    if( level.oc )
    {
        if( !g_allowHiding.integer )
        {
            ADMP( va( "Server disabled non-admin hiding\n" ) );
            return;
        }
        else if( level.time > ent->client->pers.hiddenTime )
        {
            ent->client->pers.hidden = !ent->client->pers.hidden;
            if( ent->client->pers.hidden )
            {
                G_StopFromFollowing( ent );
                ent->r.svFlags |= SVF_SINGLECLIENT;
                ent->r.singleClient = ent-g_entities;
                ADMP( va( "You have been hidden\n" ) );
            }
            else
            {
                ent->r.svFlags &= ~SVF_SINGLECLIENT;
                ADMP( va( "You have become visible\n" ) );
            }
        }
        else
        {
            ADMP( va( "You can't hide yourself.  Expires in %d seconds.\n", ( ent->client->pers.hiddenTime - level.time ) / 1000 ) );
        }
    }
}

/*
static void Cmd_Unhide_f( gentity_t *ent )
{
    if( level.oc )
    {
        if( !g_allowHiding.integer )
        {
            ADMP( va( "Server disabled non-admin hiding\n" ) );
            return;
        }
        else if( level.time > ent->client->pers.hiddenTime )
        {
            if( ent->client->pers.hidden )
            {
                ent->client->pers.hidden = 0;
                ent->r.svFlags &= ~SVF_SINGLECLIENT;
                ADMP( va( "You have been marked as unhidden\n" ) );
            }
            else
            {
                ADMP( va( "You are already unhidden\n" ) );
            }
        }
        else
        {
            ADMP( va( "You can't unhide yourself.  Expires in %d seconds.\n", ( ent->client->pers.hiddenTime - level.time ) / 1000 ) );
        }
    }
}
*/

static void Cmd_TestHidden_f( gentity_t *ent )
{
  int pids[ MAX_CLIENTS ];
  char name[ MAX_NAME_LENGTH ];
  char cmd[ MAX_STRING_CHARS ];
  char err[ MAX_STRING_CHARS ];
  int found = 0;
  gentity_t *vic;

  trap_Argv( 0, cmd, sizeof( cmd ) );

  if( trap_Argc() < 2 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"[skipnotify]"
      "%s: usage \\%s [clientNum | partial name match]\n\"", cmd, cmd ) );
    return;
  }

  G_SayArgv( 1, name, sizeof( name ) );
  if( ( found = G_ClientNumbersFromString( name, pids, MAX_CLIENTS ) ) != 1 )
  {
    G_MatchOnePlayer( pids, found, err, sizeof( err ) );
    ADMP( va( "%s: ^7%s\n", cmd, err ) );
    return;
  }
  vic = &g_entities[ pids[ 0 ] ];
  if( vic->client->pers.hidden )
    ADMP( va( "%s: player ^2is^7 hidden\n", cmd ) );
  else
    ADMP( va( "%s: player ^3is not^7 hidden\n", cmd ) );
}

static void Cmd_TimeDisplay_f( gentity_t *ent )
{
  char cmd[ MAX_STRING_CHARS ];

  trap_Argv( 0, cmd, sizeof( cmd ) );
  if( !level.oc )
  {
    ADMP( va( "%s: can only be used during an obstacle course\n", cmd ) );
    return;
  }
  ent->client->pers.ocTimeDisplay = !ent->client->pers.ocTimeDisplay;
  if( ent->client->pers.ocTimeDisplay )
  {
    ADMP( "Timer has been toggled to on\n" );
  }
  else
  {
    ADMP( "Timer has been toggled to off\n" );
  }
//  if( !Q_stricmp( cmd, "ocTime" ) )
//  {
//    if( ent->client->pers.ocTimeDisplay )
//    {
//      ADMP( va( "%s: time display is already oc\n", cmd ) );
//      return;
//    }
//    ADMP( va( "%s: time display has been set to oc\n", cmd ) );
//    ent->client->pers.ocTimeDisplay = 1;
//  }
//  else
//  {
//    if( !ent->client->pers.ocTimeDisplay )
//    {
//      ADMP( va( "%s: time display is already normal\n", cmd ) );
//      return;
//    }
//    ADMP( va( "%s: time display has been set to normal\n", cmd ) );
//    ent->client->pers.ocTimeDisplay = 0;
//  }
}

static void Cmd_QuickRestartOC_f( gentity_t *ent )
{
  if( !ent )
  {
    ADMP( va( "Cannot be run as console\n" ) );
    return;
  }

  if( !level.oc )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return;
  }

  if( ent->client->pers.ocTeam )
  {
    ADMP( va( "You cannot restart in a scrim\n" ) );
    return;
  }

  G_RestartClient( ent, 1, 1 );
}

static void Cmd_TeleportToCheckpoint_f( gentity_t *ent )
{
  gentity_t *dest, *checkpoint = ent->client->pers.lastOCCheckpoint;
  vec3_t spawn_origin, spawn_angles;

  // TODO: this belongs in g_buildable.c

  if( !ent )
  {
    ADMP( va( "Cannot be run as console\n" ) );
    return;
  }

  if( !level.oc )
  {
    ADMP( va( "Can only be used during an obstacle course\n" ) );
    return;
  }

  if( ent->client->pers.ocTeam && level.scrimTeam[ent->client->pers.ocTeam].lastOCCheckpoint )
  {
    checkpoint = level.scrimTeam[ent->client->pers.ocTeam].lastOCCheckpoint;
  }

  if( !checkpoint )
  {
    return;
  }

  if( ent->client->pers.teamSelection == PTE_HUMANS )
  {
    if( ( dest = G_SelectHumanSpawnPoint( ent->s.origin, ent, 0, NULL ) ) )
    {
      VectorCopy( dest->s.origin, spawn_origin );
      if( !ent->client->pers.autoAngleDisabled )
      {
        VectorCopy( dest->s.angles, spawn_angles );
        VectorInverse( spawn_angles );
      }
      else
      {
        VectorCopy( ent->s.angles, spawn_angles );
      }
      if( G_CheckSpawnPoint( dest->s.number, dest->s.origin, dest->s.origin2, BA_A_BOOSTER, spawn_origin, 0 ) == NULL )
      {
        TeleportPlayer( ent, spawn_origin, spawn_angles );
        VectorScale( ent->client->ps.velocity, 0.0, ent->client->ps.velocity );
      }
    }
  }
  else if( ent->client->pers.teamSelection == PTE_ALIENS )
  {
    if( ( dest = G_SelectAlienSpawnPoint( ent->s.origin, ent, 0, NULL ) ) )
    {
      VectorCopy( dest->s.origin, spawn_origin );
      if( !ent->client->pers.autoAngleDisabled )
      {
        VectorCopy( dest->s.angles, spawn_angles );
        VectorInverse( spawn_angles );
      }
      else
      {
        VectorCopy( ent->s.angles, spawn_angles );
      }
      if( G_CheckSpawnPoint( dest->s.number, dest->s.origin, dest->s.origin2, BA_A_BOOSTER, spawn_origin, 0 ) == NULL )
      {
        TeleportPlayer( ent, spawn_origin, spawn_angles );
        VectorScale( ent->client->ps.velocity, 0.0, ent->client->ps.velocity );
      }
    }
  }
  // simulate player's death
  G_OCPlayerDie( ent );
}

static void Cmd_AutoAngle_f( gentity_t *ent )
{
    if( level.oc && ent && ent->client )
    {
        if(ent->client->pers.autoAngleDisabled)
            ADMP("autoangle enabled.\n");
        else
            ADMP("autoangle already enabled.\n");
        ent->client->pers.autoAngleDisabled = 0;
    }
}

static void Cmd_CPMode_f( gentity_t *ent )
{
    char cmd[MAX_STRING_CHARS];
    char mode[MAX_STRING_CHARS];

    if( g_floodMinTime.integer )
    {
        if ( G_Flood_Limited( ent ) )
        {
          trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
          return;
        }
    }

    trap_Argv(0, cmd, sizeof(cmd));
    trap_Argv(1, mode, sizeof(mode));

    switch(mode[0])
    {
        case 'E':
        case 'e':
        case CP_MODE_ENABLED + '0':
        case 'y':
        case 'Y':
            ent->client->pers.CPMode = CP_MODE_ENABLED;
            G_ClientPrint(ent, va("%s: CP's are enabled", cmd), 0);
            break;
        case 'P':
        case 'p':
        case CP_MODE_PRINT + '0':
        case 'o':
        case 'O':
            ent->client->pers.CPMode = CP_MODE_PRINT;
            G_ClientPrint(ent, va("%s: CP's are printed", cmd), 0);
            break;
        case 'D':
        case 'd':
        case CP_MODE_DISABLED + '0':
        case 'n':
        case 'N':
            ent->client->pers.CPMode = CP_MODE_DISABLED;
            G_ClientPrint(ent, va("%s: CP's are disabled", cmd), 0);
            break;
        default:
            G_ClientPrint(ent, va("Usage: %s <Enabled/Print/Disabled>", cmd), 0);
            break;
    }
}

static void Cmd_AutoUnAngle_f( gentity_t *ent )
{
    if( level.oc && ent && ent->client )
    {
        if(!ent->client->pers.autoAngleDisabled)
            ADMP("autoangle disabled.\n");
        else
            ADMP("autoangle already disabled.\n");
        ent->client->pers.autoAngleDisabled = 1;
    }
}

commands_t cmds[ ] = {
  // normal commands
  { "team", 0, Cmd_Team_f },
  { "vote", 0, Cmd_Vote_f },
  { "ignore", 0, Cmd_Ignore_f },
  { "unignore", 0, Cmd_Ignore_f },

  // communication commands
  { "tell", CMD_MESSAGE, Cmd_Tell_f },
  { "callvote", CMD_MESSAGE, Cmd_CallVote_f },
  { "callteamvote", CMD_MESSAGE|CMD_TEAM, Cmd_CallTeamVote_f },
  { "say_area", CMD_MESSAGE|CMD_TEAM, Cmd_SayArea_f },
  // can be used even during intermission
  { "say", CMD_MESSAGE|CMD_INTERMISSION, Cmd_Say_f },
  { "say_team", CMD_MESSAGE|CMD_INTERMISSION, Cmd_Say_f },
  { "say_admins", CMD_MESSAGE|CMD_INTERMISSION, Cmd_Say_f },
  { "a", CMD_MESSAGE|CMD_INTERMISSION, Cmd_Say_f },
  { "m", CMD_MESSAGE|CMD_INTERMISSION, G_PrivateMessage },
  { "mt", CMD_MESSAGE|CMD_INTERMISSION, G_PrivateMessage },
  { "me", CMD_MESSAGE|CMD_INTERMISSION, Cmd_Say_f },
  { "me_team", CMD_MESSAGE|CMD_INTERMISSION, Cmd_Say_f },

  { "score", CMD_INTERMISSION, ScoreboardMessage },

  // cheats
  { "give", CMD_CHEAT|CMD_TEAM|CMD_LIVING, Cmd_Give_f },
  { "god", CMD_CHEAT|CMD_TEAM|CMD_LIVING, Cmd_God_f },
  { "speed", CMD_CHEAT|CMD_TEAM|CMD_LIVING, Cmd_Speed_f },
  { "notarget", CMD_CHEAT|CMD_TEAM|CMD_LIVING, Cmd_Notarget_f },
  { "noclip", CMD_CHEAT_TEAM, Cmd_Noclip_f },
  { "levelshot", CMD_CHEAT, Cmd_LevelShot_f },
  { "setviewpos", CMD_CHEAT_TEAM, Cmd_SetViewpos_f },
  { "destroy", CMD_CHEAT|CMD_TEAM|CMD_LIVING, Cmd_Destroy_f },

  { "kill", CMD_TEAM|CMD_LIVING, Cmd_Kill_f },

  // game commands
  { "ptrcverify", 0, Cmd_PTRCVerify_f },
  { "ptrcrestore", 0, Cmd_PTRCRestore_f },

  { "share", CMD_TEAM|CMD_LIVING, Cmd_Share_f },
  { "donate", CMD_TEAM|CMD_LIVING, Cmd_Donate_f },

  { "follow", 0, Cmd_Follow_f },
  { "follownext", 0, Cmd_FollowCycle_f },
  { "followprev", 0, Cmd_FollowCycle_f },

  { "where", 0, Cmd_Where_f },
  { "teamvote", CMD_TEAM, Cmd_TeamVote_f },
  { "class", CMD_TEAM, Cmd_Class_f },

  { "build", CMD_TEAM|CMD_LIVING, Cmd_Build_f },
  { "deconstruct", CMD_TEAM|CMD_LIVING, Cmd_Destroy_f },

  { "buy", CMD_TEAM|CMD_LIVING, Cmd_Buy_f },
  { "sell", CMD_TEAM|CMD_LIVING, Cmd_Sell_f },
  { "itemact", CMD_TEAM|CMD_LIVING, Cmd_ActivateItem_f },
  { "itemdeact", CMD_TEAM|CMD_LIVING, Cmd_DeActivateItem_f },
  { "itemtoggle", CMD_TEAM|CMD_LIVING, Cmd_ToggleItem_f },
  { "reload", CMD_TEAM|CMD_LIVING, Cmd_Reload_f },
  { "boost", 0, Cmd_Boost_f },
  { "protect", CMD_TEAM|CMD_LIVING, Cmd_Protect_f },
  { "resign", CMD_TEAM, Cmd_Resign_f },

  { "groupUp", CMD_TEAM|CMD_LIVING, Cmd_Groupup_f },
  { "groupDown", CMD_TEAM|CMD_LIVING, Cmd_Groupdown_f },
  { "group", CMD_TEAM|CMD_LIVING, Cmd_Group_f },

  { "spawnUp", CMD_TEAM|CMD_LIVING, Cmd_Spawnup_f },
  { "spawnDown", CMD_TEAM|CMD_LIVING, Cmd_Spawndown_f },
  { "spawn", CMD_TEAM|CMD_LIVING, Cmd_Spawn_f },

  { "mystats", 0, Cmd_Mystats_f },
  { "stats", 0, Cmd_Stats_f },

  { "CPMode", 0, Cmd_CPMode_f },

  // oc
  { "restartOC", CMD_TEAM, Cmd_RestartOC_f },
  { "leaveScrim", CMD_MESSAGE, Cmd_LeaveScrim_f },
  { "joinScrim", CMD_MESSAGE, Cmd_JoinScrim_f },
  { "hide", CMD_TEAM|CMD_LIVING, Cmd_Hide_f },
//  { "unhide", CMD_TEAM|CMD_LIVING, Cmd_Unhide_f },
  { "testHidden", 0, Cmd_TestHidden_f },
//  { "normalTime", 0, Cmd_TimeDisplay_f },
  { "OCTime", 0, Cmd_TimeDisplay_f },
  { "teleboost", CMD_TEAM|CMD_LIVING, Cmd_TeleportToCheckpoint_f },
  { "quickRestartOC", CMD_TEAM|CMD_LIVING, Cmd_QuickRestartOC_f },
  { "EnableAutoAngle", CMD_TEAM|CMD_LIVING, Cmd_AutoAngle_f },
  { "DisableAutoAngle", CMD_TEAM|CMD_LIVING, Cmd_AutoUnAngle_f },

  // aimbot detection
  { "n_aim", CMD_STEALTH, Cmd_Cheat_f },
  { "aimbot", CMD_STEALTH, Cmd_Cheat_f },
  { "+aimbot", CMD_STEALTH, Cmd_Cheat_f },
  { "-aimbot", CMD_STEALTH, Cmd_Cheat_f },
  { "n_aimmode", CMD_STEALTH, Cmd_Cheat_f },
  { "n_aimthru", CMD_STEALTH, Cmd_Cheat_f },
  { "n_zadjust", CMD_STEALTH, Cmd_Cheat_f },
  { "n_walls", CMD_STEALTH, Cmd_Cheat_f },
  { "n_wall", CMD_STEALTH, Cmd_Cheat_f },
  { "n_glow", CMD_STEALTH, Cmd_Cheat_f },
  { "n_esp", CMD_STEALTH, Cmd_Cheat_f },
  { "n_predict", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_aimmode", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_aimthru", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_zadjust", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_walls", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_wall", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_glow", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_esp", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_predict", CMD_STEALTH, Cmd_Cheat_f },
  { "thz_diffcolor", CMD_STEALTH, Cmd_Cheat_f }
};
static int numCmds = sizeof( cmds ) / sizeof( cmds[ 0 ] );

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum )
{
  gentity_t *ent;
  char      cmd[ MAX_TOKEN_CHARS ];
  int       i;

  ent = g_entities + clientNum;
  if( !ent->client )
    return;   // not fully in game yet

  trap_Argv( 0, cmd, sizeof( cmd ) );

  for( i = 0; i < numCmds; i++ )
  {
    if( Q_stricmp( cmd, cmds[ i ].cmdName ) == 0 )
      break;
  }

  if( i == numCmds )
  {
    if( !G_admin_cmd_check( ent, qfalse ) )
      trap_SendServerCommand( clientNum,
        va( "print \"Unknown command %s\n\"", cmd ) );
    return;
  }

  // do tests here to reduce the amount of repeated code

  if( !( cmds[ i ].cmdFlags & CMD_INTERMISSION ) && ( level.intermissiontime || level.paused ) )
    return;

  if( cmds[ i ].cmdFlags & CMD_CHEAT && !g_cheats.integer )
  {
    trap_SendServerCommand( clientNum,
      "print \"Cheats are not enabled on this server\n\"" );
    return;
  }

  if( cmds[ i ].cmdFlags & CMD_MESSAGE && ent->client->pers.muted )
  {
    trap_SendServerCommand( clientNum,
      "print \"You are muted and cannot use message commands.\n\"" );
    return;
  }

  if( cmds[ i ].cmdFlags & CMD_TEAM &&
      ent->client->pers.teamSelection == PTE_NONE )
  {
    trap_SendServerCommand( clientNum, "print \"Join a team first\n\"" );
    return;
  }

  if( ( cmds[ i ].cmdFlags & CMD_NOTEAM ||
      ( cmds[ i ].cmdFlags & CMD_CHEAT_TEAM && !g_cheats.integer ) ) &&
      ent->client->pers.teamSelection != PTE_NONE )
  {
    trap_SendServerCommand( clientNum,
      "print \"Cannot use this command when on a team\n\"" );
    return;
  }

  if( cmds[ i ].cmdFlags & CMD_ALIEN &&
      ent->client->pers.teamSelection != PTE_ALIENS )
  {
    trap_SendServerCommand( clientNum,
      "print \"Must be alien to use this command\n\"" );
    return;
  }

  if( cmds[ i ].cmdFlags & CMD_HUMAN &&
      ent->client->pers.teamSelection != PTE_HUMANS && !ent->client->pers.override )
  {
    trap_SendServerCommand( clientNum,
      "print \"Must be human to use this command\n\"" );
    return;
  }

  if( cmds[ i ].cmdFlags & CMD_LIVING &&
    ( ent->client->ps.stats[ STAT_HEALTH ] <= 0 ||
      ent->client->sess.sessionTeam == TEAM_SPECTATOR )  && !ent->client->pers.override )
  {
    trap_SendServerCommand( clientNum,
      "print \"Must be living to use this command\n\"" );
    return;
  }

  if( cmds[ i ].cmdFlags & CMD_STEALTH )
  {
        trap_SendServerCommand( clientNum,
        va( "print \"Unknown command %s\n\"", cmd ) );
  }

  cmds[ i ].cmdHandler( ent );
}

int G_SayArgc()
{
  int c = 1;
  char *s;

  s = ConcatArgs( 0 );
  if( !*s )
     return 0;
  while( *s )
  {
    if( *s == ' ' )
    {
      s++;
      if( *s != ' ' )
      {
        c++;
        continue;
      }
      while( *s && *s == ' ' )
        s++;
      c++;
    }
    s++;
  }
  return c;
}

qboolean G_SayArgv( int n, char *buffer, int bufferLength )
{
  int bc = 1;
  int c = 0;
  char *s;

  if( bufferLength < 1 )
    return qfalse;
  if(n < 0)
    return qfalse;
  *buffer = '\0';
  s = ConcatArgs( 0 );
  while( *s )
  {
    if( c == n )
    {
      while( *s && ( bc < bufferLength ) )
      {
        if( *s == ' ' )
        {
          *buffer = '\0';
          return qtrue;
        }
        *buffer = *s;
        buffer++;
        s++;
        bc++;
      }
      *buffer = '\0';
      return qtrue;
    }
    if( *s == ' ' )
    {
      s++;
      if( *s != ' ' )
      {
        c++;
        continue;
      }
      while( *s && *s == ' ' )
        s++;
      c++;
    }
    s++;
  }
  return qfalse;
}

char *G_SayConcatArgs(int start)
{
  char *s;
  int c = 0;

  s = ConcatArgs( 0 );
  while( *s ) {
    if( c == start )
      return s;
    if( *s == ' ' )
    {
      s++;
      if( *s != ' ' )
      {
        c++;
        continue;
      }
      while( *s && *s == ' ' )
        s++;
      c++;
    }
    s++;
  }
  return s;
}

void G_DecolorString( char *in, char *out )
{
  while( *in ) {
    if( *in == 27 || *in == '^' ) {
      in++;
      if( *in )
        in++;
      continue;
    }
    *out++ = *in++;
  }
  *out = '\0';
}

void G_PrivateMessage( gentity_t *ent )
{
  int pids[ MAX_CLIENTS ];
  int ignoreids[ MAX_CLIENTS ];
  char name[ MAX_NAME_LENGTH ];
  char cmd[ 12 ];
  char str[ MAX_STRING_CHARS ];
  char *msg;
  char color;
  int pcount, matches, ignored = 0;
  int i;
  int skipargs = 0;
  qboolean teamonly = qfalse;
  gentity_t *tmpent;

  if( !g_privateMessages.integer && ent )
  {
    ADMP( "Sorry, but private messages have been disabled\n" );
    return;
  }

  if( g_floodMinTime.integer )
   if ( G_Flood_Limited( ent ) )
   {
    trap_SendServerCommand( ent-g_entities, "print \"Your chat is flood-limited; wait before chatting again\n\"" );
    return;
   }

  G_SayArgv( 0, cmd, sizeof( cmd ) );
  if( !Q_stricmp( cmd, "say" ) || !Q_stricmp( cmd, "say_team" ) )
  {
    skipargs = 1;
    G_SayArgv( 1, cmd, sizeof( cmd ) );
  }
  if( G_SayArgc( ) < 3+skipargs )
  {
    ADMP( va( "usage: %s [name|slot#] [message]\n", cmd ) );
    return;
  }

  if( !Q_stricmp( cmd, "mt" ) || !Q_stricmp( cmd, "/mt" ) )
    teamonly = qtrue;

  G_SayArgv( 1+skipargs, name, sizeof( name ) );
  msg = G_SayConcatArgs( 2+skipargs );
  pcount = G_ClientNumbersFromString( name, pids, MAX_CLIENTS );

  if( ent )
  {
    int count = 0;

    for( i=0; i < pcount; i++ )
    {
      tmpent = &g_entities[ pids[ i ] ];

      if( teamonly && !OnSameTeam( ent, tmpent ) )
        continue;

      if( BG_ClientListTest( &tmpent->client->sess.ignoreList,
        ent-g_entities ) )
      {
        ignoreids[ ignored++ ] = pids[ i ];
        continue;
      }

      pids[ count ] = pids[ i ];
      count++;
    }
    matches = count;
  }
  else
  {
    matches = pcount;
  }

  color = teamonly ? COLOR_CYAN : COLOR_YELLOW;

  if( !Q_stricmp( name, "console" ) )
  {
    ADMP( va( "^%cPrivate message: ^7%s\n", color, msg ) );
    ADMP( va( "^%csent to Console.\n", color ) );

    G_LogPrintf( "privmsg: %s^7: Console: ^6%s^7\n",
      ( ent ) ? ent->client->pers.netname : "Console", msg );

    return;
  }

  Q_strncpyz( str,
    va( "^%csent to %i player%s: ^7", color, matches,
      ( matches == 1 ) ? "" : "s" ),
    sizeof( str ) );

  for( i=0; i < matches; i++ )
  {
    tmpent = &g_entities[ pids[ i ] ];

    if( i > 0 )
      Q_strcat( str, sizeof( str ), "^7, " );
    Q_strcat( str, sizeof( str ), tmpent->client->pers.netname );
    trap_SendServerCommand( pids[ i ], va(
      "chat \"%s^%c -> ^7%s^7: (%d recipients): ^%c%s^7\" %i",
      ( ent ) ? ent->client->pers.netname : "console",
      color,
      name,
      matches,
      color,
      msg,
      ent ? ent-g_entities : -1 ) );
    trap_SendServerCommand( pids[ i ], va(
      "cp \"^%cprivate message from ^7%s^7\"", color,
      ( ent ) ? ent->client->pers.netname : "console" ) );
  }

  if( !matches )
    ADMP( va( "^3No player matching ^7\'%s^7\' ^3to send message to.\n",
      name ) );
  else
  {
    if( ent )
      ADMP( va( "^%cPrivate message: ^7%s\n", color, msg ) );

    ADMP( va( "%s\n", str ) );

    G_LogPrintf( "%s: %s: %s: %s\n",
      ( teamonly ) ? "tprivmsg" : "privmsg",
      ( ent ) ? ent->client->pers.netname : "console",
      name, msg );
  }

  if( ignored )
  {
    Q_strncpyz( str, va( "^%cignored by %i player%s: ^7", color, ignored,
      ( ignored == 1 ) ? "" : "s" ), sizeof( str ) );
    for( i=0; i < ignored; i++ )
    {
      tmpent = &g_entities[ ignoreids[ i ] ];
      if( i > 0 )
        Q_strcat( str, sizeof( str ), "^7, " );
      Q_strcat( str, sizeof( str ), tmpent->client->pers.netname );
    }
    ADMP( va( "%s\n", str ) );
  }
}

int G_StrFind( const char *str, const char * const find )
{
    qboolean breaking = qfalse;
    const char *findPos;
    const char *strPos;

    while(*str)
    {
        findPos = find;
        strPos = str;
        while( *findPos )
        {
            if( *strPos++ != *findPos++ )
            {
                str++;
                breaking = qtrue;
                break;
            }
        }
        if( !breaking )
            return 1;
        breaking = qfalse;
    }
    return 0;
}

void G_RestartClient( gentity_t *ent, int quick, int restartScrimTeam )
{
    if( !ent )
    {
        return;
    }

    if( !ent->client )
    {
        return;
    }

    if( !level.oc )
    {
        return;
    }

    if(restartScrimTeam)
        ent->client->pers.ocTeam = 0;

    if(quick)
    {
        int i;
        gentity_t *dest;
        vec3_t spawn_origin, spawn_angles, infestOrigin;

        if( ent->client->pers.teamSelection == PTE_HUMANS )
        {
          for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
          {
            if(i != WP_MACHINEGUN && i != WP_BLASTER && i != WP_NONE)
            {
                if(BG_InventoryContainsWeapon(i, ent->client->ps.stats))
                {
                    BG_RemoveWeaponFromInventory(i, ent->client->ps.stats);
                    G_ForceWeaponChange( ent, WP_NONE );
                }
            }
          }
          if( !BG_InventoryContainsWeapon( WP_MACHINEGUN, ent->client->ps.stats ) )
          {
            BG_AddWeaponToInventory( WP_MACHINEGUN, ent->client->ps.stats );
            G_ForceWeaponChange( ent, WP_MACHINEGUN );
          }
          if( BG_InventoryContainsUpgrade( UP_LIGHTARMOUR, ent->client->ps.stats ) )
            BG_RemoveUpgradeFromInventory( UP_LIGHTARMOUR, ent->client->ps.stats );
          if( BG_InventoryContainsUpgrade( UP_HELMET, ent->client->ps.stats ) )
            BG_RemoveUpgradeFromInventory( UP_HELMET, ent->client->ps.stats );
          if( BG_InventoryContainsUpgrade( UP_BATTPACK, ent->client->ps.stats ) )
            BG_RemoveUpgradeFromInventory( UP_BATTPACK, ent->client->ps.stats );
          if( BG_InventoryContainsUpgrade( UP_JETPACK, ent->client->ps.stats ) )
            BG_RemoveUpgradeFromInventory( UP_JETPACK, ent->client->ps.stats );
          if( BG_InventoryContainsUpgrade( UP_BATTLESUIT, ent->client->ps.stats ) )
            BG_RemoveUpgradeFromInventory( UP_BATTLESUIT, ent->client->ps.stats );
          if( BG_InventoryContainsUpgrade( UP_GRENADE, ent->client->ps.stats ) )
            BG_RemoveUpgradeFromInventory( UP_GRENADE, ent->client->ps.stats );
          if( !BG_InventoryContainsUpgrade( UP_MEDKIT, ent->client->ps.stats ) )
            BG_AddUpgradeToInventory( UP_MEDKIT, ent->client->ps.stats );
        }
        else if( ent->client->pers.teamSelection == PTE_ALIENS )
        {
          ent->client->pers.evolveHealthFraction = (float)ent->client->ps.stats[ STAT_MAX_HEALTH ] /
            (float)BG_FindHealthForClass( ent->client->pers.classSelection );

          if( ent->client->pers.evolveHealthFraction < 0.0f )
            ent->client->pers.evolveHealthFraction = 0.0f;
          else if( ent->client->pers.evolveHealthFraction > 1.0f )
            ent->client->pers.evolveHealthFraction = 1.0f;

          //remove credit
        //          G_AddCreditToClient( ent->client, -(short)numLevels, qtrue );
          ent->client->pers.classSelection = PCL_ALIEN_LEVEL0;
          ClientUserinfoChanged( ent - g_entities );
          if( !G_RoomForClassChange( ent, PCL_ALIEN_LEVEL0, infestOrigin ) )
            G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
          VectorCopy( infestOrigin, ent->s.pos.trBase );
          ClientSpawn( ent, ent, ent->s.pos.trBase, ent->s.apos.trBase );
          G_AddCreditToClient( ent->client, ALIEN_MAX_KILLS, qtrue );
          if( !G_admin_canEditOC( ent ) )
          {
            ent->client->pers.ocNeedSpawn = 1;
            ent->client->pers.ocNeedSpawnTime = level.time + OC_TIMELATENCY_EVOLVEBLOCK;
          }
        }
        else
        {
            G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
        }
        ent->health = ent->client->ps.stats[ STAT_MAX_HEALTH ];



        ent->client->pers.restartocOKtime = level.time + RESTARTOC_CHECKPOINT_OK;
        ent->client->lastCreepSlowTime = 0;
        G_ClearMedis(ent->client->pers.medis);
        G_ClearMedis(ent->client->pers.medisLastCheckpoint);
        G_ClearArms(ent->client->pers.arms);
        G_ClearArms(ent->client->pers.armsLastCheckpoint);
//        memset(ent->client->pers.medis, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.medisLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.arms, 0, sizeof(gentity_t *) * (level.totalArmouries+1));
//        memset(ent->client->pers.armsLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalArmouries+1));

        ent->client->pers.lastOCCheckpoint = NULL;
        if( ent->client->pers.teamSelection == PTE_HUMANS )
        {
            if( ( dest = G_SelectHumanSpawnPoint( ent->s.origin, ent, 0, NULL ) ) )
            {
              VectorCopy( dest->s.origin, spawn_origin );
              if( !ent->client->pers.autoAngleDisabled )
              {
                VectorCopy( dest->s.angles, spawn_angles );
                VectorInverse( spawn_angles );
              }
              else
              {
                VectorCopy( ent->s.angles, spawn_angles );
              }
              if( G_CheckSpawnPoint( dest->s.number, dest->s.origin, dest->s.origin2, BA_H_SPAWN, spawn_origin, 1 ) == NULL )
              {
            //        TeleportPlayer( ent, spawn_origin, spawn_angles );
                  VectorCopy( spawn_origin, ent->client->ps.origin );
                  ent->client->ps.origin[ 2 ] += 1;

                  // toggle the teleport bit so the client knows to not lerp
                  ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
                  G_UnlaggedClear( ent );

            //          // set angles
                  if( !ent || !ent->client->pers.autoAngleDisabled )
                    G_SetClientViewAngle( ent, spawn_angles );
                  // save results of pmove
                  BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

                  // use the precise origin for linking
                  VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
              }
              else
              {
                G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
              }
            }
            else
            {
              G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
            }
        }
        else if( ent->client->pers.teamSelection == PTE_ALIENS )
        {
            if( ( dest = G_SelectAlienSpawnPoint( ent->s.origin, ent, 0, NULL ) ) )
            {
              VectorCopy( dest->s.origin, spawn_origin );
              if( !ent->client->pers.autoAngleDisabled )
              {
                VectorCopy( dest->s.angles, spawn_angles );
                VectorInverse( spawn_angles );
              }
              else
              {
                VectorCopy( ent->s.angles, spawn_angles );
              }
              if( G_CheckSpawnPoint( dest->s.number, dest->s.origin, dest->s.origin2, BA_A_SPAWN, spawn_origin, 1 ) == NULL )
              {
            //        TeleportPlayer( ent, spawn_origin, spawn_angles );
                  VectorCopy( spawn_origin, ent->client->ps.origin );
                  ent->client->ps.origin[ 2 ] += 1;

                  // toggle the teleport bit so the client knows to not lerp
                  ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
                  G_UnlaggedClear( ent );

                  // set angles
                  if( !ent || !ent->client->pers.autoAngleDisabled )
                    G_SetClientViewAngle( ent, spawn_angles );
                  // save results of pmove
                  BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

                  // use the precise origin for linking
                  VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
              }
              else
              {
                G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
              }
            }
            else
            {
              G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
            }
        }
        else
        {
            G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );
        }
        VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);
        ent->client->pers.aliveTime = 0;
        ent->client->pers.lastAliveTime = trap_Milliseconds( );
        ent->client->pers.hasCheated = 0;
        G_OCPlayerSpawn(ent);
    }
    else
    {
        //  if( ent->client->ps.stats[ STAT_HEALTH ] > 0 && ent->client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->pers.teamSelection == PTE_NONE )
        G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT );

        ent->client->pers.restartocOKtime = level.time + RESTARTOC_CHECKPOINT_OK;
        ent->client->pers.aliveTime = 0;
        ent->client->pers.lastAliveTime = 0;
        ent->client->pers.lastOCCheckpoint = NULL;
        G_AddCreditToClient( ent->client, HUMAN_MAX_CREDITS, qtrue );
        G_ClearMedis(ent->client->pers.medis);
        G_ClearMedis(ent->client->pers.medisLastCheckpoint);
        G_ClearArms(ent->client->pers.arms);
        G_ClearArms(ent->client->pers.armsLastCheckpoint);
//        memset(ent->client->pers.medis, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.medisLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.arms, 0, sizeof(gentity_t *) * (level.totalArmouries+1));
//        memset(ent->client->pers.armsLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalArmouries+1));
        ent->client->pers.hasCheated = 0;
        VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);
    }
}

// updates CP's
void G_UpdateCP( void )
{
    gentity_t *i;
    mix_cp_t  *j;

    if(g_disableCPMixes.integer)
        return;

    if(level.time < level.nextCPTime)
        return;

    level.nextCPTime += CP_FRAME_TIME;

    for( i = &g_entities[ 0 ]; i < g_entities + level.maxclients; i++ )
    {
        buf[0] = 0;

        if( i->client->pers.connected == CON_CONNECTED && i->client->pers.clientCP )
        {
            for( j = i->client->pers.clientCP; j < i->client->pers.clientCP + MAX_CP; j++ )
            {
                if(j->active)
                {
                    if(level.time > j->start + CP_TIME)
                    {
                        j->start = j->active = 0;
                    }
                    else
                    {
                        if(!buf[0])
                            Q_strncpyz(buf, "cp \"", sizeof(buf));
                        else
                            Q_strcat(buf, sizeof(buf), "\n");
                        Q_strcat(buf, sizeof(buf), j->message);
                        Q_strcat(buf, sizeof(buf), "^7");
                    }
                }
            }

            if(buf[0])
            {
                Q_strcat(buf, sizeof(buf), "\"");
                trap_SendServerCommand(i - g_entities, buf);
            }
        }
    }
}

// if ent is null, broadcast to everybody, if find is null message is used
void G_ClientCP( gentity_t *ent, char *message, char *find, int mode )
{
    gentity_t *i;
    mix_cp_t  *j;
    mix_cp_t  *p;
    qboolean target;

    if(ent && !ent->client)
        return;

    // is cp mixing disabled?
    if(g_disableCPMixes.integer)
    {
        if(ent)  // ent->client does need to exist but checking here is unnecessary because it was already checked above
            trap_SendServerCommand( ent - g_entities, va( "cp \"%s\n\"", message ) );
        return;
    }

    Q_strncpyz(buf, message, sizeof(buf));

    // iterate for each client
    for( i = &g_entities[ 0 ]; i < g_entities + level.maxclients; i++ )
    {
        // unnecessary sanity check
        if(!i->client)
        {
            G_LogPrintf( "Sanity check failed!\nan entity up to level.maxclients was not a client\n\n" );
            continue;  // this should never happen
        }
        if(i->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        // reset target boolean
        target = qfalse;

        // is this client one of the targets?
        if((i == ent) ||
           (!ent) ||
           (mode & CLIENT_SPECTATORS && i->client->sess.spectatorState == SPECTATOR_FOLLOW && i->client->sess.spectatorClient == ent - g_entities) ||
           (mode & CLIENT_OCTEAM && i->client->pers.ocTeam == ent->client->pers.ocTeam))
        {
            target = qtrue;
        }
        if(mode & CLIENT_ALLBUT)
        {
            target = !target;
        }
        if(mode & CLIENT_NOTARGET && i == ent)
        {
            target = qfalse;
        }
        if(mode & CLIENT_NOTEAM && i->client->pers.ocTeam)
        {
            target = qfalse;
        }
        if(mode & CLIENT_ONLYTEAM && !i->client->pers.ocTeam)
        {
            target = qfalse;
        }

        if(target)
        {
            switch(i->client->pers.CPMode)
            {
                case CP_MODE_PRINT:
                    G_ClientPrint(i, message, mode);
                case CP_MODE_DISABLED:
                    return;
                default:
                    break;
            }

            // now fragmented
//            // stop if the client has already reached his max cp's
//            if(i->client->pers.clientCP[MAX_CP - 1].active)
//            {
//                G_LogPrintf( "^3Warning: ^7'%p^7' remove CP for overflow - called with '%p', '%s', '%s', '%d'\n", ent, message, find, mode );
//                return;
//            }

            p = NULL;

            // iterate over each cp and remove any finds.  The first find gets the position of
            for( j = i->client->pers.clientCP; j < i->client->pers.clientCP + MAX_CP; j++ )
            {
                if(j->active)
                {
                    if(!strcmp(j->message, buf) || (find && G_StrFind(j->message, find)))
                    {
                        j->active = 0;
                        if(!p)
                            p = j;
                    }
                }
            }

            if(!p)  // if nothing was replaced...
            {
                // ...then find the first empty spot
                for( j = i->client->pers.clientCP; j < i->client->pers.clientCP + MAX_CP; j++ )
                {
                    // iterate again
                    if(!j->active)
                    {
                        p = j;
                        break;
                    }
                }
            }

            if(!p)
            {
                G_LogPrintf( "^3Warning: no room for CP (max %d) - called with '%p', '%s', '%s', '%d'\n", MAX_CP, ent, message, find, mode );
                return;
            }

            Q_strncpyz(p->message, buf, sizeof(p->message));
            p->start = level.time;
            p->active = 1;
        }
    }
}

void G_ClientPrint( gentity_t *ent, char *message, int mode )
{
    gentity_t *i;
    qboolean target;

    if(ent && !ent->client)
        return;

    // iterate for each client
    for( i = &g_entities[ 0 ]; i < g_entities + level.maxclients; i++ )
    {
        // unnecessary sanity check
        if(!i->client)
        {
            G_LogPrintf( "Sanity check failed!\nan entity up to level.maxclients was not a client\n\n" );
            continue;  // this should never happen
        }
        if(i->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        // reset target boolean
        target = qfalse;

        // is this client one of the targets?
        if((i == ent) ||
           (!ent) ||
           (mode & CLIENT_SPECTATORS && i->client->sess.spectatorState == SPECTATOR_FOLLOW && i->client->sess.spectatorClient == ent - g_entities) ||
           (mode & CLIENT_OCTEAM && i->client->pers.ocTeam == ent->client->pers.ocTeam))
        {
            target = qtrue;
        }
        if(mode & CLIENT_ALLBUT)
        {
            target = !target;
        }
        if(mode & CLIENT_NOTARGET && i == ent)
        {
            target = qfalse;
        }
        if(mode & CLIENT_NOTEAM && i->client->pers.ocTeam)
        {
            target = qfalse;
        }
        if(mode & CLIENT_ONLYTEAM && !i->client->pers.ocTeam)
        {
            target = qfalse;
        }

        if(target)
        {
            buf[0] = 0;
            Com_sprintf(buf, sizeof(buf), "print \"%s\n\"", message);
            trap_SendServerCommand(i - g_entities, buf);
        }
    }
}

void G_ToLowerCase(char *str)
{
    if(!str)
        return;

    while(*str)
    {
        if(*str >= 'A' && *str <= 'Z')
        {
            *str -= 'A' - 'a';
        }

        str++;
    }
}
