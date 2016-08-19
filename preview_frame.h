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
wxDECLARE_DYNAMIC_CLASS(PreviewFrame);
public:
	PreviewFrame(wxWindow* parent, wxWindowID id, const wxString& title,
	             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
	             long style = wxCAPTION | wxRESIZE_BORDER);

	void OnClose(wxCloseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnPreviewEvent(wxThreadEvent& event);

public:
	void ShowPhoto();

private:
	wxMutex m_preview_mutex;

	wxBitmap m_selected_photo_bitmap;

private:
	DECLARE_EVENT_TABLE()
};


class PreviewThread : public wxThread
{
public:
	PreviewThread(wxEvtHandler* event_handler, const wxSize& size);
	virtual ~PreviewThread();

public: //wxThread
	virtual wxThread::ExitCode Entry() wxOVERRIDE;

private:
	wxThread::ExitCode CleanupAndExit(int exit_code);
	bool CreatePreviewBitmap(const wxImage& image, wxBitmap& bitmap);	

private:
	wxEvtHandler* m_event_handler;
	wxSize m_size;
};

class PreviewEventPayload
{
public:
	PreviewEventPayload(const wxImage& image, const wxString& filename);
	const wxBitmap* GetBitmap() const {return &m_bitmap;}
	void GetFilename(wxString& filename) {filename=m_filename;}
private:
	wxBitmap m_bitmap;
	wxString m_filename;
};

}

#endif // _PHOTOMAILER_PREVIEW_FRAME_H_
