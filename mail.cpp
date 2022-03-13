// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "mail.h"
#endif

#include <fstream>
#include <ios>

#include <vmime/platforms/posix/posixHandler.hpp>

#include <wx/msgout.h>

#include "app.h"
#include "frame.h"
#include "mail.h"


#undef ENABLE_TRACE


#define CA_CERT_FILE            "/etc/ssl/certs/ca-certificates.crt"
#define AMAZON_SES_CA_CERT_FILE "ca-certificates/SFSRootCAG2.crt"


using namespace PhotoMailer;

#ifdef ENABLE_TRACE
class myTracer : public vmime::net::tracer
{
public:
  myTracer(const vmime::string& proto, const int connectionId) : m_proto(proto), m_connectionId(connectionId) {}
  void traceSend(const vmime::string& line) {std::cout << " [ " << m_proto << " : " << m_connectionId << " ] C: " << line << std::endl;}
  void traceReceive(const vmime::string& line) {std::cout << " [ " << m_proto << " : " << m_connectionId << " ] S: " << line << std::endl;}
private :
  const vmime::string m_proto;
  const int m_connectionId;
};

class myTracerFactory : public vmime::net::tracerFactory
{
public:
  virtual std::shared_ptr<vmime::net::tracer> create(const std::shared_ptr<vmime::net::service>& serv, const int connectionId) {return std::make_shared<myTracer>(serv->getProtocolName(), connectionId);}
	//virtual shared_ptr <tracer> create(shared_ptr <service> serv, const int connectionId) = 0;
};
#endif //ENABLE_TRACE

MailThread::MailThread(int row, const wxString& smtp_server, const wxString& smtp_port,
	           const wxString& smtp_username, const wxString& smtp_password,
	           const wxString& from, const wxString& to,
	           const wxString& subject, const wxString& filename)
: wxThread(),
  m_row(row),
  m_has_failed(false),
  m_smtp_server(smtp_server),
  m_smtp_port(smtp_port),
  m_smtp_username(smtp_username),
  m_smtp_password(smtp_password),
  m_from(from),
  m_to(to),
  m_subject(subject),
  m_filename(filename)
{
	::wxGetApp().GetMainFrame()->RegisterThread(this);
}

MailThread::~MailThread()
{
	::wxGetApp().GetMainFrame()->UnregisterThread(this);
}

wxThread::ExitCode MailThread::Entry()
{
	wxMessageOutputStderr logger;
	try {
		vmime::shared_ptr<vmime::security::cert::defaultCertificateVerifier> certificate_verifier = vmime::make_shared<vmime::security::cert::defaultCertificateVerifier>();

    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>> root_ca_list;
    root_ca_list.push_back(LoadCACertificateFile(CA_CERT_FILE));
    root_ca_list.push_back(LoadCACertificateFile(AMAZON_SES_CA_CERT_FILE));
    certificate_verifier->setX509RootCAs(root_ca_list);

		//Set up SMTP transport
		std::string smtp_url_string;
		GetSMTPUrl(smtp_url_string);
		vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();
		vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();
		vmime::shared_ptr<vmime::net::transport> transport = session->getTransport(vmime::utility::url(smtp_url_string));
		transport->setCertificateVerifier(certificate_verifier);
    transport->setProperty("connection.tls", true);
    transport->setProperty("connection.tls.required", true);

		//Set up SMTP authentication
		if (!m_smtp_username.IsEmpty())
		{
			transport->setProperty("options.need-authentication", true);
			transport->setProperty("auth.username", m_smtp_username.ToStdString());
			transport->setProperty("auth.password", m_smtp_password.ToStdString());
		}

#ifdef ENABLE_TRACE
    transport->setTracerFactory(vmime::make_shared<myTracerFactory>());
#endif

    //Connect
		transport->connect();

		//Create message
		vmime::messageBuilder mb;

		vmime::mailbox mailbox;
		std::string mailbox_string = m_from.ToStdString();
		mailbox.parse(mailbox_string, 0, mailbox_string.length());
		mb.setExpeditor(mailbox);

		mb.getRecipients().appendAddress(vmime::make_shared<vmime::mailbox>(m_to.ToStdString()));
		mb.setSubject(vmime::text(m_subject.ToStdString()));

		//Attachment
		if (m_filename.IsEmpty())
		{
			m_has_failed = true;
		}
		else
		{
			vmime::shared_ptr<vmime::fileAttachment> attachment =
				vmime::make_shared<vmime::fileAttachment>(m_filename.ToStdString(), vmime::mediaType("image/jpeg"));
			
			mb.appendAttachment(attachment);

			transport->send(mb.construct(), this);
		}

		transport->disconnect();
	}
	catch (vmime::exception& e)
	{
		m_has_failed = true;
		logger.Printf(_("Got exception: %s"), e.what());
	}
	catch (std::exception& e)
	{
		m_has_failed = true;
		logger.Printf(_("Got exception: %s"), e.what());
	}
	catch (...)
	{
		m_has_failed = true;
		logger.Output(_("Got unknown exception"));
	}

	//Send completed progress
	wxThreadEvent* threadEvent = new wxThreadEvent(wxEVT_THREAD, MAIL_PROGRESS_EVENT);
	MailProgressEventPayload* payload = new MailProgressEventPayload(m_row, m_has_failed, 100.0);
	threadEvent->SetPayload<MailProgressEventPayload*>(payload);
	::wxQueueEvent(::wxGetApp().GetMainFrame(), threadEvent);

	return static_cast<wxThread::ExitCode>(0);
}

void MailThread::start(const size_t WXUNUSED(predictedTotal))
{
}

void MailThread::progress(const size_t current, const size_t currentTotal)
{
	if (0 != currentTotal)
	{
		wxThreadEvent* threadEvent = new wxThreadEvent(wxEVT_THREAD, MAIL_PROGRESS_EVENT);
		MailProgressEventPayload* payload = new MailProgressEventPayload(m_row, m_has_failed,
																																		 static_cast<double>(current)/static_cast<double>(currentTotal));
		threadEvent->SetPayload<MailProgressEventPayload*>(payload);
		::wxQueueEvent(::wxGetApp().GetMainFrame(), threadEvent);
	}
}

void MailThread::stop(const size_t WXUNUSED(total))
{
}

vmime::shared_ptr<vmime::security::cert::X509Certificate> MailThread::LoadCACertificateFile(const std::string& filename)
{
	std::ifstream cert_file;
	cert_file.open(filename, std::ios::in|std::ios::binary);
	if (!cert_file)
		return nullptr;

	vmime::utility::inputStreamAdapter is(cert_file);
	return vmime::security::cert::X509Certificate::import(is);
}

void MailThread::GetSMTPUrl(std::string& url)
{
	url = (_("smtps://")+m_smtp_server+_(":")+m_smtp_port).ToStdString();
}



MailProgressEventPayload::MailProgressEventPayload(int row, bool has_failed, double progress)
: m_row(row),
  m_has_failed(has_failed),
  m_progress(progress)
{
}
