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
 * cg_oc.c
 *
 * OC code for cgame
 *
 * Differences in tremulous source:
 * Vote count moved before vote string
 */

#include "cg_local.h"

void CG_OCMode_f(void)
{
	if(trap_Argc() < 2)
		return;

	if(atoi(CG_Argv(1)))
		BG_OC_SetOCModeOC();
	else
		BG_OC_SetOCModeNone();
}