// This file was automatically generated by jx_menu_editor.
// Do not edit it directly!
// Any changes you make will be silently overwritten.

#ifndef _H_TreeDir_Help
#define _H_TreeDir_Help

static const JUtf8Byte* kHelpMenuStr =
"* %i __About::TreeDir %l"
"|* %i HelpTOC::JX"
"|* %i __GettingStarted::TreeDir %l"
"|* %i __Changes::TreeDir"
"|* %i __Credits::TreeDir"
;

enum {
	kAboutCmd=1,
	kTOCCmd,
	kOverviewCmd,
	kChangesCmd,
	kCreditsCmd,
};

#ifndef _H_jx_af_image_jx_jx_help_toc
#define _H_jx_af_image_jx_jx_help_toc
#include <jx-af/image/jx/jx_help_toc.xpm>
#endif

static void ConfigureHelpMenu(JXTextMenu* menu, const int offset = 0) {
	if (offset == 0 && JXMenu::GetDisplayStyle() == JXMenu::kWindowsStyle) {
		menu->SetShortcuts(JString("#h", JString::kNoCopy));
	}
	menu->SetItemImage(kTOCCmd + offset, jx_help_toc);
};

#endif
