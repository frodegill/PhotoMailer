// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "thumbnail.h"
#endif

#include "thumbnail.h"

#include "app.h"
#include "frame.h"


using namespace PhotoMailer;


ThumbnailClientData::ThumbnailClientData()
: wxClientData()
{
}


ThumbnailRenderer::ThumbnailRenderer()
: wxGridCellRenderer()
{
}

void ThumbnailRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect,
                             int row, int col, bool isSelected)
{
	wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

#ifndef CLIENTDATA_FIX
	wxGridCellAttrPtr attr_ptr = grid.GetOrCreateCellAttrPtr(row, col);
	ThumbnailClientData* thumbnail_clientdata = static_cast<ThumbnailClientData*>(attr_ptr.get()?attr_ptr->GetClientObject():nullptr);
#else
	ThumbnailClientData* thumbnail_clientdata = static_cast<ThumbnailClientData*>(attr.GetClientObject());
#endif
	if (thumbnail_clientdata)
	{
		wxBitmap thumbnail_bitmap;
		thumbnail_clientdata->GetThumbnail(thumbnail_bitmap);
		if (thumbnail_bitmap.IsOk())
		{
			dc.DrawBitmap(thumbnail_bitmap,
										rect.GetLeft() + (THUMBNAIL_SIZE-thumbnail_bitmap.GetWidth())/2,
										rect.GetTop() + (THUMBNAIL_SIZE-thumbnail_bitmap.GetHeight())/2);
		}
	}
}

wxGridCellRenderer* ThumbnailRenderer::Clone() const
{
	return new ThumbnailRenderer;
}



ThumbnailThread::ThumbnailThread(wxSemaphore* semaphore, int row, const wxString& filename)
: wxThread(),
  m_semaphore(semaphore),
  m_row(row),
  m_filename(filename)
{
	::wxGetApp().GetMainFrame()->RegisterThread(this);
}

ThumbnailThread::~ThumbnailThread()
{
	::wxGetApp().GetMainFrame()->UnregisterThread(this);
}

wxThread::ExitCode ThumbnailThread::Entry()
{
	m_semaphore->Wait();
	if (TestDestroy())
		return CleanupAndExit(-1);

	wxImage image;
	wxDateTime exif_timestamp;
	wxBitmap bitmap;
	if (m_filename.IsEmpty() ||
	    !PhotoMailerFrame::IsJpeg(m_filename) ||
	    !PhotoMailerFrame::LoadImage(m_filename, image, &exif_timestamp) ||
	    !CreateThumbnailBitmap(image, bitmap))
	{
		return CleanupAndExit(-1);
	}

	if (TestDestroy())
		return CleanupAndExit(-1);

	wxWindow* event_handler = ::wxGetApp().GetMainFrame();
	if (!event_handler || event_handler->IsBeingDeleted())
		return CleanupAndExit(-1);
		
	wxThreadEvent* threadEvent = new wxThreadEvent(wxEVT_THREAD, THUMBNAIL_EVENT);
	ThumbnailEventPayload* payload = new ThumbnailEventPayload(m_row, bitmap, exif_timestamp);
	threadEvent->SetPayload<ThumbnailEventPayload*>(payload);
	::wxQueueEvent(event_handler, threadEvent);

	return CleanupAndExit(0);
}

wxThread::ExitCode ThumbnailThread::CleanupAndExit(int exit_code)
{
	m_semaphore->Post();
	return reinterpret_cast<wxThread::ExitCode>(exit_code);
}

bool ThumbnailThread::CreateThumbnailBitmap(const wxImage& image, wxBitmap& bitmap)
{
	if (!image.IsOk())
		return false;

	int image_width = image.GetWidth();
	int image_height = image.GetHeight();
	if (0>=image_width || 0>=image_height)
		return false;

	float image_ratio = static_cast<float>(image_width)/static_cast<float>(image_height);
	if (1.0>image_ratio) //Portrait
	{
		image_width = static_cast<float>(THUMBNAIL_SIZE)*image_ratio;
		image_height = THUMBNAIL_SIZE;
	}
	else //Landscape
	{
		image_width = THUMBNAIL_SIZE;
		image_height = static_cast<float>(THUMBNAIL_SIZE)/image_ratio;
	}

	bitmap = wxBitmap(image.Scale(image_width, image_height, wxIMAGE_QUALITY_HIGH));
	return bitmap.IsOk();
}


ThumbnailEventPayload::ThumbnailEventPayload(int row, const wxBitmap& bitmap, const wxDateTime& exif_timestamp)
: m_row(row),
  m_exif_timestamp(exif_timestamp)
{
	m_bitmap = bitmap.GetSubBitmap(wxRect(0, 0, bitmap.GetWidth(), bitmap.GetHeight()));
}
