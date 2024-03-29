/******************************************************************************
 ColUtils.cpp

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "ColUtils.h"
#include <jx-af/jcore/jGlobals.h>
#include <jx-af/jcore/jAssert.h>

const JCoordinate kUIDWidth		= 60;
const JCoordinate kGIDWidth		= 60;
const JCoordinate kSizeWidth	= 60;
const JCoordinate kModeWidth	= 90;
const JCoordinate kDateWidth	= 180;

const JCoordinate kGFMBufferWidth = 6;

/******************************************************************************
 InsertFMCol

 ******************************************************************************/

bool
InsertFMCol
	(
	bool				visibleCols[],
	const JIndex		leadingCols,
	const GFMColType	type,
	JIndex*				index,
	JCoordinate*		retWidth
	)
{
	if (GFMTypeVisible(visibleCols, type))
	{
		return false;
	}

	*index = leadingCols + 1;

	if (type == kGFMUID)
	{
		*retWidth = kUIDWidth;
		visibleCols[kGFMUID] = true;
		return true;
	}

	if (type >= kGFMGID)
	{
		bool found = false;
		JSize i = 3;

		while(i <= GetFMColCount(visibleCols, leadingCols) && !found)
		{
			if (GetFMColType(i, leadingCols, visibleCols) > type)
			{
				*index = i;
				found = true;
			}
			i++;
		}

		if (!found)
		{
			*index = GetFMColCount(visibleCols, leadingCols)+1;
		}

		if (type == kGFMUID)
		{
			*retWidth = kUIDWidth;
			visibleCols[kGFMUID] = true;
		}

		else if (type == kGFMGID)
		{
			*retWidth = kGIDWidth;
			visibleCols[kGFMGID] = true;
		}

		else if (type == kGFMSize)
		{
			*retWidth = kSizeWidth;
			visibleCols[kGFMSize] = true;
		}

		else if (type == kGFMMode)
		{
			*retWidth = kModeWidth;
			visibleCols[kGFMMode] = true;
		}

		else if (type == kGFMDate)
		{
			*retWidth = kDateWidth;
			visibleCols[kGFMDate] = true;
		}
	}

	return true;
}

/******************************************************************************
 RemoveFMCol

 ******************************************************************************/

bool
RemoveFMCol
	(
	bool				visibleCols[],
	const JIndex		leadingCols,
	const GFMColType	type,
	JIndex*				index
	)
{
	JSize i		= leadingCols + 1;
	bool found	= false;

	while (i <= GetFMColCount(visibleCols, leadingCols) && !found)
	{
		if (type == GetFMColType(i, leadingCols, visibleCols))
		{
			*index = i;
			found = true;
		}
		i++;
	}

	if (!found)
	{
		return false;
	}

	if (type == kGFMUID)
	{
		visibleCols[kGFMUID] = false;
	}
	else if (type == kGFMGID)
	{
		visibleCols[kGFMGID] = false;
	}
	else if (type == kGFMSize)
	{
		visibleCols[kGFMSize] = false;
	}
	else if (type == kGFMMode)
	{
		visibleCols[kGFMMode] = false;
	}
	else if (type == kGFMDate)
	{
		visibleCols[kGFMDate] = false;
	}

	return true;
}

/******************************************************************************
 GetFMColCount

 ******************************************************************************/

JSize
GetFMColCount
	(
	const bool		visibleCols[],
	const JIndex	leadingCols
	)
{
	JSize temp = leadingCols;

	if (visibleCols[kGFMUID])
	{
		temp++;
	}

	if (visibleCols[kGFMGID])
	{
		temp++;
	}

	if (visibleCols[kGFMSize])
	{
		temp++;
	}

	if (visibleCols[kGFMMode])
	{
		temp++;
	}

	if (visibleCols[kGFMDate])
	{
		temp++;
	}

	return temp;
}

/******************************************************************************
 GetFMColWidth

 ******************************************************************************/

JSize
GetFMColWidth
	(
	const bool		visibleCols[],
	const JIndex	leadingCols,
	const JIndex	index
	)
{
	GFMColType type = GetFMColType(index, leadingCols, visibleCols);

	if (type == kGFMUID)
	{
		return kUIDWidth;
	}

	else if (type == kGFMGID)
	{
		return kGIDWidth;
	}

	else if (type == kGFMSize)
	{
		return kSizeWidth;
	}

	else if (type == kGFMMode)
	{
		return kModeWidth;
	}

	else if (type == kGFMDate)
	{
		return kDateWidth;
	}

	return 0;

}

/******************************************************************************
 GetFMColType

 ******************************************************************************/

GFMColType
GetFMColType
	(
	const JIndex	index,
	const JIndex	leadingCols,
	const bool	visibleCols[]
	)
{
	JIndex temp = leadingCols;

	if (visibleCols[kGFMUID] == true)
	{
		temp++;
		if (temp == index)
		{
			return kGFMUID;
		}
	}

	if (visibleCols[kGFMGID] == true)
	{
		temp++;
		if (temp == index)
		{
			return kGFMGID;
		}
	}

	if (visibleCols[kGFMSize] == true)
	{
		temp++;
		if (temp == index)
		{
			return kGFMSize;
		}
	}

	if (visibleCols[kGFMMode] == true)
	{
		temp++;
		if (temp == index)
		{
			return kGFMMode;
		}
	}

	if (visibleCols[kGFMDate] == true)
	{
		temp++;
		if (temp == index)
		{
			return kGFMDate;
		}
	}

	return kGFMName;
}

/******************************************************************************
 GFMTypeVisible

 ******************************************************************************/

bool
GFMTypeVisible
	(
	const bool			visibleCols[],
	const GFMColType	type
	)
{
	return
		(type == kGFMUID && visibleCols[kGFMUID])  ||
		(type == kGFMGID && visibleCols[kGFMGID])  ||
		(type == kGFMSize && visibleCols[kGFMSize]) ||
		(type == kGFMMode && visibleCols[kGFMMode]) ||
		(type == kGFMDate && visibleCols[kGFMDate]);
}

/******************************************************************************
 GetFMColTitle

 ******************************************************************************/

void
GetFMColTitle
	(
	const bool	visibleCols[],
	const JIndex	leadingCols,
	const JIndex	index,
	JString*		str
	)
{
	GFMColType type = GetFMColType(index, leadingCols, visibleCols);

	const JUtf8Byte* id = nullptr;
	if (type == kGFMUID)
	{
		id = "UIDColumnTitle::ColUtils";
	}
	else if (type == kGFMGID)
	{
		id = "GIDColumnTitle::ColUtils";
	}
	else if (type == kGFMSize)
	{
		id = "SizeColumnTitle::ColUtils";
	}
	else if (type == kGFMMode)
	{
		id = "PermissionsColumnTitle::ColUtils";
	}
	else if (type == kGFMDate)
	{
		id = "DateColumnTitle::ColUtils";
	}

	if (id != nullptr)
	{
		*str = JGetString(id);
	}
	else
	{
		str->Clear();
	}
}

/******************************************************************************
 GetFMBufferWidth

 ******************************************************************************/

JCoordinate
GetFMBufferWidth
	(
	const bool		visibleCols[],
	const JIndex	leadingCols,
	const JIndex	index
	)
{
	const GFMColType type = GetFMColType(index, leadingCols, visibleCols);
	return (type == kGFMSize ? -kGFMBufferWidth : kGFMBufferWidth);
}

/******************************************************************************
 GetRealColType

 ******************************************************************************/

GFMColType
GetRealColType
	(
	const bool		visibleCols[],
	const JIndex	leadingCols,
	const JIndex	index,
	JIndex*			subIndex
	)
{
	GFMColType type = GetFMColType(index, leadingCols, visibleCols);

	if (type <= kGFMMode ||
		(type == kGFMDate && !(GFMTypeVisible(visibleCols, kGFMMode))))
	{
		*subIndex = 1;
		return type;
	}

	JIndex i         = leadingCols + 1;
	bool found       = false;
	JIndex tempIndex = 0;

	while (i <= GetFMColCount(visibleCols, leadingCols) && !found)
	{
		if (GetFMColType(i, leadingCols, visibleCols) == kGFMMode)
		{
			tempIndex = i;
			found = true;
		}
		i++;
	}

	if (index - tempIndex >= 9)
	{
		*subIndex = 1;
		return kGFMDate;
	}

	*subIndex = index - tempIndex + 1;
	return kGFMMode;
}

/******************************************************************************
 GetFMWidthUsed

 ******************************************************************************/


JSize
GetFMWidthUsed
	(
	const bool visibleCols[]
	)
{
	JSize width = 0;

	if (GFMTypeVisible(visibleCols, kGFMUID))
	{
		width += kUIDWidth;
	}

	if (GFMTypeVisible(visibleCols, kGFMGID))
	{
		width += kGIDWidth;
	}

	if (GFMTypeVisible(visibleCols, kGFMSize))
	{
		width += kSizeWidth;
	}

	if (GFMTypeVisible(visibleCols, kGFMMode))
	{
		width += kModeWidth;
	}

	if (GFMTypeVisible(visibleCols, kGFMDate))
	{
		width += kDateWidth;
	}

	return width;
}
