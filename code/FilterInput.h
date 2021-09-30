/******************************************************************************
 FilterInput.h

	Copyright (C) 1996 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_FilterInput
#define _H_FilterInput

#include <jx-af/jx/JXInputField.h>
#include <jx-af/jcore/JString.h>

class FilterInput : public JXInputField
{
public:

	FilterInput(JXWidget* widget, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);

	virtual ~FilterInput();

	virtual void	HandleKeyPress(const JUtf8Character& c, const int keySym,
								   const JXKeyModifiers& modifiers) override;

protected:

	virtual void	HandleFocusEvent() override;			// must call inherited

private:

	JXWidget*	itsDirList;
	JString		itsInitialText;
};

#endif
