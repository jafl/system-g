/******************************************************************************
 ChooseSaveFile.cpp

	Class to test overriding of JXChooseSaveFile dialog windows.

	BASE CLASS = JXChooseSaveFile

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "ChooseSaveFile.h"
#include "ChoosePathDialog.h"
#include <jx-af/jcore/jAssert.h>

const JFileVersion kCurrentPrefsVersion	= 1;

/******************************************************************************
 Constructor

 ******************************************************************************/

ChooseSaveFile::ChooseSaveFile
	(
	JPrefsManager*	prefsMgr,
	const JPrefID&	id
	)
	:
	JXChooseSaveFile(prefsMgr, id),
	itsPathDialog(nullptr),
	itsOpenInNewWindow(true)
{
	JPrefObject::ReadPrefs();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

ChooseSaveFile::~ChooseSaveFile()
{
	JPrefObject::WritePrefs();
}

/******************************************************************************
 CreateChoosePathDialog (virtual protected)

 ******************************************************************************/

JXChoosePathDialog*
ChooseSaveFile::CreateChoosePathDialog
	(
	JXDirector*		supervisor,
	JDirInfo*		dirInfo,
	const JString&	fileFilter,
	const bool	selectOnlyWritable,
	const JString&	message
	)
{
	itsPathDialog =
		jnew ChoosePathDialog(supervisor, dirInfo, fileFilter, selectOnlyWritable);
	assert( itsPathDialog != nullptr );
	itsPathDialog->BuildWindow(itsOpenInNewWindow, message);
	return itsPathDialog;
}

/******************************************************************************
 New window

 ******************************************************************************/

bool
ChooseSaveFile::IsOpeningInNewWindow()
	const
{
	return itsOpenInNewWindow;
}

void
ChooseSaveFile::ShouldOpenInNewWindow
	(
	const bool newWindow
	)
{
	itsOpenInNewWindow = newWindow;
}

/******************************************************************************
 Receive (virtual protected)

	Listen for response from itsSaveDialog.

 ******************************************************************************/

void
ChooseSaveFile::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsPathDialog && message.Is(JXDialogDirector::kDeactivated))
	{
		const auto* info =
			dynamic_cast<const JXDialogDirector::Deactivated*>(&message);
		assert( info != nullptr );
		if (info->Successful())
		{
			itsOpenInNewWindow = itsPathDialog->OpenInNewWindow();
		}
		itsPathDialog = nullptr;
	}

	JXChooseSaveFile::Receive(sender, message);
}

/******************************************************************************
 Read/WriteSetup

 ******************************************************************************/

void
ChooseSaveFile::SGReadSetup
	(
	std::istream& input
	)
{
	JFileVersion version;
	input >> version;
	if (version > kCurrentPrefsVersion)
	{
		return;
	}

	input >> JBoolFromString(itsOpenInNewWindow);
	JXChooseSaveFile::ReadSetup(input);
}

void
ChooseSaveFile::SGWriteSetup
	(
	std::ostream& output
	)
	const
{
	output << kCurrentPrefsVersion << ' ';
	output << JBoolToString(itsOpenInNewWindow) << ' ';
	JXChooseSaveFile::WriteSetup(output);
}

/******************************************************************************
 ReadPrefs (virtual protected)

 ******************************************************************************/

void
ChooseSaveFile::ReadPrefs
	(
	std::istream& input
	)
{
	SGReadSetup(input);
}

/******************************************************************************
 WritePrefs (virtual protected)

 ******************************************************************************/

void
ChooseSaveFile::WritePrefs
	(
	std::ostream& output
	)
	const
{
	SGWriteSetup(output);
}
