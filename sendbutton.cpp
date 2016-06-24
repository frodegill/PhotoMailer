// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "sendbutton.h"
#endif

#include <wx/button.h>
#include <wx/renderer.h>
#include <wx/settings.h>

#include "sendbutton.h"

#include "frame.h"


#define PADDING_WIDTH (4)
#define PADDING_HEIGHT (4)


using namespace PhotoMailer;


SendButtonClientData::SendButtonClientData()
: wxClientData(),
  m_progress(0.0),
  m_has_failed(false),
  m_is_pressed(false)
{
}

SendButtonClientData::~SendButtonClientData()
{
}


SendButtonRenderer::SendButtonRenderer()
: wxGridCellRenderer()
{
}

SendButtonRenderer::~SendButtonRenderer()
{
}

void SendButtonRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect,
                             int row, int col, bool isSelected)
{
	wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

#ifndef CLIENTDATA_FIX
	wxGridCellAttr* attr2 = grid.GetOrCreateCellAttr(row, col);
	SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr2?attr2->GetClientObject():nullptr);
#else
	SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr.GetClientObject());
#endif

	if (sendbutton_clientdata)
	{
		float progress = sendbutton_clientdata->GetProgress();
		if (0.0<progress)
		{
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(sendbutton_clientdata->HasFailed() ? *wxRED_BRUSH : *wxGREEN_BRUSH);
			dc.DrawRectangle(wxRect(rect.GetLeft(), rect.GetTop(), rect.GetWidth()*progress, rect.GetHeight()));
		}

		if (0.0>=progress || 1.0<=progress)
		{
			wxString email = grid.GetCellValue(row, EMAIL_COLUMN);
			if (!email.IsEmpty())
			{
				int button_width = rect.GetWidth()-(2*PADDING_WIDTH);
				int button_height = rect.GetHeight()-(2*PADDING_HEIGHT);
				int default_button_height = wxButton::GetDefaultSize().GetHeight();
				if (default_button_height<button_height)
				{
					button_height = default_button_height;
				}

				if ((2*PADDING_WIDTH)<button_width && (2*PADDING_HEIGHT)<button_height)
				{
					wxRect button_rect = wxRect(rect.GetX()+PADDING_WIDTH, rect.GetY()+(rect.GetHeight()-button_height)/2, button_width, button_height);
					wxRendererNative::Get().DrawPushButton(grid.GetGridWindow(), dc, button_rect,
																								sendbutton_clientdata->GetIsPressed()?wxCONTROL_PRESSED:wxCONTROL_NONE);

					dc.SetTextForeground(grid.GetLabelTextColour());
					dc.SetBackgroundMode(wxPENSTYLE_TRANSPARENT);
					dc.DrawLabel(_("Send"), button_rect, wxALIGN_CENTER);
				}
			}
		}
	}
#ifndef CLIENTDATA_FIX
	attr2->DecRef();
#endif
}

wxGridCellRenderer* SendButtonRenderer::Clone() const
{
	return new SendButtonRenderer;
}
