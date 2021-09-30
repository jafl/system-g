/******************************************************************************
 NewGitRemoteDialog.h

	Copyright (C) 2014 by John Lindal.

 ******************************************************************************/

#ifndef _H_NewGitRemoteDialog
#define _H_NewGitRemoteDialog

#include <jx-af/jx/JXDialogDirector.h>

class JXInputField;

class NewGitRemoteDialog : public JXDialogDirector
{
public:

	NewGitRemoteDialog(JXDirector* supervisor);

	virtual ~NewGitRemoteDialog();

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
