//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// wxCrafter project file: PhotoMailer.wxcp
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#include "PhotoMailerGenerated.h"


// Declare the bitmap loading function
extern void wxCrafter1oXELPInitBitmapResources();

static bool bBitmapLoaded = false;


PhotoMailerFrameGenerated::PhotoMailerFrameGenerated(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style)
{
    if ( !bBitmapLoaded ) {
        // We need to initialise the default bitmap handler
        wxXmlResource::Get()->AddHandler(new wxBitmapXmlHandler);
        wxCrafter1oXELPInitBitmapResources();
        bBitmapLoaded = true;
    }
    
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);
    
    wxStaticBoxSizer* smtpStaticBoxSizer = new wxStaticBoxSizer( new wxStaticBox(this, wxID_ANY, _("SMTP")), wxVERTICAL);
    
    mainBoxSizer->Add(smtpStaticBoxSizer, 0, wxALL|wxEXPAND, 5);
    
    wxFlexGridSizer* smtpFlexGridSizer = new wxFlexGridSizer(5, 4, 0, 0);
    smtpFlexGridSizer->SetFlexibleDirection( wxBOTH );
    smtpFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    smtpFlexGridSizer->AddGrowableCol(1);
    smtpFlexGridSizer->AddGrowableCol(3);
    
    smtpStaticBoxSizer->Add(smtpFlexGridSizer, 1, wxALL|wxEXPAND, 0);
    
    m_smtpServerText = new wxStaticText(this, wxID_ANY, _("Server"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_smtpServerText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpServerCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_smtpServerCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_smtpServerCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_smtpPortText = new wxStaticText(this, wxID_ANY, _("Port"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_smtpPortText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpPortCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_smtpPortCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_smtpPortCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_smtpUsernameText = new wxStaticText(this, wxID_ANY, _("Username"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_smtpUsernameText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpUsernameCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_smtpUsernameCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_smtpUsernameCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_smtpPasswordText = new wxStaticText(this, wxID_ANY, _("Password"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_smtpPasswordText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpPasswordCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), wxTE_PASSWORD);
    #if wxVERSION_NUMBER >= 3000
    m_smtpPasswordCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_smtpPasswordCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_senderText = new wxStaticText(this, wxID_ANY, _("Sender email"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_senderText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_senderCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_senderCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_senderCtrl, 0, wxALL|wxEXPAND, 5);
    
    m_subjectText = new wxStaticText(this, wxID_ANY, _("Subject"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_subjectText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_subjectCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_subjectCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_subjectCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_ftpPasswordText = new wxStaticText(this, wxID_ANY, _("'ftp' password"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_ftpPasswordText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_ftpPasswordCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_ftpPasswordCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_ftpPasswordCtrl, 0, wxALL, 5);
    
    m_ftpPortText = new wxStaticText(this, wxID_ANY, _("FTP port"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_ftpPortText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_ftpPortCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_ftpPortCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_ftpPortCtrl, 0, wxALL, 5);
    
    m_dataportFromText = new wxStaticText(this, wxID_ANY, _("FTP DataPort From"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_dataportFromText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_dataportFromCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_dataportFromCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_dataportFromCtrl, 0, wxALL, 5);
    
    m_dataportToText = new wxStaticText(this, wxID_ANY, _("FTP DataPort To"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    smtpFlexGridSizer->Add(m_dataportToText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_dataportToCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_dataportToCtrl->SetHint(wxT(""));
    #endif
    
    smtpFlexGridSizer->Add(m_dataportToCtrl, 0, wxALL, 5);
    
    wxBoxSizer* directorySizer = new wxBoxSizer(wxHORIZONTAL);
    
    mainBoxSizer->Add(directorySizer, 0, wxALL|wxEXPAND, 5);
    
    m_directoryText = new wxStaticText(this, wxID_ANY, _("FTP Directory"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    directorySizer->Add(m_directoryText, 0, wxALL, 5);
    
    m_directoryPicker = new wxDirPickerCtrl(this, ID_DIRECTORY, wxEmptyString, _("Select a folder"), wxDefaultPosition, wxSize(-1,-1), wxDIRP_SMALL|wxDIRP_DEFAULT_STYLE);
    
    directorySizer->Add(m_directoryPicker, 1, wxALL, 5);
    
    m_ftpStartButton = new wxButton(this, ID_FTPSERVER_BUTTON, _("Start FTP server"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    directorySizer->Add(m_ftpStartButton, 0, wxALL, 5);
    
    m_photosGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), wxWANTS_CHARS);
    m_photosGrid->CreateGrid(0, 0);
    m_photosGrid->SetRowLabelAlignment(wxALIGN_RIGHT, wxALIGN_CENTRE);
    m_photosGrid->SetColLabelAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    #if wxVERSION_NUMBER >= 2904
    m_photosGrid->UseNativeColHeader(true);
    #endif
    m_photosGrid->EnableEditing(true);
    
    mainBoxSizer->Add(m_photosGrid, 1, wxALL|wxEXPAND, 5);
    
    SetName(wxT("PhotoMailerFrameGenerated"));
    SetSize(-1,-1);
    if (GetSizer()) {
         GetSizer()->Fit(this);
    }
    if(GetParent()) {
        CentreOnParent(wxBOTH);
    } else {
        CentreOnScreen(wxBOTH);
    }
#if wxVERSION_NUMBER >= 2900
    if(!wxPersistenceManager::Get().Find(this)) {
        wxPersistenceManager::Get().RegisterAndRestore(this);
    } else {
        wxPersistenceManager::Get().Restore(this);
    }
#endif
}

PhotoMailerFrameGenerated::~PhotoMailerFrameGenerated()
{
}
