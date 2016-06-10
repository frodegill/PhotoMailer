// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "preview_frame.h"
#endif

#include <wx/dcclient.h>
#include <libexif/exif-data.h>

#include "preview_frame.h"

#include "app.h"

using namespace PhotoMailer;


BEGIN_EVENT_TABLE(PreviewFrame, wxMiniFrame)
	EVT_CLOSE(PreviewFrame::OnClose)
	EVT_SIZE(PreviewFrame::OnSize)
	EVT_PAINT(PreviewFrame::OnPaint)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PreviewFrame, wxMiniFrame)

PreviewFrame::PreviewFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                           const wxPoint& pos, const wxSize& size, long style)
: wxMiniFrame(parent, id, title, pos, size, style),
  m_selected_photo_bitmap(nullptr)
{
	SetSize(-1,-1);
#if wxVERSION_NUMBER >= 2900
	if(!wxPersistenceManager::Get().Find(this)) {
		wxPersistenceManager::Get().RegisterAndRestore(this);
	} else {
		wxPersistenceManager::Get().Restore(this);
	}
#endif
}

PreviewFrame::~PreviewFrame()
{
	delete m_selected_photo_bitmap;
}

void PreviewFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	wxGetApp().OnPreviewFrameClosed();
	Destroy();
}

void PreviewFrame::OnSize(wxSizeEvent& event)
{
	delete m_selected_photo_bitmap; m_selected_photo_bitmap = nullptr;
	Refresh();
	event.Skip(true);
}

void PreviewFrame::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	wxSize dc_size = dc.GetSize();
	if (!m_selected_photo_bitmap)
	{
		if (!LoadPhoto(dc_size))
			return;
	}
	dc.DrawBitmap(*m_selected_photo_bitmap,
	              (dc_size.GetWidth()-m_selected_photo_bitmap->GetWidth())/2,
	              (dc_size.GetHeight()-m_selected_photo_bitmap->GetHeight())/2);
}

void PreviewFrame::ShowPhoto(const wxString& filename)
{
	m_selected_photo_filename = filename;
	delete m_selected_photo_bitmap; m_selected_photo_bitmap = nullptr;
	Refresh();
}

bool PreviewFrame::LoadPhoto(const wxSize& size)
{
	if (!m_selected_photo_image.IsOk())
	{
		if (!m_selected_photo_image.LoadFile(m_selected_photo_filename, wxBITMAP_TYPE_JPEG) ||
		    !m_selected_photo_image.IsOk())
		{
			m_selected_photo_image.Destroy();
			return false;
		}

		unsigned char orientation;
		if (GetOrientation(orientation))
		{
			switch(orientation)
			{
				case 3: m_selected_photo_image = m_selected_photo_image.Rotate180(); break;
				case 6: m_selected_photo_image = m_selected_photo_image.Rotate90(true); break;
				case 8: m_selected_photo_image = m_selected_photo_image.Rotate90(false); break;
				default: break;
			};
		};
	}

	int dc_width = size.GetWidth();
	int dc_height = size.GetHeight();
	int image_width = m_selected_photo_image.GetWidth();
	int image_height = m_selected_photo_image.GetHeight();
	if (0==dc_width || 0==dc_height || 0==image_width || 0==image_height)
		return false;

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

	if (!m_selected_photo_bitmap)
	{
		m_selected_photo_bitmap = new wxBitmap(m_selected_photo_image.Scale(image_width, image_height, wxIMAGE_QUALITY_HIGH));
		if (!m_selected_photo_bitmap->IsOk())
		{
			delete m_selected_photo_bitmap; m_selected_photo_bitmap = nullptr;
			return false;
		}
	}
	return true;
}

bool PreviewFrame::GetOrientation(unsigned char& orientation) const
{
	ExifData* ed = exif_data_new_from_file(m_selected_photo_filename.c_str());
	if (!ed)
		return false;

	ExifEntry* entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
	orientation = entry ? entry->data[0] : 0;

	exif_data_unref(ed);
	return true;
}
