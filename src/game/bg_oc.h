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

#define BG_OC_OCMode() ((oc_gameMode) ? (1) : (0))

// some constants need to be defined at compile time
#define TREMULOUS_VALUE(d, o) ((BG_OC_OCMode()) ? (o) : (d))

extern int oc_gameMode;
extern int oc_heightNeverLost;

#ifdef OC_GAME
	// game only stuff
#endif /* ifdef OC_GAME */

#ifdef OC_CGAME
	// cgame only stuff
#endif /* ifdef OC_CGAME */

#if defined(OC_CGAME) || defined(OC_GAME) || defined(OC_BGAME)
	// either game or cgame

	#define BG_OC_PMOCWallJump() 0
	#define BG_OC_PMOCGroundTraceWallJump() BG_OC_OCMode()
	#define BG_OC_PMOCPounce() 0

//	#define BG_OC_PMZeroJump() ((BG_OC_OCMode()) ? ((pm->ps->velocity[ 2 ] <= BG_Class( pm->ps->stats[ STAT_CLASS ] )->jumpMagnitude * JUMP_OC_ZERO_HEIGHT_MODIFIER) ? (0) : (1)): (1))
	#define BG_OC_PMZeroJump() ((BG_OC_OCMode()) ? ((oc_heightNeverLost) ? (1) : (0)) : (1))

	#define BG_OC_PMCheckWallJump() \
	{ \
		if( !( BG_Class( pm->ps->stats[ STAT_CLASS ] )->abilities & SCA_WALLJUMPER ) ) \
			return qfalse; \
 \
		if( pm->ps->pm_flags & PMF_RESPAWNED ) \
			return qfalse;		/* don't allow jump until all buttons are up */ \
 \
		if( pm->cmd.upmove < 10 ) \
			/* not holding jump */ \
			return qfalse; \
 \
		if( pm->ps->pm_flags & PMF_TIME_WALLJUMP ) \
			return qfalse; \
 \
		/* must wait for jump to be released */ \
		if( pm->ps->pm_flags & PMF_JUMP_HELD && \
				pm->ps->grapplePoint[ 2 ] == 1.0f ) \
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
		ProjectPointOnPlane( forward, pml.forward, pm->ps->grapplePoint ); \
		ProjectPointOnPlane( right, pml.right, pm->ps->grapplePoint ); \
 \
		VectorScale( pm->ps->grapplePoint, normalFraction, dir ); \
 \
		if( pm->cmd.forwardmove > 0 ) \
			VectorMA( dir, cmdFraction, forward, dir ); \
		else if( pm->cmd.forwardmove < 0 ) \
			VectorMA( dir, -cmdFraction, forward, dir ); \
 \
		if( pm->cmd.rightmove > 0 ) \
			VectorMA( dir, cmdFraction, right, dir ); \
		else if( pm->cmd.rightmove < 0 ) \
			VectorMA( dir, -cmdFraction, right, dir ); \
 \
		VectorMA( dir, upFraction, refNormal, dir ); \
		VectorNormalize( dir ); \
 \
		VectorMA( pm->ps->velocity, BG_Class( pm->ps->stats[ STAT_CLASS ] )->jumpMagnitude, \
							dir, pm->ps->velocity ); \
 \
		/* for a long run of wall jumps the velocity can get pretty large, this caps it */ \
		if( VectorLength( pm->ps->velocity ) > LEVEL2_WALLJUMP_MAXSPEED ) \
		{ \
			VectorNormalize( pm->ps->velocity ); \
			VectorScale( pm->ps->velocity, LEVEL2_WALLJUMP_MAXSPEED, pm->ps->velocity ); \
		} \
 \
		PM_AddEvent( EV_JUMP ); \
 \
		if( pm->cmd.forwardmove >= 0 ) \
		{ \
			if( !( pm->ps->persistant[ PERS_STATE ] & PS_NONSEGMODEL ) ) \
				PM_ForceLegsAnim( LEGS_JUMP ); \
			else \
				PM_ForceLegsAnim( NSPA_JUMP ); \
 \
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; \
		} \
		else \
		{ \
			if( !( pm->ps->persistant[ PERS_STATE ] & PS_NONSEGMODEL ) ) \
				PM_ForceLegsAnim( LEGS_JUMPB ); \
			else \
				PM_ForceLegsAnim( NSPA_JUMPBACK ); \
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
		if( BG_ClassHasAbility( pm->ps->stats[ STAT_CLASS ], SCA_WALLJUMPER ) ) \
		{ \
			ProjectPointOnPlane( movedir, pml.forward, refNormal ); \
			VectorNormalize( movedir ); \
 \
			if( pm->cmd.forwardmove < 0 ) \
				VectorNegate( movedir, movedir ); \
 \
			/* allow strafe transitions */ \
			if( pm->cmd.rightmove ) \
			{ \
				VectorCopy( pml.right, movedir ); \
 \
				if( pm->cmd.rightmove < 0 ) \
					VectorNegate( movedir, movedir ); \
			} \
 \
			/* trace into direction we are moving */ \
			VectorMA( pm->ps->origin, 0.25f, movedir, point ); \
			pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask ); \
 \
			/* if( trace.fraction < 1.0f && !( trace.surfaceFlags & ( SURF_SKY | SURF_SLICK ) ) && */ \
			/*     ( trace.entityNum == ENTITYNUM_WORLD ) ) */ \
			if( trace.fraction < 1.0f && \
				!( trace.surfaceFlags & ( SURF_SKY | SURF_SLICK ) ) ) \
			{ \
				if( !VectorCompare( trace.plane.normal, pm->ps->grapplePoint ) ) \
				{ \
					VectorCopy( trace.plane.normal, pm->ps->grapplePoint ); \
					/* PM_CheckWallJump( ); */ \
 \
					/* a return-friendly version of BG_OC_PMCheckWallJump() */ \
					{ \
						vec3_t  dir, forward, right; \
						vec3_t  refNormal = { 0.0f, 0.0f, 1.0f }; \
						float   normalFraction = 1.5f; \
						float   cmdFraction = 1.0f; \
						float   upFraction = 1.5f; \
 \
						if( !( BG_Class( pm->ps->stats[ STAT_CLASS ] )->abilities & SCA_WALLJUMPER ) ) \
							return; \
 \
						if( pm->ps->pm_flags & PMF_RESPAWNED ) \
							return;		/* don't allow jump until all buttons are up */ \
 \
						if( pm->cmd.upmove < 10 ) \
							/* not holding jump */ \
							return; \
 \
						if( pm->ps->pm_flags & PMF_TIME_WALLJUMP ) \
							return; \
 \
						/* must wait for jump to be released */ \
						if( pm->ps->pm_flags & PMF_JUMP_HELD && \
								pm->ps->grapplePoint[ 2 ] == 1.0f ) \
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
						ProjectPointOnPlane( forward, pml.forward, pm->ps->grapplePoint ); \
						ProjectPointOnPlane( right, pml.right, pm->ps->grapplePoint ); \
 \
						VectorScale( pm->ps->grapplePoint, normalFraction, dir ); \
 \
						if( pm->cmd.forwardmove > 0 ) \
							VectorMA( dir, cmdFraction, forward, dir ); \
						else if( pm->cmd.forwardmove < 0 ) \
							VectorMA( dir, -cmdFraction, forward, dir ); \
 \
						if( pm->cmd.rightmove > 0 ) \
							VectorMA( dir, cmdFraction, right, dir ); \
						else if( pm->cmd.rightmove < 0 ) \
							VectorMA( dir, -cmdFraction, right, dir ); \
 \
						VectorMA( dir, upFraction, refNormal, dir ); \
						VectorNormalize( dir ); \
 \
						VectorMA( pm->ps->velocity, BG_Class( pm->ps->stats[ STAT_CLASS ] )->jumpMagnitude, \
											dir, pm->ps->velocity ); \
 \
						/* for a long run of wall jumps the velocity can get pretty large, this caps it */ \
						if( VectorLength( pm->ps->velocity ) > LEVEL2_WALLJUMP_MAXSPEED ) \
						{ \
							VectorNormalize( pm->ps->velocity ); \
							VectorScale( pm->ps->velocity, LEVEL2_WALLJUMP_MAXSPEED, pm->ps->velocity ); \
						} \
 \
						PM_AddEvent( EV_JUMP ); \
 \
						if( pm->cmd.forwardmove >= 0 ) \
						{ \
							if( !( pm->ps->persistant[ PERS_STATE ] & PS_NONSEGMODEL ) ) \
								PM_ForceLegsAnim( LEGS_JUMP ); \
							else \
								PM_ForceLegsAnim( NSPA_JUMP ); \
 \
							pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; \
						} \
						else \
						{ \
							if( !( pm->ps->persistant[ PERS_STATE ] & PS_NONSEGMODEL ) ) \
								PM_ForceLegsAnim( LEGS_JUMPB ); \
							else \
								PM_ForceLegsAnim( NSPA_JUMPBACK ); \
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
		if( pm->ps->weapon == WP_ALEVEL3 ) \
			jumpMagnitude = pm->ps->stats[ STAT_MISC ] * LEVEL3_POUNCE_JUMP_MAG / LEVEL3_POUNCE_TIME; \
		else \
			jumpMagnitude = pm->ps->stats[ STAT_MISC ] * LEVEL3_POUNCE_JUMP_MAG_UPG / LEVEL3_POUNCE_TIME_UPG; \
	}
#endif /* if defined OC_CGAME || defined OC_GAME */

#endif /* ifndef _G_OC_H */
