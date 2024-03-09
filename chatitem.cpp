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
    ui->textBrowserQuestion->setMarkdown(question);
    ui->textBrowserAnswer->setMarkdown(answer);
}

ChatItem::~ChatItem()
{
    delete ui;
}
