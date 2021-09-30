/******************************************************************************
 FileTreeNode.cpp

	BASE CLASS = JFSFileTreeNode

	Copyright (C) 1997 by Glenn W. Bach.

 *****************************************************************************/

#include "FileTreeNode.h"
#include "FileTree.h"
#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

FileTreeNode::FileTreeNode
	(
	JDirEntry* entry
	)
	:
	JFSFileTreeNode(entry)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FileTreeNode::~FileTreeNode()
{
}

/******************************************************************************
 CreateChild (virtual protected)

 ******************************************************************************/

JFSFileTreeNode*
FileTreeNode::CreateChild
	(
	JDirEntry* entry
	)
{
	JFSFileTreeNode* node = jnew FileTreeNode(entry);
	assert( node != nullptr);
	return node;
}

/******************************************************************************
 SetChildCompareFunction

 ******************************************************************************/

void
FileTreeNode::SetChildCompareFunction
	(
	const GFMColType type
	)
{
	if (type == kGFMUID)
	{
		JFSFileTreeNode::SetChildCompareFunction(JFSFileTreeNode::CompareUserName, JListT::kSortAscending, true);
	}
	else if (type == kGFMGID)
	{
		JFSFileTreeNode::SetChildCompareFunction(JFSFileTreeNode::CompareGroupName, JListT::kSortAscending, true);
	}
	else if (type == kGFMSize)
	{
		JFSFileTreeNode::SetChildCompareFunction(JFSFileTreeNode::CompareSize, JListT::kSortDescending, true);
	}
	else if (type == kGFMDate)
	{
		JFSFileTreeNode::SetChildCompareFunction(JFSFileTreeNode::CompareDate, JListT::kSortDescending, true);
	}
	else
	{
		JFSFileTreeNode::SetChildCompareFunction(JNamedTreeNode::DynamicCastCompareNames, JListT::kSortAscending, true);
	}
}

/******************************************************************************
 GetFileTree

 ******************************************************************************/

FileTree*
FileTreeNode::GetFileTree()
{
	auto* tree = dynamic_cast<FileTree*>(GetTree());
	assert (tree != nullptr);
	return tree;
}

const FileTree*
FileTreeNode::GetFileTree()
	const
{
	const auto* tree = dynamic_cast<const FileTree*>(GetTree());
	assert (tree != nullptr);
	return tree;
}

/******************************************************************************
 GetFileParent

 ******************************************************************************/

FileTreeNode*
FileTreeNode::GetFileParent()
{
	JTreeNode* p = GetParent();
	auto* n      = dynamic_cast<FileTreeNode*>(p);
	assert( n != nullptr );
	return n;
}

const FileTreeNode*
FileTreeNode::GetFileParent()
	const
{
	const JTreeNode* p = GetParent();
	const auto* n      = dynamic_cast<const FileTreeNode*>(p);
	assert( n != nullptr );
	return n;
}

bool
FileTreeNode::GetFileParent
	(
	FileTreeNode** parent
	)
{
	JTreeNode* p;
	if (GetParent(&p))
	{
		*parent = dynamic_cast<FileTreeNode*>(p);
		assert( *parent != nullptr );
		return true;
	}
	else
	{
		*parent = nullptr;
		return false;
	}
}

bool
FileTreeNode::GetFileParent
	(
	const FileTreeNode** parent
	)
	const
{
	const JTreeNode* p;
	if (GetParent(&p))
	{
		*parent = dynamic_cast<const FileTreeNode*>(p);
		assert( *parent != nullptr );
		return true;
	}
	else
	{
		*parent = nullptr;
		return false;
	}
}

/******************************************************************************
 GetFileChild

 ******************************************************************************/

FileTreeNode*
FileTreeNode::GetFileChild
	(
	const JIndex index
	)
{
	auto* node = dynamic_cast<FileTreeNode*>(GetChild(index));
	assert (node != nullptr);
	return node;
}

const FileTreeNode*
FileTreeNode::GetFileChild
	(
	const JIndex index
	)
	const
{
	const auto* node = dynamic_cast<const FileTreeNode*>(GetChild(index));
	assert (node != nullptr);
	return node;
}
