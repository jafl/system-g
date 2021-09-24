/******************************************************************************
 SyGFolderDragSource.h

	Copyright (C) 2001 by John Lindal.

 ******************************************************************************/

#ifndef _H_SyGFolderDragSource
#define _H_SyGFolderDragSource

#include <JXImageWidget.h>

class JXPathInput;
class JXCurrentPathMenu;

class SyGFolderDragSource : public JXImageWidget
{
public:

	SyGFolderDragSource(JXPathInput* pathInput, JXCurrentPathMenu** pathMenu,
						JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);

	virtual ~SyGFolderDragSource();

protected:

	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;
	virtual void	HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;

	virtual Atom	GetDNDAction(const JXContainer* target,
								 const JXButtonStates& buttonStates,
								 const JXKeyModifiers& modifiers) override;
	virtual void	GetDNDAskActions(const JXButtonStates& buttonStates,
								  const JXKeyModifiers& modifiers,
								  JArray<Atom>* askActionList,
								  JPtrArray<JString>* askDescriptionList) override;
	virtual void	HandleDNDResponse(const JXContainer* target,
									  const bool dropAccepted, const Atom action) override;

private:

	JXPathInput*		itsPathInput;	// not owned
	JXCurrentPathMenu*	itsPathMenu;
	JPoint				itsStartPt;
	Time				itsMouseDownTime;
};

#endif
