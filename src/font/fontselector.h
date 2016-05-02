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

#ifndef FONTSELECTOR_H
#define FONTSELECTOR_H

#include <QDialog>

namespace Ui {
  class FontSelector;
}

class FontSelector : public QDialog
{
    Q_OBJECT

  public:
    explicit FontSelector(QWidget *parent = 0);
    ~FontSelector();
    QFont getFont();

  private slots:
    void on_pointSizeSlider_valueChanged(int value);
    void on_pointSizeSpinBox_valueChanged(double value);
    void on_stretchSpinBox_valueChanged(int value);
    void on_stretchSlider_valueChanged(int value);

    void on_weight_check_checked(bool checked);
    void on_fontComboBox_currentFontChanged(const QFont &f);
    virtual void accept(bool);

  private:
    void update();

  private:
    Ui::FontSelector *ui;

};

#endif // FONTSELECTOR_H
