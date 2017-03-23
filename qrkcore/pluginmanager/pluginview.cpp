#include "pluginview.h"
#include "pluginmanager/treemodel.h"
#include "ui_pluginview.h"

PluginView::PluginView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginView)
{
    ui->setupUi(this);

    m_model = new TreeModel(this);
    ui->treeView->setModel(m_model);
    ui->treeView->setWindowTitle(tr("Plugins"));
}

PluginView::~PluginView()
{
    delete ui;
}
