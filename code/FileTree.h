/******************************************************************************
 FileTree.h

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_FileTree
#define _H_FileTree

#include <jx-af/jfs/JFSFileTree.h>
#include "ColUtils.h"	// for GFMColType

class JDirInfo;
class FileTreeNode;

class FileTree : public JFSFileTree
{
public:

	FileTree(FileTreeNode* root);

	~FileTree() override;

	const JString&	GetDirectory() const;
	bool			Update(const bool force = false,
						   JFSFileTreeNodeBase** updateNode = nullptr);

	bool	HiddenVisible() const;
	void	ShowHidden(const bool show);

	void	SetWildcardFilter(const JString& filter);
	void	ClearWildcardFilter();

	void	SetNodeCompareFunction(const GFMColType type);

	FileTreeNode*		GetFileRoot();
	const FileTreeNode*	GetFileRoot() const;

	JDirInfo*		GetRootDirInfo();
	const JDirInfo*	GetRootDirInfo() const;
};

#endif
