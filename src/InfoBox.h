#include <QDockWidget>

#include "ObjectSet.h"

#ifndef INFOBOX_H
#define INFOBOX_H

class InfoBox : public QDockWidget
{
    Q_OBJECT

public:
    InfoBox(ObjectSet *objectSet,
            const QString &title, QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~InfoBox() { }
};

#endif /* INFOBOX_H */
