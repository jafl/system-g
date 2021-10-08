/******************************************************************************
 MoveToTrashProcess.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_MoveToTrashProcess
#define _H_MoveToTrashProcess

#include <jx-af/jcore/JPtrArray-JString.h>

class JSimpleProcess;
class FileTreeTable;
class FileTreeNode;

class MoveToTrashProcess : virtual public JBroadcaster
{
public:

	static void	Move(FileTreeTable* table, JPtrArray<JString>* fullNameList);

	~MoveToTrashProcess() override;

protected:

	MoveToTrashProcess(FileTreeTable* table, JPtrArray<JString>* fullNameList,
					   const JString& trashDir);

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	FileTreeTable*		itsTable;			// not owned; can be nullptr
	JPtrArray<JString>*	itsFullNameList;	// store strings so don't need table
	JString				itsTrashDir;
	JSimpleProcess*		itsProcess;

private:

	void	ProcessNextFile();

	// not allowed

	MoveToTrashProcess(const MoveToTrashProcess&) = delete;
	MoveToTrashProcess& operator=(const MoveToTrashProcess&) = delete;
};

#endif
