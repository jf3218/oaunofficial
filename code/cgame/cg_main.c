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
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include "../game/bg_promode.h"

#ifdef MISSIONPACK
#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;
#endif

int forceModelModificationCount = -1;
int blueTeamModelModificationCount = -1;
int redTeamModelModificationCount = -1;
int enemyModelModificationCount = -1;
int teamModelModificationCount = -1;
int forceTeamModelsModificationCount = -1;

int hudModificationCount = -1;

int fovModificationCount = -1;
int zoomfovModificationCount = -1;

int crosshairModificationCount = -1;
int crosshairsizeModificationCount = -1;

int hitBeepModificationCount = -1;

int nomipModificationCount = 1;


void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );
void CG_oaUnofficialCvars( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2 );
		
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		CG_oaUnofficialCvars();
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
#ifdef MISSIONPACK
		cgDC.cursorx = cgs.cursorX;
		cgDC.cursory = cgs.cursorY;
#endif
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t			cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];


vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_ammoWarningSound;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_drawSpeed;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_chatTime;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_noChat;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_scorePlum;
//unlagged - smooth clients #2
// this is done server-side now
//vmCvar_t 	cg_smoothClients;
//unlagged - smooth clients #2
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	pmove_float;
vmCvar_t	pmove_accurate;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_noTaunt;
vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_oldRail;
vmCvar_t	cg_oldRocket;
vmCvar_t	cg_plasmaTrail;
vmCvar_t	cg_trueLightning;
vmCvar_t        cg_music;


#ifdef MISSIONPACK
vmCvar_t 	cg_redTeamName;
vmCvar_t 	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
vmCvar_t	cg_singlePlayer;
vmCvar_t	cg_singlePlayerActive;
vmCvar_t	cg_recordSPDemo;
vmCvar_t	cg_recordSPDemoName;
#endif
vmCvar_t	cg_obeliskRespawnDelay;
vmCvar_t	cg_enableDust;
vmCvar_t	cg_enableBreath;

//unlagged - client options
vmCvar_t	cg_delag;
//vmCvar_t	cg_debugDelag;
//vmCvar_t	cg_drawBBox;
vmCvar_t	cg_cmdTimeNudge;
vmCvar_t	sv_fps;
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
vmCvar_t	cl_timeNudge;
//vmCvar_t	cg_latentSnaps;
//vmCvar_t	cg_latentCmds;
//vmCvar_t	cg_plOut;
//unlagged - client options

//elimination addition
vmCvar_t	cg_hitBeep;
vmCvar_t        cg_voip_teamonly;
vmCvar_t        cg_voteflags;
vmCvar_t        cg_cyclegrapple;
vmCvar_t        cg_vote_custom_commands;

vmCvar_t                cg_autovertex;

vmCvar_t	cg_crosshairPulse;

vmCvar_t	cg_crosshairColor;

vmCvar_t 	cg_deathNoticeTime;

vmCvar_t	s_ambient;

vmCvar_t	cg_nokick;

vmCvar_t	cg_hiResCharset;

vmCvar_t	cg_weaponBobbing;

vmCvar_t	cg_blueteammodel;
vmCvar_t	cg_redteammodel;
vmCvar_t	cg_enemymodel;
vmCvar_t	cg_teammodel;
vmCvar_t	cg_forceteammodels;

vmCvar_t	cg_blueHeadColor;
vmCvar_t	cg_blueTorsoColor;
vmCvar_t	cg_blueLegsColor;

vmCvar_t	cg_redHeadColor;
vmCvar_t	cg_redTorsoColor;
vmCvar_t	cg_redLegsColor;

vmCvar_t	cg_enemyHeadColor;
vmCvar_t	cg_enemyTorsoColor;
vmCvar_t	cg_enemyLegsColor;

vmCvar_t	cg_teamHeadColor;
vmCvar_t	cg_teamTorsoColor;
vmCvar_t	cg_teamLegsColor;

vmCvar_t	cg_deadBodyDarken;
vmCvar_t	cg_deadBodyColor;

vmCvar_t	cg_plasmaBallAlpha;

vmCvar_t	cg_drawItemPickups;

vmCvar_t	cg_mapConfigs;

vmCvar_t	cg_enemyWeaponColor;
vmCvar_t	cg_teamWeaponColor;
vmCvar_t	cg_forceWeaponColor;

vmCvar_t	cg_rocketTrailRadius;
vmCvar_t	cg_grenadeTrailRadius;

vmCvar_t	cg_brightItems;

vmCvar_t	cg_autoaction;

vmCvar_t	cg_autosnaps;

vmCvar_t	cg_particles;

vmCvar_t	cg_lightningStyle;

vmCvar_t	cg_hitMarks;

vmCvar_t	cg_newRewards;
vmCvar_t	cg_drawCenterprint;

vmCvar_t	cg_nomip;

//vmCvar_t	cg_lightningExplosion;
vmCvar_t	cg_noAmmoChange;
vmCvar_t	cg_lgHitSfx;
vmCvar_t	cg_crosshairHitColor;
vmCvar_t 	cg_crosshairHitColorTime;
vmCvar_t 	cg_crosshairHitColorStyle;

vmCvar_t	cg_itemFX;

vmCvar_t	aftershock_login;
vmCvar_t	aftershock_password;

vmCvar_t	cg_drawAccel;

vmCvar_t	ref_password;

vmCvar_t	cg_hud;

vmCvar_t	cg_consoleTime;

vmCvar_t	cg_multiview;

vmCvar_t	cg_multiview1_xpos;
vmCvar_t	cg_multiview1_ypos;
vmCvar_t	cg_multiview1_width;
vmCvar_t	cg_multiview1_height;

vmCvar_t	cg_multiview2_xpos;
vmCvar_t	cg_multiview2_ypos;
vmCvar_t	cg_multiview2_width;
vmCvar_t	cg_multiview2_height;
vmCvar_t	cg_multiview2_client;

vmCvar_t	cg_multiview3_xpos;
vmCvar_t	cg_multiview3_ypos;
vmCvar_t	cg_multiview3_width;
vmCvar_t	cg_multiview3_height;
vmCvar_t	cg_multiview3_client;

vmCvar_t	cg_multiview4_xpos;
vmCvar_t	cg_multiview4_ypos;
vmCvar_t	cg_multiview4_width;
vmCvar_t	cg_multiview4_height;
vmCvar_t	cg_multiview4_client;

vmCvar_t	g_promode;

vmCvar_t	cg_inverseTimer;

vmCvar_t 	cg_grenadeTrail;
vmCvar_t 	cg_rocketTrail;
vmCvar_t 	cg_plasmaTrail;

vmCvar_t 	cg_oldScoreboard;

vmCvar_t 	cg_chatBeep;
vmCvar_t 	cg_teamChatBeep;

vmCvar_t 	cg_zoomScaling;
vmCvar_t 	cg_zoomToggle;
vmCvar_t 	cg_selfOnTeamOverlay;
vmCvar_t 	cg_smoke_SG;
vmCvar_t 	cg_smoothFovChange;

vmCvar_t 	cg_killBeep;
vmCvar_t 	cg_lowAmmoWarningPercentile;
vmCvar_t 	cg_lowHealthPercentile;

vmCvar_t 	cg_flatGrenades;
vmCvar_t 	cg_bubbleTrail;

vmCvar_t 	cg_muzzleFlash;
vmCvar_t 	cg_playerLean;
vmCvar_t 	cg_explosion;
vmCvar_t 	cg_drawSpawnpoints;

vmCvar_t 	cg_mapoverview;
vmCvar_t 	cg_damagePlums;
vmCvar_t 	cg_damagePlum;
vmCvar_t 	cg_damagePlumScale;
vmCvar_t 	cg_waterWarp;
vmCvar_t	cg_hudFullScreen;
vmCvar_t	cg_fovAdjust;
vmCvar_t	cg_drawAttacker;


typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = { // bk001129
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_autoswitch, "cg_autoswitch", "0", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "3", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "115", CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE },
	{ &cg_ammoWarningSound, "cg_ammoWarningSound", "1", CVAR_ARCHIVE },
	{ &cg_drawSpeed, "cg_drawSpeed", "0", CVAR_ARCHIVE },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "600", CVAR_ARCHIVE },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_ARCHIVE },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_ARCHIVE },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_ARCHIVE },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_CHEAT },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
	{ &cg_teamChatTime, "cg_teamChatTime", "5000", CVAR_ARCHIVE },
	{ &cg_chatTime, "cg_chatTime", "5000", CVAR_ARCHIVE },
	{ &cg_forceModel, "cg_forceModel", "1", CVAR_ARCHIVE },
	{ &cg_predictItems, "cg_predictItems", "0", CVAR_ARCHIVE },
#ifdef MISSIONPACK
	{ &cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE },
#else
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
#endif
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noChat, "cg_noChat", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_blood, "cg_blood", "0", CVAR_ARCHIVE },
	{ &cg_hitBeep, "cg_hitBeep", "2", CVAR_ARCHIVE },
	{ &cg_voip_teamonly, "cg_voipTeamOnly", "1", CVAR_ARCHIVE },
	{ &cg_voteflags, "cg_voteflags", "*", CVAR_ROM },
	{ &cg_cyclegrapple, "cg_cyclegrapple", "1", CVAR_ARCHIVE },
	{ &cg_vote_custom_commands, "cg_vote_custom_commands", "", CVAR_ROM },
	{ &cg_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO },	// communicated by systeminfo
	{ &cg_autovertex, "cg_autovertex", "0", CVAR_ARCHIVE },
#ifdef MISSIONPACK
	{ &cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE },
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE },
	{ &cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO },
	{ &cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO },
	{ &cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE },
	{ &cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE },
	{ &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE },
#endif
	{ &cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO },
	{ &cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO },
	{ &cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO },
	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT },
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE },
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0 },
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0 },
	{ &cg_timescale, "timescale", "1", 0 },
	{ &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE },
//unlagged - smooth clients #2
// this is done server-side now
//	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE },
//unlagged - smooth clients #2
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT },
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO },
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO },
	{ &pmove_float, "pmove_float", "0", CVAR_SYSTEMINFO },
	{ &pmove_accurate, "pmove_accurate", "0", CVAR_SYSTEMINFO },
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE },
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE },
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE },
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE },
	{ &cg_oldRail, "cg_railStyle", "0", CVAR_ARCHIVE },
	{ &cg_oldRocket, "cg_rocketStyle", "1", CVAR_ARCHIVE },
	{ &cg_plasmaTrail, "cg_plasmaStyle", "0", CVAR_ARCHIVE },
//unlagged - client options
	{ &cg_delag, "cg_delag", "0", CVAR_ARCHIVE | CVAR_USERINFO },
//	{ &cg_debugDelag, "cg_debugDelag", "0", CVAR_USERINFO | CVAR_CHEAT },
//	{ &cg_drawBBox, "cg_drawBBox", "0", CVAR_CHEAT },
	{ &cg_cmdTimeNudge, "cg_cmdTimeNudge", "0", CVAR_ARCHIVE | CVAR_USERINFO },
	// this will be automagically copied from the server
	{ &sv_fps, "sv_fps", "40", CVAR_SYSTEMINFO },
	{ &cg_projectileNudge, "cg_projectileNudge", "0", CVAR_ARCHIVE },
	{ &cg_optimizePrediction, "cg_optimizePrediction", "1", CVAR_ARCHIVE },
	{ &cl_timeNudge, "cl_timeNudge", "0", CVAR_ARCHIVE },
//	{ &cg_latentSnaps, "cg_latentSnaps", "0", CVAR_USERINFO | CVAR_CHEAT },
//	{ &cg_latentCmds, "cg_latentCmds", "0", CVAR_USERINFO | CVAR_CHEAT },
//	{ &cg_plOut, "cg_plOut", "0", CVAR_USERINFO | CVAR_CHEAT },
//unlagged - client options
	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE},
        { &cg_music, "cg_music", "", CVAR_ARCHIVE},
//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
	{ &cg_crosshairPulse, "cg_crosshairPulse", "1", CVAR_ARCHIVE},
	{ &cg_crosshairColor, "cg_crosshairColor", "7", CVAR_ARCHIVE},
	{&cg_deathNoticeTime, "cg_deathNoticeTime", "3000", CVAR_ARCHIVE},
	{&s_ambient, "s_ambient", "1", CVAR_ARCHIVE},
	{&cg_nokick, "cg_nokick", "0", CVAR_ARCHIVE},
	{&cg_hiResCharset, "cg_hiResCharset", "1", CVAR_ARCHIVE},
	{&cg_weaponBobbing, "cg_weaponBobbing", "0", CVAR_ARCHIVE},
	{&cg_blueteammodel, "cg_blueteammodel", "skelebot/pm", CVAR_ARCHIVE},
	{&cg_redteammodel, "cg_redteammodel", "major/pm", CVAR_ARCHIVE},
	{&cg_enemymodel, "cg_enemymodel", "smarine/pm", CVAR_ARCHIVE},
	{&cg_teammodel, "cg_teammodel", "major/pm", CVAR_ARCHIVE},
	{&cg_forceteammodels, "cg_forceteammodels", "0", CVAR_ARCHIVE},
	{&cg_enemyHeadColor, "cg_enemyHeadColor", "yellow", CVAR_ARCHIVE},
	{&cg_enemyTorsoColor, "cg_enemyTorsoColor", "yellow", CVAR_ARCHIVE},
	{&cg_enemyLegsColor, "cg_enemyLegsColor", "yellow", CVAR_ARCHIVE},
	{&cg_blueHeadColor, "cg_blueHeadColor", "blue", CVAR_ARCHIVE},
	{&cg_blueTorsoColor, "cg_blueTorsoColor", "blue", CVAR_ARCHIVE},
	{&cg_blueLegsColor, "cg_blueLegsColor", "blue", CVAR_ARCHIVE},
	{&cg_redHeadColor, "cg_redHeadColor", "red", CVAR_ARCHIVE},
	{&cg_redTorsoColor, "cg_redTorsoColor", "red", CVAR_ARCHIVE},
	{&cg_redLegsColor, "cg_redLegsColor", "red", CVAR_ARCHIVE},
	{&cg_teamHeadColor, "cg_teamHeadColor", "white", CVAR_ARCHIVE},
	{&cg_teamTorsoColor, "cg_teamTorsoColor", "white", CVAR_ARCHIVE},
	{&cg_teamLegsColor, "cg_teamLegsColor", "white", CVAR_ARCHIVE},	
	{&cg_deadBodyDarken, "cg_deadBodyDarken", "1", CVAR_ARCHIVE},
	{&cg_deadBodyColor, "cg_deadBodyColor", "0x323232FF", CVAR_ARCHIVE},
	{&cg_plasmaBallAlpha, "cg_plasmaBallAlpha", "255", CVAR_ARCHIVE| CVAR_CHEAT},
	{&cg_drawItemPickups, "cg_drawItemPickups", "7", CVAR_ARCHIVE},
	{&cg_mapConfigs, "cg_mapConfigs", "0", CVAR_ARCHIVE},
	{&cg_teamWeaponColor, "cg_teamWeaponColor", "0xFFFFFFFF", CVAR_ARCHIVE},
	{&cg_enemyWeaponColor, "cg_enemyWeaponColor", "0x00FF00FF", CVAR_ARCHIVE},
	{&cg_forceWeaponColor, "cg_forceWeaponColor", "0", CVAR_ARCHIVE},
	{&cg_rocketTrailRadius, "cg_rocketTrailRadius", "64", CVAR_ARCHIVE},
	{&cg_grenadeTrailRadius, "cg_grenadeTrailRadius", "32", CVAR_ARCHIVE},
	{&cg_brightItems, "cg_brightItems", "0", CVAR_ARCHIVE},
	{&cg_autoaction, "cg_autoaction", "0", CVAR_ARCHIVE| CVAR_USERINFO},
	{&cg_autosnaps, "cg_autosnaps", "1", CVAR_ARCHIVE},
	{&cg_particles, "cg_particles", "1", CVAR_ARCHIVE},
	{&cg_lightningStyle, "cg_lightningStyle", "0", CVAR_ARCHIVE},
	{&cg_hitMarks, "cg_hitMarks", "1", CVAR_ARCHIVE},
	{&cg_newRewards, "cg_newRewards", "1", CVAR_ARCHIVE},
	{&cg_drawCenterprint, "cg_drawCenterprint", "1", CVAR_ARCHIVE},
	{&cg_nomip, "cg_nomip", "0", CVAR_ARCHIVE},
//	{&cg_lightningExplosion, "cg_lightningImpact", "0", CVAR_ARCHIVE},
	{&cg_noAmmoChange, "cg_noAmmoChange", "1", CVAR_ARCHIVE},
	{&cg_lgHitSfx, "cg_lgHitSfx", "1", CVAR_ARCHIVE},
	{&cg_crosshairHitColor, "cg_crosshairHitColor", "1", CVAR_ARCHIVE},
	{&cg_crosshairHitColorTime, "cg_crosshairHitColorTime", "500", CVAR_ARCHIVE},
	{&cg_crosshairHitColorStyle, "cg_crosshairHitColorStyle", "1", CVAR_ARCHIVE},
	{&cg_itemFX, "cg_itemFX", "7", CVAR_ARCHIVE},
	{&aftershock_login, "aftershock_login", "", CVAR_USERINFO },
	{&aftershock_password, "aftershock_password", "", CVAR_USERINFO },
	{&cg_drawAccel, "cg_drawAccel", "0", CVAR_ARCHIVE },
	{&ref_password, "ref_password", "", CVAR_USERINFO | CVAR_TEMP },
	{&cg_hud, "cg_hud", "hud/default.txt", CVAR_ARCHIVE },
	{&cg_consoleTime, "cg_consoleTime", "3000", CVAR_ARCHIVE },
	{&cg_multiview, "cg_multiview", "4", CVAR_ARCHIVE | CVAR_USERINFO },
	{&cg_multiview1_xpos, "cg_multiview1_xpos", "0", CVAR_CHEAT },
	{&cg_multiview1_ypos, "cg_multiview1_ypos", "0", CVAR_CHEAT },
	{&cg_multiview1_width, "cg_multiview1_width", "640", CVAR_CHEAT },
	{&cg_multiview1_height, "cg_multiview1_height", "480", CVAR_CHEAT },
	{&cg_multiview2_xpos, "cg_multiview2_xpos", "440", CVAR_ARCHIVE },
	{&cg_multiview2_ypos, "cg_multiview2_ypos", "160", CVAR_ARCHIVE },
	{&cg_multiview2_width, "cg_multiview2_width", "200", CVAR_ARCHIVE },
	{&cg_multiview2_height, "cg_multiview2_height", "160", CVAR_ARCHIVE },
	{&cg_multiview2_client, "cg_multiview2_client", "-1", CVAR_TEMP },
	{&cg_multiview3_xpos, "cg_multiview3_xpos", "440", CVAR_ARCHIVE },
	{&cg_multiview3_ypos, "cg_multiview3_ypos", "0", CVAR_ARCHIVE },
	{&cg_multiview3_width, "cg_multiview3_width", "200", CVAR_ARCHIVE },
	{&cg_multiview3_height, "cg_multiview3_height", "160", CVAR_ARCHIVE },
	{&cg_multiview3_client, "cg_multiview3_client", "-1", CVAR_TEMP },
	{&cg_multiview4_xpos, "cg_multiview4_xpos", "440", CVAR_ARCHIVE },
	{&cg_multiview4_ypos, "cg_multiview4_ypos", "320", CVAR_ARCHIVE },
	{&cg_multiview4_width, "cg_multiview4_width", "200", CVAR_ARCHIVE },
	{&cg_multiview4_height, "cg_multiview4_height", "160", CVAR_ARCHIVE },
	{&cg_multiview4_client, "cg_multiview4_client", "-1", CVAR_TEMP },
	{&g_promode, "g_promode", "0", CVAR_SYSTEMINFO },
	{&cg_inverseTimer, "cg_inverseTimer", "0", CVAR_ARCHIVE },
	{&cg_grenadeTrail, "cg_grenadeTrail", "1", CVAR_ARCHIVE },
	{&cg_rocketTrail, "cg_rocketTrail", "1", CVAR_ARCHIVE },
	{&cg_plasmaTrail, "cg_plasmaTrail", "0", CVAR_ARCHIVE },
	{&cg_oldScoreboard, "cg_oldScoreboard", "0", CVAR_ARCHIVE },
	{&cg_chatBeep, "cg_chatBeep", "1", CVAR_ARCHIVE },
	{&cg_teamChatBeep, "cg_teamChatBeep", "1", CVAR_ARCHIVE },
	{&cg_zoomScaling, "cg_zoomScaling", "1", CVAR_ARCHIVE },
	{&cg_zoomToggle, "cg_zoomToggle", "0", CVAR_ARCHIVE },
	{&cg_selfOnTeamOverlay, "cg_selfOnTeamOverlay", "1", CVAR_ARCHIVE },
	{&cg_smoke_SG, "cg_smoke_SG", "1", CVAR_ARCHIVE },
	{&cg_smoothFovChange, "cg_smoothFovChange", "0", CVAR_ARCHIVE },
	{&cg_killBeep, "cg_killBeep", "0", CVAR_ARCHIVE },
	{&cg_lowAmmoWarningPercentile, "cg_lowAmmoWarningPercentile", "0.4", CVAR_ARCHIVE },
	{&cg_lowHealthPercentile, "cg_lowHealthPercentile", "0.25", CVAR_ARCHIVE },
	{&cg_flatGrenades, "cg_flatGrenades", "0", CVAR_ARCHIVE },
	{&cg_bubbleTrail, "cg_bubbleTrail", "1", CVAR_ARCHIVE },
	{&cg_muzzleFlash, "cg_muzzleFlash", "1", CVAR_ARCHIVE },
	{&cg_playerLean, "cg_playerLean", "1.0", CVAR_ARCHIVE },
	{&cg_explosion, "cg_explosion", "255", CVAR_ARCHIVE },
	{&cg_drawSpawnpoints, "cg_drawSpawnpoints", "1", CVAR_ARCHIVE },
	{&cg_mapoverview, "cg_mapoverview", "0", CVAR_ARCHIVE | CVAR_CHEAT },
	{&cg_damagePlums, "cg_damagePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE },
	{&cg_damagePlum, "cg_damagePlum", "/g/mg/sg/gl/rl/lg/rg/pg/bfg/cg/ng/pl/", CVAR_USERINFO | CVAR_ARCHIVE },
	{&cg_damagePlumScale, "cg_damagePlumScale", "1.0", CVAR_USERINFO | CVAR_ARCHIVE },
	{&cg_waterWarp, "cg_waterWarp", "1", CVAR_ARCHIVE },
	{&cg_hudFullScreen, "cg_hudFullScreen", "0", CVAR_ARCHIVE},
	{&cg_fovAdjust, "cg_fovAdjust", "0", CVAR_ARCHIVE},
	{&cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE},
};

static int  cvarTableSize = ARRAY_LEN( cvarTable );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	forceModelModificationCount = cg_forceModel.modificationCount;
	blueTeamModelModificationCount = cg_blueteammodel.modificationCount;
	redTeamModelModificationCount = cg_redteammodel.modificationCount;
	enemyModelModificationCount = cg_enemymodel.modificationCount;
	teamModelModificationCount = cg_teammodel.modificationCount;
	forceTeamModelsModificationCount = cg_forceteammodels.modificationCount;

	trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE );
}

/*																																			
===================
CG_ForceModelChange
===================
*/
void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

void CG_ParseHitBeep( void ) {
        char hitBeep[256];
        char buffer[16];
        int counter = WP_GAUNTLET, counter2=0;
        int i;

        strcpy(hitBeep, cg_hitBeep.string);
        for ( i = 0; i <= strlen(hitBeep); i++ ){
                if( hitBeep[i] != '/' && i != strlen(hitBeep)){
                  buffer[counter2] = hitBeep[i];
                  counter2++;
                }
                else {
                  buffer[counter2]='\0';
                  cgs.hitBeep[counter] = atoi(buffer);
                  counter2=0;
                  buffer[counter2] = '\0';

                  counter++;
                  if( counter == WP_NUM_WEAPONS )
                            return;
                }
        }
        for( i = counter; i < WP_NUM_WEAPONS; i++ ){
                cgs.hitBeep[i] = cgs.hitBeep[counter-1];
        }
        cgs.hitBeep[WP_NONE] = cgs.hitBeep[WP_GAUNTLET];
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
//unlagged - client options
		// clamp the value between 0 and 999
		// negative values would suck - people could conceivably shoot other
		// players *long* after they had left the area, on purpose
		if ( cv->vmCvar == &cg_cmdTimeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 999 );
		}
		// cl_timenudge less than -50 or greater than 50 doesn't actually
		// do anything more than -50 or 50 (actually the numbers are probably
		// closer to -30 and 30, but 50 is nice and round-ish)
		// might as well not feed the myth, eh?
		else if ( cv->vmCvar == &cl_timeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, -50, 50 );
		}
		// don't let this go too high - no point
		/*else if ( cv->vmCvar == &cg_latentSnaps ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 10 );
		}*/
		// don't let this get too large
		/*else if ( cv->vmCvar == &cg_latentCmds ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, MAX_LATENT_CMDS - 1 );
		}*/
		// no more than 100% packet loss
		/*else if ( cv->vmCvar == &cg_plOut ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 100 );
		}*/
//unlagged - client options
                else if ( cv->vmCvar == &cg_errorDecay ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 250 );
		}
		else if ( cv->vmCvar == &cg_lowAmmoWarningPercentile ) {
			CG_Cvar_ClampFloat( cv->cvarName, cv->vmCvar, 0.0, 1.0 );
		}
		else if ( cv->vmCvar == &cg_playerLean ) {
			CG_Cvar_ClampFloat( cv->cvarName, cv->vmCvar, 0.0, 1.0 );
		}
		else if ( cv->vmCvar == &cg_crosshairHitColorStyle ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 4 );
		}
		trap_Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			trap_Cvar_Set( "teamoverlay", "1" );
		} else {
			trap_Cvar_Set( "teamoverlay", "0" );
		}
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount || blueTeamModelModificationCount != cg_blueteammodel.modificationCount || redTeamModelModificationCount != cg_redteammodel.modificationCount || enemyModelModificationCount != cg_enemymodel.modificationCount || teamModelModificationCount != cg_teammodel.modificationCount || forceTeamModelsModificationCount != cg_forceteammodels.modificationCount ) {

		forceModelModificationCount = cg_forceModel.modificationCount;
		blueTeamModelModificationCount = cg_blueteammodel.modificationCount;
		redTeamModelModificationCount = cg_redteammodel.modificationCount;
		enemyModelModificationCount = cg_enemymodel.modificationCount;
		teamModelModificationCount = cg_teammodel.modificationCount;
		forceTeamModelsModificationCount = cg_forceteammodels.modificationCount;


		CG_ForceModelChange();
	}
	
	if( hudModificationCount != cg_hud.modificationCount ){
		//for( i = 0; i < HUD_MAX ; i++ ){
			
		//}
		hudModificationCount = cg_hud.modificationCount;
		CG_ClearHud();
		CG_LoadHudFile( cg_hud.string );
	}
	
	if( fovModificationCount != cg_fov.modificationCount ){
		fovModificationCount = cg_fov.modificationCount;
		CG_ParseFov();
	}
	
	if( zoomfovModificationCount != cg_zoomFov.modificationCount ){
		zoomfovModificationCount = cg_zoomFov.modificationCount;
		CG_ParseZoomFov();
	}
	
	if( crosshairModificationCount != cg_drawCrosshair.modificationCount ){
		crosshairModificationCount = cg_drawCrosshair.modificationCount;
		CG_ParseCrosshair();
	}
	
	if( crosshairsizeModificationCount != cg_crosshairSize.modificationCount ){
		crosshairsizeModificationCount = cg_crosshairSize.modificationCount;
		CG_ParseCrosshairSize();
	}
	if( hitBeepModificationCount != cg_hitBeep.modificationCount ){
		hitBeepModificationCount = cg_hitBeep.modificationCount;
		CG_ParseHitBeep();
	}
	if( nomipModificationCount != cg_nomip.modificationCount ){
		nomipModificationCount = cg_nomip.modificationCount;
		CG_Printf("cg_picmip will be changed upon restarting\n" );
	}
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime[0] + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum[0];
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);
	
	CG_AddToConsole(text);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		trap_S_RegisterSound( item->pickup_sound, qfalse );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data, qfalse );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

	cgs.media.oneMinuteSound = trap_S_RegisterSound( "sound/feedback/1_minute.wav", qtrue );
	cgs.media.fiveMinuteSound = trap_S_RegisterSound( "sound/feedback/5_minute.wav", qtrue );
	cgs.media.suddenDeathSound = trap_S_RegisterSound( "sound/feedback/sudden_death.wav", qtrue );
	cgs.media.oneFragSound = trap_S_RegisterSound( "sound/feedback/1_frag.wav", qtrue );
	cgs.media.twoFragSound = trap_S_RegisterSound( "sound/feedback/2_frags.wav", qtrue );
	cgs.media.threeFragSound = trap_S_RegisterSound( "sound/feedback/3_frags.wav", qtrue );
	cgs.media.count3Sound = trap_S_RegisterSound( "sound/feedback/three.wav", qtrue );
	cgs.media.count2Sound = trap_S_RegisterSound( "sound/feedback/two.wav", qtrue );
	cgs.media.count1Sound = trap_S_RegisterSound( "sound/feedback/one.wav", qtrue );
	cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav", qtrue );
	cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav", qtrue );

	// N_G: Another condition that makes no sense to me, see for
	// yourself if you really meant this
	// Sago: Makes perfect sense: Load team game stuff if the gametype is a teamgame and not an exception (like GT_LMS)
	if ( ( ( cgs.gametype >= GT_TEAM ) && ( cgs.ffa_gt != 1 ) ) ||
		cg_buildScript.integer ) {

		cgs.media.captureAwardSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qtrue );
		cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/feedback/redleads.wav", qtrue );
		cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/feedback/blueleads.wav", qtrue );
		cgs.media.teamsTiedSound = trap_S_RegisterSound( "sound/feedback/teamstied.wav", qtrue );
		cgs.media.hitTeamSound = trap_S_RegisterSound( "sound/feedback/hit_teammate.wav", qtrue );

		cgs.media.redScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_red_scores.wav", qtrue );
		cgs.media.blueScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_scores.wav", qtrue );

		cgs.media.captureYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qtrue );
		cgs.media.captureOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav", qtrue );

		cgs.media.returnYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav", qtrue );
		cgs.media.returnOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );

		cgs.media.takenYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_yourteam.wav", qtrue );
		cgs.media.takenOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_opponent.wav", qtrue );

		if ( cgs.gametype == GT_CTF || cgs.gametype == GT_CTF_ELIMINATION|| cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_red_returned.wav", qtrue );
			cgs.media.blueFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_returned.wav", qtrue );
			cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav", qtrue );
			cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_flag.wav", qtrue );
		}

		if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
			// FIXME: get a replacement for this sound ?
			cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );
			cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", qtrue );
			cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", qtrue );
		}

		if ( cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF || cgs.gametype == GT_CTF_ELIMINATION ||cg_buildScript.integer ) {
			cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav", qtrue );
			cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qtrue);
		}

                if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
			cgs.media.yourBaseIsUnderAttackSound = trap_S_RegisterSound( "sound/teamplay/voc_base_attack.wav", qtrue );
		}
	}

	cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", qfalse );
	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav", qfalse );
	cgs.media.wearOffSound = trap_S_RegisterSound( "sound/items/wearoff.wav", qfalse );
	cgs.media.useNothingSound = trap_S_RegisterSound( "sound/items/use_nothing.wav", qfalse );
	cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.wav", qfalse );
	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/player/gibimp1.wav", qfalse );
	cgs.media.gibBounce2Sound = trap_S_RegisterSound( "sound/player/gibimp2.wav", qfalse );
	cgs.media.gibBounce3Sound = trap_S_RegisterSound( "sound/player/gibimp3.wav", qfalse );

#ifdef MISSIONPACK	
	
	cgs.media.useInvulnerabilitySound = trap_S_RegisterSound( "sound/items/invul_activate.wav", qfalse );
	cgs.media.invulnerabilityImpactSound1 = trap_S_RegisterSound( "sound/items/invul_impact_01.wav", qfalse );
	cgs.media.invulnerabilityImpactSound2 = trap_S_RegisterSound( "sound/items/invul_impact_02.wav", qfalse );
	cgs.media.invulnerabilityImpactSound3 = trap_S_RegisterSound( "sound/items/invul_impact_03.wav", qfalse );
	cgs.media.invulnerabilityJuicedSound = trap_S_RegisterSound( "sound/items/invul_juiced.wav", qfalse );

	cgs.media.ammoregenSound = trap_S_RegisterSound("sound/items/cl_ammoregen.wav", qfalse);
	cgs.media.doublerSound = trap_S_RegisterSound("sound/items/cl_doubler.wav", qfalse);
	cgs.media.guardSound = trap_S_RegisterSound("sound/items/cl_guard.wav", qfalse);
	cgs.media.scoutSound = trap_S_RegisterSound("sound/items/cl_scout.wav", qfalse);
        cgs.media.obeliskHitSound1 = trap_S_RegisterSound( "sound/items/obelisk_hit_01.wav", qfalse );
	cgs.media.obeliskHitSound2 = trap_S_RegisterSound( "sound/items/obelisk_hit_02.wav", qfalse );
	cgs.media.obeliskHitSound3 = trap_S_RegisterSound( "sound/items/obelisk_hit_03.wav", qfalse );
	cgs.media.obeliskRespawnSound = trap_S_RegisterSound( "sound/items/obelisk_respawn.wav", qfalse );
	
#endif

	cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.wav", qfalse );
	cgs.media.teleOutSound = trap_S_RegisterSound( "sound/world/teleout.wav", qfalse );
	cgs.media.respawnSound = trap_S_RegisterSound( "sound/items/respawn1.wav", qfalse );

	cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav", qfalse );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );
	cgs.media.talkTeamSound = trap_S_RegisterSound( "sound/player/talkTeam.wav", qfalse );
	cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav", qfalse);


	cgs.media.hitSound0 = trap_S_RegisterSound( "sound/feedback/hitlower.wav", qfalse );
	cgs.media.hitSound1 = trap_S_RegisterSound( "sound/feedback/hitlow.wav", qfalse );
	cgs.media.hitSound2 = trap_S_RegisterSound( "sound/feedback/hit.wav", qfalse );
	cgs.media.hitSound3 = trap_S_RegisterSound( "sound/feedback/hithigh.wav", qfalse );
	cgs.media.hitSound4 = trap_S_RegisterSound( "sound/feedback/hithigher.wav", qfalse );
	cgs.media.killSound = trap_S_RegisterSound( "sound/feedback/killbeep1.wav", qfalse );

#ifdef MISSIONPACK
	cgs.media.hitSoundHighArmor = trap_S_RegisterSound( "sound/feedback/hithi.wav", qfalse );
	cgs.media.hitSoundLowArmor = trap_S_RegisterSound( "sound/feedback/hitlo.wav", qfalse );
#endif

	cgs.media.impressiveSound = trap_S_RegisterSound( "sound/feedback/impressive.wav", qtrue );
	cgs.media.excellentSound = trap_S_RegisterSound( "sound/feedback/excellent.wav", qtrue );
	cgs.media.deniedSound = trap_S_RegisterSound( "sound/feedback/denied.wav", qtrue );
	cgs.media.humiliationSound = trap_S_RegisterSound( "sound/feedback/humiliation.wav", qtrue );
	cgs.media.assistSound = trap_S_RegisterSound( "sound/feedback/assist.wav", qtrue );
	cgs.media.defendSound = trap_S_RegisterSound( "sound/feedback/defense.wav", qtrue );
	cgs.media.airrocketSound = trap_S_RegisterSound( "sound/feedback/accuracy.wav", qtrue );
	cgs.media.airgrenadeSound = trap_S_RegisterSound( "sound/feedback/voc_holyshit.wav", qtrue );
	cgs.media.spawnkillSound = trap_S_RegisterSound( "sound/feedback/impressive.wav", qtrue );
	cgs.media.lgaccuracySound = trap_S_RegisterSound( "sound/feedback/accuracy.wav", qtrue );
	

#ifdef MISSIONPACK
	cgs.media.firstImpressiveSound = trap_S_RegisterSound( "sound/feedback/first_impressive.wav", qtrue );
	cgs.media.firstExcellentSound = trap_S_RegisterSound( "sound/feedback/first_excellent.wav", qtrue );
	cgs.media.firstHumiliationSound = trap_S_RegisterSound( "sound/feedback/first_gauntlet.wav", qtrue );
#endif

	cgs.media.takenLeadSound = trap_S_RegisterSound( "sound/feedback/takenlead.wav", qtrue);
	cgs.media.tiedLeadSound = trap_S_RegisterSound( "sound/feedback/tiedlead.wav", qtrue);
	cgs.media.lostLeadSound = trap_S_RegisterSound( "sound/feedback/lostlead.wav", qtrue);

//#ifdef MISSIONPACK
	cgs.media.voteNow = trap_S_RegisterSound( "sound/feedback/vote_now.wav", qtrue);
	cgs.media.votePassed = trap_S_RegisterSound( "sound/feedback/vote_passed.wav", qtrue);
	cgs.media.voteFailed = trap_S_RegisterSound( "sound/feedback/vote_failed.wav", qtrue);
//#endif

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse);
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse);
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse);

	cgs.media.jumpPadSound = trap_S_RegisterSound ("sound/world/jumppad.wav", qfalse );

	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/boot%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/flesh%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mech%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/energy%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/splash%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/clank%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name, qfalse);
	}

	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for ( i = 1 ; i < bg_numItems ; i++ ) {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );
//		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName, qfalse );
	}

	// FIXME: only needed with item
	cgs.media.flightSound = trap_S_RegisterSound( "sound/items/flight.wav", qfalse );
	cgs.media.medkitSound = trap_S_RegisterSound ("sound/items/use_medkit.wav", qfalse);
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage3.wav", qfalse);
	cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav", qfalse);
	cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav", qfalse);
	cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav", qfalse);
	cgs.media.sfx_railg = trap_S_RegisterSound ("sound/weapons/railgun/railgf1a.wav", qfalse);
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav", qfalse);
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav", qfalse);
	cgs.media.weaponHoverSound = trap_S_RegisterSound( "sound/weapons/weapon_hover.wav", qfalse );
	
#ifdef MISSIONPACK	
	cgs.media.sfx_proxexp = trap_S_RegisterSound( "sound/weapons/proxmine/wstbexpl.wav" , qfalse);
	cgs.media.sfx_nghit = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpd.wav" , qfalse);
	cgs.media.sfx_nghitflesh = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpl.wav" , qfalse);
	cgs.media.sfx_nghitmetal = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpm.wav", qfalse );
	cgs.media.sfx_chghit = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpd.wav", qfalse );
	cgs.media.sfx_chghitflesh = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpl.wav", qfalse );
	cgs.media.sfx_chghitmetal = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpm.wav", qfalse );
	cgs.media.kamikazeExplodeSound = trap_S_RegisterSound( "sound/items/kam_explode.wav", qfalse );
	cgs.media.kamikazeImplodeSound = trap_S_RegisterSound( "sound/items/kam_implode.wav", qfalse );
	cgs.media.kamikazeFarSound = trap_S_RegisterSound( "sound/items/kam_explode_far.wav", qfalse );
	
	cgs.media.wstbimplSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpl.wav", qfalse);
	cgs.media.wstbimpmSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpm.wav", qfalse);
	cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.wav", qfalse);
	cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.wav", qfalse);
#endif
	cgs.media.winnerSound = trap_S_RegisterSound( "sound/feedback/voc_youwin.wav", qfalse );
	cgs.media.loserSound = trap_S_RegisterSound( "sound/feedback/voc_youlose.wav", qfalse );
	cgs.media.youSuckSound = trap_S_RegisterSound( "sound/misc/yousuck.wav", qfalse );

	cgs.media.regenSound = trap_S_RegisterSound("sound/items/regen.wav", qfalse);
	cgs.media.protectSound = trap_S_RegisterSound("sound/items/protect3.wav", qfalse);
	cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav", qfalse );
	cgs.media.hgrenb1aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb1a.wav", qfalse);
	cgs.media.hgrenb2aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb2a.wav", qfalse);

#ifdef MISSIONPACK
	trap_S_RegisterSound("sound/player/sergei/death1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/death2.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/death3.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/jump1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/pain25_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/pain75_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/pain100_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/falling1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/gasp.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/drown.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/fall1.wav", qfalse );
	trap_S_RegisterSound("sound/player/sergei/taunt.wav", qfalse );

	trap_S_RegisterSound("sound/player/kyonshi/death1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/death2.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/death3.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/jump1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/pain25_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/pain75_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/pain100_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/falling1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/gasp.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/drown.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/fall1.wav", qfalse );
	trap_S_RegisterSound("sound/player/kyonshi/taunt.wav", qfalse );
#endif

}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

	cgs.media.botSkillShaders[0] = trap_R_RegisterShader( "menu/art/skill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShader( "menu/art/skill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShader( "menu/art/skill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShader( "menu/art/skill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShader( "menu/art/skill5.tga" );

	cgs.media.viewBloodShader = trap_R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip( "menu/tab/time.tga" );

	if( cg_nomip.integer & 256 ){
		cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff_nomip" );
		cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro_nomip" );
		cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "aftershock_shotgunSmoke_nomip" );
	}
	else{
		cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
		cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
		cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "aftershock_shotgunSmoke" );
	}
#ifdef MISSIONPACK
	cgs.media.nailPuffShader = trap_R_RegisterShader( "nailtrail" );
	cgs.media.blueProxMine = trap_R_RegisterModel( "models/weaphits/proxmineb.md3" );
#endif
	if( cg_nomip.integer & 2 ){
		cgs.media.plasmaBallShader = trap_R_RegisterShader( "sprites/plasma1_nomip" );
		cgs.media.plasmaBallShaderColor = trap_R_RegisterShader( "sprites/plasma1Color_nomip" );
	}
	else{
		cgs.media.plasmaBallShader = trap_R_RegisterShader( "sprites/plasma1" );
		cgs.media.plasmaBallShaderColor = trap_R_RegisterShader( "sprites/plasma1Color" );
	}
	
	if( cg_nomip.integer & 256 )
		cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail_nomip" );
	else
		cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail" );
	
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );
	
	if( cg_flatGrenades.integer )
		cgs.media.grenadeSkinColor = trap_R_RegisterShader( "models/players/flat" );
	else	
		cgs.media.grenadeSkinColor = trap_R_RegisterShader( "models/ammo/grenadeColor" );

	for (i = 0; i < NUM_CROSSHAIRS; i++ ) {
		if (i < 26)
			cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("aftershock_crosshair%c", 'a'+i) );
		else
			cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("aftershock_crosshair%02d", i - 26) );
 	}

	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = trap_R_RegisterShaderNoMip( "icons/noammo" );
        cgs.media.selectionShaderLeft = trap_R_RegisterShaderNoMip( "icons/selectionMarkerLeft" );
	cgs.media.selectionShaderMid = trap_R_RegisterShaderNoMip( "icons/selectionMarkerMid" );
	cgs.media.selectionShaderRight = trap_R_RegisterShaderNoMip( "icons/selectionMarkerRight" );

	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad" );
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon" );
	cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit" );
	cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon" );
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility" );
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen" );
	cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff" );

	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_CTF_ELIMINATION || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.redCubeModel = trap_R_RegisterModel( "models/powerups/orb/r_orb.md3" );
		cgs.media.blueCubeModel = trap_R_RegisterModel( "models/powerups/orb/b_orb.md3" );
		cgs.media.redCubeIcon = trap_R_RegisterShader( "icons/skull_red" );
		cgs.media.blueCubeIcon = trap_R_RegisterShader( "icons/skull_blue" );
	}

//For Double Domination:
	if ( cgs.gametype == GT_DOUBLE_D ) {
		cgs.media.ddPointSkinA[TEAM_RED] = trap_R_RegisterShaderNoMip( "icons/icona_red" );
                cgs.media.ddPointSkinA[TEAM_BLUE] = trap_R_RegisterShaderNoMip( "icons/icona_blue" );
                cgs.media.ddPointSkinA[TEAM_FREE] = trap_R_RegisterShaderNoMip( "icons/icona_white" );
                cgs.media.ddPointSkinA[TEAM_NONE] = trap_R_RegisterShaderNoMip( "icons/noammo" );
                
                cgs.media.ddPointSkinB[TEAM_RED] = trap_R_RegisterShaderNoMip( "icons/iconb_red" );
                cgs.media.ddPointSkinB[TEAM_BLUE] = trap_R_RegisterShaderNoMip( "icons/iconb_blue" );
                cgs.media.ddPointSkinB[TEAM_FREE] = trap_R_RegisterShaderNoMip( "icons/iconb_white" );
                cgs.media.ddPointSkinB[TEAM_NONE] = trap_R_RegisterShaderNoMip( "icons/noammo" );
	}

	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_CTF_ELIMINATION || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.redFlagModel = trap_R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = trap_R_RegisterModel( "models/flags/b_flag.md3" );
                cgs.media.neutralFlagModel = trap_R_RegisterModel( "models/flags/n_flag.md3" );
		cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu3" );
		cgs.media.neutralFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_neu1" );
		cgs.media.neutralFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_neu2" );
		cgs.media.neutralFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_neu3" );
		cgs.media.flagPoleModel = trap_R_RegisterModel( "models/flag2/flagpole.md3" );
		cgs.media.flagFlapModel = trap_R_RegisterModel( "models/flag2/flagflap3.md3" );

		cgs.media.redFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/red.skin" );
		cgs.media.blueFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/blue.skin" );
		cgs.media.neutralFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/white.skin" );

		cgs.media.redFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/red_base.md3" );
		cgs.media.blueFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/blue_base.md3" );
		cgs.media.neutralFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/ntrl_base.md3" );
	}

	if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
		cgs.media.neutralFlagModel = trap_R_RegisterModel( "models/flags/n_flag.md3" );
		cgs.media.flagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_neutral1" );
		cgs.media.flagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.flagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.flagShader[3] = trap_R_RegisterShaderNoMip( "icons/iconf_neutral3" );
	}

	if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
		cgs.media.overloadBaseModel = trap_R_RegisterModel( "models/powerups/overload_base.md3" );
		cgs.media.overloadTargetModel = trap_R_RegisterModel( "models/powerups/overload_target.md3" );
		cgs.media.overloadLightsModel = trap_R_RegisterModel( "models/powerups/overload_lights.md3" );
		cgs.media.overloadEnergyModel = trap_R_RegisterModel( "models/powerups/overload_energy.md3" );
	}

	if ( cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.harvesterModel = trap_R_RegisterModel( "models/powerups/harvester/harvester.md3" );
		cgs.media.harvesterRedSkin = trap_R_RegisterSkin( "models/powerups/harvester/red.skin" );
		cgs.media.harvesterBlueSkin = trap_R_RegisterSkin( "models/powerups/harvester/blue.skin" );
		cgs.media.harvesterNeutralModel = trap_R_RegisterModel( "models/powerups/obelisk/obelisk.md3" );
	}

	cgs.media.redKamikazeShader = trap_R_RegisterShader( "models/weaphits/kamikred" );
	cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff" );

	if ( ( ( cgs.gametype >= GT_TEAM ) && ( cgs.ffa_gt != 1 ) ) ||
		cg_buildScript.integer ) {

		cgs.media.friendShader = trap_R_RegisterShader( "sprites/foe" );
		cgs.media.friendShaderVisible = trap_R_RegisterShader( "sprites/foe2.tga" );
		cgs.media.redQuadShader = trap_R_RegisterShader("powerups/blueflag" );
		cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );
#ifdef MISSIONPACK
		cgs.media.blueKamikazeShader = trap_R_RegisterShader( "models/weaphits/kamikblu" );
#endif
	}

	cgs.media.armorModel = trap_R_RegisterModel( "models/powerups/armor/armor_yel.md3" );
	cgs.media.armorIcon  = trap_R_RegisterShaderNoMip( "icons/iconr_yellow" );

	cgs.media.direct_hit = trap_R_RegisterShaderNoMip("icons/direct_hit");
	cgs.media.skull = trap_R_RegisterShaderNoMip("icons/skull");

	cgs.media.armorYellow  = trap_R_RegisterShaderNoMip( "icons/armorYellow" );
	cgs.media.armorBlue  = trap_R_RegisterShaderNoMip( "icons/armorBlue" );
	cgs.media.armorRed  = trap_R_RegisterShaderNoMip( "icons/armorRed" );
	cgs.media.healthYellow = trap_R_RegisterShaderNoMip( "icons/iconh_yellow" );
	cgs.media.healthBlue = trap_R_RegisterShaderNoMip( "icons/iconh_blue" );
	cgs.media.healthRed = trap_R_RegisterShaderNoMip( "icons/iconh_red" );
	cgs.media.machinegunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
	cgs.media.shotgunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.gibAbdomen = trap_R_RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = trap_R_RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = trap_R_RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = trap_R_RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = trap_R_RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = trap_R_RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = trap_R_RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = trap_R_RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = trap_R_RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = trap_R_RegisterModel( "models/gibs/brain.md3" );

	cgs.media.smoke2 = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.balloonShader = trap_R_RegisterShaderNoMip( "sprites/balloon3" );
	cgs.media.balloonShaderVisible = trap_R_RegisterShaderNoMip( "sprites/balloon4.tga" );

	if( cg_nomip.integer & 256 )
		cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion_nomip" );
	else
		cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion" );
	
	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
	cgs.media.plasmaExplosionModel = trap_R_RegisterModel("models/weaphits/plasmaExplosion.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
//#ifdef MISSIONPACK
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/powerups/pop.md3" );
/*#else
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/misc/telep.md3" );
	cgs.media.teleportEffectShader = trap_R_RegisterShader( "teleportEffect" );
#endif*/
#ifdef MISSIONPACK
	cgs.media.kamikazeEffectModel = trap_R_RegisterModel( "models/weaphits/kamboom2.md3" );
	cgs.media.kamikazeShockWave = trap_R_RegisterModel( "models/weaphits/kamwave.md3" );
	cgs.media.kamikazeHeadModel = trap_R_RegisterModel( "models/powerups/kamikazi.md3" );
	cgs.media.kamikazeHeadTrail = trap_R_RegisterModel( "models/powerups/trailtest.md3" );
	cgs.media.guardPowerupModel = trap_R_RegisterModel( "models/powerups/guard_player.md3" );
	cgs.media.scoutPowerupModel = trap_R_RegisterModel( "models/powerups/scout_player.md3" );
	cgs.media.doublerPowerupModel = trap_R_RegisterModel( "models/powerups/doubler_player.md3" );
	cgs.media.ammoRegenPowerupModel = trap_R_RegisterModel( "models/powerups/ammo_player.md3" );
	cgs.media.invulnerabilityImpactModel = trap_R_RegisterModel( "models/powerups/shield/impact.md3" );
	cgs.media.invulnerabilityJuicedModel = trap_R_RegisterModel( "models/powerups/shield/juicer.md3" );
	cgs.media.invulnerabilityPowerupModel = trap_R_RegisterModel( "models/powerups/shield/shield.md3" );
#endif	
	cgs.media.medkitUsageModel = trap_R_RegisterModel( "models/powerups/regen.md3" );
	cgs.media.heartShader = trap_R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );
	
	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet = trap_R_RegisterShaderNoMip( "medal_gauntlet" );
	cgs.media.medalDefend = trap_R_RegisterShaderNoMip( "medal_defend" );
	cgs.media.medalAssist = trap_R_RegisterShaderNoMip( "medal_assist" );
	cgs.media.medalAirrocket = trap_R_RegisterShaderNoMip( "medal_airrocket" );
	cgs.media.medalDoubleAirrocket = trap_R_RegisterShaderNoMip( "medal_double_airrocket" );
	cgs.media.medalAirgrenade = trap_R_RegisterShaderNoMip( "medal_airgrenade" );
	cgs.media.medalFullsg = trap_R_RegisterShaderNoMip( "medal_fullsg" );
	cgs.media.medalItemdenied = trap_R_RegisterShaderNoMip( "medal_itemdenied" );
	cgs.media.medalCapture = trap_R_RegisterShaderNoMip( "medal_capture" );
	cgs.media.medalRocketrail = trap_R_RegisterShaderNoMip( "medal_rocketrail" );
	cgs.media.medalSpawnkill = trap_R_RegisterShaderNoMip( "medal_spawnkill" );
	cgs.media.medalLgaccuracy = trap_R_RegisterShaderNoMip( "medal_lgaccuracy" );
	
	cgs.media.accBackground = trap_R_RegisterShaderNoMip( "acc_background" );
	
	cgs.media.sbBackground = trap_R_RegisterShaderNoMip( "sb_background" );
	cgs.media.sbClock = trap_R_RegisterShaderNoMip( "sb_clock" );
	cgs.media.sbPing = trap_R_RegisterShaderNoMip( "sb_ping" );
	cgs.media.sbReady = trap_R_RegisterShaderNoMip( "sb_ready" );
	cgs.media.sbNotReady = trap_R_RegisterShaderNoMip( "sb_notready" );
	cgs.media.sbSkull = trap_R_RegisterShaderNoMip( "sb_skull" );
	cgs.media.sbLocked = trap_R_RegisterShaderNoMip( "sb_locked" );
	cgs.media.sbAccuracy = trap_R_RegisterShaderNoMip( "icons/iconw_proxlauncher" );


	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	// wall marks
	if( cg_nomip.integer & 16 )
		cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk_nomip" );
	else
		cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
	
	if( cg_nomip.integer & 4 || cg_nomip.integer & 8 )
		cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk_nomip" );
	else
		cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );	
	
	if( cg_nomip.integer & 1 )
		cgs.media.holeMarkShader = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk_nomip" );
	else
		cgs.media.holeMarkShader = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk" );
	
	if( cg_nomip.integer & 2 )
		cgs.media.energyMarkShader = trap_R_RegisterShader( "gfx/damage/plasma_mrk_nomip" );
	else
		cgs.media.energyMarkShader = trap_R_RegisterShader( "gfx/damage/plasma_mrk" );
	
	cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake" );
	
	if( cg_nomip.integer & 256 )
		cgs.media.bloodMarkShader = trap_R_RegisterShader( "bloodMark_nomip" );
	else
		cgs.media.bloodMarkShader = trap_R_RegisterShader( "bloodMark" );
		

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
	}

#ifdef MISSIONPACK
	// new stuff
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
        cgs.media.deathShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/death.tga");

	cgs.media.cursor = trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	cgs.media.sizeCursor = trap_R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
	cgs.media.selectCursor = trap_R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );
	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");

	trap_R_RegisterModel( "models/players/sergei/lower.md3" );
	trap_R_RegisterModel( "models/players/sergei/upper.md3" );
	trap_R_RegisterModel( "models/players/sergei/head.md3" );

	trap_R_RegisterModel( "models/players/kyonshi/lower.md3" );
	trap_R_RegisterModel( "models/players/kyonshi/upper.md3" );
	trap_R_RegisterModel( "models/players/kyonshi/head.md3" );
	
	

#endif
	cgs.media.particleSpark = trap_R_RegisterShader("spark");
	cgs.media.particlePlasma = trap_R_RegisterShader("plasmaSparkAs");
	cgs.media.ghostWeapon = trap_R_RegisterShader("ghostWeapon");
	
	CG_ClearParticles ();
	
	cgs.media.blueMarker = trap_R_RegisterShader("icons/blue.tga");
	cgs.media.redMarker = trap_R_RegisterShader("icons/red.tga");
	cgs.media.spawnPoint = trap_R_RegisterModel("models/mapobjects/spawnpoint.md3");
	cgs.media.spawnPointShader = trap_R_RegisterShader("spawnpoint");
	
	cgs.media.playericon = trap_R_RegisterShader("playericon");
	cgs.media.grenadeMapoverview = trap_R_RegisterShader("grenadeMapoverview");
	cgs.media.rocketMapoverview = trap_R_RegisterShader("rocketMapoverview");
	cgs.media.plasmaMapoverview = trap_R_RegisterShader("plasmaMapoverview");
	cgs.media.bfgMapoverview = trap_R_RegisterShader("bfgMapoverview");
/*
	for (i=1; i<MAX_PARTICLES_AREAS; i++)
	{
		{
			int rval;

			rval = CG_NewParticleArea ( CS_PARTICLES + i);
			if (!rval)
				break;
		}
	}
*/
}



/*																																			
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString(void) {
	int i;
	cg.spectatorList[0] = 0;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR ) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}


/*																																			
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	if ( *cg_music.string && Q_stricmp( cg_music.string, "none" ) ) {
		s = (char *)cg_music.string;
	} else {
		s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
        }
}
#ifdef MISSIONPACK
char *CG_GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i\n", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse; // bk001204 - why not?
}

void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource(menuFile);
	if (!handle)
		handle = trap_PC_LoadSource("ui/testhud.menu");
	if (!handle)
		return;

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}


		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	trap_PC_FreeSource(handle);
}

qboolean CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, qtrue);
    
		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token[0] ) {
			return qfalse;
		}

		CG_ParseMenu(token); 
	}
	return qfalse;
}



void CG_LoadMenus(const char *menuFile) {
	char	*token;
	char *p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		Com_Printf( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile );
		len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
		if (!f) {
			CG_Error( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!" );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		trap_FS_FCloseFile( f );
		CG_Error( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i\n", menuFile, len, MAX_MENUDEFFILE );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	
	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if ( !token[0] || token[0] == '}' ) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) {
			if (CG_Load_Menu(&p)) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}



static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
	return qfalse;
}


static int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;
	if (feederID == FEEDER_REDTEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_RED) {
				count++;
			}
		}
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_BLUE) {
				count++;
			}
		}
	} else if (feederID == FEEDER_SCOREBOARD) {
		return cg.numScores;
	}
	return count;
}


void CG_SetScoreSelection(void *p) {
	menuDef_t *menu = (menuDef_t*)p;
	playerState_t *ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == TEAM_RED) {
			red++;
		} else if (cg.scores[i].team == TEAM_BLUE) {
			blue++;
		}
		if (ps->clientNum == cg.scores[i].client) {
			cg.selectedScore = i;
		}
	}

	if (menu == NULL) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM && cgs.ffa_gt!=1) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
	}
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM && cgs.ffa_gt!=1) {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) {
	gitem_t *item;
	int scoreIndex = 0;
	clientInfo_t *info = NULL;
	int team = -1;
	score_t *sp = NULL;

	*handle = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

	if (info && info->infoValid) {
		switch (column) {
			case 0:
				if ( info->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
					item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_REDFLAG ) ) {
					item = BG_FindItemForPowerup( PW_REDFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_BLUEFLAG ) ) {
					item = BG_FindItemForPowerup( PW_BLUEFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else {
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
				}
			break;
			case 1:
				if (team == -1) {
					return "";
				} else if (info->isDead) {
                                        *handle = cgs.media.deathShader;
                                } else {
					*handle = CG_StatusHandle(info->teamTask);
				}
		  break;
			case 2:
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
				if (team == -1) {
					if (cgs.gametype == GT_TOURNAMENT) {
						return va("%i/%i", info->wins, info->losses);
					} else if (info->infoValid && info->team == TEAM_SPECTATOR ) {
						return "Spectator";
					} else {
						return "";
					}
				} else {
					if (info->teamLeader) {
						return "Leader";
					}
				}
			break;
			case 3:
				return info->name;
			break;
			case 4:
				return va("%i", info->score);
			break;
			case 5:
				return va("%4i", sp->time);
			break;
			case 6:
				if ( sp->ping == -1 ) {
					return "connecting";
				} 
				return va("%4i", sp->ping);
			break;
		}
	}

	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) {
	return 0;
}

static void CG_FeederSelection(float feederID, int index) {
	if ( cgs.gametype >= GT_TEAM && cgs.ffa_gt!=1) {
		int i, count;
		int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (index == count) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}
}

static float CG_Cvar_Get(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atof(buff);
}

void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style) {
	CG_Text_Paint(x, y, scale, color, text, 0, limit, style);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
	switch (ownerDraw) {
	  case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, 0);
	  case CG_GAME_STATUS:
			return CG_Text_Width(CG_GetGameStatusText(), scale, 0);
			break;
	  case CG_KILLER:
			return CG_Text_Width(CG_GetKillerText(), scale, 0);
			break;
	  case CG_RED_NAME:
			return CG_Text_Width(cg_redTeamName.string, scale, 0);
			break;
	  case CG_BLUE_NAME:
			return CG_Text_Width(cg_blueTeamName.string, scale, 0);
			break;


	}
	return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
  return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
  trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
  trap_CIN_RunCinematic(handle);
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu( void ) {
	char buff[1024];
	const char *hudSet;

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.drawText = &CG_Text_Paint;
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;   
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	cgDC.registerFont = &trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = trap_Cvar_Set;
	cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &trap_S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &trap_Key_SetBinding;
	//cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	//cgDC.executeText = &trap_Cmd_ExecuteText;
	cgDC.Error = &Com_Error; 
	cgDC.Print = &Com_Printf; 
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &trap_S_RegisterSound;
	cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;
	
	Init_Display(&cgDC);

	Menu_Reset();
	
	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus(hudSet);
}

void CG_AssetCache( void ) {
	//if (Assets.textFont == NULL) {
	//  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}
#endif

#define DEFAULT_MAPCONFIG "mapconfigs/default.cfg"

void CG_mapConfigs( void ){
	fileHandle_t	f;
	char* filename;
	char* buf;
	int len;

	filename = strchr( cgs.mapname, '/' );
	filename++;
	buf = strstr( filename, ".bsp");
	buf[0] = '\0';

	filename = va("mapconfigs/%s.cfg", filename );

	len = trap_FS_FOpenFile( filename, &f, FS_READ );

	if( len <= 0 ){
		CG_Printf("File %s not found, trying to exec mapconfigs/default.cfg\n", filename);
		trap_SendConsoleCommand("exec mapconfigs/default.cfg;");	
		return;
	}

	trap_FS_FCloseFile( f );
	trap_SendConsoleCommand(va("exec %s;", filename));
	
	return;
}

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;
	int i;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= trap_R_RegisterShader( "gfx/2d/bigchars" );

	
	cgs.media.charsetShader32		= trap_R_RegisterShader( "gfx/2d/bigchars32" );
	cgs.media.charsetShader64		= trap_R_RegisterShader( "gfx/2d/bigchars64" );
	cgs.media.charsetShader128		= trap_R_RegisterShader( "gfx/2d/bigchars128" );


	cgs.media.whiteShader		= trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB		= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	cg.weaponSelect = WP_MACHINEGUN;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );
    
	CG_ParseServerinfo();

	// CPM: Setup according to the pro mode settings
	s = CG_ConfigString( CS_PROMODE );
	CPM_UpdateSettings( atoi( s ) );
	// !CPM

	// load the new map
	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

#ifdef MISSIONPACK
	String_Init();
#endif

	cg.loading = qtrue;		// force players to load instead of defer

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

#ifdef MISSIONPACK
	CG_AssetCache();
	CG_LoadHudMenu();      // load new hud stuff
#endif

	cg.loading = qfalse;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

#ifdef MISSIONPACK
	CG_InitTeamChat();
#endif

	CG_ShaderStateChanged();

	//Init challenge system
	challenges_init();

	addChallenge(GENERAL_TEST);

	trap_S_ClearLoopingSounds( qtrue );

	if( cg_mapConfigs.integer )
		CG_mapConfigs();
	
	for( i = 0 ; i < MAX_RESPAWN_TIMERS ; i++ ){
		cgs.respawnTimerEntitynum[i] = -1;
		cgs.respawnTimerQuantity[i] = 0;
		cgs.respawnTimerTime[i] = -1;
		cgs.respawnTimerType[i] = -1;
		cgs.respawnTimerUsed[i] = qfalse;
		cgs.respawnTimerNextItem[i] = -1;
		cgs.respawnTimerClientNum[i] = -1;
	}
	cgs.respawnTimerNumber = 0;
	
	CG_LoadHudFile(cg_hud.string);
	CG_ParseFov();
	CG_ParseZoomFov();
	CG_ParseCrosshair();
	CG_ParseCrosshairSize();
	CG_ParseHitBeep();
	
	cg.lastFov = 90;
	cg.currentFov = 90;
	cg.fovTime = -1000;
	cg.numSpawnpoints = 0;
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
	challenges_save();
	
	if( cg.demoStarted ){
	  	cg.demoStarted = qfalse;
		trap_SendConsoleCommand("stoprecord;");
	}
	
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
#ifndef MISSIONPACK
void CG_EventHandling(int type) {
}



void CG_KeyEvent(int key, qboolean down) {
	CG_Printf( "%i\n", key );
}

void CG_MouseEvent(int x, int y) {
}
#endif

//unlagged - attack prediction #3
// moved from g_weapon.c
/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = floor(v[i]);
		} else {
			v[i] = ceil(v[i]);
		}
	}
}
//unlagged - attack prediction #3

void CG_oaUnofficialCvars( void ) {
	char rendererinfos[128];

	trap_Cvar_VariableStringBuffer("com_maxfps",rendererinfos,sizeof(rendererinfos) );
	if(atoi( rendererinfos ) > 250 )
            	trap_Cvar_Set("com_maxfps","250");

	trap_Cvar_VariableStringBuffer("cg_shadows",rendererinfos,sizeof(rendererinfos) );
	if(atoi( rendererinfos ) > 1 )
		trap_Cvar_Set("cg_shadows","1");
	
	/*trap_Cvar_VariableStringBuffer("r_vertexlight",rendererinfos,sizeof(rendererinfos) );
        if(atoi( rendererinfos ) != 0 ) {
            	trap_Cvar_Set("r_vertexlight","0");
		trap_SendConsoleCommand("vid_restart");
	}*/

        trap_Cvar_Set("con_notifytime", "-1");
}

void CG_ParseFov( void ) {
	char fov[256];
	char buffer[16];
	int counter = WP_GAUNTLET, counter2=0;
	int i;
	
	strcpy(fov, cg_fov.string);
	for ( i = 0; i <= strlen(fov); i++ ){
		if( fov[i] != '/' && i != strlen(fov)){
		  buffer[counter2] = fov[i];
		  counter2++;
		}
		else {
		  buffer[counter2]='\0';
		  cgs.fovs[counter] = atoi(buffer);
		  counter2=0;
		  buffer[counter2] = '\0';
		  
		  counter++;
		  if( counter == WP_NUM_WEAPONS )
			    return;
		}
	}
	for( i = counter; i < WP_NUM_WEAPONS; i++ ){
		cgs.fovs[i] = cgs.fovs[counter-1];
	}
	cgs.fovs[WP_NONE] = cgs.fovs[WP_GAUNTLET];
}

void CG_ParseZoomFov( void ) {
	char fov[256];
	char buffer[16];
	int counter = WP_GAUNTLET, counter2=0;
	int i;
	
	strcpy(fov, cg_zoomFov.string);
	for ( i = 0; i <= strlen(fov); i++ ){
		if( fov[i] != '/' && i != strlen(fov)){
		  buffer[counter2] = fov[i];
		  counter2++;
		}
		else {
		  buffer[counter2]='\0';
		  cgs.zoomfovs[counter] = atoi(buffer);
		  counter2=0;
		  buffer[counter2] = '\0';
		  
		  counter++;
		  if( counter == WP_NUM_WEAPONS )
			    return;
		}
	}
	for( i = counter; i < WP_NUM_WEAPONS; i++ ){
		cgs.zoomfovs[i] = cgs.zoomfovs[counter-1];
	}
	cgs.zoomfovs[WP_NONE] = cgs.zoomfovs[WP_GAUNTLET];
}

void CG_ParseCrosshair( void ) {
	char crosshair[256];
	char buffer[16];
	int counter = WP_GAUNTLET, counter2=0;
	int i;
	
	strcpy(crosshair, cg_drawCrosshair.string);
	for ( i = 0; i <= strlen(crosshair); i++ ){
		if( crosshair[i] != '/' && i != strlen(crosshair)){
		  buffer[counter2] = crosshair[i];
		  counter2++;
		}
		else {
		  buffer[counter2]='\0';
		  cgs.crosshair[counter] = atoi(buffer);
		  counter2=0;
		  buffer[counter2] = '\0';
		  
		  counter++;
		  if( counter == WP_NUM_WEAPONS )
			    return;
		}
	}
	for( i = counter; i < WP_NUM_WEAPONS; i++ ){
		cgs.crosshair[i] = cgs.crosshair[counter-1];
	}
	cgs.crosshair[WP_NONE] = cgs.crosshair[WP_GAUNTLET];
}

void CG_ParseCrosshairSize( void ) {
	char crosshair[256];
	char buffer[16];
	int counter = WP_GAUNTLET, counter2=0;
	int i;
	
	strcpy(crosshair, cg_crosshairSize.string);
	for ( i = 0; i <= strlen(crosshair); i++ ){
		if( crosshair[i] != '/' && i != strlen(crosshair)){
		  buffer[counter2] = crosshair[i];
		  counter2++;
		}
		else {
		  buffer[counter2]='\0';
		  cgs.crosshairSize[counter] = atoi(buffer);
		  counter2=0;
		  buffer[counter2] = '\0';
		  
		  counter++;
		  if( counter == WP_NUM_WEAPONS )
			    return;
		}
	}
	for( i = counter; i < WP_NUM_WEAPONS; i++ ){
		cgs.crosshairSize[i] = cgs.crosshairSize[counter-1];
	}
	cgs.crosshairSize[WP_NONE] = cgs.crosshairSize[WP_GAUNTLET];
}

