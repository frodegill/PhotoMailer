// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "pushbutton.h"
#endif

#include "pushbutton.h"

#include <wx/button.h>
#include <wx/renderer.h>
#include <wx/settings.h>

#define PADDING_WIDTH (4)
#define PADDING_HEIGHT (4)


using namespace PhotoMailer;


PushButtonRenderer::PushButtonRenderer()
: wxGridCellRenderer()
{
}

PushButtonRenderer::~PushButtonRenderer()
{
}

void PushButtonRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect,
                             int row, int col, bool isSelected)
{
	wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

	int button_width = rect.GetWidth()-(2*PADDING_WIDTH);
	int button_height = rect.GetHeight()-(2*PADDING_HEIGHT);
	int default_button_height = wxButton::GetDefaultSize().GetHeight();
	if (default_button_height<button_height)
		button_height = default_button_height;

	if ((2*PADDING_WIDTH)<button_width && (2*PADDING_HEIGHT)<button_height)
	{
		wxRect button_rect = wxRect(rect.GetX()+PADDING_WIDTH, rect.GetY()+(rect.GetHeight()-button_height)/2, button_width, button_height);
		wxRendererNative::Get().DrawPushButton(grid.GetGridWindow(), dc, button_rect);
		
		dc.SetTextForeground(grid.GetLabelTextColour());
		dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
		dc.DrawLabel(_("Send"), button_rect, wxALIGN_CENTER);
	}
}

wxGridCellRenderer* PushButtonRenderer::Clone() const
{
	return new PushButtonRenderer;
}
