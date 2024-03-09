#ifndef CHATITEM_H
#define CHATITEM_H

#include <QWidget>

namespace Ui {
class ChatItem;
}

class ChatItem : public QWidget
{
    Q_OBJECT

public:
    explicit ChatItem(QWidget *parent = nullptr);
    void setQuestionAnswer(const QString &question, const QString &answer);
    ~ChatItem();

private:
    Ui::ChatItem *ui;
};

#endif // CHATITEM_H
