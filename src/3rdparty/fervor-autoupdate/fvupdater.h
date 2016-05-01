/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2016 Christian Kvasny <chris@ckvsoft.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Button Design, and Idea for the Layout are lean out from LillePOS, Copyright 2010, Martin Koller, kollix@aon.at
 *
*/

#ifndef FVUPDATER_H
#define FVUPDATER_H

#include <QObject>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QXmlStreamReader>
class QNetworkReply;
class FvUpdateWindow;
class FvUpdateConfirmDialog;
class FvAvailableUpdate;


class FvUpdater : public QObject
{
	Q_OBJECT

public:

	// Singleton
	static FvUpdater* sharedUpdater();
	static void drop();

	// Set / get feed URL
	void SetFeedURL(QUrl feedURL);
	void SetFeedURL(QString feedURL);
	QString GetFeedURL();
	void finishUpdate(QString pathToFinish = "");
	void setRequiredSslFingerPrint(QString md5);
	QString getRequiredSslFingerPrint();	// returns md5!
	// HTTP Authentuication - for security reasons no getters are provided, only a setter
	void setHtAuthCredentials(QString user, QString pass);
	void setHtAuthUsername(QString user);
	void setHtAuthPassword(QString pass);
	void setSkipVersionAllowed(bool allowed);
	void setRemindLaterAllowed(bool allowed);
	bool getSkipVersionAllowed();
	bool getRemindLaterAllowed();

	
public slots:

	// Check for updates
	bool CheckForUpdates(bool silentAsMuchAsItCouldGet = true);

	// Aliases
	bool CheckForUpdatesSilent();
	bool CheckForUpdatesNotSilent();


	//
	// ---------------------------------------------------
	// ---------------------------------------------------
	// ---------------------------------------------------
	// ---------------------------------------------------
	//

protected:

	friend class FvUpdateWindow;		// Uses GetProposedUpdate() and others
	friend class FvUpdateConfirmDialog;	// Uses GetProposedUpdate() and others
	FvAvailableUpdate* GetProposedUpdate();


protected slots:

	// Update window button slots
	void InstallUpdate();
	void SkipUpdate();
	void RemindMeLater();

private:

	//
	// Singleton business
	//
	// (we leave just the declarations, so the compiler will warn us if we try
	//  to use those two functions by accident)
	FvUpdater();							// Hide main constructor
	~FvUpdater();							// Hide main destructor
	FvUpdater(const FvUpdater&);			// Hide copy constructor
	FvUpdater& operator=(const FvUpdater&);	// Hide assign op

	static FvUpdater* m_Instance;			// Singleton instance


	//
	// Windows / dialogs
	//
#ifdef FV_GUI
	FvUpdateWindow* m_updaterWindow;								// Updater window (NULL if not shown)
	void showUpdaterWindowUpdatedWithCurrentUpdateProposal();		// Show updater window
	void hideUpdaterWindow();										// Hide + destroy m_updaterWindow
	void updaterWindowWasClosed();									// Sent by the updater window when it gets closed
#else
	void decideWhatToDoWithCurrentUpdateProposal();                 // Perform an action which is configured in settings
#endif

	// Available update (NULL if not fetched)
	FvAvailableUpdate* m_proposedUpdate;

	// If true, don't show the error dialogs and the "no updates." dialog
	// (silentAsMuchAsItCouldGet from CheckForUpdates() goes here)
	// Useful for automatic update checking upon application startup.
	bool m_silentAsMuchAsItCouldGet;

	// Dialogs (notifications)
	bool skipVersionAllowed;
	bool remindLaterAllowed;

	void showErrorDialog(QString message, bool showEvenInSilentMode = false);			// Show an error message
	void showInformationDialog(QString message, bool showEvenInSilentMode = false);		// Show an informational message


	//
	// HTTP feed fetcher infrastructure
	//
	QUrl m_feedURL;					// Feed URL that will be fetched
	QNetworkAccessManager m_qnam;
	QNetworkReply* m_reply;
	int m_httpGetId;
	bool m_httpRequestAborted;

	void startDownloadFeed(QUrl url);	// Start downloading feed
	void cancelDownloadFeed();			// Stop downloading the current feed

	//
	// SSL Fingerprint Check infrastructure
	//
	QString m_requiredSslFingerprint;

	bool checkSslFingerPrint(QUrl urltoCheck);	// true=ssl Fingerprint accepted, false= ssl Fingerprint NOT accepted

	//
	// Htauth-Infrastructure
	//
	QString htAuthUsername;
	QString htAuthPassword;


	//
	// XML parser
	//
	QXmlStreamReader m_xml;				// XML data collector and parser
	bool xmlParseFeed();				// Parse feed in m_xml
	bool searchDownloadedFeedForUpdates(QString xmlTitle,
										QString xmlLink,
										QString xmlReleaseNotesLink,
										QString xmlPubDate,
										QString xmlEnclosureUrl,
										QString xmlEnclosureVersion,
										QString xmlEnclosurePlatform,
										unsigned long xmlEnclosureLength,
										QString xmlEnclosureType);


	//
	// Helpers
	//
	void installTranslator();			// Initialize translation mechanism
	void restartApplication();			// Restarts application after update

private slots:

	void authenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator );
	void httpFeedReadyRead();
	void httpFeedUpdateDataReadProgress(qint64 bytesRead,
										qint64 totalBytes);
	void httpFeedDownloadFinished();

	//
	// Download and install Update infrastructure
	//
	void httpUpdateDownloadFinished();
	bool unzipUpdate(const QString & filePath, const QString & extDirPath, const QString & singleFileName = QString(""));	// returns true on success

signals:
	void updatedFinishedSuccessfully();

};

#endif // FVUPDATER_H
