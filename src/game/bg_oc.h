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

#ifndef _G_OC_H
#define _G_OC_H

#ifdef _TREMULOUS_H
#error tremulous.h was included before g_oc.h
#endif /* ifndef _TREMULOUS_H */

#define BG_OCMode() ((oc_gameMode) ? (1) : (0))

// some constants need to be defined at compile time
#define TREMULOUS_VALUE(d, o) ((BG_OCMode()) ? (o) : (d))

extern int oc_gameMode;

#define BG_OCPmove_jump() ((BG_OCMode()) ? (1) : (1))  // TODO: let height be lost

#ifdef OC_GAME
#endif /* ifdef OC_GAME */

#ifdef OC_CGAME
#endif /* ifdef OC_CGAME */

#endif /* ifndef _G_OC_H */
