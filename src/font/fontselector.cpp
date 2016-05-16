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

#include "fontselector.h"
#include "ui_fontselector.h"

FontSelector::FontSelector(QFont font, QWidget *parent) :
  QDialog(parent), ui(new Ui::FontSelector), font(&font)
{
  ui->setupUi(this);

  connect(ui->OKButton, SIGNAL(clicked(bool)),this,SLOT(accept()));
  connect(ui->CancelButton, SIGNAL(clicked(bool)),this,SLOT(close()));

  ui->fontComboBox->setCurrentFont(font);
  ui->fontComboBox->setCurrentText(font.family());
  ui->pointSizeSpinBox->setValue(font.pointSize());
  ui->stretchSpinBox->setValue(font.stretch());
  ui->weightLabel->setHidden(true);
  ui->weight_check->setHidden(true);

}

FontSelector::~FontSelector()
{
  delete ui;
}

void FontSelector::on_pointSizeSlider_valueChanged(int v)
{
  ui->pointSizeSpinBox->setValue(v / 100.0);
  update();
}

void FontSelector::on_pointSizeSpinBox_valueChanged(double v)
{
  QFont f = ui->sampleText->font();
  f.setPointSizeF(v);
  ui->sampleText->setFont(f);
  ui->pointSizeSlider->setValue((int)(v * 100));
  update();
}

void FontSelector::on_stretchSpinBox_valueChanged(int v)
{
  QFont f = ui->sampleText->font();
  f.setStretch(v);
  ui->sampleText->setFont(f);
  update();
}

void FontSelector::on_stretchSlider_valueChanged(int v)
{
  ui->stretchSpinBox->setValue(v);
  update();
}

void FontSelector::on_weight_check_checked(bool checked)
{
  QFont f = ui->sampleText->font();
  f.setBold(checked);
  ui->sampleText->setFont(f);
  update();
}

void FontSelector::on_fontComboBox_currentFontChanged(const QFont &f)
{
  ui->sampleText->setFont(f);
  update();
  ui->pointSizeSlider->setValue(f.pointSize());
  ui->pointSizeSpinBox->setValue(f.pointSizeF());
}

void FontSelector::update()
{
  const QFont &f = ui->sampleText->font();
  ui->stretchSpinBox->setValue(f.stretch());
  ui->stretchSlider->setValue(f.stretch());
  ui->weight_check->setChecked(f.bold());
}

void FontSelector::accept(bool)
{
  QDialog::accept();
}

QFont FontSelector::getFont()
{
  return ui->sampleText->font();
}
