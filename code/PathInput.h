/******************************************************************************
 PathInput.h

	Copyright (C) 1996 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_PathInput
#define _H_PathInput

#include <jx-af/jx/JXPathInput.h>
#include <jx-af/jcore/JString.h>

class PathInput : public JXPathInput
{
public:

	PathInput(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);

	virtual ~PathInput();

	void	SetDirList(JXWidget* dirList);

	virtual void	HandleKeyPress(const JUtf8Character& c, const int keySym,
								   const JXKeyModifiers& modifiers) override;

protected:

	virtual void	HandleFocusEvent() override;			// must call inherited

private:

	JXWidget*	itsDirList;
	JString		itsInitialText;
};

#endif