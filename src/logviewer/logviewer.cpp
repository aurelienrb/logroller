#include "logviewer.h"

LogViewer::LogViewer(QWidget * parent) : QTreeWidget(parent)
{
    setColumnCount(3);

    QColor color1(0xFC, 0xE9, 0x4F);
    QColor color2(0xFC, 0xAF, 0x3E);
    QColor color3(0x72, 0x9F, 0xCF);
    QColor color4(0x8A, 0xE2, 0x34);

    QTreeWidgetItem * mainItem = new QTreeWidgetItem(this);
    mainItem->setText(0, "main()");
    mainItem->setText(1, "info");
    mainItem->setText(2, "Hello World!");
    mainItem->setBackgroundColor(0, color2);
    mainItem->setBackgroundColor(1, color1);
    mainItem->setBackgroundColor(2, color1);

    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 5; ++i)
    {
        QStringList l;
        if (i % 2 == 0)
            l += "f()";
        else
            l += "";
        l << "trace" << "Hello World!";
        QTreeWidgetItem * item = new QTreeWidgetItem(mainItem, l);
        item->setBackgroundColor(0, color3);
        item->setBackgroundColor(1, color4);
        item->setBackgroundColor(2, color4);
        items.append(item);
    }
    insertTopLevelItems(0, items);
}