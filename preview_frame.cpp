// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "preview_frame.h"
#endif

#include "preview_frame.h"

#include "app.h"

using namespace PhotoMailer;


BEGIN_EVENT_TABLE(PreviewFrame, wxMiniFrame)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PreviewFrame, wxMiniFrame)

PreviewFrame::PreviewFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                           const wxPoint& pos, const wxSize& size, long style)
: wxMiniFrame(parent, id, title, pos, size, style)
{
}

PreviewFrame::~PreviewFrame()
{
}
