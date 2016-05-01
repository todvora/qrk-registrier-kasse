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

#include "qrkdialog.h"

#include <QLineEdit>
#include <QValidator>

QRKDialog::QRKDialog(QWidget* parent)
    : QDialog(parent)
{

}

void QRKDialog::registerMandatoryField(QLineEdit* le, const QString& regexp)
{
    if (!le || _mandatoryFields.contains(le))
        return;

    if (regexp != "")
    {
        le->setValidator(new QRegExpValidator(QRegExp(regexp), le));
    }

    _mandatoryFields.append(le);

    QObject::connect(le, SIGNAL(textChanged(const QString&)), this, SLOT(checkLineEdits()));
}

void QRKDialog::unregisterMandatoryField(QLineEdit* le)
{
    if (!le || !_mandatoryFields.contains(le))
        return;

    QObject::disconnect(le, 0, 0, 0);

    _mandatoryFields.removeAll(le);
}

void QRKDialog::checkLineEdits()
{
    bool b = true;

    for (QList<QLineEdit*>::iterator iter = _mandatoryFields.begin(); iter != _mandatoryFields.end(); ++iter)
    {
        QLineEdit* le = *iter;

        if (le == NULL)
            continue;

        if (le->text().isEmpty() || !le->hasAcceptableInput())
        {
            b = false;
            break;
        }
    }

    emit hasAcceptableInput(b);
}
