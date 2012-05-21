#ifndef __MENU_H__
#define __MENU_H__

#include "Application.h"

class Control;

class Menu : public Application
{
public:
	Menu();
	~Menu();

	void AddControl(Control *item);
	void RemoveControl(Control *item);

	void Update(void);
	void Refresh(Control *item);

	void SetBitmap(Bitmap *destination);

protected:
	void OnButton(BUTTON_TYPE button, BUTTON_EVENT event);
	void OnPaint(Bitmap *destination);

private:
	friend class Control;
	Control *m_controls;

	void Move(bool down);
	Control* SeekClosest(Control *item);
	bool IsOnScreen(Control *item);
	static int FindInList(Control *start, Control *find);

	Bitmap *m_destination;

	Control *m_topView;
	Control *m_focus;
};

#endif // __MENU_H__
