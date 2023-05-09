/******************************************************************************
 TreeDir.cpp

	BASE CLASS = JXWindowDirector

	Copyright (C) 1997 by Glenn Bach.

 ******************************************************************************/

#include "TreeDir.h"
#include "TreeSet.h"
#include "FileTreeTable.h"
#include "FileTree.h"
#include "FolderDragSource.h"
#include "PathInput.h"
#include "TrashButton.h"
#include "PrefsMgr.h"
#include "globals.h"
#include "FileVersions.h"
#include "actionDefs.h"
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXMenuBar.h>
#include <jx-af/jx/JXToolBar.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXWDManager.h>
#include <jx-af/jx/JXWDMenu.h>
#include <jx-af/jx/JXCurrentPathMenu.h>
#include <jx-af/jx/JXHelpManager.h>
#include <jx-af/jx/JXMacWinPrefsDialog.h>
#include <jx-af/jcore/jMountUtil.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jSysUtil.h>
#include <jx-af/jcore/jFStreamUtil.h>
#include <sstream>
#include <jx-af/jcore/jAssert.h>

#include <jx-af/image/jx/jx_help_toc.xpm>

const JString kDirPrefsName    (".systemg_folder_prefs_", JString::kNoCopy);
const JString kOrigDirPrefsName(".systemG.Desktop", JString::kNoCopy);

// Preferences menu

static const JUtf8Byte* kPrefsMenuStr =
	"    Preferences..."
	"  | File bindings..."
	"  | Toolbar buttons..."
	"  | Mac/Win/X emulation..."
	"%l| Save window setup as default";

enum
{
	kEditPrefsCmd = 1,
	kEditBindings,
	kEditToolBarCmd,
	kEditMacWinPrefsCmd,
	kSaveWindSizeCmd
};

// Help menu

static const JUtf8Byte* kHelpMenuStr =
	"    About"
	"%l| Table of Contents %k F1 %i" kJXHelpTOCAction
	"  | Getting started"
	"%l| Changes"
	"  | Credits";

enum
{
	kAboutCmd = 1,
	kTOCCmd,
	kOverviewCmd,
	kChangesCmd,
	kCreditsCmd
};

/******************************************************************************
 Constructor

 ******************************************************************************/

TreeDir::TreeDir
	(
	const JString& startPath
	)
	:
	JXWindowDirector(JXGetApplication()),
	itsAutoMountFlag(false)
{
	if ((GetApplication())->IsMountPoint(startPath) &&
		!JIsMounted(startPath))
	{
		itsAutoMountFlag = true;
		(GetApplication())->DisplayBusyCursor();
		JMount(startPath, true, true);
	}

	BuildWindow(startPath);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

TreeDir::~TreeDir()
{
	if (itsAutoMountFlag)
	{
		(GetApplication())->DisplayBusyCursor();		// can take time to flush buffers
		JMount(GetDirectory(), false);
	}
}

/******************************************************************************
 Close (virtual)

 ******************************************************************************/

bool
TreeDir::Close()
{
	if (HasPrefsMgr())
	{
		SaveState();
	}
	return JXWindowDirector::Close();
}

/******************************************************************************
 GetTable

 ******************************************************************************/

FileTreeTable*
TreeDir::GetTable()
	const
{
	return itsTreeSet->GetTable();
}

/******************************************************************************
 GetName

 ******************************************************************************/

const JString&
TreeDir::GetName()
	const
{
	if (IsTrashDirectory(GetDirectory()))
	{
		return JGetString("TrashName::globals");
	}
	else
	{
		return itsPathInput->GetText()->GetText();
	}
}

/******************************************************************************
 GetMenuIcon (virtual)

 ******************************************************************************/

bool
TreeDir::GetMenuIcon
	(
	const JXImage** icon
	)
	const
{
	*icon = GetDirectorySmallIcon(GetDirectory());
	return true;
}

/******************************************************************************
 GetDirectory

 ******************************************************************************/

const JString&
TreeDir::GetDirectory()
	const
{
	return GetTable()->GetFileTree()->GetDirectory();
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

void
TreeDir::BuildWindow
	(
	const JString& startPath
	)
{
	JXCurrentPathMenu* pathMenu = nullptr;

// begin JXLayout

	auto* window = jnew JXWindow(this, 420,500, JString::empty);
	assert( window != nullptr );

	auto* menuBar =
		jnew JXMenuBar(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 420,30);
	assert( menuBar != nullptr );

	itsToolBar =
		jnew JXToolBar(GetPrefsMgr(), kSMainToolBarID, menuBar, window,
					JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 420,450);
	assert( itsToolBar != nullptr );

	itsPathInput =
		jnew PathInput(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 20,480, 340,20);
	assert( itsPathInput != nullptr );

	auto* trashButton =
		jnew TrashButton(window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 390,480, 30,20);
	assert( trashButton != nullptr );

	itsDragSrc =
		jnew FolderDragSource(itsPathInput, &pathMenu, window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 0,480, 20,20);
	assert( itsDragSrc != nullptr );

	itsUpButton =
		jnew JXTextButton(JGetString("itsUpButton::TreeDir::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 360,480, 30,20);
	assert( itsUpButton != nullptr );

// end JXLayout

	// folder setup

	JString prefsFile = JCombinePathAndName(startPath, kDirPrefsName);
	prefsFile += JGetUserName();
	if (!JFileExists(prefsFile))
	{
		const JString origPrefsFile = JCombinePathAndName(startPath, kOrigDirPrefsName);
		JRenameFile(origPrefsFile, prefsFile);
	}

	std::istream* input  = nullptr;
	const JString* prefs = nullptr;
	std::string s;
	if (!JFileReadable(prefsFile) &&
		(GetApplication())->GetMountPointPrefs(startPath, &prefs))
	{
		s.assign(prefs->GetBytes(), prefs->GetByteCount());
		input = jnew std::istringstream(s);
	}
	else
	{
		input = jnew std::ifstream(prefsFile.GetBytes());
	}
	assert( input != nullptr );

	JFileVersion vers = 0;
	JSize w, h;
	if (input->good())
	{
		*input >> vers;

		if (vers <= kCurrentDirSetupVersion)
		{
			window->ReadGeometry(*input);
		}
		else
		{
			jdelete input;
			input = nullptr;
		}
	}
	else if ((GetPrefsMgr())->GetDefaultWindowSize(&w, &h))
	{
		window->SetSize(w,h);

		jdelete input;
		input = nullptr;
	}
	window->SetCloseAction(JXWindow::kCloseDirector);
	window->SetWMClass(GetWMClassInstance(), GetFolderWindowClass());
	window->ShouldFocusWhenShow(true);
	window->SetMinSize(150, 150);

	// Up button

	ListenTo(itsUpButton, std::function([this](const JXButton::Pushed&)
	{
		GetTable()->GoUp(GetDisplay()->GetLatestKeyModifiers().meta());
	}));

	// trash button

	trashButton->SetHint(JGetString("TrashButtonHint::TreeDir"));

	// widgets

	if (input != nullptr && input->good() && vers <= kCurrentDirSetupVersion)
	{
		if (vers < 2)
		{
			JFileVersion v;
			*input >> v;
		}

		itsTreeSet =
			jnew TreeSet(*input, vers, menuBar, startPath,
							itsPathInput, pathMenu, trashButton,
							itsToolBar->GetWidgetEnclosure(),
							JXWidget::kHElastic, JXWidget::kVElastic,
							0,0, 1000,1000);
	}
	else
	{
		itsTreeSet =
			jnew TreeSet(menuBar, startPath, itsPathInput, pathMenu,
							trashButton, itsToolBar->GetWidgetEnclosure(),
							JXWidget::kHElastic, JXWidget::kVElastic,
							0,0, 1000,1000);
	}
	assert( itsTreeSet != nullptr );
	// itsTreeSet has already called FitToEnclosure()
	ListenTo(GetTable());

	// menus

	auto* windowsMenu =
		jnew JXWDMenu(JGetString("WindowsMenuTitle::JXGlobal"), menuBar,
					 JXWidget::kFixedLeft, JXWidget::kVElastic, 0,0, 10,10);
	assert( windowsMenu != nullptr );
	menuBar->AppendMenu(windowsMenu);

	itsPrefsMenu = menuBar->AppendTextMenu(JGetString("PrefsMenuTitle::JXGlobal"));
	itsPrefsMenu->SetMenuItems(kPrefsMenuStr, "TreeDir");
	itsPrefsMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsPrefsMenu->AttachHandler(this, &TreeDir::HandlePrefsMenu);

	itsHelpMenu = menuBar->AppendTextMenu(JGetString("HelpMenuTitle::JXGlobal"));
	itsHelpMenu->SetMenuItems(kHelpMenuStr, "TreeDir");
	itsHelpMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsHelpMenu->AttachHandler(this, &TreeDir::HandleHelpMenu);

	itsHelpMenu->SetItemImage(kTOCCmd, jx_help_toc);

	itsToolBar->LoadPrefs();
	if (itsToolBar->IsEmpty())
	{
		GetTable()->LoadToolBarDefaults(itsToolBar);
	}

	// clean up

	jdelete input;

	GetDisplay()->GetWDManager()->DirectorCreated(this);
}

/******************************************************************************
 SaveState

 ******************************************************************************/

void
TreeDir::SaveState()
{
	if (!GetPrefsMgr()->WillSaveFolderPrefs())
	{
		return;
	}

	const JString& path = GetDirectory();
	if (GetApplication()->IsMountPoint(path))
	{
		std::ostringstream data;
		WriteState(data);
		const std::string s = data.str();
		GetApplication()->SetMountPointPrefs(path, JString(s));
	}

	JString prefsFile = JCombinePathAndName(path, kDirPrefsName);
	prefsFile += JGetUserName();

	// don't overwrite newer version of prefs

	std::ifstream input(prefsFile.GetBytes());
	if (input.good())
	{
		JFileVersion vers;
		input >> vers;
		if (vers > kCurrentDirSetupVersion)
		{
			return;
		}
	}
	input.close();

	std::ofstream output(prefsFile.GetBytes());
	WriteState(output);
}

/******************************************************************************
 WriteState (private)

 ******************************************************************************/

void
TreeDir::WriteState
	(
	std::ostream& output
	)
{
	output << kCurrentDirSetupVersion;

	output << ' ';
	GetWindow()->WriteGeometry(output);

	output << ' ';
	itsTreeSet->SavePreferences(output);
}

/******************************************************************************
 HandleHelpMenu

 ******************************************************************************/

void
TreeDir::HandlePrefsMenu
	(
	const JIndex index
	)
{
	if (index == kEditPrefsCmd)
	{
		GetPrefsMgr()->EditPrefs();
	}
	else if (index == kEditBindings)
	{
		JXFSBindingManager::EditBindings();
	}
	else if (index == kEditToolBarCmd)
	{
		itsToolBar->Edit();
	}
	else if (index == kEditMacWinPrefsCmd)
	{
		JXMacWinPrefsDialog::EditPrefs();
	}

	else if (index == kSaveWindSizeCmd)
	{
		GetTable()->SavePrefsAsDefault();
	}
}

/******************************************************************************
 HandleHelpMenu

 ******************************************************************************/

void
TreeDir::HandleHelpMenu
	(
	const JIndex index
	)
{
	if (index == kAboutCmd)
	{
		GetApplication()->DisplayAbout();
	}
	else if (index == kTOCCmd)
	{
		JXGetHelpManager()->ShowTOC();
	}
	else if (index == kOverviewCmd)
	{
		JXGetHelpManager()->ShowSection("GettingStartedHelp");
	}
	else if (index == kChangesCmd)
	{
		JXGetHelpManager()->ShowChangeLog();
	}
	else if (index == kCreditsCmd)
	{
		JXGetHelpManager()->ShowCredits();
	}
}
