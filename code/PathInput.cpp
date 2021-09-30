/******************************************************************************
 PathInput.cpp

	Input field for entering a file path.

	BASE CLASS = JXStringInput

	Copyright (C) 1996 by Glenn W. Bach.

 ******************************************************************************/

#include <PathInput.h>
#include <jx-af/jcore/JString.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jcore/jASCIIConstants.h>

#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

PathInput::PathInput
	(
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXPathInput(enclosure, hSizing, vSizing, x,y, w,h)
{
	itsDirList = nullptr;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

PathInput::~PathInput()
{
}

/******************************************************************************
 SetDirList

	We are created before the table so we can't get the pointer
	in our constructor.

 ******************************************************************************/

void
PathInput::SetDirList
	(
	JXWidget* dirList
	)
{
	itsDirList = dirList;

	// this re-orders the list so dir list gets focus

	WantInput(false);
	WantInput(true, WantsTab(), WantsModifiedTab());
}

/******************************************************************************
 HandleKeyPress

 ******************************************************************************/

void
PathInput::HandleKeyPress
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
		JXPathInput::HandleKeyPress(c, keySym, modifiers);
	}
}

/******************************************************************************
 HandleFocusEvent

 ******************************************************************************/

void
PathInput::HandleFocusEvent()
{
	itsInitialText = GetText()->GetText();
	JXPathInput::HandleFocusEvent();
}
