/******************************************************************************
 FileTreeList.cpp

	BASE CLASS = JNamedTreeList

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "FileTreeList.h"
#include "FileTree.h"
#include "FileTreeNode.h"
#include <jx-af/jcore/JDirEntry.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

FileTreeList::FileTreeList
	(
	FileTree* tree
	)
	:
	JNamedTreeList(tree)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FileTreeList::~FileTreeList()
{
}

/******************************************************************************
 GetFileTree

 ******************************************************************************/

FileTree*
FileTreeList::GetFileTree()
{
	auto* tree = dynamic_cast<FileTree*>(GetTree());
	assert( tree != nullptr );
	return tree;
}

const FileTree*
FileTreeList::GetFileTree()
	const
{
	const auto* tree = dynamic_cast<const FileTree*>(GetTree());
	assert( tree != nullptr );
	return tree;
}

/******************************************************************************
 GetFileNode

 ******************************************************************************/

FileTreeNode*
FileTreeList::GetFileNode
	(
	const JIndex index
	)
{
	auto* node = dynamic_cast<FileTreeNode*>(GetNode(index));
	assert (node != nullptr);
	return node;
}

const FileTreeNode*
FileTreeList::GetFileNode
	(
	const JIndex index
	)
	const
{
	const auto* node = dynamic_cast<const FileTreeNode*>(GetNode(index));
	assert (node != nullptr);
	return node;
}

/******************************************************************************
 GetDirEntry

 ******************************************************************************/

const JDirEntry*
FileTreeList::GetDirEntry
	(
	const JIndex index
	)
	const
{
	return GetFileNode(index)->GetDirEntry();
}

/******************************************************************************
 ShouldOpenSibling (virtual)

	Returns true if sibling should be opened.

 ******************************************************************************/

bool
FileTreeList::ShouldOpenSibling
	(
	const JTreeNode* node
	)
{
	const auto* fsNode = dynamic_cast<const JFSFileTreeNode*>(node);
	assert( fsNode != nullptr );
	const JDirEntry* entry = fsNode->GetDirEntry();
	return !JIsVCSDirectory(entry->GetName());
}

/******************************************************************************
 ShouldOpenDescendant (virtual)

	Returns true if sibling should be opened.

 ******************************************************************************/

bool
FileTreeList::ShouldOpenDescendant
	(
	const JTreeNode* node
	)
{
	return ShouldOpenSibling(node);
}
