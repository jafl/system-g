/******************************************************************************
 TrashButton.cpp

	Provides a drop target regardless of which virtual terminal the user is on.

	BASE CLASS = JXImageButton

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#include "TrashButton.h"
#include "MoveToTrashProcess.h"
#include "FileTreeTable.h"
#include "FileTree.h"
#include "globals.h"
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXSelectionManager.h>
#include <jx-af/jx/JXDNDManager.h>
#include <jx-af/jx/JXWindowPainter.h>
#include <jx-af/jx/jXPainterUtil.h>
#include <jx-af/jx/jXUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

TrashButton::TrashButton
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
	JXImageButton(enclosure, hSizing, vSizing, x,y, w,h)
{
	SetImage(GetTrashSmallIcon(), false);
	ListenTo(this);
	ListenTo(GetApplication());

	JString trashDir;
	if (!GetTrashDirectory(&trashDir, false))
	{
		Deactivate();
	}
}

/******************************************************************************
 Destructor

 ******************************************************************************/

TrashButton::~TrashButton()
{
}

/******************************************************************************
 DrawBorder (virtual protected)

 ******************************************************************************/

void
TrashButton::DrawBorder
	(
	JXWindowPainter&	p,
	const JRect&		frame
	)
{
	if (IsDNDTarget())
	{
		JXDrawDNDBorder(p, frame, GetBorderWidth());
	}
	else
	{
		JXImageButton::DrawBorder(p, frame);
	}
}

/******************************************************************************
 UpdateDisplay

 ******************************************************************************/

void
TrashButton::UpdateDisplay()
{
	SetImage(GetTrashSmallIcon(), false);
}

/******************************************************************************
 Receive (protected)

 ******************************************************************************/

void
TrashButton::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == GetApplication() && message.Is(Application::kTrashNeedsUpdate))
	{
		UpdateDisplay();
	}

	else
	{
		if (sender == this && message.Is(JXButton::kPushed))
		{
			JString trashDir;
			if (GetTrashDirectory(&trashDir))
			{
				GetApplication()->OpenDirectory(trashDir);
			}
		}

		JXImageButton::Receive(sender, message);
	}
}

/******************************************************************************
 WillAcceptDrop (virtual protected)

 ******************************************************************************/

bool
TrashButton::WillAcceptDrop
	(
	const JArray<Atom>& typeList,
	Atom*				action,
	const JPoint&		pt,
	const Time			time,
	const JXWidget*		source
	)
{
	JString trashDir;
	if (!GetTrashDirectory(&trashDir, false))
	{
		return false;
	}

	const Atom urlXAtom = GetSelectionManager()->GetURLXAtom();

	const JSize typeCount = typeList.GetElementCount();
	for (JIndex i=1; i<=typeCount; i++)
	{
		const Atom a = typeList.GetElement(i);
		if (a == urlXAtom)
		{
			*action = GetDNDManager()->GetDNDActionPrivateXAtom();
			return true;
		}
	}

	return false;
}

/******************************************************************************
 HandleDNDEnter (virtual protected)

 ******************************************************************************/

void
TrashButton::HandleDNDEnter()
{
	SetDNDTarget(nullptr);
	SetImage(GetTrashSmallIcon(true), false);
}

/******************************************************************************
 HandleDNDLeave (virtual protected)

 ******************************************************************************/

void
TrashButton::HandleDNDLeave()
{
	SetDNDTarget(nullptr);
	SetImage(GetTrashSmallIcon(false), false);
}

/******************************************************************************
 HandleDNDDrop (virtual protected)

 ******************************************************************************/

void
TrashButton::HandleDNDDrop
	(
	const JPoint&		pt,
	const JArray<Atom>& typeList,
	const Atom			action,
	const Time			time,
	const JXWidget*		source
	)
{
	MoveFilesToTrash(time, source);
	HandleDNDLeave();
}

/******************************************************************************
 MoveFilesToTrash (static)

 ******************************************************************************/

void
TrashButton::MoveFilesToTrash
	(
	const Time		time,
	const JXWidget*	source
	)
{
	FileTreeTable* srcTable = nullptr;
	if (GetDNDSource(source, &srcTable) &&
		IsTrashDirectory((srcTable->GetFileTree())->GetDirectory()))
	{
		return;
	}

	JXDisplay* display             = JXGetApplication()->GetCurrentDisplay();
	JXSelectionManager* selManager = display->GetSelectionManager();
	JXDNDManager* dndMgr           = display->GetDNDManager();
	const Atom dndSelectionName    = dndMgr->GetDNDSelectionName();

	unsigned char* data = nullptr;
	JSize dataLength;
	Atom returnType;
	JXSelectionManager::DeleteMethod delMethod;
	if (selManager->GetData(dndSelectionName, time, selManager->GetURLXAtom(),
							&returnType, &data, &dataLength, &delMethod))
	{
		if (returnType == selManager->GetURLXAtom())
		{
			auto* fileNameList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
			assert( fileNameList != nullptr );
			JPtrArray<JString> urlList(JPtrArrayT::kDeleteAll);
			JXUnpackFileNames((char*) data, dataLength, fileNameList, &urlList);

			MoveToTrashProcess::Move(srcTable, fileNameList);
			JXReportUnreachableHosts(urlList);
		}

		selManager->DeleteData(&data, delMethod);
	}

	SetDNDSource(nullptr);
}
