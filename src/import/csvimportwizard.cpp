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

#include "csvimportwizard.h"
#include "csvimportwizardpage1.h"
#include "csvimportwizardpage2.h"
#include "csvimportwizardpage3.h"

CsvImportWizard::CsvImportWizard(QWidget *parent) :
    QWizard(parent)
{

    m_model = new QStandardItemModel();
    m_assignmentModel = new QStandardItemModel();
    m_map = new QMap<QString, QVariant>;

    m_pageImport = new CsvImportWizardPage1(this);
    m_pageAssign = new CsvImportWizardPage2(this);
    m_pageSave = new CsvImportWizardPage3(this);

    m_pageImport->setModel(m_model);
    m_pageAssign->setModel(m_model);
    m_pageAssign->setMap(m_map);
    m_pageAssign->setAssignmentModel(m_assignmentModel);
    m_pageSave->setModel(m_model);
    m_pageSave->setMap(m_map);

    addPage(m_pageImport);
    addPage(m_pageAssign);
    addPage(m_pageSave);

}
