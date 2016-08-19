//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// wxCrafter project file: PhotoMailer.wxcp
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#ifndef PROJECTS_PHOTOMAILER_PHOTOMAILER_BASE_CLASSES_H
#define PROJECTS_PHOTOMAILER_PHOTOMAILER_BASE_CLASSES_H

#include <wx/settings.h>
#include <wx/xrc/xmlres.h>
#include <wx/xrc/xh_bmp.h>
#include <wx/frame.h>
#include <wx/iconbndl.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/button.h>
#include <wx/grid.h>
#if wxVERSION_NUMBER >= 2900
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/persist/bookctrl.h>
#include <wx/persist/treebook.h>
#endif

class PhotoMailerFrameGenerated : public wxFrame
{
public:
    enum {
        ID_DIRECTORY = 10001,
        ID_FTPSERVER_BUTTON = 10002,
    };
protected:
    wxStaticText* m_smtpServerText;
    wxTextCtrl* m_smtpServerCtrl;
    wxStaticText* m_smtpPortText;
    wxTextCtrl* m_smtpPortCtrl;
    wxStaticText* m_smtpUsernameText;
    wxTextCtrl* m_smtpUsernameCtrl;
    wxStaticText* m_smtpPasswordText;
    wxTextCtrl* m_smtpPasswordCtrl;
    wxStaticText* m_senderText;
    wxTextCtrl* m_senderCtrl;
    wxStaticText* m_subjectText;
    wxTextCtrl* m_subjectCtrl;
    wxStaticText* m_ftpPasswordText;
    wxTextCtrl* m_ftpPasswordCtrl;
    wxStaticText* m_ftpPortText;
    wxTextCtrl* m_ftpPortCtrl;
    wxStaticText* m_dataportFromText;
    wxTextCtrl* m_dataportFromCtrl;
    wxStaticText* m_dataportToText;
    wxTextCtrl* m_dataportToCtrl;
    wxStaticText* m_directoryText;
    wxDirPickerCtrl* m_directoryPicker;
    wxButton* m_ftpStartButton;
    wxGrid* m_photosGrid;

protected:

public:
    wxStaticText* GetSmtpServerText() { return m_smtpServerText; }
    wxTextCtrl* GetSmtpServerCtrl() { return m_smtpServerCtrl; }
    wxStaticText* GetSmtpPortText() { return m_smtpPortText; }
    wxTextCtrl* GetSmtpPortCtrl() { return m_smtpPortCtrl; }
    wxStaticText* GetSmtpUsernameText() { return m_smtpUsernameText; }
    wxTextCtrl* GetSmtpUsernameCtrl() { return m_smtpUsernameCtrl; }
    wxStaticText* GetSmtpPasswordText() { return m_smtpPasswordText; }
    wxTextCtrl* GetSmtpPasswordCtrl() { return m_smtpPasswordCtrl; }
    wxStaticText* GetSenderText() { return m_senderText; }
    wxTextCtrl* GetSenderCtrl() { return m_senderCtrl; }
    wxStaticText* GetSubjectText() { return m_subjectText; }
    wxTextCtrl* GetSubjectCtrl() { return m_subjectCtrl; }
    wxStaticText* GetFtpPasswordText() { return m_ftpPasswordText; }
    wxTextCtrl* GetFtpPasswordCtrl() { return m_ftpPasswordCtrl; }
    wxStaticText* GetFtpPortText() { return m_ftpPortText; }
    wxTextCtrl* GetFtpPortCtrl() { return m_ftpPortCtrl; }
    wxStaticText* GetDataportFromText() { return m_dataportFromText; }
    wxTextCtrl* GetDataportFromCtrl() { return m_dataportFromCtrl; }
    wxStaticText* GetDataportToText() { return m_dataportToText; }
    wxTextCtrl* GetDataportToCtrl() { return m_dataportToCtrl; }
    wxStaticText* GetDirectoryText() { return m_directoryText; }
    wxDirPickerCtrl* GetDirectoryPicker() { return m_directoryPicker; }
    wxButton* GetFtpStartButton() { return m_ftpStartButton; }
    wxGrid* GetPhotosGrid() { return m_photosGrid; }
    PhotoMailerFrameGenerated(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("PhotoMailerFrame"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxDEFAULT_FRAME_STYLE);
    virtual ~PhotoMailerFrameGenerated();
};

#endif
