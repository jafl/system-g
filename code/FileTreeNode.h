/******************************************************************************
 FileTreeNode.h

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_FileTreeNode
#define _H_FileTreeNode

#include <jx-af/jfs/JFSFileTreeNode.h>
#include "ColUtils.h"	// need defn of GFMColType

class JDirEntry;
class FileTree;
class FileTreeList;

class FileTreeNode : public JFSFileTreeNode
{
public:

	FileTreeNode(JDirEntry* entry);

	virtual ~FileTreeNode();

	void	Init();
	void	SetChildCompareFunction(const GFMColType type);

	FileTree*			GetFileTree();
	const FileTree*		GetFileTree() const;

	FileTreeNode*		GetFileParent();
	const FileTreeNode*	GetFileParent() const;
	bool				GetFileParent(FileTreeNode** parent);
	bool				GetFileParent(const FileTreeNode** parent) const;

	FileTreeNode*		GetFileChild(const JIndex index);
	const FileTreeNode*	GetFileChild(const JIndex index) const;

protected:

	virtual JFSFileTreeNode*	CreateChild(JDirEntry* entry);
};

/******************************************************************************
 Init

	Called on root node to get things started.

 ******************************************************************************/

inline void
FileTreeNode::Init()
{
	OKToOpen();
}

#endif
