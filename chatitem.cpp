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
    // setMarkdown => question em negrigo, quebra de linha e resposta
    QString questionAnswer = QString("**%1**\n\n%2").arg(question).arg(answer);
    ui->textBrowserQuestion->setMarkdown(questionAnswer);
}

ChatItem::~ChatItem()
{
    delete ui;
}
