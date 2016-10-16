#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QTreeWidget>

class LogViewer : public QTreeWidget
{
    Q_OBJECT
public:
    LogViewer(QWidget * parent = 0);
};

#endif
