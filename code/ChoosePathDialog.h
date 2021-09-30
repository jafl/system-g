/******************************************************************************
 ChoosePathDialog.h

	Interface for the ChoosePathDialog class

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_ChoosePathDialog
#define _H_ChoosePathDialog

#include <jx-af/jx/JXChoosePathDialog.h>

class ChoosePathDialog : public JXChoosePathDialog
{
public:

	ChoosePathDialog(JXDirector* supervisor, JDirInfo* dirInfo,
					   const JString& fileFilter,
					   const bool selectOnlyWritable);

	virtual ~ChoosePathDialog();

	void		BuildWindow(const bool newWindow, const JString& message = JString::empty);

	bool	OpenInNewWindow() const;

protected:

	virtual void		Receive(JBroadcaster* sender, const Message& message) override;

private:

// begin JXLayout

	JXTextButton*   itsSelectButton;
	JXTextCheckbox* itsNewWindowCB;
	JXTextButton*   itsOpenButton;

// end JXLayout
};

#endif
