// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "frame.h"
#endif

#include <wx/confbase.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <libexif/exif-data.h>

#include "frame.h"

#include "app.h"
#include "mail.h"
#include "thumbnail.h"
#include "sendbutton.h"


void OnClientEventCallback(int event, CFtpServer::CClientEntry* /*client*/, void* arg)
{
	if (CFtpServer::CLIENT_UPLOADED != event)
		return;
	
	wxWindow* event_handler = ::wxGetApp().GetMainFrame();
	if (!event_handler || event_handler->IsBeingDeleted())
		return;
		
	wxThreadEvent* threadEvent = new wxThreadEvent(wxEVT_THREAD, FTP_UPLOAD_EVENT);
	PhotoMailer::FtpUploadEventPayload* payload = new PhotoMailer::FtpUploadEventPayload(static_cast<char*>(arg));
	threadEvent->SetPayload<PhotoMailer::FtpUploadEventPayload*>(payload);
	::wxQueueEvent(event_handler, threadEvent);
}

using namespace PhotoMailer;

// the application icon
#include "photomailer.xpm"


BEGIN_EVENT_TABLE(PhotoMailerFrame, wxFrame)
	EVT_CLOSE(PhotoMailerFrame::OnClose)
	EVT_MENU(wxID_EXIT,	PhotoMailerFrame::OnQuit)
  EVT_BUTTON(ID_FTPSERVER_BUTTON, PhotoMailerFrame::OnFtpServerButton)
	EVT_GRID_SELECT_CELL(PhotoMailerFrame::OnGridSelectCell)
	EVT_GRID_CELL_LEFT_CLICK(PhotoMailerFrame::OnGridCellLeftClick)
	EVT_GRID_CELL_CHANGED(PhotoMailerFrame::OnGridCellChanged)
	EVT_THREAD(MAIL_PROGRESS_EVENT, PhotoMailerFrame::OnMailProgress)
	EVT_THREAD(THUMBNAIL_EVENT, PhotoMailerFrame::OnThumbnailEvent)
	EVT_THREAD(FTP_UPLOAD_EVENT, PhotoMailerFrame::OnFtpUploadEvent)
END_EVENT_TABLE()

wxIMPLEMENT_CLASS(PhotoMailerFrame, wxFrame);

PhotoMailerFrame::PhotoMailerFrame(const wxString& title)
: PhotoMailerFrameGenerated(nullptr),
  m_ftp_server(nullptr),
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

		if (config->Read("FTP Password", &str))
			GetFtpPasswordCtrl()->SetValue(str);

		if (config->Read("FTP Port", &str))
			GetFtpPortCtrl()->SetValue(str);

		if (config->Read("FTP DataPort From", &str))
			GetDataportFromCtrl()->SetValue(str);

		if (config->Read("FTP DataPort To", &str))
			GetDataportToCtrl()->SetValue(str);

		if (config->Read("Directory", &str))
			GetDirectoryPicker()->SetPath(str);
	}

	InitPhotoList();
}

PhotoMailerFrame::~PhotoMailerFrame()
{
	delete m_ftp_server;
	delete m_photolist_thread_semaphore;

	wxConfigBase* config = wxConfigBase::Get();
	if (config)
	{
		config->Write("SMTP Server", GetSmtpServerCtrl()->GetValue());
		config->Write("SMTP Port", GetSmtpPortCtrl()->GetValue());
		config->Write("SMTP Username", GetSmtpUsernameCtrl()->GetValue());
		config->Write("SMTP Sender", GetSenderCtrl()->GetValue());
		config->Write("SMTP Subject", GetSubjectCtrl()->GetValue());
		config->Write("FTP Password", GetFtpPasswordCtrl()->GetValue());
		config->Write("FTP Port", GetFtpPortCtrl()->GetValue());
		config->Write("FTP DataPort From", GetDataportFromCtrl()->GetValue());
		config->Write("FTP DataPort To", GetDataportToCtrl()->GetValue());
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
	
	StopFtpServer();

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

void PhotoMailerFrame::OnFtpServerButton(wxCommandEvent& WXUNUSED(event))
{
	if (m_ftp_server)
	{
		StopFtpServer();
		GetFtpStartButton()->SetLabel(_("Start FTP Server"));
	}
	else
	{
		if (!IsValidSettings())
		{
			return;
		}

		StartFtpServer();

		RefreshPhotoList();
		GetFtpStartButton()->SetLabel(_("Stop FTP Server"));
	}
}

void PhotoMailerFrame::OnGridSelectCell(wxGridEvent& event)
{
	event.Skip();

	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();
	if (0 >= grid->GetNumberRows()) //The initial grid->AppendCols will end up calling this (it selects cell(0,0)
		return;

	int event_row = event.GetRow();
	if (m_selected_row == event_row)
		return;

	SelectPhoto(event_row);
}

void PhotoMailerFrame::OnGridCellLeftClick(wxGridEvent& event)
{
	event.Skip();

	wxMutexLocker lock(m_photolist_mutex);

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
		wxMutexLocker lock(m_photolist_mutex);

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

void PhotoMailerFrame::OnGridCellChanged(wxGridEvent& event)
{
	event.Skip();

	if (EMAIL_COLUMN != event.GetCol())
		return;

	int row = event.GetRow();
	wxString filename;
	if (!GetRowFilename(row, filename) || filename.IsEmpty())
		return;

	wxConfigBase* config = wxConfigBase::Get();
	if (!config)
		return;

	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();
	wxString email = grid->GetCellValue(row, EMAIL_COLUMN);
	config->Write(filename, email);
	config->Flush();
}

void PhotoMailerFrame::OnMailProgress(wxThreadEvent& event)
{
	MailProgressEventPayload* payload = event.GetPayload<MailProgressEventPayload*>();
	if (payload)
	{
		wxMutexLocker lock(m_photolist_mutex);

		wxGrid* grid = GetPhotosGrid();
		wxGridCellAttr* attr = grid->GetOrCreateCellAttr(payload->GetRow(), ACTION_COLUMN);
		SendButtonClientData* sendbutton_clientdata = static_cast<SendButtonClientData*>(attr?attr->GetClientObject():nullptr);
		if (sendbutton_clientdata && !sendbutton_clientdata->HasFailed())
		{
			sendbutton_clientdata->SetHasFailed(payload->HasFailed());
			sendbutton_clientdata->SetProgress(payload->GetProgress());
		}
		delete payload;

		grid->Refresh();
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
		grid->AutoSizeColumn(EMAIL_COLUMN);
	}
}

void PhotoMailerFrame::OnFtpUploadEvent(wxThreadEvent& event)
{
	FtpUploadEventPayload* payload = event.GetPayload<FtpUploadEventPayload*>();
	if (payload)
	{
		wxString path;
		payload->GetPath(path);
		delete payload;
		AddGridItem(path);
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

bool PhotoMailerFrame::IsValidSettings()
{
	if (GetSmtpServerCtrl()->GetValue().IsEmpty() ||
	    GetSenderCtrl()->GetValue().IsEmpty() ||
	    GetDirectoryPicker()->GetPath().IsEmpty())
	{
		wxMessageOutputDebug logger;
		logger.Output(_("Missing required info"));
		return false;
	}

	return true;
}

bool PhotoMailerFrame::GetRelativeFilename(const wxString& absolute, wxString& relative)
{
	wxFileName absolute_filename(absolute);
	if (!absolute_filename.MakeRelativeTo(GetDirectoryPicker()->GetPath()))
		return false;

	relative = absolute_filename.GetFullPath();
	return true;
}

void PhotoMailerFrame::InitPhotoList()
{
	wxMutexLocker lock(m_photolist_mutex);

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
	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();
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

	wxString email;
	wxConfigBase* config = wxConfigBase::Get();
	if (config)
	{
		email = config->Read(filename);
	}
	
	int current_row = -1;
	{
		wxMutexLocker lock(m_photolist_mutex);

		wxGrid* grid = GetPhotosGrid();
		if (!grid->AppendRows(1))
			return false;

		current_row = grid->GetNumberRows()-1;

		wxString cell_filename;
		if (!GetRelativeFilename(filename, cell_filename))
		{
			cell_filename = filename;
		}
		grid->SetCellValue(current_row, FILENAME_COLUMN, cell_filename);
		
		if (!email.IsEmpty())
		{
			grid->SetCellValue(current_row, EMAIL_COLUMN, email);
		}
	}

	{
		wxMutexLocker lock(m_threadlist_mutex);

		if (m_threadlist.empty())
		{
			SelectPhoto(current_row);
		}
	}

	ThumbnailThread* thread = new ThumbnailThread(m_photolist_thread_semaphore, current_row, filename);
	if (thread)
	{
		m_pending_thumbnail_count++;
		thread->Run();
	}

	return true;
}

void PhotoMailerFrame::SelectPhoto(int row)
{
	int grid_rows = -1;
	{
		wxMutexLocker lock(m_photolist_mutex);

		wxGrid* grid = GetPhotosGrid();
		grid_rows = grid->GetNumberRows();
		if (0>row || row>=grid_rows)
			return;

		grid->SelectRow(row);
		m_selected_row = row;
		
		wxMutexLocker photo_lock(m_selected_photo_mutex);
		m_selected_photo_image.Destroy();
		m_selected_photo_filename = wxEmptyString;
	}

	if (0<=m_selected_row && m_selected_row<grid_rows)
	{
		::wxGetApp().GetPreviewFrame()->ShowPhoto();
	}
}

void PhotoMailerFrame::SendMail(int row)
{
	wxMutexLocker lock(m_photolist_mutex);

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

	unsigned char orientation;
	if (!image.LoadFile(filename, wxBITMAP_TYPE_JPEG) ||
      !image.IsOk() ||
      !GetExifInfo(filename, &orientation, timestamp))
	{
		image.Destroy();
		return false;
	}

  switch(orientation)
  {
    case 3: image = image.Rotate180(); break;
    case 6: image = image.Rotate90(true); break;
    case 8: image = image.Rotate90(false); break;
    default: break;
  };
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
    wxString::const_iterator end;
		if (!entry ||
        !timestamp->ParseFormat(wxString::FromUTF8(reinterpret_cast<const char*>(entry->data)), _("%Y:%m:%d %H:%M:%S"), &end))
    {
      timestamp->SetToCurrent();
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
	if (JPEG_HEADER_LENGTH > read)
	{
		::wxSleep(1);
		ssize_t read2 = file.Read(&jpegHeader[read], JPEG_HEADER_LENGTH-read);
		read += read2;
	}
	return (JPEG_HEADER_LENGTH==read &&
	        0xFF==jpegHeader[0] &&
	        0xD8==jpegHeader[1] &&
	        0xFF==jpegHeader[2] &&
	        (0xE0==jpegHeader[3] || 0xE1==jpegHeader[3]));
}

bool PhotoMailerFrame::GetSelectedPhoto(wxImage& image, wxString& filename)
{
	if (IsBeingDeleted())
		return false;

	wxMutexLocker photo_lock(m_selected_photo_mutex);

	if (!m_selected_photo_image.IsOk())
	{
		wxMutexLocker lock(m_photolist_mutex);

		wxString absolute_filename;
		if (!GetRowFilename(m_selected_row, absolute_filename) ||
		    !GetRelativeFilename(absolute_filename, m_selected_photo_filename))
		{
			return false;
		}
		
		LoadImage(absolute_filename, m_selected_photo_image);
	}
	image = m_selected_photo_image.Copy();
	filename = m_selected_photo_filename;
	return true;
}

bool PhotoMailerFrame::GetRowFilename(int row, wxString& filename)
{
	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();
	if (!grid)
		return false;

	if (0>row || grid->GetNumberRows()<=row)
		return false;

	wxString cell_value = grid->GetCellValue(row, FILENAME_COLUMN);
	if (cell_value.IsEmpty())
		return false;

	wxFileName filename_filename(GetDirectoryPicker()->GetPath(), cell_value);
	filename = filename_filename.GetFullPath();
	return true;
}

bool PhotoMailerFrame::StartFtpServer()
{
	if (m_ftp_server)
		return true;

	m_ftp_server = new CFtpServer();
	m_ftp_server->SetClientCallback(&::OnClientEventCallback);
	m_ftp_server->SetMaxPasswordTries(3);
	m_ftp_server->SetNoLoginTimeout(60);
	m_ftp_server->SetNoTransferTimeout(60);

	unsigned long tmp;
	unsigned short int dataport_from = 100;
	unsigned short int dataport_to = 999;
	if (GetDataportFromCtrl()->GetValue().ToULong(&tmp))
		dataport_from = tmp;
	
	if (GetDataportToCtrl()->GetValue().ToULong(&tmp))
		dataport_to = tmp;

	if (dataport_to<dataport_from)
	{
		tmp = dataport_to;
		dataport_to = dataport_from;
		dataport_from = tmp;
	}

	if (!m_ftp_server->SetDataPortRange(dataport_from, dataport_to-dataport_from+1))
		return false;

	CFtpServer::CUserEntry* user = m_ftp_server->AddUser("ftp",
	                                                     GetFtpPasswordCtrl()->GetValue().ToUTF8(),
	                                                     GetDirectoryPicker()->GetPath().ToUTF8());
	if (!user)
		return false;

	user->SetMaxNumberOfClient(0);
	if (!user->SetPrivileges(CFtpServer::READFILE|CFtpServer::WRITEFILE|CFtpServer::DELETEFILE|CFtpServer::LIST|CFtpServer::CREATEDIR|CFtpServer::DELETEDIR))
		return false;

	unsigned short int port = 21;
	if (GetFtpPortCtrl()->GetValue().ToULong(&tmp))
		port = tmp;

	bool ret = (m_ftp_server->StartListening(INADDR_ANY, port) &&
	            m_ftp_server->StartAccepting());

	if (!ret && 1024>port)
	{
		wxMessageDialog dlg(this, _("Ports < 1024 requires root privileges"), _("FTP Server startup failed"));
		dlg.ShowModal();
	}

	return ret;
}

bool PhotoMailerFrame::StopFtpServer()
{
	if (!m_ftp_server)
		return true;

	m_ftp_server->StopListening();
	delete m_ftp_server;
	m_ftp_server = nullptr;
	return true;
}
