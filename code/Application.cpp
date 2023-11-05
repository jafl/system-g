/******************************************************************************
 Application.cpp

	BASE CLASS = JXApplication, JPrefObject

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#include "Application.h"
#include "PrefsMgr.h"
#include "MDIServer.h"
#include "TreeDir.h"
#include "FileTreeTable.h"
#include "AboutDialog.h"
#include "stringData.h"
#include "globals.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXToolBar.h>
#include <jx-af/jx/JXChoosePathDialog.h>
#include <jx-af/jfs/JFSFileTreeNode.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jAssert.h>

static const JUtf8Byte* kAppSignature = "systemg";

#ifdef _J_MACOS
static const JString kDefaultTermCmd       ("open -a Terminal $p", JString::kNoCopy);
static const JString kDefaultGitStatusCmd  ("git gui", JString::kNoCopy);
static const JString kDefaultGitHistoryCmd ("gitk --all", JString::kNoCopy);
#elif defined _J_CYGWIN
static const JString kDefaultTermCmd       ("xterm -title $n -n $n", JString::kNoCopy);
static const JString kDefaultGitStatusCmd  ("xterm -e git gui", JString::kNoCopy);
static const JString kDefaultGitHistoryCmd ("xterm -e gitk --all", JString::kNoCopy);
#else
static const JString kDefaultTermCmd       ("gnome-terminal --working-directory=$p", JString::kNoCopy);
static const JString kDefaultGitStatusCmd  ("git gui", JString::kNoCopy);
static const JString kDefaultGitHistoryCmd ("gitk --all", JString::kNoCopy);
#endif

static const JString kDefaultPostCheckoutCmd("jcc --reload-open", JString::kNoCopy);

const JFileVersion kCurrentPrefsVersion = 5;

	// version  5 adds itsGitHistoryCmd
	// version  4 adds itsGitStatusCmd
	// version  3 adds itsPostCheckoutCmd
	// version  2 adds itsTermCmd
	// version  1 adds itsMountPointPrefs

// JBroadcaster messages

const JUtf8Byte* Application::kTrashNeedsUpdate = "TrashNeedsUpdate::Application";
const JUtf8Byte* Application::kShortcutsChanged = "ShortcutsChanged::Application";

/******************************************************************************
 Constructor

 ******************************************************************************/

Application::Application
	(
	int*		argc,
	char*		argv[],
	bool*		displayAbout,
	JString*	prevVersStr
	)
	:
	JXApplication(argc, argv, kAppSignature, kDefaultStringData),
	JPrefObject(nullptr, kSAppID),
	itsTermCmd(kDefaultTermCmd),
	itsGitStatusCmd(kDefaultGitStatusCmd),
	itsGitHistoryCmd(kDefaultGitHistoryCmd),
	itsPostCheckoutCmd(kDefaultPostCheckoutCmd)
{
	// Create itsWindowList first so DirectorClosed() won't crash when
	// warn that prefs are unreadable.

	itsWindowList = jnew JPtrArray<TreeDir>(JPtrArrayT::kForgetAll);
	assert( itsWindowList != nullptr );

	*displayAbout = CreateGlobals(this);

	if (!*displayAbout)
	{
		*prevVersStr = GetPrefsMgr()->GetSystemGVersionStr();
		if (*prevVersStr == GetVersionNumberStr())
		{
			prevVersStr->Clear();
		}
		else
		{
			*displayAbout = true;
		}
	}
	else
	{
		prevVersStr->Clear();
	}

	itsMountPointList = jnew JMountPointList(JPtrArrayT::kDeleteAll);
	JGetUserMountPointList(itsMountPointList, &itsMountPointState);

	itsShortcutList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
	assert( itsShortcutList != nullptr );
	itsShortcutList->SetCompareFunction(CompareShortcuts);
	itsShortcutList->SetSortOrder(JListT::kSortAscending);

	itsMountPointPrefs = jnew JStringPtrMap<JString>(JPtrArrayT::kDeleteAll);
	assert( itsMountPointPrefs != nullptr );

	SetPrefInfo(GetPrefsMgr(), kSAppID);
	JPrefObject::ReadPrefs();

	ListenTo(itsShortcutList);	// don't write taskbar file unless it doesn't exist
}

/******************************************************************************
 Destructor

 ******************************************************************************/

Application::~Application()
{
	StopListening(itsShortcutList);		// don't write empty taskbar file

	JPrefObject::WritePrefs();

	jdelete itsWindowList;
	jdelete itsShortcutList;
	jdelete itsMountPointPrefs;
	jdelete itsMountPointList;

	DeleteGlobals();
}

/******************************************************************************
 OpenDirectory

 ******************************************************************************/

void
Application::OpenDirectory()
{
	auto* dlog = JXChoosePathDialog::Create(JXChoosePathDialog::kAcceptReadable);
	if (dlog->DoDialog())
	{
		OpenDirectory(dlog->GetPath());
	}
}

bool
Application::OpenDirectory
	(
	const JString&	pathName,
	TreeDir**		dir,
	JIndex*			row,
	const bool		deiconify,
	const bool		reportError,
	const bool		forceNew,
	const bool		clearSelection
	)
{
	if (dir != nullptr)
	{
		*dir = nullptr;
	}

	if (row != nullptr)
	{
		*row = 0;
	}

	JString fixedName, trueName;
	if (!JExpandHomeDirShortcut(pathName, &fixedName) ||
		!JConvertToAbsolutePath(fixedName, JString::empty, &trueName))
	{
		if (reportError)
		{
			const JUtf8Byte* map[] =
		{
				"name", pathName.GetBytes()
		};
			JGetUserNotification()->ReportError(JGetString("PathDoesNotExist::Application", map, sizeof(map)));
		}
		return false;
	}

	// if file, select it after opening the window

	JString selectName;
	if (JFileExists(trueName) ||
		!JDirectoryExists(trueName))	// broken link
	{
		JStripTrailingDirSeparator(&trueName);
		JString path;
		JSplitPathAndName(trueName, &path, &selectName);
		trueName = path;
	}

	// can't check this until after making sure that trueName is directory

	if (!JFSFileTreeNode::CanHaveChildren(trueName))
	{
		if (reportError)
		{
			const JUtf8Byte* map[] =
		{
				"name", pathName.GetBytes()
		};
			JGetUserNotification()->ReportError(JGetString("Unreadable::Application", map, sizeof(map)));
		}
		return false;
	}

	// resolve all .. in path

	JString p1;

	JStringIterator iter(&trueName);
	iter.BeginMatch();
	while (iter.Next(".."))
	{
		const JString p = iter.FinishMatch(true).GetString();
		if (!JGetTrueName(p, &p1))
		{
			if (reportError)
			{
				const JUtf8Byte* map[] =
			{
					"name", p.GetBytes()
			};
				const JString msg = JGetString("PathDoesNotExist::Application", map, sizeof(map));
				JGetUserNotification()->ReportError(msg);
			}
			return false;
		}

		iter.ReplaceLastMatch(p1);
		iter.MoveTo(JStringIterator::kStartAtBeginning, 0);
		iter.BeginMatch();
	}
	iter.Invalidate();

	JCleanPath(&trueName);

	// check if window is already open

	JString p;

	JString ancestor = trueName, n;
	JPtrArray<JString> pathList(JPtrArrayT::kDeleteAll);
	while (!JIsRootDirectory(ancestor))
	{
		const JIndex count = itsWindowList->GetElementCount();
		for (JIndex i=1; i<=count; i++)
		{
			const JString name = (itsWindowList->GetElement(i))->GetDirectory();
			if (JSameDirEntry(name, ancestor))
			{
				TreeDir* childDir = itsWindowList->GetElement(i);
				childDir->Activate();
				if (dir != nullptr)
				{
					*dir = childDir;
				}

				JPoint cell;
				(childDir->GetTable())->SelectName(pathList, selectName, &cell, clearSelection);
				if (row != nullptr)
				{
					*row = cell.y;
				}

				return true;
			}
		}

		if (forceNew)
		{
			break;
		}

		JStripTrailingDirSeparator(&ancestor);
		JSplitPathAndName(ancestor, &p, &n);
		ancestor = p;
		pathList.Prepend(n);
	}

	// create new window

	fixedName = trueName;
	JGetTrueName(fixedName, &trueName);

	auto* childDir = jnew TreeDir(trueName);

	if (deiconify)
	{
		childDir->GetWindow()->Deiconify();
	}
	childDir->Activate();

	JPoint cell;
	(childDir->GetTable())->SelectName(selectName, nullptr, &cell);
	if (row != nullptr)
	{
		*row = cell.y;
	}

	itsWindowList->Append(childDir);

	if (dir != nullptr)
	{
		*dir = childDir;
	}
	return true;
}

/******************************************************************************
 GetWindowNames

 ******************************************************************************/

void
Application::GetWindowNames
	(
	JPtrArray<JString>* names
	)
{
	const JIndex count = itsWindowList->GetElementCount();
	JString name;
	for (JIndex i=1; i<=count; i++)
	{
		name = JConvertToHomeDirShortcut(
					(itsWindowList->GetElement(i))->GetDirectory());
		names->Append(name);
	}
}

/******************************************************************************
 DirectorClosed (virtual protected)

	We cannot use dynamic_cast because the destructor chain is partially
	finished.

 ******************************************************************************/

void
Application::DirectorClosed
	(
	JXDirector* theDirector
	)
{
	itsWindowList->Remove((TreeDir*) theDirector);
	JXApplication::DirectorClosed(theDirector);
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
Application::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsShortcutList)
	{
		Broadcast(ShortcutsChanged());
	}
	else
	{
		JXApplication::Receive(sender, message);
	}
}

/******************************************************************************
 UpdateMountPointList (private)

 ******************************************************************************/

void
Application::UpdateMountPointList()
	const
{
	if (JGetUserMountPointList(itsMountPointList, &itsMountPointState))
	{
		const_cast<Application*>(this)->Broadcast(ShortcutsChanged());
	}
}

/******************************************************************************
 UpdateShortcutMenu

 ******************************************************************************/

void
Application::UpdateShortcutMenu
	(
	JXTextMenu* menu
	)
	const
{
	UpdateMountPointList();

	JIndex shortcutIndex = 1;

	for (const auto& mp : *itsMountPointList)
	{
		menu->AppendItem(*mp.path, JXMenu::kPlainType, JString::empty,
						 GetNMShortcut(&shortcutIndex), *mp.path);

		JXImage* image;
		if (GetMountPointSmallIcon(mp.type, &image))
		{
			menu->SetItemImage(menu->GetItemCount(), image, false);
		}
	}

	JString trashDir;
	if (GetTrashDirectory(&trashDir, false))
	{
		menu->AppendItem(JGetString("TrashName::globals"), JXMenu::kPlainType, JString::empty,
						 GetNMShortcut(&shortcutIndex), trashDir);
		menu->SetItemImage(menu->GetItemCount(), GetTrashSmallIcon(), false);
	}

	menu->ShowSeparatorAfter(menu->GetItemCount());

	JXImage* folderIcon = GetFolderSmallIcon();

	for (const auto* path : *itsShortcutList)
	{
		menu->AppendItem(*path, JXMenu::kPlainType, JString::empty,
						 GetNMShortcut(&shortcutIndex), *path);
		menu->SetItemImage(menu->GetItemCount(), folderIcon, false);
	}
}

/******************************************************************************
 GetNMShortcut (private)

 ******************************************************************************/

static JString kShortcutPrefix("Ctrl-");

static const JUtf8Byte* kShortcutKey[] =
{
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"
};

const JSize kShortcutKeyCount = sizeof(kShortcutKey) / sizeof(JUtf8Byte*);

JString
Application::GetNMShortcut
	(
	JIndex* i
	)
	const
{
	if (*i <= kShortcutKeyCount)
	{
		JString s = kShortcutPrefix;
		s += kShortcutKey[(*i)++ - 1];
		return s;
	}
	else
	{
		return JString::empty;
	}
}

/******************************************************************************
 LoadToolBarDefaults

 ******************************************************************************/

void
Application::LoadToolBarDefaults
	(
	JXToolBar*		toolBar,
	JXTextMenu*		shorcutMenu,
	const JIndex	firstIndex
	)
{
	const JSize count = itsMountPointList->GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		toolBar->AppendButton(shorcutMenu, firstIndex + i-1);
	}
}

/******************************************************************************
 OpenShortcut

 ******************************************************************************/

void
Application::OpenShortcut
	(
	const JIndex index
	)
{
	const JString* path = nullptr;

	JString trashDir;
	const bool hasTrashDir = GetTrashDirectory(&trashDir, false);

	const JSize mpCount         = itsMountPointList->GetElementCount();
	const JIndex shortcutOffset = mpCount + (hasTrashDir ? 1 : 0);
	if (index <= mpCount)
	{
		path = (itsMountPointList->GetElement(index)).path;
	}
	else if (hasTrashDir && index == mpCount+1)
	{
		path = &trashDir;
	}
	else
	{
		path = itsShortcutList->GetElement(index - shortcutOffset);
	}

	if (!OpenDirectory(*path, nullptr, nullptr, true, false) && index > shortcutOffset)
	{
		const JUtf8Byte* map[] =
	{
			"name", path->GetBytes()
	};
		const JString msg = JGetString("InvalidShortcut::Application", map, sizeof(map));
		if (JGetUserNotification()->AskUserYes(msg))
		{
			itsShortcutList->DeleteElement(index - shortcutOffset);
		}
	}
}

/******************************************************************************
 AddShortcut

 ******************************************************************************/

void
Application::AddShortcut
	(
	const JString& shortcut
	)
{
	auto* s = jnew JString(JConvertToHomeDirShortcut(shortcut));
	assert( s != nullptr );
	if (!itsShortcutList->InsertSorted(s, false))
	{
		jdelete s;
	}
}

/******************************************************************************
 RemoveShortcut

 ******************************************************************************/

void
Application::RemoveShortcut
	(
	const JString& shortcut
	)
{
	JString s = JConvertToHomeDirShortcut(shortcut);
	JIndex i;
	if (itsShortcutList->SearchSorted(&s, JListT::kAnyMatch, &i))
	{
		itsShortcutList->DeleteElement(i);
	}
}

/******************************************************************************
 IsMountPoint

	If returns true and type != nullptr, type is the type of the device.

 ******************************************************************************/

bool
Application::IsMountPoint
	(
	const JString&	path,
	JMountType*		type
	)
	const
{
	if (!JNameUsed(path))
	{
		return false;
	}

	UpdateMountPointList();

	const JSize count = itsMountPointList->GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		const JMountPoint info = itsMountPointList->GetElement(i);
		if (JSameDirEntry(path, *(info.path)))
		{
			if (type != nullptr)
			{
				*type = info.type;
			}
			return true;
		}
	}

	return false;
}

/******************************************************************************
 GetMountPointPrefs

 ******************************************************************************/

bool
Application::GetMountPointPrefs
	(
	const JString&	path,
	const JString**	prefs
	)
	const
{
	JString s = GetMountPointPrefsPath(path);
	return itsMountPointPrefs->GetElement(s, prefs);
}

/******************************************************************************
 SetMountPointPrefs

 ******************************************************************************/

void
Application::SetMountPointPrefs
	(
	const JString&	path,
	const JString&	prefs
	)
{
	JString s = GetMountPointPrefsPath(path);
	itsMountPointPrefs->SetElement(s, prefs, JPtrArrayT::kDelete);
}

/******************************************************************************
 GetMountPointPrefsPath (private)

 ******************************************************************************/

JString
Application::GetMountPointPrefsPath
	(
	const JString& path
	)
	const
{
	JString s = path;
	JCleanPath(&s);
	JStripTrailingDirSeparator(&s);
	return s;
}

/******************************************************************************
 Quit (virtual)

 ******************************************************************************/

void
Application::Quit()
{
	if (!IsQuitting() && HasPrefsMgr())
	{
		SaveProgramState();
	}

	JXApplication::Quit();
}

/******************************************************************************
 RestoreProgramState

 ******************************************************************************/

bool
Application::RestoreProgramState()
{
	JPtrArray<JString> children(JPtrArrayT::kDeleteAll);
	if (!GetPrefsMgr()->RestoreProgramState(&children))
	{
		return false;
	}

	const JSize count = children.GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		const JString* str = children.GetElement(i);
		OpenDirectory(*str, nullptr, nullptr, false, false);
	}

	if (itsWindowList->IsEmpty())
	{
		JString path;
		if (!JGetHomeDirectory(&path) || !JFSFileTreeNode::CanHaveChildren(path))
		{
			path = JGetRootDirectory();
		}
		OpenDirectory(path);
	}

	return true;
}

/******************************************************************************
 SaveProgramState (private)

 ******************************************************************************/

void
Application::SaveProgramState()
{
	JPtrArray<JString> children(JPtrArrayT::kDeleteAll);
	for (auto* w : *itsWindowList)
	{
		children.Append(w->GetDirectory());
	}
	GetPrefsMgr()->SaveProgramState(children);
}

/******************************************************************************
 ReadPrefs (virtual protected)

 ******************************************************************************/

void
Application::ReadPrefs
	(
	std::istream& input
	)
{
	JFileVersion vers;
	input >> vers;
	if (vers > kCurrentPrefsVersion)
	{
		return;
	}

	input >> *itsShortcutList;

	if (vers >= 1)
	{
		input >> *itsMountPointPrefs;
	}

	if (vers >= 2)
	{
		input >> itsTermCmd;
	}

	if (vers >= 3)
	{
		input >> itsPostCheckoutCmd;
	}

	if (vers >= 4)
	{
		input >> itsGitStatusCmd;
	}

	if (vers >= 5)
	{
		input >> itsGitHistoryCmd;
	}
}

/******************************************************************************
 WritePrefs (virtual protected)

 ******************************************************************************/

void
Application::WritePrefs
	(
	std::ostream& output
	)
	const
{
	// flush unused mount point prefs

	JStringPtrMapCursor<JString> cursor(itsMountPointPrefs);
	const JSize mpCount = itsMountPointList->GetElementCount();

	bool changed;
	do
	{
		changed = false;

		cursor.Reset();
		while (cursor.Next())
		{
			bool found = false;

			const JString& key = cursor.GetKey();
			for (JIndex i=1; i<=mpCount; i++)
			{
				const JMountPoint info = itsMountPointList->GetElement(i);
				if (JSameDirEntry(*(info.path), key))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				itsMountPointPrefs->DeleteElement(key);
				changed = true;
				break;
			}
		}
	}
		while (changed);

	// write data

	output << kCurrentPrefsVersion;
	output << ' ' << *itsShortcutList;
	output << ' ' << *itsMountPointPrefs;
	output << ' ' << itsTermCmd;
	output << ' ' << itsPostCheckoutCmd;
	output << ' ' << itsGitStatusCmd;
	output << ' ' << itsGitHistoryCmd;
}

/******************************************************************************
 DisplayAbout

 ******************************************************************************/

void
Application::DisplayAbout
	(
	const bool		showLicense,
	const JString&	prevVersStr
	)
{
	StartFiber([showLicense, prevVersStr]()
	{
		if (!showLicense || JGetUserNotification()->AcceptLicense())
		{
			auto* dlog = jnew AboutDialog(prevVersStr);
			dlog->DoDialog();

			JCheckForNewerVersion(GetPrefsMgr(), kSVersionCheckID);
		}
		else
		{
			ForgetPrefsMgr();
			JXGetApplication()->Quit();
		}
	});
}

/******************************************************************************
 OpenTerminal

 ******************************************************************************/

void
Application::OpenTerminal
	(
	const JString& path
	)
{
	JString cmd = itsTermCmd, fullName = path, p, n;
	JStripTrailingDirSeparator(&fullName);
	JSplitPathAndName(fullName, &p, &n);
	n        = JPrepArgForExec(n);
	fullName = JPrepArgForExec(fullName);

	JStringIterator iter(&cmd);
	while (iter.Next("$n"))
	{
		iter.ReplaceLastMatch(n);
	}

	iter.MoveTo(JStringIterator::kStartAtBeginning, 0);
	while (iter.Next("$p"))
	{
		iter.ReplaceLastMatch(fullName);
	}
	iter.Invalidate();

	const JError err = JSimpleProcess::Create(path, cmd, true);
	err.ReportIfError();
}

/******************************************************************************
 DirectoryRenamed

 ******************************************************************************/

void
Application::DirectoryRenamed
	(
	const Message& message
	)
{
	Broadcast(message);
}

/******************************************************************************
 UpdateTrash

 ******************************************************************************/

void
Application::UpdateTrash()
{
	Broadcast(TrashNeedsUpdate());
}

/******************************************************************************
 CleanUpBeforeSuddenDeath (virtual protected)

 ******************************************************************************/

void
Application::CleanUpBeforeSuddenDeath
	(
	const JXDocumentManager::SafetySaveReason reason
	)
{
	JXApplication::CleanUpBeforeSuddenDeath(reason);

	if (reason != JXDocumentManager::kAssertFired)
	{
		JPrefObject::WritePrefs();
		SaveProgramState();

		const JSize count = itsWindowList->GetElementCount();
		for (JIndex i=1; i<=count; i++)
		{
			(itsWindowList->GetElement(i))->SaveState();
		}
	}

	::CleanUpBeforeSuddenDeath(reason);		// must be last call
}

/******************************************************************************
 GetAppSignature (static)

 ******************************************************************************/

const JUtf8Byte*
Application::GetAppSignature()
{
	return kAppSignature;
}

/******************************************************************************
 InitStrings (static)

	If we are going to print something to the command line and then quit,
	we haven't initialized JX, but we still need the string data.

 ******************************************************************************/

void
Application::InitStrings()
{
	JGetStringManager()->Register(kAppSignature, kDefaultStringData);
}

/******************************************************************************
 CompareShortcuts (static private)

 ******************************************************************************/

std::weak_ordering
Application::CompareShortcuts
	(
	JString* const & s1,
	JString* const & s2
	)
{
	const bool h1 = s1->StartsWith("~");
	const bool h2 = s2->StartsWith("~");
	if (h1 && !h2)
	{
		return std::weak_ordering::less;
	}
	else if (!h1 && h2)
	{
		return std::weak_ordering::greater;
	}
	else
	{
		return JCompareStringsCaseInsensitive(s1, s2);
	}
}
