/******************************************************************************
 TreeSet.h

	Interface for the TreeSet class

	Copyright (C) 1996 by John Lindal.

 ******************************************************************************/

#ifndef _H_TreeSet
#define _H_TreeSet

#include <jx-af/jx/JXWidgetSet.h>

class JString;
class JXMenuBar;
class JXTextButton;
class JXStaticText;
class JXScrollbarSet;
class JXContainer;
class JXStringHistoryMenu;
class JXCurrentPathMenu;

class FileTreeTable;
class FileTree;
class FilterInput;
class PathInput;
class TrashButton;

class TreeSet : public JXWidgetSet
{
public:

	TreeSet(JXMenuBar* menuBar, const JString& pathName,
			   PathInput* pathInput, JXCurrentPathMenu* pathMenu,
			   TrashButton* trashButton, JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	TreeSet(std::istream& input, const JFileVersion vers,
			   JXMenuBar* menuBar, const JString& pathName,
			   PathInput* pathInput, JXCurrentPathMenu* pathMenu,
			   TrashButton* trashButton, JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	~TreeSet() override;

	FileTreeTable* GetTable() const;

	void	GoToItsPath();
	void	SetWildcardFilter(const JString& filter);
	void	SavePreferences(std::ostream& os);

	bool	FilterVisible() const;
	void	ShowFilter(const bool show);
	void	ToggleFilter();
	void	SaveFilterPref();

	void	UpdateDisplay(const JString& path);

protected:

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	FileTree*				itsFileTree;
	JXMenuBar*				itsMenuBar;
	JXTextButton*			itsEmptyButton;		// nullptr unless is trash can
	JXStaticText*			itsFilterLabel;
	FilterInput*			itsFilterInput;
	JXStringHistoryMenu*	itsFilterHistory;
	JXScrollbarSet*			itsScrollbarSet;
	FileTreeTable*			itsTable;
	PathInput*				itsPathInput;
	JXCurrentPathMenu*		itsPathMenu;

private:

	void TreeSetX(JXMenuBar* menuBar, const JString& pathName,
					 PathInput* pathInput, JXCurrentPathMenu* pathMenu,
					 TrashButton* trashButton, 
					 const JCoordinate w, const JCoordinate h);
};

/******************************************************************************
 GetTable

 ******************************************************************************/

inline FileTreeTable*
TreeSet::GetTable()
	const
{
	return itsTable;
}

#endif
