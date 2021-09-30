/******************************************************************************
 actionDefs.h

	Shared actions for use as menu item ID's and in keybinding tables.
	These preprocessor definitions allow them to be included in static
	menu definitions.

	Copyright (C) 1999 by John Lindal.

 ******************************************************************************/

#ifndef _H_actionDefs
#define _H_actionDefs

#include <jx-af/jx/jXActionDefs.h>	// for convenience

// File menu

#define kNewFolderAction      "SGNewFolder"
#define kNewTextFileAction    "SGNewTextFile"
#define kOpenAction           "SGOpen"
#define kOpenAndCloseAction   "SGOpenAndClose"
#define kOpenAndIconifyAction "SGOpenAndIconify"
#define kAlternateOpenAction  "SGAlternateOpen"
#define kRunOnFileAction      "SGRunOnFile"
#define kFindAction           "SGFind"
#define kRenameAction         "SGRename"
#define kDuplicateAction      "SGDuplicate"
#define kMakeAliasAction      "SGMakeAlias"
#define kConvertToFile        "SGConvertToFile"
#define kConvertToProgram     "SGConvertToProgram"
#define kFindOriginalAction   "SGFindOriginal"
#define kMountAction          "SGMount"
#define kEraseAction          "SGErase"
#define kOpenHomeWindowAction "SGOpenHomeWindow"
#define kViewManPageAction    "SGViewManPage"
#define kOpenTerminalAction   "SGOpenTerminal"
#define kRunCommandAction     "SGRunCommand"

// Edit menu

#define kCopyWithPathAction "SGCopyWithPath"

// View menu

#define kShowFilterAction      "SGShowFilter"
#define kShowHiddenAction      "SGShowHidden"
#define kShowUserAction        "SGShowUser"
#define kShowGroupAction       "SGShowGroup"
#define kShowSizeAction        "SGShowSize"
#define kShowPermissionsAction "SGShowPermissions"
#define kShowDateAction        "SGShowDate"
#define kShowAllAction         "SGShowAll"
#define kShowNoneAction        "SGShowNone"
#define kRefreshAction         "SGRefresh"

// Git menu

#define kGitStatusAction       "SGGitStatus"
#define kGitHistoryAction      "SGGitHistory"
#define kGitCommitAllAction    "SGGitCommitAll"
#define kGitRevertAllAction    "SGGitRevertAll"
#define kGitStashChangesAction "SGGitStashChanges"
#define kGitCreateBranchAction "SGGitCreateBranch"
#define kGitAddRemoteAction    "SGGitAddRemote"
#define kGitPruneRemoteAction  "SGGitPruneRemote"

// Shortcuts menu

#define kAddShortcutAction        "SGAddShortcut"
#define kRemoveShortcutAction     "SGRemoveShortcut"
#define kRemoveAllShortcutsAction "SGRemoveAllShortcuts"

// Windows menu

#define kBringWindowsToFrontAction  "SGBringWindowsToFront"
#define kCloseAllOtherWindowsAction "SGCloseAllOtherWindows"

#endif
