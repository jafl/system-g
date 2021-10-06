/******************************************************************************
 DuplicateProcess.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_DuplicateProcess
#define _H_DuplicateProcess

#include <jx-af/jcore/JPtrArray-JString.h>

class JSimpleProcess;
class FileTreeTable;
class FileTreeNode;

class DuplicateProcess : virtual public JBroadcaster
{
public:

	static void	Duplicate(FileTreeTable* table,
						  const JPtrArray<FileTreeNode>& nodeList);

	virtual ~DuplicateProcess();

protected:

	DuplicateProcess(FileTreeTable* table,
						const JPtrArray<FileTreeNode>& nodeList);

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveGoingAway(JBroadcaster* sender) override;

private:

	FileTreeTable*			itsTable;			// not owned; can be nullptr
	JPtrArray<FileTreeNode>	itsNodeList;		// contents not owned
	JPtrArray<JString>			itsFullNameList;	// store strings so don't need table
	JSimpleProcess*				itsProcess;
	JString						itsCurrentName;
	const bool				itsShouldEditFlag;	// BeginEditing() after process finishes

private:

	void	ProcessNextFile();

	// not allowed

	DuplicateProcess(const DuplicateProcess&) = delete;
	DuplicateProcess& operator=(const DuplicateProcess&) = delete;
};

#endif
