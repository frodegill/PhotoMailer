// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "frame.h"
#endif

#include <wx/confbase.h>

#include "frame.h"

#include "app.h"

using namespace PhotoMailer;

// the application icon
#include "photomailer.xpm"


BEGIN_EVENT_TABLE(PhotoMailerFrame, wxFrame)
	EVT_MENU(wxID_EXIT,	PhotoMailerFrame::OnQuit)
  EVT_BUTTON(ID_LISTEN, PhotoMailerFrame::OnListen)
	EVT_FSWATCHER(wxID_ANY, PhotoMailerFrame::OnDirectoryEvent)
	EVT_GRID_SELECT_CELL(PhotoMailerFrame::OnGridSelectCell)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PhotoMailerFrame, wxFrame)

PhotoMailerFrame::PhotoMailerFrame(const wxString& title)
: PhotoMailerFrameGenerated(nullptr),
  m_filesystem_watcher(nullptr),
  m_is_batch_updating(false),
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
	AddPhoto(filename);
	return wxDIR_CONTINUE;
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
				if (0 == relative_filename.Cmp(grid->GetCellValue(i, 1)))
				{
					grid->DeleteRows(i, 1, false);
					break;
				}
			}
		}
	}
	if (0!=(type&wxFSW_EVENT_CREATE) || 0!=(type&wxFSW_EVENT_RENAME))
	{
		AddPhoto((0!=(type&wxFSW_EVENT_RENAME) ? event.GetNewPath() : event.GetPath()).GetFullPath());
	}
}

void PhotoMailerFrame::OnGridSelectCell(wxGridEvent& event)
{
	int event_row = event.GetRow();
	if (m_selected_row == event_row)
		return;

	wxGrid* grid = GetPhotosGrid();
	m_selected_row = event_row;
	wxString filename;
	if (0>m_selected_row || (event_row+1)>=grid->GetNumberRows())
	{
		filename = wxEmptyString;
		return;
	}
	else
	{
		filename = GetDirectoryPicker()->GetPath() + "/" + grid->GetCellValue(m_selected_row, 1);
	}

	wxGetApp().GetPreviewFrame()->ShowPhoto(filename);
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
		grid->AppendCols(5-number_of_cols);

		wxGridCellAttr* ro_attr = new wxGridCellAttr;
		ro_attr->SetReadOnly();

		wxGridCellAttr* editor_attr = new wxGridCellAttr;
		editor_attr->SetEditor(new wxGridCellTextEditor);

		grid->SetColLabelValue(0, _("Photo"));
		grid->SetColAttr(0, ro_attr);
		grid->SetColLabelValue(1, _("Filename"));
		ro_attr->IncRef(); grid->SetColAttr(1, ro_attr);
		grid->SetColLabelValue(2, _("Time"));
		ro_attr->IncRef(); grid->SetColAttr(2, ro_attr);
		grid->SetColLabelValue(3, _("Email"));
		grid->SetColAttr(3, editor_attr);
		grid->SetColLabelValue(4, _("Action"));
	}
}

void PhotoMailerFrame::RefreshPhotoList()
{
	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();

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
	
	grid->AutoSizeColumn(1);
	grid->AutoSizeColumn(2);

	grid->EndBatch();
	m_is_batch_updating = false;
}

bool PhotoMailerFrame::AddPhoto(const wxString& filename)
{
	if (!IsJpeg(filename))
		return false;

	wxMutexLocker lock(m_photolist_mutex);

	wxGrid* grid = GetPhotosGrid();
	grid->AppendRows(1);
	int current_row = grid->GetNumberRows()-1;
	
	//Filename
	wxString cell_filename;
	if (!GetRelativeFilename(filename, cell_filename))
	{
		cell_filename = filename;
	}
	grid->SetCellValue(current_row, 1, cell_filename);

	//File last modified time
	wxStructStat stat;
	wxStat(filename, &stat);
	wxDateTime last_modified(stat.st_mtime);
	grid->SetCellValue(current_row, 2, last_modified.FormatISOCombined(' '));

	if (!m_is_batch_updating)
	{
		grid->AutoSizeColumn(1);
		grid->AutoSizeColumn(2);
		grid->SelectRow(current_row);
	}

	return true;
}
