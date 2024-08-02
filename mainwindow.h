#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcombobox.h"
#include <QMainWindow>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QStandardItemModel>

// Defina uma estrutura para representar um chat
struct QuestionAnswer {
    QString question;
    QString answer;
    QString image;
};

// Defina uma estrutura para representar uma conversa
struct Conversation {
    QString id;
    QString title;
    QList<std::shared_ptr<QuestionAnswer>> questionAnswerList;
};

struct HistoryDate
{
    QString date;
    QList<std::shared_ptr<Conversation>> conversationList;
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
    ~MainWindow() override;

private slots:
    void on_networkRequestFinished(QNetworkReply *reply);
    void onTreeViewChatsClicked(const QModelIndex &index);
    void on_actionSair_triggered();
    void on_commandLinkButtonNewChat_clicked();
    void on_actionCreditos_triggered();
    void on_actionCriar_API_Key_triggered();
    void on_treeViewChats_doubleClicked(const QModelIndex &index);
    void on_treeViewChats_customContextMenuRequested(const QPoint &pos);
    void onItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_labelImage_clicked();
    void on_toolButtonSendQuestion_clicked();

private:
    Ui::MainWindow *ui;
    QString apiKey;
    QString gptModel;
    std::shared_ptr<QuestionAnswer> lastQuestionAnswer;
    QList<std::shared_ptr<HistoryDate>> historyDateList;
    std::shared_ptr<Conversation> currentConversation;
    QProgressDialog *progressDialog = nullptr;
    void fillModelList();
    void addChatItem();
    void fillConversationTreeView();
    void loadConversations(const QString &filename);
    void saveConversations(const QString &filename);
    void createTodayConversationIfNotExists();
    void showProgressDialog(const QString &text);
    void hideProgressDialog();
    void lockUi(bool lock);
    void fillChatListWidget(std::shared_ptr<Conversation> conversation);
    std::shared_ptr<Conversation> locateConversation(QStandardItem *item);
    void closeEvent(QCloseEvent *event) override;
    void deleteChat();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resetImage();
    void setComboBoxToId(QComboBox *comboBox, QString id);
};
#endif // MAINWINDOW_H
