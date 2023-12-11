// This file was automatically generated by jx_menu_editor.
// Do not edit it directly!
// Any changes you make will be silently overwritten.

#ifndef _H_FileTreeTable_File
#define _H_FileTreeTable_File

static const JUtf8Byte* kFileMenuStr =
"* %i NewFolder::FileTreeTable"
"|* %i NewTextFile::FileTreeTable"
"|* %i Open::FileTreeTable"
"|* %i OpenAndClose::FileTreeTable"
"|* %i OpenAndIconify::FileTreeTable"
"|* %i AlternateOpen::FileTreeTable"
"|* %i RunOnFile::FileTreeTable %l"
"|* %i Find::FileTreeTable"
"|* %i __OpenRecent::FileTreeTable %l"
"|* %i Rename::FileTreeTable"
"|* %i ConvertToFile::FileTreeTable"
"|* %i ConvertToProgram::FileTreeTable %l"
"|* %i Duplicate::FileTreeTable"
"|* %i MakeAlias::FileTreeTable"
"|* %i FindOriginal::FileTreeTable %l"
"|* %i Mount::FileTreeTable"
"|* %i Erase::FileTreeTable %l"
"|* %i OpenHomeWindow::FileTreeTable %l"
"|* %i ViewManPage::FileTreeTable"
"|* %i OpenTerminal::FileTreeTable"
"|* %i RunCommand::FileTreeTable %l"
"|* %i CloseWindow::JX"
"|* %i Quit::JX"
;

enum {
	kNewDirCmd=1,
	kNewTextFileCmd,
	kOpenCmd,
	kOpenCloseCmd,
	kOpenIconifyCmd,
	kAltOpenCmd,
	kRunOnSelCmd,
	kFindCmd,
	kOpenRecentItemIndex,
	kRenameCmd,
	kConvertToFileCmd,
	kConvertToProgramCmd,
	kDuplicateCmd,
	kMakeAliasCmd,
	kFindOriginalCmd,
	kToggleMountCmd,
	kEraseDiskCmd,
	kHomeWindowCmd,
	kViewManCmd,
	kOpenTermCmd,
	kRunCmdCmd,
	kCloseCmd,
	kQuitCmd,
};

#ifndef _H_jx_af_image_jx_jx_folder_small
#define _H_jx_af_image_jx_jx_folder_small
#include <jx-af/image/jx/jx_folder_small.xpm>
#endif
#ifndef _H_jx_af_image_jx_jx_file_new
#define _H_jx_af_image_jx_jx_file_new
#include <jx-af/image/jx/jx_file_new.xpm>
#endif
#ifndef _H_jx_af_image_jx_jx_file_open
#define _H_jx_af_image_jx_jx_file_open
#include <jx-af/image/jx/jx_file_open.xpm>
#endif
#ifndef _H_jx_af_image_jx_jx_find
#define _H_jx_af_image_jx_jx_find
#include <jx-af/image/jx/jx_find.xpm>
#endif
#ifndef _H_jx_af_image_jx_jx_plain_file_small
#define _H_jx_af_image_jx_jx_plain_file_small
#include <jx-af/image/jx/jx_plain_file_small.xpm>
#endif
#ifndef _H_jx_af_image_jx_jx_executable_small
#define _H_jx_af_image_jx_jx_executable_small
#include <jx-af/image/jx/jx_executable_small.xpm>
#endif
#ifndef _H_jx_af_image_jx_jx_library_file_small
#define _H_jx_af_image_jx_jx_library_file_small
#include <jx-af/image/jx/jx_library_file_small.xpm>
#endif
#ifndef _H_home
#define _H_home
#include "home.xpm"
#endif
#ifndef _H_man
#define _H_man
#include "man.xpm"
#endif
#ifndef _H_mini_term
#define _H_mini_term
#include "mini_term.xpm"
#endif

static void ConfigureFileMenu(JXTextMenu* menu, const int offset = 0) {
	if (offset == 0 && JXMenu::GetDisplayStyle() == JXMenu::kWindowsStyle) {
		menu->SetShortcuts(JString("#f", JString::kNoCopy));
	}
	menu->SetItemImage(kNewDirCmd + offset, jx_folder_small);
	menu->SetItemImage(kNewTextFileCmd + offset, jx_file_new);
	menu->SetItemImage(kOpenCmd + offset, jx_file_open);
	menu->SetItemImage(kFindCmd + offset, jx_find);
	menu->SetItemImage(kConvertToFileCmd + offset, jx_plain_file_small);
	menu->SetItemImage(kConvertToProgramCmd + offset, jx_executable_small);
	menu->SetItemImage(kDuplicateCmd + offset, jx_library_file_small);
	menu->SetItemImage(kHomeWindowCmd + offset, home);
	menu->SetItemImage(kViewManCmd + offset, man);
	menu->SetItemImage(kOpenTermCmd + offset, mini_term);
};

#endif
