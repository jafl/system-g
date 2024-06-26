/******************************************************************************
 MoveToTrashProcess.cpp

	BASE CLASS = virtual JBroadcaster

	Copyright (C) 2000 by John Lindal.

 *****************************************************************************/

#include "MoveToTrashProcess.h"
#include "FileTreeTable.h"
#include "globals.h"
#include <jx-af/jx/JXDeleteObjectTask.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JThisProcess.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor function (static)

	table can be nullptr.  We take ownership of fullNameList.

 ******************************************************************************/

void
MoveToTrashProcess::Move
	(
	FileTreeTable*	table,
	JPtrArray<JString>*	fullNameList
	)
{
	JString trashDir;
	if (GetTrashDirectory(&trashDir, false))
	{
		jnew MoveToTrashProcess(table, fullNameList, trashDir);
	}
}

/******************************************************************************
 Constructor (protected)

 ******************************************************************************/

MoveToTrashProcess::MoveToTrashProcess
	(
	FileTreeTable*	table,
	JPtrArray<JString>*	fullNameList,
	const JString&		trashDir
	)
	:
	itsTable(table),
	itsFullNameList(fullNameList),
	itsTrashDir(trashDir),
	itsProcess(nullptr)
{
	if (itsTable != nullptr)
	{
		ClearWhenGoingAway(itsTable, &itsTable);
	}

	itsFullNameList->SetCleanUpAction(JPtrArrayT::kDeleteAll);
	ProcessNextFile();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

MoveToTrashProcess::~MoveToTrashProcess()
{
	if (itsTable != nullptr)
	{
		itsTable->UpdateDisplay(true);
	}

	UpdateTrash();

	assert( itsProcess == nullptr );
	jdelete itsFullNameList;
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
MoveToTrashProcess::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsProcess && message.Is(JProcess::kFinished))
	{
		auto& info = dynamic_cast<const JProcess::Finished&>(message);
		itsProcess->ReportError(info.Successful());

		JXDeleteObjectTask<JBroadcaster>::Delete(itsProcess);
		itsProcess = nullptr;
		ProcessNextFile();
	}
	else
	{
		JBroadcaster::Receive(sender, message);
	}
}

/******************************************************************************
 ProcessNextFile (private)

 ******************************************************************************/

void
MoveToTrashProcess::ProcessNextFile()
{
	if (itsFullNameList->IsEmpty())
	{
		JXDeleteObjectTask<JBroadcaster>::Delete(this);
		return;
	}

	JString* origName = itsFullNameList->GetFirstItem();
	JStripTrailingDirSeparator(origName);
	JString path, name;
	JSplitPathAndName(*origName, &path, &name);

	const JString newName = JCombinePathAndName(itsTrashDir, name);
	if (JNameUsed(newName))
	{
		JString root, suffix;
		if (JSplitRootAndSuffix(name, &root, &suffix))
		{
			suffix.Prepend(".");
		}
		const JString newName2 = JGetUniqueDirEntryName(itsTrashDir, root, suffix.GetBytes());
		JRenameDirEntry(newName, newName2);
	}

	const JUtf8Byte* argv[] = { "mv", "-f", origName->GetBytes(), newName.GetBytes(), nullptr };

	const JError err = JSimpleProcess::Create(&itsProcess, argv, sizeof(argv));
	err.ReportIfError();
	itsFullNameList->DeleteItem(1);		// before ProcessNextFile()

	if (err.OK())
	{
		ListenTo(itsProcess);
		JThisProcess::Ignore(itsProcess);	// detach so it always finishes
		itsProcess->SetMaxReportInterval(1000000);
	}
	else
	{
		itsProcess = nullptr;
		ProcessNextFile();
	}
}
