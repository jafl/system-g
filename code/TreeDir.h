/******************************************************************************
 TreeDir.h

	Interface for the TreeDir class

	Copyright (C) 1997 by Glenn Bach.

 ******************************************************************************/

#ifndef _H_TreeDir
#define _H_TreeDir

#include <jx-af/jx/JXWindowDirector.h>

class JXMenuBar;
class JXTextMenu;
class JXToolBar;
class JXTextButton;

class TreeSet;
class FileTreeTable;
class FolderDragSource;
class PathInput;

class TreeDir : public JXWindowDirector
{
public:

	TreeDir(const JString& startPath);

	virtual ~TreeDir();

	bool			Close() override;
	const JString&	GetName() const override;
	bool			GetMenuIcon(const JXImage** icon) const override;

	const JString&		GetDirectory() const;
	TreeSet*			GetTreeSet() const;
	FileTreeTable*	GetTable() const;

	void	SaveState();

protected:

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	TreeSet*	itsTreeSet;
	bool	itsAutoMountFlag;	// true => unmount when close

	JXTextMenu*	itsPrefsMenu;
	JXTextMenu*	itsHelpMenu;

// begin JXLayout

	JXToolBar*        itsToolBar;
	PathInput*        itsPathInput;
	FolderDragSource* itsDragSrc;
	JXTextButton*     itsUpButton;

// end JXLayout

private:

	void	BuildWindow(const JString& startPath);
	void	WriteState(std::ostream& output);

	void	HandleHelpMenu(const JIndex index);
	void	HandlePrefsMenu(const JIndex index);
};


/******************************************************************************
 GetTreeSet

 ******************************************************************************/

inline TreeSet*
TreeDir::GetTreeSet()
	const
{
	return itsTreeSet;
}

#endif
