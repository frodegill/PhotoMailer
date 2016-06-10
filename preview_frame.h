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

	void OnClose(wxCloseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);

public:
	void ShowPhoto(const wxString& filename);

private:
	bool LoadPhoto(const wxSize& size);
	bool  GetOrientation(unsigned char& orientation) const;

private:
	wxString  m_selected_photo_filename;
	wxImage   m_selected_photo_image;
	wxBitmap* m_selected_photo_bitmap;

private:
	DECLARE_EVENT_TABLE()
};

}

#endif // _PHOTOMAILER_PREVIEW_FRAME_H_
