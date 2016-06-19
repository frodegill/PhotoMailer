#ifndef _PHOTOMAILER_SENDBUTTON_H_
#define _PHOTOMAILER_SENDBUTTON_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "sendbutton.h"
#endif

#include <wx/dc.h>
#include <wx/grid.h>


#define SENDBUTTON_WIDTH  (128)
#define SENDBUTTON_HEIGHT (64)


namespace PhotoMailer
{

class SendButtonClientData : public wxClientData
{
public:
	SendButtonClientData();
	virtual ~SendButtonClientData();

	void SetProgress(float progress) {m_progress=progress;}
	float GetProgress() const {return m_progress;}
	
	void SetIsPressed(bool is_pressed) {m_is_pressed=is_pressed;}
	bool GetIsPressed() const {return m_is_pressed;}

private:
	float m_progress;
	bool  m_is_pressed;
};


class SendButtonRenderer : public wxGridCellRenderer
{
public:
	SendButtonRenderer();
	virtual ~SendButtonRenderer();
	
	virtual void Draw(wxGrid& grid,
	                  wxGridCellAttr& attr,
	                  wxDC& dc,
	                  const wxRect& rect,
	                  int row, int col,
	                  bool isSelected) wxOVERRIDE;

	virtual int GetBestWidth() const {return SENDBUTTON_WIDTH+1;}
	virtual int GetBestHeight() const {return SENDBUTTON_HEIGHT+1;}
	virtual wxSize GetBestSize(wxGrid& WXUNUSED(grid), wxGridCellAttr& WXUNUSED(attr), wxDC& WXUNUSED(dc),
	                           int WXUNUSED(row), int WXUNUSED(col)) wxOVERRIDE {return wxSize(GetBestWidth(), GetBestHeight());}

	virtual wxGridCellRenderer* Clone() const;
};

}

#endif // _PHOTOMAILER_SENDBUTTON_H_
