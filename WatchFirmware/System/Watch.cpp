#include "Watch.h"
#include "Graphics.h"
#include "MainMenu.h"

WatchFace::WatchFace()
{
	m_owner = NULL;
	m_visibles = 0;
	m_last = NULL;
	m_next = NULL;
}

WatchFace::~WatchFace()
{
	Remove();
}

void WatchFace::OnPaintAll(Bitmap *destination)
{
	int y = SplitHeight();
	OnPaintUpper(destination, y);
	OnPaintLower(destination, y);
}

void WatchFace::OnShow(int type)
{
	m_visibles |= type;
}

void WatchFace::OnHide(int type)
{
	m_visibles &= ~type;
}

void WatchFace::OnButton(BUTTON_TYPE button, BUTTON_EVENT event)
{
	if (m_owner == NULL)
		return;
	if (m_owner->m_top == NULL)
		return;
	if (m_owner->m_top != this)
		m_owner->m_top->OnButton(button, event);
}

void WatchFace::Add(WatchInterface *owner)
{
	Remove();
	m_owner = owner;
	m_next = NULL;
	m_last = m_owner->m_faces;
	if (m_last == NULL)
		m_owner->m_faces = this;
	else
	{
		while (m_last->m_next != NULL)
			m_last = m_last->m_next;
		m_last->m_next = this;
	}
}

void WatchFace::Remove(void)
{
	if (m_owner == NULL)
		return;

	if (m_last != NULL)
		m_last->m_next = m_next;
	if (m_next != NULL)
		m_next->m_last = m_last;
	if (m_owner->m_faces == this)
		m_owner->m_faces = (m_next != NULL) ? m_next : m_last;

	m_owner = NULL;
	m_last = NULL;
	m_next = NULL;
}

WatchInterface::WatchInterface()
{
	m_faces = NULL;
	m_top = NULL;
	m_bottom = NULL;
}

WatchInterface::~WatchInterface()
{
	while (m_faces != NULL)
	{
		WatchFace *face = m_faces;
		face->Remove();
		delete face;
	}
}

void WatchInterface::AddFace(WatchFace *face)
{
	face->Add(this);
	if (m_top == NULL)
	{
		if (face->AcceptUpper())
			m_top = face;
	}
	if (m_bottom == NULL)
	{
		if (face->AcceptLower())
			m_bottom = face;
	}
}

void WatchInterface::RemoveFace(WatchFace *face)
{
	if (face->m_owner != this)
		return;
	face->Remove();
}

void WatchInterface::Repaint(void)
{
	Post(MSG_PAINT, FrameBuffer);
}

void WatchInterface::OnMessage(unsigned short message, void *param)
{
	switch (message)
	{
		default:
			Application::OnMessage(message, param);
			break;
	}
}

void WatchInterface::OnShow(bool popping)
{
	Application::OnShow(popping);
	if (m_top != NULL)
		m_top->OnShow(SHOW_UPPER);
	if (m_bottom != NULL)
		m_bottom->OnShow(SHOW_LOWER);
}

void WatchInterface::OnHide(bool popping)
{
	Application::OnHide(popping);
	if (m_top != NULL)
		m_top->OnHide(SHOW_UPPER);
	if (m_bottom != NULL)
		m_bottom->OnHide(SHOW_LOWER);
}

void WatchInterface::OnButton(BUTTON_TYPE button, BUTTON_EVENT event)
{
	bool forward = false;
	switch (button)
	{
		default:
			forward = true;
			break;
#ifdef _COLIN_DEBUG
		case btLeftCentre:
#else
		case btLeftTop:
#endif
			switch (event)
			{
				case beDown:
					m_leftTopFired = false;
					break;
				case beHold:
					m_leftTopFired = true;
					forward = true;
					break;
				case beUp:
					if (!m_leftTopFired)
						Step(true);
					break;
			}
			break;
#ifdef _COLIN_DEBUG
		case btLeftTop:
#else
		case btLeftCentre:
#endif
			switch (event)
			{
				case beDown:
					m_leftCentreFired = false;
					break;
				case beHold:
					m_leftCentreFired = true;
					Application::Push(new MainMenu());
					break;
				case beUp:
					if (!m_leftCentreFired)
						forward = true;
					break;
			}
			break;
		case btLeftBottom:
			switch (event)
			{
				case beDown:
					m_leftBottomFired = false;
					break;
				case beHold:
					m_leftBottomFired = true;
					forward = true;
					break;
				case beUp:
					if (!m_leftBottomFired)
						Step(false);
					break;
			}
			break;
	}
	if (forward)
		m_bottom->OnButton(button, event);
}

void WatchInterface::OnPaint(Bitmap *destination)
{
	if (m_top == m_bottom)
		m_top->OnPaintAll(destination);
	else
	{
		int y = m_top->SplitHeight();
		m_top->OnPaintUpper(destination, y);
		m_bottom->OnPaintLower(destination, y);
	}
}

void WatchInterface::Step(bool upper)
{
	WatchFace *found;
	int flag;

	if (upper)
	{
		found = m_top;
		flag = SHOW_UPPER;
	}
	else
	{
		found = m_bottom;
		flag = SHOW_LOWER;
	}
	found->OnHide(flag);
	while (true)
	{
		if (upper ? found->AcceptUpper() : found->AcceptLower())
			break;
		found = found->m_next;
		if (found == NULL)
			found = m_faces;
	}
	found->OnShow(flag);
	if (upper)
		m_top = found;
	else
		m_bottom = found;

	Repaint();
}
