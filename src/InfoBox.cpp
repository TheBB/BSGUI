#include <QTabWidget>

#include "InfoBox.h"

InfoBox::InfoBox(ObjectSet *objectSet, QWidget *parent)
    : QTabWidget(parent)
    , _objectSet(objectSet)
{
    installEventFilter(parent);

    addTab(new QWidget(), "Selection");
    addTab(new QWidget(), "Pick");
    addTab(new QWidget(), "Files");
    addTab(new QWidget(), "Log");
}
