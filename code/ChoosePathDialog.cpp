/******************************************************************************
 ChoosePathDialog.cpp

	Clients must call BuildWindow() after constructing the object.
	Derived classes must override BuildWindow() and call SetObjects()
	at the end of their implementation.

	BASE CLASS = JXChoosePathDialog

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include <ChoosePathDialog.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXTextCheckbox.h>
#include <jx-af/jx/JXPathInput.h>
#include <jx-af/jx/JXPathHistoryMenu.h>
#include <jx-af/jx/JXNewDirButton.h>
#include <jx-af/jx/JXScrollbarSet.h>
#include <jx-af/jx/JXCurrentPathMenu.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jGlobals.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

ChoosePathDialog::ChoosePathDialog
	(
	JXDirector*		supervisor,
	JDirInfo*		dirInfo,
	const JString&	fileFilter,
	const bool	selectOnlyWritable
	)
	:
	JXChoosePathDialog(supervisor, dirInfo, fileFilter, selectOnlyWritable)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

ChoosePathDialog::~ChoosePathDialog()
{
}

/******************************************************************************
 BuildWindow

 ******************************************************************************/

void
ChoosePathDialog::BuildWindow
	(
	const bool	newWindow,
	const JString&	message
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 310,400, JString::empty);
	assert( window != nullptr );

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,310, 70,20);
	assert( cancelButton != nullptr );
	cancelButton->SetShortcuts(JGetString("cancelButton::ChoosePathDialog::shortcuts::JXLayout"));

	auto* homeButton =
		jnew JXTextButton(JGetString("homeButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 250,140, 40,20);
	assert( homeButton != nullptr );
	homeButton->SetShortcuts(JGetString("homeButton::ChoosePathDialog::shortcuts::JXLayout"));

	auto* pathLabel =
		jnew JXStaticText(JGetString("pathLabel::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,20, 40,20);
	assert( pathLabel != nullptr );
	pathLabel->SetToLabel();

	auto* scrollbarSet =
		jnew JXScrollbarSet(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 20,140, 180,190);
	assert( scrollbarSet != nullptr );

	itsSelectButton =
		jnew JXTextButton(JGetString("itsSelectButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,250, 70,20);
	assert( itsSelectButton != nullptr );
	itsSelectButton->SetShortcuts(JGetString("itsSelectButton::ChoosePathDialog::shortcuts::JXLayout"));

	auto* pathInput =
		jnew JXPathInput(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 60,20, 200,20);
	assert( pathInput != nullptr );

	auto* showHiddenCB =
		jnew JXTextCheckbox(JGetString("showHiddenCB::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,80, 130,20);
	assert( showHiddenCB != nullptr );
	showHiddenCB->SetShortcuts(JGetString("showHiddenCB::ChoosePathDialog::shortcuts::JXLayout"));

	auto* filterLabel =
		jnew JXStaticText(JGetString("filterLabel::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,50, 40,20);
	assert( filterLabel != nullptr );
	filterLabel->SetToLabel();

	auto* filterInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 60,50, 200,20);
	assert( filterInput != nullptr );

	auto* explanText =
		jnew JXStaticText(JGetString("explanText::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,340, 270,20);
	assert( explanText != nullptr );
	explanText->SetToLabel();

	auto* pathHistory =
		jnew JXPathHistoryMenu(1, JString::empty, window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,20, 30,20);
	assert( pathHistory != nullptr );

	auto* filterHistory =
		jnew JXStringHistoryMenu(1, JString::empty, window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,50, 30,20);
	assert( filterHistory != nullptr );

	auto* upButton =
		jnew JXTextButton(JGetString("upButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,140, 30,20);
	assert( upButton != nullptr );
	upButton->SetShortcuts(JGetString("upButton::ChoosePathDialog::shortcuts::JXLayout"));

	auto* newDirButton =
		jnew JXNewDirButton(window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,190, 70,20);
	assert( newDirButton != nullptr );

	itsNewWindowCB =
		jnew JXTextCheckbox(JGetString("itsNewWindowCB::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,370, 150,20);
	assert( itsNewWindowCB != nullptr );
	itsNewWindowCB->SetShortcuts(JGetString("itsNewWindowCB::ChoosePathDialog::shortcuts::JXLayout"));

	auto* currPathMenu =
		jnew JXCurrentPathMenu(JGetRootDirectory(), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 20,110, 180,20);
	assert( currPathMenu != nullptr );

	itsOpenButton =
		jnew JXTextButton(JGetString("itsOpenButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,280, 70,20);
	assert( itsOpenButton != nullptr );
	itsOpenButton->SetShortcuts(JGetString("itsOpenButton::ChoosePathDialog::shortcuts::JXLayout"));

	auto* desktopButton =
		jnew JXTextButton(JGetString("desktopButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,160, 70,20);
	assert( desktopButton != nullptr );

// end JXLayout

	itsNewWindowCB->SetState(newWindow);

	SetObjects(scrollbarSet, pathLabel, pathInput, pathHistory,
			   filterLabel, filterInput, filterHistory,
			   itsOpenButton, itsSelectButton, cancelButton,
			   upButton, homeButton, desktopButton,
			   newDirButton, showHiddenCB, currPathMenu, message);
}

/******************************************************************************
 OpenInNewWindow (public)

 ******************************************************************************/

bool
ChoosePathDialog::OpenInNewWindow()
	const
{
	return itsNewWindowCB->IsChecked();
}
