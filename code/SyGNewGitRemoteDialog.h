/******************************************************************************
 SyGNewGitRemoteDialog.h

	Copyright (C) 2014 by John Lindal.

 ******************************************************************************/

#ifndef _H_SyGNewGitRemoteDialog
#define _H_SyGNewGitRemoteDialog

#include <JXDialogDirector.h>

class JXInputField;

class SyGNewGitRemoteDialog : public JXDialogDirector
{
public:

	SyGNewGitRemoteDialog(JXDirector* supervisor);

	virtual ~SyGNewGitRemoteDialog();

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
