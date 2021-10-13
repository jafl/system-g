/******************************************************************************
 globals.cpp

	Access to global objects and factories.

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "globals.h"
#include "PrefsMgr.h"
#include "MDIServer.h"
#include "ViewManPageDialog.h"
#include "FindFileDialog.h"
#include "ChooseSaveFile.h"
#include "FileTreeTable.h"
#include "FileTreeList.h"
#include "FileTree.h"
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXImageCache.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jx/JXDNDManager.h>
#include <jx-af/jx/JXWDManager.h>
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jAssert.h>

static Application*			theApplication       = nullptr;	// owns itself
static PrefsMgr*			thePrefsMgr          = nullptr;
static MDIServer*			theMDIServer         = nullptr;	// owned by JX

static ViewManPageDialog*	theManPageDialog     = nullptr;	// not owned
static FindFileDialog*		theFindFileDialog    = nullptr;	// not owned
static ChooseSaveFile*		theAltChooseSaveFile = nullptr;

static FileTreeTable*		theDNDSource         = nullptr;	// not owned
static FileTreeTable*		theDNDTarget         = nullptr;	// not owned

// owned by JXImageCache
static JXImage*	theFileIcon                   = nullptr;
static JXImage*	theSelectedFileIcon           = nullptr;
static JXImage*	theFolderIcon                 = nullptr;
static JXImage*	theSelectedFolderIcon         = nullptr;
static JXImage* theReadOnlyFolderIcon         = nullptr;
static JXImage* theSelectedReadOnlyFolderIcon = nullptr;
static JXImage* theLockedFolderIcon           = nullptr;
static JXImage* theSelectedLockedFolderIcon   = nullptr;
static JXImage*	theExecIcon                   = nullptr;
static JXImage*	theSelectedExecIcon           = nullptr;
static JXImage*	theUnknownIcon                = nullptr;
static JXImage*	theSelectedUnknownIcon        = nullptr;

// owned by JXImageCache
static JXImage* theHDSmallIcon  = nullptr;
static JXImage* theZipSmallIcon = nullptr;
static JXImage* theFDSmallIcon  = nullptr;
static JXImage* theCDSmallIcon  = nullptr;

// owned by JXImageCache
static JXImage* theTrashEmptySmallIcon         = nullptr;
static JXImage* theTrashEmptySelectedSmallIcon = nullptr;
static JXImage* theTrashFullSmallIcon          = nullptr;
static JXImage* theTrashFullSelectedSmallIcon  = nullptr;

static const JString kTrashDirName("/.trashcan/", JString::kNoCopy);
static JString theTrashDir;			// only need to compute this once
static JDirInfo* theTrashDirInfo = nullptr;
static const JString kRecentFileDirName("/.systemg/recent_files/", JString::kNoCopy);
static JString theRecentFileDir;	// only need to compute this once
const JSize kRecentFileCount     = 20;

const long kTrashCanPerms      = 0700;
const long kRecentFileDirPerms = 0700;

static const JUtf8Byte* kFolderWindowClass = "SystemG_Folder";

// private functions

void	CreateIcons();
void	DeleteIcons();

/******************************************************************************
 CreateGlobals

 ******************************************************************************/

bool
CreateGlobals
	(
	Application* app
	)
{
	theApplication = app;

	CreateIcons();

	JString oldPrefsFile, newPrefsFile;
	if (JGetPrefsDirectory(&oldPrefsFile))
	{
		oldPrefsFile = JCombinePathAndName(oldPrefsFile, JString(".gSystemG.pref", JString::kNoCopy));
		if (JFileExists(oldPrefsFile) &&
			(JPrefsFile::GetFullName(app->GetSignature(), &newPrefsFile)).OK() &&
			!JFileExists(newPrefsFile))
		{
			JRenameFile(oldPrefsFile, newPrefsFile);
		}
	}

	bool isNew;
	thePrefsMgr = jnew PrefsMgr(&isNew);
	assert(thePrefsMgr != nullptr);

	JXInitHelp();

	auto* wdMgr = jnew JXWDManager(app->GetCurrentDisplay(), true);
	assert( wdMgr != nullptr );
	// registers itself

	theMDIServer = jnew MDIServer;
	assert( theMDIServer != nullptr );

	theManPageDialog = jnew ViewManPageDialog(JXGetPersistentWindowOwner());
	assert( theManPageDialog != nullptr );

	theFindFileDialog = jnew FindFileDialog(JXGetPersistentWindowOwner());
	assert( theFindFileDialog != nullptr );

	theAltChooseSaveFile = jnew ChooseSaveFile(thePrefsMgr, kSAltCSSetupID);
	assert( theAltChooseSaveFile != nullptr );

	JString trashDir;
	GetTrashDirectory(&trashDir, false);	// silently creates it

	return isNew;
}

/******************************************************************************
 DeleteGlobals

 ******************************************************************************/

void
DeleteGlobals()
{
	theMDIServer = nullptr;

	// window closed by JXGetPersistentWindowOwner()

	theManPageDialog->JPrefObject::WritePrefs();
	theManPageDialog = nullptr;

	theFindFileDialog->JPrefObject::WritePrefs();
	theFindFileDialog = nullptr;

	jdelete theAltChooseSaveFile;
	theAltChooseSaveFile = nullptr;

	jdelete theTrashDirInfo;
	theTrashDirInfo = nullptr;

	DeleteIcons();

	JXFSBindingManager::Destroy();

	theApplication = nullptr;
	theDNDSource   = nullptr;
	theDNDTarget   = nullptr;

	// this must be last so everybody else can use it to save their setup

	jdelete thePrefsMgr;
	thePrefsMgr = nullptr;
}

/******************************************************************************
 CleanUpBeforeSuddenDeath

	This must be the last one called by Application so we can save
	the preferences to disk.

	*** If the server is dead, you cannot call any code that contacts it.

 ******************************************************************************/

void
CleanUpBeforeSuddenDeath
	(
	const JXDocumentManager::SafetySaveReason reason
	)
{
	if (reason != JXDocumentManager::kAssertFired)
	{
		theAltChooseSaveFile->JPrefObject::WritePrefs();
		theManPageDialog->JPrefObject::WritePrefs();
		theFindFileDialog->JPrefObject::WritePrefs();
	}

	// must be last to save everything

	thePrefsMgr->CleanUpBeforeSuddenDeath(reason);
}

/******************************************************************************
 GetApplication

 ******************************************************************************/

Application*
GetApplication()
{
	assert( theApplication != nullptr );
	return theApplication;
}

/******************************************************************************
 GGetPrefsMgr

 ******************************************************************************/

PrefsMgr*
GetPrefsMgr()
{
	assert(thePrefsMgr != nullptr);
	return thePrefsMgr;
}

/******************************************************************************
 GetMDIServer

 ******************************************************************************/

MDIServer*
GetMDIServer()
{
	assert( theMDIServer != nullptr );
	return theMDIServer;
}

/******************************************************************************
 IsTrashDirectory

 ******************************************************************************/

bool
IsTrashDirectory
	(
	const JString& path
	)
{
	JString dir;
	return GetTrashDirectory(&dir, false) &&
				JSameDirEntry(dir, path);
}

/******************************************************************************
 GetTrashDirectory

 ******************************************************************************/

bool
GetTrashDirectory
	(
	JString*		path,
	const bool	reportErrors
	)
{
	if (!theTrashDir.IsEmpty())
	{
		*path = theTrashDir;
		return true;
	}

	if (!JGetPrefsDirectory(path))
	{
		if (reportErrors)
		{
			JGetUserNotification()->ReportError(JGetString("NoPrefsDir::globals"));
		}
		return false;
	}

	*path = JCombinePathAndName(*path, kTrashDirName);

	JError err = JNoError();
	if (!JDirectoryExists(*path))
	{
		err = JCreateDirectory(*path, kTrashCanPerms);
	}
	else
	{
		err = JSetPermissions(*path, kTrashCanPerms);
	}

	if (err.OK())
	{
		theTrashDir       = *path;
		const bool ok = JDirInfo::Create(theTrashDir, &theTrashDirInfo);
		assert( ok );
		return true;
	}
	else
	{
		path->Clear();
		if (reportErrors)
		{
			const JUtf8Byte* map[] =
		{
				"name", kTrashDirName.GetBytes(),
				"err",  err.GetMessage().GetBytes()
		};
			const JString msg = JGetString("CreatePrefsDirError::globals", map, sizeof(map));
			JGetUserNotification()->ReportError(msg);
		}
		return false;
	}
}

/******************************************************************************
 TrashDirectoryIsEmpty

 ******************************************************************************/

bool
TrashDirectoryIsEmpty()
{
	if (theTrashDirInfo != nullptr)
	{
		theTrashDirInfo->Update();
		return theTrashDirInfo->IsEmpty();
	}
	else
	{
		return true;
	}
}

/******************************************************************************
 UpdateTrash

 ******************************************************************************/

void
UpdateTrash()
{
	if (theTrashDirInfo != nullptr)
	{
		theTrashDirInfo->ForceUpdate();
		(GetApplication())->UpdateTrash();
	}
}

/******************************************************************************
 EmptyTrashDirectory

 ******************************************************************************/

bool
EmptyTrashDirectory()
{
	JString trashDir;
	const bool hasTrash = GetTrashDirectory(&trashDir, false);
	if (hasTrash && JKillDirectory(trashDir))
	{
		JCreateDirectory(trashDir, kTrashCanPerms);
		UpdateTrash();
		return true;
	}
	else if (hasTrash)
	{
		JGetUserNotification()->ReportError(JGetString("EmptyTrashError::globals"));
		return false;
	}
	else
	{
		return true;
	}
}

/******************************************************************************
 DeleteDirEntry

	Returns true if successful.

 ******************************************************************************/

bool
DeleteDirEntry
	(
	const JString& fullName
	)
{
	const JString sysCmd = "rm -rf " + JPrepArgForExec(fullName);
	return Exec(sysCmd);
}

/******************************************************************************
 Exec

	Returns true if successful.

 ******************************************************************************/

bool
Exec
	(
	const JString&	cmd,
	const bool	report
	)
{
	JString errOutput;
	const JError err = JRunProgram(cmd, &errOutput);
	if (!err.OK() && report)
	{
		JGetUserNotification()->ReportError(err.GetMessage());
	}
	return err.OK();
}

/******************************************************************************
 GetManPageDialog

 ******************************************************************************/

ViewManPageDialog*
GetManPageDialog()
{
	assert( theManPageDialog != nullptr );
	return theManPageDialog;
}

/******************************************************************************
 GetFindFileDialog

 ******************************************************************************/

FindFileDialog*
GetFindFileDialog()
{
	assert( theFindFileDialog != nullptr );
	return theFindFileDialog;
}

/******************************************************************************
 GetChooseSaveFile

 ******************************************************************************/

ChooseSaveFile*
GetChooseSaveFile()
{
	assert( theAltChooseSaveFile != nullptr );
	return theAltChooseSaveFile;
}

/******************************************************************************
 DND source

	dndSource can be nullptr

 ******************************************************************************/

bool
GetDNDSource
	(
	const JXWidget*		dndSource,
	FileTreeTable**	widget
	)
{
	if (dndSource != theDNDSource)
	{
		theDNDSource = nullptr;
	}

	*widget = theDNDSource;
	return theDNDSource != nullptr;
}

void
SetDNDSource
	(
	FileTreeTable* widget
	)
{
	theDNDSource = widget;
}

/******************************************************************************
 DND target

	dndTarget can be nullptr

 ******************************************************************************/

bool
GetDNDTarget
	(
	const JXContainer*	dndTarget,
	FileTreeTable**	widget
	)
{
	// We can't compare dndTarget and widget because they aren't the same
	// when dropping on an iconified window.  So we simply insist that a
	// drag is in progress.  This ensures that HandleDNDEnter/Leave() is
	// being called, so theDNDTarget is correct.

	assert( ((theApplication->GetCurrentDisplay())->GetDNDManager())->IsDragging() );

//	if (dndTarget != theDNDTarget)
//		{
//		theDNDTarget = nullptr;
//		}

	*widget = theDNDTarget;
	return theDNDTarget != nullptr;
}

void
SetDNDTarget
	(
	FileTreeTable* widget
	)
{
	theDNDTarget = widget;
}

/******************************************************************************
 GetDNDAskActions

 ******************************************************************************/

void
GetDNDAskActions
	(
	JArray<Atom>*		actionList,
	JPtrArray<JString>*	descriptionList
	)
{
	JXDNDManager* dndMgr = (JXGetApplication()->GetCurrentDisplay())->GetDNDManager();
	actionList->AppendElement(dndMgr->GetDNDActionCopyXAtom());
	actionList->AppendElement(dndMgr->GetDNDActionMoveXAtom());
	actionList->AppendElement(dndMgr->GetDNDActionLinkXAtom());

	auto* s = jnew JString(JGetString("DNDCopyDescription::globals"));
	assert( s != nullptr );
	descriptionList->Append(s);

	s = jnew JString(JGetString("DNDMoveDescription::globals"));
	assert( s != nullptr );
	descriptionList->Append(s);

	s = jnew JString(JGetString("DNDLinkDescription::globals"));
	assert( s != nullptr );
	descriptionList->Append(s);
}

/******************************************************************************
 GetWMClassInstance

 ******************************************************************************/

const JUtf8Byte*
GetWMClassInstance()
{
	return JGetString("Name").GetBytes();
}

const JUtf8Byte*
GetFolderWindowClass()
{
	return kFolderWindowClass;
}

/******************************************************************************
 GetVersionNumberStr

 ******************************************************************************/

const JString&
GetVersionNumberStr()
{
	return JGetString("VERSION");
}

/******************************************************************************
 GetVersionStr

 ******************************************************************************/

JString
GetVersionStr()
{
	const JUtf8Byte* map[] =
	{
		"version",   JGetString("VERSION").GetBytes(),
		"copyright", JGetString("COPYRIGHT").GetBytes()
	};
	return JGetString("Description::globals", map, sizeof(map));
}

/******************************************************************************
 Icons

 ******************************************************************************/

#include <jx-af/image/jx/jx_hard_disk_small.xpm>
#include <jx-af/image/jx/jx_zip_disk_small.xpm>
#include <jx-af/image/jx/jx_floppy_disk_small.xpm>
#include <jx-af/image/jx/jx_cdrom_disk_small.xpm>

#include <jx-af/image/jx/jx_trash_can_empty_small.xpm>
#include <jx-af/image/jx/jx_trash_can_empty_selected_small.xpm>
#include <jx-af/image/jx/jx_trash_can_full_small.xpm>
#include <jx-af/image/jx/jx_trash_can_full_selected_small.xpm>

#include <jx-af/image/jx/jx_folder_large.xpm>
#include <jx-af/image/jx/jx_folder_selected_large.xpm>
#include <jx-af/image/jx/jx_folder_read_only_large.xpm>
#include <jx-af/image/jx/jx_folder_read_only_selected_large.xpm>
#include "syg_home_folder_large.xpm"
#include "syg_home_folder_selected_large.xpm"
#include "syg_floppy_folder_large.xpm"
#include "syg_floppy_folder_read_only_large.xpm"
#include "syg_cdrom_folder_large.xpm"
#include "syg_cdrom_folder_read_only_large.xpm"
#include <jx-af/image/jx/jx_trash_can_empty_large.xpm>
#include <jx-af/image/jx/jx_trash_can_empty_selected_large.xpm>
#include <jx-af/image/jx/jx_trash_can_full_large.xpm>
#include <jx-af/image/jx/jx_trash_can_full_selected_large.xpm>
#include <jx-af/image/jx/jx_hard_disk_large.xpm>
#include <jx-af/image/jx/jx_hard_disk_selected_large.xpm>
#include <jx-af/image/jx/jx_floppy_disk_large.xpm>
#include <jx-af/image/jx/jx_floppy_disk_selected_large.xpm>
#include <jx-af/image/jx/jx_cdrom_disk_large.xpm>
#include <jx-af/image/jx/jx_cdrom_disk_selected_large.xpm>

#include <jx-af/image/jx/jx_plain_file_small.xpm>
#include <jx-af/image/jx/jx_plain_file_selected_small.xpm>
#include <jx-af/image/jx/jx_folder_small.xpm>
#include <jx-af/image/jx/jx_folder_selected_small.xpm>
#include <jx-af/image/jx/jx_folder_read_only_small.xpm>
#include <jx-af/image/jx/jx_folder_read_only_selected_small.xpm>
#include <jx-af/image/jx/jx_folder_locked_small.xpm>
#include <jx-af/image/jx/jx_folder_locked_selected_small.xpm>
#include <jx-af/image/jx/jx_executable_small.xpm>
#include <jx-af/image/jx/jx_executable_selected_small.xpm>
#include <jx-af/image/jx/jx_unknown_file_small.xpm>
#include <jx-af/image/jx/jx_unknown_file_selected_small.xpm>

void
CreateIcons()
{
	JXImageCache* c = theApplication->GetDisplay(1)->GetImageCache();

	theHDSmallIcon  = c->GetImage(jx_hard_disk_small);
	theZipSmallIcon = c->GetImage(jx_zip_disk_small);
	theFDSmallIcon  = c->GetImage(jx_floppy_disk_small);
	theCDSmallIcon  = c->GetImage(jx_cdrom_disk_small);

	theTrashEmptySmallIcon         = c->GetImage(jx_trash_can_empty_small);
	theTrashEmptySelectedSmallIcon = c->GetImage(jx_trash_can_empty_selected_small);
	theTrashFullSmallIcon          = c->GetImage(jx_trash_can_full_small);
	theTrashFullSelectedSmallIcon  = c->GetImage(jx_trash_can_full_selected_small);

	theFileIcon                   = c->GetImage(jx_plain_file_small);
	theSelectedFileIcon           = c->GetImage(jx_plain_file_selected_small);

	theFolderIcon                 = c->GetImage(jx_folder_small);
	theSelectedFolderIcon         = c->GetImage(jx_folder_selected_small);
	theReadOnlyFolderIcon         = c->GetImage(jx_folder_read_only_small);
	theSelectedReadOnlyFolderIcon = c->GetImage(jx_folder_read_only_selected_small);
	theLockedFolderIcon           = c->GetImage(jx_folder_locked_small);
	theSelectedLockedFolderIcon   = c->GetImage(jx_folder_locked_selected_small);

	theExecIcon                   = c->GetImage(jx_executable_small);
	theSelectedExecIcon           = c->GetImage(jx_executable_selected_small);
	theUnknownIcon                = c->GetImage(jx_unknown_file_small);
	theSelectedUnknownIcon        = c->GetImage(jx_unknown_file_selected_small);
}

void
DeleteIcons()
{
	// placeholder in case we need to do something in the future
}

JXImage*
GetFileSmallIcon
	(
	const bool selected
	)
{
	assert( theFileIcon != nullptr );
	return selected ? theSelectedFileIcon : theFileIcon;
}

JXImage*
GetFolderSmallIcon
	(
	const bool selected
	)
{
	assert( theFolderIcon != nullptr );
	return selected ? theSelectedFolderIcon : theFolderIcon;
}

JXImage*
GetReadOnlyFolderSmallIcon
	(
	const bool selected
	)
{
	assert( theReadOnlyFolderIcon != nullptr );
	return selected ? theSelectedReadOnlyFolderIcon : theReadOnlyFolderIcon;
}

JXImage*
GetLockedFolderSmallIcon
	(
	const bool selected
	)
{
	assert( theLockedFolderIcon != nullptr );
	return selected ? theSelectedLockedFolderIcon : theLockedFolderIcon;
}

JXImage*
GetExecSmallIcon
	(
	const bool selected
	)
{
	assert( theExecIcon != nullptr );
	return selected ? theSelectedExecIcon : theExecIcon;
}

JXImage*
GetUnknownSmallIcon
	(
	const bool selected
	)
{
	assert( theExecIcon != nullptr );
	return selected ? theSelectedUnknownIcon : theUnknownIcon;
}

JXImage*
GetDirectorySmallIcon
	(
	const JString& path
	)
{
	JMountType type;
	JXImage* image;
	if ((GetApplication())->IsMountPoint(path, &type) &&
		GetMountPointSmallIcon(type, &image))
	{
		return image;
	}
	else if (IsTrashDirectory(path))
	{
		return GetTrashSmallIcon();
	}
	else if (!JFSFileTreeNode::CanHaveChildren(path))
	{
		return GetLockedFolderSmallIcon();
	}
	else if (!JDirectoryWritable(path))
	{
		return GetReadOnlyFolderSmallIcon();
	}
	else
	{
		return GetFolderSmallIcon();
	}
}

JXImage*
GetTrashSmallIcon
	(
	const bool selected
	)
{
#if defined SYSTEM_G
	if (TrashDirectoryIsEmpty())
#elif defined JX_FVWM2_TASKBAR
	if (TBTrashDirectoryIsEmpty())
#endif
	{
		return selected ? theTrashEmptySelectedSmallIcon : theTrashEmptySmallIcon;
	}
	else
	{
		return selected ? theTrashFullSelectedSmallIcon : theTrashFullSmallIcon;
	}
}

bool
GetMountPointSmallIcon
	(
	const JMountType	type,
	JXImage**			image
	)
{
	if (type == kJHardDisk)
	{
		*image = theHDSmallIcon;
	}
	else if (type == kJFloppyDisk)
	{
		*image = theFDSmallIcon;
	}
	else if (type == kJCDROM)
	{
		*image = theCDSmallIcon;
	}
	else
	{
		*image = nullptr;
	}
	return *image != nullptr;
}

// Returns type value.  This value is arbitrary.  Do not store it in files.
// Never returns zero.  This value is reserved to mean "not set".

JIndex
GetMountPointLargeIcon
	(
	const JString&		path,
	FileTreeList*	fileList,
	JXPM*				plainIcon,
	JXPM*				selectedIcon
	)
{
	JMountType type;
	const bool isMP = theApplication->IsMountPoint(path, &type);

	if (!isMP)
	{
		bool writable, isTop;
		JString device, fsTypeString;
		JFileSystemType fsType;
		if (JIsMounted(path, &writable, &isTop, &device, &fsType, &fsTypeString))
		{
			type = JGetUserMountPointType(path, device, fsTypeString);
		}
		else
		{
			type = kJHardDisk;
		}
	}

	const bool writable = JDirectoryWritable(path);

	JString dir;
	if (JGetHomeDirectory(&dir) && JSameDirEntry(dir, path))
	{
		*plainIcon    = syg_home_folder_large;
		*selectedIcon = syg_home_folder_selected_large;
		return 1;
	}
	else if (IsTrashDirectory(path))
	{
		(fileList->GetFileTree())->Update();
		if (fileList->IsEmpty())
		{
			*plainIcon    = jx_trash_can_empty_large;
			*selectedIcon = jx_trash_can_empty_selected_large;
			return 2;
		}
		else
		{
			*plainIcon    = jx_trash_can_full_large;
			*selectedIcon = jx_trash_can_full_selected_large;
			return 3;
		}
	}
	else if (!isMP && type == kJFloppyDisk && !writable)
	{
		*plainIcon    = syg_floppy_folder_read_only_large;
		*selectedIcon = syg_floppy_folder_read_only_large;
		return 4;
	}
	else if (!isMP && type == kJFloppyDisk)
	{
		*plainIcon    = syg_floppy_folder_large;
		*selectedIcon = syg_floppy_folder_large;
		return 5;
	}
	else if (!isMP && type == kJCDROM && !writable)
	{
		*plainIcon    = syg_cdrom_folder_read_only_large;
		*selectedIcon = syg_cdrom_folder_read_only_large;
		return 4;
	}
	else if (!isMP && type == kJCDROM)
	{
		*plainIcon    = syg_cdrom_folder_large;
		*selectedIcon = syg_cdrom_folder_large;
		return 5;
	}
	else if (!isMP && !writable)
	{
		*plainIcon    = jx_folder_read_only_large;
		*selectedIcon = jx_folder_read_only_selected_large;
		return 6;
	}
	else if (!isMP)
	{
		*plainIcon    = jx_folder_large;
		*selectedIcon = jx_folder_selected_large;
		return 7;
	}
	else if (type == kJFloppyDisk)
	{
		*plainIcon    = jx_floppy_disk_large;
		*selectedIcon = jx_floppy_disk_selected_large;
		return 8;
	}
	else if (type == kJCDROM)
	{
		*plainIcon    = jx_cdrom_disk_large;
		*selectedIcon = jx_cdrom_disk_selected_large;
		return 9;
	}
	else
	{
		*plainIcon    = jx_hard_disk_large;
		*selectedIcon = jx_hard_disk_selected_large;
		return 11;
	}
}

/******************************************************************************
 GetRecentFileDirectory

 ******************************************************************************/

bool
GetRecentFileDirectory
	(
	JString*		path,
	const bool	reportErrors
	)
{
	if (!theRecentFileDir.IsEmpty())
	{
		*path = theRecentFileDir;
		return true;
	}

	if (!JGetPrefsDirectory(path))
	{
		if (reportErrors)
		{
			JGetUserNotification()->ReportError(JGetString("NoPrefsDir::globals"));
		}
		return false;
	}

	*path = JCombinePathAndName(*path, kRecentFileDirName);

	JError err = JNoError();
	if (!JDirectoryExists(*path))
	{
		err = JCreateDirectory(*path, kRecentFileDirPerms);
	}
	else
	{
		err = JSetPermissions(*path, kRecentFileDirPerms);
	}

	if (err.OK())
	{
		theRecentFileDir = *path;
		return true;
	}
	else
	{
		path->Clear();
		if (reportErrors)
		{
			const JUtf8Byte* map[] =
		{
				"name", kRecentFileDirName.GetBytes(),
				"err",  err.GetMessage().GetBytes()
		};
			const JString msg = JGetString("CreatePrefsDirError::globals", map, sizeof(map));
			JGetUserNotification()->ReportError(msg);
		}
		return false;
	}
}

/******************************************************************************
 AddRecentFile

 ******************************************************************************/

void
AddRecentFile
	(
	const JString& fullname
	)
{
	JString recentDir;
	JString filename;
	JString path;
	if (GetRecentFileDirectory(&recentDir) &&
		JSplitPathAndName(fullname, &path, &filename))
	{
		const JString recentFile = JCombinePathAndName(recentDir, filename);
		if (JNameUsed(recentFile))
		{
			JRemoveFile(recentFile);
			JCreateSymbolicLink(fullname, recentFile);
			return;
		}

		// remove oldest links such that only kRecentFileCount - 1 remain

		JDirInfo* info;
		if (JDirInfo::Create(recentDir, &info))
		{
			bool changed = false;

			JSize count = info->GetEntryCount();
			for (JIndex i=1; i<=count; i++)
			{
				if (info->GetEntry(i).IsBrokenLink())
				{
					JRemoveFile(info->GetEntry(i).GetFullName());
					changed = true;
				}
			}

			if (changed)
			{
				info->ForceUpdate();
			}

			count = info->GetEntryCount();
			if (count >= kRecentFileCount)
			{
				info->ChangeSort(JDirEntry::CompareModTimes, JListT::kSortDescending);
				for (JIndex i=count; i>=kRecentFileCount; i--)
				{
					JRemoveFile(info->GetEntry(i).GetFullName());
				}
			}

			// add new entry

			JCreateSymbolicLink(fullname, recentFile);
		}
	}
}
