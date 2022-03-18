/******************************************************************************
 BeginEditingTask.cpp

	BASE CLASS = JXIdleTask, virtual JBroadcaster

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#include "BeginEditingTask.h"
#include "FileTreeTable.h"
#include "FileTreeList.h"
#include "FileTreeNode.h"
#include <jx-af/jx/jXConstants.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

BeginEditingTask::BeginEditingTask
	(
	FileTreeTable*	table,
	const JPoint&	cell
	)
	:
	JXIdleTask(kJXDoubleClickTime),
	itsTable(table),
	itsNode(table->GetFileTreeList()->GetFileNode(cell.y))
{
	ClearWhenGoingAway(itsNode, &itsNode);
	ClearWhenGoingAway(itsNode, &(itsTable->itsEditTask));

	ClearWhenGoingAway(itsTable, &itsNode);
	ClearWhenGoingAway(itsTable, &itsTable);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

BeginEditingTask::~BeginEditingTask()
{
}

/******************************************************************************
 Perform

 ******************************************************************************/

void
BeginEditingTask::Perform()
{
	if (itsTable != nullptr)
	{
		itsTable->itsEditTask = nullptr;	// first action:  allows it to create another one, if necessary

		JPoint cell;
		if (itsTable->GetTableSelection().GetSingleSelectedCell(&cell) &&
			itsTable->GetFileTreeList()->GetFileNode(cell.y) == itsNode)
		{
			itsTable->BeginEditing(cell);
			itsTable->TableScrollToCell(JPoint(itsTable->GetToggleOpenColIndex(), cell.y));
		}
	}

	jdelete this;
}

/******************************************************************************
 Perform (virtual)

 ******************************************************************************/

void
BeginEditingTask::Perform
	(
	const Time	delta,
	Time*		maxSleepTime
	)
{
	if (TimeToPerform(delta, maxSleepTime))
	{
		Perform();
	}
}
