/******************************************************************************
 FileTreeTable.cpp

	BASE CLASS = JXNamedTreeListWidget

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "FileTreeTable.h"
#include "FileTreeList.h"
#include "FileTree.h"
#include "TreeDir.h"
#include "TreeSet.h"
#include "TrashButton.h"
#include "PrefsMgr.h"
#include "ChoosePathDialog.h"
#include "ViewManPageDialog.h"
#include "FindFileDialog.h"
#include "NewGitRemoteDialog.h"
#include "BeginEditingTask.h"
#include "CopyProcess.h"
#include "DuplicateProcess.h"
#include "globals.h"

#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jfs/JXFSDirMenu.h>
#include <jx-af/jfs/JXFSRunCommandDialog.h>

#include <jx-af/jx/JXApplication.h>
#include <jx-af/jx/JXWindowDirector.h>
#include <jx-af/jx/JXCloseDirectorTask.h>
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXWindowIcon.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/JXDragPainter.h>
#include <jx-af/jx/JXDNDManager.h>
#include <jx-af/jx/JXSelectionManager.h>
#include <jx-af/jx/JXFileSelection.h>
#include <jx-af/jx/JXTextSelection.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXMenuBar.h>
#include <jx-af/jx/JXSaveFileInput.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jfs/JXFSDirMenu.h>
#include <jx-af/jx/JXRadioGroupDialog.h>
#include <jx-af/jx/JXCheckboxListDialog.h>
#include <jx-af/jx/JXGetStringDialog.h>
#include <jx-af/jx/JXFunctionTask.h>
#include <jx-af/jx/JXToolBar.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jx/jXUtil.h>
#include <X11/keysym.h>

#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/JRegex.h>
#include <jx-af/jcore/JSubstitute.h>
#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jMountUtil.h>
#include <jx-af/jcore/jStreamUtil.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jMouseUtil.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jTime.h>
#include <jx-af/jcore/jASCIIConstants.h>
#include <jx-af/jcore/jAssert.h>

const Time kDirUpdateInterval = 10;		// milliseconds
const JUtf8Character kPermTestChar('w');

static const JUtf8Byte* kDNDClassID = "FileTreeTable";

#include "FileTreeTable-File.h"
#include "FileTreeTable-Edit.h"
#include "FileTreeTable-View.h"
#include "FileTreeTable-Git.h"
#include "FileTreeTable-Shortcuts.h"

static const JString kStashDisplaySuffix(": systemg-temp");

// Shortcuts menu

const JIndex kShortcutCmdOffset = 3;

// Format disk

static const JString kFormatName[] =
{
	JString("Linux", JString::kNoCopy),
	JString("DOS", JString::kNoCopy)
};

static const JString kFormatType[] =
{
	JString("ext2", JString::kNoCopy),
	JString("msdos", JString::kNoCopy)
};

/******************************************************************************
 Constructor

 ******************************************************************************/

FileTreeTable::FileTreeTable
	(
	JXMenuBar*			menuBar,
	FileTree*		fileTree,
	FileTreeList*	treeList,
	TreeSet*			treeSet,
	TrashButton*		trashButton,
	JXScrollbarSet*		scrollbarSet,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXNamedTreeListWidget(treeList, scrollbarSet, enclosure,
						  hSizing,vSizing, x,y, w,h)
{
	itsFileTree					= fileTree;
	itsFileTreeList				= treeList;
	itsTreeSet					= treeSet;
	itsTrashButton				= trashButton;
	itsMenuBar					= menuBar;
	itsContextMenu				= nullptr;
	itsUpdateTask				= nullptr;
	itsEditTask					= nullptr;
	itsUpdateNode				= nullptr;
	itsSortNode					= nullptr;
	itsFormatProcess			= nullptr;
	itsGitProcess				= nullptr;
	itsIconWidget				= nullptr;
	itsWindowIconType			= 0;

	for (JUnsignedOffset i=0; i<5; i++)
	{
		itsVisibleCols[i] = false;
	}
	itsCurrentColType = kGFMName;

	ShouldHilightTextOnly(true);
	WantInput(true);

	itsAltRowColor = JColorManager::GetGrayColor(95);

	itsPermCharWidth =
		JFontManager::GetDefaultFont().GetCharWidth(GetFontManager(), kPermTestChar);

	// menus

	itsFileMenu = menuBar->PrependTextMenu(JGetString("MenuTitle::FileTreeTable_File"));
	itsFileMenu->SetMenuItems(kFileMenuStr);
	itsFileMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsFileMenu->AttachHandlers(this,
		&FileTreeTable::UpdateFileMenu,
		&FileTreeTable::HandleFileMenu);
	ConfigureFileMenu(itsFileMenu);

	JXKeyModifiers homeModifiers(GetDisplay());
	homeModifiers.SetState(kJXMetaKeyIndex, true);
	homeModifiers.SetState(kJXShiftKeyIndex, true);
	GetWindow()->InstallMenuShortcut(itsFileMenu, kHomeWindowCmd, 'h', homeModifiers);

	JString recentDir;
	if (GetRecentFileDirectory(&recentDir))
	{
		itsRecentFilesMenu =
			jnew JXFSDirMenu(recentDir, itsFileMenu, kOpenRecentItemIndex, menuBar);
		itsRecentFilesMenu->ShouldShowPath(true);
		itsRecentFilesMenu->ShouldDereferenceLinks(true);
		itsRecentFilesMenu->ShouldDeleteBrokenLinks(true);
		ListenTo(itsRecentFilesMenu);

		JDirInfo* info;
		if (itsRecentFilesMenu->GetDirInfo(&info))
		{
			info->ChangeSort(JDirEntry::CompareModTimes, JListT::kSortDescending);
		}
	}
	else
	{
		itsFileMenu->DisableItem(kOpenRecentItemIndex);
	}

	JXTEBase* te         = GetEditMenuHandler();
	itsEditMenu          = te->AppendEditMenu(menuBar);
	const bool found = te->EditMenuCmdToIndex(JTextEditor::kCopyCmd, &itsCopyPathCmdIndex);
	assert( found );
	itsCopyPathCmdIndex++;
	itsEditMenu->InsertMenuItems(itsCopyPathCmdIndex, kEditMenuStr);
	itsEditMenu->AttachHandlers(this,
		&FileTreeTable::UpdateEditMenu,
		&FileTreeTable::HandleEditMenu);
	ConfigureEditMenu(itsEditMenu, itsCopyPathCmdIndex-1);

	itsViewMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::FileTreeTable_View"));
	itsViewMenu->SetMenuItems(kViewMenuStr);
	itsViewMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsViewMenu->AttachHandlers(this,
		&FileTreeTable::UpdateViewMenu,
		&FileTreeTable::HandleViewMenu);
	ConfigureViewMenu(itsViewMenu);

	itsGitMenu = itsMenuBar->AppendTextMenu(JGetString("MenuTitle::FileTreeTable_Git"));
	itsGitMenu->SetMenuItems(kGitMenuStr);
	itsGitMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsGitMenu->Deactivate();
	ConfigureGitMenu(itsGitMenu);

	ListenTo(itsGitMenu, std::function([this](const JXMenu::NeedsUpdate& msg)
	{
		UpdateGitMenus(msg.IsFromShortcut());
	}));

	ListenTo(itsGitMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		HandleGitMenu(msg.GetIndex());
	}));

	itsGitLocalBranchMenu =
		jnew JXTextMenu(itsGitMenu, kGitSwitchBranchItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitLocalBranchMenu != nullptr );
	itsGitLocalBranchMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitLocalBranchMenu, std::function([this](const JXMenu::NeedsUpdate&)
	{
		UpdateGitLocalBranchMenu();
	}));

	ListenTo(itsGitLocalBranchMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		SwitchToGitBranch(itsGitLocalBranchMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitPullSourceMenu =
		jnew JXTextMenu(itsGitMenu, kGitPullItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitPullSourceMenu != nullptr );
	itsGitPullSourceMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitPullSourceMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		PullBranch(itsGitPullSourceMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitPushDestMenu =
		jnew JXTextMenu(itsGitMenu, kGitPushItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitPushDestMenu != nullptr );
	itsGitPushDestMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitPushDestMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		PushBranch(itsGitPushDestMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitMergeBranchMenu =
		jnew JXTextMenu(itsGitMenu, kGitMergeFromBranchItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitMergeBranchMenu != nullptr );
	itsGitMergeBranchMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitMergeBranchMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		MergeFromGitBranch(itsGitMergeBranchMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitStashPopMenu =
		jnew JXTextMenu(itsGitMenu, kGitStashPopItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitStashPopMenu != nullptr );
	itsGitStashPopMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitStashPopMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		Unstash("pop", itsGitStashPopMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitStashApplyMenu =
		jnew JXTextMenu(itsGitMenu, kGitStashApplyItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitStashApplyMenu != nullptr );
	itsGitStashApplyMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitStashApplyMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		Unstash("apply", itsGitStashApplyMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitStashDropMenu =
		jnew JXTextMenu(itsGitMenu, kGitStashDropItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitStashDropMenu != nullptr );
	itsGitStashDropMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitStashDropMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		Unstash("drop", itsGitStashDropMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitRemoteBranchMenu =
		jnew JXTextMenu(itsGitMenu, kGitFetchBranchItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitRemoteBranchMenu != nullptr );
	itsGitRemoteBranchMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitRemoteBranchMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		FetchRemoteGitBranch(itsGitRemoteBranchMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitRemoveBranchMenu =
		jnew JXTextMenu(itsGitMenu, kGitRemoveBranchItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitRemoveBranchMenu != nullptr );
	itsGitRemoveBranchMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitRemoveBranchMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		RemoveGitBranch(itsGitRemoveBranchMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitRemoveRemoteMenu =
		jnew JXTextMenu(itsGitMenu, kGitRemoveRemoteItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitRemoveRemoteMenu != nullptr );
	itsGitRemoveRemoteMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitRemoveRemoteMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		RemoveGitRemote(itsGitRemoveRemoteMenu->GetItemText(msg.GetIndex()));
	}));

	itsGitPruneRemoteMenu =
		jnew JXTextMenu(itsGitMenu, kGitPruneRemoteItemIndex,
					   itsGitMenu->GetEnclosure());
	assert( itsGitPruneRemoteMenu != nullptr );
	itsGitPruneRemoteMenu->SetUpdateAction(JXMenu::kDisableNone);

	ListenTo(itsGitPruneRemoteMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		PruneRemoteGitBranches(itsGitPruneRemoteMenu->GetItemText(msg.GetIndex()));
	}));

	itsShortcutsMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::FileTreeTable_Shortcuts"));
	assert (itsShortcutsMenu != nullptr);
	itsShortcutsMenu->SetMenuItems(kShortcutsMenuStr);
	itsShortcutsMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsShortcutsMenu->AttachHandlers(this,
		&FileTreeTable::UpdateShortcutMenu,
		&FileTreeTable::HandleShortcutMenu);
	ConfigureShortcutsMenu(itsShortcutsMenu);

	ListenTo(GetApplication());

	UpdateShortcutMenu();

	// updating

	itsUpdateTask = jnew JXFunctionTask(kDirUpdateInterval, std::bind(&FileTreeTable::UpdateDisplay, this, false));
	assert( itsUpdateTask != nullptr );

	JXWindow* window = GetWindow();
//	if (!window->IsIconified())		// update window icon while iconified
//	{
		itsUpdateTask->Start();
//	}
	ListenTo(window);

	ListenTo(itsFileTree);
	ListenTo(itsFileTree->GetRootDirInfo());

	// shortcuts

	JXKeyModifiers modifiers(GetDisplay());
	modifiers.SetState(kJXMetaKeyIndex, true);
	window->InstallShortcut(this, kJUpArrow, modifiers);
	window->InstallShortcut(this, kJDownArrow, modifiers);

	modifiers.SetState(kJXShiftKeyIndex, true);
	window->InstallShortcut(this, kJUpArrow, modifiers);
	window->InstallShortcut(this, kJDownArrow, modifiers);

	// show file size by default

	TogglePref(4);
	UpdateInfo();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FileTreeTable::~FileTreeTable()
{
	SetDNDSource(nullptr);		// avoid dangling pointers
	SetDNDTarget(nullptr);

	jdelete itsEditTask;
	jdelete itsUpdateTask;
}

/******************************************************************************
 LoadToolBarDefaults

 ******************************************************************************/

void
FileTreeTable::LoadToolBarDefaults
	(
	JXToolBar* toolBar
	)
{
	toolBar->AppendButton(itsFileMenu, kNewDirCmd);
	toolBar->AppendButton(itsFileMenu, kOpenCmd);
	toolBar->NewGroup();
	toolBar->AppendButton(itsFileMenu, kFindCmd);
	toolBar->NewGroup();
	toolBar->AppendButton(itsFileMenu, kHomeWindowCmd);
	toolBar->NewGroup();
	toolBar->AppendButton(itsFileMenu, kViewManCmd);
	toolBar->AppendButton(itsFileMenu, kOpenTermCmd);
	toolBar->NewGroup();
	toolBar->AppendButton(itsViewMenu, kShowFilterCmd);
	toolBar->AppendButton(itsViewMenu, kShowHiddenCmd);
	toolBar->NewGroup();
	toolBar->AppendButton(itsViewMenu, kShowUserCmd);
	toolBar->AppendButton(itsViewMenu, kShowGroupCmd);
	toolBar->AppendButton(itsViewMenu, kShowSizeCmd);
	toolBar->AppendButton(itsViewMenu, kShowModeCmd);
	toolBar->AppendButton(itsViewMenu, kShowDateCmd);
	toolBar->NewGroup();
	GetApplication()->LoadToolBarDefaults(toolBar, itsShortcutsMenu,
										  kShortcutCmdOffset+1);
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
FileTreeTable::TableDrawCell
	(
	JPainter&		p,
	const JPoint&	cell,
	const JRect&	rect
	)
{
	if (cell.y % 2 == 0)
	{
		const JColorID origColor = p.GetPenColor();
		p.SetPenColor(itsAltRowColor);
		p.SetFilling(true);
		p.Rect(rect);
		p.SetFilling(false);
		p.SetPenColor(origColor);
	}

	if (JIndex(cell.x) == GetToggleOpenColIndex() ||
		JIndex(cell.x) == GetNodeColIndex())
	{
		JXNamedTreeListWidget::TableDrawCell(p,cell,rect);
		return;
	}

	const GFMColType type = GetFMColType(cell.x, GetNodeColIndex(), itsVisibleCols);
	const JString str     = GetCellString(cell);
	if (type == kGFMMode && !str.IsEmpty())
	{
		p.SetFont(GetFont());

		JRect pRect = rect;
		pRect.right = pRect.left + itsPermCharWidth;

		assert( str.GetCharacterCount() == 9 );

		JStringIterator iter(str);
		JUtf8Character c;
		while (iter.Next(&c))
		{
			p.String(pRect, JString(c), JPainter::HAlign::kCenter, JPainter::VAlign::kCenter);
			pRect.Shift(itsPermCharWidth, 0);
		}
	}
	else if (!str.IsEmpty())
	{
		p.SetFont(GetFont());

		JRect sRect = rect;
		const JCoordinate theBuffer = GetFMBufferWidth(itsVisibleCols, GetNodeColIndex(), cell.x);
		if (theBuffer > 0)
		{
			sRect.left += theBuffer;
		}
		else
		{
			sRect.right += theBuffer;
		}

		p.String(sRect, str,
				 type == kGFMSize ? JPainter::HAlign::kRight : JPainter::HAlign::kLeft,
				 JPainter::VAlign::kCenter);
	}
}

/******************************************************************************
 GetImage (virtual protected)

 ******************************************************************************/

bool
FileTreeTable::GetImage
	(
	const JIndex	index,
	const JXImage** image
	)
	const
{
	bool selected = GetTableSelection().IsSelected(index, GetNodeColIndex());

	JIndex dndIndex;
	if (GetDNDTargetIndex(&dndIndex) && dndIndex == index)
	{
		selected = true;
	}

	const FileTreeNode* node = itsFileTreeList->GetFileNode(index);
	const JDirEntry* entry   = node->GetDirEntry();
	if (node->IsOpenable() && entry->IsWritable())
	{
		*image = GetFolderSmallIcon(selected);
	}
	else if (node->IsOpenable())
	{
		*image = GetReadOnlyFolderSmallIcon(selected);
	}
	else if (entry->IsDirectory())
	{
		*image = GetLockedFolderSmallIcon(selected);
	}
	else if (!itsIgnoreExecPermFlag && entry->IsExecutable() && entry->IsFile())
	{
		*image = GetExecSmallIcon(selected);
	}
	else if (entry->IsFile() || entry->IsBrokenLink())
	{
		*image = GetFileSmallIcon(selected);
	}
	else
	{
		*image = GetUnknownSmallIcon(selected);
	}
	return true;
}

/******************************************************************************
 GetCellString (private)

 ******************************************************************************/

JString
FileTreeTable::GetCellString
	(
	const JPoint& cell
	)
	const
{
	const FileTreeNode* entry = itsFileTreeList->GetFileNode(cell.y);
	const JDirEntry* dirEntry  = entry->GetDirEntry();

	const GFMColType type = GetFMColType(cell.x, GetNodeColIndex(), itsVisibleCols);

	if (type == kGFMUID)
	{
		return dirEntry->GetUserName();
	}
	else if (type == kGFMGID)
	{
		return dirEntry->GetGroupName();
	}
	else if (type == kGFMSize && !dirEntry->IsLink())
	{
		if (dirEntry->IsDirectory())
		{
			return JString("-", JString::kNoCopy);
		}
		else
		{
			return JPrintFileSize(dirEntry->GetSize());
		}
	}
	else if (type == kGFMMode && !dirEntry->IsLink())
	{
		mode_t mode = dirEntry->GetMode();
		if (itsIgnoreExecPermFlag)
		{
			mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
		}
		return JGetPermissionsString(mode);
	}
	else if (type == kGFMDate)
	{
		return JConvertToTimeStamp(dirEntry->GetModTime());
	}
	else
	{
		return JString();
	}
}

/******************************************************************************
 SelectName

	This function forces an update of the list before searching for the
	given name.

	If parent == nullptr, the root node is assumed.

 ******************************************************************************/

bool
FileTreeTable::SelectName
	(
	const JString&			name,
	const FileTreeNode*	parent,
	JPoint*					cell,
	const bool			updateContent,
	const bool			updateView
	)
{
	if (!name.IsEmpty())
	{
		if (updateContent)
		{
			itsFileTree->Update(true);
		}

		if (parent == nullptr)
		{
			parent = itsFileTree->GetFileRoot();
		}

		const JNamedTreeNode* node;
		JIndex index;
		if (parent->FindNamedChild(name, &node) &&
			itsFileTreeList->FindNode(node, &index))
		{
			cell->Set(GetNodeColIndex(), index);
			GetTableSelection().SelectCell(*cell);

			if (updateView)
			{
				ForceAdjustToTree();	// make sure col widths are correct
				UpdateScrollbars();		// make sure they are visible
			}

			TableScrollToCell(*cell);
			return true;
		}
	}

	return false;
}

/******************************************************************************
 SelectName

	Drill down to find the name, following pathList.

 ******************************************************************************/

bool
FileTreeTable::SelectName
	(
	const JPtrArray<JString>&	pathList,
	const JString&				name,
	JPoint*						cell,
	const bool					clearSelection,
	const bool					updateContent
	)
{
	if (updateContent)
	{
		itsFileTree->Update(true);
	}
	bool updateView = updateContent;

	JTableSelection& s   = GetTableSelection();
	FileTreeNode* parent = itsFileTree->GetFileRoot();

	const JSize count = pathList.GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		const JString& name1 = *(pathList.GetItem(i));

		const JNamedTreeNode* node;
		JIndex index;
		if (parent->FindNamedChild(name1, &node) &&
			itsFileTreeList->FindNode(node, &index))
		{
			cell->Set(GetNodeColIndex(), index);

			if (i == count && name.IsEmpty())
			{
				if (clearSelection)
				{
					s.ClearSelection();
				}
				SelectName(name1, parent, cell, false, updateView);
			}
		}
		else
		{
			if (clearSelection)
			{
				s.ClearSelection();
			}
			SelectName(name1, parent, cell, false, updateView);
			return false;
		}

		if (!itsFileTreeList->IsOpen(cell->y))
		{
			itsFileTreeList->Open(cell->y);
			updateView = true;
		}
		parent = itsFileTreeList->GetFileNode(cell->y);
	}

	if (clearSelection && !name.IsEmpty())
	{
		s.ClearSelection();
	}
	return SelectName(name, parent, cell, false, updateView);
}

/******************************************************************************
 IsEditable (virtual)

	Derived classes should override this if there are some cells that
	cannot be edited.

 ******************************************************************************/

bool
FileTreeTable::IsEditable
	(
	const JPoint& cell
	)
	const
{
	return JIndex(cell.x) == GetNodeColIndex();
}

/******************************************************************************
 HandleMouseHere (virtual protected)

 ******************************************************************************/

void
FileTreeTable::HandleMouseHere
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	if (itsEditTask != nullptr && JMouseMoved(itsStartPt, pt))
	{
		itsEditTask->Perform();
	}
}

/******************************************************************************
 HandleMouseDown

 ******************************************************************************/

#include "FileTreeTable-Context.h"

void
FileTreeTable::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JTableSelection& s = GetTableSelection();
	ClearIncrementalSearchBuffer();

	itsStartPt = itsPrevPt = pt;
	itsLastClickCount      = 0;
	itsWaitingForDragFlag  = false;
	itsClearIfNotDNDFlag   = false;
	itsWaitingToEditFlag   = false;

	jdelete itsEditTask;
	itsEditTask	= nullptr;

	JPoint cell;
	NodePart part;
	if (!GetNode(pt, &cell, &part))
	{
		if (!ScrollForWheel(button, modifiers))
		{
			StartDragRect(pt, button, modifiers);
		}
	}
	else if (part == kToggleColumn || button > kJXRightButton)
	{
		JXNamedTreeListWidget::HandleMouseDown(pt, button, clickCount,
											   buttonStates, modifiers);
		if (!IsDraggingToggle())
		{
			StartDragRect(pt, button, modifiers);
		}
	}
	else if (GetFMColType(cell.x, GetNodeColIndex(), itsVisibleCols) == kGFMMode &&
			 !(itsFileTreeList->GetDirEntry(cell.y))->IsLink())
	{
		if (!s.IsSelected(cell.y, GetNodeColIndex()))
		{
			s.ClearSelection();
		}

		const JRect cellRect  = GetCellRect(cell);
		const JIndex subIndex = 1 + (pt.x - cellRect.left)/itsPermCharWidth;

		JString permsStr;
		const JDirEntry* dirEntry = itsFileTreeList->GetDirEntry(cell.y);

		static const JUtf8Byte userChar[] = { 'u', 'g', 'o' };
		permsStr.Append(userChar + (subIndex-1)/3, 1);

		const mode_t theMode  = dirEntry->GetMode() & 0x01FF;
		const mode_t posMask  = 1 << (9 - subIndex);
		const bool usePos     = (theMode & posMask) != posMask;
		permsStr.Append(usePos ? "+" : "-");

		static const JUtf8Byte permChar[] = { 'x', 'r', 'w' };
		permsStr.Append(permChar + subIndex % 3, 1);

		s.SelectCell(cell.y, GetNodeColIndex());

		JTableSelectionIterator iter(&s);
		JPoint testCell;
		while (iter.Next(&testCell))
		{
			FileTreeNode* node     = itsFileTreeList->GetFileNode(testCell.y);
			const JString fullName = node->GetDirEntry()->GetFullName();

			JString sysCmd("chmod ");
			if (modifiers.meta())
			{
				sysCmd += "-R ";
			}
			sysCmd += permsStr;
			sysCmd += " ";
			sysCmd += JPrepArgForExec(fullName);
			Exec(sysCmd);
			node->Update(true);
		}
	}
	else if (part == kBeforeImage || part == kAfterText ||
			 (part == kOtherColumn && (GetCellString(cell)).IsEmpty()))
	{
		StartDragRect(pt, button, modifiers);
	}
	else if (button == kJXRightButton)
	{
		if (itsContextMenu == nullptr)
		{
			itsContextMenu = jnew JXTextMenu(JString::empty, this, kFixedLeft, kFixedTop, 0,0, 10,10);
			itsContextMenu->SetToHiddenPopupMenu(true);
			itsContextMenu->SetMenuItems(kContextMenuStr);
			itsContextMenu->SetUpdateAction(JXMenu::kDisableNone);
			itsContextMenu->AttachHandlers(this,
				&FileTreeTable::UpdateContextMenu,
				&FileTreeTable::HandleContextMenu);
			ConfigureContextMenu(itsContextMenu);
		}

		const JPoint cell1(GetNodeColIndex(), cell.y);
		if (!s.IsSelected(cell1))
		{
			SelectSingleCell(cell1, false);
		}

		itsContextMenu->PopUp(this, pt, buttonStates, modifiers);
	}
	else if (modifiers.shift() && !modifiers.control() && !modifiers.meta())
	{
		s.InvertCell(cell.y, GetNodeColIndex());
	}
	else if (clickCount > 1)
	{
		itsWaitingForDragFlag = s.HasSelection();
		itsLastClickCount     = clickCount;		// save for HandleMouseUp()
	}
	else if (!modifiers.shift() && modifiers.control() && !modifiers.meta())
	{
		// after checking for double-click, since this inverts selection

		// does not use meta on MacOS, because meta-dbl-click used in too many places

		s.InvertCell(cell.y, GetNodeColIndex());
		itsWaitingForDragFlag = s.IsSelected(cell.y, GetNodeColIndex());
	}
	else
	{
		itsWaitingForDragFlag = true;
		itsWaitingToEditFlag  = part == kInText;
		itsEditCell           = cell;

		if (s.IsSelected(cell.y, GetNodeColIndex()))
		{
			itsClearIfNotDNDFlag = true;
		}
		else
		{
			s.ClearSelection();
			s.SelectCell(cell.y, GetNodeColIndex());
		}
	}
}

/******************************************************************************
 StartDragRect (private)

 ******************************************************************************/

void
FileTreeTable::StartDragRect
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JXKeyModifiers&	modifiers
	)
{
	if (button == kJXLeftButton)
	{
		if (!modifiers.shift())
		{
			GetTableSelection().ClearSelection();
		}

		JPainter* p = CreateDragInsidePainter();
		p->Rect(JRect(pt, pt));
	}
}

/******************************************************************************
 HandleMouseDrag (virtual protected)

 ******************************************************************************/

void
FileTreeTable::HandleMouseDrag
	(
	const JPoint&			pt,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JPainter* p = nullptr;
	if (GetDragPainter(&p))
	{
		if (pt != itsPrevPt)
		{
			if (!ScrollForDrag(pt))
			{
				p->Rect(JRect(itsStartPt, itsPrevPt));
			}
			p->Rect(JRect(itsStartPt, pt));
			itsPrevPt = pt;
		}
	}
	else if (itsWaitingForDragFlag)
	{
		assert( GetTableSelection().HasSelection() );

		if (JMouseMoved(itsStartPt, pt))
		{
			itsWaitingForDragFlag = false;
			itsClearIfNotDNDFlag  = false;
			itsWaitingToEditFlag  = false;

			auto* data = jnew JXFileSelection(this, kDNDClassID);

			ChooseDNDCursors();
			if (BeginDND(pt, buttonStates, modifiers, data))
			{
				SetDNDSource(this);
			}
		}
	}
	else
	{
		JXNamedTreeListWidget::HandleMouseDrag(pt,buttonStates,modifiers);
	}
}

/******************************************************************************
 HandleMouseUp

 ******************************************************************************/

void
FileTreeTable::HandleMouseUp
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JTableSelection& s = GetTableSelection();

	JPainter* p = nullptr;
	if (GetDragPainter(&p))
	{
		const JRect selRect(itsStartPt, itsPrevPt);
		p->Rect(selRect);		// erase rectangle

		// Pin in bounds so points below bounds don't change but points
		// above bounds do.

		JPoint startCell, endCell;
		const bool startOK = GetCell(JPinInRect(itsStartPt, GetBounds()), &startCell);
		const bool endOK   = GetCell(JPinInRect(itsPrevPt, GetBounds()), &endCell);

		const JIndex start = JMin(startCell.y, endCell.y);
		const JIndex end   = JMax(startCell.y, endCell.y);

		if (startOK || endOK)
		{
			for (JIndex i=start; i<=end; i++)
			{
				JRect r = GetNodeRect(i);
				if (JIntersection(selRect, r, &r))
				{
					if (modifiers.shift())
					{
						s.InvertCell(i, GetNodeColIndex());
					}
					else
					{
						s.SelectCell(i, GetNodeColIndex());
					}
				}
			}
		}

		DeleteDragPainter();
		Refresh();
	}
	else if (itsWaitingForDragFlag && itsLastClickCount > 1)
	{
		OpenSelection(modifiers.meta(), button == kJXMiddleButton,
					  modifiers.control() && modifiers.shift(),
					  modifiers.control());
	}
	else if (itsWaitingToEditFlag)
	{
		JPoint cell;
		if (GetCell(itsStartPt, &cell))		// might update dir contents
		{
			s.ClearSelection();
			s.SelectCell(itsEditCell);

			assert( itsEditTask == nullptr );
			itsEditTask = jnew BeginEditingTask(this, itsEditCell);
			itsEditTask->Start();
		}
	}
	else if (itsClearIfNotDNDFlag)
	{
		JPoint cell;
		if (GetCell(itsStartPt, &cell))		// might update dir contents
		{
			s.ClearSelection();
			s.SelectCell(cell.y, GetNodeColIndex());
		}
	}
	else
	{
		JXNamedTreeListWidget::HandleMouseUp(pt, button, buttonStates, modifiers);
	}

	if (itsSortNode != nullptr)
	{
		itsSortNode->SortChildren();
		itsSortNode = nullptr;
	}

	itsWaitingToEditFlag = false;
	itsClearIfNotDNDFlag = false;
}

/******************************************************************************
 HandleKeyPress

 ******************************************************************************/

void
FileTreeTable::HandleKeyPress
	(
	const JUtf8Character&	c,
	const int				keySym,
	const JXKeyModifiers&	modifiers
	)
{
	if (!(GetDisplay()->GetLatestButtonStates()).AllOff())
	{
		return;		// don't let selection change during DND
	}

	if (c == kJReturnKey || keySym == XK_F2)
	{
		ClearIncrementalSearchBuffer();

		JTableSelection& s = GetTableSelection();
		JPoint cell;
		if (IsEditing())
		{
			EndEditing();
		}
		else if (s.GetSingleSelectedCell(&cell))
		{
			BeginEditing(cell);
		}
	}

	else if ((c == kJUpArrow || c == kJDownArrow) && !IsEditing())
	{
		const bool hasSelection = GetTableSelection().HasSelection();
		if (!hasSelection && c == kJUpArrow && GetRowCount() > 0)
		{
			SelectSingleCell(JPoint(GetNodeColIndex(), GetRowCount()));
		}
		else if (!hasSelection && c == kJDownArrow && GetRowCount() > 0)
		{
			SelectSingleCell(JPoint(GetNodeColIndex(), 1));
		}
		else
		{
			HandleSelectionKeyPress(c, modifiers);
		}
		ClearIncrementalSearchBuffer();
	}

	else if (c == kJForwardDeleteKey && (GetPrefsMgr())->DelWillDelete())
	{
		ClearIncrementalSearchBuffer();
		DeleteSelected();
	}

	else
	{
		JXNamedTreeListWidget::HandleKeyPress(c, keySym, modifiers);
	}
}

/******************************************************************************
 DeleteSelected

 ******************************************************************************/

void
FileTreeTable::DeleteSelected()
{
	if (!EndEditing() || !WarnForDelete())
	{
		return;
	}

	JTableSelectionIterator iter(&(GetTableSelection()));
	JPoint cell;
	while (iter.Next(&cell))
	{
		FileTreeNode* node = itsFileTreeList->GetFileNode(cell.y);
		const JString file = (node->GetDirEntry())->GetFullName();
		JProcess* p;
		JRemoveVCS(file, false, &p);
		if (p != nullptr)
		{
			ListenTo(p);
		}
	}
}

/******************************************************************************
 WarnForDelete (private)

	We check for JDirEntry::kDir because deleting a link doesn't delete
	the directory's contents.

 ******************************************************************************/

bool
FileTreeTable::WarnForDelete()
	const
{
	const JTableSelection& s = GetTableSelection();
	if (!s.HasSelection())
	{
		return false;
	}

	JString msg;
	JPoint cell;
	if (s.GetSingleSelectedCell(&cell))
	{
		const FileTreeNode* node = itsFileTreeList->GetFileNode(cell.y);
		const JUtf8Byte* map[] =
		{
			"name", node->GetName().GetBytes()
		};
		msg = JGetString((node->GetDirEntry())->GetType() == JDirEntry::kDir ?
							"WarnDeleteSingleDir::FileTreeTable" :
							"WarnDeleteSingleFile::FileTreeTable",
						 map, sizeof(map));
	}
	else
	{
		bool hasDirs = false;
		JTableSelectionIterator iter(&s);
		while (iter.Next(&cell))
		{
			if ((itsFileTreeList->GetDirEntry(cell.y))->GetType() == JDirEntry::kDir)
			{
				hasDirs = true;
				break;
			}
		}

		const JString countStr(s.GetSelectedCellCount(), 0);
		const JUtf8Byte* map[] =
		{
			"count", countStr.GetBytes()
		};
		msg = JGetString(hasDirs ? "WarnDeleteMultipleDir::FileTreeTable" :
								   "WarnDeleteMultipleFile::FileTreeTable",
						 map, sizeof(map));
	}

	return JGetUserNotification()->AskUserNo(msg);
}

/******************************************************************************
 HandleShortcut

 ******************************************************************************/

void
FileTreeTable::HandleShortcut
	(
	const int				key,
	const JXKeyModifiers&	modifiers
	)
{
	if (key == kJUpArrow && modifiers.meta())
	{
		GoUp(modifiers.shift());
	}
	else if (key == kJDownArrow && modifiers.meta())
	{
		OpenSelection(modifiers.shift(), false, false, false);
	}
	else
	{
		JXNamedTreeListWidget::HandleShortcut(key, modifiers);
	}
}

/******************************************************************************
 GoUp

 ******************************************************************************/

void
FileTreeTable::GoUp
	(
	const bool sameWindow
	)
{
	ClearIncrementalSearchBuffer();

	if (sameWindow == GetPrefsMgr()->WillOpenNewWindows())
	{
		FileTreeNode* root    = itsFileTree->GetFileRoot();
		const JString origDir = root->GetName();
		const JError err      = root->GoUp();
		if (err.OK())
		{
			JPoint cell;
			SelectName(origDir, nullptr, &cell);
		}
		else
		{
			err.ReportIfError();
		}
	}
	else
	{
		const JString path = JCombinePathAndName(itsFileTree->GetDirectory(), JString("..", JString::kNoCopy));
		GetApplication()->OpenDirectory(path);
	}
}

/******************************************************************************
 GoTo

 ******************************************************************************/

void
FileTreeTable::GoTo
	(
	const JString&	path,
	const bool	sameWindow
	)
{
	ClearIncrementalSearchBuffer();

	if (sameWindow == GetPrefsMgr()->WillOpenNewWindows())
	{
		const JError err = itsFileTree->GetFileRoot()->GoTo(path);
		err.ReportIfError();
	}
	else
	{
		GetApplication()->OpenDirectory(path);
	}
}

/******************************************************************************
 WillAcceptDrop (virtual protected)

 ******************************************************************************/

bool
FileTreeTable::WillAcceptDrop
	(
	const JArray<Atom>& typeList,
	Atom*				action,
	const JPoint&		pt,
	const Time			time,
	const JXWidget*		source
	)
{
	if (!EndEditing())
	{
		return false;
	}

	const Atom urlXAtom = GetSelectionManager()->GetURLXAtom(),
			   xdsXAtom = GetDNDManager()->GetDNDDirectSave0XAtom();

	const JSize typeCount = typeList.GetItemCount();
	for (JIndex i=1; i<=typeCount; i++)
	{
		const Atom type = typeList.GetItem(i);
		if (type == urlXAtom || type == xdsXAtom)
		{
			if (IsTrashDirectory(itsFileTree->GetDirectory()))
			{
				*action = GetDNDManager()->GetDNDActionPrivateXAtom();
			}

			HandleDNDHere(pt, source);

			JIndex dndIndex;
			const bool accept = GetDNDTargetIndex(&dndIndex);
			const JString dest    = itsFileTree->GetFileRoot()->GetDirEntry()->GetFullName();
			return accept || JDirectoryWritable(dest);
		}
	}

	return false;
}

/******************************************************************************
 HandleDNDEnter (virtual protected)

 ******************************************************************************/

void
FileTreeTable::HandleDNDEnter()
{
	ClearDNDTargetIndex();
	SetDNDTarget(this);
}

/******************************************************************************
 HandleDNDHere (virtual protected)

 ******************************************************************************/

void
FileTreeTable::HandleDNDHere
	(
	const JPoint&	pt,
	const JXWidget* source
	)
{
	JPoint cell;
	NodePart part;
	if (!GetNode(pt, &cell, &part) ||
		part == kToggleColumn || part == kBeforeImage || part == kAfterText)
	{
		ClearDNDTargetIndex();
	}
	else
	{
		SetDNDTargetIndex(GetNearestDirIndex(cell.y, true));
	}
}

/******************************************************************************
 HandleDNDLeave (virtual protected)

 ******************************************************************************/

void
FileTreeTable::HandleDNDLeave()
{
	ClearDNDTargetIndex();
	SetDNDTarget(nullptr);
}

/******************************************************************************
 HandleDNDDrop (virtual protected)

 ******************************************************************************/

void
FileTreeTable::HandleDNDDrop
	(
	const JPoint&		pt,
	const JArray<Atom>& typeList,
	const Atom			action,
	const Time			time,
	const JXWidget*		source
	)
{
	Atom realAction                = action;
	JXDNDManager* dndMgr           = GetDNDManager();
	const Atom dndName             = dndMgr->GetDNDSelectionName();
	JXSelectionManager* selManager = GetSelectionManager();

	bool isXDS = action == dndMgr->GetDNDActionDirectSaveXAtom();
	if (!isXDS)
	{
		const Atom xdsXAtom   = dndMgr->GetDNDDirectSave0XAtom();
		const JSize typeCount = typeList.GetItemCount();
		for (JIndex i = 1; i <= typeCount; i++)
		{
			if (typeList.GetItem(i) == xdsXAtom)
			{
				isXDS = true;
				break;
			}
		}
	}

	JIndex dndIndex = 0;
	if (isXDS)
	{
		const Window dragWindow = dndMgr->GetDraggerWindow();

		Atom actualType;
		int actualFormat;
		unsigned long itemCount, remainingBytes;
		unsigned char* rawData = nullptr;
		XGetWindowProperty(*GetDisplay(), dragWindow,
						   dndMgr->GetDNDDirectSave0XAtom(),
						   0, LONG_MAX, True, selManager->GetMimePlainTextXAtom(),
						   &actualType, &actualFormat,
						   &itemCount, &remainingBytes, &rawData);

		if (actualType == selManager->GetMimePlainTextXAtom() &&
			actualFormat == 8 && itemCount > 0)
		{
			FileTreeNode* node =
				GetDNDTargetIndex(&dndIndex) ?
				itsFileTreeList->GetFileNode(dndIndex) :
				itsFileTree->GetFileRoot();

			JString path = (node->GetDirEntry())->GetFullName();
			path         = JCombinePathAndName(path, JString((char*) rawData, JString::kNoCopy));
			JString url  = JFileNameToURL(path);
			XChangeProperty(*GetDisplay(), dragWindow,
							dndMgr->GetDNDDirectSave0XAtom(),
							GetSelectionManager()->GetMimePlainTextXAtom(), 8,
							PropModeReplace,
							(unsigned char*) url.GetBytes(), url.GetByteCount());

			unsigned char* data = nullptr;
			JSize dataLength;
			Atom returnType;
			JXSelectionManager::DeleteMethod delMethod;
			if (selManager->GetData(dndName, time, dndMgr->GetDNDDirectSave0XAtom(),
									&returnType, &data, &dataLength, &delMethod))
			{
				if (*data == 0x46)		// F: failure
				{
					XChangeProperty(*GetDisplay(), dragWindow,
									dndMgr->GetDNDDirectSave0XAtom(),
									GetSelectionManager()->GetMimePlainTextXAtom(), 8,
									PropModeReplace,
									(unsigned char*) "", 0);
					JGetUserNotification()->ReportError(
						JGetString("RemoteSavingUnusupported::FileTreeTable"));
				}
				selManager->DeleteData(&data, delMethod);
			}

		}
	}
	else if (!GetDNDTargetIndex(&dndIndex) &&
			 IsTrashDirectory(itsFileTree->GetDirectory()))
	{
		TrashButton::MoveFilesToTrash(time, source);
	}
	else if (GetTrueDropAction(&realAction))
	{
		unsigned char* data = nullptr;
		JSize dataLength;
		Atom returnType;
		JXSelectionManager::DeleteMethod delMethod;
		if (selManager->GetData(dndName, time, selManager->GetURLXAtom(),
								&returnType, &data, &dataLength, &delMethod))
		{
			if (returnType == selManager->GetURLXAtom())
			{
				auto* fileNameList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
				assert( fileNameList != nullptr );
				JPtrArray<JString> urlList(JPtrArrayT::kDeleteAll);
				JXUnpackFileNames((char*) data, dataLength, fileNameList, &urlList);

				FileTreeTable* srcTable = nullptr;
				GetDNDSource(source, &srcTable);

				FileTreeNode* destNode =
					GetDNDTargetIndex(&dndIndex) ?
					itsFileTreeList->GetFileNode(dndIndex) :
					itsFileTree->GetFileRoot();

				if (realAction == dndMgr->GetDNDActionLinkXAtom())
				{
					GetTableSelection().ClearSelection();

					const JSize count = fileNameList->GetItemCount();
					for (JIndex i=1; i<=count; i++)
					{
						MakeLinkToFile(*fileNameList->GetItem(i), destNode, true);
					}

					jdelete fileNameList;
				}
				else if (realAction == dndMgr->GetDNDActionMoveXAtom())
				{
					CopyProcess::Move(srcTable, fileNameList, this, destNode);
				}
				else
				{
					CopyProcess::Copy(srcTable, fileNameList, this, destNode);
				}

				JXReportUnreachableHosts(urlList);
			}

			selManager->DeleteData(&data, delMethod);
		}
	}

	ClearDNDTargetIndex();
	SetDNDSource(nullptr);
	SetDNDTarget(nullptr);

	ClearIncrementalSearchBuffer();
}

/******************************************************************************
 GetTrueDropAction (private)

 ******************************************************************************/

bool
FileTreeTable::GetTrueDropAction
	(
	Atom* action
	)
{
	JXDNDManager* dndMgr = GetDNDManager();
	if (*action == dndMgr->GetDNDActionAskXAtom())
	{
		JArray<Atom> actionList;
		JPtrArray<JString> descriptionList(JPtrArrayT::kDeleteAll);
		if (!dndMgr->GetAskActions(&actionList, &descriptionList))
		{
			*action = dndMgr->GetDNDActionMoveXAtom();
		}
		else if (!dndMgr->ChooseDropAction(actionList, descriptionList, action))
		{
			return false;
		}
	}
	else if (IsTrashDirectory(itsFileTree->GetDirectory()))
	{
		// handle special case when dropping on folder inside trash

		*action = dndMgr->GetDNDActionMoveXAtom();
	}
	else if (*action == dndMgr->GetDNDActionPrivateXAtom())		// ask
	{
		JArray<Atom> actionList;
		JPtrArray<JString> descriptionList(JPtrArrayT::kDeleteAll);
		GetDNDAskActions(JXButtonStates(), JXKeyModifiers(GetDisplay()),
						 &actionList, &descriptionList);
		if (!dndMgr->ChooseDropAction(actionList, descriptionList, action))
		{
			return false;
		}
	}

	return true;
}

/******************************************************************************
 GetSelectionData (virtual protected)

	This is called when DND is terminated by a drop or when the target
	requests the data during a drag, but only if the delayed evaluation
	constructor for JXSelectionData was used.

	id is the string passed to the JXSelectionData constructor.

 ******************************************************************************/

void
FileTreeTable::GetSelectionData
	(
	JXSelectionData*	data,
	const JString&		id
	)
{
	if (id == kDNDClassID)
	{
		assert( GetTableSelection().HasSelection() );

		auto* fileData = dynamic_cast<JXFileSelection*>(data);
		assert( fileData != nullptr );

		auto* fileList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
		assert( fileList != nullptr );

		JTableSelectionIterator iter(&(GetTableSelection()));
		JPoint cell;
		while (iter.Next(&cell))
		{
			const JDirEntry* entry = itsFileTreeList->GetDirEntry(cell.y);
			fileList->Append(entry->GetFullName());
		}

		fileData->SetData(fileList);
	}
	else
	{
		JXNamedTreeListWidget::GetSelectionData(data, id);
	}
}

/******************************************************************************
 GetDNDAction (virtual protected)

	This is called repeatedly during the drag so the drop action can be
	changed based on the current target, buttons, and modifier keys.

 ******************************************************************************/

Atom
FileTreeTable::GetDNDAction
	(
	const JXContainer*		target,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	return GetDNDAction(this, itsFileTree->GetDirectory(), target, modifiers);
}

/******************************************************************************
 GetDNDAction (static)

	This is used by FolderDragSource.

 ******************************************************************************/

Atom
FileTreeTable::GetDNDAction
	(
	const JXWidget*			source,
	const JString&			sourcePath,
	const JXContainer*		target,
	const JXKeyModifiers&	modifiers
	)
{
	bool moveDef = true;

	FileTreeTable* sygTarget = nullptr;
	if (GetDNDTarget(target, &sygTarget))
	{
		JIndex dndIndex;
		FileTreeNode* node =
			sygTarget->GetDNDTargetIndex(&dndIndex) ?
			sygTarget->itsFileTreeList->GetFileNode(dndIndex) :
			sygTarget->itsFileTree->GetFileRoot();

		if (!JIsSamePartition(sourcePath, (node->GetDirEntry())->GetFullName()))
		{
			moveDef = false;
		}
	}

	const JXMenu::Style style = JXMenu::GetDisplayStyle();
	const bool toggleMoveToCopy = ((style == JXMenu::kMacintoshStyle && modifiers.meta()) ||
		 (style == JXMenu::kWindowsStyle   && modifiers.control()));

	if ((style == JXMenu::kMacintoshStyle && modifiers.control()) ||
		(style == JXMenu::kWindowsStyle   && modifiers.shift()))
	{
		return (source->GetDNDManager())->GetDNDActionAskXAtom();
	}
	else if ((style == JXMenu::kMacintoshStyle && modifiers.shift()) ||
			 (style == JXMenu::kWindowsStyle   && modifiers.meta()))
	{
		return (source->GetDNDManager())->GetDNDActionLinkXAtom();
	}
	else if (( toggleMoveToCopy &&  moveDef) ||
			 (!toggleMoveToCopy && !moveDef))
	{
		return (source->GetDNDManager())->GetDNDActionCopyXAtom();
	}
	else
	{
		return (source->GetDNDManager())->GetDNDActionMoveXAtom();
	}
}

/******************************************************************************
 GetDNDAskActions (virtual protected)

	This is called when the value returned by GetDropAction() changes to
	XdndActionAsk.  If GetDropAction() repeatedly returns XdndActionAsk,
	this function is not called again because it is assumed that the
	actions are the same within a single DND session.

	This function must place at least 2 elements in askActionList and
	askDescriptionList.

	The first element should be the default action.

 ******************************************************************************/

void
FileTreeTable::GetDNDAskActions
	(
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers,
	JArray<Atom>*			askActionList,
	JPtrArray<JString>*		askDescriptionList
	)
{
	::GetDNDAskActions(askActionList, askDescriptionList);
}

/******************************************************************************
 ChooseDNDCursors (private)

	Choose the appropriate set of cursors to use for DND.

 ******************************************************************************/

void
FileTreeTable::ChooseDNDCursors()
{
	bool hasDir = false, hasFile = false;

	JTableSelectionIterator iter(&(GetTableSelection()));
	JPoint cell;
	while (iter.Next(&cell) && JIndex(cell.x) == GetNodeColIndex())
	{
		if ((itsFileTreeList->GetDirEntry(cell.y))->IsDirectory())
		{
			hasDir = true;
		}
		else
		{
			hasFile = true;
		}

		if (hasDir && hasFile)
		{
			break;
		}
	}

	if (hasDir && hasFile)
	{
		itsDNDCursorType = kDNDMixedCursor;
	}
	else if (hasDir)
	{
		itsDNDCursorType = kDNDDirCursor;
	}
	else
	{
		itsDNDCursorType = kDNDFileCursor;
	}
}

/******************************************************************************
 HandleDNDResponse (virtual protected)

	This is called when the target indicates whether or not it will accept
	the drop.  If !dropAccepted, the action is undefined.  If the drop target
	is not within the same application, target is nullptr.

 ******************************************************************************/

void
FileTreeTable::HandleDNDResponse
	(
	const JXContainer*	target,
	const bool		dropAccepted,
	const Atom			action
	)
{
	JXDNDManager* dndMgr = GetDNDManager();
	JCursorIndex cursor;
	if (itsDNDCursorType == kDNDMixedCursor)
	{
		cursor = dndMgr->GetDNDFileAndDirectoryCursor(dropAccepted, action);
	}
	else if (itsDNDCursorType == kDNDDirCursor)
	{
		cursor = dndMgr->GetDNDDirectoryCursor(dropAccepted, action);
	}
	else
	{
		cursor = dndMgr->GetDNDFileCursor(dropAccepted, action);
	}
	DisplayCursor(cursor);
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
FileTreeTable::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsRecentFilesMenu && message.Is(JXFSDirMenu::kFileSelected))
	{
		const auto* info	=
			dynamic_cast<const JXFSDirMenu::FileSelected*>(&message);
		assert( info != nullptr );

		const JDirEntry entry(info->GetFileName());
		const JString* link;
		if (entry.GetLinkName(&link))
		{
			AddRecentFile(*link);
		}

		const bool alternateOpen = (GetDisplay()->GetLatestKeyModifiers()).meta();

		JPtrArray<JString> fileList(JPtrArrayT::kDeleteAll);
		fileList.Append(info->GetFileName());
		JXFSBindingManager::Exec(fileList, alternateOpen);
	}

	else if (sender == itsFormatProcess && message.Is(JProcess::kFinished))
	{
		JMount(itsFileTree->GetDirectory());
		itsFormatProcess = nullptr;
	}

	else if (sender == itsGitProcess && message.Is(JProcess::kFinished))
	{
		const auto* info =
			dynamic_cast<const JProcess::Finished*>(&message);
		assert(info != nullptr);
		if (info->Successful())
		{
			JExecute(itsFileTree->GetDirectory(), (GetApplication())->GetPostCheckoutCommand(), nullptr);
		}
		itsGitProcess = nullptr;
	}

	else if (sender == itsIconWidget && message.Is(JXWindowIcon::kHandleEnter))
	{
		HandleDNDEnter();
	}
	else if (sender == itsIconWidget && message.Is(JXWindowIcon::kHandleLeave))
	{
		HandleDNDLeave();
	}
	else if (sender == itsIconWidget && message.Is(JXWindowIcon::kHandleDrop))
	{
		const auto* info =
			dynamic_cast<const JXWindowIcon::HandleDrop*>(&message);
		assert( info != nullptr );
		HandleDNDDrop(JPoint(0,0), info->GetTypeList(), info->GetAction(),
					  info->GetTime(), info->GetSource());
	}

	else if (sender == GetApplication() && message.Is(Application::kTrashNeedsUpdate))
	{
		if (IsTrashDirectory(itsFileTree->GetDirectory()))
		{
			UpdateDisplay(true);
		}
	}
	else if (sender == GetApplication() && message.Is(Application::kShortcutsChanged))
	{
		UpdateShortcutMenu();
	}

	else if (sender == GetApplication() && message.Is(JFSFileTree::kDirectoryRenamed))
	{
		itsFileTree->GetFileRoot()->UpdatePath(message);
	}

	else
	{
		if (sender == GetWindow() && message.Is(JXWindow::kIconified))
		{
//			UpdateDisplay();
//			itsUpdateTask->Stop();
		}
		else if (sender == GetWindow() && message.Is(JXWindow::kDeiconified))
		{
			ClearIncrementalSearchBuffer();
//			UpdateDisplay();
//			itsUpdateTask->Start();
		}

		else if (sender == itsFileTree && message.Is(JFSFileTree::kDirectoryRenamed))
		{
			GetApplication()->DirectoryRenamed(message);
		}
		else if (sender == itsFileTree->GetRootDirInfo() &&
				 message.Is(JDirInfo::kPathChanged))
		{
			UpdateInfo();
		}

		else if (message.Is(JProcess::kFinished))
		{
			UpdateDisplay(true);
		}

		JXNamedTreeListWidget::Receive(sender, message);
	}
}

/******************************************************************************
 ReceiveWithFeedback (virtual protected)

 ******************************************************************************/

void
FileTreeTable::ReceiveWithFeedback
	(
	JBroadcaster*	sender,
	Message*		message
	)
{
	if (sender == itsIconWidget && message->Is(JXWindowIcon::kAcceptDrop))
	{
		auto* info =
			dynamic_cast<JXWindowIcon::AcceptDrop*>(message);
		assert( info != nullptr );
		info->ShouldAcceptDrop(WillAcceptDrop(
									info->GetTypeList(), info->GetActionPtr(),
									info->GetPoint(), info->GetTime(), info->GetSource()));
	}
	else
	{
		JXNamedTreeListWidget::ReceiveWithFeedback(sender, message);
	}
}

/******************************************************************************
 UpdateInfo (private)

 ******************************************************************************/

void
FileTreeTable::UpdateInfo()
{
	bool writable, isTop;
	JString device;
	JFileSystemType fsType;

	itsIgnoreExecPermFlag =
		JIsMounted(itsFileTree->GetRootDirInfo()->GetDirectory(),
				   &writable, &isTop, &device, &fsType) &&
		!JMountSupportsExecFlag(fsType);
	Refresh();

	SetWindowIcon();

	if (itsGitProcess != nullptr)
	{
		StopListening(itsGitProcess);
		itsGitProcess = nullptr;
	}
}

/******************************************************************************
 UpdateDisplay

 ******************************************************************************/

void
FileTreeTable::UpdateDisplay
	(
	const bool force
	)
{
	JPainter* p = nullptr;
	if (GetDragPainter(&p))
	{
		return;
	}

	if (!JFSFileTreeNode::CanHaveChildren(itsFileTree->GetDirectory()))
	{
		if (GetApplication()->GetWindowCount() == 1)
		{
			JString p;
			if (!JGetHomeDirectory(&p) || !JFSFileTreeNode::CanHaveChildren(p))
			{
				p = JGetRootDirectory();
			}
			itsFileTree->GetFileRoot()->GoTo(p);
		}
		else
		{
			JXCloseDirectorTask::Close(GetWindow()->GetDirector());
		}
	}
	else
	{
		if (force || itsUpdateNode == nullptr)
		{
			itsUpdateNode = itsFileTree->GetFileRoot();
		}

		const bool updateMenus = itsUpdateNode == itsFileTree->GetFileRoot();

		StopListening(itsUpdateNode);
		itsFileTree->Update(force, &itsUpdateNode);
		if (itsUpdateNode != nullptr)
		{
			ClearWhenGoingAway(itsUpdateNode, &itsUpdateNode);
		}

		if (updateMenus)
		{
			UpdateMenus();
		}
	}

	// trash button

	itsTrashButton->UpdateDisplay();
}

/******************************************************************************
 UpdateMenus (private)

 ******************************************************************************/

void
FileTreeTable::UpdateMenus()
{
	const bool wasActive = itsGitMenu->IsActive();

	const JString gitPath = JCombinePathAndName(itsFileTree->GetDirectory(), JGetGitDirectoryName());
	itsGitMenu->SetActive(JDirectoryExists(gitPath));
	if (!wasActive && itsGitMenu->IsActive())
	{
		InitGitBranchMenus();
	}
}

/******************************************************************************
 SetWindowIcon

 ******************************************************************************/

void
FileTreeTable::SetWindowIcon()
{
	JXPM plain(nullptr), selected(nullptr);
	const JIndex type = GetMountPointLargeIcon(itsFileTree->GetDirectory(),
												  itsFileTreeList, &plain, &selected);
	if (type != itsWindowIconType || itsIconWidget == nullptr)
	{
		itsWindowIconType = type;

		JXDisplay* display = GetDisplay();
/*		JXImage* icon1 = jnew JXImage(display, plain);
		JXImage* icon2 = jnew JXImage(display, selected);

		itsIconWidget = GetWindow()->SetIcon(icon1, icon2);
		ListenTo(itsIconWidget);
*/
		JXImage* icon3;
		const JError err = JXImage::CreateFromXPM(display, plain, &icon3);
		assert_ok( err );

		GetWindow()->SetIcon(icon3);
	}
}

/******************************************************************************
 UpdateFileMenu (private)

 ******************************************************************************/

void
FileTreeTable::UpdateFileMenu()
{
	JTableSelection& s          = GetTableSelection();
	const bool hasSelection = s.HasSelection();

	JPoint singleCell;
	itsFileMenu->SetItemEnabled(kAltOpenCmd,   hasSelection);
	itsFileMenu->SetItemEnabled(kRunOnSelCmd,  hasSelection);
	itsFileMenu->SetItemEnabled(kRenameCmd,    s.GetSingleSelectedCell(&singleCell));
	itsFileMenu->SetItemEnabled(kDuplicateCmd, hasSelection);

	// symbolic links

	bool findOriginal = false;
	if (hasSelection)
	{
		JTableSelectionIterator iter(&s);
		JPoint cell;
		while (iter.Next(&cell))
		{
			if ((itsFileTreeList->GetDirEntry(cell.y))->IsWorkingLink())
			{
				findOriginal = true;
			}
		}
	}

	itsFileMenu->SetItemEnabled(kMakeAliasCmd,        hasSelection);
	itsFileMenu->SetItemEnabled(kFindOriginalCmd,     findOriginal);
	itsFileMenu->SetItemEnabled(kConvertToFileCmd,    hasSelection);
	itsFileMenu->SetItemEnabled(kConvertToProgramCmd, hasSelection);

	// mount point

	const JString& path = itsFileTree->GetDirectory();
	itsFileMenu->SetItemEnabled(kToggleMountCmd,
		(GetApplication())->IsMountPoint(path));
	bool writable;
	if (JIsMounted(path, &writable))
	{
		itsFileMenu->SetItemText(kToggleMountCmd, JGetString("UnmountLabel::FileTreeTable"));
		itsFileMenu->SetItemEnabled(kEraseDiskCmd, writable);
	}
	else	// including if not a mount point
	{
		itsFileMenu->SetItemText(kToggleMountCmd, JGetString("MountLabel::FileTreeTable"));
		itsFileMenu->DisableItem(kEraseDiskCmd);
	}
}

/******************************************************************************
 HandleFileMenu

 ******************************************************************************/

void
FileTreeTable::HandleFileMenu
	(
	const JIndex index
	)
{
	ClearIncrementalSearchBuffer();

	if (index == kNewDirCmd)
	{
		CreateNewDirectory();
	}
	else if (index == kNewTextFileCmd)
	{
		CreateNewTextFile();
	}
	else if (index == kOpenCmd)
	{
		OpenSelection(false, false, false, false);
	}
	else if (index == kOpenCloseCmd)
	{
		OpenSelection(false, false, false, true);
	}
	else if (index == kOpenIconifyCmd)
	{
		OpenSelection(false, false, true, false);
	}
	else if (index == kAltOpenCmd)
	{
		OpenSelection(true, false, false, false);
	}
	else if (index == kRunOnSelCmd)
	{
		OpenSelection(false, true, false, false);
	}

	else if (index == kFindCmd)
	{
		const JString path = GetCommandPath();
		GetWindow()->Deiconify();
		(GetFindFileDialog())->Search(path);
	}

	else if (index == kRenameCmd)
	{
		JPoint cell;
		if (GetTableSelection().GetSingleSelectedCell(&cell))
		{
			BeginEditing(cell);
		}
	}

	else if (index == kDuplicateCmd)
	{
		DuplicateSelected();
	}
	else if (index == kMakeAliasCmd)
	{
		MakeLinks();
	}
	else if (index == kFindOriginalCmd)
	{
		FindOriginals();
	}

	else if (index == kConvertToFileCmd)
	{
		ChangeExecPermission(false);
	}
	else if (index == kConvertToProgramCmd)
	{
		ChangeExecPermission(true);
	}

	else if (index == kToggleMountCmd)
	{
		const JString& path = itsFileTree->GetDirectory();
		JMount(path, !JIsMounted(path));
	}
	else if (index == kEraseDiskCmd)
	{
		FormatDisk();
	}

	else if (index == kHomeWindowCmd)
	{
		JString homeDir;
		if (JGetHomeDirectory(&homeDir))
		{
			GoTo(homeDir, (GetDisplay()->GetLatestKeyModifiers()).shift());
		}
	}

	else if (index == kViewManCmd)
	{
		ViewManPage();
	}
	else if (index == kOpenTermCmd)
	{
		OpenTerminal();
	}
	else if (index == kRunCmdCmd)
	{
		RunCommand();
	}

	else if (index == kCloseCmd)
	{
		GetWindow()->Close();
	}
	else if (index == kQuitCmd)
	{
		JXGetApplication()->Quit();
	}
}

/******************************************************************************
 CreateNewDirectory

 ******************************************************************************/

void
FileTreeTable::CreateNewDirectory()
{
	if (!EndEditing())
	{
		return;
	}

	FileTreeNode* node = PrepareToCreateNew();

	JString dirName  = node->GetDirEntry()->GetFullName();
	dirName          = JGetUniqueDirEntryName(dirName, JGetString("NewNamePrefix::FileTreeTable"));
	const JError err = JCreateDirectory(dirName);
	if (err.OK())
	{
		JString p, n;
		JSplitPathAndName(dirName, &p, &n);
		JPoint cell;
		if (SelectName(n, node, &cell))		// updates table
		{
			BeginEditing(cell);
		}
	}
	else
	{
		JGetStringManager()->ReportError("CreateFolderError::FileTreeTable", err);
	}
}

/******************************************************************************
 CreateNewTextFile

 ******************************************************************************/

void
FileTreeTable::CreateNewTextFile()
{
	if (!EndEditing())
	{
		return;
	}

	FileTreeNode* node = PrepareToCreateNew();

	JString name = node->GetDirEntry()->GetFullName();
	name         = JGetUniqueDirEntryName(name, JGetString("NewNamePrefix::FileTreeTable"), ".txt");
	{
	std::ofstream output(name.GetBytes());
	}
	if (JFileExists(name))
	{
		JString p, n;
		JSplitPathAndName(name, &p, &n);
		JPoint cell;
		if (SelectName(n, node, &cell))		// updates table
		{
			BeginEditing(cell);
		}
	}
	else
	{
		JGetUserNotification()->ReportError(JGetString("CreateTextFileError::FileTreeTable"));
	}
}

/******************************************************************************
 PrepareToCreateNew (private)

 ******************************************************************************/

FileTreeNode*
FileTreeTable::PrepareToCreateNew()
{
	ClearIncrementalSearchBuffer();

	JTableSelection& s = GetTableSelection();
	JPoint cell;
	if (s.GetFirstSelectedCell(&cell))
	{
		const JIndex index = GetNearestDirIndex(cell.y, true);
		if (index == 0)
		{
			return itsFileTree->GetFileRoot();
		}
		else
		{
			FileTreeNode* node = itsFileTreeList->GetFileNode(index);
			GetTreeList()->Open(node);
			return node;
		}
	}
	else
	{
		return itsFileTree->GetFileRoot();
	}
}

/******************************************************************************
 GetNearestDirIndex (private)

 ******************************************************************************/

JIndex
FileTreeTable::GetNearestDirIndex
	(
	const JIndex	index,
	const bool	requireWritable
	)
{
	const JTreeList* treeList = GetTreeList();

	JIndex dirIndex = index;

	const JTreeNode* node = treeList->GetNode(index);
	if (!node->IsOpenable())
	{
		const JTreeNode* parent = node->GetParent();
		if (!treeList->FindNode(parent, &dirIndex))
		{
			return 0;	// if file not in subdirectory
		}
	}

	if (requireWritable)
	{
		return ((itsFileTreeList->GetDirEntry(dirIndex))->IsWritable() ? dirIndex : 0);
	}
	else
	{
		return dirIndex;
	}
}

/******************************************************************************
 OpenSelection (private)

 ******************************************************************************/

void
FileTreeTable::OpenSelection
	(
	const bool alternate,
	const bool alwaysRunCmd,
	const bool iconifyAfter,
	const bool closeAfter
	)
{
	ClearIncrementalSearchBuffer();

	if (!EndEditing())
	{
		return;
	}

	bool found = false;

	JPtrArray<JString> fileList(JPtrArrayT::kDeleteAll);
	fileList.SetCompareFunction(JCompareStringsCaseSensitive);

	JTableSelectionIterator iter(&GetTableSelection());
	JPoint cell;
	while (iter.Next(&cell))
	{
		FileTreeNode* node  = itsFileTreeList->GetFileNode(cell.y);
		const JDirEntry* entry = node->GetDirEntry();
		if (!alwaysRunCmd && entry->IsDirectory())
		{
			if (node->IsOpenable())
			{
				const JString newPath = (node->GetDirEntry())->GetFullName();
				GoTo(newPath, alternate);
				if (alternate)
				{
					return;
				}
			}
		}
		else if (!alwaysRunCmd && !itsIgnoreExecPermFlag && entry->IsExecutable())
		{
			const JString file = (node->GetDirEntry())->GetFullName();
			JXFSBindingManager::Exec(file, alternate);
		}
		else
		{
			auto* s = jnew JString((node->GetDirEntry())->GetFullName());
			assert( s != nullptr );
			if (!fileList.InsertSorted(s, false))
			{
				jdelete s;
			}
		}

		found = true;
	}

	if (found && !fileList.IsEmpty())
	{
		JXFSBindingManager::Exec(fileList, alternate || alwaysRunCmd);

		if (fileList.GetItemCount() == 1)
		{
			AddRecentFile(*fileList.GetItem(1));
		}
	}
	else if (!found)
	{
		auto* dlog = ChoosePathDialog::Create(JXChoosePathDialog::kAcceptReadable, itsFileTree->GetDirectory());
		if (dlog->DoDialog())
		{
			const bool sameWindow = !dlog->OpenInNewWindow();
			GoTo(dlog->GetPath(), sameWindow);
			if (sameWindow)
			{
				GetWindow()->GetDirector()->Activate();
				return;
			}
		}
	}

	// not done if display new directory in same window

	if (iconifyAfter)	// takes precedence because more modifiers than closeAfter
	{
		GetWindow()->Iconify();
	}
	else if (closeAfter)
	{
		// wait until safe to commit suicide

		JXCloseDirectorTask::Close(GetWindow()->GetDirector());
	}
}


/******************************************************************************
 DuplicateSelected

 ******************************************************************************/

void
FileTreeTable::DuplicateSelected()
{
	if (!EndEditing())
	{
		return;
	}

	JPtrArray<FileTreeNode> nodeList(JPtrArrayT::kForgetAll);

	JTableSelectionIterator iter(&(GetTableSelection()));
	JPoint cell;
	while (iter.Next(&cell))
	{
		nodeList.Append(itsFileTreeList->GetFileNode(cell.y));
	}

	DuplicateProcess::Duplicate(this, nodeList);
}

/******************************************************************************
 MakeLinks

 ******************************************************************************/

void
FileTreeTable::MakeLinks()
{
	if (!EndEditing())
	{
		return;
	}

	JPtrArray<FileTreeNode> nodeList(JPtrArrayT::kForgetAll);

	JTableSelection& s = GetTableSelection();
	JTableSelectionIterator iter(&s);
	JPoint cell;
	while (iter.Next(&cell))
	{
		// can't call MakeLinkToFile() here because it changes selection
		nodeList.Append(itsFileTreeList->GetFileNode(cell.y));
	}

	s.ClearSelection();

	const JSize count = nodeList.GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		const FileTreeNode* node = nodeList.GetItem(i);
		MakeLinkToFile(node->GetDirEntry()->GetFullName(), node->GetFileParent(), false);
	}

	if (s.GetSingleSelectedCell(&cell))
	{
		BeginEditing(cell);
	}
}

/******************************************************************************
 MakeLinkToFile (private)

 ******************************************************************************/

void
FileTreeTable::MakeLinkToFile
	(
	const JString&			origSrc,
	const FileTreeNode*	parentNode,
	const bool			allowRelative
	)
{
	JString src = origSrc;
	JString srcPath, srcName;
	JSplitPathAndName(src, &srcPath, &srcName);

	const JString destPath = (parentNode->GetDirEntry())->GetFullName();
	JString dest           = JCombinePathAndName(destPath, srcName);
	if (JNameUsed(dest))
	{
		JString root, suffix;
		if (JSplitRootAndSuffix(srcName, &root, &suffix))
		{
			suffix.Prepend(".");
		}
		root += "_alias";

		dest = JGetUniqueDirEntryName(destPath, root, suffix.GetBytes());
	}

	if (allowRelative)
	{
		const JUtf8Byte* map[] =
		{
			"name", srcName.GetBytes()
		};
		const JString msg = JGetString("AskRelativeAlias::FileTreeTable", map, sizeof(map));
		if (JGetUserNotification()->AskUserYes(msg))
		{
			src = JConvertToRelativePath(src, destPath);
		}
	}

	const JError err = JCreateSymbolicLink(src, dest);
	if (err.OK())
	{
		JString p, n;
		JSplitPathAndName(dest, &p, &n);
		JPoint cell;
		SelectName(n, parentNode, &cell);		// updates table
	}
	else
	{
		JGetStringManager()->ReportError("CreateAliasError::FileTreeTable", err);
	}
}

/******************************************************************************
 FindOriginals

 ******************************************************************************/

void
FileTreeTable::FindOriginals()
{
	JTableSelectionIterator iter(&(GetTableSelection()));

	JPoint cell;
	while (iter.Next(&cell))
	{
		const JDirEntry* entry = itsFileTreeList->GetDirEntry(cell.y);
		JString fullName;
		if (entry->IsWorkingLink() &&
			JGetTrueName(entry->GetFullName(), &fullName))
		{
			if (JDirectoryExists(fullName))
			{
				// open the containing directory, not the directory itself

				JString path, name;
				JSplitPathAndName(fullName, &path, &name);

				TreeDir* dir;
				JIndex row;
				JPoint cell;
				if (GetApplication()->OpenDirectory(path, &dir, &row, true, true, false, false))
				{
					FileTreeNode* parent = nullptr;
					if (row > 0)
					{
						dir->GetTable()->GetTableSelection().SelectCell(JPoint(GetNodeColIndex(), row), false);
						parent = dir->GetTable()->GetFileTreeList()->GetFileNode(row);
					}

					dir->GetTable()->SelectName(name, parent, &cell);
				}
			}
			else
			{
				GetApplication()->OpenDirectory(fullName, nullptr, nullptr, true, true, false, false);
			}
		}
	}
}

/******************************************************************************
 FormatDisk

 ******************************************************************************/

void
FileTreeTable::FormatDisk()
{
	if (itsFormatProcess == nullptr &&
		JGetUserNotification()->AskUserNo(JGetString("WarnEraseDisk::FileTreeTable")))
	{
		JPtrArray<JString> choiceList(JPtrArrayT::kDeleteAll);
		for (const auto& name : kFormatName)
		{
			choiceList.Append(name);
		}

		auto* dlog =
			jnew JXRadioGroupDialog(JGetString("FormatWindowTitle::FileTreeTable"),
									JGetString("FormatPrompt::FileTreeTable"), choiceList, nullptr);
		assert( dlog != nullptr );
		if (dlog->DoDialog())
		{
			const JIndex i   = dlog->GetSelectedItem();
			const JError err = JFormatPartition(itsFileTree->GetDirectory(),
												kFormatType[i-1], &itsFormatProcess);
			if (err.OK())
			{
				itsFormatProcess->ShouldDeleteWhenFinished();
				ListenTo(itsFormatProcess);
			}
			else
			{
				err.ReportIfError();
			}
		}
	}
}

/******************************************************************************
 ViewManPage

 ******************************************************************************/

void
FileTreeTable::ViewManPage()
{
	if (!EndEditing())
	{
		return;
	}

	JTableSelectionIterator iter(&(GetTableSelection()));
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);

	JPoint cell;
	while (iter.Next(&cell))
	{
		list.Append(itsFileTreeList->GetFileNode(cell.y)->GetName());
	}

	if (list.IsEmpty())
	{
		(GetManPageDialog())->Activate();
	}
	else
	{
		(GetManPageDialog())->ViewManPages(list);
	}
}

/******************************************************************************
 OpenTerminal

 ******************************************************************************/

void
FileTreeTable::OpenTerminal()
{
	if (EndEditing())
	{
		const JString path = GetCommandPath();
		GetApplication()->OpenTerminal(path);
	}
}

/******************************************************************************
 RunCommand

 ******************************************************************************/

void
FileTreeTable::RunCommand()
{
	if (EndEditing())
	{
		const JString path = GetCommandPath();
		JXFSBindingManager::Exec(path);
	}
}

/******************************************************************************
 GetCommandPath (private)

 ******************************************************************************/

JString
FileTreeTable::GetCommandPath()
	const
{
	JString path;

	JPoint cell1, cell2;
	if (GetWindow()->IsIconified())
	{
		path = itsFileTree->GetDirectory();
	}
	else if (GetTableSelection().GetFirstSelectedCell(&cell1) &&
			 GetTableSelection().GetLastSelectedCell(&cell2))
	{
		const JDirEntry* e1 = itsFileTreeList->GetDirEntry(cell1.y);
		const JDirEntry* e2 = itsFileTreeList->GetDirEntry(cell2.y);

		const JString p1 = e1->GetPath();
		const JString p2 = e2->GetPath();

		if (cell1 == cell2 && e1->IsDirectory())
		{
			path = e1->GetFullName();
		}
		else if (p1 == p2)
		{
			path = p1;
		}
	}

	if (path.IsEmpty())
	{
		path = itsFileTree->GetDirectory();
	}

	return path;
}

/******************************************************************************
 ChangeExecPermission (private)

 ******************************************************************************/

void
FileTreeTable::ChangeExecPermission
	(
	const bool canExec
	)
{
	JTableSelection& s = GetTableSelection();
	JTableSelectionIterator iter(&s);
	JPoint testCell;
	while (iter.Next(&testCell))
	{
		FileTreeNode* node     = itsFileTreeList->GetFileNode(testCell.y);
		const JString fullName = (node->GetDirEntry())->GetFullName();
		if (node->GetDirEntry()->IsFile())
		{
			JString sysCmd("chmod ");
			sysCmd += "a";
			sysCmd += (canExec ? "+" : "-");
			sysCmd += "x ";
			sysCmd += JPrepArgForExec(fullName);
			Exec(sysCmd);
			node->Update(true);
		}
	}
}

/******************************************************************************
 UpdateEditMenu (private)

 ******************************************************************************/

void
FileTreeTable::UpdateEditMenu()
{
	if (!HasFocus())
	{
		return;
	}

	JXTEBase* te = GetEditMenuHandler();

	JIndex index;
	if (te->EditMenuCmdToIndex(JTextEditor::kSelectAllCmd, &index))
	{
		itsEditMenu->EnableItem(index);
	}

	if (GetTableSelection().HasSelection() &&
		te->EditMenuCmdToIndex(JTextEditor::kCopyCmd, &index))
	{
		assert( itsCopyPathCmdIndex == index+1 );

		itsEditMenu->EnableItem(index);
		itsEditMenu->EnableItem(itsCopyPathCmdIndex);
	}
}

/******************************************************************************
 HandleEditMenu (private)

 ******************************************************************************/

void
FileTreeTable::HandleEditMenu
	(
	const JIndex index
	)
{
	if (!HasFocus())
	{
		return;
	}

	ClearIncrementalSearchBuffer();

	JTextEditor::CmdIndex cmd;
	if (!GetEditMenuHandler()->EditMenuIndexToCmd(index, &cmd))
	{
		cmd = JTextEditor::kSeparatorCmd;
	}

	if (cmd == JTextEditor::kCopyCmd)
	{
		CopySelectedFileNames(false);
	}
	else if (index == itsCopyPathCmdIndex)
	{
		CopySelectedFileNames(true);
	}
	else if (cmd == JTextEditor::kSelectAllCmd)
	{
		GetTableSelection().SelectCol(GetNodeColIndex());
	}
}

/******************************************************************************
 CopySelectedFileNames (private)

 ******************************************************************************/

void
FileTreeTable::CopySelectedFileNames
	(
	const bool useFullPath
	)
	const
{
	const JTableSelection& s = GetTableSelection();
	if (s.HasSelection())
	{
		JPtrArray<JString> list(JPtrArrayT::kDeleteAll);

		JTableSelectionIterator iter(&s);
		JPoint cell;
		while (iter.Next(&cell))
		{
			const JDirEntry* entry = itsFileTreeList->GetDirEntry(cell.y);
			if (useFullPath)
			{
				list.Append(entry->GetFullName());
			}
			else
			{
				list.Append(entry->GetName());
			}
		}

		auto* data = jnew JXTextSelection(GetDisplay(), list);
		assert( data != nullptr );

		GetSelectionManager()->SetData(kJXClipboardName, data);
	}
}

/******************************************************************************
 UpdateViewMenu

 ******************************************************************************/

void
FileTreeTable::UpdateViewMenu()
{
	if (itsTreeSet->FilterVisible())
	{
		itsViewMenu->CheckItem(kShowFilterCmd);
	}

	if (itsFileTree->HiddenVisible())
	{
		itsViewMenu->CheckItem(kShowHiddenCmd);
	}

	bool prefs[kTreePrefCount];
	GetPreferences(prefs);
	if (prefs[2])
	{
		itsViewMenu->CheckItem(kShowUserCmd);
	}
	if (prefs[3])
	{
		itsViewMenu->CheckItem(kShowGroupCmd);
	}
	if (prefs[4])
	{
		itsViewMenu->CheckItem(kShowSizeCmd);
	}
	if (prefs[5])
	{
		itsViewMenu->CheckItem(kShowModeCmd);
	}
	if (prefs[6])
	{
		itsViewMenu->CheckItem(kShowDateCmd);
	}
}

/******************************************************************************
 HandleViewMenu

 ******************************************************************************/

void
FileTreeTable::HandleViewMenu
	(
	const JIndex index
	)
{
	ClearIncrementalSearchBuffer();

	if (index == kShowFilterCmd)
	{
		itsTreeSet->ToggleFilter();
	}
	else if (index == kShowHiddenCmd)
	{
		TogglePref(0);
	}

	else if (index == kShowUserCmd)
	{
		TogglePref(2);
	}
	else if (index == kShowGroupCmd)
	{
		TogglePref(3);
	}
	else if (index == kShowSizeCmd)
	{
		TogglePref(4);
	}
	else if (index == kShowModeCmd)
	{
		TogglePref(5);
	}
	else if (index == kShowDateCmd)
	{
		TogglePref(6);
	}

	else if (index == kShowAllAttrCmd)
	{
		bool prefs[kTreePrefCount];
		GetPreferences(prefs);
		for (JIndex i = 2; i < kTreePrefCount; i++)
		{
			prefs[i] = true;
		}
		SetPreferences(prefs);
	}
	else if (index == kHideAllAttrCmd)
	{
		bool prefs[kTreePrefCount];
		GetPreferences(prefs);
		for (JIndex i = 2; i < kTreePrefCount; i++)
		{
			prefs[i] = false;
		}
		SetPreferences(prefs);
	}

	else if (index == kRefreshCmd)
	{
		UpdateDisplay(true);
	}
}

/******************************************************************************
 GetPreferences

 ******************************************************************************/

void
FileTreeTable::GetPreferences
	(
	bool prefs[]
	)
{
	prefs[0] = itsFileTree->HiddenVisible();
//	prefs[1] = itsFileTree->FilesVisible();
	prefs[1] = true;

	for (JIndex i=2; i<kTreePrefCount; i++)
	{
		prefs[i] = itsVisibleCols[i-GetNodeColIndex()];
	}
}

/******************************************************************************
 SetPreferences

 ******************************************************************************/

static const GFMColType kPrefColID[] =
{
	kGFMUID, kGFMGID, kGFMSize, kGFMMode, kGFMDate
};

void
FileTreeTable::SetPreferences
	(
	const bool prefs[]
	)
{
	itsFileTree->ShowHidden(prefs[0]);

	for (JIndex i=2; i<kTreePrefCount; i++)
	{
		const GFMColType type = kPrefColID[i-GetNodeColIndex()];
		if (prefs[i])
		{
			InsertFMTreeCol(type);
		}
		else
		{
			RemoveFMTreeCol(type);
			if (itsCurrentColType == type)
			{
				SetCurrentColIndex(GetNodeColIndex());
			}
		}
	}

	AdjustToTree();
}

/******************************************************************************
 TogglePref (private)

 ******************************************************************************/

void
FileTreeTable::TogglePref
	(
	const JIndex i
	)
{
	bool prefs[kTreePrefCount];
	GetPreferences(prefs);
	prefs[i] = !prefs[i];
	SetPreferences(prefs);
}

/******************************************************************************
 InsertFMTreeCol

 ******************************************************************************/

void
FileTreeTable::InsertFMTreeCol
	(
	const GFMColType type
	)
{
	JIndex		index;
	JCoordinate width;

	if (InsertFMCol(itsVisibleCols, GetNodeColIndex(), type, &index, &width))
	{
		InsertCols(index, 1, width);
	}
}

/******************************************************************************
 RemoveFMTreeCol

 ******************************************************************************/

void
FileTreeTable::RemoveFMTreeCol
	(
	const GFMColType type
	)
{
	JIndex index;

	if (RemoveFMCol(itsVisibleCols, GetNodeColIndex(), type, &index))
	{
		RemoveCol(index);
	}
}

/******************************************************************************
 LoadPrefs

 ******************************************************************************/

void
FileTreeTable::LoadPrefs()
{
	bool prefs[kTreePrefCount];
	GetPrefsMgr()->GetTreePreferences(prefs);
	SetPreferences(prefs);
}

/******************************************************************************
 LoadPrefs

 ******************************************************************************/

void
FileTreeTable::LoadPrefs
	(
	std::istream&			is,
	const JFileVersion	vers
	)
{
	if (vers < 2)
	{
		JFileVersion v;
		is >> v;
	}

	int type;
	is >> type;

	bool prefs[kTreePrefCount];
	if (vers >= 2)
	{
		is >> JBoolFromString(prefs[0]);
	}
	is >> JBoolFromString(prefs[2])
	   >> JBoolFromString(prefs[3])
	   >> JBoolFromString(prefs[4])
	   >> JBoolFromString(prefs[5])
	   >> JBoolFromString(prefs[6]);
	SetPreferences(prefs);

	SetCurrentColType((GFMColType) type);
}

/******************************************************************************
 SavePrefs

 ******************************************************************************/

void
FileTreeTable::SavePrefs
	(
	std::ostream& os
	)
{
	os << ' ' << (int)itsCurrentColType << ' ';

	bool prefs[kTreePrefCount];
	GetPreferences(prefs);
	os << JBoolToString(prefs[0])
	   << JBoolToString(prefs[2])
	   << JBoolToString(prefs[3])
	   << JBoolToString(prefs[4])
	   << JBoolToString(prefs[5])
	   << JBoolToString(prefs[6]);

	os << ' ';
}

/******************************************************************************
 SavePrefsAsDefault

 ******************************************************************************/

void
FileTreeTable::SavePrefsAsDefault()
{
	bool prefs[kTreePrefCount];
	GetPreferences(prefs);
	GetPrefsMgr()->SetTreePreferences(prefs);
	JXWindow* window = GetWindow();
	GetPrefsMgr()->SaveDefaultWindowSize(window->GetFrameWidth(), window->GetFrameHeight());
	itsTreeSet->SaveFilterPref();
}

/******************************************************************************
 RestoreDirState

 ******************************************************************************/

void
FileTreeTable::RestoreDirState
	(
	std::istream&		is,
	const JFileVersion	vers
	)
{
	if (vers < 2)
	{
		JFileVersion v;
		is >> v;
	}

	const JString& basePath = itsFileTree->GetDirectory();

	JSize nameCount;
	is >> nameCount;

	JString file, full;
	for (JIndex i=1; i<=nameCount; i++)
	{
		is >> file;
		if (!JConvertToAbsolutePath(file, basePath, &full))
		{
			continue;
		}

		const JSize count = itsFileTreeList->GetItemCount();	// changes after Open()
		for (JIndex j=1; j<=count; j++)
		{
			const JString& n = (itsFileTreeList->GetDirEntry(j))->GetFullName();
			if (JSameDirEntry(n, full))
			{
				itsFileTreeList->Open(j);
				break;
			}
		}
	}
}

/******************************************************************************
 SaveDirState

 ******************************************************************************/

void
FileTreeTable::SaveDirState
	(
	std::ostream& os
	)
{
	JPtrArray<JString> names(JPtrArrayT::kDeleteAll);
	const JString& basePath = itsFileTree->GetDirectory();

	JSize count = itsFileTreeList->GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		if (itsFileTreeList->IsOpen(i))
		{
			names.Append(JConvertToRelativePath(
				(itsFileTreeList->GetDirEntry(i))->GetFullName(), basePath));
		}
	}

	count = names.GetItemCount();
	os << ' ' << count;

	for (JIndex i=1; i<=count; i++)
	{
		os << ' ' << *(names.GetItem(i));
	}

	os << ' ';
}

/******************************************************************************
 IsCurrentCol

 ******************************************************************************/

bool
FileTreeTable::IsCurrentCol
	(
	const JIndex index
	)
	const
{
	GFMColType type = GetFMColType(index, GetNodeColIndex(), itsVisibleCols);
	return type == itsCurrentColType;
}

/******************************************************************************
 SetCurrentColIndex

 ******************************************************************************/

void
FileTreeTable::SetCurrentColIndex
	(
	const JIndex index
	)
{
	SetCurrentColType(GetFMColType(index, GetNodeColIndex(), itsVisibleCols));
}

/******************************************************************************
 SetCurrentColType

 ******************************************************************************/

void
FileTreeTable::SetCurrentColType
	(
	const GFMColType type
	)
{
	ClearIncrementalSearchBuffer();
	if (type != itsCurrentColType && type != kGFMMode)
	{
		itsCurrentColType = type;
		itsFileTree->SetNodeCompareFunction(type);
	}
}

/******************************************************************************
 InitGitBranchMenus (private)

	Load the data into cache so opening the menu will be quick.

 ******************************************************************************/

static const JString gitBranchCmd("git branch", JString::kNoCopy);
static const JString gitRemoteBranchCmd("git branch -r", JString::kNoCopy);

void
FileTreeTable::InitGitBranchMenus()
{
	pid_t pid;
	JExecute(itsFileTree->GetDirectory(), gitBranchCmd, &pid,
			 kJIgnoreConnection, nullptr,
			 kJTossOutput, nullptr,
			 kJTossOutput, nullptr);

	JExecute(itsFileTree->GetDirectory(), gitRemoteBranchCmd, &pid,
			 kJIgnoreConnection, nullptr,
			 kJTossOutput, nullptr,
			 kJTossOutput, nullptr);
}

/******************************************************************************
 UpdateGitMenus (private)

 ******************************************************************************/

void
FileTreeTable::UpdateGitMenus
	(
	const bool shortcut
	)
{
	if (itsGitProcess == nullptr)
	{
		itsGitMenu->EnableAll();
	}
	else
	{
		itsGitMenu->DisableAll();
	}

	if (shortcut)
	{
		return;
	}

	// fill in sub-menus

	JPtrArray<JString> localList(JPtrArrayT::kDeleteAll);
	JPtrArray<JString> remoteList(JPtrArrayT::kDeleteAll);

	JPtrArray<JString> repoList(JPtrArrayT::kDeleteAll);
	repoList.SetCompareFunction(JCompareStringsCaseInsensitive);

	JXGetApplication()->DisplayBusyCursor();

	JIndex currentIndex;
	const bool hasRemote = GetGitBranches("git branch -r", &remoteList, &currentIndex, &repoList);
	const bool hasLocal  = GetGitBranches("git branch", &localList, &currentIndex, nullptr);	// set currentIndex

	itsGitPullSourceMenu->RemoveAllItems();
	itsGitPushDestMenu->RemoveAllItems();
	itsGitRemoveRemoteMenu->RemoveAllItems();
	itsGitPruneRemoteMenu->RemoveAllItems();

	const JSize repoCount = repoList.GetItemCount();
	for (JIndex i=1; i<=repoCount; i++)
	{
		const JString* s = repoList.GetItem(i);
		itsGitPullSourceMenu->AppendItem(*s);
		itsGitPushDestMenu->AppendItem(*s);
		itsGitRemoveRemoteMenu->AppendItem(*s);
		itsGitPruneRemoteMenu->AppendItem(*s);
	}

	itsGitLocalBranchMenu->RemoveAllItems();
	itsGitMergeBranchMenu->RemoveAllItems();
	itsGitRemoveBranchMenu->RemoveAllItems();
	if (hasLocal)
	{
		const JSize localCount = localList.GetItemCount();
		for (JIndex i=1; i<=localCount; i++)
		{
			const JString* s = localList.GetItem(i);
			itsGitLocalBranchMenu->AppendItem(*s, JXMenu::kRadioType);
			itsGitMergeBranchMenu->AppendItem(*s);
			itsGitRemoveBranchMenu->AppendItem(*s);

			if (*s == itsCurrentGitBranch)
			{
				itsGitLocalBranchMenu->DisableItem(i);
				itsGitMergeBranchMenu->DisableItem(i);
				itsGitRemoveBranchMenu->DisableItem(i);
			}
		}
	}
	else if (!localList.IsEmpty())
	{
		const JString* s = localList.GetFirstItem();
		itsGitLocalBranchMenu->AppendItem(*s);
		itsGitLocalBranchMenu->DisableItem(1);

		itsGitMergeBranchMenu->AppendItem(*s);
		itsGitMergeBranchMenu->DisableItem(1);

		itsGitRemoveBranchMenu->AppendItem(*s);
		itsGitRemoveBranchMenu->DisableItem(1);
	}

	itsGitRemoteBranchMenu->RemoveAllItems();
	if (hasRemote)
	{
		if (!itsGitMergeBranchMenu->IsEmpty())
		{
			itsGitMergeBranchMenu->ShowSeparatorAfter(itsGitMergeBranchMenu->GetItemCount());
		}

		const JSize remoteCount = remoteList.GetItemCount();
		for (JIndex i=1; i<=remoteCount; i++)
		{
			itsGitRemoteBranchMenu->AppendItem(*remoteList.GetItem(i));
			itsGitMergeBranchMenu->AppendItem(*remoteList.GetItem(i));
		}
	}
	else if (!remoteList.IsEmpty())
	{
		itsGitRemoteBranchMenu->AppendItem(*remoteList.GetFirstItem());
		itsGitRemoteBranchMenu->DisableItem(1);
	}

	JPtrArray<JString> idList(JPtrArrayT::kDeleteAll);
	JPtrArray<JString> nameList(JPtrArrayT::kDeleteAll);
	GetGitStashList(&idList, &nameList);

	itsGitStashPopMenu->RemoveAllItems();
	itsGitStashApplyMenu->RemoveAllItems();
	itsGitStashDropMenu->RemoveAllItems();

	const JSize stashCount = idList.GetItemCount();
	for (JIndex i=1; i<=stashCount; i++)
	{
		itsGitStashPopMenu->AppendItem(
			*(idList.GetItem(i)), JXMenu::kPlainType, JString::empty,
			*(nameList.GetItem(i)));

		itsGitStashApplyMenu->AppendItem(
			*(idList.GetItem(i)), JXMenu::kPlainType, JString::empty,
			*(nameList.GetItem(i)));

		itsGitStashDropMenu->AppendItem(
			*(idList.GetItem(i)), JXMenu::kPlainType, JString::empty,
			*(nameList.GetItem(i)));
	}

	if (!itsCurrentGitBranch.IsEmpty())
	{
		const JUtf8Byte* map[] =
		{
			"name", itsCurrentGitBranch.GetBytes()
		};
		const JString msg = JGetString("CurrentGitBranch::FileTreeTable", map, sizeof(map));
		itsGitMenu->SetItemText(kGitSwitchBranchItemIndex, msg);
	}
}

/******************************************************************************
 UpdateGitLocalBranchMenu (private)

 ******************************************************************************/

void
FileTreeTable::UpdateGitLocalBranchMenu()
{
	const JSize count = itsGitLocalBranchMenu->GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		if (itsGitLocalBranchMenu->GetItemText(i) == itsCurrentGitBranch)
		{
			itsGitLocalBranchMenu->CheckItem(i);
			break;
		}
	}
}

/******************************************************************************
 HandleGitMenu (private)

 ******************************************************************************/

void
FileTreeTable::HandleGitMenu
	(
	const JIndex index
	)
{
	if (itsGitProcess != nullptr)
	{
		return;
	}

	if (index == kGitStatusCmd)
	{
		JSimpleProcess::Create(itsFileTree->GetDirectory(),
							   GetApplication()->GetGitStatusCommand(), true);
	}
	else if (index == kGitHistoryCmd)
	{
		JSimpleProcess::Create(itsFileTree->GetDirectory(),
							   GetApplication()->GetGitHistoryCommand(), true);
	}

	else if (index == kGitCommitAllCmd)
	{
		if (getenv("GIT_EDITOR") == nullptr)
		{
			auto* dlog =
				jnew JXGetStringDialog(JGetString("CommitBranchTitle::FileTreeTable"),
									   JGetString("CommitBranchPrompt::FileTreeTable"));
			assert( dlog != nullptr );
			if (dlog->DoDialog())
			{
				CommitGitBranch(dlog->GetString());
			}
		}
		else
		{
			CommitGitBranch(JString::empty);
		}
	}
	else if (index == kGitRevertAllCmd)
	{
		RevertGitBranch();
	}

	else if (index == kGitStashChangesCmd)
	{
		auto* dlog =
			jnew JXGetStringDialog(JGetString("StashTitle::FileTreeTable"),
								   JGetString("StashPrompt::FileTreeTable"));
		assert( dlog != nullptr );
		if (dlog->DoDialog())
		{
			Stash(dlog->GetString());
		}
	}

	else if (index == kGitCreateBranchCmd)
	{
		auto* dlog =
			jnew JXGetStringDialog(JGetString("CreateBranchTitle::FileTreeTable"),
								   JGetString("CreateBranchPrompt::FileTreeTable"));
		assert( dlog != nullptr );
		if (dlog->DoDialog())
		{
			CreateGitBranch(dlog->GetString());
		}
	}

	else if (index == kGitAddRemoteCmd)
	{
		auto* dlog = jnew NewGitRemoteDialog;
		if (dlog->DoDialog())
		{
			AddGitRemote(dlog->GetRepoURL(), dlog->GetLocalName());
		}
	}
}

/******************************************************************************
 CreateGitBranch (private)

 ******************************************************************************/

void
FileTreeTable::CreateGitBranch
	(
	const JString& branchName
	)
{
	JSimpleProcess* p;
	JString cmd("git checkout -b ");
	cmd += JPrepArgForExec(branchName);

	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, false);
	if (err.OK())
	{
		p->WaitUntilFinished();
		if (p->SuccessfulFinish())
		{
			JExecute(itsFileTree->GetDirectory(), (GetApplication())->GetPostCheckoutCommand(), nullptr);
		}
		jdelete p;
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 CommitGitBranch (private)

 ******************************************************************************/

void
FileTreeTable::CommitGitBranch
	(
	const JString& msg
	)
{
	assert( itsGitProcess == nullptr );

	JSimpleProcess* p;
	JString cmd("git commit -a");

	if (!msg.IsEmpty())
	{
		cmd += " -m ";
		cmd += JPrepArgForExec(msg);
	}

	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, true);
	if (err.OK())
	{
		itsGitProcess = p;
		ListenTo(itsGitProcess);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 RevertGitBranch (private)

 ******************************************************************************/

static const JString gitRevertCmd("git reset --hard", JString::kNoCopy);

void
FileTreeTable::RevertGitBranch()
{
	if (!JGetUserNotification()->AskUserNo(JGetString("WarnRevertBranch::FileTreeTable")))
	{
		return;
	}

	JSimpleProcess* p;
	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(),
											  gitRevertCmd, false);
	if (err.OK())
	{
		p->WaitUntilFinished();
		if (p->SuccessfulFinish())
		{
			JExecute(itsFileTree->GetDirectory(), (GetApplication())->GetPostCheckoutCommand(), nullptr);
		}
		jdelete p;
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 GetGitBranches (private)

 ******************************************************************************/

bool
FileTreeTable::GetGitBranches
	(
	const JUtf8Byte*	cmd,
	JPtrArray<JString>*	branchList,
	JIndex*				currentIndex,
	JPtrArray<JString>*	repoList
	)
{
	*currentIndex = 0;

	int fromFD;
	const JError err = JExecute(itsFileTree->GetDirectory(), JString(cmd, JString::kNoCopy), nullptr,
								kJIgnoreConnection, nullptr,
								kJCreatePipe, &fromFD);
	if (!err.OK())
	{
		branchList->Append(JGetString("NoBranchInfo::FileTreeTable"));
		itsCurrentGitBranch.Clear();
		return false;
	}

	JString line, *repo;
	while (true)
	{
		line = JReadUntil(fromFD, '\n');
		if (line.IsEmpty())
		{
			break;
		}
		else if (line.EndsWith("/HEAD") || line.Contains("->"))
		{
			continue;
		}

		const bool current = line.GetFirstCharacter() == '*';

		JStringIterator iter(&line);
		iter.SkipNext(2);
		iter.RemoveAllPrev();

		branchList->Append(line);
		if (current)
		{
			*currentIndex       = branchList->GetItemCount();
			itsCurrentGitBranch = line;
		}

		iter.BeginMatch();
		if (repoList != nullptr && iter.Next("/") && !iter.AtBeginning())
		{
			repo = jnew JString(iter.FinishMatch().GetString());
			assert( repo != nullptr );
			if (!repoList->InsertSorted(repo, false))
			{
				jdelete repo;
			}
		}
	}

	::close(fromFD);
	return !branchList->IsEmpty();
}

/******************************************************************************
 SwitchToGitBranch (private)

 ******************************************************************************/

static const JString gitStashStatusCmd("git statuz -z", JString::kNoCopy);
static const JString gitStashTempCmd("git stash save systemg-temp", JString::kNoCopy);

void
FileTreeTable::SwitchToGitBranch
	(
	const JString& branch
	)
{
	const JString& dir = itsFileTree->GetDirectory();

	// check for changes needing to be stashed

	int fromFD;
	JError err = JExecute(dir, gitStashStatusCmd, nullptr,
						  kJIgnoreConnection, nullptr,
						  kJCreatePipe, &fromFD,
						  kJAttachToFromFD);
	if (err.OK())
	{
		JString msg;
		JReadAll(fromFD, &msg);
		if (!msg.IsEmpty())
		{
			err = JExecute(dir, gitStashTempCmd, nullptr);
		}
	}

	// switch branches

	JString cmd("git checkout ");
	cmd += JPrepArgForExec(branch);

	err = JExecute(dir, cmd, nullptr,
				   kJIgnoreConnection, nullptr,
				   kJCreatePipe, &fromFD,
				   kJAttachToFromFD);
	if (err.OK())
	{
		JString msg;
		JReadAll(fromFD, &msg);
		JGetUserNotification()->DisplayMessage(msg);

		// check for stashed changes

		JPtrArray<JString> idList(JPtrArrayT::kDeleteAll);
		JPtrArray<JString> nameList(JPtrArrayT::kDeleteAll);
		if (GetGitStashList(&idList, &nameList))
		{
			const JUtf8Byte* map[] =
		{
				"branch", branch.GetBytes(),
				"stash",  kStashDisplaySuffix.GetBytes()
		};
			const JString name = JGetString("BranchIndicator::FileTreeTable", map, sizeof(map));

			const JSize count = nameList.GetItemCount();
			for (JIndex i=1; i<=count; i++)
			{
				if (name == *(nameList.GetItem(i)))
				{
					Unstash("pop", *(idList.GetItem(i)));
					break;
				}
			}
		}

		JExecute(dir, (GetApplication())->GetPostCheckoutCommand(), nullptr);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 MergeFromGitBranch (private)

 ******************************************************************************/

void
FileTreeTable::MergeFromGitBranch
	(
	const JString& branch
	)
{
	JString cmd(
		"xterm -T 'Merge from $branch' "
			"-e '( git merge --no-commit $branch ) | less'");

	const JString branchArg = JPrepArgForExec(branch);

	JSubstitute subst;
	subst.DefineVariable("branch", branchArg);
	subst.Substitute(&cmd);

	JSimpleProcess* p;
	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, true);
	if (err.OK())
	{
		itsGitProcess = p;
		ListenTo(itsGitProcess);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 FetchRemoteGitBranch (private)

 ******************************************************************************/

void
FileTreeTable::FetchRemoteGitBranch
	(
	const JString& branch
	)
{
	assert( itsGitProcess == nullptr );

	JString name = branch;

	JStringIterator iter(&name, JStringIterator::kStartAtEnd);
	if (iter.Prev("/"))
	{
		iter.SkipNext();
		iter.RemoveAllPrev();
	}
	iter.Invalidate();

	auto* dlog =
		jnew JXGetStringDialog(JGetString("FetchBranchTitle::FileTreeTable"),
							   JGetString("FetchBranchPrompt::FileTreeTable"), name);
	assert( dlog != nullptr );
	if (dlog->DoDialog())
	{
		assert( itsGitProcess == nullptr );

		JString cmd(
			"xterm -T 'Fetch branch $name' "
				"-e '( git checkout -b $name $branch ) | less'");

		const JString nameArg   = JPrepArgForExec(dlog->GetString());
		const JString branchArg = JPrepArgForExec(branch);

		JSubstitute subst;
		subst.DefineVariable("name", nameArg);
		subst.DefineVariable("branch", branchArg);
		subst.Substitute(&cmd);

		JSimpleProcess* p;
		const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, true);
		if (err.OK())
		{
			itsGitProcess = p;
			ListenTo(itsGitProcess);
		}
		else
		{
			err.ReportIfError();
		}
	}
}

/******************************************************************************
 PullBranch (private)

 ******************************************************************************/

void
FileTreeTable::PullBranch
	(
	const JString& repo
	)
{
	assert( itsGitProcess == nullptr );

	JString cmd(
		"xterm -T 'Pull from $repo' "
			"-e '( git fetch $repo -v ; git merge $repo/$branch ) | less'");

	const JString repoArg   = JPrepArgForExec(repo);
	const JString branchArg = JPrepArgForExec(itsCurrentGitBranch);

	JSubstitute subst;
	subst.DefineVariable("repo", repoArg);
	subst.DefineVariable("branch", branchArg);
	subst.Substitute(&cmd);

	JSimpleProcess* p;
	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, true);
	if (err.OK())
	{
		itsGitProcess = p;
		ListenTo(itsGitProcess);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 PushBranch (private)

 ******************************************************************************/

void
FileTreeTable::PushBranch
	(
	const JString& repo
	)
{
	JString cmd(
		"xterm -T 'Push to $repo' "
			"-e 'git push $repo HEAD | less'");

	const JString repoArg = JPrepArgForExec(repo);

	JSubstitute subst;
	subst.DefineVariable("repo", repoArg);
	subst.Substitute(&cmd);

	JSimpleProcess::Create(itsFileTree->GetDirectory(), cmd, true);
}

/******************************************************************************
 RemoveGitBranch (private)

 ******************************************************************************/

bool
FileTreeTable::RemoveGitBranch
	(
	const JString&	branch,
	const bool	force
	)
{
	if (!force)
	{
		const JUtf8Byte* map[] =
		{
			"name", branch.GetBytes()
		};
		const JString msg = JGetString("WarnRemoveBranch::FileTreeTable", map, sizeof(map));
		if (!JGetUserNotification()->AskUserNo(msg))
		{
			return false;
		}
	}

	JString cmd("git branch -D ");
	cmd += JPrepArgForExec(branch);

	JSimpleProcess* p;
	if (!JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, false).OK())
	{
		return false;
	}

	p->WaitUntilFinished();
	jdelete p;
	p = nullptr;

	// drop systemg-temp stash

	JPtrArray<JString> idList(JPtrArrayT::kDeleteAll);
	JPtrArray<JString> nameList(JPtrArrayT::kDeleteAll);
	GetGitStashList(&idList, &nameList);

	JString stashId;
	if (FindGitStash(branch, idList, nameList, &stashId))
	{
		cmd  = "git stash drop ";
		cmd += JPrepArgForExec(stashId);

		if (JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, false).OK())
		{
			p->WaitUntilFinished();
			jdelete p;
		}
	}

	return true;
}

/******************************************************************************
 FindGitStash (static private)

 ******************************************************************************/

bool
FileTreeTable::FindGitStash
	(
	const JString&				branchName,
	const JPtrArray<JString>&	idList,
	const JPtrArray<JString>&	nameList,
	JString*					id
	)
{
	const JString s = branchName + kStashDisplaySuffix;

	const JSize count = idList.GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		if (nameList.GetItem(i)->EndsWith(s))
		{
			*id = *idList.GetItem(i);
			return true;
		}
	}

	id->Clear();
	return false;
}

/******************************************************************************
 GetGitStashList (private)

 ******************************************************************************/

static const JString gitListStashesCmd("git stash list --pretty=format:'%gd;%s'", JString::kNoCopy);

bool
FileTreeTable::GetGitStashList
	(
	JPtrArray<JString>* idList,
	JPtrArray<JString>* nameList
	)
{
	int fromFD;
	const JError err = JExecute(itsFileTree->GetDirectory(),
								gitListStashesCmd, nullptr,
								kJIgnoreConnection, nullptr,
								kJCreatePipe, &fromFD);
	if (!err.OK())
	{
		nameList->Append(JGetString("NoStashes::FileTreeTable"));
		idList->Append(JString::empty);
		return false;
	}

	JString line;
	while (true)
	{
		line = JReadUntil(fromFD, '\n');
		if (line.IsEmpty())
		{
			break;
		}

		JStringIterator iter(&line);
		iter.BeginMatch();
		if (iter.Next(";") && !iter.AtBeginning() && !iter.AtEnd())
		{
			idList->Append(iter.FinishMatch().GetString());
			iter.RemoveAllPrev();
			nameList->Append(line);
		}
	}

	::close(fromFD);
	return !idList->IsEmpty();
}

/******************************************************************************
 Stash (private)

 ******************************************************************************/

void
FileTreeTable::Stash
	(
	const JString& name
	)
{
	const JString cmd = "git stash save " + JPrepArgForExec(name);

	JSimpleProcess* p;
	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, true);
	if (err.OK())
	{
		itsGitProcess = p;
		ListenTo(itsGitProcess);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 Unstash (private)

 ******************************************************************************/

void
FileTreeTable::Unstash
	(
	const JUtf8Byte*	action,
	const JString&		stashId
	)
{
	JString cmd(
		"xterm -T '$action stashed changes' "
			"-e '( git stash $action \\\\'$ref\\\\' ) | less'");

	const JString refArg = JPrepArgForExec(stashId);

	JSubstitute subst;
	subst.DefineVariable("action", JString(action, JString::kNoCopy));
	subst.DefineVariable("ref", refArg);
	subst.Substitute(&cmd);

	JSimpleProcess* p;
	const JError err = JSimpleProcess::Create(&p, itsFileTree->GetDirectory(), cmd, true);
	if (err.OK())
	{
		itsGitProcess = p;
		ListenTo(itsGitProcess);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 AddGitRemote (private)

 ******************************************************************************/

void
FileTreeTable::AddGitRemote
	(
	const JString& repoURL,
	const JString& name
	)
{
	JString cmd("git remote add -f ");
	cmd += JPrepArgForExec(name);
	cmd += " ";
	cmd += JPrepArgForExec(repoURL);

	JSimpleProcess::Create(itsFileTree->GetDirectory(), cmd, true);
}

/******************************************************************************
 RemoveGitRemote (private)

 ******************************************************************************/

void
FileTreeTable::RemoveGitRemote
	(
	const JString& name
	)
{
	const JUtf8Byte* map[] =
	{
		"name", name.GetBytes()
	};
	const JString msg = JGetString("WarnRemoveRemote::FileTreeTable", map, sizeof(map));
	if (!JGetUserNotification()->AskUserNo(msg))
	{
		return;
	}

	JString cmd("git remote rm ");
	cmd += JPrepArgForExec(name);

	JSimpleProcess::Create(itsFileTree->GetDirectory(), cmd, true);
}

/******************************************************************************
 PruneRemoteGitBranches (private)

 ******************************************************************************/

static const JRegex prunedBranchPattern = "^\\s*\\*\\s*\\[.+?\\]\\s*.+?/(.+)$";

void
FileTreeTable::PruneRemoteGitBranches
	(
	const JString& name
	)
{
	const JString nameArg = JPrepArgForExec(name);

	JSubstitute subst;
	subst.DefineVariable("name", nameArg);

	// get names of branches that will be pruned

	JString cmd("git remote prune -n $name");
	subst.Substitute(&cmd);

	int fromFD;
	const JError err = JExecute(itsFileTree->GetDirectory(), cmd, nullptr,
								kJIgnoreConnection, nullptr,
								kJCreatePipe, &fromFD,
								kJAttachToFromFD);

	JString s;
	JReadAll(fromFD, &s);

	JPtrArray<JString> branchList(JPtrArrayT::kDeleteAll);

	JStringIterator iter(s);
	while (iter.Next(prunedBranchPattern))
	{
		branchList.Append(iter.GetLastMatch().GetSubstring(1));
	}
	iter.Invalidate();

	// prune the branches

	cmd = "xterm -T 'Prune $name' "
			"-e 'git remote prune $name | less'";

	subst.Substitute(&cmd);

	JSimpleProcess::Create(itsFileTree->GetDirectory(), cmd, true);

	// check if we have local versions

	if (!branchList.IsEmpty())
	{
		JPtrArray<JString> localList(JPtrArrayT::kDeleteAll);
		JIndex i;
		GetGitBranches("git branch", &localList, &i, nullptr);

		const JString* currentBranch = localList.GetItem(i);	// before sorting

		localList.SetCompareFunction(JCompareStringsCaseInsensitive);
		localList.Sort();

		const JSize branchCount = branchList.GetItemCount();
		for (JIndex i=branchCount; i>=1; i--)
		{
			JString* branch = branchList.GetItem(i);

			JIndex j;
			if (!localList.SearchSorted(branch, JListT::kAnyMatch, &j) ||
				*branch == *currentBranch)
			{
				branchList.DeleteItem(i);
			}
		}
	}

	// ask if should delete local versions

	if (!branchList.IsEmpty())
	{
		auto* dlog =
			jnew JXCheckboxListDialog(JGetString("DeletePrunedBranchesTitle::FileTreeTable"),
									  JGetString("DeletePrunedBranchesPrompt::FileTreeTable"),
									  branchList);
		assert( dlog != nullptr );
		dlog->SelectAllItems();
		JArray<JIndex> indexList;
		if (dlog->DoDialog() && dlog->GetSelectedItems(&indexList))
		{
			for (auto i : indexList)
			{
				RemoveGitBranch(*branchList.GetItem(i), true);
			}
		}
	}
}

/******************************************************************************
 UpdateShortcutMenu (private)

 ******************************************************************************/

void
FileTreeTable::UpdateShortcutMenu()
{
	const JSize count = itsShortcutsMenu->GetItemCount();
	for (JIndex i=count; i>kShortcutCmdOffset; i--)
	{
		itsShortcutsMenu->RemoveItem(i);
	}

	StopListening(GetApplication());		// avoid double update
	GetApplication()->UpdateShortcutMenu(itsShortcutsMenu);
	ListenTo(GetApplication());
}

/******************************************************************************
 HandleShortcutMenu (private)

 ******************************************************************************/

void
FileTreeTable::HandleShortcutMenu
	(
	const JIndex index
	)
{
	ClearIncrementalSearchBuffer();

	if (index == kAddShortcutCmd)
	{
		GetApplication()->AddShortcut(itsFileTree->GetDirectory());
	}
	else if (index == kRemoveShortcutCmd)
	{
		GetApplication()->RemoveShortcut(itsFileTree->GetDirectory());
	}
	else if (index == kRemoveAllShortcutsCmd)
	{
		GetApplication()->RemoveAllShortcuts();
	}

	else
	{
		GetApplication()->OpenShortcut(index - kShortcutCmdOffset);
	}
}

/******************************************************************************
 UpdateContextMenu (private)

 ******************************************************************************/

void
FileTreeTable::UpdateContextMenu()
{
	JTableSelection& s          = GetTableSelection();
	const bool hasSelection = s.HasSelection();

	itsContextMenu->SetItemEnabled(kRunOnSelCtxCmd,  hasSelection);
	itsContextMenu->SetItemEnabled(kDuplicateCtxCmd, hasSelection);

	// symbolic links

	bool findOriginal = false;
	if (hasSelection)
	{
		JTableSelectionIterator iter(&s);
		JPoint cell;
		while (iter.Next(&cell))
		{
			if ((itsFileTreeList->GetDirEntry(cell.y))->IsWorkingLink())
			{
				findOriginal = true;
			}
		}
	}

	itsContextMenu->SetItemEnabled(kMakeAliasCtxCmd,        hasSelection);
	itsContextMenu->SetItemEnabled(kFindOriginalCtxCmd,     findOriginal);
	itsContextMenu->SetItemEnabled(kOpenCloseCtxCmd,        hasSelection);
	itsContextMenu->SetItemEnabled(OpenIconifyCtxCmd,       hasSelection);
	itsContextMenu->SetItemEnabled(kConvertToFileCtxCmd,    hasSelection);
	itsContextMenu->SetItemEnabled(kConvertToProgramCtxCmd, hasSelection);
}

/******************************************************************************
 HandleContextMenu (private)

 ******************************************************************************/

void
FileTreeTable::HandleContextMenu
	(
	const JIndex index
	)
{
	ClearIncrementalSearchBuffer();

	if (index == kDuplicateCtxCmd)
	{
		DuplicateSelected();
	}
	else if (index == kMakeAliasCtxCmd)
	{
		MakeLinks();
	}
	else if (index == kFindOriginalCtxCmd)
	{
		FindOriginals();
	}

	else if (index == kOpenCloseCtxCmd)
	{
		OpenSelection(false, false, false, true);
	}
	else if (index == OpenIconifyCtxCmd)
	{
		OpenSelection(false, false, true, false);
	}

	else if (index == kRunOnSelCtxCmd)
	{
		OpenSelection(false, true, false, false);
	}

	else if (index == kConvertToFileCtxCmd)
	{
		ChangeExecPermission(false);
	}
	else if (index == kConvertToProgramCtxCmd)
	{
		ChangeExecPermission(true);
	}
}

/******************************************************************************
 CreateTreeListInput (virtual protected)

	Derived class can override this to instantiate a derived class of
	JXInputField.

 ******************************************************************************/

JXInputField*
FileTreeTable::CreateTreeListInput
	(
	const JPoint&		cell,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
{
	JTableSelection& s = GetTableSelection();
	s.ClearSelection();
	s.SelectCell(cell);

	JXInputField* obj = jnew JXSaveFileInput(enclosure, hSizing, vSizing, x,y, w,h);
	return obj;
}

/******************************************************************************
 ExtractInputData (virtual protected)

 ******************************************************************************/

bool
FileTreeTable::ExtractInputData
	(
	const JPoint& cell
	)
{
	JXInputField* input;
	const bool ok = GetXInputField(&input);
	assert( ok );

	if (input->GetText()->IsEmpty())
	{
		return true;		// treat as cancel
	}

	FileTreeNode* node    = itsFileTreeList->GetFileNode(cell.y);
	const JString newName = input->GetText()->GetText();	// copy since need after input field gone
	const bool sort       = (GetDisplay()->GetLatestButtonStates()).AllOff();
	const JError err      = node->Rename(newName, sort);
	input                 = nullptr;				// nodes sorted => CancelEditing()
	if (!err.OK())
	{
		JGetStringManager()->ReportError("RenameError::FileTreeTable", err);
	}
	else if (sort)
	{
		ScrollToNode(node);
	}
	else
	{
		itsSortNode = node->GetFileParent();
		ClearWhenGoingAway(itsSortNode, &itsSortNode);
	}
	return err.OK();
}

/******************************************************************************
 AdjustToTree (virtual protected)

 ******************************************************************************/

void
FileTreeTable::AdjustToTree()
{
	const JFontStyle workingLink(false, true, 0, false);
	const JFontStyle brokenLink(false, true, 0, false, JColorManager::GetGrayColor(60));

	const JSize count = GetRowCount();
	for (JIndex i=1; i<=count; i++)
	{
		const JDirEntry* entry = itsFileTreeList->GetDirEntry(i);
		if (entry->IsWorkingLink())
		{
			SetCellStyle(JPoint(GetNodeColIndex(), i), workingLink);
		}
		else if (entry->IsBrokenLink())
		{
			SetCellStyle(JPoint(GetNodeColIndex(), i), brokenLink);
		}
	}

	JXNamedTreeListWidget::AdjustToTree();
}

/******************************************************************************
 GetMinCellWidth (virtual protected)

 ******************************************************************************/

JSize
FileTreeTable::GetMinCellWidth
	(
	const JPoint& cell
	)
	const
{
	if (JIndex(cell.x) > GetNodeColIndex())
	{
		const JString str = GetCellString(cell);

		const GFMColType type       = GetFMColType(cell.x, GetNodeColIndex(), itsVisibleCols);
		const JCoordinate theBuffer = JLAbs(GetFMBufferWidth(itsVisibleCols, GetNodeColIndex(), cell.x));
		JSize w;
		if (type == kGFMMode)
		{
			w = theBuffer + 9 * itsPermCharWidth;
		}
		else
		{
			w = 2 * theBuffer + GetFont().GetStringWidth(GetFontManager(), str);
		}

		const JSize wT = 2 * theBuffer + GetFont().GetStringWidth(GetFontManager(), GetColTitle(cell.x));
		return JMax(wT, w);
	}
	else
	{
		return JXNamedTreeListWidget::GetMinCellWidth(cell);
	}
}

/******************************************************************************
 GetBufferWidth

 ******************************************************************************/

JCoordinate
FileTreeTable::GetBufferWidth
	(
	const JIndex index
	)
	const
{
	if (index == GetNodeColIndex())
	{
		return (GetRowCount() == 0 ? 0 : GetImageWidth(1));
	}
	else
	{
		return GetFMBufferWidth(itsVisibleCols, GetNodeColIndex(), index);
	}
}

/******************************************************************************
 GetColTitle

 ******************************************************************************/

JString
FileTreeTable::GetColTitle
	(
	const JIndex index
	)
	const
{
	JString str;
	if (index == GetNodeColIndex())
	{
		str = JGetString("NameColumnTitle::ColUtils");
	}
	else
	{
		GetFMColTitle(itsVisibleCols, GetNodeColIndex(), index, &str);
	}
	return str;
}
