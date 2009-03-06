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

enum
{
	gameMode_none,
	gameMode_OC,

	gameMode_NULL
};

// several states or flags that aren't used only by game
int oc_gameMode = gameMode_none;  // this is global to help speed up some common checks
static int oc_heightNeverLost = 0;  // if the layout being played has a certain flag then this will be set.
static int oc_noWallWalk = 0;  // if the layout being played has a certain flag then this will be set.

//======================================================
// oc mode
//======================================================

void BG_OC_SetOCModeNone(void)
{
	oc_gameMode = gameMode_none;
}

void BG_OC_SetOCModeOC(void)
{
	oc_gameMode = gameMode_OC;
}

int BG_OC_GetOCMode(void)  // should use faster BG_OC_OCMode() instead
{
	return oc_gameMode;
}

//======================================================
// height never lost
//======================================================

void BG_OC_SetHeightNeverLost(int c)
{
	oc_heightNeverLost = c;
}

int BG_OC_GetHeightNeverLost(void)
{
	return oc_heightNeverLost;
}

//======================================================
// no wallwalk
//======================================================

void BG_OC_SetWallWalk(int c)
{
	oc_noWallWalk = c;
}

int BG_OC_GetNoWallWalk(void)
{
	return oc_noWallWalk;
}

