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

#ifndef DEFINES
#define DEFINES

#define MIN_HEIGHT 60
#define QRK_VERSION_MAJOR "0.16"
#define QRK_VERSION_MINOR "0502"

enum REGISTER_COL
{
  REGISTER_COL_COUNT,
  REGISTER_COL_PRODUCT,
  REGISTER_COL_TAX,
  REGISTER_COL_NET,
  REGISTER_COL_SINGLE,
  REGISTER_COL_TOTAL,
  REGISTER_COL_SAVE
};

enum DOCUMENT_COL
{
  DOCUMENT_COL_RECEIPT,
  DOCUMENT_COL_TYPE,
  DOCUMENT_COL_TOTAL,
  DOCUMENT_COL_DATE
};

// values for the receipt.payedBy field
enum PAYED_BY
{
  PAYED_BY_CASH,
  PAYED_BY_DEBITCARD,
  PAYED_BY_CREDITCARD,
  PAYED_BY_REPORT_EOD,
  PAYED_BY_REPORT_EOM
};

enum DEP_ACTION
{
  DEP_RECEIPT = 0
};

#endif // DEFINES

