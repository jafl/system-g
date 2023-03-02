/******************************************************************************
 NewGitRemoteDialog.h

	Copyright (C) 2014 by John Lindal.

 ******************************************************************************/

#ifndef _H_NewGitRemoteDialog
#define _H_NewGitRemoteDialog

#include <jx-af/jx/JXModalDialogDirector.h>

class JXInputField;

class NewGitRemoteDialog : public JXModalDialogDirector
{
public:

	NewGitRemoteDialog();

	~NewGitRemoteDialog() override;

	const JString&	GetRepoURL() const;
	const JString&	GetLocalName() const;

private:

// begin JXLayout

	JXInputField* itsRemoteURLInputField;
	JXInputField* itsLocalNameInputField;

// end JXLayout

private:

	void	BuildWindow();
};

#endif
