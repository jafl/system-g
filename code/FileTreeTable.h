/******************************************************************************
 FileTreeTable.h

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_FileTreeTable
#define _H_FileTreeTable

#include <jx-af/jx/JXNamedTreeListWidget.h>
#include "FileTreeNode.h"
#include "ColUtils.h"
#include <jx-af/jx/JXCursor.h>

class JProcess;
class JXMenuBar;
class JXTextMenu;
class JXFSDirMenu;
class JXToolBar;
class JXFunctionTask;
class JXWindowIcon;
class TreeSet;
class FileTreeList;
class FileTree;
class TrashButton;
class BeginEditingTask;

class FileTreeTable : public JXNamedTreeListWidget
{
	friend class BeginEditingTask;

public:

	FileTreeTable(JXMenuBar* menuBar,
					FileTree* fileTree, FileTreeList* treeList,
					TreeSet* treeSet, TrashButton* trashButton,
					JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);

	~FileTreeTable() override;

	FileTree*		GetFileTree() const;
	FileTreeList*	GetFileTreeList() const;

	void	CreateNewDirectory();
	void	CreateNewTextFile();
	void	DuplicateSelected();
	void	DeleteSelected();
	void	MakeLinks();
	void	FindOriginals();

	void	ViewManPage();
	void	OpenTerminal();
	void	RunCommand();
	void	FormatDisk();

	void	UpdateDisplay(const bool force = false);
	void	SetWindowIcon();
	void	GoUp(const bool sameWindow);
	void	GoTo(const JString& path, const bool sameWindow);

	bool	SelectName(const JString& name, const FileTreeNode* parent,
					   JPoint* cell,
					   const bool updateContent = true,
					   const bool updateView = true);
	bool	SelectName(const JPtrArray<JString>& pathList,
					   const JString& name, JPoint* cell,
					   const bool clearSelection = true,
					   const bool updateContent = true);

	void	LoadToolBarDefaults(JXToolBar* toolBar);

	void	LoadPrefs();
	void	LoadPrefs(std::istream& is, const JFileVersion vers);
	void	SavePrefs(std::ostream& os);
	void	SavePrefsAsDefault();

	void	RestoreDirState(std::istream& is, const JFileVersion vers);
	void	SaveDirState(std::ostream& os);

	JString		GetColTitle(const JIndex index) const;
	JCoordinate	GetBufferWidth(const JIndex index) const;
	bool		IsCurrentCol(const JIndex index) const;
	void		SetCurrentColIndex(const JIndex index);
	void		SetCurrentColType(const GFMColType type);

	bool		IsEditable(const JPoint& cell) const override;
	void		HandleKeyPress(const JUtf8Character& c, const int keySym,
							   const JXKeyModifiers& modifiers) override;
	void		HandleShortcut(const int key, const JXKeyModifiers& modifiers) override;

	static Atom	GetDNDAction(const JXWidget* source, const JString& sourcePath,
							 const JXContainer* target, const JXKeyModifiers& modifiers);

protected:

	void	AdjustToTree() override;
	JSize	GetMinCellWidth(const JPoint& cell) const override;
	void	TableDrawCell(JPainter &p, const JPoint& cell, const JRect& rect) override;
	bool	GetImage(const JIndex index, const JXImage** image) const override;

	void	HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers) override;
	void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
							const JSize clickCount,
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers) override;
	void	HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers) override;
	void	HandleMouseUp(const JPoint& pt, const JXMouseButton button,
						  const JXButtonStates& buttonStates,
						  const JXKeyModifiers& modifiers) override;

	bool	WillAcceptDrop(const JArray<Atom>& typeList, Atom* action,
						   const JPoint& pt, const Time time,
						   const JXWidget* source) override;
	void		HandleDNDEnter() override;
	void		HandleDNDHere(const JPoint& pt, const JXWidget* source) override;
	void		HandleDNDLeave() override;
	void		HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList,
							  const Atom action, const Time time,
							  const JXWidget* source) override;

	void		GetSelectionData(JXSelectionData* data,
								 const JString& id) override;
	Atom		GetDNDAction(const JXContainer* target,
							 const JXButtonStates& buttonStates,
							 const JXKeyModifiers& modifiers) override;
	void		GetDNDAskActions(const JXButtonStates& buttonStates,
								 const JXKeyModifiers& modifiers,
								 JArray<Atom>* askActionList,
								 JPtrArray<JString>* askDescriptionList) override;
	void		HandleDNDResponse(const JXContainer* target,
								  const bool dropAccepted, const Atom action) override;

	JXInputField*
		CreateTreeListInput(const JPoint& cell, JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h) override;

	bool	ExtractInputData(const JPoint& cell) override;

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveWithFeedback(JBroadcaster* sender, Message* message) override;

private:

	FileTree*			itsFileTree;
	FileTreeList*		itsFileTreeList;	// owned by base class
	TreeSet*			itsTreeSet;
	TrashButton*		itsTrashButton;
	JXFunctionTask*		itsUpdateTask;
	JFSFileTreeNodeBase*itsUpdateNode;

	bool				itsVisibleCols[5];
	GFMColType			itsCurrentColType;
	JColorID			itsAltRowColor;
	JCoordinate			itsPermCharWidth;
	bool				itsIgnoreExecPermFlag;

	JXMenuBar*			itsMenuBar;
	JXTextMenu*			itsFileMenu;
	JXFSDirMenu*		itsRecentFilesMenu;
	JXTextMenu*			itsEditMenu;				// not owned
	JIndex				itsCopyPathCmdIndex;
	JXTextMenu*			itsGitMenu;
	JXTextMenu*			itsGitLocalBranchMenu;
	JXTextMenu*			itsGitPullSourceMenu;
	JXTextMenu*			itsGitPushDestMenu;
	JXTextMenu*			itsGitMergeBranchMenu;
	JXTextMenu*			itsGitStashPopMenu;
	JXTextMenu*			itsGitStashApplyMenu;
	JXTextMenu*			itsGitStashDropMenu;
	JXTextMenu*			itsGitRemoteBranchMenu;
	JXTextMenu*			itsGitRemoveBranchMenu;
	JXTextMenu*			itsGitRemoveRemoteMenu;
	JXTextMenu*			itsGitPruneRemoteMenu;
	JXTextMenu* 		itsViewMenu;
	JXTextMenu*			itsShortcutMenu;

	JXTextMenu*			itsContextMenu;

	JProcess*			itsFormatProcess;			// nullptr unless formatting
	JProcess*			itsGitProcess;				// nullptr unless waiting for git

	JPtrArray<JString>*		itsPruneBranchList;		// nullptr unless pruning local branches

	// Drag-and-Drop

	JXWindowIcon*	itsIconWidget;			// not owned
	JIndex			itsWindowIconType;

	enum CursorType
	{
		kDNDFileCursor,
		kDNDDirCursor,
		kDNDMixedCursor
	};

	CursorType	itsDNDCursorType;

	// used during dragging

	JPoint		itsStartPt;
	JPoint		itsPrevPt;
	JSize		itsLastClickCount;
	bool	itsWaitingForDragFlag;
	bool	itsClearIfNotDNDFlag;

	// delayed editing

	bool				itsWaitingToEditFlag;
	BeginEditingTask*	itsEditTask;	// nullptr unless waiting to edit
	JPoint					itsEditCell;
	FileTreeNode*		itsSortNode;	// sort when mouse released

	// set by GetGitBranches()

	JString	itsCurrentGitBranch;

private:

	void	UpdateInfo();
	void	UpdateMenus();

	JString	GetCellString(const JPoint& cell) const;
	void	StartDragRect(const JPoint& pt, const JXMouseButton button,
						  const JXKeyModifiers& modifiers);

	void	UpdateFileMenu();
	void	HandleFileMenu(const JIndex index);

	FileTreeNode*	PrepareToCreateNew();

	void	OpenSelection(const bool alternate, const bool alwaysRunCmd,
						  const bool iconifyAfter, const bool closeAfter);

	void	MakeLinkToFile(const JString& src, const FileTreeNode* parentNode,
						   const bool allowRelative);
	JString	GetCommandPath() const;

	void	ChangeExecPermission(const bool canExec);

	void	UpdateEditMenu();
	void	HandleEditMenu(const JIndex item);

	void	CopySelectedFileNames(const bool useFullPath) const;

	void	InitGitBranchMenus();
	void	UpdateGitMenus(const bool shortcut);
	void	UpdateGitLocalBranchMenu();
	void	HandleGitMenu(const JIndex index);

	bool	GetGitBranches(const JUtf8Byte* cmd,
						   JPtrArray<JString>* branchList, JIndex* currentIndex,
						   JPtrArray<JString>* repoList);
	void	CreateGitBranch(const JString& branchName);
	void	CommitGitBranch(const JString& msg);
	void	RevertGitBranch();

	void	SwitchToGitBranch(const JString& branch);
	void	MergeFromGitBranch(const JString& branch);
	void	FetchRemoteGitBranch(const JString& branch);
	void	PullBranch(const JString& repo);
	void	PushBranch(const JString& repo);
	bool	RemoveGitBranch(const JString& branch, const bool force = false);

	bool	GetGitStashList(JPtrArray<JString>* idList, JPtrArray<JString>* nameList);
	void	Stash(const JString& name);
	void	Unstash(const JUtf8Byte* action, const JString& stashId);

	static bool	FindGitStash(const JString& branchName,
								 const JPtrArray<JString>& idList,
								 const JPtrArray<JString>& nameList,
								 JString* id);

	void	AddGitRemote(const JString& repoURL, const JString& name);
	void	RemoveGitRemote(const JString& repo);
	void	PruneRemoteGitBranches(const JString& name);

	void	UpdateViewMenu();
	void	HandleViewMenu(const JIndex index);

	void	InsertFMTreeCol(const GFMColType type);
	void	RemoveFMTreeCol(const GFMColType type);

	void	SetPreferences(const bool prefs[]);
	void	GetPreferences(bool prefs[]);
	void	TogglePref(const JIndex i);

	void	UpdateShortcutMenu();
	void	HandleShortcutMenu(const JIndex index);

	void	UpdateContextMenu();
	void	HandleContextMenu(const JIndex index);

	void	ChooseDNDCursors();
	bool	GetTrueDropAction(Atom* action);

	JIndex	GetNearestDirIndex(const JIndex index, const bool requireWritable);
	bool	WarnForDelete() const;
};


/******************************************************************************
 GetFileTree

 ******************************************************************************/

inline FileTree*
FileTreeTable::GetFileTree()
	const
{
	return itsFileTree;
}

/******************************************************************************
 GetFileTreeList

 ******************************************************************************/

inline FileTreeList*
FileTreeTable::GetFileTreeList()
	const
{
	return itsFileTreeList;
}

#endif
