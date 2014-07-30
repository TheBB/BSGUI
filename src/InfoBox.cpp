#include <QTabWidget>

#include "InfoBox.h"

InfoBox::InfoBox(ObjectSet *objectSet,
                 const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    installEventFilter(parent);

    QTabWidget *tabs = new QTabWidget();
    tabs->addTab(new QWidget(), "Selection");
    tabs->addTab(new QWidget(), "Pick");
    tabs->addTab(new QWidget(), "Log");

    setWidget(tabs);
}
