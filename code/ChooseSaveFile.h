/******************************************************************************
 ChooseSaveFile.h

	Interface for the ChooseSaveFile class.

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_ChooseSaveFile
#define _H_ChooseSaveFile

#include <jx-af/jx/JXChooseSaveFile.h>

class ChoosePathDialog;

class ChooseSaveFile : public JXChooseSaveFile
{

public:

	ChooseSaveFile(JPrefsManager* prefsMgr, const JPrefID& id);

	~ChooseSaveFile() override;

	void	ShouldOpenInNewWindow(const bool newWindow);
	bool	IsOpeningInNewWindow() const;

	void	SGReadSetup(std::istream& input);
	void	SGWriteSetup(std::ostream& output) const;

protected:

	JXChoosePathDialog*
	CreateChoosePathDialog(JXDirector* supervisor, JDirInfo* dirInfo,
						   const JString& fileFilter,
						   const bool selectOnlyWritable,
						   const JString& message) override;

	void	ReadPrefs(std::istream& input) override;
	void	WritePrefs(std::ostream& output) const override;

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	ChoosePathDialog*	itsPathDialog;
	bool				itsOpenInNewWindow;
};

#endif
