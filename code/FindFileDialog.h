/******************************************************************************
 FindFileDialog.h

	Copyright (C) 1996-2009 by John Lindal.

 ******************************************************************************/

#ifndef _H_FindFileDialog
#define _H_FindFileDialog

#include <jx-af/jx/JXWindowDirector.h>
#include <jx-af/jcore/JPrefObject.h>
#include <jx-af/jcore/JString.h>

class JXTextButton;
class JXTextCheckbox;
class JXRadioGroup;
class JXInputField;
class JXPathInput;

class FindFileDialog : public JXWindowDirector, public JPrefObject
{
public:

	FindFileDialog(JXDirector* supervisor);

	~FindFileDialog() override;

	void	Activate() override;

	void	Search(const JString& path);
	void	SearchFileName(const JString& path, const JString& pattern);
	void	SearchExpr(const JString& path, const JString& expr);

protected:

	void	ReadPrefs(std::istream& input) override;
	void	WritePrefs(std::ostream& output) const override;

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

// begin JXLayout

	JXRadioGroup*   itsActionRG;
	JXTextButton*   itsCloseButton;
	JXTextButton*   itsSearchButton;
	JXInputField*   itsFileInput;
	JXTextButton*   itsHelpButton;
	JXTextCheckbox* itsStayOpenCB;
	JXInputField*   itsExprInput;
	JXPathInput*    itsPathInput;
	JXTextButton*   itsChoosePathButton;

// end JXLayout

private:

	void	BuildWindow();
	void	UpdateDisplay();
	void	UpdateButtons();
	bool	Search();
};

#endif
