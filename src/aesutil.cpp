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

#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <crypto++/aes.h>
#include <crypto++/modes.h>
//#include <crypto++/filters.h>
#include <crypto++/hex.h>
//#include <crypto++/sha.h>
#include <crypto++/base64.h>
#include <crypto++/base32.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>

using namespace std;
using namespace CryptoPP;
using Name::Pad;
using Name::InsertLineBreaks;

AESUtil::AESUtil()
{

}

///
/// \brief AESUtil::HashValue
/// \param value
/// \param hashValueLen
/// \return
///
std::string AESUtil::HashValue(QString value, int hashValueLen)
{
  string source = value.toStdString();
  byte hashValue[ hashValueLen ];
  SHA256 hash;

  memset( hashValue, 0x00, hashValueLen );

  StringSource ( source, true, new HashFilter( hash, new HexEncoder( new ArraySink( hashValue, hashValueLen ))));

  return std::string(reinterpret_cast<char const*>(hashValue), hashValueLen);
}

///
/// \brief AESUtil::Base64Encode
/// \param decoded
/// \return
///
std::string AESUtil::Base64Encode(std::string decoded)
{

  std::string encoded;
  StringSource (decoded, true, new Base64Encoder( new StringSink(encoded) ));

  return encoded;
}

///
/// \brief AESUtil::Base64Decode
/// \param raw
/// \return
///
std::string AESUtil::Base64Decode(std::string raw)
{
  string encoded, decoded;
  Base64Encoder encoder;

  AlgorithmParameters params = MakeParameters(Pad(), false)(InsertLineBreaks(), false);
  encoder.IsolatedInitialize(params);
  encoder.Attach(new StringSink( encoded ));

  StringSource (raw, true, new Redirector(encoder));
  StringSource (encoded, true, new Base64Decoder(new StringSink(decoded)));

  return decoded;
}

///
/// \brief AESUtil::encryptCTR
/// \param concatenatedHashValue
/// \param in
/// \param symmetricKey
/// \return
///
QString AESUtil::encryptCTR(std::string concatenatedHashValue, std::string in, std::string symmetricKey)
{
  string out, encoded;

  HexDecoder decoder;
  decoder.Put( (byte *)symmetricKey.data(), symmetricKey.size() );
  decoder.MessageEnd();
  word64 keysize = decoder.MaxRetrievable();
  char *decodedKey = new char[keysize];
  decoder.Get((byte *)decodedKey, keysize);

  byte key[ AES::MAX_KEYLENGTH ];
  byte iv[ AES::BLOCKSIZE ];
  StringSource( reinterpret_cast<const char *>(decodedKey), true,
                new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)) );

  memset( iv, 0x00, AES::BLOCKSIZE );
  try {
    CTR_Mode<AES>::Encryption Encryptor(key,sizeof(key),(byte*)concatenatedHashValue.data());
    StringSource(in, true, new StreamTransformationFilter(Encryptor, new StringSink(out)));
    StringSource(out, true, new HexEncoder( new StringSink(encoded)));

  } catch (Exception &e) {
      cout << e.what() << endl;
  } catch (...) {
  }

  return QString::fromStdString(encoded);
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
  string out, in, decoded;
  in = encryptedTurnoverCounter.toStdString();

  HexDecoder decoder;
  decoder.Put( (byte *)symmetricKey.data(), symmetricKey.size() );
  decoder.MessageEnd();
  word64 keysize = decoder.MaxRetrievable();
  char *decodedKey = new char[keysize];
  decoder.Get((byte *)decodedKey, keysize);

  byte key[ AES::MAX_KEYLENGTH ];
  byte iv[ AES::BLOCKSIZE ];
  StringSource( reinterpret_cast<const char *>(decodedKey), true,
                new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)) );

  memset( iv, 0x00, AES::BLOCKSIZE );
  try {
    CTR_Mode<AES>::Decryption Decryptor(key,sizeof(key),(byte*)concatenatedHashValue.data());
    StringSource(in, true, new HexDecoder( new StringSink(out)));
    StringSource (out, true, new StreamTransformationFilter(Decryptor, new StringSink(decoded)));
  }
  catch (Exception &e) {
    cout << e.what() << endl;
  }
  catch (...) {
  }

  return QString::fromStdString(decoded);
}

///
/// \brief AESUtil::sigLastReceipt
/// \param value
/// \return
///
QString AESUtil::sigLastReceipt(QString value)
{
  string hashValue = HashValue(value, 8);
  string sig = Base64Encode(hashValue);

  return QString::fromStdString(sig);
}

///
/// \brief AESUtil::encrypt
/// \param concatenated
/// \param turnoverValue
/// \param key
/// \return
///
QString AESUtil::encrypt( QString concatenated, double turnoverValue, QString key)
{
  long turnover = turnoverValue * 100;
  string turnoverCounter = QString("%1").arg(turnover, 5, 10, QChar('0')).toStdString();
  string symmetricKey = key.toStdString();
  string hashValue = HashValue(concatenated);
  string iv = Base64Encode(hashValue);

  return encryptCTR(iv, turnoverCounter, symmetricKey);

}

///
/// \brief AESUtil::decrypt
/// \param concatenated
/// \param encodedTurnoverValue
/// \param key
/// \return
///
QString AESUtil::decrypt( QString concatenated, QString encodedTurnoverValue, QString key)
{

  string symmetricKey = key.toStdString();
  string concatenatedHashValue = HashValue(concatenated);
  string iv = Base64Encode(concatenatedHashValue);

  return decryptCTR(iv, encodedTurnoverValue, symmetricKey);

}

///
/// \brief AESUtil::getPrivateKey
/// \return
///
QString AESUtil::getPrivateKey()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT strValue FROM globals WHERE name='privateKey'");
  query.exec();
  if (query.next())
    return query.value(0).toString();

  QString key = generateKey();
  query.prepare(QString("INSERT INTO globals (name, strValue) VALUES('privateKey', '%1')").arg(key));

  query.exec();

  return key;

}

///
/// \brief AESUtil::generateKey
/// \return
///
QString AESUtil::generateKey()
{
  // Generate keys
  AutoSeededRandomPool rng;

  InvertibleRSAFunction params;
  params.GenerateRandomWithKeySize(rng, 1024);

  RSA::PrivateKey privateKey(params);

  ByteQueue queue;
  privateKey.DEREncodePublicKey(queue);

  std::string key;
  CryptoPP::StringSink ss1(key);
  queue.TransferTo(ss1);

  QByteArray ba;
  ba.append(key.data(), key.size());

  return QString::fromStdString(ba.toHex().toStdString());

}

///
/// \brief AESUtil::test
///
void AESUtil::test()
{
  QString source("CK-124");
  string hashValue = HashValue(source);
  string iv = Base64Encode(hashValue);
  string decoded = Base64Decode(hashValue);

  string turnoverCounter = "00123";
  string symmetricKey = "729308A8E815F6A46EB3A8AE6D5463CA7B64A0E2E11BC26A68106FC7697E727E37011";

  QString aes256encrypted = encryptCTR(iv, turnoverCounter, symmetricKey);
  QString aes256decrypted = decryptCTR(iv, aes256encrypted, symmetricKey);

  cout << source.toStdString() << endl;
  cout << hashValue << endl;
  cout << iv << endl;
  cout << decoded << endl;
  cout << aes256encrypted.toStdString() << endl;
  cout << aes256decrypted.toStdString() << endl;

}
