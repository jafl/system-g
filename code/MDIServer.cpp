/******************************************************************************
 MDIServer.cpp

	BASE CLASS = public JXMDIServer

	Copyright (C) 1997 by Glenn W. Bach.

 *****************************************************************************/

#include "MDIServer.h"
#include "globals.h"
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jx/JXChooseFileDialog.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 *****************************************************************************/

MDIServer::MDIServer()
	:
	JXMDIServer()
{
}

/******************************************************************************
 Destructor

 *****************************************************************************/

MDIServer::~MDIServer()
{
}

/******************************************************************************
 HandleMDIRequest

 *****************************************************************************/

void
MDIServer::HandleMDIRequest
	(
	const JString&				dir,
	const JPtrArray<JString>&	argList
	)
{
	const JSize argCount = argList.GetElementCount();
	Application* app  = GetApplication();

	const JString cwd = JGetCurrentDirectory();
	const JError err  = JChangeDirectory(dir);
	if (!err.OK())
	{
		err.ReportIfError();
		return;
	}

	bool restore = IsFirstTime();
	if (argCount == 2 && *(argList.GetLastElement()) == "--choose")
	{
		app->OpenDirectory();
		restore = false;
	}
	else if (argCount == 2 && *(argList.GetLastElement()) == "--open")
	{
		OpenFiles();
		if (IsFirstTime())
		{
			exit(0);
		}
	}
	else if (argCount == 2 && *(argList.GetLastElement()) == "--run")
	{
		JXFSBindingManager::Exec(dir);
	}
	else if (argCount > 1)
	{
		bool forceNew       = true;
		bool clearSelection = true;
		for (JIndex i=2; i<=argCount; i++)
		{
			const JString& arg = *(argList.GetElement(i));
			if (arg == "--no-force-new")
			{
				forceNew = false;
			}
			else if (arg == "--force-new")
			{
				forceNew = true;
			}
			else
			{
				JString path;
				if (JConvertToAbsolutePath(arg, JString::empty, &path))
				{
					app->OpenDirectory(path, nullptr, nullptr, true, true, forceNew, clearSelection);
					clearSelection = false;
				}
				restore = false;
			}
		}
	}

	JChangeDirectory(cwd);

	// if argCount == 1, restore guaranteed to be true

	if (restore && !app->RestoreProgramState())
	{
		app->OpenDirectory(dir);
	}
}

/******************************************************************************
 OpenFiles

 ******************************************************************************/

void
MDIServer::OpenFiles()
{
	auto* dlog = JXChooseFileDialog::Create(JXChooseFileDialog::kSelectMultipleFiles);
	if (dlog->DoDialog())
	{
		JPtrArray<JString> fileList(JPtrArrayT::kDeleteAll);
		dlog->GetFullNames(&fileList);
		JXFSBindingManager::Exec(fileList);
	}
}

/******************************************************************************
 PrintCommandLineHelp (static)

 ******************************************************************************/

void
MDIServer::PrintCommandLineHelp()
{
	const JUtf8Byte* map[] =
	{
		"version",   GetVersionNumberStr().GetBytes(),
		"copyright", JGetString("COPYRIGHT").GetBytes()
	};
	const JString s = JGetString("CommandLineHelp::MDIServer", map, sizeof(map));
	std::cout << std::endl << s << std::endl << std::endl;
}
