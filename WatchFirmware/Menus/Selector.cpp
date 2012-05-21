#include <string.h>
#include "Font.h"
#include "Selector.h"
#include "Menu.h"
#include "MenuResources/left.h"
#include "MenuResources/right.h"

#define FIELD_PADDING		2

Selector::Selector()
{
	m_entries = NULL;
	m_selected = NULL;
	SetFocusable(true);	// Default to focusable
}

Selector::~Selector()
{
	while (m_entries != NULL)
		delete m_entries;
}

void Selector::Add(int value, const char *text)
{
	Entry *entry = new Entry(&m_entries, value, text);
	if (m_selected == NULL)
		m_selected = entry;
}

void Selector::Remove(int value)
{
	for (Entry *entry = m_entries; entry != NULL; entry = entry->next)
	{
		if (entry->value == value)
		{
			if (m_selected == entry)
			{
				m_selected = (entry->next != NULL) ? entry->next : entry->last;
				// TODO: Repaint
			}
			delete entry;
			return;
		}
	}
}

void Selector::Select(int value)
{
	for (Entry *entry = m_entries; entry != NULL; entry = entry->next)
	{
		if (entry->value == value)
		{
			m_selected = entry;
			if (Container() != NULL)
				Container()->Refresh(this);
			return;
		}
	}
}

int Selector::Selected(void)
{
	if (m_selected == NULL)
		return -1;
	return m_selected->value;
}

void Selector::Paint(Bitmap *destination, int x, int y, int width, bool selected)
{
	int leftSize, textSize, rightSize;
	int totalWidth;
	int leftPos, rightPos, text;
	char *label = "<none>";

	if (m_selected != NULL)
		label = m_selected->text;
	textSize = m_font->Width(label) + (FIELD_PADDING * 2);
	leftSize = left.Width();
	rightSize = right.Width();
	totalWidth = leftSize + textSize + rightSize + (FIELD_PADDING * 2);

	switch (Alignment())
	{
		default:
		case caLeft:
			leftPos = x;
			break;
		case caCentre:
			leftPos = x + ((width - totalWidth) / 2);
			break;
		case caRight:
			leftPos = x + width - totalWidth;
			break;
	}
	text = leftPos + leftSize + FIELD_PADDING;
	rightPos = text + textSize + FIELD_PADDING;

	if (selected)
		destination->FillRect(text, y, text + textSize, y + m_font->Height(), true);

	m_font->Print(destination, text + FIELD_PADDING, y, label, -1, selected);

	if (CanFocus())
	{
		destination->Blit(&left, leftPos, y + ((Height() - left.Height()) / 2));
		destination->Blit(&right, rightPos, y + ((Height() - right.Height()) / 2));
	}
}

void Selector::OnPress(BUTTON_TYPE button)
{
	switch (button)
	{
		default:
			Control::OnPress(button);
			break;
		case btRightTop:
			if (m_selected == NULL)
				return;
			m_selected = m_selected->last;
			if (m_selected == NULL)
			{
				for (m_selected = m_entries; m_selected->next != NULL; m_selected = m_selected->next);
			}
			Container()->Refresh(this);
			Fire((void*)m_selected->value);
			break;
		case btRightBottom:
			if (m_selected == NULL)
				return;
			m_selected = m_selected->next;
			if (m_selected == NULL)
				m_selected = m_entries;
			Container()->Refresh(this);
			Fire((void*)m_selected->value);
			break;
	}
}

int Selector::Height(void)
{
	return m_font->Height();
}

Selector::Entry::Entry(Entry **root, int value, const char *label)
{
	m_root = root;

	this->value = value;
	int len = strlen(label);
	this->text = new char[len + 1];
	strcpy(this->text, label);

	next = NULL;
	last = *m_root;
	if (last == NULL)
		(*m_root) = this;
	else
	{
		while (last->next != NULL)
			last = last->next;
		last->next = this;
	}
}

Selector::Entry::~Entry()
{
	if (last != NULL)
		last->next = next;
	if (next != NULL)
		next->last = last;
	if ((*m_root) == this)
		(*m_root) = (next == NULL) ? last : next;

	delete[] text;
}
