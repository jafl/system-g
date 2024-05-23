/******************************************************************************
 NewGitRemoteDialog.cpp

	BASE CLASS = JXModalDialogDirector

	Copyright (C) 2014 by John Lindal.

 ******************************************************************************/

#include "NewGitRemoteDialog.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

NewGitRemoteDialog::NewGitRemoteDialog()
	:
	JXModalDialogDirector()
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

	auto* window = jnew JXWindow(this, 310,160, JGetString("WindowTitle::NewGitRemoteDialog::JXLayout"));

	auto* remoteLabel =
		jnew JXStaticText(JGetString("remoteLabel::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,20, 270,20);
	remoteLabel->SetToLabel(false);

	auto* localLabel =
		jnew JXStaticText(JGetString("localLabel::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,70, 270,20);
	localLabel->SetToLabel(false);

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,130, 60,20);
	cancelButton->SetShortcuts(JGetString("cancelButton::shortcuts::NewGitRemoteDialog::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::NewGitRemoteDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 190,130, 60,20);
	okButton->SetShortcuts(JGetString("okButton::shortcuts::NewGitRemoteDialog::JXLayout"));

	itsRemoteURLInputField =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,40, 270,20);
	itsRemoteURLInputField->SetIsRequired();

	itsLocalNameInputField =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,90, 270,20);
	itsLocalNameInputField->SetIsRequired();

// end JXLayout

	SetButtons(okButton, cancelButton);
}
