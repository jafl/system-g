/******************************************************************************
 Application.h

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_Application
#define _H_Application

#include <jx-af/jx/JXApplication.h>
#include <jx-af/jcore/JPrefObject.h>
#include <jx-af/jcore/JStringPtrMap.h>
#include <jx-af/jcore/jMountUtil.h>

class JMountPointList;
class JXWindowDirector;
class JXTextMenu;
class JXToolBar;
class TreeDir;

class Application : public JXApplication, public JPrefObject
{
public:

	Application(int* argc, char* argv[], bool* displayAbout,
				   JString* prevVersStr);

	~Application() override;

	void	Quit() override;

	void	OpenDirectory();
	bool	OpenDirectory(const JString& pathName,
						  TreeDir** dir = nullptr, JIndex* row = nullptr,
						  const bool deiconify = true,
						  const bool reportError = true,
						  const bool forceNew = true,
						  const bool clearSelection = true);

	JSize	GetWindowCount() const;
	void	GetWindowNames(JPtrArray<JString>* names);

	void	UpdateShortcutMenu(JXTextMenu* menu) const;
	void	LoadToolBarDefaults(JXToolBar* toolBar, JXTextMenu* shorcutMenu,
								const JIndex firstIndex);
	void	OpenShortcut(const JIndex index);
	void	AddShortcut(const JString& shortcut);
	void	RemoveShortcut(const JString& shortcut);
	void	RemoveAllShortcuts();

	bool	IsMountPoint(const JString& path, JMountType* type = nullptr) const;
	bool	GetMountPointPrefs(const JString& path, const JString** prefs) const;
	void	SetMountPointPrefs(const JString& path, const JString& prefs);

	void	DirectoryRenamed(const Message& message);
	void	UpdateTrash();

	void			OpenTerminal(const JString& path);
	const JString&	GetTerminalCommand() const;
	void			SetTerminalCommand(const JString& cmd);

	const JString&	GetGitStatusCommand() const;
	void			SetGitStatusCommand(const JString& cmd);

	const JString&	GetGitHistoryCommand() const;
	void			SetGitHistoryCommand(const JString& cmd);

	const JString&	GetPostCheckoutCommand() const;
	void			SetPostCheckoutCommand(const JString& cmd);

	void	DisplayAbout(const bool showLicense = false,
						 const JString& prevVersStr = JString::empty);

	bool	RestoreProgramState();
	void	SaveProgramState();

	static const JUtf8Byte*	GetAppSignature();
	static void				InitStrings();

protected:

	void	ReadPrefs(std::istream& input) override;
	void	WritePrefs(std::ostream& output) const override;

	void	DirectorClosed(JXDirector* theDirector) override;
	void	CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason) override;
	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	JPtrArray<TreeDir>*		itsWindowList;
	JMountPointList*		itsMountPointList;
	mutable JMountState		itsMountPointState;
	JPtrArray<JString>*		itsShortcutList;
	JStringPtrMap<JString>*	itsMountPointPrefs;

	JString	itsTermCmd;
	JString	itsGitStatusCmd;
	JString	itsGitHistoryCmd;
	JString	itsPostCheckoutCmd;

private:

	JString	GetNMShortcut(JIndex* i) const;

	void	UpdateMountPointList() const;
	JString	GetMountPointPrefsPath(const JString& path) const;

	static JListT::CompareResult
		CompareShortcuts(JString* const & s1, JString* const & s2);

public:

	static const JUtf8Byte* kTrashNeedsUpdate;
	static const JUtf8Byte* kShortcutsChanged;

	class TrashNeedsUpdate : public JBroadcaster::Message
		{
		public:

			TrashNeedsUpdate()
				:
				JBroadcaster::Message(kTrashNeedsUpdate)
			{ };
		};

	class ShortcutsChanged : public JBroadcaster::Message
		{
		public:

			ShortcutsChanged()
				:
				JBroadcaster::Message(kShortcutsChanged)
			{ };
		};
};


/******************************************************************************
 GetWindowCount

 ******************************************************************************/

inline JSize
Application::GetWindowCount()
	const
{
	return itsWindowList->GetElementCount();
}

/******************************************************************************
 RemoveAllShortcuts

 ******************************************************************************/

inline void
Application::RemoveAllShortcuts()
{
	itsShortcutList->CleanOut();
}

/******************************************************************************
 Terminal command

 ******************************************************************************/

inline const JString&
Application::GetTerminalCommand()
	const
{
	return itsTermCmd;
}

inline void
Application::SetTerminalCommand
	(
	const JString& cmd
	)
{
	itsTermCmd = cmd;
	itsTermCmd.TrimWhitespace();
}

/******************************************************************************
 Git status

 ******************************************************************************/

inline const JString&
Application::GetGitStatusCommand()
	const
{
	return itsGitStatusCmd;
}

inline void
Application::SetGitStatusCommand
	(
	const JString& cmd
	)
{
	itsGitStatusCmd = cmd;
	itsGitStatusCmd.TrimWhitespace();
}

/******************************************************************************
 Git history

 ******************************************************************************/

inline const JString&
Application::GetGitHistoryCommand()
	const
{
	return itsGitHistoryCmd;
}

inline void
Application::SetGitHistoryCommand
	(
	const JString& cmd
	)
{
	itsGitHistoryCmd = cmd;
	itsGitHistoryCmd.TrimWhitespace();
}

/******************************************************************************
 Post-checkout command

 ******************************************************************************/

inline const JString&
Application::GetPostCheckoutCommand()
	const
{
	return itsPostCheckoutCmd;
}

inline void
Application::SetPostCheckoutCommand
	(
	const JString& cmd
	)
{
	itsPostCheckoutCmd = cmd;
	itsPostCheckoutCmd.TrimWhitespace();
}

#endif
