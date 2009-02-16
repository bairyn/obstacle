/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

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


/*
 * ALIEN weapons
 *
 * _REPEAT  - time in msec until the weapon can be used again
 * _DMG     - amount of damage the weapon does
 *
 * ALIEN_WDMG_MODIFIER - overall damage modifier for coarse tuning
 *
 */

#include "g_oc.h"

#define _TREMULOUS_H

#ifdef _G_OC_H
#define TREMULOUS_VALUE(d, o) ((G_OCMode()) ? (d) : (o))
#else
#define TREMULOUS_VALUE(d, o) (d)
#endif


#define ALIEN_WDMG_MODIFIER         1.0f
#define ADM(d)                      ((int)((float)d*ALIEN_WDMG_MODIFIER))

#define ABUILDER_BUILD_REPEAT       TREMULOUS_VALUE(500, 500)
#define ABUILDER_CLAW_DMG           TREMULOUS_VALUE(ADM(20), ADM(20))
#define ABUILDER_CLAW_RANGE         TREMULOUS_VALUE(64.0f, 64.0f)
#define ABUILDER_CLAW_WIDTH         TREMULOUS_VALUE(4.0f, 4.0f)
#define ABUILDER_CLAW_REPEAT        TREMULOUS_VALUE(1000, 1000)
#define ABUILDER_CLAW_K_SCALE       TREMULOUS_VALUE(1.0f, 1.0f)
#define ABUILDER_BLOB_DMG           TREMULOUS_VALUE(ADM(4), ADM(4))
#define ABUILDER_BLOB_REPEAT        TREMULOUS_VALUE(1000, 1000)
#define ABUILDER_BLOB_SPEED         TREMULOUS_VALUE(800.0f, 800.0f)
#define ABUILDER_BLOB_SPEED_MOD     TREMULOUS_VALUE(0.5f, 0.5f)
#define ABUILDER_BLOB_TIME          TREMULOUS_VALUE(5000, 5000)

#define LEVEL0_BITE_DMG             TREMULOUS_VALUE(ADM(36), ADM(48))
#define LEVEL0_BITE_RANGE           TREMULOUS_VALUE(64.0f, 64.0f)
#define LEVEL0_BITE_WIDTH           TREMULOUS_VALUE(6.0f, 6.0f)
#define LEVEL0_BITE_REPEAT          TREMULOUS_VALUE(700, 500)
#define LEVEL0_BITE_K_SCALE         TREMULOUS_VALUE(1.0f, 1.0f)

#define LEVEL1_CLAW_DMG             TREMULOUS_VALUE(ADM(32), ADM(32))
#define LEVEL1_CLAW_RANGE           TREMULOUS_VALUE(64.0f, 96.0f)
#define LEVEL1_CLAW_WIDTH           TREMULOUS_VALUE(10.0f, 10.0f)
#define LEVEL1_CLAW_REPEAT          TREMULOUS_VALUE(600, 600)
#define LEVEL1_CLAW_U_REPEAT        TREMULOUS_VALUE(500, 500)
#define LEVEL1_CLAW_K_SCALE         TREMULOUS_VALUE(1.0f, 1.0f)
#define LEVEL1_CLAW_U_K_SCALE       TREMULOUS_VALUE(1.0f, 1.0f)
#define LEVEL1_GRAB_RANGE           TREMULOUS_VALUE(96.0f, 96.0f)
#define LEVEL1_GRAB_TIME            TREMULOUS_VALUE(300, 300)
#define LEVEL1_GRAB_U_TIME          TREMULOUS_VALUE(300, 300)
#define LEVEL1_PCLOUD_DMG           TREMULOUS_VALUE(ADM(4), ADM(4))
#define LEVEL1_PCLOUD_RANGE         TREMULOUS_VALUE(120.0f, 200.0f)
#define LEVEL1_PCLOUD_REPEAT        TREMULOUS_VALUE(2500, 2000)
#define LEVEL1_PCLOUD_TIME          TREMULOUS_VALUE(10000, 10000)
#define LEVEL1_REGEN_MOD            TREMULOUS_VALUE(2.0f, 2.0f)
#define LEVEL1_UPG_REGEN_MOD        TREMULOUS_VALUE(3.0f, 3.0f)

#define LEVEL2_CLAW_DMG             TREMULOUS_VALUE(ADM(40), ADM(40))
#define LEVEL2_CLAW_RANGE           TREMULOUS_VALUE(80.0f, 96.0f)
#define LEVEL2_CLAW_WIDTH           TREMULOUS_VALUE(14.0f, 12.0f)
#define LEVEL2_CLAW_REPEAT          TREMULOUS_VALUE(500, 500)
#define LEVEL2_CLAW_K_SCALE         TREMULOUS_VALUE(1.0f, 1.0f)
#define LEVEL2_CLAW_U_REPEAT        TREMULOUS_VALUE(400, 400)
#define LEVEL2_CLAW_U_K_SCALE       TREMULOUS_VALUE(1.0f, 1.0f)
#define LEVEL2_AREAZAP_DMG          TREMULOUS_VALUE(ADM(40), ADM(80))
#define LEVEL2_AREAZAP_RANGE        TREMULOUS_VALUE(120.0f, 200.0f)
#define LEVEL2_AREAZAP_CUTOFF       TREMULOUS_VALUE(300.0f, 300.0f)
#define LEVEL2_AREAZAP_REPEAT       TREMULOUS_VALUE(500, 1500)
#define LEVEL2_AREAZAP_MAX_TARGETS  TREMULOUS_VALUE(5, 3)
#define LEVEL2_WALLJUMP_MAXSPEED    TREMULOUS_VALUE(1000.0f, 1000.0f)
#define LEVEL2_WALLJUMP_NORMAL      TREMULOUS_VALUE(1.0, 1.0)      // magnitude scale from surface
#define LEVEL2_WALLJUMP_FORWARD     TREMULOUS_VALUE(1.5, 1.5)      // magnitude scale in view direction
#define LEVEL2_WALLJUMP_UP          TREMULOUS_VALUE(0.0, 0.0)      // magnitude scale up
#define LEVEL2_WALLJUMP_REPEAT      TREMULOUS_VALUE(400, 400)      // msec before new jump
#define LEVEL2_WALLJUMP_RANGE       TREMULOUS_VALUE(8.0, 8.0)      // how far away the wall can be

#define LEVEL3_CLAW_DMG             TREMULOUS_VALUE(ADM(80), ADM(80))
#define LEVEL3_CLAW_RANGE           TREMULOUS_VALUE(72.0f, 96.0f)
#define LEVEL3_CLAW_UPG_RANGE       TREMULOUS_VALUE(LEVEL3_CLAW_RANGE, LEVEL3_CLAW_RANGE) + 6.0f          
#define LEVEL3_CLAW_WIDTH           TREMULOUS_VALUE(12.0f, 16.0f)
#define LEVEL3_CLAW_REPEAT          TREMULOUS_VALUE(700, 700)
#define LEVEL3_CLAW_K_SCALE         TREMULOUS_VALUE(1.0f, 1.0f)
#define LEVEL3_CLAW_U_REPEAT        TREMULOUS_VALUE(600, 600)
#define LEVEL3_CLAW_U_K_SCALE       TREMULOUS_VALUE(1.0f, 1.0f)
#define LEVEL3_POUNCE_DMG           TREMULOUS_VALUE(ADM(100), ADM(100))
#define LEVEL3_POUNCE_RANGE         TREMULOUS_VALUE(54.0f, 72.0f)
#define LEVEL3_POUNCE_UPG_RANGE     TREMULOUS_VALUE(LEVEL3_POUNCE_RANGE, LEVEL3_POUNCE_RANGE) + 6.0f
#define LEVEL3_POUNCE_WIDTH         TREMULOUS_VALUE(14.0f, 16.0f)
#define LEVEL3_POUNCE_TIME          TREMULOUS_VALUE(700, 700)      // msec for full Dragoon pounce
#define LEVEL3_POUNCE_TIME_UPG      TREMULOUS_VALUE(700, 700)      // msec for full Adv. Dragoon pounce
#define LEVEL3_POUNCE_TIME_MIN      TREMULOUS_VALUE(200, 200)      // msec before which pounce cancels  
#define LEVEL3_POUNCE_REPEAT        TREMULOUS_VALUE(400, 400)      // msec before a new pounce starts
#define LEVEL3_POUNCE_SPEED_MOD     TREMULOUS_VALUE(0.75f, 0.75f)    // walking speed modifier for pounce charging
#define LEVEL3_POUNCE_JUMP_MAG      TREMULOUS_VALUE(700, 700)      // Dragoon pounce jump power
#define LEVEL3_POUNCE_JUMP_MAG_UPG  TREMULOUS_VALUE(800, 800)      // Adv. Dragoon pounce jump power
#define LEVEL3_BOUNCEBALL_DMG       TREMULOUS_VALUE(ADM(110), ADM(110))
#define LEVEL3_BOUNCEBALL_REPEAT    TREMULOUS_VALUE(1200, 1200)
#define LEVEL3_BOUNCEBALL_SPEED     TREMULOUS_VALUE(1000.0f, 1000.0f)
#define LEVEL3_BOUNCEBALL_RADIUS    TREMULOUS_VALUE(75, 75)
#define LEVEL3_BOUNCEBALL_REGEN     TREMULOUS_VALUE(15000, 15000)    // msec until new barb

#define LEVEL4_CLAW_DMG             TREMULOUS_VALUE(ADM(100), ADM(100))
#define LEVEL4_CLAW_RANGE           TREMULOUS_VALUE(100.0f, 128.0f)
#define LEVEL4_CLAW_WIDTH           TREMULOUS_VALUE(14.0f, 14.0f)
#define LEVEL4_CLAW_HEIGHT          TREMULOUS_VALUE(20.0f, 20.0f)
#define LEVEL4_CLAW_REPEAT          TREMULOUS_VALUE(750, 750)
#define LEVEL4_CLAW_K_SCALE         TREMULOUS_VALUE(1.0f, 1.0f)

#define LEVEL4_TRAMPLE_DMG             TREMULOUS_VALUE(ADM(111), ADM(110))
#define LEVEL4_TRAMPLE_SPEED           TREMULOUS_VALUE(2.0f, 2.0f)
#define LEVEL4_TRAMPLE_CHARGE_MIN      TREMULOUS_VALUE(375, 375)   // minimum msec to start a charge
#define LEVEL4_TRAMPLE_CHARGE_MAX      TREMULOUS_VALUE(1000, 1000)  // msec to maximum charge stored
#define LEVEL4_TRAMPLE_CHARGE_TRIGGER  TREMULOUS_VALUE(3000, 3000)  // msec charge starts on its own
#define LEVEL4_TRAMPLE_DURATION        TREMULOUS_VALUE(3000, 3000)  // msec trample lasts on full charge
//#define LEVEL4_TRAMPLE_STOP_PERCENTAGE 20    // removed from the end of trample when it isn't very useful
#define LEVEL4_TRAMPLE_STOP_PENALTY    TREMULOUS_VALUE(1, 1)     // charge lost per msec when stopped
#define LEVEL4_TRAMPLE_REPEAT          TREMULOUS_VALUE(75, 75)    // msec before a trample will rehit a player

#define LEVEL4_CRUSH_DAMAGE_PER_V      TREMULOUS_VALUE(0.5f, 0.5f)  // damage per falling velocity
#define LEVEL4_CRUSH_DAMAGE            TREMULOUS_VALUE(120, 120)   // to players only
#define LEVEL4_CRUSH_REPEAT            TREMULOUS_VALUE(500, 500)   // player damage repeat

/*
 * ALIEN classes
 *
 * _SPEED   - fraction of Q3A run speed the class can move
 * _REGEN   - health per second regained
 *
 * ALIEN_HLTH_MODIFIER - overall health modifier for coarse tuning
 *
 */

#define ALIEN_HLTH_MODIFIER         1.0f
#define AHM(h)                      ((int)((float)h*ALIEN_HLTH_MODIFIER))

#define ALIEN_VALUE_MODIFIER        1.0f
#define AVM(h)                      ((int)((float)h*ALIEN_VALUE_MODIFIER))

#define ABUILDER_SPEED              TREMULOUS_VALUE(0.7f, 0.8f)
#define ABUILDER_VALUE              TREMULOUS_VALUE(AVM(200), AVM(200))
#define ABUILDER_HEALTH             TREMULOUS_VALUE(AHM(50), AHM(50))
#define ABUILDER_REGEN              TREMULOUS_VALUE(2, 2)
#define ABUILDER_COST               TREMULOUS_VALUE(0, 0)

#define ABUILDER_UPG_SPEED          TREMULOUS_VALUE(0.7f, 1.0f)
#define ABUILDER_UPG_VALUE          TREMULOUS_VALUE(AVM(250), AVM(250))
#define ABUILDER_UPG_HEALTH         TREMULOUS_VALUE(AHM(75), AHM(75))
#define ABUILDER_UPG_REGEN          TREMULOUS_VALUE(3, 3)
#define ABUILDER_UPG_COST           TREMULOUS_VALUE(0, 0)

#define LEVEL0_SPEED                TREMULOUS_VALUE(1.4f, 1.3f)
#define LEVEL0_VALUE                TREMULOUS_VALUE(AVM(150), AVM(175))
#define LEVEL0_HEALTH               TREMULOUS_VALUE(AHM(20), AHM(25))
#define LEVEL0_REGEN                TREMULOUS_VALUE(1, 1)
#define LEVEL0_COST                 TREMULOUS_VALUE(0, 0)

#define LEVEL1_SPEED                TREMULOUS_VALUE(1.25f, 1.25f)
#define LEVEL1_VALUE                TREMULOUS_VALUE(AVM(225), AVM(225))
#define LEVEL1_HEALTH               TREMULOUS_VALUE(AHM(60), AHM(75))
#define LEVEL1_REGEN                TREMULOUS_VALUE(2, 2)
#define LEVEL1_COST                 TREMULOUS_VALUE(1, 1)

#define LEVEL1_UPG_SPEED            TREMULOUS_VALUE(1.25f, 1.25f)
#define LEVEL1_UPG_VALUE            TREMULOUS_VALUE(AVM(275), AVM(275))
#define LEVEL1_UPG_HEALTH           TREMULOUS_VALUE(AHM(80), AHM(80))
#define LEVEL1_UPG_REGEN            TREMULOUS_VALUE(3, 3)
#define LEVEL1_UPG_COST             TREMULOUS_VALUE(1, 1)

#define LEVEL2_SPEED                TREMULOUS_VALUE(1.2f, 1.2f)
#define LEVEL2_VALUE                TREMULOUS_VALUE(AVM(350), AVM(350))
#define LEVEL2_HEALTH               TREMULOUS_VALUE(AHM(150), AHM(150))
#define LEVEL2_REGEN                TREMULOUS_VALUE(4, 4)
#define LEVEL2_COST                 TREMULOUS_VALUE(1, 1)

#define LEVEL2_UPG_SPEED            TREMULOUS_VALUE(1.2f, 1.2f)
#define LEVEL2_UPG_VALUE            TREMULOUS_VALUE(AVM(450), AVM(450))
#define LEVEL2_UPG_HEALTH           TREMULOUS_VALUE(AHM(175), AHM(175))
#define LEVEL2_UPG_REGEN            TREMULOUS_VALUE(5, 5)
#define LEVEL2_UPG_COST             TREMULOUS_VALUE(1, 1)

#define LEVEL3_SPEED                TREMULOUS_VALUE(1.1f, 1.1f)
#define LEVEL3_VALUE                TREMULOUS_VALUE(AVM(500), AVM(500))
#define LEVEL3_HEALTH               TREMULOUS_VALUE(AHM(200), AHM(200))
#define LEVEL3_REGEN                TREMULOUS_VALUE(6, 6)
#define LEVEL3_COST                 TREMULOUS_VALUE(1, 1)

#define LEVEL3_UPG_SPEED            TREMULOUS_VALUE(1.1f, 1.1f)
#define LEVEL3_UPG_VALUE            TREMULOUS_VALUE(AVM(600), AVM(600))
#define LEVEL3_UPG_HEALTH           TREMULOUS_VALUE(AHM(250), AHM(250))
#define LEVEL3_UPG_REGEN            TREMULOUS_VALUE(7, 7)
#define LEVEL3_UPG_COST             TREMULOUS_VALUE(1, 1)

#define LEVEL4_SPEED                TREMULOUS_VALUE(1.2f, 1.2f)
#define LEVEL4_VALUE                TREMULOUS_VALUE(AVM(800), AVM(800))
#define LEVEL4_HEALTH               TREMULOUS_VALUE(AHM(350), AHM(400))
#define LEVEL4_REGEN                TREMULOUS_VALUE(9, 9)
#define LEVEL4_COST                 TREMULOUS_VALUE(2, 2)



/*
 * ALIEN buildables
 *
 * _BP            - build points required for this buildable
 * _BT            - build time required for this buildable
 * _REGEN         - the amount of health per second regained
 * _SPLASHDAMGE   - the amount of damage caused by this buildable when melting
 * _SPLASHRADIUS  - the radius around which it does this damage
 *
 * CREEP_BASESIZE - the maximum distance a buildable can be from an egg/overmind
 * ALIEN_BHLTH_MODIFIER - overall health modifier for coarse tuning
 *
 */

#define ALIEN_BHLTH_MODIFIER        1.0f
#define ABHM(h)                     ((int)((float)h*ALIEN_BHLTH_MODIFIER))
#define ALIEN_BVALUE_MODIFIER       0.0f
#define ABVM(h)                     ((int)((float)h*ALIEN_BVALUE_MODIFIER))

#define CREEP_BASESIZE              TREMULOUS_VALUE(700, 700)
#define CREEP_TIMEOUT               TREMULOUS_VALUE(1000, 1000)
#define CREEP_MODIFIER              TREMULOUS_VALUE(0.5f, 0.5f)
#define CREEP_ARMOUR_MODIFIER       TREMULOUS_VALUE(0.75f, 0.75f)
#define CREEP_SCALEDOWN_TIME        TREMULOUS_VALUE(3000, 3000)

#define PCLOUD_MODIFIER             TREMULOUS_VALUE(0.5f, 0.5f)
#define PCLOUD_ARMOUR_MODIFIER      TREMULOUS_VALUE(0.75f, 0.75f)

#define ASPAWN_BP                   TREMULOUS_VALUE(10, 10)
#define ASPAWN_BT                   TREMULOUS_VALUE(15000, 15000)
#define ASPAWN_HEALTH               TREMULOUS_VALUE(ABHM(250), ABHM(250))
#define ASPAWN_REGEN                TREMULOUS_VALUE(8, 8)
#define ASPAWN_SPLASHDAMAGE         TREMULOUS_VALUE(50, 50)
#define ASPAWN_SPLASHRADIUS         TREMULOUS_VALUE(100, 50)
#define ASPAWN_CREEPSIZE            TREMULOUS_VALUE(120, 120)
#define ASPAWN_VALUE                TREMULOUS_VALUE(ABVM(ASPAWN_BP), ABVM(ASPAWN_BP))

#define BARRICADE_BP                TREMULOUS_VALUE(8, 8)
#define BARRICADE_BT                TREMULOUS_VALUE(20000, 20000)
#define BARRICADE_HEALTH            TREMULOUS_VALUE(ABHM(300), ABHM(300))
#define BARRICADE_REGEN             TREMULOUS_VALUE(14, 14)
#define BARRICADE_SPLASHDAMAGE      TREMULOUS_VALUE(50, 50)
#define BARRICADE_SPLASHRADIUS      TREMULOUS_VALUE(100, 100)
#define BARRICADE_CREEPSIZE         TREMULOUS_VALUE(120, 120)
#define BARRICADE_SHRINKPROP        TREMULOUS_VALUE(0.25f, 0.25f)
#define BARRICADE_SHRINKTIMEOUT     TREMULOUS_VALUE(500, 500)
#define BARRICADE_VALUE             TREMULOUS_VALUE(ABVM(BARRICADE_BP), ABVM(0))

#define BOOSTER_BP                  TREMULOUS_VALUE(12, 12)
#define BOOSTER_BT                  TREMULOUS_VALUE(15000, 15000)
#define BOOSTER_HEALTH              TREMULOUS_VALUE(ABHM(150), ABHM(150))
#define BOOSTER_REGEN               TREMULOUS_VALUE(8, 8)
#define BOOSTER_SPLASHDAMAGE        TREMULOUS_VALUE(50, 50)
#define BOOSTER_SPLASHRADIUS        TREMULOUS_VALUE(100, 50)
#define BOOSTER_CREEPSIZE           TREMULOUS_VALUE(120, 120)
#define BOOSTER_REGEN_MOD           TREMULOUS_VALUE(3.0f, 2.0f)
#define BOOSTER_VALUE               TREMULOUS_VALUE(ABVM(BOOSTER_BP), ABVM(0))
#define BOOST_TIME                  TREMULOUS_VALUE(20000, 20000)
#define BOOST_WARN_TIME             TREMULOUS_VALUE(15000, 15000)

#define ACIDTUBE_BP                 TREMULOUS_VALUE(8, 8)
#define ACIDTUBE_BT                 TREMULOUS_VALUE(15000, 15000)
#define ACIDTUBE_HEALTH             TREMULOUS_VALUE(ABHM(125), ABHM(125))
#define ACIDTUBE_REGEN              TREMULOUS_VALUE(10, 10)
#define ACIDTUBE_SPLASHDAMAGE       TREMULOUS_VALUE(50, 50)
#define ACIDTUBE_SPLASHRADIUS       TREMULOUS_VALUE(100, 50)
#define ACIDTUBE_CREEPSIZE          TREMULOUS_VALUE(120, 120)
#define ACIDTUBE_DAMAGE             TREMULOUS_VALUE(8, 8)
#define ACIDTUBE_RANGE              TREMULOUS_VALUE(300.0f, 300.0f)
#define ACIDTUBE_REPEAT             TREMULOUS_VALUE(300, 300)
#define ACIDTUBE_REPEAT_ANIM        TREMULOUS_VALUE(2000, 2000)
#define ACIDTUBE_VALUE              TREMULOUS_VALUE(ABVM(ACIDTUBE_BP), ABVM(0))

#define HIVE_BP                     TREMULOUS_VALUE(12, 12)
#define HIVE_BT                     TREMULOUS_VALUE(20000, 20000)
#define HIVE_HEALTH                 TREMULOUS_VALUE(ABHM(175), ABHM(175))
#define HIVE_REGEN                  TREMULOUS_VALUE(10, 10)
#define HIVE_SPLASHDAMAGE           TREMULOUS_VALUE(30, 30)
#define HIVE_SPLASHRADIUS           TREMULOUS_VALUE(200, 200)
#define HIVE_CREEPSIZE              TREMULOUS_VALUE(120, 120)
#define HIVE_SENSE_RANGE            TREMULOUS_VALUE(500.0f, 400.0f)
#define HIVE_LIFETIME               TREMULOUS_VALUE(6000, 6000)
#define HIVE_REPEAT                 TREMULOUS_VALUE(3000, 3000)
#define HIVE_K_SCALE                TREMULOUS_VALUE(1.0f, 1.0f)
#define HIVE_DMG                    TREMULOUS_VALUE(100, 100)
#define HIVE_SPEED                  TREMULOUS_VALUE(384.0f, 384.0f)
#define HIVE_DIR_CHANGE_PERIOD      TREMULOUS_VALUE(500, 500)
#define HIVE_VALUE                  TREMULOUS_VALUE(ABVM(HIVE_BP), ABVM(0))

#define TRAPPER_BP                  TREMULOUS_VALUE(8, 8)
#define TRAPPER_BT                  TREMULOUS_VALUE(12000, 12000)
#define TRAPPER_HEALTH              TREMULOUS_VALUE(ABHM(50), ABHM(50))
#define TRAPPER_REGEN               TREMULOUS_VALUE(6, 6)
#define TRAPPER_SPLASHDAMAGE        TREMULOUS_VALUE(15, 15)
#define TRAPPER_SPLASHRADIUS        TREMULOUS_VALUE(100, 100)
#define TRAPPER_CREEPSIZE           TREMULOUS_VALUE(30, 30)
#define TRAPPER_RANGE               TREMULOUS_VALUE(400, 400)
#define TRAPPER_REPEAT              TREMULOUS_VALUE(1000, 1000)
#define TRAPPER_VALUE               TREMULOUS_VALUE(ABVM(TRAPPER_BP), ABVM(0))
#define LOCKBLOB_SPEED              TREMULOUS_VALUE(650.0f, 650.0f)
#define LOCKBLOB_LOCKTIME           TREMULOUS_VALUE(5000, 5000)
#define LOCKBLOB_DOT                TREMULOUS_VALUE(0.85f, 0.85f) // max angle = acos( LOCKBLOB_DOT )
#define LOCKBLOB_K_SCALE            TREMULOUS_VALUE(1.0f, 1.0f)

#define OVERMIND_BP                 TREMULOUS_VALUE(0, 0)
#define OVERMIND_BT                 TREMULOUS_VALUE(30000, 30000)
#define OVERMIND_HEALTH             TREMULOUS_VALUE(ABHM(750), ABHM(750))
#define OVERMIND_REGEN              TREMULOUS_VALUE(6, 6)
#define OVERMIND_SPLASHDAMAGE       TREMULOUS_VALUE(15, 15)
#define OVERMIND_SPLASHRADIUS       TREMULOUS_VALUE(300, 300)
#define OVERMIND_CREEPSIZE          TREMULOUS_VALUE(120, 120)
#define OVERMIND_ATTACK_RANGE       TREMULOUS_VALUE(150.0f, 150.0f)
#define OVERMIND_ATTACK_REPEAT      TREMULOUS_VALUE(1000, 1000)
#define OVERMIND_VALUE              TREMULOUS_VALUE(ABVM(30), ABVM(30))

#define HOVEL_BP                    TREMULOUS_VALUE(0, 0)
#define HOVEL_BT                    TREMULOUS_VALUE(15000, 15000)
#define HOVEL_HEALTH                TREMULOUS_VALUE(ABHM(375), ABHM(375))
#define HOVEL_REGEN                 TREMULOUS_VALUE(20, 20)
#define HOVEL_SPLASHDAMAGE          TREMULOUS_VALUE(20, 20)
#define HOVEL_SPLASHRADIUS          TREMULOUS_VALUE(200, 200)
#define HOVEL_CREEPSIZE             TREMULOUS_VALUE(120, 120)
#define HOVEL_VALUE                 TREMULOUS_VALUE(ABVM(8), ABVM(0))



/*
 * ALIEN misc
 *
 * ALIENSENSE_RANGE - the distance alien sense is useful for
 *
 */

#define ALIENSENSE_RANGE            TREMULOUS_VALUE(1000.0f, 1000.0f)
#define REGEN_BOOST_RANGE           TREMULOUS_VALUE(200.0f, 200.0f)

#define ALIEN_POISON_TIME           TREMULOUS_VALUE(10000, 5000)
#define ALIEN_POISON_DMG            TREMULOUS_VALUE(5, 5)
#define ALIEN_POISON_DIVIDER        TREMULOUS_VALUE((1.0f, (1.0f)/1.32f) //about 1.0/(time`th root of damage)

#define ALIEN_SPAWN_REPEAT_TIME     TREMULOUS_VALUE(10000, 10000)

#define ALIEN_REGEN_DAMAGE_TIME     TREMULOUS_VALUE(1500, 1500) //msec since damage that regen starts again
#define ALIEN_REGEN_NOCREEP_TIME    TREMULOUS_VALUE(3000, 1000) //msec between regen off creep

#define ALIEN_MAX_FRAGS             TREMULOUS_VALUE(9, 9)
#define ALIEN_MAX_CREDITS           TREMULOUS_VALUE((ALIEN_MAX_FRAGS*ALIEN_CREDITS_PER_FRAG), (ALIEN_MAX_FRAGS*ALIEN_CREDITS_PER_FRAG))
#define ALIEN_CREDITS_PER_FRAG      TREMULOUS_VALUE(400, 400)
#define ALIEN_TK_SUICIDE_PENALTY    TREMULOUS_VALUE(350, 350)

// Allow Aliens to wallwalk on any entity (buildables, etc) but not players
#define ALIEN_WALLWALK_ENTITIESTREMULOUS_VALUE(, )

/*
 * HUMAN weapons
 *
 * _REPEAT  - time between firings
 * _RELOAD  - time needed to reload
 * _PRICE   - amount in credits weapon costs
 *
 * HUMAN_WDMG_MODIFIER - overall damage modifier for coarse tuning
 *
 */

#define HUMAN_WDMG_MODIFIER         1.0f
#define HDM(d)                      ((int)((float)d*HUMAN_WDMG_MODIFIER))

#define BLASTER_REPEAT              TREMULOUS_VALUE(600, 600)
#define BLASTER_K_SCALE             TREMULOUS_VALUE(1.0f, 1.0f)
#define BLASTER_SPREAD              TREMULOUS_VALUE(200, 200)
#define BLASTER_SPEED               TREMULOUS_VALUE(1400, 1400)
#define BLASTER_DMG                 TREMULOUS_VALUE(HDM(10), HDM(9))
#define BLASTER_SIZE                TREMULOUS_VALUE(5, 5)

#define RIFLE_CLIPSIZE              TREMULOUS_VALUE(30, 30)
#define RIFLE_MAXCLIPS              TREMULOUS_VALUE(6, 6)
#define RIFLE_REPEAT                TREMULOUS_VALUE(90, 90)
#define RIFLE_K_SCALE               TREMULOUS_VALUE(1.0f, 1.0f)
#define RIFLE_RELOAD                TREMULOUS_VALUE(2000, 2000)
#define RIFLE_PRICE                 TREMULOUS_VALUE(0, 0)
#define RIFLE_SPREAD                TREMULOUS_VALUE(200, 200)
#define RIFLE_DMG                   TREMULOUS_VALUE(HDM(5), HDM(5))

#define PAINSAW_PRICE               TREMULOUS_VALUE(100, 100)
#define PAINSAW_REPEAT              TREMULOUS_VALUE(75, 75)
#define PAINSAW_K_SCALE             TREMULOUS_VALUE(1.0f, 1.0f)
#define PAINSAW_DAMAGE              TREMULOUS_VALUE(HDM(11), HDM(15))
#define PAINSAW_RANGE               TREMULOUS_VALUE(64.0f, 64.0f)
#define PAINSAW_WIDTH               TREMULOUS_VALUE(0.f, 0.f)
#define PAINSAW_HEIGHT              TREMULOUS_VALUE(8.f, 8.f)

#define GRENADE_PRICE               TREMULOUS_VALUE(200, 200)
#define GRENADE_REPEAT              TREMULOUS_VALUE(0, 0)
#define GRENADE_K_SCALE             TREMULOUS_VALUE(1.0f, 1.0f)
#define GRENADE_DAMAGE              TREMULOUS_VALUE(HDM(310), HDM(310))
#define GRENADE_RANGE               TREMULOUS_VALUE(192.0f, 192.0f)
#define GRENADE_SPEED               TREMULOUS_VALUE(400.0f, 400.0f)

#define SHOTGUN_PRICE               TREMULOUS_VALUE(150, 150)
#define SHOTGUN_SHELLS              TREMULOUS_VALUE(8, 8)
#define SHOTGUN_PELLETS             TREMULOUS_VALUE(12, 12) //used to sync server and client side
#define SHOTGUN_MAXCLIPS            TREMULOUS_VALUE(3, 3)
#define SHOTGUN_REPEAT              TREMULOUS_VALUE(1000, 1000)
#define SHOTGUN_K_SCALE             TREMULOUS_VALUE(1.0f, 1.0f)
#define SHOTGUN_RELOAD              TREMULOUS_VALUE(2000, 2000)
#define SHOTGUN_SPREAD              TREMULOUS_VALUE(900, 900)
#define SHOTGUN_DMG                 TREMULOUS_VALUE(HDM(4), HDM(7))
#define SHOTGUN_RANGE               TREMULOUS_VALUE((8192, (8192) * 12)

#define LASGUN_PRICE                TREMULOUS_VALUE(250, 250)
#define LASGUN_AMMO                 TREMULOUS_VALUE(200, 200)
#define LASGUN_REPEAT               TREMULOUS_VALUE(200, 200)
#define LASGUN_K_SCALE              TREMULOUS_VALUE(1.0f, 1.0f)
#define LASGUN_RELOAD               TREMULOUS_VALUE(2000, 2000)
#define LASGUN_DAMAGE               TREMULOUS_VALUE(HDM(9), HDM(9))

#define MDRIVER_PRICE               TREMULOUS_VALUE(350, 350)
#define MDRIVER_CLIPSIZE            TREMULOUS_VALUE(5, 5)
#define MDRIVER_MAXCLIPS            TREMULOUS_VALUE(4, 4)
#define MDRIVER_DMG                 TREMULOUS_VALUE(HDM(38), HDM(38))
#define MDRIVER_REPEAT              TREMULOUS_VALUE(1000, 1000)
#define MDRIVER_K_SCALE             TREMULOUS_VALUE(1.0f, 1.0f)
#define MDRIVER_RELOAD              TREMULOUS_VALUE(2000, 2000)
#define MDRIVER_MAX_HITS            TREMULOUS_VALUE(16, 16)

#define CHAINGUN_PRICE              TREMULOUS_VALUE(400, 400)
#define CHAINGUN_BULLETS            TREMULOUS_VALUE(300, 300)
#define CHAINGUN_REPEAT             TREMULOUS_VALUE(80, 80)
#define CHAINGUN_K_SCALE            TREMULOUS_VALUE(1.0f, 1.0f)
#define CHAINGUN_SPREAD             TREMULOUS_VALUE(900, 1000)
#define CHAINGUN_DMG                TREMULOUS_VALUE(HDM(5), HDM(6))

#define PRIFLE_PRICE                TREMULOUS_VALUE(400, 400)
#define PRIFLE_CLIPS                TREMULOUS_VALUE(50, 50)
#define PRIFLE_MAXCLIPS             TREMULOUS_VALUE(4, 4)
#define PRIFLE_REPEAT               TREMULOUS_VALUE(100, 100)
#define PRIFLE_K_SCALE              TREMULOUS_VALUE(1.0f, 1.0f)
#define PRIFLE_RELOAD               TREMULOUS_VALUE(2000, 2000)
#define PRIFLE_DMG                  TREMULOUS_VALUE(HDM(9), HDM(9))
#define PRIFLE_SPEED                TREMULOUS_VALUE(1200, 1000)
#define PRIFLE_SIZE                 TREMULOUS_VALUE(5, 5)

#define FLAMER_PRICE                TREMULOUS_VALUE(450, 450)
#define FLAMER_GAS                  TREMULOUS_VALUE(150, 150)
#define FLAMER_REPEAT               TREMULOUS_VALUE(200, 200)
#define FLAMER_K_SCALE              TREMULOUS_VALUE(1.0f, 1.0f)
#define FLAMER_DMG                  TREMULOUS_VALUE(HDM(20), HDM(20))
#define FLAMER_RADIUS               TREMULOUS_VALUE(50, 50)       // splash radius
#define FLAMER_SIZE                 TREMULOUS_VALUE(15, 15)       // missile bounding box
#define FLAMER_LIFETIME             TREMULOUS_VALUE(700.0f, 800.0f)
#define FLAMER_SPEED                TREMULOUS_VALUE(300.0f, 200.0f)
#define FLAMER_LAG                  TREMULOUS_VALUE(0.65f, 0.65f)  //the amount of player velocity that is added to the fireball

#define LCANNON_PRICE               TREMULOUS_VALUE(600, 600)
#define LCANNON_AMMO                TREMULOUS_VALUE(80, 90)
#define LCANNON_K_SCALE             TREMULOUS_VALUE(1.0f, 1.0f)
#define LCANNON_REPEAT              TREMULOUS_VALUE(500, 500)
#define LCANNON_RELOAD              TREMULOUS_VALUE(0, 0)
#define LCANNON_DAMAGE              TREMULOUS_VALUE(HDM(265), HDM(265))
#define LCANNON_RADIUS              TREMULOUS_VALUE(150, 150)      // primary splash damage radius
#define LCANNON_SIZE                TREMULOUS_VALUE(5, 5)        // missile bounding box radius
#define LCANNON_SECONDARY_DAMAGE    TREMULOUS_VALUE(HDM(30), HDM(27))
#define LCANNON_SECONDARY_RADIUS    TREMULOUS_VALUE(75, 75)       // secondary splash damage radius
#define LCANNON_SECONDARY_SPEED     TREMULOUS_VALUE(1400, 350)
#define LCANNON_SECONDARY_RELOAD    TREMULOUS_VALUE(2000, 2000)
#define LCANNON_SECONDARY_REPEAT    TREMULOUS_VALUE(1000, 1000)
#define LCANNON_SPEED               TREMULOUS_VALUE(700, 350)
#define LCANNON_CHARGE_TIME_MAX     TREMULOUS_VALUE(3000, 2000)
#define LCANNON_CHARGE_TIME_MIN     TREMULOUS_VALUE(100, 100)
#define LCANNON_CHARGE_TIME_WARN    TREMULOUS_VALUE(2000, 1333)
#define LCANNON_CHARGE_AMMO         TREMULOUS_VALUE(10, 10)       // ammo cost of a full charge shot

#define HBUILD_PRICE                TREMULOUS_VALUE(0, 0)
#define HBUILD_REPEAT               TREMULOUS_VALUE(1000, 1000)
#define HBUILD_HEALRATE             TREMULOUS_VALUE(18, 18)



/*
 * HUMAN upgrades
 */

#define LIGHTARMOUR_PRICE           TREMULOUS_VALUE(70, 70)
#define LIGHTARMOUR_POISON_PROTECTION TREMULOUS_VALUE(1, 1)
#define LIGHTARMOUR_PCLOUD_PROTECTION TREMULOUS_VALUE(1000, 0)

#define HELMET_PRICE                TREMULOUS_VALUE(90, 90)
#define HELMET_RANGE                TREMULOUS_VALUE(1000.0f, 1000.0f)
#define HELMET_POISON_PROTECTION    TREMULOUS_VALUE(1, 2)
#define HELMET_PCLOUD_PROTECTION    TREMULOUS_VALUE(1000, 1000)

#define MEDKIT_PRICE                TREMULOUS_VALUE(0, 0)

#define BATTPACK_PRICE              TREMULOUS_VALUE(100, 100)
#define BATTPACK_MODIFIER           TREMULOUS_VALUE(1.5f, 1.5f) //modifier for extra energy storage available

#define JETPACK_PRICE               TREMULOUS_VALUE(120, 120)
#define JETPACK_FLOAT_SPEED         TREMULOUS_VALUE(128.0f, 128.0f) //up movement speed
#define JETPACK_SINK_SPEED          TREMULOUS_VALUE(192.0f, 192.0f) //down movement speed
#define JETPACK_DISABLE_TIME        TREMULOUS_VALUE(1000, 1000) //time to disable the jetpack when player damaged
#define JETPACK_DISABLE_CHANCE      TREMULOUS_VALUE(0.3f, 0.3f)

#define BSUIT_PRICE                 TREMULOUS_VALUE(400, 400)
#define BSUIT_POISON_PROTECTION     TREMULOUS_VALUE(3, 4)
#define BSUIT_PCLOUD_PROTECTION     TREMULOUS_VALUE(3000, 3000)

#define MGCLIP_PRICE                TREMULOUS_VALUE(0, 0)

#define CGAMMO_PRICE                TREMULOUS_VALUE(0, 0)

#define GAS_PRICE                   TREMULOUS_VALUE(0, 0)

#define MEDKIT_POISON_IMMUNITY_TIME TREMULOUS_VALUE(0, 0)
#define MEDKIT_STARTUP_TIME         TREMULOUS_VALUE(4000, 4000)
#define MEDKIT_STARTUP_SPEED        TREMULOUS_VALUE(5, 5)


/*
 * HUMAN buildables
 *
 * _BP            - build points required for this buildable
 * _BT            - build time required for this buildable
 * _SPLASHDAMGE   - the amount of damage caused by this buildable when it blows up
 * _SPLASHRADIUS  - the radius around which it does this damage
 *
 * REACTOR_BASESIZE - the maximum distance a buildable can be from an reactor
 * REPEATER_BASESIZE - the maximum distance a buildable can be from a repeater
 * HUMAN_BHLTH_MODIFIER - overall health modifier for coarse tuning
 *
 */

#define HUMAN_BHLTH_MODIFIER        TREMULOUS_VALUE(1.0f, 1.0f)
#define HBHMTREMULOUS_VALUE((h), (h))                     ((int)((float)h*HUMAN_BHLTH_MODIFIER))
#define HUMAN_BVALUE_MODIFIER       TREMULOUS_VALUE(0.0f, 0.0f)
#define HBVMTREMULOUS_VALUE((h), (h))                     ((int)((float)h*(float)HUMAN_BVALUE_MODIFIER)) // remember these are measured in credits not frags (c.f. ALIEN_CREDITS_PER_FRAG)

#define REACTOR_BASESIZE            TREMULOUS_VALUE(1000, 1000)
#define REPEATER_BASESIZE           TREMULOUS_VALUE(500, 500)
#define HUMAN_DETONATION_DELAY      TREMULOUS_VALUE(5000, 5000)

#define HSPAWN_BP                   TREMULOUS_VALUE(10, 10)
#define HSPAWN_BT                   TREMULOUS_VALUE(10000, 10000)
#define HSPAWN_HEALTH               TREMULOUS_VALUE(HBHM(310), HBHM(310))
#define HSPAWN_SPLASHDAMAGE         TREMULOUS_VALUE(50, 50)
#define HSPAWN_SPLASHRADIUS         TREMULOUS_VALUE(100, 100)
#define HSPAWN_VALUE                TREMULOUS_VALUE(HBVM(HSPAWN_BP), HBVM(HSPAWN_BP))

#define MEDISTAT_BP                 TREMULOUS_VALUE(8, 8)
#define MEDISTAT_BT                 TREMULOUS_VALUE(10000, 10000)
#define MEDISTAT_HEALTH             TREMULOUS_VALUE(HBHM(190), HBHM(190))
#define MEDISTAT_SPLASHDAMAGE       TREMULOUS_VALUE(50, 50)
#define MEDISTAT_SPLASHRADIUS       TREMULOUS_VALUE(100, 100)
#define MEDISTAT_VALUE              TREMULOUS_VALUE(HBVM(MEDISTAT_BP), HBVM(0))

#define MGTURRET_BP                 TREMULOUS_VALUE(8, 8)
#define MGTURRET_BT                 TREMULOUS_VALUE(10000, 10000)
#define MGTURRET_HEALTH             TREMULOUS_VALUE(HBHM(190), HBHM(190))
#define MGTURRET_SPLASHDAMAGE       TREMULOUS_VALUE(100, 100)
#define MGTURRET_SPLASHRADIUS       TREMULOUS_VALUE(100, 100)
#define MGTURRET_ANGULARSPEED       TREMULOUS_VALUE(12, 8)
#define MGTURRET_ANGULARSPEED_GRAB  TREMULOUS_VALUE(8, 3)
#define MGTURRET_ACCURACY_TO_FIRE   TREMULOUS_VALUE(0, 0)
#define MGTURRET_VERTICALCAP        TREMULOUS_VALUE(30, 30)  // +/- maximum pitch
#define MGTURRET_REPEAT             TREMULOUS_VALUE(150, 100)
#define MGTURRET_K_SCALE            TREMULOUS_VALUE(1.0f, 1.0f)
#define MGTURRET_RANGE              TREMULOUS_VALUE(400.0f, 300.0f)
#define MGTURRET_SPREAD             TREMULOUS_VALUE(200, 200)
#define MGTURRET_DMG                TREMULOUS_VALUE(HDM(8), HDM(4))
#define MGTURRET_SPINUP_TIME        TREMULOUS_VALUE(750, 0) // time between target sighted and fire
#define MGTURRET_DROOP_RATE         TREMULOUS_VALUE(1.0f, 1.0f) // rate at which turret droops when unpowered
#define MGTURRET_VALUE              TREMULOUS_VALUE(HBVM(MGTURRET_BP), HBVM(0))

#define TESLAGEN_BP                 TREMULOUS_VALUE(10, 10)
#define TESLAGEN_BT                 TREMULOUS_VALUE(15000, 15000)
#define TESLAGEN_HEALTH             TREMULOUS_VALUE(HBHM(220), HBHM(220))
#define TESLAGEN_SPLASHDAMAGE       TREMULOUS_VALUE(50, 50)
#define TESLAGEN_SPLASHRADIUS       TREMULOUS_VALUE(100, 100)
#define TESLAGEN_REPEAT             TREMULOUS_VALUE(250, 250)
#define TESLAGEN_K_SCALE            TREMULOUS_VALUE(4.0f, 4.0f)
#define TESLAGEN_RANGE              TREMULOUS_VALUE(150, 250)
#define TESLAGEN_DMG                TREMULOUS_VALUE(HDM(10), HDM(9))
#define TESLAGEN_VALUE              TREMULOUS_VALUE(HBVM(TESLAGEN_BP), HBVM(0))

#define DC_BP                       TREMULOUS_VALUE(8, 8)
#define DC_BT                       TREMULOUS_VALUE(10000, 10000)
#define DC_HEALTH                   TREMULOUS_VALUE(HBHM(190), HBHM(190))
#define DC_SPLASHDAMAGE             TREMULOUS_VALUE(50, 50)
#define DC_SPLASHRADIUS             TREMULOUS_VALUE(100, 100)
#define DC_ATTACK_PERIOD            TREMULOUS_VALUE(10000, 10000) // how often to spam "under attack"
#define DC_HEALRATE                 TREMULOUS_VALUE(3, 3)
#define DC_RANGE                    TREMULOUS_VALUE(10000, 10000)
#define DC_VALUE                    TREMULOUS_VALUE(HBVM(DC_BP), HBVM(0))

#define ARMOURY_BP                  TREMULOUS_VALUE(10, 10)
#define ARMOURY_BT                  TREMULOUS_VALUE(10000, 10000)
#define ARMOURY_HEALTH              TREMULOUS_VALUE(HBHM(420), HBHM(420))
#define ARMOURY_SPLASHDAMAGE        TREMULOUS_VALUE(50, 50)
#define ARMOURY_SPLASHRADIUS        TREMULOUS_VALUE(100, 100)
#define ARMOURY_VALUE               TREMULOUS_VALUE(HBVM(ARMOURY_BP), HBVM(0))

#define REACTOR_BP                  TREMULOUS_VALUE(0, 0)
#define REACTOR_BT                  TREMULOUS_VALUE(20000, 20000)
#define REACTOR_HEALTH              TREMULOUS_VALUE(HBHM(930), HBHM(930))
#define REACTOR_SPLASHDAMAGE        TREMULOUS_VALUE(200, 200)
#define REACTOR_SPLASHRADIUS        TREMULOUS_VALUE(300, 300)
#define REACTOR_ATTACK_RANGE        TREMULOUS_VALUE(100.0f, 100.0f)
#define REACTOR_ATTACK_REPEAT       TREMULOUS_VALUE(1000, 1000)
#define REACTOR_ATTACK_DAMAGE       TREMULOUS_VALUE(40, 40)
#define REACTOR_ATTACK_DCC_REPEAT   TREMULOUS_VALUE(1000, 1000)
#define REACTOR_ATTACK_DCC_RANGE    TREMULOUS_VALUE(150.0f, 150.0f)
#define REACTOR_ATTACK_DCC_DAMAGE   TREMULOUS_VALUE(40, 40)
#define REACTOR_VALUE               TREMULOUS_VALUE(HBVM(30), HBVM(30))

#define REPEATER_BP                 TREMULOUS_VALUE(0, 0)
#define REPEATER_BT                 TREMULOUS_VALUE(10000, 10000)
#define REPEATER_HEALTH             TREMULOUS_VALUE(HBHM(250), HBHM(250))
#define REPEATER_SPLASHDAMAGE       TREMULOUS_VALUE(50, 50)
#define REPEATER_SPLASHRADIUS       TREMULOUS_VALUE(100, 100)
#define REPEATER_INACTIVE_TIME      TREMULOUS_VALUE(90000, 90000)
#define REPEATER_VALUE              TREMULOUS_VALUE(HBVM(2), HBVM(0))

/*
 * HUMAN misc
 */

#define HUMAN_SPRINT_MODIFIER       TREMULOUS_VALUE(1.2f, 1.2f)
#define HUMAN_JOG_MODIFIER          TREMULOUS_VALUE(1.0f, 1.0f)
#define HUMAN_BACK_MODIFIER         TREMULOUS_VALUE(0.8f, 0.8f)
#define HUMAN_SIDE_MODIFIER         TREMULOUS_VALUE(0.9f, 0.9f)
#define HUMAN_DODGE_SIDE_MODIFIER   TREMULOUS_VALUE(2.9f, 2.9f)
#define HUMAN_DODGE_UP_MODIFIER     TREMULOUS_VALUE(0.5f, 0.5f)
#define HUMAN_DODGE_TIMEOUT         TREMULOUS_VALUE(500, 500)
#define HUMAN_LAND_FRICTION         TREMULOUS_VALUE(3.f, 3.f)

#define STAMINA_STOP_RESTORE        TREMULOUS_VALUE(25, 25)
#define STAMINA_WALK_RESTORE        TREMULOUS_VALUE(15, 15)
#define STAMINA_MEDISTAT_RESTORE    TREMULOUS_VALUE(30, 30) // stacked on STOP or WALK
#define STAMINA_SPRINT_TAKE         TREMULOUS_VALUE(8, 8)
#define STAMINA_JUMP_TAKE           TREMULOUS_VALUE(250, 250)
#define STAMINA_DODGE_TAKE          TREMULOUS_VALUE(250, 250)
#define STAMINA_BREATHING_LEVEL     TREMULOUS_VALUE(0, 0)

#define HUMAN_SPAWN_REPEAT_TIME     TREMULOUS_VALUE(10000, 10000)
#define HUMAN_REGEN_DAMAGE_TIME     TREMULOUS_VALUE(2000, 2000) //msec since damage before dcc repairs

#define HUMAN_MAX_CREDITS           TREMULOUS_VALUE(2000, 2000)
#define HUMAN_TK_SUICIDE_PENALTY    TREMULOUS_VALUE(150, 150)

/*
 * Misc
 */

#define MIN_FALL_DISTANCE           TREMULOUS_VALUE(30.0f, 30.0f) //the fall distance at which fall damage kicks in
#define MAX_FALL_DISTANCE           TREMULOUS_VALUE(120.0f, 120.0f) //the fall distance at which maximum damage is dealt
#define AVG_FALL_DISTANCE           TREMULOUS_VALUE(((MIN_FALL_DISTANCE+MAX_FALL_DISTANCE), ((MIN_FALL_DISTANCE+MAX_FALL_DISTANCE))/2.0f)

#define FREEKILL_PERIOD             TREMULOUS_VALUE(120000, 120000) //msec
#define FREEKILL_ALIEN              TREMULOUS_VALUE(ALIEN_CREDITS_PER_FRAG, ALIEN_CREDITS_PER_FRAG)
#define FREEKILL_HUMAN              TREMULOUS_VALUE(LEVEL0_VALUE, LEVEL0_VALUE)

#define DEFAULT_ALIEN_BUILDPOINTS   TREMULOUS_VALUE("100", "100")
#define DEFAULT_ALIEN_QUEUE_TIME    TREMULOUS_VALUE("1250", "1250")
#define DEFAULT_ALIEN_STAGE_THRESH TREMULOUS_VALUE("8000", "8000")
#define DEFAULT_ALIEN_MAX_STAGE     TREMULOUS_VALUE("2", "2")
#define DEFAULT_HUMAN_BUILDPOINTS   TREMULOUS_VALUE("100", "100")
#define DEFAULT_HUMAN_QUEUE_TIME    TREMULOUS_VALUE("1250", "1250")
#define DEFAULT_HUMAN_STAGE_THRESH TREMULOUS_VALUE("4000", "4000")

#define DEFAULT_HUMAN_MAX_STAGE     TREMULOUS_VALUE("2", "2")

#define DAMAGE_FRACTION_FOR_KILL    TREMULOUS_VALUE(0.5f, 0.5f) //how much damage players (versus structures) need to
                                         //do to increment the stage kill counters
                                         
#define MAXIMUM_BUILD_TIME          TREMULOUS_VALUE(20000, 20000) // used for pie timer

