/******************************************************************************
 SyGFileTree.h

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_SyGFileTree
#define _H_SyGFileTree

#include <jx-af/jfs/JFSFileTree.h>
#include "SyGColUtils.h"	// need defn of GFMColType

class JDirInfo;
class SyGFileTreeNode;

class SyGFileTree : public JFSFileTree
{
public:

	SyGFileTree(SyGFileTreeNode* root);

	virtual ~SyGFileTree();

	const JString&	GetDirectory() const;
	bool			Update(const bool force = false,
						   JFSFileTreeNodeBase** updateNode = nullptr);

	bool	HiddenVisible() const;
	void	ShowHidden(const bool show);

	void	SetWildcardFilter(const JString& filter);
	void	ClearWildcardFilter();

	void	SetNodeCompareFunction(const GFMColType type);

	SyGFileTreeNode*		GetSyGRoot();
	const SyGFileTreeNode*	GetSyGRoot() const;

	JDirInfo*		GetRootDirInfo();
	const JDirInfo*	GetRootDirInfo() const;
};

#endif
