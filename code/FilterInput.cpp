/******************************************************************************
 FilterInput.cpp

	Input field for entering a wildcard filter.

	BASE CLASS = JXInputField

	Copyright (C) 1996 by Glenn W. Bach.

 ******************************************************************************/

#include <FilterInput.h>
#include <jx-af/jcore/JString.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jcore/jASCIIConstants.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

FilterInput::FilterInput
	(
	JXWidget*			widget,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXInputField(enclosure, hSizing, vSizing, x,y, w,h)
{
	itsDirList = widget;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FilterInput::~FilterInput()
{
}

/******************************************************************************
 HandleKeyPress

 ******************************************************************************/

void
FilterInput::HandleKeyPress
	(
	const JUtf8Character&	c,
	const int				keySym,
	const JXKeyModifiers&	modifiers
	)
{
	if (c == kJReturnKey)
	{
		itsDirList->Focus();
	}

	else if (c == kJEscapeKey)
	{
		GetText()->SetText(itsInitialText);
		itsDirList->Focus();
	}

	else
	{
		JXInputField::HandleKeyPress(c, keySym, modifiers);
	}
}

/******************************************************************************
 HandleFocusEvent

 ******************************************************************************/

void
FilterInput::HandleFocusEvent()
{
	itsInitialText = GetText()->GetText();
	JXInputField::HandleFocusEvent();
}
