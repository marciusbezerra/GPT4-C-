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
        conversationsGroupedByDate[conversation->date].append(*conversation);
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
    auto question = ui->textEditQuestion->toPlainText();

    if(apiKey.isEmpty()) {
        QMessageBox::warning(this, "API Key não informada", "Informe a API Key para continuar");
        statusBar()->showMessage("API Key não informada", 3000);
        return;
    }

    if(question.isEmpty()) {
        QMessageBox::warning(this, "Pergunta não informada", "Digite uma pergunta para continuar");
        statusBar()->showMessage("Digite uma pergunta", 3000);
        return;
    }

    lastQuestionAnswer = std::make_shared<QuestionAnswer>();
    lastQuestionAnswer->question = question;
    lastQuestionAnswer->answer = "";
    lastQuestionAnswer->image = "";

    createTodayConversationIfNotExists();
    currentConversation->questionAnswerList.append(lastQuestionAnswer);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::on_networkRequestFinished);

    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    // QNetworkRequest request(QUrl("https://webhook.site/a5457910-7a1e-4168-9ceb-b02e8039ca24"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonObject body;
    QJsonArray messagesArray;

    for (const auto &questionAnswer : currentConversation->questionAnswerList) {
        QJsonObject chatObject;
        chatObject["role"] = "user";
        chatObject["content"] = questionAnswer->question;
        if (!questionAnswer->answer.isEmpty()) {
            chatObject["role"] = "assistant";
            chatObject["content"] = questionAnswer->answer;
        }
        if (!questionAnswer->image.isEmpty()) {
            chatObject["image"] = questionAnswer->image;
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

    lastQuestionAnswer->answer = answer;

    fillChatListWidget();
    saveConversations("conversations.json");
}

void MainWindow::createTodayConversationIfNotExists() {

    if (!currentConversation) {

        // se não existe nenhuma conversa, para a data de hoje...
        for (const auto &conversation : conversations) {
            if (conversation->date == QDateTime::currentDateTime().toString("dd/MM/yyyy")) {
                currentConversation = conversation;
                break;
            }
        }

        // se não existe nenhuma conversa para a data de hoje, cria uma nova
        if (!currentConversation) {
            currentConversation = std::make_shared<Conversation>();
            currentConversation->id = QUuid::createUuid().toString();
            currentConversation->date = QDateTime::currentDateTime().toString("dd/MM/yyyy");
            currentConversation->title = "Conversa #00001";
            currentConversation->questionAnswerList = QList<std::shared_ptr<QuestionAnswer>>();
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

    conversations = QList<std::shared_ptr<Conversation>>();

    for (int i = 0; i < array.size(); ++i) {
        QJsonObject conversationObject = array[i].toObject();
        auto conversation = std::make_shared<Conversation>();
        conversation->id = conversationObject["id"].toString();
        conversation->date = conversationObject["date"].toString();
        conversation->title = conversationObject["title"].toString();

        QJsonArray chatsArray = conversationObject["questionAnswerList"].toArray();
        for (int j = 0; j < chatsArray.size(); ++j) {
            QJsonObject chatObject = chatsArray[j].toObject();
            auto questionAnswer = std::make_shared<QuestionAnswer>();
            questionAnswer->question = chatObject["question"].toString();
            questionAnswer->answer = chatObject["answer"].toString();
            questionAnswer->image = chatObject["image"].toString();
            conversation->questionAnswerList.append(questionAnswer);
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
        conversationObject["id"] = conversation->id;
        conversationObject["date"] = conversation->date;
        conversationObject["title"] = conversation->title;

        QJsonArray chatsArray;
        for (const auto &questionAnswer : conversation->questionAnswerList) {
            QJsonObject chatObject;
            chatObject["question"] = questionAnswer->question;
            chatObject["answer"] = questionAnswer->answer;
            chatObject["image"] = questionAnswer->image;
            chatsArray.append(chatObject);
        }

        conversationObject["questionAnswerList"] = chatsArray;
        data.append(conversationObject);
    }

    QJsonDocument doc(data);
    file.write(doc.toJson());
}

void MainWindow::on_treeViewChats_clicked(const QModelIndex &index)
{
    if (index.parent().isValid()) {
        QString conversationId = index.data(Qt::UserRole).toString();
        for (const auto &conversation : conversations) {
            if (conversation->id == conversationId) {
                currentConversation = conversation;
                break;
            }
        }
        fillChatListWidget();
    }
}

void MainWindow::fillChatListWidget()
{
    ui->textBrowserQuestionAnswers->clear();
    if (currentConversation) {
        for (const auto &questionAnswer : currentConversation->questionAnswerList) {
            QString formattedText = QString("<b>Você:</b> %1<br><br><b>GPT:</b> %2<br><hr>")
                .arg(questionAnswer->question, questionAnswer->answer);
            ui->textBrowserQuestionAnswers->append(formattedText);
        }

        // ui->textBrowserQuestionAnswers->setMarkdown(ui->textBrowserQuestionAnswers->toPlainText());

        ui->textBrowserQuestionAnswers->moveCursor(QTextCursor::End);
        ui->textBrowserQuestionAnswers->ensureCursorVisible();
    }
}
