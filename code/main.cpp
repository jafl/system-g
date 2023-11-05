/******************************************************************************
 systemg.cpp

	Copyright (C) 1997 by Glenn W. Bach.

 ******************************************************************************/

#include "Application.h"
#include "MDIServer.h"
#include "globals.h"
#include "PrefsMgr.h"
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jx/JXSplashWindow.h>
#include <jx-af/jcore/jCommandLine.h>
#include <jx-af/jcore/jTime.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jSysUtil.h>
#include <jx-af/jcore/jAssert.h>

void ParseTextOptions(const int argc, char* argv[]);
void PrintVersion();

/******************************************************************************
 main

 ******************************************************************************/

int
main
	(
	int		argc,
	char*	argv[]
	)
{
	ParseTextOptions(argc, argv);

	if (!MDIServer::WillBeMDIServer(Application::GetAppSignature(), argc, argv))
	{
		return 0;
	}

	bool displayAbout;
	JString prevVersStr;
	auto* app = jnew Application(&argc, argv, &displayAbout, &prevVersStr);

	JXApplication::StartFiber([argc, argv]()
	{
		GetMDIServer()->HandleCmdLineOptions(argc, argv);

		JXFSBindingManager::Initialize();	// notify user of any upgrades
	});

	if (displayAbout)
	{
		app->DisplayAbout(true, prevVersStr);
	}
	else
	{
		JCheckForNewerVersion(GetPrefsMgr(), kSVersionCheckID);
	}

	app->Run();
	return 0;
}

/******************************************************************************
 ParseTextOptions

	Handle the command line options that don't require opening an X display.

	Options that don't call exit() should use JXApplication::RemoveCmdLineOption()
	so ParseXOptions won't gag.

 ******************************************************************************/

void
ParseTextOptions
	(
	const int	argc,
	char*		argv[]
	)
{
	long index = 1;
	while (index < argc)
	{
		if (JIsVersionRequest(argv[index]))
		{
			Application::InitStrings();
			PrintVersion();
			exit(0);
		}
		else if (JIsHelpRequest(argv[index]))
		{
			Application::InitStrings();
			MDIServer::PrintCommandLineHelp();
			exit(0);
		}
		index++;
	}
}

/******************************************************************************
 PrintVersion

 ******************************************************************************/

void
PrintVersion()
{
	std::cout << std::endl;
	std::cout << GetVersionStr() << std::endl;
	std::cout << std::endl;
}
