/*
===========================================================================
Copyright (C) 2009 Byron Johnson

This file is part of Tremfusion.

Tremfusion is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremfusion is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremfusion; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#define OC_CGAME
#define OC_GAME
#define OC_BGAME

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "g_local.h"
#include "bg_oc.h"

enum
{
	gameMode_none,
	gameMode_OC,

	gameMode_NULL
};

int oc_gameMode = gameMode_OC;
int oc_heightNeverLost = 0;  // if the layout being played has a certain flag then this will be set.
