#ifndef _PHOTOMAILER_PREVIEW_FRAME_H_
#define _PHOTOMAILER_PREVIEW_FRAME_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "preview_frame.h"
#endif

#include <wx/minifram.h>


namespace PhotoMailer
{

class PreviewFrame : public wxMiniFrame
{
DECLARE_DYNAMIC_CLASS(PreviewFrame)
public:
	PreviewFrame(wxWindow* parent, wxWindowID id, const wxString& title,
	             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
	             long style = wxCAPTION | wxRESIZE_BORDER);
	virtual ~PreviewFrame();

private:

private:
	DECLARE_EVENT_TABLE()
};

}

#endif // _PHOTOMAILER_PREVIEW_FRAME_H_
