#include "Menu.h"
#include "Control.h"
#include "Graphics.h"

Menu::Menu()
{
	m_controls = NULL;
	m_topView = NULL;
	m_focus = NULL;
}

Menu::~Menu()
{
	while (m_controls != NULL)
	{
		Control *item = m_controls;
		RemoveControl(item);
		delete item;
	}
}

void Menu::AddControl(Control *item)
{
	item->Add(this);
	if (m_topView == NULL)
		m_topView = item;
	if (m_focus == NULL)
	{
		for (m_focus = item; ((m_focus != NULL) && (!m_focus->CanFocus())); m_focus = m_focus->m_next);
		if (m_focus != NULL)
			m_focus->SetFocus(true);
	}
}

Control* Menu::SeekClosest(Control *item)
{
	if (item->m_next != NULL)
		return item->m_next;
	if (item->m_last != NULL)
		return item->m_last;
	return m_controls;
}

void Menu::RemoveControl(Control *item)
{
	if (item->m_container == this)
	{
		if (item == m_topView)
			m_topView = SeekClosest(m_topView);
		if (item == m_focus)
		{
			m_focus->SetFocus(false);
			// TODO: Only focus focusable controls
			if (m_focus != NULL)
				m_focus->SetFocus(true);
		}
		item->Remove();
	}
}

bool Menu::IsOnScreen(Control *item)
{
	int current, maximum;
	Control *display;

	maximum = m_destination->Height();
	current = 0;
	for (display = m_topView; display != NULL; display = display->m_next)
	{
		if (!display->Visible())
			continue;
		current += display->Height();
		if (current >= maximum)	// Off the bottom of the screen (even partially)
			break;
		if (item == display)
			return true;
	}
	return false;
}

int Menu::FindInList(Control *start, Control *find)
{
	Control *search;
	int i;

	for (search = start, i = 0; search != NULL; search = search->m_next, i++)
	{
		if (search == find)
			return i;
	}
	for (search = start, i = 0; search != NULL; search = search->m_last, i--)
	{
		if (search == find)
			return i;
	}
	return 0;	// Not found, or "find is start", is 0
}

void Menu::Update(void)
{
	if (!IsOnScreen(m_focus))
	{
		int dir = FindInList(m_topView, m_focus);
		if (dir == 0)
		{
			// Item not in list? Suspicious
			return;
		}
		if (dir < 0)
		{
			// First, move up until view is visible
			do
			{
				do
				{
					m_topView = m_topView->m_last;
				}
				while ((m_topView != NULL) && (!m_topView->Visible()));
			}
			while (!IsOnScreen(m_focus));
			// Second, optionally find top label
			Control *label = m_topView->m_last;
			while ((label != NULL) && ((!label->Visible()) || label->CanFocus()))
				label = label->m_last;
			if (label != NULL)
			{
				Control *minTop = m_topView;
				m_topView = label;
				if (!IsOnScreen(m_focus))
					m_topView = minTop;
			}
		}
		else
		{
			// Simply move down until item visible
			do
			{
				do
				{
					m_topView = m_topView->m_next;
				}
				while (!m_topView->Visible());
			}
			while (!IsOnScreen(m_focus));
		}
	}
	OnPaint(m_destination);
}

void Menu::Move(bool down)
{
	Control *find = m_focus;
	if (find == NULL)
		return;
	do {
		if (down)
		{
			find = find->m_next;
			if (find == NULL)
				find = m_controls;
		}
		else
		{
			find = find->m_last;
			if (find == NULL)
			{
				find = m_controls;
				while (find->m_next != NULL)
					find = find->m_next;
			}
		}
		if (find == m_focus)
		{
			// Avoid an infinite loop, if we're on the input it must be the only one (or none)
			return;
		}
	} while((!find->CanFocus()) || (!find->Visible()));
	m_focus->SetFocus(false);
	m_focus = find;
	m_focus->SetFocus(true);
	Update();
}

void Menu::OnButton(BUTTON_TYPE button, BUTTON_EVENT event)
{
	if (event != beDown)
		return;
	switch (button)
	{
		case btLeftTop:			// Scroll up
			Move(false);
			break;
		case btLeftCentre:		// Exit menu
			Application::Pop(this);
			break;
		case btLeftBottom:		// Scroll down
			Move(true);
			break;
		case btRightTop:		// Scroll left
		case btRightCentre:		// Next field
		case btRightBottom:		// Scroll right
			if (m_focus != NULL)
				m_focus->OnPress(button);
			break;
	}
}

void Menu::OnPaint(Bitmap *destination)
{
	m_destination = destination;
	Refresh(NULL);
}

void Menu::Refresh(Control *refresh)
{
	int width = m_destination->Width();
	int maximum = m_destination->Height();
	m_destination->Begin();
	if (refresh == NULL)
		m_destination->FillRect(0, 0, width, maximum, false);
	Control *item = m_topView;
	for (int current = 0; (item != NULL) && (current < maximum); item = item->m_next)
	{
		if (!item->Visible())
			continue;
		int height = item->Height();
		if ((current + height) >= maximum)
			break;
		if (refresh == item)
		{
			m_destination->FillRect(0, current, width, current + height, false);
			item->Paint(m_destination, 0, current, width, item == m_focus);
			break;	// Only need to draw one
		}
		if (refresh == NULL)	// I hate code duplication
			item->Paint(m_destination, 0, current, width, item == m_focus);
		current += height;
	}
	m_destination->End();
}
