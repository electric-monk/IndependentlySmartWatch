#include <string.h>
#include "Font.h"
#include "Menu.h"
#include "NumEntry.h"

#define DIGIT_PADDING			1

static unsigned char CharacterForDigit(unsigned char value)
{
	if (value < 10)
		return '0' + value;
	return 'A' + value - 10;
}

NumberEntry::NumberEntry()
{
	m_count = 0;
	m_index = 0;
	m_items = NULL;
	m_font = NULL;
	SetFocusable(true);
}

NumberEntry::~NumberEntry()
{
	if (m_items != NULL)
		delete[] m_items;
}

void NumberEntry::SetFormat(const char *format)
{
	if (m_items != NULL)
		delete[] m_items;
	m_index = 0;
	m_count = CountEntries(format);
	m_items = new HOLDER[m_count];
	for (int i = 0; i < m_count; i++)
		format = ParseEntry(format, m_items + i);
}

void NumberEntry::SetValueAt(int index, int value)
{
	m_items[index].value = value;
	if (Container() != NULL)
		Container()->Refresh(this);
}

int NumberEntry::RangeValueAt(int index, int length)
{
	int result;

	for (result = 0; length != 0; length--, index++)
	{
		result *= 10;
		result += ValueAt(index);
	}
	return result;
}

void NumberEntry::SetRangeValueAt(int index, int length, int value)
{
	for (index += length - 1; length != 0; length--, index--, value /= 10)
		m_items[index].value = value % 10;
	if (Container() != NULL)
		Container()->Refresh(this);
}

void NumberEntry::Paint(Bitmap *destination, int x, int y, int width, bool selected)
{
	int totalWidth;

	totalWidth = 0;
	for (int i = 0; i < m_count; i++)
		totalWidth += Width(m_items + i);

	switch (Alignment())
	{
		default:
		case caLeft:
			// x is already left
			break;
		case caCentre:
			x += (width - totalWidth) / 2;
			break;
		case caRight:
			x += width - totalWidth;
			break;
	}

	int itemHeight = Height();
	for (int i = 0; i < m_count; i++)
	{
		HOLDER *item = m_items + i;
		int itemWidth = Width(item);
		bool invert = selected && (m_index == i);
		if (invert)
			destination->FillRect(x, y, x + itemWidth, y + itemHeight, true);
		Print(destination, item, x, y, invert);
		x += itemWidth;
	}
}

void NumberEntry::OnPress(BUTTON_TYPE button)
{
	switch (button)
	{
		default:
			Control::OnPress(button);
			break;
		case btRightTop:
			m_items[m_index].value++;
			if (m_items[m_index].value > m_items[m_index].max)
				m_items[m_index].value = m_items[m_index].min;
			Container()->Refresh(this);
			Fire((void*)m_index);
			break;
		case btRightCentre:
		{
			int find = m_index;
			do {
				find++;
				if (find >= m_count)
					find = 0;
				if (find == m_index)	// Sanity check
					break;
			} while (m_items[find].type != tDigit);
			m_index = find;
			Container()->Refresh(this);
		}
			break;
		case btRightBottom:
			m_items[m_index].value--;
			if (m_items[m_index].value < m_items[m_index].min)
				m_items[m_index].value = m_items[m_index].max;
			Container()->Refresh(this);
			Fire((void*)m_index);
			break;
	}
}

int NumberEntry::Height(void)
{
	return m_font->Height();
}

const char* NumberEntry::ParseEntry(const char *input, HOLDER *entry)
{
	if ((*input) == '[')
	{
		input++;
		// Parse minimum
		entry->min = 0;
		while (((*input) != '-') && ((*input) != '\0'))
		{
			entry->min *= 10;
			entry->min += (*input) - '0';
			input++;
		}
		if ((*input) == '-')
			input++;
		// Parse maximum
		entry->max = 0;
		while (((*input) != ']') && ((*input) != '\0'))
		{
			entry->max *= 10;
			entry->max += (*input) - '0';
			input++;
		}
		if ((*input) == ']')
			input++;
		// Done
		entry->type = tDigit;
		entry->value = entry->min;
		return input;
	}
	entry->type = tCharacter;
	entry->value = *input;
	return input + 1;
}

int NumberEntry::CountEntries(const char *input)
{
	int count = 0;
	bool ignore = false;
	while ((*input) != '\0')
	{
		if (ignore)
		{
			if ((*input) == ']')
				ignore = false;
		}
		else
		{
			if ((*input) == '[')
				ignore = true;
			count++;
		}
		input++;
	}
	return count;
}

char NumberEntry::GetCharacter(HOLDER *entry)
{
	switch (entry->type)
	{
		default:
			return 0;
		case tDigit:
			return CharacterForDigit(entry->value);
		case tCharacter:
			return entry->value;
	}
}

void NumberEntry::Print(Bitmap *destination, HOLDER *entry, int x, int y, bool inverse)
{
	char temp = GetCharacter(entry);
	if (temp == 0)
		return;
	m_font->Print(destination, x + DIGIT_PADDING, y, &temp, 1, inverse);
}

int NumberEntry::Width(HOLDER *item)
{
	char temp = GetCharacter(item);
	if (temp == 0)
		return 0;
	return m_font->Width(&temp, 1) + (DIGIT_PADDING * 2);
}
