/******************************************************************************
 EditPrefsDialog.cpp

	BASE CLASS = JXModalDialogDirector

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#include "EditPrefsDialog.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXTextCheckbox.h>
#include <jx-af/jx/JXRadioGroup.h>
#include <jx-af/jx/JXTextRadioButton.h>
#include <jx-af/jx/JXSaveFileInput.h>
#include <jx-af/jx/JXFontManager.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

EditPrefsDialog::EditPrefsDialog
	(
	const JString&	terminalCmd,
	const JString&	manViewCmd,
	const JString&	gitStatusCmd,
	const JString&	gitHistoryCmd,
	const JString&	postCheckoutCmd,
	const bool	del,
	const bool	newWindows,
	const bool	perFolderPrefs
	)
	:
	JXModalDialogDirector()
{
	BuildWindow(terminalCmd, manViewCmd, gitStatusCmd, gitHistoryCmd,
				postCheckoutCmd, del, newWindows, perFolderPrefs);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

EditPrefsDialog::~EditPrefsDialog()
{
}

/******************************************************************************
 BuildWindow (protected)

 ******************************************************************************/

void
EditPrefsDialog::BuildWindow
	(
	const JString&	terminalCmd,
	const JString&	manViewCmd,
	const JString&	gitStatusCmd,
	const JString&	gitHistoryCmd,
	const JString&	postCheckoutCmd,
	const bool		del,
	const bool		newWindows,
	const bool		perFolderPrefs
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 470,360, JGetString("WindowTitle::EditPrefsDialog::JXLayout"));

	itsOpenNewWindowsCB =
		jnew JXTextCheckbox(JGetString("itsOpenNewWindowsCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,20, 220,20);

	auto* openHint =
		jnew JXStaticText(JGetString("openHint::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 40,40, 410,20);
	openHint->SetToLabel(false);

	itsDelCB =
		jnew JXTextCheckbox(JGetString("itsDelCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,70, 220,20);

	auto* warnLabel =
		jnew JXStaticText(JGetString("warnLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 40,90, 310,20);
	warnLabel->SetToLabel(false);

	itsAllowSpaceCB =
		jnew JXTextCheckbox(JGetString("itsAllowSpaceCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,120, 220,20);

	itsFolderPrefsCB =
		jnew JXTextCheckbox(JGetString("itsFolderPrefsCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,150, 220,20);

	auto* manPageLabel =
		jnew JXStaticText(JGetString("manPageLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,190, 145,20);
	manPageLabel->SetToLabel(false);

	auto* terminalLabel =
		jnew JXStaticText(JGetString("terminalLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,210, 145,20);
	terminalLabel->SetToLabel(false);

	auto* statusLabel =
		jnew JXStaticText(JGetString("statusLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,240, 205,20);
	statusLabel->SetToLabel(false);

	auto* historyLabel =
		jnew JXStaticText(JGetString("historyLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,260, 205,20);
	historyLabel->SetToLabel(false);

	auto* branchLabel =
		jnew JXStaticText(JGetString("branchLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,280, 205,20);
	branchLabel->SetToLabel(false);

	auto* branchHint =
		jnew JXStaticText(JGetString("branchHint::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 215,300, 240,20);
	branchHint->SetToLabel(false);

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 105,330, 60,20);
	cancelButton->SetShortcuts(JGetString("cancelButton::shortcuts::EditPrefsDialog::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedTop, 305,330, 60,20);
	okButton->SetShortcuts(JGetString("okButton::shortcuts::EditPrefsDialog::JXLayout"));

	itsManInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 155,190, 300,20);
	itsManInput->SetIsRequired();
	itsManInput->SetFont(JFontManager::GetDefaultMonospaceFont());
	itsManInput->SetHint(JGetString("itsManInput::EditPrefsDialog::JXLayout"));

	itsTerminalInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 155,210, 300,20);
	itsTerminalInput->SetIsRequired();
	itsTerminalInput->SetFont(JFontManager::GetDefaultMonospaceFont());
	itsTerminalInput->SetHint(JGetString("itsTerminalInput::EditPrefsDialog::JXLayout"));

	itsGitStatusInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 215,240, 240,20);
	itsGitStatusInput->SetIsRequired();
	itsGitStatusInput->SetFont(JFontManager::GetDefaultMonospaceFont());

	itsGitHistoryInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 215,260, 240,20);
	itsGitHistoryInput->SetIsRequired();
	itsGitHistoryInput->SetFont(JFontManager::GetDefaultMonospaceFont());

	itsPostCheckoutInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 215,280, 240,20);
	itsPostCheckoutInput->SetIsRequired();
	itsPostCheckoutInput->SetFont(JFontManager::GetDefaultMonospaceFont());

// end JXLayout

	SetButtons(okButton, cancelButton);

	openHint->SetFontSize(JFontManager::GetDefaultFontSize()-2);
	warnLabel->SetFontSize(JFontManager::GetDefaultFontSize()-2);
	branchHint->SetFontSize(JFontManager::GetDefaultFontSize()-2);

	itsManInput->GetText()->SetText(manViewCmd);
	itsTerminalInput->GetText()->SetText(terminalCmd);
	itsGitStatusInput->GetText()->SetText(gitStatusCmd);
	itsGitHistoryInput->GetText()->SetText(gitHistoryCmd);
	itsPostCheckoutInput->GetText()->SetText(postCheckoutCmd);

	itsOpenNewWindowsCB->SetState(newWindows);
	itsDelCB->SetState(del);
	itsAllowSpaceCB->SetState(JXSaveFileInput::WillAllowSpace());
	itsFolderPrefsCB->SetState(perFolderPrefs);
}

/******************************************************************************
 GetPrefs

 ******************************************************************************/

void
EditPrefsDialog::GetPrefs
	(
	JString*	terminalCmd,
	JString*	manViewCmd,
	JString*	gitStatusCmd,
	JString*	gitHistoryCmd,
	JString*	postCheckoutCmd,
	bool*	del,
	bool*	newWindows,
	bool*	perFolderPrefs
	)
	const
{
	*terminalCmd     = itsTerminalInput->GetText()->GetText();
	*manViewCmd      = itsManInput->GetText()->GetText();
	*gitStatusCmd    = itsGitStatusInput->GetText()->GetText();
	*gitHistoryCmd   = itsGitHistoryInput->GetText()->GetText();
	*postCheckoutCmd = itsPostCheckoutInput->GetText()->GetText();

	*newWindows     = itsOpenNewWindowsCB->IsChecked();
	*del            = itsDelCB->IsChecked();
	*perFolderPrefs = itsFolderPrefsCB->IsChecked();

	JXSaveFileInput::ShouldAllowSpace(itsAllowSpaceCB->IsChecked());
}
