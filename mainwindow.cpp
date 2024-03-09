#include "chatitem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardItemModel>
#include <QDateTime>
#include <QMessageBox>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextDocument>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadConversations("conversations.json");
    fillConversationTreeView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillConversationTreeView() {
    QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(ui->treeViewChats->model());
    if (!model) {
        model = new QStandardItemModel(this);
        ui->treeViewChats->setModel(model);
    }

    QStandardItem *lastItem = nullptr;
    QStandardItem *lastChildItem = nullptr;

    // Agrupe as conversas por data
    QMap<QString, QList<Conversation>> conversationsGroupedByDate;
    for (const auto &conversation : conversations) {
        conversationsGroupedByDate[conversation.date].append(conversation);
    }

    // Preencha a árvore com as conversas agrupadas por data
    for (auto it = conversationsGroupedByDate.begin(); it != conversationsGroupedByDate.end(); ++it) {
        QStandardItem *lastItem = new QStandardItem(QIcon(":/new/images/resources/images/send.png"), it.key());
        model->appendRow(lastItem);

        for (const auto &conversation : it.value()) {
            QStandardItem *conversationItem = new QStandardItem(QIcon(":/new/images/resources/images/request.png"), conversation.title);
            conversationItem->setData(conversation.id, Qt::UserRole);
            lastItem->appendRow(conversationItem);
            lastChildItem = conversationItem;
        }
    }

    if (lastItem && lastChildItem) {
        ui->treeViewChats->expand(lastItem->index());
        ui->treeViewChats->setCurrentIndex(lastChildItem->index());
    }
}

void MainWindow::on_pushButtonSendQuestion_clicked()
{
    apiKey = ui->lineEditApiKey->text();
    lastChat.question = ui->textEditQuestion->toPlainText();
    lastChat.answer = "";
    lastChat.image = "";

    if(apiKey.isEmpty()) {
        QMessageBox::warning(this, "API Key não informada", "Informe a API Key para continuar");
        statusBar()->showMessage("API Key não informada", 3000);
        return;
    }

    if(lastChat.question.isEmpty()) {
        QMessageBox::warning(this, "Pergunta não informada", "Digite uma pergunta para continuar");
        statusBar()->showMessage("Digite uma pergunta", 3000);
        return;
    }

    createTodayConversationIfNotExists();
    currentConversation.chats.append(lastChat);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::on_networkRequestFinished);

    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    // QNetworkRequest request(QUrl("https://webhook.site/a5457910-7a1e-4168-9ceb-b02e8039ca24"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonObject body;
    QJsonArray messagesArray;

    for (const auto &chat : currentConversation.chats) {
        QJsonObject chatObject;
        chatObject["role"] = "user";
        chatObject["content"] = chat.question;
        if (!chat.answer.isEmpty()) {
            chatObject["role"] = "assistant";
            chatObject["content"] = chat.answer;
        }
        if (!chat.image.isEmpty()) {
            chatObject["image"] = chat.image;
        }
        messagesArray.append(chatObject);
    }

    body["messages"] = messagesArray;
    //body["model"] = "gpt-4-turbo-preview";
    body["model"] = "gpt-3.5-turbo";
    body["temperature"] = 0.5;
    body["max_tokens"] = 4000;

    manager->post(request, QJsonDocument(body).toJson());
}

void MainWindow::on_networkRequestFinished(QNetworkReply *reply)
{

    if(reply->error() != QNetworkReply::NoError) {
        auto error = reply->readAll();
        if (error.startsWith("{")) {
            QJsonDocument docJson = QJsonDocument::fromJson(error);
            QJsonObject obj = docJson.object();
            QString message = obj["error"].toObject()["message"].toString();
            QMessageBox::warning(this, "Erro na requisição", message);
            return;
        }
        QMessageBox::warning(this, "Erro na requisição", "Erro ao enviar a pergunta: " + reply->errorString());
        return;
    }

    QJsonDocument docJson = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = docJson.object();
    QString answer = obj["choices"].toArray().first().toObject()["message"].toObject()["content"].toString();

    // QTextDocument doc;
    // doc.setMarkdown(answer);
    // QString html = doc.toHtml();

    lastChat.answer = answer;

    addChat(lastChat.question, lastChat.answer);
    conversations.append(currentConversation);
    saveConversations("conversations.json");
}

void MainWindow::addChat(QString question, QString answer) {
    ChatItem *item = new ChatItem(this);
    auto *witem = new QListWidgetItem();
    witem->setSizeHint(item->sizeHint());
    // witem->setSizeHint(QSize(891, 90));
    ui->listWidgetChat->addItem(witem);
    ui->listWidgetChat->setItemWidget(witem, item);
    item->setQuestionAnswer(question, answer);
}

void MainWindow::createTodayConversationIfNotExists() {

    if (currentConversation.id.isEmpty()) {

        // se não existe nenhuma conversa, para a data de hoje...
        for (const auto &conversation : conversations) {
            if (conversation.date == QDateTime::currentDateTime().toString("dd/MM/yyyy")) {
                currentConversation = conversation;
                break;
            }
        }

        // se não existe nenhuma conversa para a data de hoje, cria uma nova
        if (currentConversation.id.isEmpty()) {
            currentConversation.id = QUuid::createUuid().toString();
            currentConversation.date = QDateTime::currentDateTime().toString("dd/MM/yyyy");
            currentConversation.title = "Conversa #00001";
            // currentConversation.chats = {};
            conversations.append(currentConversation);
            fillConversationTreeView();
        }
    }

}

void MainWindow::loadConversations(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Não foi possível abrir o arquivo para leitura.");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonArray array = doc.array();

    conversations.clear();
    for (int i = 0; i < array.size(); ++i) {
        QJsonObject conversationObject = array[i].toObject();
        Conversation conversation;
        conversation.id = conversationObject["id"].toString();
        conversation.date = conversationObject["date"].toString();
        conversation.title = conversationObject["title"].toString();

        QJsonArray chatsArray = conversationObject["chats"].toArray();
        for (int j = 0; j < chatsArray.size(); ++j) {
            QJsonObject chatObject = chatsArray[j].toObject();
            Chat chat;
            chat.question = chatObject["question"].toString();
            chat.answer = chatObject["answer"].toString();
            chat.image = chatObject["image"].toString();
            conversation.chats.append(chat);
        }

        conversations.append(conversation);
    }
}

void MainWindow::saveConversations(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Não foi possível abrir o arquivo para escrita.");
        return;
    }

    QJsonArray data;
    for (const auto &conversation : conversations) {
        QJsonObject conversationObject;
        conversationObject["id"] = conversation.id;
        conversationObject["date"] = conversation.date;
        conversationObject["title"] = conversation.title;

        QJsonArray chatsArray;
        for (const auto &chat : conversation.chats) {
            QJsonObject chatObject;
            chatObject["question"] = chat.question;
            chatObject["answer"] = chat.answer;
            chatObject["image"] = chat.image;
            chatsArray.append(chatObject);
        }

        conversationObject["chats"] = chatsArray;
        data.append(conversationObject);
    }

    QJsonDocument doc(data);
    file.write(doc.toJson());
}

void MainWindow::on_treeViewChats_clicked(const QModelIndex &index)
{
    if (index.parent().isValid()) {
        QString conversationId = index.data().toString();
        for (const auto &conversation : conversations) {
            if (conversation.id == conversationId) {
                currentConversation = conversation;
                break;
            }
        }
        fillChatListWidget();
    }
}

void MainWindow::fillChatListWidget()
{
    ui->listWidgetChat->clear();
    for (const auto &chat : currentConversation.chats) {
        addChat(chat.question, chat.answer);
    }
}
