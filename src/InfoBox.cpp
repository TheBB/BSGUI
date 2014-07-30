#include "InfoBox.h"

InfoBox::InfoBox(ObjectSet *objectSet,
                 const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    installEventFilter(parent);
}
