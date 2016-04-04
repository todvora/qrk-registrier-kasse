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
 */

#include "aesutil.h"
#include "base32decode.h"
#include "base32encode.h"

#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>

using namespace std;
using namespace CryptoPP;

///
/// \brief AESUtil::getPrivateKey
/// \return
///
QString AESUtil::getPrivateKey()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT value, strValue FROM globals WHERE name='privateKey'");
  query.exec();
  if (query.next()) {
    int val = query.value(0).toInt();
    /* increment manual for keyversions check */
    if (val == 1)
      return query.value(1).toString();

    /* remove this in future before 2017 */
    query.prepare("DELETE from globals WHERE name='privateKey'");
    query.exec();
  }
  QString key = generateKey();
  query.prepare(QString("INSERT INTO globals (name, value, strValue) VALUES('privateKey', 1, '%1')").arg(key));

  query.exec();

  return key;

}

///
/// \brief AESUtil::encryptTurnoverCounter
/// \param concatenated
/// \param turnoverCounter
/// \param symmetricKey
/// \return
///
QString AESUtil::encryptTurnoverCounter( QString concatenated, qlonglong turnoverCounter, QString symmetricKey)
{
  QString hashValue = HashValue(concatenated);
  return encryptCTR(hashValue.toStdString(), turnoverCounter, symmetricKey.toStdString());
}

///
/// \brief AESUtil::decryptTurnoverCounter
/// \param concatenated
/// \param encodedTurnoverCounter
/// \param symmetricKey
/// \return
///
QString AESUtil::decryptTurnoverCounter( QString concatenated, QString encodedTurnoverCounter, QString symmetricKey)
{
  QString hashValue = HashValue(concatenated);
  return decryptCTR(hashValue.toStdString(), encodedTurnoverCounter, symmetricKey.toStdString());
}

///
/// \brief AESUtil::sigLastReceipt
/// \param value
/// \return
///
QString AESUtil::sigReceipt(QString value)
{
  QString hashValue = HashValue(value);
  QString sig = base64_encode(hashValue);

  return sig;
}

///
/// \brief AESUtil::generateKey
/// \return
///
QString AESUtil::generateKey()
{

  AutoSeededRandomPool prng;

  /* generate key for encrypt */
  unsigned char key[AES::MAX_KEYLENGTH];
  prng.GenerateBlock(key, sizeof(key));

  string encoded;
  encoded.clear();

  StringSource ssk(key, sizeof(key), true, new HexEncoder( new StringSink(encoded)));

  return QString::fromStdString(encoded);
}

// private
///
/// \brief AESUtil::HashValue
/// \param value
/// \return
///
QString AESUtil::HashValue(QString value)
{
  string source = value.toStdString();
  byte hashValue[AES::MAX_KEYLENGTH];
  SHA256 hash;

  memset( hashValue, 0x00, AES::MAX_KEYLENGTH );

  StringSource ss( source, true, new HashFilter( hash, new HexEncoder(new ArraySink( hashValue, sizeof(hashValue)))));
  return QString::fromStdString(std::string(reinterpret_cast<char const*>(hashValue), sizeof(hashValue)));
}

///
/// \brief AESUtil::encryptCTR
/// \param concatenatedHashValue
/// \param turnoverCounter
/// \param symmetricKey
/// \return
///
QString AESUtil::encryptCTR(std::string concatenatedHashValue, qlonglong turnoverCounter, std::string symmetricKey)
{
  byte data[AES::BLOCKSIZE];
  memset( data, 0x00, AES::BLOCKSIZE );

  union {
      qlonglong source;
      byte data[8];
  } Union;

  Union.source = turnoverCounter;

  for (int i = 0; i < 8; i++)
    data[7 - i] = Union.data[i];

  byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
  StringSource ssk(symmetricKey , true, new HexDecoder( new ArraySink(key, AES::MAX_KEYLENGTH)));
  StringSource ssv(concatenatedHashValue, true, new HexDecoder( new ArraySink(iv, AES::BLOCKSIZE)));

  byte out[AES::BLOCKSIZE/2];
  memset( out, 0x00, AES::BLOCKSIZE/2 );

  try {
    CTR_Mode<AES>::Encryption encryption(key,sizeof(key),iv, sizeof(iv));
    encryption.ProcessData(out,data,sizeof(data));
  } catch (Exception &e) {
    qDebug() << "Error: " << e.what();
  }

  QByteArray ba;
  ba.append(reinterpret_cast<const char *>(out), sizeof(out));
  return ba.toBase64();
}

///
/// \brief AESUtil::decryptCTR
/// \param concatenatedHashValue
/// \param encryptedTurnoverCounter
/// \param symmetricKey
/// \return
///
QString AESUtil::decryptCTR(std::string concatenatedHashValue, QString encryptedTurnoverCounter, std::string symmetricKey)
{
  QByteArray baBase64;
  baBase64.append(encryptedTurnoverCounter);
  string encryptedTurnoverCounterHex = QByteArray::fromBase64(baBase64).toHex().toStdString();

  byte data[AES::BLOCKSIZE];
  memset( data, 0x00, AES::BLOCKSIZE );

  byte recovered[AES::BLOCKSIZE/2];
  memset( recovered, 0x00, AES::BLOCKSIZE/2 );

  byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
  StringSource ssk(symmetricKey , true, new HexDecoder( new ArraySink(key, AES::MAX_KEYLENGTH)));
  StringSource ssv(concatenatedHashValue, true, new HexDecoder( new ArraySink(iv, AES::BLOCKSIZE)));
  StringSource ssd(encryptedTurnoverCounterHex, true, new HexDecoder( new ArraySink(data, AES::BLOCKSIZE)));

  try {
    CTR_Mode<AES>::Decryption decryption(key,sizeof(key),iv, sizeof(iv));
    decryption.ProcessData(recovered,data,AES::BLOCKSIZE);
  } catch (Exception &e) {
    qDebug() << "Error: " << e.what();
  }

  /* we need only 8 Bytes for Turnover minimum is 5 Byte */
  union {
      qlonglong source;
      byte data[AES::BLOCKSIZE/2];
  } Union;
  memset( Union.data, 0x00, AES::BLOCKSIZE/2 );

  /* reconstruct reverse */
  for (int i=0; i < 8;i++)
    Union.data[i] = recovered[7 - i];

  return QString::number(Union.source);
}

///
/// \brief AESUtil::toHex
/// \param ba
/// \return
///
QString AESUtil::toHex(QByteArray ba)
{
  string encoded;
  StringSource ss(ba.toStdString(), true, new HexEncoder( new StringSink(encoded)));

  return QString::fromStdString(encoded);
}

///
/// \brief AESUtil::base32_encode
/// \param str
/// \return
///
QByteArray AESUtil::base32_encode(QString str)
{
  string decoded = str.toUtf8().toStdString();

  size_t req = Base32Encode::GetLength(decoded.size());
  char encoded[req];

  memset( encoded, 0x00, req);
  size_t size = Base32Encode::Encode(encoded, decoded.data(),decoded.size());

  return  QByteArray::fromStdString(std::string(encoded, size)).simplified();
}

///
/// \brief AESUtil::base64_encode
/// \param str
/// \return
///
QByteArray AESUtil::base64_encode(QString str)
{
  string decoded = str.toStdString();
  string encoded;

  StringSource ss(decoded, true, new Base64Encoder(new StringSink(encoded)));

  return QByteArray::fromStdString(encoded).simplified();
}

QByteArray AESUtil::base64Url_encode(QString str)
{
  string decoded = str.toStdString();
  string encoded;

  // StringSource ss(decoded, true, new Base64URLEncoder(new StringSink(encoded)));

  return QByteArray::fromStdString(encoded).simplified();
}

///
/// \brief AESUtil::base32_decode
/// \param str
/// \return
///
QByteArray AESUtil::base32_decode(QString str)
{

  string encoded = str.toUtf8().toStdString();

  size_t req = Base32Decode::GetLength(encoded.size());
  char decoded[req];

  memset( decoded, 0x00, req);
  size_t size = Base32Decode::Decode(decoded, encoded.data(),encoded.size());

  return  QByteArray::fromStdString(std::string(decoded, size));

}

///
/// \brief AESUtil::base64_decode
/// \param str
/// \param hex
/// \return
///
QByteArray AESUtil::base64_decode(QString str, bool hex)
{
  QByteArray ba;
  ba.clear();
  ba.append(str);
  if (hex)
    return QByteArray::fromBase64(ba).toHex();

  return QByteArray::fromBase64(ba);
}
