/******************************************************************************
 EditPrefsDialog.h

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_EditPrefsDialog
#define _H_EditPrefsDialog

#include <jx-af/jx/JXDialogDirector.h>
#include <jx-af/jcore/JPtrArray-JString.h>

class JXInputField;
class JXRadioGroup;
class JXTextCheckbox;

class EditPrefsDialog : public JXDialogDirector
{
public:

	EditPrefsDialog(const JString& terminalCmd,
					   const JString& manViewCmd,
					   const JString& gitStatusCmd,
					   const JString& gitHistoryCmd,
					   const JString& postCheckoutCmd,
					   const bool del, const bool newWindows,
					   const bool perFolderPrefs);

	~EditPrefsDialog() override;

	void	GetPrefs(JString* terminalCmd, JString* manViewCmd,
					 JString* gitStatusCmd, JString* gitHistoryCmd,
					 JString* postCheckoutCmd, bool* del,
					 bool* newWindows, bool* perFolderPrefs) const;

private:

// begin JXLayout

	JXInputField*   itsManInput;
	JXInputField*   itsTerminalInput;
	JXInputField*   itsGitStatusInput;
	JXTextCheckbox* itsDelCB;
	JXTextCheckbox* itsAllowSpaceCB;
	JXInputField*   itsGitHistoryInput;
	JXInputField*   itsPostCheckoutInput;
	JXTextCheckbox* itsOpenNewWindowsCB;
	JXTextCheckbox* itsFolderPrefsCB;

// end JXLayout

private:

	void	BuildWindow(const JString& terminalCmd,
						const JString& manViewCmd,
						const JString& gitStatusCmd,
						const JString& gitHistoryCmd,
						const JString& postCheckoutCmd,
						const bool del, const bool newWindows,
						const bool perFolderPrefs);
};

#endif
