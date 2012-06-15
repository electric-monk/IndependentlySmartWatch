#ifndef __WATCH_H__
#define __WATCH_H__

#include "Application.h"

#define SHOW_UPPER		0x01
#define SHOW_LOWER		0x02

class WatchInterface;

class WatchFace
{
public:
	WatchFace();
	virtual ~WatchFace();

	virtual int SplitHeight(void) = 0;
	virtual void OnPaintUpper(Bitmap *destination, int height) = 0;
	virtual void OnPaintLower(Bitmap *destination, int y) = 0;
	virtual void OnPaintAll(Bitmap *destination);

	virtual void OnShow(int type);
	virtual void OnHide(int type);

	// Only certain events are fed - left-up hold, left-down hold, left-centre press, and all right keys.
	virtual void OnButton(BUTTON_TYPE button, BUTTON_EVENT event);

	// Returning false to these will result in the next face being selected
	virtual bool AcceptUpper(void) = 0;	// Return true if internally changed 'upper face' mode.
	virtual bool AcceptLower(void) = 0;

	WatchInterface* Owner(void) { return m_owner; }
	int Visible(void) { return m_visibles; }

private:
	friend class WatchInterface;

	void Add(WatchInterface *owner);
	void Remove(void);

	WatchInterface *m_owner;
	int m_visibles;
	WatchFace *m_last, *m_next;
};

class WatchInterface : public Application
{
public:
	WatchInterface();
	~WatchInterface();

	void AddFace(WatchFace *face);
	void RemoveFace(WatchFace *face);

	void Repaint(void);
protected:
	void OnMessage(unsigned short message, void *param);
	void OnShow(bool popping);
	void OnHide(bool popping);
	void OnButton(BUTTON_TYPE button, BUTTON_EVENT event);
	void OnPaint(Bitmap *destination);

private:
	void Step(bool upper);

	friend class WatchFace;
	WatchFace *m_faces;
	WatchFace *m_top, *m_bottom;

	bool m_leftBottomFired, m_leftCentreFired, m_leftTopFired;
};

#endif // __WATCH_H__
