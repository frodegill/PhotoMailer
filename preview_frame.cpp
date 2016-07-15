// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "preview_frame.h"
#endif

#include <wx/dcclient.h>

#include "preview_frame.h"

#include "app.h"

using namespace PhotoMailer;

// the preview window icon
#include "photomailer_preview.xpm"


BEGIN_EVENT_TABLE(PreviewFrame, wxMiniFrame)
	EVT_CLOSE(PreviewFrame::OnClose)
	EVT_SIZE(PreviewFrame::OnSize)
	EVT_PAINT(PreviewFrame::OnPaint)
	EVT_THREAD(PREVIEW_EVENT, PreviewFrame::OnPreviewEvent)
END_EVENT_TABLE()

wxIMPLEMENT_CLASS(PreviewFrame, wxMiniFrame);

PreviewFrame::PreviewFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                           const wxPoint& pos, const wxSize& size, long style)
: wxMiniFrame(parent, id, title, pos, size, style)
{
	SetIcon(wxIcon(photomailer_preview_xpm));
	SetSize(-1,-1);
#if wxVERSION_NUMBER >= 2900
	if(!wxPersistenceManager::Get().Find(this)) {
		wxPersistenceManager::Get().RegisterAndRestore(this);
	} else {
		wxPersistenceManager::Get().Restore(this);
	}
#endif
}

void PreviewFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	::wxGetApp().OnPreviewFrameClosed();
	Destroy();
}

void PreviewFrame::OnSize(wxSizeEvent& event)
{
	ShowPhoto();
	event.Skip(true);
}

void PreviewFrame::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxMutexLocker lock(m_preview_mutex);

	if (m_selected_photo_bitmap.IsOk())
	{
		wxPaintDC dc(this);
		wxSize dc_size = dc.GetSize();
		dc.DrawBitmap(m_selected_photo_bitmap,
									(dc_size.GetWidth()-m_selected_photo_bitmap.GetWidth())/2,
									(dc_size.GetHeight()-m_selected_photo_bitmap.GetHeight())/2);
	}
}

void PreviewFrame::OnPreviewEvent(wxThreadEvent& event)
{
	wxMutexLocker lock(m_preview_mutex);

	PreviewEventPayload* payload = event.GetPayload<PreviewEventPayload*>();
	if (payload)
	{
		const wxBitmap* bitmap = payload->GetBitmap();
		m_selected_photo_bitmap = bitmap->GetSubBitmap(wxRect(0, 0, bitmap->GetWidth(), bitmap->GetHeight()));
		delete payload;
		Refresh();
	}
}

void PreviewFrame::ShowPhoto()
{
	PreviewThread* thread = new PreviewThread(this, GetClientSize());
	if (thread)
	{
		thread->Run();
	}
}


PreviewThread::PreviewThread(wxEvtHandler* event_handler, const wxSize& size)
: wxThread(),
  m_event_handler(event_handler),
  m_size(size)
{
	::wxGetApp().GetMainFrame()->RegisterThread(this);
}

PreviewThread::~PreviewThread()
{
	::wxGetApp().GetMainFrame()->UnregisterThread(this);
}

wxThread::ExitCode PreviewThread::Entry()
{
	if (TestDestroy())
		return CleanupAndExit(-1);

	wxImage image;
	if (!::wxGetApp().GetMainFrame()->GetSelectedPhoto(image) || !image.IsOk())
		return CleanupAndExit(-1);

	int dc_width = m_size.GetWidth();
	int dc_height = m_size.GetHeight();
	int image_width = image.GetWidth();
	int image_height = image.GetHeight();
	if (0==dc_width || 0==dc_height || 0==image_width || 0==image_height)
		return CleanupAndExit(-1);

	float dc_ratio = static_cast<float>(dc_width)/static_cast<float>(dc_height);
	float image_ratio = static_cast<float>(image_width)/static_cast<float>(image_height);
	if (dc_ratio>image_ratio)
	{
		image_width = static_cast<float>(dc_height)*image_ratio;
		image_height = dc_height;
	}
	else
	{
		image_width = dc_width;
		image_height = static_cast<float>(dc_width)/image_ratio;
	}

	wxThreadEvent* threadEvent = new wxThreadEvent(wxEVT_THREAD, PREVIEW_EVENT);
	PreviewEventPayload* payload = new PreviewEventPayload(image.Scale(image_width, image_height, wxIMAGE_QUALITY_NORMAL));
	threadEvent->SetPayload<PreviewEventPayload*>(payload);
	::wxQueueEvent(m_event_handler, threadEvent);

	return CleanupAndExit(0);
}

wxThread::ExitCode PreviewThread::CleanupAndExit(int exit_code)
{
	return reinterpret_cast<wxThread::ExitCode>(exit_code);
}


PreviewEventPayload::PreviewEventPayload(const wxImage& image)
{
	m_bitmap = wxBitmap(image);
}
