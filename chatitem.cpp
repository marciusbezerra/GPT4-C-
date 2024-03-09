#include "chatitem.h"
#include "ui_chatitem.h"

ChatItem::ChatItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatItem)
{
    ui->setupUi(this);
}

void ChatItem::setQuestionAnswer(const QString &question, const QString &answer)
{
    QString formattedText = QString("<html><body>"
                                    "<p><b>Você:</b> %1</p>"
                                    "<p><b>GPT:</b> %2</p>"
                                    "</body></html>").arg(question, answer);
    ui->labelQuestionAnswer->setText(formattedText);
    ui->labelQuestionAnswer->adjustSize();
}

ChatItem::~ChatItem()
{
    delete ui;
}
