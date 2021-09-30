/******************************************************************************
 CopyProcess.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_CopyProcess
#define _H_CopyProcess

#include <jx-af/jcore/JPtrArray-JString.h>
#include <jx-af/jcore/jVCSUtil.h>

class JSimpleProcess;
class FileTreeTable;
class FileTreeNode;

class CopyProcess : virtual public JBroadcaster
{
public:

	static void	Copy(FileTreeTable* srcTable, JPtrArray<JString>* srcNameList,
					 FileTreeTable* destTable, FileTreeNode* destNode);
	static void	Move(FileTreeTable* srcTable, JPtrArray<JString>* srcNameList,
					 FileTreeTable* destTable, FileTreeNode* destNode);

	virtual ~CopyProcess();

protected:

	CopyProcess(FileTreeTable* srcTable, JPtrArray<JString>* srcNameList,
				   FileTreeTable* destTable, FileTreeNode* destNode,
				   const bool isCopy);

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	FileTreeTable*			itsSrcTable;		// not owned; can be nullptr
	JPtrArray<JString>*			itsSrcNameList;		// usually from DND data
	FileTreeTable*			itsDestTable;		// not owned; can be nullptr
	FileTreeNode*			itsDestNode;		// not owned; can be nullptr
	JVCSType					itsVCSType;
	const bool				itsIsMoveFlag;
	JSimpleProcess*				itsProcess;

private:

	void	Start(const JSize prefixCount);
	void	RemoveExecutePermissions(const JString& srcPath,
									 const JString& destPath);

	static bool	CleanSrcList(JPtrArray<JString>* srcNameList,
								 FileTreeNode* destNode);
	static bool	ActionIsUseful(const JString& srcName, const JString& destPath,
								   JString* destName);
	static bool	OKToReplace(const JString& srcName, const JString& destName,
								bool* ask, bool* first);

	// not allowed

	CopyProcess(const CopyProcess&) = delete;
	CopyProcess& operator=(const CopyProcess&) = delete;
};

#endif
