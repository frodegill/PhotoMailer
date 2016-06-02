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
    
    wxFlexGridSizer* flexGridSizer38 = new wxFlexGridSizer(3, 4, 0, 0);
    flexGridSizer38->SetFlexibleDirection( wxBOTH );
    flexGridSizer38->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    flexGridSizer38->AddGrowableCol(1);
    flexGridSizer38->AddGrowableCol(3);
    
    smtpStaticBoxSizer->Add(flexGridSizer38, 1, wxALL|wxEXPAND, 0);
    
    m_smtpServerText = new wxStaticText(this, wxID_ANY, _("Server"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer38->Add(m_smtpServerText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpServerCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_smtpServerCtrl->SetHint(wxT(""));
    #endif
    
    flexGridSizer38->Add(m_smtpServerCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_smtpPortText = new wxStaticText(this, wxID_ANY, _("Port"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer38->Add(m_smtpPortText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpPortCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_smtpPortCtrl->SetHint(wxT(""));
    #endif
    
    flexGridSizer38->Add(m_smtpPortCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_smtpUsernameText = new wxStaticText(this, wxID_ANY, _("Username"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer38->Add(m_smtpUsernameText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpUsernameCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_smtpUsernameCtrl->SetHint(wxT(""));
    #endif
    
    flexGridSizer38->Add(m_smtpUsernameCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_smtpPasswordText = new wxStaticText(this, wxID_ANY, _("Password"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer38->Add(m_smtpPasswordText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_smtpPasswordCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), wxTE_PASSWORD);
    #if wxVERSION_NUMBER >= 3000
    m_smtpPasswordCtrl->SetHint(wxT(""));
    #endif
    
    flexGridSizer38->Add(m_smtpPasswordCtrl, 1, wxALL|wxEXPAND, 5);
    
    m_senderText = new wxStaticText(this, wxID_ANY, _("Sender email"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer38->Add(m_senderText, 0, wxALL|wxALIGN_RIGHT, 5);
    
    m_senderCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0);
    #if wxVERSION_NUMBER >= 3000
    m_senderCtrl->SetHint(wxT(""));
    #endif
    
    flexGridSizer38->Add(m_senderCtrl, 0, wxALL|wxEXPAND, 5);
    
    wxBoxSizer* directorySizer = new wxBoxSizer(wxHORIZONTAL);
    
    mainBoxSizer->Add(directorySizer, 0, wxALL|wxEXPAND, 5);
    
    m_directoryText = new wxStaticText(this, wxID_ANY, _("Directory"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    directorySizer->Add(m_directoryText, 0, wxALL, 5);
    
    m_directoryPicker = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, _("Select a folder"), wxDefaultPosition, wxSize(-1,-1), wxDIRP_SMALL|wxDIRP_DEFAULT_STYLE);
    
    directorySizer->Add(m_directoryPicker, 1, wxALL, 5);
    
    m_directoryListenButton = new wxButton(this, wxID_ANY, _("Listen"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    directorySizer->Add(m_directoryListenButton, 0, wxALL, 5);
    
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
