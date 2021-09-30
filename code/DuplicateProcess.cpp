/******************************************************************************
 DuplicateProcess.cpp

	BASE CLASS = virtual JBroadcaster

	Copyright (C) 2000 by John Lindal.

 *****************************************************************************/

#include "DuplicateProcess.h"
#include "FileTreeTable.h"
#include <jx-af/jx/JXDeleteObjectTask.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JThisProcess.h>
#include <jx-af/jcore/JDirEntry.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor function (static)

 ******************************************************************************/

void
DuplicateProcess::Duplicate
	(
	FileTreeTable*					table,
	const JPtrArray<FileTreeNode>&	nodeList
	)
{
	auto* p = jnew DuplicateProcess(table, nodeList);
	assert( p != nullptr );
}

/******************************************************************************
 Constructor (protected)

 ******************************************************************************/

DuplicateProcess::DuplicateProcess
	(
	FileTreeTable*					table,
	const JPtrArray<FileTreeNode>&	nodeList
	)
	:
	itsTable(table),
	itsNodeList(JPtrArrayT::kForgetAll),
	itsFullNameList(JPtrArrayT::kDeleteAll),
	itsProcess(nullptr),
	itsShouldEditFlag(nodeList.GetElementCount() == 1)
{
	itsTable->GetTableSelection().ClearSelection();
	ClearWhenGoingAway(itsTable, &itsTable);

	const JSize count = nodeList.GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		auto* node = const_cast<FileTreeNode*>(nodeList.GetElement(i));
		itsNodeList.Append(node);
		itsFullNameList.Append((node->GetDirEntry())->GetFullName());
		ListenTo(node);
	}

	ProcessNextFile();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

DuplicateProcess::~DuplicateProcess()
{
	assert( itsProcess == nullptr );
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
DuplicateProcess::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsProcess && message.Is(JProcess::kFinished))
	{
		const auto* info =
			dynamic_cast<const JProcess::Finished*>(&message);
		if (info->Successful() && itsTable != nullptr && !itsTable->IsEditing())
		{
			FileTreeNode* node   = itsNodeList.GetFirstElement();
			FileTreeNode* parent = nullptr;
			if (node != nullptr)
			{
				parent = node->GetFileParent();
			}

			JPoint cell;
			const bool found = itsTable->SelectName(itsCurrentName, parent, &cell);		// updates table
			if (itsShouldEditFlag && found)
			{
				itsTable->BeginEditing(cell);
			}
		}

		JXDeleteObjectTask<JBroadcaster>::Delete(itsProcess);
		itsProcess = nullptr;
		itsNodeList.RemoveElement(1);
		ProcessNextFile();
	}
	else
	{
		JBroadcaster::Receive(sender, message);
	}
}

/******************************************************************************
 ReceiveGoingAway (virtual protected)

 ******************************************************************************/

void
DuplicateProcess::ReceiveGoingAway
	(
	JBroadcaster* sender
	)
{
	JIndex nodeIndex  = 0;
	const JSize count = itsNodeList.GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		if (sender == itsNodeList.GetElement(i))
		{
			nodeIndex = i;
			break;
		}
	}

	if (nodeIndex > 0)
	{
		itsNodeList.SetToNull(nodeIndex, JPtrArrayT::kForget);
	}
	else
	{
		JBroadcaster::ReceiveGoingAway(sender);
	}
}

/******************************************************************************
 ProcessNextFile (private)

 ******************************************************************************/

void
DuplicateProcess::ProcessNextFile()
{
	if (itsFullNameList.IsEmpty())
	{
		JXDeleteObjectTask<JBroadcaster>::Delete(this);
		return;
	}

	const JString* origName = itsFullNameList.GetFirstElement();
	JString path, name, root, suffix;
	JSplitPathAndName(*origName, &path, &name);
	if (JSplitRootAndSuffix(name, &root, &suffix))
	{
		suffix.Prepend(".");
	}

	root += "_copy";
	name  = JGetUniqueDirEntryName(path, root, suffix.GetBytes());

	JSplitPathAndName(name, &path, &itsCurrentName);

	const JUtf8Byte* argv[] = { "cp", "-Rdf", origName->GetBytes(), name.GetBytes(), nullptr };

	JVCSType type;
	if (JIsManagedByVCS(*origName, &type) && type == kJSVNType)
	{
		argv[0] = "svn";
		argv[1] = "cp";
	}

	const JError err = JSimpleProcess::Create(&itsProcess, argv, sizeof(argv));
	err.ReportIfError();
	itsFullNameList.DeleteElement(1);		// before ProcessNextFile()

	if (err.OK())
	{
		ListenTo(itsProcess);
		JThisProcess::Ignore(itsProcess);	// detach so it always finishes
	}
	else
	{
		itsProcess = nullptr;
		itsNodeList.RemoveElement(1);
		ProcessNextFile();
	}
}
