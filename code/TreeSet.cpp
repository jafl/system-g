/******************************************************************************
 TreeSet.cpp

	Maintains an enclosure for all the objects required by a CBCodeEditor.

	BASE CLASS = JXWidgetSet

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#include "TreeSet.h"

#include "HeaderWidget.h"
#include "FileTreeTable.h"
#include "FileTreeList.h"
#include "FileTree.h"
#include "FileTreeNode.h"
#include "FilterInput.h"
#include "PathInput.h"
#include "PrefsMgr.h"
#include "globals.h"

#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindowDirector.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXMenuBar.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXScrollbarSet.h>
#include <jx-af/jx/JXStringHistoryMenu.h>
#include <jx-af/jx/JXCurrentPathMenu.h>
#include <jx-af/jx/JXCloseDirectorTask.h>
#include <jx-af/jx/JXFontManager.h>

#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

const JCoordinate kEmptyButtonWidth = 50;

/******************************************************************************
 Constructor

 ******************************************************************************/

TreeSet::TreeSet
	(
	JXMenuBar*			menuBar,
	const JString&		pathName,
	PathInput*		pathInput,
	JXCurrentPathMenu*	pathMenu,
	TrashButton*		trashButton,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXWidgetSet(enclosure, hSizing, vSizing, x,y, w,h)
{
	TreeSetX(menuBar, pathName, pathInput, pathMenu, trashButton, w,h);
	itsTable->LoadPrefs();
	ShowFilter((GetPrefsMgr())->GetFilterStatePref());
	itsTable->Focus();
}

TreeSet::TreeSet
	(
	std::istream&		input,
	const JFileVersion	vers,
	JXMenuBar*			menuBar,
	const JString&		pathName,
	PathInput*		pathInput,
	JXCurrentPathMenu*	pathMenu,
	TrashButton*		trashButton,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXWidgetSet(enclosure, hSizing, vSizing, x,y, w,h)
{
	if (vers < 2)
	{
		JFileVersion v;
		input >> v;

		bool showHidden;
		input >> JBoolFromString(showHidden);
	}

	TreeSetX(menuBar, pathName, pathInput, pathMenu, trashButton, w,h);

	JString filter;
	input >> filter;
	itsFilterInput->GetText()->SetText(filter);

	itsFilterHistory->ReadSetup(input);
	itsTable->LoadPrefs(input, vers);

	bool showFilter;
	input >> JBoolFromString(showFilter);
	ShowFilter(showFilter);
	itsTable->Focus();

	itsTable->RestoreDirState(input, vers);

	// must be after directories have been opened

	if (vers >= 3)
	{
		itsTable->ReadScrollSetup(input);
	}
}

void TreeSet::TreeSetX
	(
	JXMenuBar*			menuBar,
	const JString&		pathName,
	PathInput*		pathInput,
	JXCurrentPathMenu*	pathMenu,
	TrashButton*		trashButton,
	const JCoordinate	w,
	const JCoordinate	h
	)
{
	itsMenuBar     = menuBar;
	itsEmptyButton = nullptr;

	const JFont& font              = JFontManager::GetDefaultMonospaceFont();
	const JCoordinate filterHeight = font.GetLineHeight(GetFontManager());
	const JCoordinate headerHeight = HeaderWidget::GetPreferredHeight(GetFontManager());

	// file list -- created first so it gets focus by default

	itsScrollbarSet =
		jnew JXScrollbarSet(this, kHElastic, kVElastic,
							0, filterHeight, w, h - filterHeight);

	JString path = pathName;
	if (!JFSFileTreeNode::CanHaveChildren(path))
	{
		path = JGetRootDirectory();
		JXCloseDirectorTask::Close(GetWindow()->GetDirector());
	}

	auto* entry = jnew JDirEntry(path);
	assert( entry != nullptr && JFSFileTreeNode::CanHaveChildren(*entry) );
	auto* root = jnew FileTreeNode(entry);
	itsFileTree = jnew FileTree(root);
	auto* treeList = jnew FileTreeList(itsFileTree);

	JXContainer* encl = itsScrollbarSet->GetScrollEnclosure();
	JRect enclApG     = encl->GetApertureGlobal();

	itsTable = jnew FileTreeTable(itsMenuBar, itsFileTree, treeList,
									 this, trashButton,
									 itsScrollbarSet, encl, kHElastic, kVElastic,
									 0, headerHeight,
									 enclApG.width(), enclApG.height()-headerHeight);
	assert( itsTable != nullptr );
	ListenTo(itsFileTree->GetRootDirInfo());

	auto* colHeader =
		jnew HeaderWidget(itsTable, itsScrollbarSet, encl,
							 kHElastic, kFixedTop,
							 0,0, enclApG.width(),
							 headerHeight);
	assert(colHeader != nullptr);

	// header:  filter

	itsFilterLabel =
		jnew JXStaticText(JGetString("FilterLabel::TreeSet"), this, kFixedLeft, kFixedTop,
						  5,0, 40, filterHeight);
	assert( itsFilterLabel != nullptr );
	itsFilterLabel->SetToLabel();

	itsFilterHistory =
		jnew JXStringHistoryMenu(10, JString::empty, this, kFixedRight, kFixedTop,
								 0,0, 30, filterHeight);
	ListenTo(itsFilterHistory);
	itsFilterHistory->Place(w - itsFilterHistory->GetFrameWidth(), 0);
	itsFilterHistory->SetDefaultFont(font, true);

	itsFilterInput =
		jnew FilterInput(itsTable, this, kHElastic, kFixedTop,
							45,0, w - 45 - itsFilterHistory->GetFrameWidth(), filterHeight);
	assert( itsFilterInput != nullptr );
	itsFilterInput->SetFont(font);

	// footer:  path input, drag source

	itsPathInput = pathInput;
	itsPathInput->SetDirList(itsTable);
	itsPathInput->GetText()->SetText(path);

	itsPathMenu = pathMenu;

	// after all instance variables initialized

	ListenTo(itsFilterInput);
	ListenTo(itsPathInput);
	ListenTo(itsPathMenu);

	// share Edit menu

	JXTEBase* te = itsTable->GetEditMenuHandler();
	JXTextMenu* menu;
	if (te->GetEditMenu(&menu))
	{
		itsPathInput->ShareEditMenu(te);
		itsFilterInput->ShareEditMenu(te);
	}

	// filter starts out hidden

	ShowFilter(false);
	UpdateDisplay(path);

	FitToEnclosure();	// must be called before ReadScrollSetup()

	SetNeedsInternalFTC();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

TreeSet::~TreeSet()
{
	jdelete itsFileTree;
}

/******************************************************************************
 Receive (protected)

 ******************************************************************************/

void
TreeSet::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsPathInput && message.Is(JXWidget::kLostFocus))
	{
		GoToItsPath();
	}
	else if (sender == itsPathMenu && message.Is(JXMenu::kItemSelected))
	{
		itsTable->GoTo(itsPathMenu->GetPath(message),
					   GetDisplay()->GetLatestKeyModifiers().meta());
	}

	else if (sender == itsFilterInput && message.Is(JXWidget::kLostFocus))
	{
		SetWildcardFilter(itsFilterInput->GetText()->GetText());
	}
	else if (sender == itsFilterHistory && message.Is(JXMenu::kItemSelected))
	{
		SetWildcardFilter(itsFilterHistory->GetItemText(message));
	}

	else if (sender == itsEmptyButton && message.Is(JXButton::kPushed))
	{
		EmptyTrashDirectory();
	}

	else if (sender == itsFileTree->GetRootDirInfo() &&
			 message.Is(JDirInfo::kPathChanged))
	{
		UpdateDisplay(itsFileTree->GetDirectory());
	}

	else
	{
		JXWidgetSet::Receive(sender, message);
	}
}

/******************************************************************************
 UpdateDisplay

 ******************************************************************************/

void
TreeSet::UpdateDisplay
	(
	const JString& path
	)
{
	// window title

	JString s = path, name;
	JStripTrailingDirSeparator(&s);

	const bool isTrashDir = IsTrashDirectory(s);

	if (JIsRootDirectory(s))
	{
		name = s;
	}
	else if (isTrashDir)
	{
		name = JGetString("TrashName::globals");
	}
	else
	{
		JString p;
		JSplitPathAndName(s, &p, &name);
	}
	GetWindow()->SetTitle(name);

	// path input

	const JString p = JConvertToHomeDirShortcut(path);
	itsPathInput->GetText()->SetText(p);

	// Empty button (for trash)

	JXContainer* encl   = itsMenuBar->GetEnclosure();
	const JCoordinate w = encl->GetBoundsWidth();
	if (isTrashDir && itsEmptyButton == nullptr)
	{
		itsMenuBar->SetSize(w - kEmptyButtonWidth, kJXDefaultMenuBarHeight);

		itsEmptyButton =
			jnew JXTextButton(JGetString("EmptyTrashLabel::TreeSet"), encl, kFixedRight, kFixedTop,
							 w - kEmptyButtonWidth, 0,
							 kEmptyButtonWidth, kJXDefaultMenuBarHeight);
		assert( itsEmptyButton != nullptr );
		ListenTo(itsEmptyButton);
	}
	else if (!isTrashDir && itsEmptyButton != nullptr)
	{
		itsMenuBar->SetSize(w, kJXDefaultMenuBarHeight);

		jdelete itsEmptyButton;
		itsEmptyButton = nullptr;
	}
}

/******************************************************************************
 GoToItsPath

 ******************************************************************************/

void
TreeSet::GoToItsPath()
{
	JString path;
	if (itsPathInput->GetPath(&path) &&
		!JSameDirEntry(path, itsFileTree->GetDirectory()))
	{
		const JError err = itsFileTree->GetFileRoot()->GoTo(path);
		err.ReportIfError();
	}
}

/******************************************************************************
 SetWildcardFilter

 ******************************************************************************/

void
TreeSet::SetWildcardFilter
	(
	const JString& filter
	)
{
	const JString f = filter;		// use copy in case filter is reference from input or menu
	itsFilterInput->GetText()->SetText(f);
	itsFilterHistory->AddString(f);
	itsFileTree->SetWildcardFilter(f);
}

/******************************************************************************
 SavePreferences

 ******************************************************************************/

void
TreeSet::SavePreferences
	(
	std::ostream& os
	)
{
	os << ' ' << itsFilterInput->GetText()->GetText();

	os << ' ';
	itsFilterHistory->WriteSetup(os);

	os << ' ';
	itsTable->SavePrefs(os);

	os << ' ' << JBoolToString(itsFilterInput->IsVisible());

	os << ' ';
	itsTable->SaveDirState(os);

	os << ' ';
	itsTable->WriteScrollSetup(os);

	os << ' ';
}

/******************************************************************************
 FilterVisible

 ******************************************************************************/

bool
TreeSet::FilterVisible()
	const
{
	return itsFilterInput->IsVisible();
}

/******************************************************************************
 ShowFilter

 ******************************************************************************/

void
TreeSet::ShowFilter
	(
	const bool show
	)
{
	const JCoordinate filterHeight = itsFilterInput->GetFrameHeight();

	if (show && !itsFilterInput->WouldBeVisible())
	{
		itsFilterInput->Show();
		itsFilterInput->Focus();
		itsFilterLabel->Show();
		itsFilterHistory->Show();
		itsScrollbarSet->Place(0, filterHeight);
		itsScrollbarSet->AdjustSize(0, -filterHeight);
		SetWildcardFilter(itsFilterInput->GetText()->GetText());
	}
	else if (!show && itsFilterInput->WouldBeVisible())
	{
		if (itsFilterInput->HasFocus())
		{
			itsTable->Focus();
		}

		itsFilterInput->Hide();
		itsFilterLabel->Hide();
		itsFilterHistory->Hide();
		itsScrollbarSet->Place(0, 0);
		itsScrollbarSet->AdjustSize(0,filterHeight);
		itsFileTree->ClearWildcardFilter();
	}
}

/******************************************************************************
 ToggleFilter

 ******************************************************************************/

void
TreeSet::ToggleFilter()
{
	ShowFilter(!itsFilterInput->IsVisible());
}

/******************************************************************************
 SaveFilterPref

 ******************************************************************************/

void
TreeSet::SaveFilterPref()
{
	GetPrefsMgr()->SaveFilterStatePref(itsFilterInput->IsVisible());
}
