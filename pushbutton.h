#ifndef _PHOTOMAILER_PUSHBUTTON_H_
#define _PHOTOMAILER_PUSHBUTTON_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "pushbutton.h"
#endif

#include <wx/dc.h>
#include <wx/grid.h>


#define PUSHBUTTON_WIDTH  (128)
#define PUSHBUTTON_HEIGHT (64)


namespace PhotoMailer
{

class PushButtonRenderer : public wxGridCellRenderer
{
public:
	PushButtonRenderer();
	virtual ~PushButtonRenderer();
	
	virtual void Draw(wxGrid& grid,
	                  wxGridCellAttr& attr,
	                  wxDC& dc,
	                  const wxRect& rect,
	                  int row, int col,
	                  bool isSelected) wxOVERRIDE;

	virtual int GetBestWidth() const {return PUSHBUTTON_WIDTH+1;}
	virtual int GetBestHeight() const {return PUSHBUTTON_HEIGHT+1;}
	virtual wxSize GetBestSize(wxGrid& WXUNUSED(grid), wxGridCellAttr& WXUNUSED(attr), wxDC& WXUNUSED(dc),
	                           int WXUNUSED(row), int WXUNUSED(col)) wxOVERRIDE {return wxSize(GetBestWidth(), GetBestHeight());}

	virtual wxGridCellRenderer* Clone() const;
};

}

#endif // _PHOTOMAILER_PUSHBUTTON_H_
