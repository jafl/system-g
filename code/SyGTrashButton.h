/******************************************************************************
 SyGTrashButton.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_SyGTrashButton
#define _H_SyGTrashButton

#include <JXImageButton.h>

class SyGTrashButton : public JXImageButton
{
public:

	SyGTrashButton(JXContainer* enclosure,
				   const HSizingOption hSizing, const VSizingOption vSizing,
				   const JCoordinate x, const JCoordinate y,
				   const JCoordinate w, const JCoordinate h);

	virtual ~SyGTrashButton();

	void	UpdateDisplay();

	static void	MoveFilesToTrash(const Time time, const JXWidget* source);

protected:

	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame) override;

	virtual bool	WillAcceptDrop(const JArray<Atom>& typeList, Atom* action,
								   const JPoint& pt, const Time time,
								   const JXWidget* source) override;
	virtual void	HandleDNDEnter() override;
	virtual void	HandleDNDLeave() override;
	virtual void	HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList,
								  const Atom action, const Time time,
								  const JXWidget* source) override;

	virtual void Receive(JBroadcaster* sender, const Message& message) override;
};

#endif
