/******************************************************************************
 SyGTreeDir.h

	Interface for the SyGTreeDir class

	Copyright (C) 1997 by Glenn Bach.

 ******************************************************************************/

#ifndef _H_SyGTreeDir
#define _H_SyGTreeDir

#include <jx-af/jx/JXWindowDirector.h>

class JXMenuBar;
class JXTextMenu;
class JXToolBar;
class JXTextButton;

class SyGTreeSet;
class SyGFileTreeTable;
class SyGFolderDragSource;
class SyGPathInput;

class SyGTreeDir : public JXWindowDirector
{
public:

	SyGTreeDir(const JString& startPath);

	virtual ~SyGTreeDir();

	virtual bool			Close() override;
	virtual const JString&	GetName() const override;
	virtual bool			GetMenuIcon(const JXImage** icon) const override;

	const JString&		GetDirectory() const;
	SyGTreeSet*			GetTreeSet() const;
	SyGFileTreeTable*	GetTable() const;

	void	SaveState();

protected:

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	SyGTreeSet*	itsTreeSet;
	bool	itsAutoMountFlag;	// true => unmount when close

	JXTextMenu*	itsPrefsMenu;
	JXTextMenu*	itsHelpMenu;

// begin JXLayout

	JXToolBar*           itsToolBar;
	SyGPathInput*        itsPathInput;
	SyGFolderDragSource* itsDragSrc;
	JXTextButton*        itsUpButton;

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

inline SyGTreeSet*
SyGTreeDir::GetTreeSet()
	const
{
	return itsTreeSet;
}

#endif
