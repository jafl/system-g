/******************************************************************************
 SyGChoosePathDialog.h

	Interface for the SyGChoosePathDialog class

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_SyGChoosePathDialog
#define _H_SyGChoosePathDialog

#include <JXChoosePathDialog.h>

class SyGChoosePathDialog : public JXChoosePathDialog
{
public:

	SyGChoosePathDialog(JXDirector* supervisor, JDirInfo* dirInfo,
					   const JString& fileFilter,
					   const bool selectOnlyWritable);

	virtual ~SyGChoosePathDialog();

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
