/******************************************************************************
 FileTreeList.h

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_FileTreeList
#define _H_FileTreeList

#include <jx-af/jcore/JNamedTreeList.h>

class JDirEntry;
class FileTree;
class FileTreeNode;

class FileTreeList : public JNamedTreeList
{
public:

	FileTreeList(FileTree* tree);

	~FileTreeList() override;

	FileTree*		GetFileTree();
	const FileTree*	GetFileTree() const;

	FileTreeNode*		GetFileNode(const JIndex index);
	const FileTreeNode*	GetFileNode(const JIndex index) const;

//	JDirEntry*			GetDirEntry(const JIndex index);
	const JDirEntry*	GetDirEntry(const JIndex index) const;

protected:

	bool	ShouldOpenSibling(const JTreeNode* node) override;
	bool	ShouldOpenDescendant(const JTreeNode* node) override;
};

#endif
