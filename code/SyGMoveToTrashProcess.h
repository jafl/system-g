/******************************************************************************
 SyGMoveToTrashProcess.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_SyGMoveToTrashProcess
#define _H_SyGMoveToTrashProcess

#include <jx-af/jcore/JPtrArray-JString.h>

class JSimpleProcess;
class SyGFileTreeTable;
class SyGFileTreeNode;

class SyGMoveToTrashProcess : virtual public JBroadcaster
{
public:

	static void	Move(SyGFileTreeTable* table, JPtrArray<JString>* fullNameList);

	virtual ~SyGMoveToTrashProcess();

protected:

	SyGMoveToTrashProcess(SyGFileTreeTable* table, JPtrArray<JString>* fullNameList,
						  const JString& trashDir);

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	SyGFileTreeTable*	itsTable;			// not owned; can be nullptr
	JPtrArray<JString>*	itsFullNameList;	// store strings so don't need table
	JString				itsTrashDir;
	JSimpleProcess*		itsProcess;

private:

	void	ProcessNextFile();

	// not allowed

	SyGMoveToTrashProcess(const SyGMoveToTrashProcess&) = delete;
	SyGMoveToTrashProcess& operator=(const SyGMoveToTrashProcess&) = delete;
};

#endif
