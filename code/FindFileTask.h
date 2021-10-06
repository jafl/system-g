/******************************************************************************
 FindFileTask.h

	Copyright @ 2008-09 by John Lindal.

 ******************************************************************************/

#ifndef _H_FindFileTask
#define _H_FindFileTask

#include <ace/LSOCK_Stream.h>
#include <ace/UNIX_Addr.h>
#include <jx-af/jcore/JMessageProtocol.h>	// template; requires ace includes

class JProcess;
class TreeDir;

class FindFileTask : public JBroadcaster
{
public:

	typedef JMessageProtocol<ACE_LSOCK_STREAM>	RecordLink;

public:

	static bool	Create(TreeDir* dir, const JString& path,
					   const JString& expr, FindFileTask** task);

	virtual ~FindFileTask();

protected:

	FindFileTask(TreeDir* dir, const JString& relPath,
					JProcess* p, int outFD, int errFD);

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveGoingAway(JBroadcaster* sender) override;

private:

	TreeDir*			itsDirector;		// not owned
	JPtrArray<JString>*	itsPathList;
	JProcess*			itsProcess;			// can be nullptr
	RecordLink*			itsMessageLink;		// can be nullptr
	RecordLink*			itsErrorLink;		// can be nullptr
	JString				itsErrors;			// cache while process is running
	bool				itsFoundFilesFlag;

private:

	void	ReceiveMessageLine();
	void	ReceiveErrorLine();
	void	DisplayErrors();

	static void	SplitPath(const JString& origRelPath, JPtrArray<JString>* pathList);

	// not allowed

	FindFileTask(const FindFileTask&) = delete;
	FindFileTask& operator=(const FindFileTask&) = delete;
};

#endif
