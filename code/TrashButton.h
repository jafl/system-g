/******************************************************************************
 TrashButton.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_TrashButton
#define _H_TrashButton

#include <jx-af/jx/JXImageButton.h>

class TrashButton : public JXImageButton
{
public:

	TrashButton(JXContainer* enclosure,
				   const HSizingOption hSizing, const VSizingOption vSizing,
				   const JCoordinate x, const JCoordinate y,
				   const JCoordinate w, const JCoordinate h);

	~TrashButton() override;

	void	UpdateDisplay();

	static void	MoveFilesToTrash(const Time time, const JXWidget* source);

protected:

	void	DrawBorder(JXWindowPainter& p, const JRect& frame) override;

	bool	WillAcceptDrop(const JArray<Atom>& typeList, Atom* action,
						   const JPoint& pt, const Time time,
						   const JXWidget* source) override;
	void	HandleDNDEnter() override;
	void	HandleDNDLeave() override;
	void	HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList,
						  const Atom action, const Time time,
						  const JXWidget* source) override;

	void Receive(JBroadcaster* sender, const Message& message) override;
};

#endif
