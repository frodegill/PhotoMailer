#ifndef _PHOTOMAILER_APP_H_
#define _PHOTOMAILER_APP_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "app.h"
#endif

#include <wx/app.h>
#include "frame.h"


namespace PhotoMailer
{

class PhotoMailerApp : public wxApp
{
DECLARE_DYNAMIC_CLASS(PhotoMailer);
public:
	PhotoMailerApp();
	virtual ~PhotoMailerApp();

  virtual bool OnInit() wxOVERRIDE;

public:
	PhotoMailerFrame* GetMainFrame() const {return m_main_frame;}

private:
	PhotoMailerFrame* m_main_frame;
};

}

DECLARE_APP(PhotoMailer::PhotoMailerApp);

#endif // _PHOTOMAILER_APP_H_
