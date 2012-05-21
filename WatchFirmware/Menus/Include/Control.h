#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "Buttons.h"

class Application;
class Bitmap;
class Menu;

typedef enum {
	caLeft,
	caCentre,
	caRight,
} ControlAlignment;

class Control
{
public:
	Control();

	Application* Target(void) { return m_target; }
	void SetTarget(Application *target) { m_target = target; }

	unsigned short Message(void) { return m_message; }
	void SetMessage(unsigned short message) { m_message = message; }

	bool Visible(void) { return m_visible; }
	void SetVisible(bool visible);

	bool CanFocus(void) { return m_focusable; }
	void SetFocusable(bool focusable) { m_focusable = focusable; }

	ControlAlignment Alignment(void) { return m_alignment; }
	void SetAlignment(ControlAlignment alignment) { m_alignment = alignment; }

	virtual void Paint(Bitmap *destination, int x, int y, int width, bool selected) = 0;
	virtual void OnPress(BUTTON_TYPE button);
	virtual void SetFocus(bool focus);

	virtual int Height(void) = 0;

protected:
	void Fire(void *data);

	Menu* Container(void) { return m_container; }

private:
	friend class Menu;
	Menu *m_container;
	Control *m_last, *m_next;

	void Add(Menu *container);
	void Remove(void);

private:
	Application *m_target;
	unsigned short m_message;

	bool m_visible, m_focusable;
	ControlAlignment m_alignment;
};

#endif // __CONTROL_H__
