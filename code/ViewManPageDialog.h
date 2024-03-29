/******************************************************************************
 ViewManPageDialog.h

	Interface for the ViewManPageDialog class

	Copyright (C) 1996-98 by John Lindal.

 ******************************************************************************/

#ifndef _H_ViewManPageDialog
#define _H_ViewManPageDialog

#include <jx-af/jx/JXWindowDirector.h>
#include <jx-af/jcore/JPrefObject.h>
#include <jx-af/jcore/JString.h>

class JXTextButton;
class JXInputField;
class JXCharInput;
class JXTextCheckbox;
class JXStringHistoryMenu;

class ViewManPageDialog : public JXWindowDirector, public JPrefObject
{
public:

	ViewManPageDialog(JXDirector* supervisor);

	~ViewManPageDialog() override;

	void	Activate() override;

	void	ViewManPage(const JString& item, const JUtf8Character& index = JUtf8Character(' '),
						const bool apropos = false);
	void	ViewManPages(const JPtrArray<JString>& list);

	static const JString&	GetViewManPageCommand();
	static void				SetViewManPageCommand(const JString& cmd);

protected:

	void	ReadPrefs(std::istream& input) override;
	void	WritePrefs(std::ostream& output) const override;

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	static JString	itsViewCmd;		// static so PrefsMgr can set it

// begin JXLayout

	JXStringHistoryMenu* itsFnHistoryMenu;
	JXTextCheckbox*      itsAproposCheckbox;
	JXTextCheckbox*      itsStayOpenCB;
	JXTextButton*        itsCloseButton;
	JXTextButton*        itsHelpButton;
	JXTextButton*        itsViewButton;
	JXInputField*        itsFnName;
	JXCharInput*         itsManIndex;

// end JXLayout

private:

	void	BuildWindow();
	void	UpdateDisplay();
	void	ViewManPage();
	void	SetFunction(const JString& historyStr);
	void	AddToHistory(const JString& pageName, const JUtf8Character& pageIndex,
						 const bool apropos);
};


/******************************************************************************
 View man page command (static)

 ******************************************************************************/

inline const JString&
ViewManPageDialog::GetViewManPageCommand()
{
	return itsViewCmd;
}

inline void
ViewManPageDialog::SetViewManPageCommand
	(
	const JString& cmd
	)
{
	itsViewCmd = cmd;
	itsViewCmd.TrimWhitespace();
}

#endif
