/******************************************************************************
 SyGFileTreeNode.h

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_SyGFileTreeNode
#define _H_SyGFileTreeNode

#include <jx-af/jfs/JFSFileTreeNode.h>
#include "SyGColUtils.h"	// need defn of GFMColType

class JDirEntry;
class SyGFileTree;
class SyGFileTreeList;

class SyGFileTreeNode : public JFSFileTreeNode
{
public:

	SyGFileTreeNode(JDirEntry* entry);

	virtual ~SyGFileTreeNode();

	void	Init();
	void	SetChildCompareFunction(const GFMColType type);

	SyGFileTree*			GetSyGFileTree();
	const SyGFileTree*		GetSyGFileTree() const;

	SyGFileTreeNode*		GetSyGParent();
	const SyGFileTreeNode*	GetSyGParent() const;
	bool				GetSyGParent(SyGFileTreeNode** parent);
	bool				GetSyGParent(const SyGFileTreeNode** parent) const;

	SyGFileTreeNode*		GetSyGChild(const JIndex index);
	const SyGFileTreeNode*	GetSyGChild(const JIndex index) const;

protected:

	virtual JFSFileTreeNode*	CreateChild(JDirEntry* entry);
};

/******************************************************************************
 Init

	Called on root node to get things started.

 ******************************************************************************/

inline void
SyGFileTreeNode::Init()
{
	OKToOpen();
}

#endif
