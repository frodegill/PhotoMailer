#ifndef _PHOTOMAILER_APP_H_
#define _PHOTOMAILER_APP_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "app.h"
#endif

#include <wx/app.h>

#include "frame.h"
#include "mail.h"
#include "preview_frame.h"


#define EQUALS (0)


namespace PhotoMailer
{

class PhotoMailerApp : public wxApp
{
wxDECLARE_DYNAMIC_CLASS(PhotoMailer);
public:
	PhotoMailerApp();
	virtual ~PhotoMailerApp();

  virtual bool OnInit() wxOVERRIDE;

	void OnPreviewFrameClosed() {m_preview_frame = nullptr;}

public:
	PhotoMailerFrame* GetMainFrame() const {return m_main_frame;}
	PreviewFrame* GetPreviewFrame();
	Mail* GetMail() const {return m_mail;}

private:
	PhotoMailerFrame* m_main_frame;
	PreviewFrame*     m_preview_frame;

	Mail*             m_mail;
};

}

DECLARE_APP(PhotoMailer::PhotoMailerApp);

#endif // _PHOTOMAILER_APP_H_
