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
/*
=======================================================================

DEMOS MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK0			"menu/art_blueish/back_0"
#define ART_BACK1			"menu/art_blueish/back_1"
#define ART_GO0				"menu/art_blueish/play_0"
#define ART_GO1				"menu/art_blueish/play_1"
#define ART_FRAMEL			"menu/art_blueish/frame2_l"
#define ART_FRAMER			"menu/art_blueish/frame1_r"
#define ART_ARROWS			"menu/art_blueish/arrows_vert_0"
#define ART_ARROWUP		"menu/art_blueish/arrows_vert_top"
#define ART_ARROWDN		"menu/art_blueish/arrows_vert_bot"

#define MAX_DEMOS			256
#define NAMEBUFSIZE			( MAX_DEMOS * 16 )

#define ID_BACK				10
#define ID_GO				11
#define ID_LIST				12
#define ID_SCROLLDN			13
#define ID_SCROLLUP				14

#define ARROWS_WIDTH		128
#define ARROWS_HEIGHT		48

static void Demos_MenuInit( qboolean firstStart );
static char	*currentFolder = "";

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menulist_s		list;

	menubitmap_s	arrows;
	menubitmap_s	left;
	menubitmap_s	right;
	menubitmap_s	back;
	menubitmap_s	go;

	int				numDemos;
	int				numFolders;
	char			names[NAMEBUFSIZE];
	char			folders[NAMEBUFSIZE];
	char			*demolist[MAX_DEMOS];
} demos_t;

static demos_t	s_demos;


/*
===============
Demos_MenuEvent
===============
*/
static void Demos_MenuEvent( void *ptr, int event ) {
	char *buffer;
	int len;
	char	path[512];
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GO:
		
		//finds the .dm_ string in the current item
		buffer = strstr( s_demos.list.itemnames[s_demos.list.curvalue], ".dm_");
		
		//The current Item is not a demo
		if( !buffer ){
			len = strlen( s_demos.list.itemnames[s_demos.list.curvalue] );
			//if( strcmp( s_demos.list.itemnames[s_demos.list.curvalue], ".." ) != 0 ){
				if( currentFolder )
					strcat(currentFolder, "/");
				currentFolder = "";
				strcat(currentFolder, s_demos.list.itemnames[s_demos.list.curvalue]);
			//}
			Demos_MenuInit( qfalse );
			UI_PushMenu( &s_demos.menu );
			
		}
		else{
			UI_ForceMenuOff ();
			if( currentFolder ){
				Com_sprintf(path, sizeof(path),"demo %s/%s\n", currentFolder, s_demos.list.itemnames[s_demos.list.curvalue]);
				trap_Cmd_ExecuteText( EXEC_APPEND, path );
			}
			else{
				Com_sprintf(path, sizeof(path),"demo %s\n", s_demos.list.itemnames[s_demos.list.curvalue]);
				trap_Cmd_ExecuteText( EXEC_APPEND, path );
			}				
		}
		//Com_Printf("%s - %s\n", currentFolder, path);
		break;
	case ID_BACK:
		for(; *currentFolder; currentFolder++)
			*currentFolder = '\0';
		UI_PopMenu();
		break;

	case ID_SCROLLUP:
		ScrollList_Key( &s_demos.list, K_UPARROW );
		break;

	case ID_SCROLLDN:
		ScrollList_Key( &s_demos.list, K_DOWNARROW );
                break;

	}
}


/*
=================
UI_DemosMenu_Key
=================
*/
static sfxHandle_t UI_DemosMenu_Key( int key ) {
	menucommon_s	*item;

        item = Menu_ItemAtCursor( &s_demos.menu );

        if( key == K_MWHEELUP ) {
            ScrollList_Key( &s_demos.list, K_UPARROW );
        }

        if( key == K_MWHEELDOWN ) {
            ScrollList_Key( &s_demos.list, K_DOWNARROW );
        }

	return Menu_DefaultKey( &s_demos.menu, key );
}


static void meowdrawdemo( void ) {
    Menu_Draw(&s_demos.menu);
}

static void sortDemos( const char **list, int itemnum ){
	int i,j;
	const char* buffer;
	
	for( j = 0; j < itemnum; j++ )
		for( i = 0; i < itemnum - j; i++ ){
			if( Q_stricmp(list[i],list[i+1]) == -1 && list[i+1]){
				buffer = list[i];
				list[i] = list[i+1];
				list[i+1] = buffer;
			}
		}
		
	
}

/*
===============
Demos_MenuInit
===============
*/
static void Demos_MenuInit( qboolean firstStart ) {
	int		i;
	int		len;
	char	*demoname, extension[16];
	char	folder[128];
	//const char	*buffer;
	
	if( firstStart ){
		currentFolder = NULL;
		memset( &s_demos, 0 ,sizeof(demos_t) );
	
	
	
	s_demos.menu.key = UI_DemosMenu_Key;

	Demos_Cache();

	s_demos.menu.fullscreen = qtrue;
        s_demos.menu.wrapAround = qtrue;
        s_demos.menu.draw = meowdrawdemo;

	s_demos.banner.generic.type		= MTYPE_BTEXT;
	s_demos.banner.generic.x		= 320;
	s_demos.banner.generic.y		= 16;
	s_demos.banner.string			= "DEMOS";
	s_demos.banner.color			= color_white;
	s_demos.banner.style			= UI_CENTER;

	s_demos.framel.generic.type		= MTYPE_BITMAP;
	s_demos.framel.generic.name		= ART_FRAMEL;
	s_demos.framel.generic.flags	= QMF_INACTIVE;
	s_demos.framel.generic.x		= 0;
	s_demos.framel.generic.y		= 78;
	s_demos.framel.width			= 256;
	s_demos.framel.height			= 329;

	s_demos.framer.generic.type		= MTYPE_BITMAP;
	s_demos.framer.generic.name		= ART_FRAMER;
	s_demos.framer.generic.flags	= QMF_INACTIVE;
	s_demos.framer.generic.x		= 376;
	s_demos.framer.generic.y		= 76;
	s_demos.framer.width			= 256;
	s_demos.framer.height			= 334;

	s_demos.arrows.generic.type		= MTYPE_BITMAP;
	s_demos.arrows.generic.name		= ART_ARROWS;
	s_demos.arrows.generic.flags	= QMF_INACTIVE;
	s_demos.arrows.generic.x		= 512+48+12;
	s_demos.arrows.generic.y		= 240-64+48;
	s_demos.arrows.width			= 64;
	s_demos.arrows.height			= 128;

	s_demos.left.generic.type		= MTYPE_BITMAP;
	s_demos.left.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_demos.left.generic.x			= 512+48+12;
	s_demos.left.generic.y			= 240-64+48;
	s_demos.left.generic.id			= ID_SCROLLUP;
	s_demos.left.generic.callback	= Demos_MenuEvent;
	s_demos.left.width				= 64;
	s_demos.left.height				= 64;
	s_demos.left.focuspic			= ART_ARROWUP;

	s_demos.right.generic.type		= MTYPE_BITMAP;
	s_demos.right.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_demos.right.generic.x			= 512+48+12;
	s_demos.right.generic.y			= 240+48;
	s_demos.right.generic.id		= ID_SCROLLDN;
	s_demos.right.generic.callback	= Demos_MenuEvent;
	s_demos.right.width				= 64;
	s_demos.right.height			= 64;
	s_demos.right.focuspic			= ART_ARROWDN;

	s_demos.back.generic.type		= MTYPE_BITMAP;
	s_demos.back.generic.name		= ART_BACK0;
	s_demos.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_demos.back.generic.id			= ID_BACK;
	s_demos.back.generic.callback	= Demos_MenuEvent;
	s_demos.back.generic.x			= 0;
	s_demos.back.generic.y			= 480-64;
	s_demos.back.width				= 128;
	s_demos.back.height				= 64;
	s_demos.back.focuspic			= ART_BACK1;

	s_demos.go.generic.type			= MTYPE_BITMAP;
	s_demos.go.generic.name			= ART_GO0;
	s_demos.go.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_demos.go.generic.id			= ID_GO;
	s_demos.go.generic.callback		= Demos_MenuEvent;
	s_demos.go.generic.x			= 640;
	s_demos.go.generic.y			= 480-64;
	s_demos.go.width				= 128;
	s_demos.go.height				= 64;
	s_demos.go.focuspic				= ART_GO1;

	s_demos.list.generic.type		= MTYPE_SCROLLLIST;
	s_demos.list.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS|QMF_SMALLFONT;
	s_demos.list.generic.callback	= Demos_MenuEvent;
	s_demos.list.generic.id			= ID_LIST;
	s_demos.list.generic.x			= 22;
	s_demos.list.generic.y			= 50;
	s_demos.list.width				= 70;
	s_demos.list.height				= 23;
	
	}

	if( currentFolder )
		Com_sprintf(folder, sizeof(folder),"demos/%s", currentFolder);
	else{
		Com_sprintf(folder, sizeof(folder),"demos");
	}
	
	s_demos.numFolders = trap_FS_GetFileList( folder, "/", s_demos.folders, sizeof(s_demos.folders) );
	
	Com_sprintf(extension, sizeof(extension), "dm_%d", (int)trap_Cvar_VariableValue( "protocol" ) );
	
	s_demos.list.numitems = trap_FS_GetFileList( folder, extension, s_demos.names, sizeof(s_demos.names) );
	
	Com_Printf("%s\n", folder);
	
	s_demos.list.numitems			+= s_demos.numFolders;
	s_demos.list.itemnames			= (const char **)s_demos.demolist;
	//s_demos.list.columns			= 1;
	
	

	if (!s_demos.list.numitems) {
		strcpy( s_demos.names, "No Demos Found." );
		s_demos.list.numitems = 1;

		//degenerate case, not selectable
		s_demos.go.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
	}
	else if (s_demos.list.numitems > MAX_DEMOS)
		s_demos.list.numitems = MAX_DEMOS;

	demoname = s_demos.folders;
	for ( i = 0; i < s_demos.numFolders; i++ ) {
		s_demos.list.itemnames[i] = demoname;

		// strip extension
		len = strlen( demoname );
		if (!Q_stricmp(demoname +  len - 4,".dm3"))
			demoname[len-4] = '\0';

//		Q_strupr(demoname);

		demoname += len + 1;
	}
	demoname =  s_demos.names;
	for ( i = s_demos.numFolders; i < s_demos.list.numitems+1; i++ ) {
		s_demos.list.itemnames[i] = demoname;

		// strip extension
		len = strlen( demoname );
		if (!Q_stricmp(demoname +  len - 4,".dm3"))
			demoname[len-4] = '\0';

//		Q_strupr(demoname);

		demoname += len + 1;
	}
	
	sortDemos( s_demos.list.itemnames, s_demos.list.numitems );
	
	Menu_AddItem( &s_demos.menu, &s_demos.banner );
	Menu_AddItem( &s_demos.menu, &s_demos.framel );
	Menu_AddItem( &s_demos.menu, &s_demos.framer );
	Menu_AddItem( &s_demos.menu, &s_demos.list );
	Menu_AddItem( &s_demos.menu, &s_demos.arrows );
	Menu_AddItem( &s_demos.menu, &s_demos.left );
	Menu_AddItem( &s_demos.menu, &s_demos.right );
	Menu_AddItem( &s_demos.menu, &s_demos.back );
	Menu_AddItem( &s_demos.menu, &s_demos.go );	
}

/*
=================
Demos_Cache
=================
*/
void Demos_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_GO0 );
	trap_R_RegisterShaderNoMip( ART_GO1 );
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_ARROWS );
	trap_R_RegisterShaderNoMip( ART_ARROWUP );
	trap_R_RegisterShaderNoMip( ART_ARROWDN );
}

/*
===============
UI_DemosMenu
===============
*/
void UI_DemosMenu( void ) {
	
	
	Com_Printf("%s\n", currentFolder);
	Demos_MenuInit( qtrue );
	UI_PushMenu( &s_demos.menu );
}
