#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Label.h"

class Button : public Label
{
public:
	Button();

	void Paint(Bitmap *destination, int x, int y, int width, bool selected);
	void OnPress(BUTTON_TYPE button);
};

#endif // __BUTTON_H__
