#include <QTabWidget>

#include "ObjectSet.h"

#ifndef INFOBOX_H
#define INFOBOX_H

class InfoBox : public QTabWidget
{
    Q_OBJECT

public:
    InfoBox(ObjectSet *objectSet, QWidget *parent = NULL);
    ~InfoBox() { }

private:
    ObjectSet *_objectSet;
};

#endif /* INFOBOX_H */
