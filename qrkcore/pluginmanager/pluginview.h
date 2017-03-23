#ifndef PLUGINVIEW_H
#define PLUGINVIEW_H

#include "qrkcore_global.h"

#include <QDialog>

namespace Ui {
class QRK_EXPORT PluginView;
}

class TreeModel;
class QRK_EXPORT PluginView : public QDialog
{
    Q_OBJECT

public:
    explicit PluginView(QWidget *parent = 0);
    ~PluginView();

private:
    Ui::PluginView *ui;
    TreeModel *m_model;
};

#endif // PLUGINVIEW_H
