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

	auto* window = jnew JXWindow(this, 470,380, JString::empty);

	itsManInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 155,190, 300,20);
	assert( itsManInput != nullptr );

	itsTerminalInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 155,210, 300,20);
	assert( itsTerminalInput != nullptr );

	itsGitStatusInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 215,260, 240,20);
	assert( itsGitStatusInput != nullptr );

	auto* terminalLabel =
		jnew JXStaticText(JGetString("terminalLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,210, 145,20);
	assert( terminalLabel != nullptr );
	terminalLabel->SetToLabel();

	auto* manPageLabel =
		jnew JXStaticText(JGetString("manPageLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,190, 145,20);
	assert( manPageLabel != nullptr );
	manPageLabel->SetToLabel();

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 105,350, 60,20);
	assert( cancelButton != nullptr );
	cancelButton->SetShortcuts(JGetString("cancelButton::EditPrefsDialog::shortcuts::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedTop, 305,350, 60,20);
	assert( okButton != nullptr );
	okButton->SetShortcuts(JGetString("okButton::EditPrefsDialog::shortcuts::JXLayout"));

	auto* cmdHint =
		jnew JXStaticText(JGetString("cmdHint::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 155,230, 300,20);
	assert( cmdHint != nullptr );
	cmdHint->SetFontSize(JFontManager::GetDefaultFontSize()-2);
	cmdHint->SetToLabel();

	itsDelCB =
		jnew JXTextCheckbox(JGetString("itsDelCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,70, 220,20);
	assert( itsDelCB != nullptr );

	itsAllowSpaceCB =
		jnew JXTextCheckbox(JGetString("itsAllowSpaceCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,120, 220,20);
	assert( itsAllowSpaceCB != nullptr );

	auto* warnLabel =
		jnew JXStaticText(JGetString("warnLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 40,90, 310,20);
	assert( warnLabel != nullptr );
	warnLabel->SetFontSize(JFontManager::GetDefaultFontSize()-2);
	warnLabel->SetToLabel();

	auto* branchLabel =
		jnew JXStaticText(JGetString("branchLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,300, 205,20);
	assert( branchLabel != nullptr );
	branchLabel->SetToLabel();

	auto* branchHint =
		jnew JXStaticText(JGetString("branchHint::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 215,320, 240,20);
	assert( branchHint != nullptr );
	branchHint->SetFontSize(JFontManager::GetDefaultFontSize()-2);
	branchHint->SetToLabel();

	auto* statusLabel =
		jnew JXStaticText(JGetString("statusLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,260, 205,20);
	assert( statusLabel != nullptr );
	statusLabel->SetToLabel();

	itsGitHistoryInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 215,280, 240,20);
	assert( itsGitHistoryInput != nullptr );

	auto* historyLabel =
		jnew JXStaticText(JGetString("historyLabel::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,280, 205,20);
	assert( historyLabel != nullptr );
	historyLabel->SetToLabel();

	itsPostCheckoutInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 215,300, 240,20);
	assert( itsPostCheckoutInput != nullptr );

	itsOpenNewWindowsCB =
		jnew JXTextCheckbox(JGetString("itsOpenNewWindowsCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,20, 220,20);
	assert( itsOpenNewWindowsCB != nullptr );

	auto* openHint =
		jnew JXStaticText(JGetString("openHint::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 40,40, 410,20);
	assert( openHint != nullptr );
	openHint->SetFontSize(JFontManager::GetDefaultFontSize()-2);
	openHint->SetToLabel();

	itsFolderPrefsCB =
		jnew JXTextCheckbox(JGetString("itsFolderPrefsCB::EditPrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,150, 220,20);
	assert( itsFolderPrefsCB != nullptr );

// end JXLayout

	window->SetTitle(JGetString("WindowTitle::EditPrefsDialog"));
	SetButtons(okButton, cancelButton);

	const JFont& font = JFontManager::GetDefaultMonospaceFont();

	itsManInput->GetText()->SetText(manViewCmd);
	itsManInput->SetIsRequired();
	itsManInput->SetFont(font);

	itsTerminalInput->GetText()->SetText(terminalCmd);
	itsTerminalInput->SetIsRequired();
	itsTerminalInput->SetFont(font);

	itsGitStatusInput->GetText()->SetText(gitStatusCmd);
	itsGitStatusInput->SetIsRequired();
	itsGitStatusInput->SetFont(font);

	itsGitHistoryInput->GetText()->SetText(gitHistoryCmd);
	itsGitHistoryInput->SetIsRequired();
	itsGitHistoryInput->SetFont(font);

	itsPostCheckoutInput->GetText()->SetText(postCheckoutCmd);
	itsPostCheckoutInput->SetIsRequired();
	itsPostCheckoutInput->SetFont(font);

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
