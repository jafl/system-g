/******************************************************************************
 NewGitRemoteDialog.cpp

	BASE CLASS = JXDialogDirector

	Copyright (C) 2014 by John Lindal.

 ******************************************************************************/

#include <NewGitRemoteDialog.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

NewGitRemoteDialog::NewGitRemoteDialog
	(
	JXDirector* supervisor
	)
	:
	JXDialogDirector(supervisor, true)
{
	BuildWindow();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

NewGitRemoteDialog::~NewGitRemoteDialog()
{
}

/******************************************************************************
 GetRepoURL

 ******************************************************************************/

const JString&
NewGitRemoteDialog::GetRepoURL()
	const
{
	return itsRemoteURLInputField->GetText()->GetText();
}

/******************************************************************************
 GetLocalName

 ******************************************************************************/

const JString&
NewGitRemoteDialog::GetLocalName()
	const
{
	return itsLocalNameInputField->GetText()->GetText();
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

void
NewGitRemoteDialog::BuildWindow()
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 310,160, JString::empty);
	assert( window != nullptr );

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 190,130, 60,20);
	assert( okButton != nullptr );
	okButton->SetShortcuts(JGetString("okButton::NewGitRemoteDialog::shortcuts::JXLayout"));

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,130, 60,20);
	assert( cancelButton != nullptr );
	cancelButton->SetShortcuts(JGetString("cancelButton::NewGitRemoteDialog::shortcuts::JXLayout"));

	itsRemoteURLInputField =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,40, 270,20);
	assert( itsRemoteURLInputField != nullptr );

	auto* remoteLabel =
		jnew JXStaticText(JGetString("remoteLabel::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,20, 270,20);
	assert( remoteLabel != nullptr );
	remoteLabel->SetToLabel();

	itsLocalNameInputField =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,90, 270,20);
	assert( itsLocalNameInputField != nullptr );

	auto* localLabel =
		jnew JXStaticText(JGetString("localLabel::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,70, 270,20);
	assert( localLabel != nullptr );
	localLabel->SetToLabel();

// end JXLayout

	window->SetTitle(JGetString("WindowTitle::NewGitRemoteDialog"));
	SetButtons(okButton, cancelButton);

	itsRemoteURLInputField->SetIsRequired();
	itsLocalNameInputField->SetIsRequired();
}
