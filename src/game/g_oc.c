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

/*
 * g_oc.c
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
 * several which are recommended: client-side speedometer, notarget buildables, cmd
 * stealth and weak aimbot detection, crash, connectMessage, CPMode, restart CP,
 * no auto-vote, LayoutLoad memory leak fix, speed, disableWeaponSounds, 
 * g_connectMessage, and 'x is building' message.
 */

#define OC_GAME

#include "g_local.h"

//======================================================
// admin
//======================================================

g_admin_hide_t *g_admin_hides[MAX_ADMIN_HIDES];

extern g_admin_level_t *g_admin_levels[MAX_ADMIN_LEVELS];
extern g_admin_admin_t *g_admin_admins[MAX_ADMIN_ADMINS];
extern g_admin_ban_t *g_admin_bans[MAX_ADMIN_BANS];
extern g_admin_command_t *g_admin_commands[MAX_ADMIN_COMMANDS];
extern g_admin_namelog_t *g_admin_namelog[MAX_ADMIN_NAMELOGS];

qboolean G_admin_canEditOC(void *ent)
{
	if(!BG_OC_OCMode())
		return qfalse;

	if(!ent)
		return qfalse;

	if(level.ocEditMode == 0)
		return qfalse;

	if(level.ocEditMode == 1 && !G_admin_permission(ent, '('))
		return qfalse;

	return qtrue;
}

qboolean G_admin_editoc(void *ent_v, int skiparg)
{
  gentity_t *ent = (gentity_t *)ent_v;
  gentity_t *client;
  char command[MAX_ADMIN_CMD_LEN];
  int i;
  G_SayArgv(skiparg+1, command, sizeof(command));
  if(BG_OC_OCMode())
  {
    if(!Q_stricmp(command, "0") || !Q_stricmp(command, "off"))
    {
      AP(va("print \"^3!editoc: ^7Admin cheating and oc editing ^5DISABLED^7 to ^2off^7 by ^7%s^7\n\"",
              (ent) ? ent->client->pers.netname : "console"));
      G_LogPrintf(va("print \"^3!editoc: ^7Admin cheating and oc editing ^5DISABLED^7 to ^2off^7 by ^7%s^7\n\"",
              (ent) ? ent->client->pers.netname : "console"));
      level.ocEditMode = 0;
      for(i = 0; i < level.maxclients; i++)
      {
        client = &g_entities[i];
        if(client->client)
          client->client->pers.noAuO = 0;
      }
    }
    else if(!Q_stricmp(command, "1") || !Q_stricmp(command, "allwithflag"))
    {
      AP(va("print \"^3!editoc: ^7Admin cheating and oc editing ^5ENABLED^7 to ^2allwithflag^7 by ^7%s^7\n\"",
              (ent) ? ent->client->pers.netname : "console"));
      G_LogPrintf(va("print \"^3!editoc: ^7Admin cheating and oc editing ^5ENABLED^7 to ^2allwithflag^7 by ^7%s^7\n\"",
              (ent) ? ent->client->pers.netname : "console"));
      level.ocEditMode = 1;
      for(i = 0; i < level.maxclients; i++)
      {
        client = &g_entities[i];
        if(client->client)
          client->client->pers.noAuO = 0;
      }
    }
    else if(!Q_stricmp(command, "2") || !Q_stricmp(command, "all"))
    {
      AP(va("print \"^3!editoc: ^7Admin cheating and oc editing ^5ENABLED^7 to ^1all^7 by ^7%s^7\n\"",
              (ent) ? ent->client->pers.netname : "console"));
      G_LogPrintf(va("print \"^3!editoc: ^7Admin cheating and oc editing ^5ENABLED^7 to ^1all^7 by ^7%s^7\n\"",
              (ent) ? ent->client->pers.netname : "console"));
      level.ocEditMode = 2;
      for(i = 0; i < level.maxclients; i++)
      {
        client = &g_entities[i];
        if(client->client)
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

qboolean G_admin_layoutsave(void *ent, int skiparg)
{
  char layout[MAX_QPATH];
  char command[MAX_ADMIN_CMD_LEN], *cmd;
  char output[MAX_STRING_CHARS];

  G_SayArgv(skiparg, command, sizeof(command));
  cmd = command;
  if(cmd && *cmd == '!')
    cmd++;

  if(G_SayArgc() < 2 + skiparg)
  {
    ADMP(va("^3!%s: ^7usage: !%s [layout]\n", cmd, cmd));
    return qfalse;
  }

  G_SayArgv(skiparg + 1, layout, sizeof(layout));
  G_StrToLower(layout);

  if(!(*layout == 'o' && *(layout + 1) == 'c') || !(g_ocReview.integer && Q_stricmp(cmd, "layoutsave")))
  {
    trap_SendConsoleCommand(EXEC_APPEND, va("layoutsave %s", layout));
    Q_strncpyz(output, va("layout saved as '%s'", layout), sizeof(output));
    EXCOLOR(output);
    AP(va("print \"^3!layoutsave: ^7%s^7 by ^7%s^7\n\"", output,
            (ent) ? ((gentity_t *) ent)->client->pers.netname : "console"));
  }
  else
  {
    trap_SendConsoleCommand(EXEC_APPEND, va("layoutsave %s_review", layout));
    Q_strncpyz(output, va("layout saved for review as '%s_review'", layout), sizeof(output));
    EXCOLOR(output);
    AP(va("print \"^3!layoutsave: ^7%s^7 by ^7%s^7\n\"", output,
            (ent) ? ((gentity_t *) ent)->client->pers.netname : "console"));
  }
  return qtrue;
}

qboolean G_admin_devmap(void *ent, int skiparg)
{
  char map[MAX_QPATH];
  char layout[MAX_QPATH] = { "" };

  if(G_SayArgc() < 2 + skiparg)
  {
    ADMP("^3!devmap: ^7usage: !devmap [map] (layout)\n");
    return qfalse;
  }

  G_SayArgv(skiparg + 1, map, sizeof(map));

  if(!trap_FS_FOpenFile(va("maps/%s.bsp", map), NULL, FS_READ))
  {
    ADMP(va("^3!devmap: ^7invalid map name '%s'\n", map));
    return qfalse;
  }

  if(G_SayArgc() > 2 + skiparg)
  {
    G_SayArgv(skiparg + 2, layout, sizeof(layout));
    if(!Q_stricmp(layout, "*BUILTIN*") ||
      trap_FS_FOpenFile(va("layouts/%s/%s.dat", map, layout),
        NULL, FS_READ) > 0)
    {
      trap_Cvar_Set("g_layouts", layout);
    }
    else
    {
      ADMP(va("^3!devmap: ^7invalid layout name '%s'\n", layout));
      return qfalse;
    }
  }

  trap_SendConsoleCommand(EXEC_APPEND, va("devmap %s", map));
  level.restarted = qtrue;
  AP(va("print \"^3!devmap: ^7map '%s' started by %s^7 with cheats %s\n\"", map,
          (ent) ? ((gentity_t *) ent)->client->pers.netname : "console",
          (layout[0]) ? va("(forcing layout '%s')", layout) : ""));
  return qtrue;
}

static qboolean admin_create_hide(gentity_t *ent, char *netname, char *guid, char *ip, int seconds, char *reason, int hidden)
{
  g_admin_hide_t *h = NULL;
  qtime_t qt;
  int t;
  int i;
  int j;
  qboolean foundAdminTrueName=qfalse;

  t = trap_RealTime(&qt);
  h = BG_Alloc(sizeof(g_admin_hide_t));

  if(!h)
    return qfalse;

  Q_strncpyz(h->name, netname, sizeof(h->name));
  Q_strncpyz(h->guid, guid, sizeof(h->guid));
  Q_strncpyz(h->ip, ip, sizeof(h->ip));

  //strftime(h->made, sizeof(b->made), "%m/%d/%y %H:%M:%S", lt);
  Q_strncpyz(h->made, va("%02i/%02i/%02i %02i:%02i:%02i",
    (qt.tm_mon + 1), qt.tm_mday, (qt.tm_year - 100),
    qt.tm_hour, qt.tm_min, qt.tm_sec),
    sizeof(h->made));

  if(ent) {
    //Get admin true name
    for(j = 0; j < MAX_ADMIN_ADMINS && g_admin_admins[j]; j++)
    {
      if(!Q_stricmp(g_admin_admins[j]->guid, ent->client->pers.guid))
      {
          Q_strncpyz(h->hider, g_admin_admins[j]->name, sizeof(h->hider));
      foundAdminTrueName=qtrue;
        break;
      }
    }
    if(foundAdminTrueName==qfalse) Q_strncpyz(h->hider, ent->client->pers.netname, sizeof(h->hider));
  }
  else
    Q_strncpyz(h->hider, "console", sizeof(h->hider));
  if(!seconds)
    h->expires = 0;
  else
    h->expires = t + seconds;
  if(!*reason)
    Q_strncpyz(h->reason, "hidden by admin", sizeof(h->reason));
  else
    Q_strncpyz(h->reason, reason, sizeof(h->reason));
  if(!hidden)
    h->hidden = 0;
  else
    h->hidden = hidden;
  for(i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[i]; i++)
    ;
  if(i == MAX_ADMIN_HIDES)
  {
    ADMP("^3!hides: ^7too many hides\n");
    BG_Free(h);
    return qfalse;
  }
  g_admin_hides[i] = h;
  return qtrue;
}

qboolean G_admin_hide_check(char *userinfo, char *reason, int rlen, int *hidden, int *hiddenTime, int *id)
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
  t = trap_RealTime(NULL);
  if(!*userinfo)
    return qfalse;
  ip = Info_ValueForKey(userinfo, "ip");
  if(!*ip)
    return qfalse;
  guid = Info_ValueForKey(userinfo, "cl_guid");
  for(i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[i]; i++)
  {
    // 0 is for perm hide
    if(g_admin_hides[i]->expires != 0 &&
         (g_admin_hides[i]->expires - t) < 1)
      continue;
    if(reason)
        Com_sprintf(reason, rlen, "%s", g_admin_hides[i]->reason);
    if(hidden)
        *hidden = g_admin_hides[i]->hidden;
    if(hiddenTime)
    {
        if(!g_admin_hides[i]->expires)
        {
            *hiddenTime = level.time + INFINITE;
            while(*hiddenTime < level.time - 10)
                (*hiddenTime)--;
        }
        else
        {
            *hiddenTime = level.time + (g_admin_hides[i]->expires - t) * 1000;
            while(*hiddenTime < level.time - 10)
                (*hiddenTime)--;
        }
    }
    if(id)
        *id = i;
    
    if(strstr(ip, g_admin_hides[i]->ip))
    {
      return qtrue;
    }
    if(*guid && !Q_stricmp(g_admin_hides[i]->guid, guid))
    {
      return qtrue;
    }
  }
  return qfalse;
}

qboolean G_admin_hide(void *ent, int skiparg)
{
  int seconds;
  char secs[7];
  char duration[32];
  int pids[MAX_CLIENTS], found;
  char name[MAX_NAME_LENGTH], *reason, err[MAX_STRING_CHARS];
  char command[MAX_ADMIN_CMD_LEN], *cmd;
  gentity_t *vic;

  if(!BG_OC_OCMode())
  {
    ADMP("^3!hide: ^7can only be used during an obstacle course\n");
    return qfalse;
  }
//  // admins can still hide
//  if(!g_allowHiding.integer)
//  {
//    ADMP("^3!hide: ^7hiding has been disabled\n");
//    return qfalse;
//  }

  if(G_SayArgc() < 2 + skiparg)
  {
    ADMP("^3!hide: ^7usage: !hide [name|slot#] (reason)\n");
    return qfalse;
  }
  G_SayArgv(skiparg, command, sizeof(command));
  cmd = command;
  if(cmd && *cmd == '!')
    cmd++;
  G_SayArgv(1 + skiparg, name, sizeof(name));
  reason = G_SayConcatArgs(2 + skiparg);
  if((found = G_ClientNumbersFromString(name, pids, MAX_CLIENTS)) != 1)
  {
    G_MatchOnePlayer(pids, found, err, sizeof(err));
    ADMP(va("^3!hide: ^7%s\n", err));
    return qfalse;
  }
  if(!admin_higher(ent, &g_entities[pids[0]]))
  {
    ADMP("^3!hide: ^7sorry, but your intended victim has a higher admin"
        " level than you\n");
    return qfalse;
  }
  vic = &g_entities[pids[0]];
  if(Q_stricmp(cmd, "hide"))
  {
    int id, t;
    char userinfo[MAX_INFO_STRING];
    // remove hide
    trap_GetUserinfo(vic - g_entities, userinfo, sizeof(userinfo));
    if(G_admin_hide_check(userinfo, NULL, 0, NULL, NULL, &id))
    {
        t = trap_RealTime(NULL);
        g_admin_hides[id]->expires = t;
        if(g_admin.string[0])
            admin_writeconfig();
    }
    vic->r.svFlags &= ~SVF_SINGLECLIENT;
    vic->client->pers.hidden = qfalse;
    if(G_SayArgc() == 2 + skiparg)
    {
        AP(va("print \"^3!unhide: ^7%s^7 has been unhidden by ^7%s\n\"",
                vic->client->pers.netname,
                (ent) ? ((gentity_t *) ent)->client->pers.netname : "console"));
    }
    else
    {
        G_SayArgv(2 + skiparg, secs, sizeof(secs));
        seconds = G_admin_parse_time(secs);
        G_admin_duration(seconds, duration, sizeof(duration));
        admin_create_hide(ent, vic->client->pers.netname, vic->client->pers.guid, vic->client->pers.ip, seconds, (*reason) ? reason : "unhidden by admin", 0);
        admin_writeconfig();

        vic->client->pers.hiddenTime = level.time + seconds * 1000;
        AP(va("print \"^3!unhide: ^7%s^7 has been unhidden by ^7%s; force duration: ^7%s^7\n\"",
                vic->client->pers.netname,
                (ent) ? ((gentity_t *) ent)->client->pers.netname : "console", duration));
    }
  }
  else
  {
    int id, t;
    char userinfo[MAX_INFO_STRING];
    // remove hide
    trap_GetUserinfo(vic - g_entities, userinfo, sizeof(userinfo));
    if(G_admin_hide_check(userinfo, NULL, 0, NULL, NULL, &id))
    {
        t = trap_RealTime(NULL);
        g_admin_hides[id]->expires = t;
        if(g_admin.string[0])
            admin_writeconfig();
    }
    vic->client->pers.hidden = qtrue;
    G_StopFromFollowing(vic, 0);
    vic->r.svFlags |= SVF_SINGLECLIENT;
    vic->r.singleClient = vic-g_entities;
    CPx(pids[0], "cp \"^1You've been hidden\"");
    if(G_SayArgc() == 2 + skiparg)
    {
        AP(va("print \"^3!hide: ^7%s^7 has been hidden by ^7%s\n\"",
                vic->client->pers.netname,
                (ent) ? ((gentity_t *) ent)->client->pers.netname : "console"));
    }
    else
    {
        G_SayArgv(2 + skiparg, secs, sizeof(secs));
        seconds = G_admin_parse_time(secs);
        G_admin_duration(seconds, duration, sizeof(duration));
        admin_create_hide(ent, vic->client->pers.netname, vic->client->pers.guid, vic->client->pers.ip, seconds, (*reason) ? reason : "hidden by admin", 1);
        admin_writeconfig();

        vic->client->pers.hiddenTime = level.time + seconds * 1000;
        AP(va("print \"^3!hide: ^7%s^7 has been hidden by ^7%s; force duration: ^7%s^7\n\"",
                vic->client->pers.netname,
                (ent) ? ((gentity_t *) ent)->client->pers.netname : "console", duration));
    }
  }

  ClientUserinfoChanged(pids[0]);
  return qtrue;
}

qboolean G_admin_showhides(void *ent, int skiparg)
{
  int i, found = 0;
  int max = -1, count;
  int t;
  char duration[32];
  int max_name = 1, max_hider = 1, colorlen;
  int len;
  int secs;
  int start = 0;
  char filter[MAX_NAME_LENGTH] = {""};
  char date[11];
  char *made;
  int j, k;
  char n1[MAX_NAME_LENGTH * 2] = {""};
  char n2[MAX_NAME_LENGTH * 2] = {""};
  qboolean numeric = qtrue;
  char *ip_match = NULL;
  int ip_match_len = 0;
  char name_match[MAX_NAME_LENGTH] = {""};

  t = trap_RealTime(NULL);

  for(i = 0; i < MAX_ADMIN_HIDES && g_admin_hides[i]; i++)
  {
    if(g_admin_hides[i]->expires != 0 &&
        (g_admin_hides[i]->expires - t) < 1)
    {
      continue;
    }
    found++;
    max = i;
  }

  if(max < 0)
  {
    ADMP("^3!showhides: ^7no hides to display\n");
    return qfalse;
  }

  if(G_SayArgc() >= 2 + skiparg)
  {
    G_SayArgv(1 + skiparg, filter, sizeof(filter));
    if(G_SayArgc() >= 3 + skiparg)
    {
      start = atoi(filter);
      G_SayArgv(2 + skiparg, filter, sizeof(filter));
    }
    for(i = 0; i < sizeof(filter) && filter[i] ; i++)
    {
      if(!isdigit(filter[i]) &&
          filter[i] != '.' && filter[i] != '-')
      {
        numeric = qfalse;
        break;
      }
    }
    if(!numeric)
    {
      G_SanitiseString(filter, name_match, sizeof(name_match));
    }
    else if(strchr(filter, '.'))
    {
      ip_match = filter;
      ip_match_len = strlen(ip_match);
    }
    else
    {
      start = atoi(filter);
      filter[0] = '\0';
    }
    // showhides 1 means start with hide 0
    if(start > 0)
      start--;
    else if(start < 0)
    {
      for(i = max, count = 0; i >= 0 && count < -start; i--)
        if(g_admin_hides[i]->expires == 0 ||
          (g_admin_hides[i]->expires - t) > 0)
          count++;
      start = i + 1;
    }
  }

  if(start < 0)
    start = 0;

  if(start > max)
  {
    ADMP(va("^3!showhides: ^7%d is the last valid hides\n", max + 1));
    return qfalse;
  }

  for(i = start, count = 0; i <= max && count < MAX_ADMIN_SHOWHIDES; i++)
  {
    if(g_admin_hides[i]->expires != 0 &&
      (g_admin_hides[i]->expires - t) < 1)
      continue;

    if(name_match[0])
    {
      G_SanitiseString(g_admin_hides[i]->name, n1, sizeof(n1));
      if(!strstr(n1, name_match))
        continue;
    }
    if(ip_match &&
      Q_strncmp(ip_match, g_admin_hides[i]->ip, ip_match_len))
        continue;

    count++;

    len = Q_PrintStrlen(g_admin_hides[i]->name);
    if(len > max_name)
      max_name = len;
    len = Q_PrintStrlen(g_admin_hides[i]->hider);
    if(len > max_hider)
      max_hider = len;
  }

  ADMBP_begin();
  for(i = start, count = 0; i <= max && count < MAX_ADMIN_SHOWHIDES; i++)
  {
    if(g_admin_hides[i]->expires != 0 &&
      (g_admin_hides[i]->expires - t) < 1)
      continue;

    if(name_match[0])
    {
      G_SanitiseString(g_admin_hides[i]->name, n1, sizeof(n1));
      if(!strstr(n1, name_match))
        continue;
    }
    if(ip_match &&
      Q_strncmp(ip_match, g_admin_hides[i]->ip, ip_match_len))
        continue;

    count++;

    // only print out the the date part of made
    date[0] = '\0';
    made = g_admin_hides[i]->made;
    for(j = 0; made && *made; j++)
    {
      if((j + 1) >= sizeof(date))
        break;
      if(*made == ' ')
        break;
      date[j] = *made;
      date[j + 1] = '\0';
      made++;
    }

    secs = (g_admin_hides[i]->expires - t);
    G_admin_duration(secs, duration, sizeof(duration));

    for(colorlen = k = 0; g_admin_hides[i]->name[k]; k++)
      if(Q_IsColorString(&g_admin_hides[i]->name[k]))
        colorlen += 2;
    Com_sprintf(n1, sizeof(n1), "%*s", max_name + colorlen,
                 g_admin_hides[i]->name);

    for(colorlen = k = 0; g_admin_hides[i]->hider[k]; k++)
      if(Q_IsColorString(&g_admin_hides[i]->hider[k]))
        colorlen += 2;
    Com_sprintf(n2, sizeof(n2), "%*s", max_hider + colorlen,
                 g_admin_hides[i]->hider);

    ADMBP(va("%4i %s^7 %-15s %-8s %s^7 %-10s %-12s\n     \\__ %s\n",
             (i + 1),
             n1,
             g_admin_hides[i]->ip,
             date,
             n2,
             duration,
			 g_admin_hides[i]->hidden ? ("hidden") : ("unhidden"),
             g_admin_hides[i]->reason));
  }

  if(name_match[0] || ip_match)
  {
    ADMBP(va("^3!showhides:^7 found %d matching hides by %s.  ",
             count,
             (ip_match) ? "IP" : "name"));
  }
  else
  {
    ADMBP(va("^3!showhides:^7 showing hides %d - %d of %d (%d total).",
             (found) ? (start + 1) : 0,
             i,
             max + 1,
             found));
  }

  if(i <= max)
    ADMBP(va("  run !showhides %d%s%s to see more",
             i + 1,
             (filter[0]) ? " " : "",
             (filter[0]) ? filter : ""));
  ADMBP("\n");
  ADMBP_end();
  return qtrue;
}

qboolean G_admin_adjusthide(void *ent, int skiparg)
{
  int hnum;
  int length;
  int hidden = 1;
  int expires;
  char duration[32] = {""};
  char *reason = NULL;
  char hs[5];
  char secs[7];
  char hiddenC[7];

  if(G_SayArgc() < 3 + skiparg)
  {
    ADMP("^3!adjusthide: ^7usage: !adjusthide [hide#] [time] [(c) hidden] [reason]\n");
    return qfalse;
  }
  G_SayArgv(1 + skiparg, hs, sizeof(hs));
  hnum = atoi(hs);
  if(hnum < 1 || hnum > MAX_ADMIN_HIDES || !g_admin_hides[hnum - 1])
  {
    ADMP("^3!adjusthide: ^7invalid hide#\n");
    return qfalse;
  }

  G_SayArgv(2 + skiparg, secs, sizeof(secs));
  G_SayArgv(3 + skiparg, hiddenC, sizeof(hiddenC));
  length = G_admin_parse_time(secs);
  if(length < 0 && ((secs[0] == 'c' || secs[0] == 'C' || secs[0] == 'h' || secs[0] == 'H') && ((secs[1] == ' ' || secs[1] == '\t' || secs[1] == '-') || (secs[1] >= '0' && secs[1] <= '9') || (hiddenC[0] && hiddenC[0] >= '0' && hiddenC[0] <= '9'))) && G_SayArgc() > 3 + skiparg)
  {
    G_SayArgv(3 + skiparg, secs, sizeof(secs));
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
    g_admin_hides[hnum - 1]->hidden = hidden;
  }
  else if(length < 0)
    reason = G_SayConcatArgs(2 + skiparg);
  else
  {
    if(length != 0)
      expires = trap_RealTime(NULL) + length;
    else if(length >= 0)
      expires = 0;
    else
    {
      ADMP("^3!adjusthide: ^7hide time must be positive\n");
      return qfalse;
    }

    g_admin_hides[hnum - 1]->expires = expires;
    G_admin_duration((length) ? length : -1, duration, sizeof(duration));
    reason = G_SayConcatArgs(3 + skiparg);
  }
  if(reason && *reason)
    Q_strncpyz(g_admin_hides[hnum - 1]->reason, reason,
      sizeof(g_admin_hides[hnum - 1]->reason));
  AP(va("print \"^3!adjusthide: ^7hide #%d for %s^7 has been updated by %s^7 "
    "%s%s%s%s%s%s\n\"",
    hnum,
    g_admin_hides[hnum - 1]->name,
    (ent) ? ((gentity_t *) ent)->client->pers.netname : "console",
    (length < 0 && ((secs[0] == 'c' || secs[0] == 'C' || secs[0] == 'h' || secs[0] == 'H') && ((secs[1] == ' ' || secs[1] == '\t' || secs[1] == '-') || (secs[1] >= '0' && secs[1] <= '9') || (hiddenC[0] && hiddenC[0] >= '0' && hiddenC[0] <= '9'))) && G_SayArgc() > 3 + skiparg) ? (hidden ? "hidden: hidden" : "hidden: unhidden") : "",
    (length >= 0) ? "duration: " : "",
    duration,
    (length >= 0 && reason && *reason) ? ", " : "",
    (reason && *reason) ? "reason: " : "",
    reason ? reason : ""));
  if(ent)
    Q_strncpyz(g_admin_hides[hnum - 1]->hider, ((gentity_t *) ent)->client->pers.netname,
      sizeof(g_admin_hides[hnum - 1]->hider));
  if(g_admin.string[0])
    admin_writeconfig();
  return qtrue;
}

//======================================================
// cvars
//======================================================

void G_OC_PreRegisterCvars(void)
{
}

void G_OC_RegisterCvars(void)
{
}

//======================================================
// oc stats, ratings and layouts
//======================================================

typedef struct
{
	char mapname[64];
	char layoutname[64];
	char rating[32];
} ratings_table_t;

typedef struct
{
	int buildable;
	vec3_t origin;
	vec3_t angles;
	vec3_t origin2;
	vec3_t angles2;
	int groupID;
	int reserved;
	float reserved2;
} layout_table_t;

static ratings_table_t *ratings_table = NULL;

static int G_OC_CompareLayoutBuildables(layout_table_t *a, layout_table_t *b)
{
	return a->origin[2] - b->origin[2];
}

void G_OC_LayoutLoad(char *layout)
{
	fileHandle_t f;
	int len;
	char *layoutPtr;
	char map[MAX_QPATH];
	int buildable = BA_NONE;
	vec3_t origin = { 0.0f, 0.0f, 0.0f };
	vec3_t angles = { 0.0f, 0.0f, 0.0f };
	vec3_t origin2 = { 0.0f, 0.0f, 0.0f };
	vec3_t angles2 = { 0.0f, 0.0f, 0.0f };
	int groupID = 0;
	int reserved = 0;
	float reserved2 = 0.0f;
	char line[MAX_STRING_CHARS];
	int i = 0, j = 0;
	layout_table_t *l;
	static layout_table_t layoutTable[MAX_GENTITIES];

	if(!BG_OC_OCMode())
		return;

	if(!level.layout[0] || !Q_stricmp(level.layout, "*BUILTIN*"))
		return;

	l = layoutTable;

	if(!layout)
	{
	  trap_Cvar_VariableStringBuffer("mapname", map, sizeof(map));
	  len = trap_FS_FOpenFile(va("layouts/%s/%s.dat", map, level.layout),
		&f, FS_READ);
	  if(len < 0)
	  {
		G_Printf("ERROR: layout %s could not be opened\n", level.layout);
		return;
	  }
	  layout = BG_Alloc(len + 1);
	  trap_FS_Read(layout, len, f);
	  *(layout + len) = '\0';
	  trap_FS_FCloseFile(f);
	}
	layoutPtr = layout;

	while(*layout)
	{
		if(i >= sizeof(line) - 1)
		{
		  G_Printf(S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
		   va("layouts/%s/%s.dat", map, level.layout), line);
		  return;
		}
		line[i++] = *layout;
		line[i] = '\0';
		if(*layout == '\n')
		{
		  i = 0;
		  sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f %f %f %d %d %f\n",
			&buildable,
			&origin[0], &origin[1], &origin[2],
			&angles[0], &angles[1], &angles[2],
			&origin2[0], &origin2[1], &origin2[2],
			&angles2[0], &angles2[1], &angles2[2],
			&groupID, &reserved, &reserved2);

		  if(buildable > BA_NONE && buildable < BA_NUM_BUILDABLES)
		  {
			l->buildable = buildable;
			VectorCopy(origin, l->origin);
			VectorCopy(angles, l->angles);
			VectorCopy(origin2, l->origin2);
			VectorCopy(angles2, l->angles2);
			l->groupID = groupID;
			l->reserved = reserved;
			l->reserved2 = reserved2;
			l++;
			if(++j >= sizeof(layoutTable) / sizeof(layoutTable[0]))
			{
				G_ClientPrint(NULL, va("^1Error: ^7Too many buildables (%d)!", sizeof(layoutTable) / sizeof(layoutTable[0])), CLIENT_NULL);
				G_ClientCP(NULL, va("^1Error: ^7Too many buildables (%d)!", sizeof(layoutTable) / sizeof(layoutTable[0])), NULL, CLIENT_NULL);
				G_LogPrintf("^1Error: ^7Too many buildables (%d)!\n", sizeof(layoutTable) / sizeof(layoutTable[0]));
				return;
			}
		  }
		  else if(!(buildable > BA_NONE && buildable < BA_NUM_BUILDABLES))
		  {
			G_Printf(S_COLOR_YELLOW "WARNING: bad buildable number (%d) in "
			  " layout.  skipping\n", buildable);
		  }
		}
		layout++;
	}

	BG_Free(layoutPtr);

	// build each buildable after ordering by position of Z axis
	qsort(layoutTable, j, sizeof(layoutTable[0]), (int(*)())G_OC_CompareLayoutBuildables);
	for(i = 0; i < j; i++)
	{
		l = &layoutTable[i];
		G_LayoutBuildItem(l->buildable, l->origin, l->angles, l->origin2, l->angles2, l->groupID, l->reserved, l->reserved2);
	}

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_entities[i].client && level.clients[i].pers.connected != CON_CONNECTED)
		{
			if(level.clients[i].pers.medis)
				BG_Free(level.clients[i].pers.medis);
			if(level.totalMedistations)
				level.clients[i].pers.medis = BG_Alloc((level.totalMedistations) * sizeof(gentity_t *));
			else
				level.clients[i].pers.medis = NULL;
			if(level.clients[i].pers.medisLastCheckpoint)
				BG_Free(level.clients[i].pers.medisLastCheckpoint);
			if(level.totalMedistations)
				level.clients[i].pers.medisLastCheckpoint = BG_Alloc((level.totalMedistations) * sizeof(gentity_t *));
			else
				level.clients[i].pers.medisLastCheckpoint = NULL;
			if(level.clients[i].pers.arms)
				BG_Free(level.clients[i].pers.arms);
			if(level.totalArmouries)
				level.clients[i].pers.arms = BG_Alloc((level.totalArmouries) * sizeof(gentity_t *));
			else
				level.clients[i].pers.arms = NULL;
			if(level.clients[i].pers.armsLastCheckpoint)
				BG_Free(level.clients[i].pers.armsLastCheckpoint);
			if(level.totalArmouries)
				level.clients[i].pers.armsLastCheckpoint = BG_Alloc((level.totalArmouries) * sizeof(gentity_t *));
			else
				level.clients[i].pers.armsLastCheckpoint = NULL;
		}
	}

	G_CountSpawns();
	G_CalculateBuildPoints();
	G_CalculateStages();
}

void G_OC_LoadRatings(void)
{
	fileHandle_t f;
	int len;
	int i = 0;
	char line[MAX_STRING_CHARS], *l, *l2, tmp;
	char mapname[MAX_STRING_CHARS], layoutname[MAX_STRING_CHARS], rating[MAX_STRING_CHARS];
	char *ratings;
	ratings_table_t *r;

	len = trap_FS_FOpenFile("info/info-ratings.dat", &f, FS_READ);
	if(len < 0)
	{
		G_Printf("WARNING: info/info-ratings.dat could not be opened\n");
		return;
	}

	ratings = BG_Alloc(len + 1);
	trap_FS_Read(ratings, len, f);
	*(ratings + len) = '\0';
	trap_FS_FCloseFile(f);

	while(*ratings)
	{
		if(i >= sizeof(line) - 1)
		{
			G_Printf(S_COLOR_RED "ERROR: line overflow in info/info-ratings.dat before \"%s\"\n", line);
			return;
		}
		line[i++] = *ratings;
		line[i] = '\0';
		if(*ratings == '\n')
		{
			i = 0;

			// first extract mapname
			l = line;
			while(*l != ' ' && *l != '\t' && *l != '\n')
			{
				if(!*l)
				{
					G_Printf(S_COLOR_RED "ERROR: malformed line in info/info-ratings.dat before \"%s\"\n", line);
					return;
				}

				l++;
			}

			tmp = *l;
			*l = 0;
			Q_strncpyz(mapname, line, sizeof(mapname));
			*l = tmp;

			// next extract layoutname
			//set l2 to the first non-whitespace character and l the first
			//first whitespace character
			l2 = l;
			while(*l2 == ' ' || *l == '\t' || *l2 == '\n')
			{
				if(!*l2)
				{
					G_Printf(S_COLOR_RED "ERROR: malformed line in info/info-ratings.dat before \"%s\"\n", line);
					return;
				}

				l2++;
			}

			l = l2;
			while(*l != ' ' && *l != '\t' && *l != '\n')
			{
				if(!*l)
				{
					G_Printf(S_COLOR_RED "ERROR: malformed line in info/info-ratings.dat before \"%s\"\n", line);
					return;
				}

				l++;
			}

			tmp = *l;
			*l = 0;
			Q_strncpyz(layoutname, l2, sizeof(layoutname));
			*l = tmp;

			// next extract rating
			//set l2 to the first non-whitespace character and l the first
			//first whitespace character
			l2 = l;
			while(*l2 == ' ' || *l == '\t' || *l2 == '\n')
			{
				if(!*l2)
				{
					G_Printf(S_COLOR_RED "ERROR: malformed line in info/info-ratings.dat before \"%s\"\n", line);
					return;
				}

				l2++;
			}

			l = l2;
//            while(*l != ' ' && *l != '\t' && *l != '\n')
			while(*l != '\n')
			{
//                if(!*l)
//                {
//                    G_Printf(S_COLOR_RED "ERROR: malformed line in info/info-ratings.dat before \"%s\"\n", line);
//                    return;
//                }

				l++;
			}

			tmp = *l;
			*l = 0;
			Q_strncpyz(rating, l2, sizeof(rating));
			*l = tmp;

			if(!ratings_table)
			{
				// allocate room for the table
				ratings_table = BG_Alloc(G_OC_MAX_LAYOUT_RATINGS * sizeof(ratings_table_t));
			}

			for(r = ratings_table; r < ratings_table + G_OC_MAX_LAYOUT_RATINGS; r++)
			{
				if(!*((const char *)r))
				{
					Q_strncpyz(r->mapname, mapname, sizeof(r->mapname));
					Q_strncpyz(r->layoutname, layoutname, sizeof(r->layoutname));
					Q_strncpyz(r->rating, rating, sizeof(r->rating));
					break;
				}
			}

			if(r >= ratings_table + G_OC_MAX_LAYOUT_RATINGS)
			{
				G_Printf(S_COLOR_RED "ERROR: too many ratings to hold (%d) on line \"%s\"", G_OC_MAX_LAYOUT_RATINGS, line);
				return;
			}
		}

		ratings++;
	}
}

char *G_OC_Rating(char *mapname, char *layoutname)
{
	ratings_table_t *r;

	if(!ratings_table)
		G_OC_LoadRatings();

	if(!ratings_table)
		return NULL;

	if(!layoutname)
		layoutname = level.layout;

	if(mapname)
	{
		for(r = ratings_table; r < ratings_table + G_OC_MAX_LAYOUT_RATINGS; r++)
		{
			if(!*((const char *)r))
			{
				return NULL;
			}

			if(strcmp(mapname, r->mapname) == 0 && strcmp(layoutname, r->layoutname) == 0)
			{
				return r->rating;
			}
		}
	}

	return NULL;
}

//======================================================
// game
//======================================================

void G_OC_RestartClient(gentity_t *ent, int quick, int resetScrimTeam)
{
	if(!ent)
	{
		return;
	}

	if(!ent->client)
	{
		return;
	}

	if(!BG_OC_OCMode())
	{
		return;
	}

	if(resetScrimTeam)
		ent->client->pers.scrimTeam = 0;

	if(quick)
	{
		// this is really crappy

		int i;
		gentity_t *dest;
		vec3_t spawn_origin, spawn_angles, infestOrigin;

		if(ent->client->pers.teamSelection == TEAM_HUMANS)
		{
		  for(i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++)
		  {
			if(i != WP_MACHINEGUN && i != WP_BLASTER && i != WP_NONE)
			{
				if(ent->client->ps.stats[STAT_WEAPON] != i)
				{
					ent->client->ps.stats[STAT_WEAPON] = WP_NONE;
      				G_ForceWeaponChange(ent, WP_NONE);
				}
			}
		  }
		  if(ent->client->ps.stats[STAT_WEAPON] != WP_MACHINEGUN)
		  {
			ent->client->ps.stats[STAT_WEAPON] = WP_MACHINEGUN;
			G_ForceWeaponChange(ent, WP_MACHINEGUN);
		  }
		  if(BG_InventoryContainsUpgrade(UP_LIGHTARMOUR, ent->client->ps.stats))
			BG_RemoveUpgradeFromInventory(UP_LIGHTARMOUR, ent->client->ps.stats);
		  if(BG_InventoryContainsUpgrade(UP_HELMET, ent->client->ps.stats))
			BG_RemoveUpgradeFromInventory(UP_HELMET, ent->client->ps.stats);
		  if(BG_InventoryContainsUpgrade(UP_BATTPACK, ent->client->ps.stats))
			BG_RemoveUpgradeFromInventory(UP_BATTPACK, ent->client->ps.stats);
		  if(BG_InventoryContainsUpgrade(UP_JETPACK, ent->client->ps.stats))
			BG_RemoveUpgradeFromInventory(UP_JETPACK, ent->client->ps.stats);
		  if(BG_InventoryContainsUpgrade(UP_BATTLESUIT, ent->client->ps.stats))
			BG_RemoveUpgradeFromInventory(UP_BATTLESUIT, ent->client->ps.stats);
		  if(BG_InventoryContainsUpgrade(UP_GRENADE, ent->client->ps.stats))
			BG_RemoveUpgradeFromInventory(UP_GRENADE, ent->client->ps.stats);
		  if(!BG_InventoryContainsUpgrade(UP_MEDKIT, ent->client->ps.stats))
			BG_AddUpgradeToInventory(UP_MEDKIT, ent->client->ps.stats);
		}
		else if(ent->client->pers.teamSelection == TEAM_ALIENS)
		{
		  ent->client->pers.evolveHealthFraction = (float)ent->client->ps.stats[STAT_MAX_HEALTH] /
			(float) BG_Class(ent->client->pers.classSelection)->health;

		  if(ent->client->pers.evolveHealthFraction < 0.0f)
			ent->client->pers.evolveHealthFraction = 0.0f;
		  else if(ent->client->pers.evolveHealthFraction > 1.0f)
			ent->client->pers.evolveHealthFraction = 1.0f;

		  //remove credit
		//          G_AddCreditToClient(ent->client, -(short)numLevels, qtrue);
		  ent->client->pers.classSelection = PCL_ALIEN_LEVEL0;
		  ClientUserinfoChanged(ent - g_entities);
		  if(!G_RoomForClassChange(ent, PCL_ALIEN_LEVEL0, infestOrigin))
			G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
		  VectorCopy(infestOrigin, ent->s.pos.trBase);
		  ClientSpawn(ent, ent, ent->s.pos.trBase, ent->s.apos.trBase);
		  G_AddCreditToClient(ent->client, ALIEN_MAX_FRAGS, qtrue);
		  if(!G_admin_canEditOC(ent))
		  {
			ent->client->pers.needEvolve = 1;
			ent->client->pers.evolveTime = level.time + G_OC_EVOLVEBLOCK_TIME;  // start class thing needs improvement
		  }
		}
		else
		{
			G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
		}
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];



		ent->client->pers.nextCheckpointTime = level.time + G_OC_RESTARTOC_TIME;
		ent->client->lastCreepSlowTime = 0;
		G_OC_ClearMedis(ent->client->pers.medis);
		G_OC_ClearMedis(ent->client->pers.medisLastCheckpoint);
		G_OC_ClearArms(ent->client->pers.arms);
		G_OC_ClearArms(ent->client->pers.armsLastCheckpoint);
//        memset(ent->client->pers.medis, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.medisLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.arms, 0, sizeof(gentity_t *) * (level.totalArmouries+1));
//        memset(ent->client->pers.armsLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalArmouries+1));

		ent->client->pers.checkpoint = NULL;
		if(ent->client->pers.teamSelection == TEAM_HUMANS)
		{
			if((dest = G_SelectHumanSpawnPoint(ent->s.origin, ent, 0, NULL)))
			{
			  VectorCopy(dest->s.origin, spawn_origin);
			  if(!ent->client->pers.autoAngleDisabled)
			  {
				VectorCopy(dest->s.angles, spawn_angles);
				VectorInverse(spawn_angles);
				spawn_angles[ROLL] = 0;
			  }
			  else
			  {
				VectorCopy(ent->s.angles, spawn_angles);
				spawn_angles[ROLL] = 0;
			  }
			  if(G_CheckSpawnPoint(dest->s.number, dest->s.origin, dest->s.origin2, BA_H_SPAWN, spawn_origin) == NULL)
			  {
			//        TeleportPlayer(ent, spawn_origin, spawn_angles);
				  VectorCopy(spawn_origin, ent->client->ps.origin);
				  ent->client->ps.origin[2] += 1;

				  // toggle the teleport bit so the client knows to not lerp
				  ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
				  G_UnlaggedClear(ent);

			//          // set angles
				  if(!ent || !ent->client->pers.autoAngleDisabled)
					SetClientViewAngle(ent, spawn_angles);
				  // save results of pmove
				  BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);

				  // use the precise origin for linking
				  VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
			  }
			  else
			  {
				G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
			  }
			}
			else
			{
			  G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
			}
		}
		else if(ent->client->pers.teamSelection == TEAM_ALIENS)
		{
			if((dest = G_SelectAlienSpawnPoint(ent->s.origin, ent, 0, NULL)))
			{
			  VectorCopy(dest->s.origin, spawn_origin);
			  if(!ent->client->pers.autoAngleDisabled)
			  {
				VectorCopy(dest->s.angles, spawn_angles);
				VectorInverse(spawn_angles);
				spawn_angles[ROLL] = 0;
			  }
			  else
			  {
				VectorCopy(ent->s.angles, spawn_angles);
				spawn_angles[ROLL] = 0;
			  }
			  if(G_CheckSpawnPoint(dest->s.number, dest->s.origin, dest->s.origin2, BA_A_SPAWN, spawn_origin) == NULL)
			  {
			//        TeleportPlayer(ent, spawn_origin, spawn_angles);
				  VectorCopy(spawn_origin, ent->client->ps.origin);
				  ent->client->ps.origin[2] += 1;

				  // toggle the teleport bit so the client knows to not lerp
				  ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
				  G_UnlaggedClear(ent);

				  // set angles
				  if(!ent || !ent->client->pers.autoAngleDisabled)
					SetClientViewAngle(ent, spawn_angles);
				  // save results of pmove
				  BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);

				  // use the precise origin for linking
				  VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
			  }
			  else
			  {
				G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
			  }
			}
			else
			{
			  G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
			}
		}
		else
		{
			G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);
		}
		VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);
		ent->client->pers.aliveTime = 0;
		ent->client->pers.lastAliveTime = trap_Milliseconds();
		ent->client->pers.hasCheated = 0;
		ent->client->pers.grenadeUsed = 0;
		G_OC_PlayerSpawn(ent);
	}
	else
	{
		//  if(ent->client->ps.stats[STAT_HEALTH] > 0 && ent->client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->pers.teamSelection == PTE_NONE)
		G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT);

		ent->client->pers.nextCheckpointTime = level.time + G_OC_RESTARTOC_TIME;
		ent->client->pers.aliveTime = 0;
		ent->client->pers.lastAliveTime = 0;
		ent->client->pers.checkpoint = NULL;
		G_AddCreditToClient(ent->client, HUMAN_MAX_CREDITS, qtrue);
		G_OC_ClearMedis(ent->client->pers.medis);
		G_OC_ClearMedis(ent->client->pers.medisLastCheckpoint);
		G_OC_ClearArms(ent->client->pers.arms);
		G_OC_ClearArms(ent->client->pers.armsLastCheckpoint);
//        memset(ent->client->pers.medis, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.medisLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalMedistations+1));
//        memset(ent->client->pers.arms, 0, sizeof(gentity_t *) * (level.totalArmouries+1));
//        memset(ent->client->pers.armsLastCheckpoint, 0, sizeof(gentity_t *) * (level.totalArmouries+1));
		ent->client->pers.hasCheated = 0;
		ent->client->pers.grenadeUsed = 0;
		VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);
	}
}

int G_OC_UseMedi(gentity_t *ent, gentity_t *medi)
{
	gentity_t *client;
	int i;

	if(!BG_OC_OCMode())
		return 0;

	if(!medi)
		return 0;

	if(medi->s.modelindex != BA_H_MEDISTAT)  // not a medi
		return 0;

	if(!ent)
		return 0;

	if(!ent->client)  // not a client
		return 0;

	if(!ent->client->pers.medis)  // this function can only be called when there are medis, so this should never happen
		return 0;

	if(!medi->powered)  // an unpowered medistation
		return 0;

	if(!G_OC_CanUseBonus (ent))  // if the player is cheating or using equipment
		return 0;

	if(!level.totalMedistations)  // this shouldn't happen for obvious reasons
		return 0;

	// heal all players
	if(ent->client && ent->client->ps.stats[STAT_TEAM] == TEAM_HUMANS && ent->health < ent->client->ps.stats[STAT_MAX_HEALTH] && ent->client->ps.pm_type != PM_DEAD && ent != medi->enemy)
	{
		ent->client->ps.stats[STAT_STATE] |= SS_HEALING_ACTIVE;

		ent->health++;
	}

	// medi can be used

	// if the player is on a scrim team
	if(ent->client->pers.scrimTeam)
	{
		g_oc_scrimTeam_t *t;
		G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam);
		// only handle if the scrim is playing and it is a medi scrim
		if(level.ocScrimState == G_OC_STATE_PLAY && level.ocScrimMode == G_OC_MODE_MEDI)
		{
			// first merge all medis
			gentity_t **tmp = BG_Alloc(level.totalMedistations * sizeof(gentity_t *));
			memcpy(tmp, t->medis, level.totalMedistations * sizeof(gentity_t *));  // memcpy should be faster than merge itself
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				client = g_entities + i;

				if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam == ent->client->pers.scrimTeam)
					G_OC_MergeMedis(tmp, client->client->pers.medis);
			}

			// now continue testing the teams medis
			if(G_OC_AllMedis(tmp))
			{
				G_OC_AppendMedi(tmp, medi);
				G_OC_AppendMedi(ent->client->pers.medis, medi);
				if(t->time)
				{
					// team has already won
					G_ClientCP(ent, va("Medical Stations: %d/%d\n^2You Win!", level.totalMedistations, level.totalMedistations), NULL, CLIENT_SPECTATORS);  // repeat same message cp because a medi check is rarely used only once
				}
				else
				{
					// team has won
					if(!level.scrimEndTime)
						level.scrimEndTime = level.time + G_OC_SCRIMAFTERTIME;
					t->time = level.time;
					if(G_OC_HasScrimFinished())
					{
						// everbody's won, so don't wait for the timer and don't end immediately after
						level.scrimEndTime = level.time + 3000;
					}
					level.scrimWinOrder++;
//                    G_ClientCP(ent, "New Medi!", NULL, CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New medi! (%d/%d) (%s^7)", G_OC_NumberOfMedis(tmp), level.totalMedistations, ent->client->pers.netname), CLIENT_SPECTATORS | CLIENT_SCRIMTEAM);
					if(level.scrimWinOrder == 1)
					{
						G_ClientPrint(NULL, va("^7%s^7 (%ss^7)^2 wins the oc scrim! (%d/%d medical stations) - %dm%ds%dms", t->name, G_OC_HumanNameForWeapon(t->weapon), G_OC_NumberOfMedis(tmp), level.totalMedistations, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME)), CLIENT_NULL);
						G_LogPrintf("^7%s^7 (%ss^7)^2 wins the oc scrim! (%d/%d medical stations) - %dm%ds%dms\n", t->name, G_OC_HumanNameForWeapon(t->weapon), G_OC_NumberOfMedis(tmp), level.totalMedistations, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME));
					}
					else
					{
						G_ClientPrint(NULL, va("^7%s^7 (%ss^7)^2 finishes the oc scrim %d%s (%d/%d medical stations) - %dm%ds%dms", t->name, G_OC_HumanNameForWeapon(t->weapon), level.scrimWinOrder, SUFN(level.scrimWinOrder), G_OC_NumberOfMedis(tmp), level.totalMedistations, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME)), CLIENT_NULL);
						G_LogPrintf("^7%s^7 (%ss^7)^2 finishes the oc scrim %d%s (%d/%d medical stations) - %dm%ds%dms\n", t->name, G_OC_HumanNameForWeapon(t->weapon), level.scrimWinOrder, SUFN(level.scrimWinOrder), G_OC_NumberOfMedis(tmp), level.totalMedistations, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME));
					}

					if(!(t->flags & G_OC_SCRIMFLAG_NOTSINGLETEAM))
					{
						char *record = G_OC_MediStats(ent->client, level.totalMedistations, G_OC_SCRIMTIME);
						if(record && *record)
						{
							G_ClientPrint(NULL, va("^7%s^7 (%ss^7)^2 wins a record!%s", ent->client->pers.netname, G_OC_HumanNameForWeapon(t->weapon), record), CLIENT_NULL);
							G_LogPrintf(NULL, va("^7%s^7 (%ss^7)^2 wins a record!%s\n", ent->client->pers.netname, G_OC_HumanNameForWeapon(t->weapon), record));
						}
					}
				}
			}
			else
			{
				if(G_OC_HasMediBeenUsed(medi, tmp))
				{
					// player stepped on an already used medi

					// first see if it is new to the _player_, but hasn't been stepped on by another player
					if(!G_OC_HasMediBeenUsed(medi, ent->client->pers.medis) && !G_OC_HasMediBeenUsed(medi, t->medis))  // check for t->medis to see if the medi is permanently secured for the team (via a checkpoint), and if it is don't alert
					{
						// the medi has already been secured by somebody, so send
						// new medi message to client (and spectators) only
						G_OC_AppendMedi(ent->client->pers.medis, medi);
						G_ClientCP(ent, "New Medi!", NULL, CLIENT_SPECTATORS);
						G_ClientPrint(ent, va("New Medi! (%d/%d) (^2already secured by another player^7)", G_OC_NumberOfMedis(tmp), level.totalMedistations), CLIENT_SPECTATORS);
					}
					G_ClientCP(ent, va("Medical Stations: %d/%d", G_OC_NumberOfMedis(tmp), level.totalMedistations), "Medical Stations", CLIENT_SPECTATORS);
				}
				else
				{
					// new medi
					G_OC_AppendMedi(tmp, medi);
					G_OC_AppendMedi(ent->client->pers.medis, medi);
					if(G_OC_AllMedis(tmp))
					{
						BG_Free(tmp);
						return G_OC_UseMedi(ent, medi);
					}
					G_ClientCP(ent, "New Medi!", NULL, CLIENT_SPECTATORS);
					G_ClientCP(ent, va("Medical Stations: %d/%d", G_OC_NumberOfMedis(tmp), level.totalMedistations), "Medical Stations", CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Medi! (%d/%d)", G_OC_NumberOfMedis(tmp), level.totalMedistations), CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Medi! (%d/%d) (%s^7)", G_OC_NumberOfMedis(tmp), level.totalMedistations, ent->client->pers.netname), CLIENT_SCRIMTEAM | CLIENT_NOTARGET);
				}
			}

			BG_Free(tmp);
		}
	}
	// the player is not on a scrim team
	else
	{
		if(G_OC_AllMedis(ent->client->pers.medis))
		{
			// player has already won
			G_ClientCP(ent, va("Medical Stations: %d/%d\n^2You Win!", level.totalMedistations, level.totalMedistations), NULL, CLIENT_SPECTATORS);  // repeat same message cp because a medi check is rarely used only once
		}
		else if(G_OC_HasMediBeenUsed(medi, ent->client->pers.medis))
		{
			// player stepped on an already used medi
			G_ClientCP(ent, va("Medical Stations: %d/%d", G_OC_NumberOfMedis(ent->client->pers.medis), level.totalMedistations), "Medical Stations", CLIENT_SPECTATORS);
		}
		else
		{
			char *record;
			// new medi
			G_OC_AppendMedi(ent->client->pers.medis, medi);
			if(G_OC_AllMedis(ent->client->pers.medis))
			{
				// player has won
				G_OC_AppendMedi(ent->client->pers.medis, medi);
				ent->client->pers.mediTime = ent->client->pers.aliveTime;
				record = G_OC_MediStats(ent->client, level.totalMedistations, ent->client->pers.mediTime);
				AP(va("print \"^7%s^7 has used every bonus medical station! (%d^7/%d^7) (%dm:%ds%dms)%s\n\"", ent->client->pers.netname, level.totalMedistations, level.totalMedistations, MINS(ent->client->pers.mediTime), SECS(ent->client->pers.mediTime), MSEC(ent->client->pers.mediTime), record));
				G_LogPrintf(va("^7%s^7 has used every bonus medical station! (%d^7/%d^7) (%dm:%ds%dms)%s\n", ent->client->pers.netname, level.totalMedistations, level.totalMedistations, MINS(ent->client->pers.mediTime), SECS(ent->client->pers.mediTime), MSEC(ent->client->pers.mediTime), record));
				G_ClientCP(ent, va("Medical Stations: %d/%d\n^2You Win!", level.totalMedistations, level.totalMedistations), NULL, CLIENT_SPECTATORS);
				return 0;
			}
			G_ClientCP(ent, "New Medi!", NULL, CLIENT_SPECTATORS);
			G_ClientCP(ent, va("Medical Stations: %d/%d", G_OC_NumberOfMedis(ent->client->pers.medis), level.totalMedistations), "Medical Stations", CLIENT_SPECTATORS);
			G_ClientPrint(ent, va("New Medi! (%d/%d)", G_OC_NumberOfMedis(ent->client->pers.medis), level.totalMedistations), CLIENT_SPECTATORS);
			record = G_OC_MediStats(ent->client, G_OC_NumberOfMedis(ent->client->pers.medis), ent->client->pers.aliveTime);
			if(record && *record)
			{
				AP(va("print \"^7%s^7 has used a new medi! (%d^7/%d^7) (%dm%ds%dms)%s\n\"", ent->client->pers.netname, G_OC_NumberOfMedis(ent->client->pers.medis), level.totalMedistations, MINS(ent->client->pers.aliveTime), SECS(ent->client->pers.aliveTime), MSEC(ent->client->pers.aliveTime), record));
				G_LogPrintf("^7%s^7 has used a new medi! (%d^7/%d^7) (%dm%ds%dms)%s\n", ent->client->pers.netname, G_OC_NumberOfMedis(ent->client->pers.medis), level.totalMedistations, MINS(ent->client->pers.aliveTime), SECS(ent->client->pers.aliveTime), MSEC(ent->client->pers.aliveTime), record);
			}
		}
	}

	return 0;
}

// sync can be an expensive function
int G_OC_SyncMedis(gentity_t **medis, int len)
{
	// medis should contain a null terminator at medis[len]
	int i, j, k, tmp, tmp2;

	// OC only
	if(!BG_OC_OCMode())
		return 0;

	// first eliminate anything that isn't powered or isn't a medi
	for(i = 0; i < len; i++)
	{
		if(medis[i])
		{
			if(medis[i]->s.modelindex != BA_H_MEDISTAT)
			{
				medis[i] = NULL;
				continue;
			}
			if(!medis[i]->powered)
			{
				medis[i] = NULL;
				continue;
			}
		}
	}

	// eliminate duplicates
	for(i = 0; i < len; i++)
	{
		for(j = i + 1; j < len; j++)
		{
			for(k = 0; k < 2; k++)
			{
				memcpy(&tmp, &medis[i], sizeof(gentity_t *));
				memcpy(&tmp2, &medis[j], sizeof(gentity_t *));
				tmp  ^= tmp2;
				tmp2 ^= tmp;
				tmp  ^= tmp2;
				memcpy(&medis[j], &tmp2, sizeof(gentity_t *));
				memcpy(&medis[i], &tmp, sizeof(gentity_t *));
			}
		}
	}

	// continually move empty elements right (the stupid and expensive part)
	for(i = 0; i < len; i++)
	{
		for(j = 0; i < len; i++)
		{
			if(!medis[j])
			{
				if(medis[j] != medis[j + 1])
				{
					memcpy(&tmp, &medis[j], sizeof(gentity_t *));
					memcpy(&tmp2, &medis[j + 1], sizeof(gentity_t *));
					tmp  ^= tmp2;
					tmp2 ^= tmp;
					tmp  ^= tmp2;
					memcpy(&medis[j + 1], &tmp2, sizeof(gentity_t *));
					memcpy(&medis[j], &tmp, sizeof(gentity_t *));
				}
			}
		}
	}

	return 0;
}

int G_OC_MergeMedis(gentity_t **dst, gentity_t **src)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; src[i]; i++)
	{
		G_OC_AppendMedi(dst, src[i]);
	}

	return 0;
}

int G_OC_AppendMedi(gentity_t **medis, gentity_t *medi)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; medis[i]; i++)
	{
		if(medis[i] == medi)
		{
			return 0;
		}
	}

	medis[i] = medi;

	return 0;
}

int G_OC_RemoveMedi(gentity_t **medis, gentity_t *medi)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; medis[i]; i++)
	{
		if(medis[i] == medi)
		{
			memmove(&medis[i], &medis[i + 1], sizeof(gentity_t *) * (level.totalMedistations - i));
			break;
		}
	}

	return 0;
}

int G_OC_AllMedis(gentity_t **medis)
{
	if(!BG_OC_OCMode())
		return 0;

	return !(level.totalMedistations - G_OC_NumberOfMedis(medis));
}

int G_OC_NumberOfMedis(gentity_t **medis)
{
	int i, count = 0;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; i < level.totalMedistations; i++) if(medis[i]) count++; else break;

	return count;
}

int G_OC_HasMediBeenUsed(gentity_t *medi, gentity_t **medis)
{
	int i;
	gentity_t *ent;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; medis[i]; i++)
	{
		ent = medis[i];

		if(ent == medi)
		{
			return 1;
		}
	}

	return 0;
}

int G_OC_ClearMedis(gentity_t **medis)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	if(!medis)
		return 0;

	for(i = 0; i < level.totalMedistations; i++)
	{
		medis[i] = NULL;
	}

	return 0;
}

int G_OC_UseArm(gentity_t *ent, gentity_t *arm)
{
	gentity_t *client;
	int i;

	if(!BG_OC_OCMode())
		return 0;

	if(!arm)
		return 0;

	if(arm->s.modelindex != BA_H_ARMOURY)  // not an arm
		return 0;

	if(!ent)
		return 0;

	if(!ent->client)  // not a client
		return 0;

	if(!ent->client->pers.arms)  // this function can only be called when there are arms, so this should never happen
		return 0;

	if(!arm->powered)  // an unpowered armoury
		return 0;

	if(!G_OC_CanUseBonus (ent))  // if the player is cheating or using equipment
		return 0;

	if(!level.totalArmouries)  // this shouldn't happen for obvious reasons
		return 0;

	// arm can be used

	// if the player is on a scrim team
	if(ent->client->pers.scrimTeam)
	{
		g_oc_scrimTeam_t *t;
		G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam);
		// only handle if the scrim is playing and it is an arm scrim
		if(level.ocScrimState == G_OC_STATE_PLAY && level.ocScrimMode == G_OC_MODE_ARM)
		{
			// first merge all arms
			gentity_t **tmp = BG_Alloc(level.totalArmouries * sizeof(gentity_t *));
			memcpy(tmp, t->arms, level.totalArmouries * sizeof(gentity_t *));  // memcpy should be faster than merge itself
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				client = g_entities + i;

				if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam == ent->client->pers.scrimTeam)
					G_OC_MergeArms(tmp, client->client->pers.arms);
			}

			// now continue testing the teams arms
			if(G_OC_AllArms(tmp) || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
			{
				G_OC_AppendArm(ent->client->pers.arms, arm);
				if(t->time)
				{
					// team has already won
					if(level.totalArmouries == 1)
						G_ClientCP(ent, va("^a^r^m^2You Win!"), "^a^r^m", CLIENT_SPECTATORS);
					else
						G_ClientCP(ent, va("^a^r^mArmouries: %d/%d\n^2You Win!", level.totalArmouries, level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
				}
				else
				{
					// team has won
					if(!level.scrimEndTime)
						level.scrimEndTime = level.time + G_OC_SCRIMAFTERTIME;
					t->time = level.time;
					if(G_OC_HasScrimFinished())
					{
						// everbody's won, so don't wait for the timer and don't end immediately after
						level.scrimEndTime = level.time + 3000;
					}
					level.scrimWinOrder++;
//                    G_ClientCP(ent, "^a^r^mNew Armoury!", "^a^r^m", CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Armoury! (%d/%d) (%s^7)", G_OC_NumberOfArms(tmp), level.totalArmouries, ent->client->pers.netname), CLIENT_SPECTATORS | CLIENT_SCRIMTEAM);
					if(level.scrimWinOrder == 1)
					{
						G_ClientPrint(NULL, va("^7%s^7 (%ss^7)^2 wins the oc scrim! (%d/%d armouries) - %dm%ds%dms", t->name, G_OC_HumanNameForWeapon(t->weapon), G_OC_NumberOfArms(tmp), level.totalArmouries, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME)), CLIENT_NULL);
						G_LogPrintf("^7%s^7 (%ss^7)^2 wins the oc scrim! (%d/%d armouries) - %dm%ds%dms\n", t->name, G_OC_HumanNameForWeapon(t->weapon), G_OC_NumberOfArms(tmp), level.totalArmouries, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME));
					}
					else
					{
						G_ClientPrint(NULL, va("^7%s^7 (%ss^7)^2 finishes the oc scrim %d%s (%d/%d armouries) - %dm%ds%dms", t->name, G_OC_HumanNameForWeapon(t->weapon), level.scrimWinOrder, SUFN(level.scrimWinOrder), G_OC_NumberOfArms(tmp), level.totalArmouries, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME)), CLIENT_NULL);
						G_LogPrintf("^7%s^7 (%ss^7)^2 finishes the oc scrim %d%s (%d/%d armouries) - %dm%ds%dms\n", t->name, G_OC_HumanNameForWeapon(t->weapon), level.scrimWinOrder, SUFN(level.scrimWinOrder), G_OC_NumberOfArms(tmp), level.totalArmouries, MINS(G_OC_SCRIMTIME), SECS(G_OC_SCRIMTIME), MSEC(G_OC_SCRIMTIME));
					}

					if(!(t->flags & G_OC_SCRIMFLAG_NOTSINGLETEAM))
					{
						char *record = G_OC_WinStats(ent->client, level.totalArmouries, G_OC_SCRIMTIME);
						if(record && *record)
						{
							G_ClientPrint(NULL, va("^7%s^7 (%ss^7)^2 wins a record!%s", ent->client->pers.netname, G_OC_HumanNameForWeapon(t->weapon), record), CLIENT_NULL);
							G_LogPrintf(NULL, va("^7%s^7 (%ss^7)^2 wins a record!%s\n", ent->client->pers.netname, G_OC_HumanNameForWeapon(t->weapon), record));
						}
					}
				}
			}
			else
			{
				if(G_OC_HasArmBeenUsed(arm, tmp))
				{
					// player used an already used arm

					// first see if it is new to the _player_, but hasn't been used by another player
					if(!G_OC_HasArmBeenUsed(arm, ent->client->pers.arms) && !G_OC_HasArmBeenUsed(arm, t->arms))  // check for t->medis to see if the medi is permanently secured for the team (via a checkpoint), and if it is don't alert
					{
						// the arm has already been secured by somebody, so send
						// new arm message to client (and spectators) only
						G_OC_AppendArm(ent->client->pers.arms, arm);
						G_ClientCP(ent, "New Armoury!", NULL, CLIENT_SPECTATORS);
						G_ClientPrint(ent, va("New Armoury! (%d/%d) (^2already secured by another player^7)", G_OC_NumberOfArms(tmp), level.totalArmouries), CLIENT_SPECTATORS);
					}
					G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(tmp), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
				}
				else
				{
					// new arm
					G_OC_AppendArm(tmp, arm);
					G_OC_AppendArm(ent->client->pers.arms, arm);
					if(G_OC_AllArms(tmp))
					{
						BG_Free(tmp);
						return G_OC_UseArm(ent, arm);
					}
					G_ClientCP(ent, "New Armoury!", NULL, CLIENT_SPECTATORS);
					G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(tmp), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Armoury! (%d/%d)", G_OC_NumberOfArms(tmp), level.totalArmouries), CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Armoury! (%d/%d) (%s^7)", G_OC_NumberOfArms(tmp), level.totalArmouries, ent->client->pers.netname), CLIENT_SCRIMTEAM | CLIENT_NOTARGET);
				}
			}

			BG_Free(tmp);
		}
		else if(level.ocScrimState == G_OC_STATE_PLAY && level.ocScrimMode == G_OC_MODE_MEDI)
		{
			// keep track of the arms during a medi scrim (so that weapons,
			// upgrades, etc can be bought) but don't give any records or wins
			// first merge all arms
			gentity_t **tmp = BG_Alloc(level.totalArmouries * sizeof(gentity_t *));
			memcpy(tmp, t->arms, level.totalArmouries * sizeof(gentity_t *));  // memcpy should be faster than merge itself
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				client = g_entities + i;

				if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam == ent->client->pers.scrimTeam)
					G_OC_MergeArms(tmp, client->client->pers.arms);
			}

			// now continue testing the teams arms
			if(G_OC_AllArms(tmp) || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
			{
				t->flags |= G_OC_SCRIMFLAG_EQUIPMENT;
				G_OC_AppendArm(ent->client->pers.arms, arm);
				if(!G_OC_HasArmBeenUsed(arm, tmp))
				{
					G_OC_AppendArm(tmp, arm);
					G_ClientCP(ent, "Team has used enough armouries\nyou can now buy equipment!", NULL, CLIENT_SPECTATORS);
					G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(tmp), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("Enough armouries have been used by the team! (%d/%d)", G_OC_NumberOfArms(tmp), level.totalArmouries), CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("Enough armouries have been used by the team! (%d/%d) (%s^7)", G_OC_NumberOfArms(tmp), level.totalArmouries, ent->client->pers.netname), CLIENT_SCRIMTEAM | CLIENT_NOTARGET);
				}
			}
			else
			{
				if(G_OC_HasArmBeenUsed(arm, tmp))
				{
					// player used an already used arm

					// first see if it is new to the _player_, but hasn't been used by another player
					if(!G_OC_HasArmBeenUsed(arm, ent->client->pers.arms) && !G_OC_HasArmBeenUsed(arm, t->arms))
					{
						// the arm has already been secured by somebody, so send
						// new arm message to client (and spectators) only
						G_OC_AppendArm(ent->client->pers.arms, arm);
						G_ClientCP(ent, "New Armoury!", NULL, CLIENT_SPECTATORS);
						G_ClientPrint(ent, va("New Armoury! (%d/%d) (^2already secured by another player^7)", G_OC_NumberOfArms(tmp), level.totalArmouries), CLIENT_SPECTATORS);
					}
					G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(tmp), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
				}
				else
				{
					// new arm
					G_OC_AppendArm(tmp, arm);
					G_OC_AppendArm(ent->client->pers.arms, arm);
					if(G_OC_AllArms(tmp))
					{
						BG_Free(tmp);
						return G_OC_UseArm(ent, arm);
					}
					G_ClientCP(ent, "New Armoury!", NULL, CLIENT_SPECTATORS);
					G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(tmp), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Armoury! (%d/%d)", G_OC_NumberOfArms(tmp), level.totalArmouries), CLIENT_SPECTATORS);
					G_ClientPrint(ent, va("New Armoury! (%d/%d) (%s^7)", G_OC_NumberOfArms(tmp), level.totalArmouries, ent->client->pers.netname), CLIENT_SCRIMTEAM | CLIENT_NOTARGET);
				}
			}

			BG_Free(tmp);
		}
	}
	// the player is not on a scrim team
	else
	{
		if(G_OC_AllArms(ent->client->pers.arms) || (G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM) && G_OC_NumberOfArms(ent->client->pers.arms)))
		{
			// player has already won
			if(level.totalArmouries == 1)
				G_ClientCP(ent, va("^a^r^m^2You Win!"), "^a^r^m", CLIENT_SPECTATORS);
			else
				G_ClientCP(ent, va("^a^r^mArmouries: %d/%d\n^2You Win!", level.totalArmouries, level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
		}
		else if(G_OC_HasArmBeenUsed(arm, ent->client->pers.arms))
		{
			// player stepped on an already used arm
			G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(ent->client->pers.arms), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
		}
		else
		{
			// new arm
			G_OC_AppendArm(ent->client->pers.arms, arm);
			if(G_OC_AllArms(ent->client->pers.arms) || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
			{
				// player has won
				char *record;
				G_OC_AppendArm(ent->client->pers.arms, arm);
				ent->client->pers.winTime = ent->client->pers.aliveTime;
				record = G_OC_WinStats(ent->client, level.totalArmouries, ent->client->pers.winTime);
				if(level.totalArmouries == 1 || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
				{
					G_ClientCP(ent, va("^a^r^m^2You Win!"), "^a^r^m", CLIENT_SPECTATORS);
					AP(va("print \"^7%s^7 wins! (%dm:%ds%dms)%s\n\"", ent->client->pers.netname, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime), record));
					G_LogPrintf(va("^7%s^7 wins! (%dm:%ds%dms)%s\n", ent->client->pers.netname, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime), record));
				}
				else
				{
					if(level.totalArmouries == 1)
					{
						AP(va("print \"^7%s^7 wins! (%d^7/%d^7) (%dm:%ds%dms)%s\n\"", ent->client->pers.netname, level.totalArmouries, level.totalArmouries, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime), record));
						G_LogPrintf(va("^7%s^7 wins! (%d^7/%d^7) (%dm:%ds%dms)%s\n", ent->client->pers.netname, level.totalArmouries, level.totalArmouries, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime), record));
						G_ClientCP(ent, va("^a^r^mArmouries: %d/%d\n^2You Win!", level.totalArmouries, level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
					}
					else
					{
						AP(va("print \"^7%s^7 wins! (%dm:%ds%dms)%s\n\"", ent->client->pers.netname, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime), record));
						G_LogPrintf(va("^7%s^7 wins! (%dm:%ds%dms)%s\n", ent->client->pers.netname, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime), record));
						G_ClientCP(ent, va("^a^r^m^2You Win!"), "^a^r^m", CLIENT_SPECTATORS);
					}
				}
				return 0;
			}
			G_ClientCP(ent, "New Armoury!", NULL, CLIENT_SPECTATORS);
			G_ClientCP(ent, va("^a^r^mArmouries: %d/%d", G_OC_NumberOfArms(ent->client->pers.arms), level.totalArmouries), "^a^r^m", CLIENT_SPECTATORS);
			G_ClientPrint(ent, va("New Armoury! (%d/%d)", G_OC_NumberOfArms(ent->client->pers.arms), level.totalArmouries), CLIENT_SPECTATORS);
		}
	}

	return 0;
}

// sync can be an expensive function
int G_OC_SyncArms(gentity_t **arms, int len)
{
	// arms should contain a null terminator at arms[len]
	int i, j, k, tmp, tmp2;

	// first eliminate anything that isn't powered or isn't an arm
	for(i = 0; i < len; i++)
	{
		if(arms[i])
		{
			if(arms[i]->s.modelindex != BA_H_ARMOURY)
			{
				arms[i] = NULL;
				continue;
			}
			if(!arms[i]->powered)
			{
				arms[i] = NULL;
				continue;
			}
		}
	}

	// eliminate duplicates
	for(i = 0; i < len; i++)
	{
		for(j = i + 1; j < len; j++)
		{
			for(k = 0; k < 2; k++)
			{
				memcpy(&tmp, &arms[i], sizeof(gentity_t *));
				memcpy(&tmp2, &arms[j], sizeof(gentity_t *));
				tmp  ^= tmp2;
				tmp2 ^= tmp;
				tmp  ^= tmp2;
				memcpy(&arms[j], &tmp2, sizeof(gentity_t *));
				memcpy(&arms[i], &tmp, sizeof(gentity_t *));
			}
		}
	}

	// continually move empty elements right (the stupid and expensive part)
	for(i = 0; i < len; i++)
	{
		for(j = 0; i < len; i++)
		{
			if(!arms[j])
			{
				if(arms[j] != arms[j + 1])
				{
					memcpy(&tmp, &arms[j], sizeof(gentity_t *));
					memcpy(&tmp2, &arms[j + 1], sizeof(gentity_t *));
					tmp  ^= tmp2;
					tmp2 ^= tmp;
					tmp  ^= tmp2;
					memcpy(&arms[j + 1], &tmp2, sizeof(gentity_t *));
					memcpy(&arms[j], &tmp, sizeof(gentity_t *));
				}
			}
		}
	}

	return 0;
}

int G_OC_MergeArms(gentity_t **dst, gentity_t **src)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; src[i]; i++)
	{
		G_OC_AppendArm(dst, src[i]);
	}

	return 0;
}

int G_OC_AppendArm(gentity_t **arms, gentity_t *arm)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; arms[i]; i++)
	{
		if(arms[i] == arm)
		{
			return 0;
		}
	}

	arms[i] = arm;

	return 0;
}

int G_OC_RemoveArm(gentity_t **arms, gentity_t *arm)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; arms[i]; i++)
	{
		if(arms[i] == arm)
		{
			memmove(&arms[i], &arms[i + 1], sizeof(gentity_t *) * (level.totalArmouries - i));
			break;
		}
	}

	return 0;
}

int G_OC_AllArms(gentity_t **arms)
{
	if(!BG_OC_OCMode())
		return 0;

	return !(level.totalArmouries - G_OC_NumberOfArms(arms));
}

int G_OC_NumberOfArms(gentity_t **arms)
{
	int i, count = 0;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; i < level.totalArmouries; i++) if(arms[i]) count++; else break;

	return count;
}

int G_OC_HasArmBeenUsed(gentity_t *arm, gentity_t **arms)
{
	int i;
	gentity_t *ent;

	if(!BG_OC_OCMode())
		return 0;

	for(i = 0; arms[i]; i++)
	{
		ent = arms[i];

		if(ent == arm)
		{
			return 1;
		}
	}

	return 0;
}

int G_OC_ClearArms(gentity_t **arms)
{
	int i;

	if(!BG_OC_OCMode())
		return 0;

	if(!arms)
		return 0;

	for(i = 0; i < level.totalArmouries; i++)
	{
		arms[i] = NULL;
	}

	return 0;
}

int G_OC_BuildableBuilt(gentity_t *ent)  // called when ent is built
{
	int i;
	gentity_t **tmp;
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
		return 0;

	if(ent->s.modelindex == BA_H_MEDISTAT)
	{
		level.totalMedistations++;

//        for(si = level.scrimTeam + 1; si; si = si->next)
		for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
		{
			if(si->active)
			{
				if(si->medis)
				{
					tmp = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
					memcpy(tmp, si->medis, level.totalMedistations * sizeof(gentity_t *));
					BG_Free(si->medis);
					si->medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
					memcpy(si->medis, tmp, level.totalMedistations * sizeof(gentity_t *));
					BG_Free(tmp);
				}
				else
				{
					si->medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
				}
			}
		}
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.medis)
			{
				tmp = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
				memcpy(tmp, level.clients[i].pers.medis, level.totalMedistations * sizeof(gentity_t *));
				BG_Free(level.clients[i].pers.medis);
				level.clients[i].pers.medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
				memcpy(level.clients[i].pers.medis, tmp, level.totalMedistations * sizeof(gentity_t *));
				BG_Free(tmp);
			}
			else if(g_entities[i].client && level.clients[i].pers.connected)
			{
				level.clients[i].pers.medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
			}

			if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.medisLastCheckpoint)
			{
				tmp = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
				memcpy(tmp, level.clients[i].pers.medisLastCheckpoint, level.totalMedistations * sizeof(gentity_t *));
				BG_Free(level.clients[i].pers.medisLastCheckpoint);
				level.clients[i].pers.medisLastCheckpoint = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
				memcpy(level.clients[i].pers.medisLastCheckpoint, tmp, level.totalMedistations * sizeof(gentity_t *));
				BG_Free(tmp);
			}
			else if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED)
			{
				level.clients[i].pers.medisLastCheckpoint = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
			}
		}
	}
	else if(ent->s.modelindex == BA_H_ARMOURY)
	{
		level.totalArmouries++;

//        for(si = level.scrimTeam + 1; si; si = si->next)
		for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
		{
			if(si->active)
			{
				if(si->arms)
				{
					tmp = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
					memcpy(tmp, si->arms, level.totalArmouries * sizeof(gentity_t *));
					BG_Free(si->arms);
					si->arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
					memcpy(si->arms, tmp, level.totalArmouries * sizeof(gentity_t *));
					BG_Free(tmp);
				}
				else
				{
					si->arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
				}
			}
		}
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.arms)
			{
				tmp = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
				memcpy(tmp, level.clients[i].pers.arms, level.totalArmouries * sizeof(gentity_t *));
				BG_Free(level.clients[i].pers.arms);
				level.clients[i].pers.arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
				memcpy(level.clients[i].pers.arms, tmp, level.totalArmouries * sizeof(gentity_t *));
				BG_Free(tmp);
			}
			else if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED)
			{
				level.clients[i].pers.arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
			}

			if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.armsLastCheckpoint)
			{
				tmp = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
				memcpy(tmp, level.clients[i].pers.armsLastCheckpoint, level.totalArmouries * sizeof(gentity_t *));
				BG_Free(level.clients[i].pers.armsLastCheckpoint);
				level.clients[i].pers.armsLastCheckpoint = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
				memcpy(level.clients[i].pers.armsLastCheckpoint, tmp, level.totalArmouries * sizeof(gentity_t *));
				BG_Free(tmp);
			}
			else if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED)
			{
				level.clients[i].pers.armsLastCheckpoint = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
			}
		}
	}
	else if(ent->s.modelindex == BA_H_SPAWN)
	{
		level.numNodes++;
	}

	return 0;
}

int G_OC_BuildableDestroyed(gentity_t *ent)  // called when a buildable no longer exists (before it is really freed)
{
	int i;
	gentity_t **tmp;
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
		return 0;

	if(!ent->powered && ent->verifyUnpowered)
		return 0;

	if(ent->s.modelindex == BA_H_MEDISTAT)
	{
		level.totalMedistations--;

		if(level.totalMedistations)
		{
//            for(si = level.scrimTeam + 1; si; si = si->next)
			for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
			{
				if(si->active)
				{
					tmp = BG_Alloc((level.totalMedistations + 2) * sizeof(gentity_t *));
					memcpy(tmp, si->medis, (level.totalMedistations) * sizeof(gentity_t *));
					G_OC_RemoveMedi(tmp, ent);
					BG_Free(si->medis);
					si->medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
					memcpy(si->medis, tmp, level.totalMedistations * sizeof(gentity_t *));
					BG_Free(tmp);
				}
			}
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.medis && level.clients[i].pers.medisLastCheckpoint)  // if medis is not allocated, neither should medisLastCheckpoint.  The inverse is also true
				{
					tmp = BG_Alloc((level.totalMedistations + 2) * sizeof(gentity_t *));
					memcpy(tmp, level.clients[i].pers.medis, (level.totalMedistations) * sizeof(gentity_t *));
					G_OC_RemoveMedi(tmp, ent);
					BG_Free(level.clients[i].pers.medis);
					level.clients[i].pers.medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
					memcpy(level.clients[i].pers.medis, tmp, level.totalMedistations * sizeof(gentity_t *));
					BG_Free(tmp);

					tmp = BG_Alloc((level.totalMedistations + 2) * sizeof(gentity_t *));
					memcpy(tmp, level.clients[i].pers.medisLastCheckpoint, (level.totalMedistations) * sizeof(gentity_t *));
					G_OC_RemoveMedi(tmp, ent);
					BG_Free(level.clients[i].pers.medisLastCheckpoint);
					level.clients[i].pers.medisLastCheckpoint = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
					memcpy(level.clients[i].pers.medisLastCheckpoint, tmp, level.totalMedistations * sizeof(gentity_t *));
					BG_Free(tmp);
				}
			}
		}
		else
		{
//            for(si = level.scrimTeam + 1; si; si = si->next)
			for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
			{
				if(si->active)
				{
					BG_Free(si->medis);
					si->medis = NULL;
				}
			}
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.medis && level.clients[i].pers.medisLastCheckpoint)  // if medis is not allocated, neither should medisLastCheckpoint.  The inverse is also true
				{
					BG_Free(level.clients[i].pers.medis);
					level.clients[i].pers.medis = NULL;

					BG_Free(level.clients[i].pers.medisLastCheckpoint);
					level.clients[i].pers.medisLastCheckpoint = NULL;
				}
			}
		}
	}
	else if(ent->s.modelindex == BA_H_ARMOURY)
	{
		level.totalArmouries--;

		if(level.totalArmouries)
		{
//            for(si = level.scrimTeam + 1; si; si = si->next)
			for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
			{
				if(si->active)
				{
					tmp = BG_Alloc((level.totalArmouries + 2) * sizeof(gentity_t *));
					memcpy(tmp, si->arms, (level.totalArmouries) * sizeof(gentity_t *));
					G_OC_RemoveArm(tmp, ent);
					BG_Free(si->arms);
					si->arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
					memcpy(si->arms, tmp, level.totalArmouries * sizeof(gentity_t *));
					BG_Free(tmp);
				}
			}
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.arms && level.clients[i].pers.armsLastCheckpoint)  // if medis is not allocated, neither should medisLastCheckpoint.  The inverse is also true
				{
					tmp = BG_Alloc((level.totalArmouries + 2) * sizeof(gentity_t *));
					memcpy(tmp, level.clients[i].pers.arms, (level.totalArmouries) * sizeof(gentity_t *));
					G_OC_RemoveArm(tmp, ent);
					BG_Free(level.clients[i].pers.arms);
					level.clients[i].pers.arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
					memcpy(level.clients[i].pers.arms, tmp, level.totalArmouries * sizeof(gentity_t *));
					BG_Free(tmp);

					tmp = BG_Alloc((level.totalArmouries + 2) * sizeof(gentity_t *));
					memcpy(tmp, level.clients[i].pers.armsLastCheckpoint, (level.totalArmouries) * sizeof(gentity_t *));
					G_OC_RemoveArm(tmp, ent);
					BG_Free(level.clients[i].pers.armsLastCheckpoint);
					level.clients[i].pers.armsLastCheckpoint = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
					memcpy(level.clients[i].pers.armsLastCheckpoint, tmp, level.totalArmouries * sizeof(gentity_t *));
					BG_Free(tmp);
				}
			}
		}
		else
		{
//            for(si = level.scrimTeam + 1; si; si = si->next)
			for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
			{
				if(si->active)
				{
					BG_Free(si->arms);
					si->arms = NULL;
				}
			}
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED && level.clients[i].pers.arms && level.clients[i].pers.armsLastCheckpoint)  // if medis is not allocated, neither should medisLastCheckpoint.  The inverse is also true
				{
					BG_Free(level.clients[i].pers.arms);
					level.clients[i].pers.arms = NULL;

					BG_Free(level.clients[i].pers.armsLastCheckpoint);
					level.clients[i].pers.armsLastCheckpoint = NULL;
				}
			}
		}
	}
	else if(ent->s.modelindex == BA_H_SPAWN)
	{
		level.numNodes--;
	}

	return 0;
}

int G_OC_Checkpoint(gentity_t *checkpoint, gentity_t *ent)  // called when a player, ent, activates a checkpoint
{
	if(!BG_OC_OCMode())
		return 0;

	if(!checkpoint)
		return 0;

	if(!ent)
		return 0;

	if(!ent->client)
		return 0;

	if(!checkpoint)
		return 0;

	if(checkpoint->client)
		return 0;

	if(checkpoint->s.modelindex != BA_A_BOOSTER)
		return 0;

	// prevent against checkpoint exploit
	if(ent->client->pers.nextCheckpointTime && level.time < ent->client->pers.nextCheckpointTime)
		return 0;

	if(ent->health <= 0)
		return 0;

	// do nothing if preparing for a match
	if(ent->client->pers.scrimTeam && level.ocScrimState < G_OC_STATE_PLAY)
		return 0;

	// good to go...
	ent->client->pers.nextCheckpointTime = 0;

	// "Checkpoint!" if the player shot a new checkpoint
	if(ent->client->pers.checkpoint != checkpoint)  // &&
	if(!ent->client->pers.scrimTeam || level.scrimTeam[ent->client->pers.scrimTeam].checkpoint != checkpoint)
	{
		G_ClientPrint(ent, "Checkpoint!", CLIENT_SPECTATORS);
		if(ent->client->pers.scrimTeam)
			G_ClientPrint(ent, va("Checkpoint! (%s^7)", ent->client->pers.netname), CLIENT_SCRIMTEAM | CLIENT_NOTARGET);
	}
	G_ClientCP(ent, "Checkpoint!", NULL, CLIENT_SPECTATORS);

	// update checkpoint

	// update checkpoint data
	if(ent->client->pers.scrimTeam)
	{
		g_oc_scrimTeam_t *t;
		G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam);
		G_OC_MergeMedis(t->medis, ent->client->pers.medis);
		G_OC_MergeMedis(t->arms, ent->client->pers.arms);
		G_OC_ClearMedis(ent->client->pers.medis);
		G_OC_ClearArms(ent->client->pers.arms);
		t->checkpoint = checkpoint;
	}
	else
	{
		ent->client->pers.checkpoint = checkpoint;
		if(level.totalMedistations && ent->client->pers.medisLastCheckpoint && ent->client->pers.medis)
			memcpy(ent->client->pers.medisLastCheckpoint, ent->client->pers.medis, (level.totalMedistations + 1) * sizeof(gentity_t *));
		else if(level.totalMedistations)
			G_ClientPrint(ent, "^1Error saving checkpoint information", CLIENT_SPECTATORS);
		if(level.totalArmouries && ent->client->pers.armsLastCheckpoint && ent->client->pers.arms)
			memcpy(ent->client->pers.armsLastCheckpoint, ent->client->pers.arms, (level.totalArmouries + 1) * sizeof(gentity_t *));
		else if(level.totalArmouries)
			G_ClientPrint(ent, "^1Error saving checkpoint information", CLIENT_SPECTATORS);
	}

	return 0;
}

void G_OC_PlayerMaxAmmo(gentity_t *ent)
{
	gclient_t *client = ent->client;

	if(client->ps.weapon != WP_ALEVEL3_UPG &&
		BG_Weapon(client->ps.weapon)->infiniteAmmo)
		return;

	client->ps.ammo = BG_Weapon(client->ps.weapon)->maxAmmo;
	client->ps.clips = BG_Weapon(client->ps.weapon)->maxClips;

	if(BG_Weapon(client->ps.weapon)->usesEnergy &&
		BG_InventoryContainsUpgrade(UP_BATTPACK, client->ps.stats))
		client->ps.ammo = (int)((float)client->ps.ammo * BATTPACK_MODIFIER);
}

void G_OC_PlayerMaxClips(gentity_t *ent)
{
	gclient_t *client = ent->client;

	if(client->ps.weapon != WP_ALEVEL3_UPG &&
		BG_Weapon(client->ps.weapon)->infiniteAmmo)
		return;

	client->ps.clips = BG_Weapon(client->ps.weapon)->maxClips;
}

void G_OC_PlayerMaxCash(gentity_t *ent)
{
	G_AddCreditToClient(ent->client, ALIEN_MAX_CREDITS, qtrue);
}

void G_OC_PlayerMaxHealth(gentity_t *ent)
{
	ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
	BG_AddUpgradeToInventory(UP_MEDKIT, ent->client->ps.stats);
}

int G_OC_PlayerSpawn(gentity_t *ent)  // called when a player spawns
{
	if(!BG_OC_OCMode())
		return 0;

	if(ent && ent->client)
	{
		ent->client->pers.lastAliveTime = trap_Milliseconds();  // reset to spawn time so that time during death doesn't count
		G_OC_PlayerMaxAmmo(ent);
		if(!ent->client->pers.checkpoint)
			G_OC_PlayerMaxCash(ent);
		G_OC_PlayerMaxHealth(ent);
		VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);
//		if(ent->client->pers.teamSelection == TEAM_ALIENS)
//		{
//                VectorScale(ent->s.angles2, 0.0, ent->s.angles2);
//		}
	}

	return 0;
}

int G_OC_PlayerDie(gentity_t *ent)  // called when a player dies
{
	int i, j;
	gentity_t *client;

	if(!BG_OC_OCMode())
		return 0;

	if(ent->client->pers.scrimTeam)
	{
		if(level.ocScrimState == G_OC_STATE_PLAY)
		{
			if(level.ocScrimMode == G_OC_MODE_MEDI)
			{
				int lost = 0;
				gentity_t *medi;
				g_oc_scrimTeam_t *t;
				G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam);

				// don't do anything if they already won
				if(t->time)
					return 0;

				// iterate over each medi.  If no other player has the same medi
				// notify the team that is lost x medis
				for(i = 0, medi = ent->client->pers.medis[0]; medi; medi = ent->client->pers.medis[++i])
				{
					qboolean used = qfalse;
					if(!G_OC_HasMediBeenUsed(medi, t->medis))
					{
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							client = g_entities + j;
							if(client->client && client->client->pers.scrimTeam == ent->client->pers.scrimTeam && G_OC_HasMediBeenUsed(medi, client->client->pers.medis))
							{
								used = qtrue;
							}
						}
					}
					if(!used)
						lost++;
				}
				G_OC_ClearMedis(ent->client->pers.medis);
				if(lost)
				{
					G_ClientPrint(ent, va("Your team lost %d medical stations!", lost), CLIENT_SCRIMTEAM);
				}
			}
//				if(level.ocScrimMode == G_OC_MODE_ARM)  // keep track of arms during a medi scrim
			{
				int lost = 0;
				gentity_t *arm;
				g_oc_scrimTeam_t *t;
				G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam);

				// iterate over each arm.  If no other player has the same arm
				// notify the team that is lost x arms
				for(i = 0, arm = ent->client->pers.arms[0]; arm; arm = ent->client->pers.arms[++i])
				{
					qboolean used = qfalse;
					if(!G_OC_HasArmBeenUsed(arm, t->arms))
					{
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							client = g_entities + j;
							if(client->client && client->client->pers.scrimTeam == ent->client->pers.scrimTeam && G_OC_HasArmBeenUsed(arm, client->client->pers.arms))
							{
								used = qtrue;
							}
						}
					}
					if(!used)
						lost++;
				}
				G_OC_ClearArms(ent->client->pers.arms);
				if(lost)
				{
					// see if the equipment flag needs to be reset
					if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
					{
						qboolean used = qfalse;
						// iterate through each client and see if there's still at least 1 used arm
						if(t->arms[0])  // note that arm and medi data are not fragmented, so if the first slot doesn't have an arm there are no arms in this array
						{
							used = qtrue;
						}
						if(!used)
						{
							// still no arm (saved for the team by checkpoints), check if any player has an arm
							for(i = 0; i < MAX_CLIENTS; i++)
							{
								client = g_entities + i;
								if(client->client && client->client->pers.scrimTeam == ent->client->pers.scrimTeam)
								{
									if(client->client->pers.arms[0])
									{
										used = qtrue;
										break;
									}
								}
							}
						}
						if(!used)
						{
							if(t->flags & G_OC_SCRIMFLAG_EQUIPMENT)
							{
								// let them know if they can't buy equipment anymore if they previously could
								G_ClientPrint(ent, "Your team can no longer buy new equipment!", CLIENT_SCRIMTEAM);
							}
							t->flags &= ~G_OC_SCRIMFLAG_EQUIPMENT;
						}
					}
					else
					{
						// they lost at least 1 arm, so reset
						if(t->flags & G_OC_SCRIMFLAG_EQUIPMENT)
						{
							// let them know if they can't buy equipment anymore if they previously could
							G_ClientPrint(ent, "Your team can no longer buy new equipment!", CLIENT_SCRIMTEAM);
						}
						t->flags &= ~G_OC_SCRIMFLAG_EQUIPMENT;
					}
//					if(G_OC_AllArms(tmp) || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
//					{
//					}
					G_ClientPrint(ent, va("Your team lost %d armouries!", lost), CLIENT_SCRIMTEAM);
				}
			}
		}
	}
	else
	{
		if(level.totalMedistations)
		{
			int lost = G_OC_NumberOfMedis(ent->client->pers.medis) - G_OC_NumberOfMedis(ent->client->pers.medisLastCheckpoint);
			memcpy(ent->client->pers.medis, ent->client->pers.medisLastCheckpoint, (level.totalMedistations + 1) * sizeof(gentity_t *));
			if(lost)
			{
				G_ClientPrint(ent, va("You lost %d medis!", lost), CLIENT_SPECTATORS);
			}
		}
		if(level.totalArmouries)
		{
			int lost = G_OC_NumberOfArms(ent->client->pers.arms) - G_OC_NumberOfArms(ent->client->pers.armsLastCheckpoint);
			memcpy(ent->client->pers.arms, ent->client->pers.armsLastCheckpoint, (level.totalArmouries + 1) * sizeof(gentity_t *));
			if(lost)
			{
				G_ClientPrint(ent, va("You lost %d armouries!", lost), CLIENT_SPECTATORS);
			}
		}
	}

	return 0;
}

int G_OC_CanUseBonus(gentity_t *ent)
{
	if(!BG_OC_OCMode())
		return 0;

	if(ent->client->pers.hasCheated)
	{
		// silently fail
		return 0;
	}
	if(G_admin_canEditOC(ent))
	{
		// silently fail
		return 0;
	}
	if(BG_InventoryContainsUpgrade(UP_JETPACK, ent->client->ps.stats))
	{
		G_ClientCP(ent, G_OC_NOBONUSJETPACKMESSAGE, NULL, CLIENT_SPECTATORS);
	}
	if(!G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_LUCIJUMP))
	{
		if(BG_InventoryContainsWeapon(WP_LUCIFER_CANNON, ent->client->ps.stats))
		{
			G_ClientCP(ent, G_OC_NOBONUSLCANNONMESSAGE, NULL, CLIENT_SPECTATORS);
		}
	}
	if(BG_InventoryContainsWeapon(WP_FLAMER, ent->client->ps.stats))
	{
		G_ClientCP(ent, G_OC_NOBONUSFLAMERMESSAGE, NULL, CLIENT_SPECTATORS);
	}
	if(ent->client->pers.grenadeUsed)
	{
		G_ClientCP(ent, G_OC_NOBONUSGRENADEMESSAGE, NULL, CLIENT_SPECTATORS);
	}

	return 1;
}

//int G_OC_WeaponIsReserved(weapon_t weapon)
int G_OC_WeaponIsReserved(int weapon)
{
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
		return 0;

	if(level.ocScrimState <= G_OC_STATE_NONE)
		return 0;

//    for(si = level.scrimTeam + 1; si; si = si->next)
	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			if(weapon == si->weapon)
			{
				return 1;
			}
		}
	}

	return 0;
}

int G_OC_WeaponRemoveReserved(gentity_t *ent)
{
	int res = 0;
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
		return 0;

	if(!ent)
		return 0;

	if(!ent->client)
		return 0;

	if(level.ocScrimState <= G_OC_STATE_NONE)
		return 0;

//    for(si = level.scrimTeam + 1; si; si = si->next)
	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			if(ent->client->ps.stats[STAT_WEAPON] == si->weapon)
			{
				ent->client->ps.stats[STAT_WEAPON] = WP_NONE;
				G_ForceWeaponChange(ent, WP_NONE);
				res = 1;
			}
		}
	}

	return res;
}

int G_OC_IsSingleScrim(void)  // are no more than one players per team active?
{
	int i;
	gentity_t *ent;
	int numberOf[G_OC_MAX_SCRIM_TEAMS];

	if(!BG_OC_OCMode())
		return 0;

	// do not check for an active scrim playing because the function can be
	// called to determine how to start a scrim

	for(i = 0; i < G_OC_MAX_SCRIM_TEAMS; i++)
	{
		numberOf[i] = 0;
	}

	for(i = 0; i < level.maxclients; i++)
	{
		ent = &g_entities[i];

		if(ent->client && ent->client->pers.connected == CON_CONNECTED && ent->client->pers.scrimTeam < G_OC_MAX_SCRIM_TEAMS)
			numberOf[ent->client->pers.scrimTeam]++;
	}

	for(i = 1; i < G_OC_MAX_SCRIM_TEAMS; i++)  // skip the NULL oc team
	{
		if(numberOf[i] > 1)
			return 0;
	}

	return 1;
}

int G_OC_EndScrim(void)
{
	int i;
	gentity_t *ent;
	g_oc_scrimTeam_t *si;
	//g_oc_scrimTeam_t *tmp;

	if(!BG_OC_OCMode())
		return 0;

	if(level.ocScrimState <= G_OC_STATE_NONE)
		return 0;

	level.scrimEndTime = 0;

	G_ClientPrint(NULL, "The OC scrim ends", CLIENT_NULL);

	for(i = 0; i < level.maxclients; i++)
	{
		ent = &g_entities[i];

		if(ent->client->pers.scrimTeam)
			G_OC_RestartClient(ent, 1, 1);
	}

//    for(si = level.scrimTeam + 1; si;)
	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			if(!si->time)
			{
				if(level.ocScrimMode == G_OC_MODE_MEDI)
				{
					gentity_t **tmp = BG_Alloc(level.totalMedistations * sizeof(gentity_t *));
					memcpy(tmp, si->medis, level.totalMedistations * sizeof(gentity_t *));
					for(i = 0; i < MAX_CLIENTS; i++)
					{
						ent = g_entities + i;

						if(ent->client && ent->client->pers.connected == CON_CONNECTED && ent->client->pers.scrimTeam == si - level.scrimTeam)
							G_OC_MergeMedis(tmp, ent->client->pers.medis);
					}

					G_ClientPrint(NULL, va("^7%s^2 (%ss^7) loses the OC scrim (%d/%d medical stations)", si->name, G_OC_HumanNameForWeapon(si->weapon), G_OC_NumberOfMedis(tmp), level.totalMedistations), CLIENT_NULL);

					BG_Free(tmp);
				}
				if(level.ocScrimMode == G_OC_MODE_ARM)
				{
					gentity_t **tmp = BG_Alloc(level.totalArmouries * sizeof(gentity_t *));
					memcpy(tmp, si->arms, level.totalArmouries * sizeof(gentity_t *));
					for(i = 0; i < MAX_CLIENTS; i++)
					{
						ent = g_entities + i;

						if(ent->client && ent->client->pers.connected == CON_CONNECTED && ent->client->pers.scrimTeam == si - level.scrimTeam)
							G_OC_MergeArms(tmp, ent->client->pers.arms);
					}

					G_ClientPrint(NULL, va("^7%s^2 (%ss^7) loses the OC scrim (%d/%d armouries)", si->name, G_OC_HumanNameForWeapon(si->weapon), G_OC_NumberOfArms(tmp), level.totalArmouries), CLIENT_NULL);

					BG_Free(tmp);
				}
			}

//            tmp = si->next;
//            BG_Free(si);
//            si = tmp;
			BG_Free(si->medis);
			BG_Free(si->arms);
			si->medis = si->arms = NULL;
			si->active = 0;
		}
	}

	level.ocScrimState = G_OC_STATE_NONE;

	return 0;
}

static g_oc_scrimTeam_t *G_OC_ScrimTeam(char *name)
{
	g_oc_scrimTeam_t *si;
	char teamName[sizeof(level.scrimTeam->name)];
	char buf[sizeof(level.scrimTeam->name)];

	if(!BG_OC_OCMode())
		return NULL;

	if(!name)
		return NULL;

	Q_strncpyz(teamName, name, sizeof(teamName));
	G_SanitiseString(teamName, buf, sizeof(buf));

	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			if(sizeof(si->name) > sizeof(teamName))
				continue;  // wimp out
			G_SanitiseString(si->name, teamName, sizeof(buf));
			if(!strcmp(buf, teamName))
				return si;
		}
	}

	return NULL;
}

static g_oc_scrimTeam_t *G_OC_NewScrimTeam(char *name, weapon_t weapon, char *err, int errlen)
{
	g_oc_scrimTeam_t *si;
	char buf[sizeof(level.scrimTeam->name)];

	if(!BG_OC_OCMode())
		return 0;

	if(!name)
		return 0;

	if(err)
		*err = 0;

	Q_strncpyz(buf, name, sizeof(buf));

	// check if the weapon is valid
	if(!G_OC_ValidScrimWeapon(weapon))
	{
		if(err)
			Q_strncpyz(err, va("invalid weapon %s^7", G_OC_HumanNameForWeapon(weapon)), errlen);
		return NULL;
	}

	// check the name is valid
	if(!G_OC_ValidScrimTeamName(buf))
	{
		if(err)
			Q_strncpyz(err, va("invalid team name %s^7", name), errlen);
		return NULL;
	}

	// does a team with the same name already exist?
	if(G_OC_ScrimTeam(buf))
	{
		if(err)
			Q_strncpyz(err, va("%s^7 already exists", buf), errlen);
		return NULL;
	}

	// iterate through scrim teams and find an available team
	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(!si->active)
		{
			// initialize some stuff
			if(si->medis)
				BG_Free(si->medis);
			if(si->arms)
				BG_Free(si->arms);
			si->checkpoint = NULL;
			si->flags = 0;
			memset(si, 0x00000000, sizeof(g_oc_scrimTeam_t));

			Q_strncpyz(si->name, buf, sizeof(si->name));
			si->weapon = weapon;

			if(level.totalMedistations)
				si->medis = BG_Alloc((level.totalMedistations + 1) * sizeof(gentity_t *));
			if(level.totalArmouries)
				si->arms = BG_Alloc((level.totalArmouries + 1) * sizeof(gentity_t *));
			si->active = 1;
			return si;
		}
	}

	if(err)
		Q_strncpyz(err, va("the maximum number of scrim teams has been reached (%d)", G_OC_MAX_SCRIM_TEAMS), errlen);
	return NULL;
}

//int G_OC_ValidScrimWeapon(weapon_t weapon)
int G_OC_ValidScrimWeapon(int weapon)
{
	if(!BG_OC_OCMode())
		return 0;

	if(!weapon)
		return 0;

	if(weapon == WP_NONE)
		return 0;

	if(G_OC_TeamForWeapon(weapon) != TEAM_HUMANS)
		return 0;

	if(!BG_WeaponAllowedInStage(weapon, g_humanStage.integer) || !BG_WeaponIsAllowed(weapon))
		return 0;

	// don't allow common weapons (blaster, rifle, ckit, etc) or some strange weapons
	if(weapon == WP_BLASTER)
		return 0;

	if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_LUCIJUMP))
	{
		if(weapon == WP_LUCIFER_CANNON)
			return 0;
	}
	else
	{
		if(weapon == WP_MACHINEGUN)
			return 0;
	}

	if(weapon == WP_HBUILD)
		return 0;

//	if(weapon == WP_HBUILD2)
//		return 0;

	if(weapon == WP_LOCKBLOB_LAUNCHER)
		return 0;

	if(weapon == WP_HIVE)
		return 0;

	if(weapon == WP_TESLAGEN)
		return 0;

	if(weapon == WP_MGTURRET)
		return 0;

//    if(weapon == WP_GRENADE)
//        return 0;

	return 1;
}

int G_OC_ValidScrimTeamName(char *name)
{
	if(!BG_OC_OCMode())
		return 0;

	if(!name)
		return 0;

	// team name needs at least two characters
	if(strlen(name) < 2 || !*name)
		return 0;

	if(!strcmp(name, "0"))
	{
		return 0;
	}

	for(; *name; name++)
	{
		if(*name == ';')
			return 0;
		if(*name == '%')
			return 0;
		if(*name == '"')
			return 0;
		if(*name >= 0x7f)
			return 0;
		if(*name < 32)
			return 0;
	}

	return 1;
}

int G_OC_JoinPlayerToScrimTeam(gentity_t *ent, gentity_t *reportEnt, char *teamName, char *weaponName)
{
	g_oc_scrimTeam_t *t;
	char err[MAX_STRING_CHARS];
	weapon_t weapon;

	if(!BG_OC_OCMode())
		return 0;

	if((t = G_OC_ScrimTeam(teamName)))
	{
		// if he's already on a scrim team, remove him from it first
		if(ent->client->pers.scrimTeam)
		{
			G_OC_RemovePlayerFromScrimTeam(ent);
		}

		// existing team
		t->flags |= G_OC_SCRIMFLAG_NOTSINGLETEAM;
		ent->client->pers.scrimTeam = t - level.scrimTeam;
		G_ClientPrint(NULL, va("%s^7 joined scrim team %s^7 (%ss^7)", ent->client->pers.netname, t->name, G_OC_HumanNameForWeapon(t->weapon)), CLIENT_NULL);
	}
	else
	{
		// new team
		weapon = BG_WeaponByName(weaponName)->number;

		if(G_OC_WeaponIsReserved(weapon))
		{
			G_ClientPrint(reportEnt, va("/joinScrim: the %s^7 is already in use by another team", G_OC_HumanNameForWeapon(t->weapon)), CLIENT_NULL);
			return 0;
		}

		if(!(t = G_OC_NewScrimTeam(teamName, weapon, err, sizeof(err))))
		{
			G_ClientPrint(reportEnt, va("/joinScrim: couldn't create scrim team: %s", err), CLIENT_NULL);
			return 0;
		}

		// scrim team is good

		// if he's already on a scrim team, remove him from it first
		if(ent->client->pers.scrimTeam)
		{
			G_OC_RemovePlayerFromScrimTeam(ent);
		}

		ent->client->pers.scrimTeam = t - level.scrimTeam;
		G_ClientPrint(NULL, va("%s^7 created and joined scrim team %s^7 (%ss^7)", ent->client->pers.netname, t->name, G_OC_HumanNameForWeapon(t->weapon)), CLIENT_NULL);
	}

	return 0;
}

int G_OC_RemovePlayerFromScrimTeam(gentity_t *ent)
{
	int i, otherPlayers = 0, otherTeams = 0, scrimTeam;
	gentity_t *client;

	if(!BG_OC_OCMode())
		return 0;

	if(!ent)
		return 0;

	if(!ent->client)
		return 0;

	scrimTeam = ent->client->pers.scrimTeam;

	if(!scrimTeam)
		return 0;

	G_OC_RestartClient(ent, 1, 1);
	G_ClientPrint(NULL, va("%s^7 let team %s^7 down", ent->client->pers.netname, level.scrimTeam[scrimTeam].name), CLIENT_NULL);

	for(i = 0; i < level.maxclients; i++)
	{
		client = &g_entities[i];

		if(client->client->pers.scrimTeam == scrimTeam)
			otherPlayers = 1;
	}

	if(!otherPlayers)
	{
		G_ClientPrint(NULL, va("OC Scrim team %s^7 no longer exists", level.scrimTeam[scrimTeam].name), CLIENT_NULL);
		BG_Free(level.scrimTeam[scrimTeam].medis);
		BG_Free(level.scrimTeam[scrimTeam].arms);
		level.scrimTeam[scrimTeam].checkpoint = NULL;
		level.scrimTeam[scrimTeam].flags  = 0;
		level.scrimTeam[scrimTeam].active = 0;

		for(i = 0; i < level.maxclients; i++)
		{
			client = &g_entities[i];

			if(client->client->pers.scrimTeam)
				otherTeams = 1;
		}


		if(!otherTeams)
		{
			G_OC_EndScrim();
			G_ClientPrint(NULL, "OC Scrim cancelled", CLIENT_NULL);
		}
	}

	return 0;
}

int G_OC_NumScrimTeams(void)
{
	int numTeams = 0;
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
		return 0;

	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
		if(si->active)
			numTeams++;

	return numTeams;
}

int G_OC_TooFewScrimTeams(void)
{
	if(!BG_OC_OCMode())
		return 0;

	if(G_OC_EmptyScrim() || G_OC_NumScrimTeams() < G_OC_MIN_SCRIM_TEAMS)
	{
		return 1;
	}

	return 0;
}

int G_OC_EmptyScrim(void)
{
	int i;
	g_oc_scrimTeam_t *si;
	qboolean t = qfalse, p = qfalse;

	if(!BG_OC_OCMode())
		return 0;

	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			t = qtrue;
		}
	}

	for(i = 0; i < level.maxclients; i++)
	{
		if(g_entities[i].client->pers.scrimTeam)
			p = qtrue;
	}

	if(t != p)
	{
		// something went wrong...
		G_OC_EndScrim();
	}

	return !(t || p);
}

int G_OC_HasScrimFinished(void)  // have all teams beaten the OC?
{
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
		return 0;

//    for(i = level.scrimTeam + 1; i; i = i->next)
	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			if(!si->time)
			{
				return 0;
			}
		}
	}

	return 1;
}

typedef struct funnies_s funnies_t;

static struct funnies_s
{
	char *text;
	char *text2;
	char *text3;
}
funnies[] = 
{
	{"PC's are like air conditioners.  If you open Windows(R) they don't work.", "", ""},
	{"There are 10 kinds of people in this world: those who understand binary, and those who don't.", "", ""},
	{"If I had one dollar for every brain you didn't have, I'd have one dollar.", "", ""},
	{"I'm going to join in and turn this game around 360 degrees!", "", ""},
	{"If we don't succeed, we run the risk of failure.", "", ""},
	{"Man was predestined to have free will.", "", ""},
	{"PC's are like air conditioners.  If you open Windows(R) they don't work.", "", ""},
	{"What's one of the most well known oxymorons?  One on wikipedia: Microsoft Works.", "", ""},
	{"If life is without walls, who needs windows and gates?", "", ""},
	{"A conclusion is simply the place where someone got tired of thinking.  The remarkable thing about Shakespeare is that he really is very good, in spite of all the people who say he is very good.", "", ""},
	{"I remember the time I was kidnapped and they sent a piece of my finger to my father.  He said he wanted more proof.", "", ""},
	{"Man: If I were married to you I'd put poison in your coffee.\nWoman: If I were married to you I'd drink it.", "", ""},
	{"Don't take life too seriously.  You'll never get out of it alive.", "", ""},
	{"I'd rather be pissed off than pissed on.", "", ""},
	{"The future isn't what it used to be.", "", ""},
	{"I know I'm getting better at golf because I'm hitting fewer spectators.", "", ""},
	{"You have a right to your opinions.  I just don't want to hear them.", "", ""},
	{"He who doesn't laugh probably didn't get the joke.", "", ""},
	{"I'm not prejudiced.  I hate everyone equally.", "", ""},
	{"PC's are like air conditioners.  If you open Windows(R) they don't work.", "", ""},
	{"Well, it's no secret that the best thing about a secret is secretly telling someone your secret, thereby adding another secret to their secret collection of secrets, secretly.", "", ""},
	{"Outside of the killings, Washington has one of the lowest crime rates in the country.", "", ""},
	{"Half this game is ninety percent mental.", "", ""},
	{"Traditionally, most of Australia's imports come from overseas.", "", ""},
	{"I stand by all the misstatements that I've made.", "", ""},
	{"Dan Quayle: We're going to have the best-educated American people in the world.", "(Dan Quayle never really said this)", "(Dan Quayle never really said this)"},
	{"The word 'genius' isn't applicable in football.  A genius is a guy like Norman Einstein.", "", ""},
	{"We are ready for any unforeseen event that may or may not occur.", "", ""},
	{"Eating is unhealthy.", "", ""},
	{"<Khalsa> Hi\n<Khalsa> anyone with windows + some smarts around?\n<bob0> doubt it", "", ""},
	{"Learning English embiggens your brain.  What?  It's a perfectly cromulent word.", "", ""},
	{NULL, NULL, NULL}
};

void G_OC_Lol(gentity_t *ent)
{
	funnies_t *f = &funnies[rand() % (sizeof(funnies) / sizeof(funnies[0]))];
	if(f->text && *f->text)
		G_ClientPrint(ent, f->text, CLIENT_SPECTATORS);
	if(f->text2 && *f->text2)
		G_ClientCP(ent, f->text2, NULL, CLIENT_NULL | CLIENT_NEVERREPLACE);
	if(f->text3 && *f->text3)
		G_ClientCP(ent, f->text3, NULL, CLIENT_SPECTATORS | CLIENT_NOTARGET | CLIENT_NEVERREPLACE);
}


//// STATS ////

static void G_SanitiseNameWhitespaceColor(char *in, char *out, int len)
{
	qboolean skip = qtrue;
	int spaces = 0;

	len--;

	while(*in && len > 0)
	{
		// strip leading white space
		if(*in == ' ')
		{
			if(skip)
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

//		if(Q_IsColorString(in))
//		{
//			in += 2;    // skip color code
//			continue;
//		}

		if(*in < 32)
		{
			in++;
			continue;
		}

		if(*in == 0x01)
		{
			in++;
			continue;
		}

		*out++ = tolower(*in++);
		len--;
	}
	out -= spaces;
	*out = 0;
}

static int G_OC_CompareStats(stat_t *a, stat_t *b)
{
	if(a->count != b->count)
		return b->count - a->count;

	if(a->time != b->time)
		return a->time  - b->time;

	return 0;
}

static qboolean G_OC_StatsEqual(stat_t *a, stat_t *b)
{
	// return *a == *b

	if(a->count != b->count)
		return qfalse;

	if(a->time  != b->time)
		return qfalse;

	if(strcmp(a->name,      b->name)      != 0)
		return qfalse;

	if(strcmp(a->date,      b->date)      != 0)
		return qfalse;

	if(strcmp(a->guid,      b->guid)      != 0)
		return qfalse;

	if(strcmp(a->ip,        b->ip)        != 0)
		return qfalse;

	if(strcmp(a->adminName, b->adminName) != 0)
		return qfalse;

	return qtrue;
}

static qboolean G_OC_SameGuy(stat_t *a, stat_t *b)
{
	if(a->guid      == b->guid)
		return qtrue;

	if(a->ip        == b->ip)
		return qtrue;

	if(a->adminName == b->adminName)
		return qtrue;

	return qfalse;
}

/*
=================
G_OC_Stats

Possible insert record into stats
================
*/
static char *G_OC_Stats(const char *filename, gclient_t *client, int count, int time)
{
	fileHandle_t f;
	char line[ MAX_STRING_CHARS ];
	int numRecords;
	stat_t records[G_OC_STAT_MAXRECORDS + 1];  // there needs to be room for one more slot so that the new record can be compared
	char *stat, *statHead;
	int i, len;
	stat_t *r;
	stat_t currentRecord;
	int record;

	/// Userinfo variables ///
	char name[MAX_NAME_LENGTH] = {""};
	char realName[MAX_NAME_LENGTH] = {""};  // realname
	char date[MAX_CVAR_VALUE_STRING] = {""};
	qtime_t qt;
	int t;
	char *ip;
	char userinfo[MAX_INFO_STRING];
	char pureName[MAX_NAME_LENGTH] = {""};  // tmp
	char cleanName[MAX_NAME_LENGTH] = {""};  // tmp
	qboolean firstNewlineReached = qfalse;

	/// stats disabled? ///
	if(!g_statsEnabled.integer || g_statsRecords.integer <= 0 || g_statsRecords.integer >= G_OC_STAT_MAXRECORDS)
		return "";

	/// other checks ///
	if(!BG_OC_OCMode() || !level.layout || !*level.layout)
		return "";

	if(g_cheats.integer)
	{
		trap_SendServerCommand(client - level.clients, "print \"Cannot store record with cheats enable\n\"");

		return "";
	}

	numRecords = g_statsRecords.integer;
	if(numRecords < 0)
		return "";
	if(numRecords >= G_OC_STAT_MAXRECORDS)
		numRecords = G_OC_STAT_MAXRECORDS;

	/// set userdata stuff ///

	G_SanitiseString(client->pers.netname, cleanName, sizeof(cleanName));
	realName[0] = '\0';
	pureName[0] = '\0';
	cleanName[0] = '\0';
	for(i = 0; i < MAX_ADMIN_ADMINS && g_admin_admins[i]; i++)
	{
		if(!Q_stricmp(g_admin_admins[i]->guid, client->pers.guid))
		{
			//l = g_admin_admins[i]->level;
			G_SanitiseString(g_admin_admins[i]->name, pureName, sizeof(pureName));
			if(Q_stricmp(cleanName, pureName))
			{
				Q_strncpyz(realName, g_admin_admins[i]->name, sizeof(realName));
			}
			break;
		}
	}
	i = 0;
	if(realName[0] && pureName[0])
		strcpy(realName, pureName);
	else if(cleanName[0])
		strcpy(realName, cleanName);
	else
		strcpy(realName, "noname");

	trap_GetUserinfo(client - level.clients, userinfo, sizeof(userinfo));
	ip = Info_ValueForKey(userinfo, "ip");
	if(!ip)
		return " - ip error";

	t = trap_RealTime(&qt);
	trap_Cvar_VariableStringBuffer("gamedate", date, sizeof(date));
	strcat(date, va(" %d:%02i", qt.tm_hour, qt.tm_min));

	// set name
	G_SanitiseNameWhitespaceColor(client->pers.netname, name, sizeof(name));

	/// load stats file ///
	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if( len < 0 )
	{
		trap_FS_FCloseFile(f);
		if(trap_FS_FOpenFile(filename, &f, FS_APPEND) < 0)
		{
			trap_FS_FCloseFile(f);
			G_LogPrintf("G_OC_Stats: could not open %s\n", filename);
			return "";
		}
		else
		{
			trap_FS_FCloseFile(f);
			len = trap_FS_FOpenFile(filename, &f, FS_READ);
		}
	}
	statHead = stat = BG_Alloc(len + 1);
	trap_FS_Read(stat, len, f);
	stat[len] = '\0';
	trap_FS_FCloseFile(f);

	i = record = 0;
	while( *stat )
	{
		if( i >= sizeof( line ) - 1 )
		{
			G_LogPrintf( S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
			filename, line );

			BG_Free(statHead);

			return "";
		}

		line[i++] = *stat;
		line[i] = '\0';
		if(*stat == '\n' && record < numRecords)
		{
			if(firstNewlineReached)  // ignore the header of the file (first line specifying number of arms and medis for /stats)
			{
				r = &records[record++];
				char buf[MAX_STRING_CHARS];
				char *p;

				strncpy(buf, line, sizeof(buf));

				memset(r, 0, sizeof(stat_t));

				#define LOADBUF \
				do \
				{ \
					p = buf; \
 \
					while(*p > 0x01 && p - buf < sizeof(buf) - 1) \
					{ \
						buf[p - buf] = *p; \
 \
						p++; \
					} \
 \
					buf[p - buf] = 0; \
				} while(0)

				// count
				LOADBUF;
				r->count = atoi(buf);

				// time
				LOADBUF;
				r->time  = atoi(buf);

				// name
				LOADBUF;
				Com_sprintf(r->name, sizeof(r->name), "%s", buf);

				// date
				LOADBUF;
				Com_sprintf(r->date, sizeof(r->date), "%s", buf);

				// guid
				LOADBUF;
				Com_sprintf(r->guid, sizeof(r->guid), "%s", buf);

				// ip
				LOADBUF;
				Com_sprintf(r->ip, sizeof(r->ip), "%s", buf);

				// adminName
				LOADBUF;
				Com_sprintf(r->adminName, sizeof(r->adminName), "%s", buf);

				#undef LOADBUF
			}
			else
			{
				firstNewlineReached = qtrue;
			}
		}

		++stat;
	}

	/// add record ///
	r = &records[record++];
	Com_sprintf(stat, sizeof(stat), "%d%c%d%c%s%c%s%c%s%c%s%c%s\n", count, '\1', time, '\1', name, '\1', date, '\1', client->pers.guid, '\1', ((ip) ? (ip) : ("noip")), '\1', realName);

	r->count = count;
	r->time  = time;
	Com_sprintf(r->name, sizeof(r->name), "%s", name);
	Com_sprintf(r->date, sizeof(r->date), "%s", date);
	Com_sprintf(r->date, sizeof(r->date), "%s", client->pers.guid);
	Com_sprintf(r->date, sizeof(r->date), "%s", ((ip) ? (ip) : ("noip")));
	Com_sprintf(r->name, sizeof(r->name), "%s", realName);

	memcpy(&currentRecord, r, sizeof(currentRecord));

	qsort(records, record + 1, MAX_STRING_CHARS, (int(*)()) G_OC_CompareStats);

	/// remove the worse stats from duplicate users ///
	for(i = 0; i < record; i++)
	{
		int j;

		for(j = i + 1; j < record; j++)
		{
			stat_t *a = &records[i], *b = &records[j];

			if(G_OC_SameGuy(a, b))
			{
				if(G_OC_CompareStats(a, b) > 0)
				{
					// remove b
					memmove(b - 1, b, numRecords - (b - records));

					--record;
				}
				else
				{
					// remove a
					memmove(a, a + 1, numRecords - 1 - (a - records));

					--record;
				}
			}
		}
	}

	record = MAX(record, numRecords - 1);  // truncate

	#define WRITETHING(s) trap_FS_Write((s), strlen((s)), f)

	#define WRITEFILE \
	do \
	{ \
		char numString[MAX_STRING_CHARS]; \
		int j; \
		i = 0; \
 \
		len = trap_FS_FOpenFile(filename, &f, FS_WRITE); \
		if(len < 0) \
		{ \
			G_Printf("G_OC_Stats: could not open %s\n", filename); \
			return ""; \
		} \
 \
		strncpy(numString, va("%d %d\n", level.totalArmouries, level.totalMedistations), sizeof(numString)); \
 \
		G_Printf("G_OC_Stats: saving stats to %s\n", filename); \
 \
		trap_FS_Write(numString, strlen(numString), f); \
 \
		/* Write file */ \
		for(j = 0; j < record; j++) \
		{ \
			r = &records[j]; \
 \
			WRITETHING(va("%d\x01%d\x01%s\x01%s\x01%s\x01%s\x01%s\n", r->count, r->time, r->name, r->date, r->guid, r->ip, r->adminName)); \
		} \
 \
		trap_FS_FCloseFile(f); \
	} while(0)

	/// Finish up ///
	for(i = 0; i < record; i++)
	{
		r = &records[i];

		if(G_OC_StatsEqual(r, &currentRecord))
		{
			static char s[MAX_STRING_CHARS];

			WRITEFILE;

			Com_sprintf(s, MAX_STRING_CHARS, " ^2New Record!: #%d^7", record + 1);

			return s;
		}
	}

	if(g_alwaysSaveStats.integer)
	{
		WRITEFILE;
	}

	#undef WRITEFILE
	#undef WRITETHING

	return "";
}

/*
=================
G_OC_MediStats

Medi stats
================
*/
char *G_OC_MediStats(void *client, int count, int time)
{
	char map[MAX_QPATH];

	trap_Cvar_VariableStringBuffer("mapname", map, sizeof(map));
	if(!map[0])
	{
		G_LogPrintf("G_OC_MediStats: no map is loaded\n");
		return "";
	}
	G_StrToLower(level.layout);

	return G_OC_Stats(va("stats/%s/%s/med.dat", map, level.layout), (gclient_t *) client, count, time);
}

/*
=================
G_OC_WinStats

Armoury stats
================
*/
char *G_OC_WinStats(void *client, int count, int time)
{
	char map[MAX_QPATH];

	trap_Cvar_VariableStringBuffer("mapname", map, sizeof(map));
	if(!map[0])
	{
		G_LogPrintf("G_OC_WinStats: no map is loaded\n");
		return "";
	}
	G_StrToLower(level.layout);

	// if ONEARM is set, always treat count as 1
	if(G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
		count = 1;

	return G_OC_Stats(va("stats/%s/%s/win.dat", map, level.layout), (gclient_t *) client, count, time);
}


/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f(gentity_t *ent)
{
	fileHandle_t f;
	int arms = 0;
	int medis = 0;
	int	len, i=0, j=0;
	char arg1[MAX_STRING_TOKENS];
	char arg2[MAX_STRING_TOKENS];
	char *statsWin, *statsMedi, *statsWinPtr, *statsMediPtr;
	char *linePtr;
//	char *linePtr2;
	char map[MAX_QPATH];
	char layout[MAX_QPATH];
	char fileName[MAX_OSPATH];
	char line[MAX_STRING_CHARS];
	char name[MAX_STRING_CHARS];
	char dateTime[MAX_STRING_CHARS];
	char data[MAX_STRING_CHARS];
	int	score, record, count;

	if(!BG_OC_OCMode())
		return;

	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));

	if(level.ocLoadTime || !ent || !ent->client)
		return;

	if(strchr(arg1, ';') || strchr(arg2, ';') || strchr(arg1, '/') || strchr(arg2, '/') || strchr(arg1, '\\') || strchr(arg2, '\\') || strchr(arg1, '\"') || strchr(arg2, '\"') || strchr(arg1, '.') || strchr(arg2, '.') || strchr(arg1, '\n') || strchr(arg2, '\n'))
	{
		trap_SendServerCommand(ent-g_entities, "print \"Invalid stat string\n\"");
		return;
	}

	if(trap_Argc() > 4 || trap_Argc() < 1)
	{
		ADMP("stats: Usage:\n/stats: see current oc stats\n/stats mapname: see mapname with layout oc's stats\n/stats mapname layoutname: see mapname with layoutname's stats\n/stats mapname layoutname more: a third argument will give more details of records");
		return;
	}

	if(trap_Argc() < 2)
		trap_Cvar_VariableStringBuffer("mapname", map, sizeof(map));
	else if(trap_Argc() <= 4)
		Q_strncpyz(map, arg1, sizeof(map));
	else
		return;
	if(trap_Argc() == 3 || trap_Argc() == 4)
		Q_strncpyz(layout, arg2, sizeof(layout));
	else if(trap_Argc() == 2)
		strcpy(layout, "oc");
	else if(trap_Argc() < 2)
		Q_strncpyz(layout, level.layout, sizeof(layout));
	else
		return;
	Com_sprintf(fileName, sizeof(fileName), "stats/%s/%s/win.dat", map, layout);

	len = trap_FS_FOpenFile(fileName, &f, FS_READ);
	if(len < 0)
	{
		trap_FS_FCloseFile(f);
		if(level.totalArmouries <= 0 && trap_Argc() <= 2)
		{
				if(trap_FS_FOpenFile(fileName, &f, FS_APPEND) < 0)
				{
						trap_FS_FCloseFile(f);
						ADMP("stat: No records (needs at least 1 medi record)\n");
						return;
				}
				else
				{
					trap_FS_Write(va("%d %d\n", level.totalArmouries, level.totalMedistations), strlen(va("%d %d\n", level.totalArmouries, level.totalMedistations)), f);
					trap_FS_FCloseFile(f);
					len = trap_FS_FOpenFile(fileName, &f, FS_READ);
				}
		}
		else
		{
			ADMP("stat: No records (needs at least 1 medi and arm record)\n");
			return;
		}
	}
	statsWin = BG_Alloc(len + 1);
	statsWinPtr = statsWin;
	trap_FS_Read(statsWin, len, f);
	*(statsWin + len) = '\0';
	trap_FS_FCloseFile(f);

	Com_sprintf(fileName, sizeof(fileName), "stats/%s/%s/med.dat", map, layout);

	len = trap_FS_FOpenFile(fileName, &f, FS_READ);
	if(len < 0)
	{
		trap_FS_FCloseFile(f);
		if(level.totalMedistations <= 0 && !level.ocLoadTime && trap_Argc() <= 2)
		{
				if(trap_FS_FOpenFile(fileName, &f, FS_APPEND) < 0)
				{
						trap_FS_FCloseFile(f);
						ADMP("stat: No records (needs at least 1 arm record)\n");
						return;
				}
				else
				{
					trap_FS_Write(va("%d %d\n", level.totalArmouries, level.totalMedistations), strlen(va("%d %d\n", level.totalArmouries, level.totalMedistations)), f);
					trap_FS_FCloseFile(f);
					len = trap_FS_FOpenFile(fileName, &f, FS_READ);
				}
		}
		else
		{
			ADMP("stat: No records (needs at least 1 medi and arm record)\n");
			return;
		}
	}
	statsMedi = BG_Alloc(len + 1);
	statsMediPtr = statsMedi;
	trap_FS_Read(statsMedi, len, f);
	*(statsMedi + len) = '\0';
	trap_FS_FCloseFile(f);

	data[0] = i = j = 0;
	while(*statsWinPtr > 1 && *statsWinPtr != ' ' && *statsWinPtr != '\n' && i < MAX_STRING_CHARS)	// parse total arms
	{
		data[i++] = *statsWinPtr++;
		data[i] = 0;
	} statsWinPtr++; i = j = 0;
	while(*statsWinPtr > 1 && *statsWinPtr != ' ' && *statsWinPtr != '\n' && i < MAX_STRING_CHARS) statsWinPtr++;	// skip total medis
	;;statsWinPtr++; i = j = 0;
	arms = atoi(data);

	data[0] = i = j = 0;
	while(*statsMediPtr > 1 && *statsMediPtr != ' ' && *statsMediPtr != '\n' && i < MAX_STRING_CHARS) statsMediPtr++;	// skip total arms
	;;statsMediPtr++; i = j = 0;
	while(*statsMediPtr > 1 && *statsMediPtr != ' ' && *statsMediPtr != '\n' && i < MAX_STRING_CHARS)	// parse total medis
	{
		data[i++] = *statsMediPtr++;
		data[i] = 0;
	} statsMediPtr++; i = j = 0;

	medis = atoi(data);

	if(medis > 0)
	{
		if(trap_Argc() < 4)
			trap_SendServerCommand(ent - g_entities, va("print \"%s %s Most Medical Stations Used\n\n---\nName - Count - Time\n\n\"", map, layout));
		else
			trap_SendServerCommand(ent - g_entities, va("print \"%s %s Most Medical Stations Used\n\n---\nName - Count - Time - Date\n\n\"", map, layout));
		i = 0;
		record = 0;
		while(*statsMediPtr)
		{
			if(i >= sizeof(line) - 1)
			{
				BG_Free(statsWin);
				BG_Free(statsMedi);
				G_Printf(S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
				 va("stats/%s/%s/med.dat", map, layout), line);
				return;
			}
			line[i++] = *statsMediPtr;
			line[i] = '\0';
			if(*statsMediPtr == '\n')
			{
				i = j = 0;
//				sscanf(line, "%d %d", &count, &score);
				linePtr = line;
				count = atoi(linePtr++);
				while(*linePtr && *linePtr > 1) linePtr++;
				if(*linePtr) linePtr++;
				score = atoi(linePtr);
				linePtr = line;
//				linePtr2 = line;	// originally used to put a terminator after name
				for(i = 0; i < 2; i++) // twice to skip first two numbers
				{
					while(*linePtr && *linePtr >	1)
					{
						linePtr++;
					}
					while(*linePtr && *linePtr <= 1)
					{
						linePtr++;
					}
//					while(*linePtr2 && *linePtr2 != '\n')
//					{
//						linePtr2++;
//					}
//					while(*linePtr2 && *linePtr2 == '\n')
//					{
//						*(linePtr2++) = '\0';
//					}
				} i = 0;
				name[0] = '\0';
				dateTime[0] = '\0';
				while(i + 1 < MAX_STRING_CHARS && linePtr[i] > 1)
				{
					name[i] = linePtr[i];
					i++;
					name[i] = '\0';
				}
				if(linePtr[i++])
				while(j + 1 < MAX_STRING_CHARS && i + 1 < MAX_STRING_CHARS && linePtr[i] > 1)
				{
					dateTime[j++] = linePtr[i++];
					dateTime[j] = '\0';
				} i = j = 0;
				record++;
				if(trap_Argc() < 4)
//, score / 60000, (score - ((score / 60000) * 60000)) / 1000, score - ((score / 1000) * 1000))
					trap_SendServerCommand(ent - g_entities, va("print \"^7#^7%d^7: ^7%s^7 - %d/%d ^7%dm:%ds:%dms^7\n\"", record, name, count, medis, MINS(score), SECS(score), MSEC(score)));
				else
					trap_SendServerCommand(ent - g_entities, va("print \"^7#^7%d^7: ^7%s^7 - %d/%d ^7%dm:%ds:%dms^7 - %s^7\n\"", record, name, count, medis, MINS(score), SECS(score), MSEC(score), dateTime));
			}
			statsMediPtr++;
		}
	}
	else
	{
		trap_SendServerCommand(ent-g_entities, va("print \"No Medical Stations\n\""));
	}

	if(arms > 0)
	{
		if(trap_Argc() < 4)
			trap_SendServerCommand(ent - g_entities, va("print \"\n\n\n%s %s Best Winning Times\n\n---\nName - Time\n\n\"", map, layout));
		else
			trap_SendServerCommand(ent - g_entities, va("print \"\n\n\n%s %s Best Winning Times\n\n---\nName - Time - Date\n\n\"", map, layout));

		i = 0;
		record = 0;

		while(*statsWinPtr)
		{
			if(i >= sizeof(line) - 1)
			{
				BG_Free(statsWin);
				BG_Free(statsMedi);
				G_Printf(S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
				 va("stats/%s/%s/win.dat", map, layout), line);
				return;
			}
			line[i++] = *statsWinPtr;
			line[i] = '\0';
			if(*statsWinPtr == '\n')
			{
				i = j = 0;
//				sscanf(line, "%d %d", &count, &score);
				linePtr = line;
				count = atoi(linePtr++);
				while(*linePtr && *linePtr > 1) linePtr++;
				if(*linePtr) linePtr++;
				score = atoi(linePtr);
				linePtr = line;
//				linePtr2 = line;	// originally used to put a terminator after name
				for(i = 0; i < 2; i++) // twice to skip first two numbers
				{
					while(*linePtr && *linePtr >	1)
					{
						linePtr++;
					}
					while(*linePtr && *linePtr <= 1)
					{
						linePtr++;
					}
//					while(*linePtr2 && *linePtr2 != '\n')
//					{
//						linePtr2++;
//					}
//					while(*linePtr2 && *linePtr2 == '\n')
//					{
//						*(linePtr2++) = '\0';
//					}
				} i = 0;
				name[0] = '\0';
				dateTime[0] = '\0';
				while(i + 1 < MAX_STRING_CHARS && linePtr[i] > 1)
				{
					name[i] = linePtr[i];
					i++;
					name[i] = '\0';
				}
				if(linePtr[i++])
				while(j + 1 < MAX_STRING_CHARS && i + 1 < MAX_STRING_CHARS && linePtr[i] > 1)
				{
					dateTime[j++] = linePtr[i++];
					dateTime[j] = '\0';
				} i = j = 0;
				record++;
				if(trap_Argc() < 4)
					trap_SendServerCommand(ent - g_entities, va("print \"^7#^7%d^7: ^7%s^7 - ^7%dm:%ds:%dms^7\n\"", record, name, MINS(score), SECS(score), MSEC(score)));
				else
					trap_SendServerCommand(ent - g_entities, va("print \"^7#^7%d^7: ^7%s^7 - ^7%dm:%ds:%dms^7 - %s^7\n\"", record, name, MINS(score), SECS(score), MSEC(score), dateTime));
			}
			statsWinPtr++;
		}
	}
	else
	{
		trap_SendServerCommand(ent-g_entities, va("print \"No Armouries\n\""));
	}

	BG_Free(statsMedi);
	BG_Free(statsWin);
}

/*
=================
Cmd_Mystats_f
=================
*/
void Cmd_Mystats_f(gentity_t *ent)
{
	gentity_t *client;
	char color[MAX_STRING_CHARS];
	int percent, i;

	if(!BG_OC_OCMode())
		return;

	if(level.ocLoadTime || !ent || !ent->client)
		return;
	G_ClientPrint(ent, "Your Obstacle Course Information--", CLIENT_NULL);
	if(ent->client->pers.scrimTeam)
	{
		if(level.ocScrimState > G_OC_STATE_PREP)
		{
				if(level.ocScrimMode == G_OC_MODE_ARM && level.totalArmouries && level.scrimTeam[ent->client->pers.scrimTeam].arms)
				{
					gentity_t **tmp = BG_Alloc(level.totalArmouries * sizeof(gentity_t *));
					memcpy(tmp, level.scrimTeam[ent->client->pers.scrimTeam].arms, level.totalArmouries * sizeof(gentity_t *));	// memcpy should be faster than merge itself
					for(i = 0; i < MAX_CLIENTS; i++)
					{
							client = g_entities + i;

							if(client->client && client->client->pers.scrimTeam == ent->client->pers.scrimTeam)
								G_OC_MergeArms(tmp, client->client->pers.arms);
					}

					percent = (int)(100 * (G_OC_NumberOfArms(tmp)) / (level.totalArmouries));
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

					if(level.totalArmouries == 1 || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
					{
							if(G_OC_NumberOfArms(tmp))
							{
								G_ClientPrint(ent, va("Armouries: ^2Win^7 - %dm%ds%dms)", MINS(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer)), SECS(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer)), MSEC(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer))), CLIENT_NULL);
							}
							else
							{
								G_ClientPrint(ent, "Armouries: ^1None", CLIENT_NULL);
							}
					}
					else if(G_OC_AllArms(ent->client->pers.arms))
					{
							G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent) - ^2%dm%ds%dms", G_OC_NumberOfArms(tmp), level.totalArmouries, color, percent, MINS(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer)), SECS(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer)), MSEC(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer))), CLIENT_NULL);
					}
					else
					{
						G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent)", G_OC_NumberOfArms(tmp), level.totalArmouries, color, percent), CLIENT_NULL);
					}

					BG_Free(tmp);
				}
				else if(level.ocScrimMode == G_OC_MODE_MEDI && level.totalMedistations && level.scrimTeam[ent->client->pers.scrimTeam].medis)
				{
					gentity_t **tmp = BG_Alloc(level.totalMedistations * sizeof(gentity_t *));
					memcpy(tmp, level.scrimTeam[ent->client->pers.scrimTeam].medis, level.totalMedistations * sizeof(gentity_t *));	// memcpy should be faster than merge itself
					for(i = 0; i < MAX_CLIENTS; i++)
					{
							client = g_entities + i;

							if(client->client && client->client->pers.scrimTeam == ent->client->pers.scrimTeam)
									G_OC_MergeMedis(tmp, client->client->pers.medis);
					}

					percent = (int)(100 * (G_OC_NumberOfMedis(tmp)) / (level.totalMedistations));
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

					G_ClientPrint(ent, va("Medical Stations: %d/%d (%s%d^7 percent)%s", G_OC_NumberOfMedis(tmp), level.totalMedistations, color, percent, G_OC_AllMedis(tmp) ? va(" - ^2%dm%ds%dms", MINS(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer)), SECS(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer)), MSEC(level.scrimTeam[ent->client->pers.scrimTeam].time - (level.ocStartTime + g_ocWarmup.integer))) : ("")), CLIENT_NULL);

					BG_Free(tmp);
			}
		}
		return;
	}
	else
	{
		if(level.totalArmouries && ent->client->pers.arms)
		{
			percent = (int)(100 * (G_OC_NumberOfArms(ent->client->pers.arms)) / (level.totalArmouries));
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

			if(level.totalArmouries == 1 || G_OC_TestLayoutFlag(level.layout, G_OC_OCFLAG_ONEARM))
			{
				if(G_OC_NumberOfArms(ent->client->pers.arms))
				{
						G_ClientPrint(ent, va("Armouries: ^2Win^7 - %dm%ds%dms)", MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime)), CLIENT_NULL);
				}
				else
				{
						G_ClientPrint(ent, "Armouries: ^1None", CLIENT_NULL);
				}
			}
			else if(G_OC_AllArms(ent->client->pers.arms))
			{
				G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent) - ^2%dm%ds%dms", G_OC_NumberOfArms(ent->client->pers.arms), level.totalArmouries, color, percent, MINS(ent->client->pers.winTime), SECS(ent->client->pers.winTime), MSEC(ent->client->pers.winTime)), CLIENT_NULL);
			}
			else
			{
				G_ClientPrint(ent, va("Armouries: %d/%d (%s%d^7 percent)", G_OC_NumberOfArms(ent->client->pers.arms), level.totalArmouries, color, percent), CLIENT_NULL);
			}
		}
		if(level.totalMedistations && ent->client->pers.medis)
		{
			percent = (int)(100 * (G_OC_NumberOfMedis(ent->client->pers.medis)) / (level.totalMedistations));
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

			G_ClientPrint(ent, va("Medical Stations: %d/%d (%s%d^7 percent)%s", G_OC_NumberOfMedis(ent->client->pers.medis), level.totalMedistations, color, percent, G_OC_AllMedis(ent->client->pers.medis) ? va(" - ^2%dm%ds%dms", MINS(ent->client->pers.mediTime), SECS(ent->client->pers.mediTime), MSEC(ent->client->pers.mediTime)) : ("")), CLIENT_NULL);
		}
	}
}

/*
=================
Cmd_Groupup_f
=================
*/
void Cmd_Groupup_f(gentity_t *ent)
{
	int count = 0, i;
	vec3_t forward, end;
	trace_t tr;
	gentity_t *traceEnt, *countEnt;

	if(!BG_OC_OCMode())
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorMA(ent->client->ps.origin, 100, forward, end);

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID);
	traceEnt = &g_entities[tr.entityNum];

	if(tr.fraction < 1.0f && (traceEnt->s.eType == ET_BUILDABLE) && G_admin_canEditOC(ent))
	{
		traceEnt->groupID++;
		if(traceEnt->s.modelindex == BA_H_SPAWN && traceEnt->groupID >= level.numNodes)
			traceEnt->groupID = 0;
		for(i = 1, countEnt = g_entities + i; i < level.num_entities; countEnt++, i++)
		{
			if(countEnt->s.modelindex == BA_H_SPAWN && countEnt->groupID == traceEnt->groupID && countEnt->powered && !(countEnt->health <= 0) && countEnt->spawned)
			{
					count++;
			}
		}
		if(traceEnt->s.modelindex == BA_H_SPAWN || traceEnt->s.modelindex == BA_A_SPAWN)
			trap_SendServerCommand(ent-g_entities, va("print \"Structure grouped as %d (%d total telenodes in group)%s\n\"", traceEnt->groupID, count, ((traceEnt->groupID) ? ("") : (" (spawning)"))));
		else
			trap_SendServerCommand(ent-g_entities, va("print \"Structure grouped as %d%s\n\"", traceEnt->groupID, traceEnt->groupID == 2 ? " (unpowered)" : (traceEnt->groupID == 1 ? " (powered)" : (traceEnt->groupID == 0 ? " (default behaviour)" : " (undefined behaviour)"))));
	}
}

/*
=================
Cmd_Groupdown_f
=================
*/
void Cmd_Groupdown_f(gentity_t *ent)
{
	int count = 0, i;
	vec3_t forward, end;
	trace_t tr;
	gentity_t *traceEnt, *countEnt;

	if(!BG_OC_OCMode())
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorMA(ent->client->ps.origin, 100, forward, end);

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID);
	traceEnt = &g_entities[tr.entityNum];

	if(tr.fraction < 1.0f && (traceEnt->s.eType == ET_BUILDABLE) && G_admin_canEditOC(ent))
	{
		traceEnt->groupID--;
		if(traceEnt->groupID < 0)
			traceEnt->groupID = ((traceEnt->s.modelindex == BA_H_SPAWN) ? (level.numNodes - 1) : (0));
		for(i = 1, countEnt = g_entities + i; i < level.num_entities; countEnt++, i++)
		{
			if(countEnt->s.modelindex == BA_H_SPAWN && countEnt->groupID == traceEnt->groupID && countEnt->powered && !(countEnt->health <= 0) && countEnt->spawned)
			{
				count++;
			}
		}
		if(traceEnt->s.modelindex == BA_H_SPAWN || traceEnt->s.modelindex == BA_A_SPAWN)
			trap_SendServerCommand(ent-g_entities, va("print \"Structure grouped as %d (%d total telenodes in group)%s\n\"", traceEnt->groupID, count, ((traceEnt->groupID) ? ("") : (" (spawning)"))));
		else
			trap_SendServerCommand(ent-g_entities, va("print \"Structure grouped as %d%s\n\"", traceEnt->groupID, traceEnt->groupID == 2 ? " (unpowered)" : (traceEnt->groupID == 1 ? " (powered)" : (traceEnt->groupID == 0 ? " (default behaviour)" : " (undefined behaviour)"))));
	}
}

/*
=================
Cmd_Group_f
=================
*/
void Cmd_Group_f(gentity_t *ent)
{
	int count = 0, i;
	vec3_t forward, end;
	trace_t tr;
	gentity_t *traceEnt, *countEnt;

	if(!BG_OC_OCMode())
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorMA(ent->client->ps.origin, 100, forward, end);

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID);
	traceEnt = &g_entities[tr.entityNum];

	if(tr.fraction < 1.0f && (traceEnt->s.eType == ET_BUILDABLE) && G_admin_canEditOC(ent))
	{
		for(i = 1, countEnt = g_entities + i; i < level.num_entities; countEnt++, i++)
		{
			if(countEnt->s.modelindex == BA_H_SPAWN && countEnt->groupID == traceEnt->groupID && countEnt->powered && !(countEnt->health <= 0) && countEnt->spawned)
			{
				count++;
			}
		}
		if(traceEnt->s.modelindex == BA_H_SPAWN || traceEnt->s.modelindex == BA_A_SPAWN)
			trap_SendServerCommand(ent-g_entities, va("print \"Structure grouped as %d (%d total telenodes in group)%s\n\"", traceEnt->groupID, count, ((traceEnt->groupID) ? ("") : (" (spawning)"))));
		else
			trap_SendServerCommand(ent-g_entities, va("print \"Structure grouped as %d%s\n\"", traceEnt->groupID, traceEnt->groupID == 2 ? " (unpowered)" : (traceEnt->groupID == 1 ? " (powered)" : (traceEnt->groupID == 0 ? " (default behaviour)" : " (undefined behaviour)"))));
	}
}

void Cmd_RestartOC_f(gentity_t *ent)
{
	if(!ent)
	{
		ADMP("Cannot be run as console\n");
		return;
	}

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(ent->client->pers.scrimTeam)
	{
		ADMP("You cannot restart in a scrim\n");
		return;
	}

	G_OC_RestartClient(ent, 0, 1);
}

void Cmd_LeaveScrim_f(gentity_t *ent)
{
	if(!ent)
	{
		ADMP("Cannot be run as console\n");
		return;
	}

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(!ent->client->pers.scrimTeam)
	{
		ADMP("You are not on a scrim team\n");
		return;
	}

	G_OC_RemovePlayerFromScrimTeam(ent);
}

void Cmd_JoinScrim_f(gentity_t *ent)
{
	char *teamName;
	char weaponName[MAX_STRING_CHARS];

	if(!ent)
	{
		ADMP("Cannot be run as console\n");
		return;
	}

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(ent->client->pers.scrimTeam)
	{
		ADMP("You have already joined a scrim team\n");
		return;
	}

	if(level.ocScrimState > G_OC_STATE_NONE)
	{
		ADMP("The scrim has already started\n");
		return;
	}

	if(trap_Argc() < 3)
	{
		ADMP(va("Usage: /joinScrim [weaponIfNewTeam] [team]\n"));
		return;
	}

	teamName = ConcatArgs(2);
	trap_Argv(1, weaponName, sizeof(weaponName));
	G_OC_JoinPlayerToScrimTeam(ent, ent, teamName, weaponName);
}

void Cmd_ListScrim_f(gentity_t *ent)
{
	int i;
	int id;
	gentity_t *client;
	char buf[MAX_STRING_CHARS] = {""}, *tmp = "";
	g_oc_scrimTeam_t *si;

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	switch(level.ocScrimState)
	{
		case G_OC_STATE_NONE:
			tmp = "None";
		case G_OC_STATE_PREP:
			tmp = "Initializing";
		case G_OC_STATE_WARM:
			tmp = "Warmup";
		case G_OC_STATE_PLAY:
			tmp = "Actice - playing";
		default:
			tmp = "Unknown";
	}

	Q_strcat(buf, sizeof(buf), va("Scrim state: %s\n", tmp));

	switch(level.ocScrimMode)
	{
		case G_OC_MODE_MEDI:
			tmp = "Medi scrim";
		case G_OC_MODE_ARM:
			tmp = "Arm scrim";
		default:
			tmp = "None";
	}

	Q_strcat(buf, sizeof(buf), va("Scrim mode: %s\n", tmp));

	Q_strcat(buf, sizeof(buf), "\n");

	for(si = level.scrimTeam + 1; si < level.scrimTeam + G_OC_MAX_SCRIM_TEAMS; si++)
	{
		if(si->active)
		{
			qboolean comma = qfalse, first = qtrue;

			id = si - level.scrimTeam;

			Q_strcat(buf, sizeof(buf), "^1-^2-^6-^4-^2-^4-^1-^2-^6-^4-^2-^4-^2\\_\n^7");
			Q_strcat(buf, sizeof(buf), "^1-^2-^6-^4-^2-^4-^1-^2-^6-^4-^2-^4-^2\\__\n\n^7");

			Q_strcat(buf, sizeof(buf), va("^2Team name: %s\n", si->name));
			Q_strcat(buf, sizeof(buf), va("^2Weapon: %s\n", G_OC_HumanNameForWeapon(si->weapon)));
			Q_strcat(buf, sizeof(buf), "^2Players: ");
			// if were to list number of medis and arms, doing it here would be a good idea, but listing them in a global list command like this is not a good idea.  At the most, list them only if their joined to that team, but that's superflous because /mystats can be used
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				client = g_entities + i;

				if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam)
				{
//                    if(client->client->pers.scrimTeam == id)  // commenting this out causes the format to be consistent, so if one player has a comma the entire format is changed.  This is good for consitency.
					{
						if(strchr(client->client->pers.netname, ','))
						{
							comma = qtrue;
						}
					}
				}
			}

			// iterate again now that we know which format to use
			if(comma)
			{
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					client = g_entities + i;

					if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam)
					{
						if(client->client->pers.scrimTeam == id)
						{
							if(!first)
							{
								Q_strcat(buf, sizeof(buf), ", ");
							}
							first = qfalse;

							Q_strcat(buf, sizeof(buf), va("^7%s^7", ent->client->pers.netname));
						}
					}
				}
			}
			else
			{
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					client = g_entities + i;

					if(client->client && client->client->pers.connected == CON_CONNECTED && client->client->pers.scrimTeam)
					{
						if(client->client->pers.scrimTeam == id)
						{
							if(!first)
							{
								Q_strcat(buf, sizeof(buf), "\n");
							}
							first = qfalse;

							Q_strcat(buf, sizeof(buf), va("  - ^7%s^7\n^7", ent->client->pers.netname));
						}
					}
				}
			}

			Q_strcat(buf, sizeof(buf), "\n\n");
		}
	}

	G_ClientPrint(ent, buf, CLIENT_NULL);
}

void Cmd_BuildableOverride_f(gentity_t *ent)
{
	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	ent->client->pers.buildableOverride = !ent->client->pers.buildableOverride;
	if(ent->client->pers.buildableOverride)
	{
		G_ClientPrint(ent, "Buildable Override enabled", CLIENT_NULL);
	}
	else
	{
		G_ClientPrint(ent, "Buildable Override disabled", CLIENT_NULL);
	}
}

void Cmd_Hide_f(gentity_t *ent)
{
	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(!g_allowHiding.integer)
	{
		ADMP("Server disabled non-admin hiding\n");
		return;
	}

	if(level.time > ent->client->pers.hiddenTime)
	{
		ent->client->pers.hidden = !ent->client->pers.hidden;
		if(ent->client->pers.hidden)
		{
			G_StopFromFollowing(ent, 0);
			ent->r.svFlags |= SVF_SINGLECLIENT;
			ent->r.singleClient = ent-g_entities;
			ADMP("You have been hidden\n");
		}
		else
		{
			ent->r.svFlags &= ~SVF_SINGLECLIENT;
			ADMP("You have become visible\n");
		}
	}
	else
	{
		ADMP(va("You can't hide yourself.  Expires in %d seconds.\n", (ent->client->pers.hiddenTime - level.time) / 1000));
	}
}

void Cmd_TestHidden_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH];
	char cmd[MAX_STRING_CHARS];
	char err[MAX_STRING_CHARS];
	int found = 0;
	gentity_t *vic;

	trap_Argv(0, cmd, sizeof(cmd));

	if(trap_Argc() < 2)
	{
		G_ClientPrint(ent, va("%s: usage \\%s [clientNum | partial name match]", cmd, cmd), CLIENT_NULL);
		return;
	}

	G_SayArgv(1, name, sizeof(name));
	if((found = G_ClientNumbersFromString(name, pids, MAX_CLIENTS)) != 1)
	{
		G_MatchOnePlayer(pids, found, err, sizeof(err));
		G_ClientPrint(ent, va("%s: ^7%s", cmd, err), CLIENT_NULL);
		return;
	}

	vic = &g_entities[pids[0]];

	if(vic->client->pers.hidden)
		G_ClientPrint(ent, va("%s: player ^2is^7 hidden", cmd), CLIENT_NULL);
	else
		G_ClientPrint(ent, va("%s: player ^3is not^7 hidden", cmd), CLIENT_NULL);
}

void Cmd_QuickRestartOC_f(gentity_t *ent)
{
	if(!ent)
	{
		ADMP("Cannot be run as console\n");
		return;
	}

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(ent->client->pers.scrimTeam)
	{
		ADMP(va("You cannot restart in a scrim\n"));
		return;
	}

	G_OC_RestartClient(ent, 1, 1);
}

void Cmd_AskLayout_f(gentity_t *ent)
{
	static char layout[MAX_STRING_CHARS];

	if(!ent)
	{
		ADMP("Cannot be run as console\n");
		return;
	}

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(trap_Argc() > 1)
	{
		strncpy(layout, level.layout, sizeof(layout));
	}
	else
	{
		trap_Argv(1, layout, sizeof(layout));
	}

	G_ClientPrint(ent, G_OC_ParseLayoutFlags(layout), CLIENT_NULL);
}

void Cmd_TeleportToCheckpoint_f(gentity_t *ent)
{
	gentity_t *dest;
	gentity_t *checkpoint;
	vec3_t spawn_origin, spawn_angles;

	if(!ent)
	{
		ADMP("Cannot be run as console\n");
		return;
	}

	if(!ent->client)
	{
		return;
	}

	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	checkpoint = ent->client->pers.checkpoint;
	if(ent->client->pers.scrimTeam)
	{
		g_oc_scrimTeam_t *t;
		G_OC_GETTEAM(t, level.scrimTeam, ent->client->pers.scrimTeam);

		checkpoint = t->checkpoint;
	}

	if(!checkpoint)
		return;

	// simulate player's death
	G_OC_PlayerDie(ent);

	if(ent->client->pers.teamSelection == TEAM_HUMANS)
	{
		if((dest = G_SelectHumanSpawnPoint(ent->s.origin, ent, 0, NULL)))
		{
			VectorCopy(dest->s.origin, spawn_origin);
			if(!ent->client->pers.autoAngleDisabled)
			{
				VectorCopy(dest->s.angles, spawn_angles);
				VectorInverse(spawn_angles);
			}
			else
			{
				VectorCopy(ent->s.angles, spawn_angles);
			}
			if(G_CheckSpawnPoint(dest->s.number, dest->s.origin, dest->s.origin2, BA_A_BOOSTER, spawn_origin) == NULL)
			{
				TeleportPlayer(ent, spawn_origin, spawn_angles);
				VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);  // no velocity after teleportation
			}
		}
	}
	else if(ent->client->pers.teamSelection == TEAM_ALIENS)
	{
		if((dest = G_SelectAlienSpawnPoint(ent->s.origin, ent, 0, NULL)))
		{
			VectorCopy(dest->s.angles, spawn_angles);
			VectorInverse(spawn_angles);
		}
		else
		{
			VectorCopy(ent->s.angles, spawn_angles);
		}
		if(G_CheckSpawnPoint(dest->s.number, dest->s.origin, dest->s.origin2, BA_A_BOOSTER, spawn_origin) == NULL)
		{
			TeleportPlayer(ent, spawn_origin, spawn_angles);
			VectorScale(ent->client->ps.velocity, 0.0, ent->client->ps.velocity);
		}
	}
}

void Cmd_AutoAngle_f(gentity_t *ent)
{
	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(!ent || !ent->client)
	{
		return;
	}

	if(ent->client->pers.autoAngleDisabled)
		G_ClientPrint(ent, "AutoAngle enabled.", CLIENT_NULL);
	else
		G_ClientPrint(ent, "AutoAngle already enabled.", CLIENT_NULL);
	ent->client->pers.autoAngleDisabled = 0;
}

void Cmd_AutoUnAngle_f(gentity_t *ent)
{
	if(!BG_OC_OCMode())
	{
		ADMP("Can only be used during an obstacle course\n");
		return;
	}

	if(!ent || !ent->client)
	{
		return;
	}

	if(!ent->client->pers.autoAngleDisabled)
		G_ClientPrint(ent, "AutoAngle disabled.", CLIENT_NULL);
	else
		G_ClientPrint(ent, "AutoAngle already disabled.", CLIENT_NULL);
	ent->client->pers.autoAngleDisabled = 1;
}

//======================================================
// OC flags
//======================================================

/*
==================
G_OC_ParseLayoutFlags

Used to cat onto vote strings
==================
*/

char *G_OC_ParseLayoutFlags(char *layout)
{
	int  num = 0;
	static char ret[MAX_STRING_CHARS];

	static char out[4 * MAX_STRING_CHARS];

	out[0] = 0;

//	if(!BG_OC_OCMode())
//		return;
	if(!layout || !layout[0] || *(layout) != 'o' || *((layout) + 1) != 'c')  // must be an oc
		return "";

	if(!G_OC_LayoutExtraFlags(layout))  // no extra flags
		return "";

	strcpy(ret, " (layout uses options '");

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ONEARM))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ONEARM_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_NOCREEP))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_NOCREEP_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ALIENONLY))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ALIENONLY_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_BOTHTEAMS))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_BOTHTEAMS_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_NOWALLWALK))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_NOWALLWALK_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AGRANGER))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_AGRANGER_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AGRANGERUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_AGRANGERUPG_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ADRETCH))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ADRETCH_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ABASILISK))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ABASILISK_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ABASILISKUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ABASILISKUPG_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AMARAUDER))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_AMARAUDER_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AMARAUDERUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_AMARAUDERUPG_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ADRAGOON))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ADRAGOON_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ADRAGOONUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ADRAGOONUPG_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ATYRANT))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_ATYRANT_NAME);
	}

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_LUCIJUMP))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, G_OC_OCFLAG_LUCIJUMP_NAME);
	}

	strcat(ret, "')");

	return ret;
}

/*
==================
G_OC_TestLayoutFlag

Similar to G_admin_permission
==================
*/

qboolean G_OC_TestLayoutFlag(char *layout, char *flag)
{
	char *flagPtr = flag;  // the flag to test
	char *flags   = layout;  // the layout to test

	G_StrToLower(flags);
	G_StrToLower(layout);

//	if(!BG_OC_OCMode())
//		return qfalse;
	if(!layout || !layout[0] || *(layout) != 'o' || *((layout) + 1) != 'c')  // must be an oc
		return qfalse;

	flags += strlen("oc");

	while(*flags)
	{
		if(*flags == '_')
		{
			break;
		}
		else if(*flags == *flagPtr)
		{
			if(*flagPtr != '^')
				return qtrue;
			while(*flags == '^' && *(flags++) == *(flagPtr++))
				if(*flags == '_')
					break;
			if(*flags == '_')
				break;
			if(*flags == *flagPtr && *flags != '_' && *flagPtr != '_' && *flags != '^')
				return qtrue;
			else
				break;
		}
		else if(*flags == '^')
		{
			while(*(flags++) == '^')
				if(*flags == '_')
					break;
			continue;
		}

		flags++;
	}

	return qfalse;
}

/*
==================
G_OC_LayoutExtraFlags

Test for any defined flags
==================
*/

qboolean G_OC_LayoutExtraFlags(char *layout)
{
	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ALIENONLY))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ONEARM))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_NOCREEP))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_BOTHTEAMS))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_NOWALLWALK))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AGRANGER))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AGRANGERUPG))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ADRETCH))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ABASILISK))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_LUCIJUMP))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ABASILISKUPG))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AMARAUDER))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_AMARAUDERUPG))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ADRAGOON))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ADRAGOONUPG))
		return qtrue;

	if(G_OC_TestLayoutFlag(layout, G_OC_OCFLAG_ATYRANT))
		return qtrue;

	return qfalse;
}
