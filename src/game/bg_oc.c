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

#define OC_BGAME

#include "bg_oc.h"
#include "bg_public.h"

enum
{
	gameMode_none,
	gameMode_OC,

	gameMode_NULL
};

// several states or flags that aren't used only by game
int oc_gameMode = gameMode_none;
static int oc_noWallWalk = 0;  // if the layout being played has a certain flag then this will be set.

//======================================================
// oc mode
//======================================================

void BG_OC_SetOCModeNone(void)
{
	oc_gameMode = gameMode_none;

	// reload overrides, so the client doesn't stick with old class .cfg files
	// for GPP configs, but instead let it re-initialize with the new OC mode
	// config strings, so the client has the same (correct) old configs
	// (1.1-style) as the server for OC mode.
	BG_InitClassConfigs( );
	BG_InitBuildableConfigs( );
	BG_InitWeaponConfigs( );
	BG_InitAllowedGameElements( );
}

void BG_OC_SetOCModeOC(void)
{
	oc_gameMode = gameMode_OC;

	// reload overrides, so the client doesn't stick with old class .cfg files
	// for GPP configs, but instead let it re-initialize with the new OC mode
	// config strings, so the client has the same (correct) old configs
	// (1.1-style) as the server for OC mode.
	BG_InitClassConfigs( );
	BG_InitBuildableConfigs( );
	BG_InitWeaponConfigs( );
	BG_InitAllowedGameElements( );
}

int BG_OC_GetOCMode(void)  // should use faster BG_OC_OCMode() instead
{
	return oc_gameMode;
}

//======================================================
// no wallwalk
//======================================================

void BG_OC_SetNoWallWalk(int c)
{
	oc_noWallWalk = c;
}

int BG_OC_GetNoWallWalk(void)
{
	if(!BG_OC_GetOCMode())
	{
		return 0;
	}

	return oc_noWallWalk;
}

//======================================================
// OC flags
//======================================================

/*
==================
G_OC_ParseLayoutFlags

Human readable options string
==================
*/

const char *BG_OC_ParseLayoutFlags(char *layout)
{
	int num = 0;
	static char ret[MAX_STRING_CHARS];

	ret[0] = 0;

	// Don't check for OC mode since this function can be called in non-OC mode

	if(!layout || !layout[0] || *(layout) != 'o' || *((layout) + 1) != 'c')  // must be an oc
		return "";

	if(!BG_OC_LayoutExtraFlags(layout))  // no extra flags
		return "";

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ONEARM))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ONEARM_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_NOCREEP))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_NOCREEP_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_HUMANS))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_HUMANS_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_NOWALLWALK))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_NOWALLWALK_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AGRANGER))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_AGRANGER_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AGRANGERUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_AGRANGERUPG_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRETCH))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ADRETCH_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ABASILISK))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ABASILISK_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ABASILISKUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ABASILISKUPG_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AMARAUDER))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_AMARAUDER_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AMARAUDERUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_AMARAUDERUPG_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRAGOON))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ADRAGOON_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRAGOONUPG))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ADRAGOONUPG_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ATYRANT))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_ATYRANT_NAME);
	}

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_LUCIJUMP))
	{
		if(num++)
			strcat(ret, ", ");
		strcat(ret, BG_OC_OCFLAG_LUCIJUMP_NAME);
	}

	return ret;
}

/*
==================
BG_OC_TestLayoutFlag

Similar to OC v1.4's G_admin_permission
==================
*/

int BG_OC_TestLayoutFlag(char *layout, const char *flag)
{
	const char *flagPtr = flag;  // the flag to test
	const char *flags   = layout;  // the layout to test

	//BG_StrToLower(flags);
	BG_StrToLower(layout);

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
BG_OC_LayoutExtraFlags

Test if any flags exist for a layout
==================
*/

int BG_OC_LayoutExtraFlags(char *layout)
{
	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ONEARM))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_NOCREEP))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_HUMANS))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_NOWALLWALK))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AGRANGER))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AGRANGERUPG))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRETCH))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ABASILISK))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_LUCIJUMP))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ABASILISKUPG))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AMARAUDER))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AMARAUDERUPG))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRAGOON))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRAGOONUPG))
		return qtrue;

	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ATYRANT))
		return qtrue;

	return qfalse;
}

int BG_OC_Aliens(char *layout)
{
	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AGRANGER) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AGRANGERUPG) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRETCH) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ABASILISK) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ABASILISKUPG) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AMARAUDER) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_AMARAUDERUPG) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRAGOON) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ADRAGOONUPG) || BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_ATYRANT))
		return qtrue;

	return qfalse;
}

int BG_OC_Humans(char *layout)
{
	if(BG_OC_TestLayoutFlag(layout, BG_OC_OCFLAG_HUMANS))
		return qtrue;
	if(!BG_OC_Aliens(layout))
		return qtrue;

	return qfalse;
}
