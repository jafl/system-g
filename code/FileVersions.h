/******************************************************************************
 FileVersions.h

	Version information for unstructured data files

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_FileVersions
#define _H_FileVersions

#include <jx-af/jcore/jTypes.h>

const JFileVersion kCurrentDirSetupVersion = 4;

// version 4:
//	Saves relative paths of open directories.
// version 3:
//	Saves scrollbar setup.
// version 2:
//	Unified version numbers of TreeDir and its contents.
//	By using single version number, contents can't have higher version number and fail.
//	Removed HiddenEntriesVisible() from TreeSet.
//	FileTreeTable saves pref[0].

const JFileVersion kCurrentPrefsFileVersion = 11;

// version 11:
//	Added WillSaveFolderPrefs().
// version 10:
//	Added WillOpenNewWindows().
// version 9:
//	Added git status cmd to Application.
// version 8:
//	ID = 11 (terminal cmd) merged with App prefs.
//	ID = 12 (man page cmd) merged with ViewManPageDialog prefs.
// version 7:
//	Removed ID = 6, 8, 9, 16, 17 (all unused)
// version 6:
//	Removed ID = 13, 14, 15, 20.
//	Added DelShouldDelete (ID=13).
// version 5:
//	Converted kSShortcutStringsID (3) to kSAppID (3).

#endif
