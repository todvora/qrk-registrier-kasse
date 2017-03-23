/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2017 Christian Kvasny <chris@ckvsoft.at>
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

#include "rk_signaturesmartcard.h"

#include <QString>
#include <QDebug>
#include <iostream>

#ifndef SCARD_ATTR_VALUE
#define SCARD_ATTR_VALUE(Class, Tag) ((((ULONG)(Class)) << 16) | ((ULONG)(Tag)))
#endif

#ifndef SCARD_CLASS_ICC_STATE
#define SCARD_CLASS_ICC_STATE       9   /**< ICC State specific definitions */
#endif

#ifndef SCARD_ATTR_ATR_STRING
#define SCARD_ATTR_ATR_STRING SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0303) /**< Answer to reset (ATR) string. */
#endif

#ifndef SCARD_E_NO_READERS_AVAILABLE
#define SCARD_E_NO_READERS_AVAILABLE                ((DWORD)0x8010002EL)
//
// MessageId: SCARD_E_NO_READERS_AVAILABLE
//
// MessageText:
//
//   Cannot find a smart card reader.
#endif

#ifndef SCARD_E_NO_RESRC_MNGR_RUNNING
#define SCARD_E_NO_RESRC_MNGR_RUNNING               ((DWORD)0x8010001DL)
//
// MessageId: SCARD_E_NO_RESRC_MNGR_RUNNING
//
// MessageText:
//
//   The Smart card resource manager is not running.
#endif

#ifndef SCARD_W_WRONG_CHV
#define SCARD_W_WRONG_CHV                           ((DWORD)0x8010006BL)
//
// MessageId: SCARD_W_WRONG_CHV
//
// MessageText:
//
//   The card cannot be accessed because the wrong PIN was presented.
#endif

#ifndef SCARD_W_CHV_BLOCKED
#define SCARD_W_CHV_BLOCKED                         ((DWORD)0x8010006CL)
//
// MessageId: SCARD_W_CHV_BLOCKED
//
// MessageText:
//
//   The card cannot be accessed because the maximum number of PIN entry attempts has been reached.
#endif

#ifndef SCARD_E_BAD_SEEK
#define SCARD_E_BAD_SEEK                            ((DWORD)0x80100029L)
//
// MessageId: SCARD_E_BAD_SEEK
//
// MessageText:
//
// There was an error trying to set the smart card file object pointer.
#endif

#ifndef SCARD_E_CERTIFICATE_UNAVAILABLE
#define SCARD_E_CERTIFICATE_UNAVAILABLE             ((DWORD)0x8010002DL)
//
// MessageId: SCARD_E_CERTIFICATE_UNAVAILABLE
//
// MessageText:
//
// The requested certificate could not be obtained.
#endif

#ifndef SCARD_E_COMM_DATA_LOST
#define SCARD_E_COMM_DATA_LOST                      ((DWORD)0x8010002FL)
//
// MessageId: SCARD_E_COMM_DATA_LOST
//
// MessageText:
//
// A communications error with the smart card has been detected.  Retry the operation.
#endif

#ifndef SCARD_E_DIR_NOT_FOUND
#define SCARD_E_DIR_NOT_FOUND                       ((DWORD)0x80100023L)
//
// MessageId: SCARD_E_DIR_NOT_FOUND
//
// MessageText:
//
// The identified directory does not exist in the smart card.
#endif

#ifndef SCARD_E_FILE_NOT_FOUND
#define SCARD_E_FILE_NOT_FOUND                      ((DWORD)0x80100024L)
//
// MessageId: SCARD_E_FILE_NOT_FOUND
//
// MessageText:
//
// The identified file does not exist in the smart card.
#endif

#ifndef SCARD_E_ICC_CREATEORDER
#define SCARD_E_ICC_CREATEORDER                     ((DWORD)0x80100021L)
//
// MessageId: SCARD_E_ICC_CREATEORDER
//
// MessageText:
//
// The requested order of object creation is not supported.
#endif

#ifndef SCARD_E_ICC_INSTALLATION
#define SCARD_E_ICC_INSTALLATION                    ((DWORD)0x80100020L)
//
// MessageId: SCARD_E_ICC_INSTALLATION
//
// MessageText:
//
// No Primary Provider can be found for the smart card.
#endif

#ifndef SCARD_E_INVALID_CHV
#define SCARD_E_INVALID_CHV                         ((DWORD)0x8010002AL)
//
// MessageId: SCARD_E_INVALID_CHV
//
// MessageText:
//
// The supplied PIN is incorrect.
#endif

#ifndef SCARD_E_NO_ACCESS
#define SCARD_E_NO_ACCESS                           ((DWORD)0x80100027L)
//
// MessageId: SCARD_E_NO_ACCESS
//
// MessageText:
//
// Access is denied to this file.
#endif

#ifndef SCARD_E_NO_DIR
#define SCARD_E_NO_DIR                              ((DWORD)0x80100025L)
//
// MessageId: SCARD_E_NO_DIR
//
// MessageText:
//
// The supplied path does not represent a smart card directory.
#endif

#ifndef SCARD_E_NO_FILE
#define SCARD_E_NO_FILE                             ((DWORD)0x80100026L)
//
// MessageId: SCARD_E_NO_FILE
//
// MessageText:
//
// The supplied path does not represent a smart card file.
#endif

#ifndef SCARD_E_NO_SUCH_CERTIFICATE
#define SCARD_E_NO_SUCH_CERTIFICATE                 ((DWORD)0x8010002CL)
//
// MessageId: SCARD_E_NO_SUCH_CERTIFICATE
//
// MessageText:
//
// The requested certificate does not exist.
#endif

#ifndef SCARD_E_UNEXPECTED
#define SCARD_E_UNEXPECTED                          ((DWORD)0x8010001FL)
//
// MessageId: SCARD_E_UNEXPECTED
//
// MessageText:
//
// An unexpected card error has occurred.
#endif

#ifndef SCARD_E_UNKNOWN_RES_MNG
#define SCARD_E_UNKNOWN_RES_MNG                     ((DWORD)0x8010002BL)
//
// MessageId: SCARD_E_UNKNOWN_RES_MNG
//
// MessageText:
//
// An unrecognized error code was returned from a layered component.
#endif

#ifndef SCARD_E_UNSUPPORTED_FEATURE
#define SCARD_E_UNSUPPORTED_FEATURE                 ((DWORD)0x80100022L)
//
// MessageId: SCARD_E_UNSUPPORTED_FEATURE
//
// MessageText:
//
// This smart card does not support the requested feature.
#endif

#ifndef SCARD_E_WRITE_TOO_MANY
#define SCARD_E_WRITE_TOO_MANY                      ((DWORD)0x80100028L)
//
// MessageId: SCARD_E_WRITE_TOO_MANY
//
// MessageText:
//
// The smartcard does not have enough memory to store the information.
#endif

#ifndef SCARD_W_CANCELLED_BY_USER
#define SCARD_W_CANCELLED_BY_USER                   ((DWORD)0x8010006EL)
//
// MessageId: SCARD_W_CANCELLED_BY_USER
//
// MessageText:
//
// The action was cancelled by the user.
#endif

#ifndef SCARD_W_EOF
#define SCARD_W_EOF                                 ((DWORD)0x8010006DL)
//
// MessageId: SCARD_W_EOF
//
// MessageText:
//
// The end of the smart card file has been reached.
#endif

#ifndef SCARD_W_SECURITY_VIOLATION
#define SCARD_W_SECURITY_VIOLATION                  ((DWORD)0x8010006AL)
//
// MessageId: SCARD_W_SECURITY_VIOLATION
//
// MessageText:
//
// Access was denied because of a security violation.
#endif

/**
 * @brief RKSignatureSmartCard::RKSignatureSmartCard
 * @param readerName
 */
RKSignatureSmartCard::RKSignatureSmartCard(QString readerName)
{
    // Establish the resource manager context
    long rv = SCardEstablishContext(
                SCARD_SCOPE_USER, // Scope of the resource manager context.
                NULL,             // r.f.u
                NULL,             // r.f.u
                &m_hContext);       // Returns the resource manager handle.
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
    }

    m_reader = readerName;
    m_hCard = 0;
}

/**
 * @brief RKSignatureSmartCard::~RKSignatureSmartCard
 */
RKSignatureSmartCard::~RKSignatureSmartCard()
{
    disconnect();
    if (m_hContext) {
        // Release the Resource Manager Context.
        long rv = SCardReleaseContext(m_hContext);
        m_hContext = 0;
        if (SCARD_S_SUCCESS != rv) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        }
    }
}

/**
 * @brief RKSignatureSmartCard::getMessage
 * @param id
 * @return
 */
QString RKSignatureSmartCard::getMessage(long id) {

    switch(id){

    case SCARD_F_INTERNAL_ERROR:
        return "An internal consistency check failed.";

    case SCARD_E_INVALID_HANDLE:
        return "The supplied handle was not valid.";

    case SCARD_E_INVALID_TARGET:
        return "Registry startup information is missing or not valid.";

    case SCARD_F_WAITED_TOO_LONG:
        return "An internal consistency timer has expired.";

    case SCARD_E_INSUFFICIENT_BUFFER:
        return "The data buffer for returned data is too small for the returned data.";

    case SCARD_E_TIMEOUT:
        return "The user-specified time-out value has expired.";

    case SCARD_E_SHARING_VIOLATION:
        return "The smart card cannot be accessed because of other outstanding connections.";

    case SCARD_E_NO_SMARTCARD:
        return "The operation requires a smart card, but no smart card is currently in the device.";

    case SCARD_E_UNKNOWN_CARD:
        return "The specified smart card name is not recognized.";

    case SCARD_E_PROTO_MISMATCH:
        return "The requested protocols are incompatible with the protocol currently in use with the card.";

    case SCARD_E_NOT_READY:
        return "The reader or card is not ready to accept commands.";

    case SCARD_E_SYSTEM_CANCELLED:
        return "The action was canceled by the system, presumably to log off or shut down.";

    case SCARD_F_COMM_ERROR:
        return "An internal communications error has been detected.";

    case SCARD_F_UNKNOWN_ERROR:
        return "An internal error has been detected, but the source is unknown.";

    case SCARD_E_INVALID_ATR:
        return "An ATR string obtained from the registry is not a valid ATR string.";

    case SCARD_E_READER_UNAVAILABLE:
        return "The specified reader is not currently available for use.";

    case SCARD_P_SHUTDOWN:
        return "The operation has been aborted to allow the server application to exit.";

    case SCARD_E_PCI_TOO_SMALL:
        return "The PCI receive buffer was too small.";

    case SCARD_E_READER_UNSUPPORTED:
        return "The reader driver does not meet minimal requirements for support.";

    case SCARD_E_CARD_UNSUPPORTED:
        return "The smart card does not meet minimal requirements for support.";

    case SCARD_E_SERVICE_STOPPED:
        return "The smart card resource manager has shut down.";

    case SCARD_W_UNSUPPORTED_CARD:
        return "The reader cannot communicate with the card, due to ATR string configuration conflicts.";

    case SCARD_W_UNRESPONSIVE_CARD:
        return "The smart card is not responding to a reset.";

    case SCARD_W_UNPOWERED_CARD:
        return "Power has been removed from the smart card, so that further communication is not possible.";

    case SCARD_W_RESET_CARD:
        return "The smart card was reset.";

    case SCARD_W_REMOVED_CARD:
        return "The smart card has been removed, so further communication is not possible.";

    case SCARD_E_NO_READERS_AVAILABLE:
        return "Cannot find a smart card reader.";

    case SCARD_E_NO_RESRC_MNGR_RUNNING:
        return "The Smart card resource manager is not running.";

        /*		case ERROR_BROKEN_PIPE:
            return "The client attempted a smart card operation in a remote session, such as a client session running on a terminal server, and the operating system in use does not support smart card redirection.";
*/
    case SCARD_E_BAD_SEEK:
        return	"An error occurred in setting the smart card file object pointer.";

    case SCARD_E_CANCELLED:
        return "The action was canceled by an SCardCancel request.";

    case SCARD_E_CANT_DISPOSE:
        return "The system could not dispose of the media in the requested manner.";

    case SCARD_E_CERTIFICATE_UNAVAILABLE:
        return "The requested certificate could not be obtained.";

    case SCARD_E_COMM_DATA_LOST:
        return "A communications error with the smart card has been detected.";

    case SCARD_E_DIR_NOT_FOUND:
        return "The specified directory does not exist in the smart card.";

    case SCARD_E_DUPLICATE_READER:
        return "The reader driver did not produce a unique reader name.";

    case SCARD_E_FILE_NOT_FOUND:
        return "The specified file does not exist in the smart card.";

    case SCARD_E_ICC_CREATEORDER:
        return "The requested order of object creation is not supported.";

    case SCARD_E_ICC_INSTALLATION:
        return "No primary provider can be found for the smart card.";

    case SCARD_E_INVALID_CHV:
        return "The supplied PIN is incorrect.";

    case SCARD_E_INVALID_PARAMETER:
        return "One or more of the supplied parameters could not be properly interpreted.";

    case SCARD_E_INVALID_VALUE:
        return "One or more of the supplied parameter values could not be properly interpreted.";

    case SCARD_E_NO_ACCESS:
        return "Access is denied to the file.";

    case SCARD_E_NO_DIR:
        return "The supplied path does not represent a smart card directory.";

    case SCARD_E_NO_FILE:
        return "The supplied path does not represent a smart card file.";

    case SCARD_E_NO_MEMORY:
        return "Not enough memory available to complete this command.";

    case SCARD_E_NO_SUCH_CERTIFICATE:
        return "The requested certificate does not exist.";

    case SCARD_E_NOT_TRANSACTED:
        return "An attempt was made to end a nonexistent transaction.";

        /*		case SCARD_E_UNEXPECTED:
            return "An unexpected card error has occurred.";
*/
    case SCARD_E_UNKNOWN_READER:
        return "The specified reader name is not recognized.";

    case SCARD_E_UNKNOWN_RES_MNG:
        return "An unrecognized error code was returned.";

    case SCARD_E_UNSUPPORTED_FEATURE:
        return "This smart card does not support the requested feature.";

    case SCARD_E_WRITE_TOO_MANY:
        return "An attempt was made to write more data than would fit in the target object.";

    case SCARD_S_SUCCESS:
        return "No error was encountered.";

    case SCARD_W_CANCELLED_BY_USER:
        return "The action was canceled by the user.";

    case SCARD_W_CHV_BLOCKED:
        return "The card cannot be accessed because the maximum number of PIN entry attempts has been reached.";

    case SCARD_W_EOF:
        return "The end of the smart card file has been reached.";

    case SCARD_W_SECURITY_VIOLATION:
        return "Access was denied because of a security violation.";

    case SCARD_W_WRONG_CHV:
        return "The card cannot be accessed because the wrong PIN was presented.";

    default:
        return "Unknown error.";

    }
}

/**
 * @brief RKSignatureSmartCard::getReaders
 * @param list
 */
void RKSignatureSmartCard::getReaders(QStringList *list)
{
    SCARDCONTEXT scContext; // Resource manager handle
    long rv = SCardEstablishContext(
                SCARD_SCOPE_USER, // Scope of the resource manager context.
                NULL,             // r.f.u
                NULL,             // r.f.u
                &scContext);      // Returns the resource manager handle.
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return;
    }
    // The SCardListReaders function provides the list of readers
    LPTSTR mszReaders = NULL;

#ifdef SCARD_AUTOALLOCATE
    unsigned long cch = SCARD_AUTOALLOCATE;
    rv = SCardListReaders(
                scContext,          // Resource manager handle.
                NULL,               // NULL: list all readers in the system
                (LPTSTR)&mszReaders,         // Returs the card readers list.
                &cch);
#else
    DWORD cch;
    rv = SCardListReaders(
                scContext,
                NULL,
                NULL,
                &cch);

    mszReaders = (LPTSTR) calloc(cch, sizeof(char));
    rv = SCardListReaders(
                scContext,
                NULL,
                mszReaders,
                &cch);

#endif
    if (SCARD_E_NO_READERS_AVAILABLE != rv) {
        if (SCARD_S_SUCCESS != rv) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
            return;
        }
    } else {
        return;
    }

    // Extract the reader strings form the null separated string and
    // get the total number of readers.
    LPTSTR pReader = mszReaders;
    while (*pReader != '\0')
    {
#ifdef WIN32
        list->append(QString::fromWCharArray(pReader));
        pReader += wcslen((wchar_t *)pReader) + 1;
#else
        list->append(QString::fromLocal8Bit(pReader));
        pReader += strlen(pReader)+1;
#endif
    }

#ifdef SCARD_AUTOALLOCATE
    // Releases memory that has been returned from the resource manager
    // using the SCARD_AUTOALLOCATE length designator.
    rv = SCardFreeMemory(scContext, mszReaders);
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);

    }
#else
    free(mszReaders);
#endif

    // Release the Resource Manager Context.
    rv = SCardReleaseContext(scContext);
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);

    }
}

/**
 * @brief RKSignatureSmartCard::isCardPresent
 * @return
 */
bool RKSignatureSmartCard::isCardPresent()
{
    SCARD_READERSTATE sReaderState;

    sReaderState.szReader = (LPTSTR) getReader(m_reader);
    sReaderState.dwCurrentState = SCARD_STATE_UNAWARE;
    sReaderState.dwEventState = SCARD_STATE_UNAWARE;

    // The SCardGetStatusChange function blocks execution until the current
    // availability of the cards in a specific set of readers changes.
    long rv = SCardGetStatusChange(
                m_hContext,      // Resource manager handle.
                30,            // Max. amount of time (in milliseconds) to wait for an action.
                &sReaderState, // Reader state
                1);            // Number of readers
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return false;
    }

    // Check if card is already present
    return ((sReaderState.dwEventState & SCARD_STATE_PRESENT) == SCARD_STATE_PRESENT);
}

/**
 * @brief RKSignatureSmartCard::getReader
 * @param readerName
 * @return
 */
LPTSTR RKSignatureSmartCard::getReader(QString readerName)
{
#ifdef WIN32
    return (LPTSTR)readerName.utf16();
#else
    return (LPTSTR)readerName.toLocal8Bit().data();
#endif
}

/**
 * @brief RKSignatureSmartCard::connect
 * @return
 */
bool RKSignatureSmartCard::connect()
{
    if (!m_hCard) {
        // Establishes a connection to a smart card contained by a specific reader.
        long rv = SCardConnect(
                    m_hContext,              // Resource manager handle.
                    getReader(m_reader),            // Reader name.
                    SCARD_SHARE_EXCLUSIVE, // Share Mode.
                    SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,     // Preferred protocols (T=0 or T=1).
                    &m_hCard,                // Returns the card handle.
                    &m_activeProtocol);      // Active protocol.
        if (SCARD_S_SUCCESS != rv) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
            return false;
        }

        switch (m_activeProtocol)
        {
        case SCARD_PROTOCOL_T0:
            break;

        case SCARD_PROTOCOL_T1:
            break;

        case SCARD_PROTOCOL_UNDEFINED:
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(SCARD_PROTOCOL_UNDEFINED);
            return false;
        }
    }

    return true;
}

/**
 * @brief RKSignatureSmartCard::disconnect
 * @return
 */
bool RKSignatureSmartCard::disconnect(){
    if (m_hCard) {
        // Terminates the smart card connection.^M
        long rv = SCardDisconnect(m_hCard, SCARD_UNPOWER_CARD);
        if (SCARD_S_SUCCESS != rv) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
            return false;
        }
        m_hCard = 0;
    }
    return true;
}

/**
 * @brief RKSignatureSmartCard::getATR
 * @param atr
 * @param length
 * @return
 */
bool RKSignatureSmartCard::getATR(unsigned char atr[33], DWORD &length)
{
#ifdef WIN32
    WCHAR szReader[200];
#else
    char szReader[200];
#endif

    DWORD cch = 200;
    DWORD dwState, dwProtocol;
    unsigned char pbAtr[33];
    length = 33;

    // Determine the status.
    // hCardHandle was set by an earlier call to SCardConnect.
    long rv = SCardStatus(
                m_hCard,
                szReader,
                &cch,
                &dwState,
                &dwProtocol,
                pbAtr,
                &length
                );
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
    }

    for (unsigned long i = 0; i < length; i++) {
        atr[i] = pbAtr[i];
    }

    return true;
}

/**
 * @brief RKSignatureSmartCard::getAtrString
 * @param atr
 * @param atrLen
 * @return
 */
bool RKSignatureSmartCard::getAtrString(unsigned char * atr, DWORD * atrLen)
{
    unsigned char * pbAttr = NULL;

#ifdef SCARD_AUTOALLOCATE
    unsigned long cByte = SCARD_AUTOALLOCATE;

    // Gets the current reader attributes for the given handle.
    long rv = SCardGetAttrib(
                m_hCard,                    // Card handle.
                SCARD_ATTR_ATR_STRING,    // Attribute identifier.
                (unsigned char *)&pbAttr, // Attribute buffer.
                &cByte);                  // Returned attribute length.
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return false;
    }

#else
    DWORD cByte;

    long rv = SCardGetAttrib(m_hCard, SCARD_ATTR_ATR_STRING, NULL, &cByte);
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return false;
    }

    pbAttr = (unsigned char*) malloc(cByte);
    rv = SCardGetAttrib(m_hCard, SCARD_ATTR_ATR_STRING, pbAttr, &cByte);
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return false;
    }
#endif

    if (atr != NULL) {
        for (unsigned long i = 0; i < cByte; i++) {
            atr[i] = pbAttr[i];
        }
        *atrLen = cByte;
    }

#ifdef SCARD_AUTOALLOCATE
    // Releases memory that has been returned from the resource manager
    // using the SCARD_AUTOALLOCATE length designator.
    rv = SCardFreeMemory(m_hContext, pbAttr);
    if (SCARD_S_SUCCESS != rv) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return false;
    }
#else
    free(pbAttr);
#endif

    return true;
}

/**
 * @brief RKSignatureSmartCard::transmit
 * @param txBuffer
 * @param txLength
 * @param rxBuffer
 * @param rxLength
 * @return
 */
bool RKSignatureSmartCard::transmit(const unsigned char *txBuffer, unsigned long txLength, unsigned char *rxBuffer, DWORD *rxLength)
{
    LPCSCARD_IO_REQUEST  ioRequest;
    switch (m_activeProtocol)
    {
    case SCARD_PROTOCOL_T0:
        ioRequest = SCARD_PCI_T0;
        break;

    case SCARD_PROTOCOL_T1:
        ioRequest = SCARD_PCI_T1;
        break;

    default:
        ioRequest = SCARD_PCI_RAW;
        break;
    }

    *rxLength = MAX_APDU_BUFFER_SIZE;

    long rv = SCardTransmit(
                m_hCard,     // Card handle.
                ioRequest, // Pointer to the send protocol header.
                txBuffer,  // Send buffer.
                txLength,  // Send buffer length.
                NULL,      // Pointer to the rec. protocol header.
                rxBuffer,  // Receive buffer.
                rxLength); // Receive buffer length.
    if (SCARD_S_SUCCESS != rv) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << getMessage(rv);
        return false;
    }

    return true;
}

/**
 * @brief RKSignatureSmartCard::selectApplication
 * @return
 */
bool RKSignatureSmartCard::selectApplication()
{

    if (!isCardPresent()) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << "There is no Smart card in the reader";
        return false;
    }
    if (!connect()) {
        qWarning() << "There is a Smart card in the reader but the connection failed";
        return false;
    }
    // There is a Smart card in the reader and connected successful
    return true;
}
