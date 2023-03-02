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

	static ChoosePathDialog*
		Create(const SelectPathType type,
			   const JString& startPath = JString::empty,
			   const JString& fileFilter = JString::empty,
			   const JString& message = JString::empty);

	~ChoosePathDialog() override;

	bool	OpenInNewWindow() const;

protected:

	ChoosePathDialog(const SelectPathType select,
					 const JString& fileFilter);

private:

// begin JXLayout

	JXTextButton*   itsSelectButton;
	JXTextCheckbox* itsNewWindowCB;
	JXTextButton*   itsOpenButton;

// end JXLayout

private:

	void	BuildWindow(const JString& startPath, const JString& message);
};

#endif
