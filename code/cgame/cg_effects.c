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
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"


/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing ) {
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;

	if ( cg_noProjectileTrail.integer || !cg_bubbleTrail.integer ) {
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		le->pos.trDelta[0] = crandom()*5;
		le->pos.trDelta[1] = crandom()*5;
		le->pos.trDelta[2] = crandom()*5 + 6;

		VectorAdd (move, vec, move);
	}
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ) {
	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;
//	int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
	re->rotation = Q_random( &seed ) * 360;
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = r;
	le->color[1] = g; 
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = hShader;

	// rage pro can't alpha fade, so use a different shader
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		re->customShader = cgs.media.smokePuffRageProShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
	} else {
		re->shaderRGBA[0] = le->color[0] * 0xff;
		re->shaderRGBA[1] = le->color[1] * 0xff;
		re->shaderRGBA[2] = le->color[2] * 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

/*
==================
CG_LaunchSparks
==================
*/
void CG_LaunchSparks( vec3_t origin ) {
	int number; // number of particles
	int jump; // amount to nudge the particles trajectory vector up by
	int speed; // speed of particles
	int light; // amount of light for each particle
	vec4_t lColor; // color of light for each particle
	qhandle_t shader; // shader to use for the particles
	int index;
	vec3_t randVec, tempVec;
	
	if( !cg_particles.integer )
		return;

	// set defaults
	number = 64;
	jump = 50;
	speed = 100;
	light = 50;
	lColor[0] = 1.0f;
	lColor[1] = 1.0f;
	lColor[2] = 1.0f;
	lColor[3] = 1.0f; // alpha
	
	shader = cgs.media.particlePlasma;

	for( index = 0; index < number; index++ ) {
		localEntity_t *le;
		refEntity_t *re;

		le = CG_AllocLocalEntity(); //allocate a local entity
		re = &le->refEntity;
		le->leFlags = LEF_PUFF_DONT_SCALE; //don't change the particle size
		le->leType = LE_MOVE_SCALE_FADE;// particle should fade over time
		le->startTime = cg.time; // set the start time of the particle to the current time
		le->endTime = cg.time + 1000 + random() * 1000; //set the end time
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 5*random();
		re->customShader = shader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
		le->light = light;
		VectorCopy( lColor, le->lightColor );
		le->color[3] = 1.0;
		le->pos.trType = TR_LINEAR; // moves in a gravity affected arc
		le->pos.trTime = cg.time;
		le->bounceFactor = 0.0f;
		VectorCopy( origin, le->pos.trBase );
		VectorCopy( origin, re->origin );

		tempVec[0] = crandom(); //between 1 and -1
		tempVec[1] = crandom();
		tempVec[2] = crandom();	
		
		VectorNormalize(tempVec);
		VectorScale(tempVec, speed, randVec);
		randVec[2] += jump; //nudge the particles up a bit
		VectorCopy( randVec, le->pos.trDelta );
	}
}

/*
==================
CG_LaunchParticleTrail
==================
*/
void CG_LaunchParticleTrail( vec3_t origin ) {
	int number; // number of particles
	int jump; // amount to nudge the particles trajectory vector up by
	int speed; // speed of particles
	int light; // amount of light for each particle
	vec4_t lColor; // color of light for each particle
	qhandle_t shader; // shader to use for the particles
	int index;
	vec3_t randVec, tempVec;
	
	if( !cg_particles.integer )
		return;

	// set defaults
	number = 3;
	jump = 0;
	speed = 30;
	light = 50;
	lColor[0] = 1.0f;
	lColor[1] = 0.3f;
	lColor[2] = 0.3f;
	lColor[3] = 1.0f; // alpha
	
	shader = cgs.media.particleSpark;

	for( index = 0; index < number; index++ ) {
		localEntity_t *le;
		refEntity_t *re;

		le = CG_AllocLocalEntity(); //allocate a local entity
		re = &le->refEntity;
		le->leFlags = LEF_PUFF_DONT_SCALE; //don't change the particle size
		le->leType = LE_MOVE_SCALE_FADE;// particle should fade over time
		le->startTime = cg.time; // set the start time of the particle to the current time
		le->endTime = cg.time + 500 + random() * 300; //set the end time
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 10*random();
		re->customShader = shader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
		le->light = light;
		VectorCopy( lColor, le->lightColor );
		le->color[3] = 1.0;
		le->pos.trType = TR_LINEAR; // moves in a gravity affected arc
		le->pos.trTime = cg.time;
		le->bounceFactor = 0.0f;
		VectorCopy( origin, le->pos.trBase );
		VectorCopy( origin, re->origin );

		tempVec[0] = crandom(); //between 1 and -1
		tempVec[1] = crandom();
		tempVec[2] = crandom();	
		
		VectorNormalize(tempVec);
		VectorScale(tempVec, speed, randVec);
		randVec[2] += jump; //nudge the particles up a bit
		VectorCopy( randVec, le->pos.trDelta );
	}
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
void CG_SpawnEffect( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 500;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

/*#ifndef MISSIONPACK
	re->customShader = cgs.media.teleportEffectShader;
#endif*/
	re->hModel = cgs.media.teleportEffectModel;
	AxisClear( re->axis );

	VectorCopy( org, re->origin );
//#ifdef MISSIONPACK
	re->origin[2] += 16;
/*#else
	re->origin[2] -= 24;
#endif*/
	CG_LaunchSparks( org );
		
}

/*
===============
CG_LightningBoltBeam
===============
*/
void CG_LightningBoltBeam( vec3_t start, vec3_t end ) {
	localEntity_t	*le;
	refEntity_t		*beam;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SHOWREFENTITY;
	le->startTime = cg.time;
	le->endTime = cg.time + 50;

	beam = &le->refEntity;

	VectorCopy( start, beam->origin );
	// this is the end point
	VectorCopy( end, beam->oldorigin );

	beam->reType = RT_LIGHTNING;
	beam->customShader = cgs.media.lightningShader[0];
}

/*
==================
CG_KamikazeEffect
==================
*/
void CG_KamikazeEffect( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_KAMIKAZE;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;//2250;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.kamikazeEffectModel;

	VectorCopy( org, re->origin );

}

/*
==================
CG_ObeliskExplode
==================
*/
void CG_ObeliskExplode( vec3_t org, int entityNum ) {
	localEntity_t	*le;
	vec3_t origin;

	// create an explosion
	VectorCopy( org, origin );
	origin[2] += 64;
	le = CG_MakeExplosion( origin, vec3_origin,
						   cgs.media.dishFlashModel,
						   cgs.media.rocketExplosionShader,
						   600, qtrue );
	le->light = 300;
	le->lightColor[0] = 1;
	le->lightColor[1] = 0.75;
	le->lightColor[2] = 0.0;
}

/*
==================
CG_ObeliskPain
==================
*/
void CG_ObeliskPain( vec3_t org ) {
	float r;
	sfxHandle_t sfx;

	// hit sound
	r = rand() & 3;
	if ( r < 2 ) {
		sfx = cgs.media.obeliskHitSound1;
	} else if ( r == 2 ) {
		sfx = cgs.media.obeliskHitSound2;
	} else {
		sfx = cgs.media.obeliskHitSound3;
	}
	trap_S_StartSound ( org, ENTITYNUM_NONE, CHAN_BODY, sfx );
}


/*
==================
CG_InvulnerabilityImpact
==================
*/
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles ) {
	localEntity_t	*le;
	refEntity_t		*re;
	int				r;
	sfxHandle_t		sfx;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULIMPACT;
	le->startTime = cg.time;
	le->endTime = cg.time + 1000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityImpactModel;

	VectorCopy( org, re->origin );
	AnglesToAxis( angles, re->axis );

	r = rand() & 3;
	if ( r < 2 ) {
		sfx = cgs.media.invulnerabilityImpactSound1;
	} else if ( r == 2 ) {
		sfx = cgs.media.invulnerabilityImpactSound2;
	} else {
		sfx = cgs.media.invulnerabilityImpactSound3;
	}
	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, sfx );
}

/*
==================
CG_InvulnerabilityJuiced
==================
*/
void CG_InvulnerabilityJuiced( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULJUICED;
	le->startTime = cg.time;
	le->endTime = cg.time + 10000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityJuicedModel;

	VectorCopy( org, re->origin );
	VectorClear(angles);
	AnglesToAxis( angles, re->axis );

	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, cgs.media.invulnerabilityJuicedSound );
}

/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum( int client, vec3_t org, int score ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;
	static vec3_t lastPos;

	// only visualize for the client that scored
	if (client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0) {
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;
	le->startTime = cg.time;
	le->endTime = cg.time + 4000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = score;
	
	VectorCopy( org, le->pos.trBase );
	if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20) {
		le->pos.trBase[2] -= 20;
	}

	//CG_Printf( "Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis( angles, re->axis );
}

/*
==================
CG_ParseDamagePlum
==================
*/
#define MAX_DAMAGEPLUM_LENGTH 38 //currently the longest string is "/g/mg/sg/gl/rl/lg/rg/pg/bfg/cg/ng/pl/\0" (38 chars)
qboolean CG_ParseDamagePlum( int mod ) {
	int length;
	char damagePlum[MAX_CVAR_VALUE_STRING];

	trap_Cvar_VariableStringBuffer( "cg_damagePlum", damagePlum, sizeof( damagePlum ) );
	length = strlen(cg_damagePlum.string);
	if ( ( length > MAX_DAMAGEPLUM_LENGTH-1 ) || ( strcmp(&damagePlum[0], "/") && strcmp(&damagePlum[length-1], "/") ) )
		return qtrue;

	if ((Q_stristr(damagePlum,"/g/") && mod == MOD_GAUNTLET) ||
		(Q_stristr(damagePlum,"/mg/") && mod == MOD_MACHINEGUN) ||
		(Q_stristr(damagePlum,"/sg/") && mod == MOD_SHOTGUN) ||
		(Q_stristr(damagePlum,"/gl/") && mod == MOD_GRENADE) ||
		(Q_stristr(damagePlum,"/gl/") && mod == MOD_GRENADE_SPLASH) ||
		(Q_stristr(damagePlum,"/rl/") && mod == MOD_ROCKET) ||
		(Q_stristr(damagePlum,"/rl/") && mod == MOD_ROCKET_SPLASH) ||
		(Q_stristr(damagePlum,"/lg/") && mod == MOD_LIGHTNING) ||
		(Q_stristr(damagePlum,"/rg/") && mod == MOD_RAILGUN) ||
		(Q_stristr(damagePlum,"/pg/") && mod == MOD_PLASMA) ||
		(Q_stristr(damagePlum,"/pg/") && mod == MOD_PLASMA_SPLASH) ||
		(Q_stristr(damagePlum,"/bfg/") && mod == MOD_BFG) ||
		(Q_stristr(damagePlum,"/bfg/") && mod == MOD_BFG_SPLASH) ||
		(Q_stristr(damagePlum,"/cg/") && mod == MOD_CHAINGUN) ||
		(Q_stristr(damagePlum,"/ng/") && mod == MOD_NAIL) ||
		(Q_stristr(damagePlum,"/pl/") && mod == MOD_PROXIMITY_MINE))
			return qtrue;
		else
			return qfalse;
}

/*
==================
CG_DamagePlum
==================
*/
void CG_DamagePlum( int client, vec3_t org, int damage, int mod ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;
	static vec3_t lastPos;
	qboolean retDamagePlum;

	retDamagePlum = CG_ParseDamagePlum ( mod );

	// only visualize for the client that damaged
	if (client != cg.predictedPlayerState.clientNum || cg_damagePlums.integer == 0 || !retDamagePlum) {
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_DAMAGEPLUM;
	le->startTime = cg.time;
	le->endTime = cg.time + 1000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = damage;
	le->light = mod;
	
	VectorCopy( org, le->pos.trBase );
	if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20) {
		le->pos.trBase[2] -= 20;
	}

	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 12;

	VectorClear(angles);
	AnglesToAxis( angles, re->axis );
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, qhandle_t shader,
								int msec, qboolean isSprite ) {
	float			ang;
	localEntity_t	*ex;
	int				offset;
	vec3_t			tmpVec, newOrigin;

	if ( msec <= 0 ) {
		CG_Error( "CG_MakeExplosion: msec = %i", msec );
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if ( isSprite ) {
		ex->leType = LE_SPRITE_EXPLOSION;

		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale( dir, 16, tmpVec );
		VectorAdd( tmpVec, origin, newOrigin );
	} else {
		ex->leType = LE_EXPLOSION;
		VectorCopy( origin, newOrigin );

		// set axis with random rotate
		if ( !dir ) {
			AxisClear( ex->refEntity.axis );
		} else {
			ang = rand() % 360;
			VectorCopy( dir, ex->refEntity.axis[0] );
			RotateAroundDirection( ex->refEntity.axis, ang );
		}
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy( newOrigin, ex->refEntity.origin );
	VectorCopy( newOrigin, ex->refEntity.oldorigin );

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

	return ex;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed( vec3_t origin, int entityNum ) {
	localEntity_t	*ex;

	if ( !cg_hitMarks.integer ) {
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;

	ex->startTime = cg.time;
	
	
	VectorCopy ( origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	if( cg_blood.integer ){
		ex->endTime = ex->startTime + 500;
		ex->refEntity.radius = 24;
		ex->refEntity.customShader = cgs.media.bloodExplosionShader;
	}
	else{
		ex->endTime = ex->startTime + 1500;
		ex->refEntity.radius = 24;
		ex->refEntity.customShader = cgs.media.particleSpark;
	}
	// don't show player's own blood in view
	if ( entityNum == cg.snap->ps.clientNum ) {
		ex->refEntity.renderfx |= RF_THIRD_PERSON;
	}
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchGib( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.6f;

	le->leBounceSoundType = LEBS_BLOOD;
	le->leMarkType = LEMT_BLOOD;
}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		if( cg_gibs.integer ){
			VectorCopy( playerOrigin, origin );
			CG_LaunchSparks( origin );
		}
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	if ( rand() & 1 ) {
		CG_LaunchGib( origin, velocity, cgs.media.gibSkull );
	} else {
		CG_LaunchGib( origin, velocity, cgs.media.gibBrain );
	}

	// allow gibs to be turned off for speed
	if ( !cg_gibs.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibAbdomen );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibArm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibChest );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFist );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFoot );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibForearm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibIntestine );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchExplode( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 6000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;

	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
void CG_BigExplode( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*1.5;
	velocity[1] = crandom()*EXP_VELOCITY*1.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*2.0;
	velocity[1] = crandom()*EXP_VELOCITY*2.0;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*2.5;
	velocity[1] = crandom()*EXP_VELOCITY*2.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );
}

