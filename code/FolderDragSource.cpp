/******************************************************************************
 FolderDragSource.cpp

	Displays a folder icon that the user can drag and also JXCurrentPathMenu.

	BASE CLASS = JXImageWidget

	Copyright (C) 2001 by John Lindal.

 ******************************************************************************/

#include "FolderDragSource.h"
#include "FileTreeTable.h"
#include "globals.h"
#include <jx-af/jx/JXFileSelection.h>
#include <jx-af/jx/JXDNDManager.h>
#include <jx-af/jx/JXSelectionManager.h>
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXPathInput.h>
#include <jx-af/jx/JXCurrentPathMenu.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jx/JXImageCache.h>
#include <jx-af/jcore/jMouseUtil.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

#include <jx-af/image/jx/jx_folder_small.xpm>

/******************************************************************************
 Constructor

 ******************************************************************************/

FolderDragSource::FolderDragSource
	(
	JXPathInput*		pathInput,
	JXCurrentPathMenu**	pathMenu,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXImageWidget(enclosure, hSizing, vSizing, x,y, w,h),
	itsPathInput(pathInput)
{
	SetImage(GetDisplay()->GetImageCache()->GetImage(jx_folder_small), false);

	SetHint(JGetString("Hint::FolderDragSource"));

	itsPathMenu = jnew JXCurrentPathMenu(JGetRootDirectory(), this, kFixedLeft, kFixedTop, 0,0, 10,10);
	itsPathMenu->Hide();
	itsPathMenu->SetToHiddenPopupMenu(true);
	*pathMenu = itsPathMenu;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FolderDragSource::~FolderDragSource()
{
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
FolderDragSource::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	itsStartPt       = pt;
	itsMouseDownTime = JXGetApplication()->GetCurrentTime();
}

/******************************************************************************
 HandleMouseDrag (virtual protected)

 ******************************************************************************/

void
FolderDragSource::HandleMouseDrag
	(
	const JPoint&			pt,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JString path;
	const bool pathValid = itsPathInput->GetPath(&path);

	if (pathValid && !JMouseMoved(itsStartPt, pt) &&
		JXGetApplication()->GetCurrentTime() >= itsMouseDownTime + kJXDoubleClickTime)
	{
		itsPathMenu->SetPath(path);
		itsPathMenu->PopUp(this, pt, buttonStates, modifiers);
	}
	else if (pathValid && JMouseMoved(itsStartPt, pt))
	{
		JPtrArray<JString> list(JPtrArrayT::kForgetAll);
		list.Append(&const_cast<JString&>(path));

		auto* data = jnew JXFileSelection(GetDisplay(), list);
		BeginDND(pt, buttonStates, modifiers, data);
	}
}

/******************************************************************************
 GetDNDAction (virtual protected)

 ******************************************************************************/

Atom
FolderDragSource::GetDNDAction
	(
	const JXContainer*		target,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JString path;
	const bool pathValid = itsPathInput->GetPath(&path);
	assert( pathValid );	// checked before BeginDND()

	return FileTreeTable::GetDNDAction(this, path, target, modifiers);
}

/******************************************************************************
 GetDNDAskActions (virtual protected)

	This is called when the value returned by GetDropAction() changes to
	XdndActionAsk.  If GetDropAction() repeatedly returns XdndActionAsk,
	this function is not called again because it is assumed that the
	actions are the same within a single DND session.

	This function must place at least 2 elements in askActionList and
	askDescriptionList.

	The first element should be the default action.

 ******************************************************************************/

void
FolderDragSource::GetDNDAskActions
	(
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers,
	JArray<Atom>*			askActionList,
	JPtrArray<JString>*		askDescriptionList
	)
{
	::GetDNDAskActions(askActionList, askDescriptionList);
}

/******************************************************************************
 HandleDNDResponse (virtual protected)

 ******************************************************************************/

void
FolderDragSource::HandleDNDResponse
	(
	const JXContainer*	target,
	const bool		dropAccepted,
	const Atom			action
	)
{
	DisplayCursor(GetDNDManager()->GetDNDDirectoryCursor(dropAccepted, action));
}
