/******************************************************************************
 SyGFileTreeList.h

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_SyGFileTreeList
#define _H_SyGFileTreeList

#include <jx-af/jcore/JNamedTreeList.h>

class JDirEntry;
class SyGFileTree;
class SyGFileTreeNode;

class SyGFileTreeList : public JNamedTreeList
{
public:

	SyGFileTreeList(SyGFileTree* tree);

	virtual ~SyGFileTreeList();

	SyGFileTree*		GetSyGFileTree();
	const SyGFileTree*	GetSyGFileTree() const;

	SyGFileTreeNode*		GetSyGNode(const JIndex index);
	const SyGFileTreeNode*	GetSyGNode(const JIndex index) const;

//	JDirEntry*			GetDirEntry(const JIndex index);
	const JDirEntry*	GetDirEntry(const JIndex index) const;

protected:

	virtual bool	ShouldOpenSibling(const JTreeNode* node);
	virtual bool	ShouldOpenDescendant(const JTreeNode* node);
};

#endif
