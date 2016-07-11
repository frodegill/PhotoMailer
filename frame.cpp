// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "frame.h"
#endif

#include <wx/confbase.h>
#include <libexif/exif-data.h>

#include "frame.h"

#include "app.h"
#include "mail.h"
#include "thumbnail.h"
#include "sendbutton.h"

using namespace PhotoMailer;

// the application icon
#include "photomailer.xpm"


BEGIN_EVENT_TABLE(PhotoMailerFrame, wxFrame)
	EVT_CLOSE(PhotoMailerFrame::OnClose)
	EVT_MENU(wxID_EXIT,	PhotoMailerFrame::OnQuit)
  EVT_BUTTON(ID_LISTEN, PhotoMailerFrame::OnListen)
	EVT_FSWATCHER(wxID_ANY, PhotoMailerFrame::OnDirectoryEvent)
	EVT_GRID_SELECT_CELL(PhotoMailerFrame::OnGridSelectCell)
	EVT_GRID_CELL_LEFT_CLICK(PhotoMailerFrame::OnGridCellLeftClick)
	EVT_THREAD(MAIL_PROGRESS_EVENT, PhotoMailerFrame::OnMailProgress)
	EVT_THREAD(THUMBNAIL_EVENT, PhotoMailerFrame::OnThumbnailEvent)
END_EVENT_TABLE()

wxIMPLEMENT_CLASS(PhotoMailerFrame, wxFrame);

PhotoMailerFrame::PhotoMailerFrame(const wxString& title)
: PhotoMailerFrameGenerated(nullptr),
  m_filesystem_watcher(nullptr),
  m_is_shutting_down(false),
  m_photolist_thread_semaphore(nullptr),
  m_pending_thumbnail_count(0),
  m_selected_row(-1),
  m_pressed_send_button_row(-1)
{
	int cpu_count = wxThread::GetCPUCount();
	m_photolist_thread_semaphore = new wxSemaphore(cpu_count, cpu_count);

	GetPhotosGrid()->GetGridWindow()->Connect(wxEVT_LEFT_UP,
	                                          static_cast<wxObjectEventFunction>(&PhotoMailerFrame::OnGridMouseUp),
	                                          NULL, this);

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
	delete m_photolist_thread_semaphore;

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

void PhotoMailerFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	DestroyThreads();
	WaitForDestroyedThreads();

	PreviewFrame* preview_frame = ::wxGetApp().GetPreviewFrame();
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
	event.Skip();

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
		wxString filename;
		::wxGetApp().GetPreviewFrame()->ShowPhoto();
	}
}

void PhotoMailerFrame::OnGridCellLeftClick(wxGridEvent& event)
{
	event.Skip();

	wxGrid* grid = GetPhotosGrid();
	wxGridCellAttr* attr;
	if (-1 != m_pressed_send_button_row)
	{
		attr = grid->GetOrCreateCellAttr(m_pressed_send_button_row, ACTION_COLUMN);
		SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr?attr->GetClientObject():nullptr);
		if (sendbutton_clientdata)
		{
			sendbutton_clientdata->SetIsPressed(false);
		}
		attr->DecRef();
	}

	if (ACTION_COLUMN == event.GetCol())
	{
		m_pressed_send_button_row = event.GetRow();
		attr = grid->GetOrCreateCellAttr(m_pressed_send_button_row, ACTION_COLUMN);
		SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr?attr->GetClientObject():nullptr);
		if (sendbutton_clientdata)
		{
			sendbutton_clientdata->SetIsPressed(true);
			grid->ForceRefresh();
		}
		attr->DecRef();

		SendMail(m_pressed_send_button_row);
	}
}

void PhotoMailerFrame::OnGridMouseUp(wxEvent& WXUNUSED(event))
{
	if (-1 != m_pressed_send_button_row)
	{
		wxGrid* grid = GetPhotosGrid();
		wxGridCellAttr* attr = grid->GetOrCreateCellAttr(m_pressed_send_button_row, ACTION_COLUMN);
		SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr?attr->GetClientObject():nullptr);
		if (sendbutton_clientdata && sendbutton_clientdata->GetIsPressed())
		{
			sendbutton_clientdata->SetIsPressed(false);
			grid->ForceRefresh();
		}
		attr->DecRef();
	}
}

void PhotoMailerFrame::OnMailProgress(wxThreadEvent& event)
{
	MailProgressEventPayload* payload = event.GetPayload<MailProgressEventPayload*>();
	if (payload)
	{
		wxGrid* grid = GetPhotosGrid();
		wxGridCellAttr* attr = grid->GetOrCreateCellAttr(payload->GetRow(), ACTION_COLUMN);
		SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr?attr->GetClientObject():nullptr);
		if (sendbutton_clientdata && !sendbutton_clientdata->HasFailed())
		{
			sendbutton_clientdata->SetHasFailed(payload->HasFailed());
			sendbutton_clientdata->SetProgress(payload->GetProgress());
		}
		delete payload;
	}
}

void PhotoMailerFrame::OnThumbnailEvent(wxThreadEvent& event)
{
	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();
	ThumbnailEventPayload* payload = event.GetPayload<ThumbnailEventPayload*>();
	if (payload)
	{
		int row = payload->GetRow();
		if (0<=row && row<grid->GetNumberRows())
		{
			wxGridCellAttr* thumbnail_attr = new wxGridCellAttr;
			if (thumbnail_attr)
			{
				ThumbnailClientData* thumbnail_clientdata = new ThumbnailClientData;
				if (!thumbnail_clientdata)
				{
					thumbnail_attr->DecRef();
				}
				else
				{
					const wxBitmap* bitmap = payload->GetBitmap();
					thumbnail_clientdata->SetThumbnail(*bitmap);
					thumbnail_attr->SetClientObject(thumbnail_clientdata);
					grid->SetAttr(row, THUMBNAIL_COLUMN, thumbnail_attr);
				}
			}
			
			grid->SetCellValue(row, TIMESTAMP_COLUMN, payload->GetTimestamp().FormatISOCombined(' '));

			wxGridCellAttr* sendbutton_attr = new wxGridCellAttr;
			if (sendbutton_attr)
			{
				SendButtonClientData* sendbutton_clientdata = new SendButtonClientData;
				sendbutton_attr->SetClientObject(sendbutton_clientdata);
				grid->SetAttr(row, ACTION_COLUMN, sendbutton_attr);
			}
		}
		delete payload;
	}

	if (0<m_pending_thumbnail_count)
		m_pending_thumbnail_count--;

	if (0==m_pending_thumbnail_count)
	{
		grid->AutoSizeColumn(FILENAME_COLUMN);
		grid->AutoSizeColumn(TIMESTAMP_COLUMN);
	}
}

void PhotoMailerFrame::RegisterThread(wxThread* thread)
{
	wxMutexLocker lock(m_threadlist_mutex);

	m_threadlist.insert(thread);
	if (m_is_shutting_down)
	{
		thread->Delete();
	}
}

void PhotoMailerFrame::UnregisterThread(wxThread* thread)
{
	wxMutexLocker lock(m_threadlist_mutex);

	m_threadlist.erase(thread);
}

void PhotoMailerFrame::DestroyThreads()
{
	wxMutexLocker lock(m_threadlist_mutex);
	
	m_is_shutting_down = true;
	for(std::unordered_set<wxThread*>::iterator it = m_threadlist.begin(); it!=m_threadlist.end(); ++it) {
		(*it)->Delete();
	}
}

void PhotoMailerFrame::WaitForDestroyedThreads()
{
	int old_count = -1;
	int thread_count;
	int waits = 0;
	do {
		{
			wxMutexLocker lock(m_threadlist_mutex);
			thread_count = m_threadlist.size();
		}

		if (old_count == thread_count)
		{
			waits++;
		}
		else
		{
			old_count = thread_count;
			waits = 0;
		}

		if (0 < thread_count)
		{
			::wxSleep(1);
		}
	} while (10>waits && 0<thread_count);
}

bool PhotoMailerFrame::IsValidSettings() const
{
	//TODO
	return true;
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
		grid->SetDefaultCellOverflow(false);

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

	int number_of_rows = grid->GetNumberRows();
	if (0 < number_of_rows)
	{
		grid->DeleteRows(0, number_of_rows);
	}

	m_pending_thumbnail_count = 0;

	wxDir listen_dir(GetDirectoryPicker()->GetPath());
	if (listen_dir.IsOpened())
	{
		listen_dir.Traverse(*this);
	}
	
	grid->AutoSizeColumn(FILENAME_COLUMN);
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

	ThumbnailThread* thread = new ThumbnailThread(m_photolist_thread_semaphore, current_row, filename);
	if (thread)
	{
		m_pending_thumbnail_count++;
		thread->Run();
	}

	return true;
}

void PhotoMailerFrame::SendMail(int row)
{
	wxGrid* grid = GetPhotosGrid();
	if (0>row || grid->GetNumberRows()<=row)
		return;

	wxGridCellAttr* attr = grid->GetOrCreateCellAttr(row, ACTION_COLUMN);
	SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr?attr->GetClientObject():nullptr);
	if (!sendbutton_clientdata)
		return;

	wxString filename;
	GetRowFilename(row, filename);
	
	MailThread* thread = new MailThread(row, GetSmtpServerCtrl()->GetValue(), GetSmtpPortCtrl()->GetValue(),
		GetSmtpUsernameCtrl()->GetValue(), GetSmtpPasswordCtrl()->GetValue(),
		GetSenderCtrl()->GetValue(), grid->GetCellValue(row, EMAIL_COLUMN),
		GetSubjectCtrl()->GetValue(), filename);

	if (!thread)
	{
		sendbutton_clientdata->SetHasFailed(true);
		return;
	}

	sendbutton_clientdata->SetHasFailed(false);
	sendbutton_clientdata->SetProgress(0.0);
	thread->Run();
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

bool PhotoMailerFrame::IsJpeg(const wxString& filename)
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

const wxImage* PhotoMailerFrame::GetSelectedPhoto()
{
	if (IsBeingDeleted())
		return nullptr;

	if (!m_selected_photo_image.IsOk())
	{
		wxMutexLocker lock(m_photolist_mutex);

		wxString filename;
		if (!GetRowFilename(m_selected_row, filename))
			return nullptr;

		LoadImage(filename, m_selected_photo_image);
	}
	return &m_selected_photo_image;
}

bool PhotoMailerFrame::GetRowFilename(int row, wxString& filename)
{
	wxGrid* grid = GetPhotosGrid();
	if (!grid)
		return false;

	if (0>row || grid->GetNumberRows()<=row)
		return false;

	wxString cell_value = grid->GetCellValue(row, FILENAME_COLUMN);
	if (cell_value.IsEmpty())
		return false;

	filename = GetDirectoryPicker()->GetPath() + "/" + cell_value;
	return true;
}
