
// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "app.h"
#endif

#include <wx/config.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>

#include "app.h"

IMPLEMENT_APP(PhotoMailer::PhotoMailerApp);


using namespace PhotoMailer;

IMPLEMENT_CLASS(PhotoMailerApp, wxApp)

PhotoMailerApp::PhotoMailerApp()
: wxApp(),
  m_main_frame(NULL)
{
}

PhotoMailerApp::~PhotoMailerApp()
{
//	delete m_main_frame; //Deleted by wxWidgets
}

bool PhotoMailerApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	::wxInitAllImageHandlers();

	//Create app settings directory if not already present
	wxString config_dir = wxStandardPaths::Get().GetUserLocalDataDir();
	if (!wxDir::Exists(config_dir)) 
	{
		::wxMkdir(config_dir);
	}

	//Init config
	wxConfig* config = new wxConfig(GetAppName(), wxEmptyString, ".settings", wxEmptyString, wxCONFIG_USE_SUBDIR|wxCONFIG_USE_LOCAL_FILE);
	if (!config)
	{
		wxASSERT_MSG(false, _("Could not initialize ConfigBase"));
		return false;
	}
	wxConfigBase::Set(config);

	//Initialize main frame
	m_main_frame = new PhotoMailerFrame(_("PhotoMailer"));
	if (!m_main_frame)
	{
		wxASSERT_MSG(false, _("wxMailto_Frame is NULL"));
		return false;
	}

	m_main_frame->Show(true);

	return true;
}
