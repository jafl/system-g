/******************************************************************************
 FindFileDialog.cpp

	BASE CLASS = JXWindowDirector, JPrefObject

	Copyright (C) 1996-2009 by John Lindal.

 ******************************************************************************/

#include "FindFileDialog.h"
#include "FindFileTask.h"
#include "ViewManPageDialog.h"
#include "PrefsMgr.h"
#include "globals.h"

#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXTextCheckbox.h>
#include <jx-af/jx/JXRadioGroup.h>
#include <jx-af/jx/JXTextRadioButton.h>
#include <jx-af/jx/JXPathInput.h>
#include <jx-af/jx/JXFontManager.h>
#include <jx-af/jx/JXCSFDialogBase.h>

#include <jx-af/jcore/jProcessUtil.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jAssert.h>

enum
{
	kFindFileAction = 1,
	kFindExprAction
};

// setup information

const JFileVersion kCurrentSetupVersion = 0;

/******************************************************************************
 Constructor

 ******************************************************************************/

FindFileDialog::FindFileDialog
	(
	JXDirector* supervisor
	)
	:
	JXWindowDirector(supervisor),
	JPrefObject(GetPrefsMgr(), kSFindFilePrefID)
{
	BuildWindow();
	JPrefObject::ReadPrefs();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

FindFileDialog::~FindFileDialog()
{
	// prefs written by DeleteGlobals()
}

/******************************************************************************
 Activate (virtual)

 ******************************************************************************/

void
FindFileDialog::Activate()
{
	JXWindowDirector::Activate();

	if (IsActive() && itsFileInput->IsActive())
	{
		itsFileInput->Focus();
		itsFileInput->SelectAll();
	}
	else if (IsActive() && itsExprInput->IsActive())
	{
		itsExprInput->Focus();
		itsExprInput->SelectAll();
	}
}

/******************************************************************************
 BuildWindow (protected)

 ******************************************************************************/

void
FindFileDialog::BuildWindow()
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 360,220, JString::empty);
	assert( window != nullptr );

	itsActionRG =
		jnew JXRadioGroup(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,50, 340,120);
	assert( itsActionRG != nullptr );

	itsCloseButton =
		jnew JXTextButton(JGetString("itsCloseButton::FindFileDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,190, 60,20);
	assert( itsCloseButton != nullptr );
	itsCloseButton->SetShortcuts(JGetString("itsCloseButton::FindFileDialog::shortcuts::JXLayout"));

	itsSearchButton =
		jnew JXTextButton(JGetString("itsSearchButton::FindFileDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 280,190, 60,20);
	assert( itsSearchButton != nullptr );
	itsSearchButton->SetShortcuts(JGetString("itsSearchButton::FindFileDialog::shortcuts::JXLayout"));

	itsFileInput =
		jnew JXInputField(itsActionRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 35,30, 295,20);
	assert( itsFileInput != nullptr );

	auto* fileLabel =
		jnew JXTextRadioButton(kFindFileAction, JGetString("fileLabel::FindFileDialog::JXLayout"), itsActionRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,10, 270,20);
	assert( fileLabel != nullptr );
	fileLabel->SetShortcuts(JGetString("fileLabel::FindFileDialog::shortcuts::JXLayout"));

	itsHelpButton =
		jnew JXTextButton(JGetString("itsHelpButton::FindFileDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 200,190, 60,20);
	assert( itsHelpButton != nullptr );
	itsHelpButton->SetShortcuts(JGetString("itsHelpButton::FindFileDialog::shortcuts::JXLayout"));

	itsStayOpenCB =
		jnew JXTextCheckbox(JGetString("itsStayOpenCB::FindFileDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,190, 90,20);
	assert( itsStayOpenCB != nullptr );

	itsExprInput =
		jnew JXInputField(itsActionRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 35,90, 295,20);
	assert( itsExprInput != nullptr );

	auto* findLabel =
		jnew JXTextRadioButton(kFindExprAction, JGetString("findLabel::FindFileDialog::JXLayout"), itsActionRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,70, 270,20);
	assert( findLabel != nullptr );
	findLabel->SetShortcuts(JGetString("findLabel::FindFileDialog::shortcuts::JXLayout"));

	auto* startLabel =
		jnew JXStaticText(JGetString("startLabel::FindFileDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,20, 50,20);
	assert( startLabel != nullptr );
	startLabel->SetToLabel();

	itsPathInput =
		jnew JXPathInput(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 70,20, 210,20);
	assert( itsPathInput != nullptr );

	itsChoosePathButton =
		jnew JXTextButton(JGetString("itsChoosePathButton::FindFileDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 280,20, 60,20);
	assert( itsChoosePathButton != nullptr );

// end JXLayout

	window->SetTitle(JGetString("WindowTitle::FindFileDialog"));
	window->SetCloseAction(JXWindow::kDeactivateDirector);
	window->PlaceAsDialogWindow();
	window->LockCurrentMinSize();
	window->ShouldFocusWhenShow(true);

	itsActionRG->SetBorderWidth(0);

	ListenTo(itsChoosePathButton);
	ListenTo(itsActionRG);
	ListenTo(itsSearchButton);
	ListenTo(itsCloseButton);
	ListenTo(itsHelpButton);

	itsPathInput->ShouldAllowInvalidPath();

	const JFont& font = JFontManager::GetDefaultMonospaceFont();

	itsFileInput->GetText()->SetCharacterInWordFunction(JXCSFDialogBase::IsCharacterInWord);
	itsFileInput->SetFont(font);
	ListenTo(itsFileInput);

	itsExprInput->GetText()->SetCharacterInWordFunction(JXCSFDialogBase::IsCharacterInWord);
	itsExprInput->SetFont(font);
	ListenTo(itsExprInput);

	itsStayOpenCB->SetState(true);

	UpdateDisplay();
}

/******************************************************************************
 UpdateDisplay (private)

 ******************************************************************************/

void
FindFileDialog::UpdateDisplay()
{
	const JIndex action = itsActionRG->GetSelectedItem();
	if (action == kFindFileAction)
	{
		itsExprInput->Deactivate();
		itsFileInput->Activate();
		itsFileInput->Focus();
		itsFileInput->SelectAll();
	}
	else if (action == kFindExprAction)
	{
		itsFileInput->Deactivate();
		itsExprInput->Activate();
		itsExprInput->Focus();
		itsExprInput->SelectAll();
	}

	UpdateButtons();
}

/******************************************************************************
 UpdateButtons (private)

 ******************************************************************************/

void
FindFileDialog::UpdateButtons()
{
	JXInputField* field = nullptr;

	const JIndex action = itsActionRG->GetSelectedItem();
	if (action == kFindFileAction)
	{
		field = itsFileInput;
	}
	else if (action == kFindExprAction)
	{
		field = itsExprInput;
	}

	assert( field != nullptr );
	if (field->GetText()->IsEmpty())
	{
		itsSearchButton->Deactivate();
	}
	else
	{
		itsSearchButton->Activate();
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
FindFileDialog::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsSearchButton && message.Is(JXButton::kPushed))
	{
		if (Search() && !itsStayOpenCB->IsChecked())
		{
			Deactivate();
		}
	}
	else if (sender == itsCloseButton && message.Is(JXButton::kPushed))
	{
		GetWindow()->KillFocus();
		Deactivate();
	}
	else if (sender == itsHelpButton && message.Is(JXButton::kPushed))
	{
		(GetManPageDialog())->ViewManPage(JString("find", JString::kNoCopy));
	}

	else if (sender == itsChoosePathButton && message.Is(JXButton::kPushed))
	{
		itsPathInput->ChoosePath(JString::empty);
	}

	else if (sender == itsActionRG && message.Is(JXRadioGroup::kSelectionChanged))
	{
		UpdateDisplay();
	}

	else if ((sender == itsFileInput || sender == itsExprInput) &&
			 (message.Is(JStyledText::kTextSet) ||
			  message.Is(JStyledText::kTextChanged)))
	{
		UpdateButtons();
	}

	else
	{
		JXWindowDirector::Receive(sender, message);
	}
}

/******************************************************************************
 Search (private)

 ******************************************************************************/

bool
FindFileDialog::Search()
{
	itsPathInput->ShouldAllowInvalidPath(false);
	if (!itsPathInput->InputValid())
	{
		itsPathInput->ShouldAllowInvalidPath(true);
		itsPathInput->Focus();
		return false;
	}
	itsPathInput->ShouldAllowInvalidPath(true);

	JString path;
	const bool ok = itsPathInput->GetPath(&path);
	assert( ok );

	const JIndex action = itsActionRG->GetSelectedItem();
	if (action == kFindFileAction && itsFileInput->InputValid())
	{
		SearchFileName(path, itsFileInput->GetText()->GetText());
	}
	else if (action == kFindExprAction && itsExprInput->InputValid())
	{
		SearchExpr(path, itsExprInput->GetText()->GetText());
	}

	return true;
}

/******************************************************************************
 Search

 ******************************************************************************/

void
FindFileDialog::Search
	(
	const JString& path
	)
{
	itsPathInput->GetText()->SetText(path);
	Activate();
}

/******************************************************************************
 SearchFileName

 ******************************************************************************/

void
FindFileDialog::SearchFileName
	(
	const JString& path,
	const JString& pattern
	)
{
	JString expr("-iname ");
	expr += JPrepArgForExec(pattern);

	SearchExpr(path, expr);
}

/******************************************************************************
 SearchExpr

 ******************************************************************************/

void
FindFileDialog::SearchExpr
	(
	const JString& path,
	const JString& expr
	)
{
	JString e("( ");
	e += expr;
	e += " )";

	const JUtf8Byte** vcsDirName;
	const JSize vcsDirNameCount = JGetVCSDirectoryNames(&vcsDirName);
	for (JUnsignedOffset i=0; i<vcsDirNameCount; i++)
	{
		e += " -a ! -path */";
		e += vcsDirName[i];
		e += "/*";
	}

	TreeDir* dir;
	if ((GetApplication())->OpenDirectory(path, &dir, nullptr, true, true, false, true))
	{
		FindFileTask* task;
		FindFileTask::Create(dir, path, e, &task);
	}
}

/******************************************************************************
 ReadPrefs (virtual)

 ******************************************************************************/

void
FindFileDialog::ReadPrefs
	(
	std::istream& input
	)
{
	JFileVersion vers;
	input >> vers;
	if (vers > kCurrentSetupVersion)
	{
		return;
	}

	JXWindow* window = GetWindow();
	window->ReadGeometry(input);
	window->Deiconify();

	bool stayOpen;
	input >> JBoolFromString(stayOpen);
	itsStayOpenCB->SetState(stayOpen);
}

/******************************************************************************
 WritePrefs (virtual)

 ******************************************************************************/

void
FindFileDialog::WritePrefs
	(
	std::ostream& output
	)
	const
{
	output << kCurrentSetupVersion;

	output << ' ';
	GetWindow()->WriteGeometry(output);

	output << ' ' << JBoolToString(itsStayOpenCB->IsChecked());
}
