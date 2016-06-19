// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "frame.h"
#endif

#include <wx/confbase.h>
#include <libexif/exif-data.h>

#include "frame.h"

#include "app.h"
#include "thumbnail.h"
#include "sendbutton.h"

using namespace PhotoMailer;

// the application icon
#include "photomailer.xpm"


BEGIN_EVENT_TABLE(PhotoMailerFrame, wxFrame)
	EVT_IDLE(PhotoMailerFrame::OnIdle)
	EVT_CLOSE(PhotoMailerFrame::OnClose)
	EVT_MENU(wxID_EXIT,	PhotoMailerFrame::OnQuit)
  EVT_BUTTON(ID_LISTEN, PhotoMailerFrame::OnListen)
	EVT_FSWATCHER(wxID_ANY, PhotoMailerFrame::OnDirectoryEvent)
	EVT_GRID_SELECT_CELL(PhotoMailerFrame::OnGridSelectCell)
END_EVENT_TABLE()

wxIMPLEMENT_CLASS(PhotoMailerFrame, wxFrame);

PhotoMailerFrame::PhotoMailerFrame(const wxString& title)
: PhotoMailerFrameGenerated(nullptr),
  m_filesystem_watcher(nullptr),
  m_is_batch_updating(false),
  m_processed_grid_row(-1),
  m_selected_row(-1)
{
	SetIcon(wxIcon(photomailer_xpm));
	SetTitle(title);

	wxConfigBase* config = wxConfigBase::Get();
	if (config)
	{
		wxString str;
		if (config->Read("SMTP Server", &str))
			GetSmtpServerCtrl()->SetValue(str);

		if (config->Read("SMTP Port", &str))
			GetSmtpPortCtrl()->SetValue(str);

		if (config->Read("SMTP Username", &str))
			GetSmtpUsernameCtrl()->SetValue(str);

		GetSmtpPasswordCtrl()->Clear();

		if (config->Read("SMTP Sender", &str))
			GetSenderCtrl()->SetValue(str);

		if (config->Read("SMTP Subject", &str))
			GetSubjectCtrl()->SetValue(str);

		if (config->Read("Directory", &str))
			GetDirectoryPicker()->SetPath(str);
	}

	InitPhotoList();
}

PhotoMailerFrame::~PhotoMailerFrame()
{
	delete m_filesystem_watcher;

	wxConfigBase* config = wxConfigBase::Get();
	if (config)
	{
		config->Write("SMTP Server", GetSmtpServerCtrl()->GetValue());
		config->Write("SMTP Port", GetSmtpPortCtrl()->GetValue());
		config->Write("SMTP Username", GetSmtpUsernameCtrl()->GetValue());
		config->Write("SMTP Sender", GetSenderCtrl()->GetValue());
		config->Write("SMTP Subject", GetSubjectCtrl()->GetValue());
		config->Write("Directory", GetDirectoryPicker()->GetPath());
		config->Flush();
	}
}

wxDirTraverseResult PhotoMailerFrame::OnFile(const wxString& filename)
{
	AddGridItem(filename);
	return wxDIR_CONTINUE;
}

void PhotoMailerFrame::OnIdle(wxIdleEvent& WXUNUSED(event))
{
	wxGrid* grid = GetPhotosGrid();
	if (m_filesystem_watcher && (m_processed_grid_row+1) < grid->GetNumberRows())
		ProcessGridRow();
}

void PhotoMailerFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	PreviewFrame* preview_frame = wxGetApp().GetPreviewFrame();
	if (preview_frame)
	{
		preview_frame->Destroy();
	}

	Destroy();
}

void PhotoMailerFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void PhotoMailerFrame::OnListen(wxCommandEvent& WXUNUSED(event))
{
	if (m_filesystem_watcher)
	{
		delete m_filesystem_watcher;
		m_filesystem_watcher = nullptr;
		GetDirectoryListenButton()->SetLabel(_("Listen"));
	}
	else
	{
		if (!IsValidSettings())
		{
			return;
		}
		m_filesystem_watcher = new wxFileSystemWatcher();
		m_filesystem_watcher->SetOwner(this);
		m_filesystem_watcher->AddTree(GetDirectoryPicker()->GetDirName(), wxFSW_EVENT_CREATE|wxFSW_EVENT_DELETE|wxFSW_EVENT_RENAME);
		RefreshPhotoList();
		GetDirectoryListenButton()->SetLabel(_("Stop"));
	}
}

void PhotoMailerFrame::OnDirectoryEvent(wxFileSystemWatcherEvent& event)
{
	int type = event.GetChangeType();
	if (0!=(type&wxFSW_EVENT_DELETE) || 0!=(type&wxFSW_EVENT_RENAME))
	{
		wxString relative_filename;
		if (GetRelativeFilename(event.GetPath().GetFullPath(), relative_filename))
		{
			wxGrid* grid = GetPhotosGrid();
			int i = grid->GetNumberRows();
			while (--i>=0)
			{
				if (EQUALS == relative_filename.Cmp(grid->GetCellValue(i, FILENAME_COLUMN)))
				{
					grid->DeleteRows(i, FILENAME_COLUMN, false);
					break;
				}
			}
		}
	}
	if (0!=(type&wxFSW_EVENT_CREATE) || 0!=(type&wxFSW_EVENT_RENAME))
	{
		AddGridItem((0!=(type&wxFSW_EVENT_RENAME) ? event.GetNewPath() : event.GetPath()).GetFullPath());
	}
}

void PhotoMailerFrame::OnGridSelectCell(wxGridEvent& event)
{
	wxGrid* grid = GetPhotosGrid();
	if (0 >= grid->GetNumberRows()) //The initial grid->AppendCols will end up calling this (it selects cell(0,0)
		return;

	int event_row = event.GetRow();
	if (m_selected_row == event_row)
		return;

	m_selected_row = event_row;
	m_selected_photo_image.Destroy();

	if (0<=m_selected_row && event_row<grid->GetNumberRows())
	{
		wxString filename = GetDirectoryPicker()->GetPath() + "/" + grid->GetCellValue(m_selected_row, FILENAME_COLUMN);
		wxGetApp().GetPreviewFrame()->ShowPhoto(filename);
	}
}

bool PhotoMailerFrame::IsValidSettings() const
{
	//TODO
	return true;
}

bool PhotoMailerFrame::IsJpeg(const wxString& filename) const
{
	wxFile file(filename);
	if (!file.IsOpened())
		return false;

	static const ssize_t JPEG_HEADER_LENGTH = 11;
	wxUint8 jpegHeader[JPEG_HEADER_LENGTH];
	ssize_t read = file.Read(&jpegHeader[0], JPEG_HEADER_LENGTH);
	return (JPEG_HEADER_LENGTH==read &&
	        0xFF==jpegHeader[0] &&
	        0xD8==jpegHeader[1] &&
	        0xFF==jpegHeader[2] &&
	        (0xE0==jpegHeader[3] || 0xE1==jpegHeader[3]));
}

bool PhotoMailerFrame::GetRelativeFilename(const wxString& absolute, wxString& relative)
{
	wxString listen_dir = GetDirectoryPicker()->GetPath();
	size_t listen_dir_length = listen_dir.Len();
	if (listen_dir_length >= absolute.Len())
		return false;

	wxString rest;
	if (!absolute.StartsWith(listen_dir, &rest))
		return false;

	if (rest.StartsWith(_("./")))
	{
		relative = rest;
	}
	else if (rest.StartsWith(_("/")))
	{
		relative = _(".") + rest;
	}
	else {
		relative = _("./") + rest;
	}
	return true;
}

void PhotoMailerFrame::InitPhotoList()
{
	wxGrid* grid = GetPhotosGrid();
	int number_of_cols = grid->GetNumberCols();
	if (5 >= number_of_cols)
	{
    grid->HideRowLabels();
		grid->DisableDragRowSize();
		grid->AppendCols(5-number_of_cols);
		grid->SetDefaultCellAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);

		wxGridCellAttr* photo_attr = new wxGridCellAttr;
		photo_attr->SetReadOnly();
		ThumbnailRenderer* thumbnail_renderer = new ThumbnailRenderer;
		photo_attr->SetRenderer(thumbnail_renderer);

		wxGridCellAttr* ro_attr = new wxGridCellAttr;
		ro_attr->SetReadOnly();

		wxGridCellAttr* editor_attr = new wxGridCellAttr;
		editor_attr->SetEditor(new wxGridCellTextEditor);

		wxGridCellAttr* action_attr = new wxGridCellAttr;
		action_attr->SetReadOnly();
		SendButtonRenderer* sendbutton_renderer = new SendButtonRenderer;
		action_attr->SetRenderer(sendbutton_renderer);

		grid->SetDefaultRowSize(thumbnail_renderer->GetBestHeight());
		grid->SetColLabelValue(THUMBNAIL_COLUMN, _("Thumbnail"));
		grid->SetColAttr(THUMBNAIL_COLUMN, photo_attr);
		grid->SetColSize(THUMBNAIL_COLUMN, thumbnail_renderer->GetBestWidth());
		grid->SetColLabelValue(FILENAME_COLUMN, _("Filename"));
		grid->SetColAttr(FILENAME_COLUMN, ro_attr);
		grid->SetColLabelValue(TIMESTAMP_COLUMN, _("Time"));
		ro_attr->IncRef(); grid->SetColAttr(TIMESTAMP_COLUMN, ro_attr);
		grid->SetColLabelValue(EMAIL_COLUMN, _("Email"));
		grid->SetColAttr(EMAIL_COLUMN, editor_attr);
		grid->SetColLabelValue(ACTION_COLUMN, _("Action"));
		grid->SetColAttr(ACTION_COLUMN, action_attr);
	}
}

void PhotoMailerFrame::RefreshPhotoList()
{
	wxGrid* grid = GetPhotosGrid();

	wxMutexLocker lock(m_photolist_mutex);

	grid->BeginBatch();
	m_is_batch_updating = true;

	int number_of_rows = grid->GetNumberRows();
	if (0 < number_of_rows)
	{
		grid->DeleteRows(0, number_of_rows);
	}

	wxDir listen_dir(GetDirectoryPicker()->GetPath());
	if (listen_dir.IsOpened())
	{
		listen_dir.Traverse(*this);
	}
	
	grid->AutoSizeColumn(FILENAME_COLUMN);

	grid->EndBatch();
	m_is_batch_updating = false;
}

bool PhotoMailerFrame::AddGridItem(const wxString& filename)
{
	if (!IsJpeg(filename))
		return false;

	wxGrid* grid = GetPhotosGrid();

	wxMutexLocker lock(m_photolist_mutex);

	grid->AppendRows(1);
	int current_row = grid->GetNumberRows()-1;

	wxString cell_filename;
	if (!GetRelativeFilename(filename, cell_filename))
	{
		cell_filename = filename;
	}
	grid->SetCellValue(current_row, FILENAME_COLUMN, cell_filename);

	return true;
}

bool PhotoMailerFrame::ProcessGridRow()
{
	wxGrid* grid = GetPhotosGrid();

	wxMutexLocker lock(m_photolist_mutex);

	if ((m_processed_grid_row+1) >= grid->GetNumberRows())
		return true;

	m_processed_grid_row++;

	wxString filename = GetDirectoryPicker()->GetPath() + "/" + grid->GetCellValue(m_processed_grid_row, FILENAME_COLUMN);

	if (!IsJpeg(filename))
		return false;

	wxImage image;
	wxDateTime exif_timestamp;
	if (LoadImage(filename, image, &exif_timestamp))
	{
		wxGridCellAttr* thumbnail_attr = new wxGridCellAttr;
		if (thumbnail_attr)
		{
			ThumbnailClientData* thumbnail_clientdata = new ThumbnailClientData;
			if (!thumbnail_clientdata || !thumbnail_clientdata->SetThumbnail(image))
			{
				delete thumbnail_clientdata;
			}
			else
			{
				thumbnail_attr->SetClientObject(thumbnail_clientdata);
				grid->SetAttr(m_processed_grid_row, THUMBNAIL_COLUMN, thumbnail_attr);
			}
		}

		grid->SetCellValue(m_processed_grid_row, TIMESTAMP_COLUMN, exif_timestamp.FormatISOCombined(' '));

		wxGridCellAttr* sendbutton_attr = new wxGridCellAttr;
		if (sendbutton_attr)
		{
			SendButtonClientData* sendbutton_clientdata = new SendButtonClientData;
			sendbutton_attr->SetClientObject(sendbutton_clientdata);
			grid->SetAttr(m_processed_grid_row, ACTION_COLUMN, sendbutton_attr);
		}
	}

	if (!m_is_batch_updating)
	{
		grid->AutoSizeColumn(FILENAME_COLUMN);
		grid->AutoSizeColumn(TIMESTAMP_COLUMN);
	}

	return true;
}

bool PhotoMailerFrame::LoadImage(const wxString& filename, wxImage& image, wxDateTime* timestamp)
{
	if (filename.IsEmpty())
		return false;

	if (!image.LoadFile(filename, wxBITMAP_TYPE_JPEG) || !image.IsOk())
	{
		image.Destroy();
		return false;
	}

	unsigned char orientation;
	if (GetExifInfo(filename, &orientation, timestamp))
	{
		switch(orientation)
		{
			case 3: image = image.Rotate180(); break;
			case 6: image = image.Rotate90(true); break;
			case 8: image = image.Rotate90(false); break;
			default: break;
		};
	}
	return true;
}

bool PhotoMailerFrame::GetExifInfo(const wxString& filename, unsigned char* orientation, wxDateTime* timestamp)
{
	if (!orientation && !timestamp) //Noop
		return true;

	ExifData* ed = exif_data_new_from_file(filename.c_str());
	if (!ed)
		return false;

	if (orientation)
	{
		ExifEntry* entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
		*orientation = entry ? entry->data[0] : 0;
	}

	if (timestamp)
	{
		ExifEntry* entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_DATE_TIME);
		if (entry)
		{
			wxString::const_iterator end;
			if (!timestamp->ParseFormat(wxString::FromUTF8(reinterpret_cast<const char*>(entry->data)), _("%Y:%m:%d %H:%M:%S"), &end))
			{
				timestamp->SetToCurrent();
			}
		}
	}

	exif_data_unref(ed);
	return true;
}

const wxImage* PhotoMailerFrame::GetSelectedPhoto()
{
	if (!m_selected_photo_image.IsOk())
	{
		wxGrid* grid = GetPhotosGrid();

		wxMutexLocker lock(m_photolist_mutex);

		wxString filename = GetDirectoryPicker()->GetPath() + "/" + grid->GetCellValue(m_selected_row, FILENAME_COLUMN);
		LoadImage(filename, m_selected_photo_image);
	}
	return &m_selected_photo_image;
}
