/******************************************************************************
 globals.h

	Copyright (C) 1999 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_globals
#define _H_globals

// we include these for convenience

#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jx/JXDocumentManager.h>
#include <jx-af/jcore/jMountUtil.h>
#include <jx-af/jcore/JXPM.h>
#include "Application.h"

class JXContainer;
class JXWidget;
class PrefsMgr;
class MDIServer;
class ViewManPageDialog;
class FindFileDialog;
class ChooseSaveFile;
class FileTreeTable;
class FileTreeList;

Application*		GetApplication();
PrefsMgr*			GetPrefsMgr();
MDIServer*			GetMDIServer();
ViewManPageDialog*	GetManPageDialog();
FindFileDialog*		GetFindFileDialog();
ChooseSaveFile*		GetChooseSaveFile();

bool	IsTrashDirectory(const JString& path);
bool	GetTrashDirectory(JString* path, const bool reportErrors = true);
bool	TrashDirectoryIsEmpty();
void	UpdateTrash();
bool	EmptyTrashDirectory();

bool	DeleteDirEntry(const JString& fullName);
bool	Exec(const JString& cmd, const bool report = true);

	// called by Application

bool	CreateGlobals(Application* app);
void	DeleteGlobals();
void	CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason);

	// called by RecentFileMenu

bool	GetRecentFileDirectory(JString* path, const bool reportErrors = true);
void	AddRecentFile(const JString& fullname);

	// used during intra-app DND

bool	GetDNDSource(const JXWidget* dndSource, FileTreeTable** widget);
void	SetDNDSource(FileTreeTable* widget);

bool	GetDNDTarget(const JXContainer* dndTarget, FileTreeTable** widget);
void	SetDNDTarget(FileTreeTable* widget);

void	GetDNDAskActions(JArray<Atom>* actionList,
						 JPtrArray<JString>* descriptionList);

	// icons

JXImage*	GetFileSmallIcon(const bool selected = false);
JXImage*	GetFolderSmallIcon(const bool selected = false);
JXImage*	GetReadOnlyFolderSmallIcon(const bool selected = false);
JXImage*	GetLockedFolderSmallIcon(const bool selected = false);
JXImage*	GetExecSmallIcon(const bool selected = false);
JXImage*	GetUnknownSmallIcon(const bool selected = false);
JXImage*	GetDirectorySmallIcon(const JString& path);
JIndex		GetMountPointLargeIcon(const JString& path, FileTreeList* fileList,
								   JXPM* plainIcon, JXPM* selectedIcon);

bool		GetMountPointSmallIcon(const JMountType type, JXImage** image);
JXImage*	GetTrashSmallIcon(const bool selected = false);

	// version info

const JString&	GetVersionNumberStr();
JString			GetVersionStr();

	// called by Directors

const JUtf8Byte*	GetWMClassInstance();
const JUtf8Byte*	GetFolderWindowClass();

#endif
