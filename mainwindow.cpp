#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardItemModel>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonSendQuestion_clicked()
{
    // adicionar icone e texto para ui->listWidgetChat
    ui->listWidgetChat->addItem("Você: " + ui->textEditQuestion->toPlainText());
    ui->listWidgetChat->addItem("Bot: Resposta do bot");
    ui->listWidgetChat->scrollToBottom();
    ui->textEditQuestion->clear();

    addChatItem();
}

void MainWindow::addChatItem()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(ui->listViewChats->model());
    if (!model) {
        model = new QStandardItemModel(this);
        ui->listViewChats->setModel(model);
    }

    QString currentDate = QDateTime::currentDateTime().toString("dd/MM/yyyy");
    QStandardItem *item;

    // Check if the date already exists in the list
    bool dateExists = false;
    for(int i = 0; i < model->rowCount(); i++) {
        item = model->item(i);
        if(item->text() == currentDate) {
            dateExists = true;
            break;
        }
    }

    // If the date does not exist, add it
    if(!dateExists) {
        item = new QStandardItem(QIcon(":/new/images/resources/images/send.png"), currentDate);
        model->appendRow(item);
    }


    // If no child is selected, add a new child
    if(!item->hasChildren()) {
        QStandardItem *childItem = new QStandardItem(QIcon(":/new/images/resources/images/request.png"), "Nova conversa");
        item->appendRow(childItem);
    }

}

