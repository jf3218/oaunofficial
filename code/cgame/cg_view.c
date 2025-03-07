/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
    vec3_t		angles;

    cg.testGun = qfalse;
    memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
    if ( trap_Argc() < 2 ) {
        return;
    }

    Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
    cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

    if ( trap_Argc() == 3 ) {
        cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
        cg.testModelEntity.frame = 1;
        cg.testModelEntity.oldframe = 0;
    }
    if (! cg.testModelEntity.hModel ) {
        CG_Printf( "Can't register model\n" );
        return;
    }

    VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

    angles[PITCH] = 0;
    angles[YAW] = 180 + cg.refdefViewAngles[1];
    angles[ROLL] = 0;

    AnglesToAxis( angles, cg.testModelEntity.axis );
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
    CG_TestModel_f();

    if ( !cg.testModelEntity.hModel ) {
        return;
    }

    cg.testGun = qtrue;
    cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
    cg.testModelEntity.frame++;
    CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
    cg.testModelEntity.frame--;
    if ( cg.testModelEntity.frame < 0 ) {
        cg.testModelEntity.frame = 0;
    }
    CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
    cg.testModelEntity.skinNum++;
    CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
    cg.testModelEntity.skinNum--;
    if ( cg.testModelEntity.skinNum < 0 ) {
        cg.testModelEntity.skinNum = 0;
    }
    CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
    int		i;

    // re-register the model, because the level may have changed
    cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
    if (! cg.testModelEntity.hModel ) {
        CG_Printf ("Can't register model\n");
        return;
    }

    // if testing a gun, set the origin reletive to the view origin
    if ( cg.testGun ) {
        VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
        VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
        VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
        VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

        // allow the position to be adjusted
        for (i=0 ; i<3 ; i++) {
            cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
            cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
            cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
        }
    }

    trap_R_AddRefEntityToScene( &cg.testModelEntity );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
    int		size;

    // the intermission should allways be full screen
    if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
        size = 100;
    } else {
        // bound normal viewsize
        if (cg_viewsize.integer < 30) {
            trap_Cvar_Set ("cg_viewsize","30");
            size = 30;
        } else if (cg_viewsize.integer > 100) {
            trap_Cvar_Set ("cg_viewsize","100");
            size = 100;
        } else {
            size = cg_viewsize.integer;
        }

    }
    cg.refdef.width = cgs.glconfig.vidWidth*size/100;
    cg.refdef.width &= ~1;

    cg.refdef.height = cgs.glconfig.vidHeight*size/100;
    cg.refdef.height &= ~1;

    cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
    cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView( void ) {
    vec3_t		forward, right, up;
    vec3_t		view;
    vec3_t		focusAngles;
    trace_t		trace;
    static vec3_t	mins = { -4, -4, -4 };
    static vec3_t	maxs = { 4, 4, 4 };
    vec3_t		focusPoint;
    float		focusDist;
    float		forwardScale, sideScale;

    cg.refdef.vieworg[2] += cg.predictedPlayerState.viewheight;

    VectorCopy( cg.refdefViewAngles, focusAngles );

    // if dead, look at killer
    if ( (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) &&
            (cgs.gametype !=GT_ELIMINATION && cgs.gametype !=GT_CTF_ELIMINATION && cgs.gametype !=GT_LMS) ) {
        focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
        cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
    }

    if ( focusAngles[PITCH] > 45 ) {
        focusAngles[PITCH] = 45;		// don't go too far overhead
    }
    AngleVectors( focusAngles, forward, NULL, NULL );

    VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );

    VectorCopy( cg.refdef.vieworg, view );

    view[2] += 8;

    cg.refdefViewAngles[PITCH] *= 0.5;

    AngleVectors( cg.refdefViewAngles, forward, right, up );

    forwardScale = cos( cg_thirdPersonAngle.value / 180 * M_PI );
    sideScale = sin( cg_thirdPersonAngle.value / 180 * M_PI );
    VectorMA( view, -cg_thirdPersonRange.value * forwardScale, forward, view );
    VectorMA( view, -cg_thirdPersonRange.value * sideScale, right, view );

    // trace a ray from the origin to the viewpoint to make sure the view isn't
    // in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

    if (!cg_cameraMode.integer) {
        CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

        if ( trace.fraction != 1.0 ) {
            VectorCopy( trace.endpos, view );
            view[2] += (1.0 - trace.fraction) * 32;
            // try another trace to this position, because a tunnel may have the ceiling
            // close enogh that this is poking out

            CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
            VectorCopy( trace.endpos, view );
        }
    }


    VectorCopy( view, cg.refdef.vieworg );

    // select pitch to look at focus point from vieword
    VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
    focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
    if ( focusDist < 1 ) {
        focusDist = 1;	// should never happen
    }
    cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );
    cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
    int		timeDelta;

    // smooth out stair climbing
    timeDelta = cg.time - cg.stepTime;
    if ( timeDelta < STEP_TIME ) {
        cg.refdef.vieworg[2] -= cg.stepChange
                                * (STEP_TIME - timeDelta) / STEP_TIME;
    }
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
    float			*origin;
    float			*angles;
    float			bob;
    float			ratio;
    float			delta;
    float			speed;
    float			f;
    vec3_t			predictedVelocity;
    int				timeDelta;


    if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
        return;
    }

    origin = cg.refdef.vieworg;
    angles = cg.refdefViewAngles;

    // if dead, fix the angle and don't add any kick
    if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
        angles[ROLL] = 0;
        angles[PITCH] = 0;
        angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
        origin[2] += cg.predictedPlayerState.viewheight;
        return;
    }
	

    if (!cg_nokick.integer) {
        // add angles based on weapon kick
        VectorAdd (angles, cg.kick_angles, angles);

        // add angles based on damage kick
        if ( cg.damageTime && cgs.gametype!=GT_ELIMINATION && cgs.gametype!=GT_CTF_ELIMINATION && cgs.gametype!=GT_LMS) {
            ratio = cg.time - cg.damageTime;
            if ( ratio < DAMAGE_DEFLECT_TIME ) {
                ratio /= DAMAGE_DEFLECT_TIME;
                angles[PITCH] += ratio * cg.v_dmg_pitch;
                angles[ROLL] += ratio * cg.v_dmg_roll;
            } else {
                ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
                if ( ratio > 0 ) {
                    angles[PITCH] += ratio * cg.v_dmg_pitch;
                    angles[ROLL] += ratio * cg.v_dmg_roll;
                }
            }
        }
    }

    // add pitch based on fall kick
#if 0
    ratio = ( cg.time - cg.landTime) / FALL_TIME;
    if (ratio < 0)
        ratio = 0;
    angles[PITCH] += ratio * cg.fall_value;
#endif

    // add angles based on velocity
    VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

    delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
    angles[PITCH] += delta * cg_runpitch.value;

    delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
    angles[ROLL] -= delta * cg_runroll.value;

    // add angles based on bob

    // make sure the bob is visible even at low speeds
    speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

    delta = cg.bobfracsin * cg_bobpitch.value * speed;
    if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
        delta *= 3;		// crouching
    angles[PITCH] += delta;
    delta = cg.bobfracsin * cg_bobroll.value * speed;
    if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
        delta *= 3;		// crouching accentuates roll
    if (cg.bobcycle & 1)
        delta = -delta;
    angles[ROLL] += delta;

//===================================

    // add view height
    origin[2] += cg.predictedPlayerState.viewheight;

    // smooth out duck height changes
    timeDelta = cg.time - cg.duckTime;
    if ( timeDelta < DUCK_TIME) {
        cg.refdef.vieworg[2] -= cg.duckChange
                                * (DUCK_TIME - timeDelta) / DUCK_TIME;
    }

    // add bob height
    bob = cg.bobfracsin * cg.xyspeed * cg_bobup.value;
    if (bob > 6) {
        bob = 6;
    }

    origin[2] += bob;


    // add fall height
    delta = cg.time - cg.landTime;
    if ( delta < LAND_DEFLECT_TIME ) {
        f = delta / LAND_DEFLECT_TIME;
        cg.refdef.vieworg[2] += cg.landChange * f;
    } else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
        delta -= LAND_DEFLECT_TIME;
        f = 1.0 - ( delta / LAND_RETURN_TIME );
        cg.refdef.vieworg[2] += cg.landChange * f;
    }

    // add step offset
    CG_StepOffset();

    // add kick offset

    VectorAdd (origin, cg.kick_origin, origin);

    // pivot the eye based on a neck length
#if 0
    {
#define	NECK_LENGTH		8
        vec3_t			forward, up;

        cg.refdef.vieworg[2] -= NECK_LENGTH;
        AngleVectors( cg.refdefViewAngles, forward, NULL, up );
        VectorMA( cg.refdef.vieworg, 3, forward, cg.refdef.vieworg );
        VectorMA( cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg );
    }
#endif
}

//======================================================================

void CG_ZoomDown_f( void ) {
    if( cg_zoomToggle.integer ){
	if( cg.zoomed )
	    cg.zoomed = qfalse;
	else
	    cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
    }
    else{
	if ( cg.zoomed ) {
	    return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
    }
}

void CG_ZoomUp_f( void ) {
    if( cg_zoomToggle.integer )
	return;
    if ( !cg.zoomed ) {
        return;
    }
    cg.zoomed = qfalse;
    cg.zoomTime = cg.time;
}

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void ) {
    float	x;
    float	phase;
    float	v;
    int		contents;
    float	fov_x, fov_y;
    float	zoomFov;
    float	f;
    int		inwater;

    if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
        // if in intermission, use a fixed value
        fov_x = 90;
    } else {
        // user selectable
        if ( cgs.dmflags & DF_FIXED_FOV ) {
            // dmflag to prevent wide fov for all clients
            fov_x = 90;
        } else {
            fov_x = cgs.fovs[cg.predictedPlayerState.weapon];//cg_fov.value;
            if ( fov_x < 1 ) {
                fov_x = 1;
            } else if ( fov_x > 160 ) {
                fov_x = 160;
            }
            if( cg.currentFov != fov_x && cg_smoothFovChange.integer  && cg_zoomScaling.value > 0.0 ){
		cg.lastFov = cg.currentFov;
		cg.currentFov = fov_x;
		cg.fovTime = cg.time;
	    }
        }

        if ( cgs.dmflags & DF_FIXED_FOV ) {
            // dmflag to prevent wide fov for all clients
            zoomFov = 22.5;
        } else {
            // account for zooms
            zoomFov = cgs.zoomfovs[cg.predictedPlayerState.weapon];//cg_zoomFov.value;
            if ( zoomFov < 1 ) {
                zoomFov = 1;
            } else if ( zoomFov > 160 ) {
                zoomFov = 160;
            }

            /*if ( (cgs.fairflags & FF_LOCK_CVARS_BASIC) && zoomFov>140 )
                zoomFov = 140;*/
        }

        if ( cg.zoomed ) {
	    if( cg_zoomScaling.value > 0 )
		f = ( cg.time - cg.zoomTime ) / (float)(cg_zoomScaling.value * ZOOM_TIME);
	    else
		f = 2.0;
            if ( f > 1.0 ) {
                fov_x = zoomFov;
            } else {
                fov_x = fov_x + f * ( zoomFov - fov_x );
            }
        } else {
	    if( cg_zoomScaling.value > 0 )
		f = ( cg.time - cg.zoomTime ) / (float)(cg_zoomScaling.value * ZOOM_TIME);
	    else
		f = 2.0;
          
            if ( f > 1.0 ) {
                //fov_x = fov_x;
		
		if( cg_smoothFovChange.integer )
		    f = ( cg.time - cg.fovTime ) / (float)(cg_zoomScaling.value * ZOOM_TIME);
		else
		    f = 2.0;
		
		if( f <= 1.0 )
		    fov_x = cg.lastFov + f * ( fov_x - cg.lastFov );
		
            } else {
                fov_x = zoomFov + f * ( fov_x - zoomFov );
            }
        }
    }

    if ( cg_fovAdjust.integer ) {
        // Based on LordHavoc's code for Darkplaces
        // http://www.quakeworld.nu/forum/topic/53/what-does-your-qw-look-like/page/30
        const float baseAspect = 0.75f; // 3/4
        const float aspect = (float)cg.refdef.width/(float)cg.refdef.height;
        const float desiredFov = fov_x;

        fov_x = atan2( tan( desiredFov * M_PI / 360.0f ) * baseAspect * aspect, 1 ) * 360.0f / M_PI;
    }

    x = cg.refdef.width / tan( fov_x / 360 * M_PI );
    fov_y = atan2( cg.refdef.height, x );
    fov_y = fov_y * 360 / M_PI;

    if ( !cg_waterWarp.integer ) {
        inwater = qfalse;
    } else {
        // warp if underwater
        contents = CG_PointContents( cg.refdef.vieworg, -1 );
        if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
            phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
            v = WAVE_AMPLITUDE * sin( phase );
            fov_x += v;
            fov_y -= v;
            inwater = qtrue;
        }
        else {
            inwater = qfalse;
        }
    }

    // set it
    cg.refdef.fov_x = fov_x;
    cg.refdef.fov_y = fov_y;

    if ( !cg.zoomed ) {
        cg.zoomSensitivity = 1;
    } else {
        cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
    }

    return inwater;
}



/*
===============
CG_DamageBlendBlob

===============
*/
static void CG_DamageBlendBlob( void ) {
    int			t;
    int			maxTime;
    refEntity_t		ent;

    if ( !cg.damageValue ) {
        return;
    }

    //if (cg.cameraMode) {
    //	return;
    //}

    // ragePro systems can't fade blends, so don't obscure the screen
    if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
        return;
    }

    maxTime = DAMAGE_TIME;
    t = cg.time - cg.damageTime;
    if ( t <= 0 || t >= maxTime ) {
        return;
    }


    memset( &ent, 0, sizeof( ent ) );
    ent.reType = RT_SPRITE;
    ent.renderfx = RF_FIRST_PERSON;

    VectorMA( cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin );
    VectorMA( ent.origin, cg.damageX * -10, cg.refdef.viewaxis[1], ent.origin );
    VectorMA( ent.origin, cg.damageY * 6, cg.refdef.viewaxis[2], ent.origin );

    ent.radius = cg.damageValue * 3;
    ent.customShader = cgs.media.viewBloodShader;
    ent.shaderRGBA[0] = 255;
    ent.shaderRGBA[1] = 255;
    ent.shaderRGBA[2] = 255;
    ent.shaderRGBA[3] = 200 * ( 1.0 - ((float)t / maxTime) );
    trap_R_AddRefEntityToScene( &ent );
}


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
    playerState_t	*ps;

    memset( &cg.refdef, 0, sizeof( cg.refdef ) );

    // strings for in game rendering
    // Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
    // Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

    // calculate size of 3D view
    CG_CalcVrect();

    ps = &cg.predictedPlayerState;
    /*
    	if (cg.cameraMode) {
    		vec3_t origin, angles;
    		if (trap_getCameraInfo(cg.time, &origin, &angles)) {
    			VectorCopy(origin, cg.refdef.vieworg);
    			angles[ROLL] = 0;
    			VectorCopy(angles, cg.refdefViewAngles);
    			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
    			return CG_CalcFov();
    		} else {
    			cg.cameraMode = qfalse;
    		}
    	}
    */
    // intermission view
    if ( ps->pm_type == PM_INTERMISSION ) {
        VectorCopy( ps->origin, cg.refdef.vieworg );
        VectorCopy( ps->viewangles, cg.refdefViewAngles );
        AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
        return CG_CalcFov();
    }

    cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
    cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
    cg.xyspeed = sqrt( ps->velocity[0] * ps->velocity[0] +
                       ps->velocity[1] * ps->velocity[1] );


    VectorCopy( ps->origin, cg.refdef.vieworg );
    VectorCopy( ps->viewangles, cg.refdefViewAngles );

    if (cg_cameraOrbit.integer) {
        if (cg.time > cg.nextOrbitTime) {
            cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
            cg_thirdPersonAngle.value += cg_cameraOrbit.value;
        }
    }
    // add error decay
    if ( cg_errorDecay.value > 0 ) {
        int		t;
        float	f;

        t = cg.time - cg.predictedErrorTime;
        f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
        if ( f > 0 && f < 1 ) {
            VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
        } else {
            cg.predictedErrorTime = 0;
        }
    }

    if ( cg.renderingThirdPerson ) {
        // back away from character
        CG_OffsetThirdPersonView();
    } else {
        // offset for local bobbing and kicks
        CG_OffsetFirstPersonView();
    }

    // position eye reletive to origin
    AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

    if ( cg.hyperspace ) {
        cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
    }

    // field of view
    return CG_CalcFov();
}


/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds( void ) {
    int		i;
    int		t;

    // powerup timers going away
    for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
        t = cg.snap->ps.powerups[i];
        if ( t <= cg.time ) {
            continue;
        }
        if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
            continue;
        }
        if ( ( t - cg.time ) / POWERUP_BLINK_TIME != ( t - cg.oldTime ) / POWERUP_BLINK_TIME ) {
            trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_ITEM, cgs.media.wearOffSound );
        }
    }
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
    if ( !sfx )
        return;
    cg.soundBuffer[cg.soundBufferIn] = sfx;
    cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
    if (cg.soundBufferIn == cg.soundBufferOut) {
        cg.soundBufferOut++;
    }
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
    if ( cg.soundTime < cg.time ) {
        if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
            trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
            cg.soundBuffer[cg.soundBufferOut] = 0;
            cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
            //cg.soundTime = cg.time + 750;
	    cg.soundTime = cg.time + 1500;
        }
    }
}

/*
=====================
CG_SetMultiviewRect
=====================
*/
static void CG_SetMultiviewRect( int j ) {
    switch (j) {
    case 2:
        cg.refdef.x = cg_multiview2_xpos.integer * cgs.screenXScale;
        cg.refdef.y = cg_multiview2_ypos.integer * cgs.screenYScale;
        cg.refdef.width = cg_multiview2_width.integer * cgs.screenXScale;
        cg.refdef.height = cg_multiview2_height.integer * cgs.screenYScale;
        break;
    case 3:
        cg.refdef.x = cg_multiview3_xpos.integer * cgs.screenXScale;
        cg.refdef.y = cg_multiview3_ypos.integer * cgs.screenYScale;
        cg.refdef.width = cg_multiview3_width.integer * cgs.screenXScale;
        cg.refdef.height = cg_multiview3_height.integer * cgs.screenYScale;
        break;
    case 4:
        cg.refdef.x = cg_multiview4_xpos.integer * cgs.screenXScale;
        cg.refdef.y = cg_multiview4_ypos.integer * cgs.screenYScale;
        cg.refdef.width = cg_multiview4_width.integer * cgs.screenXScale;
        cg.refdef.height = cg_multiview4_height.integer * cgs.screenYScale;
        break;
    }
}

/*
=====================
CG_EntityNumIsAllowed
=====================
*/
static qboolean CG_EntityNumIsAllowed( int i, int j ) {
    if ( cg_multiview.integer > 1 ) {
        if ( cg_multiview.integer == 2 ) {
            if ( j == 2 &&
                cg_multiview2_client.integer == i )
                return qtrue;
        }
        else if ( cg_multiview.integer == 3 ) {
            if ( j == 2 &&
                cg_multiview2_client.integer == i )
                return qtrue;
            if ( j == 3 &&
                cg_multiview3_client.integer == i )
                return qtrue;
        }
        else {
            if ( j == 2 &&
                cg_multiview2_client.integer == i )
                return qtrue;
            if ( j == 3 &&
                cg_multiview3_client.integer == i )
                return qtrue;
            if ( j == 4 &&
                cg_multiview4_client.integer == i )
                return qtrue;
        }
        return qfalse;
    }
    else
        return qfalse;
}

/*
=====================
CG_GetEntityNumForMV
=====================
*/
static int CG_GetEntityNumForMV( int window, int offset ) {
    int clientNum = -1;
    int i;

    switch ( window ) {
    case 2:
        clientNum = cg_multiview2_client.integer;
        break;
    case 3:
        clientNum = cg_multiview3_client.integer;
        break;
    case 4:
        clientNum = cg_multiview4_client.integer;
        break;
    default:
        return -1;
    }

    for ( i = offset; i < cg.snap->numEntities; i++ ) {
        if ( ( cg.snap->entities[i].eType == ET_PLAYER ) &&
             ( cg.snap->ps.clientNum != cg.snap->entities[i].clientNum ) &&
             ( clientNum == -1 || CG_EntityNumIsAllowed( cg.snap->entities[i].clientNum, window ) ) ) {
            if ( cg.snap->entities[i].eFlags & EF_DEAD )
                continue;
            return i;
        }
    }
    return -1;
}

/*
=====================
CG_AddMultiviewWindow
=====================
*/
static void CG_AddMultiviewWindow( stereoFrame_t stereoView ) {
    int i, j, start = 0;
    int health, armor, team, weapon;
    int ammo[8];
    float refdef[4];
    qboolean renderingThirdPerson;

    // Save the main-spec values, we will change the values for the MV-hud
    for ( i = WP_MACHINEGUN; i <= WP_BFG; i++ ) {
        ammo[i-WP_MACHINEGUN] = cg.snap->ps.ammo[i];
        cg.snap->ps.ammo[i] = -1;
    }

    health = cg.snap->ps.stats[STAT_HEALTH];
    armor = cg.snap->ps.stats[STAT_ARMOR];
    team = cg.snap->ps.persistant[PERS_TEAM];
    weapon = cg_entities[cg.snap->ps.clientNum].currentState.weapon;
    renderingThirdPerson = cg.renderingThirdPerson;
    refdef[0] = cg.refdef.x;
    refdef[1] = cg.refdef.y;
    refdef[2] = cg.refdef.width;
    refdef[3] = cg.refdef.height;

    for ( j = 1; j <= MAX_MULTIVIEW && j <= cg_multiview.integer; ++j ) {
        i = CG_GetEntityNumForMV( j, start );
        CG_SetMultiviewRect( j );

        if ( i == -1 )
            continue;
        else if ( ( j == 2 &&
                cg_multiview2_client.integer != -1 ) ||
                ( j == 3 &&
                cg_multiview3_client.integer != -1 ) ||
                ( j == 4 &&
                cg_multiview4_client.integer != -1 ) )
            start = 0;
        else
            start = i+1;

        VectorCopy( cg_entities[cg.snap->entities[i].clientNum].lerpOrigin, cg.refdef.vieworg );
        AnglesToAxis( cg_entities[cg.snap->entities[i].clientNum].lerpAngles, cg.refdef.viewaxis );

        cg.snap->ps.stats[STAT_HEALTH] = cgs.clientinfo[cg.snap->entities[i].clientNum].health;
        cg.snap->ps.stats[STAT_ARMOR] = cgs.clientinfo[cg.snap->entities[i].clientNum].armor;
        cg.snap->ps.persistant[PERS_TEAM] = cgs.clientinfo[cg.snap->entities[i].clientNum].team;
        cg_entities[cg.snap->ps.clientNum].currentState.weapon = cg.snap->entities[i].weapon;

        cg.refdef.vieworg[2] += cg.predictedPlayerState.viewheight;
        cg.window = j-1;

        // Show the main-spec model in the MV-windows
        cg.renderingThirdPerson = qtrue;

        // build the render lists
        if ( !cg.hyperspace ) {
            CG_AddPacketEntities( cg.snap->entities[i].clientNum );			// adter calcViewValues, so predicted player state is correct
            CG_AddMarks();
            CG_AddParticles ();
            CG_AddLocalEntities();
        }
        cg.renderingThirdPerson = renderingThirdPerson;

        // actually issue the rendering calls
        CG_DrawActive( stereoView, qfalse );
        CG_DrawMVDhud( stereoView, j-1 );

	CG_DrawStringHud ( HUD_FOLLOW, qtrue, va ( "following %s", cgs.clientinfo[ cg.snap->entities[i].clientNum ].name ) );
    }

    // Set everything back to the main-spec values
    cg.snap->ps.stats[STAT_HEALTH] = health;
    cg.snap->ps.stats[STAT_ARMOR] = armor;
    cg.snap->ps.persistant[PERS_TEAM] = team;
    cg_entities[cg.snap->ps.clientNum].currentState.weapon = weapon;
    cg.refdef.x = refdef[0];
    cg.refdef.y = refdef[1];
    cg.refdef.width = refdef[2];
    cg.refdef.height = refdef[3];

    for ( i = WP_MACHINEGUN; i <= WP_BFG; i++ ) {
        cg.snap->ps.ammo[i] = ammo[i-WP_MACHINEGUN];
    }
}

/*
=====================
CG_AddSpawnpoints
=====================
*/
void CG_AddSpawnpoints( void ){
	refEntity_t		re;
	int i;
	int 			dist[3] = {0,0,0};
	int 			distnum[3] = {-1,-1,-1};
	int 			distbuf;
	vec3_t 			delta;
	
	if( !cg_drawSpawnpoints.integer )
		return;
	
	//CG_Printf("AddSpawnpoints();\n");
	memset( &re, 0, sizeof(re));
	re.hModel = cgs.media.spawnPoint;
	re.customShader = cgs.media.spawnPointShader;
	
	if( cgs.gametype == GT_TOURNAMENT ){
		for( i=0; i < cg.numSpawnpoints; i++ ){
			VectorSubtract( cg.spawnOrg[i], cg.snap->ps.origin, delta );
			distbuf = VectorLength( delta );
			if( distbuf > dist[0] || distnum[0] == -1 ){
				dist[2] = dist[1];
				distnum[2] = distnum[1];
				dist[1]= dist[0];
				distnum[1] = distnum[0];
				
				dist[0] = distbuf;
				distnum[0] = i;
			} else if( distbuf > dist[1] || distnum[1] == -1 ){
				dist[2] = dist[1];
				distnum[2] = distnum[1];
				
				dist[1] = distbuf;
				distnum[1] = i;
			} else if( distbuf > dist[2] || distnum[2] == -1 ){
				dist[2] = distbuf;
				distnum[2] = i;
			}
		}
	}
	
	for( i=0; i < cg.numSpawnpoints; i++ ){
	    VectorCopy( cg.spawnOrg[i], re.origin);
	    AnglesToAxis(cg.spawnAngle[i], re.axis);
	    
	    if( cg.spawnTeam[i] == TEAM_FREE ){
		    if( cgs.gametype == GT_TOURNAMENT ) {
			    if( i == distnum[0] || i == distnum[1] || i == distnum[2] ) {
				    re.shaderRGBA[0] = 255;
				    re.shaderRGBA[1] = 0;
				    re.shaderRGBA[2] = 0;
				    re.shaderRGBA[3] = 255;
			    } else {
				    re.shaderRGBA[0] = 0;
				    re.shaderRGBA[1] = 255;
				    re.shaderRGBA[2] = 0;
				    re.shaderRGBA[3] = 255;
			    }
		    } else {
			    re.shaderRGBA[0] = 0;
			    re.shaderRGBA[1] = 255;
			    re.shaderRGBA[2] = 0;
			    re.shaderRGBA[3] = 255;
		    }
	    } else if( cg.spawnTeam[i] == TEAM_BLUE ){
		    re.shaderRGBA[0] = 0;
		    re.shaderRGBA[1] = 0;
		    re.shaderRGBA[2] = 255;
		    re.shaderRGBA[3] = 255;
	    } else if( cg.spawnTeam[i] == TEAM_RED ){
		    re.shaderRGBA[0] = 255;
		    re.shaderRGBA[1] = 0;
		    re.shaderRGBA[2] = 0;
		    re.shaderRGBA[3] = 255;
	    }
	    trap_R_AddRefEntityToScene(&re);
	}
}

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
    int inwater;
    vec4_t color;

    cg.time = serverTime;

    cg.demoPlayback = demoPlayback;

    // update cvars
    CG_UpdateCvars();

    // if we are only updating the screen as a loading
    // pacifier, don't even try to read snapshots
    if ( cg.infoScreenText[0] != 0 ) {
        CG_DrawInformation();
        return;
    }

    // any looped sounds will be respecified as entities
    // are added to the render list
    trap_S_ClearLoopingSounds(qfalse);

    // clear all the render lists
    trap_R_ClearScene();

    // set up cg.snap and possibly cg.nextSnap
    CG_ProcessSnapshots();

    // if we haven't received any snapshots yet, all
    // we can draw is the information screen
    if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
        CG_DrawInformation();
        return;
    }

    // let the client system know what our weapon and zoom settings are
    trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

    // this counter will be bumped for every valid scene we generate
    cg.clientFrame++;

    // update cg.predictedPlayerState
    CG_PredictPlayerState();

    // decide on third person view
    cg.renderingThirdPerson = cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR
        && (cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0));

    // build cg.refdef
    inwater = CG_CalcViewValues();

    // first person blend blobs, done after AnglesToAxis
    if ( !cg.renderingThirdPerson ) {
        CG_DamageBlendBlob();
    }

    // build the render lists
    if ( !cg.hyperspace ) {
        CG_AddPacketEntities( -1 );			// adter calcViewValues, so predicted player state is correct
        CG_AddMarks();
        CG_AddParticles ();
        CG_AddLocalEntities();
	if ( cg.warmup != 0 /*&& cgs.gametype < GT_TEAM*/ )
		CG_AddSpawnpoints();
    }
    CG_AddViewWeapon( &cg.predictedPlayerState );

    // add buffered sounds
    CG_PlayBufferedSounds();

    // finish up the rest of the refdef
    if ( cg.testModelEntity.hModel ) {
        CG_AddTestModel();
    }
    cg.refdef.time = cg.time;
    memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

    // warning sounds when powerup is wearing off
    CG_PowerupTimerSounds();

    // update audio positions
    trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

    // make sure the lagometerSample and frame timing isn't done twice when in stereo
    if ( stereoView != STEREO_RIGHT ) {
        cg.frametime = cg.time - cg.oldTime;
        if ( cg.frametime < 0 ) {
            cg.frametime = 0;
        }
        cg.oldTime = cg.time;
        CG_AddLagometerFrameInfo();
    }
    if (cg_timescale.value != cg_timescaleFadeEnd.value) {
        if (cg_timescale.value < cg_timescaleFadeEnd.value) {
            cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
            if (cg_timescale.value > cg_timescaleFadeEnd.value)
                cg_timescale.value = cg_timescaleFadeEnd.value;
        }
        else {
            cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
            if (cg_timescale.value < cg_timescaleFadeEnd.value)
                cg_timescale.value = cg_timescaleFadeEnd.value;
        }
        if (cg_timescaleFadeSpeed.value) {
            trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
        }
    }
    
    // clear around the rendered view if sized down
    CG_TileClear();

    //---------------------Multiview-------------------------------

    // actually issue the rendering calls
    CG_DrawActive( stereoView, qtrue );
    
    if( cg.snap->ps.stats[STAT_HEALTH] <= 0 &&
        cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR &&
        cgs.fadeToBlack ) {
        if( cg.deathtime == 0 )
            cg.deathtime = cg.time;
        if( !( cgs.allowMultiview && (
            cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR ) &&
            cg_multiview.integer > 1 ) ) {
                color[0] = colorBlack[0];
                color[1] = colorBlack[1];
                color[2] = colorBlack[2];
                color[3] = (cg.time - cg.deathtime)/3000.0f;
                if( color[3] > 0.3f )
                    color[3] = 0.3f;
                CG_FillRect(0, 0, 640, 480, color);
        }
    }
    else
        cg.deathtime = 0;
    
    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
            ( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) )
        return;

    if ( cg_stats.integer ) {
        CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
    }

    if ( cgs.allowMultiview && (
        cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR ) &&
        cg_multiview.integer > 1 ) {
        CG_DrawMVDhud( stereoView, qfalse );
        CG_AddMultiviewWindow( stereoView );
        cg.window = qfalse;
        CG_Draw2D( stereoView, qtrue );
    }
    else {
        cg.window = qfalse;
        CG_Draw2D( stereoView, qfalse );
    }

    //---------------------------Multiview end----------------------


}

