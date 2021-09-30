/******************************************************************************
 FileTree.cpp

	BASE CLASS = JFSFileTree

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "FileTree.h"
#include "FileTreeNode.h"
#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

FileTree::FileTree
	(
	FileTreeNode* root
	)
	:
	JFSFileTree(root)
{
	root->Init();	// build child list
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FileTree::~FileTree()
{
}

/******************************************************************************
 GetDirectory

 ******************************************************************************/

const JString&
FileTree::GetDirectory()
	const
{
	return GetRootDirInfo()->GetDirectory();
}

/******************************************************************************
 Update

	Returns true if anything changed.

 ******************************************************************************/

bool
FileTree::Update
	(
	const bool				force,
	JFSFileTreeNodeBase**	updateNode
	)
{
	if (force || updateNode == nullptr || *updateNode == nullptr)
	{
		return GetFileRoot()->Update(force, updateNode);
	}
	else
	{
		return (**updateNode).Update(force, updateNode);
	}
}

/******************************************************************************
 HiddenVisible

 ******************************************************************************/

bool
FileTree::HiddenVisible()
	const
{
	return GetRootDirInfo()->HiddenVisible();
}

/******************************************************************************
 ShowHidden

 ******************************************************************************/

void
FileTree::ShowHidden
	(
	const bool show
	)
{
	GetRootDirInfo()->ShowHidden(show);
}

/******************************************************************************
 SetWildcardFilter

 ******************************************************************************/

void
FileTree::SetWildcardFilter
	(
	const JString& filter
	)
{
	GetRootDirInfo()->SetWildcardFilter(filter);
}

/******************************************************************************
 ClearWildcardFilter

 ******************************************************************************/

void
FileTree::ClearWildcardFilter()
{
	GetRootDirInfo()->ClearWildcardFilter();
}

/******************************************************************************
 SetNodeCompareFunction

 ******************************************************************************/

void
FileTree::SetNodeCompareFunction
	(
	const GFMColType type
	)
{
	GetFileRoot()->SetChildCompareFunction(type);
}

/******************************************************************************
 GetFileRoot

 ******************************************************************************/

FileTreeNode*
FileTree::GetFileRoot()
{
	auto* root = dynamic_cast<FileTreeNode*>(GetRoot());
	assert( root != nullptr );
	return root;
}

const FileTreeNode*
FileTree::GetFileRoot()
	const
{
	const auto* root = dynamic_cast<const FileTreeNode*>(GetRoot());
	assert( root != nullptr );
	return root;
}

/******************************************************************************
 GetRootDirInfo

 ******************************************************************************/

JDirInfo*
FileTree::GetRootDirInfo()
{
	JDirInfo* info;
	const bool ok = GetFileRoot()->GetDirInfo(&info);
	assert( ok );
	return info;
}

const JDirInfo*
FileTree::GetRootDirInfo()
	const
{
	const JDirInfo* info;
	const bool ok = GetFileRoot()->GetDirInfo(&info);
	assert( ok );
	return info;
}
