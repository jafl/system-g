/******************************************************************************
 SyGMDIServer.h

	Interface for the SyGMDIServer class

	Copyright (C) 1997 by Glenn W. Bach.

	Base code generated by Codemill v0.1.0

 *****************************************************************************/

#ifndef _H_SyGMDIServer
#define _H_SyGMDIServer

#include <JXMDIServer.h>

class SyGMDIServer : public JXMDIServer
{
public:

	SyGMDIServer();

	virtual	~SyGMDIServer();

	static void	PrintCommandLineHelp();

protected:

	virtual void	HandleMDIRequest(const JString& dir,
									 const JPtrArray<JString>& argList) override;

private:

	void	OpenFiles();
};

#endif
