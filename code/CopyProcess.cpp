/******************************************************************************
 CopyProcess.cpp

	BASE CLASS = virtual JBroadcaster

	Copyright (C) 2000 by John Lindal.

 *****************************************************************************/

#include "CopyProcess.h"
#include "FileTreeTable.h"
#include "globals.h"
#include <jx-af/jx/JXDeleteObjectTask.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JThisProcess.h>
#include <jx-af/jfs/JFSFileTree.h>
#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jMountUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor function (static)

	srcTable can be nullptr.  We take ownership of srcNameList.

 ******************************************************************************/

void
CopyProcess::Copy
	(
	FileTreeTable*		srcTable,
	JPtrArray<JString>*	srcNameList,
	FileTreeTable*		destTable,
	FileTreeNode*		destNode
	)
{
	if (CleanSrcList(srcNameList, destNode))
	{
		jnew CopyProcess(srcTable, srcNameList, destTable, destNode, true);
	}
}

void
CopyProcess::Move
	(
	FileTreeTable*		srcTable,
	JPtrArray<JString>*	srcNameList,
	FileTreeTable*		destTable,
	FileTreeNode*		destNode
	)
{
	if (CleanSrcList(srcNameList, destNode))
	{
		jnew CopyProcess(srcTable, srcNameList, destTable, destNode, false);
	}
}

/******************************************************************************
 CleanSrcList (static private)

	Removes items that would go nowhere or which user doesn't want to
	overwrite.  If nothing is left, deletes the list.

 ******************************************************************************/

bool
CopyProcess::CleanSrcList
	(
	JPtrArray<JString>*	srcNameList,
	FileTreeNode*		destNode
	)
{
	JString destPath = (destNode->GetDirEntry())->GetFullName();
	JAppendDirSeparator(&destPath);

	JSize count = srcNameList->GetItemCount();
	JString destName;
	bool ask, first = true;
	for (JIndex i=1; i<=count; i++)
	{
		const JString* srcName = srcNameList->GetItem(i);
		if (!ActionIsUseful(*srcName, destPath, &destName) ||
			!OKToReplace(*srcName, destName, &ask, &first))
		{
			srcNameList->DeleteItem(i);
			i--;
			count--;	// don't confuse user by going in reverse order
		}
	}

	if (srcNameList->IsEmpty())
	{
		jdelete srcNameList;
		return false;
	}
	else
	{
		return true;
	}
}

/******************************************************************************
 ActionIsUseful (static private)

	Returns false if move/copy would produce no effect.  Otherwise,
	*destName is the destination path and name.

 ******************************************************************************/

bool
CopyProcess::ActionIsUseful
	(
	const JString&	srcName,
	const JString&	destPath,
	JString*		destName
	)
{
	if (!JNameUsed(srcName) || JSameDirEntry(srcName, destPath))
	{
		return false;
	}

	JString fullName(srcName), srcPath, name;
	JStripTrailingDirSeparator(&fullName);
	JSplitPathAndName(fullName, &srcPath, &name);

	if (JSameDirEntry(srcPath, destPath))
	{
		return false;
	}
	else
	{
		*destName = JCombinePathAndName(destPath, name);
		return true;
	}
}

/******************************************************************************
 OKToReplace (static private)

 ******************************************************************************/

bool
CopyProcess::OKToReplace
	(
	const JString&	srcName,
	const JString&	destName,
	bool*		ask,
	bool*		first
	)
{
	JString destPath, name;
	JSplitPathAndName(destName, &destPath, &name);	// src has same name

	if (!JNameUsed(destName))
	{
		if (JDirectoryExists(srcName) &&
			JDirectoryExists(destPath))		// can't move into self
		{
			JString trueSrc, trueDest;
			bool ok = JGetTrueName(srcName, &trueSrc);
			assert( ok );
			ok = JGetTrueName(destPath, &trueDest);
			assert( ok );

			if (trueDest.StartsWith(trueSrc))
			{
				const JUtf8Byte* map[] =
				{
					"src", name.GetBytes()
				};
				const JString msg = JGetString("NoMoveIntoSelf::CopyProcess", map, sizeof(map));
				JGetUserNotification()->ReportError(msg);
				return false;
			}
		}

		return true;
	}

	const bool srcIsDir  = JDirectoryExists(srcName);
	const bool destIsDir = JDirectoryExists(destName);
	if (srcIsDir != destIsDir)			// can't replace one type with the other
	{
		const JUtf8Byte* map[] =
		{
			"src_type",  (srcIsDir  ? "directory" : "file"),
			"dest_type", (destIsDir ? "directory" : "file"),
			"dest",      name.GetBytes()
		};
		const JString msg = JGetString("OnlyReplaceSameType::CopyProcess", map, sizeof(map));
		JGetUserNotification()->ReportError(msg);
		return false;
	}
	else if (srcIsDir && destIsDir)		// can't be replaced by contents or v.v.
	{
		JString trueSrc, trueDest;
		bool ok = JGetTrueName(srcName, &trueSrc);
		assert( ok );
		ok = JGetTrueName(destName, &trueDest);
		assert( ok );

		if (trueSrc.StartsWith(trueDest))
		{
			const JUtf8Byte* map[] =
			{
				"dest", name.GetBytes()
			};
			const JString msg = JGetString("NoReplaceWithContents::CopyProcess", map, sizeof(map));
			JGetUserNotification()->ReportError(msg);
			return false;
		}
		else if (trueDest.StartsWith(trueSrc))
		{
			const JUtf8Byte* map[] =
			{
				"src", name.GetBytes()
			};
			const JString msg = JGetString("NoMoveIntoSelf::CopyProcess", map, sizeof(map));
			JGetUserNotification()->ReportError(msg);
			return false;
		}
	}

	if (*first)
	{
		*ask   = !JGetUserNotification()->AskUserYes(JGetString("AskAutoReplace::CopyProcess"));
		*first = false;
	}

	if (*ask)
	{
		const JUtf8Byte* map[] =
		{
			"name", name.GetBytes()
		};
		const JString msg = JGetString("AskReplace::CopyProcess", map, sizeof(map));
		if (!JGetUserNotification()->AskUserYes(msg))
		{
			return false;
		}
	}

	return DeleteDirEntry(destName);
}

/******************************************************************************
 Constructor (protected)

 ******************************************************************************/

CopyProcess::CopyProcess
	(
	FileTreeTable*	srcTable,
	JPtrArray<JString>*	srcNameList,
	FileTreeTable*	destTable,
	FileTreeNode*	destNode,
	const bool		isCopy
	)
	:
	itsSrcTable(srcTable),
	itsSrcNameList(srcNameList),
	itsDestTable(destTable),
	itsDestNode(destNode),
	itsIsMoveFlag(!isCopy),
	itsProcess(nullptr)
{
	assert( !itsSrcNameList->IsEmpty() );
	itsSrcNameList->SetCleanUpAction(JPtrArrayT::kDeleteAll);

	JString destPath = (itsDestNode->GetDirEntry())->GetFullName();
	JAppendDirSeparator(&destPath);

	itsVCSType = kJUnknownVCSType;
	{
	JVCSType type1 = kJUnknownVCSType, type2 = kJUnknownVCSType, type3 = kJUnknownVCSType;
	bool anyVCS = false, allVCS = false, sameVCS = false;

	const JSize srcCount = srcNameList->GetItemCount();
	JString path, name;
	for (JIndex i=1; i<=srcCount; i++)
	{
		const JString* src = srcNameList->GetItem(i);
		const bool isDir   = JDirectoryExists(*src);
		bool isVCS3        = false;
		if (isDir)
		{
			isVCS3 = JIsManagedByVCS(*src, &type3);
			JSplitPathAndName(*src, &path, &name);
		}
		else
		{
			path = *src;
		}

		bool isVCS;
		if (isDir && !isVCS3)
		{
			isVCS = false;
			type2 = kJUnknownVCSType;
		}
		else
		{
			isVCS = JIsManagedByVCS(path, &type2);
		}

		if (i == 1)
		{
			type1 = type2;
			anyVCS = allVCS = sameVCS = isVCS;
		}
		else if (isVCS)
		{
			if (type2 != type1)
			{
				sameVCS = false;
			}
			anyVCS = true;
		}
		else
		{
			sameVCS = allVCS = false;
		}
	}

	if (allVCS && sameVCS)
	{
		type2 = JGetVCSType(destPath);
		if (isCopy && type1 == type2 && type1 != kJGitType)
		{
			if (JGetUserNotification()->AskUserYes(JGetString("UseVCSCopy::CopyProcess")))
			{
				itsVCSType = type1;
			}
		}
		else if (!isCopy && type1 == type2)
		{
			itsVCSType = type1;
		}
		else if (!isCopy &&
				 ((type2 == kJUnknownVCSType && !JGetUserNotification()->AskUserNo(JGetString("NoTargetVCSMove::CopyProcess"))) ||
				  (type2 != kJUnknownVCSType && !JGetUserNotification()->AskUserNo(JGetString("MixedVCSMove::CopyProcess")))))
		{
			JXDeleteObjectTask<JBroadcaster>::Delete(this);
			return;
		}
	}
	else if (!isCopy &&
			 ((allVCS && !JGetUserNotification()->AskUserNo(JGetString("MixedVCSMove::CopyProcess"))) ||
			  (anyVCS && !JGetUserNotification()->AskUserNo(JGetString("SomeVCSMove::CopyProcess")))))
	{
		JXDeleteObjectTask<JBroadcaster>::Delete(this);
		return;
	}
	}

	if (itsSrcTable != nullptr)
	{
		itsSrcTable->GetTableSelection().ClearSelection();
		ClearWhenGoingAway(itsSrcTable, &itsSrcTable);
	}

	itsDestTable->GetTableSelection().ClearSelection();
	ClearWhenGoingAway(itsDestTable, &itsDestTable);
	ClearWhenGoingAway(itsDestTable, &itsDestNode);
	ClearWhenGoingAway(itsDestNode, &itsDestNode);

	JSize prefixCount = 0;
	if (isCopy && itsVCSType == kJSVNType)
	{
		prefixCount = 2;
		itsSrcNameList->InsertAtIndex(1, "svn");
		itsSrcNameList->InsertAtIndex(2, "cp");
	}
	else if (isCopy)
	{
		prefixCount = 2;
		itsSrcNameList->InsertAtIndex(1, "cp");
		itsSrcNameList->InsertAtIndex(2, "-Rdf");
	}
	else if (itsVCSType == kJSVNType)
	{
		prefixCount = 3;
		itsSrcNameList->InsertAtIndex(1, "svn");
		itsSrcNameList->InsertAtIndex(2, "mv");
		itsSrcNameList->InsertAtIndex(3, "--force");
	}
	else if (itsVCSType == kJGitType)
	{
		prefixCount = 3;
		itsSrcNameList->InsertAtIndex(1, "git");
		itsSrcNameList->InsertAtIndex(2, "mv");
		itsSrcNameList->InsertAtIndex(3, "-f");
	}
	else
	{
		prefixCount = 2;
		itsSrcNameList->InsertAtIndex(1, "mv");
		itsSrcNameList->InsertAtIndex(2, "-f");
	}

	itsSrcNameList->Append(destPath);

	Start(prefixCount);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CopyProcess::~CopyProcess()
{
	assert( itsProcess == nullptr );
	jdelete itsSrcNameList;
}

/******************************************************************************
 Start (private)

 ******************************************************************************/

void
CopyProcess::Start
	(
	const JSize prefixCount
	)
{
	const JString* destPath = itsSrcNameList->GetLastItem();

	const JError err = JSimpleProcess::Create(&itsProcess, *destPath, *itsSrcNameList);

	// clean up to make Receive simpler

	for (JIndex i=1; i<=prefixCount; i++)
	{
		itsSrcNameList->DeleteItem(1);
	}

	err.ReportIfError();
	if (err.OK())
	{
		ListenTo(itsProcess);
		JThisProcess::Ignore(itsProcess);	// detach so it always finishes
		itsProcess->SetMaxReportInterval(1000000);
	}
	else
	{
		JXDeleteObjectTask<JBroadcaster>::Delete(this);
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
CopyProcess::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsProcess && message.Is(JProcess::kFinished))
	{
		JString* destPath = itsSrcNameList->GetLastItem();
		itsSrcNameList->RemoveItem(itsSrcNameList->GetItemCount());

		const auto* info = dynamic_cast<const JProcess::Finished*>(&message);
		itsProcess->ReportError(info->Successful());

		JXDeleteObjectTask<JBroadcaster>::Delete(itsProcess);
		itsProcess = nullptr;

		bool done = true;
		if (info->Successful())
		{
			if (itsSrcTable != nullptr)
			{
				itsSrcTable->UpdateDisplay(true);
			}

			bool selectName = itsDestNode != nullptr;
			assert( selectName == (itsDestTable != nullptr) );

			if (selectName && itsDestTable->IsEditing())
			{
				itsDestTable->UpdateDisplay(true);
				selectName = false;
			}

			const JSize count = itsSrcNameList->GetItemCount();
			JString path, name, oldName, newName, device;
			JFileSystemType fsType;
			for (JIndex i=1; i<=count; i++)
			{
				JString* fullName = itsSrcNameList->GetItem(i);
				JStripTrailingDirSeparator(fullName);
				JSplitPathAndName(*fullName, &path, &name);

				bool writable, isTop;
				if (JIsMounted(*fullName, &writable, &isTop, &device, &fsType) &&
					!JMountSupportsExecFlag(fsType))
				{
					RemoveExecutePermissions(*fullName, *destPath);
				}

				if (selectName)
				{
					JPoint cell;
					itsDestTable->SelectName(name, itsDestNode, &cell);	// updates table
				}

				newName = JCombinePathAndName(*destPath, name);
				if (itsIsMoveFlag && JDirectoryExists(newName))
				{
					oldName = *(itsSrcNameList->GetItem(i));
					JAppendDirSeparator(&oldName);
					JAppendDirSeparator(&newName);
					JFSFileTree::DirectoryRenamed msg(oldName, newName);
					(GetApplication())->DirectoryRenamed(msg);
				}
			}
		}
		else if (itsVCSType != kJUnknownVCSType && itsIsMoveFlag &&
				JGetUserNotification()->AskUserYes(JGetString("AskPlainVCSMove::CopyProcess")))
		{
			done = false;

			itsSrcNameList->InsertAtIndex(1, "mv");
			itsSrcNameList->InsertAtIndex(2, "-f");
			itsSrcNameList->Append(destPath);
			Start(2);
		}

		if (done)
		{
			JXDeleteObjectTask<JBroadcaster>::Delete(this);
			jdelete destPath;
		}
	}
	else
	{
		JBroadcaster::Receive(sender, message);
	}
}

/******************************************************************************
 RemoveExecutePermissions (private)

 ******************************************************************************/

void
CopyProcess::RemoveExecutePermissions
	(
	const JString& srcPath,
	const JString& destPath
	)
{
	JString path, name;
	JSplitPathAndName(srcPath, &path, &name);
	path = JCombinePathAndName(destPath, name);

	if (JFileExists(srcPath))
	{
		mode_t p;
		if (JGetPermissions(path, &p))
		{
			p &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
			JSetPermissions(path, p);
		}
		return;
	}

	JDirInfo* info;
	if (JDirInfo::Create(srcPath, &info))
	{
		const JSize count = info->GetEntryCount();
		for (JIndex i=1; i<=count; i++)
		{
			const JDirEntry& entry = info->GetEntry(i);
			RemoveExecutePermissions(entry.GetFullName(), path);
		}
		jdelete info;
	}
}
