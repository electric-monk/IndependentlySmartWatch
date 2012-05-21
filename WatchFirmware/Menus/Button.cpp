#include "Button.h"
#include "Bitmap.h"
#include "MenuResources/right.h"

#define BUTTON_PADDING			2

Button::Button()
{
	SetFocusable(true);
}

void Button::Paint(Bitmap *destination, int x, int y, int width, bool selected)
{
	int rightBound = x + width;
	int labelWidth = width - ((2 * BUTTON_PADDING) + right.Width());
	Label::Paint(destination, x, y, labelWidth, selected);
	if (selected)
		destination->FillRect(x + labelWidth, y, x + rightBound, y + Height(), true);
	destination->Blit(&right, x + labelWidth + BUTTON_PADDING, y + ((Height() - right.Height()) / 2), selected ? BLIT_POSTINVERT : 0);
}

void Button::OnPress(BUTTON_TYPE button)
{
	switch (button)
	{
		default:
			Label::OnPress(button);
			break;
		case btRightTop:
		case btRightCentre:
		case btRightBottom:
			Fire((void*)button);
			break;
	}
}
