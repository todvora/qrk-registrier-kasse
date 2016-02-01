#include "fvupdatewindow.h"
#include "ui_fvupdatewindow.h"
#include "fvupdater.h"
#include "fvavailableupdate.h"
#include <QApplication>
#include <QCloseEvent>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QDebug>


FvUpdateWindow::FvUpdateWindow(QWidget *parent, bool skipVersionAllowed, bool remindLaterAllowed) :
        QWidget(parent, Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTitleHint),
	m_ui(new Ui::FvUpdateWindow)
{
	m_ui->setupUi(this);

	m_appIconScene = 0;

	if(!skipVersionAllowed)
		m_ui->skipThisVersionButton->hide();
	if(!remindLaterAllowed)
		m_ui->remindMeLaterButton->hide();

	// Delete on close
	setAttribute(Qt::WA_DeleteOnClose, true);

	// Set the "new version is available" string
	QString newVersString = m_ui->newVersionIsAvailableLabel->text().arg(QApplication::applicationName());
	m_ui->newVersionIsAvailableLabel->setText(newVersString);

	// Connect buttons
	connect(m_ui->installUpdateButton, SIGNAL(clicked()),
			FvUpdater::sharedUpdater(), SLOT(InstallUpdate()));
	connect(m_ui->skipThisVersionButton, SIGNAL(clicked()),
			FvUpdater::sharedUpdater(), SLOT(SkipUpdate()));
	connect(m_ui->remindMeLaterButton, SIGNAL(clicked()),
			FvUpdater::sharedUpdater(), SLOT(RemindMeLater()));
}

FvUpdateWindow::~FvUpdateWindow()
{
        // m_ui->releaseNotesWebView->stop();
	delete m_ui;
}

bool FvUpdateWindow::UpdateWindowWithCurrentProposedUpdate()
{
	FvAvailableUpdate* proposedUpdate = FvUpdater::sharedUpdater()->GetProposedUpdate();
	if (! proposedUpdate) {
		return false;
	}

	QString downloadString = m_ui->wouldYouLikeToDownloadLabel->text()
			.arg(QApplication::applicationName(), proposedUpdate->GetEnclosureVersion(), QApplication::applicationVersion());
	m_ui->wouldYouLikeToDownloadLabel->setText(downloadString);

        QEventLoop eventLoop;

        // "quit()" the event-loop, when the network request "finished()"
        QNetworkAccessManager mgr;
        QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

        // the HTTP request
        QNetworkRequest req( proposedUpdate->GetReleaseNotesLink() );
        QNetworkReply *reply = mgr.get(req);
        eventLoop.exec(); // blocks stack until "finished()" has been called

        if (reply->error() == QNetworkReply::NoError) {
            //success
//            qDebug() << "Success" <<reply->readAll();
            m_ui->releaseNotesWebView->setHtml(reply->readAll());
            delete reply;
        }
        else {
            //failure
            qDebug() << "Failure" <<reply->errorString();
            delete reply;
        }

        return true;
}

void FvUpdateWindow::closeEvent(QCloseEvent* event)
{
        FvUpdater::sharedUpdater()->updaterWindowWasClosed();
	event->accept();
}
