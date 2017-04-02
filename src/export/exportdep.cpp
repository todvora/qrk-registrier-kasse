/*
 * Manipulationssicherheit
 * Die Datenbank für das DEP wird in der Hauptdatenbank mitgeführt, ist allerdings vom User
 * nicht direkt les- oder änderbar.
 * Durch die Kombination aus fortlaufender Sequenznummer und dem Hashwert aus
 * den Zeileneinträgen ist das DEP auch nach dem Export vor Manipulation geschützt.
*/

#include "exportdep.h"
#include "exportdialog.h"
#include "RK/rk_signaturemodule.h"
#include "singleton/spreadsignal.h"
#include "utils/utils.h"
#include "preferences/qrksettings.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>

#include <QDebug>

ExportDEP::ExportDEP(QWidget *parent)
    : QDialog(parent)
{

}

ExportDEP::~ExportDEP()
{
    SpreadSignal::setProgressBarValue(-1);
}

void ExportDEP::Export()
{
    ExportDialog *dlg = new ExportDialog(true);
    if (dlg->exec() == QDialog::Accepted ) {
        QString filename = dlg->getFilename();
        if (depExport(filename, dlg->getFrom(), dlg->getTo())) {
            delete dlg;
            SpreadSignal::setProgressBarValue(-1);
            QMessageBox::information(0, tr("Export"), tr("DEP (Daten-Erfassungs-Protokol) wurde nach %1 exportiert.").arg(filename));
        } else {
            delete dlg;
            SpreadSignal::setProgressBarValue(-1);
            QMessageBox::warning(0, tr("Export"), tr("DEP (Daten-Erfassungs-Protokol) konnte nicht nach %1 exportiert werden.\nÜberprüfen Sie bitte Ihre Schreibberechtigung.").arg(filename));
        }
    } else {
        delete dlg;
    }
}

QJsonDocument ExportDEP::depExport(int from, int to)
{
    QJsonObject root;
    QJsonArray group;
    QJsonObject object;

    object["Belege-kompakt"] = getReceipts(from, to);
    object["Signaturzertifikat"] = "";

    group.append(object);

    root["Belege-Gruppe"] = group;

    QJsonDocument doc(root);

    return doc;
}

QJsonDocument ExportDEP::mapExport()
{
    QJsonObject root;
    QJsonObject map;

    map = RKSignatureModule::getCertificateMap();

    root["base64AESKey"] = RKSignatureModule::getPrivateTurnoverKeyBase64();
    root["certificateOrPublicKeyMap"] = map;

    QJsonDocument doc(root);

    return doc;
}

QJsonArray ExportDEP::getReceipts(int from, int to)
{
    QJsonArray receipts;
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare(QString("SELECT data FROM dep WHERE receiptNum BETWEEN :from AND :to ORDER by id"));
    query.bindValue(":from", from);
    query.bindValue(":to", to);

    query.exec();

    int i = 0;
    int count = query.record().count();

    while (query.next()) {
        i++;
        SpreadSignal::setProgressBarValue(((float)i / (float)count) * 100);
        receipts.append(query.value(0).toString());
    }

    return receipts;
}

bool ExportDEP::depExport(QString filename)
{
    QFile *outputFile = new QFile(filename);

    /* Try and open a file for output */
    outputFile->open(QIODevice::WriteOnly | QIODevice::Text);

    /* Check it opened OK */
    if(!outputFile->isOpen()){
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error, unable to open" << filename << "for output";
        return false;
    }

    int beginID = 1;
    int endID = getLastMonthReceiptId();

    QJsonDocument dep = depExport(beginID, endID);
    QTextStream outStreamDEP(outputFile);
    outStreamDEP << dep.toJson();
    /* Close the file */
    outputFile->close();

    if (endID == -1)
        return false;

    return true;
}

bool ExportDEP::depExport(QString outputDir, QString from, QString to)
{
    QString filenameDEPExport = QDir::toNativeSeparators(outputDir + "/dep-export.json");
    QString filenameMAPExport = QDir::toNativeSeparators(outputDir + "/cryptographicMaterialContainer.json");

    m_outputFileDEP = new QFile(filenameDEPExport);
    m_outputFileMAP = new QFile(filenameMAPExport);

    /* Try and open a file for output */
    m_outputFileDEP->open(QIODevice::WriteOnly | QIODevice::Text);
    m_outputFileMAP->open(QIODevice::WriteOnly | QIODevice::Text);

    /* Check it opened OK */
    if(!m_outputFileDEP->isOpen()){
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error, unable to open" << m_outputFileDEP->fileName() << "for output";
        return false;
    }

    /* Check it opened OK */
    if(!m_outputFileMAP->isOpen()){
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error, unable to open" << m_outputFileDEP->fileName() << "for output";
        return false;
    }

    /* Point a QTextStream object at the file */
    QTextStream outStreamDEP(m_outputFileDEP);
    QTextStream outStreamMAP(m_outputFileMAP);

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare(QString("SELECT MIN(receiptNum) as begin, MAX(receiptNum) as end FROM receipts WHERE timestamp BETWEEN :fromDate AND :toDate"));
    query.bindValue(":fromDate", from);
    query.bindValue(":toDate", to);
    query.exec();

    if (query.next()) {
        QJsonDocument dep = depExport(query.value("begin").toInt(), query.value("end").toInt());
        QJsonDocument map = mapExport();
        outStreamDEP << dep.toJson();
        outStreamMAP << map.toJson();
    }

    /* Close the file */
    m_outputFileDEP->close();
    m_outputFileMAP->close();

    return true;
}

int ExportDEP::getLastMonthReceiptId()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare(QString("SELECT MAX(receiptNum) as maxID FROM receipts WHERE payedBy=4"));
    query.exec();
    if (query.next()) {
        int maxID = query.value("maxID").toInt();
        query.prepare(QString("SELECT payedBy FROM receipts WHERE receiptNum=:id"));
        query.bindValue(":id", maxID +1);
        query.exec();
        if (query.next()) {
             if (query.value("payedBy").toInt() == 8)
                 return maxID +1;
        }
    }
    return -1;
}

bool ExportDEP::createBackup()
{
    QrkSettings settings;
    QString directoryname = settings.value("externalDepDirectory", "").toString();

    if (Utils::isDirectoryWritable(directoryname)) {
        QString filename = QDir::toNativeSeparators(directoryname + "/DEP_backup_" + QDateTime::currentDateTime().toString(Qt::ISODate).replace(':',"").replace('-',"") + ".json");
        if (depExport(filename))
            return true;
    }

    return false;
}
