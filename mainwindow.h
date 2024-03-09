#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

// Defina uma estrutura para representar um chat
struct QuestionAnswer {
    QString question;
    QString answer;
    QString image;
};

// Defina uma estrutura para representar uma conversa
struct Conversation {
    QString id;
    QString date;
    QString title;
    QList<std::shared_ptr<QuestionAnswer>> questionAnswerList;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonSendQuestion_clicked();
    void on_networkRequestFinished(QNetworkReply *reply);
    void on_treeViewChats_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QString apiKey;
    std::shared_ptr<QuestionAnswer> lastQuestionAnswer;
    QList<std::shared_ptr<Conversation>> conversations;
    std::shared_ptr<Conversation> currentConversation;
    void addChatItem();
    void fillConversationTreeView();
    void loadConversations(const QString &filename);
    void saveConversations(const QString &filename);
    void createTodayConversationIfNotExists();
    void addQuestionAnswerWidget(QString question, QString answer);
    void fillChatListWidget();
};
#endif // MAINWINDOW_H
