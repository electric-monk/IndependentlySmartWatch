#include "Menu.h"
#include "Control.h"
#include "Graphics.h"

Control::Control()
{
	m_container = NULL;
	m_last = m_next = NULL;
	m_target = NULL;
	m_message = -1;

	m_visible = true;
	m_focusable = false;
	m_alignment = caLeft;
}

void Control::SetVisible(bool visible)
{
	m_visible = visible;
	if (m_container != NULL)
		m_container->Update();	// TODO: Separate that out
}

void Control::OnPress(BUTTON_TYPE button)
{
	// Nothing by default
}

void Control::SetFocus(bool focus)
{
	// Nothing by default
}

void Control::Fire(void *data)
{
	if (m_target != NULL)
		m_target->Post(m_message, data);
}

void Control::Add(Menu *container)
{
	if (m_container != NULL)
		Remove();
	m_container = container;
	m_last = m_container->m_controls;
	if (m_last == NULL)
		m_container->m_controls = this;
	else
	{
		while (m_last->m_next != NULL)
			m_last = m_last->m_next;
		m_last->m_next = this;
	}
}

void Control::Remove(void)
{
	if (m_container == NULL)
		return;
	if (m_last != NULL)
		m_last->m_next = m_next;
	if (m_next != NULL)
		m_next->m_last = m_last;
	if (m_container->m_controls == this)
		m_container->m_controls = m_next;
	m_container = NULL;
	m_last = m_next = NULL;
}
